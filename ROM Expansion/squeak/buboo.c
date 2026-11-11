/* Buboo; ported from Kirby: Squeak Squad (USA), mid-boss 0x36 (its body 0x02108EAC, vtable 0x02180A78; brain 0x02105938, its states
 * 0x0210523C-0x02105830), as SQUEAK_BUBOO, a mid-boss on mini-boss type 0x41 (squeak.h's SQUEAK_MIDBOSS), and its bubbles
 * (0x02109E00, blown by 0x02105CA8 and 0x02109F18: SQUEAK_BUBOO_BUBBLE, drawn with its graphics). Values from param/prmMB12Ml0.kgb
 * (its moves, its bubbles'), prmMB12Pl0.kgb (brain) and prmMB12Sl0.kgb (HP 140); speeds 20.12 a frame, Y up; its boxes from
 * enemy/buboo.rca. A red crab hiding in a cluster of bubbles: the cluster weaves and spirals about the screen, blows bubbles, and
 * drops Waddle Dees in big ones; five hits burst it, and the crab scuttles about blowing bubbles at Kirby till it blows a new one.
 * • Its fight starts as a mini-boss's, then (0x02108700, 0x02108684, 0x02108620) it waits 75 frames, untouchable, still; its brain
 *   runs meanwhile.
 * • Body (the cluster): it floats (anim 1: 0x02107F78, 0x02107EB4) bobbing (0x02105AB0: 20 frames at 0.2 up and 0.01 a frame, 20
 *   down, a frame still, on its base speed up and down; across and its base easing to 0 by 0.05: 0x02105B50). B (0x02105CE0): with
 *   up or down and fewer than 5 enemies about (0x0201B76C: KatAM's enemy types, Buboo's own bubbles not counted, here), it flies
 *   (0x021068D4, 0x02106750) to the screen's middle across, 80 pixels under its top (1.0 at most, 0.05 a frame, turning toward it),
 *   and there (within 16 pixels) shakes 60 frames (2 pixels either way), drops a big bubble with a Waddle Dee in it (24 pixels under
 *   it: 0x02106694) and stays 60 frames (0x02106618); else it flies there the same way (0x02106EB4, 0x02106CC0), swells (anim 4) and
 *   shakes 120 frames (0x02106C04), then 180 frames (0x02106A44) blows a little bubble (32 pixels over it) every 16, 20, 28 or 36
 *   frames (20, 40, 20, 10 in 100, else 16), and shakes 60 more (anim 1: 0x02106998). A (0x02105D88): with up or down it circles
 *   (0x021073CC, 0x02107234: to the screen's middle, 96 under its top, within 8 pixels; then 0x02107080: forward 2.1 and up 1.4 times
 *   the cosine and sine of its angle, which turns 5 degrees a frame, half a degree less each half turn: a widening spiral) while A
 *   is held; else it weaves (0x02107C7C, 0x02107AD4: to the screen's middle, 80 under its top, within 8 pixels; then 0x021079BC,
 *   0x021078A4, 0x02107790, 0x02107674, 0x0210755C, 0x02107444: forward faster to 1.6 by 0.1 a frame, its speed up and down eased
 *   between 1.6 and -1.6 by 0.1 a frame, plus the bob's 0.01 left on: a half takes 29 or 35 frames, as logged on the DS), turning
 *   round once a loop (0x02107674), while A is held. Its pad alone flies it (0x02107E10, 0x02107D50: 1.0 at most, 0.05 a frame)
 *   without bobbing. It's kept on the screen (0x02108C4C).
 * • Hit (0x021088D4: 36 frames safe after each): the cluster loses bubbles (anim 9: 0x0210857C, 0x02108528); the fifth hit bursts it
 *   (anim 10: 0x0210847C, 0x02108428) and the crab falls (anim 13, gravity 0.1 to 3.6: 0x02106F90, 0x02106F1C), scuttles at 3.0
 *   (anim 11: 0x021080F0, 0x02108024), turning at walls (and at the fight's area's edges: KatAM's mini-boss area, Squeak
 *   Squad's arenas being walled), 600 frames, blowing a bubble at Kirby every 60 (from the first frame), then
 *   (0x02108384, 0x021082B8, 0x02108204, 0x021081B0: anims 14, 15 and 16, still sliding as it was) blows a new cluster round itself.
 *   Its brain lets go of everything while the crab is out (0x0210523C).
 * • Brain (Squeak Squad's random 0-99; one state a frame): it faces Kirby (0x02105830), waits 60 frames (20 in 100: none:
 *   0x021057CC, 0x0210579C), weaves 120, 180 or 210 frames (30, 30, 40 in 100: 0x02105680, 0x0210563C) and, its body still again,
 *   picks (0x021054B4): 30 in 100 blows little bubbles (B: 0x0210577C, 0x02105700), 30 drops a Waddle Dee (B with up: 0x021053D0,
 *   0x02105354), 40 circles (A with up, 600 frames: 0x02105434, 0x021053F0, then it waits again); a pick made 3 times before is
 *   swapped (its count starting over) for one of the other two, half and half. After a B (its body still), half the time it holds A
 *   60 frames (a short weave: 0x02105328, 0x021052F4) and weaves again, else presses B again (0x021052D4, 0x02105298: its body busy,
 *   unread) and waits 60 frames (0x02105270) to wait again.
 * • Its bubbles (0x02109E00): a little one blown by the cluster (0x0210999C) flies off at 2.4 at 44 to 143 degrees (11 degrees apart:
 *   at random), falling 0.1 a frame to 1.8, through everything and out of the stage's edges (its flag 0x20000), then gone
 *   (0x02109948); the crab's (0x02109844, 0x02109608: its wait, 40 frames, set to 0 the frame it starts, so it goes at once) goes
 *   from 8 pixels over the crab at 1.5 toward Kirby across and up (never down: the angle of his distance's sizes), slowing by
 *   21/4096ths of that a frame, 180 frames or till it meets the terrain (or the floor), then pops (anim 6: 0x021094E4, 0x02109928);
 *   the big one (0x02109420, 0x021092F0) sinks 0.01 a frame to 1.8, a Waddle Dee in it 4 pixels over its spot (KatAM's, its feet
 *   there: its own, walking on as it was spawned to, once the bubble lets it go), till it meets the terrain (or the floor), then
 *   pops (anim 3: 0x021091EC, 0x021091CC), letting the Waddle Dee go; an inhale pops it too.
 *   Touching Kirby (0x02109CE4), any bubble bursts (efc_cmn1's animation 1: 0x02109170), letting its Waddle Dee go (his body
 *   doesn't hurt them: KatAM's unk5C 0x80); Kirby's attacks beat them as KatAM's (Squeak Squad's burst them).
 * Its sounds (Squeak Squad's, SqueakSfx): 0x10D, 0x135, 0x152, 0x153, 0x165, 0x173, 0x174, 0x175, 0x176, 0x17C. Not ported: the music change (0x020B0684),
 * Squeak Squad's hit flash (its palette, 0x021059D8) and shakes (KatAM's), its own defeat (0x02106378, 0x02106530..: a hop, a slide
 * shedding dust, a shudder: KatAM's mini-boss defeat instead), its ability (none), the other mode's pad (0x021087C8), the DS's rumble
 * (0x0221C6EC). Before its fight it falls to the floor, as the framework's mid-bosses do (SqueakMidbossWait; Squeak Squad's stays where
 * it is). KatAM's hits take HP at KatAM's rate: only those 36 frames apart count toward the burst, as Squeak Squad's. */
#include "../squeak.h"
#include "trig.h"
#include "task.h"

// Animations (buboo.cea);
enum {
    ANIM_STILL, ANIM_CLUSTER, ANIM_BIG, ANIM_BIG_POP, ANIM_SWELL, ANIM_SMALL, ANIM_SMALL_POP, ANIM_7, ANIM_8, ANIM_HURT, ANIM_BURST,
    ANIM_CRAB_WALK, ANIM_CRAB_STILL, ANIM_CRAB_FALL, ANIM_CRAB_STOP, ANIM_CRAB_BLOW, ANIM_REFORM,
};

