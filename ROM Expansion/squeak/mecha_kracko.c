/* Mecha-Kracko (Squeak Squad's); ported from Kirby: Squeak Squad (USA), boss 0x39 (its body 0x02132D7C, vtable 0x02184908: +0x34 per
 * frame 0x02132FFC, +0x30 its hit 0x02132990, +0x2C its touch 0x02132C80; its states 0x02130510-0x02132744, their table 0x02184730),
 * as SQUEAK_MECHA_KRACKO, a boss on the mid-bosses' framework (mini-boss type 0x40: the camera held on its area, the HP bar, the music,
 * KatAM's defeat). Values from param/PrmBB03Ml0.kgb (its HP, PrmBB03Sl0: 320, then 400 in its second form; here KatAM's bosses' 60
 * for one player as Mrs. Moley's 300: 64 and 80); speeds 20.12 a frame, Y up; drawn with enemy/kracko (a cloud over a spiked core,
 * its .mca's composites), its eye a sprite of its own (SQUEAK_MECHA_KRACKO_EYE, 0x0213025C); its boxes from kracko.rca. Recorded on
 * the DS in its own stage (map/a3s7s2: the spots below; its moves' frames, the second form's forced by a RAM write).
 * • Its arena (Squeak Squad's spots are from the camera: 0x02132840; here from the camera view's middle across, and the floor where it
 *   was placed up and down, a3s7s2's camera top 144 pixels over Kirby's floor): the side spots 80 pixels either side of the middle and
 *   80 over the floor (the one on its side, or the other); the low line 8 under the floor, its middle and 256 pixels either side (off
 *   the view).
 * • It comes in from over the middle's right (a3s7s2's spawn, 48 right, 136 up: 0x0213268C) to its side's spot (120 frames), then
 *   waits 90 frames (0x02132598, 0x021324A4: at its side's spot; 30 after some moves, 0x021325E0) and picks (0x021322A4):
 *   its first form: 5 in 9 (with more than 3 of its enemies out, always) a move, else an enemy; a move 4 in 9 its wave across
 *   (0x02132020: 5 arcs of 30 frames, 68 pixels high, to the other spot), else its swoop (0x02131CF4: down to the low line's middle
 *   and up on the other side, 120 frames each way, 4.0 across at first); after either, 4 in 9 the short wait, else the long one.
 *   Its enemy (0x02131B30, 0x021319C8: shaking 30 frames at its spot): a Starman 2 in 9 (the ported one), a Waddle Dee 3, a Waddle Doo
 *   4 (KatAM's), dropped where it is, facing the middle.
 *   Its second form: of 100 (with more than 3 enemies out, of 70) under 25 its dive (0x0213180C: at 2.0 to 60 over Kirby, no higher
 *   than the spots; 15 frames still, then 90 frames' electric field round it, 0x021316B8, 0x02131650; back at 2.0, 0x021311C8), under
 *   45 its crossing (0x0213153C: bolts down from it 15 frames at its spot, halfway across, the other spot; 60 frames' steps), under
 *   65 its low sweep (0x02130FF0: 120 frames out of view at its height on its side, down to the low line, 150 frames across the
 *   view, back in at its height to its side's spot, 120 frames; then an enemy, 7 in 8 with more than 8 out), under 70 its swoop
 *   in the electric field, then the low sweep (0x02131E4C), else an enemy.
 * • Its first form's last hit (0x02132990: Squeak Squad's base pause, 0x020216E8, 0x02021648: 60 frames) breaks its cloud
 *   (0x02130C64, 0x02130BB0: .mca's 4, its core alone, .mca's 0), it shakes 60 frames (0x02130ABC: 4 pixels down and up, every other
 *   frame), its cloud gathers again (.mca's 1, 0x02130A2C) and its second form's HP fills the bar; it goes back to its spot and
 *   swoops in its electric field. Hit in all its poses: its composites' parts have no hit boxes of their own (kracko.rca has none
 *   for their animations), so its sprite's first box is hit (animation 0's, its cloud: 0x02014218, 0x0201322C; as recorded in its
 *   first form); its field and bolts hurt Kirby (KatAM's one box for its touch and its hits: theirs then, as the framework's union
 *   of the two); while its HP is out (its pause, its cloud breaking and gathering) it's untouchable (Squeak Squad's hit handler
 *   and touch take nothing then).
 * Its sounds: Squeak Squad's (SqueakSfx). Not ported: its hit flash (enemy/kracko_dmg isn't in the game: its hurt palette its own, no flash), the stage's slowdown
 * in its pause (0x020216E8: the time object's speed), its low sweep and defeat filling the arena's missing cloud-floor tiles
 * (0x02216CB8: KatAM's rooms have none), its own defeat (0x02130924, 0x02130648, 0x02130584, 0x02130510: KatAM's mini-boss defeat),
 * Daroach bringing it in (the stage's scene, its own object), its boss music. */
#include "../squeak.h"
#include "kirby.h"
#include "constants/object_types.h"

