/* Gaw Gaw; ported from Kirby: Squeak Squad (USA), enemy 0x28 (its body 0x020B0068, vtable 0x021753C4; brain 0x020AC780), as
 * SQUEAK_GAW_GAW. Values from param/prmCE22Ml0.kgb (movement; MlG differs only in the other mode's gravity), prmCE22Pl0.kgb (brain)
 * and prmCE22Sl0.kgb (HP 20); speeds 20.12 a frame, Y up; its boxes from enemy/gawgaw.rca (its animations from the .rca's second). A
 * wolf: it hops, crouches and pounces at Kirby (its jaws grab him), chews on him and throws him away; it clings to walls and climbs.
 * • Brain (the spawn's first parameter byte, KatAM's template unk12):
 *   0, 2 and any other (0x020AC6A8): it faces Kirby (past 1 pixel), stands 44 frames, then three times in four (else 44 more) hops
 *     (A: its pad down 60%, none 30%, up 10%: 0x020AC59C), lets go once in the air and, landed, starts over;
 *   1 (0x020AC538): it faces Kirby, presses A (a crouch) and holds B (0x020AC358, 0x020AC2D4: the pounce's charge) till it's in the
 *     air or 30 frames on, lets go and, half the time, holds forward; landed (0x020AC1F4), if it met a wall and still faces Kirby it
 *     jumps (0x020AC4BC, 0x020AC158, 0x020AC13C), else it starts over (0x020AC4E0: Kirby "within" its params' 72.0 and 18.0, floats
 *     read as whole numbers, so always; the random walk after it, 0x020AC37C, is never reached); holding Kirby still (its body's mode
 *     7) in the air, it starts over at once;
 *   3 (0x020AC084): it faces Kirby, then stands.
 * • Body (gravity 0.17 a frame, falls no faster than 2.4): spawned, it falls (0x020AFC70, 0x020AF9D8) and, touching a wall either side,
 *   clings to it facing it; it stands (anim 1; 0x020AF938, 0x020AF890), walks at 2.2 while its pad is forward (anim 4; 0x020AF7F0,
 *   0x020AF6B0); A hops (0x020AF4D8: anim 9, then 10 and 11; 1.85, 3.0 or 4.0 up as its pad's down, none or up), forward in the air
 *   2.2; B on the ground bites (0x020AEF7C: anims 21, 22 up, 23 down; its brains never press it); B while crouching for a hop
 *   pounces (0x020AEE5C: anim 12 61 frames, 13, then 14: 3.2 forward, 1.85 up); its leap's jaws (gawgaw.rca's kind 6, anim 14)
 *   touching Kirby, it holds him 8 pixels ahead (0x020AFEB0): going forward, it bites (anim 15, 26 frames) and shakes him (anim 19, 34
 *   frames: 0x020AE280, 0x020AE118, 0x020ADE68) and throws him back (2.3 back, 2.1 up), springing back itself (1.6 back, 2.8 up,
 *   falling 0.1 a frame, anim 20: 0x020ADDBC); still, it bites (anim 15) and chews (anim 17, 90 frames: 0x020AE7F0, 0x020AE67C,
 *   0x020AE3D0) and throws him (1.0 back, 1.0 up), springing back (falling 0.17, anim 18: 0x020AE324). Squeak Squad's thrown Kirby
 *   loses 6 of 36 HP as he lands (its hold's kind 6 hit); here KatAM's throw that hurts on landing (Kirby_110 flag 2, 1 HP), as Bio
 *   Spark's. While it holds him its touch doesn't hurt (its +0x18 bit 2), nor his body it.
 *   On a wall (its pad up while its hop meets a wall ahead: 0x020AC9B8): it clings (anim 2: 0x020ADBCC, 0x020ADA84), still (anim 3:
 *   0x020AD894, 0x020AD74C), climbs up at 0.4 (anim 6: 0x020AD6A4) or 2.2 (anim 5: 0x020AD2A4), down at 0.4 (anim 8: 0x020AD4AC) or
 *   2.2 (anim 7: 0x020AD0BC) as its pad's up or down (2.2 with its pad's C, which only its walk into a wall presses); pressed into the
 *   wall at 1.0; past the wall's end (0x020AC880) it stands (turning round going down); down on the ground with its pad back it turns
 *   and walks or stands (0x020ACA0C); A jumps off (0x020ACD40: turned round, 3.0 up, 0x020AF5B0); B crouches (anim 26, 61 frames),
 *   turns round and pounces off it (0x020AEB18, 0x020AEA40, 0x020AE918, 0x020AE894).
 * • Boxes (gawgaw.rca, per animation frame, as uja.c's): its body's hit box (12 pixels behind to 10 ahead, 1 to 19 up), the bites'
 *   reach 16 pixels ahead (or up, down); the leap only grabs, its touch harmless.
 * Its sounds (Squeak Squad's, SqueakSfx): 0x124 the hops and crouches, 0x13F the bite and chew, 0x140 the leap. Not ported: its ability (Squeak Squad's: none),
 * the other mode (the brain's +8 not 3: MlG's gravity, A in the air), its contact damages (Squeak Squad's: KatAM's 1), its event 0x15
 * (vtable +0x24, 0x020AFD90: lets Kirby go where it is; what sends it wasn't found). */
#include "../squeak.h"
#include "main.h"
#include "random.h"

void sub_0804E09C(struct Kirby *);

