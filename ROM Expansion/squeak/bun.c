/* Bun; ported from Kirby: Squeak Squad (USA), enemy 0x2B (its body 0x02056AA4, vtable 0x0216A114; brain 0x02055470), as SQUEAK_BUN.
 * Values from param/prmCE28Ml0.kgb (movement; MlG is the same), prmCE28Pl0.kgb (brain) and prmCE28Sl0.kgb (HP 35); speeds 20.12 a
 * frame, Y up; its boxes from enemy/bun.rca (its animations from the .rca's second). A long-eared creature that grabs Kirby with its
 * ear: it walks or runs at him, reaches, and caught he's carried, swung overhead and round it three times and thrown away.
 * Swallowed, it gives Throw (as on the DS).
 * • Brain (the spawn's first parameter byte, KatAM's template unk12; Kirby's spot his feet, as Squeak Squad's):
 *   0 and any but 1 and 2 (0x020553A4): it faces Kirby (past 1 pixel); then (0x02055314) stands 31 frames (40%: 0x020552E4), 61
 *     (20%) or none, and looks (0x02055220): Kirby within 120 pixels ahead, 8 under its feet to 20 over them, it runs (pad forward
 *     and its +0x44) up to 33 frames (0x0205513C) till he's within 40 ahead and 32 up or down: it stops and presses B one frame
 *     (0x02055124), then waits for its body to be done (0x020550FC, its +0x1F8) and starts over; a wall, or the time over, it stops
 *     and starts over. Kirby not in front, it walks 121 frames (0x02054FF4), turning at walls, running as above if he comes within
 *     32 behind to 120 ahead, then looks again;
 *   1 (0x02054FD4): it faces Kirby and stands till he's within 40 pixels across and 32 up or down (0x02054F78), presses B
 *     (0x02054F60), waits for its body (0x02054F38) and starts over;
 *   2 (0x02054F18): it faces Kirby, then stands.
 * • Body (gravity 0.1 a frame, falls no faster than 2.4): it stands (anim 1: 0x0205670C, 0x02056678), walks at 0.5 (anim 2:
 *   0x020565E0, 0x020564FC) or runs at 1.4 (anim 3: 0x02056464, 0x02056390); A jumps (2.5 up, anim 4: 0x02056260, 0x020561CC; its
 *   brains never press it); off the ground it falls (anim 1: 0x02055844, 0x02055734; forward keeps it going at 0.5: 0x020567C4). B
 *   reaches with its ear (anim 5, 21 frames: 0x02056128, 0x0205604C; its grab box, kind 6, 13 to 35 pixels ahead, the last 3), then
 *   anim 12 (3 frames: 0x02055F80) and 7 (16: 0x02055F14). Its ear touching Kirby (0x0205695C) it holds him where he is till anim 5
 *   is over (0x02055E30), carries him 28 pixels ahead (16 frames), then overhead, 28 behind and 30 up (40 more; anim 6:
 *   0x02055CCC), swings him round itself three times (anim 8, 24 frames: 0x02055B54; ahead, under, behind, over, 2 frames each, his
 *   feet towards it, from the table at 0x02186A44), lifts him 8 pixels ahead, 26 up (3 frames) and throws him from 16 ahead at 4.6
 *   (anim 9, 19 frames: 0x02055940); then it stands. Squeak Squad's thrown Kirby loses 6 of 36 HP as he lands (the hold's kind 6
 *   hit); here KatAM's throw that hurts on landing (Kirby_110 flag 2, 1 HP), as Gaw Gaw's. From the grab to the throw's end its
 *   touch doesn't hurt (its +0x18 bits 1 and 2), nor his body it while it holds him; a hit lets him go (0x020568EC).
 * • Kirby's poses held: KatAM's own turning Kirby (its thrown Kirby's frames, 0x4A-0x59: 0x58 upright, 0x54 and 0x4C on his side,
 *   0x50 upside down) for Squeak Squad's held poses (its Kirby's vtable +0x10: 3 and 4 upright; 8, 11, 17 and 4 in the swing), as
 *   recorded on the DS.
 * • Boxes (bun.rca, per animation frame, as uja.c's): its body's hit box (14 pixels either side, 1 to 29 up), the reach's touch box
 *   alone with its grab box.
 * Not ported: its sounds (0x124 the jump, 0x122 the swing every 10 frames), its other mode (the brain's +8 not 3: B and A in the air),
 * Squeak Squad's damages (6 of 36 the throw: KatAM's 1), its event 0x15 (vtable +0x24, 0x0205681C: lets Kirby go where he is). */
