/* Daroach and Dark Daroach (Squeak Squad's); ported from Kirby: Squeak Squad (USA), boss 0x3D (his body 0x02126F48, vtable 0x02183680:
 * +0x3C his idle 0x02126658, +0x30 his hit 0x02126B8C, +0x2C his touch 0x02126E00; his states 0x02124DCC-0x021267EC), as
 * SQUEAK_DAROACH_BOSS (the stage a6s7s2's, parameter 0) and SQUEAK_DARK_DAROACH (a8s4s2's, parameter 1: +0x1F8, his "phase": the
 * other palette, dancho01.cla's, for both his sprites, and his phase's moves), bosses on the mid-bosses' framework (mini-boss type
 * 0x40: the camera held on its area, the HP bar, the music, KatAM's defeat). Values from param/prmBB07Ml0.kgb (moves) and
 * prmBB07Sl0.kgb (his HP, 400: here KatAM's bosses' 80, as Bohboh's 350 is 70); speeds 20.12 a frame, Y up; drawn with
 * enemy/dancho00 and dancho01 (the sprite his body switches to, its +0xE4: his wand's poses and beam), their .mca's composites. Recorded
 * on the DS in his stage (map/a6s7s2, loaded by repointing the stage table: his points, his warps, hops, beam, bombs and stars).
 * • His arena (a6s7s2: 288 by 240, Kirby's floor 80 up; here KatAM's camera view's middle across, the stage's 144, and the floor
 *   where he was placed, as Bohboh's): his points (0x02209CC0) 0 and 1 (80 left and right of the middle, 92 over the floor), 2 and 3
 *   (as far out, 20 over it) and 4 (8 right of the middle, 64 over it); the zones Kirby is in (the stage's grid): left of 16 before
 *   the middle 0 (64 over the floor or more) or 2, right of 16 past it 1 or 3, between 4. He starts at point 4, his spawn 4 right of
 *   the middle and 92 over the floor.
 * • His intro (0x021267EC, 0x02126790): his float (animation 1) once, then (his HP bar) he idles.
 * • Idle (0x02126658): faces in from his point (0 and 2 right, 1 and 3 left; at 4 toward Kirby, when he's under 16 pixels ahead),
 *   waits 10 frames 3 in 9, else 40, then picks (0x02126414) by Kirby's zone (0x0212688C: whether his point is on Kirby's side, the
 *   point on that side up top or down low, and the low one across, or point 2 from the middle, then the low point across from his
 *   own): off Kirby's side, 1 in 9 he waits 60 more, 3 in 9 a warp (to Kirby's side, once 3 in 9; else across low, three warps), else
 *   his stars (3 in 9) or a bomb; on it, 3 in 9 a warp to the far point (3 in 9) or a hop across low, else his stars or his beam.
 *   - His warp (0x02126140-0x02125F4C): he vanishes (animation 2), 10 frames later shows at the point (3), facing in; more warps
 *     go between the low points and the top ones (to 2 from 0 and 1, else 1).
 *   - His hop (0x02126294, 0x02126228): to the point in 40 frames (under 32 pixels across), 30 (under 128) or 50, 0.1 gravity.
 *   - His beam (0x021257F8-0x0212512C): a hop to his point's x at Kirby's height (40 frames, 0.1 gravity: the floor stops him),
 *     his wand raised (dancho01's 1), its charge (dancho01.mca's 0; Dark Daroach's 1) 60 frames following Kirby's height (at 0.05
 *     of the distance a frame, 2.5 at most), a frame's pause, his aim (dancho01's 3), the beam (.mca's 2; Dark Daroach's 3, backing
 *     off at 0.3) 120 frames, his wand down (16), then a hop again 3 in 9, else a warp to Kirby's far point 3 in 9 or a hop across.
 *   - His bomb (0x02125B28-0x02125950): wound up (animation 8), thrown (9: 0x02124334, 16 ahead and 16 up), then idle 3 in 9, else
 *     a warp to Kirby's side or three across.
 *   - His stars (0x02125E6C-0x02125C08): his throw (.mca's 1, then 2) of three stars (0x02124920, 16 ahead and 16 up), then idle 3
 *     in 9, else after his stars a warp or a hop, after a pick a warp or three, as after his bomb.
 * • Hit (0x02126B8C, 0x0201322C): anywhere but in his warp (dancho00.rca's boxes flagged 0x8000 in its animations 2 and 3) or hidden;
 *   where an animation has no box of its own, his sprite's first, animation 0's (0x02014218: 16 behind, 12 ahead, 1 to 40 up), as
 *   recorded; 10 frames safe after a hit (+0xEC). His touch hurts (dancho00.rca's kind 0 boxes: none in his warp; in his charge 1
 *   frame in 8, in his beam 4 in 6).
 * • What he brings in (SQUEAK_DAROACH_BOSS_THING, SQUEAK_DARK_DAROACH_THING):
 *   - His stars (0x02124894, 0x021246F4, 0x021243D8): at 4.0, 45° down ahead (the second 75°, the third 15° from straight down);
 *     off the floor up again, off a ceiling down, off a wall turned round, each at one of the three angles (rand(3)); gone after
 *     100 frames, on touching Kirby (its touch, 0x02124844) or with him (a star burst); can be inhaled (its box 0x102's byte 0,
 *     KatAM's: no ability); Squeak Squad's attacks don't break it (its hit handler takes only inhales), KatAM's do.
 *   - His bomb (0x02124290, 0x0212404C, 0x02123E80): lobbed at Kirby, 45 frames, 0.05 gravity; on the ground it bursts (the stage's
 *     burst, its blast's box: dancho00's 21), a Bomb when inhaled (0x0203A9C4's 9). Dark Daroach's (animation 11) stops where it
 *     lands (0.5 across at most), and 180 frames later lights (12), its fire (.mca's 3: a column 128 up, its sparks) 120 frames,
 *     then out (.mca's 4); gone with him (a star burst).
 *   - His beam's reach (dancho01.rca's box on the beam's part, 48 ahead: 31 to 332 pixels ahead, 4 under to 5 over his spot; Dark
 *     Daroach's 31 to 320, 14 under to 15 over): KatAM's boxes reach 127 pixels from an object, so two of these, each half of it,
 *     kept on him while the beam shows; hurting, drawing nothing, no attack hurts them.
 * • Dark Daroach's defeat (0x02124EFC, then 0x02124DCC every frame; the owner: "Daroach when beaten on the first phase doesn't
 *   trigger the expected animation when it dies"): KatAM's mini-boss defeat caught the frame it starts (DaroachWatch, his unk7C, as
 *   Dark Nebula's) and his own instead: he stops where he is (his speeds none), the screen shakes (0x02035FC4's 3, 5: KatAM's
 *   shortest), his wand's sprite gone (+0xE4), dancho00's 27 (his beaten pose). DEFEAT_BURST frames on (+0x7C): efc_cmn0's 10 at
 *   him (0x02039B28), his wand (0x02124D18: dancho00's 28, an object of its own, SPAWN_AHEAD ahead and SPAWN_UP up) thrown off at
 *   1.0 ahead and 4.0 up, 0.1 gravity (0x02124B9C), bouncing off the floor at 4.0 up and off walls turned round (0x02124A7C; sound
 *   0x7D each), a bigger shake (4, 15: KatAM's longest timed one) and his own colors back (0x02075FF4's "enemy/dancho00": the dark
 *   one's gone, Daroach's palette). DEFEAT_END frames on (+0x90) the stage moves on (0x020B3CE4): here he's gone, and the room's
 *   way on (squeak/gimmicks.c's PhaseWarp, waiting for him) takes the Kirbys to the dark star's chase. Daroach's own defeat (his
 *   stage's: a fanfare, the boss's shard) is KatAM's mini-boss defeat, as the other bosses'.
 * His hurt flash: the DS's (Hit(): enemy/dan_dmg's palettes, KatAMExpansionSqueak.cs's DamagePalette, squeak.c's SqueakDamageColors).
 * His sounds: Squeak Squad's (SqueakSfx). Not ported: the screen's shakes in his fight, the wand's rest (0x021249E8: once back in a cell of the stage's kind 4, it hangs there and flies off), touching an
 * invincible Kirby (KatAM's own), his boss music. */
