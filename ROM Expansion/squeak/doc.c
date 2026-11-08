/* Doc; ported from Kirby: Squeak Squad (USA), the Squeak Squad's inventor in his flying saucer, enemy 0x43 (its body 0x020DD82C, vtable
 * 0x0217C874; the things it makes 0x020DF354, by 0x020DF530), as SQUEAK_DOC and SQUEAK_DOC_THING. Values from param/PrmDB02Ml0.kgb (its
 * body's and its things'), PrmDB02Sl0.kgb (HP 120); speeds 20.12 a frame, Y up. Its body switches between two sprites (its +0xE4):
 * enemy/doc and enemy/doc_at1 (its robot arm's attacks); here one character (KatAMExpansionSqueak.cs's ExtraFile), its saucer and its
 * arm's parts in other colors two layers of their own (Layers); doc_at1.mca's composite animations as Doc's own part (with its
 * layers) and their other parts as held effects (SQUEAK_FX_DOC_*: the gun's ball and rings, the claw's bomb, the cannon's smoke).
 * • It flies: along a curve (0x020DAC0C, 0x020DA91C: a quadratic Bezier from where it is to its target, its control point beside the
 *   middle by 0.6 of the other axis' distance, 1.2 a frame (its pace's), facing its target; there within 8 pixels, or its time over),
 *   then hovers (0x020DA774, 0x020DA590: 1 or 2 circles of its spot, 1.6 out of the first, 0.6 round, 0.05 a frame's turn; none after
 *   3 hovers in a row), facing Kirby.
 * • Its routine (0x020D8F14, its start): no chest in its hands, it flies beside Kirby (0x020D78F8: 64 pixels to his side, level or 16
 *   up, the side toward the screen's middle 9 times in 10), hovers, then (0x020D8934) any closed chest in the room (its score, 20 less
 *   its route's parts away plus its bonus, 40 or more: here any chest) it goes for it (0x020D87B8, 0x020D8708, 0x020D8608: there, it
 *   takes it on a touch), else it attacks (0x020D84F4: flies beside Kirby, or half the time to a corner of the screen (0x020D76EC: by
 *   its last spot's odds), hovers 1 or 2 circles, then (0x020D8418) 5% hovers again, 40% its gun (only beside Kirby: level, its bolt
 *   straight ahead; 16 up, 9 times in 10 down at 35 degrees), 35% its claw (above Kirby, then 1 or 2 more stops 64 to either side,
 *   then over him: the bomb it drops falls onto his height and bursts), 20% its cannon (52 pixels beside Kirby, 24 down: smoke 120
 *   frames, 73 pixels ahead), then hovers and starts over.
 * • A chest in its hands (8 pixels under it, swaying), it flies home (0x020D8000, 0x020D7F8C, 0x020D7E5C: Squeak Squad's escape hole,
 *   here the spot it was placed at), stops 96 frames and is gone with it (0x020D7E10).
 * • Its pace (0x020DB328, every 60 frames): heading home, off the screen half its speed, Kirby near 1.4 times; else off the screen 2.6
 *   times, Kirby near half.
 * • Kirby's inhale: it can't be inhaled; it shakes on the spot (doc's anim 21: 0x020D7114, 0x020D7068), facing him, 8 frames on.
 * • Hits: a hit (not one that knocks it) flinches it (0x020DCDE8: its hurt pose, shaking 16 frames on the spot: here KatAM's hurt,
 *   held where it was hit; then hopped up 1.0 away, 1.9 up, falling at most 30 frames); its HP down to 90% the first time, it's
 *   knocked out of its saucer (0x020DC5F0: shaking 16 frames, KatAM's hurt as well; flung 4.0 away, 4.5 up off the screen, then falling back from the screen's top as the saucer's lid (doc's anim 13) over Kirby
 *   through the terrain to his height, landing (dropping its chest); 12 frames on, the lid bursts or flies off, a missile with it 60%
 *   of the rest, and Doc pops out (2.4 back, 3.6 up), lands, lies dizzy 120 frames, gets up, sparks, and its empty saucer flies in from
 *   the screen's top to pick it up (0x020DB7E4-0x020DB4C4, its thing 6); hit while dizzy: no damage, one time in five a treat. Each
 *   other hit, one time in five (not within 40 frames), a treat (0x020DAF6C: Squeak Squad's foods 10-14: KatAM's small food).
 * Not ported: its sounds, the screen's shakes, its elemental hurt poses, its knockback at HP 0 with 7 treats (KatAM's defeat instead),
 * its dizzy stars, its anger when Kirby carries the stage's chest (stage mode 2: doc_col_ang's palette, 2.6 times its speeds, 36 frames
 * untouchable after each hit), the stages' routes (here straight to its target), its leaving when the stage's Squeak chase is off. */
#include "../squeak.h"
#include "chest.h"
#include "random.h"
#include "constants/object_types.h"

// Animations (doc.cea's, then doc_at1.cea's: KatAMExpansionSqueak.cs's OnlyAnims, in this order);
enum {
    ANIM_HOVER, ANIM_POP, ANIM_HURT, ANIM_HURT_2, ANIM_LID, ANIM_LYING, ANIM_GET_UP, ANIM_FIX, ANIM_FIX_END, ANIM_BRACE, ANIM_TAKE,
    ANIM_GUN_UP, ANIM_GUN_AIM, ANIM_GUN_FIRE, ANIM_GUN2_UP, ANIM_GUN2_AIM, ANIM_GUN2_FIRE, ANIM_CLAW, ANIM_CLAW_DROP, ANIM_CLAW_BACK,
    ANIM_CANNON_UP, ANIM_CANNON_FIRE, ANIM_CANNON_HOLD, ANIM_CANNON_BACK,
};
// Its things' (doc_at1's 6, 13, 15, 16, 21, 29, then doc's 20, 19);
enum { THING_ANIM_BOLT, THING_ANIM_BOLT_LOW, THING_ANIM_MISSILE, THING_ANIM_MISSILE_DOWN, THING_ANIM_BOMB, THING_ANIM_BOMB_BURST,
       THING_ANIM_LID, THING_ANIM_SAUCER };
enum { THING_BOLT, THING_BOLT_LOW, THING_BOMB, THING_MISSILE, THING_LID_SHORT, THING_LID, THING_SAUCER, THING_BURST };

// PrmDB02Ml0.kgb (+0x2F0);
#define GUN_X 0x2C000                   // +0x0C..+0x2C: its things' spots from it;
#define GUN_Y 0x26000
#define GUN2_X 0x3A000
#define GUN2_Y 0x8000
#define BOMB_X 0x22000
#define BOMB_Y 0x8000
#define GRAVITY 0x19A                   // +0x3C (its things' +0x34 too);
#define KNOCK_X 0x4000                  // +0x48, +0x4C: flung out of its saucer;
#define KNOCK_Y 0x4800
#define LID_DROP 0x4000                 // +0x50, +0x54: the lid falling;
#define LID_FALL 0x4000
#define LID_SIT 12                      // +0x5C;
#define POP_X 0x299A                    // +0x60, +0x64: Doc out of the lid;
#define POP_Y 0x399A
#define POP_TIME 8                      // +0x68;
#define DIZZY_TIME 120                  // +0x6C;
#define TAKE_TIME 0x30                  // +0x70, +0x74;
#define HOLE_TIME 0x60
#define FLY_SPEED 0x1334                // +0x7C;
#define NEAR_X 0x40000                  // +0x80..+0x8C: its spots beside Kirby;
#define NEAR_Y 0x2000
#define NEAR_X2 0x40000
#define NEAR_Y2 0x10000
#define CORNER_LEFT 26                  // +0x90..+0x9C: its spots on the screen (DS: 26, 223 across, 56, 194 down);
#define CORNER_RIGHT 33                 // (from the screen's right);
#define CORNER_TOP 56
#define CORNER_BOTTOM 2                 // (under the screen's bottom);
#define CLAW_WAIT_1 12                  // +0xA4..+0xB8: its attacks' waits;
#define CLAW_WAIT_2 30
#define CLAW_WAIT_3 30
#define CANNON_WAIT_1 60
#define CANNON_WAIT_2 120
#define CANNON_WAIT_3 30
#define HOVER_R 0x99A                   // +0xBC, +0xC0;
#define HOVER_TURN 0xCD
#define PACE_TIME 60                    // +0xC8;
#define FAST 0x299A                     // +0xD4, +0xD8, +0xE4, +0xE8;
#define SLOW 0x800
#define HOME_FAST 0x1667
#define HOME_SLOW 0x800
#define BOLT_SPEED 0xD000               // Its things': +0x1C, +0x20, +0x24 (degrees);
#define BOLT_LOW_ANGLE 325
#define MAX_FALL 0x2800
// The code's;
#define HOVER_START 0x199A              // 0x020DA774's first speed out;
#define CURVE 0x99A                     // 0x020D8C74's;
#define CARRY_Y -0x8000                 // 0x022014CC: its chest 8 pixels under it;
#define TREAT_WAIT 40
#define TREAT_CHANCE 20
#define FLINCH_X 0x1000
#define FLINCH_Y 0x1E66
#define FLINCH_FALL 30
#define BRACE_END 8
#define GUN_SHOT 10                     // 0x020D9FEC: its 11th frame;
#define GUN_TIME 58                     // doc_at1.mca's composites' frames;
#define CLAW_TIME 66
#define NEAR_KIRBY 64