#include "../squeak.h"
#include "main.h"
#include "random.h"

void sub_0804E09C(struct Kirby *);

// Animations (bun.cea);
enum { ANIM_STILL, ANIM_STAND, ANIM_WALK, ANIM_RUN, ANIM_JUMP, ANIM_REACH, ANIM_CARRY, ANIM_RECOVER, ANIM_SWING, ANIM_THROW,
       ANIM_HURT, ANIM_HURT_2, ANIM_REACH_END };

// prmCE28Ml0.kgb;
#define WALK_SPEED 0x800
#define JUMP_SPEED 0x2800
#define GRAVITY 0x19A
#define MAX_FALL -0x2667
#define RUN_SPEED 0x1667
// prmCE28Pl0.kgb;
#define SHORT_CHANCE 40
#define LOOK_CHANCE 40
#define SHORT_WAIT 30
#define LONG_WAIT 60
#define SIGHT 0x78000                   // Kirby in sight: 120 pixels ahead (32 behind, walking), 8 under its feet to 20 over them;
#define SIGHT_BEHIND 0x20000
#define SIGHT_UP 0x14000
#define SIGHT_DOWN 0x8000
#define RUN_TIME 32
#define WALK_TIME 120
#define REACH_X 0x28000                 // Kirby in reach: 40 pixels ahead, 32 up or down (variant 1: 40 either way);
#define REACH_Y 0x20000
// The code's;
#define FACE_DISTANCE 0x1000
#define KIRBY_FEET 7                    // KatAM's Kirby's spot is 7 pixels over his feet, Squeak Squad's at them;
#define CARRY_TIME 16
#define LIFT_TIME 3
#define THROW_X 16                      // The throw: from 16 pixels ahead, 26 up (KatAM's: his spot), at 4.6 (0x499A: KatAM's x 0x10);
#define THROW_Y (-26 - KIRBY_FEET)
#define THROW_SPEED 74

enum {
    BODY_START, BODY_STAND, BODY_WALK_ENTRY, BODY_WALK, BODY_RUN_ENTRY, BODY_RUN, BODY_JUMP_ENTRY, BODY_RISE, BODY_FALL_ENTRY,
    BODY_FALL, BODY_REACH_ENTRY, BODY_REACH, BODY_REACH_END, BODY_RECOVER, BODY_GRABBED, BODY_CARRY, BODY_SWING, BODY_THROW,
};
enum {
    BRAIN_FACE, BRAIN_PICK, BRAIN_WAIT, BRAIN_LOOK, BRAIN_WALK, BRAIN_RUN, BRAIN_PRESS, BRAIN_RELEASE, BRAIN_WATCH_FACE,
    BRAIN_WATCH, BRAIN_WATCH_PRESS, BRAIN_WATCH_RELEASE, BRAIN_STILL_FACE, BRAIN_STILL,
};

#define COUNT(s) ((s)->vars[0])         // 0x1FC: the hold's frames;
#define BUSY(s) ((s)->vars[1])          // 0x1F8: reaching or holding (its brain waits for it);
#define HOLDING(s) ((s)->vars[2])       // Its +0x18 bits 1 and 2: from the grab to the throw's end (its touch harmless);
#define RUN(s) ((s)->vars[3])           // Its brain's +0x44: run;
#define WAIT(s) ((s)->vars[4])          // The brain's 0x98;
#define HARM(s) ((s)->vars[5])          // KatAM's unk68 from InitObject: its touch's harm;
#define BOX(s) (*(u8 *)&(s)->vars[6])   // This frame's box (sBunBoxes' index) and the last frame's;
#define LAST_BOX(s) (*((u8 *)&(s)->vars[6] + 1))
#define HELD(s) (*(struct Kirby **)&(s)->vars[7])   // 0x164;

