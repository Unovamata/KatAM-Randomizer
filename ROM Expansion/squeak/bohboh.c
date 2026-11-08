/* Bohboh (Squeak Squad's); ported from Kirby: Squeak Squad (USA), boss 0x3C (its body 0x02111370, vtable 0x02181770: +0x34 per frame
 * 0x02111568, +0x30 its hit 0x02110F98, +0x2C its touch 0x021111CC, +0x84 letting Kirby go hurt 0x02110D50; its states
 * 0x0210E16C-0x02110C7C), as SQUEAK_BOHBOH, a boss on the mid-bosses' framework (mini-boss type 0x40: the camera held on its area,
 * the HP bar, the music, KatAM's defeat). Values from param/PrmBB06Ml0.kgb (moves), PrmBB06Pl0.kgb (picks) and PrmBB06Sl0.kgb (its
 * HP, 350: here KatAM's bosses' 60 for one player as Mrs. Moley's 300 is, 70); speeds 20.12 a frame, Y up; drawn with enemy/bohboh
 * (its .mca's composites); its boxes from bohboh.rca. Recorded on the DS in its own stage (map/a5s7s2, reached from a5s7s1's door:
 * the points and zones below; all its moves, its grab and the lava's drops).
 * • Its arena (a5s7s2's camera held with its middle 272 across, 56 over Kirby's floor; here KatAM's camera view's middle across and
 *   the floor where it was placed, as Mecha-Kracko's): a lava pool 128 pixels across the middle at the floor's level (zone 2; its
 *   zones 1 left of it, 0 right of it), its points (0x02209CC0) 0 (84 right of the middle, 96 over the floor) and 1 (72 left of
 *   it, as high), linked to each other (the points' table), and 2 (the middle, 36 under the floor, in the lava). Its views
 *   (0x02209D00, from the camera view's middle): 0 the view and 48 pixels beyond its sides and bottom, 16 over its top; 1 56 and
 *   24; 4 the view alone.
 * • Its intro (0x02110C7C-0x021109F4): far off in the background (small: its animation 4), 236 pixels left of the middle and 100
 *   over the floor, it waits 60 frames, flies right at 2.0 into view 0 and on out of view 1, then comes in big from 80 right of and
 *   80 over point 0 to it (60 frames), facing left. It brings the lava's hurt in (0x02127824's kind 2: the stage draws the lava).
 * • It waits (0x021108EC: 30 frames 50 in 100, else 60), then picks (0x02110828): with fewer than two of the stage's enemies out
 *   (0x0220A224) of 100 under 20 its leap, 30 its charge, 40 its pass, 60 its swoop, else it waits again; with more, 50 its leap,
 *   70 its charge, 90 its pass, else it waits.
 *   - Its leap (0x021104D8-0x02110290): from its point to the other, 3.0 across, 3.5 down at first, 0.11 up a frame (no more up or
 *     down once out of the view); over the other point's zone it turns round and keeps on, then past the point settles there in 8
 *     frames.
 *   - Its charge (0x02110150-0x0210FEF8): at its height, 2.0, past the other point, then back past its own, a fire dropped from it
 *     every 30 frames (0x021278CC: 0.1 gravity, bursting on the ground).
 *   - Its pass (0x0210FE4C-0x0210FB48): on at 2.0 out of view 1, dropping its fires; back small in the background into view 0 and
 *     out of view 1 on the other side; in big as in its intro to the point on that side (80 out, 80 over, 60 frames).
 *   After either (0x0210FA6C, 0x0210F9C0), with fewer than two enemies out, 60 frames still and its swoop 30 in 100.
 *   - Its swoop (0x0210F874-0x0210F380): it backs up (0.4 back, 0.2 up, 60 frames: animation 5), then dives at Kirby's feet at 4.0
 *     (animation 6, its grabbing boxes); out of view 1, 60 frames, in big to the point on its side of the middle. Into the lava, it
 *     dives (0x0210F288: 0.2 back, 2.0 up, 0.2 gravity), the lava hurting it (20 of its HP), and sits in it 120 frames (0x0210EEE0:
 *     untouchable in its dive, hit by its own box in the lava; every 40 + rand(20) frames something falls from a spot across the
 *     view's top: one of the stage's enemies 30 in 100 (a Waddle Dee, a Pengy, a Sir Kibble, a Hot Head, one of each at most:
 *     KatAM's own), else a fire (0x02127824's kind 1: 0.1 gravity, bursting on the ground)); then it rises to the point on its side
 *     (0.5 up: .mca's 2), in 60 frames more turns to the middle, and after a flare (.mca's 3, 20 frames) waits.
 *   - Grabbing Kirby in its swoop (0x021111CC: its touch's kind 6; 0x0210EB40-0x0210E788): it carries him 22 pixels under it to the
 *     other point (60 frames: .mca's 1), throws him down at 5.0 toward point 2 (animation 10), and once he's in the lava lets him go
 *     hurt (0x02110D50: 6 of his HP). It can't be hit while it holds him.
 * • Hit (0x02110F98): anywhere but far off (its animation 4) and in its dive (7) (bohboh.rca's hurt boxes there flagged 0x8000:
 *   Squeak Squad's 0x0201322C, untouchable; where an animation has none, its sprite's first box, animation 0's, is hit: 0x02014218,
 *   as recorded), nor while it holds Kirby; 32 frames safe after.
 * • The lava's hurt (0x0212737C: an object of its own over the lava, 128 pixels across and 10 high at the floor, hurting what touches
 *   it; the stage's lava isn't in KatAM's rooms, so here it shows nothing, as on the DS).
 * Not ported: its sounds, the camera's shakes, its hit flash (enemy/bo_bo_dmg's palettes: KatAM's), the lava (the stage's tiles, its
 * bubbles: a5s7s2's), its own defeat (0x0210E730-0x0210E294: KatAM's mini-boss defeat), its boss music. */
#include "../squeak.h"
#include "kirby.h"
#include "constants/object_types.h"

void sub_0804E09C(struct Kirby *);
void sub_08086C48(struct Object *);    // KatAM's defeat of an object (its drop);
void sub_0809D060(struct Object *);    // KatAM's mini-boss defeat (object.c, static);

// Animations (bohboh.cea's);
enum {
    ANIM_STILL, ANIM_FLY, ANIM_FLY_FLARE, ANIM_ACROSS, ANIM_FAR, ANIM_WIND_UP, ANIM_SWOOP, ANIM_DIVE, ANIM_LAVA, ANIM_CLAWS, ANIM_THROW,
    ANIM_FIRE_BIG, ANIM_FIRE_SMALL,
};
// Its .mca's composites: flying, carrying Kirby, rising out of the lava, its flare;
enum { COMPOSITE_FLY, COMPOSITE_CARRY, COMPOSITE_RISE, COMPOSITE_FLARE };