enum {
    BODY_START, BODY_FLY_ENTRY, BODY_FLY, BODY_FLY_NEAR_ENTRY, BODY_FLY_NEAR, BODY_HOVER, BODY_THINK_KIRBY, BODY_HOVER_THEN_PICK,
    BODY_PICK_PATH, BODY_ATTACK_PATH, BODY_CHEST_PATH, BODY_CHEST_ROUTE, BODY_CHEST_NEAR, BODY_PICK_ATTACK, BODY_HOVER_AGAIN,
    BODY_AFTER, BODY_GUN_PICK, BODY_CANNON_GO, BODY_CANNON_PICK, BODY_CLAW_GO, BODY_CLAW_STEP, BODY_CLAW_LAST, BODY_CLAW_START,
    BODY_HOME, BODY_HOME_GO, BODY_AT_HOME, BODY_GONE_HOME, BODY_GUN_UP, BODY_GUN_AIM, BODY_GUN_FIRE, BODY_CLAW, BODY_CLAW_DROP,
    BODY_CLAW_WAIT, BODY_CLAW_BACK, BODY_CANNON_UP, BODY_CANNON_FIRE, BODY_CANNON_HOLD, BODY_CANNON_BACK, BODY_TAKE, BODY_TAKE_PAUSE,
    BODY_BRACE_ENTRY, BODY_BRACE, BODY_FLINCH_ENTRY, BODY_FLINCH_RISE, BODY_FLINCH_TOP, BODY_FLINCH_FALL,
    BODY_KNOCK_ENTRY, BODY_KNOCK_FLY, BODY_KNOCK_TOP, BODY_LID_DROP, BODY_LID_FALL, BODY_LID_LANDED, BODY_LID_SIT,
    BODY_POP_RISE, BODY_POP_FALL, BODY_LYING, BODY_GET_UP, BODY_FIX, BODY_WAIT_SAUCER,
};
// Its spots (+0x2BC): beside Kirby 1-4, the screen's corners 5-8, the route 9, a chest 10;
enum { SPOT_NONE, SPOT_LEFT, SPOT_RIGHT, SPOT_LEFT_UP, SPOT_RIGHT_UP, SPOT_BOTTOM_LEFT, SPOT_BOTTOM_RIGHT, SPOT_TOP_LEFT,
       SPOT_TOP_RIGHT, SPOT_ROUTE, SPOT_CHEST };
enum { PACE_NONE, PACE_FAST, PACE_SLOW, PACE_HOME_FAST, PACE_HOME_SLOW };

#define COUNT(s) (((s16 *)&(s)->vars[0])[0])
#define PAUSE(s) (((s16 *)&(s)->vars[0])[1])        // +0x2B8 (and +0x184);
#define LAST_FLAGS(s) ((s)->vars[1])
#define NEXT(s) (((u8 *)&(s)->vars[2])[0])          // +0x250: after a flight or hover;
#define LOOPS(s) (((u8 *)&(s)->vars[2])[1])         // +0x2A4: the hover's circles left (and the flight's first frames);
#define HOVERS(s) (((u8 *)&(s)->vars[2])[2])        // +0x2A8;
#define PHASE(s) (((u8 *)&(s)->vars[2])[3])         // +0x198: 0, 1 once knocked out of its saucer;
#define CHEST(s) (*(struct Chest **)&(s)->vars[3])  // +0x16C;
#define SWAY(s) (&(s)->vars[4])                     // +0x248, +0x24C;
#define SPOT_X(s) ((s)->vars[6])                    // +0x130, +0x134: its hover's middle, where it shakes;
#define SPOT_Y(s) ((s)->vars[7])
#define HOME_X(s) ((s)->vars[8])
#define HOME_Y(s) ((s)->vars[9])
#define HARM(s) ((s)->vars[10])
#define LAST_UNKC(s) ((s)->vars[11])
#define REGRAB(s) (((u8 *)&(s)->vars[12])[0])       // +0x1F8;
#define TREAT_TIMER(s) (((u8 *)&(s)->vars[12])[1])  // +0x2DC;
#define PACE(s) (((u8 *)&(s)->vars[12])[2])         // +0x1E4;
#define PACE_TIMER(s) (((u8 *)&(s)->vars[12])[3])   // +0x224;
#define FLAGS(s) (((u8 *)&(s)->vars[13])[0])
#define BOX(s) (((u8 *)&(s)->vars[13])[1])
#define SPOT(s) (((u8 *)&(s)->vars[13])[2])         // +0x2BC;
#define REPEATS(s) (((u8 *)&(s)->vars[13])[3])      // +0x2C4;
#define SIDE(s) (((u8 *)&(s)->vars[14])[0])         // +0x2C0;
#define GUN(s) (((u8 *)&(s)->vars[14])[1])          // +0x2A0;
#define BRACE_IDLE(s) (((u8 *)&(s)->vars[14])[2])   // +0x2D8;
#define DIZZY(s) (((u8 *)&(s)->vars[14])[3])        // +0x240;
#define LAST_HP(s) (((s16 *)&(s)->vars[15])[0])
#define FLY_N(s) (((s16 *)&(s)->vars[15])[1])       // +0x284;
#define P0_X(s) ((s)->vars[16])                     // +0x264..+0x278: its curve;
#define P0_Y(s) ((s)->vars[17])
#define P1_X(s) ((s)->vars[18])
#define P1_Y(s) ((s)->vars[19])
#define P2_X(s) ((s)->vars[20])
#define P2_Y(s) ((s)->vars[21])
#define FLY_DUR(s) (((s16 *)&(s)->vars[22])[0])     // +0x288;
#define HOVER_PHASE(s) (((u8 *)&(s)->vars[22])[2])  // +0x2AC;
#define FLY_HOLD(s) (((u8 *)&(s)->vars[22])[3])     // +0x2A4 in a flight: its first frames, no arrival;
#define HOVER_X(s) ((s)->vars[23])                  // +0x290, +0x294;
#define HOVER_Y(s) ((s)->vars[26])
#define HOVER_VX(s) (((s16 *)&(s)->vars[24])[0])    // +0x298, +0x29C;
#define HOVER_VY(s) (((s16 *)&(s)->vars[24])[1])
#define AWAY(s) (((u8 *)&(s)->vars[25])[0])         // +0x190: pushed left (Kirby on its right);
#define GOING_HOME(s) (((u8 *)&(s)->vars[25])[1])   // +0x22C;
// Its flags;
#define FLAG_HURT 1
#define FLAG_KNOCK 2
#define FLAG_INHALED 4                  // +0x2D0;
#define FLAG_SAUCER 8                   // Its saucer's on its way;
// Its things;
#define T_COUNT(s) ((s)->vars[0])       // +0x130;
#define T_HARM(s) ((s)->vars[1])
#define T_BOX(s) (*(u8 *)&(s)->vars[2])
#define T_DOC(s) (*(struct SqueakObject **)&(s)->vars[3])   // +0x12C;
#define T_TARGET_Y(s) ((s)->vars[4])    // +0x138;

// Boxes (doc.rca's; doc_at1.rca's composites' and things': its animations 5 on from the .cea's);
static const struct SqueakBox sDocBoxes[] = {
    { { -17, -33, 17, 1 }, TRUE, 0 },
    { { -17, -33, 17, 1 }, FALSE, 0 },
    { { -10, -13, 8, 1 }, FALSE, 0 },
    { { -17, -40, 73, 1 }, TRUE, 0 },
};
#define BOX_SMOKE 3
static const struct SqueakBoxFrame sDocHarms[] = { { 1, 0 } };
static const struct SqueakBoxFrame sDocHarmless[] = { { 1, 1 } };
static const struct SqueakBoxFrame sDocSmall[] = { { 1, 2 } };
static const struct SqueakBoxFrame sDocSmoke[] = { { 1, BOX_SMOKE } };
static const struct SqueakBoxAnim sDocBoxAnims[] = {
    { sDocHarms, 1 }, { sDocHarmless, 1 }, { sDocHarmless, 1 }, { sDocHarmless, 1 }, { sDocHarmless, 1 }, { sDocSmall, 1 },
    { sDocSmall, 1 }, { sDocSmall, 1 }, { sDocSmall, 1 }, { sDocHarmless, 1 }, { sDocHarms, 1 },
    { sDocHarms, 1 }, { sDocHarms, 1 }, { sDocHarms, 1 }, { sDocHarms, 1 }, { sDocHarms, 1 }, { sDocHarms, 1 }, { sDocHarms, 1 },
    { sDocHarms, 1 }, { sDocHarms, 1 }, { sDocHarms, 1 }, { sDocSmoke, 1 }, { sDocHarms, 1 }, { sDocHarms, 1 },
};
static const struct SqueakBoxes sDocBoxData = { sDocBoxes, sDocBoxAnims, ARRAY_COUNT(sDocBoxAnims) };
static const struct SqueakBox sThingBoxes[] = {
    { { -6, -5, 6, 4 }, TRUE, 0 },
    { { -12, -9, 0, 0 }, TRUE, 0 },
    { { -6, -5, 6, 4 }, FALSE, 0 },
    { { -5, -7, 6, 0 }, FALSE, 0 },
    { { -7, -6, 6, 6 }, TRUE, 0 },
    { { -20, -20, 20, 20 }, TRUE, 0 },
    { { -17, -33, 17, 1 }, FALSE, 0 },
};
static const struct SqueakBoxFrame sThing0[] = { { 1, 0 } };
static const struct SqueakBoxFrame sThing1[] = { { 1, 1 } };
static const struct SqueakBoxFrame sThing2[] = { { 1, 2 } };
static const struct SqueakBoxFrame sThing3[] = { { 1, 3 } };
static const struct SqueakBoxFrame sThing4[] = { { 1, 4 } };
static const struct SqueakBoxFrame sThing5[] = { { 1, 5 } };
static const struct SqueakBoxFrame sThing6[] = { { 1, 6 } };
static const struct SqueakBoxAnim sThingBoxAnims[] = {
    { sThing0, 1 }, { sThing1, 1 }, { sThing2, 1 }, { sThing3, 1 }, { sThing4, 1 }, { sThing5, 1 }, { sThing6, 1 }, { sThing6, 1 },
};
static const struct SqueakBoxes sThingBoxData = { sThingBoxes, sThingBoxAnims, ARRAY_COUNT(sThingBoxAnims) };