// bun.rca's boxes (its animations from the .rca's second; boxes.py);
static const struct SqueakBox sBunBoxes[] = {
    { { -14, -29, 14, -1 }, TRUE, 0 },
    { { -20, -29, 8, -1 }, TRUE, 0 },
    { { -6, -20, 9, -5 }, TRUE, 1, { { 13, -22, 35, 0 } } },
    { { -17, -29, 14, -1 }, TRUE, 0 },
    { { -17, -36, 14, -1 }, TRUE, 0 },
    { { -14, -29, 14, -1 }, FALSE, 0 },
};
static const struct SqueakBoxFrame sBunAnim0[] = { { 1, 0 } };
static const struct SqueakBoxFrame sBunAnim1[] = { { 36, 0 } };
static const struct SqueakBoxFrame sBunAnim2[] = { { 62, 0 } };
static const struct SqueakBoxFrame sBunAnim3[] = { { 32, 0 } };
static const struct SqueakBoxFrame sBunAnim4[] = { { 1, 0 } };
static const struct SqueakBoxFrame sBunAnim5[] = { { 18, 1 }, { 3, 2 } };
static const struct SqueakBoxFrame sBunAnim6[] = { { 16, 0 }, { 40, 3 } };
static const struct SqueakBoxFrame sBunAnim7[] = { { 16, 0 } };
static const struct SqueakBoxFrame sBunAnim8[] = { { 24, 4 } };
static const struct SqueakBoxFrame sBunAnim9[] = { { 19, 0 } };
static const struct SqueakBoxFrame sBunAnim10[] = { { 4, 5 } };
static const struct SqueakBoxFrame sBunAnim11[] = { { 4, 5 } };
static const struct SqueakBoxFrame sBunAnim12[] = { { 3, 0 } };
static const struct SqueakBoxFrame sBunAnim13[] = { { 1, 5 } };
static const struct SqueakBoxFrame sBunAnim14[] = { { 1, 0 } };
static const struct SqueakBoxAnim sBunBoxAnims[] = {
    { sBunAnim0, 1 },
    { sBunAnim1, 1 },
    { sBunAnim2, 1 },
    { sBunAnim3, 1 },
    { sBunAnim4, 1 },
    { sBunAnim5, 2 },
    { sBunAnim6, 2 },
    { sBunAnim7, 1 },
    { sBunAnim8, 1 },
    { sBunAnim9, 1 },
    { sBunAnim10, 1 },
    { sBunAnim11, 1 },
    { sBunAnim12, 1 },
    { sBunAnim13, 1 },
    { sBunAnim14, 1 },
};
static const struct SqueakBoxes sBunBoxData = { sBunBoxes, sBunBoxAnims, 15 };

/* Kirby held (kirby->unk110, each one entry and its end; KatAM's hold places him, facing as it, the entry's x ahead of it, y under
 * its spot; drawn in front of it, its third field 1, as Box Boxer's holds and the DS's; flag 0x400 keeps him held, 0x200 leaves him
 * where he is), in KatAM's turning Kirby's poses: just grabbed; carried ahead; overhead; swung ahead, under, behind and over it
 * (his feet towards it); lifted; where he's thrown from; thrown away hurt as he lands (flag 2), or let go as he is (flag 1); */