// PrmBB06Ml0.kgb;
#define FALL_ACCEL 0x19A        // +0x08: 0.1 gravity (its fires');
#define SAFE_TIME 32            // +0x0C;
#define INTRO_WAIT 60           // +0x14;
#define SPEED_INTRO 0x2000      // +0x18;
#define IN_X 80                 // +0x1C, +0x20, +0x24: in from 80 out and 80 over a point, 60 frames;
#define IN_UP 80
#define IN_TIME 60
#define WAIT_SHORT 30           // +0x30, +0x34;
#define WAIT_LONG 60
#define POINT_RIGHT 1           // +0x38, +0x3C: the points (+1) right and left of the middle;
#define POINT_LEFT 2
#define LEAP_VX 0x3000          // +0x44, +0x48, +0x4C;
#define LEAP_VY (-0x3800)
#define LEAP_AY 0x1C3
#define SETTLE_TIME 8           // +0x50;
#define CHARGE_SPEED 0x2000     // +0x58;
#define DROP_TIME 30            // +0x5C;
#define PASS_SPEED 0x2000       // +0x64;
#define WIND_TIME 60            // +0x70, +0x74, +0x78;
#define WIND_VX (-0x667)
#define WIND_VY 0x334
#define SWOOP_SPEED 0x4000      // +0x7C;
#define OUT_TIME 60             // +0x80;
#define DIVE_VX (-0x334)        // +0x84..+0x90;
#define DIVE_VY 0x2000
#define DIVE_AY (-0x334)
#define LAVA_TIME 120           // +0x94;
#define RISE_SPEED 0x800        // +0x98;
#define RISE_STOP_TIME 60       // +0x9C;
#define FLARE_TIME 20           // +0xA0;
#define CARRY_TIME 60           // +0xA4;
#define THROW_SPEED 0x5000      // +0xA8;
#define THROW_POINT 3           // +0xAC: point 2 (+1);
// PrmBB06Pl0.kgb;
#define WAIT_SHORT_CHANCE 50    // +0x00;
#define PICK_MANY_LEAP 50       // +0x08..+0x10: with two enemies out or more;
#define PICK_MANY_CHARGE 70
#define PICK_MANY_PASS 90
#define PICK_LEAP 20            // +0x18..+0x24;
#define PICK_CHARGE 30
#define PICK_PASS 40
#define PICK_SWOOP 60
#define AFTER_SWOOP 30          // +0x2C;
#define LAVA_DROP 40            // +0x34, +0x38;
#define LAVA_DROP_RAND 20
#define LAVA_ENEMY 30           // +0x3C;
// PrmBB06Sl0.kgb's 350 HP: the lava's 20 (0x0212737C's box) as KatAM's 70 to it;
#define LAVA_DAMAGE 4

// a5s7s2 (pixels from the camera view's middle across and the floor where it was placed);
#define LAVA_HALF 64            // Its zone 2, the lava: 208-336 of 544 (the middle 272);
#define P0_X 84                 // Its points: 0 (356, 176), 1 (200, 176), 2 (272, 44); the floor 80;
#define P1_X (-72)
#define P_UP 96
#define P2_DOWN 36
#define START_X (-236)          // a5s7s2's spawn (36, 180);
#define START_UP 100
#define HOLD_DOWN 22            // 0x0210E978: Kirby 22 pixels under it (his feet), 1 ahead;
#define HOLD_AHEAD 1
#define FEET 7                  // Kirby's feet, 7 pixels under KatAM's spot;

enum { VIEW_0, VIEW_1, VIEW_4 };
enum { ENEMY_WADDLE_DEE, ENEMY_PENGY, ENEMY_SIR_KIBBLE, ENEMY_HOT_HEAD, ENEMIES };
// What it brings in (0x02127824, 0x021278CC: SQUEAK_BOHBOH_THING's variant): its fire, a fire from the top, the lava's hurt;
enum { THING_FIRE, THING_FIRE_TOP, THING_LAVA };

enum {
    BB_INTRO, BB_INTRO_WAIT, BB_INTRO_FLY, BB_INTRO_PASS, BB_INTRO_IN, BB_IDLE, BB_IDLE_WAIT, BB_PICK, BB_PICK_FEW, BB_PICK_MANY,
    BB_LEAP, BB_LEAP_FLY, BB_LEAP_BACK, BB_SETTLE, BB_CHARGE, BB_CHARGE_GO, BB_CHARGE_TURN, BB_CHARGE_BACK, BB_PASS, BB_PASS_GO,
    BB_PASS_BACK, BB_PASS_OUT, BB_PASS_IN, BB_AFTER, BB_AFTER_WAIT, BB_SWOOP, BB_SWOOP_WIND, BB_SWOOP_GO, BB_SWOOP_OUT,
    BB_SWOOP_REENTER, BB_SWOOP_IN, BB_DIVE, BB_DIVE_FALL, BB_LAVA, BB_RISE, BB_RISE_STOP, BB_RISE_END, BB_FLARE, BB_GRAB, BB_CARRY,
    BB_THROW, BB_THROW_ON,
};

#define TIMER(s) ((s)->vars[0])         // +0x1FC;
#define POINT(s) ((s)->vars[1])         // +0x200: the point it's heading for;
#define MANY(s) ((s)->vars[2])          // +0x204: two of the stage's enemies out or more;
#define THROW_VX(s) ((s)->vars[3])      // +0x208: Kirby thrown across;
#define SAFE(s) ((s)->vars[4])          // +0x1F8;
#define DROP(s) ((s)->vars[5])          // +0x200 in the lava: the next drop;
#define LAST_HP(s) ((s)->vars[6])
#define HARM(s) ((s)->vars[7])
#define FLOOR(s) ((s)->vars[8])         // The floor where it was placed (a5s7s2's Kirby's, the lava's top);
#define LANDED(s) ((s)->vars[9])        // Squeak Squad's flag 0x4000: its last move met the floor;
#define LAVA(s) ((s)->vars[10])         // The lava's hurt;
#define HELD(s) (*(struct Kirby **)&(s)->vars[11])  // +0x164;
#define ENEMY(s, i) (*(struct Object **)&(s)->vars[12 + (i)])

/* Kirby held (kirby->unk110): 22 pixels under it (his feet), a pixel ahead, facing as it (0x0210E978's mode 4); thrown, where its
 * code puts him (mode 3: the script holds him, flag 0x200: doesn't place him); let go hurt (0x02110D50's damage) or as he is; */