// Its composite animations (kracko.mca's: Squeak Squad's keys 0x3000000 | n);
enum {
    COMPOSITE_BARE,             // Its core alone (its cloud gone);
    COMPOSITE_GATHER,           // Its cloud gathering again (28 frames);
    COMPOSITE_IDLE,             // Its cloud over its core;
    COMPOSITE_MOVE,
    COMPOSITE_BREAK,            // Its cloud breaking up (34 frames);
    COMPOSITE_UNUSED,
    COMPOSITE_FIELD,            // Its electric field;
    COMPOSITE_BOLTS,            // Its bolts down;
};
// Its eye's (SQUEAK_MECHA_KRACKO_EYE's: kracko.cea's 8-15), looking right, up-right, up, up-left, left, down-left, down, down-right;
#define ANIM_EYE 0

// PrmBB03Ml0.kgb;
#define SPOT_X 80               // +0x00, +0x04: the side spots, 80 pixels from the middle, 64 under the camera's top (80 over the floor);
#define SPOT_UP 80
#define LOW_UP (-8)             // +0x08: the low line 152 under the camera's top (8 under the floor);
#define WAIT_SHORT 30           // +0x0C, +0x10;
#define WAIT_EXTRA 60
#define WAVE_HALF 30            // +0x18, +0x1C, +0x20: its wave's arcs, how many, how high;
#define WAVES 5
#define WAVE_RISE 0x44000
#define INTRO_TIME 120          // +0x24;
#define SWOOP_TIME 120          // +0x28, +0x2C;
#define SWOOP_SPEED 0x4000
#define SHAKE_VY 0x4000         // +0x34: its shakes (4 pixels);
#define SPAWN_TIME 30           // +0x38;
#define DIVE_OVER 60            // +0x3C, +0x40;
#define DIVE_SPEED 0x2000
#define ZAP_WAIT 15             // +0x44, +0x48;
#define ZAP_TIME 90
#define BACK_SPEED 0x2000       // +0x4C;
#define CROSS_TIME 60           // +0x50;
#define OUT_TIME 120            // +0x58, +0x5C;
#define SWEEP_TIME 150
#define SHUDDER_TIME 60         // +0x64;
#define SECOND_HP_NUM 5         // PrmBB03Sl0: its second form's 400 HP to its first's 320;
#define SECOND_HP_DEN 4

#define OFF_X 256               // 0x02132840's kinds 4-8: the low line 256 pixels from the middle;
#define INTRO_X 48              // a3s7s2's spawn (192, 104: 48 right of the camera view's middle, 136 over the floor);
#define INTRO_UP 136
#define KO_TIME 60              // 0x02021648;
#define EYE_FRONT 0x40          // Its eye drawn in front of it (KatAM: a lower order);

// 0x02132840's kinds;
enum { SPOT_NEAR, SPOT_FAR, SPOT_LEFT, SPOT_RIGHT, SPOT_OUT_NEAR, SPOT_OUT_FAR, SPOT_OUT_LEFT, SPOT_LOW_MIDDLE, SPOT_OUT_RIGHT };

enum {
    MK_START, MK_INTRO, MK_INTRO_AIM, MK_INTRO_FLY, MK_WAIT_LONG, MK_WAIT_SHORT, MK_PLACE, MK_IDLE, MK_PICK, MK_WAVE, MK_WAVE_FLY,
    MK_RETURN_PICK, MK_SWOOP_PLAIN, MK_SWOOP_FIELD, MK_SWOOP, MK_SWOOP_FLY, MK_SPAWN, MK_SPAWN_SHAKE, MK_DIVE, MK_DIVE_FLY, MK_DIVE_STOP,
    MK_DIVE_FIELD, MK_BACK, MK_BACK_FLY, MK_CROSS, MK_CROSS_BOLTS, MK_CROSS_STEP, MK_SWEEP_OUT, MK_SWEEP_GO, MK_SWEEP, MK_SWEEP_BACK,
    MK_KO, MK_KO_WAIT, MK_BREAK, MK_BREAK_WAIT, MK_SHUDDER, MK_GATHER,
};

#define ENEMIES 12              // The enemies it dropped (Squeak Squad counts the stage's: 0x0201B76C);
#define PHASE(s) ((s)->vars[0])         // Squeak Squad's +0x438: its second form;
#define TIMER(s) ((s)->vars[1])         // +0xB4;
#define TIMER2(s) ((s)->vars[2])        // +0xB8;
#define SWEEP_AFTER(s) ((s)->vars[3])   // +0xBC: its swoop in its field (the low sweep after);
#define FIELD_NEXT(s) ((s)->vars[4])    // +0xC0: back from its second form's start (its field swoop next);
#define STEP_V(s) ((s)->vars[5])        // +0xC4: its crossing's speed;
#define LAST_HP(s) ((s)->vars[6])
#define HARM(s) ((s)->vars[7])
#define FLOOR(s) ((s)->vars[8])         // The floor where it was placed (a3s7s2's Kirby's);
#define EYE(s) ((s)->vars[9])
#define ENEMY(s, i) (*(struct Object **)&(s)->vars[12 + (i)])

