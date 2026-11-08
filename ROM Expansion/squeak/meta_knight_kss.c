/* Meta Knight (Squeak Squad's); ported from Kirby: Squeak Squad (USA), boss 0x37 (his body 0x02116C7C, vtable 0x02181E80: +0x3C his
 * idle 0x02116300, +0x34 per frame 0x02116ED4, +0x30 his hit 0x021166BC, +0x2C his touch 0x02116ACC, +0x24 a spat star on him
 * 0x021164F4, +0x18 his flash and intro 0x02113018; his states 0x02113A48-0x02116468, his picks 0x02113190), as
 * SQUEAK_META_KNIGHT, a boss on the mid-bosses' framework (mini-boss type 0x40: the camera held on its area, the HP bar, the music,
 * KatAM's defeat). Values from param/PrmBB01Ml0.kgb (moves) and PrmBB01Sl0.kgb (his HP, 400: here KatAM's bosses' 80, as
 * Daroach's); speeds 20.12 a frame, Y up; drawn with enemy/metaknight_cmn (his body, 0x02022F00's slot 0) and metaknight_ext (slot
 * 1, the sprite his body switches to, its +0xE4: his sword's big moves; its .mca's composites), both in metaknight_cmn's colors.
 * Recorded on the DS in his stage (map/a7s7s2, the Halberd's deck, loaded by repointing the stage table: his intro, walks, runs,
 * jumps, slashes, his drop through the thin floor, his beams, his wave, his tornado catching Kirby, his thrust).
 * • His arena (a7s7s2: 288 by 240, the deck 80 up, its walls at the DS view's sides, 16 and 272: he stops 9 pixels in; here
 *   KatAM's camera view's sides, and the floor where he was placed): its zones (0x02209C7C, the stage's grid of 16-pixel tiles from
 *   its left, the view's middle less 144): from 32 under the floor to 64 over it 0, then to 128 over it 1, to 160 over it 1 but
 *   every other tile 2-8; under it the left 64 pixels 9, the right 80 10, between 0; elsewhere none.
 * • His intro (0x02116468, 0x021163B8): animation 1, 150 frames (the screen rumbling: 17 frames from his 86th, 180 from his 163rd,
 *   0x02113018), then he runs at Kirby.
 * • His fight (0x02115CD4, his mode +0x228): walking at Kirby (1.5 a frame, mode 1), back (1.0, mode 2), running (2.4, mode 3); at
 *   Kirby (under 48 pixels ahead) in modes 1 and 3, and in modes 7-9 at once, he picks (0x02113190): Kirby ahead within 48 pixels
 *   and under 16 over him: rand(100) 20 a follow (mode 4, 180 frames), 20 nothing, 15 his dash, 15 his double slash, 15 his
 *   thrust, 6 his tornado, 6 a beam, else three; 16-80 over him: 20 his up slash, 30 his wave, else a jump; else in his zone: 21 his
 *   tornado, 20 a beam, 10 three, 15 a follow (mode 6, 120 frames), 18 a walk at him, else a run; in another zone 36 a walk, 35 a
 *   run, else Kirby down in zone 0 a drop through the thin floor he stands on (2.0 down), up a jump. Following (modes 4-6), every 30
 *   frames by Kirby's distance ahead: behind turned round, under 48 back, to 96 a walk, further a run. Walking back into a wall,
 *   a walk at him or a jump (rand(100) 50); a wall ahead, his backflip; off a ledge a jump (2.4 a frame or more across) or a fall.
 *   - Idle (0x02116300): rand(100) over 40 a follow (mode 4, 120 frames), else picking (mode 7).
 *   - Jump (0x021155EC-0x02115440): 2.4 across, 5.0 up, 0.2 gravity; stopped at the top, his pose's end, then (Kirby in no zone
 *     or zone 0) rand(100) 40 a hop, 30 his dash, else a fall; else a hop. Hop (0x021150D8-0x02115034): 3.0 up, 0.1 gravity,
 *     stopping across (1.5 a frame), then a fall. Fall (0x02113C78, 0x02113B24): 0.2 gravity, stopping across; every 20 frames a
 *     pick (any slash, thrust, tornado or beam his dash; his up slash; his wave); landed, a walk at Kirby.
 *   - Backflip (0x021153AC-0x02115278): 1.5 back, 5.0 up, 0.2 gravity; a wall behind 0.5 across and a hop; at the top, his pose,
 *     then a hop.
 *   - Dash (0x02114FB8-0x02114E54): drawn (animation 18), his slash (19 on the ground, 20 in the air), facing Kirby (stopping
 *     within 30 units), then a star 16 pixels ahead (0x0212389C's kind 3) and a walk.
 *   - Up slash (0x02114DA8-0x02114C94): ready (21), the slash (22); in the air a fall.
 *   - Double slash (0x02114C10-0x021149B0): 23 then 24, facing Kirby.
 *   - Thrust (0x021148E0-0x02114604): 5.0 up, 0.2 gravity, held 30 frames (ext's 1), up (2, 3), stopped at the top till his pose's
 *     end, down (4: 0.2 gravity); landed, stars 16 pixels either side (kinds 3 and 4), the screen shaking (30 frames), ext's .mca's
 *     0, 60 frames.
 *   - Tornado (0x02113ED0-0x02113D10): a hop back (3.0 back, 1.5 up, 0.1 gravity: ext's 19); landed, ext's .mca's 3, the screen
 *     shaking (54 frames), 24, then his tornado (0x021122F8's kind 2) and 25, 100 frames, then a walk.
 *   - Beams (0x021144FC-0x02114228): a hop back (5.0 back, 1.5 up, 0.2 gravity: ext's 7); landed (8), the screen shaking (62
 *     frames), .mca's 1, then his swing (11) with a beam on its 3rd frame (kind 0); three: the swing again without, then with, twice.
 *   - Wave (0x02114148-0x02113FB0): a hop (0.1 back, 2.0 up, 0.1 gravity: ext's 13); landed, .mca's 2, 16, then the wave (kind 1)
 *     and 17, the screen shaking (21 frames).
 * • Hit (0x021166BC): 20 frames safe after a hit (+0x230, which also counts down each hit while it lasts), 13 after a dodge; a hit
 *   in modes 0-2 and 4-7 is dodged (none taken: 0x02115BD0 on the ground, his dodge, 1.0 back or turned round, 5.0 up 1 in 3 else
 *   0.2, then a backflip; 0x02115A38 in the air, landing idle) rand(100) under 50, or under 25 and taken with a hop away after
 *   (0x021151EC: 1.5 back, 2.0 up, 0.5 gravity, 15 frames) while Kirby has under 17 of his 36 HP; 30 more every third hit, and
 *   when he has under 20 of 400 (16 dodging). Where he's hit: his sprite's first box (metaknight_cmn.rca's animation 0: 9 either
 *   side, 3 to 19 up); his touch hurts (his boxes' kind 0) but in his intro, as his sword's reach (metaknight_*.rca's kind 0
 *   boxes on his slashes' frames: a THING_REACH, SQUEAK_META_KNIGHT_SPARKLE drawing nothing, kept on him).
 * • What he brings in (SQUEAK_META_KNIGHT_THING, _BEAM, _WAVE, _SPARKLE, drawn from metaknight_ext; no attack hurts them, 0x021120B8):
 *   - His beam (0x02111F90, 0x02111F34): ext's 12, 2.5 a frame, gone after 240 frames or at a wall.
 *   - His wave (0x02111E44, 0x02111E04): ext's 18 where he stands (39 behind to 49 ahead, 1 under to 19 over), 60 frames.
 *   - His tornado (0x02111CFC, 0x02111B70): ext's .mca's 4, 1.5 a frame, gone after 400 frames or at a wall; past the arena's
 *     points 9 and 10 (80 pixels from its middle) away from him, at its loop's end, .mca's 5, then gone. Its box (191 up from its
 *     spot, 0x101 kind 6) catches Kirby (0x02111A84, 0x021119B0): held where he's caught, lifted 2 pixels a frame to 140 over the
 *     floor, sparkles (ext's 31) about him every 12 frames, let go hurt after 34 frames (0x0211190C: 10 of his 36 HP).
 *   - His stars (Squeak Squad's 0x0212389C kinds 3 and 4: SQUEAK_STAR's SQUEAK_STAR_SWORD_AHEAD/BEHIND): a Sword when inhaled.
 * Not ported: his sounds, his hit flash (enemy/metaknight_dmg's palette: KatAM's), his flinch when a star Kirby spits hits him
 * (0x021164F4: 10 HP, his pose 28 for 18 frames; KatAM's hits don't say what hit), guarding the attacks Squeak Squad flags 0x10
 * (his box's flag 4; KatAM's have no such flag), his own defeat (0x02113A48-0x02113514 and his things of 0x02112F48: KatAM's
 * mini-boss defeat), the intro's camera (0x02209F84), his boss music. */
