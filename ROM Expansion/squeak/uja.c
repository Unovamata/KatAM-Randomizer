/* Uja; ported from Kirby: Squeak Squad (USA), enemy 0x11 (its body 0x020840C8, vtable 0x0216ED6C; brain 0x02080AD0), as SQUEAK_UJA.
 * Values from param/prmNE18Ml0.kgb (movement; MlG the same), prmNE18Pl0.kgb (brain) and prmNE18Sl0.kgb (HP 40); speeds 20.12 a
 * frame, Y up; its boxes from enemy/uja.rca. A dark puff that floats and bobs; near Kirby it turns into a sword that lunges at him,
 * or a glove that grabs him, smothers him in the puff and throws him away hurt.
 * • Brain (the spawn's first parameter byte, KatAM's template unk12; each first turns to face Kirby, past 1 pixel, then gives its body
 *   600 frames before it turns round and flies away: pad up/down 5):
 *   1 (0x020807E8): presses A (the sword) and goes at Kirby (0x02080610): across when he's more than 20 pixels away, up or down when
 *     more than 20 pixels above or below; with him ahead within 48 pixels across and 12 up or down, or within 48 up or down and 12
 *     across, it stops and presses B one frame (a lunge), then goes on;
 *   any other (0x020809F8): presses B (the glove) and goes at Kirby (0x02080854) when he's more than 8 pixels away, either way.
 * • Body (every state sets its flag 0x200 through vtable +0x40, 0x02080DE0: it passes through the terrain, held in the stage):
 *   the puff (0x0208112C, 0x02081004; anim 12) floats: its pad turns it round, its speeds slow by 0xA4 a frame (0x02080C80), it
 *   bobs (0x02080BD8: 0.1 up for 20 frames, 0.1 down for 20, a frame still; 0x19A + 0x67 a frame); A makes the sword, B the glove;
 *   the sword or glove (anims 8 and 9, the puff shrinking, then 5 or 2) floats at Kirby: forward speeds it up by 0xA4 a frame to 0.8
 *   (0x02080D44), up or down too; pressed up or down with no direction it turns its point or fingers that way (anims 8, 11, then 6
 *   or 3 up, 7 or 4 down), back with a direction (anims 10 then 9); turning round, it shrinks and grows back facing the other way;
 *   after 600 frames (from the form's start or its last turn round) it's a puff again (anim 8 or 10, then the puff's start);
 *   B lunges the sword (anims 17, 18 up, 19 down: it thrusts out 32 pixels and back, 41 frames);
 *   the glove grabs a Kirby who touches its fingers (its touch reaction 0x02083F2C, kind 6: uja.rca's boxes of that kind in anims 2,
 *   3 and 4): it's where he is, closes on him (anim 13, 14 or 15 as it pointed, holding him 4 pixels up) and smothers him in the
 *   puff (anim 25) 120 frames, 5 fewer each frame his pad changes (stick across or up and down, A, B); then it throws him back
 *   (Squeak Squad: 4.6 a frame, 6 of 36 HP at once, 120 frames he's untouchable; here KatAM's hurt that knocks him back at 1.7,
 *   Squeak Squad's own hurt's speed: Kirby_110 flag 0x10, sub_0804EA18, 1 HP, his ability lost as any KatAM hit does), turns round
 *   and flies away;
 *   flying away (0x02080F04, pad up/down 5; anim 12): 1.2 forward, rising faster and faster (0x334 a frame) to 2.0;
 *   pad up/down 6 (0x02083E10, no brain of Uja's asks for it): the puff grows from nothing (anim 1), then starts over.
 * • Boxes (uja.rca, per animation frame: LoadBoxes 0x02014010): KatAM's one hit box (sub_0803E2B0's: the hits it takes, the hurt it
 *   does by touch) is each frame's box that's hit (tag 0x102) and the kind 0 boxes that hurt (0x101) in one; but the glove's frames, whose fingers grab (kind 6 boxes), take the kind 0 box alone, so its fingers reach Kirby before
 *   its touch hurts him (as on the DS; it's hit there only); with no kind 0 box (anim 4, the glove pointing down, all fingers)
 *   it does no harm (unk68 0, as KatAM's Blockin).
 * Kirby's inhale takes it as any enemy (on the DS too: pulled in, it's swallowed as a Waddle Dee).
 * Not ported: its sounds (0x136 the puff and glove, 0x137 the sword), its other mode (the brain's +8 not 3: flying away or done
 * holding Kirby, it's a puff again), its contact damages (4, 6 for the sword, 9 for its tip at full stretch, of 36: KatAM's 1). */
#include "../squeak.h"
#include "main.h"

void sub_0804E09C(struct Kirby *);

// Animations (uja.cea);
enum { ANIM_STILL, ANIM_GROW, ANIM_GLOVE, ANIM_GLOVE_UP, ANIM_GLOVE_DOWN, ANIM_SWORD, ANIM_SWORD_UP, ANIM_SWORD_DOWN, ANIM_SHRINK,
       ANIM_SHRINK_2, ANIM_SHRINK_3, ANIM_TURN, ANIM_PUFF, ANIM_CLOSE, ANIM_CLOSE_UP, ANIM_CLOSE_DOWN, ANIM_16, ANIM_LUNGE,
       ANIM_LUNGE_UP, ANIM_LUNGE_DOWN, ANIM_HURT_GLOVE, ANIM_HURT_GLOVE_2, ANIM_HURT_SWORD, ANIM_HURT_SWORD_2, ANIM_HURT_PUFF,
       ANIM_SMOTHER };