static const struct Kirby_110 sCarry[] = {
    { HOLD_AHEAD, HOLD_DOWN - FEET, -1, 0xFF, 0x57, 0x400 }, { 0, 0, 0, 1, 0, 0x400 },
};
static const struct Kirby_110 sThrown[] = { { 0, 0, -1, 0xFF, 0x57, 0x600 }, { 0, 0, 0, 1, 0, 0x600 } };
static const struct Kirby_110 sHurt[] = { { 0, 13, -1, 1, 0, 0x402 }, { 0, 0, 0, 1, 0, 0 } };
static const struct Kirby_110 sLetGo[] = { { 0, 0, -1, 1, 0, 0x401 }, { 0, 0, 0, 1, 0, 0 } };

/* bohboh.rca (KatAM's: x ahead, y down, pixels from its spot): its touch's box (tag 0x101 kind 0) and where it's hit (0x102: its own
 * in its animations 0, 4, 7 and 8, else animation 0's, the sprite's first: 0x02014218), its swoop's grabbing boxes (0x101 kind 6);
 * none far off (4: its box flagged, 100 pixels over it) and in its throw (10); */
enum { BOX_STILL, BOX_SWOOP, BOX_DIVE, BOX_LAVA, BOX_NONE };
static const struct SqueakBox sBohbohBoxes[] = {
    [BOX_STILL] = { { -25, -42, 20, -5 }, TRUE, 0 },
    [BOX_SWOOP] = { { -25, -42, 16, -12 }, TRUE, 2, { { 16, -42, 31, 6 }, { -25, -12, 16, 6 } } },
    [BOX_DIVE] = { { -36, -52, 28, -5 }, TRUE, 0 },
    [BOX_LAVA] = { { -34, -49, 20, -5 }, TRUE, 0 },
};

static void BohbohMain(struct Object *obj);

static u32 Random(u32 n) {
    return Rand16() * n >> 16;
}

static s32 T(struct SqueakObject *s) {
    return (s16)s->stateTimer;
}

static s32 Abs(s32 v) {
    return v < 0 ? -v : v;
}

// FxMul(); Squeak Squad's 20.12 multiply (Mecha-Kracko's), in 32-bit pieces;
static s32 FxMul(s32 a, s32 b) {
    bool32 minus = (a < 0) != (b < 0);
    u32 ua = a < 0 ? -(u32)a : (u32)a, ub = b < 0 ? -(u32)b : (u32)b;
    u32 al = ua & 0xFFFF, ah = ua >> 16, bl = ub & 0xFFFF, bh = ub >> 16;
    u32 lo = al * bl, mid1 = ah * bl, mid2 = al * bh, hi = ah * bh, t;

    t = lo + (mid1 << 16); hi += (mid1 >> 16) + (t < lo); lo = t;
    t = lo + (mid2 << 16); hi += (mid2 >> 16) + (t < lo); lo = t;
    t = lo + (minus ? 0x7FF : 0x800); hi += t < lo; lo = t;
    t = (hi << 20) | (lo >> 12);
    return minus ? -(s32)t : (s32)t;
}

// FxSqrt(); The square root of a 20.12 value;
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

static struct LevelInfo *Level(struct SqueakObject *s) {
    return &gCurLevelInfo[s->obj.base.unk56];
}

// Middle(); The camera view's middle across (Squeak Squad's camera + 0x14: a5s7s2's 272; here KatAM's view, 240 across);
static s32 Middle(struct SqueakObject *s) {
    return Level(s)->viewportPosition.x + (DISPLAY_WIDTH / 2 << 8);
}

/* InView(); 0x02209D00: a spot in the camera's view and the margins beyond it (its table 0x022275F4: over, under, left, right),
 * from the view's middle (Squeak Squad's 256 by 192; KatAM's 240 by 160); */
static bool32 InView(struct SqueakObject *s, s32 x, s32 y, u8 view) {
    static const u8 sMargins[][4] = { [VIEW_0] = { 16, 48, 48, 48 }, [VIEW_1] = { 24, 56, 56, 56 }, [VIEW_4] = { 0, 0, 0, 0 } };
    const u8 *m = sMargins[view];
    s32 dx = x - Middle(s), up = Level(s)->viewportPosition.y + (DISPLAY_HEIGHT / 2 << 8) - y;

    return dx > -((m[2] + DISPLAY_WIDTH / 2) << 8) && dx < (m[3] + DISPLAY_WIDTH / 2) << 8
           && up > -((m[1] + DISPLAY_HEIGHT / 2) << 8) && up < (m[0] + DISPLAY_HEIGHT / 2) << 8;
}

static bool32 Seen(struct SqueakObject *s, u8 view) {
    return InView(s, s->obj.base.x, s->obj.base.y, view);
}

// Zone(); 0x02209C7C: a5s7s2's zone grid across (1 left of the lava, 2 the lava, 0 right of it);
static u8 Zone(struct SqueakObject *s, s32 x) {
    s32 d = x - Middle(s);

    if (d < -(LAVA_HALF << 8)) return 1;
    if (d >= LAVA_HALF << 8) return 0;
    return 2;
}

// Linked(); The points' table: the point its zone's point is linked to (0 and 1 to each other; 2 none: -1);
static s32 Linked(struct SqueakObject *s) {
    switch (Zone(s, s->obj.base.x)) {
    case 0: return 1;
    case 1: return 0;
    default: return -1;
    }
}

// Point(); 0x02209CC0: a point of a5s7s2's;
static void Point(struct SqueakObject *s, s32 n, s32 *x, s32 *y) {
    switch (n) {
    case 0: *x = Middle(s) + (P0_X << 8); *y = FLOOR(s) - (P_UP << 8); break;
    case 1: *x = Middle(s) + (P1_X << 8); *y = FLOOR(s) - (P_UP << 8); break;
    default: *x = Middle(s); *y = FLOOR(s) + (P2_DOWN << 8); break;
    }
}

// InLava(); 0x022098F8 == 5: a spot (feet) at the lava's top or under it, over the lava's zone;
static bool32 InLava(struct SqueakObject *s, s32 x, s32 y) {
    return Zone(s, x) == 2 && y >= FLOOR(s);
}

static bool32 FacesLeft(struct SqueakObject *s) {
    return s->obj.base.flags & 1;
}

static void Face(struct SqueakObject *s, bool32 left) {
    s->obj.base.flags = (s->obj.base.flags & ~1) | (left ? 1 : 0);
}

// Div(); Squeak Squad's 0x0200C80C on 20.12 values (a whole quotient, rounded toward 0);
static s32 Div(s32 a, s32 b) {
    return b == 0 ? 0 : a / b;
}

// Ahead(), Up(); A spot from its own, forward (as it faces) and up, Squeak Squad's units;
static s32 Across(struct SqueakObject *s, s32 x) {
    return (x - s->obj.base.x) << 4;
}

static s32 Up(struct SqueakObject *s, s32 y) {
    return (s->obj.base.y - y) << 4;
}

