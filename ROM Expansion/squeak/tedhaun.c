/* Tedhaun; ported from Kirby: Squeak Squad (USA), mid-boss 0x35 (its body 0x02103B4C, vtable 0x0218042C; brain 0x02100FE8, its states
 * 0x021009F0-0x02100F00), as SQUEAK_TEDHAUN, a mid-boss on mini-boss type 0x42 (squeak.h's SQUEAK_MIDBOSS), and its three little
 * ghosts (0x02104F74, 0x02104E34: SQUEAK_TEDHAUN_GHOST, its animation 9). Values from param/prmMB11Ml0.kgb (its moves, its ghosts'),
 * prmMB11Pl0.kgb (brain) and prmMB11Sl0.kgb (HP 140); speeds 20.12 a frame, Y up; its boxes from enemy/tedhaun.rca. A floating ghost
 * in a hat that bobs about, grabs Kirby, and keeps three little ghosts round it that it sends sweeping across the screen.
 * • Its fight starts as a mini-boss's, then (0x021033FC, 0x02103374, 0x02103310) it waits 75 frames, untouchable, drifting.
 * • Body: it floats (anim 1: 0x02103244, 0x02103114) bobbing (0x02101170: 20 frames at 0.5 up and 0.05 a frame, 20 down, its base
 *   speed easing to 0 by 0.05: 0x02101210); B (0x021029CC) it winds up (anim 4, 28 frames) and calls (anim 5, 20 frames: its ghosts
 *   go, 0x02102910, 0x021028BC); A with up or down (0x02102E94, 0x02102CB8) it flies to 48 pixels under the screen's top middle (at
 *   0.8 a frame, 0.05 more a frame; turning toward it, 8 frames between turns) and, there, circles while A is held (0x02102118,
 *   0x02101FB8, 0x02101E20, 0x02101CC8, 0x02101B28: forward at 2.4, then easing (0.048 a frame across, 0.024 up and down) down to 1.2,
 *   back, up and forward again); A alone (0x02102C0C, 0x02102A80) it chases Kirby while A is held (0.12 a frame either way, at most
 *   0.8, turning to face him); its pad alone (0x02103070, 0x02102F6C) flies it (1.0 at most, 0.05 a frame) without bobbing. Its
 *   float's front and underside grab Kirby (tedhaun.rca's grab boxes, 0x021037B8): it reaches (anim 2, 28 frames: 0x02102804,
 *   0x0210266C), carries him to 88 pixels under the screen's top, 64 pixels in from the side it's on (0x02102414: 1.0 at most, 0.05
 *   a frame), holds him there 180 frames shaking him 2 pixels (0x0210232C) while a ghost comes from the screen's edge at him, and
 *   throws him (0x021021FC: 2.8 ahead, 0.8 up; KatAM's throw that hurts as he lands, 1 HP).
 * • Brain (Squeak Squad's random 0-99; one state a frame): it faces Kirby (0x02100F00) and picks (0x02100DDC): 45 in 100 circles (A
 *   with up, held 300, 420 or 480 frames: 30, 40, 30 in 100: 0x02100AB4), 30 chases (A, held 180, 240 or 280: 0x02100A30), 25 sends
 *   its ghosts (0x02100D98: it flies to 48 pixels in from the screen's nearer top corner, 56 under the top (0x02100C90, 0x02100D14),
 *   faces Kirby, waits 16 frames (0x02100C50), and once its three ghosts are ready presses B (0x02100BC4, 0x02100B44), then waits for
 *   them all back); the same pick more than twice in a row (circles, chases or ghosts) and it picks one of the other two, half and
 *   half. While A is held it faces Kirby.
 * • Ghosts (0x02104A70, 0x021049A8, 0x02104904, 0x021047B8, 0x021046B4, 0x021045C4, 0x0210450C): each of its three spots (behind it
 *   24 pixels and 20 down, behind 32 and 28 up, ahead 24 and 16 down) gets a ghost 20 frames after it has none: the ghost comes out
 *   of it 24 pixels off (at 110, 66 or 132 degrees), flickering 16 frames, and keeps to its spot (1.6 at most across and 1.0 up and
 *   down, 0.16 and 0.1 a frame; within 32 pixels 1.6 and 0.16, facing as Tedhaun). Its call (B) sends them, the first at once, the
 *   others 30 and 80 frames later: each sweeps across the screen 600 frames (to 32 pixels from its far side and back, 3.2 at most,
 *   0.15 a frame, and at Kirby's height, 0.5 at most, 0.05 a frame), then goes back to its spot and is ready again 360 frames later.
 *   A hit or a ghost going out of the stage ends it (a burst, 0x02103E1C, 0x02103E7C); Tedhaun beaten, they burst 45, 80 and 120
 *   frames later. The ghost its grab calls comes from the screen's top left or right corner (the side away from it) and homes in on
 *   Kirby 180 frames (1.0 at most, 0.05 a frame; within 4 pixels 1.6 and 0.16), then bursts (0x0210439C, 0x02104304).
 * • Hit, it doesn't flinch (0x021034BC, 0x0210357C); its defeat is KatAM's mini-boss's.
 * Its sounds (Squeak Squad's, SqueakSfx): 0x17B, 0x55, 0x10D, 0x12D, 0x152, 0x153, 0x17C, 0x17D. Not ported: the camera's moves at its grab (0x02035EC8,
 * 0x02035E04), Squeak Squad's hit flash and shakes (KatAM's), its own defeat (0x02101904..: a hop, a slide, a shudder, its ghosts let
 * go), the other mode; its ghosts' flicker is KatAM's hiding of them every other frame. */
#include "../squeak.h"

void RequestScreenShake(u16, struct ObjectBase *);
void sub_0804E09C(struct Kirby *);

// Animations (tedhaun.cea);
enum { ANIM_STILL, ANIM_FLOAT, ANIM_GRAB, ANIM_HOLD, ANIM_WIND_UP, ANIM_CALL, ANIM_THROW, ANIM_DEFEAT, ANIM_8, ANIM_GHOST };