// Animations (gawgaw.cea);
enum { ANIM_STILL, ANIM_STAND, ANIM_WALL, ANIM_WALL_STILL, ANIM_WALK, ANIM_WALL_UP_FAST, ANIM_WALL_UP, ANIM_WALL_DOWN_FAST,
       ANIM_WALL_DOWN, ANIM_CROUCH, ANIM_RISE, ANIM_FALL, ANIM_CHARGE, ANIM_LEAP_START, ANIM_LEAP, ANIM_BITE_HOLD, ANIM_16, ANIM_CHEW,
       ANIM_CHEW_THROW, ANIM_SHAKE, ANIM_SPRING_BACK, ANIM_BITE, ANIM_BITE_UP, ANIM_BITE_DOWN, ANIM_HURT, ANIM_HURT_2,
       ANIM_WALL_CHARGE };

// prmCE22Ml0.kgb;
#define GRAVITY 0x2B9
#define WALK_SPEED 0x2334
#define HOP_SPEED_DOWN 0x1D9A           // Its hops' speeds up (its pad down, none, up); the wall jump's the middle one;
#define HOP_SPEED 0x3000
#define HOP_SPEED_UP 0x4000
#define LEAP_SPEED_Y 0x1D9A
#define LEAP_SPEED_X 0x3334
// prmCE22Pl0.kgb;
#define HOP_CHANCE 75
#define HOP_DOWN 60
#define HOP_NONE 30
#define NEAR_X 0x42900000               // (72.0 and 18.0 as floats, read as whole numbers);
#define NEAR_Y 0x41900000
// The code's;
#define MAX_FALL -0x2666
#define AIR_SPEED 0x2333
#define WALL_PRESS 0x1000
#define WALL_SLOW 0x666
#define WALL_FAST 0x2333
#define CHARGE_TIME 60
#define SPRING_X 0x199A
#define SPRING_Y 0x2CCD
#define SPRING_GRAVITY 0x19A
#define FACE_DISTANCE 0x1000
#define STAND_TIME 44
#define CROUCH_TIME 30
#define JUMP_TIME 60
#define POUNCE_CHANCE 50
#define KIRBY_FEET 7                    // KatAM's Kirby's spot is 7 pixels over his feet, Squeak Squad's at them;
#define HOLD_X -8                       // Kirby 8 pixels ahead of it (his x: he faces it), level with it;
#define THROW_X -37                     // Squeak Squad's thrown Kirby's -0x24CD (his back) and 0x219A up: KatAM's x 0x10;
#define THROW_Y 34
#define TOSS_X -16                      // The chew's: -0x1000 and 0x1000;
#define TOSS_Y 16

enum {
    BODY_SPAWN, BODY_SPAWN_FALL, BODY_START, BODY_STAND, BODY_WALK_ENTRY, BODY_WALK, BODY_HOP_ENTRY, BODY_CROUCH, BODY_RISE,
    BODY_FALL_ENTRY, BODY_FALL, BODY_BITE_ENTRY, BODY_BITE, BODY_POUNCE_ENTRY, BODY_CHARGE, BODY_LEAP_START, BODY_LEAP,
    BODY_GRAB_ENTRY, BODY_GRAB, BODY_SHAKE, BODY_SPRING_BACK, BODY_GRAB_STILL_ENTRY, BODY_GRAB_STILL, BODY_CHEW, BODY_SPRING_BACK_2,
    BODY_WALL_JUMP_ENTRY, BODY_WALL_POUNCE_ENTRY,
    BODY_WALL_CHARGE, BODY_WALL_LEAP_START, BODY_WALL_LEAP, BODY_WALL_ENTRY, BODY_WALL, BODY_WALL_STILL_ENTRY, BODY_WALL_STILL,
    BODY_WALL_UP_ENTRY, BODY_WALL_UP, BODY_WALL_DOWN_ENTRY, BODY_WALL_DOWN, BODY_WALL_UP_FAST_ENTRY, BODY_WALL_UP_FAST,
    BODY_WALL_DOWN_FAST_ENTRY, BODY_WALL_DOWN_FAST,
};
enum {
    BRAIN_HOP_START, BRAIN_HOP_WAIT_START, BRAIN_HOP_WAIT, BRAIN_HOP, BRAIN_HOP_UP, BRAIN_HOP_LAND, BRAIN_POUNCE_START, BRAIN_LOOK,
    BRAIN_CROUCH, BRAIN_CHARGE, BRAIN_POUNCING, BRAIN_JUMP, BRAIN_JUMPING, BRAIN_JUMP_LAND, BRAIN_STILL_START, BRAIN_STILL,
};

// Its body's modes (+0x214: its brain reads 7, holding Kirby still);
enum { MODE_STAND, MODE_WALK, MODE_WALL_JUMP, MODE_HOP, MODE_FALL, MODE_BITE, MODE_POUNCE, MODE_GRAB_STILL, MODE_GRAB, MODE_WALL,
       MODE_WALL_STILL, MODE_WALL_UP, MODE_WALL_DOWN, MODE_WALL_UP_FAST, MODE_WALL_DOWN_FAST, MODE_SPAWN = 0xFF };

#define HEIGHT(s) ((s)->vars[0])        // 0x210: its hop's height (its pad down, none, up: 0-2);
#define COUNT(s) ((s)->vars[1])         // 0x1F8: the charges' frames;
#define MODE(s) ((s)->vars[2])          // 0x214;
#define PAD_C(s) ((s)->vars[3])         // Its brain's +0x2C/+0x48 (C): only its walk into a wall presses it;
#define WAIT(s) ((s)->vars[4])          // The brain's 0x9C;
#define MET_WALL(s) ((s)->vars[5])      // The brain's 0xA0: a wall met in a pounce;
#define HARM(s) ((s)->vars[6])          // KatAM's unk68 from InitObject: its touch's harm;
#define BOX(s) (*(u8 *)&(s)->vars[7])   // This frame's box: sGawBoxes' index;
#define HELD(s) (*(struct Kirby **)&(s)->vars[8])   // 0x164;