/* Move(); 0x02013D18 with its flags 0x200 (through the terrain) and 0x20000 (not held in the stage: it flies out of view, its intro
 * from past the room's side), as Mecha-Kracko's; and whether it met the floor's line coming down (Squeak Squad's terrain, met but
 * not stopping it: a5s7s2's floor and lava are one line); */
static void Move(struct SqueakObject *s) {
    struct ObjectBase *base = &s->obj.base;

    s->vx += s->ax;
    s->vy += s->ay;
    base->xspeed = SQUEAK_SPEED(base->flags & 1 ? -s->vx : s->vx);
    base->yspeed = SQUEAK_SPEED(s->vy);
    base->unk48 = base->x;
    base->unk4C = base->y;
    base->x += base->xspeed;
    base->y -= base->yspeed;
    base->unk62 = 0;
    s->terrain = 0;
    s->moved = TRUE;
    LANDED(s) = base->y >= FLOOR(s) && s->vy <= 0;
}

/* EnemiesOut(); The stage's enemies it dropped still about (0x0220A224: a5s7s2's records 1-4 out); Drop(): 0x0220A230, one where
 * the stage puts it, if it isn't out (a Waddle Dee, a Pengy, a Sir Kibble, a Hot Head: KatAM's own, walking); */
static u8 EnemiesOut(struct SqueakObject *s) {
    u8 i, n = 0;

    for (i = 0; i < ENEMIES; i++) {
        struct Object *e = ENEMY(s, i);

        if (e && e->base.header.kind == 1 && e->base.parent == &s->obj && !(e->base.flags & 0x1000) && e->unk80 > 0)
            n++;
        else
            ENEMY(s, i) = NULL;
    }
    return n;
}

static void DropEnemy(struct SqueakObject *s, u8 i, s32 x, s32 y) {
    static const u8 sTypes[ENEMIES] = { OBJ_WADDLE_DEE_1, OBJ_PENGY, OBJ_SIR_KIBBLE, OBJ_HOT_HEAD };
    struct Object *e;

    EnemiesOut(s);
    if (ENEMY(s, i) || gKirbys[gLocalPlayerId].base.roomId != s->obj.base.roomId) return;
    e = CreateObjTemplateAndObj(s->obj.base.unk56, 1, 0x24, x >> 8, y >> 8, 0, 0x1F, 0, 0, sTypes[i], 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0);
    if (e == NULL) return;
    ENEMY(s, i) = e;
    e->base.parent = &s->obj;
}

// Thing(); What it brings in (0x02127824's kind at a spot, 0x021278CC's at its own), SQUEAK_BOHBOH_THING;
static struct SqueakObject *Thing(struct SqueakObject *s, u8 kind, s32 x, s32 y) {
    struct SqueakObject *t = SqueakSpawn(s, SQUEAK_BOHBOH_THING, kind, 0, 0);

    if (t == NULL) return NULL;
    t->obj.base.x = x;
    t->obj.base.y = y;
    t->obj.base.parent = &s->obj;
    return t;
}

// LavaDrop(); 0x0210EEE0: from a spot across the view's top: one of the stage's enemies, else a fire;
static void LavaDrop(struct SqueakObject *s) {
    s32 x = Level(s)->viewportPosition.x + (Random(DISPLAY_WIDTH) << 8), y = Level(s)->viewportPosition.y;

    if (Random(100) < LAVA_ENEMY) DropEnemy(s, Random(ENEMIES), x, y);
    else Thing(s, THING_FIRE_TOP, x, y);
}

/* Lava(); The lava's hurt (0x02110C7C: 0x02127824's kind 2 at the middle of the floor), made once, again if lost; kept at the camera
 * view's middle (a5s7s2's camera doesn't move: KatAM's may still be settling on the area); */
static void Lava(struct SqueakObject *s) {
    struct SqueakObject *l = (struct SqueakObject *)LAVA(s);

    if (l && l->obj.base.header.kind == 1 && !(l->obj.base.flags & 0x1000) && l->obj.base.parent == &s->obj && SqueakIsSqueak(&l->obj)
        && l->enemy == SQUEAK_BOHBOH_THING) {
        l->obj.base.x = Middle(s);
        l->obj.base.y = FLOOR(s);
        return;
    }
    LAVA(s) = (s32)Thing(s, THING_LAVA, Middle(s), FLOOR(s));
}