// prmMB11Ml0.kgb (20.12);
#define FLY_CAP 0x1000          // +0x20, +0x24: its pad's flight and its carry, at most; +0x28, +0x2C: a frame;
#define FLY_ACCEL 205
#define FRICTION 205            // +0x30, +0x34;
#define BOB_SPEED 0x800         // +0x3C, +0x44;
#define BOB_ACCEL 205
#define TO_TOP_CAP 0xCCD        // +0x4C, +0x50, +0x54, +0x58;
#define TO_TOP_ACCEL 205
#define LOOP_CAP_X 0x2667       // +0x5C, +0x60, +0x64, +0x68;
#define LOOP_CAP_Y 0x1334
#define LOOP_ACCEL_X 197
#define LOOP_ACCEL_Y 99
#define CHASE_CAP 0xCCD         // +0xEC, +0xF0, +0xF4, +0xF8;
#define CHASE_ACCEL 0x1EC
#define GHOST_RADIUS 0x18000    // The ghosts' start, 24 pixels round it, moving out at 0.25;
#define GHOST_OUT 0x400
// Its own;
#define INTRO 75
#define BOB 20
#define TURN_WAIT 8
#define NEAR 0x4000             // Within 4 pixels: there;
#define HOLD 180
#define SLOTS 3
#define GHOST_REFILL 20
#define TOP_MIDDLE_Y 48         // Pixels under the screen's top: its circling; the ghosts' corner; its carry;
#define CORNER_Y 56
#define CARRY_Y 88
#define CARRY_X 64
#define CORNER_X 48
#define FACE_DISTANCE 0x1000

enum {
    BODY_START, BODY_FLOAT, BODY_INTRO, BODY_FLY_ENTRY, BODY_FLY, BODY_WIND_UP, BODY_CALL, BODY_CALLED, BODY_CHASE_ENTRY, BODY_CHASE,
    BODY_TO_TOP_ENTRY, BODY_TO_TOP, BODY_LOOP_ENTRY, BODY_LOOP_DOWN, BODY_LOOP_BACK, BODY_LOOP_UP, BODY_LOOP_FORWARD, BODY_GRAB,
    BODY_REACH, BODY_CARRY, BODY_HOLD, BODY_THROW,
};
enum {
    BRAIN_START, BRAIN_PICK, BRAIN_CIRCLE, BRAIN_CHASE, BRAIN_HOLDING, BRAIN_GHOSTS, BRAIN_CORNER_LEFT, BRAIN_CORNER_RIGHT,
    BRAIN_AT_CORNER, BRAIN_FACE_WAIT, BRAIN_READY_WAIT, BRAIN_BACK_WAIT,
};
// Its ghosts' kinds (their variant): 0-2 its spots', SQUEAK_TEDHAUN_HOMER the grab's;
#define SQUEAK_TEDHAUN_HOMER 3

#define BUSY(s) ((s)->vars[0])          // 0x208: 0 floating, 1 flying, 2 circling, 3 chasing, 4 calling, 5 grabbing;
#define COUNT(s) ((s)->vars[1])         // 0x20C;
#define LOOP_X(s) ((s)->vars[2])        // 0x220, 0x224: the circle's speeds; 0x23C: its way round; 0x244: the bob's base;
#define LOOP_Y(s) ((s)->vars[3])
#define LOOP_WAY(s) (*(u8 *)&(s)->vars[4])
#define TURN_COOL(s) (*((u8 *)&(s)->vars[4] + 1))  // 0x1FC;
#define BASE_Y(s) ((s)->vars[5])
#define BOB_COUNT(s) ((s)->vars[6])     // 0x248;
#define BRAIN_TIMER(s) ((s)->vars[7])   // The brain's 0x98;
#define HELD(s) (*(struct Kirby **)&(s)->vars[8])   // 0x164;
#define SPOT_X(s) ((s)->vars[9])        // 0x1A0, 0x1A4: where it flies to;
#define SPOT_Y(s) ((s)->vars[10])
#define SLOT(s, i) (*((u8 *)&(s)->vars[11] + (i)))      // 0x254: a ghost there; 0x258: ready (not sent); 0x25C: the refill's frames;
#define READY(s, i) (*((u8 *)&(s)->vars[12] + (i)))
#define REFILL(s, i) (*((u8 *)&(s)->vars[13] + (i)))
#define SEEN(s, i) (*((u8 *)&(s)->vars[14] + (i)))       // Its ghost was alive this frame;
#define BOX(s) (*((u8 *)&(s)->vars[11] + 3))
#define BRAIN_CIRCLES(s) (*((u8 *)&(s)->vars[12] + 3)) // The brain's 0xA8, 0xB0, 0xAC: its picks in a row;
#define BRAIN_CHASES(s) (*((u8 *)&(s)->vars[13] + 3))
#define BRAIN_GHOST_PICKS(s) (*((u8 *)&(s)->vars[14] + 3))
#define HARM(s) ((s)->vars[15])

// The ghosts' fields;
#define G_PARENT(s) (*(struct SqueakObject **)&(s)->vars[0])   // 0x12C;
#define G_COUNT(s) ((s)->vars[1])       // 0x150;
#define G_DOOM(s) ((s)->vars[2])        // 0x154, 0x158: Tedhaun beaten, its frames left;
#define G_BOX(s) (*(u8 *)&(s)->vars[3])
#define G_HARM(s) ((s)->vars[4])
enum { GHOST_START, GHOST_APPEAR, GHOST_FOLLOW, GHOST_WAIT, GHOST_SWEEP, GHOST_HOME, GHOST_BURST };

// tedhaun.rca's boxes (boxes.py): its body, its float's with two grabbing boxes (ahead and under it), its ghosts';
static const struct SqueakBox sTedhaunBoxes[] = {
    { { -17, -30, 18, 1 }, TRUE, 0 },
    { { -7, -23, 8, -7 }, TRUE, 2, { { 8, -23, 13, -1 }, { -7, -7, 8, -2 } } },
    { { -10, -19, 13, 0 }, TRUE, 0 },
};
static const struct SqueakBoxFrame sTedhaunAnim0[] = { { 1, 0 } };
static const struct SqueakBoxFrame sTedhaunAnim1[] = { { 16, 1 } };
static const struct SqueakBoxFrame sTedhaunAnim2[] = { { 28, 0 } };
static const struct SqueakBoxFrame sTedhaunAnim3[] = { { 8, 0 } };
static const struct SqueakBoxFrame sTedhaunAnim4[] = { { 28, 0 } };
static const struct SqueakBoxFrame sTedhaunAnim5[] = { { 20, 0 } };
static const struct SqueakBoxFrame sTedhaunAnim6[] = { { 1, 0 } };
static const struct SqueakBoxFrame sTedhaunAnim7[] = { { 18, 0 } };
static const struct SqueakBoxFrame sTedhaunAnim8[] = { { 1, 0 } };
static const struct SqueakBoxFrame sTedhaunAnim9[] = { { 16, 2 } };
static const struct SqueakBoxFrame sTedhaunAnim10[] = { { 1, 2 } };
static const struct SqueakBoxFrame sTedhaunAnim11[] = { { 1, 2 } };
static const struct SqueakBoxAnim sTedhaunBoxAnims[] = {
    { sTedhaunAnim0, 1 }, { sTedhaunAnim1, 1 }, { sTedhaunAnim2, 1 }, { sTedhaunAnim3, 1 }, { sTedhaunAnim4, 1 },
    { sTedhaunAnim5, 1 }, { sTedhaunAnim6, 1 }, { sTedhaunAnim7, 1 }, { sTedhaunAnim8, 1 }, { sTedhaunAnim9, 1 },
    { sTedhaunAnim10, 1 }, { sTedhaunAnim11, 1 },
};
// (tedhaun.rca's boxes flagged 0x8000 in its grab and hold, 2 and 3: untouchable);
static const struct SqueakBoxes sTedhaunBoxData = { sTedhaunBoxes, sTedhaunBoxAnims, 12, { SQUEAK_ANIM_LO(ANIM_GRAB) | SQUEAK_ANIM_LO(ANIM_HOLD) } };

