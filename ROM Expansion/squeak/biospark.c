/* Bio Spark; ported from Kirby: Squeak Squad (USA), enemy 0x25 (its body 0x0207CF18, vtable 0x0216E5A4; brain 0x0207A284; its kunai
 * and leaf 0x0207D770), as SQUEAK_BIO_SPARK, SQUEAK_BIO_SPARK_KUNAI and SQUEAK_BIO_SPARK_LEAF. Values from param/prmCE18Ml0.kgb
 * (movement; MlG differs only in the other mode's jumps), prmCE18Pl0.kgb (brain) and prmCE18Sl0.kgb (HP 20); speeds 20.12 a frame, Y
 * up; its boxes from enemy/biospark.rca (its animations from the .rca's second: biospark.cea's animation k is the .rca's k + 1). A
 * ninja: it hops about, throws kunai, walks at Kirby slashing, hides as a leaf, and jumps to grab him, carries him above the screen and
 * drops him (an izuna drop).
 * • Brain (the spawn's first parameter byte, KatAM's template unk12; prmCE18Pl0.kgb's two halves, the same values): once on the
 *   ground it faces Kirby (past 1 pixel), then
 *   0 (0x0207A1A4) and 1 (0x02079E40, first: up 11 frames, hiding as a leaf, and it waits till Kirby is within 64 pixels across and
 *   48 up or down, 0x02079DDC): Kirby within 64 across and 48 up or down, 20% of the time it presses B (the kunai or, half the time
 *   pressing forward too, the slash: 0x0207A0CC), waits for its animation to end three times (0x0207A0A0, 0x0207A084, 0x0207A068),
 *   faces Kirby and starts over; else it picks (0x02079F8C: random 0-99) down (30), forward (10), forward and down (10), A (30),
 *   forward and A (10), down and A (5), forward, down and A (5), lets go of A next frame and of the rest 4 frames on (0x02079EE8),
 *   and once on the ground (its pad as it was) faces Kirby if he's behind it (after 2 frames) and starts over;
 *   2 (0x02079A88, 0x02079A78): it stands, its pad let go.
 * • Body (0x0207A594 reads the pad: B at once; the rest after it's been pressed 4 frames, a neutral pad starting over): it stands
 *   (anim 1; 2 frames before it reads its pad); down hops up (1.2, anim 23), forward hops forward (1.0 across), forward and down
 *   back; A jumps (3.0 up, anim 2), down and A higher (5.0), forward and A (or forward, down and A) forward too; in the air, forward
 *   keeps it going forward, falling 0.2 a frame no faster than 2.4; up hides it (anim 19, then the leaf, 16) till A or B, when it
 *   comes out (anim 17, then 18, leaving a leaf that drifts back and up at 1.0 and goes, 34 frames, anim 20) and stands after 32
 *   frames;
 *   B throws kunai (anims 12, 16 frames, then 13): three, 16 frames apart, from 16 pixels ahead and 4 up, flying on at 4.0 through
 *   everything (anim 15) till they touch a wall (a star burst, efc_cmn0's animation 7) or leave the stage; then 32 frames (anim 14);
 *   forward and B slashes (anim 3, then 4, its blade reaching 43 pixels ahead): it walks on at 0.5 for 120 frames, then stops (anim
 *   5, 30 frames);
 *   the jump's grab boxes (anim 2: biospark.rca's kind 6) touching Kirby (its touch reaction 0x0207CDD4), it grabs him (anim 6, 12
 *   frames, him 8 pixels ahead of it, 8 up, facing it), goes up through the terrain at 8.0 (anim 7, him level with it) for 32
 *   frames or till it's 16 pixels over the screen's top, hangs there 33 frames (anim 9), then drops at 8.0 through the terrain till
 *   it's under 4 pixels over where it grabbed him; then out of the terrain, the ground or the terrain under it (its feet, either side
 *   of its body box) lands it: Kirby flies away from it at 2.0, 2.8 up, hurt as he lands (Squeak Squad: its hit kind 0x8001, Kirby's
 *   state +0x100: 6 of 36 HP when he lands; here KatAM's throw that hurts on landing, Kirby_110 flag 2, sub_0804F32C: 1 HP), 120
 *   frames untouchable by it, and the screen shakes (Squeak Squad's 12 frames, about 3 pixels up and down: KatAM's shortest,
 *   RequestScreenShake 1, 14 frames). All the while its touch doesn't hurt (its +0x18 bit 2), nor Kirby's body it (KatAM's grabbers').
 * • Boxes (biospark.rca, per animation frame, as uja.c's): the slash's blade (anim 4) hurts 43 pixels ahead; the jump (anim 2) only
 *   grabs, its touch harmless; the leaf it hides as hurts.
 * Kirby's inhale takes it, its kunai and the leaf it hides as as any enemy; a hit lets Kirby go as he is.
 * Not ported: its sounds (0x124 the hops, 0x13A hiding, 0x13B coming out, 0x13C the slash's steps, 0x13D the kunai, 0x13E the grab),
 * its ability (Squeak Squad's Ninja: KatAM has none), the other mode (the brain's +8 not 3: B and A in the air, MlG's jumps), its
 * contact damages (Squeak Squad's 4 for the kunai and the slash: KatAM's 1), and its vtable +0x24 handler (0x0207CCB4: an event 0x15
 * while holding Kirby lets him go hurt; what sends it wasn't found). */
#include "../squeak.h"
#include "main.h"
#include "random.h"

void sub_0804E09C(struct Kirby *);
void RequestScreenShake(u16, struct ObjectBase *);

// Animations (biospark.cea);
enum { ANIM_0, ANIM_STAND, ANIM_JUMP, ANIM_SLASH_READY, ANIM_SLASH, ANIM_SLASH_END, ANIM_GRAB, ANIM_RISE, ANIM_8, ANIM_DROP,
       ANIM_10, ANIM_11, ANIM_THROW_READY, ANIM_THROW, ANIM_THROW_END, ANIM_KUNAI, ANIM_HIDDEN, ANIM_APPEAR, ANIM_APPEARED, ANIM_HIDE,
       ANIM_LEAF, ANIM_HURT, ANIM_HURT_2, ANIM_HOP };