#include "../squeak.h"
#include "kirby.h"
#include "constants/kirby.h"

void RequestScreenShake(u16, struct ObjectBase *);
void sub_0804E09C(struct Kirby *);

/* Animations (KatAMExpansionSqueak.cs keeps the ones his code plays, in this order: metaknight_cmn.cea's 1, 2, 4-14, 16, 18-25, 28,
 * then metaknight_ext.cea's 1-11, 13-17, 19-25, the sprite his body switches to); */
enum {
    ANIM_INTRO, ANIM_IDLE, ANIM_WALK, ANIM_BACK, ANIM_RUN, ANIM_JUMP, ANIM_RISE, ANIM_TOP, ANIM_FALL, ANIM_LAND, ANIM_HOP, ANIM_FLIP,
    ANIM_FLIP_RISE, ANIM_DODGE, ANIM_DRAW, ANIM_SLASH, ANIM_AIR_SLASH, ANIM_UP_READY, ANIM_UP_SLASH, ANIM_DOUBLE, ANIM_DOUBLE_2,
    ANIM_HURT, ANIM_COUNTER,
    X_CHARGE, X_CHARGED, X_RISE, X_THRUST, X_LANDED, X_LANDED_FX, X_HOP_BACK, X_READY, X_BEAM_READY, X_BEAM_READY_FX, X_SWING,
    X_WAVE_HOP, X_RAISED, X_RAISED_FX, X_WAVE_UP, X_WAVE_DOWN, X_TORNADO_HOP, X_CALL, X_CALL_FX, X_CALL_FX_2, X_CALL_FX_3,
    X_TORNADO_RAISED, X_TORNADO_POSE,
};
// Composites (metaknight_cmn.mca's 0, 1 and 3, which he doesn't play, then metaknight_ext.mca's 0-3);
enum { C_THRUST_LAND = 3, C_BEAM_READY, C_WAVE_RAISE, C_TORNADO_CALL };
/* What he brings in: SQUEAK_META_KNIGHT_THING his tornado (metaknight_ext.mca's 4 and 5, their parts), SQUEAK_META_KNIGHT_BEAM,
 * _WAVE and _SPARKLE metaknight_ext.cea's 12, 18 and 31, each its animation 0; */
enum { T_TORNADO, T_TORNADO_END };
#define T_ANIM 0

// PrmBB01Ml0.kgb;
#define SAFE_TIME 20            // +0x00;
#define WALK_SPEED 0x1800       // +0x08 (and his slowing across in the air, 0x02113484);
#define BACK_SPEED 0x1000       // +0x0C;
#define RUN_SPEED 0x2667        // +0x10 (and his jump's across);
#define JUMP_OFF 0x199A         // +0x1C: off a ledge this fast or more, a jump;
#define JUMP_VY 0x5000          // +0x20, +0x24;
#define GRAVITY 0x334
#define FLIP_VY 0x5000          // +0x28..+0x30;
#define FLIP_VX 0x1800
#define FLIP_GRAVITY 0x334
#define HOP_VY 0x3000           // +0x34..+0x3C;
#define WALL_VX 0x800
#define HOP_GRAVITY 0x19A
#define DODGE_SAFE 13           // +0x40, +0x44;
#define DODGE 50
#define IDLE_FOLLOW 40          // +0x48, +0x4C;
#define IDLE_STAND 30
#define PICK_A 40               // +0x54, +0x58: his picks' weights near Kirby;
#define PICK_B 60
#define FALL_GRAVITY 0x334      // +0x5C;
#define DASH_VX 0x29            // +0x64, +0x68;
#define CLOSE 30
#define DOUBLE_VX 0x29          // +0x74, +0x78;
#define DOUBLE_VX_2 0x52
#define THRUST_VY 0x5000        // +0x7C..+0x84;
#define THRUST_GRAVITY 0x334
#define THRUST_WAIT 60
#define BEAM_HOP_VX 0x5000      // +0x8C..+0x9C;
#define BEAM_HOP_VY 0x1800
#define BEAM_HOP_GRAVITY 0x334
#define BEAM_SPEED 0x2800
#define BEAM_TIME 240
#define WAVE_HOP_VY 0x2000      // +0xA0..+0xAC;
#define WAVE_HOP_VX 0x19A
#define WAVE_HOP_GRAVITY 0x19A
#define WAVE_TIME 60
#define TORNADO_HOP_VY 0x1800   // +0xB0..+0xC4;
#define TORNADO_HOP_VX 0x3000
#define TORNADO_HOP_GRAVITY 0x19A
#define TORNADO_WAIT 100
#define TORNADO_SPEED 0x1800
#define TORNADO_TIME 400
#define THRUST_SHAKE 30         // +0xF8;
#define COUNTER_VY 0x2000       // +0x100, +0x108;
#define COUNTER_GRAVITY 0x800
// His code's;
#define NEAR_AHEAD 0x30000      // 0x02113190 (its globals 0x0218194C, 0x02181950, 0x02181968: 48, 16, 80 pixels);
#define NEAR_UP 0x10000
#define HIGH_UP 0x50000
#define FOLLOW_NEAR 180
#define FOLLOW_ZONE 120
#define DROP_VY (-0x2000)       // 0x02115CD4's case 0xC;
#define DODGE_SLOW 0xCCC        // 0x02115BD0: 0.8, 1.0, 0.2;
#define DODGE_VX 0x1000
#define DODGE_LOW 0x333
#define SIDE 9                  // a7s7s2's walls: he stops 9 pixels in;
#define ZONE_LEFT 144           // a7s7s2's left: its middle less 144 pixels;
#define POINT_OUT 80            // a7s7s2's points 9 and 10 (64 and 224 across): 80 pixels either side of its middle;
#define LIFT 0x200              // 0x021119B0: Kirby lifted 2 pixels a frame (KatAM's units), to 140 over the floor (point 2 less 4);
#define LIFT_TOP 140
#define LIFT_TIME 34
#define SPARKLE_EVERY 12
#define SHAKE_SHORT 1           // KatAM's timed shakes (1-3: 14, 18, 24 frames) for Squeak Squad's;
#define SHAKE_MID 2
#define SHAKE_LONG 3

enum {
    MK_INTRO, MK_INTRO_WAIT, MK_IDLE, MK_MAIN, MK_WALK, MK_BACK, MK_RUN, MK_JUMP, MK_JUMP_ANIM, MK_JUMP_RISE, MK_JUMP_TOP, MK_HOP,
    MK_HOP_ANIM, MK_HOP_RISE, MK_FALL, MK_FALLING, MK_FLIP, MK_FLIP_ANIM, MK_FLIP_RISE, MK_FLIP_TOP, MK_COUNTER, MK_COUNTER_GO,
    MK_DODGE, MK_DODGE_GO, MK_DODGE_AIR, MK_DODGE_AIR_GO, MK_DASH, MK_DASH_SWING, MK_DASH_GO, MK_UP, MK_UP_WAIT, MK_UP_SWING,
    MK_UP_GO, MK_DOUBLE, MK_DOUBLE_1, MK_DOUBLE_1_GO, MK_DOUBLE_2, MK_DOUBLE_2_GO, MK_THRUST, MK_THRUST_CHARGE, MK_THRUST_RISE,
    MK_THRUST_TOP, MK_THRUST_FALL, MK_THRUST_LAND, MK_TORNADO, MK_TORNADO_HOP, MK_TORNADO_RAISE, MK_TORNADO_CALL, MK_TORNADO_WAIT,
    MK_BEAM, MK_BEAM_HOP, MK_BEAM_LAND, MK_BEAM_READY, MK_BEAM_SWING, MK_WAVE, MK_WAVE_HOP, MK_WAVE_RAISE, MK_WAVE_DOWN, MK_WAVE_END,
};
// 0x02113190's picks;
enum {
    PICK_WALK = 1, PICK_BACK, PICK_RUN, PICK_FOLLOW, PICK_FOLLOW_BACK, PICK_FOLLOW_RUN, PICK_JUMP = 0xB, PICK_DROP, PICK_DASH,
    PICK_UP, PICK_DOUBLE, PICK_THRUST, PICK_TORNADO, PICK_BEAM, PICK_BEAMS, PICK_WAVE,
};
enum { THING_BEAM, THING_WAVE, THING_TORNADO, THING_SPARKLE = 4, THING_REACH };
// Squeak Squad's flags after his move (0x0201355C);
#define AIR 0x40
#define WALL_AHEAD 0x800
#define WALL_BEHIND 0x1000
#define CEILING 0x2000