/* Kirby held (kirby->unk110), facing it, 8 pixels ahead of it and 8 under its spot (his feet: KatAM's spot 7 over them), shaken 2
 * pixels up and down (0x0210232C: up on its frames 0 of 8, down on 2); thrown 2.8 ahead and 0.8 up (KatAM's 16ths: his back) hurt as
 * he lands; let go as he is; */
#define HOLD_X -8
static const struct Kirby_110 sHold[] = { { HOLD_X, 1, -1, 0xFF, 0x57, 0x400 }, { 0, 0, 0, 1, 0, 0x400 } };
static const struct Kirby_110 sHoldUp[] = { { HOLD_X, -1, -1, 0xFF, 0x57, 0x400 }, { 0, 0, 0, 1, 0, 0x400 } };
static const struct Kirby_110 sHoldDown[] = { { HOLD_X, 3, -1, 0xFF, 0x57, 0x400 }, { 0, 0, 0, 1, 0, 0x400 } };
static const struct Kirby_110 sThrow[] = { { -45, 13, -1, 1, 0, 0x402 }, { 0, 0, 0, 1, 0, 0 } };
static const struct Kirby_110 sLetGo[] = { { 0, 0, -1, 1, 0, 0x401 }, { 0, 0, 0, 1, 0, 0 } };

static void TedhaunMain(struct Object *obj);

static s32 Roll(void) {
    return Rand16() * 100 >> 16;
}

// Camera(); The screen's top left (KatAM's units);
static s32 CameraX(struct SqueakObject *s) {
    return gCurLevelInfo[s->obj.base.unk56].viewportPosition.x;
}
static s32 CameraY(struct SqueakObject *s) {
    return gCurLevelInfo[s->obj.base.unk56].viewportPosition.y;
}

static bool32 StillHeld(struct SqueakObject *s) {
    struct Kirby *kirby = HELD(s);

    return kirby && kirby->base.unk6C == &s->obj.base
           && (kirby->unk110 == sHold || kirby->unk110 == sHoldUp || kirby->unk110 == sHoldDown);
}

static void LetGo(struct SqueakObject *s, const struct Kirby_110 *how) {
    struct Kirby *kirby = HELD(s);

    if (StillHeld(s)) {
        kirby->unk110 = how;
        kirby->base.counter = -1;
    }
    HELD(s) = NULL;
    s->obj.base.unk6C = NULL;
}

// Turn(); 0x02101A20's: turned round, unless it turned less than TURN_WAIT frames ago;
static bool32 Turn(struct SqueakObject *s) {
    if (TURN_COOL(s)) return FALSE;
    SqueakTurnAround(s);
    TURN_COOL(s) = TURN_WAIT;
    return TRUE;
}

// Toward(); A speed eased by `accel` toward `target` (no further);
static s32 Toward(s32 v, s32 target, s32 accel) {
    if (v < target) return v + accel > target ? target : v + accel;
    if (v > target) return v - accel < target ? target : v - accel;
    return v;
}

// Bob(); 0x02101170: its bob on its base speed up and down (`on`: else its base alone);
static void Bob(struct SqueakObject *s, bool32 on) {
    s32 a = on ? BOB_SPEED : 0, b = on ? BOB_ACCEL : 0;

    s->ax = 0;
    if (BOB_COUNT(s) > BOB) {
        s->vy = BASE_Y(s) + a;
        s->ay = b;
    } else if (BOB_COUNT(s) > 0) {
        s->vy = BASE_Y(s) - a;
        s->ay = -b;
    }
    if (BOB_COUNT(s) < 1) {
        s->vy = BASE_Y(s);
        s->ay = 0;
        BOB_COUNT(s) = 2 * BOB;
    } else {
        BOB_COUNT(s)--;
    }
}

// Friction(); 0x02101210: no A and no direction, its speed across eases to 0; no up or down, its base speed too;
static void Friction(struct SqueakObject *s) {
    if (!s->padA && s->padDir == 0) s->vx = Toward(s->vx, 0, FRICTION);
    if (s->padY == 0) BASE_Y(s) = Toward(BASE_Y(s), 0, FRICTION);
}

// PadFly(); 0x021012D4: forward and up or down speed it up;
static void PadFly(struct SqueakObject *s) {
    if (s->padDir > 0) s->vx = s->vx >= FLY_CAP ? FLY_CAP : s->vx + FLY_ACCEL;
    if (s->padY > 0) BASE_Y(s) = BASE_Y(s) >= FLY_CAP ? FLY_CAP : BASE_Y(s) + FLY_ACCEL;
    if (s->padY < 0) BASE_Y(s) = BASE_Y(s) <= -FLY_CAP ? -FLY_CAP : BASE_Y(s) - FLY_ACCEL;
}

// LoopMove(); 0x02101A20: the circle's speeds (no bob), back turning it round (its way round and speed with it);
static void LoopMove(struct SqueakObject *s) {
    if (s->padDir < 0 && Turn(s)) {
        LOOP_WAY(s) ^= 1;
        LOOP_X(s) = -LOOP_X(s);
    }
    s->vx = LOOP_X(s);
    s->vy = LOOP_Y(s);
    s->ax = s->ay = 0;
}

/* FlyTo(); 0x02102CB8 (`carry` FALSE) / 0x02102414 (TRUE): toward (SPOT_X, SPOT_Y) (KatAM's units), at most `cap` and `accel` a frame;
 * TRUE once within NEAR both ways. To the top it turns toward it and speeds up forward; carrying it doesn't turn, backs if need be,
 * and stops each way within NEAR; */