enum { HOLD_GRABBED, HOLD_CARRY, HOLD_OVERHEAD, HOLD_SWING, HOLD_LIFT = HOLD_SWING + 4, HOLD_THROW, HOLD_COUNT };
#define UPRIGHT 0x58
static const struct Kirby_110 sHolds[HOLD_COUNT][2] = {
    [HOLD_GRABBED] = { { 0, 0, 1, 0xFF, UPRIGHT, 0x600 }, { 0, 0, 0, 1, 0, 0x400 } },
    [HOLD_CARRY] = { { 28, -KIRBY_FEET, 1, 0xFF, UPRIGHT, 0x400 }, { 0, 0, 0, 1, 0, 0x400 } },
    [HOLD_OVERHEAD] = { { -28, -30 - KIRBY_FEET, 1, 0xFF, UPRIGHT, 0x400 }, { 0, 0, 0, 1, 0, 0x400 } },
    [HOLD_SWING] = { { 20, -11 - KIRBY_FEET, 1, 0xFF, 0x54, 0x400 }, { 0, 0, 0, 1, 0, 0x400 } },
    [HOLD_SWING + 1] = { { -2, 6 - KIRBY_FEET, 1, 0xFF, 0x50, 0x400 }, { 0, 0, 0, 1, 0, 0x400 } },
    [HOLD_SWING + 2] = { { -30, -15 - KIRBY_FEET, 1, 0xFF, 0x4C, 0x400 }, { 0, 0, 0, 1, 0, 0x400 } },
    [HOLD_SWING + 3] = { { -4, -37 - KIRBY_FEET, 1, 0xFF, UPRIGHT, 0x400 }, { 0, 0, 0, 1, 0, 0x400 } },
    [HOLD_LIFT] = { { 8, -26 - KIRBY_FEET, 1, 0xFF, UPRIGHT, 0x400 }, { 0, 0, 0, 1, 0, 0x400 } },
    [HOLD_THROW] = { { THROW_X, THROW_Y, 1, 0xFF, UPRIGHT, 0x400 }, { 0, 0, 0, 1, 0, 0x400 } },
};
static const struct Kirby_110 sThrow[] = { { THROW_SPEED, 0, -1, 1, 0, 0x402 }, { 0, 0, 0, 1, 0, 0 } };
static const struct Kirby_110 sLetGo[] = { { 0, 0, -1, 1, 0, 0x401 }, { 0, 0, 0, 1, 0, 0 } };

static void BunMain(struct Object *obj);

static u32 Random(void) {
    return Rand16() * 100 >> 16;
}

// StillHeld(); The Kirby it holds is still held by it (KatAM's hold lets him go on its own when it's beaten);
static bool32 StillHeld(struct SqueakObject *s) {
    struct Kirby *kirby = HELD(s);

    return kirby && kirby->base.unk6C == &s->obj.base && kirby->unk110 >= sHolds[0] && kirby->unk110 < sHolds[HOLD_COUNT];
}

// LetGo(); Kirby thrown away (sThrow) or let go as he is (sLetGo), the table started over;
static void LetGo(struct SqueakObject *s, const struct Kirby_110 *how) {
    struct Kirby *kirby = HELD(s);

    if (StillHeld(s)) {
        kirby->unk110 = how;
        kirby->base.counter = -1;
    }
    HELD(s) = NULL;
    s->obj.base.unk6C = NULL;
}

/* Hold(); Kirby held as `hold` says, there this frame (Squeak Squad's hold, its Kirby's vtable +0x10, places him in its update; KatAM's
 * from his next one: his spot set here too); a new pose starts the table over; */
static void Hold(struct SqueakObject *s, u8 hold) {
    struct Kirby *kirby = HELD(s);
    const struct Kirby_110 *h = sHolds[hold];

    if (!StillHeld(s)) return;
    if (kirby->unk110 != h) {
        kirby->unk110 = h;
        kirby->base.counter = -1;
    }
    // (facing as it, turned round with it: Squeak Squad's hold turns the offset with its holder);
    kirby->base.flags = (kirby->base.flags & ~1) | (s->obj.base.flags & 1);
    if (!(h->flags & 0x200)) {
        kirby->base.x = s->obj.base.x + (kirby->base.flags & 1 ? -h->unk0 : h->unk0) * 0x100;
        kirby->base.y = s->obj.base.y + h->unk1 * 0x100;
    }
}

/* Grab(); 0x0205695C: its ear touched a Kirby: it holds him where he is (KatAM's hold, sub_0804E09C, which faces him as it) and stops;
 * its touch harmless till the throw's end (its +0x18 bits 1 and 2); */