// prmCE18Ml0.kgb;
#define GRAVITY 0x334
#define HOP_SPEED 0x1334
#define JUMP_SPEED 0x3000
#define HIGH_JUMP_SPEED 0x5000
#define AIR_SPEED 0x1000
#define LEAF_SPEED_X 0x1000
#define LEAF_SPEED_Y 0x1000
#define APPEAR_TIME 30
#define SLASH_TIME 120
#define SLASH_SPEED 0x800
#define KUNAI_X 0x10000
#define KUNAI_Y 0x4000
#define KUNAI_SPEED 0x4000
#define THROW_READY_TIME 16
#define THROW_END_TIME 32
#define KUNAI_1 0
#define KUNAI_2 16
#define KUNAI_3 32
#define GRAB_TIME 12
#define RISE_SPEED 0x8000
#define RISE_TIME 32
#define DROP_SPEED 0x8000
#define HANG_TIME 32
// prmCE18Pl0.kgb (each half);
#define NEAR_X 0x40000
#define NEAR_Y 0x30000
#define B_FORWARD_CHANCE 50
#define PICK_DOWN 30
#define PICK_FORWARD 10
#define PICK_FORWARD_DOWN 10
#define PICK_A 30
#define PICK_FORWARD_A 10
#define PICK_DOWN_A 5
// The code's;
#define MAX_FALL -0x2666
#define FACE_DISTANCE 0x1000
#define ATTACK_CHANCE 20
#define HIDE_TIME 10
#define MOVE_TIME 4
#define TURN_WAIT 2
#define STAND_WAIT 2
#define PAD_HOLD 3
#define LEAF_X -0x8000
#define LEAF_Y 0x6000
#define ABOVE_SCREEN 0x10000            // (Squeak Squad's units);
#define LAND_ABOVE 0x400                // Under 4 pixels over where it grabbed him, it lands (KatAM's units);
#define LAND_PROBE 0x200                // Its probes 2 pixels under its feet (0x2000);
#define KIRBY_FEET 7                    // KatAM's Kirby's spot is 7 pixels over his feet (his box's bottom), Squeak Squad's at them;
#define HOLD_X -8                       // Kirby 8 pixels ahead of it (his x: he faces it), 8 up while it grabs;
#define GRAB_UP 8
#define THROW_X -32                     // Squeak Squad's thrown Kirby's -0x2000 (his back) and 0x2CCD up: KatAM's 0x200 and 0x2D0 (x 0x10);
#define THROW_Y 45
#define THROW_GUARD 120                 // The thrown Kirby's 0xEC: 120 frames he and it don't touch;

// Its pad's moves (0x0207A594's 0x1F8);
enum { MOVE_NONE, MOVE_HOP_BACK, MOVE_HOP_UP, MOVE_HOP, MOVE_JUMP, MOVE_JUMP_FORWARD, MOVE_HIGH_JUMP, MOVE_HIGH_JUMP_FORWARD,
       MOVE_HIDE, MOVE_B };
enum {
    BODY_START, BODY_STAND, BODY_HOP_BACK, BODY_HOP_UP, BODY_HOP, BODY_JUMP, BODY_JUMP_FORWARD, BODY_HIGH_JUMP,
    BODY_HIGH_JUMP_FORWARD, BODY_HOP_RISE, BODY_RISE, BODY_JUMP_RISE, BODY_HOP_FALL_ENTRY, BODY_HOP_FALL, BODY_FALL_ENTRY,
    BODY_FALL, BODY_THROW_ENTRY, BODY_THROW_READY, BODY_THROW, BODY_THROW_END, BODY_SLASH_ENTRY, BODY_SLASH_READY, BODY_SLASH,
    BODY_SLASH_END, BODY_HIDE_ENTRY, BODY_HIDE, BODY_HIDDEN, BODY_APPEAR, BODY_APPEARED, BODY_IZUNA_ENTRY, BODY_IZUNA_GRAB,
    BODY_IZUNA_RISE, BODY_IZUNA_HANG, BODY_IZUNA_DROP,
};
enum {
    BRAIN_START, BRAIN_HIDE_START, BRAIN_HIDE, BRAIN_WAKE, BRAIN_STILL_START, BRAIN_STILL, BRAIN_NEAR, BRAIN_PICK, BRAIN_MOVE,
    BRAIN_TURN, BRAIN_TURNED, BRAIN_PRESS_B, BRAIN_RELEASE_B, BRAIN_WAIT, BRAIN_WAIT_2,
};

#define MOVE(s) ((s)->vars[0])          // 0x1F8: the move its pad asks for;
#define PAD_TIME(s) ((s)->vars[1])      // 0x210: frames its pad's been pressed (to 3);
#define WAIT(s) ((s)->vars[2])          // 0x218 (stand), 0x214 (throw, slash), 0x208 (the leaf's, the izuna's);
#define COUNT(s) ((s)->vars[3])         // 0x204: the throw's frames;
#define BACK(s) ((s)->vars[4])          // 0x200: hopping back (its pad forward keeps it going back);
#define HANG(s) ((s)->vars[5])          // 0x20C: the izuna's hang;
#define GRAB_Y(s) ((s)->vars[6])        // 0x1A4: where it grabbed him (KatAM's y);
#define BRAIN_WAIT(s) ((s)->vars[7])    // The brain's 0x98;
#define HARM(s) ((s)->vars[8])          // KatAM's unk68 from InitObject: its touch's harm;
#define BOX(s) (*(u8 *)&(s)->vars[9])   // This frame's box: sBioBoxes' index;
#define HELD(s) (*(struct Kirby **)&(s)->vars[10])  // 0x164;
#define GUARD(s) ((s)->vars[11])        // Frames left of THROW_GUARD;