static bool32 FlyTo(struct SqueakObject *s, s32 cap, s32 accel, bool32 carry) {
    struct ObjectBase *b = &s->obj.base;
    s32 dx = SPOT_X(s) - b->x, dy = b->y - SPOT_Y(s), near = NEAR >> 4;    // (dy: up);
    bool32 nearX = dx > -near && dx < near, nearY = dy > -near && dy < near;

    if (nearX && nearY) return TRUE;
    if (carry && nearX) s->vx = 0;
    else if (dx != 0 && (dx > 0) == !(b->flags & 1)) s->vx = s->vx < cap ? s->vx + accel : cap;
    else if (carry) s->vx = s->vx > -cap ? s->vx - accel : -cap;
    else if (dx != 0) Turn(s);
    if (carry && nearY) s->vy = 0;
    else if (dy > 0) s->vy = s->vy < cap ? s->vy + accel : cap;
    else if (dy < 0) s->vy = s->vy > -cap ? s->vy - accel : -cap;
    s->ax = s->ay = 0;
    return FALSE;
}

// Ghosts(); Each empty spot gets a ghost GHOST_REFILL frames on (0x02101B28's loop); a ghost that didn't show this frame is gone;
static void Ghosts(struct SqueakObject *s) {
    struct SqueakObject *g;
    u8 i;

    for (i = 0; i < SLOTS; i++) {
        if (SLOT(s, i) && !SEEN(s, i)) {
            SLOT(s, i) = 0;
            READY(s, i) = 1;
            REFILL(s, i) = 0;
        }
        SEEN(s, i) = 0;
        if (SLOT(s, i)) continue;
        if (REFILL(s, i) < GHOST_REFILL) {
            REFILL(s, i)++;
            continue;
        }
        g = SqueakSpawn(s, SQUEAK_TEDHAUN_GHOST, i, 0, 0);
        if (g == NULL) continue;
        G_PARENT(g) = s;
        SLOT(s, i) = SEEN(s, i) = READY(s, i) = 1;
    }
}

static bool32 AllReady(struct SqueakObject *s) {
    return SLOT(s, 0) && READY(s, 0) == 1 && SLOT(s, 1) && READY(s, 1) == 1 && SLOT(s, 2) && READY(s, 2) == 1;
}

// Buttons(); 0x021013FC: B its call; A with up or down its circling, A alone its chase;
static bool32 Buttons(struct SqueakObject *s) {
    if (s->padB) {
        SqueakSetState(s, BODY_WIND_UP);
        return TRUE;
    }
    if (s->padA) {
        SqueakSetState(s, s->padY ? BODY_TO_TOP_ENTRY : BODY_CHASE_ENTRY);
        return TRUE;
    }
    return FALSE;
}