/* Its boxes (gawgaw.rca: its animations' frames, each one's tags and boxes, as KatAM's; boxes.py, by the notes' rca layout: the
 * .rca's animation k + 1 is the .cea's k); */
static const struct SqueakBox sGawBoxes[] = {
    { { -12, -19, 10, -1 }, TRUE, 0 },
    { { -11, -22, 8, 0 }, TRUE, 0 },
    { { -12, -19, 10, 5 }, TRUE, 0 },
    { { -12, -21, 10, -1 }, TRUE, 0 },
    { { -13, -19, 10, -1 }, TRUE, 0 },
    { { -10, -23, 2, -5 }, FALSE, 2, { { -7, -21, -1, -9 }, { -1, -21, 5, -9 } } },
    { { -12, -20, 10, -1 }, TRUE, 0 },
    { { -12, -19, 16, -1 }, TRUE, 0 },
    { { -12, -30, 10, -1 }, TRUE, 0 },
    { { -12, -19, 10, 11 }, TRUE, 0 },
    { { -12, -19, 10, -1 }, FALSE, 0 },
};
static const struct SqueakBoxFrame sGawAnim0[] = { { 4, 0 } };
static const struct SqueakBoxFrame sGawAnim1[] = { { 44, 0 } };
static const struct SqueakBoxFrame sGawAnim2[] = { { 1, 1 }, { 43, 0 } };
static const struct SqueakBoxFrame sGawAnim3[] = { { 44, 2 } };
static const struct SqueakBoxFrame sGawAnim4[] = { { 27, 0 } };
static const struct SqueakBoxFrame sGawAnim5[] = { { 27, 0 } };
static const struct SqueakBoxFrame sGawAnim6[] = { { 38, 0 } };
static const struct SqueakBoxFrame sGawAnim7[] = { { 27, 2 } };
static const struct SqueakBoxFrame sGawAnim8[] = { { 38, 2 } };
static const struct SqueakBoxFrame sGawAnim9[] = { { 8, 0 } };
static const struct SqueakBoxFrame sGawAnim10[] = { { 1, 3 } };
static const struct SqueakBoxFrame sGawAnim11[] = { { 10, 0 } };
static const struct SqueakBoxFrame sGawAnim12[] = { { 4, 4 } };
static const struct SqueakBoxFrame sGawAnim13[] = { { 2, 0 } };
static const struct SqueakBoxFrame sGawAnim14[] = { { 1, 5 } };
static const struct SqueakBoxFrame sGawAnim15[] = { { 26, 0 } };
static const struct SqueakBoxFrame sGawAnim16[] = { { 10, 0 } };
static const struct SqueakBoxFrame sGawAnim17[] = { { 90, 6 } };
static const struct SqueakBoxFrame sGawAnim18[] = { { 30, 0 } };
static const struct SqueakBoxFrame sGawAnim19[] = { { 34, 6 } };
static const struct SqueakBoxFrame sGawAnim20[] = { { 16, 0 } };
static const struct SqueakBoxFrame sGawAnim21[] = { { 12, 0 }, { 1, 7 }, { 1, 7 }, { 16, 0 } };
static const struct SqueakBoxFrame sGawAnim22[] = { { 12, 0 }, { 1, 8 }, { 1, 8 }, { 16, 0 } };
static const struct SqueakBoxFrame sGawAnim23[] = { { 1, 0 }, { 11, 0 }, { 1, 9 }, { 1, 9 }, { 16, 0 } };
static const struct SqueakBoxFrame sGawAnim24[] = { { 1, 10 } };
static const struct SqueakBoxFrame sGawAnim25[] = { { 1, 10 } };
static const struct SqueakBoxFrame sGawAnim26[] = { { 4, 0 } };
static const struct SqueakBoxAnim sGawBoxAnims[] = {
    { sGawAnim0, 1 },
    { sGawAnim1, 1 },
    { sGawAnim2, 2 },
    { sGawAnim3, 1 },
    { sGawAnim4, 1 },
    { sGawAnim5, 1 },
    { sGawAnim6, 1 },
    { sGawAnim7, 1 },
    { sGawAnim8, 1 },
    { sGawAnim9, 1 },
    { sGawAnim10, 1 },
    { sGawAnim11, 1 },
    { sGawAnim12, 1 },
    { sGawAnim13, 1 },
    { sGawAnim14, 1 },
    { sGawAnim15, 1 },
    { sGawAnim16, 1 },
    { sGawAnim17, 1 },
    { sGawAnim18, 1 },
    { sGawAnim19, 1 },
    { sGawAnim20, 1 },
    { sGawAnim21, 4 },
    { sGawAnim22, 4 },
    { sGawAnim23, 5 },
    { sGawAnim24, 1 },
    { sGawAnim25, 1 },
    { sGawAnim26, 1 },
};
static const struct SqueakBoxes sGawBoxData = { sGawBoxes, sGawBoxAnims, 27 };

/* Kirby held (kirby->unk110), facing it (KatAM's hold faces him as it: turned round), 8 pixels ahead of it, level with it, in KatAM's
 * grabbers' pose (0x57); thrown away hurt as he lands (flag 2), or let go as he is (flag 1), as Bio Spark's; */
static const struct Kirby_110 sHold[] = { { HOLD_X, -KIRBY_FEET, -1, 0xFF, 0x57, 0x400 }, { 0, 0, 0, 1, 0, 0x400 } };
static const struct Kirby_110 sThrow[] = { { THROW_X, THROW_Y, -1, 1, 0, 0x402 }, { 0, 0, 0, 1, 0, 0 } };
static const struct Kirby_110 sToss[] = { { TOSS_X, TOSS_Y, -1, 1, 0, 0x402 }, { 0, 0, 0, 1, 0, 0 } };
static const struct Kirby_110 sLetGo[] = { { 0, 0, -1, 1, 0, 0x401 }, { 0, 0, 0, 1, 0, 0 } };

