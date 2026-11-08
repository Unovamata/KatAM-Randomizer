/* Perara; ported from Kirby: Squeak Squad (USA), enemy 0x0D (its body 0x0206FD54, brain 0x0206E85C), as SQUEAK_PERARA.
 * Squeak Squad's enemies are a body (its moves) driven by a brain (a virtual pad: a direction, buttons A and B); both are here.
 * Values from param/prmNE14Ml0.kgb (movement) and prmNE14Pl0.kgb (brain); speeds 20.12 a frame, Y up:
 * • Brain: faces Kirby (0x0206FF08, past 1 pixel), then waits for him within 60 pixels across and 120 up or down (0x0206FF94);
 *   then presses B; on the ground it waits up to 300 frames for him within 80 pixels across (0x0206FF64) and presses A (jump), then
 *   after 80 frames presses B again (float) and so on.
 * • Body: in the air it floats in place (0x0206FAF0: anim 1) and settles (anim 2) on the ground (0x0206F9E8). A (0x0206F734):
 *   crouches (anim 4, its 58 frames), launches (0x0206F5E4: anim 5, two puffs) at 4.0 up for 8 frames, then slows by 0x334 a frame
 *   (0x0206F4D0, anim 6) and once falling holds (0x0206F468) until anim 6 is over. B in the air (0x0206F314): anim 9 for 16 frames,
 *   then floats down swaying (anim 3; 0x0206F008, 0x0206EE8C, 0x0206ECFC, 0x0206EB84): each sway accelerates 0x0A4 a frame to 1.2
 *   across while sinking at 0.6, then slows back while rising to 0.4, and the other way; between sways it may wait 0, 8, 16 or 24
 *   frames (10, 40, 25 and 25 in 100, 0x0206EAD0). Landing settles it. A direction held in the air (0x0206F854) moves it across, up
 *   to 1.2, bobbing (0x0206E938: 20 frames up, 20 down);
 * Not ported: the puffs at the launch (0x02021900, effect 3) and its sound (0x131); */
#include "../squeak.h"
#include "random.h"

// Animations (perara.cea);
enum { ANIM_STILL, ANIM_FLOAT, ANIM_LANDED, ANIM_SWAY, ANIM_CROUCH, ANIM_LAUNCH, ANIM_APEX, ANIM_HURT, ANIM_HURT_2, ANIM_SWAY_START };

// prmNE14Ml0.kgb;
#define WALK_SPEED 0x1334
#define WALK_SLOW 0x029
#define WALK_ACCEL 0x0A4
#define BOB_SPEED 0x4CD
#define BOB_ACCEL 0x19A
#define LAUNCH_SPEED 0x4000
#define LAUNCH_SLOW 0x334
#define LAUNCH_DELAY 8
#define SWAY_DELAY 16
#define SWAY_ACCEL 0x0A4
#define SWAY_SINK 0x99A
#define SWAY_SINK_ACCEL 0x029
#define SWAY_SPEED 0x1334
#define SWAY_RISE_ACCEL 0x0A4
#define SWAY_RISE 0x667
#define MAX_UP -0x2666
// prmNE14Pl0.kgb;
#define WAKE_X 0x3C000
#define WAKE_Y 0x78000
#define JUMP_X 0x50000
#define JUMP_WAIT 300
#define FLOAT_WAIT 80


/* Body states: each a Squeak Squad state function, the entries (_ENTRY) setting things up and chaining to the state that runs every
 * frame (SqueakRun: a state returning TRUE runs the next one this same frame, as Squeak Squad's); */
enum { BODY_AIR_ENTRY, BODY_AIR, BODY_WALK_ENTRY, BODY_WALK, BODY_LAND_ENTRY, BODY_GROUND, BODY_CROUCH_ENTRY, BODY_CROUCH, BODY_LAUNCH,
       BODY_APEX, BODY_SWAY_ENTRY, BODY_SWAY_START, BODY_SWAY_OUT, BODY_SWAY_BACK, BODY_SWAY_OUT_2, BODY_SWAY_BACK_2 };
// Brain states (one each frame);
enum { BRAIN_START, BRAIN_WAIT, BRAIN_WATCH, BRAIN_GROUND, BRAIN_JUMPED, BRAIN_FLOAT };