#include "../squeak.h"
#include "kirby.h"
#include "constants/kirby.h"

void RequestScreenShake(u16, struct ObjectBase *);

// Animations (dancho00.cea's, then dancho01.cea's from WAND: the sprite his body switches to);
enum {
    ANIM_STILL, ANIM_FLOAT, ANIM_WARP_OUT, ANIM_WARP_IN, ANIM_4, ANIM_5, ANIM_6, ANIM_7, ANIM_BOMB_WIND, ANIM_BOMB_THROW, ANIM_BOMB,
    ANIM_DARK_BOMB, ANIM_FUSE, ANIM_13, ANIM_14, ANIM_15, ANIM_16, ANIM_17, ANIM_18, ANIM_19, ANIM_20, ANIM_BLAST, ANIM_STAR,
};
#define WAND 30
enum { WAND_RAISE = WAND + 1, WAND_AIM = WAND + 3, WAND_DOWN = WAND + 16 };
// Composites (dancho00.mca's, then dancho01.mca's from WAND_COMPOSITES);
enum { COMPOSITE_0, COMPOSITE_STARS_WIND, COMPOSITE_STARS_THROW, COMPOSITE_FIRE, COMPOSITE_FIRE_OUT, COMPOSITE_5, COMPOSITE_6, COMPOSITE_STAR };
#define WAND_COMPOSITES 8
enum { COMPOSITE_CHARGE = WAND_COMPOSITES, COMPOSITE_DARK_CHARGE, COMPOSITE_BEAM, COMPOSITE_DARK_BEAM };

// prmBB07Ml0.kgb;
#define IDLE_SHORT 10           // +0x00, +0x04, +0x08;
#define IDLE_LONG 40
#define IDLE_MORE 60
#define HOP_NEAR 40             // +0x0C, +0x10, +0x14: his hops' frames (and his beam's move);
#define HOP_MID 30
#define HOP_FAR 50
#define GRAVITY 0x19A           // +0x18: 0.1;
#define WARP_HIDDEN 10          // +0x1C;
#define SPAWN_AHEAD 16          // +0x20, +0x24: his stars and bombs, ahead and up;
#define SPAWN_UP 16
#define STAR_SPEED 0x4000       // +0x28;
#define STAR_ANGLE 45           // (0x2D000) and +0x2C's 30 either side;
#define STAR_SPREAD 30
#define STAR_TIME 100           // +0x30;
#define BOMB_TIME 45            // +0x38;
#define BOMB_GRAVITY 0xCD       // +0x3C: 0.05;
#define BOMB_SLIDE 0x800        // +0x40, +0x44: Dark Daroach's bomb landed, at 0.5 across at most, none up;
#define FUSE_WAIT 180           // +0x48, +0x4C;
#define FIRE_TIME 120
#define CHARGE_TIME 60          // +0x58, +0x5C, +0x60;
#define CHARGE_CAP 0x2800
#define CHARGE_FOLLOW 0xCD
#define AIM_PAUSE 1             // +0x64, +0x68, +0x6C;
#define BEAM_BACK 0x4CD
#define BEAM_TIME 120
#define SAFE_TIME 10            // 0x02126B8C's +0xEC;
#define DEFEAT_BURST 120        // +0x7C: Dark Daroach's defeat, his wand thrown off;
#define DEFEAT_END 600          // +0x90: and the stage moving on;
// (0x02124B9C, 0x02124A7C: his wand's throw, 1.0 ahead and 4.0 up, its gravity (0x021833A0's -0.1, as a float times 4096), its bounce);
#define WAND_AHEAD 0x1000
#define WAND_UP 0x4000
#define WAND_GRAVITY 0x199
#define WAND_BOUNCE 0x4000
/* (KatAM's screen shakes, RequestScreenShake: 1 its shortest, 14 frames; 3 its longest timed one, 24: the DS's (3, 5) at his defeat's
 * start, (4, 15) at the burst); */
#define SHAKE_DEFEAT 1
#define SHAKE_BURST 3

// a6s7s2 (from the camera view's middle across and the floor, pixels): his points and zones;
#define P_OUT 80
#define P_TOP 92
#define P_LOW 20
#define P4_X 8
#define P4_UP 64
#define START_X 4
#define ZONE_HALF 16
#define BEHIND 16               // 0x02126A94: Kirby under 16 pixels ahead at point 4, he turns round;

enum {
    DR_START, DR_INTRO, DR_IDLE, DR_DECIDE, DR_WARP, DR_WARP_OUT, DR_WARP_HIDDEN, DR_WARP_IN, DR_HOP, DR_HOP_GO, DR_BEAM, DR_BEAM_GO,
    DR_BEAM_RAISE, DR_BEAM_CHARGE, DR_BEAM_PAUSE, DR_BEAM_AIM, DR_BEAM_FIRE, DR_BEAM_END, DR_BOMB, DR_BOMB_WIND, DR_BOMB_THROWN,
    DR_STARS, DR_STARS_WIND, DR_STARS_THROWN, DR_DEFEAT,
};
enum { THING_STAR, THING_STAR_2, THING_STAR_3, THING_BOMB, THING_BEAM_NEAR, THING_BEAM_FAR, THING_WAND };
// (dancho00's: his beaten pose, his wand spinning);
#define ANIM_DEFEAT 27
#define ANIM_WAND 28

