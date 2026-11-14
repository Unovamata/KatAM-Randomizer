/* Crimp; ported from Kirby: Squeak Squad (USA), enemy 0x0C (its body 0x020CE310, brain 0x020CC53C, its rock 0x020CBF94), as
 * SQUEAK_CRIMP and SQUEAK_CRIMP_ROCK. Values from param/prmNE13Ml0.kgb (movement, the rock's too; MlG the same), prmNE13Pl0.kgb
 * (brain) and prmNE13Sl0.kgb (HP 10); speeds 20.12 a frame, Y up. It clings to walls, its back to them, climbs up and down them
 * and throws rocks at Kirby.
 * • Brain (0x020CC478, any variant): on a wall it climbs up (Kirby below it: down) and turns back at the wall's end (14 pixels past
 *   it no wall: 0x020CC630), at a ceiling or the ground; Kirby within 64 pixels each way, it throws (B), then waits 160 frames. On
 *   the ground it presses A once and a direction (half the time turning round), waits 10 frames, and once on the ground or at a wall
 *   goes back to climbing; neither does anything to its body in this mode.
 * • Body: stands (anim 8) or falls (anim 7, 0x19A a frame, no faster than 2.4; its pad moves it at 1.2 in the air); a wall ahead or
 *   beside it, it grabs it (anim 1), its back to it, and clings, pushing into it (1.0 a frame); its pad climbs it up or down at 0.3
 *   (anim 2) while there's wall beside it 8 pixels on; B throws (anim 3): 20 frames in, a rock leaves 18 pixels ahead, 6 up,
 *   flying on at 1.6 and falling 0.2 a frame (anim 11), bursting (efc_cmn1's animation 1) on whatever it hits. No wall beside it
 *   any more, it starts over (stands, or falls).
 * Its sounds (Squeak Squad's, SqueakSfx): 0x12F the throw, 0x135 the rock bursting. Its walls are fully solid tiles only
 * (SqueakTouchesWall: whole slope cells made it cling to the air over ledges; Squeak Squad's also takes a slope under its surface,
 * not ported: its spots are all beside walls). The stage's wall-distance nudge (vtable +0x38, +0x3C) only at its spot (OutOfWall:
 * its box starts in its wall; KatAM's terrain keeps it out after that). Not ported: the rock's burst when an attack hits it
 * (KatAM's own defeat instead), and the other mode (the brain's +8 not 3; never so for this brain): A jumps off walls (2.5 up). */
#include "../squeak.h"
#include "random.h"

// Animations (climb.cea);
enum { ANIM_STILL, ANIM_CLING, ANIM_CLIMB, ANIM_THROW, ANIM_FALL = 7, ANIM_STAND, ANIM_HURT, ANIM_HURT_2, ANIM_ROCK };

// prmNE13Ml0.kgb;
#define AIR_SPEED 0x1334
#define GRAVITY 0x19A
#define MAX_FALL -0x2667
#define CLIMB_SPEED 0x4CD
#define ROCK_X 0x12000
#define ROCK_Y 0x6000
#define ROCK_SPEED 0x199A
#define ROCK_GRAVITY 0x334
#define PUSH -0x1000
#define PROBE 0x8000
#define WALL_END 0xE000
// prmNE13Pl0.kgb;
#define THROW_RANGE_X 0x40000
#define THROW_RANGE_Y 0x40000
#define TURN_CHANCE 50
#define GROUND_WAIT 10
#define THROW_WAIT 160
#define THROW_AT 20

enum {
    BODY_START, BODY_STAND, BODY_FALL_ENTRY, BODY_FALL, BODY_GRAB, BODY_CLING, BODY_CLIMB_UP_ENTRY, BODY_CLIMB_UP,
    BODY_CLIMB_DOWN_ENTRY, BODY_CLIMB_DOWN, BODY_THROW_ENTRY, BODY_THROW,
};
enum { BRAIN_START, BRAIN_SIDE, BRAIN_CLIMB, BRAIN_THROW, BRAIN_THROWN, BRAIN_GROUND, BRAIN_GROUND_PRESS, BRAIN_GROUND_WAIT, BRAIN_LAND };

#define COUNTER(s) ((s)->vars[0])       // 0x1F8: the throw's frames;
#define CLIMB(s) ((s)->vars[1])         // The brain's 0x98: 1 up, -1 down;
#define BRAIN_WAIT(s) ((s)->vars[2])    // 0x9C;
#define THROW_TIMER(s) ((s)->vars[3])   // 0xA0;

// AirControl(); 0x020CE20C: its pad turns it round, forward moves it on at 1.2, nothing stops it across;
static void AirControl(struct SqueakObject *s) {
    if (s->padDir < 0) SqueakTurnAround(s);
    if (s->padDir > 0) {
        s->vx = AIR_SPEED;
        s->maxVx = 0;
    }
}