// The body's and brain's own fields (squeak.h's SqueakObject has the physics and the pad);
#define WAIT(s) ((s)->vars[0])          // 0x20C: frames to wait between sways;
#define TIMER(s) ((s)->vars[1])         // 0x208 (the launch), 0x210 (the sway's start);
#define BRAIN_TIMER(s) ((s)->vars[2])   // 0x98;
#define BRAIN_WAIT_TIMER(s) ((s)->vars[3])  // 0x9C;
#define PHASE(s) ((s)->vars[4])         // 0x214: 1 in the air, 2 on the ground, 3 moving across, 4 jumping, 5 floating;
#define WALK(s) ((s)->vars[5])          // 0x1F8: its speed across in the air;
#define BOB_TIMER(s) ((s)->vars[6])     // 0x200;
#define VX(s) ((s)->vx)
#define VY(s) ((s)->vy)
#define AX(s) ((s)->ax)
#define AY(s) ((s)->ay)
#define PAD_DIR(s) ((s)->padDir)
#define PAD_A(s) ((s)->padA)
#define PAD_B(s) ((s)->padB)
#define BRAIN(s) ((s)->brain)

#define Move SqueakMove
#define Stop SqueakStop
#define OnGround SqueakOnGround
#define TurnAround SqueakTurnAround
#define CapUp(s) SqueakCapFall(s, MAX_UP)

/* Bob(); 0x0206E938: in the air it bobs, 20 frames up then 20 down (each frame 0x4CD and 0x19A of acceleration from still), across
 * at its speed across; */
static void Bob(struct SqueakObject *s) {
    if (BOB_TIMER(s) >= 21) {
        VX(s) = WALK(s);
        VY(s) = BOB_SPEED;
        AX(s) = 0;
        AY(s) = BOB_ACCEL;
    } else if (BOB_TIMER(s) > 0) {
        VX(s) = WALK(s);
        VY(s) = -BOB_SPEED;
        AX(s) = 0;
        AY(s) = -BOB_ACCEL;
    }
    if (BOB_TIMER(s) < 1) {
        VX(s) = WALK(s);
        VY(s) = AX(s) = AY(s) = 0;
        BOB_TIMER(s) = 40;
        return;
    }
    BOB_TIMER(s)--;
}

// SlowWalk(); 0x0206EA10: with no direction held its speed across falls 0x029 a frame;
static void SlowWalk(struct SqueakObject *s) {
    if (PAD_DIR(s) != 0 || WALK(s) == 0) return;
    if (WALK(s) < 0 && (WALK(s) += WALK_SLOW) > 0) WALK(s) = 0;
    if (WALK(s) > 0 && (WALK(s) -= WALK_SLOW) < 0) WALK(s) = 0;
}

// SpeedUpWalk(); 0x0206EA68: holding forward it rises 0x0A4 a frame, to 1.2;
static void SpeedUpWalk(struct SqueakObject *s) {
    if (PAD_DIR(s) <= 0) return;
    if (WALK(s) >= WALK_SPEED) WALK(s) = WALK_SPEED;
    else WALK(s) += WALK_ACCEL;
}

// PickWait(); 0x0206EAD0: 0, 8, 16 or 24 frames (10, 40, 25 and 25 in 100);
static void PickWait(struct SqueakObject *s) {
    u16 roll = Rand16() % 100;

    WAIT(s) = roll < 10 ? 0 : roll < 50 ? 8 : roll < 75 ? 16 : 24;
}

/* Sway(); One sway (0x0206F008 and the three after it): towards (vx, vy) at its acceleration, capped there (vx from above when
 * `vxMax`, else from below; vy from above); once there (after its wait) the next, at (nextAx, nextAy), and nextVy when setVy; */
static bool32 Sway(struct SqueakObject *s, s32 vx, s32 vy, bool8 vxMax, u8 next, s32 nextAx, s32 nextVy, s32 nextAy, bool8 setVy) {
    if (VX(s) == vx && VY(s) == vy) {
        if (WAIT(s) < 1) {
            PickWait(s);
            AX(s) = nextAx;
            if (setVy) VY(s) = nextVy;
            AY(s) = nextAy;
            SqueakSetState(s, next);
            return TRUE;
        }
        WAIT(s)--;
    }
    if (vxMax ? VX(s) > vx : VX(s) < vx) {
        VX(s) = vx;
        AX(s) = 0;
    }
    if (VY(s) > vy) {
        VY(s) = vy;
        AY(s) = 0;
    }
    CapUp(s);
    Move(s);
    if (OnGround(s)) SqueakSetState(s, BODY_LAND_ENTRY);
    return FALSE;
}