static void DocMain(struct Object *obj);
void sub_0809B6A8(struct Object *);     // KatAM's hurt (object.c's, static: build.sh links it);

static u32 Random(void) {
    return Rand16() * 100 >> 16;
}

static struct Chest *Held(struct SqueakObject *s) {
    return SqueakHeldChest(&CHEST(s));
}

static void Carry(struct SqueakObject *s) {
    SqueakCarryChest(s, &CHEST(s), SWAY(s), 0, CARRY_Y);
}

static s32 KirbyX(struct SqueakObject *s) {
    return SQ_X(s->obj.kirby3);
}

static s32 KirbyY(struct SqueakObject *s) {
    return SQ_Y(s->obj.kirby3);
}

// The screen's left and top (Squeak Squad's units; its camera +0x6C);
static s32 CamLeft(struct SqueakObject *s) {
    return gCurLevelInfo[s->obj.base.unk56].viewportPosition.x << 4;
}

static s32 CamTop(struct SqueakObject *s) {
    return -(gCurLevelInfo[s->obj.base.unk56].viewportPosition.y << 4);
}

static void SetPos(struct SqueakObject *s, s32 x, s32 y) {
    s->obj.base.x = x >> 4;
    s->obj.base.y = -y >> 4;
}

static void Still(struct SqueakObject *s) {
    s->vx = s->vy = s->ax = s->ay = s->maxVx = s->maxVy = s->fx = s->fy = 0;
}

// FaceToward(); Facing `x` (Squeak Squad's flag 4 left);
static void FaceToward(struct SqueakObject *s, s32 x) {
    s32 here = SQ_X(&s->obj);

    if (here < x) {
        if (s->obj.base.flags & 1) SqueakTurnAround(s);
    } else if (!(s->obj.base.flags & 1)) {
        SqueakTurnAround(s);
    }
}

// FxMul(); Squeak Squad's 20.12 multiply ((a * b + 0x800) >> 12 on the 64-bit product), in 32-bit pieces (KatAM has no __muldi3);
static s32 FxMul(s32 a, s32 b) {
    bool32 minus = (a < 0) != (b < 0);
    u32 ua = a < 0 ? -(u32)a : (u32)a, ub = b < 0 ? -(u32)b : (u32)b;
    u32 al = ua & 0xFFFF, ah = ua >> 16, bl = ub & 0xFFFF, bh = ub >> 16;
    u32 lo = al * bl, mid1 = ah * bl, mid2 = al * bh, hi = ah * bh, t;

    t = lo + (mid1 << 16); hi += (mid1 >> 16) + (t < lo); lo = t;
    t = lo + (mid2 << 16); hi += (mid2 >> 16) + (t < lo); lo = t;
    // rounding: (p + 0x800) >> 12 for p >= 0, -((|p| + 0x7FF) >> 12) for p < 0;
    t = lo + (minus ? 0x7FF : 0x800); hi += t < lo; lo = t;
    t = (hi << 20) | (lo >> 12);
    return minus ? -(s32)t : (s32)t;
}


// FxSqrt(); 0x021430D0: the square root of a 20.12 value;
static s32 FxSqrt(s32 v) {
    u64 n = (u64)(v < 0 ? 0 : v) << 12, r = 0, bit = (u64)1 << 62;

    while (bit > n) bit >>= 2;
    while (bit) {
        if (n >= r + bit) {
            n -= r + bit;
            r = (r >> 1) + bit;
        } else {
            r >>= 1;
        }
        bit >>= 2;
    }
    return (s32)r;
}

// OnScreen(); 0x020A2D0C: its spot in the camera's view (0x02209D00's 4);
static bool32 OnScreen(struct SqueakObject *s) {
    struct LevelInfo *level = &gCurLevelInfo[s->obj.base.unk56];
    s32 x = s->obj.base.x - level->viewportPosition.x, y = s->obj.base.y - level->viewportPosition.y;

    return x >= 0 && x <= DISPLAY_WIDTH * 0x100 && y >= 0 && y <= DISPLAY_HEIGHT * 0x100;
}

static bool32 KirbyNear(struct SqueakObject *s) {
    s32 dx = (s->obj.kirby3->base.x - s->obj.base.x) >> 8, dy = (s->obj.kirby3->base.y - s->obj.base.y) >> 8;

    return OnScreen(s) && dx * dx + dy * dy < NEAR_KIRBY * NEAR_KIRBY;
}

// Pace(); 0x020DB328: every 60 frames;
static void Pace(struct SqueakObject *s) {
    if (PACE_TIMER(s) > 0) {
        PACE_TIMER(s)--;
        return;
    }
    PACE_TIMER(s) = PACE_TIME;
    if (GOING_HOME(s)) PACE(s) = !OnScreen(s) ? PACE_HOME_SLOW : KirbyNear(s) ? PACE_HOME_FAST : PACE_NONE;
    else PACE(s) = !OnScreen(s) ? PACE_FAST : KirbyNear(s) ? PACE_SLOW : PACE_NONE;
}

// Speed(); 0x020DB3D0: its flight's speed at its pace;
static s32 Speed(struct SqueakObject *s) {
    static const s32 sTimes[] = { 0x1000, FAST, SLOW, HOME_FAST, HOME_SLOW };

    return FxMul(FLY_SPEED, sTimes[PACE(s)]);
}

/* FlyStart(); 0x020DAC0C: a curve from (x0, y0) to (x2, y2), its control point beside the middle by `curve` of the other axis'
 * distance (as the code's four cases), its frames the distance at its speed; */
static void FlyStart(struct SqueakObject *s, s32 x0, s32 y0, s32 x2, s32 y2, s32 curve) {
    s32 dx = x2 > x0 ? x2 - x0 : x0 - x2, dy = y2 > y0 ? y2 - y0 : y0 - y2, ox = FxMul(dy, curve), oy = FxMul(dx, curve);

    P0_X(s) = x0;
    P0_Y(s) = y0;
    P2_X(s) = x2;
    P2_Y(s) = y2;
    FLY_N(s) = 0;
    FLY_HOLD(s) = 6;
    // (0x02143178, the 20.12 division: the distance over the speed, its whole frames);
    FLY_DUR(s) = FxSqrt(FxMul(dx, dx) + FxMul(dy, dy)) / Speed(s);
    if (x0 <= x2 && y0 <= y2) {
        P1_X(s) = x0 + dx / 2 - ox;
        P1_Y(s) = oy + y0 + dy / 2;
    } else if (x0 <= x2) {
        P1_X(s) = ox + x0 + dx / 2;
        P1_Y(s) = oy + (y0 - dy / 2);
    } else if (y0 <= y2) {
        P1_X(s) = ox + (x0 - dx / 2);
        P1_Y(s) = oy + y0 + dy / 2;
    } else {
        P1_X(s) = (x0 - dx / 2) - ox;
        P1_Y(s) = oy + (y0 - dy / 2);
    }
}

// FlyStep(); 0x020DA91C: its spot on the curve this frame; TRUE once there (within 8 pixels, after its first 6 frames) or its time's over;
static bool32 FlyStep(struct SqueakObject *s) {
    s32 t = FLY_DUR(s) ? (FLY_N(s) << 12) / FLY_DUR(s) : 0x1000, u = 0x1000 - t, a = FxMul(t, t), b = FxMul(t << 1, u), c = FxMul(u, u), x, y;

    if (P0_X(s) < P2_X(s)) FaceToward(s, SQ_X(&s->obj) + 1);
    else if (P2_X(s) < P0_X(s)) FaceToward(s, SQ_X(&s->obj) - 1);
    Still(s);
    x = FxMul(a, P2_X(s)) + FxMul(c, P0_X(s)) + FxMul(b, P1_X(s));
    y = FxMul(a, P2_Y(s)) + FxMul(c, P0_Y(s)) + FxMul(b, P1_Y(s));
    SetPos(s, x, y);
    if (FLY_HOLD(s) > 0) {
        FLY_HOLD(s)--;
    } else if (P2_X(s) - 0x8000 < x && x < P2_X(s) + 0x8000 && P2_Y(s) - 0x8000 < y && y < P2_Y(s) + 0x8000) {
        return TRUE;
    }
    return ++FLY_N(s) >= FLY_DUR(s);
}

// Fly(); A flight to (x, y) (0x020D8C74: its curve 0.6; `near`: 0x020D8AF8, straight, done over Kirby), then `next`;
static void Fly(struct SqueakObject *s, s32 x, s32 y, bool8 near, u8 next) {
    FlyStart(s, SQ_X(&s->obj), SQ_Y(&s->obj), x, y, near ? 0 : CURVE);
    NEXT(s) = next;
    SqueakPlay(s, ANIM_HOVER);
    Still(s);
    SqueakSetState(s, near ? BODY_FLY_NEAR : BODY_FLY);
}

