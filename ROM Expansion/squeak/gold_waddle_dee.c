/* Gold Waddle Dee; ported from Kirby: Squeak Squad (USA), enemy 0x0E (its body 0x020B6764, brain 0x020B4EF4), as
 * SQUEAK_GOLD_WADDLE_DEE. Values from param/prmNE15Ml0.kgb (movement; its MlG, the other mode's, is the same) and prmNE15Pl0.kgb
 * (brain); speeds 20.12 a frame, Y up. Squeak Squad's enemies are a body (its moves) driven by a brain (a virtual pad); both are here.
 * • Brain: the spawn record's first parameter byte (KatAM's template unk12) picks it (0x020B4EF4); each first turns to face Kirby
 *   (0x020B6918, past 1 pixel):
 *   0 (and any other): walks on, the way it faces, while on the ground; a wall ahead turns it round (0x020B4DB0, every other frame);
 *   1: walks back and forth, turning 24 pixels either side of where it started, then walking 30 frames before it looks again;
 *   2: walks, and jumps once when Kirby is within 48 pixels across;
 *   3: walks, and jumps every 121 frames (once landed);
 *   4: as 0 (the first check doesn't ask for the ground);
 *   5: stands still.
 * • Body: stands (anim 1) and walks (anim 3) at 1.25, a puff of dust every 20 frames; A jumps (anim 4, then 5) at 2.5 up, falling
 *   0x19A a frame, no faster than 2.4; walking off a ledge it stops in the air for 5 frames (anim 2, "whoa"), then falls. In water it
 *   sinks (0x07B a frame, to 0.9) or swims (anim 12, 10 up, 11 down) at 1.2 the way its pad says, and jumps out once out of it.
 * Not ported: its sounds (0x6C jump, 0x118 ledge, 0x134, 0x139 landing, 9 step), and the other mode
 * (the brain's +8 not 3; never so for this brain): A in the air jumps again; */
#include "../squeak.h"

// Animations (gwaddledee.cea);
enum { ANIM_STILL, ANIM_STAND, ANIM_WHOA, ANIM_WALK, ANIM_JUMP, ANIM_FALL, ANIM_6, ANIM_7, ANIM_8, ANIM_9, ANIM_SWIM_UP, ANIM_SWIM_DOWN,
       ANIM_SWIM, ANIM_HURT, ANIM_HURT_2 };

// prmNE15Ml0.kgb;
#define WALK_SPEED 0x1400
#define JUMP_SPEED 0x2800
#define GRAVITY 0x19A
#define SWIM_SPEED 0x1334
#define MAX_FALL -0x2666
#define SINK_ACCEL -0x07B
#define SINK_SPEED -0xE66
// prmNE15Pl0.kgb;
#define PATROL_RANGE 0x18000
#define JUMP_RANGE 0x30000
#define JUMP_WAIT 120
#define PATROL_BACK 30
#define FACE_DISTANCE 0x1000
#define DUST_EVERY 20
#define LEDGE_HOVER 6

/* Body states: each a Squeak Squad state function; the entries (_ENTRY) set things up and chain to the state that runs every frame
 * (SqueakRun: a state returning TRUE runs the next one this same frame, as Squeak Squad's dispatcher does); */
enum {
    BODY_STAND_ENTRY, BODY_STAND, BODY_WALK_ENTRY, BODY_WALK, BODY_JUMP_ENTRY, BODY_JUMP, BODY_JUMP_FALL, BODY_LEDGE_ENTRY, BODY_LEDGE,
    BODY_LEDGE_FALL, BODY_B_ENTRY, BODY_B, BODY_SWIM_IDLE_ENTRY, BODY_SWIM_IDLE, BODY_SWIM_ENTRY, BODY_SWIM, BODY_SWIM_DOWN_FWD_ENTRY,
    BODY_SWIM_DOWN_FWD, BODY_SWIM_UP_FWD_ENTRY, BODY_SWIM_UP_FWD, BODY_SWIM_DOWN_ENTRY, BODY_SWIM_DOWN, BODY_SWIM_UP_ENTRY,
    BODY_SWIM_UP,
};
// Brain states (one each frame);
enum {
    BRAIN_FACE, BRAIN_WALK, BRAIN_WALK_HOLD, BRAIN_PATROL_FACE, BRAIN_PATROL, BRAIN_PATROL_BACK, BRAIN_JUMP_AT_FACE, BRAIN_JUMP_AT,
    BRAIN_JUMPED, BRAIN_HOP_FACE, BRAIN_HOP, BRAIN_HOP_WAIT, BRAIN_WALK2_FACE, BRAIN_WALK2, BRAIN_WALK2_HOLD, BRAIN_STAND_FACE,
    BRAIN_STAND,
};