static void Grab(struct SqueakObject *s, struct Kirby *kirby) {
    HELD(s) = kirby;
    kirby->base.unk6C = &s->obj.base;
    kirby->unk110 = sHolds[HOLD_GRABBED];
    s->obj.base.unk6C = kirby;
    kirby->base.flags |= 0x40000;
    sub_0804E09C(kirby);
    HOLDING(s) = TRUE;
    SqueakStop(s);
    SqueakSetState(s, BODY_GRABBED);
}

// Gravity(); 0x020556C0: on the ground, still; else falling GRAVITY a frame, no faster than MAX_FALL;
static void Gravity(struct SqueakObject *s) {
    if (SqueakOnGround(s)) {
        s->vy = s->ay = 0;
    } else if (!SqueakCapFall(s, MAX_FALL)) {
        s->ay = -GRAVITY;
    }
}

// AirControl(); 0x020567C4: forward, it goes at WALK_SPEED (no cap); none, it stops across;
static void AirControl(struct SqueakObject *s) {
    if (s->padDir > 0) {
        s->vx = WALK_SPEED;
        s->maxVx = 0;
    } else if (s->padDir == 0) {
        s->vx = 0;
    }
}

// Buttons(); 0x020555B0, 0x0205556C: B reaches, A jumps;
static bool32 Buttons(struct SqueakObject *s) {
    if (s->padB) {
        SqueakSetState(s, BODY_REACH_ENTRY);
        return TRUE;
    }
    if (s->padA) {
        SqueakSetState(s, BODY_JUMP_ENTRY);
        return TRUE;
    }
    return FALSE;
}

// Next(); Its animation over: `anim` and the state after;
static bool32 Next(struct SqueakObject *s, u8 anim, u8 state) {
    SqueakPlay(s, anim);
    SqueakSetState(s, state);
    COUNT(s) = 0;
    return TRUE;
}