#define TIMER(s) ((s)->vars[0])         // +0xB4;
#define WARPS(s) ((s)->vars[1])         // +0xB8;
#define POINT(s) ((s)->vars[2])         // +0x200;
#define SAFE(s) ((s)->vars[3])          // +0xEC;
#define LAST_HP(s) ((s)->vars[4])
#define HARM(s) ((s)->vars[5])
#define FLOOR(s) ((s)->vars[6])         // The floor where he was placed (a6s7s2's Kirby's);
#define BEAM(s, i) ((s)->vars[8 + (i)]) // His beam's reach, its two halves;
#define DEFEAT(s) ((s)->vars[10])       // His defeat under way (Dark Daroach's: DaroachWatch);
#define KEPT_FLAGS(s) ((s)->vars[11])   // His flags as his last frame left them (KatAM's mini-boss defeat changes them: DaroachWatch);
#define KEPT_C(s) ((s)->vars[12])
#define KEPT_5C(s) ((s)->vars[13])

/* dancho00.rca / dancho01.rca (KatAM's: x ahead, y down, pixels from his spot): where he's hit (his first box, animation 0's) and his
 * touch's box (kind 0) in one; */
static const s8 sBody[4] = { -16, -40, 12, -1 };

static u32 Random(u32 n) {
    return Rand16() * n >> 16;
}

static s32 T(struct SqueakObject *s) {
    return (s16)s->stateTimer;
}