// kracko.rca (left, top, right, bottom; Y up): its cloud's hurt box (animation 0's); its field's (6) and bolts' (7) attack boxes by frame;
static const s16 sCloudBox[4] = { -37, 26, 34, -26 };
static const s16 sFieldBoxes[][4] = {
    { -64, 37, 59, -38 }, { -46, 61, 45, -30 }, { -34, 39, 64, -40 }, { -43, 28, 45, -57 }, { -61, 43, 52, -38 }, { -45, 56, 40, -35 },
    { -41, 39, 67, -41 }, { -43, 26, 46, -61 }, { -61, 43, 52, -38 }, { -39, 65, 44, -34 }, { -33, 41, 69, -40 }, { -40, 24, 43, -57 },
    { -63, 37, 50, -34 }, { -45, 56, 45, -28 }, { -45, 36, 68, -39 }, { -45, 25, 44, -54 }, { -69, 36, 44, -36 }, { -45, 56, 42, -31 },
    { -49, 38, 64, -39 }, { -40, 30, 42, -58 }, { -67, 37, 46, -38 }, { -43, 56, 40, -30 }, { -44, 37, 63, -38 }, { -47, 25, 46, -64 },
};
static const s16 sBoltBoxes[][4] = {
    { -37, 26, 34, -26 }, { -11, 26, 18, -140 }, { -30, 17, -5, -121 }, { -48, -121, -23, -160 }, { 7, 24, 32, -127 },
    { 21, -120, 46, -164 }, { -10, 7, 15, -144 }, { -46, 27, 45, -51 }, { -39, 25, -1, -126 }, { -59, -112, -34, -168 },
    { 7, 24, 41, -127 }, { 33, -136, 58, -174 }, { -42, 18, 44, -60 }, { -11, 19, 14, -132 }, { -42, 18, -17, -133 },
    { -54, -127, -29, -164 }, { 12, 23, 37, -128 }, { 30, -109, 55, -161 },
};

static u32 Random(u32 n) {
    return Rand16() * n >> 16;
}

static s32 T(struct SqueakObject *s) {
    return (s16)s->stateTimer;
}

// FxMul(); Squeak Squad's 20.12 multiply ((a * b + 0x800) >> 12 on the 64-bit product), in 32-bit pieces (KatAM has no __muldi3);
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

// Middle(); The camera view's middle across (Squeak Squad's camera + 128: here KatAM's view, 240 across);
static s32 Middle(struct SqueakObject *s) {
    return gCurLevelInfo[s->obj.base.unk56].viewportPosition.x + (DISPLAY_WIDTH / 2 << 8);
}

// Spot(); 0x02132840: a spot of its arena (KatAM's 24.8), by kind;
static void Spot(struct SqueakObject *s, u8 kind, s32 *x, s32 *y) {
    s32 middle = Middle(s);

    switch (kind) {
    case SPOT_NEAR: kind = middle < s->obj.base.x ? SPOT_RIGHT : SPOT_LEFT; break;
    case SPOT_FAR: kind = middle < s->obj.base.x ? SPOT_LEFT : SPOT_RIGHT; break;
    case SPOT_OUT_NEAR: kind = middle < s->obj.base.x ? SPOT_OUT_RIGHT : SPOT_OUT_LEFT; break;
    case SPOT_OUT_FAR: kind = middle < s->obj.base.x ? SPOT_OUT_LEFT : SPOT_OUT_RIGHT; break;
    }
    switch (kind) {
    case SPOT_LEFT: *x = middle - (SPOT_X << 8); *y = FLOOR(s) - (SPOT_UP << 8); break;
    case SPOT_RIGHT: *x = middle + (SPOT_X << 8); *y = FLOOR(s) - (SPOT_UP << 8); break;
    case SPOT_OUT_LEFT: *x = middle - (OFF_X << 8); *y = FLOOR(s) - (LOW_UP << 8); break;
    case SPOT_OUT_RIGHT: *x = middle + (OFF_X << 8); *y = FLOOR(s) - (LOW_UP << 8); break;
    default: *x = middle; *y = FLOOR(s) - (LOW_UP << 8); break;
    }
}

// Ahead(), Up(); 0x0201AE58: how far a spot is ahead of it (it faces right all its fight) and over it, Squeak Squad's units;
static s32 Ahead(struct SqueakObject *s, s32 x) {
    return (x - s->obj.base.x) << 4;
}

static s32 Up(struct SqueakObject *s, s32 y) {
    return (s->obj.base.y - y) << 4;
}

/* MoveFree(); Its moves (Squeak Squad's flag 0x20000: not held in the stage, nor in its terrain: it flies out of view and through the
 * floor's line); */
static void MoveFree(struct SqueakObject *s) {
    struct ObjectBase *base = &s->obj.base;

    s->vx += s->ax;
    s->vy += s->ay;
    base->xspeed = SQUEAK_SPEED(s->vx);
    base->yspeed = SQUEAK_SPEED(s->vy);
    base->unk48 = base->x;
    base->unk4C = base->y;
    base->x += base->xspeed;
    base->y -= base->yspeed;
    base->unk62 = 0;
    s->terrain = 0;
    s->moved = TRUE;
}

// Toward(); Its speed to a spot at `speed`, and the frames it takes (0x0213180C, 0x021311C8: the distance over the speed);
static void Toward(struct SqueakObject *s, s32 x, s32 y, s32 speed, bool32 roundY) {
    s32 dx = Ahead(s, x), dy = Up(s, y), d = FxSqrt(FxMul(dx, dx) + FxMul(dy, dy)), k;

    if (d == 0) {
        TIMER(s) = 0;
        return;
    }
    k = (speed << 12) / d;
    s->vx = FxMul(dx, k);
    // (the dive's up speed is the speed times the distance up over the distance, as a 32-bit product: 0x0213180C);
    s->vy = roundY ? FxMul(dy, k) : (s32)((u32)speed * (u32)dy) / d;
    TIMER(s) = d / speed;
}