/* Its boxes (biospark.rca: its animations' frames, each one's tags and boxes, as KatAM's; boxes.py, by the notes' rca layout: the
 * .rca's animation k + 1 is the .cea's k); */
static const struct SqueakBox sBioBoxes[] = {
    { { -12, -21, 10, -1 }, TRUE, 0 },
    { { -12, -21, 10, -1 }, FALSE, 2, { { -6, -17, 4, -4 }, { 4, -17, 9, -2 } } },
    { { -12, -22, 43, 0 }, TRUE, 0 },
    { { -12, -22, 10, -1 }, TRUE, 0 },
    { { -9, -7, 9, 0 }, TRUE, 0 },
    { { -9, -18, 9, 0 }, TRUE, 0 },
    { { -6, -16, 6, -4 }, TRUE, 0 },
    { { -12, -21, 10, -1 }, FALSE, 0 },
};
static const struct SqueakBoxFrame sBioAnim0[] = { { 1, 0 } };
static const struct SqueakBoxFrame sBioAnim1[] = { { 1, 0 } };
static const struct SqueakBoxFrame sBioAnim2[] = { { 1, 1 } };
static const struct SqueakBoxFrame sBioAnim3[] = { { 16, 0 } };
static const struct SqueakBoxFrame sBioAnim4[] = { { 6, 2 } };
static const struct SqueakBoxFrame sBioAnim5[] = { { 30, 0 } };
static const struct SqueakBoxFrame sBioAnim6[] = { { 1, 0 } };
static const struct SqueakBoxFrame sBioAnim7[] = { { 1, 0 } };
static const struct SqueakBoxFrame sBioAnim8[] = { { 1, 0 } };
static const struct SqueakBoxFrame sBioAnim9[] = { { 1, 3 } };
static const struct SqueakBoxFrame sBioAnim10[] = { { 1, 3 } };
static const struct SqueakBoxFrame sBioAnim11[] = { { 1, 0 } };
static const struct SqueakBoxFrame sBioAnim12[] = { { 16, 0 } };
static const struct SqueakBoxFrame sBioAnim13[] = { { 40, 0 } };
static const struct SqueakBoxFrame sBioAnim14[] = { { 32, 0 } };
static const struct SqueakBoxFrame sBioAnim15[] = { { 1, 4 } };
static const struct SqueakBoxFrame sBioAnim16[] = { { 1, 5 }, { 3, 5 } };
static const struct SqueakBoxFrame sBioAnim17[] = { { 6, 5 } };
static const struct SqueakBoxFrame sBioAnim18[] = { { 1, 0 } };
static const struct SqueakBoxFrame sBioAnim19[] = { { 1, 5 }, { 6, 6 } };
static const struct SqueakBoxFrame sBioAnim21[] = { { 1, 7 } };
static const struct SqueakBoxFrame sBioAnim22[] = { { 1, 7 } };
static const struct SqueakBoxFrame sBioAnim23[] = { { 1, 0 } };
static const struct SqueakBoxAnim sBioBoxAnims[] = {
    { sBioAnim0, 1 },
    { sBioAnim1, 1 },
    { sBioAnim2, 1 },
    { sBioAnim3, 1 },
    { sBioAnim4, 1 },
    { sBioAnim5, 1 },
    { sBioAnim6, 1 },
    { sBioAnim7, 1 },
    { sBioAnim8, 1 },
    { sBioAnim9, 1 },
    { sBioAnim10, 1 },
    { sBioAnim11, 1 },
    { sBioAnim12, 1 },
    { sBioAnim13, 1 },
    { sBioAnim14, 1 },
    { sBioAnim15, 1 },
    { sBioAnim16, 2 },
    { sBioAnim17, 1 },
    { sBioAnim18, 1 },
    { sBioAnim19, 2 },
    { NULL, 0 },
    { sBioAnim21, 1 },
    { sBioAnim22, 1 },
    { sBioAnim23, 1 },
};
static const struct SqueakBoxes sBioBoxData = { sBioBoxes, sBioBoxAnims, 24 };

/* Kirby held (kirby->unk110), facing it (KatAM's hold faces him as it: turned round): 8 pixels ahead of it, 8 up while it grabs him,
 * level with it while it carries him; in KatAM's grabbers' pose (0x57, Mr. Frosty's, Box Boxer's), behind it; each followed by an
 * end the hold never reaches; */
static const struct Kirby_110 sGrab[] = { { HOLD_X, -KIRBY_FEET - GRAB_UP, -1, 0xFF, 0x57, 0x400 }, { 0, 0, 0, 1, 0, 0x400 } };
static const struct Kirby_110 sCarry[] = { { HOLD_X, -KIRBY_FEET, -1, 0xFF, 0x57, 0x400 }, { 0, 0, 0, 1, 0, 0x400 } };
/* Thrown away from it hurt when he lands (flag 2: sub_0804F32C, at (x, y) x 0x10 along his facing), or let go as he is (flag 1); with
 * 0x400 too, or KatAM's hold lets him go its own way first (see maiga.c's); */
static const struct Kirby_110 sThrow[] = { { THROW_X, THROW_Y, -1, 1, 0, 0x402 }, { 0, 0, 0, 1, 0, 0 } };
static const struct Kirby_110 sLetGo[] = { { 0, 0, -1, 1, 0, 0x401 }, { 0, 0, 0, 1, 0, 0 } };

static void BioSparkMain(struct Object *obj);

// Random(); Squeak Squad's random 0-99 (KatAM's numbers);
static u32 Random(void) {
    return Rand16() * 100 >> 16;
}

// KirbyBehind(); 0x02079EE8's: Kirby (any way across) behind it;
static bool32 KirbyBehind(struct SqueakObject *s) {
    s32 kirby = SQ_X(s->obj.kirby3), x = SQ_X(&s->obj);

    return s->obj.base.flags & 1 ? kirby > x : kirby < x;
}