// Pressed(); 0x0206F424 (A: crouches to jump), and B in the air (floats down);
static bool32 Pressed(struct SqueakObject *s, bool32 b) {
    if (b && PAD_B(s)) {
        SqueakSetState(s, BODY_SWAY_ENTRY);
        return TRUE;
    }
    if (PAD_A(s)) {
        SqueakSetState(s, BODY_CROUCH_ENTRY);
        return TRUE;
    }
    return FALSE;
}

static bool32 Body(struct SqueakObject *s) {
    switch (s->state) {
    case BODY_AIR_ENTRY:    // 0x0206FBE4;
        PHASE(s) = 1;
        SqueakPlay(s, ANIM_FLOAT);
        Stop(s);
        SqueakSetState(s, BODY_AIR);
        return TRUE;
    case BODY_AIR:          // 0x0206FAF0;
        if (Pressed(s, TRUE)) return TRUE;
        if (PAD_DIR(s) != 0) {
            SqueakSetState(s, BODY_WALK_ENTRY);
            return TRUE;
        }
        SlowWalk(s);
        Bob(s);
        Move(s);
        if (OnGround(s)) SqueakSetState(s, BODY_LAND_ENTRY);
        return FALSE;
    case BODY_WALK_ENTRY:   // 0x0206F994;
        PHASE(s) = 3;
        Stop(s);
        SqueakSetState(s, BODY_WALK);
        return TRUE;
    case BODY_WALK:         // 0x0206F854;
        if (Pressed(s, TRUE)) return TRUE;
        if (PAD_DIR(s) == 0) {
            WALK(s) = 0;
            SqueakSetState(s, BODY_AIR_ENTRY);
            return TRUE;
        }
        if (PAD_DIR(s) < 0) TurnAround(s);
        SpeedUpWalk(s);
        SlowWalk(s);
        Bob(s);
        Move(s);
        if (OnGround(s)) {
            WALK(s) = 0;
            SqueakSetState(s, BODY_LAND_ENTRY);
        }
        return FALSE;
    case BODY_LAND_ENTRY:   // 0x0206FA34;
        PHASE(s) = 2;
        SqueakPlay(s, ANIM_LANDED);
        Stop(s);
        SqueakSetState(s, BODY_GROUND);
        return TRUE;
    case BODY_GROUND:       // 0x0206F9E8;
        if (Pressed(s, FALSE)) return TRUE;
        if (PAD_DIR(s) < 0) TurnAround(s);
        Move(s);
        return FALSE;
    case BODY_CROUCH_ENTRY: // 0x0206F734;
        PHASE(s) = 4;
        SqueakPlay(s, ANIM_CROUCH);
        Stop(s);
        VY(s) = LAUNCH_SPEED;
        SqueakSetState(s, BODY_CROUCH);
        return TRUE;
    case BODY_CROUCH:       // 0x0206F5E4: launches once anim 4 is over;
        if (s->animEnded) {
            SqueakPlay(s, ANIM_LAUNCH);
            SqueakSetState(s, BODY_LAUNCH);
            return FALSE;
        }
        TIMER(s) = LAUNCH_DELAY;
        return FALSE;
    case BODY_LAUNCH:       // 0x0206F4D0: 8 frames at 4.0 up, then slowing;
        if (TIMER(s) == 0) {
            TIMER(s) = -1;
            SqueakPlay(s, ANIM_APEX);
            AY(s) = -LAUNCH_SLOW;
        }
        Move(s);
        if (VY(s) < 0) SqueakSetState(s, BODY_APEX);
        TIMER(s)--;
        return FALSE;
    case BODY_APEX:         // 0x0206F468;
        if (s->animEnded) {
            SqueakSetState(s, BODY_AIR_ENTRY);
            return TRUE;
        }
        return FALSE;
    case BODY_SWAY_ENTRY:   // 0x0206F314: keeps its speed, no acceleration;
        PHASE(s) = 5;
        SqueakPlay(s, ANIM_SWAY_START);
        AX(s) = AY(s) = 0;
        SqueakSetState(s, BODY_SWAY_START);
        TIMER(s) = SWAY_DELAY;
        return TRUE;
    case BODY_SWAY_START:   // 0x0206F198;
        if (TIMER(s) < 1) {
            TIMER(s) = 0;
            SqueakPlay(s, ANIM_SWAY);
            PickWait(s);
            Stop(s);
            AX(s) = SWAY_ACCEL;
            VY(s) = -SWAY_SINK;
            AY(s) = SWAY_SINK_ACCEL;
            SqueakSetState(s, BODY_SWAY_OUT);
            return TRUE;
        }
        TIMER(s)--;
        CapUp(s);
        Move(s);
        if (OnGround(s)) SqueakSetState(s, BODY_LAND_ENTRY);
        return FALSE;
    case BODY_SWAY_OUT:     // 0x0206F008: to 1.2 forward, sinking at 0.6;
        return Sway(s, SWAY_SPEED, -SWAY_SINK, TRUE, BODY_SWAY_BACK, -SWAY_ACCEL, 0, SWAY_RISE_ACCEL, FALSE);
    case BODY_SWAY_BACK:    // 0x0206EE8C: back to 0 across, rising to 0.4;
        return Sway(s, 0, SWAY_RISE, FALSE, BODY_SWAY_OUT_2, -SWAY_ACCEL, -SWAY_SINK, SWAY_SINK_ACCEL, TRUE);
    case BODY_SWAY_OUT_2:   // 0x0206ECFC: to 1.2 backward, sinking at 0.6;
        return Sway(s, -SWAY_SPEED, -SWAY_SINK, FALSE, BODY_SWAY_BACK_2, SWAY_ACCEL, 0, SWAY_RISE_ACCEL, FALSE);
    case BODY_SWAY_BACK_2:  // 0x0206EB84: back to 0 across, rising to 0.4;
        return Sway(s, 0, SWAY_RISE, TRUE, BODY_SWAY_OUT, SWAY_ACCEL, -SWAY_SINK, SWAY_SINK_ACCEL, TRUE);
    }
    return FALSE;
}