static bool32 Body(struct SqueakObject *s) {
    struct ObjectBase *b = &s->obj.base;
    struct Kirby *kirby = s->obj.kirby3;
    s32 cap;

    switch (s->state) {
    case BODY_START:        // 0x02103244 (vtable +0x3C): (its hum stopped: started again by its float, 0x021037B8);
        BUSY(s) = 0;
        SqueakStopSfx(s, 0x17B);
        SqueakPlay(s, ANIM_FLOAT);
        SqueakSetState(s, BODY_FLOAT);
        return TRUE;
    case BODY_FLOAT:        // 0x02103114;
        if (Buttons(s)) return TRUE;
        if (s->padDir > 0 || s->padY) {
            SqueakSetState(s, BODY_FLY_ENTRY);
            return TRUE;
        }
        Friction(s);
        Bob(s, TRUE);
        if (s->padDir < 0) Turn(s);
        SqueakMove(s);
        break;
    case BODY_INTRO:        // 0x02103310: untouchable, drifting INTRO frames;
        if (COUNT(s) < 1) {
            b->flags &= ~0x200;
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        COUNT(s)--;
        SqueakMove(s);
        break;
    case BODY_FLY_ENTRY:    // 0x02103070;
        BUSY(s) = 1;
        SqueakPlay(s, ANIM_FLOAT);
        SqueakSetState(s, BODY_FLY);
        return TRUE;
    case BODY_FLY:          // 0x02102F6C;
        if (Buttons(s)) return TRUE;
        if (s->padDir == 0 && s->padY == 0) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        PadFly(s);
        Friction(s);
        Bob(s, FALSE);
        if (s->padDir < 0) Turn(s);
        SqueakMove(s);
        break;
    case BODY_WIND_UP:      // 0x021029CC;
        SqueakPlay(s, ANIM_WIND_UP);
        SqueakStop(s);
        SqueakSfx(s, 0x55);
        SqueakSetState(s, BODY_CALL);
        return TRUE;
    case BODY_CALL:         // 0x02102910: its wind-up over, it calls;
        if (s->animEnded) {
            BUSY(s) = 4;
            SqueakPlay(s, ANIM_CALL);
            SqueakSetState(s, BODY_CALLED);
            return TRUE;
        }
        SqueakMove(s);
        break;
    case BODY_CALLED:       // 0x021028BC;
        if (s->animEnded) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        SqueakMove(s);
        break;
    case BODY_CHASE_ENTRY:  // 0x02102C0C;
        BUSY(s) = 1;
        SqueakPlay(s, ANIM_FLOAT);
        SqueakStop(s);
        SqueakSetState(s, BODY_CHASE);
        return TRUE;
    case BODY_CHASE:        // 0x02102A80: at Kirby while A is held;
        BUSY(s) = 3;
        if (!s->padA) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        s->ax = CHASE_ACCEL;
        if (b->flags & 1 ? kirby->base.x > b->x : kirby->base.x < b->x) Turn(s);
        s->ay = kirby->base.y < b->y ? CHASE_ACCEL : kirby->base.y > b->y ? -CHASE_ACCEL : 0;
        SqueakMove(s);
        if (s->vx > CHASE_CAP) s->vx = CHASE_CAP, s->ax = 0;
        else if (s->vx < -CHASE_CAP) s->vx = -CHASE_CAP, s->ax = 0;
        if (s->vy > CHASE_CAP) s->vy = CHASE_CAP, s->ay = 0;
        else if (s->vy < -CHASE_CAP) s->vy = -CHASE_CAP, s->ay = 0;
        break;
    case BODY_TO_TOP_ENTRY: // 0x02102E94: to 48 pixels under the screen's top middle;
        BUSY(s) = 2;
        SqueakPlay(s, ANIM_FLOAT);
        SqueakStop(s);
        SPOT_X(s) = CameraX(s) + ((DISPLAY_WIDTH / 2) << 8);
        SPOT_Y(s) = CameraY(s) + (TOP_MIDDLE_Y << 8);
        SqueakSetState(s, BODY_TO_TOP);
        return TRUE;
    case BODY_TO_TOP:       // 0x02102CB8;
        if (FlyTo(s, TO_TOP_CAP, TO_TOP_ACCEL, FALSE)) {
            TURN_COOL(s) = 0;
            SqueakStop(s);
            SqueakSetState(s, BODY_LOOP_ENTRY);
            return TRUE;
        }
        SqueakMove(s);
        break;
    case BODY_LOOP_ENTRY:   // 0x02102118: forward at its top speed;
        SqueakStop(s);
        LOOP_X(s) = LOOP_CAP_X;
        LOOP_Y(s) = LOOP_WAY(s) = 0;
        SqueakSetState(s, BODY_LOOP_DOWN);
        return TRUE;
    case BODY_LOOP_DOWN:    // 0x02101FB8 .. 0x02101B28: forward, down, back, up and forward again while A is held;
    case BODY_LOOP_BACK:
    case BODY_LOOP_UP:
    case BODY_LOOP_FORWARD:
        if (!s->padA) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        cap = LOOP_WAY(s) ? -LOOP_CAP_X : LOOP_CAP_X;
        switch (s->state) {
        case BODY_LOOP_DOWN:
            if (LOOP_X(s) == 0 && LOOP_Y(s) == -LOOP_CAP_Y) {
                SqueakSetState(s, BODY_LOOP_BACK);
                return TRUE;
            }
            LOOP_X(s) = Toward(LOOP_X(s), 0, LOOP_ACCEL_X);
            LOOP_Y(s) = Toward(LOOP_Y(s), -LOOP_CAP_Y, LOOP_ACCEL_Y);
            break;
        case BODY_LOOP_BACK:
            if (LOOP_X(s) == -cap && LOOP_Y(s) == 0) {
                SqueakSetState(s, BODY_LOOP_UP);
                return TRUE;
            }
            LOOP_X(s) = Toward(LOOP_X(s), -cap, LOOP_ACCEL_X);
            LOOP_Y(s) = Toward(LOOP_Y(s), 0, LOOP_ACCEL_Y);
            break;
        case BODY_LOOP_UP:
            if (LOOP_X(s) == 0 && LOOP_Y(s) == LOOP_CAP_Y) {
                SqueakSetState(s, BODY_LOOP_FORWARD);
                return TRUE;
            }
            LOOP_X(s) = Toward(LOOP_X(s), 0, LOOP_ACCEL_X);
            LOOP_Y(s) = Toward(LOOP_Y(s), LOOP_CAP_Y, LOOP_ACCEL_Y);
            break;
        default:
            if (LOOP_X(s) == cap && LOOP_Y(s) == 0) {
                SqueakSetState(s, BODY_LOOP_DOWN);
                return TRUE;
            }
            LOOP_X(s) = Toward(LOOP_X(s), cap, LOOP_ACCEL_X);
            LOOP_Y(s) = Toward(LOOP_Y(s), 0, LOOP_ACCEL_Y);
            break;
        }
        LoopMove(s);
        SqueakMove(s);
        break;
    case BODY_GRAB:         // 0x02102804: it reaches;
        BUSY(s) = 5;
        SqueakPlay(s, ANIM_GRAB);
        SqueakStop(s);
        COUNT(s) = 0;
        SqueakSetState(s, BODY_REACH);
        return TRUE;
    case BODY_REACH:        // 0x0210266C: its reach over, to 88 under the screen's top, 64 in from its side;
        if (s->animEnded) {
            if (b->x > CameraX(s) + ((DISPLAY_WIDTH / 2) << 8)) {
                SPOT_X(s) = CameraX(s) + ((DISPLAY_WIDTH - CARRY_X) << 8);
                if (!(b->flags & 1)) Turn(s);
            } else {
                SPOT_X(s) = CameraX(s) + (CARRY_X << 8);
                if (b->flags & 1) Turn(s);
            }
            SPOT_Y(s) = CameraY(s) + (CARRY_Y << 8);
            COUNT(s) = 0;
            SqueakSetState(s, BODY_CARRY);
            return TRUE;
        }
        COUNT(s)++;
        SqueakMove(s);
        break;
    case BODY_CARRY:        // 0x02102414;
        if (FlyTo(s, FLY_CAP, FLY_ACCEL, TRUE)) {
            SqueakStop(s);
            COUNT(s) = HOLD;
            {
                struct SqueakObject *g = SqueakSpawn(s, SQUEAK_TEDHAUN_GHOST, SQUEAK_TEDHAUN_HOMER, 0, 0);

                if (g) G_PARENT(g) = s;
            }
            SqueakSetState(s, BODY_HOLD);
            return TRUE;
        }
        COUNT(s)++;
        SqueakMove(s);
        break;
    case BODY_HOLD:         // 0x0210232C: HOLD frames, then the throw;
        if (COUNT(s) < 1) {
            SqueakStop(s);
            LetGo(s, sThrow);
            SqueakPlay(s, ANIM_THROW);
            SqueakSetState(s, BODY_THROW);
            return TRUE;
        }
        COUNT(s)--;
        SqueakMove(s);
        break;
    case BODY_THROW:        // 0x021021FC;
        if (s->animEnded) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        SqueakMove(s);
        break;
    }
    return FALSE;
}

// Pick(); 0x02100DDC: circles (45 in 100), chases (30) or ghosts (25), the same more than twice in a row and one of the others;
static void Pick(struct SqueakObject *s) {
    s32 r1 = Roll(), r2 = Roll(), kind = r1 <= 44 ? 0 : r1 < 75 ? 1 : 2;

    BRAIN_TIMER(s) = 0;
    switch (kind) {
    case 0:
        if (BRAIN_CIRCLES(s) > 1) {
            BRAIN_CIRCLES(s) = 0;
            kind = r2 < 50 ? 1 : 2;
        }
        break;
    case 1:
        if (BRAIN_CHASES(s) > 1) {
            BRAIN_CHASES(s) = 0;
            kind = r2 > 49 ? 2 : 0;
        }
        break;
    default:
        if (BRAIN_GHOST_PICKS(s) > 1) {
            BRAIN_GHOST_PICKS(s) = 0;
            kind = r2 > 49 ? 1 : 0;
        }
        break;
    }
    if (kind == 0) {
        s->brain = BRAIN_CIRCLE;
        BRAIN_CIRCLES(s)++;
    } else if (kind == 1) {
        s->brain = BRAIN_CHASE;
        BRAIN_CHASES(s)++;
    } else {
        s->brain = BRAIN_GHOSTS;
        BRAIN_GHOST_PICKS(s)++;
    }
}

// Brain(); Its pad (0x02100F00 and the states it leads to), one state a frame;
static void Brain(struct SqueakObject *s) {
    struct ObjectBase *b = &s->obj.base;
    s32 roll, tx, ty, dx, dy;

    switch (s->brain) {
    case BRAIN_START:       // 0x02100F00;
        SqueakFaceKirby(s, FACE_DISTANCE);
        BRAIN_CIRCLES(s) = BRAIN_CHASES(s) = BRAIN_GHOST_PICKS(s) = 0;
        s->brain = BRAIN_PICK;
        break;
    case BRAIN_PICK:
        Pick(s);
        break;
    case BRAIN_CIRCLE:      // 0x02100AB4: A with up, held 300, 420 or 480 frames (the last frame's pick), once it circles;
        roll = Roll();
        s->padA = 1;
        s->padDir = 0;
        s->padY = 1;
        BRAIN_TIMER(s) = roll < 30 ? 300 : roll < 70 ? 420 : 480;
        if (BUSY(s) == 2) s->brain = BRAIN_HOLDING;
        break;
    case BRAIN_CHASE:       // 0x02100A30: A, held 180, 240 or 280 frames, once it chases;
        roll = Roll();
        s->padA = 1;
        s->padDir = s->padY = 0;
        BRAIN_TIMER(s) = roll < 30 ? 180 : roll < 70 ? 240 : 280;
        if (BUSY(s) == 3) s->brain = BRAIN_HOLDING;
        break;
    case BRAIN_HOLDING:     // 0x021009F0: facing Kirby; its time over, A let go;
        if (BRAIN_TIMER(s) < 1) {
            s->padDir = s->padY = 0;
            s->padA = 0;
            s->brain = BRAIN_PICK;
            break;
        }
        SqueakFaceKirby(s, FACE_DISTANCE);
        BRAIN_TIMER(s)--;
        break;
    case BRAIN_GHOSTS:      // 0x02100D98: to the nearer top corner;
        s->brain = b->x <= CameraX(s) + ((DISPLAY_WIDTH / 2) << 8) ? BRAIN_CORNER_LEFT : BRAIN_CORNER_RIGHT;
        break;
    case BRAIN_CORNER_LEFT: // 0x02100D14, 0x02100C90 (0x02100968: its pad toward it, within 16 pixels there);
    case BRAIN_CORNER_RIGHT:
        tx = CameraX(s) + ((s->brain == BRAIN_CORNER_LEFT ? CORNER_X : DISPLAY_WIDTH - CORNER_X) << 8);
        ty = CameraY(s) + (CORNER_Y << 8);
        if (b->x < tx) s->padDir = b->flags & 1 ? -1 : 1;
        else if (b->x > tx) s->padDir = b->flags & 1 ? 1 : -1;
        if (b->y > ty) s->padY = 1;
        else if (b->y < ty) s->padY = -1;
        dx = b->x - tx;
        dy = b->y - ty;
        if (dx > -0x1000 && dx < 0x1000 && dy > -0x1000 && dy < 0x1000) s->brain = BRAIN_AT_CORNER;
        break;
    case BRAIN_AT_CORNER:   // 0x02100C78, 0x02100C50;
        s->padDir = s->padY = 0;
        SqueakFaceKirby(s, FACE_DISTANCE);
        BRAIN_TIMER(s) = 16;
        s->brain = BRAIN_FACE_WAIT;
        break;
    case BRAIN_FACE_WAIT:   // 0x02100C20;
        s->padDir = s->padY = 0;
        if (BRAIN_TIMER(s) > 0) BRAIN_TIMER(s)--;
        else s->brain = BRAIN_READY_WAIT;
        break;
    case BRAIN_READY_WAIT:  // 0x02100BC4: its ghosts all ready, B;
        if (!AllReady(s)) break;
        s->padDir = s->padY = 0;
        s->padB = 1;
        BRAIN_TIMER(s) = 2;
        s->brain = BRAIN_BACK_WAIT;
        break;
    case BRAIN_BACK_WAIT:   // 0x02100B44: facing Kirby, its ghosts all ready again;
        s->padDir = s->padY = 0;
        SqueakFaceKirby(s, FACE_DISTANCE);
        s->padB = 0;
        if (!AllReady(s)) break;
        if (BRAIN_TIMER(s) < 1) s->brain = BRAIN_PICK;
        else BRAIN_TIMER(s)--;
        break;
    }
}

static void TedhaunMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;
    struct Kirby *kirby;
    u8 t;

    obj->kirby3 = FindTargetKirby(&obj->base);
    if (TURN_COOL(s)) TURN_COOL(s)--;
    SqueakMidbossUpdate(s);
    Brain(s);
    SqueakRun(s, Body);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
    if (s->state != BODY_INTRO) Ghosts(s);
    SqueakSetBoxes(s, &sTedhaunBoxData, HELD(s) || s->state == BODY_THROW ? 0 : HARM(s), &BOX(s));
    // (Kirby held, shaken (0x0210232C: up on its frames 0 of 8, down on 2), his body no harm to it, as KatAM's grabbers');
    if (StillHeld(s)) {
        t = s->state == BODY_HOLD ? COUNT(s) % 8 : COUNT(s) % 8;
        HELD(s)->unk110 = t < 1 ? sHoldUp : t == 2 ? sHoldDown : sHold;
        obj->base.unk5C |= 0x80;
    } else {
        if (HELD(s)) LetGo(s, sLetGo);
        obj->base.unk5C &= ~0x80;
    }
    // (its float's grabbing boxes and its hum, 0x021037B8);
    if (!s->loopSound && obj->unk80 > 0 && s->state != BODY_INTRO) SqueakLoopSfx(s, 0x17B);
    kirby = obj->unk80 > 0 && !HELD(s) && s->anim == ANIM_FLOAT && s->state != BODY_INTRO ? SqueakGrabbing(s, &sTedhaunBoxes[BOX(s)]) : NULL;
    if (kirby) {
        HELD(s) = kirby;
        kirby->base.unk6C = &obj->base;
        kirby->unk110 = sHold;
        obj->base.unk6C = kirby;
        kirby->base.flags |= 0x40000;
        sub_0804E09C(kirby);
        kirby->base.flags ^= 1;
        SqueakStop(s);
        SqueakSfx(s, 0x17D);
        SqueakSetState(s, BODY_GRAB);
    }
}