// Kirby held;
static bool32 StillHeld(struct SqueakObject *s) {
    struct Kirby *kirby = HELD(s);

    return kirby && kirby->base.unk6C == &s->obj.base && (kirby->unk110 == sCarry || kirby->unk110 == sThrown);
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

/* Hittable(); 0x0201322C: not far off nor diving (bohboh.rca's boxes there flagged 0x8000; its composites hit by animation 0's box,
 * as recorded), and 0x02110F98: not holding Kirby, not just hit; */
static bool32 Hittable(struct SqueakObject *s) {
    return (s->composite != SQUEAK_NO_COMPOSITE || (s->anim != ANIM_FAR && s->anim != ANIM_DIVE)) && !HELD(s) && SAFE(s) == 0;
}

// Hurt(); The lava's hurt as it dives in (0x0212737C's box: its 20), KatAM's mini-boss defeat if it's its last (Mrs. Moley's crash);
static void Hurt(struct SqueakObject *s) {
    struct Object *obj = &s->obj;

    if (SAFE(s) > 0) return;
    SAFE(s) = SAFE_TIME;
    obj->unk80 -= LAVA_DAMAGE;
    LAST_HP(s) = obj->unk80;
    sub_08034E14(obj);
    if (obj->unk80 > 0) return;
    obj->unk80 = 0;
    obj->base.flags &= ~(0x400 | 0x200);
    sub_08086C48(obj);
    sub_0809D060(obj);
}

// Reenter(); Out of view: in from IN_X out and IN_UP over a point, to it in IN_TIME frames (Squeak Squad's 80 / 60 a frame);
static void Reenter(struct SqueakObject *s, s32 n, bool32 left) {
    s32 x, y;

    Point(s, n, &x, &y);
    Face(s, left);
    s->obj.base.x = left ? x + (IN_X << 8) : x - (IN_X << 8);
    s->obj.base.y = y - (IN_UP << 8);
    s->vx = Div(IN_X << 12, IN_TIME);
    s->vy = -Div(IN_UP << 12, IN_TIME);
    TIMER(s) = IN_TIME;
    SqueakPlay(s, ANIM_ACROSS);
}

// DropFire(); 0x021278CC(0, it, 0): every DROP_TIME frames, its fire where it is;
static void DropFire(struct SqueakObject *s) {
    if (--TIMER(s) < 1) {
        Thing(s, THING_FIRE, s->obj.base.x, s->obj.base.y);
        TIMER(s) = DROP_TIME;
    }
}

// Passed(); Past a point's x the way it faces;
static bool32 Passed(struct SqueakObject *s, s32 x) {
    return FacesLeft(s) ? s->obj.base.x < x : x < s->obj.base.x;
}

// Plunge(); Kirby thrown down at THROW_SPEED (0x0210E830, 0x0210E788): into the lava he's let go hurt;
static bool32 Plunge(struct SqueakObject *s) {
    struct Kirby *kirby = HELD(s);

    if (!StillHeld(s)) {
        HELD(s) = NULL;
        return FALSE;
    }
    if (InLava(s, kirby->base.x, kirby->base.y + (FEET << 8))) {
        LetGo(s, sHurt);
        SqueakSetState(s, BB_IDLE);
        return TRUE;
    }
    kirby->base.x += SQUEAK_SPEED(THROW_VX(s));
    kirby->base.y += SQUEAK_SPEED(THROW_SPEED);
    Move(s);
    return FALSE;
}

static bool32 Bohboh(struct SqueakObject *s) {
    struct Kirby *kirby = s->obj.kirby3;
    s32 x, y, dx, dy, d, k;

    switch (s->state) {
    case BB_INTRO:          // 0x02110C7C: far off at a5s7s2's spawn, facing right; the lava's hurt brought in;
        s->obj.base.x = Middle(s) + (START_X << 8);
        s->obj.base.y = FLOOR(s) - (START_UP << 8);
        s->obj.base.flags &= ~0x400;
        SqueakPlay(s, ANIM_FAR);
        SqueakStop(s);
        TIMER(s) = INTRO_WAIT;
        Face(s, FALSE);
        Lava(s);
        SqueakSetState(s, BB_INTRO_WAIT);
        return TRUE;
    case BB_INTRO_WAIT:     // 0x02110C08;
        if (--TIMER(s) < 1) {
            s->vx = SPEED_INTRO;
            SqueakSetState(s, BB_INTRO_FLY);
        }
        break;
    case BB_INTRO_FLY:      // 0x02110BB8: into view 0;
        if (Seen(s, VIEW_0)) SqueakSetState(s, BB_INTRO_PASS);
        Move(s);
        break;
    case BB_INTRO_PASS:     // 0x02110A5C: out of view 1, in big to its zone's point;
        if (Seen(s, VIEW_1)) {
            Move(s);
            break;
        }
        Reenter(s, Zone(s, s->obj.base.x), !FacesLeft(s));
        SqueakSetState(s, BB_INTRO_IN);
        break;
    case BB_INTRO_IN:       // 0x021109F4;
        if (--TIMER(s) < 1) {
            SqueakSetState(s, BB_IDLE);
            return TRUE;
        }
        Move(s);
        break;
    case BB_IDLE:           // 0x021108EC;
        TIMER(s) = Random(100) < WAIT_SHORT_CHANCE ? WAIT_SHORT : WAIT_LONG;
        SqueakPlayComposite(s, COMPOSITE_FLY);
        SqueakStop(s);
        SqueakSetState(s, BB_IDLE_WAIT);
        return TRUE;
    case BB_IDLE_WAIT:      // 0x02110898;
        if (--TIMER(s) < 1) {
            SqueakSetState(s, BB_PICK);
            break;
        }
        Move(s);
        break;
    case BB_PICK:           // 0x02110828;
        MANY(s) = EnemiesOut(s) >= 2;
        SqueakSetState(s, MANY(s) ? BB_PICK_MANY : BB_PICK_FEW);
        return TRUE;
    case BB_PICK_FEW:       // 0x0211072C;
        d = Random(100);
        SqueakSetState(s, d < PICK_LEAP ? BB_LEAP : d < PICK_CHARGE ? BB_CHARGE : d < PICK_PASS ? BB_PASS
                          : d < PICK_SWOOP ? BB_SWOOP : BB_IDLE);
        return TRUE;
    case BB_PICK_MANY:      // 0x02110654;
        d = Random(100);
        SqueakSetState(s, d < PICK_MANY_LEAP ? BB_LEAP : d < PICK_MANY_CHARGE ? BB_CHARGE : d < PICK_MANY_PASS ? BB_PASS : BB_IDLE);
        return TRUE;
    case BB_LEAP:           // 0x021104D8: to the other point, down and up;
        if ((POINT(s) = Linked(s)) < 0) {
            SqueakSetState(s, BB_IDLE);
            return TRUE;
        }
        Point(s, POINT(s), &x, &y);
        Face(s, Across(s, x) < 1);
        SqueakStop(s);
        s->vx = LEAP_VX;
        s->vy = LEAP_VY;
        s->ay = LEAP_AY;
        SqueakPlay(s, ANIM_ACROSS);
        SqueakSetState(s, BB_LEAP_FLY);
        return TRUE;
    case BB_LEAP_FLY:       // 0x02110434: over the point's zone it turns round and keeps on;
        if (!Seen(s, VIEW_4)) s->vy = s->ay = 0;
        if (Zone(s, s->obj.base.x) == POINT(s)) {
            SqueakTurnAround(s);
            SqueakSetState(s, BB_LEAP_BACK);
            return TRUE;
        }
        Move(s);
        break;
    case BB_LEAP_BACK:      // 0x02110304: past the point, it settles there;
        if (!Seen(s, VIEW_4)) s->vy = s->ay = 0;
        Point(s, POINT(s), &x, &y);
        // (facing right, it settles left of the point; facing left, right of it: its turn kept it going on);
        if (!(FacesLeft(s) ? x < s->obj.base.x : s->obj.base.x < x)) {
            Move(s);
            break;
        }
        s->vx = Div(Abs(Across(s, x)), SETTLE_TIME);
        s->vy = Div(Up(s, y), SETTLE_TIME);
        s->ay = 0;
        TIMER(s) = SETTLE_TIME;
        SqueakSetState(s, BB_SETTLE);
        return TRUE;
    case BB_SETTLE:         // 0x02110290;
        if (--TIMER(s) < 1) {
            Point(s, POINT(s), &s->obj.base.x, &s->obj.base.y);
            SqueakSetState(s, BB_IDLE);
            return TRUE;
        }
        Move(s);
        break;
    case BB_CHARGE:         // 0x02110150: across at its height to the other point, dropping its fires;
        if ((POINT(s) = Linked(s)) < 0) {
            SqueakSetState(s, BB_IDLE);
            return TRUE;
        }
        s->vx = CHARGE_SPEED;
        TIMER(s) = 0;
        SqueakPlay(s, ANIM_ACROSS);
        SqueakSetState(s, BB_CHARGE_GO);
        return TRUE;
    case BB_CHARGE_GO:      // 0x02110080: past it, it turns (its speed kept: back the other way);
    case BB_CHARGE_BACK:    // 0x0210FEF8;
        DropFire(s);
        Point(s, POINT(s), &x, &y);
        if (FacesLeft(s) ? s->obj.base.x < x : x < s->obj.base.x) {
            Face(s, !FacesLeft(s));
            SqueakSetState(s, s->state == BB_CHARGE_GO ? BB_CHARGE_TURN : BB_AFTER);
            return TRUE;
        }
        Move(s);
        break;
    case BB_CHARGE_TURN:    // 0x0210FFC8;
        if ((POINT(s) = Linked(s)) < 0) {
            SqueakSetState(s, BB_IDLE);
            return TRUE;
        }
        SqueakSetState(s, BB_CHARGE_BACK);
        return TRUE;
    case BB_PASS:           // 0x0210FE4C: on out of view, dropping its fires;
        s->vx = PASS_SPEED;
        TIMER(s) = 0;
        SqueakPlay(s, ANIM_ACROSS);
        SqueakSetState(s, BB_PASS_GO);
        return TRUE;
    case BB_PASS_GO:        // 0x0210FD44: out of view 1, back far off;
        DropFire(s);
        if (!Seen(s, VIEW_1)) {
            Face(s, !FacesLeft(s));
            s->vx = SPEED_INTRO;
            SqueakPlay(s, ANIM_FAR);
            SqueakSetState(s, BB_PASS_BACK);
            return TRUE;
        }
        Move(s);
        break;
    case BB_PASS_BACK:      // 0x0210FCEC;
        if (Seen(s, VIEW_0)) {
            SqueakSetState(s, BB_PASS_OUT);
            return TRUE;
        }
        Move(s);
        break;
    case BB_PASS_OUT:       // 0x0210FB9C: out of view 1 on the other side, in big to that zone's point;
        if (Seen(s, VIEW_1)) {
            Move(s);
            break;
        }
        Reenter(s, Zone(s, s->obj.base.x), !FacesLeft(s));
        SqueakSetState(s, BB_PASS_IN);
        break;
    case BB_PASS_IN:        // 0x0210FB48;
        if (--TIMER(s) < 1) {
            SqueakSetState(s, BB_AFTER);
            return TRUE;
        }
        Move(s);
        break;
    case BB_AFTER:          // 0x0210FA6C;
        if (MANY(s)) {
            SqueakSetState(s, BB_IDLE);
            return TRUE;
        }
        TIMER(s) = WAIT_LONG;
        SqueakPlayComposite(s, COMPOSITE_FLY);
        SqueakStop(s);
        SqueakSetState(s, BB_AFTER_WAIT);
        return TRUE;
    case BB_AFTER_WAIT:     // 0x0210F9C0;
        if (--TIMER(s) < 1) {
            SqueakSetState(s, Random(100) < AFTER_SWOOP ? BB_SWOOP : BB_IDLE);
            return TRUE;
        }
        Move(s);
        break;
    case BB_SWOOP:          // 0x0210F874: backing up, toward the other point (where it carries Kirby);
        if ((POINT(s) = Linked(s)) < 0) {
            SqueakSetState(s, BB_IDLE);
            return TRUE;
        }
        SqueakStop(s);
        s->vx = WIND_VX;
        s->vy = WIND_VY;
        TIMER(s) = WIND_TIME;
        SqueakPlay(s, ANIM_WIND_UP);
        SqueakSetState(s, BB_SWOOP_WIND);
        return TRUE;
    case BB_SWOOP_WIND:     // 0x0210F638: at Kirby's feet, SWOOP_SPEED (Squeak Squad's angle: its cosine and sine, rounded);
        if (--TIMER(s) > 0) {
            Move(s);
            break;
        }
        dx = Across(s, kirby->base.x);
        dy = Up(s, kirby->base.y + (FEET << 8));
        d = FxSqrt(FxMul(dx, dx) + FxMul(dy, dy));
        k = d ? (SWOOP_SPEED << 12) / d : 0;
        s->vx = FxMul(dx, k);
        s->vy = FxMul(dy, k);
        Face(s, s->vx < 0);
        s->vx = Abs(s->vx);
        SqueakPlay(s, ANIM_SWOOP);
        SqueakSetState(s, BB_SWOOP_GO);
        return TRUE;
    case BB_SWOOP_GO:       // 0x0210F584: out of view 1, or into the lava;
        if (!Seen(s, VIEW_1)) {
            SqueakStop(s);
            TIMER(s) = OUT_TIME;
            SqueakSetState(s, BB_SWOOP_OUT);
            return TRUE;
        }
        if (LANDED(s) && InLava(s, s->obj.base.x, s->obj.base.y)) {
            SqueakSetState(s, BB_DIVE);
            return TRUE;
        }
        Move(s);
        break;
    case BB_SWOOP_OUT:      // 0x0210F530;
        if (--TIMER(s) < 1) {
            SqueakSetState(s, BB_SWOOP_REENTER);
            return TRUE;
        }
        Move(s);
        break;
    case BB_SWOOP_REENTER:  // 0x0210F3D4: in to the point on its side of the middle;
        if (s->obj.base.x < Middle(s)) {
            POINT(s) = POINT_LEFT - 1;
            Reenter(s, POINT(s), FALSE);
        } else {
            POINT(s) = POINT_RIGHT - 1;
            Reenter(s, POINT(s), TRUE);
        }
        SqueakSetState(s, BB_SWOOP_IN);
        return TRUE;
    case BB_SWOOP_IN:       // 0x0210F380;
        if (--TIMER(s) < 1) {
            SqueakSetState(s, BB_IDLE);
            return TRUE;
        }
        Move(s);
        break;
    case BB_DIVE:           // 0x0210F288: into the lava, hurt by it; out of it (the lava's top holds it: 0x0201355C) up again;
        Hurt(s);
        if (s->obj.unk80 <= 0) return FALSE;
        s->obj.base.y = FLOOR(s);
        s->vx = DIVE_VX;
        s->vy = DIVE_VY;
        s->ax = 0;
        s->ay = DIVE_AY;
        SqueakPlay(s, ANIM_DIVE);
        SqueakSetState(s, BB_DIVE_FALL);
        Move(s);
        break;
    case BB_DIVE_FALL:      // 0x0210F15C: down in the lava, it sits there;
        if (LANDED(s) && InLava(s, s->obj.base.x, s->obj.base.y)) {
            TIMER(s) = LAVA_TIME;
            DROP(s) = 0;
            SqueakPlay(s, ANIM_LAVA);
            SqueakStop(s);
            s->obj.base.y = FLOOR(s);
            SqueakSetState(s, BB_LAVA);
            break;
        }
        Move(s);
        if (LANDED(s)) s->obj.base.y = FLOOR(s);
        break;
    case BB_LAVA:           // 0x0210EEE0: things falling from the top; then up to the point on its side;
        if (DROP(s)-- < 1) {
            DROP(s) = LAVA_DROP + Random(LAVA_DROP_RAND);
            LavaDrop(s);
        }
        if (--TIMER(s) > 0) {
            Move(s);
            break;
        }
        POINT(s) = s->obj.base.x < Middle(s) ? POINT_LEFT - 1 : POINT_RIGHT - 1;
        Point(s, POINT(s), &x, &y);
        Face(s, !(s->obj.base.x < x));
        TIMER(s) = Div(Abs(Up(s, y)), RISE_SPEED) + 1;
        SqueakStop(s);
        s->vx = Div(Abs(Across(s, x)), TIMER(s));
        s->vy = RISE_SPEED;
        SqueakPlayComposite(s, COMPOSITE_RISE);
        SqueakSetState(s, BB_RISE);
        return TRUE;
    case BB_RISE:           // 0x0210EE18: there, it turns to the middle;
        if (--TIMER(s) >= 0) {
            Move(s);
            break;
        }
        Face(s, s->obj.base.x >= Middle(s));
        TIMER(s) = RISE_STOP_TIME;
        Point(s, POINT(s), &x, &y);
        s->vx = 0;
        s->vy = Div(Up(s, y), RISE_STOP_TIME);
        SqueakSetState(s, BB_RISE_STOP);
        return TRUE;
    case BB_RISE_STOP:      // 0x0210EDB4;
        if (--TIMER(s) < 1) {
            s->vy = 0;
            if (s->animEnded) {
                SqueakSetState(s, BB_RISE_END);
                return TRUE;
            }
        }
        Move(s);
        break;
    case BB_RISE_END:       // 0x0210ECEC: its flare;
        if (s->animEnded) {
            TIMER(s) = FLARE_TIME;
            SqueakPlayComposite(s, COMPOSITE_FLARE);
            SqueakSetState(s, BB_FLARE);
            return TRUE;
        }
        Move(s);
        break;
    case BB_FLARE:          // 0x0210EC98;
        if (--TIMER(s) < 1) {
            SqueakSetState(s, BB_IDLE);
            return TRUE;
        }
        Move(s);
        break;
    case BB_GRAB:           // 0x0210EB40: Kirby carried to the other point;
        Point(s, POINT(s), &x, &y);
        TIMER(s) = CARRY_TIME;
        s->vx = Div(Across(s, x), TIMER(s));
        s->vy = Div(Up(s, y), TIMER(s));
        Face(s, s->vx < 0);
        s->vx = Abs(s->vx);
        if (HELD(s)) HELD(s)->base.flags = (HELD(s)->base.flags & ~1) | (s->obj.base.flags & 1);
        SqueakPlayComposite(s, COMPOSITE_CARRY);
        SqueakSetState(s, BB_CARRY);
        return TRUE;
    case BB_CARRY:          // 0x0210E978: thrown down toward point 2 (the lava), facing the middle;
        if (--TIMER(s) >= 0) {
            Move(s);
            break;
        }
        if (!StillHeld(s)) {
            HELD(s) = NULL;
            SqueakSetState(s, BB_IDLE);
            return TRUE;
        }
        Point(s, THROW_POINT - 1, &x, &y);
        TIMER(s) = Abs(Div((HELD(s)->base.y + (FEET << 8) - y) << 4, THROW_SPEED)) + 1;
        THROW_VX(s) = Abs(Div((x - HELD(s)->base.x) << 4, TIMER(s)));
        Face(s, s->obj.base.x >= Middle(s));
        if (FacesLeft(s)) THROW_VX(s) = -THROW_VX(s);
        HELD(s)->base.flags = (HELD(s)->base.flags & ~1) | (s->obj.base.flags & 1);
        HELD(s)->unk110 = sThrown;
        SqueakStop(s);
        SqueakPlay(s, ANIM_THROW);
        SqueakSetState(s, BB_THROW);
        return TRUE;
    case BB_THROW:          // 0x0210E830;
        if (s->animEnded) {
            SqueakPlayComposite(s, COMPOSITE_FLY);
            SqueakSetState(s, BB_THROW_ON);
            return TRUE;
        }
        // Fallthrough
    case BB_THROW_ON:       // 0x0210E788;
        if (!HELD(s)) {
            if (s->state == BB_THROW_ON) {
                SqueakSetState(s, BB_IDLE);
                return TRUE;
            }
            Move(s);
            break;
        }
        return Plunge(s);
    }
    return FALSE;
}

/* Hit(); 0x02110F98 (KatAM's mini-bosses take hits without a hurt state of their own: a fall of its HP is the hit): SAFE_TIME frames
 * safe after one; a hit where it can't be hit undone; */
static void Hit(struct SqueakObject *s) {
    struct Object *obj = &s->obj;
    s32 damage = LAST_HP(s) - obj->unk80;

    if (damage > 0 && obj->unk80 > 0 && !(obj->base.flags & 0x1000)) {
        if (!Hittable(s)) obj->unk80 = LAST_HP(s);
        else SAFE(s) = SAFE_TIME + 1;
    }
    if (SAFE(s) > 0) SAFE(s)--;
    LAST_HP(s) = obj->unk80;
}

/* Boxes(); Its box by its animation (bohboh.rca's: its touch's and where it's hit, animation 0's where it has none of its own, as a
 * composite's), hit only where it can be (KatAM's level 3; else 6, as Gordo's: no attack hurts it); none far off and in its throw; */
static void Boxes(struct SqueakObject *s) {
    struct ObjectBase *base = &s->obj.base;
    const struct SqueakBox *b;
    u8 box = BOX_STILL;

    if (s->composite == SQUEAK_NO_COMPOSITE) {
        switch (s->anim) {
        case ANIM_FAR: case ANIM_THROW: case ANIM_CLAWS: box = BOX_NONE; break;
        case ANIM_SWOOP: box = BOX_SWOOP; break;
        case ANIM_DIVE: box = BOX_DIVE; break;
        case ANIM_LAVA: box = BOX_LAVA; break;
        }
    }
    if (box == BOX_NONE || (base->flags & 0x400)) {
        base->flags |= 0x200;
        base->unk68 = 0;
        return;
    }
    b = &sBohbohBoxes[box];
    base->flags &= ~0x200;
    sub_0803E2B0(base, b->box[0], b->box[1], b->box[2], b->box[3]);
    base->unk68 = HARM(s);
    base->unk5C = (base->unk5C & ~7) | (Hittable(s) ? 3 : 6);
}

/* Hold(); Kirby held, his body no harm to it (KatAM's grabbers'); its swoop's grabbing boxes taking him (0x021111CC's kind 6:
 * stopped, carried from the next frame on); */
static void Hold(struct SqueakObject *s) {
    struct Object *obj = &s->obj;
    struct Kirby *kirby;

    if (HELD(s) && !StillHeld(s)) LetGo(s, sLetGo);
    if (HELD(s)) obj->base.unk5C |= 0x80;
    else obj->base.unk5C &= ~0x80;
    if (HELD(s) || s->anim != ANIM_SWOOP || s->composite != SQUEAK_NO_COMPOSITE || s->state != BB_SWOOP_GO) return;
    kirby = SqueakGrabbing(s, &sBohbohBoxes[BOX_SWOOP]);
    if (kirby == NULL) return;
    HELD(s) = kirby;
    kirby->base.unk6C = &obj->base;
    kirby->unk110 = sCarry;
    obj->base.unk6C = kirby;
    kirby->base.flags |= 0x40000;
    sub_0804E09C(kirby);
    SqueakStop(s);
    SqueakSetState(s, BB_GRAB);
}

static void BohbohMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    obj->kirby3 = FindTargetKirby(&obj->base);
    Hit(s);
    SqueakRun(s, Bohboh);
    if (obj->unk80 <= 0) {
        if (HELD(s)) LetGo(s, sLetGo);
        return;
    }
    // (its HP bar Squeak Squad's boss's, which its enemies' hits don't take: KatAM's goes to whatever is hit, taken back);
    sub_08034E14(obj);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
    Boxes(s);
    Hold(s);
    if (s->state >= BB_INTRO_WAIT) Lava(s);
}