/* ReadPad(); 0x0207A594: B asks for its move at once; the rest once the pad's been pressed 4 frames running (PAD_TIME to 3): A (with
 * down: the high jumps; forward: forward), up (hiding), forward (with down: back), down; a neutral pad starts the count over; */
static void ReadPad(struct SqueakObject *s) {
    if (s->padB) {
        if (MOVE(s) < MOVE_B) MOVE(s) = MOVE_B;
        return;
    }
    if (PAD_TIME(s) < PAD_HOLD) {
        PAD_TIME(s)++;
    } else if (s->padA) {
        if (s->padY >= 0) MOVE(s) = s->padDir ? MOVE_JUMP_FORWARD : MOVE_JUMP;
        else MOVE(s) = s->padDir ? MOVE_HIGH_JUMP_FORWARD : MOVE_HIGH_JUMP;
        return;
    } else if (s->padY > 0) {
        MOVE(s) = MOVE_HIDE;
        return;
    } else if (s->padDir) {
        MOVE(s) = s->padY == 0 ? MOVE_HOP : MOVE_HOP_BACK;
        return;
    } else if (s->padY < 0) {
        MOVE(s) = MOVE_HOP_UP;
    } else if (MOVE(s) < MOVE_HOP) {
        MOVE(s) = MOVE_NONE;
    }
    if (!s->padDir && !s->padY && !s->padA && !s->padB) PAD_TIME(s) = 0;
}

// StartMove(); 0x0207A3FC: the move its pad asked for (1-8) starts;
static bool32 StartMove(struct SqueakObject *s) {
    static const u8 sMoves[] = { 0, BODY_HOP_BACK, BODY_HOP_UP, BODY_HOP, BODY_JUMP, BODY_JUMP_FORWARD, BODY_HIGH_JUMP,
                                 BODY_HIGH_JUMP_FORWARD, BODY_HIDE_ENTRY };

    if (MOVE(s) < MOVE_HOP_BACK || MOVE(s) > MOVE_HIDE) return FALSE;
    SqueakSetState(s, sMoves[MOVE(s)]);
    MOVE(s) = MOVE_NONE;
    return TRUE;
}

// PressedB(); 0x0207A718: B throws kunai; with forward, the slash;
static bool32 PressedB(struct SqueakObject *s) {
    if (!s->padB) return FALSE;
    SqueakSetState(s, s->padDir < 1 ? BODY_THROW_ENTRY : BODY_SLASH_ENTRY);
    return TRUE;
}

// AirControl(); 0x0207CC4C: forward, it goes forward at AIR_SPEED (hopping back, back);
static void AirControl(struct SqueakObject *s) {
    if (s->padDir > 0) s->vx = BACK(s) ? -AIR_SPEED : AIR_SPEED;
}

// Gravity(); 0x0207A668: on the ground, still; else falling GRAVITY a frame, no faster than MAX_FALL;
static void Gravity(struct SqueakObject *s) {
    if (SqueakOnGround(s)) {
        s->vy = s->ay = 0;
    } else if (!SqueakCapFall(s, MAX_FALL)) {
        s->ay = -GRAVITY;
    }
}

/* Leap(); The hops' and jumps' start (0x0207C18C...0x0207C970): its animation (Squeak Squad marks it done at once, its flag 8), no
 * caps, accelerations nor friction (`still`: no speed either), up at `vy`, falling GRAVITY a frame; then `next`; */
static bool32 Leap(struct SqueakObject *s, u8 anim, bool8 still, s32 vx, s32 vy, bool8 back, u8 next) {
    SqueakPlay(s, anim);
    if (still) SqueakStop(s);
    s->maxVx = s->maxVy = s->ax = s->fx = s->fy = 0;
    if (!still) s->vx = vx;
    s->vy = vy;
    s->ay = -GRAVITY;
    s->animEnded = TRUE;
    BACK(s) = back;
    MOVE(s) = MOVE_NONE;
    SqueakSetState(s, next);
    return TRUE;
}

// FallStart(); 0x0207BCC4, 0x0207BEA0: falling (`anim`), its speeds as they were, no caps nor friction;
static bool32 FallStart(struct SqueakObject *s, u8 anim, u8 next) {
    SqueakPlay(s, anim);
    s->maxVx = s->maxVy = s->ax = s->fx = s->fy = 0;
    s->ay = -GRAVITY;
    SqueakSetState(s, next);
    return TRUE;
}

// Landing(); Its state done: standing (its start) on the ground, else falling;
static bool32 Landing(struct SqueakObject *s) {
    SqueakSetState(s, SqueakOnGround(s) ? BODY_START : BODY_FALL_ENTRY);
    return TRUE;
}

// StillHeld(); The Kirby it holds is still held by it (KatAM's hold lets him go on its own when it's beaten);
static bool32 StillHeld(struct SqueakObject *s) {
    struct Kirby *kirby = HELD(s);

    return kirby && kirby->base.unk6C == &s->obj.base && (kirby->unk110 == sGrab || kirby->unk110 == sCarry);
}

// LetGo(); Kirby let go: thrown away hurt (sThrow) or as he is (sLetGo), the table started over;
static void LetGo(struct SqueakObject *s, const struct Kirby_110 *how) {
    struct Kirby *kirby = HELD(s);

    if (StillHeld(s)) {
        kirby->unk110 = how;
        kirby->base.counter = -1;
    }
    HELD(s) = NULL;
    s->obj.base.unk6C = NULL;
}

// Carry(); Kirby held as `how` (if it still holds him);
static void Carry(struct SqueakObject *s, const struct Kirby_110 *how) {
    if (StillHeld(s)) HELD(s)->unk110 = how;
}

/* Grab(); 0x0207CDD4: a Kirby touched its grab boxes: it holds him (KatAM's hold, sub_0804E09C, which faces him as it: then turned to
 * face it), stops, and its izuna starts; */