// TedhaunWatch(); Its pre-callback: KatAM's hit put it out of its own state, or it's beaten: it lets Kirby go as he is;
static void TedhaunWatch(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    if (HELD(s) && (obj->unk78 != TedhaunMain || obj->unk80 <= 0 || (obj->base.flags & 0x1000))) LetGo(s, sLetGo);
}

/* TedhaunStart(); Its fight's start (and after a hit: it goes on, as KatAM's mini-bosses do; holding Kirby, it lets him go): the first
 * time, its intro (untouchable, drifting INTRO frames), then its body's start; */
void TedhaunStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, -1, TedhaunMain);
    obj->unk7C = TedhaunWatch;
    SqueakThroughTerrain(s, TRUE);
    if (HELD(s)) {
        LetGo(s, sLetGo);
        SqueakSetState(s, BODY_START);
    }
    if (s->started) return;
    SqueakPlay(s, ANIM_FLOAT);
    SqueakStop(s);
    COUNT(s) = INTRO;
    obj->base.flags |= 0x200;
    BOB_COUNT(s) = 0;
    SqueakSetState(s, BODY_INTRO);
}

// TedhaunInit(); Its box (tedhaun.rca's body), its bounds (KatAM's), facing left, its ghosts' spots empty;
void TedhaunInit(struct SqueakObject *s) {
    u8 i;

    sub_0803E2B0(&s->obj.base, -17, -30, 18, 1);
    ObjectSetBounds(&s->obj.base, -16, -30, 16, 0);
    HARM(s) = s->obj.base.unk68;
    s->obj.base.flags |= 1;
    for (i = 0; i < SLOTS; i++) {
        SLOT(s, i) = SEEN(s, i) = REFILL(s, i) = 0;
        READY(s, i) = 1;
    }
    SqueakPlay(s, ANIM_FLOAT);
}