// BohbohWatch(); Its pre-callback: KatAM's hit put it out of its own state, or it's beaten: it lets Kirby go as he is;
static void BohbohWatch(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    if (HELD(s) && (obj->unk78 != BohbohMain || obj->unk80 <= 0 || (obj->base.flags & 0x1000))) LetGo(s, sLetGo);
}

/* BohbohStart(); Its fight's start (and after a hit: it goes on, as KatAM's mini-bosses do). It isn't held in the camera's area
 * (Squeak Squad's flag 0x20000: it flies out of view), through the terrain (its flag 0x200); */
void BohbohStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, -1, BohbohMain);
    obj->unk7C = BohbohWatch;
    SqueakThroughTerrain(s, TRUE);
    if (s->started) return;
    LAST_HP(s) = obj->unk80;
    SAFE(s) = 0;
    HELD(s) = NULL;
    LAVA(s) = 0;
    SqueakSetState(s, BB_INTRO);
}

/* BohbohInit(); Its box (its body's), its bounds (KatAM's), the floor where it's placed (a5s7s2's Kirby's, the lava's top); hidden,
 * still, till its fight; */
void BohbohInit(struct SqueakObject *s) {
    struct Object *obj = &s->obj;
    u8 i;

    sub_0803E2B0(&obj->base, -17, -35, 15, -5);
    ObjectSetBounds(&obj->base, -16, -32, 16, 0);
    HARM(s) = obj->base.unk68;
    SqueakStandOnFloor(obj);
    FLOOR(s) = obj->base.y;
    for (i = 0; i < ENEMIES; i++) ENEMY(s, i) = NULL;
    obj->base.flags |= 0x400;
    obj->base.unk62 |= 4;
    SqueakPlayComposite(s, COMPOSITE_FLY);
}