// prmMB12Ml0.kgb (20.12);
#define GRAVITY 0x19A           // +0x0C, +0x1C: the crab's fall;
#define MAX_FALL -0x399A
#define FLY_CAP 0x1000          // +0x30, +0x34: its flights and its pad's, at most; +0x38, +0x3C: a frame;
#define FLY_ACCEL 205
#define WEAVE_CAP 0x199A        // +0x48, +0x4C, +0x50, +0x54;
#define WEAVE_ACCEL 0x19A
#define BOB_SPEED 0x334         // +0x5C, +0x64;
#define BOB_ACCEL 41
#define SPOT_DOWN 80            // +0x40/+0x44, +0x74/+0x78: its spot (the screen's middle across), pixels under the screen's top;
#define CIRCLE_DOWN 96          // +0x68/+0x6C: the circle's;
#define SWELL 120               // +0x7C, +0x80, +0x84;
#define SHOOT 180
#define AFTER 60
#define CRAB_SPEED 0x3000       // +0xD8, +0xDC, +0xE0;
#define CRAB_TIME 600
#define CRAB_SHOT 60
#define SMALL_SPEED 0x2667      // The bubbles': +0xAC, +0xB0, +0xB4;
#define BUBBLE_GRAVITY 0x19A
#define BUBBLE_FALL -0x1CCD
#define AIM_UP 0x8000           // +0xBC/+0xC0, +0xC4, +0xC8, +0xD0;
#define AIM_SPEED 0x1800
#define AIM_DRAG 21
#define AIM_TIME 180
// Its code's own;
#define INTRO 75
#define BOB 20
#define DROP_SHAKE 60
#define SHAKE 0x200             // 2 pixels (KatAM's units);
#define NEAR 0x1000             // Within 16 pixels: there (B's flights); 8 (A's);
#define CLOSE 0x800
#define CIRCLE_SPEED 0x1666     // 0x02107080: 1.4 times the sine up, 1.5 times that (2.1) the cosine across;
#define CIRCLE_WIDE 0x1800
#define CIRCLE_STEP 0x5000      // Its angle's turn a frame (20.12 degrees), half a degree less each half turn;
#define CIRCLE_SLOW 0x800
#define HALF_TURN 0xB4000
#define FULL_TURN 0x168000
#define HITS 5
#define SAFE_TIME 36
#define MAX_ENEMIES 5
#define SMALL_UP 0x20000        // 0x02105CA8: its bubbles' spots (20.12, up): a little one's 32 over it, a big one's 24 under it;
#define BIG_DOWN -0x18000
#define BIG_HELD 4              // Pixels over a big bubble's spot: its Waddle Dee's feet (KatAM's Waddle Dee's spot 9 over them);
#define DEE_FEET 9
#define FACE_DISTANCE 0x1000

enum {
    BODY_START, BODY_STAND, BODY_INTRO, BODY_PAD_ENTRY, BODY_PAD, BODY_WEAVE_ENTRY, BODY_WEAVE_TO, BODY_WEAVE_1, BODY_WEAVE_2,
    BODY_WEAVE_3, BODY_WEAVE_4, BODY_WEAVE_5, BODY_WEAVE_6, BODY_CIRCLE_ENTRY, BODY_CIRCLE_TO, BODY_CIRCLE, BODY_SHOOT_ENTRY,
    BODY_SHOOT_TO, BODY_SHOOT_SWELL, BODY_SHOOT, BODY_SHOOT_AFTER, BODY_DROP_ENTRY, BODY_DROP_TO, BODY_DROP_SHAKE, BODY_DROP_AFTER,
    BODY_HURT_ENTRY, BODY_HURT, BODY_BURST_ENTRY, BODY_BURST, BODY_CRAB_FALL_ENTRY, BODY_CRAB_FALL, BODY_CRAB_ENTRY, BODY_CRAB,
    BODY_REFORM_ENTRY, BODY_REFORM_1, BODY_REFORM_2, BODY_REFORM_3,
};
enum {
    BRAIN_START, BRAIN_IDLE, BRAIN_WAIT_PICK, BRAIN_WAIT, BRAIN_WEAVE, BRAIN_WEAVING, BRAIN_PICK, BRAIN_SHOOT, BRAIN_SHOOT_WAIT,
    BRAIN_DROP, BRAIN_DROP_WAIT, BRAIN_CIRCLE, BRAIN_CIRCLING, BRAIN_A_AGAIN, BRAIN_A_AGAIN_WAIT, BRAIN_B_AGAIN, BRAIN_B_AGAIN_WAIT,
    BRAIN_REST,
};
// Its bubbles' kinds (their variant: 0x02109E00's);
enum { BUBBLE_SMALL, BUBBLE_AIMED, BUBBLE_BIG };

#define BUSY(s) ((s)->vars[0])          // 0x204: 0 floating, 1 the crab, 3 its pad, 4 weaving, 5 circling, 7 blowing, 8 dropping,
                                        // 9 hurt, 10 bursting, 11 re-forming;
#define CLUSTER(s) ((s)->vars[1])       // 0x208: the crab in its cluster;
#define HIT_COUNT(s) ((s)->vars[2])     // 0x20C;
#define COUNT(s) ((s)->vars[3])         // 0x210, 0x214: the crab's bubble's frames; 0x218: the little bubbles';
#define CRAB_COUNT(s) ((s)->vars[4])
#define SHOT(s) ((s)->vars[5])
#define SAFE(s) ((s)->vars[6])          // 0x1F8: frames safe after a hit;
#define BASE_Y(s) ((s)->vars[7])        // 0x23C, 0x240: the bob's base speed and frames;
#define BOB_COUNT(s) ((s)->vars[8])
#define SPOT_X(s) ((s)->vars[9])        // 0x1A0, 0x1A4: where it flies to; where it shakes;
#define SPOT_Y(s) ((s)->vars[10])
#define ANGLE(s) ((s)->vars[11])        // 0x230, 0x22C: the circle's angle and half turns;
#define TURNS(s) ((s)->vars[12])
#define BRAIN_TIMER(s) ((s)->vars[13])  // The brain's 0x98;
#define SHOOTS(s) (*(u8 *)&(s)->vars[14])           // The brain's 0xA4, 0xA8, 0xAC: its picks of each kind;
#define DROPS(s) (*((u8 *)&(s)->vars[14] + 1))
#define CIRCLES(s) (*((u8 *)&(s)->vars[14] + 2))
#define BOX(s) (*((u8 *)&(s)->vars[14] + 3))
#define HARM(s) (*(s16 *)&(s)->vars[15])            // KatAM's unk68 from InitObject: its touch's harm; its HP last frame;
#define LAST_HP(s) (*((s16 *)&(s)->vars[15] + 1))

// Its bubbles' fields;
#define B_PARENT(s) (*(struct SqueakObject **)&(s)->vars[0])    // 0x140: the crab (an aimed one's);
#define B_TIMER(s) ((s)->vars[1])       // 0x13C;
#define B_AGE(s) ((s)->vars[2])         // 0x130: its frames, to 2;
#define B_HELD(s) (*(struct Object **)&(s)->vars[3])            // 0x134: its Waddle Dee; its own function and flags, kept;
#define B_HELD_FUNC(s) (*(void (**)(struct Object *))&(s)->vars[4])
#define B_HELD_FLAGS(s) ((s)->vars[5])
#define B_BOX(s) (*(u8 *)&(s)->vars[6])
#define B_HARM(s) (*(s16 *)&(s)->vars[7])
#define B_HELD_SPEED(s) ((s)->vars[8])
enum { BUBBLE_START, BUBBLE_FLY, BUBBLE_FOLLOW, BUBBLE_AIM, BUBBLE_POP_SMALL, BUBBLE_CARRY, BUBBLE_POP_BIG, BUBBLE_POPPING, BUBBLE_BURST };
// (a held Waddle Dee: no gravity, terrain, collisions or move of its own);
#define HELD_FLAGS (0x40 | 0x100 | 0x200 | 0x800)