static void Grab(struct SqueakObject *s, struct Kirby *kirby) {
    HELD(s) = kirby;
    kirby->base.unk6C = &s->obj.base;
    kirby->unk110 = sGrab;
    s->obj.base.unk6C = kirby;
    kirby->base.flags |= 0x40000;
    sub_0804E09C(kirby);
    kirby->base.flags ^= 1;
    SqueakStop(s);
    SqueakSetState(s, BODY_IZUNA_ENTRY);
}

// Landed(); 0x0207A784's ground: the ground under it, or the terrain at its feet (the middle, either side of its box) within 2 pixels;
static bool32 Landed(struct SqueakObject *s) {
    struct ObjectBase *b = &s->obj.base;
    s32 y = b->y + LAND_PROBE;

    return SqueakOnGround(s) || NidlIsFloor(&s->obj, b->x, y) || NidlIsFloor(&s->obj, b->x + b->unk3C * 0x100, y)
           || NidlIsFloor(&s->obj, b->x + b->unk3E * 0x100, y);
}

static bool32 Body(struct SqueakObject *s) {
    s32 t;

    switch (s->state) {
    case BODY_START:        // 0x0207CB88 (vtable +0x3C);
        SqueakPlay(s, ANIM_STAND);
        SqueakStop(s);
        WAIT(s) = STAND_WAIT;
        SqueakSetState(s, BODY_STAND);
        return TRUE;
    case BODY_STAND:        // 0x0207CAD4;
        t = WAIT(s)--;
        if (t < 1) {
            ReadPad(s);
            if (PressedB(s) || StartMove(s)) return TRUE;
        }
        if (s->padDir < 0) SqueakTurnAround(s);
        SqueakMove(s);
        if (!SqueakOnGround(s)) SqueakSetState(s, BODY_FALL_ENTRY);
        break;
    case BODY_HOP_BACK:     // 0x0207C6AC;
        return Leap(s, ANIM_HOP, FALSE, -AIR_SPEED, HOP_SPEED, TRUE, BODY_RISE);
    case BODY_HOP_UP:       // 0x0207C81C;
        return Leap(s, ANIM_HOP, FALSE, 0, HOP_SPEED, FALSE, BODY_HOP_RISE);
    case BODY_HOP:          // 0x0207C970;
        return Leap(s, ANIM_HOP, FALSE, AIR_SPEED, HOP_SPEED, FALSE, BODY_HOP_RISE);
    case BODY_JUMP:         // 0x0207C588;
        return Leap(s, ANIM_JUMP, TRUE, 0, JUMP_SPEED, FALSE, BODY_JUMP_RISE);
    case BODY_JUMP_FORWARD: // 0x0207C41C;
        return Leap(s, ANIM_JUMP, FALSE, AIR_SPEED, JUMP_SPEED, FALSE, BODY_RISE);
    case BODY_HIGH_JUMP:    // 0x0207C2F8;
        return Leap(s, ANIM_JUMP, TRUE, 0, HIGH_JUMP_SPEED, FALSE, BODY_JUMP_RISE);
    case BODY_HIGH_JUMP_FORWARD:    // 0x0207C18C;
        return Leap(s, ANIM_JUMP, FALSE, AIR_SPEED, HIGH_JUMP_SPEED, FALSE, BODY_RISE);
    case BODY_HOP_RISE:     // 0x0207BFAC;
    case BODY_RISE:         // 0x0207C04C;
    case BODY_JUMP_RISE:    // 0x0207C0EC;
        AirControl(s);
        if (s->padDir < 0) SqueakTurnAround(s);
        SqueakMove(s);
        if (s->vy < 0) {
            SqueakSetState(s, s->state == BODY_HOP_RISE ? BODY_HOP_FALL_ENTRY : BODY_FALL_ENTRY);
            return TRUE;
        }
        break;
    case BODY_HOP_FALL_ENTRY:   // 0x0207BCC4;
        return FallStart(s, ANIM_HOP, BODY_HOP_FALL);
    case BODY_FALL_ENTRY:   // 0x0207BEA0;
        return FallStart(s, ANIM_JUMP, BODY_FALL);
    case BODY_HOP_FALL:     // 0x0207BBF4;
    case BODY_FALL:         // 0x0207BDD0;
        ReadPad(s);
        AirControl(s);
        if (s->padDir < 0) SqueakTurnAround(s);
        SqueakCapFall(s, MAX_FALL);
        SqueakMove(s);
        if (SqueakOnGround(s)) {
            SqueakSetState(s, BODY_START);
            SqueakStop(s);
            return TRUE;
        }
        break;

    case BODY_THROW_ENTRY:  // 0x0207B380;
        MOVE(s) = MOVE_NONE;
        SqueakPlay(s, ANIM_THROW_READY);
        SqueakStop(s);
        WAIT(s) = THROW_READY_TIME;
        SqueakSetState(s, BODY_THROW_READY);
        return TRUE;
    case BODY_THROW_READY:  // 0x0207B2A0;
        t = WAIT(s)--;
        if (t < 1) {
            SqueakPlay(s, ANIM_THROW);
            COUNT(s) = 0;
            SqueakSetState(s, BODY_THROW);
            return TRUE;
        }
        Gravity(s);
        SqueakMove(s);
        break;
    case BODY_THROW:        // 0x0207B154: a kunai at its frames 0, 16 and 32 (0x0207A398, 0x0207D848);
        if (s->animEnded) {
            WAIT(s) = THROW_END_TIME;
            SqueakPlay(s, ANIM_THROW_END);
            SqueakSetState(s, BODY_THROW_END);
            return TRUE;
        }
        if (COUNT(s) == KUNAI_1 || COUNT(s) == KUNAI_2 || COUNT(s) == KUNAI_3)
            SqueakSpawn(s, SQUEAK_BIO_SPARK_KUNAI, 0, KUNAI_X, KUNAI_Y);
        COUNT(s)++;
        Gravity(s);
        SqueakMove(s);
        break;
    case BODY_THROW_END:    // 0x0207B0B0;
        t = WAIT(s)--;
        if (t < 1) return Landing(s);
        Gravity(s);
        SqueakMove(s);
        break;
    case BODY_SLASH_ENTRY:  // 0x0207B718;
        MOVE(s) = MOVE_NONE;
        SqueakPlay(s, ANIM_SLASH_READY);
        SqueakStop(s);
        SqueakSetState(s, BODY_SLASH_READY);
        return TRUE;
    case BODY_SLASH_READY:  // 0x0207B604;
        if (s->animEnded) {
            SqueakPlay(s, ANIM_SLASH);
            WAIT(s) = SLASH_TIME;
            s->vx = SLASH_SPEED;
            SqueakSetState(s, BODY_SLASH);
            return TRUE;
        }
        Gravity(s);
        SqueakMove(s);
        break;
    case BODY_SLASH:        // 0x0207B50C;
        if (WAIT(s) < 1) {
            WAIT(s) = 0;
            SqueakPlay(s, ANIM_SLASH_END);
            s->vx = 0;
            SqueakSetState(s, BODY_SLASH_END);
            return TRUE;
        }
        WAIT(s)--;
        Gravity(s);
        SqueakMove(s);
        break;
    case BODY_SLASH_END:    // 0x0207B470;
        if (s->animEnded) return Landing(s);
        Gravity(s);
        SqueakMove(s);
        break;

    case BODY_HIDE_ENTRY:   // 0x0207BB3C;
        MOVE(s) = MOVE_NONE;
        SqueakPlay(s, ANIM_HIDE);
        SqueakStop(s);
        SqueakSetState(s, BODY_HIDE);
        return TRUE;
    case BODY_HIDE:         // 0x0207BA4C;
        if (s->animEnded) {
            SqueakPlay(s, ANIM_HIDDEN);
            SqueakSetState(s, BODY_HIDDEN);
            return TRUE;
        }
        if (s->padDir < 0) SqueakTurnAround(s);
        Gravity(s);
        SqueakMove(s);
        break;
    case BODY_HIDDEN:       // 0x0207B968: till A or B;
        if (s->padA || s->padB) {
            SqueakPlay(s, ANIM_APPEAR);
            SqueakSetState(s, BODY_APPEAR);
            return TRUE;
        }
        if (s->padDir < 0) SqueakTurnAround(s);
        Gravity(s);
        SqueakMove(s);
        break;
    case BODY_APPEAR:       // 0x0207B854: out, it leaves the leaf (0x0207A398, 8 pixels behind it, 6 up);
        if (s->animEnded) {
            SqueakPlay(s, ANIM_APPEARED);
            WAIT(s) = APPEAR_TIME;
            SqueakSpawn(s, SQUEAK_BIO_SPARK_LEAF, 0, LEAF_X, LEAF_Y);
            SqueakSetState(s, BODY_APPEARED);
            return TRUE;
        }
        Gravity(s);
        SqueakMove(s);
        break;
    case BODY_APPEARED:     // 0x0207B7D8;
        t = WAIT(s)--;
        if (t < 0) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        Gravity(s);
        SqueakMove(s);
        break;

    case BODY_IZUNA_ENTRY:  // 0x0207AF94: through the terrain (as each of the izuna's states);
        MOVE(s) = MOVE_NONE;
        SqueakPlay(s, ANIM_GRAB);
        SqueakThroughTerrain(s, TRUE);
        SqueakStop(s);
        WAIT(s) = GRAB_TIME;
        GRAB_Y(s) = s->obj.base.y;
        SqueakSetState(s, BODY_IZUNA_GRAB);
        return TRUE;
    case BODY_IZUNA_GRAB:   // 0x0207AE00;
        t = WAIT(s)--;
        if (t < 1) {
            SqueakPlay(s, ANIM_RISE);
            s->vy = RISE_SPEED;
            WAIT(s) = RISE_TIME;
            SqueakSetState(s, BODY_IZUNA_RISE);
            return TRUE;
        }
        Carry(s, sGrab);
        SqueakMove(s);
        break;
    case BODY_IZUNA_RISE:   // 0x0207AB4C: 16 pixels over the screen's top (its camera's: 0x0220A3E4), or its time up, it hangs;
        {
            s32 top = gCurLevelInfo[s->obj.base.unk56].viewportPosition.y - SQUEAK_SPEED(ABOVE_SCREEN);

            t = WAIT(s)--;
            if (s->obj.base.y < top || t < 1) {
                if (s->obj.base.y < top) {
                    s->obj.base.y = top;
                    WAIT(s) = t;
                }
                s->vy = -DROP_SPEED;
                WAIT(s) = RISE_TIME - WAIT(s) - 1;
                HANG(s) = HANG_TIME;
                SqueakPlay(s, ANIM_DROP);
                SqueakSetState(s, BODY_IZUNA_HANG);
                return TRUE;
            }
        }
        Carry(s, sCarry);
        SqueakMove(s);
        break;
    case BODY_IZUNA_HANG:   // 0x0207AA34: still (it doesn't move), its time up it drops;
        Carry(s, sCarry);
        t = HANG(s)--;
        if (t < 1) {
            SqueakStop(s);
            s->vy = -DROP_SPEED;
            SqueakSetState(s, BODY_IZUNA_DROP);
        }
        break;
    case BODY_IZUNA_DROP:   // 0x0207A784: under 4 pixels over where it grabbed him, out of the terrain; landed, it throws him;
        Carry(s, sCarry);
        if (s->obj.base.y > GRAB_Y(s) - LAND_ABOVE) {
            if (SqueakOnGround(s)) s->vy = s->ay = 0;
            SqueakThroughTerrain(s, FALSE);
            if (Landed(s)) {
                SqueakSetState(s, BODY_START);
                if (HELD(s)) {
                    LetGo(s, sThrow);
                    GUARD(s) = THROW_GUARD;
                }
                RequestScreenShake(1, &s->obj.base);
            }
        } else {
            s->vy = -DROP_SPEED;
            s->ay = 0;
        }
        SqueakMove(s);
        break;
    }
    return FALSE;
}