#define TIMER(s) ((s)->vars[0])         // +0xB4;
#define COUNT(s) ((s)->vars[1])         // +0xB8;
#define MODE(s) ((s)->vars[2])          // +0x228;
#define FOLLOW(s) ((s)->vars[3])        // +0x22C;
#define SAFE(s) ((s)->vars[4])          // +0x230;
#define STREAK(s) ((s)->vars[5])        // +0x43C;
#define INTRO(s) ((s)->vars[6])         // +0x438;
#define LAST_HP(s) ((s)->vars[7])
#define HARM(s) ((s)->vars[8])
#define FLOOR(s) ((s)->vars[9])         // The floor where he was placed (a7s7s2's deck);
#define REACH(s) ((s)->vars[10])        // His sword's reach (a THING_REACH);
#define FLAGS(s) ((s)->vars[11])        // Squeak Squad's flags after his last move;
#define THROUGH(s) ((s)->vars[12])      // His flag 0x100 this frame: through the terrain (his drop);

// His sprite's first box (metaknight_cmn.rca's animation 0, KatAM's: x ahead, y down): where he's hit, and his touch's;
static const s8 sBody[4] = { -9, -19, 9, -3 };

/* His sword's reach (metaknight_cmn.rca's / _ext.rca's kind 0 boxes on his slashes, KatAM's: x ahead, y down), by animation and its
 * frames (from, to); */
struct Reach {
    u8 anim;
    u8 from;
    u8 to;
    s8 box[4];
};
static const struct Reach sReach[] = {
    { ANIM_SLASH, 3, 4, { 6, -37, 48, 7 } },
    { ANIM_AIR_SLASH, 0, 19, { -17, -41, 22, 5 } },
    { ANIM_UP_SLASH, 2, 3, { -18, -53, 16, -16 } },
    { ANIM_UP_SLASH, 4, 5, { -10, -75, 21, -16 } },
    { ANIM_UP_SLASH, 6, 14, { -18, -53, 16, -16 } },
    { ANIM_DOUBLE, 11, 12, { 6, -37, 52, 11 } },
    { ANIM_DOUBLE, 27, 28, { 6, -37, 52, 11 } },
    { ANIM_DOUBLE_2, 14, 15, { 6, -31, 57, 9 } },
    { X_THRUST, 0, 255, { -7, -17, 12, 17 } },
    { X_SWING, 3, 3, { 6, -32, 54, 7 } },
    { X_WAVE_UP, 0, 255, { -7, -44, 8, 0 } },
    { X_WAVE_DOWN, 0, 255, { -18, -23, 24, 7 } },
};
// (ext's .mca's 2: its part 14, his body, raised: 12 behind to 13 ahead, 3 to 54 up);
static const s8 sRaised[4] = { -12, -54, 13, -3 };

static u32 Random(u32 n) {
    return Rand16() * n >> 16;
}

static s32 T(struct SqueakObject *s) {
    return (s16)s->stateTimer;
}

static struct LevelInfo *Level(struct SqueakObject *s) {
    return &gCurLevelInfo[s->obj.base.unk56];
}

// Middle(); The camera view's middle across (a7s7s2's 144; here KatAM's view, 240 across), as Daroach's;
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

// FaceKirby(); Turned round when Kirby is behind him (0x0201AE58's distance under 0);
static void FaceKirby(struct SqueakObject *s) {
    if (Ahead(s, s->obj.kirby3->base.x) < 0) SqueakTurnAround(s);
}

/* Zone(); 0x02209C7C: a spot's zone in a7s7s2's grid (its 16-pixel tiles from its left, the view's middle less 144; up from the
 * floor where he was placed, the stage's 80); -1 out of it; */
static s32 Zone(struct SqueakObject *s, s32 x, s32 y) {
    s32 up = (FLOOR(s) - y) >> 8, dx = x - (Middle(s) - (ZONE_LEFT << 8)), col;

    if (dx < 0) return -1;
    col = (dx >> 8) / 16;
    if (col >= 18 || up >= 160 || up < -80) return -1;
    if (up >= 128) return col >= 2 && col <= 14 && !(col & 1) ? col / 2 + 1 : 1;
    if (up >= 64) return 1;
    if (up >= -32) return 0;
    return col < 4 ? 9 : col > 12 ? 10 : 0;
}

/* ThinFloor(); Standing on a floor he drops through (0x02115CD4's floor object, its vtable +0x74: a7s7s2's thin decks; KatAM's
 * collision tiles Kirby drops through: 0x0E, the slopes 0x10-0x13, 0x18, 0x19); */
static bool32 ThinFloor(struct SqueakObject *s) {
    u8 tile;

    if (!SqueakOnGround(s)) return FALSE;
    tile = GetCollisionTile(s->obj.base.unk56, s->obj.base.x >> 12, (s->obj.base.y + 0x100) >> 12);
    return tile == 0x0E || (tile >= 0x10 && tile <= 0x13) || tile == 0x18 || tile == 0x19;
}

/* Move(); 0x02013D18 on the terrain (through it with his flag 0x100: his drop), then Squeak Squad's flags: 0x40 in the air, 0x800 a
 * wall ahead, 0x1000 behind, 0x2000 a ceiling; the arena's walls the camera view's sides (a7s7s2's at the DS view's: he stops SIDE
 * pixels in); */
static void Move(struct SqueakObject *s) {
    struct ObjectBase *base = &s->obj.base;
    s32 left = Level(s)->viewportPosition.x + (SIDE << 8), right = Level(s)->viewportPosition.x + ((DISPLAY_WIDTH - SIDE) << 8);
    u32 f = 0;

    SqueakThroughTerrain(s, THROUGH(s));
    SqueakMove(s);
    if (!SqueakOnGround(s)) f |= AIR;
    if (SqueakWallAhead(s)) f |= WALL_AHEAD;
    if (s->terrain & 2) f |= WALL_BEHIND;
    if (SqueakCeiling(s)) f |= CEILING;
    if (base->x <= left) {
        base->x = left;
        f |= FacesLeft(s) ? WALL_AHEAD : WALL_BEHIND;
    } else if (base->x >= right) {
        base->x = right;
        f |= FacesLeft(s) ? WALL_BEHIND : WALL_AHEAD;
    }
    FLAGS(s) = f;
}

static bool32 InAir(struct SqueakObject *s) {
    return FLAGS(s) & AIR;
}

// Clear(); Squeak Squad's speeds' caps, accelerations and friction to 0 (and the speed up), his speed across kept;
static void Clear(struct SqueakObject *s) {
    s->vy = s->maxVx = s->maxVy = s->ax = s->ay = s->fx = s->fy = 0;
}

// Slow(); 0x02113484: no acceleration across, slowing by WALK_SPEED a frame (his friction);
static void Slow(struct SqueakObject *s) {
    s->ax = 0;
    s->fx = WALK_SPEED;
}

// Thing(); What he brings in (its character by its kind), dx ahead and dy up of him (Squeak Squad's units), facing as he does;
static const u8 sThingEnemies[] = {
    [THING_BEAM] = SQUEAK_META_KNIGHT_BEAM, [THING_WAVE] = SQUEAK_META_KNIGHT_WAVE, [THING_TORNADO] = SQUEAK_META_KNIGHT_THING,
    [THING_SPARKLE] = SQUEAK_META_KNIGHT_SPARKLE, [THING_REACH] = SQUEAK_META_KNIGHT_SPARKLE,
};

static struct SqueakObject *Thing(struct SqueakObject *s, u8 kind, s32 dx, s32 dy) {
    struct SqueakObject *t = SqueakSpawn(s, sThingEnemies[kind], kind, dx, dy);

    if (t == NULL) return NULL;
    t->obj.base.parent = &s->obj;
    Face(t, FacesLeft(s));
    return t;
}

// Star(); 0x0212389C's kinds 3 and 4: a star (a Sword when inhaled) 16 pixels ahead of him or behind;
static void Star(struct SqueakObject *s, bool32 behind) {
    struct SqueakObject *t = SqueakSpawn(s, SQUEAK_STAR, behind ? SQUEAK_STAR_SWORD_BEHIND : SQUEAK_STAR_SWORD_AHEAD,
                                         behind ? -0x10000 : 0x10000, 0);

    if (t) t->obj.base.parent = &s->obj;
}