static void GawGawMain(struct Object *obj);

static u32 Random(void) {
    return Rand16() * 100 >> 16;
}

// StillHeld(); The Kirby it holds is still held by it (KatAM's hold lets him go on its own when it's beaten);
static bool32 StillHeld(struct SqueakObject *s) {
    struct Kirby *kirby = HELD(s);

    return kirby && kirby->base.unk6C == &s->obj.base && kirby->unk110 == sHold;
}

// LetGo(); Kirby thrown away (sThrow, sToss) or let go as he is (sLetGo), the table started over;
static void LetGo(struct SqueakObject *s, const struct Kirby_110 *how) {
    struct Kirby *kirby = HELD(s);

    if (StillHeld(s)) {
        kirby->unk110 = how;
        kirby->base.counter = -1;
    }
    HELD(s) = NULL;
    s->obj.base.unk6C = NULL;
}

/* Grab(); 0x020AFEB0: its jaws touched a Kirby: it holds him (KatAM's hold, sub_0804E09C, which faces him as it: then turned to face
 * it) and stops; going forward, its bite and shake, else its bite and chew; */
static void Grab(struct SqueakObject *s, struct Kirby *kirby) {
    HELD(s) = kirby;
    kirby->base.unk6C = &s->obj.base;
    kirby->unk110 = sHold;
    s->obj.base.unk6C = kirby;
    kirby->base.flags |= 0x40000;
    sub_0804E09C(kirby);
    kirby->base.flags ^= 1;
    SqueakStop(s);
    SqueakSetState(s, s->padDir == 0 ? BODY_GRAB_STILL_ENTRY : BODY_GRAB_ENTRY);
}

// Gravity(); 0x020ACE78: on the ground, still; else falling GRAVITY a frame, no faster than MAX_FALL;
static void Gravity(struct SqueakObject *s) {
    if (SqueakOnGround(s)) {
        s->vy = s->ay = 0;
    } else if (!SqueakCapFall(s, MAX_FALL)) {
        s->ay = -GRAVITY;
    }
}

// AirControl(); 0x020AFD54: back turns it round; forward, AIR_SPEED (no cap);
static void AirControl(struct SqueakObject *s) {
    if (s->padDir < 0) SqueakTurnAround(s);
    if (s->padDir > 0) {
        s->vx = AIR_SPEED;
        s->maxVx = 0;
    }
}

// Still(); The speeds' caps, accelerations and friction none;
static void Still(struct SqueakObject *s) {
    s->maxVx = s->maxVy = s->ax = s->ay = s->fx = s->fy = 0;
}

// SetState(); A state that sets its body's mode;
static bool32 SetState(struct SqueakObject *s, u8 state, u8 mode) {
    SqueakSetState(s, state);
    MODE(s) = mode;
    return TRUE;
}

// OnWall(); The wall it clings to: met ahead (Squeak Squad's 0x800), or either side (both ends of its box: the stage's mode 2);
static bool32 OnWall(struct SqueakObject *s) {
    return SqueakWallAhead(s) || SqueakTouches(s, SQUEAK_LEFT, 0, TRUE) || SqueakTouches(s, SQUEAK_RIGHT, 0, TRUE);
}

/* WallEntry(); 0x020ADBCC, 0x020AD894: on a wall (anim), no caps, pressed into it at WALL_PRESS; no wall there, it falls; C let go; */
static bool32 WallEntry(struct SqueakObject *s, u8 anim, u8 next) {
    SqueakPlay(s, anim);
    Still(s);
    s->vx = WALL_PRESS;
    s->vy = 0;
    SqueakSetState(s, OnWall(s) ? next : BODY_FALL_ENTRY);
    PAD_C(s) = 0;
    return TRUE;
}

// GrabWall(); 0x020AC9B8: a wall ahead and its pad up: it clings to it;
static bool32 GrabWall(struct SqueakObject *s) {
    if (!SqueakWallAhead(s) || s->padY <= 0) return FALSE;
    SqueakSetState(s, BODY_WALL_ENTRY);
    return TRUE;
}

// The wall's pad: 0x020ACB7C (up, or forward), 0x020ACB2C (down, or back), 0x020ACBCC / 0x020ACC20 (neither: still);
static bool32 WallUp(struct SqueakObject *s) {
    return s->padDir >= 0 && (s->padY > 0 || s->padDir > 0) && SetState(s, BODY_WALL_UP_ENTRY, MODE_WALL_UP);
}
static bool32 WallDown(struct SqueakObject *s) {
    return s->padDir < 1 && (s->padY < 0 || s->padDir < 0) && SetState(s, BODY_WALL_DOWN_ENTRY, MODE_WALL_DOWN);
}
static bool32 WallStill(struct SqueakObject *s, u8 state, u8 mode) {
    return s->padDir == 0 && s->padY == 0 && SetState(s, state, mode);
}

// WallLand(); 0x020ACA0C: down on the ground with its pad back: it turns round and walks (forward or C) or stands;
static bool32 WallLand(struct SqueakObject *s) {
    if (!SqueakOnGround(s) || s->padDir >= 0) return FALSE;
    SqueakTurnAround(s);
    SqueakSetState(s, s->padDir >= 1 || PAD_C(s) ? BODY_WALK_ENTRY : BODY_START);
    return TRUE;
}

// WallJump(); 0x020ACD40: A (not forward): it turns round, moves off and jumps (0x020AF5B0, from the next frame);
static bool32 WallJump(struct SqueakObject *s) {
    if (!s->padA || s->padDir >= 1) return FALSE;
    SqueakTurnAround(s);
    s->vx = WALL_PRESS;
    SqueakMove(s);
    SqueakSetState(s, BODY_WALL_JUMP_ENTRY);
    return TRUE;
}