/* EnemiesOut(); Its enemies still about (Squeak Squad counts the stage's: 0x0201B76C); SpawnEnemy(): 0x021319C8: one where it is,
 * facing the middle: a Starman 2 in 9 (the ported one), a Waddle Dee 3 in 9, a Waddle Doo 4 in 9 (KatAM's: walking, Squeak Squad's
 * variant 0); none when it can count no more; */
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

static void SpawnEnemy(struct SqueakObject *s) {
    u32 r = Random(9);
    struct Object *e;
    u8 i;

    EnemiesOut(s);
    for (i = 0; i < ENEMIES && ENEMY(s, i); i++) ;
    if (i >= ENEMIES || gKirbys[gLocalPlayerId].base.roomId != s->obj.base.roomId) return;
    if (r < 2) {
        struct SqueakObject *star = SqueakSpawn(s, SQUEAK_STARMAN, 0, 0, 0);

        e = star ? &star->obj : NULL;
    } else {
        e = CreateObjTemplateAndObj(s->obj.base.unk56, 1, 0x24, s->obj.base.x >> 8, s->obj.base.y >> 8, 0, 0x1F, 0, 0,
                                    r < 5 ? OBJ_WADDLE_DEE_1 : OBJ_WADDLE_DOO, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    }
    if (e == NULL) return;
    ENEMY(s, i) = e;
    e->base.parent = &s->obj;
    if (Middle(s) < s->obj.base.x) e->base.flags |= 1;
    else e->base.flags &= ~1;
}

// Changing(); Its first form's HP out: its pause, its cloud breaking and gathering (Squeak Squad's HP 0 till its second form's);
static bool32 Changing(struct SqueakObject *s) {
    return s->state >= MK_KO && s->state <= MK_GATHER;
}

// Hittable(); Its HP not out (anywhere else: its sprite's first box, 0x0201322C);
static bool32 Hittable(struct SqueakObject *s) {
    return !Changing(s);
}

static bool32 MechaKracko(struct SqueakObject *s) {
    s32 x, y, ax;
    u32 r;

    switch (s->state) {
    case MK_START:          // Its fight's start: where a3s7s2 has it (Squeak Squad's spawn), facing right (all its fight), through the
        SqueakStop(s);      // terrain (KatAM's flag 0x100: its low line is under the floor's);
        s->obj.base.flags = (s->obj.base.flags & ~(1 | 0x400)) | 0x100;
        s->obj.base.x = Middle(s) + (INTRO_X << 8);
        s->obj.base.y = FLOOR(s) - (INTRO_UP << 8);
        SqueakSetState(s, MK_INTRO);
        return TRUE;
    case MK_INTRO:          // 0x02132744;
        SqueakPlayComposite(s, COMPOSITE_IDLE);
        SqueakSetState(s, MK_INTRO_AIM);
        return TRUE;
    case MK_INTRO_AIM:      // 0x0213268C: to its side's spot in INTRO_TIME frames;
        SqueakStop(s);
        Spot(s, SPOT_NEAR, &x, &y);
        s->vx = Ahead(s, x) / INTRO_TIME;
        s->vy = Up(s, y) / INTRO_TIME;
        SqueakSetState(s, MK_INTRO_FLY);
        return TRUE;
    case MK_INTRO_FLY:      // 0x02132624;
        if (T(s) >= INTRO_TIME) {
            SqueakSetState(s, MK_WAIT_LONG);
            return TRUE;
        }
        MoveFree(s);
        break;
    case MK_WAIT_LONG:      // 0x02132598;
        TIMER(s) = WAIT_SHORT + WAIT_EXTRA;
        SqueakSetState(s, MK_PLACE);
        return TRUE;
    case MK_WAIT_SHORT:     // 0x021325E0;
        TIMER(s) = WAIT_SHORT;
        SqueakSetState(s, MK_PLACE);
        return TRUE;
    case MK_PLACE:          // 0x021324A4: at its side's spot;
        Spot(s, SPOT_NEAR, &s->obj.base.x, &s->obj.base.y);
        SqueakStop(s);
        SqueakPlayComposite(s, COMPOSITE_IDLE);
        SqueakSetState(s, MK_IDLE);
        return TRUE;
    case MK_IDLE:           // 0x021322A4;
        if (T(s) < TIMER(s)) {
            MoveFree(s);
            break;
        }
        if (!PHASE(s)) {
            SqueakSetState(s, Random(9) < (EnemiesOut(s) > 3 ? 10 : 5) ? MK_PICK : MK_SPAWN);
            return TRUE;
        }
        r = EnemiesOut(s) > 3 ? Random(70) : Random(100);
        if (r < 25) SqueakSetState(s, MK_DIVE);
        else if (r < 45) SqueakSetState(s, MK_CROSS);
        else if (r < 65) SqueakSetState(s, MK_SWEEP_OUT);
        else if (r < 70) SqueakSetState(s, MK_SWOOP_FIELD);
        else SqueakSetState(s, MK_SPAWN);
        return TRUE;
    case MK_PICK:           // 0x02132220;
        SqueakSetState(s, Random(9) < 4 ? MK_WAVE : MK_SWOOP_PLAIN);
        return TRUE;
    case MK_WAVE:           // 0x02132020: to the other spot in WAVES arcs of WAVE_HALF frames, WAVE_RISE high;
        SqueakStop(s);
        Spot(s, SPOT_FAR, &x, &y);
        s->vx = Ahead(s, x) / (WAVES * WAVE_HALF - 1);
        s->vy = Up(s, y) / (WAVES * WAVE_HALF - 1);
        s->ay = -WAVE_RISE / ((WAVE_HALF / 2) * (WAVE_HALF / 2));
        s->vy -= s->ay * (WAVE_HALF / 2 - 1);
        TIMER(s) = WAVE_HALF;
        TIMER2(s) = WAVES;
        SqueakPlayComposite(s, COMPOSITE_IDLE);
        SqueakSfx(s, 0x18E);
        SqueakSetState(s, MK_WAVE_FLY);
        return TRUE;
    case MK_WAVE_FLY:       // 0x02131F7C;
        if (--TIMER(s) < 1) {
            TIMER(s) = WAVE_HALF;
            SqueakSfx(s, 0x18E);
            s->ay = -s->ay;
            if (--TIMER2(s) < 1) {
                SqueakSetState(s, MK_RETURN_PICK);
                return TRUE;
            }
        }
        MoveFree(s);
        break;
    case MK_RETURN_PICK:    // 0x0213219C;
        SqueakSetState(s, Random(9) < 4 ? MK_WAIT_SHORT : MK_WAIT_LONG);
        return TRUE;
    case MK_SWOOP_PLAIN:    // 0x02131EE4;
        SqueakPlayComposite(s, COMPOSITE_MOVE);
        SWEEP_AFTER(s) = FALSE;
        SqueakSetState(s, MK_SWOOP);
        return TRUE;
    case MK_SWOOP_FIELD:    // 0x02131E4C: in its field, the low sweep after;
        SqueakPlayComposite(s, COMPOSITE_FIELD);
        SWEEP_AFTER(s) = TRUE;
        SqueakSetState(s, MK_SWOOP);
        return TRUE;
    case MK_SWOOP:          // 0x02131CF4: away at SWOOP_SPEED, down to the low line's middle in SWOOP_TIME frames, and as far on;
        SqueakStop(s);
        Spot(s, SPOT_LOW_MIDDLE, &x, &y);
        {
            s32 dx = Ahead(s, x), dy = Up(s, y), ux = dx < 0 ? -dx : dx, uy = dy < 0 ? -dy : dy;
            s32 accelX = (ux + SWOOP_SPEED * SWOOP_TIME) * 2 / (SWOOP_TIME * SWOOP_TIME);
            s32 accelY = uy * 2 / (SWOOP_TIME * SWOOP_TIME);

            s->vx = dx < 1 ? SWOOP_SPEED : -SWOOP_SPEED;
            s->ax = dx < 1 ? -accelX : accelX;
            s->vy = -(SWOOP_TIME * accelY);
            s->ay = accelY;
        }
        TIMER(s) = SWOOP_TIME - 1;
        TIMER2(s) = TIMER(s) * 2 - 1;
        SqueakSfx(s, 0x187);
        if (SWEEP_AFTER(s)) SqueakLoopSfx(s, 0x190);
        SqueakSetState(s, MK_SWOOP_FLY);
        return TRUE;
    case MK_SWOOP_FLY:      // 0x02131C1C: its pull across turned round halfway;
        if (T(s) > TIMER2(s)) {
            SqueakStopSfx(s, 0x190);
            SqueakSetState(s, SWEEP_AFTER(s) ? MK_SWEEP_OUT : MK_RETURN_PICK);
            return TRUE;
        }
        ax = s->ax;
        if (T(s) == TIMER(s)) SqueakSfx(s, 0x187);
        if (T(s) == TIMER(s)) s->ax = 0;
        MoveFree(s);
        if (T(s) == TIMER(s)) s->ax = -ax;
        break;
    case MK_SPAWN:          // 0x02131B30;
        SqueakStop(s);
        s->vy = SHAKE_VY;
        SqueakPlayComposite(s, COMPOSITE_MOVE);
        SqueakSetState(s, MK_SPAWN_SHAKE);
        return TRUE;
    case MK_SPAWN_SHAKE:    // 0x021319C8: shaking at its spot SPAWN_TIME frames, then its enemy;
        Spot(s, SPOT_NEAR, &s->obj.base.x, &s->obj.base.y);
        if (!(T(s) & 2)) s->obj.base.y -= SQUEAK_SPEED(SHAKE_VY);
        MoveFree(s);
        if (T(s) > SPAWN_TIME) {
            SpawnEnemy(s);
            SqueakSfx(s, 0x18D);
            SqueakSetState(s, MK_WAIT_SHORT);
        }
        break;
    case MK_DIVE:           // 0x0213180C: to DIVE_OVER pixels over Kirby, no higher than the spots;
        SqueakStop(s);
        Spot(s, SPOT_NEAR, &x, &y);
        {
            s32 ty = s->obj.kirby3->base.y - (DIVE_OVER << 8);

            Toward(s, s->obj.kirby3->base.x, ty < y ? y : ty, DIVE_SPEED, FALSE);
        }
        SqueakPlayComposite(s, COMPOSITE_MOVE);
        SqueakSetState(s, MK_DIVE_FLY);
        return TRUE;
    case MK_DIVE_FLY:       // 0x02131778;
        if (TIMER(s) <= T(s)) {
            SqueakStop(s);
            SqueakSetState(s, MK_DIVE_STOP);
            return TRUE;
        }
        MoveFree(s);
        break;
    case MK_DIVE_STOP:      // 0x021316B8;
        if (T(s) >= ZAP_WAIT) {
            SqueakPlayComposite(s, COMPOSITE_FIELD);
            SqueakSfx(s, 0x18C);
            SqueakLoopSfx(s, 0x190);
            SqueakSetState(s, MK_DIVE_FIELD);
            return TRUE;
        }
        MoveFree(s);
        break;
    case MK_DIVE_FIELD:     // 0x02131650;
        if (T(s) >= ZAP_TIME) {
            SqueakStopSfx(s, 0x190);
            SqueakSetState(s, MK_BACK);
            return TRUE;
        }
        MoveFree(s);
        break;
    case MK_BACK:           // 0x021311C8: back to its side's spot;
        SqueakStop(s);
        Spot(s, SPOT_NEAR, &x, &y);
        Toward(s, x, y, BACK_SPEED, TRUE);
        SqueakPlayComposite(s, COMPOSITE_MOVE);
        SqueakSfx(s, 0x18E);
        SqueakSetState(s, MK_BACK_FLY);
        return TRUE;
    case MK_BACK_FLY:       // 0x021310F4;
        if (TIMER(s) <= T(s)) {
            SqueakStop(s);
            SqueakSetState(s, FIELD_NEXT(s) ? MK_SWOOP_FIELD : MK_WAIT_LONG);
            FIELD_NEXT(s) = FALSE;
            return TRUE;
        }
        MoveFree(s);
        break;
    case MK_CROSS:          // 0x0213153C: to the other spot in two steps, bolts at each stop;
        SqueakStop(s);
        Spot(s, SPOT_FAR, &x, &y);
        STEP_V(s) = Ahead(s, x) / (CROSS_TIME * 2 + 2);
        TIMER(s) = 0;
        SqueakPlayComposite(s, COMPOSITE_BOLTS);
        SqueakSetState(s, MK_CROSS_BOLTS);
        return TRUE;
    case MK_CROSS_BOLTS:    // 0x02131434;
        if (T(s) < ZAP_WAIT) {
            MoveFree(s);
            break;
        }
        if (++TIMER(s) > 2) {
            SqueakSetState(s, MK_WAIT_LONG);
            return TRUE;
        }
        s->vx = STEP_V(s);
        SqueakPlayComposite(s, COMPOSITE_MOVE);
        SqueakSfx(s, 0x188);
        SqueakSetState(s, MK_CROSS_STEP);
        return TRUE;
    case MK_CROSS_STEP:     // 0x02131374;
        if (T(s) >= CROSS_TIME) {
            s->vx = s->vy = 0;
            SqueakPlayComposite(s, COMPOSITE_BOLTS);
            SqueakSetState(s, MK_CROSS_BOLTS);
            return TRUE;
        }
        MoveFree(s);
        break;
    case MK_SWEEP_OUT:      // 0x02130FF0: out of view on its side, at its height;
        SqueakStop(s);
        Spot(s, SPOT_OUT_NEAR, &x, &y);
        s->vx = Ahead(s, x) / OUT_TIME;
        SqueakPlayComposite(s, COMPOSITE_MOVE);
        SqueakSetState(s, MK_SWEEP_GO);
        return TRUE;
    case MK_SWEEP_GO:       // 0x02130F40: down to the low line, across to the other side's;
        if (T(s) >= OUT_TIME) {
            Spot(s, SPOT_OUT_FAR, &x, &y);
            s->obj.base.y = y;
            s->vx = Ahead(s, x) / SWEEP_TIME;
            TIMER(s) = 0;
            SqueakSfx(s, 0x186);
            SqueakSetState(s, MK_SWEEP);
            return TRUE;
        }
        MoveFree(s);
        break;
    case MK_SWEEP:          // 0x02130DF8 (the cloud floor it fills: not ported); back up, in to its side's spot;
        if (T(s) >= SWEEP_TIME) {
            SqueakStop(s);
            Spot(s, SPOT_NEAR, &x, &y);
            s->obj.base.y = y;
            s->vx = Ahead(s, x) / OUT_TIME;
            SqueakSetState(s, MK_SWEEP_BACK);
            return TRUE;
        }
        MoveFree(s);
        break;
    case MK_SWEEP_BACK:     // 0x02130D40: an enemy after (7 in 8 with more than 8 out);
        if (T(s) >= OUT_TIME) {
            SqueakSetState(s, Random(8) < (EnemiesOut(s) > 8 ? 7 : 10) ? MK_SPAWN : MK_WAIT_LONG);
            return TRUE;
        }
        MoveFree(s);
        break;
    case MK_KO:             // 0x020216E8: its first form's last hit;
        SqueakStop(s);
        SqueakSfx(s, 0x10D);
        SqueakSetState(s, MK_KO_WAIT);
        return TRUE;
    case MK_KO_WAIT:        // 0x02021648;
        if (T(s) >= KO_TIME) {
            SqueakSetState(s, MK_BREAK);
            return TRUE;
        }
        break;
    case MK_BREAK:          // 0x02130C64;
        SqueakStop(s);
        SqueakPlayComposite(s, COMPOSITE_BREAK);
        SqueakSetState(s, MK_BREAK_WAIT);
        return TRUE;
    case MK_BREAK_WAIT:     // 0x02130BB0;
        if (s->animEnded) {
            SqueakPlayComposite(s, COMPOSITE_BARE);
            SqueakSfx(s, 0x18C);
            SqueakSetState(s, MK_SHUDDER);
            return TRUE;
        }
        MoveFree(s);
        break;
    case MK_SHUDDER:        // 0x02130ABC: 4 pixels down and back up, every other frame;
        if (T(s) >= SHUDDER_TIME) {
            SqueakPlayComposite(s, COMPOSITE_GATHER);
            SqueakSfx(s, 0x18F);
            SqueakSetState(s, MK_GATHER);
            return TRUE;
        }
        if (T(s) & 1) s->vy = (T(s) >> 1) & 1 ? SHAKE_VY : -SHAKE_VY;
        MoveFree(s);
        s->vy = 0;
        break;
    case MK_GATHER:         // 0x02130A2C: its second form, its HP the bar's full;
        if (s->animEnded) {
            struct Object *obj = &s->obj;

            PHASE(s) = TRUE;
            obj->unk80 = s->hpBase;
            s->maxHp = s->hpBase;
            s->hpBase = 0;
            LAST_HP(s) = obj->unk80;
            sub_08034E14(obj);
            FIELD_NEXT(s) = TRUE;
            SqueakSetState(s, MK_BACK);
            return TRUE;
        }
        MoveFree(s);
        break;
    }
    return FALSE;
}

/* Hit(); 0x02132990 (KatAM's mini-bosses take hits without a hurt state of their own: a fall of its HP is the hit): its first form's
 * HP out (the HP over its second form's), its pause and change; a hit where its boxes take none undone; */
static void Hit(struct SqueakObject *s) {
    struct Object *obj = &s->obj;

    if (obj->unk80 < LAST_HP(s) && obj->unk80 > 0 && !(obj->base.flags & 0x1000)) {
        if (!Hittable(s)) {
            obj->unk80 = LAST_HP(s);
        } else if (!PHASE(s) && obj->unk80 <= s->hpBase) {
            obj->unk80 = s->hpBase;
            SqueakSetState(s, MK_KO);
        }
    }
    LAST_HP(s) = obj->unk80;
}

/* Boxes(); Its box by its composite's frame (kracko.rca's, KatAM's Y down, within KatAM's reach of 127 pixels: its bolts' ends under
 * the floor): its cloud hit and hurting, its field and bolts hurting and hit (KatAM's one box), none while its cloud breaks and
 * gathers or its HP is out (untouchable: KatAM's level 6, as Gordo's); */
static void Boxes(struct SqueakObject *s) {
    struct ObjectBase *base = &s->obj.base;
    const s16 *b;

    switch (s->composite) {
    case COMPOSITE_IDLE: case COMPOSITE_MOVE: b = sCloudBox; break;
    case COMPOSITE_FIELD: b = sFieldBoxes[s->compositeTime % ARRAY_COUNT(sFieldBoxes)]; break;
    case COMPOSITE_BOLTS: b = sBoltBoxes[s->compositeTime % ARRAY_COUNT(sBoltBoxes)]; break;
    default: b = NULL; break;
    }
    if (b == NULL || Changing(s)) {
        base->flags |= 0x200;
        base->unk68 = 0;
        return;
    }
    base->flags &= ~0x200;
    sub_0803E2B0(base, b[0], -b[1] > 127 ? 127 : -b[1], b[2], -b[3] > 127 ? 127 : -b[3]);
    base->unk68 = HARM(s);
    base->unk5C = (base->unk5C & ~7) | (Hittable(s) ? 3 : 6);
}

/* Eye(); Its eye (0x0213025C, drawn by a sprite of its own: SQUEAK_MECHA_KRACKO_EYE): made once its fight starts, again if lost; */
static void Eye(struct SqueakObject *s) {
    struct SqueakObject *e = (struct SqueakObject *)EYE(s);

    if (e && e->obj.base.header.kind == 1 && !(e->obj.base.flags & 0x1000) && e->obj.base.parent == &s->obj
        && SqueakIsSqueak(&e->obj) && e->enemy == SQUEAK_MECHA_KRACKO_EYE)
        return;
    e = SqueakSpawn(s, SQUEAK_MECHA_KRACKO_EYE, 0, 0, 0);
    EYE(s) = (s32)e;
    if (e) e->obj.base.parent = &s->obj;
}

static void MechaKrackoMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    obj->kirby3 = FindTargetKirby(&obj->base);
    Hit(s);
    SqueakRun(s, MechaKracko);
    if (obj->unk80 <= 0) return;
    // (its HP bar Squeak Squad's boss's, which its enemies' hits don't take: KatAM's goes to whatever is hit, taken back);
    sub_08034E14(obj);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
    Boxes(s);
    Eye(s);
}