// Hover(); 0x020DA774: circling its spot `loops` times (none after 3 hovers in a row), facing Kirby, then `next`;
static void Hover(struct SqueakObject *s, u8 loops, u8 next) {
    SqueakPlay(s, ANIM_HOVER);
    Still(s);
    if (HOVERS(s) < 3) {
        LOOPS(s) = loops;
        HOVERS(s)++;
    } else {
        LOOPS(s) = 0;
        HOVERS(s) = 0;
    }
    FaceToward(s, KirbyX(s));
    HOVER_PHASE(s) = 0;
    SPOT_X(s) = SQ_X(&s->obj);
    SPOT_Y(s) = SQ_Y(&s->obj);
    HOVER_X(s) = HOVER_Y(s) = 0;
    HOVER_VX(s) = HOVER_START;
    HOVER_VY(s) = 0;
    NEXT(s) = next;
    SqueakSetState(s, BODY_HOVER);
}

// HoverStep(); 0x020DA590: its turn round the circle; FALSE once its circles are done;
static bool32 HoverStep(struct SqueakObject *s) {
    if (LOOPS(s) < 1) return FALSE;
    switch (HOVER_PHASE(s)) {
    case 0:
        HOVER_VX(s) -= HOVER_TURN;
        HOVER_VY(s) += HOVER_TURN;
        if (HOVER_VX(s) < 1 && HOVER_VY(s) >= HOVER_R) {
            HOVER_VX(s) = 0;
            HOVER_VY(s) = HOVER_R;
            HOVER_PHASE(s) = 1;
        }
        break;
    case 1:
        HOVER_VX(s) -= HOVER_TURN;
        HOVER_VY(s) -= HOVER_TURN;
        if (HOVER_VX(s) <= -HOVER_R && HOVER_VY(s) < 1) {
            HOVER_VX(s) = -HOVER_R;
            HOVER_VY(s) = 0;
            HOVER_PHASE(s) = 2;
        }
        break;
    case 2:
        HOVER_VX(s) += HOVER_TURN;
        HOVER_VY(s) -= HOVER_TURN;
        if (HOVER_VX(s) >= 0 && HOVER_VY(s) <= -HOVER_R) {
            HOVER_VX(s) = 0;
            HOVER_VY(s) = -HOVER_R;
            HOVER_PHASE(s) = 3;
        }
        break;
    case 3:
        HOVER_VX(s) += HOVER_TURN;
        HOVER_VY(s) += HOVER_TURN;
        if (HOVER_VX(s) >= HOVER_R && HOVER_VY(s) >= 0) {
            HOVER_VX(s) = HOVER_R;
            HOVER_VY(s) = 0;
            HOVER_PHASE(s) = 0;
            LOOPS(s)--;
        }
        break;
    }
    HOVER_X(s) += HOVER_VX(s);
    HOVER_Y(s) += HOVER_VY(s);
    SetPos(s, SPOT_X(s) + HOVER_X(s), SPOT_Y(s) + HOVER_Y(s));
    return TRUE;
}

static bool32 AnyChest(struct SqueakObject *s, struct Chest **chest) {
    bool8 any = FALSE;

    *chest = SqueakNearestChest(s, &any);
    return *chest != NULL;
}

/* BesideKirby(); 0x020D78F8: a spot beside Kirby by the odds of his side of the screen's middle (its code's: the middle 96 above the
 * screen's top, so he's always under it); */
static void BesideKirby(struct SqueakObject *s, s32 *x, s32 *y) {
    static const u8 sOdds[4][4] = { { 0, 5, 5, 90 }, { 5, 0, 90, 5 }, { 5, 90, 0, 5 }, { 90, 5, 5, 0 } };
    s32 cx = CamLeft(s) + (DISPLAY_WIDTH / 2 << 12), cy = CamTop(s) + 0x60000, kx = KirbyX(s), ky = KirbyY(s);
    u32 r;
    u8 row = 0, i, sum = 0;

    if ((kx < cx + 0x8000 && cx - 0x8000 < kx) || (ky < cy + 0x8000 && cy - 0x8000 < ky)) {
        r = Random();
        row = r < 25 ? 0 : r < 50 ? 1 : r < 75 ? 2 : 3;
    } else if (kx < cx && cy < ky) {
        row = 0;
    } else if (cx < kx && cy < ky) {
        row = 1;
    } else if (kx < cx && ky < cy) {
        row = 2;
    } else if (cx < kx && ky < cy) {
        row = 3;
    }
    r = Random();
    for (i = 0; i < 4; i++) {
        sum += sOdds[row][i];
        if (r < sum) break;
    }
    // (its spots 3, 4, 1, 2 in that order);
    switch (i) {
    case 0: SPOT(s) = SPOT_LEFT_UP; *x = kx - NEAR_X2; *y = ky + NEAR_Y2; break;
    case 1: SPOT(s) = SPOT_RIGHT_UP; *x = kx + NEAR_X2; *y = ky + NEAR_Y2; break;
    case 2: SPOT(s) = SPOT_LEFT; *x = kx - NEAR_X; *y = ky - NEAR_Y; break;
    default: SPOT(s) = SPOT_RIGHT; *x = kx + NEAR_X; *y = ky - NEAR_Y; break;
    }
}

/* Corner(); 0x020D76EC: a corner of the screen by its last spot's odds (its first: its side of the screen's middle; the screen's
 * edges as KatAM's, the DS's distances from them kept); */
static void Corner(struct SqueakObject *s, s32 *x, s32 *y) {
    static const u8 sOdds[8][4] = { { 0, 50, 50, 0 }, { 80, 0, 0, 20 }, { 50, 0, 0, 50 }, { 0, 20, 80, 0 }, { 50, 30, 20, 0 },
                                    { 30, 60, 10, 0 }, { 20, 0, 50, 30 }, { 10, 0, 30, 60 } };
    s32 cx = CamLeft(s) + (DISPLAY_WIDTH / 2 << 12), cy = CamTop(s) + 0x60000, dx = SQ_X(&s->obj), dy = SQ_Y(&s->obj);
    s32 left = CamLeft(s) + (CORNER_LEFT << 12), right = CamLeft(s) + ((DISPLAY_WIDTH - CORNER_RIGHT) << 12);
    s32 top = CamTop(s) - (CORNER_TOP << 12), bottom = CamTop(s) - ((DISPLAY_HEIGHT + CORNER_BOTTOM) << 12);
    u32 r;
    u8 row, i, sum = 0;

    switch (SPOT(s)) {
    case SPOT_NONE: row = cx < dx && dy <= cy ? 0 : 2; break;
    case SPOT_LEFT: row = 5; break;
    case SPOT_RIGHT: row = 7; break;
    case SPOT_LEFT_UP: row = 4; break;
    case SPOT_RIGHT_UP: row = 6; break;
    case SPOT_BOTTOM_LEFT: row = 0; break;
    case SPOT_BOTTOM_RIGHT: row = 1; break;
    case SPOT_TOP_LEFT: row = 2; break;
    case SPOT_TOP_RIGHT: row = 3; break;
    default: row = 0; break;
    }
    r = Random();
    for (i = 0; i < 4; i++) {
        sum += sOdds[row][i];
        if (r < sum) break;
    }
    // (its spots 7, 8, 5, 6 in that order);
    switch (i) {
    case 0: SPOT(s) = SPOT_TOP_LEFT; *x = left; *y = top; break;
    case 1: SPOT(s) = SPOT_TOP_RIGHT; *x = right; *y = top; break;
    case 2: SPOT(s) = SPOT_BOTTOM_LEFT; *x = left; *y = bottom; break;
    default: SPOT(s) = SPOT_BOTTOM_RIGHT; *x = right; *y = bottom; break;
    }
}

// Thing(); 0x020D71F8: one of its things at its offset from it (ahead as it faces);
static void Thing(struct SqueakObject *s, u8 kind, s32 dx, s32 dy) {
    struct SqueakObject *t = SqueakSpawn(s, SQUEAK_DOC_THING, kind, dx, dy);

    if (t) T_DOC(t) = s;
}

// Part(); A composite animation's other part (its effect) at (x, y) pixels from it, as it faces right, y down;
static void Part(struct SqueakObject *s, u8 effect, s32 x, s32 y, bool8 held) {
    s32 dx = (s->obj.base.flags & 1 ? -x : x) << 12, dy = -y << 12;

    if (held) SqueakHoldEffect(s, effect, dx, dy);
    else SqueakSpawnEffect(s, effect, dx, dy, FALSE);
}