/* Its ghosts: their spots beside it (Squeak Squad's 20.12, Y up, x ahead: its params +0x84-0x98), when they come out (0x02104A70's
 * angles: 110, 66 and 132 degrees, cos and sin in 1/256ths), how long after its call each goes (+0x74-0x7C); */
static const s32 sSpotX[] = { -0x18000, -0x20000, 0x18000 }, sSpotY[] = { -0x14000, 0x1C000, -0x10000 };
static const s16 sOutCos[] = { -88, 104, -171 }, sOutSin[] = { 241, 234, 190 };
static const u8 sCallDelay[] = { 0, 30, 80 };
static const u8 sDoom[] = { 45, 80, 120 };
#define G_FOLLOW_CAP_X 0x199A       // +0xB0, +0xB4, +0xB8, +0xBC; within 32 pixels +0x9C, +0xA4 (and +0xA0, +0xA8 up and down);
#define G_FOLLOW_CAP_Y 0x1000
#define G_FOLLOW_ACCEL_X 0x290
#define G_FOLLOW_ACCEL_Y 0x19A
#define G_NEAR_CAP 0x199A
#define G_NEAR_ACCEL 0x290
#define G_SWEEP_CAP_X 0x3334        // +0xD8, +0xDC, +0xE0, +0xE4;
#define G_SWEEP_CAP_Y 0x800
#define G_SWEEP_ACCEL_X 0x267
#define G_SWEEP_ACCEL_Y 205
#define G_HOME_CAP 0x1000           // +0xC4, +0xC8, +0xCC, +0xD0;
#define G_HOME_ACCEL 205
#define G_SWEEP 600
#define G_BACK 360
#define G_HOME 180
#define G_APPEAR 16
#define G_SWEEP_EDGE 32

// GhostParent(); Its Tedhaun, if it's still there (a mid-boss Tedhaun, not beaten);
static struct SqueakObject *GhostParent(struct SqueakObject *g) {
    struct SqueakObject *p = G_PARENT(g);

    if (p == NULL || !SqueakIsMidboss(&p->obj) || p->enemy != SQUEAK_TEDHAUN || (p->obj.base.flags & 0x1000)) return NULL;
    return p;
}

/* Steer(); 0x02103FD8: toward (tx, ty) (KatAM's units): `far`/`near` caps and accels (near: within `within` across; near both ways
 * the up and down ones too); ahead, faster; behind, turned round (near its Tedhaun's spot (`keep`), facing as it, backing); */
static void Steer(struct SqueakObject *g, s32 tx, s32 ty, s32 capX, s32 capY, s32 accX, s32 accY, s32 within, struct SqueakObject *keep) {
    struct ObjectBase *b = &g->obj.base;
    s32 dx = tx - b->x, dy = b->y - ty, way = 0;
    bool32 left = b->flags & 1;

    if (keep && dx > -within && dx < within) {
        capX = G_NEAR_CAP;
        accX = G_NEAR_ACCEL;
        if ((keep->obj.base.flags & 1) != left) way = 2;
        else way = dx == 0 ? 0 : (dx > 0) != left ? 1 : -1;
        if (dy > -within && dy < within) capY = G_NEAR_CAP, accY = G_NEAR_ACCEL;
    } else if (dx != 0) {
        way = (dx > 0) != left ? 1 : 2;
    }
    if (way == 2) SqueakTurnAround(g);
    g->ax = way == 1 ? accX : way == -1 ? -accX : 0;
    g->ay = dy > 0 ? accY : dy < 0 ? -accY : 0;
    if (g->vx > capX) g->vx = capX, g->ax = 0;
    else if (g->vx < -capX) g->vx = -capX, g->ax = 0;
    if (g->vy > capY) g->vy = capY, g->ay = 0;
    else if (g->vy < -capY) g->vy = -capY, g->ay = 0;
}

// Spot(); Its spot by its Tedhaun (KatAM's units);
static void Spot(struct SqueakObject *g, struct SqueakObject *p, s32 *x, s32 *y) {
    s32 sx = SQUEAK_SPEED(sSpotX[g->variant]);

    *x = p->obj.base.x + (p->obj.base.flags & 1 ? -sx : sx);
    *y = p->obj.base.y - SQUEAK_SPEED(sSpotY[g->variant]);
}