// buboo.rca's boxes (boxes.py): the cluster's, a big bubble's, a little one's, the crab's;
static const struct SqueakBox sBubooBoxes[] = {
    { { -16, -26, 16, 2 }, TRUE, 0 },
    { { -13, -29, 13, -3 }, TRUE, 0 },
    { { -10, -20, 10, 0 }, TRUE, 0 },
    { { -11, -19, 11, 2 }, TRUE, 0 },
    { { -10, -19, 14, 1 }, TRUE, 0 },
};
static const struct SqueakBoxFrame sBubooAnim0[] = { { 1, 0 } };
static const struct SqueakBoxFrame sBubooAnim1[] = { { 24, 0 } };
static const struct SqueakBoxFrame sBubooAnim2[] = { { 4, 1 } };
static const struct SqueakBoxFrame sBubooAnim3[] = { { 1, 1 }, { 3, 0 } };
static const struct SqueakBoxFrame sBubooAnim4[] = { { 16, 0 } };
static const struct SqueakBoxFrame sBubooAnim5[] = { { 1, 2 }, { 3, 2 } };
static const struct SqueakBoxFrame sBubooAnim6[] = { { 1, 2 }, { 3, 0 } };
static const struct SqueakBoxFrame sBubooAnim7[] = { { 12, 0 } };
static const struct SqueakBoxFrame sBubooAnim8[] = { { 2, 0 }, { 4, 0 } };
static const struct SqueakBoxFrame sBubooAnim9[] = { { 27, 0 } };
static const struct SqueakBoxFrame sBubooAnim10[] = { { 24, 0 } };
static const struct SqueakBoxFrame sBubooAnim11[] = { { 1, 3 }, { 11, 0 } };
static const struct SqueakBoxFrame sBubooAnim12[] = { { 1, 3 } };
static const struct SqueakBoxFrame sBubooAnim13[] = { { 1, 3 }, { 11, 0 } };
static const struct SqueakBoxFrame sBubooAnim14[] = { { 20, 0 } };
static const struct SqueakBoxFrame sBubooAnim15[] = { { 32, 0 } };
static const struct SqueakBoxFrame sBubooAnim16[] = { { 16, 0 }, { 8, 0 }, { 18, 0 } };
static const struct SqueakBoxFrame sBubooAnim17[] = { { 1, 0 } };
static const struct SqueakBoxFrame sBubooAnim18[] = { { 1, 0 } };
static const struct SqueakBoxFrame sBubooAnim19[] = { { 16, 4 } };
static const struct SqueakBoxFrame sBubooAnim20[] = { { 1, 4 } };
static const struct SqueakBoxAnim sBubooBoxAnims[] = {
    { sBubooAnim0, 1 }, { sBubooAnim1, 1 }, { sBubooAnim2, 1 }, { sBubooAnim3, 2 }, { sBubooAnim4, 1 }, { sBubooAnim5, 2 },
    { sBubooAnim6, 2 }, { sBubooAnim7, 1 }, { sBubooAnim8, 2 }, { sBubooAnim9, 1 }, { sBubooAnim10, 1 }, { sBubooAnim11, 2 },
    { sBubooAnim12, 1 }, { sBubooAnim13, 2 }, { sBubooAnim14, 1 }, { sBubooAnim15, 1 }, { sBubooAnim16, 3 }, { sBubooAnim17, 1 },
    { sBubooAnim18, 1 }, { sBubooAnim19, 1 }, { sBubooAnim20, 1 },
};
// (buboo.rca's boxes flagged 0x8000 in its bubbles' pops, 3 and 6: untouchable);
static const struct SqueakBoxes sBubooBoxData = { sBubooBoxes, sBubooBoxAnims, 21, { SQUEAK_ANIM_LO(ANIM_BIG_POP) | SQUEAK_ANIM_LO(ANIM_SMALL_POP) } };

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

// Sine(); Squeak Squad's sine table (0x0215CE20, 4.12) by KatAM's (gSineTable: 1024 a turn, 2.14) at an angle (20.12 degrees);
static s32 Sine(s32 angle) {
    return gSineTable[(angle * 2 / 45 >> 6) & 0x3FF] >> 2;
}
static s32 Cosine(s32 angle) {
    return gSineTable[((angle * 2 / 45 >> 6) & 0x3FF) + 0x100] >> 2;
}

// Ease(); A speed eased by `accel` toward 0 (no further);
static s32 Ease(s32 v, s32 accel) {
    if (v > 0) return v - accel < 0 ? 0 : v - accel;
    if (v < 0) return v + accel > 0 ? 0 : v + accel;
    return v;
}

// Bob(); 0x02105AB0: its bob on its base speed up and down (`on`: else its base alone);
static void Bob(struct SqueakObject *s, bool32 on) {
    s32 a = on ? BOB_SPEED : 0, b = on ? BOB_ACCEL : 0;

    if (BOB_COUNT(s) > BOB) {
        s->ax = 0;
        s->vy = BASE_Y(s) + a;
        s->ay = b;
    } else if (BOB_COUNT(s) > 0) {
        s->ax = 0;
        s->vy = BASE_Y(s) - a;
        s->ay = -b;
    }
    if (BOB_COUNT(s) < 1) {
        s->ax = 0;
        s->vy = BASE_Y(s);
        s->ay = 0;
        BOB_COUNT(s) = 2 * BOB;
    } else {
        BOB_COUNT(s)--;
    }
}

// Friction(); 0x02105B50: no A and no direction, its speed across eases to 0; no up or down, its base speed too;
static void Friction(struct SqueakObject *s) {
    if (!s->padA && s->padDir == 0) s->vx = Ease(s->vx, FLY_ACCEL);
    if (s->padY == 0) BASE_Y(s) = Ease(BASE_Y(s), FLY_ACCEL);
}

// PadFly(); 0x02105C14: forward and up or down speed it up;
static void PadFly(struct SqueakObject *s) {
    if (s->padDir > 0) s->vx = s->vx < FLY_CAP ? s->vx + FLY_ACCEL : FLY_CAP;
    if (s->padY > 0) BASE_Y(s) = BASE_Y(s) < FLY_CAP ? BASE_Y(s) + FLY_ACCEL : FLY_CAP;
    if (s->padY < 0) BASE_Y(s) = BASE_Y(s) <= -FLY_CAP ? -FLY_CAP : BASE_Y(s) - FLY_ACCEL;
}

// Gravity(); 0x02105E24: on the ground, still; else falling GRAVITY a frame, no faster than MAX_FALL;
static void Gravity(struct SqueakObject *s) {
    if (SqueakOnGround(s)) {
        s->vy = s->ay = 0;
    } else if (!SqueakCapFall(s, MAX_FALL)) {
        s->ay = -GRAVITY;
    }
}

/* FlyTo(); Its flights to its spot (B's 0x02106CC0 and 0x02106750; A's 0x02107AD4 and 0x02107234, `deadZone`: within 8 pixels across
 * it stops speeding or turning): TRUE once within `near` (unmoved); else forward faster (1.0 at most) with the spot ahead, turned
 * round with it behind, and up or down toward it; */
static bool32 FlyTo(struct SqueakObject *s, s32 near, bool32 deadZone) {
    struct ObjectBase *b = &s->obj.base;
    s32 dx = b->x - SPOT_X(s), dy = b->y - SPOT_Y(s);
    u8 way;

    if (dx > -near && dx < near && dy > -near && dy < near) return TRUE;
    way = dx < 0 ? !(b->flags & 1) : dx > 0 ? (b->flags & 1) != 0 : 0;
    if (deadZone && dx > -CLOSE && dx < CLOSE) way = 2;
    if (way == 1) s->vx = s->vx < FLY_CAP ? s->vx + FLY_ACCEL : FLY_CAP;
    else if (way == 0) SqueakTurnAround(s);
    if (dy > 0) s->vy = s->vy < FLY_CAP ? s->vy + FLY_ACCEL : FLY_CAP;
    else if (dy < 0) s->vy = s->vy <= -FLY_CAP ? -FLY_CAP : s->vy - FLY_ACCEL;
    SqueakMove(s);
    return FALSE;
}