// Treat(); 0x020DAF6C: one hit in five (not within 40 frames of the last), a food flung up from it;
static void Treat(struct SqueakObject *s) {
    struct Object *obj = &s->obj;

    if (TREAT_TIMER(s) > 0 || Random() >= TREAT_CHANCE) return;
    TREAT_TIMER(s) = TREAT_WAIT;
    CreateObjTemplateAndObj(obj->base.unk56, 1, 0x24, obj->base.x >> 8, obj->base.y >> 8, 0, 0x1F, 0, 0, OBJ_SMALL_FOOD,
                            RandLessThan(5), 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

// Gravity(); Its vtable +0x40 (-2.5, -0.1): on the ground still, else falling;
static void Gravity(struct SqueakObject *s) {
    if (SqueakOnGround(s)) {
        s->vy = s->ay = 0;
        return;
    }
    s->ay = -GRAVITY;
    if (s->vy < -MAX_FALL) {
        s->vy = -MAX_FALL;
        s->ay = 0;
    }
}

static void CapFall(struct SqueakObject *s) {
    if (s->vy < -MAX_FALL + 1) {
        s->vy = -MAX_FALL;
        s->ay = 0;
    }
}

static bool32 Body(struct SqueakObject *s) {
    s32 x, y;
    u32 r;
    struct Chest *chest;

    switch (s->state) {
    case BODY_START:        // 0x020D8F14: through the terrain, flying; a chest in its hands, home; else beside Kirby;
        SqueakThroughTerrain(s, TRUE);
        SqueakPlay(s, ANIM_HOVER);
        Still(s);
        SqueakSetState(s, Held(s) ? BODY_HOME : BODY_THINK_KIRBY);
        return TRUE;
    case BODY_FLY:          // 0x020D8C20 (and 0x020D8D94);
        if (FlyStep(s)) SqueakSetState(s, NEXT(s));
        Carry(s);
        break;
    case BODY_FLY_NEAR:     // 0x020D8A58: over Kirby (within 32 pixels across), or there;
        if (KirbyX(s) - 0x20000 < SQ_X(&s->obj) && SQ_X(&s->obj) < KirbyX(s) + 0x20000) SqueakSetState(s, NEXT(s));
        else if (FlyStep(s)) SqueakSetState(s, NEXT(s));
        Carry(s);
        break;
    case BODY_HOVER:        // 0x020DA590;
        if (!HoverStep(s)) {
            Still(s);
            SqueakSetState(s, NEXT(s));
            return TRUE;
        }
        Carry(s);
        break;
    case BODY_THINK_KIRBY:  // 0x020D8888: beside Kirby, a hover, then 0x020D8934;
        BesideKirby(s, &x, &y);
        Fly(s, x, y, FALSE, BODY_HOVER_THEN_PICK);
        return TRUE;
    case BODY_HOVER_THEN_PICK:  // 0x020D89BC;
        Hover(s, Random() < 50 ? 1 : 2, BODY_PICK_PATH);
        return TRUE;
    case BODY_PICK_PATH:    // 0x020D8934: a chest worth it (its score 40 on: here any closed one) or Kirby;
        SqueakSetState(s, AnyChest(s, &chest) ? BODY_CHEST_PATH : BODY_ATTACK_PATH);
        return TRUE;
    case BODY_ATTACK_PATH:  // 0x020D84F4: a chest about, it starts over; else beside Kirby or to a corner, then its attack's pick;
        if (Held(s) || AnyChest(s, &chest)) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        if (Random() < 50) BesideKirby(s, &x, &y);
        else Corner(s, &x, &y);
        Fly(s, x, y, FALSE, BODY_PICK_ATTACK);
        return TRUE;
    case BODY_CHEST_PATH:   // 0x020D87B8: to the chest;
        if (Held(s) || !AnyChest(s, &chest)) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        SqueakChestFeet(chest, &x, &y);
        SPOT(s) = SPOT_CHEST;
        Fly(s, x, y, FALSE, BODY_CHEST_ROUTE);
        return TRUE;
    case BODY_CHEST_ROUTE:  // 0x020D8708 (the same part of the stage: here always) and 0x020D8680;
        if (!AnyChest(s, &chest)) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        SqueakChestFeet(chest, &x, &y);
        SPOT(s) = SPOT_CHEST;
        Fly(s, x, y, FALSE, BODY_CHEST_NEAR);
        return TRUE;
    case BODY_CHEST_NEAR:   // 0x020D8608;
        if (!AnyChest(s, &chest)) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        Fly(s, SQ_X(&s->obj), SQ_Y(&s->obj), FALSE, BODY_CHEST_PATH);
        return TRUE;
    case BODY_PICK_ATTACK:  // 0x020D8418: a hover of 1 or 2 circles, then 5% another, 40% the gun, 35% the claw, 20% the cannon;
        r = Random();
        Hover(s, Random() < 50 ? 1 : 2, r < 5 ? BODY_HOVER_AGAIN : r < 45 ? BODY_GUN_PICK : r < 80 ? BODY_CLAW_GO : BODY_CANNON_GO);
        return TRUE;
    case BODY_HOVER_AGAIN:  // 0x020D83F0;
        Hover(s, 1, BODY_AFTER);
        return TRUE;
    case BODY_AFTER:        // 0x020D8040;
        Hover(s, Random() < 50 ? 1 : 2, BODY_START);
        return TRUE;
    case BODY_GUN_PICK:     // 0x020D838C, 0x020DA4E0: beside Kirby only: level, its straight shot; 16 up, 9 times in 10 its low one;
        LOOPS(s) = 0;
        r = Random();
        if (SPOT(s) < SPOT_LEFT || SPOT(s) > SPOT_RIGHT_UP) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        GUN(s) = SPOT(s) <= SPOT_RIGHT || r < 10 ? 0 : 1;
        NEXT(s) = BODY_AFTER;
        FaceToward(s, KirbyX(s));
        SqueakPlay(s, GUN(s) ? ANIM_GUN2_UP : ANIM_GUN_UP);
        SqueakSetState(s, BODY_GUN_UP);
        return TRUE;
    case BODY_CANNON_GO:    // 0x020D810C, 0x020D7374: 52 pixels beside Kirby (on the screen's middle's side of him... as the code: away from it), 24 down;
        LOOPS(s) = 0;
        SPOT(s) = SPOT_NONE;
        x = KirbyX(s) + (SQ_X(&s->obj) < CamLeft(s) + (DISPLAY_WIDTH / 2 << 12) ? -0x34000 : 0x34000);
        Fly(s, x, KirbyY(s) - 0x18000, FALSE, BODY_CANNON_PICK);
        return TRUE;
    case BODY_CANNON_PICK:  // 0x020D80A8, 0x020D9B10, 0x020D99D4;
        LOOPS(s) = 0;
        NEXT(s) = BODY_AFTER;
        FaceToward(s, KirbyX(s));
        SqueakPlay(s, ANIM_CANNON_UP);
        COUNT(s) = 0;
        SqueakSetState(s, BODY_CANNON_UP);
        return TRUE;
    case BODY_CLAW_GO:      // 0x020D82DC, 0x020D7474: 32 over Kirby; its side as it faces, 1 or 2 more stops;
        LOOPS(s) = 0;
        SPOT(s) = SPOT_NONE;
        SIDE(s) = s->obj.base.flags & 1 ? 0 : 1;
        REPEATS(s) = Random() < 50 ? 0 : 1;
        Fly(s, KirbyX(s), KirbyY(s) + 0x20000, FALSE, BODY_CLAW_STEP);
        return TRUE;
    case BODY_CLAW_STEP:    // 0x020D8234, 0x020D73F8: 64 to its side of Kirby, 32 over him, then the other side;
        LOOPS(s) = 0;
        SPOT(s) = SPOT_NONE;
        x = KirbyX(s) + (SIDE(s) ? 0x40000 : -0x40000);
        FlyStart(s, SQ_X(&s->obj), SQ_Y(&s->obj), x, KirbyY(s) + 0x20000, 0);
        if (REPEATS(s) < 1) NEXT(s) = BODY_CLAW_LAST;
        else {
            NEXT(s) = BODY_CLAW_STEP;
            REPEATS(s)--;
        }
        SIDE(s) = !SIDE(s);
        SqueakPlay(s, ANIM_HOVER);
        Still(s);
        SqueakSetState(s, BODY_FLY);
        return TRUE;
    case BODY_CLAW_LAST:    // 0x020D81C8: toward its side till over Kirby;
        LOOPS(s) = 0;
        SPOT(s) = SPOT_NONE;
        x = KirbyX(s) + (SIDE(s) ? 0x40000 : -0x40000);
        Fly(s, x, KirbyY(s) + 0x20000, TRUE, BODY_CLAW_START);
        return TRUE;
    case BODY_CLAW_START:   // 0x020D8170, 0x020D9E68: its claw (doc_at1.mca's composite 0: its claw arm, the bomb in it);
        LOOPS(s) = 0;
        NEXT(s) = BODY_AFTER;
        FaceToward(s, KirbyX(s));
        SqueakPlay(s, ANIM_CLAW);
        Part(s, SQUEAK_FX_DOC_BOMB, 0, 0, TRUE);
        SPOT_X(s) = SQ_X(&s->obj);
        SPOT_Y(s) = SQ_Y(&s->obj);
        COUNT(s) = 0;
        SqueakSetState(s, BODY_CLAW);
        return TRUE;
    case BODY_HOME:         // 0x020D8000, 0x020D7F8C (its hole's part of the stage: here always the same), 0x020D7F34;
        GOING_HOME(s) = TRUE;
        SPOT(s) = SPOT_NONE;
        FlyStart(s, SQ_X(&s->obj), SQ_Y(&s->obj), HOME_X(s), HOME_Y(s), 0);
        NEXT(s) = BODY_AT_HOME;
        SqueakPlay(s, ANIM_HOVER);
        Still(s);
        SqueakSetState(s, BODY_FLY);
        return TRUE;
    case BODY_AT_HOME:      // 0x020D7E5C: within 8 pixels of its hole, a pause, then gone with its chest; else on;
        x = SQ_X(&s->obj);
        y = SQ_Y(&s->obj);
        if (HOME_X(s) - 0x8000 < x && x < HOME_X(s) + 0x8000 && HOME_Y(s) - 0x8000 < y && y < HOME_Y(s) + 0x8000) {
            NEXT(s) = BODY_GONE_HOME;
            PAUSE(s) = HOLE_TIME;
            SqueakSetState(s, BODY_TAKE);
            return TRUE;
        }
        SqueakSetState(s, BODY_HOME);
        return TRUE;
    case BODY_GONE_HOME:    // 0x020D7E10;
        if (Held(s)) CHEST(s)->obj.base.flags |= 0x1000;
        CHEST(s) = NULL;
        s->obj.base.flags |= 0x1000;
        break;
    case BODY_GUN_UP:       // 0x020DA1F8: its arm up, then aiming;
        if (s->animEnded) {
            SqueakPlay(s, GUN(s) ? ANIM_GUN2_AIM : ANIM_GUN_AIM);
            SqueakSetState(s, BODY_GUN_AIM);
        }
        break;
    case BODY_GUN_AIM:      // 0x020DA0A0: a loop, then the shot (doc_at1.mca's composite 1 or 2);
        if (s->animEnded) {
            if (GUN(s)) {
                SqueakPlay(s, ANIM_GUN2_FIRE);
                Part(s, SQUEAK_FX_DOC_BALL_LOW, 0, 0, FALSE);
                Part(s, SQUEAK_FX_DOC_RINGS_LOW, 2, 2, FALSE);
            } else {
                SqueakPlay(s, ANIM_GUN_FIRE);
                Part(s, SQUEAK_FX_DOC_BALL, -1, 0, FALSE);
                Part(s, SQUEAK_FX_DOC_RINGS, 0, 0, FALSE);
            }
            COUNT(s) = 0;
            SqueakSetState(s, BODY_GUN_FIRE);
        }
        break;
    case BODY_GUN_FIRE:     // 0x020D9FEC: its 11th frame, the bolt; its composite's frames over, on;
        if (COUNT(s) >= GUN_TIME) {
            SqueakSetState(s, NEXT(s));
            return TRUE;
        }
        if (COUNT(s)++ == GUN_SHOT) {
            if (GUN(s)) Thing(s, THING_BOLT_LOW, GUN2_X, GUN2_Y);
            else Thing(s, THING_BOLT, GUN_X, GUN_Y);
        }
        break;
    case BODY_CLAW:         // 0x020D9D68: its composite's 66 frames, 12 more, then the bomb dropped;
        if (COUNT(s)++ >= CLAW_TIME + CLAW_WAIT_1) {
            SqueakEndHeldEffect(s);
            SqueakPlay(s, ANIM_CLAW_DROP);
            Thing(s, THING_BOMB, BOMB_X, BOMB_Y);
            COUNT(s) = 0;
            SqueakSetState(s, BODY_CLAW_DROP);
        }
        break;
    case BODY_CLAW_DROP:    // 0x020D9CCC, 0x020D9BDC: 30 frames after its animation, 30 more, then its arm back;
        if (s->animEnded && COUNT(s)++ >= CLAW_WAIT_2 + CLAW_WAIT_3) {
            SqueakPlay(s, ANIM_CLAW_BACK);
            SqueakSetState(s, BODY_CLAW_BACK);
        }
        break;
    case BODY_CLAW_BACK:    // 0x020D9B5C;
    case BODY_CANNON_BACK:  // 0x020D966C;
        if (s->animEnded) {
            SqueakSetState(s, NEXT(s));
            return TRUE;
        }
        break;
    case BODY_CANNON_UP:    // 0x020D98D4: 60 frames after its animation, its smoke (doc_at1.mca's composite 3);
        if (s->animEnded && COUNT(s)++ >= CANNON_WAIT_1) {
            SqueakPlay(s, ANIM_CANNON_FIRE);
            Part(s, SQUEAK_FX_DOC_SMOKE, 29, -28, TRUE);
            COUNT(s) = 0;
            SqueakSetState(s, BODY_CANNON_FIRE);
        }
        break;
    case BODY_CANNON_FIRE:  // 0x020D97E4: 120 frames of smoke (its box on 73 pixels ahead), then still 30, then its cannon back;
        if (COUNT(s)++ >= CANNON_WAIT_2) {
            SqueakEndHeldEffect(s);
            SqueakPlay(s, ANIM_CANNON_HOLD);
            COUNT(s) = 0;
            SqueakSetState(s, BODY_CANNON_HOLD);
        }
        break;
    case BODY_CANNON_HOLD:  // 0x020D96EC;
        if (COUNT(s)++ >= CANNON_WAIT_3) {
            SqueakPlay(s, ANIM_CANNON_BACK);
            SqueakSetState(s, BODY_CANNON_BACK);
        }
        break;
    case BODY_TAKE:         // 0x020D9298: its take pose, still, PAUSE frames, then NEXT;
        SqueakPlay(s, ANIM_TAKE);
        Still(s);
        SqueakSetState(s, BODY_TAKE_PAUSE);
        return TRUE;
    case BODY_TAKE_PAUSE:   // 0x020D922C;
        if (PAUSE(s) < 0) {
            PAUSE(s) = 0;
            SqueakSetState(s, NEXT(s));
            return TRUE;
        }
        PAUSE(s)--;
        Carry(s);
        break;
    case BODY_BRACE_ENTRY:  // 0x020D7114;
        SqueakPlay(s, ANIM_BRACE);
        Still(s);
        SqueakSetState(s, BODY_BRACE);
        return TRUE;
    case BODY_BRACE:        // 0x020D7068: facing Kirby; 8 frames after the inhale, it starts over;
        FaceToward(s, KirbyX(s));
        Carry(s);
        if (!(FLAGS(s) & FLAG_INHALED)) {
            if (BRACE_IDLE(s) > BRACE_END - 1) {
                SqueakSetState(s, BODY_START);
                break;
            }
            BRACE_IDLE(s)++;
        }
        FLAGS(s) &= ~FLAG_INHALED;
        break;
    case BODY_FLINCH_ENTRY: // 0x020DCDE8: its hurt pose (one of two), its 16 frames' shake KatAM's hurt (DocWatch), then flung away;
    case BODY_KNOCK_ENTRY:  // 0x020DC5F0, 0x020DCCBC, 0x020DC4C0;
        SqueakPlay(s, Random() < 50 ? ANIM_HURT : ANIM_HURT_2);
        Still(s);
        // (away from Kirby: forward when it faces that way);
        if (s->state == BODY_FLINCH_ENTRY) {
            s->vx = AWAY(s) == !!(s->obj.base.flags & 1) ? FLINCH_X : -FLINCH_X;
            s->vy = FLINCH_Y;
            s->ay = -GRAVITY;
            SqueakSetState(s, BODY_FLINCH_RISE);
        } else {
            s->vx = AWAY(s) == !!(s->obj.base.flags & 1) ? KNOCK_X : -KNOCK_X;
            s->vy = KNOCK_Y;
            SqueakSetState(s, BODY_KNOCK_FLY);
        }
        return TRUE;
    case BODY_FLINCH_RISE:  // 0x020DCB34;
        SqueakMove(s);
        if (s->vy < 0) SqueakSetState(s, BODY_FLINCH_TOP);
        Carry(s);
        break;
    case BODY_FLINCH_TOP:   // 0x020DC97C;
        CapFall(s);
        if (s->animEnded) {
            COUNT(s) = FLINCH_FALL;
            SqueakSetState(s, BODY_FLINCH_FALL);
            return TRUE;
        }
        SqueakMove(s);
        Carry(s);
        break;
    case BODY_FLINCH_FALL:  // 0x020DC8F0: on the ground, or 30 frames on, it starts over;
        CapFall(s);
        SqueakMove(s);
        if (SqueakOnGround(s) || COUNT(s)-- < 1) SqueakSetState(s, BODY_START);
        Carry(s);
        break;
    case BODY_KNOCK_FLY:    // 0x020DC2A0: off the screen (32 pixels past its sides, or over its top);
        x = SQ_X(&s->obj);
        if ((s->vx < 0) == !(s->obj.base.flags & 1) ? x <= CamLeft(s) - 0x20000 : CamLeft(s) + ((DISPLAY_WIDTH + 32) << 12) <= x) {
            SqueakSetState(s, BODY_KNOCK_TOP);
        } else if (CamTop(s) <= SQ_Y(&s->obj)) {
            SqueakSetState(s, BODY_KNOCK_TOP);
        } else {
            SqueakMove(s);
            Carry(s);
            break;
        }
        // (fall through)
    case BODY_KNOCK_TOP:    // 0x020DC104: the lid falling from the screen's top over Kirby, through the terrain to his height;
        SqueakPlay(s, ANIM_LID);
        Still(s);
        SetPos(s, KirbyX(s), CamTop(s));
        SPOT_X(s) = KirbyX(s);
        SPOT_Y(s) = KirbyY(s);
        s->vy = -LID_DROP;
        SqueakSetState(s, BODY_LID_DROP);
        break;
    case BODY_LID_DROP:     // 0x020DBFCC: within 5 pixels of Kirby's height, solid;
        y = SQ_Y(&s->obj);
        if (SPOT_Y(s) - 0x5000 < y && y < SPOT_Y(s) + 0x5000) {
            SqueakThroughTerrain(s, FALSE);
            SqueakSetState(s, BODY_LID_FALL);
            return TRUE;
        }
        if (s->vy <= -LID_FALL) {
            s->vy = -LID_FALL;
            s->ay = 0;
        }
        SqueakMove(s);
        Carry(s);
        break;
    case BODY_LID_FALL:     // 0x020DBF40;
        if (SqueakOnGround(s)) {
            SqueakSetState(s, BODY_LID_LANDED);
            return TRUE;
        }
        if (s->vy <= -LID_FALL) {
            s->vy = -LID_FALL;
            s->ay = 0;
        }
        SqueakMove(s);
        Carry(s);
        break;
    case BODY_LID_LANDED:   // 0x020DBDE0: still, its chest dropped (10 touches before another);
        Still(s);
        if (Held(s)) {
            SqueakDropChest(s, &CHEST(s));
            REGRAB(s) = 10;
        }
        COUNT(s) = LID_SIT;
        SqueakSetState(s, BODY_LID_SIT);
        break;
    case BODY_LID_SIT:      // 0x020DBBF4: 12 frames, then the lid bursts (and a missile 60%) or flies off, and Doc pops out;
        if (COUNT(s)-- >= 0) {
            Gravity(s);
            SqueakMove(s);
            break;
        }
        if (Random() < 30) {
            Thing(s, THING_LID, 0, 0);
        } else {
            Thing(s, THING_BURST, 0, 0);
            if (Random() < 60) Thing(s, THING_MISSILE, 0, 0);
        }
        s->vx = -POP_X;
        s->vy = POP_Y;
        s->ay = -GRAVITY;
        SqueakPlay(s, ANIM_POP);
        SqueakSetState(s, BODY_POP_RISE);
        break;
    case BODY_POP_RISE:     // 0x020DBB70;
        if (s->vy < 0) {
            COUNT(s) = POP_TIME;
            SqueakSetState(s, BODY_POP_FALL);
            return TRUE;
        }
        CapFall(s);
        SqueakMove(s);
        break;
    case BODY_POP_FALL:     // 0x020DB7E4: landed, lying dizzy 120 frames;
        CapFall(s);
        SqueakMove(s);
        if (SqueakOnGround(s)) {
            SPOT_X(s) = SQ_X(&s->obj);
            SPOT_Y(s) = SQ_Y(&s->obj);
            SqueakPlay(s, ANIM_LYING);
            Still(s);
            COUNT(s) = DIZZY_TIME;
            DIZZY(s) = TRUE;
            SqueakSetState(s, BODY_LYING);
        }
        break;
    case BODY_LYING:        // 0x020DB6CC (hits drop treats: DocWatch); 3 pixels up and down a treat's 40 frames;
        if (COUNT(s)-- < 0) {
            SqueakPlay(s, ANIM_GET_UP);
            SqueakSetState(s, BODY_GET_UP);
            return TRUE;
        }
        Gravity(s);
        SqueakMove(s);
        break;
    case BODY_GET_UP:       // 0x020DB5F4;
        if (s->animEnded) {
            SqueakPlay(s, ANIM_FIX);
            SqueakSetState(s, BODY_FIX);
        }
        Gravity(s);
        SqueakMove(s);
        break;
    case BODY_FIX:          // 0x020DB4F4: its saucer called;
        if (s->animEnded) {
            Thing(s, THING_SAUCER, 0, 0);
            SqueakPlay(s, ANIM_FIX_END);
            FLAGS(s) |= FLAG_SAUCER;
            SqueakSetState(s, BODY_WAIT_SAUCER);
        }
        Gravity(s);
        SqueakMove(s);
        break;
    case BODY_WAIT_SAUCER:  // 0x020DB4C4: till the saucer's there (DocSaucerArrived);
        Gravity(s);
        SqueakMove(s);
        break;
    }
    return FALSE;
}

// DocSaucerArrived(); 0x020DE500: its saucer's picked it up: back in it;
static void DocSaucerArrived(struct SqueakObject *s) {
    FLAGS(s) &= ~FLAG_SAUCER;
    DIZZY(s) = FALSE;
    SqueakSetState(s, BODY_START);
}

/* DocMain(); Its body; its pace; Kirby's inhale reaching it, it shakes (its hit reaction to kinds 3-5, not while knocked or
 * dizzy); a closed chest it touches, it takes (0x020DD0AC's 0x1001: not within 10 touches of dropping one); */
static void DocMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;
    struct Chest *chest;

    if (TREAT_TIMER(s) > 0) TREAT_TIMER(s)--;
    Held(s);
    Pace(s);
    if (s->state < BODY_FLINCH_ENTRY && NidlInhaler(&obj->base)) {
        FLAGS(s) |= FLAG_INHALED;
        BRACE_IDLE(s) = 0;
        if (s->state != BODY_BRACE) SqueakSetState(s, BODY_BRACE_ENTRY);
    }
    SqueakRun(s, Body);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
    if (!Held(s) && s->state < BODY_FLINCH_ENTRY && (chest = SqueakTouchedChest(s)) != NULL) {
        if (REGRAB(s) > 0) {
            REGRAB(s)--;
        } else {
            SqueakTakeChest(&CHEST(s), SWAY(s), chest);
            PAUSE(s) = TAKE_TIME;
            NEXT(s) = BODY_START;
            SqueakSetState(s, BODY_TAKE);
        }
    }
    SqueakSetBoxes(s, &sDocBoxData, HARM(s), &BOX(s));
    /* The cannon's smoke (doc_at1.rca's harm box, 73 pixels ahead) and its body in KatAM's one box: Kirby's body (KatAM's attack
     * kind 0x80, its touch) doesn't hurt it then, the smoke only hurts him there, as on the DS (as Bubble Head's stream); */
    if (BOX(s) == BOX_SMOKE) obj->base.unk5C |= 0x80;
    else obj->base.unk5C &= ~0x80;
    /* (knocked out, till it lies dizzy: untouchable, KatAM's 0x200; Squeak Squad's +0x1C0 set to 2 by 0x020DC5F0, its hit handler
     * taking nothing then; dizzy, hits only roll treats: DocWatch); */
    if (s->state >= BODY_KNOCK_ENTRY && !DIZZY(s)) obj->base.flags |= 0x200;
    else obj->base.flags &= ~0x200;
    LAST_FLAGS(s) = obj->base.flags;
    LAST_UNKC(s) = obj->base.unkC;
    LAST_HP(s) = obj->unk80;
}

/* DocWatch(); Its pre-callback: hit while dizzy: no damage, a treat roll (KatAM's hurt undone); KatAM's hit put it out of its own
 * state, or it's beaten or gone: it drops its chest (then 10 touches before it takes one again); a hit's first frame: a treat roll,
 * and its HP down to 90% for the first time a knockout of its saucer once KatAM's hurt is over, else its flinch; */
static void DocWatch(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    if (DIZZY(s) && obj->unk78 == sub_0809B6A8 && !(FLAGS(s) & FLAG_HURT)) {
        Treat(s);
        obj->unk80 = LAST_HP(s);
        obj->base.flags = LAST_FLAGS(s);
        obj->base.unkC = LAST_UNKC(s);
        obj->unk98 = obj->unk9A = 0;
        obj->base.xspeed = obj->base.yspeed = 0;
        obj->base.objBase55 = 0;
        obj->unk78 = DocMain;
        return;
    }
    if (obj->unk78 != DocMain || obj->unk80 <= 0 || (obj->base.flags & 0x1000)) {
        if (Held(s)) {
            SqueakDropChest(s, &CHEST(s));
            REGRAB(s) = 10;
        }
        if (!(FLAGS(s) & FLAG_HURT) && !(obj->base.flags & 0x1000) && obj->unk80 > 0) {
            Treat(s);
            SPOT_X(s) = SQ_X(obj);
            SPOT_Y(s) = SQ_Y(obj);
            AWAY(s) = obj->kirby3->base.x > obj->base.x;
            if (PHASE(s) == 0 && obj->unk80 * 100 <= gSqueakEnemyHp[s->enemy] * 90) {
                PHASE(s) = 1;
                FLAGS(s) |= FLAG_KNOCK;
            }
        }
        FLAGS(s) |= FLAG_HURT;
        /* (KatAM's hurt in the place of its shake, 0x020DCCBC's and 0x020DC4C0's 16 frames: on the spot it was hit at, no knockback
         * nor gravity, as on the DS: its flinch or knockout after it starts from there); */
        if (obj->unk80 > 0 && !(obj->base.flags & 0x1000)) {
            obj->base.xspeed = obj->base.yspeed = 0;
            obj->unk98 = obj->unk9A = 0;
            SetPos(s, SPOT_X(s), SPOT_Y(s));
        }
    } else {
        FLAGS(s) &= ~FLAG_HURT;
    }
}

// DocGone(); Its end (squeak.c's SqueakDestroy): it drops its chest;
void DocGone(struct SqueakObject *s) {
    SqueakDropChest(s, &CHEST(s));
}

void DocStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, 0, DocMain);
    obj->unk7C = DocWatch;
    if (!s->started) {
        HOME_X(s) = SQ_X(obj);
        HOME_Y(s) = SQ_Y(obj);
        SqueakSetState(s, BODY_START);
        return;
    }
    SqueakEndHeldEffect(s);
    // (KatAM's hurt over: its own flinch, or its knockout; away from Kirby: the hit's side);
    FLAGS(s) &= ~FLAG_INHALED;
    if (FLAGS(s) & FLAG_SAUCER) {
        SqueakSetState(s, BODY_WAIT_SAUCER);
        return;
    }
    if (FLAGS(s) & FLAG_KNOCK) {
        FLAGS(s) &= ~FLAG_KNOCK;
        SqueakThroughTerrain(s, TRUE);
        SqueakSetState(s, BODY_KNOCK_ENTRY);
        return;
    }
    SqueakSetState(s, BODY_FLINCH_ENTRY);
}