// The body's and brain's own fields;
#define COUNTER(s) ((s)->vars[0])       // 0x200 (the walk's dust), 0x204 (the ledge's hover);
#define BRAIN_TIMER(s) ((s)->vars[1])   // The brain's 0xA4;
#define HOME_X(s) ((s)->vars[2])        // The brain's 0x9C: where it started (variant 1);

// InWater(); vtable +0x80 (0x020B5078), first in each state on land: in water, it swims;
static bool32 InWater(struct SqueakObject *s) {
    if (SqueakInWater(s)) {
        SqueakSetState(s, BODY_SWIM_IDLE_ENTRY);
        return TRUE;
    }
    return FALSE;
}

// PressedA(); 0x020B52CC: A jumps;
static bool32 PressedA(struct SqueakObject *s) {
    if (s->padA) {
        SqueakSetState(s, BODY_JUMP_ENTRY);
        return TRUE;
    }
    return FALSE;
}

// PressedB(); 0x020B5288;
static bool32 PressedB(struct SqueakObject *s) {
    if (s->padB) {
        SqueakSetState(s, BODY_B_ENTRY);
        return TRUE;
    }
    return FALSE;
}

// AirControl(); 0x020B65D8: in the air, its pad turns it round, forward moves it at its walking speed, nothing stops it across;
static void AirControl(struct SqueakObject *s) {
    if (s->padDir < 0) SqueakTurnAround(s);
    if (s->padDir > 0) {
        s->vx = WALK_SPEED;
        s->maxVx = 0;
    } else if (s->padDir == 0) {
        s->vx = 0;
    }
}

/* Swim checks (0x020B5030-0x020B523C): out of the water it jumps; else the pad picks the swim (up/down, across); `self` is the state
 * asking, which doesn't check for itself; */
static bool32 SwimChecks(struct SqueakObject *s, u8 self) {
    u8 next;

    if (!SqueakInWater(s)) {
        SqueakSetState(s, BODY_JUMP_ENTRY);
        return TRUE;
    }
    if (s->padY == 0) next = s->padDir == 0 ? BODY_SWIM_IDLE_ENTRY : BODY_SWIM_ENTRY;
    else if (s->padY > 0) next = s->padDir == 0 ? BODY_SWIM_UP_ENTRY : BODY_SWIM_UP_FWD_ENTRY;
    else next = s->padDir == 0 ? BODY_SWIM_DOWN_ENTRY : BODY_SWIM_DOWN_FWD_ENTRY;
    if (next == self) return FALSE;
    SqueakSetState(s, next);
    return TRUE;
}

// Swim(); A swim's every frame (0x020B5354, 0x020B54C8, 0x020B5638, 0x020B5790, 0x020B58EC): its pad turns it round;
static bool32 Swim(struct SqueakObject *s, u8 entry) {
    if (SwimChecks(s, entry)) return TRUE;
    if (s->padDir < 0) SqueakTurnAround(s);
    SqueakMove(s);
    return FALSE;
}

