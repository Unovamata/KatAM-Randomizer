/* Starman; ported from Kirby: Squeak Squad (USA), enemy 0x26 (its body 0x020877B8, vtable 0x0216F1F4; brain 0x02084B10), as
 * SQUEAK_STARMAN. Values from param/prmCE20Ml0.kgb (movement; Ml1, the spawn's second parameter byte 1, the same; MlG differs only in
 * the other mode), prmCE20Pl0.kgb (brain) and prmCE20Sl0.kgb (HP 10); speeds 20.12 a frame, Y up; its boxes from enemy/starman.rca
 * (its animations from the .rca's second). A caped star man who walks, flaps up, glides down, rockets up and dives.
 * • Brain (the spawn's first parameter byte, KatAM's template unk12; each first faces Kirby, past 1 pixel):
 *   0 (0x02084820): forward, B held while it's under 12 pixels over where it started (0x020847B8): it hovers there, flying on;
 *   1 (0x02084788): walks (back at a wall) till Kirby is within 96 pixels across and up to 192 over it (0x020846DC); then flies at
 *     him, B held, 240 frames (0x020845C0); right over him (4 pixels) near the screen's top it lets go and dives (down, 16 frames
 *     once landed), or at a ceiling dives harder (down 2); after its time it lands and waits 52 frames (0x0208444C) and goes again;
 *     after the second time it flies away up through everything (up 2, 0x020843C8);
 *   2 (0x02084A40): walks 120, 180, 240, 300, 360 or 420 frames (random 0-99 by 20, 20, 20, 10, 10, 10; back at a wall), then
 *     presses up and B 34 frames (a rocket: but its lift-off takes 38, so it flaps with B let go and skids, as on the DS), lands,
 *     waits 16 frames and starts over;
 *   3 (0x02084370): stands.
 * • Body (vtable +0x40, 0x02084C20: variant 0 passes through the terrain, held in the stage): it stands or walks (anim 1, 0.8);
 *   B lifts it off (anim 2, 38 frames) and it flaps (anim 3): B held it rises (0.05 a frame) to 1.8, let go it sinks to 0.8, never
 *   over the screen's top, flying forward (0.04 a frame to 0.8; its pad let go, 0.02); landing it skids (anim 9, 0.04 a frame,
 *   uphill 0.06) and stands; up rockets it (anim 4, 0.1 a frame to 2.4) and, let go, slows to fall at 1.8 and dives; down dives
 *   (anim 5, 0.1 a frame to 3.0; down 2: its speed up turned down) and bounces off the ground (anim 6, 2.0 up, then 7 falling, 8
 *   landing); forward from a dive or a rocket it glides (anim 3, 0.8 down); up 2 climbs up through everything (anim 4) for good;
 *   walking off a ledge it falls (forward: glides, 1.8 down at first).
 * Kirby's inhale takes it as any enemy.
 * Not ported: its ability (Squeak Squad's: KatAM's own Hi-Jump is the NiDL Starman's; the owner's call), its sounds, its other mode
 * (the brain's +8 not 3: A jumps, 2.5 up), its despawn once it has left the screen (KatAM's own), its contact damage (KatAM's 1).
 * Squeak Squad's slope kinds under its skid (the stage's vtable +0x30: 1-2 and 14-15) are read here from KatAM's floor 4 pixels
 * ahead (uphill: KatAM's floor a pixel over its feet there). */
#include "../squeak.h"
#include "main.h"
#include "random.h"

// Animations (starman.cea);
enum { ANIM_0, ANIM_WALK, ANIM_LIFT, ANIM_FLAP, ANIM_RISE, ANIM_DIVE, ANIM_BOUNCE, ANIM_BOUNCE_FALL, ANIM_LAND, ANIM_SKID,
       ANIM_HURT, ANIM_HURT_2 };