// prmNE18Ml0.kgb;
#define FLOAT_SPEED 0xCCD
#define FLOAT_SLOW 0xA4
#define FLOAT_ACCEL 0xA4
#define BOB_SPEED 0x19A
#define BOB_ACCEL 0x67
#define FLY_SPEED 0x1334
#define FLY_RISE 0x2000
#define FLY_ACCEL 0x334
#define FORM_TIME 600
#define HOLD_TIME 120
/* prmNE18Pl0.kgb: the glove's brain ([0]-[6]), then the sword's ([7]-[13]): how far Kirby is (across and up or down, the same) before
 * it goes at him; the box it stops in with him ahead (across, up or down), and with him above or below; its time; */
#define GLOVE_NEAR 0x8000
#define GLOVE_AHEAD_ACROSS 0x8000
#define GLOVE_AHEAD_UP 0xC000
#define GLOVE_ABOVE_ACROSS 0xC000
#define GLOVE_ABOVE_UP 0x8000
#define GLOVE_TIME 600
#define SWORD_NEAR 0x14000
#define SWORD_AHEAD_ACROSS 0x30000
#define SWORD_AHEAD_UP 0xC000
#define SWORD_ABOVE_ACROSS 0xC000
#define SWORD_ABOVE_UP 0x30000
#define SWORD_TIME 600
// The code's;
#define BOB_TIME 40
#define BOB_HALF 20
#define FACE_DISTANCE 0x1000
#define MASH 5
#define GRAB_UP 4
#define KIRBY_FEET 7                    // KatAM's Kirby's spot is 7 pixels over his feet (his box's bottom), Squeak Squad's at them;
#define THROW_SPEED -27                 // Squeak Squad's hurt Kirby's 0x1B34 back: KatAM's 0x1B0 (x 0x10);
#define THROW_GUARD 120                 // The thrown Kirby's 0xEC: 120 frames he and it don't touch;
#define PAD_FLY 5
#define PAD_GROW 6

enum { HURT_PUFF, HURT_GLOVE, HURT_SWORD };
enum {
    BODY_START, BODY_PUFF, BODY_FLY_ENTRY, BODY_FLY, BODY_GROW_ENTRY, BODY_GROW, BODY_VANISH,
    BODY_SWORD_ENTRY, BODY_SWORD_TURN, BODY_SWORD_DRAW, BODY_SWORD, BODY_SWORD_UP, BODY_SWORD_DOWN, BODY_SWORD_TO_UP,
    BODY_SWORD_TO_UP_2, BODY_SWORD_TO_DOWN, BODY_SWORD_TO_DOWN_2, BODY_LUNGE, BODY_LUNGE_UP, BODY_LUNGE_DOWN,
    BODY_GLOVE_ENTRY, BODY_GLOVE_TURN, BODY_GLOVE_DRAW, BODY_GLOVE, BODY_GLOVE_UP, BODY_GLOVE_DOWN, BODY_GLOVE_TO_UP,
    BODY_GLOVE_TO_UP_2, BODY_GLOVE_TO_DOWN, BODY_GLOVE_TO_DOWN_2, BODY_CLOSE, BODY_SMOTHER,
};
enum {
    BRAIN_GLOVE_FACE, BRAIN_GLOVE_PRESS, BRAIN_GLOVE, BRAIN_SWORD_FACE, BRAIN_SWORD_PRESS, BRAIN_SWORD, BRAIN_LUNGE, BRAIN_LUNGED,
    BRAIN_LEAVE, BRAIN_TURN, BRAIN_GONE,
};

#define TIMER(s) ((s)->vars[0])         // 0x22C: the form's frames left, then the smother's;
#define BASE_VY(s) ((s)->vars[1])       // 0x1FC: its speed up/down under the bob;
#define BOB(s) ((s)->vars[2])           // 0x200;
#define TURNING(s) ((s)->vars[3])       // 0x228: turns round once it's shrunk;
#define POINTS(s) ((s)->vars[4])        // 0x248: the glove points ahead (0), up (1), down (2): how it closes;
#define HURT(s) ((s)->vars[5])          // Which hurt animations its state set (0x02021A88);
#define KEYS(s) ((s)->vars[6])          // 0x238-0x244: the held Kirby's pad last frame;
#define BRAIN_WAIT(s) ((s)->vars[7])    // The brain's 0x9C;
#define HARM(s) ((s)->vars[8])          // KatAM's unk68 from InitObject: its touch's harm;
#define GRABS(s) (*(u8 *)&(s)->vars[9]) // This frame's box: sUjaBoxes' index;
#define HELD(s) (*(struct Kirby **)&(s)->vars[10])  // 0x164;
#define GUARD(s) ((s)->vars[11])        // Frames left of THROW_GUARD;