/* DocInit(); Its box (doc.rca's, set each frame), facing left (0x020DD82C's flag 4); KatAM's inhale can't take it (unk5C 0x20), as
 * Squeak Squad's; flying through the terrain; */
void DocInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -17, -33, 17, 1);
    ObjectSetBounds(&s->obj.base, -10, -24, 10, 0);
    HARM(s) = s->obj.base.unk68;
    s->obj.base.flags |= 1;
    s->obj.base.unk5C |= 0x20;
    SqueakThroughTerrain(s, TRUE);
}

/* Its things (0x020DF354, made by 0x020DF530 at an offset from it, ahead as it faces): its bolts (0x020DF124, 0x020DEF84: 13.0 a
 * frame at 0 or 325 degrees, through everything, gone 48 pixels off the screen); its bomb (0x020DEE28, 0x020DECF8, 0x020DEC9C:
 * falling from its claw, through everything, onto Kirby's height (within 8 pixels), bursting 8 frames); a missile (0x020DEB7C,
 * 0x020DEA08, 0x020DE998: flung 2.8 forward, 2.6 up, turning at walls, lying where it lands, 300 frames in all, then bursting); the
 * saucer's lid (0x020DE7F8, 0x020DE6F8, 0x020DE688: flung 2.0 forward, 1.6 up, lying 300 frames, 16 for kind 4, then bursting); the
 * empty saucer (0x020DE528, 0x020DE36C: from the screen's top middle to Doc at 4.0 a frame each way, within 4 pixels it picks him
 * up); a burst (0x020DE324: the effect, gone); */