static bool32 Dark(struct SqueakObject *s) {
    return s->enemy == SQUEAK_DARK_DAROACH;
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

// Quot(); Squeak Squad's 0x0200C80C (a whole quotient, rounded toward 0);
static s32 Quot(s32 a, s32 b) {
    return b == 0 ? 0 : a / b;
}

static struct LevelInfo *Level(struct SqueakObject *s) {
    return &gCurLevelInfo[s->obj.base.unk56];
}

// Middle(); The camera view's middle across (a6s7s2's 144; here KatAM's view, 240 across), as Bohboh's;
static s32 Middle(struct SqueakObject *s) {
    return Level(s)->viewportPosition.x + (DISPLAY_WIDTH / 2 << 8);
}

static bool32 FacesLeft(struct SqueakObject *s) {
    return s->obj.base.flags & 1;
}

static void Face(struct SqueakObject *s, bool32 left) {
    s->obj.base.flags = (s->obj.base.flags & ~1) | (left ? 1 : 0);
}

// Ahead(), Up(); 0x0201AE58: how far a spot is ahead of him (behind: under 0) and over him, Squeak Squad's units;
static s32 Ahead(struct SqueakObject *s, s32 x) {
    s32 d = (x - s->obj.base.x) << 4;

    return FacesLeft(s) ? -d : d;
}

static s32 Up(struct SqueakObject *s, s32 y) {
    return (s->obj.base.y - y) << 4;
}

// Point(); 0x02209CC0: a point of a6s7s2's (KatAM's units);
static void Point(struct SqueakObject *s, s32 n, s32 *x, s32 *y) {
    switch (n) {
    case 0: *x = Middle(s) - (P_OUT << 8); *y = FLOOR(s) - (P_TOP << 8); break;
    case 1: *x = Middle(s) + (P_OUT << 8); *y = FLOOR(s) - (P_TOP << 8); break;
    case 2: *x = Middle(s) - (P_OUT << 8); *y = FLOOR(s) - (P_LOW << 8); break;
    case 3: *x = Middle(s) + (P_OUT << 8); *y = FLOOR(s) - (P_LOW << 8); break;
    default: *x = Middle(s) + (P4_X << 8); *y = FLOOR(s) - (P4_UP << 8); break;
    }
}

/* Zones(); 0x0212688C: Kirby's zone in the stage's grid, and from it whether his point is on Kirby's side (out[0]: his zone's or the
 * one over or under it), Kirby's side's low point (out[1]; from the middle, 4), the low point across (out[2]; from the middle the one
 * across from Kirby's half), and point 2 from the top points, else 1 (out[3]); */
static void Zones(struct SqueakObject *s, s32 *out) {
    struct Kirby *kirby = s->obj.kirby3;
    s32 dx = kirby->base.x - Middle(s), pt = POINT(s);

    out[3] = pt < 2 ? 2 : 1;
    if (dx < -(ZONE_HALF << 8)) {               // (zones 0 and 2: up top or down low alike);
        out[0] = pt == 0 || pt == 2;
        out[1] = 2;
        out[2] = 3;
    } else if (dx >= ZONE_HALF << 8) {          // (1 and 3);
        out[0] = pt == 1 || pt == 3;
        out[1] = 3;
        out[2] = 2;
    } else {                                    // (4);
        out[0] = pt == 4;
        out[1] = 4;
        out[2] = kirby->base.x < Middle(s) - (ZONE_HALF << 8) ? 3 : 2;
    }
}

// FaceIn(); 0x02126A94: facing in from his point (at point 4, round when Kirby is under BEHIND pixels ahead);
static void FaceIn(struct SqueakObject *s) {
    switch (POINT(s)) {
    case 0: case 2: Face(s, FALSE); break;
    case 1: case 3: Face(s, TRUE); break;
    default:
        if (Ahead(s, s->obj.kirby3->base.x) < BEHIND << 12) SqueakTurnAround(s);
        break;
    }
}

// Warp(); To point n, `warps` times (0x02126140);
static bool32 Warp(struct SqueakObject *s, s32 n, s32 warps) {
    POINT(s) = n;
    WARPS(s) = warps;
    SqueakSetState(s, DR_WARP);
    return TRUE;
}

static bool32 Hop(struct SqueakObject *s, s32 n) {
    POINT(s) = n;
    SqueakSetState(s, DR_HOP);
    return TRUE;
}

/* Lob(); A move to a spot in `t` frames under `gravity`, as his hops (0x02126294) and his bomb's (0x0212404C): t's speed across,
 * up the distance's over t, plus the fall's over half of it; */
static void Lob(struct SqueakObject *s, s32 x, s32 y, s32 t, s32 gravity) {
    s->ay = -gravity;
    s->vx = Quot(Ahead(s, x), t);
    s->vy = Quot(Up(s, y), t) - s->ay * Quot(t, 2);
}

// Thing(); What he brings in (SQUEAK_DAROACH_BOSS_THING / SQUEAK_DARK_DAROACH_THING), at a spot, facing as he does;
static struct SqueakObject *Thing(struct SqueakObject *s, u8 kind, s32 x, s32 y) {
    struct SqueakObject *t = SqueakSpawn(s, Dark(s) ? SQUEAK_DARK_DAROACH_THING : SQUEAK_DAROACH_BOSS_THING, kind, 0, 0);

    if (t == NULL) return NULL;
    t->obj.base.x = x;
    t->obj.base.y = y;
    t->obj.base.parent = &s->obj;
    Face(t, FacesLeft(s));
    return t;
}

// Throw(); 0x02124334, 0x02124920: a star or his bomb, SPAWN_AHEAD ahead of him and SPAWN_UP up;
static void Throw(struct SqueakObject *s, u8 kind) {
    s32 x = s->obj.base.x + (FacesLeft(s) ? -(SPAWN_AHEAD << 8) : SPAWN_AHEAD << 8);

    Thing(s, kind, x, s->obj.base.y - (SPAWN_UP << 8));
    SqueakSfx(s, 0x7D);
}

// Beam(); His beam's reach while it shows: its two halves made once, again if lost (they keep to him themselves);
static void Beam(struct SqueakObject *s) {
    u8 i;

    for (i = 0; i < 2; i++) {
        struct SqueakObject *b = (struct SqueakObject *)BEAM(s, i);

        if (b && b->obj.base.header.kind == 1 && !(b->obj.base.flags & 0x1000) && b->obj.base.parent == &s->obj && SqueakIsSqueak(&b->obj)
            && b->variant == THING_BEAM_NEAR + i)
            continue;
        BEAM(s, i) = (s32)Thing(s, THING_BEAM_NEAR + i, s->obj.base.x, s->obj.base.y);
        /* (drawn by nothing: none of KatAM's 512 VRAM tiles, his own frame's size each, taken from his beam's parts: they'd show
         * the tiles before theirs, or none); */
        if (BEAM(s, i)) SqueakNoSprite((struct SqueakObject *)BEAM(s, i));
    }
}

// Stop(); Squeak Squad's speeds, accelerations, caps and friction to 0;
static void Stop(struct SqueakObject *s) {
    SqueakStop(s);
}

// Decide(); 0x02126414's pick (two rand(9) draws), from Kirby's zone;
static bool32 Decide(struct SqueakObject *s) {
    s32 out[4];
    u32 r1, r2;

    Zones(s, out);
    r1 = Random(9);
    r2 = Random(9);
    if (!out[0]) {
        if (r1 < 1) {
            TIMER(s) += IDLE_MORE;
            return FALSE;
        }
        if (r1 < 4) return r2 < 3 ? Warp(s, out[1], 1) : Warp(s, out[3], 3);
        if (r2 < 3) {
            WARPS(s) = 1;
            SqueakSetState(s, DR_STARS);
            return TRUE;
        }
        SqueakSetState(s, DR_BOMB);
        return TRUE;
    }
    if (r1 < 3) return r2 < 3 ? Warp(s, out[2], 1) : Hop(s, out[3]);
    if (r2 < 3) {
        WARPS(s) = 0;
        SqueakSetState(s, DR_STARS);
        return TRUE;
    }
    POINT(s) = out[2];
    SqueakSetState(s, DR_BEAM);
    return TRUE;
}

// After(); After his bomb (0x02125950) or stars picked off Kirby's side (0x02125C08, WARPS 1): idle 3 in 9, else a warp or three;
static bool32 After(struct SqueakObject *s) {
    s32 out[4];

    if (Random(9) < 3) {
        SqueakSetState(s, DR_IDLE);
        return TRUE;
    }
    Zones(s, out);
    return Random(9) < 3 ? Warp(s, out[1], 1) : Warp(s, out[3], 3);
}

/* NormalColors(); Daroach's own colors (0x02075FF4's "enemy/dancho00": the dark one gone at his defeat's burst): Daroach's (SQUEAK_DAROACH_BOSS's)
 * palette, his cells' and Dark Daroach's colors at the same indexes (KatAMExpansionSqueak.cs's PaletteFile); */
static void NormalColors(struct SqueakObject *x) {
    u16 key = gSqueakData.enemies[SQUEAK_DAROACH_BOSS].animId;

    if (key != 0) x->obj.base.sprite.palId = SqueakLoadPalette(key, SQUEAK_HURT_VARIANTS, x->obj.base.roomId);
}

/* DefeatBurst(); 0x02124DCC at DEFEAT_BURST: efc_cmn0's 10 at him (0x02039B28), his wand thrown off (0x02124D18: SPAWN_AHEAD ahead and
 * SPAWN_UP up), the bigger shake and his own colors back; */
static void DefeatBurst(struct SqueakObject *s) {
    struct SqueakObject *wand;
    s32 x = s->obj.base.x + (FacesLeft(s) ? -(SPAWN_AHEAD << 8) : SPAWN_AHEAD << 8);

    SqueakSpawnEffect(s, SQUEAK_FX_DAROACH_BURST, 0, 0, FALSE);
    wand = Thing(s, THING_WAND, x, s->obj.base.y - (SPAWN_UP << 8));
    if (wand) NormalColors(wand);
    RequestScreenShake(SHAKE_BURST, &s->obj.base);
    NormalColors(s);
}

static bool32 DaroachBoss(struct SqueakObject *s) {
    struct Kirby *kirby = s->obj.kirby3;
    s32 x, y, t, out[4];

    switch (s->state) {
    case DR_START:          // His spawn (a6s7s2's 148, 68), at point 4;
        s->obj.base.x = Middle(s) + (START_X << 8);
        s->obj.base.y = FLOOR(s) - (P_TOP << 8);
        s->obj.base.flags &= ~0x400;
        POINT(s) = 4;
        SqueakSetState(s, DR_INTRO);
        return TRUE;
    case DR_INTRO:          // 0x021267EC, 0x02126790: his float once, then his HP bar (the framework's);
        if (s->stateTimer == 0) {
            SqueakPlay(s, ANIM_FLOAT);
            SqueakSfx(s, 0x1AB);
            break;
        }
        if (s->animEnded) {
            SqueakSetState(s, DR_IDLE);
            return TRUE;
        }
        break;
    case DR_IDLE:           // 0x02126658;
        FaceIn(s);
        Stop(s);
        SqueakPlay(s, ANIM_FLOAT);
        TIMER(s) = Random(9) < 3 ? IDLE_SHORT : IDLE_LONG;
        SqueakSetState(s, DR_DECIDE);
        return TRUE;
    case DR_DECIDE:         // 0x02126414;
        if (T(s) < TIMER(s)) {
            SqueakMove(s);
            break;
        }
        return Decide(s);
    case DR_WARP:           // 0x02126140;
        SqueakPlay(s, ANIM_WARP_OUT);
        Stop(s);
        SqueakSfx(s, 0x1AA);
        SqueakSetState(s, DR_WARP_OUT);
        return TRUE;
    case DR_WARP_OUT:       // 0x021260D0: hidden (his flag 1), at the point;
        if (s->animEnded) {
            s->obj.base.flags |= 0x400;
            Point(s, POINT(s), &s->obj.base.x, &s->obj.base.y);
            SqueakSetState(s, DR_WARP_HIDDEN);
            return TRUE;
        }
        SqueakMove(s);
        break;
    case DR_WARP_HIDDEN:    // 0x02126008;
        if (T(s) >= WARP_HIDDEN) {
            s->obj.base.flags &= ~0x400;
            FaceIn(s);
            SqueakPlay(s, ANIM_WARP_IN);
            SqueakSfx(s, 0x1AB);
            SqueakSetState(s, DR_WARP_IN);
            return TRUE;
        }
        SqueakMove(s);
        break;
    case DR_WARP_IN:        // 0x02125F4C: more warps between the low points and the top ones;
        if (s->animEnded) {
            if (--WARPS(s) < 1) {
                SqueakSetState(s, DR_IDLE);
                return TRUE;
            }
            POINT(s) = POINT(s) < 2 ? 2 : 1;
            SqueakSetState(s, DR_WARP);
            return TRUE;
        }
        SqueakMove(s);
        break;
    case DR_HOP:            // 0x02126294;
        SqueakPlay(s, ANIM_FLOAT);
        Stop(s);
        Point(s, POINT(s), &x, &y);
        t = Abs(Ahead(s, x));
        t = t < 32 << 12 ? HOP_NEAR : t < 128 << 12 ? HOP_MID : HOP_FAR;
        Lob(s, x, y, t, GRAVITY);
        TIMER(s) = t;
        SqueakSetState(s, DR_HOP_GO);
        return TRUE;
    case DR_HOP_GO:         // 0x02126228;
        if (T(s) >= TIMER(s)) {
            Point(s, POINT(s), &s->obj.base.x, &s->obj.base.y);
            SqueakSetState(s, DR_IDLE);
            return TRUE;
        }
        SqueakMove(s);
        break;
    case DR_BEAM:           // 0x021257F8: to his point's x at Kirby's height;
        FaceIn(s);
        SqueakPlay(s, ANIM_FLOAT);
        Stop(s);
        Point(s, POINT(s), &x, &y);
        Lob(s, x, kirby->base.y, HOP_NEAR, GRAVITY);
        TIMER(s) = HOP_NEAR;
        SqueakSetState(s, DR_BEAM_GO);
        return TRUE;
    case DR_BEAM_GO:        // 0x021256F8;
        if (T(s) >= TIMER(s)) {
            Stop(s);
            SqueakPlay(s, WAND_RAISE);
            SqueakSfx(s, 0x1B1);
            SqueakSetState(s, DR_BEAM_RAISE);
            return TRUE;
        }
        SqueakMove(s);
        break;
    case DR_BEAM_RAISE:     // 0x021255CC;
        if (s->animEnded) {
            SqueakPlayComposite(s, Dark(s) ? COMPOSITE_DARK_CHARGE : COMPOSITE_CHARGE);
            SqueakLoopSfx(s, 0x1B6);
            s->maxVy = CHARGE_CAP;
            SqueakSetState(s, DR_BEAM_CHARGE);
            return TRUE;
        }
        SqueakMove(s);
        break;
    case DR_BEAM_CHARGE:    // 0x021254F4: following Kirby's height;
        if (T(s) > CHARGE_TIME) {
            Stop(s);
            SqueakSetState(s, DR_BEAM_PAUSE);
            return TRUE;
        }
        s->vy = FxMul(Up(s, kirby->base.y), CHARGE_FOLLOW);
        SqueakMove(s);
        break;
    case DR_BEAM_PAUSE:     // 0x0212543C;
        if (T(s) > AIM_PAUSE) {
            SqueakPlay(s, WAND_AIM);
            SqueakSetState(s, DR_BEAM_AIM);
            return TRUE;
        }
        SqueakMove(s);
        break;
    case DR_BEAM_AIM:       // 0x02125300;
        if (s->animEnded) {
            SqueakPlayComposite(s, Dark(s) ? COMPOSITE_DARK_BEAM : COMPOSITE_BEAM);
            SqueakStopSfx(s, 0x1B6);
            SqueakSfx(s, 0x1AF);
            if (Dark(s)) s->vx = -BEAM_BACK;
            SqueakSetState(s, DR_BEAM_FIRE);
            return TRUE;
        }
        SqueakMove(s);
        break;
    case DR_BEAM_FIRE:      // 0x0212523C;
        if (T(s) > BEAM_TIME) {
            s->vx = s->vy = 0;
            SqueakPlay(s, WAND_DOWN);
            SqueakSetState(s, DR_BEAM_END);
            return TRUE;
        }
        Beam(s);
        SqueakMove(s);
        break;
    case DR_BEAM_END:       // 0x0212512C;
        if (s->animEnded) {
            if (Random(9) < 3) {
                SqueakSetState(s, DR_HOP);
                return TRUE;
            }
            Zones(s, out);
            return Random(9) < 3 ? Warp(s, out[2], 1) : Hop(s, out[3]);
        }
        SqueakMove(s);
        break;
    case DR_BOMB:           // 0x02125B28;
        FaceIn(s);
        SqueakPlay(s, ANIM_BOMB_WIND);
        Stop(s);
        SqueakSetState(s, DR_BOMB_WIND);
        return TRUE;
    case DR_BOMB_WIND:      // 0x02125A68;
        if (s->animEnded) {
            SqueakPlay(s, ANIM_BOMB_THROW);
            Throw(s, THING_BOMB);
            SqueakSetState(s, DR_BOMB_THROWN);
            return TRUE;
        }
        SqueakMove(s);
        break;
    case DR_BOMB_THROWN:    // 0x02125950;
        if (s->animEnded) return After(s);
        SqueakMove(s);
        break;
    case DR_STARS:          // 0x02125E6C;
        FaceIn(s);
        SqueakPlayComposite(s, COMPOSITE_STARS_WIND);
        Stop(s);
        SqueakSetState(s, DR_STARS_WIND);
        return TRUE;
    case DR_STARS_WIND:     // 0x02125DAC;
        if (s->animEnded) {
            SqueakPlayComposite(s, COMPOSITE_STARS_THROW);
            Throw(s, THING_STAR);
            Throw(s, THING_STAR_2);
            Throw(s, THING_STAR_3);
            SqueakSetState(s, DR_STARS_THROWN);
            return TRUE;
        }
        SqueakMove(s);
        break;
    case DR_DEFEAT:         // 0x02124DCC: his beaten pose (DefeatStart); the burst; then gone, the room's way on taking the Kirbys on;
        if (s->stateTimer == DEFEAT_BURST) DefeatBurst(s);
        if (s->stateTimer >= DEFEAT_END) s->obj.base.flags |= 0x1000;
        break;
    case DR_STARS_THROWN:   // 0x02125C08;
        if (!s->animEnded) {
            SqueakMove(s);
            break;
        }
        if (WARPS(s) != 0) return After(s);
        if (Random(9) < 3) {
            SqueakSetState(s, DR_IDLE);
            return TRUE;
        }
        Zones(s, out);
        return Random(9) < 3 ? Warp(s, out[2], 1) : Hop(s, out[3]);
    }
    return FALSE;
}

/* Untouchable(); 0x0201322C: in his warp (dancho00.rca's boxes flagged 0x8000 in its animations 2 and 3), hidden, or just hit
 * (+0xEC); */
static bool32 Untouchable(struct SqueakObject *s) {
    return (s->composite == SQUEAK_NO_COMPOSITE && (s->anim == ANIM_WARP_OUT || s->anim == ANIM_WARP_IN))
           || (s->obj.base.flags & 0x400) || SAFE(s) > 0;
}

/* Hit(); 0x02126B8C (KatAM's mini-bosses take hits without a hurt state of their own: a fall of his HP is the hit): SAFE_TIME frames
 * safe after one (+0xEC), a hit where he can't be hit undone; his hurt flash as the DS's (0x020144F4 counts +0xEC down, then his
 * vtable +0x20, 0x02126B30, while it's over 0: dan_dmg's colors when it's 3 to 5 or 9 to 11, (it / 3) odd, else his own); */
static void Hit(struct SqueakObject *s) {
    struct Object *obj = &s->obj;

    if (SAFE(s) > 0) SAFE(s)--;
    SqueakDamageColors(s, !DEFEAT(s) && SAFE(s) > 0 && (SAFE(s) / 3) % 2 != 0);
    if (obj->unk80 < LAST_HP(s) && obj->unk80 > 0 && !(obj->base.flags & 0x1000)) {
        if (Untouchable(s) || s->level) obj->unk80 = LAST_HP(s);
        else SAFE(s) = SAFE_TIME;
    }
    LAST_HP(s) = obj->unk80;
}

/* Harms(); His touch's boxes (dancho00.rca's / dancho01.rca's kind 0): none in his warp nor dancho00's animation 0; his charge's
 * 1 frame in 8 (dancho01's 2, the composite's body: its box on its first frame), his beam's 4 in 6 (dancho01's 4); */
static bool32 Harms(struct SqueakObject *s) {
    switch (s->composite) {
    case COMPOSITE_CHARGE: case COMPOSITE_DARK_CHARGE: return s->compositeTime % 8 == 0;
    case COMPOSITE_BEAM: case COMPOSITE_DARK_BEAM: return s->compositeTime % 6 < 4;
    case SQUEAK_NO_COMPOSITE: return s->anim != ANIM_STILL && s->anim != ANIM_WARP_OUT && s->anim != ANIM_WARP_IN;
    default: return TRUE;
    }
}

// Boxes(); His box (where he's hit and his touch's), hit only where he can be (KatAM's level 3; else 6, as Gordo's), none hidden;
static void Boxes(struct SqueakObject *s) {
    struct ObjectBase *base = &s->obj.base;

    if (base->flags & 0x400) {
        base->flags |= 0x200;
        base->unk68 = 0;
        return;
    }
    base->flags &= ~0x200;
    sub_0803E2B0(base, sBody[0], sBody[1], sBody[2], sBody[3]);
    base->unk68 = Harms(s) ? HARM(s) : 0;
    SqueakUntouchable(s, Untouchable(s));
}

/* DefeatStart(); 0x02124EFC, Dark Daroach's HP out: still where he is (his speeds none), his composite and wand's sprite gone (+0xE4),
 * the shake, dancho00's 27; then his defeat (0x02124DCC); */
static void DefeatStart(struct SqueakObject *s) {
    DEFEAT(s) = TRUE;
    SqueakStop(s);
    s->ay = 0;
    if (s->loopSound) SqueakStopSfx(s, s->loopSound);
    SqueakDamageColors(s, FALSE);
    SqueakPlay(s, ANIM_DEFEAT);
    RequestScreenShake(SHAKE_DEFEAT, &s->obj.base);
    SqueakSetState(s, DR_DEFEAT);
}

/* DefeatFrame(); His frame in his defeat: his states', drawn, no box (none hit, no harm), no HP bar taken back (it shows none left); */
static void DefeatFrame(struct SqueakObject *s) {
    struct ObjectBase *base = &s->obj.base;

    SqueakRun(s, DaroachBoss);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
    base->flags |= 0x200;
    base->unk68 = 0;
}

/* DaroachWatch(); KatAM's object's own before its Main (unk7C, every frame, after its hits), Dark Daroach's: his HP out, KatAM's
 * mini-boss defeat (sub_0809AF38's sub_0809D060: unk78 its falling body's, his flags, unkC and unk5C changed, his HP 0) put back as
 * his last frame left them (KEPT_*), his HP kept at 0 (KatAM's HP bar empty), and his own defeat instead (DefeatStart), as Dark
 * Nebula's (squeak/dark_nebula.c's DarkNebulaWatch); */
static void DaroachBossMain(struct Object *obj);
static void DaroachWatch(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    if (DEFEAT(s) || obj->unk80 > 0 || (obj->base.flags & 0x1000)) return;
    obj->unk78 = DaroachBossMain;
    obj->base.flags = KEPT_FLAGS(s);
    obj->base.unkC = KEPT_C(s);
    obj->base.unk5C = KEPT_5C(s);
    obj->unk80 = 0;
    // (in a hop 0x02124EFC waits, his flag 0x40: his defeat once it's over, on its point; the owner: "When beaten, Dark Daroach
    // just floats in mid air", beaten in a hop and stopped there);
    if (s->state == DR_HOP_GO) return;
    obj->base.xspeed = obj->base.yspeed = 0;
    DefeatStart(s);
}

static void DaroachBossMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    obj->kirby3 = FindTargetKirby(&obj->base);
    if (DEFEAT(s)) {
        DefeatFrame(s);
        return;
    }
    SqueakMidbossUpdate(s);
    Hit(s);
    SqueakRun(s, DaroachBoss);
    if (obj->unk80 <= 0) {
        // (Dark Daroach beaten in a hop: it goes on to its end, DaroachWatch's defeat then);
        if (Dark(s) && s->state == DR_HOP_GO) {
            SqueakFinishFrame(s);
            SqueakAnimate(s);
        }
        return;
    }
    // (his HP bar Squeak Squad's boss's, which his things' hits don't take: KatAM's goes to whatever is hit, taken back);
    sub_08034E14(obj);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
    Boxes(s);
    KEPT_FLAGS(s) = obj->base.flags;
    KEPT_C(s) = obj->base.unkC;
    KEPT_5C(s) = obj->base.unk5C;
}