static bool32 Body(struct SqueakObject *s) {
    switch (s->state) {
    case BODY_STAND_ENTRY:  // 0x020B6530 (vtable +0x3C: its start);
        SqueakPlay(s, ANIM_STAND);
        SqueakStop(s);
        SqueakSetState(s, BODY_STAND);
        return TRUE;
    case BODY_STAND:        // 0x020B6494;
        if (InWater(s) || PressedA(s)) return TRUE;
        if (s->padDir != 0) {
            SqueakSetState(s, BODY_WALK_ENTRY);
            return TRUE;
        }
        if (PressedB(s)) return TRUE;
        SqueakMove(s);
        if (!SqueakOnGround(s)) SqueakSetState(s, BODY_LEDGE_ENTRY);
        return FALSE;
    case BODY_WALK_ENTRY:   // 0x020B63E4;
        SqueakPlay(s, ANIM_WALK);
        SqueakStop(s);
        SqueakSetState(s, BODY_WALK);
        COUNTER(s) = 0;
        return TRUE;
    case BODY_WALK:         // 0x020B62A8;
        if (InWater(s) || PressedB(s) || PressedA(s)) return TRUE;
        if (s->padDir == 0) {
            SqueakSetState(s, BODY_STAND_ENTRY);
            return TRUE;
        }
        s->vx = WALK_SPEED;
        if (s->padDir < 0) SqueakTurnAround(s);
        SqueakMove(s);
        if (!SqueakOnGround(s)) {
            SqueakSetState(s, BODY_LEDGE_ENTRY);
            return FALSE;
        }
        // (every 20 frames from the first, a puff of dust, and sound 9);
        if (COUNTER(s) % DUST_EVERY == 0) SqueakSpawnEffect(s, SQUEAK_FX_DUST, 0, 0, FALSE);
        COUNTER(s)++;
        return FALSE;
    case BODY_JUMP_ENTRY:   // 0x020B61AC;
        SqueakPlay(s, ANIM_JUMP);
        SqueakStop(s);
        s->vy = JUMP_SPEED;
        s->ay = -GRAVITY;
        SqueakSetState(s, BODY_JUMP);
        return TRUE;
    case BODY_JUMP:         // 0x020B60B4: anim 4 over, anim 5;
        if (InWater(s)) return TRUE;
        AirControl(s);
        if (s->animEnded) {
            SqueakPlay(s, ANIM_FALL);
            SqueakSetState(s, BODY_JUMP_FALL);
            return TRUE;
        }
        if (s->vy < 0 && PressedA(s)) return TRUE;
        SqueakCapFall(s, MAX_FALL);
        SqueakMove(s);
        return FALSE;
    case BODY_JUMP_FALL:    // 0x020B6020;
        if (InWater(s)) return TRUE;
        AirControl(s);
        if (s->vy < 0 && PressedA(s)) return TRUE;
        SqueakMove(s);
        if (SqueakOnGround(s)) SqueakSetState(s, BODY_STAND_ENTRY);
        return FALSE;
    case BODY_LEDGE_ENTRY:  // 0x020B5F60: off a ledge;
        SqueakPlay(s, ANIM_WHOA);
        SqueakStop(s);
        SqueakSetState(s, BODY_LEDGE);
        COUNTER(s) = 0;
        return TRUE;
    case BODY_LEDGE:        // 0x020B5D88: 5 frames in the air, then falls; anim 2 over, anim 5;
        if (InWater(s)) return TRUE;
        if (++COUNTER(s) == LEDGE_HOVER) {
            s->maxVx = s->maxVy = s->ax = s->fx = s->fy = 0;
            s->ay = -GRAVITY;
        } else if (COUNTER(s) > LEDGE_HOVER) {
            if (s->animEnded) {
                SqueakPlay(s, ANIM_FALL);
                SqueakSetState(s, BODY_LEDGE_FALL);
                return TRUE;
            }
            SqueakCapFall(s, MAX_FALL);
        }
        SqueakMove(s);
        if (SqueakOnGround(s)) SqueakSetState(s, BODY_STAND_ENTRY);
        return FALSE;
    case BODY_LEDGE_FALL:   // 0x020B5CD0;
        if (InWater(s)) return TRUE;
        AirControl(s);
        SqueakCapFall(s, MAX_FALL);
        SqueakMove(s);
        if (SqueakOnGround(s)) SqueakSetState(s, BODY_STAND_ENTRY);
        return FALSE;
    case BODY_B_ENTRY:      // 0x020B5C18 (B: its brains never press it);
        SqueakPlay(s, ANIM_WHOA);
        SqueakStop(s);
        SqueakSetState(s, BODY_B);
        return TRUE;
    case BODY_B:            // 0x020B5B6C;
        if (InWater(s)) return TRUE;
        if (s->animEnded) {
            SqueakSetState(s, SqueakOnGround(s) ? BODY_STAND_ENTRY : BODY_LEDGE_ENTRY);
            return TRUE;
        }
        AirControl(s);
        SqueakCapFall(s, MAX_FALL);
        SqueakMove(s);
        return FALSE;
    case BODY_SWIM_IDLE_ENTRY:  // 0x020B5AC4;
        SqueakPlay(s, ANIM_STAND);
        SqueakStop(s);
        SqueakSetState(s, BODY_SWIM_IDLE);
        return TRUE;
    case BODY_SWIM_IDLE:    // 0x020B5A44: sinks;
        if (SwimChecks(s, BODY_SWIM_IDLE_ENTRY)) return TRUE;
        SqueakSinkTo(s, SINK_ACCEL, SINK_SPEED);
        SqueakMove(s);
        return FALSE;
    case BODY_SWIM_ENTRY:   // 0x020B56E8;
        SqueakPlay(s, ANIM_SWIM);
        SqueakStop(s);
        SqueakSetState(s, BODY_SWIM);
        return TRUE;
    case BODY_SWIM:         // 0x020B5638: across;
        s->vx = SWIM_SPEED;
        return Swim(s, BODY_SWIM_ENTRY);
    case BODY_SWIM_DOWN_FWD_ENTRY:  // 0x020B5420;
        SqueakPlay(s, ANIM_SWIM);
        SqueakStop(s);
        SqueakSetState(s, BODY_SWIM_DOWN_FWD);
        return TRUE;
    case BODY_SWIM_DOWN_FWD:    // 0x020B5354;
        s->vx = SWIM_SPEED;
        s->vy = -SWIM_SPEED;
        return Swim(s, BODY_SWIM_DOWN_FWD_ENTRY);
    case BODY_SWIM_UP_FWD_ENTRY:    // 0x020B5590;
        SqueakPlay(s, ANIM_SWIM);
        SqueakStop(s);
        SqueakSetState(s, BODY_SWIM_UP_FWD);
        return TRUE;
    case BODY_SWIM_UP_FWD:  // 0x020B54C8;
        s->vx = SWIM_SPEED;
        s->vy = SWIM_SPEED;
        return Swim(s, BODY_SWIM_UP_FWD_ENTRY);
    case BODY_SWIM_DOWN_ENTRY:  // 0x020B5810;
        SqueakPlay(s, ANIM_SWIM_DOWN);
        SqueakStop(s);
        s->vy = -SWIM_SPEED;
        SqueakSetState(s, BODY_SWIM_DOWN);
        return TRUE;
    case BODY_SWIM_DOWN:    // 0x020B5790;
        return Swim(s, BODY_SWIM_DOWN_ENTRY);
    case BODY_SWIM_UP_ENTRY:    // 0x020B596C;
        SqueakPlay(s, ANIM_SWIM_UP);
        SqueakStop(s);
        s->vy = SWIM_SPEED;
        SqueakSetState(s, BODY_SWIM_UP);
        return TRUE;
    case BODY_SWIM_UP:      // 0x020B58EC;
        return Swim(s, BODY_SWIM_UP_ENTRY);
    }
    return FALSE;
}