/* Its boxes (uja.rca: its animations' frames, each one's tags and boxes, as KatAM's; boxes.py, by the notes' rca layout); */
static const struct SqueakBox sUjaBoxes[] = {
    { { -15, -15, -4, -5 }, TRUE, 0 },
    { { -13, -18, -2, -8 }, TRUE, 0 },
    { { -10, -20, 1, -10 }, TRUE, 0 },
    { { -10, -23, 8, -10 }, TRUE, 0 },
    { { -3, -19, 15, -11 }, TRUE, 2, { { 15, -26, 24, -10 }, { 5, -26, 24, -19 } } },
    { { -8, -37, 0, -17 }, TRUE, 2, { { -9, -45, 7, -37 }, { 0, -45, 7, -25 } } },
    { { -12, -28, 13, 13 }, FALSE, 3, { { -5, -21, 3, -1 }, { -6, -1, 10, 7 }, { 3, -13, 10, 7 } } },
    { { -14, -31, 14, -7 }, TRUE, 0 },
    { { -16, -33, 10, -7 }, TRUE, 0 },
    { { -12, -34, 17, -6 }, TRUE, 0 },
    { { -12, -34, 18, -4 }, TRUE, 0 },
    { { -13, -30, 13, -9 }, TRUE, 0 },
    { { -13, -28, 13, -9 }, TRUE, 0 },
    { { -12, -30, 13, -1 }, TRUE, 0 },
    { { -12, -30, 13, -9 }, TRUE, 0 },
    { { -12, -32, 27, -8 }, TRUE, 0 },
    { { -13, -44, 13, -4 }, TRUE, 0 },
    { { -12, -29, 14, 13 }, TRUE, 0 },
    { { -12, -20, 13, -1 }, TRUE, 0 },
    { { -12, -30, 13, -7 }, TRUE, 0 },
    { { -17, -26, 8, -4 }, TRUE, 0 },
    { { -22, -30, 3, -8 }, TRUE, 0 },
    { { -21, -23, 19, -10 }, TRUE, 0 },
    { { -19, -23, 19, -10 }, TRUE, 0 },
    { { -17, -23, 23, -10 }, TRUE, 0 },
    { { -12, -23, 27, -10 }, TRUE, 0 },
    { { -5, -23, 35, -10 }, TRUE, 0 },
    { { 11, -23, 53, -10 }, TRUE, 0 },
    { { -25, -23, 13, -9 }, TRUE, 0 },
    { { -23, -21, 11, -6 }, TRUE, 0 },
    { { -15, -33, 9, -9 }, TRUE, 0 },
    { { -7, -34, 10, -3 }, TRUE, 0 },
    { { -14, -23, 11, 4 }, TRUE, 0 },
    { { -13, -23, 13, 5 }, TRUE, 0 },
    { { -4, -39, 11, 2 }, TRUE, 0 },
    { { -4, -39, 11, 0 }, TRUE, 0 },
    { { -4, -39, 11, -2 }, TRUE, 0 },
    { { -4, -45, 11, -6 }, TRUE, 0 },
    { { -4, -52, 11, -14 }, TRUE, 0 },
    { { -4, -69, 11, -30 }, TRUE, 0 },
    { { -6, -29, 9, 5 }, TRUE, 0 },
    { { -5, -34, 10, -4 }, TRUE, 0 },
    { { -10, -30, 16, -7 }, TRUE, 0 },
    { { -10, -37, 7, -5 }, TRUE, 0 },
    { { -10, -45, 13, -15 }, TRUE, 0 },
    { { -12, -45, 13, -15 }, TRUE, 0 },
    { { -10, -39, 3, -1 }, TRUE, 0 },
    { { -10, -38, 3, -1 }, TRUE, 0 },
    { { -10, -36, 3, -1 }, TRUE, 0 },
    { { -10, -32, 3, 6 }, TRUE, 0 },
    { { -10, -24, 3, 13 }, TRUE, 0 },
    { { -10, -8, 3, 30 }, TRUE, 0 },
    { { -10, -44, 5, -10 }, TRUE, 0 },
    { { -10, -37, 7, -3 }, TRUE, 0 },
    { { -18, -35, 8, -7 }, FALSE, 0 },
    { { -18, -20, 11, 3 }, FALSE, 0 },
    { { -8, -34, 13, -6 }, FALSE, 0 },
    { { -16, -19, 10, 1 }, FALSE, 0 },
    { { -21, -26, 4, -7 }, FALSE, 0 },
};
static const struct SqueakBoxFrame sUjaAnim1[] = { { 8, 0 }, { 8, 1 }, { 12, 2 }, { 16, 3 } };
static const struct SqueakBoxFrame sUjaAnim2[] = { { 16, 4 } };
static const struct SqueakBoxFrame sUjaAnim3[] = { { 16, 5 } };
static const struct SqueakBoxFrame sUjaAnim4[] = { { 12, 6 } };
static const struct SqueakBoxFrame sUjaAnim5[] = { { 16, 7 } };
static const struct SqueakBoxFrame sUjaAnim6[] = { { 16, 8 } };
static const struct SqueakBoxFrame sUjaAnim7[] = { { 1, 9 }, { 15, 10 } };
static const struct SqueakBoxFrame sUjaAnim8[] = { { 12, 11 } };
static const struct SqueakBoxFrame sUjaAnim9[] = { { 12, 12 } };
static const struct SqueakBoxFrame sUjaAnim10[] = { { 12, 13 } };
static const struct SqueakBoxFrame sUjaAnim11[] = { { 12, 14 } };
static const struct SqueakBoxFrame sUjaAnim12[] = { { 8, 14 } };
static const struct SqueakBoxFrame sUjaAnim13[] = { { 5, 15 } };
static const struct SqueakBoxFrame sUjaAnim14[] = { { 5, 16 } };
static const struct SqueakBoxFrame sUjaAnim15[] = { { 1, 17 }, { 4, 17 } };
static const struct SqueakBoxFrame sUjaAnim16[] = { { 8, 18 } };
static const struct SqueakBoxFrame sUjaAnim17[] = { { 4, 19 }, { 2, 20 }, { 3, 21 }, { 6, 21 }, { 1, 22 }, { 1, 23 }, { 1, 24 }, { 1, 25 }, { 1, 26 }, { 1, 27 }, { 16, 28 }, { 4, 29 } };
static const struct SqueakBoxFrame sUjaAnim18[] = { { 4, 30 }, { 2, 31 }, { 3, 32 }, { 6, 33 }, { 1, 34 }, { 1, 35 }, { 1, 36 }, { 1, 37 }, { 1, 38 }, { 1, 39 }, { 16, 40 }, { 4, 41 } };
static const struct SqueakBoxFrame sUjaAnim19[] = { { 4, 42 }, { 2, 43 }, { 3, 44 }, { 5, 45 }, { 1, 45 }, { 1, 46 }, { 1, 47 }, { 1, 48 }, { 1, 49 }, { 1, 50 }, { 1, 51 }, { 16, 52 }, { 4, 53 } };
static const struct SqueakBoxFrame sUjaAnim20[] = { { 1, 54 } };
static const struct SqueakBoxFrame sUjaAnim21[] = { { 1, 55 } };
static const struct SqueakBoxFrame sUjaAnim22[] = { { 1, 56 } };
static const struct SqueakBoxFrame sUjaAnim23[] = { { 1, 57 } };
static const struct SqueakBoxFrame sUjaAnim24[] = { { 1, 58 } };
static const struct SqueakBoxFrame sUjaAnim25[] = { { 8, 18 } };
static const struct SqueakBoxAnim sUjaBoxAnims[] = {
    { NULL, 0 },
    { sUjaAnim1, 4 },
    { sUjaAnim2, 1 },
    { sUjaAnim3, 1 },
    { sUjaAnim4, 1 },
    { sUjaAnim5, 1 },
    { sUjaAnim6, 1 },
    { sUjaAnim7, 2 },
    { sUjaAnim8, 1 },
    { sUjaAnim9, 1 },
    { sUjaAnim10, 1 },
    { sUjaAnim11, 1 },
    { sUjaAnim12, 1 },
    { sUjaAnim13, 1 },
    { sUjaAnim14, 1 },
    { sUjaAnim15, 2 },
    { sUjaAnim16, 1 },
    { sUjaAnim17, 12 },
    { sUjaAnim18, 12 },
    { sUjaAnim19, 13 },
    { sUjaAnim20, 1 },
    { sUjaAnim21, 1 },
    { sUjaAnim22, 1 },
    { sUjaAnim23, 1 },
    { sUjaAnim24, 1 },
    { sUjaAnim25, 1 },
};
static const struct SqueakBoxes sUjaBoxData = { sUjaBoxes, sUjaBoxAnims, 26 };