/* DaroachBossStart(); His fight's start (and after a hit: it goes on, as KatAM's mini-bosses do), on the floor's terrain (Squeak
 * Squad's flags without 0x200); Dark Daroach's defeat his own (DaroachWatch); */
void DaroachBossStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, -1, DaroachBossMain);
    if (Dark(s)) obj->unk7C = DaroachWatch;
    SqueakThroughTerrain(s, FALSE);
    if (s->started) return;
    LAST_HP(s) = obj->unk80;
    SAFE(s) = 0;
    BEAM(s, 0) = BEAM(s, 1) = 0;
    DEFEAT(s) = FALSE;
    KEPT_FLAGS(s) = obj->base.flags;
    KEPT_C(s) = obj->base.unkC;
    KEPT_5C(s) = obj->base.unk5C;
    SqueakSetState(s, DR_START);
}

// DaroachBossInit(); His box, his bounds (KatAM's), the floor where he's placed (a6s7s2's Kirby's); hidden, still, till his fight;
void DaroachBossInit(struct SqueakObject *s) {
    struct Object *obj = &s->obj;

    sub_0803E2B0(&obj->base, sBody[0], sBody[1], sBody[2], sBody[3]);
    ObjectSetBounds(&obj->base, -16, -40, 12, 0);
    HARM(s) = obj->base.unk68;
    SqueakStandOnFloor(obj);
    FLOOR(s) = SqueakBossFloor(s);
    obj->base.flags |= 0x400;
    obj->base.unk62 |= 4;
    SqueakPlay(s, ANIM_FLOAT);
}