static bool32 Body(struct SqueakObject *s) {
    switch (s->state) {
    case BODY_START:        // 0x0205670C (vtable +0x3C): its brain may go on;
        BUSY(s) = 0;
        SqueakPlay(s, ANIM_STAND);
        SqueakStop(s);
        SqueakSetState(s, BODY_STAND);
        return TRUE;
    case BODY_STAND:        // 0x02056678;
        if (Buttons(s)) return TRUE;
        if (s->padDir != 0) {
            SqueakSetState(s, BODY_WALK_ENTRY);
            return TRUE;
        }
        SqueakMove(s);
        if (!SqueakOnGround(s)) SqueakSetState(s, BODY_FALL_ENTRY);
        break;
    case BODY_WALK_ENTRY:   // 0x020565E0;
        SqueakPlay(s, ANIM_WALK);
        SqueakStop(s);
        SqueakSetState(s, BODY_WALK);
        return TRUE;
    case BODY_WALK:         // 0x020564FC: B, A, run, none;
        if (Buttons(s)) return TRUE;
        if (RUN(s)) {
            SqueakSetState(s, BODY_RUN_ENTRY);
            return TRUE;
        }
        if (s->padDir == 0) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        s->vx = WALK_SPEED;
        if (s->padDir < 0) SqueakTurnAround(s);
        SqueakMove(s);
        if (!SqueakOnGround(s)) SqueakSetState(s, BODY_FALL_ENTRY);
        break;
    case BODY_RUN_ENTRY:    // 0x02056464;
        SqueakPlay(s, ANIM_RUN);
        SqueakStop(s);
        SqueakSetState(s, BODY_RUN);
        return TRUE;
    case BODY_RUN:          // 0x02056390: B, A, none;
        if (Buttons(s)) return TRUE;
        if (s->padDir == 0) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        s->vx = RUN_SPEED;
        if (s->padDir < 0) SqueakTurnAround(s);
        SqueakMove(s);
        if (!SqueakOnGround(s)) SqueakSetState(s, BODY_FALL_ENTRY);
        break;
    case BODY_JUMP_ENTRY:   // 0x02056260;
        SqueakPlay(s, ANIM_JUMP);
        s->vy = s->maxVx = s->maxVy = s->ax = s->ay = s->fx = s->fy = 0;
        s->vy = JUMP_SPEED;
        s->ay = -GRAVITY;
        SqueakSetState(s, BODY_RISE);
        return TRUE;
    case BODY_RISE:         // 0x020561CC;
        AirControl(s);
        if (s->padDir < 0) SqueakTurnAround(s);
        SqueakMove(s);
        if (s->vy < 0) SqueakSetState(s, BODY_FALL_ENTRY);
        break;
    case BODY_FALL_ENTRY:   // 0x02055844: its speeds as they were;
        SqueakPlay(s, ANIM_STAND);
        s->maxVx = s->maxVy = s->ax = s->ay = s->fx = s->fy = 0;
        s->ay = -GRAVITY;
        SqueakSetState(s, BODY_FALL);
        return TRUE;
    case BODY_FALL:         // 0x02055734: landed, it walks on if it was going across;
        AirControl(s);
        SqueakCapFall(s, MAX_FALL);
        SqueakMove(s);
        if (SqueakOnGround(s)) {
            SqueakSetState(s, s->vx == 0 ? BODY_START : BODY_WALK_ENTRY);
            s->vy = s->ay = s->ax = 0;
            return TRUE;
        }
        break;
    case BODY_REACH_ENTRY:  // 0x02056128;
        BUSY(s) = 1;
        SqueakPlay(s, ANIM_REACH);
        SqueakStop(s);
        SqueakSetState(s, BODY_REACH);
        return TRUE;
    case BODY_REACH:        // 0x0205604C;
        if (s->animEnded) return Next(s, ANIM_REACH_END, BODY_REACH_END);
        if (s->padDir < 0) SqueakTurnAround(s);
        Gravity(s);
        SqueakMove(s);
        break;
    case BODY_REACH_END:    // 0x02055F80;
        if (s->animEnded) return Next(s, ANIM_RECOVER, BODY_RECOVER);
        Gravity(s);
        SqueakMove(s);
        break;
    case BODY_RECOVER:      // 0x02055F14;
        if (s->animEnded) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        Gravity(s);
        SqueakMove(s);
        break;
    case BODY_GRABBED:      // 0x02055E30: its reach over, the carry;
        if (s->animEnded) return Next(s, ANIM_CARRY, BODY_CARRY);
        if (s->padDir < 0) SqueakTurnAround(s);
        Gravity(s);
        SqueakMove(s);
        break;
    case BODY_CARRY:        // 0x02055CCC: Kirby ahead CARRY_TIME frames, then overhead;
        if (s->animEnded) return Next(s, ANIM_SWING, BODY_SWING);
        if (HELD(s)) {
            Hold(s, COUNT(s) < CARRY_TIME ? HOLD_CARRY : HOLD_OVERHEAD);
            COUNT(s)++;
        }
        if (s->padDir < 0) SqueakTurnAround(s);
        Gravity(s);
        SqueakMove(s);
        break;
    case BODY_SWING:        // 0x02055B54: round it, 2 frames a spot (0x02186A44);
        if (s->animEnded) return Next(s, ANIM_THROW, BODY_THROW);
        if (HELD(s)) {
            Hold(s, HOLD_SWING + (COUNT(s) >> 1 & 3));
            COUNT(s)++;
        }
        Gravity(s);
        SqueakMove(s);
        break;
    /* 0x02055940: lifted LIFT_TIME frames, then thrown from THROW_X ahead, THROW_Y up at THROW_SPEED (KatAM's throw, falling its own way,
     * hurt as he lands); its animation over, its touch hurts again. Squeak Squad places its thrown Kirby there and he moves from his
     * next update; KatAM's Kirby takes the throw in the frame it's given (after its update): placed there this frame, let go the next,
     * his path is the DS's frame for frame; */
    case BODY_THROW:
        if (s->animEnded) {
            HOLDING(s) = FALSE;
            SqueakSetState(s, SqueakOnGround(s) ? BODY_START : BODY_FALL_ENTRY);
            return TRUE;
        }
        if (HELD(s)) {
            if (COUNT(s) < LIFT_TIME) Hold(s, HOLD_LIFT);
            else if (COUNT(s) == LIFT_TIME) Hold(s, HOLD_THROW);
            else LetGo(s, sThrow);
            COUNT(s)++;
        }
        Gravity(s);
        SqueakMove(s);
        break;
    }
    return FALSE;
}