enum { THING_START, THING_BOLT_FLY, THING_BOMB_FALL, THING_BOMB_BURST, THING_TOSSED, THING_LYING, THING_SAUCER_FLY, THING_BURSTING };

static bool32 InView(struct SqueakObject *s) {
    struct LevelInfo *level = &gCurLevelInfo[s->obj.base.unk56];
    s32 x = s->obj.base.x - level->viewportPosition.x - (DISPLAY_WIDTH / 2 << 8);
    s32 y = s->obj.base.y - level->viewportPosition.y - (DISPLAY_HEIGHT / 2 << 8);

    return x > -((DISPLAY_WIDTH / 2 + 48) << 8) && x < (DISPLAY_WIDTH / 2 + 48) << 8 && y > -((DISPLAY_HEIGHT / 2 + 16) << 8)
           && y < (DISPLAY_HEIGHT / 2 + 48) << 8;
}

// Angle(); 0x0215CE20's sine table: a speed at `degrees` (0 ahead, counterclockwise);
static void Angle(struct SqueakObject *s, s32 speed, s32 degrees) {
    // (cos, sin of 0 and 325 degrees, 20.12);
    if (degrees == 0) {
        s->vx = speed;
        s->vy = 0;
    } else {
        s->vx = FxMul(speed, 0xD1B);
        s->vy = FxMul(speed, -0x92E);
    }
}