/* Kirby held (kirby->unk110): where it is (his feet at its spot), 4 pixels up while it closes on him; behind it, in KatAM's grabbers'
 * pose (0x57, Mr. Frosty's, Box Boxer's); each followed by an end the hold never reaches; */
static const struct Kirby_110 sClose[] = { { 0, -KIRBY_FEET - GRAB_UP, -1, 0xFF, 0x57, 0x400 }, { 0, 0, 0, 1, 0, 0x400 } };
static const struct Kirby_110 sSmother[] = { { 0, -KIRBY_FEET, -1, 0xFF, 0x57, 0x400 }, { 0, 0, 0, 1, 0, 0x400 } };
/* Thrown back hurt (flag 0x10: sub_0804EA18, KatAM's hurt, at his facing's x speed, x 0x10; he faces as it), or let go as he is
 * (flag 1); with 0x400 too, or KatAM's hold lets him go its own way first (see maiga.c's); */
static const struct Kirby_110 sThrow[] = { { THROW_SPEED, 0, -1, 1, 0, 0x410 }, { 0, 0, 0, 1, 0, 0 } };
static const struct Kirby_110 sLetGo[] = { { 0, 0, -1, 1, 0, 0x401 }, { 0, 0, 0, 1, 0, 0 } };

static void UjaMain(struct Object *obj);

// KirbyKeys(); A Kirby's pad (A, B, the D-pad: Squeak Squad's 0xE8 fields 0x14-0x20 it watches), as maiga.c's;
static u16 KirbyKeys(struct Kirby *k) {
    if (!(gUnk_0203AD10 & 0x12) && k->base.unk56 == 0) return gHeldKeys & 0xF3;
    return k->movementState & 0xF3;
}

static s32 Toward0(s32 v, s32 step) {
    if (v > 0) return v - step < 0 ? 0 : v - step;
    if (v < 0) return v + step > 0 ? 0 : v + step;
    return 0;
}

/* Bob(); 0x02080BD8 (with 1): its speed up/down is BASE_VY with the bob: up BOB_SPEED (and BOB_ACCEL a frame) the first 20 of its 40
 * frames, down the next 20, then a frame BASE_VY alone; */
static void Bob(struct SqueakObject *s) {
    if (BOB(s) <= BOB_HALF) {
        if (BOB(s) > 0) {
            s->vy = BASE_VY(s) - BOB_SPEED;
            s->ax = 0;
            s->ay = -BOB_ACCEL;
        }
    } else {
        s->vy = BASE_VY(s) + BOB_SPEED;
        s->ax = 0;
        s->ay = BOB_ACCEL;
    }
    if (BOB(s) < 1) {
        s->vy = BASE_VY(s);
        s->ax = s->ay = 0;
        BOB(s) = BOB_TIME;
    } else {
        BOB(s)--;
    }
}

// Slow(); 0x02080C80: no pad across, its speed across slows by FLOAT_SLOW a frame; none up/down, BASE_VY too;
static void Slow(struct SqueakObject *s) {
    if (s->padDir == 0 && s->vx != 0) s->vx = Toward0(s->vx, FLOAT_SLOW);
    if (s->padY == 0) BASE_VY(s) = Toward0(BASE_VY(s), FLOAT_SLOW);
}

// Steer(); 0x02080D44: forward speeds it up to FLOAT_SPEED; up and down speed BASE_VY up to it;
static void Steer(struct SqueakObject *s) {
    if (s->padDir > 0) s->vx = s->vx < FLOAT_SPEED ? s->vx + FLOAT_ACCEL : FLOAT_SPEED;
    if (s->padY > 0) BASE_VY(s) = BASE_VY(s) < FLOAT_SPEED ? BASE_VY(s) + FLOAT_ACCEL : FLOAT_SPEED;
    if (s->padY < 0) BASE_VY(s) = BASE_VY(s) <= -FLOAT_SPEED ? -FLOAT_SPEED : BASE_VY(s) - FLOAT_ACCEL;
}