/* Pick(); 0x02113190: his move by where Kirby is (`mode` returned for none: his mode kept); a follow sets its frames (+0x22C); */
static u8 Pick(struct SqueakObject *s, u8 mode) {
    struct Kirby *kirby = s->obj.kirby3;
    s32 ahead = Ahead(s, kirby->base.x), up = Up(s, kirby->base.y), zone;
    u32 r, t;

    if (ahead >= 0 && up >= 0 && ahead < NEAR_AHEAD) {
        if (up < NEAR_UP) {
            r = Random(100);
            if (r < (t = PICK_A * 50 / 100)) {
                FOLLOW(s) = FOLLOW_NEAR;
                return PICK_FOLLOW;
            }
            if (r < (t += PICK_A * 50 / 100)) return PICK_BACK;
            if (r < (t += PICK_B * 25 / 100)) return PICK_DASH;
            if (r < (t += PICK_B * 25 / 100)) return PICK_DOUBLE;
            if (r < (t += PICK_B * 25 / 100)) return PICK_THRUST;
            if (r < (t += PICK_B * 1000 / 10000)) return PICK_TORNADO;
            if (r < t + PICK_B * 1000 / 10000) return PICK_BEAM;
            return PICK_BEAMS;
        }
        if (up < HIGH_UP) {
            r = Random(100);
            if (r < 20) return PICK_UP;
            if (r < 50) return PICK_WAVE;
            return PICK_JUMP;
        }
    }
    zone = Zone(s, kirby->base.x, kirby->base.y);
    if (zone == Zone(s, s->obj.base.x, s->obj.base.y)) {
        r = Random(100);
        if (r <= 20) return PICK_TORNADO;
        if (r <= 40) return PICK_BEAM;
        if (r <= 50) return PICK_BEAMS;
        if (r <= 65) {
            FOLLOW(s) = FOLLOW_ZONE;
            return PICK_FOLLOW_RUN;
        }
        if (r <= 83) return PICK_WALK;
        return PICK_RUN;
    }
    r = Random(100);
    if (r <= 35) return PICK_WALK;
    if (r <= 70) return PICK_RUN;
    if (zone == 0) return PICK_DROP;
    if (zone >= 1) return PICK_JUMP;
    return mode;
}

/* Intro(); 0x02113018's timeline (+0x438, from his intro on): the screen rumbling, 17 frames from his 86th, 180 from his 163rd (here
 * KatAM's shakes, again as each ends); */
static void Intro(struct SqueakObject *s) {
    s32 t = INTRO(s);

    if (t < 0) return;
    INTRO(s) = ++t;
    if (t == 86) RequestScreenShake(SHAKE_MID, &s->obj.base);
    else if (t >= 163 && t < 343 && (t - 163) % 14 == 0) RequestScreenShake(SHAKE_SHORT, &s->obj.base);
    else if (t >= 500) INTRO(s) = -1;
}