/* EdgeAhead(); The crab at the fight's area's edge ahead (KatAM's mini-boss area, unkA4-unkA8, its box kept 8 pixels inside it:
 * ObjXSomething), a wall to it, as Squeak Squad's arenas' walls (as Big Metalun's roll's AtEdge); */
static bool32 EdgeAhead(struct SqueakObject *s) {
    struct Object *obj = &s->obj;

    if (obj->unkA8 <= obj->unkA4) return FALSE;
    if (obj->base.flags & 1) return obj->base.x <= (obj->unkA4 + 8 - obj->base.unk3C) << 8;
    return obj->base.x >= (obj->unkA8 - 8 - obj->base.unk3E) << 8;
}

// ToSpot(); Its spot: the screen's middle across, `down` pixels under its top;
static void ToSpot(struct SqueakObject *s, s32 down) {
    SPOT_X(s) = CameraX(s) + ((DISPLAY_WIDTH / 2) << 8);
    SPOT_Y(s) = CameraY(s) + (down << 8);
}

// AtSpot(); 0x02106C04's: where it stopped, shaken (2 pixels right on frames 0-1 of 8, left on 4-5);
static void AtSpot(struct SqueakObject *s, bool32 shake) {
    s32 x = SPOT_X(s), t = COUNT(s) % 8;

    if (shake && t < 2) x += SHAKE;
    else if (shake && t >= 4 && t < 6) x -= SHAKE;
    s->obj.base.x = x;
    s->obj.base.y = SPOT_Y(s);
}

// HereSpot(); Where it is, as its spot (its shake's);
static void HereSpot(struct SqueakObject *s) {
    SPOT_X(s) = s->obj.base.x;
    SPOT_Y(s) = s->obj.base.y;
}

/* Enemies(); 0x0201B76C (the stage's enemies, kind 2): KatAM's enemies in its room (its object types under 0x38, alive), Buboo's
 * bubbles not counted (Squeak Squad's bubbles are thrown things, not enemies), the Waddle Dees in them counted; */
static u8 Enemies(struct SqueakObject *s) {
    u8 i, n = 0;

    for (i = 0; i < MAX_TASK_NUM; i++) {
        struct Task *t = &gTasks[i];
        struct Object *obj;

        if (t->main != ObjectMain) continue;
        obj = TaskGetStructPtr(t);
        if (obj->base.roomId != s->obj.base.roomId || obj->type >= 0x38 || obj->unk80 <= 0 || (obj->base.flags & 0x1000)) continue;
        if (obj->type == SQUEAK_TYPE && ((struct SqueakObject *)obj)->enemy == SQUEAK_BUBOO_BUBBLE) continue;
        n++;
    }
    return n;
}

// Buttons(); 0x02105CE0 (B: with up or down and room for more enemies the drop, else the little bubbles) and 0x02105D88 (A: with up or
// down the circle, else the weave; its pad alone, the pad's flight);
static bool32 Buttons(struct SqueakObject *s) {
    if (s->padB) {
        SqueakSetState(s, s->padY != 0 && Enemies(s) < MAX_ENEMIES ? BODY_DROP_ENTRY : BODY_SHOOT_ENTRY);
        return TRUE;
    }
    if (s->padA) {
        SqueakSetState(s, s->padY != 0 ? BODY_CIRCLE_ENTRY : BODY_WEAVE_ENTRY);
        return TRUE;
    }
    if (s->padDir > 0 || s->padY != 0) {
        SqueakSetState(s, BODY_PAD_ENTRY);
        return TRUE;
    }
    return FALSE;
}

// Weave(); The weave's states' start (0x021079BC..0x02107444): A with up or down, the circle; A let go, it floats; else forward faster;
static bool32 Weave(struct SqueakObject *s) {
    if (s->padY != 0 && s->padA) {
        SqueakSetState(s, BODY_CIRCLE_ENTRY);
        return TRUE;
    }
    if (!s->padA) {
        SqueakSetState(s, BODY_START);
        return TRUE;
    }
    s->vx = s->vx + WEAVE_ACCEL > WEAVE_CAP ? WEAVE_CAP : s->vx + WEAVE_ACCEL;
    return FALSE;
}

// WeaveTo(); Its speed up or down eased by WEAVE_ACCEL toward `to`: there (set to it), TRUE; else it moves;
static bool32 WeaveTo(struct SqueakObject *s, s32 to) {
    if (to > s->vy) {
        s->vy += WEAVE_ACCEL;
        if (s->vy >= to) {
            s->vy = to;
            return TRUE;
        }
    } else {
        s->vy -= WEAVE_ACCEL;
        if (s->vy <= to) {
            s->vy = to;
            return TRUE;
        }
    }
    SqueakMove(s);
    return FALSE;
}