/* Brain(); Its pad (0x0207A1A4, 0x02079E40, 0x02079A88 and the states they lead to: prmCE18Pl0.kgb's halves, the same values, as one
 * here); */
static void Brain(struct SqueakObject *s) {
    u32 pick;

    switch (s->brain) {
    case BRAIN_START:       // 0x0207A1A4;
        if (!SqueakOnGround(s)) break;
        SqueakFaceKirby(s, FACE_DISTANCE);
        s->brain = BRAIN_NEAR;
        break;
    case BRAIN_HIDE_START:  // 0x02079E40: up (hiding) 11 frames;
        if (!SqueakOnGround(s)) break;
        s->padDir = 0;
        s->padY = 1;
        SqueakFaceKirby(s, FACE_DISTANCE);
        BRAIN_WAIT(s) = HIDE_TIME;
        s->brain = BRAIN_HIDE;
        break;
    case BRAIN_HIDE:        // 0x02079E08;
        s->padDir = 0;
        if (BRAIN_WAIT(s)-- < 1) {
            s->padY = 0;
            s->brain = BRAIN_WAKE;
        }
        break;
    case BRAIN_WAKE:        // 0x02079DDC: Kirby near;
        s->padDir = s->padY = 0;
        if (SqueakKirbyWithin(s, NEAR_X, NEAR_Y)) s->brain = BRAIN_NEAR;
        break;
    case BRAIN_STILL_START: // 0x02079A88;
        if (!SqueakOnGround(s)) break;
        SqueakFaceKirby(s, FACE_DISTANCE);
        s->brain = BRAIN_STILL;
        break;
    case BRAIN_STILL:       // 0x02079A78;
        s->padDir = s->padY = 0;
        break;
    case BRAIN_NEAR:        // 0x0207A124, 0x02079D5C;
        s->brain = SqueakKirbyWithin(s, NEAR_X, NEAR_Y) && Random() < ATTACK_CHANCE ? BRAIN_PRESS_B : BRAIN_PICK;
        break;
    case BRAIN_PICK:        // 0x02079F8C, 0x02079BC4;
        pick = Random();
        if (pick < PICK_DOWN) {
            s->padDir = 0;
            s->padY = -1;
        } else if (pick < PICK_DOWN + PICK_FORWARD) {
            s->padDir = 1;
            s->padY = 0;
        } else if (pick < PICK_DOWN + PICK_FORWARD + PICK_FORWARD_DOWN) {
            s->padDir = 1;
            s->padY = -1;
        } else {
            pick -= PICK_DOWN + PICK_FORWARD + PICK_FORWARD_DOWN;
            s->padDir = pick < PICK_A ? 0 : pick < PICK_A + PICK_FORWARD_A ? 1 : pick < PICK_A + PICK_FORWARD_A + PICK_DOWN_A ? 0 : 1;
            s->padY = pick < PICK_A + PICK_FORWARD_A ? 0 : -1;
            s->padA = 1;
        }
        BRAIN_WAIT(s) = MOVE_TIME;
        s->brain = BRAIN_MOVE;
        break;
    case BRAIN_MOVE:        // 0x02079EE8, 0x02079B20: A let go at once, the rest after MOVE_TIME frames; on the ground, Kirby behind
                            // it, it faces him;
        s->padA = 0;
        if (BRAIN_WAIT(s) < 1) {
            BRAIN_WAIT(s) = 0;
            s->padDir = s->padY = 0;
        } else {
            BRAIN_WAIT(s)--;
        }
        if (SqueakOnGround(s)) {
            BRAIN_WAIT(s) = TURN_WAIT;
            s->brain = KirbyBehind(s) ? BRAIN_TURN : BRAIN_NEAR;
        }
        break;
    case BRAIN_TURN:        // 0x02079EAC, 0x02079AE4;
        if (BRAIN_WAIT(s) < 1) {
            BRAIN_WAIT(s) = 0;
            SqueakFaceKirby(s, FACE_DISTANCE);
            s->brain = BRAIN_TURNED;
        } else {
            BRAIN_WAIT(s)--;
        }
        break;
    case BRAIN_TURNED:      // 0x02079E78, 0x02079AB0;
        if (BRAIN_WAIT(s) < 1) {
            BRAIN_WAIT(s) = 0;
            s->padDir = s->padY = 0;
            s->brain = BRAIN_NEAR;
        } else {
            BRAIN_WAIT(s)--;
        }
        break;
    case BRAIN_PRESS_B:     // 0x0207A0CC, 0x02079D04: half the time forward too (the slash);
        s->padB = 1;
        if (Random() < B_FORWARD_CHANCE) {
            s->padDir = 1;
            s->padY = 0;
        }
        s->brain = BRAIN_RELEASE_B;
        break;
    case BRAIN_RELEASE_B:   // 0x0207A0A0, 0x02079CD8;
        s->padDir = s->padY = 0;
        s->padB = 0;
        if (s->animEnded) s->brain = BRAIN_WAIT;
        break;
    case BRAIN_WAIT:        // 0x0207A084, 0x02079CBC;
        if (s->animEnded) s->brain = BRAIN_WAIT_2;
        break;
    case BRAIN_WAIT_2:      // 0x0207A068, 0x02079CA0;
        if (s->animEnded) s->brain = BRAIN_TURN;
        break;
    }
}