static void Brain(struct SqueakObject *s) {
    switch (BRAIN(s)) {
    case BRAIN_START:   // 0x0206E790: turn to face Kirby;
        SqueakFaceKirby(s, 0x1000);
        BRAIN(s) = BRAIN_WAIT;
        break;
    case BRAIN_WAIT:    // 0x0206E75C;
        PAD_DIR(s) = 0;
        if (SqueakKirbyWithin(s, WAKE_X, WAKE_Y)) {
            PAD_B(s) = 1;
            BRAIN(s) = BRAIN_WATCH;
        }
        break;
    case BRAIN_WATCH:   // 0x0206E6EC;
        PAD_B(s) = 0;
        if (s->state == BODY_GROUND || OnGround(s)) {
            BRAIN(s) = BRAIN_GROUND;
            BRAIN_WAIT_TIMER(s) = JUMP_WAIT;
        }
        if (PHASE(s) == 1) BRAIN(s) = BRAIN_FLOAT;
        break;
    case BRAIN_GROUND:  // 0x0206E670;
        if (SqueakKirbyWithin(s, JUMP_X, 0) || BRAIN_WAIT_TIMER(s) < 1) {
            BRAIN_WAIT_TIMER(s) = 0;
            BRAIN_TIMER(s) = FLOAT_WAIT;
            PAD_A(s) = 1;
            BRAIN(s) = BRAIN_JUMPED;
            return;
        }
        BRAIN_WAIT_TIMER(s)--;
        break;
    case BRAIN_JUMPED:  // 0x0206E630;
        PAD_A(s) = 0;
        PAD_DIR(s) = 0;
        if (BRAIN_TIMER(s) < 1) {
            BRAIN_TIMER(s) = 0;
            PAD_B(s) = 1;
            BRAIN(s) = BRAIN_WATCH;
            return;
        }
        BRAIN_TIMER(s)--;
        break;
    case BRAIN_FLOAT:   // 0x0206E73C;
        PAD_DIR(s) = 0;
        PAD_B(s) = 1;
        BRAIN(s) = BRAIN_WATCH;
        break;
    }
}


static void PeraraMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    Brain(s);
    SqueakRun(s, Body);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
}

void PeraraStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, 0, PeraraMain);
    // (after a hit only the body starts over, at vtable +0x3C; the brain goes on);
    if (!s->started) BRAIN(s) = BRAIN_START;
    SqueakSetState(s, BODY_AIR_ENTRY);
}

/* PeraraInit(); Its box (perara.rca's first: 10 wide, 10 high over its feet, the origin), and its feet on the ground (it hovers
 * in the air until it lands); */
void PeraraInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -5, -10, 5, 0);
    ObjectSetBounds(&s->obj.base, -5, -10, 5, 0);
    SqueakStandOnFloor(&s->obj);
}