/* What he brings in (the variant): his stars (0x02124894: three at their angles), his bomb (0x02124290: Dark Daroach's his phase's
 * own) and his beam's reach (its two halves); all gone when he's beaten (a star burst); */
#define T_HARM(t) ((t)->vars[0])
#define T_TIMER(t) ((t)->vars[1])
#define T_TOUCHED(t) ((t)->vars[2])     // (a star touched a Kirby: it bursts the frame after, KatAM's hit on him first);
#define T_LOOK(t) ((t)->vars[3])        // (its last own frame, shown while KatAM holds it: DaroachThingLook());

static bool32 Beaten(struct SqueakObject *t) {
    struct SqueakObject *b = (struct SqueakObject *)t->obj.base.parent;

    return b == NULL || b->obj.base.header.kind != 1 || (b->obj.base.flags & 0x1000) || b->obj.unk80 <= 0 || !SqueakIsSqueak(&b->obj)
           || (b->enemy != SQUEAK_DAROACH_BOSS && b->enemy != SQUEAK_DARK_DAROACH);
}

static bool32 DarkThing(struct SqueakObject *t) {
    return t->enemy == SQUEAK_DARK_DAROACH_THING;
}

static void Burst(struct SqueakObject *t) {
    SqueakSpawnEffect(t, SQUEAK_FX_STARS, 0, 0, FALSE);
    t->obj.base.flags |= 0x1000;
}