static void BioSparkMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;
    struct Kirby *kirby;
    bool32 izuna;

    Brain(s);
    SqueakRun(s, Body);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
    // (from its grab to its landing, its touch doesn't hurt: its +0x18 bit 2);
    izuna = s->state >= BODY_IZUNA_ENTRY;
    SqueakSetBoxes(s, &sBioBoxData, izuna ? 0 : HARM(s), &BOX(s));
    if (izuna) obj->base.unk68 = 0;
    kirby = obj->unk80 > 0 && !izuna ? SqueakGrabbing(s, &sBioBoxes[BOX(s)]) : NULL;
    if (kirby) Grab(s, kirby);
    /* Holding a Kirby, his body (KatAM's attack kind 0x80, its touch) doesn't hurt it (as KatAM's mini-bosses that hold him); nor
     * while it slashes (its blade, KatAM's one box with its body, only hurts him there, as on the DS); the Kirby it threw is
     * untouchable THROW_GUARD frames (Squeak Squad's 0xEC): it's untouchable then (KatAM's 0x200, as Uja's); */
    if (HELD(s) || s->state == BODY_SLASH) obj->base.unk5C |= 0x80;
    else obj->base.unk5C &= ~0x80;
    if (GUARD(s) > 0) GUARD(s)--;
    if (GUARD(s) > 0) obj->base.flags |= 0x200;
    else obj->base.flags &= ~0x200;
}