/* What it brings in (0x02127750: the variant its kind): 0 its fire (0x021275DC, 0x0212756C: dropped from it, its animation 12), 1 a
 * fire from the top (0x021274A8, 0x02127444: 11), both falling at 0.1 gravity and bursting on the ground (0x020B48E0's type 6, the
 * stage's alike), gone out of view 1; hurting Kirby, not hurt by his body (it doesn't burst on him), any hit breaking it; 2 the
 * lava's hurt (0x0212737C: drawn by nothing, its box 128 pixels across and 10 high round the floor's line, hurting Kirby; no attack
 * hurts it), gone with Bohboh; */
#define T_HARM(s) ((s)->vars[0])

static void Burst(struct SqueakObject *t) {
    SqueakSpawnEffect(t, SQUEAK_FX_BURST, 0, 0, FALSE);
    t->obj.base.flags |= 0x1000;
}

static bool32 BohbohGone(struct SqueakObject *t) {
    struct SqueakObject *b = (struct SqueakObject *)t->obj.base.parent;

    return b == NULL || b->obj.base.header.kind != 1 || (b->obj.base.flags & 0x1000) || b->obj.unk80 <= 0 || !SqueakIsSqueak(&b->obj)
           || b->enemy != SQUEAK_BOHBOH;
}