static bool32 MetaKnight(struct SqueakObject *s) {
    struct Kirby *kirby = s->obj.kirby3;
    bool32 pick = FALSE;
    s32 d;
    u8 a;

    switch (s->state) {
    case MK_INTRO:          // 0x02116468: his pose, his timeline started;
        SqueakPlay(s, ANIM_INTRO);
        SqueakStop(s);
        TIMER(s) = 0;
        INTRO(s) = 0;
        SqueakSetState(s, MK_INTRO_WAIT);
        break;
    case MK_INTRO_WAIT:     // 0x021163B8;
        if (!s->animEnded) break;
        MODE(s) = 6;
        SqueakSetState(s, MK_RUN);
        return TRUE;
    case MK_IDLE:           // 0x02116300 (his vtable's +0x3C);
        SqueakPlay(s, ANIM_IDLE);
        SqueakStop(s);
        TIMER(s) = 0;
        if (Random(100) > IDLE_FOLLOW) {
            FOLLOW(s) = FOLLOW_ZONE;
            MODE(s) = 4;
        } else {
            MODE(s) = 7;
        }
        SqueakSetState(s, MK_MAIN);
        return TRUE;
    case MK_WALK:           // 0x021157BC;
        FaceKirby(s);
        SqueakPlay(s, ANIM_WALK);
        SqueakStop(s);
        s->vx = WALK_SPEED;
        TIMER(s) = 0;
        MODE(s) = 1;
        SqueakSetState(s, MK_MAIN);
        return TRUE;
    case MK_BACK:           // 0x0211573C;
        SqueakPlay(s, ANIM_BACK);
        SqueakStop(s);
        s->vx = -BACK_SPEED;
        TIMER(s) = 0;
        MODE(s) = 2;
        SqueakSetState(s, MK_MAIN);
        return TRUE;
    case MK_RUN:            // 0x02115694;
        FaceKirby(s);
        SqueakPlay(s, ANIM_RUN);
        SqueakStop(s);
        s->vx = RUN_SPEED;
        TIMER(s) = 0;
        MODE(s) = 3;
        SqueakSetState(s, MK_MAIN);
        return TRUE;
    case MK_MAIN:           // 0x02115CD4;
        switch (MODE(s)) {
        case 0:
            if (++TIMER(s) > 3) {
                SqueakSetState(s, MK_WALK);
                return TRUE;
            }
            break;
        case 1: case 3:
            d = Ahead(s, kirby->base.x);
            if (d < 0) SqueakTurnAround(s);
            else if (d < NEAR_AHEAD) pick = TRUE;
            break;
        case 2:
            if (FLAGS(s) & WALL_BEHIND) {
                SqueakSetState(s, Random(100) < 50 ? MK_WALK : MK_JUMP);
                return TRUE;
            }
            break;
        case 4: case 5: case 6:
            if (FOLLOW(s) < 1) {
                MODE(s) = 0;
                SqueakSetState(s, MK_IDLE);
                return TRUE;
            }
            if (FOLLOW(s) % 30 == 0) {
                d = Ahead(s, kirby->base.x);
                if (d < 1 || d > NEAR_AHEAD) {
                    if (d < NEAR_AHEAD || d > 2 * NEAR_AHEAD - 1) {
                        if (d < 2 * NEAR_AHEAD) {
                            SqueakTurnAround(s);
                        } else if (MODE(s) != 6) {
                            MODE(s) = 6;
                            SqueakSetState(s, MK_RUN);
                            return TRUE;
                        }
                    } else if (MODE(s) != 4) {
                        MODE(s) = 4;
                        SqueakSetState(s, MK_WALK);
                        return TRUE;
                    }
                } else if (MODE(s) != 5) {
                    MODE(s) = 5;
                    SqueakSetState(s, MK_BACK);
                    return TRUE;
                }
            }
            FOLLOW(s)--;
            break;
        default:
            pick = TRUE;
            break;
        }
        if (pick) {
            switch (a = Pick(s, MODE(s))) {
            case PICK_WALK: SqueakSetState(s, MK_WALK); return TRUE;
            case PICK_RUN: SqueakSetState(s, MK_RUN); return TRUE;
            case PICK_FOLLOW: case PICK_FOLLOW_BACK: case PICK_FOLLOW_RUN: MODE(s) = a; break;
            case PICK_JUMP: SqueakSetState(s, MK_JUMP); return TRUE;
            case PICK_DROP:
                if (ThinFloor(s)) {
                    THROUGH(s) = TRUE;
                    s->vy = DROP_VY;
                }
                TIMER(s) = 0;
                MODE(s) = 0;
                break;
            case PICK_DASH: SqueakSetState(s, MK_DASH); return FALSE;
            case PICK_UP: SqueakSetState(s, MK_UP); return FALSE;
            case PICK_DOUBLE: SqueakSetState(s, MK_DOUBLE); return FALSE;
            case PICK_THRUST: SqueakSetState(s, MK_THRUST); return FALSE;
            case PICK_TORNADO: SqueakSetState(s, MK_TORNADO); return FALSE;
            case PICK_BEAMS: MODE(s) = 10;  // (and on);
            case PICK_BEAM: SqueakSetState(s, MK_BEAM); return FALSE;
            case PICK_WAVE: SqueakSetState(s, MK_WAVE); return FALSE;
            }
        }
        Move(s);
        THROUGH(s) = FALSE;
        if (FLAGS(s) & WALL_AHEAD) {
            SqueakSetState(s, MK_FLIP);
            break;
        }
        if (!InAir(s)) break;
        SqueakSetState(s, s->vx < JUMP_OFF ? MK_FALL : MK_JUMP);
        break;
    case MK_JUMP:           // 0x021155EC;
        SqueakPlay(s, ANIM_JUMP);
        Clear(s);
        s->vx = RUN_SPEED;
        s->vy = JUMP_VY;
        s->ay = -GRAVITY;
        SqueakSetState(s, MK_JUMP_ANIM);
        return TRUE;
    case MK_JUMP_ANIM:      // 0x0211559C;
        SqueakPlay(s, ANIM_RISE);
        SqueakSetState(s, MK_JUMP_RISE);
        return TRUE;
    case MK_JUMP_RISE:      // 0x0211553C;
        Move(s);
        if (s->vy < 1) {
            SqueakStop(s);
            SqueakPlay(s, ANIM_TOP);
            SqueakSetState(s, MK_JUMP_TOP);
        }
        break;
    case MK_JUMP_TOP:       // 0x02115440: at his pose's end, by Kirby's zone;
        Move(s);
        if (!s->animEnded) break;
        if (Zone(s, kirby->base.x, kirby->base.y) < 1) {
            u32 r = Random(100);

            SqueakSetState(s, r < 40 ? MK_HOP : r < 70 ? MK_DASH : MK_FALL);
        } else {
            SqueakSetState(s, MK_HOP);
        }
        return TRUE;
    case MK_HOP:            // 0x021150D8;
        SqueakPlay(s, ANIM_JUMP);
        SqueakStop(s);
        s->vy = HOP_VY;
        s->ay = -HOP_GRAVITY;
        TIMER(s) = 0;
        SqueakSetState(s, MK_HOP_ANIM);
        return TRUE;
    case MK_HOP_ANIM:       // 0x02115094;
        SqueakPlay(s, ANIM_HOP);
        SqueakSetState(s, MK_HOP_RISE);
        return TRUE;
    case MK_HOP_RISE:       // 0x02115034;
        Slow(s);
        Move(s);
        if (s->vy < 1) {
            SqueakPlay(s, ANIM_TOP);
            SqueakSetState(s, MK_FALL);
        }
        break;
    case MK_FALL:           // 0x02113C78;
        SqueakPlay(s, ANIM_FALL);
        Clear(s);
        s->ay = -FALL_GRAVITY;
        TIMER(s) = 0;
        SqueakSetState(s, MK_FALLING);
        return TRUE;
    case MK_FALLING:        // 0x02113B24: every 20 frames a pick; landed, a walk at Kirby;
        Slow(s);
        Move(s);
        if (TIMER(s) % 20 == 5) {
            switch (Pick(s, MODE(s))) {
            case PICK_DASH: case PICK_DOUBLE: case PICK_THRUST: case PICK_TORNADO: case PICK_BEAM: case PICK_BEAMS:
                SqueakSetState(s, MK_DASH);
                return FALSE;
            case PICK_UP:
                SqueakSetState(s, MK_UP);
                return FALSE;
            case PICK_WAVE:
                SqueakSetState(s, MK_WAVE);
                return FALSE;
            }
        }
        if (InAir(s)) {
            TIMER(s)++;
            break;
        }
        SqueakPlay(s, ANIM_LAND);
        s->vy = s->ay = s->ax = 0;
        MODE(s) = 7;
        SqueakSetState(s, MK_WALK);
        return TRUE;
    case MK_FLIP:           // 0x021153AC: his backflip;
        SqueakPlay(s, ANIM_FLIP);
        SqueakStop(s);
        s->vx = -FLIP_VX;
        s->vy = FLIP_VY;
        s->ay = -FLIP_GRAVITY;
        TIMER(s) = 0;
        SqueakSetState(s, MK_FLIP_ANIM);
        return TRUE;
    case MK_FLIP_ANIM:      // 0x02115368;
        SqueakPlay(s, ANIM_FLIP_RISE);
        SqueakSetState(s, MK_FLIP_RISE);
        return TRUE;
    case MK_FLIP_RISE:      // 0x021152C8;
        Move(s);
        if (FLAGS(s) & WALL_BEHIND) {
            s->vx = WALL_VX;
            SqueakSetState(s, MK_HOP);
            break;
        }
        if (s->vy < 1) {
            SqueakStop(s);
            SqueakPlay(s, ANIM_TOP);
            SqueakSetState(s, MK_FLIP_TOP);
        }
        break;
    case MK_FLIP_TOP:       // 0x02115278;
        Move(s);
        if (!s->animEnded) break;
        s->vx = 0;
        SqueakSetState(s, MK_HOP);
        return TRUE;
    case MK_COUNTER:        // 0x021151EC: a hop away after a hit;
        SqueakPlay(s, ANIM_COUNTER);
        SqueakStop(s);
        s->vx = -FLIP_VX;
        s->vy = COUNTER_VY;
        s->ay = -COUNTER_GRAVITY;
        TIMER(s) = COUNT(s) = 0;
        SqueakSetState(s, MK_COUNTER_GO);
        return TRUE;
    case MK_COUNTER_GO:     // 0x02115154;
        Move(s);
        if (!InAir(s)) {
            TIMER(s) = 1;
            SqueakStop(s);
        }
        if (TIMER(s) > 0 && COUNT(s)++ > 14) {
            TIMER(s) = COUNT(s) = 0;
            MODE(s) = 7;
            SqueakSetState(s, MK_WALK);
        }
        break;
    case MK_DODGE:          // 0x02115BD0: on the ground;
        SqueakPlay(s, ANIM_DODGE);
        s->vx = s->vx >= 0 && s->vx < DODGE_SLOW ? -DODGE_VX : -s->vx;
        s->vy = Random(3) < 1 ? JUMP_VY : DODGE_LOW;
        s->ay = -GRAVITY;
        TIMER(s) = 0;
        MODE(s) = 8;
        SqueakSetState(s, MK_DODGE_GO);
        return TRUE;
    case MK_DODGE_GO:       // 0x02115B14: while safe, then a backflip;
        if (SAFE(s) > 0) {
            Move(s);
            break;
        }
        MODE(s) = 7;
        s->vx = 0;
        SqueakSetState(s, MK_FLIP);
        break;
    case MK_DODGE_AIR:      // 0x02115A38;
        SqueakPlay(s, ANIM_DODGE);
        s->vx = -s->vx;
        s->vy = Random(3) < 1 ? JUMP_VY : DODGE_LOW;
        s->ay = -GRAVITY;
        TIMER(s) = 0;
        MODE(s) = 8;
        SqueakSetState(s, MK_DODGE_AIR_GO);
        return TRUE;
    case MK_DODGE_AIR_GO:   // 0x02115924: landed, idle; a ceiling or a hit after it, a fall;
        Move(s);
        if (!InAir(s)) {
            MODE(s) = 7;
            SqueakSetState(s, MK_IDLE);
            break;
        }
        if (FLAGS(s) & CEILING) {
            MODE(s) = 7;
            SqueakSetState(s, MK_FALL);
            break;
        }
        if (SAFE(s) < 0) {
            MODE(s) = 7;
            SqueakSetState(s, MK_FALL);
            return TRUE;
        }
        break;
    case MK_DASH:           // 0x02114FB8: drawn, at Kirby;
        SqueakPlay(s, ANIM_DRAW);
        FaceKirby(s);
        s->vx = DASH_VX;
        MODE(s) = 9;
        SqueakSetState(s, MK_DASH_SWING);
        return TRUE;
    case MK_DASH_SWING:     // 0x02114F44;
        SqueakPlay(s, InAir(s) ? ANIM_AIR_SLASH : ANIM_SLASH);
        SqueakSetState(s, MK_DASH_GO);
        return TRUE;
    case MK_DASH_GO:        // 0x02114E54: facing Kirby (stopped within CLOSE), then a star ahead and a walk;
        d = Ahead(s, kirby->base.x);
        if (d < 0) {
            SqueakTurnAround(s);
        } else if (d < CLOSE) {
            s->vx = s->ax = s->maxVx = s->maxVy = s->fx = s->fy = 0;
        }
        Move(s);
        if (!s->animEnded) break;
        Star(s, FALSE);
        MODE(s) = 1;
        SqueakSetState(s, MK_WALK);
        break;
    case MK_UP:             // 0x02114DA8;
        FaceKirby(s);
        SqueakPlay(s, ANIM_UP_READY);
        s->vx = 0;
        s->maxVx = s->maxVy = s->ax = s->ay = s->fx = s->fy = 0;
        MODE(s) = 9;
        SqueakSetState(s, MK_UP_WAIT);
        return TRUE;
    case MK_UP_WAIT:        // 0x02114D64;
        if (!s->animEnded) break;
        SqueakSetState(s, MK_UP_SWING);
        return TRUE;
    case MK_UP_SWING:       // 0x02114D14;
        SqueakPlay(s, ANIM_UP_SLASH);
        SqueakSetState(s, MK_UP_GO);
        return TRUE;
    case MK_UP_GO:          // 0x02114C94: in the air, a fall;
        Move(s);
        if (s->animEnded) {
            MODE(s) = 7;
            SqueakSetState(s, MK_WALK);
            return TRUE;
        }
        if (InAir(s)) SqueakSetState(s, MK_FALL);
        break;
    case MK_DOUBLE:         // 0x02114C10;
        FaceKirby(s);
        SqueakPlay(s, ANIM_DRAW);
        SqueakStop(s);
        s->vx = DOUBLE_VX;
        MODE(s) = 9;
        SqueakSetState(s, MK_DOUBLE_1);
        return TRUE;
    case MK_DOUBLE_1:       // 0x02114BCC;
        SqueakPlay(s, ANIM_DOUBLE);
        SqueakSetState(s, MK_DOUBLE_1_GO);
        return TRUE;
    case MK_DOUBLE_1_GO:    // 0x02114B2C;
        if (s->animEnded) {
            SqueakSetState(s, MK_DOUBLE_2);
            return TRUE;
        }
        Move(s);
        break;
    case MK_DOUBLE_2:       // 0x02114AD0;
        SqueakPlay(s, ANIM_DOUBLE_2);
        SqueakStop(s);
        s->vx = DOUBLE_VX_2;
        SqueakSetState(s, MK_DOUBLE_2_GO);
        return TRUE;
    case MK_DOUBLE_2_GO:    // 0x021149B0: facing Kirby (stopped within CLOSE);
        d = Ahead(s, kirby->base.x);
        if (d < 0) {
            SqueakTurnAround(s);
        } else if (d < CLOSE) {
            s->vx = 0;
            s->maxVx = s->maxVy = s->ax = s->ay = s->fx = s->fy = 0;
        }
        if (s->animEnded) {
            MODE(s) = 7;
            SqueakSetState(s, MK_WALK);
            return TRUE;
        }
        Move(s);
        break;
    case MK_THRUST:         // 0x021148E0: held, then up (his speed across kept);
        FaceKirby(s);
        SqueakPlay(s, X_CHARGE);
        Clear(s);
        s->vy = THRUST_VY;
        s->ay = -THRUST_GRAVITY;
        TIMER(s) = 0;
        MODE(s) = 9;
        SqueakSetState(s, MK_THRUST_CHARGE);
        return TRUE;
    case MK_THRUST_CHARGE:  // 0x02114868: 30 frames, not moving;
        if (++TIMER(s) > 30) {
            TIMER(s) = 0;
            SqueakPlay(s, X_CHARGED);
            SqueakSetState(s, MK_THRUST_RISE);
            return TRUE;
        }
        break;
    case MK_THRUST_RISE:    // 0x02114808;
        Move(s);
        if (s->vy < 1) {
            SqueakStop(s);
            SqueakPlay(s, X_RISE);
            SqueakSetState(s, MK_THRUST_TOP);
            return TRUE;
        }
        break;
    case MK_THRUST_TOP:     // 0x02114750: still till his pose's end, then down;
        if (s->animEnded) {
            Clear(s);
            s->ay = -GRAVITY;
            RequestScreenShake(SHAKE_SHORT, &s->obj.base);
            SqueakPlay(s, X_THRUST);
            SqueakSetState(s, MK_THRUST_FALL);
            return TRUE;
        }
        Move(s);
        break;
    case MK_THRUST_FALL:    // 0x02114678: landed, stars either side, the screen shaking;
        if (InAir(s)) {
            Move(s);
            break;
        }
        TIMER(s) = 0;
        Star(s, FALSE);
        Star(s, TRUE);
        RequestScreenShake(SHAKE_LONG, &s->obj.base);
        SqueakPlayComposite(s, C_THRUST_LAND);
        SqueakSetState(s, MK_THRUST_LAND);
        return TRUE;
    case MK_THRUST_LAND:    // 0x02114604;
        if (++TIMER(s) > THRUST_WAIT) {
            TIMER(s) = 0;
            MODE(s) = 7;
            SqueakSetState(s, MK_WALK);
            return TRUE;
        }
        break;
    case MK_TORNADO:        // 0x02113ED0: a hop back;
        SqueakPlay(s, X_TORNADO_HOP);
        FaceKirby(s);
        Clear(s);
        s->vx = -TORNADO_HOP_VX;
        s->vy = TORNADO_HOP_VY;
        s->ay = -TORNADO_HOP_GRAVITY;
        TIMER(s) = 0;
        MODE(s) = 9;
        SqueakSetState(s, MK_TORNADO_HOP);
        return TRUE;
    case MK_TORNADO_HOP:    // 0x02113E4C: landed, his call, the screen shaking;
        Move(s);
        if (InAir(s)) break;
        SqueakStop(s);
        SqueakPlayComposite(s, C_TORNADO_CALL);
        RequestScreenShake(SHAKE_LONG, &s->obj.base);
        SqueakSetState(s, MK_TORNADO_RAISE);
        break;
    case MK_TORNADO_RAISE:  // 0x02113DF4;
        Move(s);
        if (!s->animEnded) break;
        SqueakPlay(s, X_TORNADO_RAISED);
        SqueakSetState(s, MK_TORNADO_CALL);
        break;
    case MK_TORNADO_CALL:   // 0x02113D84: his tornado where he stands;
        Move(s);
        if (!s->animEnded) break;
        TIMER(s) = 0;
        Thing(s, THING_TORNADO, 0, 0);
        SqueakPlay(s, X_TORNADO_POSE);
        SqueakSetState(s, MK_TORNADO_WAIT);
        break;
    case MK_TORNADO_WAIT:   // 0x02113D10;
        Move(s);
        if (++TIMER(s) > TORNADO_WAIT) {
            TIMER(s) = 0;
            MODE(s) = 7;
            SqueakSetState(s, MK_WALK);
            return TRUE;
        }
        break;
    case MK_BEAM:           // 0x021144FC: a hop back (three: COUNT 3);
        FaceKirby(s);
        SqueakPlay(s, X_HOP_BACK);
        Clear(s);
        s->vx = -BEAM_HOP_VX;
        s->vy = BEAM_HOP_VY;
        s->ay = -BEAM_HOP_GRAVITY;
        TIMER(s) = 0;
        COUNT(s) = MODE(s) == 10 ? 3 : 0;
        MODE(s) = 9;
        SqueakSetState(s, MK_BEAM_HOP);
        return TRUE;
    case MK_BEAM_HOP:       // 0x02114488;
        Move(s);
        if (InAir(s)) break;
        SqueakStop(s);
        SqueakPlay(s, X_READY);
        SqueakSetState(s, MK_BEAM_LAND);
        break;
    case MK_BEAM_LAND:      // 0x021143CC;
        Move(s);
        if (!s->animEnded) break;
        RequestScreenShake(SHAKE_LONG, &s->obj.base);
        SqueakPlayComposite(s, C_BEAM_READY);
        SqueakSetState(s, MK_BEAM_READY);
        break;
    case MK_BEAM_READY:     // 0x02114324: (ready, or a swing without a beam) then a swing;
        Move(s);
        if (!s->animEnded) break;
        SqueakPlay(s, X_SWING);
        SqueakSetState(s, MK_BEAM_SWING);
        break;
    case MK_BEAM_SWING:     // 0x02114228: its beam on the swing's 3rd frame;
        Move(s);
        if (s->composite == SQUEAK_NO_COMPOSITE && s->anim == X_SWING && SqueakAnimTime(s) == 3) Thing(s, THING_BEAM, 0, 0);
        if (!s->animEnded) break;
        if (--COUNT(s) < 1) {
            MODE(s) = 7;
            SqueakSetState(s, MK_WALK);
        } else {
            SqueakPlay(s, X_SWING);
            SqueakSetState(s, MK_BEAM_READY);
        }
        break;
    case MK_WAVE:           // 0x02114148: a hop;
        SqueakPlay(s, X_WAVE_HOP);
        FaceKirby(s);
        Clear(s);
        s->vx = -WAVE_HOP_VX;
        s->vy = WAVE_HOP_VY;
        s->ay = -WAVE_HOP_GRAVITY;
        TIMER(s) = 0;
        MODE(s) = 9;
        SqueakSetState(s, MK_WAVE_HOP);
        return TRUE;
    case MK_WAVE_HOP:       // 0x021140E0;
        Move(s);
        if (InAir(s)) break;
        SqueakStop(s);
        SqueakPlayComposite(s, C_WAVE_RAISE);
        SqueakSetState(s, MK_WAVE_RAISE);
        break;
    case MK_WAVE_RAISE:     // 0x02114088;
        Move(s);
        if (!s->animEnded) break;
        SqueakPlay(s, X_WAVE_UP);
        SqueakSetState(s, MK_WAVE_DOWN);
        break;
    case MK_WAVE_DOWN:      // 0x02114004: the wave where he stands, the screen shaking;
        Move(s);
        if (!s->animEnded) break;
        Thing(s, THING_WAVE, 0, 0);
        RequestScreenShake(SHAKE_MID, &s->obj.base);
        SqueakPlay(s, X_WAVE_DOWN);
        SqueakSetState(s, MK_WAVE_END);
        break;
    case MK_WAVE_END:       // 0x02113FB0;
        Move(s);
        if (!s->animEnded) break;
        MODE(s) = 7;
        SqueakSetState(s, MK_WALK);
        break;
    }
    return FALSE;
}