// prmCE20Ml0.kgb;
#define WALK_SPEED 0xCCD
#define JUMP_SPEED 0x2800
#define GRAVITY 0x19A
#define SKID_SLOW 0xA4
#define SKID_SLOW_UPHILL 0xF6
#define AIR_SPEED 0xCCD
#define FLY_ACCEL 0xA4
#define FLY_ACCEL_IDLE 0x52
#define FLY_LIFT 0xCD
#define MAX_FALL 0x2667
#define FLY_SPEED 0xCCD
#define FLY_RISE 0x1CCD
#define GLIDE_START 0x1CCD
#define GLIDE_LIFT 0x29
#define SINK_SPEED 0xCCD
#define ROCKET_ACCEL 0x19A
#define ROCKET_SPEED 0x2667
#define DIVE_SPEED 0x3000
#define SLOW_X 0x19A
#define BOUNCE_SPEED 0x2000
// prmCE20Pl0.kgb;
#define WALK_TIMES 6
#define ROCKET_TIME 0x22
#define LANDED_WAIT 0x10
#define HOVER_HEIGHT 0xC000
#define CHASE_ACROSS 0x60000
#define CHASE_UP 0xC0000
#define CHASE_TIME 0xF0
#define OVER_KIRBY 0x4000
#define DIVE_WAIT 0x10
#define CHASES 2
#define CHASE_REST 0x34
// The code's;
#define FACE_DISTANCE 0x1000
#define NEAR_TOP 0x8000                 // Near the screen's top: under 8 pixels below it;
#define KIRBY_FEET 0x7000               // KatAM's Kirby's spot is 7 pixels over his feet (Squeak Squad's at them);
#define PAD_DIVE_HARD -2
#define PAD_CLIMB 2

enum {
    BODY_START, BODY_STAND, BODY_WALK_ENTRY, BODY_WALK, BODY_LIFT_ENTRY, BODY_LIFT, BODY_FLAP_ENTRY, BODY_FLAP, BODY_FLY_ENTRY,
    BODY_FLY, BODY_SLOW_RISE_ENTRY, BODY_SLOW_RISE, BODY_RISE_ENTRY, BODY_RISE, BODY_DIVE_ENTRY, BODY_DIVE, BODY_DIVE_HARD_ENTRY,
    BODY_DIVE_HARD, BODY_BOUNCE_ENTRY, BODY_BOUNCE, BODY_BOUNCE_FALL, BODY_LAND, BODY_CLIMB_ENTRY, BODY_CLIMB, BODY_SKID_ENTRY,
    BODY_SKID, BODY_GLIDE_ENTRY, BODY_GLIDE, BODY_FALL_ENTRY, BODY_FALL, BODY_JUMP_ENTRY, BODY_JUMP,
};
enum {
    BRAIN_HOVER_START, BRAIN_HOVER_PRESS, BRAIN_HOVER_FORWARD, BRAIN_HOVER, BRAIN_CHASE_START, BRAIN_WALK_FACE, BRAIN_WALK,
    BRAIN_CHASE, BRAIN_DIVE, BRAIN_DIVE_HARD, BRAIN_CHASE_LANDED, BRAIN_CHASE_REST, BRAIN_LEAVE, BRAIN_LEAVE_UP, BRAIN_LEAVE_PRESS,
    BRAIN_LEAVING, BRAIN_PATROL_START, BRAIN_PATROL_PICK, BRAIN_PATROL, BRAIN_ROCKET, BRAIN_ROCKET_LANDED, BRAIN_STAND_START, BRAIN_STAND,
};

#define OLD_X(s) ((s)->vars[0])         // 0x1FC: its x last frame (the fly control's: no move, it stops);
#define BRAIN_WAIT(s) ((s)->vars[1])    // The brain's 0x98;
#define BRAIN_TIME(s) ((s)->vars[2])    // The brain's 0xA0;
#define CHASED(s) ((s)->vars[3])        // The brain's 0xA8: chases so far;
#define START_Y(s) ((s)->vars[4])       // The brain's 0xB0: where it started (Squeak Squad's y, up);
#define HARM(s) ((s)->vars[5])          // KatAM's unk68 from InitObject;
#define BOX(s) (*(u8 *)&(s)->vars[6])   // This frame's box;

/* Its boxes (starman.rca: the .rca's animation k + 1 is the .cea's k; boxes.py); */
static const struct SqueakBox sStarBoxes[] = {
    { { -8, -17, 8, -1 }, TRUE, 0 },
    { { -9, -16, 7, 0 }, TRUE, 0 },
    { { -9, -17, 7, -1 }, TRUE, 0 },
    { { -8, -17, 8, -1 }, FALSE, 0 },
};
static const struct SqueakBoxFrame sStarAnim0[] = { { 1, 0 } };
static const struct SqueakBoxFrame sStarAnim1[] = { { 52, 0 } };
static const struct SqueakBoxFrame sStarAnim2[] = { { 38, 1 } };
static const struct SqueakBoxFrame sStarAnim3[] = { { 18, 0 } };
static const struct SqueakBoxFrame sStarAnim4[] = { { 1, 2 }, { 3, 2 } };
static const struct SqueakBoxFrame sStarAnim5[] = { { 1, 2 } };
static const struct SqueakBoxFrame sStarAnim6[] = { { 4, 2 } };
static const struct SqueakBoxFrame sStarAnim7[] = { { 8, 2 } };
static const struct SqueakBoxFrame sStarAnim8[] = { { 4, 2 } };
static const struct SqueakBoxFrame sStarAnim9[] = { { 1, 0 } };
static const struct SqueakBoxFrame sStarAnim10[] = { { 1, 3 } };
static const struct SqueakBoxFrame sStarAnim11[] = { { 1, 3 } };
static const struct SqueakBoxAnim sStarBoxAnims[] = {
    { sStarAnim0, 1 },
    { sStarAnim1, 1 },
    { sStarAnim2, 1 },
    { sStarAnim3, 1 },
    { sStarAnim4, 2 },
    { sStarAnim5, 1 },
    { sStarAnim6, 1 },
    { sStarAnim7, 1 },
    { sStarAnim8, 1 },
    { sStarAnim9, 1 },
    { sStarAnim10, 1 },
    { sStarAnim11, 1 },
};
static const struct SqueakBoxes sStarBoxData = { sStarBoxes, sStarBoxAnims, 12 };