static bool32 Body(struct SqueakObject *s) {
    s32 roll;

    switch (s->state) {
    case BODY_START:        // 0x02107F78 (vtable +0x3C);
        BUSY(s) = 0;
        SqueakPlay(s, ANIM_CLUSTER);
        CLUSTER(s) = 1;
        SqueakSetState(s, BODY_STAND);
        return TRUE;
    case BODY_STAND:        // 0x02107EB4;
        if (Buttons(s)) return TRUE;
        if (s->padDir < 0) SqueakTurnAround(s);
        Friction(s);
        Bob(s, TRUE);
        SqueakMove(s);
        break;
    case BODY_INTRO:        // 0x02108620: untouchable, still, INTRO frames;
        if (COUNT(s) < 1) {
            s->obj.base.flags &= ~0x200;
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        COUNT(s)--;
        SqueakMove(s);
        break;
    case BODY_PAD_ENTRY:    // 0x02107E10;
        BUSY(s) = 3;
        SqueakPlay(s, ANIM_CLUSTER);
        SqueakSetState(s, BODY_PAD);
        return TRUE;
    case BODY_PAD:          // 0x02107D50;
        if (s->padB) {
            SqueakSetState(s, s->padY != 0 && Enemies(s) < MAX_ENEMIES ? BODY_DROP_ENTRY : BODY_SHOOT_ENTRY);
            return TRUE;
        }
        if (s->padY != 0 && s->padA) {
            SqueakSetState(s, BODY_CIRCLE_ENTRY);
            return TRUE;
        }
        if (s->padDir == 0 && s->padY == 0) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        PadFly(s);
        Friction(s);
        Bob(s, FALSE);
        if (s->padDir < 0) SqueakTurnAround(s);
        SqueakMove(s);
        break;
    case BODY_WEAVE_ENTRY:  // 0x02107C7C;
        BUSY(s) = 4;
        SqueakPlay(s, ANIM_CLUSTER);
        ToSpot(s, SPOT_DOWN);
        SqueakSetState(s, BODY_WEAVE_TO);
        return TRUE;
    case BODY_WEAVE_TO:     // 0x02107AD4;
        if (s->padY != 0 && s->padA) {
            SqueakSetState(s, BODY_CIRCLE_ENTRY);
            return TRUE;
        }
        if (FlyTo(s, CLOSE, TRUE)) {
            SqueakSetState(s, BODY_WEAVE_1);
            return TRUE;
        }
        break;
    case BODY_WEAVE_1:      // 0x021079BC: up to 1.6;
        if (Weave(s)) return TRUE;
        if (WeaveTo(s, WEAVE_CAP)) {
            SqueakSetState(s, BODY_WEAVE_2);
            return TRUE;
        }
        break;
    case BODY_WEAVE_2:      // 0x021078A4: down to -1.6;
        if (Weave(s)) return TRUE;
        if (WeaveTo(s, -WEAVE_CAP)) {
            SqueakSetState(s, BODY_WEAVE_3);
            return TRUE;
        }
        break;
    case BODY_WEAVE_3:      // 0x02107790: up to 0;
        if (Weave(s)) return TRUE;
        if (WeaveTo(s, 0)) {
            SqueakSetState(s, BODY_WEAVE_4);
            return TRUE;
        }
        break;
    case BODY_WEAVE_4:      // 0x02107674: up to 1.6, then it turns round;
        if (Weave(s)) return TRUE;
        if (WeaveTo(s, WEAVE_CAP)) {
            SqueakTurnAround(s);
            SqueakSetState(s, BODY_WEAVE_5);
            return TRUE;
        }
        break;
    case BODY_WEAVE_5:      // 0x0210755C: down to -1.6;
        if (Weave(s)) return TRUE;
        if (WeaveTo(s, -WEAVE_CAP)) {
            SqueakSetState(s, BODY_WEAVE_6);
            return TRUE;
        }
        break;
    case BODY_WEAVE_6:      // 0x02107444: up to 1.6, and down again (0x021078A4);
        if (Weave(s)) return TRUE;
        if (WeaveTo(s, WEAVE_CAP)) {
            SqueakSetState(s, BODY_WEAVE_2);
            return TRUE;
        }
        break;
    case BODY_CIRCLE_ENTRY: // 0x021073CC;
        BUSY(s) = 5;
        ToSpot(s, CIRCLE_DOWN);
        SqueakSetState(s, BODY_CIRCLE_TO);
        return TRUE;
    case BODY_CIRCLE_TO:    // 0x02107234;
        if (FlyTo(s, CLOSE, TRUE)) {
            ANGLE(s) = TURNS(s) = 0;
            SqueakSetState(s, BODY_CIRCLE);
            return TRUE;
        }
        break;
    case BODY_CIRCLE:       // 0x02107080: A let go, it floats; at its angle, then turned on;
        if (!s->padA) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        s->vx = (((Cosine(ANGLE(s)) * CIRCLE_SPEED + 0x800) >> 12) * CIRCLE_WIDE + 0x800) >> 12;
        s->vy = (Sine(ANGLE(s)) * CIRCLE_SPEED + 0x800) >> 12;
        SqueakMove(s);
        ANGLE(s) += CIRCLE_STEP - CIRCLE_SLOW * TURNS(s);
        if (ANGLE(s) == HALF_TURN) TURNS(s)++;
        if (ANGLE(s) >= FULL_TURN) {
            ANGLE(s) = 0;
            TURNS(s)++;
        }
        break;
    case BODY_SHOOT_ENTRY:  // 0x02106EB4;
        ToSpot(s, SPOT_DOWN);
        SqueakSetState(s, BODY_SHOOT_TO);
        return TRUE;
    case BODY_SHOOT_TO:     // 0x02106CC0: there, it swells;
        if (FlyTo(s, NEAR, FALSE)) {
            BUSY(s) = 7;
            COUNT(s) = SWELL;
            HereSpot(s);
            SqueakPlay(s, ANIM_SWELL);
            SqueakSetState(s, BODY_SHOOT_SWELL);
            return TRUE;
        }
        break;
    case BODY_SHOOT_SWELL:  // 0x02106C04;
        if (COUNT(s) < 1) {
            AtSpot(s, FALSE);
            COUNT(s) = SHOOT;
            SHOT(s) = 0;
            SqueakSetState(s, BODY_SHOOT);
            return TRUE;
        }
        COUNT(s)--;
        AtSpot(s, TRUE);
        SqueakMove(s);
        break;
    case BODY_SHOOT:        // 0x02106A44: a little bubble now and then;
        if (SHOT(s) < 1) {
            roll = Roll();
            SqueakSpawn(s, SQUEAK_BUBOO_BUBBLE, BUBBLE_SMALL, 0, SMALL_UP);
            SHOT(s) = roll < 20 ? 16 : roll < 60 ? 20 : roll < 80 ? 28 : roll < 90 ? 36 : 16;
        } else {
            SHOT(s)--;
        }
        if (COUNT(s) < 1) {
            AtSpot(s, FALSE);
            SqueakPlay(s, ANIM_CLUSTER);
            COUNT(s) = AFTER;
            SqueakSetState(s, BODY_SHOOT_AFTER);
            return TRUE;
        }
        COUNT(s)--;
        AtSpot(s, TRUE);
        SqueakMove(s);
        break;
    case BODY_SHOOT_AFTER:  // 0x02106998;
        if (COUNT(s) < 1) {
            AtSpot(s, FALSE);
            SqueakStop(s);
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        COUNT(s)--;
        AtSpot(s, TRUE);
        SqueakMove(s);
        break;
    case BODY_DROP_ENTRY:   // 0x021068D4;
        SqueakPlay(s, ANIM_CLUSTER);
        ToSpot(s, SPOT_DOWN);
        SqueakSetState(s, BODY_DROP_TO);
        return TRUE;
    case BODY_DROP_TO:      // 0x02106750;
        if (FlyTo(s, NEAR, FALSE)) {
            BUSY(s) = 8;
            COUNT(s) = DROP_SHAKE;
            HereSpot(s);
            SqueakSetState(s, BODY_DROP_SHAKE);
            return TRUE;
        }
        break;
    case BODY_DROP_SHAKE:   // 0x02106694: a big bubble, a Waddle Dee in it;
        if (COUNT(s) < 1) {
            AtSpot(s, FALSE);
            COUNT(s) = DROP_SHAKE;
            SHOT(s) = 0;
            SqueakSpawn(s, SQUEAK_BUBOO_BUBBLE, BUBBLE_BIG, 0, BIG_DOWN);
            SqueakSetState(s, BODY_DROP_AFTER);
            return TRUE;
        }
        COUNT(s)--;
        AtSpot(s, TRUE);
        SqueakMove(s);
        break;
    case BODY_DROP_AFTER:   // 0x02106618;
        if (COUNT(s) < 1) {
            AtSpot(s, FALSE);
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        COUNT(s)--;
        AtSpot(s, FALSE);
        SqueakMove(s);
        break;
    case BODY_HURT_ENTRY:   // 0x0210857C;
        BUSY(s) = 9;
        SqueakPlay(s, ANIM_HURT);
        SqueakSetState(s, BODY_HURT);
        return TRUE;
    case BODY_HURT:         // 0x02108528;
    case BODY_REFORM_3:     // 0x021081B0;
        if (s->animEnded) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        SqueakMove(s);
        break;
    case BODY_BURST_ENTRY:  // 0x0210847C;
        BUSY(s) = 10;
        SqueakPlay(s, ANIM_BURST);
        COUNT(s) = 0;
        SqueakSetState(s, BODY_BURST);
        return TRUE;
    case BODY_BURST:        // 0x02108428;
        if (s->animEnded) {
            SqueakSetState(s, BODY_CRAB_FALL_ENTRY);
            return TRUE;
        }
        SqueakMove(s);
        break;
    case BODY_CRAB_FALL_ENTRY:  // 0x02106F90;
        SqueakPlay(s, ANIM_CRAB_FALL);
        SqueakStop(s);
        s->ay = -GRAVITY;
        SqueakSetState(s, BODY_CRAB_FALL);
        return TRUE;
    case BODY_CRAB_FALL:    // 0x02106F1C;
        SqueakCapFall(s, MAX_FALL);
        SqueakMove(s);
        if (SqueakOnGround(s)) {
            SqueakStop(s);
            SqueakSetState(s, BODY_CRAB_ENTRY);
            return TRUE;
        }
        break;
    case BODY_CRAB_ENTRY:   // 0x021080F0;
        BUSY(s) = 1;
        SqueakPlay(s, ANIM_CRAB_WALK);
        COUNT(s) = CRAB_TIME;
        CRAB_COUNT(s) = 0;
        SqueakSetState(s, BODY_CRAB);
        return TRUE;
    case BODY_CRAB:         // 0x02108024: scuttling, turning at walls, a bubble at Kirby every CRAB_SHOT frames;
        if (COUNT(s) < 1) {
            SqueakSetState(s, BODY_REFORM_ENTRY);
            return TRUE;
        }
        COUNT(s)--;
        if (s->stateTimer % 10 == 0) SqueakSfx(s, 0x176);
        s->vx = CRAB_SPEED;
        if (SqueakWallAhead(s) || EdgeAhead(s)) SqueakTurnAround(s);
        Gravity(s);
        SqueakMove(s);
        if (CRAB_COUNT(s) < 1) {
            struct SqueakObject *b = SqueakSpawn(s, SQUEAK_BUBOO_BUBBLE, BUBBLE_AIMED, 0, 0);

            if (b) B_PARENT(b) = s;
            CRAB_COUNT(s) = CRAB_SHOT;
        } else {
            CRAB_COUNT(s)--;
        }
        break;
    case BODY_REFORM_ENTRY: // 0x02108384;
        BUSY(s) = 11;
        SqueakPlay(s, ANIM_CRAB_STOP);
        SqueakSetState(s, BODY_REFORM_1);
        return TRUE;
    case BODY_REFORM_1:     // 0x021082B8;
        if (s->animEnded) {
            SqueakPlay(s, ANIM_CRAB_BLOW);
            SqueakSfx(s, 0x174);
            SqueakSetState(s, BODY_REFORM_2);
            return TRUE;
        }
        SqueakMove(s);
        break;
    case BODY_REFORM_2:     // 0x02108204;
        if (s->animEnded) {
            SqueakPlay(s, ANIM_REFORM);
            SqueakSetState(s, BODY_REFORM_3);
            return TRUE;
        }
        SqueakMove(s);
        break;
    }
    return FALSE;
}

// Hurt(); 0x021088D4: a hit it lives through, 36 frames after the last: the cluster loses bubbles, the fifth time it bursts;
static void Hurt(struct SqueakObject *s) {
    SAFE(s) = SAFE_TIME;
    if (!CLUSTER(s)) return;
    if (++HIT_COUNT(s) < HITS) {
        SqueakSetState(s, BODY_HURT_ENTRY);
    } else {
        HIT_COUNT(s) = 0;
        CLUSTER(s) = 0;
        SqueakSetState(s, BODY_BURST_ENTRY);
    }
}

// Pick(); 0x021054B4: little bubbles, a Waddle Dee or the circle; one picked 3 times before is swapped for another, half and half;
static void Pick(struct SqueakObject *s) {
    s32 roll = Roll();
    u8 kind = roll < 30 ? 0 : roll < 60 ? 1 : 2;

    if (kind == 0 && SHOOTS(s) > 2) {
        SHOOTS(s) = 0;
        kind = Roll() < 50 ? 1 : 2;
    } else if (kind == 1 && DROPS(s) > 2) {
        DROPS(s) = 0;
        kind = Roll() > 49 ? 2 : 0;
    } else if (kind == 2 && CIRCLES(s) > 2) {
        CIRCLES(s) = 0;
        kind = Roll() > 49 ? 1 : 0;
    }
    if (kind == 0) {
        s->brain = BRAIN_SHOOT;
        SHOOTS(s)++;
    } else if (kind == 1) {
        s->brain = BRAIN_DROP;
        DROPS(s)++;
    } else {
        s->brain = BRAIN_CIRCLE;
        CIRCLES(s)++;
    }
}

// Brain(); Its pad (0x02105830 and the states it leads to), one state a frame; while the crab is out, it lets go (0x0210586C);
static void Brain(struct SqueakObject *s) {
    s32 roll;

    if (!CLUSTER(s)) s->brain = BRAIN_IDLE;
    switch (s->brain) {
    case BRAIN_START:       // 0x02105830;
        SqueakFaceKirby(s, FACE_DISTANCE);
        BRAIN_TIMER(s) = 0;
        SHOOTS(s) = DROPS(s) = CIRCLES(s) = 0;
        s->brain = BRAIN_WAIT_PICK;
        break;
    case BRAIN_IDLE:        // 0x0210523C;
        s->padDir = s->padY = 0;
        s->padB = s->padA = 0;
        if (CLUSTER(s)) s->brain = BRAIN_WAIT_PICK;
        break;
    case BRAIN_WAIT_PICK:   // 0x021057CC;
        s->padDir = s->padY = 0;
        BRAIN_TIMER(s) = Roll() < 20 ? 0 : 60;
        s->brain = BRAIN_WAIT;
        break;
    case BRAIN_WAIT:        // 0x0210579C;
        s->padDir = s->padY = 0;
        if (BRAIN_TIMER(s) > 0) BRAIN_TIMER(s)--;
        else s->brain = BRAIN_WEAVE;
        break;
    case BRAIN_WEAVE:       // 0x02105680: A, 120, 180 or 210 frames;
        s->padDir = s->padY = 0;
        s->padA = 1;
        roll = Roll();
        BRAIN_TIMER(s) = roll < 30 ? 120 : roll < 60 ? 180 : 210;
        s->brain = BRAIN_WEAVING;
        break;
    case BRAIN_WEAVING:     // 0x0210563C: A let go, and once its body is still, the pick;
        s->padDir = s->padY = 0;
        if (BRAIN_TIMER(s) < 1) {
            s->padA = 0;
            if (BUSY(s) == 0) s->brain = BRAIN_PICK;
        } else {
            BRAIN_TIMER(s)--;
        }
        break;
    case BRAIN_PICK:
        Pick(s);
        break;
    case BRAIN_SHOOT:       // 0x0210577C: B;
        s->padDir = s->padY = 0;
        s->padB = 1;
        s->brain = BRAIN_SHOOT_WAIT;
        break;
    case BRAIN_SHOOT_WAIT:  // 0x02105700, 0x02105354: B let go; its body still, A again or B again, half and half;
    case BRAIN_DROP_WAIT:
        s->padDir = s->padY = 0;
        s->padB = 0;
        if (BUSY(s) == 0) s->brain = Roll() < 50 ? BRAIN_A_AGAIN : BRAIN_B_AGAIN;
        break;
    case BRAIN_DROP:        // 0x021053D0: B with up;
        s->padDir = 0;
        s->padY = 1;
        s->padB = 1;
        s->brain = BRAIN_DROP_WAIT;
        break;
    case BRAIN_CIRCLE:      // 0x02105434: A with up, 600 frames (20, 20, 60 in 100: all 600);
        s->padDir = s->padY = 0;
        s->padA = 1;
        roll = Roll();
        BRAIN_TIMER(s) = 600;
        s->brain = BRAIN_CIRCLING;
        break;
    case BRAIN_CIRCLING:    // 0x021053F0;
        s->padDir = 0;
        s->padY = 1;
        s->padA = 1;
        if (BRAIN_TIMER(s) < 1) {
            s->padDir = s->padY = 0;
            s->padA = 0;
            s->brain = BRAIN_WAIT_PICK;
        } else {
            BRAIN_TIMER(s)--;
        }
        break;
    case BRAIN_A_AGAIN:     // 0x02105328: A, 60 frames;
        s->padDir = s->padY = 0;
        s->padA = 1;
        BRAIN_TIMER(s) = 60;
        s->brain = BRAIN_A_AGAIN_WAIT;
        break;
    case BRAIN_A_AGAIN_WAIT:    // 0x021052F4: then the weave again;
        s->padDir = s->padY = 0;
        if (BRAIN_TIMER(s) < 1) {
            s->padA = 0;
            s->brain = BRAIN_WEAVE;
        } else {
            BRAIN_TIMER(s)--;
        }
        break;
    case BRAIN_B_AGAIN:     // 0x021052D4: B;
        s->padDir = s->padY = 0;
        s->padB = 1;
        s->brain = BRAIN_B_AGAIN_WAIT;
        break;
    case BRAIN_B_AGAIN_WAIT:    // 0x02105298: its body still, 60 frames' rest;
        s->padDir = s->padY = 0;
        s->padB = 0;
        if (BUSY(s) == 0) {
            BRAIN_TIMER(s) = 60;
            s->brain = BRAIN_REST;
        }
        break;
    case BRAIN_REST:        // 0x02105270;
        if (BRAIN_TIMER(s) > 0) BRAIN_TIMER(s)--;
        else s->brain = BRAIN_WAIT_PICK;
        break;
    }
}

static void BubooMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;
    s32 top;

    obj->kirby3 = FindTargetKirby(&obj->base);
    SqueakMidbossUpdate(s);
    // (KatAM's hit took HP: Squeak Squad's hit reaction, its own 36 frames safe after one);
    if (obj->unk80 < LAST_HP(s) && obj->unk80 > 0 && SAFE(s) == 0 && s->state != BODY_INTRO) Hurt(s);
    LAST_HP(s) = obj->unk80;
    Brain(s);
    SqueakRun(s, Body);
    SqueakFinishFrame(s);
    // (0x02108C4C: kept on the screen, its spot not above its top nor under its bottom);
    if (obj->unk80 > 0) {
        top = CameraY(s);
        if (obj->base.y < top) obj->base.y = top;
        if (obj->base.y > top + (DISPLAY_HEIGHT << 8)) obj->base.y = top + (DISPLAY_HEIGHT << 8);
    }
    if (SAFE(s) > 0) SAFE(s)--;
    if (s->stateTimer % 5 == 0 && obj->unk80 > 0) SqueakSfx(s, 0x173);
    SqueakAnimate(s);
    SqueakSetBoxes(s, &sBubooBoxData, HARM(s), &BOX(s));
}

/* BubooStart(); Its fight's start (and after a hit: it goes on, as KatAM's mini-bosses do): the first time, its intro (untouchable,
 * still, INTRO frames), then its body's start; */
void BubooStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, -1, BubooMain);
    if (s->started) return;
    SqueakPlay(s, ANIM_CLUSTER);
    SqueakStop(s);
    COUNT(s) = INTRO;
    CLUSTER(s) = 1;
    LAST_HP(s) = obj->unk80;
    obj->base.flags |= 0x200;
    SqueakSetState(s, BODY_INTRO);
}