/* MechaKrackoStart(); Its fight's start (and after a hit: it goes on, as KatAM's mini-bosses do). It isn't held in the camera's area
 * (Squeak Squad's flag 0x20000: it flies out of view; KatAM keeps a mini-boss, flag 0x2000000, out of view); */
void MechaKrackoStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, -1, MechaKrackoMain);
    if (s->started) return;
    PHASE(s) = FALSE;
    FIELD_NEXT(s) = FALSE;
    LAST_HP(s) = obj->unk80;
    SqueakSetState(s, MK_START);
}

/* MechaKrackoInit(); Its box (its cloud's), its bounds (KatAM's), the floor where it's placed, its HP: its first form's over its
 * second's (PrmBB03Sl0's 400 to 320 of its own), the bar its first's; hidden, still, till its fight; */
void MechaKrackoInit(struct SqueakObject *s) {
    struct Object *obj = &s->obj;
    s16 first = obj->unk80;

    sub_0803E2B0(&obj->base, sCloudBox[0], -sCloudBox[1], sCloudBox[2], -sCloudBox[3]);
    ObjectSetBounds(&obj->base, -16, -16, 16, 16);
    HARM(s) = obj->base.unk68;
    SqueakStandOnFloor(obj);
    FLOOR(s) = obj->base.y;
    s->hpBase = first * SECOND_HP_NUM / SECOND_HP_DEN;
    s->maxHp = first;
    obj->unk80 = first + s->hpBase;
    obj->base.flags |= 0x400;
    // (on the floor while it waits: the mid-boss wait's fall, NidlFall, stops on it; the engine's terrain is off for it, flag 0x40);
    obj->base.unk62 |= 4;
    SqueakPlayComposite(s, COMPOSITE_IDLE);
}