// Random(); Squeak Squad's random 0-99 (KatAM's numbers);
static u32 Random(void) {
    return Rand16() * 100 >> 16;
}

// KirbyX(), KirbyY(); The Kirby it goes by (his feet, Squeak Squad's units);
static s32 KirbyX(struct SqueakObject *s) {
    return SQ_X(s->obj.kirby3);
}
static s32 KirbyY(struct SqueakObject *s) {
    return SQ_Y(s->obj.kirby3) - KIRBY_FEET;
}

// ScreenTop(); The screen's top (its camera's: the stage object's +0x70, 0x0220A3E4), KatAM's y;
static s32 ScreenTop(struct SqueakObject *s) {
    return gCurLevelInfo[s->obj.base.unk56].viewportPosition.y;
}

// PressedB(); The body's B&!A (its brain's +0x20 set, +0x38 not: this brain never presses A);
#define PRESSED_B(s) ((s)->padB != 0)

// Zero(); No caps, accelerations nor friction (its speeds stay);
static void Zero(struct SqueakObject *s) {
    s->maxVx = s->maxVy = s->ax = s->ay = s->fx = s->fy = 0;
}

// FlyControl(); 0x02087538: blocked (its x as last frame's), it stops; forward speeds it up (FLY_ACCEL; its pad let go, FLY_ACCEL_IDLE);
static void FlyControl(struct SqueakObject *s) {
    if (OLD_X(s) == s->obj.base.x) s->vx = s->ax = 0;
    OLD_X(s) = s->obj.base.x;
    if (s->padDir > 0) s->ax = FLY_ACCEL;
    if (s->padDir == 0 && s->vx > 0) s->ax = FLY_ACCEL_IDLE;
    if (s->padDir < 0) SqueakTurnAround(s);
}

// AirControl(); 0x020875F8: forward, AIR_SPEED; its pad let go, still across;
static void AirControl(struct SqueakObject *s) {
    if (s->padDir > 0) s->vx = AIR_SPEED;
    if (s->padDir < 0) s->vx = -s->vx;
    if (s->padDir == 0 && s->vx != 0) s->vx = s->ax = 0;
}

// SlowX(); Its speed across toward 0 by SLOW_X;
static void SlowX(struct SqueakObject *s) {
    if (s->vx > 0 && (s->vx -= SLOW_X) < 0) s->vx = 0;
    if (s->vx < 0 && (s->vx += SLOW_X) > 0) s->vx = 0;
}

// Steer(); The flying states' pad, its direction let go: up rockets, down dives (down 2 harder);
static bool32 Steer(struct SqueakObject *s) {
    if (s->padDir != 0) return FALSE;
    if (s->padY > 0) SqueakSetState(s, BODY_RISE_ENTRY);
    else if (s->padY == PAD_DIVE_HARD) SqueakSetState(s, BODY_DIVE_HARD_ENTRY);
    else if (s->padY < 0) SqueakSetState(s, BODY_DIVE_ENTRY);
    else return FALSE;
    return TRUE;
}

// FlapStart(); B (with A let go) starts it flapping (from the walk, the lift off first), turned round if its pad says so;
static bool32 FlapStart(struct SqueakObject *s, u8 next) {
    if (!PRESSED_B(s)) return FALSE;
    if (s->padDir < 0) SqueakTurnAround(s);
    SqueakSetState(s, next);
    return TRUE;
}

// FlyEntry(); The flapping states' start: animation 3, no caps, rising or sinking at `ay`, forward to FLY_SPEED;
static bool32 FlyEntry(struct SqueakObject *s, s32 ay, u8 next) {
    SqueakPlay(s, ANIM_FLAP);
    Zero(s);
    s->ay = ay;
    s->maxVx = FLY_SPEED;
    s->ax = FLY_ACCEL;
    SqueakSetState(s, next);
    return TRUE;
}