// Float(); The forms' frame: slows, bobs, moves (and, `steer`, steers first);
static void Float(struct SqueakObject *s, bool8 steer) {
    if (steer) Steer(s);
    Slow(s);
    Bob(s);
    SqueakMove(s);
}

// Play(); An animation and the hurt ones that go with it (0x02021A88: the puff's 24, the glove's 20 and 21, the sword's 22 and 23);
static void Play(struct SqueakObject *s, u8 anim, u8 hurt) {
    HURT(s) = hurt;
    SqueakPlay(s, anim);
}

// TurnRound(); 0x02082594, 0x02083B5C: shrunk, it turns round to go the other way at the speed it had;
static void TurnRound(struct SqueakObject *s) {
    if (!TURNING(s)) return;
    TURNING(s) = FALSE;
    s->vx = -s->vx;
    SqueakTurnAround(s);
}

// Leaving(); Pad up/down 5 (every form's first check, after the time's): it turns round as asked, and flies away;
static bool32 Leaving(struct SqueakObject *s) {
    if (s->padY != PAD_FLY) return FALSE;
    if (s->padDir < 0) SqueakTurnAround(s);
    SqueakSetState(s, BODY_FLY_ENTRY);
    return TRUE;
}

// Formed(); A form's states: its time up, it shrinks (`anim`) back to a puff;
static bool32 Formed(struct SqueakObject *s, u8 anim) {
    if (TIMER(s) >= 0) return FALSE;
    Play(s, anim, HURT_PUFF);
    SqueakSetState(s, BODY_VANISH);
    return TRUE;
}

/* Shifting(); A shift's frame: its animation done, the next (`anim`, `hurt`, `next`); else it floats on (no steering) and its time
 * goes on; */
static bool32 Shifting(struct SqueakObject *s, u8 anim, u8 hurt, u8 next) {
    if (s->animEnded) {
        Play(s, anim, hurt);
        SqueakSetState(s, next);
        return TRUE;
    }
    Float(s, FALSE);
    TIMER(s)--;
    return FALSE;
}

// StillHeld(); The Kirby it holds is still held by it (KatAM's hold lets him go on its own when it's beaten);
static bool32 StillHeld(struct SqueakObject *s) {
    struct Kirby *kirby = HELD(s);

    return kirby && kirby->base.unk6C == &s->obj.base && (kirby->unk110 == sClose || kirby->unk110 == sSmother);
}

// LetGo(); Kirby let go: thrown back hurt (sThrow) or as he is (sLetGo), the table started over;
static void LetGo(struct SqueakObject *s, const struct Kirby_110 *how) {
    struct Kirby *kirby = HELD(s);

    if (StillHeld(s)) {
        kirby->unk110 = how;
        kirby->base.counter = -1;
    }
    HELD(s) = NULL;
    s->obj.base.unk6C = NULL;
}

/* Grab(); 0x02083F2C: a Kirby touched its fingers: it's where he is (his feet), and holds him (KatAM's hold, sub_0804E09C: he faces as
 * it); its anim closes as it pointed; */
static void Grab(struct SqueakObject *s, struct Kirby *kirby) {
    static const u8 sCloses[] = { ANIM_CLOSE, ANIM_CLOSE_UP, ANIM_CLOSE_DOWN };

    HELD(s) = kirby;
    s->obj.base.x = kirby->base.x;
    s->obj.base.y = kirby->base.y + KIRBY_FEET * 0x100;
    kirby->base.unk6C = &s->obj.base;
    kirby->unk110 = sClose;
    s->obj.base.unk6C = kirby;
    kirby->base.flags |= 0x40000;
    sub_0804E09C(kirby);
    SqueakStop(s);
    TIMER(s) = 0;
    Play(s, sCloses[POINTS(s) < 3 ? POINTS(s) : 0], HURT_GLOVE);
    SqueakSetState(s, BODY_CLOSE);
}