/* WallMove(); The wall's states' move (0x020ACED0's...): pressed into it at WALL_PRESS, A jumps off; moved, past the wall's end on
 * the side it faces (0x020AC880, the stage's mode 1: either end of its box), it stands (going down, turned round); */
static void WallMove(struct SqueakObject *s) {
    s->vx = WALL_PRESS;
    if (WallJump(s)) return;
    SqueakMove(s);
    if (!SqueakTouches(s, s->obj.base.flags & 1 ? SQUEAK_LEFT : SQUEAK_RIGHT, 0, FALSE)) {
        if (s->padY < 0) SqueakTurnAround(s);
        SqueakSetState(s, BODY_START);
    }
}

// Hold(); Kirby held where it is (Squeak Squad's each frame: his speeds none, 8 pixels ahead of it);
static void Hold(struct SqueakObject *s) {
    if (StillHeld(s)) HELD(s)->unk110 = sHold;
}

/* Throw(); Kirby thrown (`how`), and it springs back (SPRING_X back, SPRING_Y up, falling `gravity` a frame; `anim`); its touch hurts
 * again (its +0x18 bits 2 and 4); */
static bool32 Throw(struct SqueakObject *s, const struct Kirby_110 *how, s32 gravity, u8 anim, u8 next) {
    LetGo(s, how);
    SqueakStop(s);
    s->vx = -SPRING_X;
    s->vy = SPRING_Y;
    s->ay = -gravity;
    SqueakPlay(s, anim);
    SqueakSetState(s, next);
    return TRUE;
}