static bool32 Ghost(struct SqueakObject *g) {
    struct SqueakObject *p = GhostParent(g);
    struct ObjectBase *b = &g->obj.base;
    struct Kirby *kirby = g->obj.kirby3;
    s32 x, y;

    // (0x02103F24: its Tedhaun gone, or beaten and its time over, it bursts);
    if (g->state != GHOST_BURST) {
        if (p == NULL) {
            SqueakSetState(g, GHOST_BURST);
            return TRUE;
        }
        if (p->obj.unk80 <= 0) {
            if (!G_DOOM(g)) G_DOOM(g) = sDoom[g->variant < SLOTS ? g->variant : 0];
            else if (--G_DOOM(g) < 1) {
                SqueakSetState(g, GHOST_BURST);
                return TRUE;
            }
        }
        if (g->variant < SLOTS) SEEN(p, g->variant) = 1;
    }
    switch (g->state) {
    case GHOST_START:       // 0x02104A70 (0x0210439C, the grab's: at the screen's top corner away from it, facing in);
        SqueakPlay(g, ANIM_GHOST);
        G_COUNT(g) = 0;
        if (g->variant >= SLOTS) {
            G_COUNT(g) = G_HOME;
            SqueakStop(g);
            SqueakSfx(g, 0x17C);
            if (p->obj.base.x > CameraX(g) + ((DISPLAY_WIDTH / 2) << 8)) {
                b->x = CameraX(g);
                b->flags &= ~1;
            } else {
                b->x = CameraX(g) + (0x100 << 8);
                b->flags |= 1;
            }
            b->y = CameraY(g);
            SqueakSetState(g, GHOST_HOME);
            return TRUE;
        }
        x = (sOutCos[g->variant] * GHOST_RADIUS) >> 8;
        g->vx = sOutCos[g->variant] * GHOST_OUT >> 8;
        g->vy = sOutSin[g->variant] * GHOST_OUT >> 8;
        b->x += SQUEAK_SPEED(b->flags & 1 ? -x : x);
        b->y -= SQUEAK_SPEED((sOutSin[g->variant] * GHOST_RADIUS) >> 8);
        SqueakSetState(g, GHOST_APPEAR);
        return TRUE;
    case GHOST_APPEAR:      // 0x021049A8: flickering G_APPEAR frames;
        if (G_COUNT(g) > G_APPEAR - 1) {
            b->flags &= ~0x400;
            G_COUNT(g) = 0;
            SqueakSetState(g, GHOST_FOLLOW);
            return TRUE;
        }
        if (G_COUNT(g) % (G_COUNT(g) < 5 ? 2 : G_COUNT(g) < 9 ? 4 : 6) < 1) b->flags |= 0x400;
        else b->flags &= ~0x400;
        G_COUNT(g)++;
        SqueakMove(g);
        break;
    case GHOST_FOLLOW:      // 0x021047B8: at its spot; ready (or back from a sweep, G_COUNT frames on); its Tedhaun calling, it goes;
        if (G_COUNT(g) == 0) {
            READY(p, g->variant) = 1;
            if (BUSY(p) == 4) {
                G_COUNT(g) = sCallDelay[g->variant];
                SqueakSetState(g, GHOST_WAIT);
                return TRUE;
            }
        } else {
            READY(p, g->variant) = 2;
            G_COUNT(g)--;
        }
        Spot(g, p, &x, &y);
        Steer(g, x, y, G_FOLLOW_CAP_X, G_FOLLOW_CAP_Y, G_FOLLOW_ACCEL_X, G_FOLLOW_ACCEL_Y, 0x2000, p);
        SqueakMove(g);
        break;
    case GHOST_WAIT:        // 0x021046B4: its call's delay at its spot, then G_SWEEP frames sweeping (0x021045C4);
        READY(p, g->variant) = 2;
        if (G_COUNT(g) < 1) {
            SqueakStop(g);
            G_COUNT(g) = G_SWEEP;
            SqueakSfx(g, 0x17C);
            SqueakSetState(g, GHOST_SWEEP);
            return TRUE;
        }
        G_COUNT(g)--;
        Spot(g, p, &x, &y);
        Steer(g, x, y, G_FOLLOW_CAP_X, G_FOLLOW_CAP_Y, G_FOLLOW_ACCEL_X, G_FOLLOW_ACCEL_Y, 0x2000, p);
        SqueakMove(g);
        break;
    case GHOST_SWEEP:       // 0x0210450C: across the screen and back at Kirby's height;
        READY(p, g->variant) = 2;
        if (G_COUNT(g) < 1) {
            G_COUNT(g) = G_BACK;
            SqueakSetState(g, GHOST_FOLLOW);
            return TRUE;
        }
        G_COUNT(g)--;
        x = CameraX(g) + ((b->flags & 1 ? G_SWEEP_EDGE : DISPLAY_WIDTH - G_SWEEP_EDGE + 16) << 8);
        Steer(g, x, kirby->base.y, G_SWEEP_CAP_X, G_SWEEP_CAP_Y, G_SWEEP_ACCEL_X, G_SWEEP_ACCEL_Y, 0, NULL);
        SqueakMove(g);
        break;
    case GHOST_HOME:        // 0x02104304: at Kirby, G_HOME frames;
        if (G_COUNT(g) < 1) {
            SqueakSetState(g, GHOST_BURST);
            return TRUE;
        }
        G_COUNT(g)--;
        Steer(g, kirby->base.x, kirby->base.y, G_HOME_CAP, G_HOME_CAP, G_HOME_ACCEL, G_HOME_ACCEL, NEAR >> 4, NULL);
        SqueakMove(g);
        break;
    case GHOST_BURST:       // 0x02103E1C, 0x02103E7C: it bursts and is gone;
        RequestScreenShake(1, b);       // (KatAM's shortest timed shake, 14 frames: 4-6 shake on till stopped);
        SqueakSpawnEffect(g, SQUEAK_FX_BURST, 0, 0, FALSE);
        SqueakSfx(g, 0x12D);
        b->flags |= 0x1000;
        break;
    }
    if (g->state != GHOST_BURST && !SqueakInLevel(g)) b->flags |= 0x1000;
    return FALSE;
}

static void TedhaunGhostMain(struct Object *obj) {
    struct SqueakObject *g = (struct SqueakObject *)obj;

    obj->kirby3 = FindTargetKirby(&obj->base);
    SqueakRun(g, Ghost);
    SqueakFinishFrame(g);
    SqueakAnimate(g);
    SqueakSetBoxes(g, &sTedhaunBoxData, G_HARM(g), &G_BOX(g));
}

// TedhaunGhostStart(); Its start (and after a hit: a hit ends it, its HP 1), through everything;
void TedhaunGhostStart(struct Object *obj) {
    struct SqueakObject *g = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, -1, TedhaunGhostMain);
    SqueakThroughTerrain(g, TRUE);
    if (!g->started) SqueakSetState(g, GHOST_START);
}

// TedhaunGhostInit(); Its box (tedhaun.rca's ghost's);
void TedhaunGhostInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -10, -19, 13, 0);
    ObjectSetBounds(&s->obj.base, -10, -19, 13, 0);
    G_HARM(s) = s->obj.base.unk68;
}