// Still(); The speeds' caps, accelerations and friction off (the fall's entry; the speeds stay);
static void Still(struct SqueakObject *s) {
    s->maxVx = s->maxVy = s->ax = s->ay = s->fx = s->fy = 0;
}

// Beside(); A wall beside it (left or right), `dy` up from it (walls only: a slope or a thin floor isn't one);
static bool32 Beside(struct SqueakObject *s, s32 dy) {
    return SqueakTouchesWall(s, SQUEAK_LEFT, dy, FALSE) || SqueakTouchesWall(s, SQUEAK_RIGHT, dy, FALSE);
}

// PressedB(); 0x020CC8E4;
static bool32 PressedB(struct SqueakObject *s) {
    if (s->padB) {
        SqueakSetState(s, BODY_THROW_ENTRY);
        return TRUE;
    }
    return FALSE;
}

static bool32 Body(struct SqueakObject *s) {
    switch (s->state) {
    case BODY_START:        // 0x020CE16C (vtable +0x3C);
        SqueakPlay(s, ANIM_STAND);
        SqueakStop(s);
        SqueakSetState(s, BODY_STAND);
        return TRUE;
    case BODY_STAND:        // 0x020CDEA4: a wall ahead or beside it (all along), it grabs it; off the ground, it falls;
        if (SqueakWallAhead(s) || SqueakTouchesWall(s, SQUEAK_LEFT, 0, TRUE) || SqueakTouchesWall(s, SQUEAK_RIGHT, 0, TRUE)) {
            SqueakSetState(s, BODY_GRAB);
            break;
        }
        if (!SqueakOnGround(s)) SqueakSetState(s, BODY_FALL_ENTRY);
        break;
    case BODY_FALL_ENTRY:   // 0x020CDBCC;
        SqueakPlay(s, ANIM_FALL);
        Still(s);
        s->ay = -GRAVITY;
        SqueakSetState(s, BODY_FALL);
        return TRUE;
    case BODY_FALL:         // 0x020CDACC;
        AirControl(s);
        SqueakCapFall(s, MAX_FALL);
        SqueakMove(s);
        if (SqueakWallAhead(s)) SqueakSetState(s, BODY_GRAB);
        else if (SqueakOnGround(s)) SqueakSetState(s, BODY_START);
        break;
    case BODY_GRAB:         // 0x020CD8DC: its back to the wall;
        SqueakPlay(s, ANIM_CLING);
        SqueakStop(s);
        if (SqueakTouchesWall(s, SQUEAK_LEFT, 0, TRUE)) {
            if (s->obj.base.flags & 1) SqueakTurnAround(s);
            SqueakSetState(s, BODY_CLING);
        } else if (SqueakTouchesWall(s, SQUEAK_RIGHT, 0, TRUE)) {
            if (!(s->obj.base.flags & 1)) SqueakTurnAround(s);
            SqueakSetState(s, BODY_CLING);
        } else {
            SqueakSetState(s, BODY_FALL_ENTRY);
        }
        return TRUE;
    case BODY_CLING:        // 0x020CD5E4;
        if (PressedB(s)) return TRUE;
        if (s->padY > 0) {
            SqueakPlay(s, ANIM_CLIMB);
            SqueakSetState(s, BODY_CLIMB_UP);
            return TRUE;
        }
        if (s->padY < 0) {
            SqueakPlay(s, ANIM_CLIMB);
            SqueakSetState(s, BODY_CLIMB_DOWN);
            return TRUE;
        }
        s->vx = PUSH;
        s->vy = 0;
        SqueakMove(s);
        if (!Beside(s, 0)) SqueakSetState(s, BODY_START);
        break;
    case BODY_CLIMB_UP_ENTRY:   // 0x020CD550;
        SqueakPlay(s, ANIM_CLIMB);
        SqueakSetState(s, BODY_CLIMB_UP);
        return TRUE;
    case BODY_CLIMB_UP:     // 0x020CD1E8: no wall beside it 8 pixels up, it starts over;
        if (PressedB(s)) return TRUE;
        if (s->padY < 0) {
            SqueakSetState(s, BODY_CLIMB_DOWN_ENTRY);
            return TRUE;
        }
        if (s->padY == 0) {
            SqueakPlay(s, ANIM_CLING);
            SqueakSetState(s, BODY_CLING);
            return TRUE;
        }
        s->vx = PUSH;
        s->vy = CLIMB_SPEED;
        SqueakMove(s);
        if (!Beside(s, PROBE)) SqueakSetState(s, BODY_START);
        break;
    case BODY_CLIMB_DOWN_ENTRY: // 0x020CD154;
        SqueakPlay(s, ANIM_CLIMB);
        SqueakSetState(s, BODY_CLIMB_DOWN);
        return TRUE;
    case BODY_CLIMB_DOWN:   // 0x020CCDF0: no wall beside it 8 pixels down and nothing under that, or on the ground, it starts over;
        if (PressedB(s)) return TRUE;
        if (s->padY > 0) {
            SqueakSetState(s, BODY_CLIMB_UP_ENTRY);
            return TRUE;
        }
        if (s->padY == 0) {
            SqueakPlay(s, ANIM_CLING);
            SqueakSetState(s, BODY_CLING);
            return TRUE;
        }
        s->vx = PUSH;
        s->vy = -CLIMB_SPEED;
        SqueakMove(s);
        if (!Beside(s, -PROBE) && (!SqueakTouches(s, SQUEAK_DOWN, -PROBE, FALSE) || SqueakOnGround(s)))
            SqueakSetState(s, BODY_START);
        break;
    case BODY_THROW_ENTRY:  // 0x020CCBBC;
        SqueakPlay(s, ANIM_THROW);
        Still(s);
        s->vy = 0;
        COUNTER(s) = 0;
        SqueakSetState(s, BODY_THROW);
        return TRUE;
    case BODY_THROW:        // 0x020CC974: 20 frames in, the rock (0x020CC60C); once the throw's over, it clings;
        if (s->animEnded) {
            SqueakSetState(s, BODY_CLING);
            return TRUE;
        }
        if (COUNTER(s)++ == THROW_AT) SqueakSpawn(s, SQUEAK_CRIMP_ROCK, 0, ROCK_X, ROCK_Y);
        s->vx = PUSH;
        s->vy = 0;
        SqueakMove(s);
        if (!Beside(s, 0) && (!SqueakTouches(s, SQUEAK_DOWN, 0, FALSE) || SqueakOnGround(s))) SqueakSetState(s, BODY_START);
        break;
    }
    return FALSE;
}