/* KirbyAt(); The closest Kirby's feet (Squeak Squad's spot) between `behind` and `ahead` of it across and `down` and `up` of its feet
 * (bounds excluded: 0x0205513C's, 0x02055220's); */
static bool32 KirbyAt(struct SqueakObject *s, s32 behind, s32 ahead, s32 down, s32 up) {
    struct Object *obj = &s->obj;
    s32 x = SQ_X(obj->kirby3) - SQ_X(obj), y = SQ_Y(obj->kirby3) - KIRBY_FEET * 0x1000 - SQ_Y(obj);

    if (obj->base.flags & 1) x = -x;
    return x > -behind && x < ahead && y > -down && y < up;
}

// Stand(); The brain's 0x02054F08: no direction, its pad's up and down none;
static void Stand(struct SqueakObject *s) {
    s->padDir = s->padY = 0;
}

// Run(); The brain's 0x02054EF8 (forward, run) and 0x02054EF0 (none, no run);
static void Run(struct SqueakObject *s, bool8 run) {
    s->padDir = run;
    RUN(s) = run;
}

// Brain(); Its pad (0x020553A4, 0x02054FD4, 0x02054F18 and the states they lead to), one state a frame;
static void Brain(struct SqueakObject *s) {
    switch (s->brain) {
    case BRAIN_FACE:        // 0x020553A4;
        SqueakFaceKirby(s, FACE_DISTANCE);
        s->brain = BRAIN_PICK;
        break;
    case BRAIN_PICK: {      // 0x02055314;
        u32 r;

        Stand(s);
        r = Random();
        if (r < SHORT_CHANCE) {
            WAIT(s) = SHORT_WAIT;
            s->brain = BRAIN_WAIT;
        } else if (r < SHORT_CHANCE + LOOK_CHANCE) {
            s->brain = BRAIN_LOOK;
        } else {
            WAIT(s) = LONG_WAIT;
            s->brain = BRAIN_WAIT;
        }
        break;
    }
    case BRAIN_WAIT:        // 0x020552E4;
        Stand(s);
        if (WAIT(s) < 1) s->brain = BRAIN_LOOK;
        else WAIT(s)--;
        break;
    case BRAIN_LOOK:        // 0x02055220: Kirby in front, it runs; else it walks;
        if (KirbyAt(s, 0, SIGHT, SIGHT_DOWN, SIGHT_UP)) {
            WAIT(s) = RUN_TIME;
            Run(s, TRUE);
            s->brain = BRAIN_RUN;
        } else {
            WAIT(s) = WALK_TIME;
            s->brain = BRAIN_WALK;
        }
        break;
    case BRAIN_WALK:        // 0x02054FF4: back at a wall;
        if (KirbyAt(s, SIGHT_BEHIND, SIGHT, SIGHT_DOWN, SIGHT_UP)) {
            WAIT(s) = RUN_TIME;
            Run(s, TRUE);
            s->brain = BRAIN_RUN;
            break;
        }
        s->padDir = 1;
        if (WAIT(s) < 1) s->brain = BRAIN_LOOK;
        else WAIT(s)--;
        if (SqueakWallAhead(s)) s->padDir = -s->padDir;
        break;
    case BRAIN_RUN:         // 0x0205513C: Kirby in reach, it stops and reaches; a wall or its time over, it stops;
        if (KirbyAt(s, 0, REACH_X, REACH_Y, REACH_Y)) {
            Run(s, FALSE);
            s->brain = BRAIN_PRESS;
            break;
        }
        if (WAIT(s) > 0 && !SqueakWallAhead(s)) {
            WAIT(s)--;
            break;
        }
        WAIT(s) = 0;
        Run(s, FALSE);
        Stand(s);
        s->brain = BRAIN_PICK;
        break;
    case BRAIN_PRESS:       // 0x02055124;
        s->padB = 1;
        s->brain = BRAIN_RELEASE;
        break;
    case BRAIN_RELEASE:     // 0x020550FC: its body done, it starts over;
        s->padB = 0;
        if (!BUSY(s)) s->brain = BRAIN_PICK;
        break;
    case BRAIN_WATCH_FACE:  // 0x02054FD4;
        SqueakFaceKirby(s, FACE_DISTANCE);
        s->brain = BRAIN_WATCH;
        break;
    case BRAIN_WATCH:       // 0x02054F78;
        Stand(s);
        if (KirbyAt(s, REACH_X, REACH_X, REACH_Y, REACH_Y)) s->brain = BRAIN_WATCH_PRESS;
        break;
    case BRAIN_WATCH_PRESS: // 0x02054F60;
        s->padB = 1;
        s->brain = BRAIN_WATCH_RELEASE;
        break;
    case BRAIN_WATCH_RELEASE:   // 0x02054F38;
        s->padB = 0;
        if (!BUSY(s)) s->brain = BRAIN_WATCH_FACE;
        break;
    case BRAIN_STILL_FACE:  // 0x02054F18;
        SqueakFaceKirby(s, FACE_DISTANCE);
        s->brain = BRAIN_STILL;
        break;
    case BRAIN_STILL:       // 0x02054F10;
        Stand(s);
        break;
    }
}