// BubooInit(); Its box (buboo.rca's cluster), its bounds (KatAM's), facing left, its bob from its first frame;
void BubooInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -16, -26, 16, 2);
    ObjectSetBounds(&s->obj.base, -16, -26, 16, 0);
    HARM(s) = s->obj.base.unk68;
    s->obj.base.flags |= 1;
    CLUSTER(s) = 1;
    SqueakPlay(s, ANIM_CLUSTER);
}

/* Its bubbles. A big one's Waddle Dee is KatAM's (object 0x00, its spawn's walk, its room slot none): held, its own function and its
 * gravity, terrain, collisions and move kept off (HELD_FLAGS) till the bubble lets it go; */
static void HeldDee(struct Object *obj) {
}

// HeldAlive(); Its Waddle Dee still there and held by it;
static bool32 HeldAlive(struct SqueakObject *b) {
    struct Object *dee = B_HELD(b);

    return dee && dee->base.header.kind && dee->type == OBJ_WADDLE_DEE_1 && dee->unk78 == HeldDee && dee->base.parent == &b->obj
           && !(dee->base.flags & 0x1000);
}

// LetGo(); 0x021091EC's: its Waddle Dee let go, its own again;
static void LetGo(struct SqueakObject *b) {
    struct Object *dee = B_HELD(b);

    if (HeldAlive(b)) {
        dee->unk78 = B_HELD_FUNC(b);
        dee->base.flags = (dee->base.flags & ~HELD_FLAGS) | B_HELD_FLAGS(b);
        dee->base.xspeed = B_HELD_SPEED(b);
        dee->base.yspeed = 0;
        dee->base.parent = NULL;
    }
    B_HELD(b) = NULL;
}