static bool32 Body(struct SqueakObject *s) {
    bool32 ended;

    switch (s->state) {
    case BODY_SPAWN:        // 0x020AFC70: it falls (anim 11);
        SqueakPlay(s, ANIM_FALL);
        SqueakStop(s);
        s->ay = -GRAVITY;
        COUNT(s) = 4;
        return SetState(s, BODY_SPAWN_FALL, MODE_SPAWN);
    case BODY_WALL_JUMP_ENTRY:  // 0x020AF5B0: 3.0 up, its hop's rise;
        SqueakPlay(s, ANIM_RISE);
        SqueakStop(s);
        SqueakSfx(s, 0x124);
        s->vy = HOP_SPEED;
        s->ay = -GRAVITY;
        HEIGHT(s) = 1;
        return SetState(s, BODY_RISE, MODE_WALL_JUMP);
    case BODY_SPAWN_FALL:   // 0x020AF9D8: a wall either side (both ends of its box), it faces it and clings to it;
        if (SqueakTouches(s, SQUEAK_LEFT, 0, TRUE) || SqueakTouches(s, SQUEAK_RIGHT, 0, TRUE)) {
            bool32 left = SqueakTouches(s, SQUEAK_LEFT, 0, TRUE);

            if (left != !!(s->obj.base.flags & 1)) SqueakTurnAround(s);
            SqueakPlay(s, ANIM_WALL);
            SqueakSetState(s, BODY_WALL_ENTRY);
            return TRUE;
        }
        SqueakMove(s);
        if (SqueakOnGround(s)) {
            SqueakSetState(s, BODY_START);
            s->vy = s->ay = s->ax = 0;
            break;
        }
        SqueakCapFall(s, MAX_FALL);
        break;
    case BODY_START:        // 0x020AF938;
        SqueakPlay(s, ANIM_STAND);
        SqueakStop(s);
        return SetState(s, BODY_STAND, MODE_STAND);
    case BODY_STAND:        // 0x020AF890: A hops, B bites, forward (or C) walks;
        if (s->padA) return SetState(s, BODY_HOP_ENTRY, MODE_HOP);
        if (s->padB) return SetState(s, BODY_BITE_ENTRY, MODE_BITE);
        if (s->padDir >= 1 || PAD_C(s)) return SetState(s, BODY_WALK_ENTRY, MODE_WALK);
        if (s->padDir < 0) SqueakTurnAround(s);
        SqueakMove(s);
        if (!SqueakOnGround(s)) SqueakSetState(s, BODY_FALL_ENTRY);
        break;
    case BODY_WALK_ENTRY:   // 0x020AF7F0 (its walk from the next frame: it returns 0);
        SqueakPlay(s, ANIM_WALK);
        SqueakStop(s);
        SetState(s, BODY_WALK, MODE_WALK);
        break;
    case BODY_WALK:         // 0x020AF6B0: into a wall with its pad up, C and up it fast;
        if (s->padA) return SetState(s, BODY_HOP_ENTRY, MODE_HOP);
        if (s->padB) return SetState(s, BODY_BITE_ENTRY, MODE_BITE);
        if (SqueakWallAhead(s) && s->padY > 0) {
            PAD_C(s) = 1;
            return SetState(s, BODY_WALL_UP_FAST_ENTRY, MODE_WALL_UP_FAST);
        }
        if (!s->padY && !s->padDir && !PAD_C(s)) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        s->vx = WALK_SPEED;
        if (s->padDir < 0) SqueakTurnAround(s);
        SqueakMove(s);
        if (!SqueakOnGround(s)) SqueakSetState(s, BODY_FALL_ENTRY);
        break;
    case BODY_HOP_ENTRY:    // 0x020AF4D8: its height as its pad's down, none or up;
        SqueakPlay(s, ANIM_CROUCH);
        SqueakStop(s);
        SqueakSfx(s, 0x124);
        HEIGHT(s) = s->padY < 0 ? 0 : s->padY < 1 ? 1 : 2;
        return SetState(s, BODY_CROUCH, MODE_HOP);
    case BODY_CROUCH:       // 0x020AF334: B pounces; crouched, it hops;
        if (s->padB) {
            SqueakSetState(s, BODY_POUNCE_ENTRY);
            return TRUE;
        }
        if (s->animEnded) {
            SqueakPlay(s, ANIM_RISE);
            s->vy = HEIGHT(s) == 0 ? HOP_SPEED_DOWN : HEIGHT(s) == 1 ? HOP_SPEED : HOP_SPEED_UP;
            s->ay = -GRAVITY;
            SqueakSetState(s, BODY_RISE);
            return TRUE;
        }
        if (GrabWall(s)) return TRUE;
        SqueakMove(s);
        break;
    case BODY_RISE:         // 0x020AF2BC;
        if (GrabWall(s)) return TRUE;
        AirControl(s);
        SqueakMove(s);
        if (s->vy < 0) SqueakSetState(s, BODY_FALL_ENTRY);
        break;
    case BODY_FALL_ENTRY:   // 0x020AF1B4: its speeds as they were;
        SqueakPlay(s, ANIM_FALL);
        Still(s);
        s->ay = -GRAVITY;
        return SetState(s, BODY_FALL, MODE_FALL);
    case BODY_FALL:         // 0x020AF0F4;
        AirControl(s);
        if (GrabWall(s)) return TRUE;
        SqueakMove(s);
        if (SqueakOnGround(s)) {
            SqueakSetState(s, BODY_START);
            s->vy = s->ay = s->ax = 0;
            break;
        }
        SqueakCapFall(s, MAX_FALL);
        break;
    case BODY_BITE_ENTRY:   // 0x020AEF7C: up, down or ahead as its pad;
        SqueakStop(s);
        SqueakPlay(s, s->padY < 0 ? ANIM_BITE_DOWN : s->padY == 0 ? ANIM_BITE : ANIM_BITE_UP);
        SqueakSfx(s, 0x13F);
        SqueakSetState(s, BODY_BITE);
        return TRUE;
    case BODY_BITE:         // 0x020AEF14;
        if (s->animEnded) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        SqueakMove(s);
        break;
    case BODY_POUNCE_ENTRY: // 0x020AEE5C;
        SqueakPlay(s, ANIM_CHARGE);
        SqueakStop(s);
        SqueakSfx(s, 0x124);
        COUNT(s) = CHARGE_TIME;
        return SetState(s, BODY_CHARGE, MODE_POUNCE);
    case BODY_CHARGE:       // 0x020AED7C: CHARGE_TIME + 1 frames;
    case BODY_WALL_CHARGE:  // 0x020AEA40: then turned round from the wall;
        if (COUNT(s)-- == 0) {
            if (s->state == BODY_WALL_CHARGE) SqueakTurnAround(s);
            else SqueakSfx(s, 0x140);
            SqueakPlay(s, ANIM_LEAP_START);
            SqueakStop(s);
            SqueakSetState(s, s->state == BODY_WALL_CHARGE ? BODY_WALL_LEAP_START : BODY_LEAP_START);
            return TRUE;
        }
        SqueakMove(s);
        break;
    case BODY_LEAP_START:   // 0x020AEC54, 0x020AE918: LEAP_SPEED_X forward, LEAP_SPEED_Y up;
    case BODY_WALL_LEAP_START:
        if (s->animEnded) {
            SqueakPlay(s, ANIM_LEAP);
            SqueakStop(s);
            s->vx = LEAP_SPEED_X;
            s->vy = LEAP_SPEED_Y;
            s->ay = -GRAVITY;
            SqueakSetState(s, s->state == BODY_LEAP_START ? BODY_LEAP : BODY_WALL_LEAP);
            return TRUE;
        }
        SqueakMove(s);
        break;
    case BODY_LEAP:         // 0x020AEBD0, 0x020AE894: its jaws grab (GawGawMain); landed, it stands;
    case BODY_WALL_LEAP:
        if (GrabWall(s)) return TRUE;
        SqueakCapFall(s, MAX_FALL);
        SqueakMove(s);
        if (SqueakOnGround(s)) {
            SqueakSetState(s, BODY_START);
            SqueakStop(s);
        }
        break;
    case BODY_GRAB_ENTRY:   // 0x020AE280;
        SqueakPlay(s, ANIM_BITE_HOLD);
        SqueakStop(s);
        return SetState(s, BODY_GRAB, MODE_GRAB);
    case BODY_GRAB:         // 0x020AE118: its bite, then the shake;
        Hold(s);
        if (s->animEnded) {
            SqueakPlay(s, ANIM_SHAKE);
            SqueakSetState(s, BODY_SHAKE);
            return TRUE;
        }
        Gravity(s);
        SqueakMove(s);
        break;
    case BODY_SHAKE:        // 0x020ADE68: shaken, Kirby thrown back;
        Hold(s);
        if (s->animEnded) return Throw(s, sThrow, SPRING_GRAVITY, ANIM_SPRING_BACK, BODY_SPRING_BACK);
        Gravity(s);
        SqueakMove(s);
        break;
    case BODY_GRAB_STILL_ENTRY: // 0x020AE7F0;
        SqueakPlay(s, ANIM_BITE_HOLD);
        SqueakStop(s);
        return SetState(s, BODY_GRAB_STILL, MODE_GRAB_STILL);
    case BODY_GRAB_STILL:   // 0x020AE67C: its bite, then the chew;
        Hold(s);
        if (s->animEnded) {
            SqueakPlay(s, ANIM_CHEW);
            SqueakSfx(s, 0x13F);
            SqueakSetState(s, BODY_CHEW);
            return TRUE;
        }
        Gravity(s);
        SqueakMove(s);
        break;
    case BODY_CHEW:         // 0x020AE3D0: chewed, Kirby tossed;
        Hold(s);
        if (s->animEnded) return Throw(s, sToss, GRAVITY, ANIM_CHEW_THROW, BODY_SPRING_BACK_2);
        Gravity(s);
        SqueakMove(s);
        break;
    case BODY_SPRING_BACK:  // 0x020ADDBC, 0x020AE324: landed, it stands once its animation's over (sliding till then);
    case BODY_SPRING_BACK_2:
        ended = s->animEnded;
        SqueakMove(s);
        if (SqueakOnGround(s)) {
            if (ended) SqueakSetState(s, BODY_START);
            s->vy = s->ay = s->ax = 0;
            break;
        }
        SqueakCapFall(s, MAX_FALL);
        break;
    case BODY_WALL_POUNCE_ENTRY:    // 0x020AEB18: crouched on the wall;
        SqueakPlay(s, ANIM_WALL_CHARGE);
        SqueakStop(s);
        SqueakSfx(s, 0x124);
        COUNT(s) = CHARGE_TIME;
        return SetState(s, BODY_WALL_CHARGE, MODE_POUNCE);
    case BODY_WALL_ENTRY:   // 0x020ADBCC;
        return WallEntry(s, ANIM_WALL, BODY_WALL);
    case BODY_WALL_STILL_ENTRY:     // 0x020AD894;
        return WallEntry(s, ANIM_WALL_STILL, BODY_WALL_STILL);
    case BODY_WALL_UP_ENTRY:        // 0x020AD6A4, 0x020AD4AC, 0x020AD2A4, 0x020AD0BC: their speeds up or down;
    case BODY_WALL_DOWN_ENTRY:
    case BODY_WALL_UP_FAST_ENTRY:
    case BODY_WALL_DOWN_FAST_ENTRY: {
        static const u8 sAnims[] = { ANIM_WALL_UP, ANIM_WALL_DOWN, ANIM_WALL_UP_FAST, ANIM_WALL_DOWN_FAST };
        static const s32 sSpeeds[] = { WALL_SLOW, -WALL_SLOW, WALL_FAST, -WALL_FAST };
        u8 i = (s->state - BODY_WALL_UP_ENTRY) / 2;

        s->vy = sSpeeds[i];
        SqueakPlay(s, sAnims[i]);
        SqueakSetState(s, s->state + 1);
        return TRUE;
    }
    case BODY_WALL:         // 0x020ADA84;
    case BODY_WALL_STILL:   // 0x020AD74C;
        if (s->padB) return SetState(s, BODY_WALL_POUNCE_ENTRY, MODE_POUNCE);
        if (WallUp(s) || WallDown(s)) return TRUE;
        s->vy = 0;
        WallMove(s);
        break;
    case BODY_WALL_UP:      // 0x020AD554: C up fast; down; neither, it clings;
        if (s->padB) return SetState(s, BODY_WALL_POUNCE_ENTRY, MODE_POUNCE);
        if (PAD_C(s)) return SetState(s, BODY_WALL_UP_FAST_ENTRY, MODE_WALL_UP_FAST);
        if (WallDown(s) || WallStill(s, BODY_WALL_ENTRY, MODE_WALL)) return TRUE;
        WallMove(s);
        break;
    case BODY_WALL_DOWN:    // 0x020AD34C: C down fast; up; neither, still; the ground;
        if (s->padB) return SetState(s, BODY_WALL_POUNCE_ENTRY, MODE_POUNCE);
        if (PAD_C(s)) return SetState(s, BODY_WALL_DOWN_FAST_ENTRY, MODE_WALL_DOWN_FAST);
        if (WallUp(s) || WallStill(s, BODY_WALL_STILL_ENTRY, MODE_WALL_STILL) || WallLand(s)) return TRUE;
        WallMove(s);
        break;
    case BODY_WALL_UP_FAST: // 0x020AD164: down; neither, it clings;
        if (s->padB) return SetState(s, BODY_WALL_POUNCE_ENTRY, MODE_POUNCE);
        if (WallDown(s) || WallStill(s, BODY_WALL_ENTRY, MODE_WALL)) return TRUE;
        WallMove(s);
        break;
    case BODY_WALL_DOWN_FAST:   // 0x020ACED0: up; neither, still; the ground;
        if (s->padB) return SetState(s, BODY_WALL_POUNCE_ENTRY, MODE_POUNCE);
        if (WallUp(s) || WallStill(s, BODY_WALL_STILL_ENTRY, MODE_WALL_STILL) || WallLand(s)) return TRUE;
        WallMove(s);
        break;
    }
    return FALSE;
}