/* StarAngle(); A star's speed at one of its angles (45°, and 30° either side: rand(3)'s 1 and 2), the DS's sine and cosine table
 * (0x0215CE20's pairs) times STAR_SPEED: across its sine, its cosine down (`down`) or up; */
static void StarAngle(struct SqueakObject *t, u8 kind, bool32 down) {
    // (sin, cos) of 45°, 75° and 15°, 20.12;
    static const s16 sSinCos[3][2] = { { 0xB50, 0xB50 }, { 0xF74, 0x424 }, { 0x424, 0xF74 } };

    t->vx = FxMul(sSinCos[kind][0], STAR_SPEED);
    t->vy = FxMul(sSinCos[kind][1], STAR_SPEED);
    if (down) t->vy = -t->vy;
}

static bool32 DaroachThing(struct SqueakObject *t) {
    struct SqueakObject *b = (struct SqueakObject *)t->obj.base.parent;
    struct Kirby *kirby;
    s32 x;

    switch (t->variant) {
    case THING_STAR: case THING_STAR_2: case THING_STAR_3:
        if (t->state == 0) {            // 0x021246F4;
            StarAngle(t, t->variant - THING_STAR, TRUE);
            SqueakPlayComposite(t, COMPOSITE_STAR);
            SqueakSfx(t, 0x1AC);
            SqueakSetState(t, 1);
        }
        // 0x021243D8: off the floor up, off a ceiling down, off a wall turned round (its way up or down kept);
        if (SqueakOnGround(t)) {
            StarAngle(t, Random(3), FALSE);
            SqueakSfx(t, 0x1B4);
        }
        if (SqueakCeiling(t)) {
            StarAngle(t, Random(3), TRUE);
            SqueakSfx(t, 0x1B4);
        }
        if (SqueakWallAhead(t)) {
            bool32 down = t->vy < 1;

            SqueakSfx(t, 0x1B4);
            SqueakTurnAround(t);
            StarAngle(t, Random(3), down);
        }
        SqueakMove(t);
        /* (its touch on a Kirby, 0x02124844's +0x12C, bursts it too: the DS's touch comes with his hit, KatAM's collision after
         * the objects' frames, so here the frame after, the star still there for KatAM to hurt him with; the owner: "Dark
         * Daroach's stars have no hitbox", bursting on him unhurt); */
        if (T(t) > STAR_TIME || Beaten(t) || t->obj.unk80 <= 0 || T_TOUCHED(t)) Burst(t);
        else if (SqueakTouchingKirby(t)) T_TOUCHED(t) = TRUE;
        break;
    case THING_BOMB:
        switch (t->state) {
        case 0:                         // 0x0212404C: lobbed at Kirby, BOMB_TIME frames;
            kirby = b ? b->obj.kirby3 : NULL;
            SqueakPlay(t, DarkThing(t) ? ANIM_DARK_BOMB : ANIM_BOMB);
            SqueakStop(t);
            if (kirby) Lob(t, kirby->base.x, kirby->base.y, BOMB_TIME, BOMB_GRAVITY);
            else t->ay = -BOMB_GRAVITY;
            SqueakSetState(t, 1);
            return TRUE;
        case 1:                         // 0x02123E80: on the ground, a blast; Dark Daroach's stops;
            SqueakMove(t);
            if (!SqueakOnGround(t)) break;
            if (!DarkThing(t)) {
                SqueakStop(t);
                SqueakSpawnEffect(t, SQUEAK_FX_BURST, 0, 0, FALSE);
                SqueakSfx(t, 0x12D);
                SqueakPlay(t, ANIM_BLAST);
                t->obj.kirbyAbility = 0;
                SqueakSetState(t, 2);
                return TRUE;
            }
            t->vx = t->vx < 1 ? 0 : t->vx < BOMB_SLIDE ? t->vx : BOMB_SLIDE;
            t->vy = 0;
            SqueakSfx(t, 0x135);
            SqueakSetState(t, 3);
            break;
        case 2:                         // 0x02123E3C;
            SqueakMove(t);
            if (t->animEnded) t->obj.base.flags |= 0x1000;
            break;
        case 3:                         // 0x02123D5C: landed again, still;
            SqueakMove(t);
            if (SqueakOnGround(t)) {
                SqueakStop(t);
                SqueakSfx(t, 0x14D);
                SqueakSetState(t, 4);
            }
            break;
        case 4:                         // 0x02123CD8, 0x02123BFC: FUSE_WAIT frames, then lit;
            SqueakMove(t);
            if (T(t) >= FUSE_WAIT) {
                SqueakPlay(t, ANIM_FUSE);
                SqueakSetState(t, 5);
            }
            break;
        case 5:                         // 0x02123B10;
            SqueakMove(t);
            if (t->animEnded) {
                SqueakPlayComposite(t, COMPOSITE_FIRE);
                SqueakSfx(t, 0x1B0);
                t->obj.kirbyAbility = 0;
                SqueakSetState(t, 6);
            }
            break;
        case 6:                         // 0x02123A3C: its fire FIRE_TIME frames, to its animation's end;
            SqueakMove(t);
            if (T(t) >= FIRE_TIME && t->animEnded) {
                SqueakPlayComposite(t, COMPOSITE_FIRE_OUT);
                SqueakSetState(t, 7);
            }
            break;
        case 7:                         // 0x021239FC;
            SqueakMove(t);
            if (t->animEnded) t->obj.base.flags |= 0x1000;
            break;
        }
        if (Beaten(t) && !(t->obj.base.flags & 0x1000)) Burst(t);
        break;
    case THING_WAND:                    // 0x02124B9C, 0x02124A7C: Dark Daroach's wand, thrown off at his defeat, bouncing;
        if (t->state == 0) {
            SqueakPlay(t, ANIM_WAND);
            t->vx = WAND_AHEAD;
            t->vy = WAND_UP;
            t->ay = -WAND_GRAVITY;
            SqueakSetState(t, 1);
        }
        if (SqueakWallAhead(t)) {
            SqueakSfx(t, 0x7D);
            SqueakTurnAround(t);
        }
        if (t->vy < 1 && SqueakOnGround(t)) {
            SqueakSfx(t, 0x7D);
            t->vy = WAND_BOUNCE;
        }
        SqueakMove(t);
        // (gone with him: the room moved on);
        if (b == NULL || b->obj.base.header.kind != 1 || (b->obj.base.flags & 0x1000)) t->obj.base.flags |= 0x1000;
        break;
    default:                            // His beam's reach: on him, its half ahead of him;
        if (Beaten(t) || b->state != DR_BEAM_FIRE) {
            t->obj.base.flags |= 0x1000;
            break;
        }
        Face(t, FacesLeft(b));
        x = t->variant == THING_BEAM_NEAR ? 106 : DarkThing(t) ? 250 : 256;
        t->obj.base.x = b->obj.base.x + (FacesLeft(b) ? -(x << 8) : x << 8);
        t->obj.base.y = b->obj.base.y;
        break;
    }
    return FALSE;
}