// Hold(); Its Waddle Dee placed in it: its feet BIG_HELD pixels over its spot;
static void Hold(struct SqueakObject *b) {
    struct Object *dee = B_HELD(b);

    if (!HeldAlive(b)) {
        B_HELD(b) = NULL;
        return;
    }
    dee->base.x = b->obj.base.x;
    dee->base.y = b->obj.base.y - ((BIG_HELD + DEE_FEET) << 8);
}

// Burst(); 0x02109170: gone in a burst (touching Kirby), its Waddle Dee let go;
static void Burst(struct SqueakObject *b) {
    LetGo(b);
    SqueakSpawnEffect(b, SQUEAK_FX_BURST, 0, 0, FALSE);
    SqueakSfx(b, 0x135);
    b->obj.base.flags |= 0x1000;
}

// Gone(); Out of the stage (0x02209D00): gone (its Waddle Dee let go: Squeak Squad's would stay held);
static void Gone(struct SqueakObject *b) {
    LetGo(b);
    b->obj.base.flags |= 0x1000;
}

/* Aim(); 0x02109608: at Kirby, 1.5 along the angle of his distance's sizes (across and up: never down), toward him across as the crab
 * faces, slowing by AIM_DRAG/4096 of that a frame (KatAM's sine: the angle by halves, its tangent against his distance's); */
static void Aim(struct SqueakObject *b, struct Object *crab) {
    struct ObjectBase *base = &b->obj.base;
    struct Kirby *kirby = b->obj.kirby3;
    s32 dx = (kirby->base.x - base->x) >> 4, dy = (kirby->base.y - base->y) >> 4, lo = 0, hi = 0x100, mid, sn, cs;

    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    while (hi - lo > 1) {
        mid = (lo + hi) / 2;
        if (dy * gSineTable[mid + 0x100] > dx * gSineTable[mid]) lo = mid;
        else hi = mid;
    }
    sn = gSineTable[lo] >> 2;
    cs = gSineTable[lo + 0x100] >> 2;
    b->vx = (cs * AIM_SPEED + 0x800) >> 12;
    b->vy = (sn * AIM_SPEED + 0x800) >> 12;
    if (crab->base.flags & 1 ? kirby->base.x > base->x : kirby->base.x < base->x) b->vx = -b->vx;
    b->ax = -((AIM_DRAG * b->vx + 0x800) >> 12);
    b->ay = -((AIM_DRAG * b->vy + 0x800) >> 12);
}

/* MoveFree(); A little bubble's move: through everything, and not held in the stage (its flag 0x20000: the framework's move through
 * the terrain keeps a character inside the stage's edges), so it leaves it; */
static void MoveFree(struct SqueakObject *b) {
    struct ObjectBase *base = &b->obj.base;

    b->vx += b->ax;
    b->vy += b->ay;
    base->xspeed = SQUEAK_SPEED(base->flags & 1 ? -b->vx : b->vx);
    base->yspeed = SQUEAK_SPEED(b->vy);
    base->unk48 = base->x;
    base->unk4C = base->y;
    base->x += base->xspeed;
    base->y -= base->yspeed;
    base->unk62 = 0;
    b->terrain = 0;
    b->moved = TRUE;
}

// Popped(); An aimed or big bubble's flight ends: the terrain met (a wall either side, a ceiling) or the floor;
static bool32 Popped(struct SqueakObject *b) {
    return (b->terrain & (1 | 2 | 8)) || SqueakOnGround(b);
}