/* DiveEntry(); 0x020855C8, 0x02085290: animation 5, falling ROCKET_ACCEL a frame (its cap, -DIVE_SPEED there, left to the dive's own
 * clamp to DIVE_SPEED); */
static bool32 DiveEntry(struct SqueakObject *s, u8 next) {
    SqueakPlay(s, ANIM_DIVE);
    s->maxVx = s->maxVy = s->fx = s->fy = s->ax = 0;
    s->ay = -ROCKET_ACCEL;
    SqueakSetState(s, next);
    return TRUE;
}

// Uphill(); Squeak Squad's slope kinds facing up them (KatAM's floor a pixel over its feet 4 pixels ahead);
static bool32 Uphill(struct SqueakObject *s) {
    struct ObjectBase *b = &s->obj.base;

    return NidlIsFloor(&s->obj, b->x + (b->flags & 1 ? -0x400 : 0x400), b->y - 0x100);
}

static bool32 Body(struct SqueakObject *s) {
    switch (s->state) {
    case BODY_START:        // 0x0208744C (vtable +0x3C);
        if (s->variant == 0) {
            SqueakSetState(s, BODY_FLY_ENTRY);
            return TRUE;
        }
        SqueakPlay(s, ANIM_WALK);
        SqueakStop(s);
        SqueakSetState(s, BODY_STAND);
        return TRUE;
    case BODY_STAND:        // 0x02087328;
        if (FlapStart(s, BODY_LIFT_ENTRY)) return TRUE;
        if (s->padDir == 0) {
            SqueakMove(s);
            if (!SqueakOnGround(s)) SqueakSetState(s, BODY_FALL_ENTRY);
            break;
        }
        SqueakSetState(s, BODY_WALK_ENTRY);
        return TRUE;
    case BODY_WALK_ENTRY:   // 0x02087280;
        SqueakPlay(s, ANIM_WALK);
        SqueakStop(s);
        SqueakSetState(s, BODY_WALK);
        return TRUE;
    case BODY_WALK:         // 0x020870C4;
        if (FlapStart(s, BODY_LIFT_ENTRY)) return TRUE;
        if (s->padDir == 0 && s->padY == 0) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        s->vx = WALK_SPEED;
        if (s->padDir < 0) SqueakTurnAround(s);
        SqueakMove(s);
        if (!SqueakOnGround(s)) {
            if (s->padDir == 0) {
                SqueakSetState(s, BODY_FALL_ENTRY);
            } else {
                s->vy = -GLIDE_START;
                SqueakSetState(s, BODY_GLIDE_ENTRY);
            }
        }
        break;
    case BODY_LIFT_ENTRY:   // 0x02084CD0;
        SqueakPlay(s, ANIM_LIFT);
        SqueakSetState(s, BODY_LIFT);
        return TRUE;
    case BODY_LIFT:         // 0x02084C60;
        if (s->animEnded) {
            SqueakSetState(s, BODY_FLAP_ENTRY);
            return TRUE;
        }
        break;
    case BODY_FLAP_ENTRY:   // 0x02086450;
        if (s->vy > FLY_RISE) {
            SqueakPlay(s, ANIM_FLAP);
            SqueakSetState(s, BODY_SLOW_RISE_ENTRY);
            return TRUE;
        }
        return FlyEntry(s, FLY_LIFT, BODY_FLAP);
    case BODY_FLAP:         // 0x020861E8: never over the screen's top;
        if (s->padY == PAD_CLIMB) {
            SqueakSetState(s, BODY_CLIMB_ENTRY);
            return TRUE;
        }
        if (Steer(s)) return TRUE;
        if (!s->padB) {
            if (s->vy <= -SINK_SPEED) {
                s->vy = -SINK_SPEED;
                s->ay = 0;
            } else {
                s->ay = -FLY_LIFT;
            }
            if (SqueakOnGround(s)) {
                s->vy = s->ay = s->ax = 0;
                SqueakSetState(s, BODY_SKID_ENTRY);
                return TRUE;
            }
        } else if (s->vy < FLY_RISE) {
            s->ay = FLY_LIFT;
        } else {
            s->vy = FLY_RISE;
            s->ay = 0;
        }
        FlyControl(s);
        SqueakMove(s);
        if (s->obj.base.y < ScreenTop(s)) s->obj.base.y = ScreenTop(s);
        break;
    case BODY_FLY_ENTRY:    // 0x020860AC (variant 0);
        return FlyEntry(s, FLY_LIFT, BODY_FLY);
    case BODY_FLY:          // 0x02085F90;
        if (!s->padB) {
            if (s->vy <= -FLY_RISE) {
                s->vy = -FLY_RISE;
                s->ay = 0;
            } else {
                s->ay = -FLY_LIFT;
            }
        } else if (s->vy < FLY_RISE) {
            s->ay = FLY_LIFT;
        } else {
            s->vy = FLY_RISE;
            s->ay = 0;
        }
        FlyControl(s);
        SqueakMove(s);
        break;
    case BODY_SLOW_RISE_ENTRY:  // 0x02085E50;
        return FlyEntry(s, -FLY_LIFT, BODY_SLOW_RISE);
    case BODY_SLOW_RISE:    // 0x02085CC0: rising faster than FLY_RISE, it slows to it and flaps;
        if (Steer(s)) return TRUE;
        if (s->vy > FLY_RISE) {
            FlyControl(s);
            SqueakMove(s);
            break;
        }
        s->vy = FLY_RISE;
        s->ay = 0;
        SqueakSetState(s, BODY_FLAP_ENTRY);
        return TRUE;
    case BODY_RISE_ENTRY:   // 0x02085BA4;
        SqueakPlay(s, ANIM_RISE);
        Zero(s);
        s->ay = ROCKET_ACCEL;
        s->maxVy = ROCKET_SPEED;
        SqueakSetState(s, BODY_RISE);
        return TRUE;
    case BODY_RISE:         // 0x02085894;
        if (s->padY < 1) {
            if (s->padY == PAD_DIVE_HARD || s->padY < 0) {
                SqueakSetState(s, s->padY == PAD_DIVE_HARD ? BODY_DIVE_HARD_ENTRY : BODY_DIVE_ENTRY);
                return TRUE;
            }
            s->ay = -FLY_LIFT;
            if (s->vy < -FLY_RISE) {
                s->vy = -FLY_RISE;
                SqueakSetState(s, BODY_DIVE_ENTRY);
                break;
            }
            if (SqueakOnGround(s)) {
                s->vy = s->ay = s->ax = 0;
                SqueakSetState(s, BODY_START);
                return TRUE;
            }
        } else {
            s->ay = ROCKET_ACCEL;
            s->maxVy = ROCKET_SPEED;
        }
        if (s->padY == 0 && s->padDir != 0) {
            SqueakSetState(s, BODY_FLAP_ENTRY);
            return TRUE;
        }
        SlowX(s);
        if (s->padDir < 0) SqueakTurnAround(s);
        AirControl(s);
        SqueakMove(s);
        if (OLD_X(s) == s->obj.base.x) s->vx = 0;
        OLD_X(s) = s->obj.base.x;
        break;
    case BODY_DIVE_ENTRY:   // 0x020855C8;
        return DiveEntry(s, BODY_DIVE);
    case BODY_DIVE_HARD_ENTRY:  // 0x02085290: its speed up turned down;
        s->vy = -s->vy;
        return DiveEntry(s, BODY_DIVE_HARD);
    case BODY_DIVE:         // 0x020853D0;
    case BODY_DIVE_HARD:    // 0x02085144;
        if (s->padY > 0) {
            SqueakSetState(s, BODY_RISE_ENTRY);
            return TRUE;
        }
        if (SqueakOnGround(s)) {
            s->vy = s->ay = s->ax = 0;
            SqueakSetState(s, BODY_BOUNCE_ENTRY);
            return TRUE;
        }
        if (s->vy < -DIVE_SPEED) {
            s->vy = -DIVE_SPEED;
            s->ay = 0;
        }
        if (s->padY == 0 && s->padDir != 0) {
            if (s->padDir < 0) SqueakTurnAround(s);
            SqueakSetState(s, BODY_GLIDE_ENTRY);
            return TRUE;
        }
        if (s->state == BODY_DIVE) {
            SlowX(s);
            if (s->padDir < 0) SqueakTurnAround(s);
            AirControl(s);
        } else if (s->padDir < 0) {
            SqueakTurnAround(s);
        }
        SqueakMove(s);
        break;
    case BODY_BOUNCE_ENTRY: // 0x02085010;
        SqueakPlay(s, ANIM_BOUNCE);
        Zero(s);
        s->vy = BOUNCE_SPEED;
        s->ay = -GRAVITY;
        SqueakSetState(s, BODY_BOUNCE);
        return TRUE;
    case BODY_BOUNCE:       // 0x02084F1C;
    case BODY_BOUNCE_FALL:  // 0x02084DFC;
        if (s->state == BODY_BOUNCE && s->vy < 0) {
            SqueakPlay(s, ANIM_BOUNCE_FALL);
            SqueakSetState(s, BODY_BOUNCE_FALL);
            return TRUE;
        }
        if (s->state == BODY_BOUNCE_FALL && SqueakOnGround(s)) {
            SqueakPlay(s, ANIM_LAND);
            s->vx = s->vy = s->ax = s->ay = 0;
            SqueakSetState(s, BODY_LAND);
            return TRUE;
        }
        if (SqueakWallAhead(s)) {
            s32 vx = s->vx;

            SqueakTurnAround(s);
            s->vx = vx;
        }
        SqueakMove(s);
        break;
    case BODY_LAND:         // 0x02084D84;
        if (s->animEnded) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        SqueakMove(s);
        break;
    case BODY_CLIMB_ENTRY:  // 0x0208576C: up through everything, for good;
        SqueakPlay(s, ANIM_RISE);
        SqueakThroughTerrain(s, TRUE);
        s->maxVx = s->maxVy = s->fx = s->fy = s->ax = 0;
        s->ay = ROCKET_ACCEL;
        s->maxVy = ROCKET_SPEED;
        SqueakSetState(s, BODY_CLIMB);
        return TRUE;
    case BODY_CLIMB:        // 0x020856EC;
        SqueakThroughTerrain(s, TRUE);
        if (s->padDir < 0) SqueakTurnAround(s);
        AirControl(s);
        SqueakMove(s);
        break;
    case BODY_SKID_ENTRY:   // 0x020867BC;
        SqueakPlay(s, ANIM_SKID);
        Zero(s);
        s->vy = 0;
        s->ax = -SKID_SLOW;
        SqueakSetState(s, BODY_SKID);
        return TRUE;
    case BODY_SKID:         // 0x020865D4;
        s->ax = Uphill(s) ? -SKID_SLOW_UPHILL : -SKID_SLOW;
        if (s->vx < 1) s->vx = s->ax = 0;
        if (s->vx != 0) {
            SqueakMove(s);
            if (!SqueakOnGround(s)) SqueakSetState(s, BODY_FALL_ENTRY);
            break;
        }
        SqueakSetState(s, BODY_START);
        return TRUE;
    case BODY_GLIDE_ENTRY:  // 0x02086AD8;
        return FlyEntry(s, GLIDE_LIFT, BODY_GLIDE);
    case BODY_GLIDE:        // 0x020868D0: down at SINK_SPEED;
        if (FlapStart(s, BODY_FLAP_ENTRY) || Steer(s)) return TRUE;
        FlyControl(s);
        if (s->vy >= -SINK_SPEED) {
            s->vy = -SINK_SPEED;
            s->ay = 0;
        }
        SqueakMove(s);
        if (SqueakOnGround(s)) {
            s->vy = s->ay = s->ax = 0;
            SqueakSetState(s, BODY_SKID_ENTRY);
            return TRUE;
        }
        break;
    case BODY_FALL_ENTRY:   // 0x02086DA8;
        SqueakPlay(s, ANIM_WALK);
        Zero(s);
        s->ay = -GRAVITY;
        SqueakSetState(s, BODY_FALL);
        return TRUE;
    case BODY_FALL:         // 0x02086C14;
        if (FlapStart(s, BODY_FLAP_ENTRY)) return TRUE;
        if (s->vy < -MAX_FALL) {
            s->vy = -MAX_FALL;
            s->ay = 0;
        }
        if (s->padDir < 0) SqueakTurnAround(s);
        AirControl(s);
        SqueakMove(s);
        if (SqueakOnGround(s)) {
            s->vy = s->ay = s->ax = 0;
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        break;
    case BODY_JUMP_ENTRY:   // 0x02086F90 (A: this brain never presses it);
        SqueakPlay(s, ANIM_WALK);
        Zero(s);
        s->vy = JUMP_SPEED;
        s->ay = -GRAVITY;
        SqueakSetState(s, BODY_JUMP);
        return TRUE;
    case BODY_JUMP:         // 0x02086EBC;
        if (FlapStart(s, BODY_FLAP_ENTRY)) return TRUE;
        if (s->padDir < 0) SqueakTurnAround(s);
        AirControl(s);
        SqueakMove(s);
        if (s->vy < 0) SqueakSetState(s, BODY_FALL_ENTRY);
        break;
    }
    return FALSE;
}

// FaceKirby(); 0x020879C0;
static void FaceKirby(struct SqueakObject *s) {
    SqueakFaceKirby(s, FACE_DISTANCE);
}

// ChaseOver(); 0x02084550's and 0x020844DC's end: chased CHASES times, it leaves; else it walks again;
static void ChaseOver(struct SqueakObject *s) {
    if (CHASED(s) >= CHASES) {
        s->brain = BRAIN_LEAVE;
        FaceKirby(s);
    } else {
        s->brain = BRAIN_WALK_FACE;
    }
}

static void Brain(struct SqueakObject *s) {
    static const u16 sWalkTimes[WALK_TIMES] = { 120, 180, 240, 300, 360, 420 };
    static const u8 sWalkWeights[WALK_TIMES] = { 20, 20, 20, 10, 10, 10 };
    s32 x = SQ_X(&s->obj), y = SQ_Y(&s->obj), kx, ky;
    u32 pick, i;

    switch (s->brain) {
    // Variant 0 (0x02084820): hovers HOVER_HEIGHT over where it started;
    case BRAIN_HOVER_START:
        FaceKirby(s);
        START_Y(s) = y;
        s->brain = BRAIN_HOVER_PRESS;
        break;
    case BRAIN_HOVER_PRESS: // 0x02084808;
        s->padB = 1;
        s->brain = BRAIN_HOVER_FORWARD;
        break;
    case BRAIN_HOVER_FORWARD:   // 0x020847F4;
        s->padDir = 1;
        s->brain = BRAIN_HOVER;
        break;
    case BRAIN_HOVER:       // 0x020847B8;
        s->padDir = 1;
        s->padB = 1;
        if (START_Y(s) + HOVER_HEIGHT < y) s->padB = 0;
        break;
    // Variant 1 (0x02084788): walks till Kirby's near over it, then chases him;
    case BRAIN_CHASE_START:
        FaceKirby(s);
        s->brain = BRAIN_WALK_FACE;
        BRAIN_WAIT(s) = CHASED(s) = 0;
        break;
    case BRAIN_WALK_FACE:   // 0x02084768;
        FaceKirby(s);
        s->brain = BRAIN_WALK;
        break;
    case BRAIN_WALK:        // 0x020846DC;
        kx = KirbyX(s);
        ky = KirbyY(s);
        s->padDir = SqueakWallAhead(s) ? -1 : 1;
        if (kx < x + CHASE_ACROSS && x - CHASE_ACROSS < kx && ky < y + CHASE_UP && y <= ky) {
            BRAIN_WAIT(s) = CHASE_TIME;
            s->brain = BRAIN_CHASE;
            CHASED(s)++;
        }
        break;
    case BRAIN_CHASE:       // 0x020845C0;
        kx = KirbyX(s);
        s->padDir = 1;
        s->padY = 0;
        if ((x < kx && (s->obj.base.flags & 1)) || (kx < x && !(s->obj.base.flags & 1))) s->padDir = -1;
        s->padB = 1;
        if (kx < x + OVER_KIRBY && x - OVER_KIRBY < kx && SQ_Y(&s->obj) >= -ScreenTop(s) * 0x10 - NEAR_TOP) {
            s->padB = 0;
            s->padDir = 0;
            s->padY = -1;
            BRAIN_WAIT(s) = DIVE_WAIT;
            s->brain = BRAIN_DIVE;
        }
        if (SqueakCeiling(s)) {
            s->padDir = 0;
            s->padY = PAD_DIVE_HARD;
            BRAIN_WAIT(s) = DIVE_WAIT;
            s->brain = BRAIN_DIVE_HARD;
        }
        if (BRAIN_WAIT(s) < 0) {
            s->padB = 0;
            s->brain = BRAIN_CHASE_LANDED;
        }
        BRAIN_WAIT(s)--;
        break;
    case BRAIN_DIVE:        // 0x02084550;
    case BRAIN_DIVE_HARD:   // 0x020844DC;
        s->padDir = 0;
        s->padY = s->brain == BRAIN_DIVE ? -1 : PAD_DIVE_HARD;
        if (!SqueakOnGround(s)) break;
        if (s->brain == BRAIN_DIVE_HARD) s->padB = 0;
        if (BRAIN_WAIT(s) < 1) ChaseOver(s);
        else BRAIN_WAIT(s)--;
        break;
    case BRAIN_CHASE_LANDED:    // 0x020844B0;
        if (!SqueakOnGround(s)) break;
        s->padDir = s->padY = 0;
        s->brain = BRAIN_CHASE_REST;
        BRAIN_TIME(s) = CHASE_REST;
        break;
    case BRAIN_CHASE_REST:  // 0x0208444C;
        s->padDir = 1;
        FaceKirby(s);
        if (BRAIN_TIME(s) > 0) {
            BRAIN_TIME(s)--;
            break;
        }
        ChaseOver(s);
        break;
    case BRAIN_LEAVE:       // 0x02084438;
        s->padDir = 0;
        s->brain = BRAIN_LEAVE_UP;
        break;
    case BRAIN_LEAVE_UP:    // 0x020843C8: away from Kirby, up 2 (it climbs through everything);
        kx = KirbyX(s);
        if (x < kx && !(s->obj.base.flags & 1)) s->padDir = -1;
        if (kx < x && (s->obj.base.flags & 1)) s->padDir = -1;
        s->padY = PAD_CLIMB;
        s->padB = 1;
        s->brain = BRAIN_LEAVE_PRESS;
        break;
    case BRAIN_LEAVE_PRESS: // 0x020843B0;
        s->padB = 1;
        s->brain = BRAIN_LEAVING;
        break;
    case BRAIN_LEAVING:     // 0x020843A4;
        s->padDir = 1;
        s->padB = 1;
        break;
    // Variant 2 (0x02084A40): walks a while, then rockets up;
    case BRAIN_PATROL_START:
        FaceKirby(s);
        s->brain = BRAIN_PATROL_PICK;
        BRAIN_WAIT(s) = 0;
        break;
    case BRAIN_PATROL_PICK: // 0x0208493C;
        s->padDir = 1;
        s->brain = BRAIN_PATROL;
        pick = Random();
        for (i = 0; i < WALK_TIMES - 1 && pick >= sWalkWeights[i]; i++) pick -= sWalkWeights[i];
        BRAIN_TIME(s) = pick < sWalkWeights[i] ? sWalkTimes[i] : BRAIN_TIME(s);
        break;
    case BRAIN_PATROL:      // 0x020848D4;
        if (BRAIN_WAIT(s) >= BRAIN_TIME(s)) {
            BRAIN_WAIT(s) = ROCKET_TIME;
            s->padDir = 0;
            s->padY = 1;
            s->padB = 1;
            s->brain = BRAIN_ROCKET;
            break;
        }
        s->padDir = SqueakWallAhead(s) ? -1 : 1;
        BRAIN_WAIT(s)++;
        break;
    case BRAIN_ROCKET:      // 0x02084888;
        if (BRAIN_WAIT(s) < 1) {
            s->padDir = s->padY = 0;
            s->padB = 0;
            if (SqueakOnGround(s)) {
                BRAIN_WAIT(s) = LANDED_WAIT;
                s->brain = BRAIN_ROCKET_LANDED;
            }
        } else {
            BRAIN_WAIT(s)--;
        }
        break;
    case BRAIN_ROCKET_LANDED:   // 0x02084854;
        if (!SqueakOnGround(s)) break;
        if (BRAIN_WAIT(s) < 1) s->brain = BRAIN_PATROL_START;
        else BRAIN_WAIT(s)--;
        break;
    // Variant 3 (0x02084370): stands;
    case BRAIN_STAND_START:
        FaceKirby(s);
        START_Y(s) = y;
        s->brain = BRAIN_STAND;
        break;
    case BRAIN_STAND:       // 0x02084368;
        s->padDir = s->padY = 0;
        break;
    }
}

static void SqueakStarmanMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    // (vtable +0x40, 0x02084C20: variant 0 through the terrain, held in the stage; the climb sets it too);
    if (s->state != BODY_CLIMB_ENTRY && s->state != BODY_CLIMB) SqueakThroughTerrain(s, s->variant == 0);
    Brain(s);
    SqueakRun(s, Body);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
    SqueakSetBoxes(s, &sStarBoxData, HARM(s), &BOX(s));
}

void SqueakStarmanStart(struct Object *obj) {
    static const u8 sBrains[] = { BRAIN_HOVER_START, BRAIN_CHASE_START, BRAIN_PATROL_START, BRAIN_STAND_START };
    struct SqueakObject *s = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, 0, SqueakStarmanMain);
    if (!s->started) s->brain = sBrains[s->variant < 4 ? s->variant : 0];
    SqueakSetState(s, BODY_START);
}

/* SqueakStarmanInit(); Its body box (starman.rca's: 8 pixels either side, 1 to 17 up; here from its feet: KatAM's bounds), facing left
 * (0x020877B8); its hit box and harm set each frame (SqueakSetBoxes()); */
void SqueakStarmanInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -8, -17, 8, -1);
    ObjectSetBounds(&s->obj.base, -8, -17, 8, 0);
    HARM(s) = s->obj.base.unk68;
    s->obj.base.flags |= 1;
    SqueakStandOnFloor(&s->obj);
}