// WallEnds(); 0x020CC630: 14 pixels up (0) or down (1), no wall beside it nor above (below) it;
static bool32 WallEnds(struct SqueakObject *s, bool8 down) {
    s32 dy = down ? -WALL_END : WALL_END;

    return !Beside(s, dy) && !SqueakTouches(s, down ? SQUEAK_DOWN : SQUEAK_UP, dy, FALSE);
}

static void Brain(struct SqueakObject *s) {
    switch (s->brain) {
    case BRAIN_START:       // 0x020CC478;
        s->padDir = s->padY = 0;
        s->brain = BRAIN_SIDE;
        break;
    case BRAIN_SIDE:        // 0x020CC434: Kirby above it (or level), up; below, down;
        s->padDir = s->padY = 0;
        s->brain = BRAIN_CLIMB;
        CLIMB(s) = SQ_Y(s->obj.kirby3) >= SQ_Y(&s->obj) ? 1 : -1;
        break;
    case BRAIN_CLIMB:       // 0x020CC314;
        if (s->terrain & 2) {
            // (on its wall, behind it: Kirby near, it throws once the wait's over);
            if (THROW_TIMER(s) < 1) {
                if (SqueakKirbyWithin(s, THROW_RANGE_X, THROW_RANGE_Y)) {
                    s->padDir = s->padY = 0;
                    s->padB = 1;
                    s->brain = BRAIN_THROW;
                    break;
                }
            } else {
                THROW_TIMER(s)--;
            }
        } else if (SqueakOnGround(s)) {
            s->brain = BRAIN_GROUND;
        }
        if (SqueakCeiling(s)) CLIMB(s) = -1;
        if (SqueakOnGround(s)) CLIMB(s) = 1;
        if (CLIMB(s) == 1 && WallEnds(s, FALSE)) CLIMB(s) = -1;
        if (CLIMB(s) == -1 && WallEnds(s, TRUE)) CLIMB(s) = 1;
        s->padDir = 0;
        s->padY = CLIMB(s);
        break;
    case BRAIN_THROW:       // 0x020CC2F8 (B held a frame), 0x020CC2D4;
        s->padB = 0;
        THROW_TIMER(s) = THROW_WAIT;
        s->brain = BRAIN_CLIMB;
        break;
    case BRAIN_GROUND:      // 0x020CC268: half the time it turns round (Squeak Squad's random 0-99 under 50: not; KatAM's numbers);
        if ((Rand16() * 100 >> 16) >= TURN_CHANCE) {
            s->padDir = -1;
            s->padY = 0;
        }
        s->brain = BRAIN_GROUND_PRESS;
        break;
    case BRAIN_GROUND_PRESS:    // 0x020CC23C;
        s->padA = 1;
        s->padDir = 1;
        s->padY = 0;
        BRAIN_WAIT(s) = GROUND_WAIT;
        s->brain = BRAIN_GROUND_WAIT;
        break;
    case BRAIN_GROUND_WAIT: // 0x020CC1FC;
        s->padA = 0;
        s->padDir = s->padY = 0;
        if (BRAIN_WAIT(s) == 0) s->brain = BRAIN_LAND;
        else BRAIN_WAIT(s)--;
        break;
    case BRAIN_LAND:        // 0x020CC1C4;
        if (SqueakOnGround(s) || (s->terrain & 3)) s->brain = BRAIN_CLIMB;
        break;
    }
}