/* Its eye (0x0213025C): where it is, while its cloud shows (.mca's 2, 3, 6 and 7), looking at Kirby: kracko.cea's 8-15 by the way
 * to him (straight across under tan 22.5°'s slope, along it over tan 67.5°'s: 0x022016F0's, 1696 and 9888 in 20.12), in front of it;
 * untouchable, through the terrain as it, gone with it; */
#define TAN_22 1696
#define TAN_67 9888

static u8 EyeLook(struct SqueakObject *k) {
    // (the slope 0x02143178 takes, dx / dy in 20.12: from KatAM's 24.8, as the room's distances keep it within 32 bits);
    s32 dx = k->obj.kirby3->base.x - k->obj.base.x, dy = k->obj.base.y - k->obj.kirby3->base.y, r;

    if (dy == 0) return dx > 0 ? 0 : 4;
    r = (dx << 12) / dy;
    if (dy > 0) {
        if (dx > 0) return r > TAN_67 ? 0 : r > TAN_22 ? 1 : 2;
        return -r > TAN_67 ? 4 : -r > TAN_22 ? 3 : 2;
    }
    if (dx > 0) return -r > TAN_67 ? 0 : -r > TAN_22 ? 7 : 6;
    return r > TAN_67 ? 4 : r > TAN_22 ? 5 : 6;
}