static bool32 Bubble(struct SqueakObject *b) {
    struct SqueakObject *crab;
    s32 angle;

    switch (b->state) {
    case BUBBLE_START:      // 0x02109E00's: by its kind;
        if (b->variant == BUBBLE_SMALL) {
            // 0x0210999C: off at 2.4 at 44 to 143 degrees, falling, through everything;
            angle = (Rand16() * 10 >> 16) + 4;
            angle = angle * 11 << 12;
            b->vx = (Cosine(angle) * SMALL_SPEED + 0x800) >> 12;
            b->vy = (Sine(angle) * SMALL_SPEED + 0x800) >> 12;
            b->ay = -BUBBLE_GRAVITY;
            SqueakThroughTerrain(b, TRUE);
            SqueakPlay(b, ANIM_SMALL);
            SqueakSfx(b, 0x174);
            SqueakSetState(b, BUBBLE_FLY);
        } else if (b->variant == BUBBLE_AIMED) {
            // 0x02109844;
            SqueakThroughTerrain(b, FALSE);
            SqueakStop(b);
            SqueakPlay(b, ANIM_SMALL);
            SqueakSetState(b, BUBBLE_FOLLOW);
        } else {
            // 0x02109420: its Waddle Dee (0x02109F18: facing as it, Kirby's way);
            SqueakThroughTerrain(b, FALSE);
            SqueakStop(b);
            b->ay = -41;
            SqueakPlay(b, ANIM_BIG);
            B_TIMER(b) = 0;
            if (b->obj.kirby3->base.x < b->obj.base.x ? !(b->obj.base.flags & 1) : (b->obj.base.flags & 1)) SqueakTurnAround(b);
            B_HELD(b) = CreateObjTemplateAndObj(b->obj.base.unk56, 1, 0x24, b->obj.base.x >> 8, b->obj.base.y >> 8, 0, 0x1F, 0, 0,
                                                OBJ_WADDLE_DEE_1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
            if (B_HELD(b)) {
                struct Object *dee = B_HELD(b);

                B_HELD_FUNC(b) = dee->unk78;
                B_HELD_FLAGS(b) = dee->base.flags & HELD_FLAGS;
                B_HELD_SPEED(b) = dee->base.xspeed;
                dee->unk78 = HeldDee;
                dee->base.flags = (dee->base.flags | HELD_FLAGS) & ~1;
                dee->base.flags |= b->obj.base.flags & 1;
                dee->base.xspeed = dee->base.yspeed = 0;
                dee->base.parent = &b->obj;
                Hold(b);
            }
            SqueakSetState(b, BUBBLE_CARRY);
        }
        return TRUE;
    case BUBBLE_FLY:        // 0x02109948: falling no faster than 1.8; out of the stage, gone;
        if (b->vy < BUBBLE_FALL) {
            b->vy = BUBBLE_FALL;
            b->ay = 0;
        }
        MoveFree(b);
        if (!SqueakInLevel(b)) Gone(b);
        break;
    case BUBBLE_FOLLOW:     // 0x02109608: from 8 pixels over the crab (its wait set to 0 first: at once), at Kirby;
        crab = B_PARENT(b);
        if (!crab || !crab->obj.base.header.kind || (crab->obj.base.flags & 0x1000)) {
            Gone(b);
            break;
        }
        b->obj.base.x = crab->obj.base.x;
        b->obj.base.y = crab->obj.base.y - SQUEAK_SPEED(AIM_UP);
        Aim(b, &crab->obj);
        B_TIMER(b) = AIM_TIME;
        SqueakSetState(b, BUBBLE_AIM);
        return TRUE;
    case BUBBLE_AIM:        // 0x021094E4: AIM_TIME frames, or the terrain or floor met: it pops;
        SqueakMove(b);
        if (!Popped(b) && B_TIMER(b) > 0) {
            if (!SqueakInLevel(b)) Gone(b);
        } else {
            SqueakStop(b);
            SqueakPlay(b, ANIM_SMALL_POP);
            SqueakSetState(b, BUBBLE_POP_SMALL);
        }
        B_TIMER(b)--;
        break;
    case BUBBLE_POP_SMALL:  // 0x02109928, 0x021091CC: its pop over, gone;
    case BUBBLE_POPPING:
        SqueakMove(b);
        if (b->animEnded) b->obj.base.flags |= 0x1000;
        break;
    case BUBBLE_CARRY:      // 0x021092F0: sinking no faster than 1.8, its Waddle Dee in it; the terrain or floor met, it pops;
        if (b->vy < BUBBLE_FALL) {
            b->vy = BUBBLE_FALL;
            b->ay = 0;
        }
        SqueakMove(b);
        Hold(b);
        if (Popped(b)) {
            SqueakStop(b);
            SqueakSetState(b, BUBBLE_POP_BIG);
            return TRUE;
        }
        if (!SqueakInLevel(b)) Gone(b);
        break;
    case BUBBLE_POP_BIG:    // 0x021091EC: its Waddle Dee let go;
        SqueakStop(b);
        SqueakPlay(b, ANIM_BIG_POP);
        SqueakSfx(b, 0x175);
        LetGo(b);
        b->obj.base.unk68 = 0;
        b->obj.base.flags |= 0x200;
        SqueakSetState(b, BUBBLE_POPPING);
        return TRUE;
    }
    return FALSE;
}

// BubooBubbleWatch(); Its pre-callback: KatAM's hit beat it or took it out of its own state: its Waddle Dee let go;
static void BubooBubbleWatch(struct Object *obj);

static void BubooBubbleMain(struct Object *obj) {
    struct SqueakObject *b = (struct SqueakObject *)obj;

    obj->kirby3 = FindTargetKirby(&obj->base);
    // (0x02109D78: its frames, to 2; from then, touching Kirby it bursts, a big one inhaled pops);
    if (B_AGE(b) < 2) B_AGE(b)++;
    if (B_AGE(b) >= 2 && (b->state == BUBBLE_FLY || b->state == BUBBLE_AIM || b->state == BUBBLE_CARRY)) {
        if (SqueakTouchingKirby(b)) {
            Burst(b);
            return;
        }
        if (b->variant == BUBBLE_BIG && b->state == BUBBLE_CARRY && NidlInhaler(&obj->base)) SqueakSetState(b, BUBBLE_POP_BIG);
    }
    SqueakRun(b, Bubble);
    SqueakFinishFrame(b);
    SqueakAnimate(b);
    SqueakSetBoxes(b, &sBubooBoxData, b->state == BUBBLE_POP_SMALL || b->state == BUBBLE_POPPING ? 0 : B_HARM(b), &B_BOX(b));
}

static void BubooBubbleWatch(struct Object *obj) {
    struct SqueakObject *b = (struct SqueakObject *)obj;

    if (B_HELD(b) && (obj->unk78 != BubooBubbleMain || obj->unk80 <= 0 || (obj->base.flags & 0x1000))) LetGo(b);
}

// BubooBubbleStart(); Its start (and after a hit: a hit ends it, its HP 1);
void BubooBubbleStart(struct Object *obj) {
    struct SqueakObject *b = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, -1, BubooBubbleMain);
    obj->unk7C = BubooBubbleWatch;
    if (b->started) return;
    SqueakSetState(b, BUBBLE_START);
}

/* BubooBubbleInit(); Its box (buboo.rca's little bubble's, or the big one's), its bounds (KatAM's); Kirby's body doesn't hurt any (it
 * bursts instead: KatAM's unk5C 0x80, as Bubble Head's bubble's); a big one isn't inhaled (it pops); an aimed one's crab is set by the
 * crab (B_PARENT); */
void BubooBubbleInit(struct SqueakObject *b) {
    b->obj.base.unk5C |= 0x80;
    if (b->variant == BUBBLE_BIG) {
        sub_0803E2B0(&b->obj.base, -13, -29, 13, -3);
        ObjectSetBounds(&b->obj.base, -13, -29, 13, -3);
        b->obj.base.unk5C |= 0x20;
    } else {
        sub_0803E2B0(&b->obj.base, -10, -20, 10, 0);
        ObjectSetBounds(&b->obj.base, -10, -20, 10, 0);
    }
    B_HARM(b) = b->obj.base.unk68;
    B_PARENT(b) = NULL;
    B_HELD(b) = NULL;
    B_AGE(b) = 0;
    SqueakPlay(b, b->variant == BUBBLE_BIG ? ANIM_BIG : ANIM_SMALL);
}