/* BioSparkWatch(); Its pre-callback (obj->unk7C: every frame, hit or beaten too): KatAM's hit put it out of its own state (unk78 not
 * BioSparkMain), or it's beaten: it lets Kirby go as he is (0x0207CD78); */
static void BioSparkWatch(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    if (HELD(s) && (obj->unk78 != BioSparkMain || obj->unk80 <= 0 || (obj->base.flags & 0x1000))) LetGo(s, sLetGo);
}

void BioSparkStart(struct Object *obj) {
    static const u8 sBrains[] = { BRAIN_START, BRAIN_HIDE_START, BRAIN_STILL_START };
    struct SqueakObject *s = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, 0, BioSparkMain);
    obj->unk7C = BioSparkWatch;
    SqueakThroughTerrain(s, FALSE);
    if (!s->started) s->brain = sBrains[s->variant < 3 ? s->variant : 0];
    if (HELD(s)) LetGo(s, sLetGo);
    SqueakSetState(s, BODY_START);
}

/* BioSparkInit(); Its body box (biospark.rca's 0x201/0x202: 6 pixels either side, 2 to 14 up; here from its feet: KatAM's bounds),
 * facing left (0x0207CF18); its hit box and harm set each frame (SqueakSetBoxes()); */
void BioSparkInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -12, -21, 10, -1);
    ObjectSetBounds(&s->obj.base, -6, -14, 6, 0);
    HARM(s) = s->obj.base.unk68;
    s->obj.base.flags |= 1;
    SqueakStandOnFloor(&s->obj);
}

/* Its kunai and leaf (0x0207D770: kind 0 the kunai, 1 the leaf; their own move, through everything, the stage's edges not holding them,
 * Squeak Squad's flags 0x200 and 0x20000); */
static void Drift(struct SqueakObject *s) {
    struct ObjectBase *b = &s->obj.base;

    b->unk48 = b->x;
    b->unk4C = b->y;
    b->x += SQUEAK_SPEED(b->flags & 1 ? -s->vx : s->vx);
    b->y -= SQUEAK_SPEED(s->vy);
    b->xspeed = b->yspeed = 0;
}

/* The kunai (0x0207D490, 0x0207D414, 0x0207D350, 0x0207D338): flies on at KUNAI_SPEED; touching a wall (0x0201355C's flag 0x400) or out
 * of the stage, it shows a star burst (0x020B48E0's type 3) and is gone at its animation's end; */
static bool32 Kunai(struct SqueakObject *s) {
    switch (s->state) {
    case 0:
        s->vx = KUNAI_SPEED;
        SqueakPlay(s, ANIM_KUNAI);
        SqueakSetState(s, 1);
        return TRUE;
    case 1:
        Drift(s);
        if (SqueakTouches(s, SQUEAK_LEFT, 0, FALSE) || SqueakTouches(s, SQUEAK_RIGHT, 0, FALSE) || !SqueakInLevel(s)) {
            SqueakPlay(s, ANIM_KUNAI);
            s->obj.base.unk68 = 0;
            s->obj.base.flags |= 0x200;
            SqueakSpawnEffect(s, SQUEAK_FX_STARS, 0, 0, FALSE);
            SqueakSetState(s, 2);
            return TRUE;
        }
        break;
    case 2:
        if (s->animEnded) s->obj.base.flags |= 0x1000;
        break;
    }
    return FALSE;
}

static void BioSparkKunaiMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    SqueakRun(s, Kunai);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
}

void BioSparkKunaiStart(struct Object *obj) {
    ObjectSetFunc(obj, 0, BioSparkKunaiMain);
    SqueakThroughTerrain((struct SqueakObject *)obj, TRUE);
    SqueakSetState((struct SqueakObject *)obj, 0);
}

// BioSparkKunaiInit(); Its box (biospark.rca's: 9 pixels either side, 0 to 7 up);
void BioSparkKunaiInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -9, -7, 9, 0);
    ObjectSetBounds(&s->obj.base, -9, -7, 9, 0);
}

/* The leaf (0x0207D27C, 0x0207D25C): drifts back and up (LEAF_SPEED_X, LEAF_SPEED_Y) till its animation's end; it has no boxes (it
 * touches nothing: KatAM's 0x200, no harm); */
static bool32 Leaf(struct SqueakObject *s) {
    switch (s->state) {
    case 0:
        s->vx = -LEAF_SPEED_X;
        s->vy = LEAF_SPEED_Y;
        SqueakPlay(s, ANIM_LEAF);
        SqueakSetState(s, 1);
        return TRUE;
    case 1:
        Drift(s);
        if (s->animEnded) s->obj.base.flags |= 0x1000;
        break;
    }
    return FALSE;
}

static void BioSparkLeafMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    SqueakRun(s, Leaf);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
}

void BioSparkLeafStart(struct Object *obj) {
    ObjectSetFunc(obj, 0, BioSparkLeafMain);
    SqueakThroughTerrain((struct SqueakObject *)obj, TRUE);
    obj->base.flags |= 0x200;
    obj->base.unk68 = 0;
    SqueakSetState((struct SqueakObject *)obj, 0);
}

void BioSparkLeafInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -4, -8, 4, 0);
    ObjectSetBounds(&s->obj.base, -4, -8, 4, 0);
}