static void Brain(struct SqueakObject *s) {
    switch (s->brain) {
    case BRAIN_FACE:        // 0x020B4E28;
        SqueakFaceKirby(s, FACE_DISTANCE);
        s->brain = BRAIN_WALK;
        break;
    case BRAIN_WALK:        // 0x020B4DB0: on the ground forward, a wall ahead turns it round;
        s->padDir = SqueakOnGround(s) ? 1 : 0;
        if (SqueakWallAhead(s)) s->padDir = -s->padDir;
        s->brain = BRAIN_WALK_HOLD;
        break;
    case BRAIN_WALK_HOLD:   // 0x020B4DA0;
        s->brain = BRAIN_WALK;
        break;
    case BRAIN_PATROL_FACE: // 0x020B4D6C;
        SqueakFaceKirby(s, FACE_DISTANCE);
        s->brain = BRAIN_PATROL;
        HOME_X(s) = SQ_X(&s->obj);
        break;
    case BRAIN_PATROL:      // 0x020B4CC0: 24 pixels past where it started, it turns round;
        s->padDir = 1;
        if (s->obj.base.flags & 1 ? SQ_X(&s->obj) < HOME_X(s) - PATROL_RANGE : SQ_X(&s->obj) > HOME_X(s) + PATROL_RANGE) {
            s->padDir = -1;
            BRAIN_TIMER(s) = PATROL_BACK;
            s->brain = BRAIN_PATROL_BACK;
        }
        break;
    case BRAIN_PATROL_BACK: // 0x020B4C88;
        s->padDir = 1;
        if (--BRAIN_TIMER(s) < 0) {
            BRAIN_TIMER(s) = 0;
            s->brain = BRAIN_PATROL;
        }
        break;
    case BRAIN_JUMP_AT_FACE:    // 0x020B4C68;
        SqueakFaceKirby(s, FACE_DISTANCE);
        s->brain = BRAIN_JUMP_AT;
        break;
    case BRAIN_JUMP_AT:     // 0x020B4C38: Kirby within 48 pixels across, A;
        s->padDir = 1;
        if (SqueakKirbyWithin(s, JUMP_RANGE, 0)) {
            s->padA = 1;
            s->brain = BRAIN_JUMPED;
        }
        break;
    case BRAIN_JUMPED:      // 0x020B4C30 (for good);
        s->padA = 0;
        break;
    case BRAIN_HOP_FACE:    // 0x020B4C04;
        SqueakFaceKirby(s, FACE_DISTANCE);
        s->brain = BRAIN_HOP;
        BRAIN_TIMER(s) = JUMP_WAIT;
        break;
    case BRAIN_HOP:         // 0x020B4BCC;
        s->padDir = 1;
        if (BRAIN_TIMER(s) < 0) {
            s->padA = 1;
            s->brain = BRAIN_HOP_WAIT;
            break;
        }
        BRAIN_TIMER(s)--;
        break;
    case BRAIN_HOP_WAIT:    // 0x020B4B9C: landed (no speed up or down), it waits again;
        s->padA = 0;
        if (s->vy == 0) {
            BRAIN_TIMER(s) = JUMP_WAIT;
            s->brain = BRAIN_HOP;
        }
        break;
    case BRAIN_WALK2_FACE:  // 0x020B4B48;
        SqueakFaceKirby(s, FACE_DISTANCE);
        s->brain = BRAIN_WALK2;
        break;
    case BRAIN_WALK2:       // 0x020B4AE4;
        s->padDir = 1;
        if (SqueakWallAhead(s)) s->padDir = -s->padDir;
        s->brain = BRAIN_WALK2_HOLD;
        break;
    case BRAIN_WALK2_HOLD:  // 0x020B4AD4;
        s->brain = BRAIN_WALK;
        break;
    case BRAIN_STAND_FACE:  // 0x020B4B70;
        SqueakFaceKirby(s, FACE_DISTANCE);
        s->brain = BRAIN_STAND;
        BRAIN_TIMER(s) = JUMP_WAIT;
        break;
    case BRAIN_STAND:       // 0x020B4B68;
        s->padDir = 0;
        break;
    }
}

static void GoldWaddleDeeMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    Brain(s);
    SqueakRun(s, Body);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
}

// The brain's first state per variant (0x020B4EF4's switch on the spawn's first parameter byte);
static const u8 sFirstBrain[] = { BRAIN_FACE, BRAIN_PATROL_FACE, BRAIN_JUMP_AT_FACE, BRAIN_HOP_FACE, BRAIN_WALK2_FACE, BRAIN_STAND_FACE };

void GoldWaddleDeeStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, 0, GoldWaddleDeeMain);
    // (after a hit only the body starts over, at vtable +0x3C; the brain goes on);
    if (!s->started) s->brain = s->variant < ARRAY_COUNT(sFirstBrain) ? sFirstBrain[s->variant] : BRAIN_FACE;
    SqueakSetState(s, BODY_STAND_ENTRY);
}

// GoldWaddleDeeInit(); Its box (gwaddledee.rca's first) and its feet on the ground;
void GoldWaddleDeeInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -6, -12, 6, 0);
    ObjectSetBounds(&s->obj.base, -6, -12, 6, 0);
    SqueakStandOnFloor(&s->obj);
}