static void EyeMain(struct Object *obj) {
    struct SqueakObject *e = (struct SqueakObject *)obj, *k = (struct SqueakObject *)obj->base.parent;
    u8 c;

    if (k == NULL || k->obj.base.header.kind != 1 || (k->obj.base.flags & 0x1000) || k->obj.unk80 <= 0 || !SqueakIsSqueak(&k->obj)
        || k->enemy != SQUEAK_MECHA_KRACKO) {
        obj->base.flags |= 0x1000;
        return;
    }
    k->obj.kirby3 = FindTargetKirby(&k->obj.base);
    obj->base.x = k->obj.base.x;
    obj->base.y = k->obj.base.y;
    obj->base.flags &= ~1;
    obj->base.sprite.unk14 = k->obj.base.sprite.unk14 - EYE_FRONT;
    c = k->composite;
    if ((k->obj.base.flags & 0x400)
        || !(c == COMPOSITE_IDLE || c == COMPOSITE_MOVE || c == COMPOSITE_FIELD || c == COMPOSITE_BOLTS)) {
        obj->base.flags |= 0x400;
        return;
    }
    obj->base.flags &= ~0x400;
    if (e->anim != ANIM_EYE + EyeLook(k)) SqueakPlay(e, ANIM_EYE + EyeLook(k));
    SqueakAnimate(e);
}

void MechaKrackoEyeStart(struct Object *obj) {
    ObjectSetFunc(obj, -1, EyeMain);
    obj->base.flags |= 0x200;
    obj->base.unk68 = 0;
}

void MechaKrackoEyeInit(struct SqueakObject *e) {
    sub_0803E2B0(&e->obj.base, -4, -4, 4, 4);
    ObjectSetBounds(&e->obj.base, -4, -4, 4, 4);
    e->obj.base.unk5C |= 0x20 | 0x80;
    e->obj.base.flags |= 0x100 | 0x200 | 0x400 | 0x1000000 | 0x2000000;
    SqueakPlay(e, ANIM_EYE);
}