/* ThingBoxes(); A thing's box by its frame (dancho00.rca's: a star's 8 either side, 7 under to 5 over; a bomb's 5 either side, 1 to 12
 * up; its blast 23 either side, 18 under to 23 over; Dark Daroach's bomb's fire a column 11 across and its sparks 24 out, up to KatAM's
 * reach of 127 pixels (its top part to 168), then its last flames 103 up),
 * its beam's halves (31 to 181 and 181 to 332 ahead, 4 under to 5 over; Dark Daroach's to 320, 14 under to 15 over); */
static void ThingBoxes(struct SqueakObject *t) {
    struct ObjectBase *base = &t->obj.base;

    switch (t->variant) {
    case THING_BOMB:
        if (t->composite == COMPOSITE_FIRE) sub_0803E2B0(base, -33, -127, 28, 1);
        else if (t->composite == COMPOSITE_FIRE_OUT) sub_0803E2B0(base, -9, -103, 4, 1);
        else if (t->anim == ANIM_BLAST) sub_0803E2B0(base, -23, -23, 22, 18);
        else sub_0803E2B0(base, -5, -12, 5, -1);
        // (its blast's box flagged: untouchable, dancho00.rca's 21);
        SqueakUntouchable(t, t->anim == ANIM_BLAST || t->composite != SQUEAK_NO_COMPOSITE);
        break;
    case THING_BEAM_NEAR:
        if (DarkThing(t)) sub_0803E2B0(base, -75, -15, 75, 14);
        else sub_0803E2B0(base, -75, -5, 75, 4);
        break;
    case THING_BEAM_FAR:
        if (DarkThing(t)) sub_0803E2B0(base, -69, -15, 70, 14);
        else sub_0803E2B0(base, -75, -5, 76, 4);
        break;
    default:
        sub_0803E2B0(base, -8, -5, 8, 7);
        break;
    }
}

static void DaroachThingMain(struct Object *obj) {
    struct SqueakObject *t = (struct SqueakObject *)obj;

    obj->kirby3 = FindTargetKirby(&obj->base);
    SqueakRun(t, DaroachThing);
    SqueakFinishFrame(t);
    if (t->variant < THING_BEAM_NEAR || t->variant == THING_WAND) SqueakAnimate(t);
    ThingBoxes(t);
    if (obj->unk83 >= SQUEAK_HURT_VARIANTS) T_LOOK(t) = obj->unk83;
}

/* DaroachThingLook(); A thing's frame as drawn (squeak.c's SqueakAnimFor()): KatAM's inhale shows a hurt frame (variants 0-2), a
 * thing's his (it's drawn with his graphics: the owner, "Dark Daroach fight, when inhaling its projectiles, they turn into
 * daroach"), so a thing KatAM holds keeps its own last one; */
u8 DaroachThingLook(struct SqueakObject *t) {
    if (t->obj.unk83 >= SQUEAK_HURT_VARIANTS || T_LOOK(t) < SQUEAK_HURT_VARIANTS) return t->obj.unk83;
    return T_LOOK(t);
}

void DaroachBossThingStart(struct Object *obj) {
    struct SqueakObject *t = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, -1, DaroachThingMain);
    // (his bomb a Bomb when inhaled: 0x0203A9C4's 9; his stars none, 0x0203A82C's 0);
    obj->kirbyAbility = t->variant == THING_BOMB ? KIRBY_ABILITY_BOMB : 0;
    if (t->started) return;
    SqueakSetState(t, 0);
}

/* DaroachBossThingInit(); Its box and bounds; flying free of the stage's hold (flag 0x20000), on the terrain; his beam's reach drawn
 * by nothing, through everything, no attack hurting it (KatAM's level 6), not inhaled, his body no harm to it; */
void DaroachBossThingInit(struct SqueakObject *t) {
    struct ObjectBase *base = &t->obj.base;

    T_HARM(t) = base->unk68;
    T_TOUCHED(t) = FALSE;
    T_LOOK(t) = 0;
    base->flags |= 0x1000000 | 0x2000000;
    ThingBoxes(t);
    // (Kirby's body no harm to his stars and bombs: a star's touch bursts it, a bomb's leaves it be, 0x02124844, 0x02124240);
    base->unk5C |= 0x80;
    switch (t->variant) {
    case THING_BEAM_NEAR: case THING_BEAM_FAR:
        ObjectSetBounds(base, -8, -8, 8, 8);
        base->unk5C = (base->unk5C & ~7) | 6 | 0x20 | 0x80;
        base->flags |= 0x100 | 0x400;
        break;
    case THING_BOMB:
        ObjectSetBounds(base, -5, -12, 5, 0);
        break;
    case THING_WAND:
        // (his wand: no harm, nothing hits it, not inhaled; on the floor's terrain, bouncing);
        ObjectSetBounds(base, -8, -8, 8, 8);
        base->unk5C = (base->unk5C & ~7) | 6 | 0x20 | 0x80;
        base->flags |= 0x200;
        base->unk68 = 0;
        T_HARM(t) = 0;
        break;
    default:
        ObjectSetBounds(base, -7, -6, 7, 6);
        break;
    }
}