// Facing(); 0x020AC1F4's: it faces Kirby;
static bool32 Facing(struct SqueakObject *s) {
    return SQ_X(&s->obj) > SQ_X(s->obj.kirby3) ? (s->obj.base.flags & 1) != 0 : !(s->obj.base.flags & 1);
}

// Brain(); Its pad (0x020AC6A8, 0x020AC538, 0x020AC084 and the states they lead to);
static void Brain(struct SqueakObject *s) {
    u32 r;

    switch (s->brain) {
    case BRAIN_HOP_START:       // 0x020AC6A8;
        SqueakFaceKirby(s, FACE_DISTANCE);
        s->brain = BRAIN_HOP_WAIT_START;
        break;
    case BRAIN_HOP_WAIT_START:  // 0x020AC688;
        s->padDir = 0;
        WAIT(s) = STAND_TIME;
        s->brain = BRAIN_HOP_WAIT;
        break;
    case BRAIN_HOP_WAIT:        // 0x020AC610;
        if (--WAIT(s) == 0) s->brain = Random() < HOP_CHANCE ? BRAIN_HOP : BRAIN_HOP_WAIT_START;
        break;
    case BRAIN_HOP:             // 0x020AC59C: its pad down, none or up;
        r = Random();
        s->padA = 1;
        s->padY = r < HOP_DOWN ? -1 : r < HOP_DOWN + HOP_NONE ? 0 : 1;
        s->brain = BRAIN_HOP_UP;
        break;
    case BRAIN_HOP_UP:          // 0x020AC574;
        if (!SqueakOnGround(s)) {
            s->padY = 0;
            s->padA = 0;
            s->brain = BRAIN_HOP_LAND;
        }
        break;
    case BRAIN_HOP_LAND:        // 0x020AC558;
        if (SqueakOnGround(s)) s->brain = BRAIN_HOP_START;
        break;
    case BRAIN_POUNCE_START:    // 0x020AC538;
        SqueakFaceKirby(s, FACE_DISTANCE);
        s->brain = BRAIN_LOOK;
        break;
    case BRAIN_LOOK:            // 0x020AC4E0: (always within);
        s->padDir = 0;
        WAIT(s) = STAND_TIME;
        if (SqueakKirbyWithin(s, NEAR_X, NEAR_Y)) {
            MET_WALL(s) = 0;
            SqueakFaceKirby(s, FACE_DISTANCE);
            s->brain = BRAIN_CROUCH;
        }
        break;
    case BRAIN_CROUCH:          // 0x020AC358;
        s->padDir = 0;
        s->padA = 1;
        WAIT(s) = CROUCH_TIME;
        s->brain = BRAIN_CHARGE;
        break;
    case BRAIN_CHARGE:          // 0x020AC2D4: B held; in the air or CROUCH_TIME on, let go, forward half the time;
        s->padB = 1;
        if (SqueakOnGround(s) && --WAIT(s) != 0) break;
        s->padA = s->padB = 0;
        s->padDir = Random() < POUNCE_CHANCE ? 0 : 1;
        s->brain = BRAIN_POUNCING;
        break;
    case BRAIN_POUNCING:        // 0x020AC1F4;
        if (SqueakWallAhead(s)) MET_WALL(s) = 1;
        if (!SqueakOnGround(s)) {
            if (MODE(s) == MODE_GRAB_STILL) {
                s->padDir = 0;
                s->brain = BRAIN_LOOK;
            }
            break;
        }
        if (MET_WALL(s)) MET_WALL(s) = Facing(s);
        s->brain = MET_WALL(s) ? BRAIN_JUMP : BRAIN_LOOK;
        break;
    case BRAIN_JUMP:            // 0x020AC4BC;
        s->padA = 1;
        WAIT(s) = JUMP_TIME;
        s->brain = BRAIN_JUMPING;
        break;
    case BRAIN_JUMPING:         // 0x020AC158;
        if (SqueakOnGround(s) && --WAIT(s) != 0) break;
        s->padA = 0;
        s->brain = BRAIN_JUMP_LAND;
        break;
    case BRAIN_JUMP_LAND:       // 0x020AC13C;
        if (SqueakOnGround(s)) s->brain = BRAIN_LOOK;
        break;
    case BRAIN_STILL_START:     // 0x020AC084;
        SqueakFaceKirby(s, FACE_DISTANCE);
        s->brain = BRAIN_STILL;
        break;
    case BRAIN_STILL:           // 0x020AC07C;
        s->padDir = 0;
        break;
    }
}