/* Hit(); 0x021166BC (KatAM's mini-bosses take hits without a hurt state of their own: a fall of his HP is the hit): safe (+0x230,
 * counting down each hit too), else dodged in modes 0-2 and 4-7 (none taken) or taken, with a hop away after when Kirby is low; */
static void Hit(struct SqueakObject *s) {
    struct Object *obj = &s->obj;
    struct Kirby *kirby = obj->kirby3;
    s32 chance = DODGE, max = SqueakMaxHp(obj);
    bool32 counter = FALSE, low = kirby->hp * 36 < 17 * kirby->maxHp, modes;
    u32 r;

    // (0x02113018: his safe frames count down; their last but one, a hit more in his streak);
    if (SAFE(s) > 0 && --SAFE(s) == 1) STREAK(s)++;
    if (obj->unk80 < LAST_HP(s) && obj->unk80 > 0 && !(obj->base.flags & 0x1000)) {
        if (--SAFE(s) > 0) {
            obj->unk80 = LAST_HP(s);
        } else {
            r = Random(100);
            modes = MODE(s) <= 2 || (MODE(s) >= 4 && MODE(s) <= 7);
            if (low) chance /= 2;
            else if (kirby->hp * 36 > 50 * kirby->maxHp) chance = 100;
            if (STREAK(s) == 3) {
                chance += 30;
                STREAK(s) = 0;
            }
            if (low ? obj->unk80 * 20 < max : obj->unk80 * 25 < max) chance += 30;
            if (r < (u32)chance && modes) {
                if (low) {
                    counter = TRUE;
                } else {
                    obj->unk80 = LAST_HP(s);
                    SAFE(s) = DODGE_SAFE;
                    SqueakSetState(s, InAir(s) ? MK_DODGE_AIR : MK_DODGE);
                    LAST_HP(s) = obj->unk80;
                    return;
                }
            }
            if (counter) SqueakSetState(s, MK_COUNTER);
            SAFE(s) = SAFE_TIME;
        }
    }
    LAST_HP(s) = obj->unk80;
}