/* BunMain(); Its ear's grab as on the DS: Squeak Squad's touch reactions run in the next frame's collision pass, before its update,
 * and its boxes' time runs a frame behind its animation's (logged: the reach's grab box from its 19th frame, Kirby caught in its
 * 20th): here the box of two frames back, before its update; */
static void BunMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;
    struct Kirby *kirby;

    kirby = obj->unk80 > 0 && !HELD(s) ? SqueakGrabbing(s, &sBunBoxes[LAST_BOX(s)]) : NULL;
    if (kirby) Grab(s, kirby);
    Brain(s);
    SqueakRun(s, Body);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
    LAST_BOX(s) = BOX(s);
    SqueakSetBoxes(s, &sBunBoxData, HOLDING(s) ? 0 : HARM(s), &BOX(s));
    // (its +0x18 bit 2 also takes it out of every attack's reach: 0x0201322C, untouchable);
    if (HOLDING(s)) SqueakUntouchable(s, TRUE);
    // (holding Kirby, his body doesn't hurt it, as KatAM's grabbers' and Gaw Gaw's);
    if (HELD(s)) obj->base.unk5C |= 0x80;
    else obj->base.unk5C &= ~0x80;
}

/* BunWatch(); Its pre-callback: KatAM's hit put it out of its own state (unk78 not BunMain), or it's beaten: it lets Kirby go as he is
 * (vtable +0x30, 0x020568EC); */
static void BunWatch(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    if (HELD(s) && (obj->unk78 != BunMain || obj->unk80 <= 0 || (obj->base.flags & 0x1000))) LetGo(s, sLetGo);
}

void BunStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, 0, BunMain);
    obj->unk7C = BunWatch;
    SqueakThroughTerrain(s, FALSE);
    if (HELD(s)) LetGo(s, sLetGo);
    HOLDING(s) = FALSE;
    if (!s->started) {
        switch (s->variant) {
        case 1:
            s->brain = BRAIN_WATCH_FACE;
            break;
        case 2:
            s->brain = BRAIN_STILL_FACE;
            break;
        default:
            s->brain = BRAIN_FACE;
            break;
        }
    }
    SqueakSetState(s, BODY_START);
}

/* BunInit(); Its box (bun.rca's hit box, 14 pixels either side, 1 to 29 up; its body box 6 either side, 1 to 19 up: here from its feet,
 * KatAM's bounds), facing left (0x02056AA4); its hit box and harm set each frame (SqueakSetBoxes()); */
void BunInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -14, -29, 14, -1);
    ObjectSetBounds(&s->obj.base, -6, -19, 6, 0);
    HARM(s) = s->obj.base.unk68;
    s->obj.base.flags |= 1;
    SqueakStandOnFloor(&s->obj);
}