static bool32 ThingBody(struct SqueakObject *s) {
    struct SqueakObject *doc = T_DOC(s);
    s32 y;

    switch (s->state) {
    case THING_START:
        switch (s->variant) {
        case THING_BOLT:
        case THING_BOLT_LOW:
            SqueakPlay(s, s->variant == THING_BOLT ? THING_ANIM_BOLT : THING_ANIM_BOLT_LOW);
            Angle(s, BOLT_SPEED, s->variant == THING_BOLT ? 0 : BOLT_LOW_ANGLE);
            SqueakThroughTerrain(s, TRUE);
            SqueakSetState(s, THING_BOLT_FLY);
            break;
        case THING_BOMB:
            SqueakPlay(s, THING_ANIM_BOMB);
            T_TARGET_Y(s) = SQ_Y(s->obj.kirby3);
            s->ay = -GRAVITY;
            SqueakThroughTerrain(s, TRUE);
            SqueakSetState(s, THING_BOMB_FALL);
            break;
        case THING_MISSILE:
            SqueakPlay(s, THING_ANIM_MISSILE);
            s->vx = 0x2CCD;
            s->vy = 0x299A;
            s->ay = -GRAVITY;
            T_COUNT(s) = 300;
            SqueakSetState(s, THING_TOSSED);
            break;
        case THING_LID_SHORT:
        case THING_LID:
            SqueakPlay(s, THING_ANIM_LID);
            s->vx = 0x2000;
            s->vy = 0x199A;
            s->ay = -GRAVITY;
            T_COUNT(s) = s->variant == THING_LID ? 300 : 16;
            SqueakSetState(s, THING_TOSSED);
            break;
        case THING_SAUCER:
            SqueakPlay(s, THING_ANIM_SAUCER);
            SqueakThroughTerrain(s, TRUE);
            SetPos(s, CamLeft(s) + (DISPLAY_WIDTH / 2 << 12), CamTop(s));
            SqueakSetState(s, THING_SAUCER_FLY);
            break;
        default:
            SqueakSetState(s, THING_BURSTING);
            break;
        }
        return TRUE;
    case THING_BOLT_FLY:    // 0x020DF0F8;
        SqueakMove(s);
        if (!InView(s)) s->obj.base.flags |= 0x1000;
        break;
    case THING_BOMB_FALL:   // 0x020DECF8: within 8 pixels of Kirby's height, bursting;
        y = SQ_Y(&s->obj);
        if (T_TARGET_Y(s) - 0x8000 < y && y < T_TARGET_Y(s) + 0x8000) {
            T_COUNT(s) = 0;
            Still(s);
            SqueakSpawnEffect(s, SQUEAK_FX_BURST, 0, 0, FALSE);
            SqueakPlay(s, THING_ANIM_BOMB_BURST);
            SqueakSetState(s, THING_BOMB_BURST);
            break;
        }
        SqueakMove(s);
        if (!InView(s)) s->obj.base.flags |= 0x1000;
        break;
    case THING_BOMB_BURST:  // 0x020DEC9C: 8 frames;
        if (T_COUNT(s)++ >= 8) s->obj.base.flags |= 0x1000;
        break;
    case THING_TOSSED:      // 0x020DEA08, 0x020DE6F8: landed, lying;
        if (T_COUNT(s)-- < 1) {
            SqueakSetState(s, THING_BURSTING);
            return TRUE;
        }
        if (s->variant == THING_MISSILE && SqueakWallAhead(s)) SqueakTurnAround(s);
        CapFall(s);
        SqueakMove(s);
        if (!InView(s)) s->obj.base.flags |= 0x1000;
        if (SqueakOnGround(s)) {
            Still(s);
            if (s->variant == THING_MISSILE) SqueakPlay(s, THING_ANIM_MISSILE_DOWN);
            SqueakSetState(s, THING_LYING);
        }
        break;
    case THING_LYING:       // 0x020DE998, 0x020DE688;
        if (T_COUNT(s)-- < 1) {
            SqueakSetState(s, THING_BURSTING);
            return TRUE;
        }
        SqueakMove(s);
        if (!InView(s)) s->obj.base.flags |= 0x1000;
        break;
    case THING_SAUCER_FLY:  // 0x020DE36C: to Doc, 4.0 a frame each way; within 4 pixels, Doc's back in it;
        if (doc == NULL) {
            s->obj.base.flags |= 0x1000;
            break;
        }
        {
            s32 x = SQ_X(&s->obj), tx = SQ_X(&doc->obj), ty = SQ_Y(&doc->obj);

            y = SQ_Y(&s->obj);
            if (tx - 0x4000 < x && x < tx + 0x4000) x = tx;
            else {
                x += x < tx ? 0x4000 : -0x4000;
                if ((x < tx) == !!(s->obj.base.flags & 1)) SqueakTurnAround(s);
            }
            if (ty - 0x4000 < y && y < ty + 0x4000) y = ty;
            else y += y < ty ? 0x4000 : -0x4000;
            SetPos(s, x, y);
            if (tx - 0x4000 < x && x < tx + 0x4000 && ty - 0x4000 < y && y < ty + 0x4000) {
                s->obj.base.flags |= 0x1000;
                DocSaucerArrived(doc);
            }
        }
        break;
    case THING_BURSTING:    // 0x020DE324;
        SqueakSpawnEffect(s, SQUEAK_FX_BURST, 0, 0, FALSE);
        s->obj.base.flags |= 0x1000;
        break;
    }
    return FALSE;
}

static void DocThingMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    // (Doc gone: its saucer and bomb let go of it);
    if (T_DOC(s) && (T_DOC(s)->obj.base.flags & 0x1000)) T_DOC(s) = NULL;
    SqueakRun(s, ThingBody);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
    SqueakSetBoxes(s, &sThingBoxData, T_HARM(s), &T_BOX(s));
}

void DocThingStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, 0, DocThingMain);
    if (s->started) return;
    SqueakSetState(s, THING_START);
}

void DocThingInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -6, -5, 6, 4);
    ObjectSetBounds(&s->obj.base, -5, -5, 5, 5);
    T_HARM(s) = s->obj.base.unk68;
    /* (Kirby's body doesn't hurt it, nor shows its HP bar, 0x080365C8's flag 0x1000000: Squeak Squad's attack things go through him,
     * their hit handler 0x020B4964 taking no touch); */
    s->obj.base.unk5C |= 0x80;
    s->obj.base.flags |= 0x1000000;
}