/* Reach(); His sword's reach this frame (sReach by his animation's frames; ext's .mca's 2, raised): its box on THING_REACH, made once,
 * again if lost; */
static void Reach(struct SqueakObject *s) {
    struct SqueakObject *r = (struct SqueakObject *)REACH(s);
    const s8 *box = NULL;
    u32 t;
    u8 i;

    if (s->composite == C_WAVE_RAISE) {
        box = sRaised;
    } else if (s->composite == SQUEAK_NO_COMPOSITE) {
        t = SqueakAnimTime(s);
        for (i = 0; i < ARRAY_COUNT(sReach); i++)
            if (sReach[i].anim == s->anim && t >= sReach[i].from && t <= sReach[i].to) box = sReach[i].box;
    }
    if (!(r && r->obj.base.header.kind == 1 && !(r->obj.base.flags & 0x1000) && r->obj.base.parent == &s->obj && SqueakIsSqueak(&r->obj)
          && r->enemy == SQUEAK_META_KNIGHT_SPARKLE && r->variant == THING_REACH)) {
        r = Thing(s, THING_REACH, 0, 0);
        REACH(s) = (s32)r;
        if (r == NULL) return;
    }
    r->obj.base.x = s->obj.base.x;
    r->obj.base.y = s->obj.base.y;
    Face(r, FacesLeft(s));
    if (box) {
        sub_0803E2B0(&r->obj.base, box[0], box[1], box[2], box[3]);
        r->obj.base.unk68 = HARM(s);
    } else {
        sub_0803E2B0(&r->obj.base, -1, -1, 1, 1);
        r->obj.base.unk68 = 0;
    }
}

// Boxes(); His box (where he's hit and his touch's: none in his intro);
static void Boxes(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, sBody[0], sBody[1], sBody[2], sBody[3]);
    s->obj.base.unk68 = s->composite == SQUEAK_NO_COMPOSITE && s->anim == ANIM_INTRO ? 0 : HARM(s);
}

static void MetaKnightMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    obj->kirby3 = FindTargetKirby(&obj->base);
    SqueakMidbossUpdate(s);
    Hit(s);
    SqueakRun(s, MetaKnight);
    if (obj->unk80 <= 0) return;
    // (his HP bar Squeak Squad's boss's, which his things' hits don't take: KatAM's goes to whatever is hit, taken back);
    sub_08034E14(obj);
    Intro(s);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
    Boxes(s);
    Reach(s);
}

/* SqueakMetaKnightStart(); His fight's start (and after a hit: it goes on, as KatAM's mini-bosses do), on the terrain; */
void SqueakMetaKnightStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, -1, MetaKnightMain);
    SqueakThroughTerrain(s, FALSE);
    if (s->started) return;
    LAST_HP(s) = obj->unk80;
    SAFE(s) = STREAK(s) = MODE(s) = FOLLOW(s) = 0;
    INTRO(s) = -1;
    REACH(s) = THROUGH(s) = FLAGS(s) = 0;
    SqueakSetState(s, MK_INTRO);
}

// SqueakMetaKnightInit(); His box, his bounds (KatAM's), the floor where he's placed (a7s7s2's deck); still till his fight;
void SqueakMetaKnightInit(struct SqueakObject *s) {
    struct Object *obj = &s->obj;

    sub_0803E2B0(&obj->base, sBody[0], sBody[1], sBody[2], sBody[3]);
    ObjectSetBounds(&obj->base, -SIDE, -20, SIDE, 0);
    HARM(s) = obj->base.unk68;
    SqueakStandOnFloor(obj);
    FLOOR(s) = obj->base.y;
    SqueakPlay(s, ANIM_INTRO);
}

/* What he brings in (the variant): his beam, his wave, his tornado (and the sparkles about Kirby in it), his sword's reach; all gone
 * when he's beaten; */
#define T_HARM(t) ((t)->vars[0])
#define T_KIRBY(t) (*(struct Kirby **)&(t)->vars[1])  // The Kirby in the tornado (+0x130);
#define T_LIFT(t) ((t)->vars[2])                      // The frames he's been in it;

/* Kirby held in the tornado (kirby->unk110): where its code puts him (flag 0x200; KatAM's grabbers' 0x400), let go hurt (0x0211190C:
 * 10 of his 36 HP; KatAM's hurt); */
static const struct Kirby_110 sLifted[] = { { 0, 0, -1, 0xFF, 0x57, 0x600 }, { 0, 0, 0, 1, 0, 0x600 } };
static const struct Kirby_110 sDropped[] = { { 0, 0, -1, 1, 0, 0x602 }, { 0, 0, 0, 1, 0, 0 } };

static bool32 Beaten(struct SqueakObject *t) {
    struct SqueakObject *b = (struct SqueakObject *)t->obj.base.parent;

    return b == NULL || b->obj.base.header.kind != 1 || (b->obj.base.flags & 0x1000) || b->obj.unk80 <= 0 || !SqueakIsSqueak(&b->obj)
           || b->enemy != SQUEAK_META_KNIGHT;
}

// Lifted(); The Kirby in the tornado still its own;
static bool32 Lifted(struct SqueakObject *t) {
    struct Kirby *kirby = T_KIRBY(t);

    return kirby && kirby->base.unk6C == &t->obj && kirby->unk110 == sLifted;
}

static void Drop(struct SqueakObject *t, bool32 hurt) {
    struct Kirby *kirby = T_KIRBY(t);

    if (Lifted(t)) {
        kirby->unk110 = hurt ? sDropped : sDropped + 1;
        kirby->base.counter = -1;
    }
    T_KIRBY(t) = NULL;
}

// KirbyBox(); A Kirby's hit box in the room (sub_0803E2B0's: its center's offset and half sizes, turned with his facing);
static void KirbyBox(struct ObjectBase *b, s32 *r) {
    s32 x = b->x + ((b->flags & 1) ? -(b->unk38 + b->unk3A) : b->unk38 + b->unk3A) * 0x100, y = b->y + (b->unk39 + b->unk3B) * 0x100;

    r[0] = x - b->unk3A * 0x100;
    r[2] = x + b->unk3A * 0x100;
    r[1] = y - b->unk3B * 0x100;
    r[3] = y + b->unk3B * 0x100;
}

/* Catch(); The tornado's box (ext's 26: 0x101 kind 6, 11 behind to 10 ahead, 1 to 191 up) on a Kirby free to grab: held where he
 * is (0x02111A84: his own speeds 5.0 up and 0.2 gravity, which the hold keeps from him); */