static bool32 BohbohThing(struct SqueakObject *t) {
    switch (t->state) {
    case 0:
        SqueakStop(t);
        if (t->variant == THING_LAVA) {
            SqueakSetState(t, 2);
            return TRUE;
        }
        t->ay = -FALL_ACCEL;
        SqueakPlay(t, t->variant);      // (its OnlyAnims: bohboh.cea's 12, 11);
        SqueakSetState(t, 1);
        return TRUE;
    case 1:                 // 0x0212756C, 0x02127444;
        SqueakMove(t);
        if (SqueakOnGround(t)) {
            Burst(t);
            break;
        }
        if (!InView(t, t->obj.base.x, t->obj.base.y, VIEW_1)) t->obj.base.flags |= 0x1000;
        break;
    case 2:                 // 0x0212736C;
        if (BohbohGone(t)) t->obj.base.flags |= 0x1000;
        break;
    }
    return FALSE;
}

static void BohbohThingMain(struct Object *obj) {
    struct SqueakObject *t = (struct SqueakObject *)obj;

    obj->kirby3 = FindTargetKirby(&obj->base);
    SqueakRun(t, BohbohThing);
    SqueakFinishFrame(t);
    if (t->variant != THING_LAVA) SqueakAnimate(t);
}

void BohbohThingStart(struct Object *obj) {
    struct SqueakObject *t = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, -1, BohbohThingMain);
    if (t->started) return;
    SqueakSetState(t, 0);
}

void BohbohThingInit(struct SqueakObject *t) {
    struct ObjectBase *base = &t->obj.base;

    T_HARM(t) = base->unk68;
    base->unk5C |= 0x80;
    base->flags |= 0x1000000 | 0x2000000;
    switch (t->variant) {
    case THING_FIRE:
        sub_0803E2B0(base, -10, -10, 10, 10);
        ObjectSetBounds(base, -7, -7, 7, 7);
        break;
    case THING_FIRE_TOP:
        sub_0803E2B0(base, -12, -12, 12, 12);
        ObjectSetBounds(base, -7, -7, 7, 7);
        break;
    default:
        sub_0803E2B0(base, -64, -2, 64, 8);
        ObjectSetBounds(base, -64, -2, 64, 8);
        base->unk5C = (base->unk5C & ~7) | 6 | 0x20;
        base->flags |= 0x100 | 0x400;
        break;
    }
}