static bool32 Body(struct SqueakObject *s) {
    struct Kirby *kirby;
    s32 timer;

    switch (s->state) {
    case BODY_START:        // 0x0208112C (vtable +0x3C);
        Play(s, ANIM_PUFF, HURT_PUFF);
        SqueakSetState(s, BODY_PUFF);
        return TRUE;
    case BODY_PUFF:         // 0x02081004;
        if (s->padY == PAD_FLY) {
            if (s->padDir < 0) SqueakTurnAround(s);
            SqueakSetState(s, BODY_FLY_ENTRY);
            return TRUE;
        }
        if (s->padY == PAD_GROW) {
            SqueakSetState(s, BODY_GROW_ENTRY);
            return TRUE;
        }
        if (s->padA) {
            SqueakSetState(s, BODY_SWORD_ENTRY);
            return TRUE;
        }
        if (s->padB) {
            SqueakSetState(s, BODY_GLOVE_ENTRY);
            return TRUE;
        }
        if (s->padDir < 0) {
            SqueakTurnAround(s);
            s->vx = -s->vx;
        }
        Float(s, FALSE);
        break;
    case BODY_FLY_ENTRY:    // 0x02080F04;
        Play(s, ANIM_PUFF, HURT_PUFF);
        SqueakStop(s);
        s->vx = FLY_SPEED;
        s->vy = 0;
        SqueakSetState(s, BODY_FLY);
        return TRUE;
    case BODY_FLY:          // 0x02080E4C;
        if (s->vy < FLY_RISE) s->vy += FLY_ACCEL;
        else s->vy = FLY_RISE;
        SqueakMove(s);
        break;
    case BODY_GROW_ENTRY:   // 0x02083E10;
        Play(s, ANIM_GROW, HURT_PUFF);
        SqueakStop(s);
        SqueakSetState(s, BODY_GROW);
        return TRUE;
    case BODY_GROW:         // 0x02083D8C;
        if (s->animEnded) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        Float(s, FALSE);
        break;
    case BODY_VANISH:       // 0x02081200;
        if (s->animEnded) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        Float(s, FALSE);
        TIMER(s)--;
        break;

    case BODY_SWORD_ENTRY:  // 0x020826BC: the puff shrinks (and its time starts over);
        Play(s, ANIM_SHRINK, HURT_PUFF);
        SqueakStop(s);
        TIMER(s) = FORM_TIME;
        SqueakSetState(s, BODY_SWORD_TURN);
        return TRUE;
    case BODY_SWORD_TURN:   // 0x02082594;
        if (s->animEnded) TurnRound(s);
        return Shifting(s, ANIM_SHRINK_2, HURT_PUFF, BODY_SWORD_DRAW);
    case BODY_SWORD_DRAW:   // 0x0208248C;
        return Shifting(s, ANIM_SWORD, HURT_SWORD, BODY_SWORD);
    case BODY_SWORD:        // 0x0208215C: pointing ahead;
        if (Formed(s, ANIM_SHRINK) || Leaving(s)) return TRUE;
        if (s->padB) {
            Play(s, ANIM_LUNGE, HURT_SWORD);
            SqueakSetState(s, BODY_LUNGE);
            return TRUE;
        }
        if (s->padDir == 0 && s->padY != 0) {
            Play(s, ANIM_SHRINK, HURT_PUFF);
            SqueakSetState(s, s->padY > 0 ? BODY_SWORD_TO_UP : BODY_SWORD_TO_DOWN);
            return TRUE;
        }
        if (s->padDir < 0) {
            TURNING(s) = TRUE;
            Play(s, ANIM_SHRINK, HURT_PUFF);
            SqueakSetState(s, BODY_SWORD_ENTRY);
            return TRUE;
        }
        Float(s, TRUE);
        TIMER(s)--;
        break;
    case BODY_SWORD_UP:     // 0x02081CAC;
    case BODY_SWORD_DOWN:   // 0x020817F8;
        if (Formed(s, ANIM_SHRINK_3) || Leaving(s)) return TRUE;
        if (s->padB) {
            Play(s, s->state == BODY_SWORD_UP ? ANIM_LUNGE_UP : ANIM_LUNGE_DOWN, HURT_SWORD);
            SqueakSetState(s, s->state == BODY_SWORD_UP ? BODY_LUNGE_UP : BODY_LUNGE_DOWN);
            return TRUE;
        }
        if (s->padDir == 0 && (s->state == BODY_SWORD_UP ? s->padY < 0 : s->padY > 0)) {
            Play(s, s->state == BODY_SWORD_UP ? ANIM_SHRINK_3 : ANIM_SHRINK, HURT_PUFF);
            SqueakSetState(s, s->state == BODY_SWORD_UP ? BODY_SWORD_TO_DOWN : BODY_SWORD_TO_UP);
            return TRUE;
        }
        if (s->padDir != 0) {
            if (s->padDir < 0) TURNING(s) = TRUE;
            Play(s, ANIM_SHRINK_3, HURT_PUFF);
            SqueakSetState(s, BODY_SWORD_TURN);
            return TRUE;
        }
        Float(s, TRUE);
        TIMER(s)--;
        break;
    case BODY_SWORD_TO_UP:  // 0x02082058;
        return Shifting(s, ANIM_TURN, HURT_PUFF, BODY_SWORD_TO_UP_2);
    case BODY_SWORD_TO_UP_2:    // 0x02081F50;
        return Shifting(s, ANIM_SWORD_UP, HURT_SWORD, BODY_SWORD_UP);
    case BODY_SWORD_TO_DOWN:    // 0x02081BA8;
        return Shifting(s, ANIM_TURN, HURT_PUFF, BODY_SWORD_TO_DOWN_2);
    case BODY_SWORD_TO_DOWN_2:  // 0x02081AA0;
        return Shifting(s, ANIM_SWORD_DOWN, HURT_SWORD, BODY_SWORD_DOWN);
    case BODY_LUNGE:        // 0x0208162C;
        return Shifting(s, ANIM_SWORD, HURT_SWORD, BODY_SWORD);
    case BODY_LUNGE_UP:     // 0x02081460;
        return Shifting(s, ANIM_SWORD_UP, HURT_SWORD, BODY_SWORD_UP);
    case BODY_LUNGE_DOWN:   // 0x02081294;
        return Shifting(s, ANIM_SWORD_DOWN, HURT_SWORD, BODY_SWORD_DOWN);

    case BODY_GLOVE_ENTRY:  // 0x02083C84: the puff shrinks (and its time starts over);
        Play(s, ANIM_SHRINK, HURT_PUFF);
        SqueakStop(s);
        TIMER(s) = FORM_TIME;
        SqueakSetState(s, BODY_GLOVE_TURN);
        return TRUE;
    case BODY_GLOVE_TURN:   // 0x02083B5C;
        if (s->animEnded) TurnRound(s);
        return Shifting(s, ANIM_SHRINK_2, HURT_PUFF, BODY_GLOVE_DRAW);
    case BODY_GLOVE_DRAW:   // 0x02083A4C;
        if (s->animEnded) POINTS(s) = 0;
        return Shifting(s, ANIM_GLOVE, HURT_GLOVE, BODY_GLOVE);
    case BODY_GLOVE:        // 0x02083754: pointing ahead;
        if (Formed(s, ANIM_SHRINK) || Leaving(s)) return TRUE;
        if (s->padDir == 0 && s->padY != 0) {
            Play(s, ANIM_SHRINK, HURT_PUFF);
            SqueakSetState(s, s->padY > 0 ? BODY_GLOVE_TO_UP : BODY_GLOVE_TO_DOWN);
            return TRUE;
        }
        if (s->padDir < 0) {
            TURNING(s) = TRUE;
            Play(s, ANIM_SHRINK, HURT_PUFF);
            SqueakSetState(s, BODY_GLOVE_ENTRY);
            return TRUE;
        }
        Float(s, TRUE);
        TIMER(s)--;
        break;
    case BODY_GLOVE_UP:     // 0x020832C8;
    case BODY_GLOVE_DOWN:   // 0x02082E38;
        if (Formed(s, ANIM_SHRINK_3) || Leaving(s)) return TRUE;
        if (s->padDir == 0 && (s->state == BODY_GLOVE_UP ? s->padY < 0 : s->padY > 0)) {
            Play(s, s->state == BODY_GLOVE_UP ? ANIM_SHRINK_3 : ANIM_SHRINK, HURT_PUFF);
            SqueakSetState(s, s->state == BODY_GLOVE_UP ? BODY_GLOVE_TO_DOWN : BODY_GLOVE_TO_UP);
            return TRUE;
        }
        if (s->padDir != 0) {
            if (s->padDir < 0) TURNING(s) = TRUE;
            Play(s, ANIM_SHRINK_3, HURT_PUFF);
            SqueakSetState(s, BODY_GLOVE_TURN);
            return TRUE;
        }
        Float(s, TRUE);
        TIMER(s)--;
        break;
    case BODY_GLOVE_TO_UP:  // 0x02083650;
        return Shifting(s, ANIM_TURN, HURT_PUFF, BODY_GLOVE_TO_UP_2);
    case BODY_GLOVE_TO_UP_2:    // 0x02083540;
        if (s->animEnded) POINTS(s) = 1;
        return Shifting(s, ANIM_GLOVE_UP, HURT_GLOVE, BODY_GLOVE_UP);
    case BODY_GLOVE_TO_DOWN:    // 0x020831C4;
        return Shifting(s, ANIM_TURN, HURT_PUFF, BODY_GLOVE_TO_DOWN_2);
    case BODY_GLOVE_TO_DOWN_2:  // 0x020830B4;
        if (s->animEnded) POINTS(s) = 2;
        return Shifting(s, ANIM_GLOVE_DOWN, HURT_GLOVE, BODY_GLOVE_DOWN);

    case BODY_CLOSE:        // 0x020829DC: closing on him; then the smother, and his pad as it was;
        if (!StillHeld(s)) {
            HELD(s) = NULL;
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        if (s->animEnded) {
            TIMER(s) = HOLD_TIME;
            Play(s, ANIM_SMOTHER, HURT_PUFF);
            HELD(s)->unk110 = sSmother;
            KEYS(s) = KirbyKeys(HELD(s));
            SqueakSetState(s, BODY_SMOTHER);
            return TRUE;
        }
        SqueakMove(s);
        break;
    case BODY_SMOTHER:      // 0x020827C4: its time up, it throws him, turns round and flies away; his pad changing shortens it;
        if (!StillHeld(s)) {
            HELD(s) = NULL;
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        kirby = HELD(s);
        timer = TIMER(s)--;
        if (timer < 1) {
            LetGo(s, sThrow);
            GUARD(s) = THROW_GUARD;
            SqueakTurnAround(s);
            SqueakSetState(s, BODY_FLY_ENTRY);
            return TRUE;
        }
        if (KirbyKeys(kirby) != KEYS(s)) {
            KEYS(s) = KirbyKeys(kirby);
            TIMER(s) -= MASH;
        }
        SqueakMove(s);
        break;
    }
    return FALSE;
}

/* Track(); 0x02080854 (the glove's), 0x02080610 (the sword's): at Kirby (his feet), across when he's more than `near` either way
 * (turning round when he's behind it), up or down when more than `near` above or below; with him ahead within (`across`, `up`), or
 * above or below within (`across2`, `up2`), it stops that way and it's TRUE (no time off); */
static bool32 Track(struct SqueakObject *s, s32 near, s32 across, s32 up, s32 across2, s32 up2) {
    struct Object *obj = &s->obj;
    s32 kx = SQ_X(obj->kirby3), ky = SQ_Y(obj->kirby3) - KIRBY_FEET * 0x1000, x = SQ_X(obj), y = SQ_Y(obj);
    bool8 left = obj->base.flags & 1;

    if (kx < x - near) {
        if (!left) {
            s->padDir = -1;
        } else {
            s->padDir = 1;
            if (kx > x - across && kx < x + across && ky > y - up && ky < y + up) {
                s->padDir = 0;
                return TRUE;
            }
        }
    } else if (kx > x + near) {
        if (!left) {
            s->padDir = 1;
            if (kx > x - across && kx < x + across && ky > y - up && ky < y + up) {
                s->padDir = 0;
                return TRUE;
            }
        } else {
            s->padDir = -1;
        }
    } else {
        s->padDir = 0;
    }
    if (ky < y - near || ky > y + near) {
        s->padY = ky < y - near ? -1 : 1;
        if (kx > x - across2 && kx < x + across2 && ky > y - up2 && ky < y + up2) {
            s->padY = 0;
            return TRUE;
        }
    } else {
        s->padY = 0;
    }
    return FALSE;
}

static void Brain(struct SqueakObject *s) {
    switch (s->brain) {
    case BRAIN_GLOVE_FACE:  // 0x020809F8;
        SqueakFaceKirby(s, FACE_DISTANCE);
        BRAIN_WAIT(s) = GLOVE_TIME;
        s->brain = BRAIN_GLOVE_PRESS;
        break;
    case BRAIN_GLOVE_PRESS: // 0x020809CC;
        s->padDir = 0;
        s->padB = 1;
        s->brain = BRAIN_GLOVE;
        break;
    case BRAIN_GLOVE:       // 0x02080854 (its box can't hold Kirby: more than 8 pixels away isn't within 8);
        if (Track(s, GLOVE_NEAR, GLOVE_AHEAD_ACROSS, GLOVE_AHEAD_UP, GLOVE_ABOVE_ACROSS, GLOVE_ABOVE_UP)) break;
        if (BRAIN_WAIT(s) < 1) s->brain = BRAIN_LEAVE;
        else BRAIN_WAIT(s)--;
        break;
    case BRAIN_SWORD_FACE:  // 0x020807E8;
        SqueakFaceKirby(s, FACE_DISTANCE);
        BRAIN_WAIT(s) = SWORD_TIME;
        s->brain = BRAIN_SWORD_PRESS;
        break;
    case BRAIN_SWORD_PRESS: // 0x020807C0;
        s->padDir = 0;
        s->padA = 1;
        s->brain = BRAIN_SWORD;
        break;
    case BRAIN_SWORD:       // 0x02080610;
        if (Track(s, SWORD_NEAR, SWORD_AHEAD_ACROSS, SWORD_AHEAD_UP, SWORD_ABOVE_ACROSS, SWORD_ABOVE_UP)) {
            s->brain = BRAIN_LUNGE;
            break;
        }
        if (BRAIN_WAIT(s) < 1) s->brain = BRAIN_LEAVE;
        else BRAIN_WAIT(s)--;
        break;
    case BRAIN_LUNGE:       // 0x020805E4;
        s->padDir = s->padY = 0;
        s->padB = 1;
        s->brain = BRAIN_LUNGED;
        break;
    case BRAIN_LUNGED:      // 0x020805BC;
        s->padB = 0;
        s->brain = BRAIN_SWORD_PRESS;
        break;
    case BRAIN_LEAVE:       // 0x0208083C, 0x02080590 (the sword's lets go of B);
        s->padDir = s->padY = 0;
        if (s->variant == 1) s->padB = 0;
        s->brain = BRAIN_TURN;
        break;
    case BRAIN_TURN:        // 0x02080820, 0x02080574;
        s->padDir = -1;
        s->padY = PAD_FLY;
        s->brain = BRAIN_GONE;
        break;
    case BRAIN_GONE:        // 0x02080814, 0x02080568;
        s->padDir = 0;
        s->padY = PAD_FLY;
        break;
    }
}

static void UjaMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;
    struct Kirby *kirby;

    Brain(s);
    SqueakRun(s, Body);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
    SqueakSetBoxes(s, &sUjaBoxData, HARM(s), &GRABS(s));
    kirby = s->obj.unk80 > 0 && !HELD(s) ? SqueakGrabbing(s, &sUjaBoxes[GRABS(s)]) : NULL;
    if (kirby) Grab(s, kirby);
    /* Holding a Kirby, his body (KatAM's attack kind 0x80, its touch) doesn't hurt it (as KatAM's mini-bosses that hold him); the
     * Kirby it threw is untouchable THROW_GUARD frames (Squeak Squad's 0xEC): it's untouchable then (KatAM's 0x200, as Maiga's: KatAM's
     * hurt Kirby, flashing, kills what he touches); */
    if (HELD(s)) obj->base.unk5C |= 0x80;
    else obj->base.unk5C &= ~0x80;
    if (GUARD(s) > 0) GUARD(s)--;
    if (GUARD(s) > 0) obj->base.flags |= 0x200;
    else obj->base.flags &= ~0x200;
}

/* UjaWatch(); Its pre-callback (obj->unk7C: every frame, hit or beaten too): KatAM's hit put it out of its own state (unk78 not UjaMain),
 * or it's beaten: it lets Kirby go as he is; KatAM's hurt frames (0 and 1) shown as its state's (0x02021A88's: the puff's 24, the
 * glove's 20 and 21, the sword's 22 and 23: their cells); */
static void UjaWatch(struct Object *obj) {
    static const u8 sHurtCells[][2] = { { 60, 60 }, { 58, 59 }, { 45, 55 } };
    struct SqueakObject *s = (struct SqueakObject *)obj;

    if (HELD(s) && (obj->unk78 != UjaMain || obj->unk80 <= 0 || (obj->base.flags & 0x1000))) LetGo(s, sLetGo);
    if (obj->unk78 != UjaMain && obj->unk83 < SQUEAK_HURT_VARIANTS)
        SqueakSetCell(s, sHurtCells[HURT(s) < 3 ? HURT(s) : 0][obj->unk83 ? 1 : 0]);
}

void UjaStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, 0, UjaMain);
    obj->unk7C = UjaWatch;
    SqueakThroughTerrain(s, TRUE);
    if (!s->started) s->brain = s->variant == 1 ? BRAIN_SWORD_FACE : BRAIN_GLOVE_FACE;
    if (HELD(s)) LetGo(s, sLetGo);
    TURNING(s) = FALSE;
    SqueakSetState(s, BODY_START);
}

/* UjaInit(); Its body box (uja.rca's, every frame's: 5 pixels either side, 8 to 18 over its spot: KatAM's bounds); its hit box (the
 * puff's) and harm set each frame (SqueakSetBoxes()); */
void UjaInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -12, -30, 13, -9);
    ObjectSetBounds(&s->obj.base, -5, -18, 5, -8);
    HARM(s) = s->obj.base.unk68;
}