static void CrimpMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    Brain(s);
    SqueakRun(s, Body);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
}

void CrimpStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, 0, CrimpMain);
    if (!s->started) s->brain = BRAIN_START;
    SqueakSetState(s, BODY_START);
}

/* OutOfWall(); Squeak Squad's wall-distance nudge (the stage's vtable +0x38, +0x3C): its spot is in the cell beside its wall, its
 * box up to 16 pixels into it; out of the wall, its edge against it (either end of its side, its feet's row or its head's); */
static void OutOfWall(struct SqueakObject *s) {
    struct ObjectBase *b = &s->obj.base;
    s32 head = b->y + b->unk3D * 0x100, feet = b->y + b->unk3F * 0x100 - 0x100;
    u8 i;

    for (i = 0; i < 16; i++) {
        s32 right = b->x + b->unk3E * 0x100, left = b->x + b->unk3C * 0x100;

        // (its edge on the wall's: the probes look 1/256 of a pixel past it);
        if (SqueakIsWall(&s->obj, right, feet) || SqueakIsWall(&s->obj, right, head)) b->x = (b->x - 0x100) | 0xFF;
        else if (SqueakIsWall(&s->obj, left, feet) || SqueakIsWall(&s->obj, left, head)) b->x = (b->x + 0x100) & ~0xFF;
        else break;
    }
    b->unk48 = b->x;
}

/* CrimpInit(); Its box (climb.rca's first: 12 pixels either side, 3 to 18 up; as the others', from its feet), facing left; out of
 * its wall, and on the floor only when there's no wall beside it (its spots are beside walls, often over the floor: Squeak Squad's
 * Crimps cling there from the start); */
void CrimpInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -12, -15, 12, 0);
    ObjectSetBounds(&s->obj.base, -12, -15, 12, 0);
    s->obj.base.flags |= 1;
    OutOfWall(s);
    if (!SqueakTouchesWall(s, SQUEAK_LEFT, 0, TRUE) && !SqueakTouchesWall(s, SQUEAK_RIGHT, 0, TRUE)) SqueakStandOnFloor(&s->obj);
}

/* Its rock (0x020CBD0C, 0x020CBCA8): flies on at 1.6, falling 0.2 a frame; on whatever it touches it bursts; out of the stage, it's
 * gone; */
static bool32 Rock(struct SqueakObject *s) {
    switch (s->state) {
    case 0:
        SqueakPlay(s, ANIM_ROCK);
        SqueakSfx(s, 0x12F);
        s->vx = ROCK_SPEED;
        s->maxVy = 0;
        s->ay = -ROCK_GRAVITY;
        SqueakSetState(s, 1);
        return TRUE;
    case 1:
        SqueakMove(s);
        if (s->terrain || SqueakOnGround(s)) {
            SqueakSpawnEffect(s, SQUEAK_FX_BURST, 0, 0, FALSE);
            SqueakSfx(s, 0x135);
            s->obj.base.flags |= 0x1000;
        } else if (!SqueakInLevel(s)) {
            s->obj.base.flags |= 0x1000;
        }
        break;
    }
    return FALSE;
}

static void CrimpRockMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    SqueakRun(s, Rock);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
}

void CrimpRockStart(struct Object *obj) {
    ObjectSetFunc(obj, 0, CrimpRockMain);
    SqueakSetState((struct SqueakObject *)obj, 0);
}

/* CrimpRockInit(); Its box (climb.rca's second: 6 pixels either side, 5 to 15 up, from its feet); Kirby's body doesn't hurt it, nor
 * shows its HP bar (its hit handler 0x020CBDDC hands touches to 0x020B4964, which takes none: it hurts him and flies on to the
 * terrain; KatAM's touch knocked it back into its Crimp); */
void CrimpRockInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -6, -10, 5, 0);
    ObjectSetBounds(&s->obj.base, -6, -10, 5, 0);
    s->obj.base.unk5C |= 0x80;
    s->obj.base.flags |= 0x1000000;
}