static bool32 Catch(struct SqueakObject *t) {
    s32 x = t->obj.base.x, y = t->obj.base.y, r[4], h[4];
    u8 k;

    r[0] = x + ((t->obj.base.flags & 1) ? -10 : -11) * 0x100;
    r[2] = x + ((t->obj.base.flags & 1) ? 11 : 10) * 0x100;
    r[1] = y - 191 * 0x100;
    r[3] = y - 1 * 0x100;
    for (k = 0; k < gNumKirbys; k++) {
        struct Kirby *kirby = &gKirbys[k];

        if (kirby->base.roomId != t->obj.base.roomId || !SqueakKirbyFree(kirby)) continue;
        KirbyBox(&kirby->base, h);
        if (h[2] < r[0] || h[0] > r[2] || h[3] < r[1] || h[1] > r[3]) continue;
        T_KIRBY(t) = kirby;
        T_LIFT(t) = 0;
        kirby->base.unk6C = &t->obj;
        kirby->unk110 = sLifted;
        kirby->base.flags |= 0x40000;
        sub_0804E09C(kirby);
        if (!kirby->unk110) {
            T_KIRBY(t) = NULL;
            return FALSE;
        }
        return TRUE;
    }
    return FALSE;
}

/* Lift(); 0x021119B0: Kirby lifted LIFT a frame to LIFT_TOP over the floor, sparkles about him every SPARKLE_EVERY frames; after
 * LIFT_TIME frames (at the top at once), let go hurt (0x0211190C); */
static bool32 Lift(struct SqueakObject *t) {
    struct SqueakObject *b = (struct SqueakObject *)t->obj.base.parent;
    struct Kirby *kirby = T_KIRBY(t);
    s32 limit = LIFT_TIME;
    struct SqueakObject *sp;

    if (Beaten(t) || !Lifted(t)) {
        Drop(t, FALSE);
        return FALSE;
    }
    if (kirby->base.y - LIFT > FLOOR(b) - (LIFT_TOP << 8)) kirby->base.y -= LIFT;
    else limit = 0;
    if (T_LIFT(t) % SPARKLE_EVERY == 0) {
        sp = SqueakSpawn(t, SQUEAK_META_KNIGHT_SPARKLE, THING_SPARKLE, 0, 0);
        if (sp) {
            sp->obj.base.x = kirby->base.x;
            sp->obj.base.y = kirby->base.y;
            sp->obj.base.parent = t->obj.base.parent;
        }
    }
    if (T_LIFT(t)++ > limit) Drop(t, TRUE);
    return TRUE;
}

static bool32 MetaKnightThing(struct SqueakObject *t) {
    struct SqueakObject *b = (struct SqueakObject *)t->obj.base.parent;
    struct ObjectBase *base = &t->obj.base;
    s32 x;

    switch (t->variant) {
    case THING_BEAM:            // 0x02111F90, 0x02111F34: gone after BEAM_TIME frames or at a wall;
        if (t->state == 0) {
            SqueakStop(t);
            t->vx = BEAM_SPEED;
            SqueakPlay(t, T_ANIM);
            SqueakSetState(t, 1);
        }
        SqueakMove(t);
        x = base->x - Level(t)->viewportPosition.x;
        if (T(t) > BEAM_TIME || SqueakWallAhead(t) || x < 0 || x > DISPLAY_WIDTH << 8) base->flags |= 0x1000;
        break;
    case THING_WAVE:            // 0x02111E44, 0x02111E04;
        if (t->state == 0) {
            SqueakStop(t);
            SqueakPlay(t, T_ANIM);
            SqueakSetState(t, 1);
        }
        SqueakMove(t);
        if (T(t) > WAVE_TIME) base->flags |= 0x1000;
        break;
    case THING_TORNADO:         // 0x02111CFC, 0x02111B70, 0x021119B0;
        switch (t->state) {
        case 0:
            SqueakStop(t);
            t->vx = TORNADO_SPEED;
            SqueakPlayComposite(t, T_TORNADO);
            SqueakSetState(t, 1);
            // fallthrough
        case 1:                 // 0x02111B70: on, catching Kirby (moving on: 0x021119B0); past the arena's points 9 and 10 away
                                // from him, its end;
            if (T_KIRBY(t) || (!Beaten(t) && Catch(t))) {
                Lift(t);
                SqueakMove(t);
                break;
            }
            SqueakMove(t);
            x = base->x - Level(t)->viewportPosition.x;
            if (T(t) > TORNADO_TIME || SqueakWallAhead(t) || x <= SIDE << 8 || x >= (DISPLAY_WIDTH - SIDE) << 8) {
                base->flags |= 0x1000;
                break;
            }
            if (!Beaten(t) && t->animEnded && ((base->x < Middle(t) - (POINT_OUT << 8) && Middle(t) - (POINT_OUT << 8) < b->obj.base.x)
                                               || (Middle(t) + (POINT_OUT << 8) < base->x && b->obj.base.x < Middle(t) + (POINT_OUT << 8)))) {
                SqueakStop(t);
                SqueakPlayComposite(t, T_TORNADO_END);
                SqueakSetState(t, 2);
            }
            break;
        case 2:                 // 0x02111B58;
            if (t->animEnded) base->flags |= 0x1000;
            break;
        }
        break;
    case THING_SPARKLE:         // 0x021115C4, 0x021115AC;
        if (t->state == 0) {
            SqueakStop(t);
            SqueakPlay(t, T_ANIM);
            SqueakSetState(t, 1);
        }
        if (t->animEnded) base->flags |= 0x1000;
        break;
    default:                    // His sword's reach: kept on him (Reach());
        break;
    }
    if (Beaten(t) && !(base->flags & 0x1000)) {
        if (t->variant == THING_TORNADO) Drop(t, FALSE);
        if (t->variant != THING_REACH) SqueakSpawnEffect(t, SQUEAK_FX_STARS, 0, 0, FALSE);
        base->flags |= 0x1000;
    }
    return FALSE;
}

/* ThingBoxes(); A thing's box (metaknight_ext.rca's: the beam's 13 behind to 12 ahead, 7 under to 37 over; the wave's 39 behind to
 * 49 ahead, 1 under to 19 over), hurting; the tornado's and the sparkles' none (the tornado catches: Catch()); */
static void ThingBoxes(struct SqueakObject *t) {
    struct ObjectBase *base = &t->obj.base;

    switch (t->variant) {
    case THING_BEAM:
        sub_0803E2B0(base, -13, -37, 12, 7);
        base->unk68 = T_HARM(t);
        break;
    case THING_WAVE:
        sub_0803E2B0(base, -39, -19, 49, 1);
        base->unk68 = T_HARM(t);
        break;
    case THING_TORNADO: case THING_SPARKLE:
        sub_0803E2B0(base, -1, -1, 1, 1);
        base->unk68 = 0;
        break;
    }
}

static void MetaKnightThingMain(struct Object *obj) {
    struct SqueakObject *t = (struct SqueakObject *)obj;

    obj->kirby3 = FindTargetKirby(&obj->base);
    SqueakRun(t, MetaKnightThing);
    SqueakFinishFrame(t);
    if (t->variant != THING_REACH) SqueakAnimate(t);
    ThingBoxes(t);
}

void SqueakMetaKnightThingStart(struct Object *obj) {
    struct SqueakObject *t = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, -1, MetaKnightThingMain);
    obj->kirbyAbility = 0;
    if (t->started) return;
    SqueakSetState(t, 0);
}

/* SqueakMetaKnightThingInit(); Its box and bounds; free of the stage's hold (flag 0x20000); no attack hurts it (0x021120B8: KatAM's
 * level 6), not inhaled, Kirby's body no harm to it; the beam and the tornado on the terrain (its walls end them), the others
 * through it; his sword's reach drawn by nothing; */
void SqueakMetaKnightThingInit(struct SqueakObject *t) {
    struct ObjectBase *base = &t->obj.base;

    T_HARM(t) = base->unk68;
    T_KIRBY(t) = NULL;
    base->flags |= 0x1000000 | 0x2000000;
    base->unk5C = (base->unk5C & ~7) | 6 | 0x20 | 0x80;
    ThingBoxes(t);
    switch (t->variant) {
    case THING_BEAM: case THING_TORNADO:
        ObjectSetBounds(base, -8, -16, 8, 0);
        break;
    case THING_REACH:
        ObjectSetBounds(base, -8, -8, 8, 8);
        base->flags |= 0x100 | 0x400;
        sub_0803E2B0(base, -1, -1, 1, 1);
        base->unk68 = 0;
        break;
    default:
        ObjectSetBounds(base, -8, -8, 8, 8);
        base->flags |= 0x100;
        break;
    }
}