static void GawGawMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;
    struct Kirby *kirby;

    Brain(s);
    SqueakRun(s, Body);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
    // (holding Kirby, its touch doesn't hurt: its +0x18 bit 2, set by the grab, let go by the throw);
    SqueakSetBoxes(s, &sGawBoxData, HELD(s) ? 0 : HARM(s), &BOX(s));
    if (HELD(s)) obj->base.unk68 = 0;
    // (that bit also takes it out of every attack's reach: 0x0201322C, untouchable);
    if (HELD(s)) SqueakUntouchable(s, TRUE);
    kirby = obj->unk80 > 0 && !HELD(s) ? SqueakGrabbing(s, &sGawBoxes[BOX(s)]) : NULL;
    if (kirby) Grab(s, kirby);
    // (holding Kirby, his body doesn't hurt it, as KatAM's grabbers' and Bio Spark's);
    if (HELD(s)) obj->base.unk5C |= 0x80;
    else obj->base.unk5C &= ~0x80;
}

/* GawGawWatch(); Its pre-callback: KatAM's hit put it out of its own state (unk78 not GawGawMain), or it's beaten: it lets Kirby go as
 * he is (vtable +0x30, 0x020AFE60); */
static void GawGawWatch(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    if (HELD(s) && (obj->unk78 != GawGawMain || obj->unk80 <= 0 || (obj->base.flags & 0x1000))) LetGo(s, sLetGo);
}

void GawGawStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, 0, GawGawMain);
    obj->unk7C = GawGawWatch;
    SqueakThroughTerrain(s, FALSE);
    if (HELD(s)) LetGo(s, sLetGo);
    if (!s->started) {
        s->brain = s->variant == 1 ? BRAIN_POUNCE_START : s->variant == 3 ? BRAIN_STILL_START : BRAIN_HOP_START;
        SqueakSetState(s, BODY_SPAWN);
    } else {
        SqueakSetState(s, BODY_START);
    }
}

/* GawGawInit(); Its body box (gawgaw.rca's 0x201/0x202: 5 pixels either side, 1 to 11 up; here from its feet: KatAM's bounds),
 * facing left (0x020B0068); its hit box and harm set each frame (SqueakSetBoxes()); */
void GawGawInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -12, -19, 10, -1);
    ObjectSetBounds(&s->obj.base, -5, -11, 5, 0);
    HARM(s) = s->obj.base.unk68;
    s->obj.base.flags |= 1;
}
