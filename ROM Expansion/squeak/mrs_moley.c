/* Mrs. Moley (Squeak Squad's); ported from Kirby: Squeak Squad (USA), boss 0x3A (her body 0x0212CF40, vtable 0x02183F60: +0x34 per
 * frame 0x0212D198, +0x30 her hit 0x0212C73C, +0x2C her touch 0x0212CD2C; her states 0x0212A9C8-0x0212C68C), as SQUEAK_MRS_MOLEY, a boss
 * on the mid-bosses' framework (mini-boss type 0x40: the camera held on her area, the HP bar, the music, KatAM's defeat). Values from
 * param/PrmBB04Ml0.kgb (her HP 300, PrmBB04Sl0: here KatAM's bosses' 60 for one player, her crash's 20 and her last stand's 20 alike
 * scaled, to 4); speeds 20.12 a frame, Y up; drawn with enemy/mrsMoley_cmn, _ext00 and _ext01 (the sprites her body switches to,
 * 0x02022F00's slots 0-2), both .mca's composites (her dive's sparkles, the things in her hand); her boxes from their .rca's. Recorded
 * on the DS in her own stage (map/a2s7s2: the holes and zones below; her throws, hides, dive and plunge: the states' frames).
 * • Her arena (a2s7s2's points and zones, 288 pixels across; here KatAM's camera view, 240, which the fight holds, the distances from
 *   its sides kept; the front holes 8 pixels over the floor where she was placed, as a2s7s2's): two front holes 40 pixels in from
 *   either side, two back holes 12 pixels higher, 72 from the left and 56 from the right, and the middle's 16 higher, 8 right of the
 *   middle. Her front self is big, the only one that can be hit; her back self small, out of reach (the stage's background), as
 *   Squeak Squad's.
 * • She pops up (0x0212BE3C, 0x0212BDF0: 78 frames, grit flying out of a front hole: 0x0212A6FC), looks about (0x0212BD84-0x0212BC9C:
 *   11 and 27 frames) and after 20 frames more (0x0212BB30) picks: 70 in 100 an attack, else (and in the back) down she goes.
 *   In front, her attack is 2-4 throws (0x0212ADB4-0x0212A9C8: 28 frames each, the thing let go 68 pixels up 6 frames into her
 *   swing: a rock 15, a stone 15, a pebble 15, a bolt 13, a ring 12 (KatAM's Wheel, as Squeak Squad's), a bomb 8 (Bomb), a can 7
 *   (Fire) or a saw 15 (Cutter) in 100); in the back, her dive.
 * • Down (0x0212BABC-0x0212B924: 13 and 6 frames), she waits under 40 frames (30 after going down by choice: 0x0212B88C) and comes up
 *   (0x0212B6C8) by Kirby's side of the arena: Kirby left, the right front hole 70 in 100, else the right back one, facing him; Kirby
 *   right, the left ones alike; Kirby in the middle, the left front one 35, the left back one 15, the right front one 35, else the
 *   right back one.
 * • Her dive (0x0212B594-0x0212B408: sparkles, down, 10 frames, out): from the back she leaps (5.0 up, petals falling from her every
 *   30 frames: 0x0212B2BC) out of sight (32 pixels below her, 184 over the front holes: a2s7s2's zone 6); her shadow follows Kirby on
 *   the ground (0x02129C30, 0x02129B74: 120 frames) and she over it 140 frames (0x0212B1EC: Kirby by a side, halfway to its front
 *   hole each frame), then plunges (0x0212C210, 0x0212BFB8: upside down, 0.2 gravity). Into a front hole (Kirby by a side) she sinks
 *   (0x0212BF58-0x0212BEB0: 53 frames, grit flying); into the middle she's stuck head first (0x0212B16C: the screen shaking, 20 of her
 *   HP lost), struggles 40 and 160 frames (0x0212B11C, 0x0212B0C0, 0x0212AF18: hit all the while) and hops (1.2 up, 40 frames) into
 *   the front hole ahead of her, turned away from Kirby (0x0212AE04), and sinks.
 * • Her last 20 HP (0x0212C73C: Squeak Squad's 20 of 300), she comes up in the middle back hole (0x0212B630) and always dives.
 * • Hit in front (0x0212C73C): 20 frames safe (KatAM's hits then undone), two petals knocked off her (0x0212A624's kinds 1 and 2).
 * Her sounds: Squeak Squad's (SqueakSfx). Not ported: Squeak Squad's hit flash (enemy/morly_dmg's palettes: KatAM's) and shakes (KatAM's), her own defeat
 * (0x0212C594-0x0212C278: KatAM's mini-boss defeat), her boss music. */
#include "../squeak.h"
#include "kirby.h"
#include "constants/kirby.h"

void RequestScreenShake(u16, struct ObjectBase *);
void sub_08086C48(struct Object *);    // KatAM's defeat of an object (its drop);
void sub_0809D060(struct Object *);    // KatAM's mini-boss defeat (object.c, static);

/* Animations (OnlyAnims: mrsMoley_cmn's 15, 3-13; _ext00's 1-8, 13-16; _ext01's 1-3, the things in her hand); her small (back) self's
 * five, then the big (front) one's alike: BIG on; */
enum {
    ANIM_DIZZY, ANIM_S_POP, ANIM_S_LOOK, ANIM_S_LOOK2, ANIM_S_DOWN, ANIM_S_GONE, ANIM_POP, ANIM_LOOK, ANIM_LOOK2, ANIM_DOWN, ANIM_GONE,
    ANIM_STUCK, ANIM_DIVE_POSE, ANIM_SPARKLE, ANIM_SPARKLE2, ANIM_JUMP_OUT, ANIM_RISING, ANIM_PLUNGE, ANIM_FLAIL, ANIM_BURST, ANIM_CRAWL,
    ANIM_HOP, ANIM_HOLED, ANIM_SINK, ANIM_WIND_UP, ANIM_THROW, ANIM_FOLLOW,
};
#define BIG (ANIM_POP - ANIM_S_POP)
// Her composite animations (_ext00.mca's: the dive's sparkles from the back, from the middle, her landing; _ext01.mca's throws);
enum { COMPOSITE_BACK_DIVE, COMPOSITE_MIDDLE_DIVE, COMPOSITE_LAND, COMPOSITE_THROW };

// PrmBB04Ml0.kgb;
#define HIDE_TIME 40                    // +0x00, +0x04: under a hole;
#define HIDE_TIME_CHOSEN 30
#define ATTACK 70                       // +0x08: her pick's attack;
#define PICK_WAIT 20                    // +0x0C;
#define KIRBY_LEFT_FRONT 70             // +0x10..+0x24: the hole she comes up in;
#define KIRBY_RIGHT_FRONT 70
#define MIDDLE_LEFT_FRONT 35
#define MIDDLE_RIGHT_FRONT 35
#define MIDDLE_LEFT_BACK 15
#define SAFE_TIME 20                    // +0x28;
#define DIVE_PAUSE 10                   // +0x2C;
#define LEAP_VY 0x5000                  // +0x30;
#define TRACK_TIME 140                  // +0x34;
#define PLUNGE_GRAVITY 0x334            // +0x38;
#define CRASH_DAMAGE 4                  // +0x48: Squeak Squad's 20 of her 300 HP;
#define LAST_STAND 4                    // +0x4C: Squeak Squad's 20 of 300;
#define STRUGGLE_TIME 160               // +0x50;
#define HOP_VY 0x1334                   // +0x58, +0x5C;
#define HOP_GRAVITY 0xF6
#define THROWS 3                        // +0x13C, +0x140: 2 throws and 0-2 more;

#define INTRO_TIME 30                   // 0x0212C608;
#define CRASH_WAIT 40                   // 0x0212B11C;
#define HOP_TIME 40                     // 0x0212AF18: its hop's frames (the distance over 40);
#define HOP_CHECK 30                    // 0x0212AE04;
#define HOP_NEAR 0x10000                // 0x0212AF18: a hop under 16 pixels falls at half the gravity, under 32 at 2/3;
#define HOP_MID 0x20000
#define HOLE_ABOVE 8                    // 0x0212AE04: within 8 pixels over the hole, she stops falling;
#define THROW_AT 5                      // 0x0212AA44: her swing's frame the thing is let go;
#define THROW_X 0                       // (68 pixels up);
#define THROW_Y 0x44000
#define SHADOW_Y 8                      // 0x0212B2BC: her shadow 8 pixels under the front holes' level (on the ground);
#define HOLE_UP 8                       // a2s7s2's front holes 8 pixels over its ground (Kirby stands at 64, they're at 72, Y up);
#define PETAL_Y 40                      // 0x0212C73C: the petals a hit knocks off, 40 pixels up;
#define SHAKE_CRASH 3                   // KatAM's timed screen shakes (1-3: 14, 18, 24 frames; 4 on loop till stopped): Squeak Squad's
#define SHAKE_HOLE 2                    // 4 for 60 frames as KatAM's longest, its 3 for 35 as its 18 frames;

// Her arena (a2s7s2: from the room's sides, in pixels, as its points 0-4; y up from the front holes' level);
#define FRONT_X 40
#define BACK_LEFT_X 72
#define BACK_RIGHT_X 56
#define MIDDLE_X 8
#define BACK_Y 12
#define MIDDLE_Y 16
#define MIDDLE_DROP 2                   // 0x0212B630: 2 pixels under the middle hole;
#define INTRO_UP 1                      // 0x0212C68C, 0x0212C608;
#define SIDE_ZONE 80                    // a2s7s2's zones 0 and 1: 80 pixels from either side (5, the middle, between);
#define SKY 184                         // Its zone 6: over 184 pixels above the front holes;
#define SKY_PROBE 32                    // 0x0212B2BC: 32 pixels under her;

enum { POINT_FRONT_LEFT, POINT_FRONT_RIGHT, POINT_MIDDLE, POINT_BACK_RIGHT, POINT_BACK_LEFT };
enum { MODE_MIDDLE, MODE_BACK, MODE_FRONT };     // Squeak Squad's +0x1FC: her middle back hole (her last stand's), a back one, a front one;
enum { PHASE_ON, PHASE_CHOSE, PHASE_LAST };      // +0x1F8: on as ever, down by choice, her last stand;
enum { ZONE_LEFT, ZONE_RIGHT, ZONE_MIDDLE };     // a2s7s2's zones 0 and 4, 1 and 3, the others;

// What she throws (SQUEAK_MOLEY_THING's variants: Squeak Squad's kinds), her dirt (SQUEAK_MOLEY_DIRT's: kind | way << 2);
enum { THING_ROCK, THING_STONE, THING_PEBBLE, THING_BOLT, THING_RING, THING_BOMB, THING_CAN, THING_SAW, THING_GRIT, THING_GRIT2 };
enum { DIRT_BURST, DIRT_PETAL, DIRT_PETAL2, DIRT_SHADOW };

enum {
    MOLEY_START, MOLEY_INTRO, MOLEY_POP, MOLEY_POP_WAIT, MOLEY_LOOK, MOLEY_LOOK_WAIT, MOLEY_LOOK_END, MOLEY_PICK, MOLEY_THROW,
    MOLEY_THROW_WIND, MOLEY_THROW_PICK, MOLEY_THROW_HOLD, MOLEY_THROW_END, MOLEY_DOWN, MOLEY_DOWN_WAIT, MOLEY_GONE, MOLEY_HIDE,
    MOLEY_UNDER, MOLEY_EMERGE, MOLEY_MIDDLE, MOLEY_DIVE, MOLEY_DIVE_DOWN, MOLEY_DIVE_GONE, MOLEY_DIVE_WAIT, MOLEY_LEAP, MOLEY_RISE,
    MOLEY_TRACK, MOLEY_PLUNGE, MOLEY_FALL, MOLEY_CRASH, MOLEY_CRASH_WAIT, MOLEY_CRASH_UP, MOLEY_STRUGGLE, MOLEY_HOP, MOLEY_HOLE,
    MOLEY_SINK, MOLEY_SINK_END,
};

#define MODE(s) ((s)->vars[0])
#define PHASE(s) ((s)->vars[1])
#define COUNT(s) ((s)->vars[2])         // Squeak Squad's +0x204;
#define THROWS_LEFT(s) ((s)->vars[3])   // +0x208;
#define ITEM(s) ((s)->vars[4])          // +0x20C: what she throws (her pick's order: 0x0212AB90);
#define LANDED(s) ((s)->vars[5])        // +0x214: her plunge landed (her shadow goes);
#define SAFE(s) ((s)->vars[6])          // +0x200: frames left safe after a hit;
#define LAST_HP(s) ((s)->vars[7])
#define HARM(s) ((s)->vars[8])
#define FLOOR(s) ((s)->vars[9])         // The front holes' level (HOLE_UP over the floor she was placed on);
#define TIMER(s) ((s)->vars[10])        // +0xB4;
#define BOX(s) (((u8 *)&(s)->vars[11])[0])
#define AWAY(s) (((u8 *)&(s)->vars[11])[1])     // Squeak Squad's flag 1: down a hole, out of reach;
#define ARENA(s) ((s)->vars[12])        // The arena's width: her room's (a2s7s2's 288 pixels), taken at her fight's start;

// The thing's and dirt's own;
#define T_LIFE(s) ((s)->vars[0])        // +0x138: its frames left;
#define T_BOUNCES(s) ((s)->vars[1])     // +0x130;
#define T_HARM(s) ((s)->vars[2])
#define T_BOX(s) (((u8 *)&(s)->vars[3])[0])
#define T_FREE(s) (((u8 *)&(s)->vars[3])[1])    // A pebble through the floor;
#define D_LIFE(s) ((s)->vars[0])        // +0x134;
#define D_TIMER(s) ((s)->vars[1])       // +0xB4;

// boxes_multi.py: mrsMoley_cmn's .rca, _ext00's, _ext01's, in OnlyAnims' order;
static const struct SqueakBox sMoleyBoxes[] = {
    { { -13, -25, 9, -4 }, FALSE, 0 },
    { { -26, -34, 13, -2 }, TRUE, 0 },
    { { -26, -37, 13, -2 }, TRUE, 0 },
    { { -26, -41, 13, -2 }, TRUE, 0 },
    { { -26, -42, 13, -2 }, TRUE, 0 },
    { { -26, -38, 13, -2 }, TRUE, 0 },
    { { -26, -18, 13, -2 }, TRUE, 0 },
    { { -26, -62, 24, -2 }, TRUE, 0 },
    { { -24, -79, 13, -2 }, TRUE, 0 },
    { { -57, -38, 24, -2 }, TRUE, 0 },
    { { -50, -51, 24, -12 }, TRUE, 0 },
    { { -18, -42, 25, 1 }, TRUE, 0 },
    { { -16, -42, 21, 1 }, FALSE, 0 },
    { { -25, -44, 13, 1 }, TRUE, 0 },
};
static const struct SqueakBoxFrame sMoleyAnim0[] = { { 1, 0 } };
static const struct SqueakBoxFrame sMoleyAnim1[] = { { 47, 0 } };
static const struct SqueakBoxFrame sMoleyAnim2[] = { { 11, 0 } };
static const struct SqueakBoxFrame sMoleyAnim3[] = { { 20, 0 } };
static const struct SqueakBoxFrame sMoleyAnim4[] = { { 13, 0 } };
static const struct SqueakBoxFrame sMoleyAnim5[] = { { 6, 0 } };
static const struct SqueakBoxFrame sMoleyAnim6[] = { { 47, 0 } };
static const struct SqueakBoxFrame sMoleyAnim7[] = { { 1, 0 } };
static const struct SqueakBoxFrame sMoleyAnim8[] = { { 3, 1 }, { 1, 2 }, { 2, 3 }, { 2, 4 }, { 13, 2 } };
static const struct SqueakBoxFrame sMoleyAnim9[] = { { 5, 5 }, { 3, 1 }, { 5, 6 } };
static const struct SqueakBoxFrame sMoleyAnim10[] = { { 6, 0 } };
static const struct SqueakBoxFrame sMoleyAnim11[] = { { 1, 7 } };
static const struct SqueakBoxFrame sMoleyAnim12[] = { { 7, 0 } };
static const struct SqueakBoxFrame sMoleyAnim15[] = { { 9, 0 } };
static const struct SqueakBoxFrame sMoleyAnim16[] = { { 1, 0 } };
static const struct SqueakBoxFrame sMoleyAnim17[] = { { 1, 8 } };
static const struct SqueakBoxFrame sMoleyAnim18[] = { { 4, 7 } };
static const struct SqueakBoxFrame sMoleyAnim20[] = { { 4, 7 }, { 9, 9 } };
static const struct SqueakBoxFrame sMoleyAnim21[] = { { 1, 10 } };
static const struct SqueakBoxFrame sMoleyAnim22[] = { { 1, 11 } };
static const struct SqueakBoxFrame sMoleyAnim23[] = { { 28, 11 }, { 4, 11 }, { 4, 11 }, { 4, 11 }, { 4, 11 }, { 8, 12 } };
static const struct SqueakBoxFrame sMoleyAnim24[] = { { 10, 13 } };
static const struct SqueakBoxFrame sMoleyAnim25[] = { { 8, 13 } };
static const struct SqueakBoxFrame sMoleyAnim26[] = { { 8, 13 } };
static const struct SqueakBoxAnim sMoleyBoxAnims[] = {
    { sMoleyAnim0, 1 }, { sMoleyAnim1, 1 }, { sMoleyAnim2, 1 }, { sMoleyAnim3, 1 }, { sMoleyAnim4, 1 }, { sMoleyAnim5, 1 },
    { sMoleyAnim6, 1 }, { sMoleyAnim7, 1 }, { sMoleyAnim8, 5 }, { sMoleyAnim9, 3 }, { sMoleyAnim10, 1 }, { sMoleyAnim11, 1 },
    { sMoleyAnim12, 1 }, { NULL, 0 }, { NULL, 0 }, { sMoleyAnim15, 1 }, { sMoleyAnim16, 1 }, { sMoleyAnim17, 1 },
    { sMoleyAnim18, 1 }, { NULL, 0 }, { sMoleyAnim20, 2 }, { sMoleyAnim21, 1 }, { sMoleyAnim22, 1 }, { sMoleyAnim23, 6 },
    { sMoleyAnim24, 1 }, { sMoleyAnim25, 1 }, { sMoleyAnim26, 1 },
};
static const struct SqueakBoxes sMoleyBoxData = { sMoleyBoxes, sMoleyBoxAnims, ARRAY_COUNT(sMoleyBoxAnims) };

// boxes_multi.py: mrsMoley_ext01's .rca, in the thing's OnlyAnims' order (the pebbles have none);
static const struct SqueakBox sThingBoxes[] = {
    { { -8, -9, 9, 9 }, TRUE, 0 },
    { { -5, -5, 6, 6 }, TRUE, 0 },
    { { -2, -2, 2, 2 }, TRUE, 0 },
    { { -5, -6, 6, 5 }, TRUE, 0 },
    { { -6, -6, 6, 7 }, TRUE, 0 },
};
static const struct SqueakBoxFrame sThingAnim0[] = { { 40, 0 } };
static const struct SqueakBoxFrame sThingAnim1[] = { { 40, 1 } };
static const struct SqueakBoxFrame sThingAnim2[] = { { 20, 2 } };
static const struct SqueakBoxFrame sThingAnim3[] = { { 40, 3 } };
static const struct SqueakBoxFrame sThingAnim4[] = { { 24, 3 } };
static const struct SqueakBoxFrame sThingAnim5[] = { { 12, 3 } };
static const struct SqueakBoxFrame sThingAnim6[] = { { 40, 4 } };
static const struct SqueakBoxFrame sThingAnim7[] = { { 8, 3 } };
static const struct SqueakBoxFrame sThingAnim10[] = { { 8, 3 } };
static const struct SqueakBoxAnim sThingBoxAnims[] = {
    { sThingAnim0, 1 }, { sThingAnim1, 1 }, { sThingAnim2, 1 }, { sThingAnim3, 1 }, { sThingAnim4, 1 }, { sThingAnim5, 1 },
    { sThingAnim6, 1 }, { sThingAnim7, 1 }, { NULL, 0 }, { NULL, 0 }, { sThingAnim10, 1 },
};
static const struct SqueakBoxes sThingBoxData = { sThingBoxes, sThingBoxAnims, ARRAY_COUNT(sThingBoxAnims) };

static u32 Random(u32 n) {
    return Rand16() * n >> 16;
}

static bool32 FacesLeft(struct SqueakObject *s) {
    return (s->obj.base.flags & 1) != 0;
}

static void Face(struct SqueakObject *s, bool32 left) {
    s->obj.base.flags = (s->obj.base.flags & ~1) | (left ? 1 : 0);
}

/* ArenaLeft(), ArenaWidth(); The arena: her room, Squeak Squad's stage (288 pixels across, its holes drawn at its points). It was
 * the camera's view at her fight's start, 240 across, so she came up away from the holes drawn (the owner: "Mrs. Moley is off the
 * ground and off centered": 25 pixels right of the left one, 23 left of the right one, the camera 25 in); */
static s32 ArenaLeft(struct SqueakObject *s) {
    return 0;
}

static s32 ArenaWidth(struct SqueakObject *s) {
    return ARENA(s);
}

// Point(); 0x02209CC0: a2s7s2's points 0-4, the holes (40, 248, 152, 232 and 72 across; KatAM's 24.8);
static void Point(struct SqueakObject *s, u8 n, s32 *x, s32 *y) {
    s32 w = ArenaWidth(s);
    s32 across[] = { [POINT_FRONT_LEFT] = FRONT_X, [POINT_FRONT_RIGHT] = w - FRONT_X, [POINT_MIDDLE] = w / 2 + MIDDLE_X,
                     [POINT_BACK_RIGHT] = w - BACK_RIGHT_X, [POINT_BACK_LEFT] = BACK_LEFT_X };
    s32 up[] = { [POINT_FRONT_LEFT] = 0, [POINT_FRONT_RIGHT] = 0, [POINT_MIDDLE] = MIDDLE_Y, [POINT_BACK_RIGHT] = BACK_Y,
                 [POINT_BACK_LEFT] = BACK_Y };

    *x = ArenaLeft(s) + (across[n] << 8);
    *y = FLOOR(s) - (up[n] << 8);
}

// Sky(); a2s7s2's zone 6: over SKY pixels above the front holes;
static bool32 Sky(struct SqueakObject *s, s32 y) {
    return FLOOR(s) - y > SKY << 8;
}

// Zone(); 0x02209C7C: a2s7s2's zone at a spot, left, right or the middle (its sky as the middle: Squeak Squad's default cases);
static u8 Zone(struct SqueakObject *s, s32 x, s32 y) {
    x -= ArenaLeft(s);
    if (Sky(s, y)) return ZONE_MIDDLE;
    if (x < SIDE_ZONE << 8) return ZONE_LEFT;
    if (x >= (ArenaWidth(s) - SIDE_ZONE) << 8) return ZONE_RIGHT;
    return ZONE_MIDDLE;
}

static u8 KirbyZone(struct SqueakObject *s) {
    return Zone(s, s->obj.kirby3->base.x, s->obj.kirby3->base.y);
}

// Ahead(); 0x0201AE58: how far a spot is ahead of her (behind: under 0), Squeak Squad's units;
static s32 Ahead(struct SqueakObject *s, s32 x) {
    s32 dx = (x - s->obj.base.x) << 4;

    return FacesLeft(s) ? -dx : dx;
}

/* MoveFree(); Her moves (Squeak Squad's flag 0x20000: not held in the stage, nor in its terrain: she leaps out of it and plunges into
 * the holes); */
static void MoveFree(struct SqueakObject *s) {
    struct ObjectBase *base = &s->obj.base;

    s->vx += s->ax;
    s->vy += s->ay;
    base->xspeed = SQUEAK_SPEED(FacesLeft(s) ? -s->vx : s->vx);
    base->yspeed = SQUEAK_SPEED(s->vy);
    base->unk48 = base->x;
    base->unk4C = base->y;
    base->x += base->xspeed;
    base->y -= base->yspeed;
    base->unk62 = 0;
    s->terrain = 0;
    s->moved = TRUE;
}

// Body(); Her small (back) self's animation, or the big one's in front;
static void Body(struct SqueakObject *s, u8 anim) {
    SqueakPlay(s, MODE(s) == MODE_FRONT ? anim + BIG : anim);
}

/* SpawnAt(); One of her things or dirt at a spot (KatAM's 24.8), facing her way; Squeak Squad's offsets from her are its spot's, not
 * turned with her (0x02129958's kinds 8-10, 0x0212A624's 1-3); */
static struct SqueakObject *SpawnAt(struct SqueakObject *s, u8 enemy, u8 variant, s32 x, s32 y) {
    s32 dx = (x - s->obj.base.x) << 4, dy = (s->obj.base.y - y) << 4;

    return SqueakSpawn(s, enemy, variant, FacesLeft(s) ? -dx : dx, dy);
}

// Dirt(); Her dirt at a spot (its parent her: her shadow follows her down);
static void Dirt(struct SqueakObject *s, u8 kind, u8 way, s32 x, s32 y) {
    struct SqueakObject *d = SpawnAt(s, SQUEAK_MOLEY_DIRT, kind | way << 2, x, y);

    if (d) d->obj.base.parent = &s->obj;
}

/* Grit(); 0x0212A6FC: grit flying out of a front hole while she comes up or sinks (her state's frame, every 60: 2, 26, 50 a grain
 * 16 right and up, way 1; 8, 32, 56 one 8 left, 24 up, way 4; 14, 38 one 8 right, 24 up, way 3; 20, 44 one 16 left, 16 up, way 4); */
static void Grit(struct SqueakObject *s) {
    s32 x = s->obj.base.x, y = s->obj.base.y;
    struct SqueakObject *g;
    u8 kind, way;
    s8 dx, dy;

    if (MODE(s) != MODE_FRONT) return;
    switch (s->stateTimer % 60) {
    case 2: case 26: case 50: kind = THING_GRIT, dx = 16, dy = 16, way = 1; break;
    case 8: case 32: case 56: kind = THING_GRIT2, dx = -8, dy = 24, way = 4; break;
    case 14: case 38: kind = THING_GRIT2, dx = 8, dy = 24, way = 3; break;
    case 20: case 44: kind = THING_GRIT, dx = -16, dy = 16, way = 4; break;
    default: return;
    }
    g = SpawnAt(s, SQUEAK_MOLEY_THING, kind, x + (dx << 8), y - (dy << 8));
    if (g) g->vars[4] = way;
}

/* Throw(); 0x02129958: the thing her pick chose (ITEM: her pick's order, 0x0212AA44's kinds), 68 pixels up, her way; a bolt flies
 * 380 frames, the others 240 (the thing's own); */
static void Throw(struct SqueakObject *s) {
    static const u8 sKinds[] = { THING_ROCK, THING_BOLT, THING_RING, THING_BOMB, THING_STONE, THING_PEBBLE, THING_CAN, THING_SAW };

    SqueakSpawn(s, SQUEAK_MOLEY_THING, sKinds[ITEM(s)], THROW_X, THROW_Y);
}

/* PickThing(); 0x0212AB90: what she throws (a roll of 0-99 against PrmBB04Ml0's +0x144..+0x15C: 15, 15, 15, 13, 12, 8, 7, the rest)
 * and her swing with it in her hand (_ext01.mca's composites; the pebble's, none: her bare swing); */
static void PickThing(struct SqueakObject *s) {
    static const u8 sWeights[] = { 15, 15, 15, 13, 12, 8, 7 };
    static const u8 sOrder[] = { 0, 4, 5, 1, 2, 3, 6 };     // (the weights' items, in their order);
    static const s8 sComposite[] = { 0, 1, 2, 3, 4, -1, 5, 6 };
    s32 r = Random(100);
    u8 i;

    ITEM(s) = 7;
    for (i = 0; i < ARRAY_COUNT(sWeights); i++) {
        if ((r -= sWeights[i]) < 0) {
            ITEM(s) = sOrder[i];
            break;
        }
    }
    if (sComposite[ITEM(s)] < 0) SqueakPlay(s, ANIM_THROW);
    else SqueakPlayComposite(s, COMPOSITE_THROW + sComposite[ITEM(s)]);
}

/* Petals(); 0x0212C73C: two petals knocked off her by a hit, 40 pixels up (a roll of 0-2: two of kind 1, one of each, or two of
 * kind 2; ways 1-3 facing right, 4-6 left); */
static void Petals(struct SqueakObject *s) {
    static const s8 sAt[3][2][2] = { { { 2, 4 }, { 6, 4 } }, { { 0, 6 }, { 4, 4 } }, { { 3, -2 }, { 9, -10 } } };
    static const u8 sKind[3][2] = { { DIRT_PETAL, DIRT_PETAL }, { DIRT_PETAL, DIRT_PETAL2 }, { DIRT_PETAL2, DIRT_PETAL2 } };
    static const u8 sWay[3][2] = { { 1, 2 }, { 3, 1 }, { 2, 3 } };
    u8 r = Random(3), i;

    for (i = 0; i < 2; i++)
        Dirt(s, sKind[r][i], sWay[r][i] + (FacesLeft(s) ? 3 : 0), s->obj.base.x + (sAt[r][i][0] << 8),
             s->obj.base.y - ((PETAL_Y + sAt[r][i][1]) << 8));
}

/* Crash(); 0x0212BFB8: her plunge into the middle hurts her (PrmBB04Ml0's +0x48), KatAM's mini-boss defeat if it's her last (as the
 * Meta-Knights' code does: sub_08086C48, sub_0809D060); */
static void Crash(struct SqueakObject *s) {
    struct Object *obj = &s->obj;

    obj->unk80 -= CRASH_DAMAGE;
    LAST_HP(s) = obj->unk80;
    sub_08034E14(obj);
    if (obj->unk80 > 0) return;
    obj->unk80 = 0;
    obj->base.flags &= ~(0x400 | 0x200);
    sub_08086C48(obj);
    sub_0809D060(obj);
}

static bool32 Moley(struct SqueakObject *s) {
    s32 x, y, d;
    u32 r;

    switch (s->state) {
    case MOLEY_START:       // 0x0212C68C: at the right front hole, a pixel up, facing left;
        ARENA(s) = gCurLevelInfo[s->obj.base.unk56].roomWidth;
        SqueakStop(s);
        AWAY(s) = TRUE;
        TIMER(s) = 0;
        Point(s, POINT_FRONT_RIGHT, &s->obj.base.x, &s->obj.base.y);
        s->obj.base.y -= INTRO_UP << 8;
        Face(s, TRUE);
        MODE(s) = MODE_FRONT;
        SqueakSetState(s, MOLEY_INTRO);
        break;
    case MOLEY_INTRO:       // 0x0212C608;
        if (++TIMER(s) > INTRO_TIME) {
            SqueakStop(s);
            TIMER(s) = 0;
            AWAY(s) = FALSE;
            s->obj.base.flags &= ~0x400;
            s->obj.base.y += INTRO_UP << 8;
            SqueakSetState(s, MOLEY_POP);
            return TRUE;
        }
        MoveFree(s);
        break;
    case MOLEY_POP:         // 0x0212BE3C;
        Body(s, ANIM_S_POP);
        SqueakStop(s);
        TIMER(s) = 0;
        SqueakSetState(s, MOLEY_POP_WAIT);
        return TRUE;
    case MOLEY_POP_WAIT:    // 0x0212BDF0;
        MoveFree(s);
        if (s->animEnded) SqueakSetState(s, MOLEY_LOOK);
        else Grit(s);
        break;
    case MOLEY_LOOK:        // 0x0212BD84;
        Body(s, ANIM_S_LOOK);
        SqueakStop(s);
        SqueakSetState(s, MOLEY_LOOK_WAIT);
        return TRUE;
    case MOLEY_LOOK_WAIT:   // 0x0212BCF4;
        MoveFree(s);
        if (s->animEnded) {
            Body(s, ANIM_S_LOOK2);
            SqueakSfx(s, 0x191);
            SqueakSetState(s, MOLEY_LOOK_END);
        }
        break;
    case MOLEY_LOOK_END:    // 0x0212BC9C;
        MoveFree(s);
        if (s->animEnded) {
            COUNT(s) = PICK_WAIT;
            SqueakSetState(s, MOLEY_PICK);
        }
        break;
    case MOLEY_PICK:        // 0x0212BB30: PICK_WAIT frames more, then a roll (her last stand: 0, an attack);
        MoveFree(s);
        if (--COUNT(s) >= 0) break;
        r = PHASE(s) == PHASE_LAST ? 0 : Random(100);
        if (r >= ATTACK) {
            PHASE(s) = PHASE_CHOSE;
            SqueakSetState(s, MOLEY_DOWN);
        } else if (MODE(s) == MODE_FRONT) {
            THROWS_LEFT(s) = Random(THROWS);
            SqueakSetState(s, MOLEY_THROW);
        } else {
            SqueakSetState(s, MOLEY_DIVE);
        }
        break;
    case MOLEY_THROW:       // 0x0212ADB4;
        SqueakPlay(s, ANIM_WIND_UP);
        SqueakStop(s);
        TIMER(s) = 0;
        SqueakSetState(s, MOLEY_THROW_WIND);
        return TRUE;
    case MOLEY_THROW_WIND:  // 0x0212AD64;
        MoveFree(s);
        if (s->animEnded) {
            SqueakSfx(s, 0x198);
            SqueakSetState(s, MOLEY_THROW_PICK);
        }
        break;
    case MOLEY_THROW_PICK:  // 0x0212AB90;
        MoveFree(s);
        PickThing(s);
        SqueakSetState(s, MOLEY_THROW_HOLD);
        break;
    case MOLEY_THROW_HOLD:  // 0x0212AA44: let go on its THROW_AT frame;
        MoveFree(s);
        if (s->animEnded) {
            SqueakPlay(s, ANIM_FOLLOW);
            SqueakSetState(s, MOLEY_THROW_END);
        }
        if (s->stateTimer == THROW_AT) Throw(s);
        break;
    case MOLEY_THROW_END:   // 0x0212A9C8: 2-4 throws in all;
        MoveFree(s);
        if (s->animEnded) {
            SqueakSetState(s, THROWS_LEFT(s) < 0 ? MOLEY_DOWN : MOLEY_THROW);
            THROWS_LEFT(s)--;
        }
        break;
    case MOLEY_DOWN:        // 0x0212BABC;
        Body(s, ANIM_S_DOWN);
        SqueakStop(s);
        TIMER(s) = 0;
        SqueakSetState(s, MOLEY_DOWN_WAIT);
        return TRUE;
    case MOLEY_DOWN_WAIT:   // 0x0212BA24;
        MoveFree(s);
        if (!s->animEnded) break;
        Body(s, ANIM_S_GONE);
        SqueakSfx(s, 0x194);
        SqueakStop(s);
        TIMER(s) = 0;
        SqueakSetState(s, MOLEY_GONE);
        return TRUE;
    case MOLEY_GONE:        // 0x0212B9D8;
        MoveFree(s);
        if (s->animEnded) {
            SqueakSetState(s, MOLEY_HIDE);
            return TRUE;
        }
        break;
    case MOLEY_HIDE:        // 0x0212B924: under the hole, out of reach;
        if (s->stateTimer % 8 == 1) SqueakSfx(s, 0x194);
        if (s->animEnded) {
            AWAY(s) = TRUE;
            if (PHASE(s) == PHASE_CHOSE) {
                PHASE(s) = PHASE_ON;
                COUNT(s) = HIDE_TIME_CHOSEN;
            } else {
                COUNT(s) = HIDE_TIME;
            }
            SqueakSetState(s, MOLEY_UNDER);
        }
        MoveFree(s);
        break;
    case MOLEY_UNDER:       // 0x0212B88C: (its digging, every 8 frames from its second);
        if (s->stateTimer % 8 == 1) SqueakSfx(s, 0x194);
        if (--COUNT(s) < 0) SqueakSetState(s, PHASE(s) == PHASE_LAST ? MOLEY_MIDDLE : MOLEY_EMERGE);
        MoveFree(s);
        break;
    case MOLEY_EMERGE:      // 0x0212B6C8: a hole by Kirby's side of the arena;
        r = Random(100);
        switch (KirbyZone(s)) {
        case ZONE_LEFT:
            Face(s, TRUE);
            MODE(s) = r < KIRBY_LEFT_FRONT ? MODE_FRONT : MODE_BACK;
            Point(s, MODE(s) == MODE_FRONT ? POINT_FRONT_RIGHT : POINT_BACK_RIGHT, &x, &y);
            break;
        case ZONE_RIGHT:
            Face(s, FALSE);
            MODE(s) = r < KIRBY_RIGHT_FRONT ? MODE_FRONT : MODE_BACK;
            Point(s, MODE(s) == MODE_FRONT ? POINT_FRONT_LEFT : POINT_BACK_LEFT, &x, &y);
            break;
        default:
            if (r < MIDDLE_LEFT_FRONT) {
                Face(s, FALSE);
                MODE(s) = MODE_FRONT;
                Point(s, POINT_FRONT_LEFT, &x, &y);
            } else if (r < MIDDLE_LEFT_FRONT + MIDDLE_LEFT_BACK) {
                Face(s, FALSE);
                MODE(s) = MODE_BACK;
                Point(s, POINT_BACK_LEFT, &x, &y);
            } else if (r < MIDDLE_LEFT_FRONT + MIDDLE_LEFT_BACK + MIDDLE_RIGHT_FRONT) {
                Face(s, TRUE);
                MODE(s) = MODE_FRONT;
                Point(s, POINT_FRONT_RIGHT, &x, &y);
            } else {
                Face(s, TRUE);
                MODE(s) = MODE_BACK;
                Point(s, POINT_BACK_RIGHT, &x, &y);
            }
            break;
        }
        s->obj.base.x = x;
        s->obj.base.y = y;
        AWAY(s) = FALSE;
        MoveFree(s);
        SqueakSetState(s, MOLEY_POP);
        return TRUE;
    case MOLEY_MIDDLE:      // 0x0212B630: her last stand, the middle back hole, facing right (out of reach: Squeak Squad's flag 1 kept);
        Face(s, FALSE);
        SqueakSfx(s, 0x195);
        TIMER(s) = 0;
        MODE(s) = MODE_MIDDLE;
        LANDED(s) = FALSE;
        Point(s, POINT_MIDDLE, &s->obj.base.x, &s->obj.base.y);
        s->obj.base.y += MIDDLE_DROP << 8;
        SqueakSetState(s, MOLEY_LOOK);
        return TRUE;
    case MOLEY_DIVE:        // 0x0212B594: her sparkles;
        SqueakPlayComposite(s, MODE(s) == MODE_MIDDLE ? COMPOSITE_MIDDLE_DIVE : COMPOSITE_BACK_DIVE);
        SqueakSfx(s, 0x195);
        LANDED(s) = FALSE;
        SqueakStop(s);
        SqueakSetState(s, MOLEY_DIVE_DOWN);
        return TRUE;
    case MOLEY_DIVE_DOWN:   // 0x0212B540;
        MoveFree(s);
        if (s->animEnded) {
            SqueakPlay(s, ANIM_S_DOWN);
            SqueakSetState(s, MOLEY_DIVE_GONE);
        }
        break;
    case MOLEY_DIVE_GONE:   // 0x0212B4E4;
        MoveFree(s);
        if (s->animEnded) {
            SqueakPlay(s, ANIM_S_GONE);
            TIMER(s) = 0;
            SqueakSetState(s, MOLEY_DIVE_WAIT);
        }
        break;
    case MOLEY_DIVE_WAIT:   // 0x0212B46C: DIVE_PAUSE frames, then out;
        MoveFree(s);
        if (TIMER(s)++ > DIVE_PAUSE && s->animEnded) {
            SqueakPlay(s, ANIM_JUMP_OUT);
            SqueakSetState(s, MOLEY_LEAP);
        }
        break;
    case MOLEY_LEAP:        // 0x0212B408;
        MoveFree(s);
        if (s->animEnded) {
            SqueakPlay(s, ANIM_RISING);
            s->vy = LEAP_VY;
            SqueakSetState(s, MOLEY_RISE);
        }
        break;
    case MOLEY_RISE:        // 0x0212B2BC: up till SKY_PROBE under her is the sky, petals falling off her;
        y = s->obj.base.y + (SKY_PROBE << 8);
        MoveFree(s);
        if (!Sky(s, y)) {
            // (from a back hole a petal every 30 frames, its 1st and 16th; from the middle every 20, its 1st and 11th);
            d = MODE(s) == MODE_BACK ? s->stateTimer % 30 : s->stateTimer % 20;
            if (d == 1) Dirt(s, DIRT_PETAL, 0, s->obj.base.x, s->obj.base.y);
            else if (d == (MODE(s) == MODE_BACK ? 16 : 11)) Dirt(s, DIRT_PETAL2, 0, s->obj.base.x, s->obj.base.y);
            break;
        }
        SqueakStop(s);
        Dirt(s, DIRT_SHADOW, 0, s->obj.kirby3->base.x, FLOOR(s) + (SHADOW_Y << 8));
        TIMER(s) = 0;
        SqueakSetState(s, MOLEY_TRACK);
        return TRUE;
    case MOLEY_TRACK:       // 0x0212B1EC: over Kirby (by a side: halfway to its front hole each frame);
        TIMER(s)++;
        switch (KirbyZone(s)) {
        case ZONE_LEFT:
            Point(s, POINT_FRONT_LEFT, &x, &y);
            s->obj.base.x = (x + s->obj.base.x) / 2;
            break;
        case ZONE_RIGHT:
            Point(s, POINT_FRONT_RIGHT, &x, &y);
            s->obj.base.x = (x + s->obj.base.x) / 2;
            break;
        default:
            s->obj.base.x = s->obj.kirby3->base.x;
            break;
        }
        if (TIMER(s) > TRACK_TIME) SqueakSetState(s, MOLEY_PLUNGE);
        break;
    case MOLEY_PLUNGE:      // 0x0212C210: upside down, in front now;
        SqueakPlay(s, ANIM_PLUNGE);
        s->ay = -PLUNGE_GRAVITY;
        TIMER(s) = 0;
        MODE(s) = MODE_FRONT;
        SqueakSetState(s, MOLEY_FALL);
        return TRUE;
    case MOLEY_FALL:        // 0x0212BFB8: down to the front holes' level: into the middle, stuck (hurt); else into a hole;
        if (s->obj.base.y < FLOOR(s)) {
            MoveFree(s);
            break;
        }
        LANDED(s) = TRUE;
        SqueakPlayComposite(s, COMPOSITE_LAND);
        SqueakStop(s);
        if (Zone(s, s->obj.base.x, s->obj.base.y) == ZONE_MIDDLE) {
            SqueakSfx(s, 0x15A);
            SqueakSetState(s, MOLEY_CRASH);
            Crash(s);
        } else {
            SqueakSfx(s, 0x197);
            SqueakSetState(s, MOLEY_HOLE);
        }
        if (PHASE(s) == PHASE_LAST) PHASE(s) = PHASE_ON;
        return s->obj.unk80 > 0;
    case MOLEY_CRASH:       // 0x0212B16C: stuck head first, petals bursting, the screen shaking;
        SqueakPlay(s, ANIM_STUCK);
        SqueakStop(s);
        TIMER(s) = 0;
        Dirt(s, DIRT_BURST, 0, s->obj.base.x, s->obj.base.y);
        RequestScreenShake(SHAKE_CRASH, &s->obj.base);
        SqueakSetState(s, MOLEY_CRASH_WAIT);
        return TRUE;
    case MOLEY_CRASH_WAIT:  // 0x0212B11C;
        MoveFree(s);
        if (s->stateTimer > CRASH_WAIT) {
            TIMER(s) = 0;
            SqueakSetState(s, MOLEY_CRASH_UP);
        }
        break;
    case MOLEY_CRASH_UP:    // 0x0212B0C0;
        MoveFree(s);
        if (s->animEnded) {
            TIMER(s) = 0;
            SqueakPlay(s, ANIM_CRAWL);
            SqueakSetState(s, MOLEY_STRUGGLE);
        }
        break;
    case MOLEY_STRUGGLE:    // 0x0212AF18: STRUGGLE_TIME frames, then a hop (HOP_TIME frames) into the front hole away from Kirby;
        TIMER(s)++;
        MoveFree(s);
        if (TIMER(s) <= STRUGGLE_TIME) break;
        SqueakPlay(s, ANIM_HOP);
        if (Ahead(s, s->obj.kirby3->base.x) > 0) SqueakTurnAround(s);
        SqueakStop(s);
        s->vy = HOP_VY;
        s->ay = -HOP_GRAVITY;
        Point(s, POINT_FRONT_LEFT, &x, &y);
        if (Ahead(s, x) < 0) {
            Point(s, POINT_FRONT_RIGHT, &x, &y);
            d = (x - s->obj.base.x) << 4;
        } else {
            d = (s->obj.base.x - x) << 4;
        }
        s->vx = d / HOP_TIME;
        if (d < HOP_NEAR) s->ay /= 2;
        else if (d < HOP_MID) s->ay = s->ay * 2 / 3;
        TIMER(s) = 0;
        SqueakSetState(s, MOLEY_HOP);
        break;
    case MOLEY_HOP:         // 0x0212AE04: after HOP_CHECK frames, within HOLE_ABOVE pixels over the hole she stops falling, past it in;
        MoveFree(s);
        if (++TIMER(s) <= HOP_CHECK) break;
        Point(s, FacesLeft(s) ? POINT_FRONT_LEFT : POINT_FRONT_RIGHT, &x, &y);
        if (s->obj.base.y < y - (HOLE_ABOVE << 8)) break;
        s->vy = 0;
        if (FacesLeft(s) ? s->obj.base.x <= x : s->obj.base.x >= x) SqueakSetState(s, MOLEY_HOLE);
        break;
    case MOLEY_HOLE:        // 0x0212BF58: into the hole, the screen shaking;
        SqueakPlay(s, ANIM_HOLED);
        SqueakStop(s);
        TIMER(s) = 0;
        RequestScreenShake(SHAKE_HOLE, &s->obj.base);
        SqueakSetState(s, MOLEY_SINK);
        return TRUE;
    case MOLEY_SINK:        // 0x0212BEFC;
        MoveFree(s);
        if (s->animEnded) {
            SqueakPlay(s, ANIM_SINK);
            SqueakSetState(s, MOLEY_SINK_END);
        }
        Grit(s);
        break;
    case MOLEY_SINK_END:    // 0x0212BEB0;
        MoveFree(s);
        if (s->animEnded) SqueakSetState(s, MOLEY_HIDE);
        Grit(s);
        break;
    }
    return FALSE;
}

/* Hit(); 0x0212C73C (KatAM's mini-bosses take hits without a hurt state of their own: a fall of her HP is the hit): only her front self
 * (out of a hole) is hit, SAFE_TIME frames safe after one (KatAM's damage undone), two petals knocked off, her last stand from
 * LAST_STAND HP; */
static void Hit(struct SqueakObject *s) {
    struct Object *obj = &s->obj;

    if (SAFE(s) > 0) SAFE(s)--;
    if (obj->unk80 < LAST_HP(s) && obj->unk80 > 0 && !(obj->base.flags & 0x1000)) {
        if (SAFE(s) > 0 || MODE(s) != MODE_FRONT || AWAY(s)) {
            obj->unk80 = LAST_HP(s);
        } else {
            SAFE(s) = SAFE_TIME;
            Petals(s);
            if (obj->unk80 <= LAST_STAND) PHASE(s) = PHASE_LAST;
        }
    }
    LAST_HP(s) = obj->unk80;
}

static void MrsMoleyMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    obj->kirby3 = FindTargetKirby(&obj->base);
    SqueakMidbossUpdate(s);
    Hit(s);
    SqueakRun(s, Moley);
    if (obj->unk80 <= 0) return;
    SqueakFinishFrame(s);
    SqueakAnimate(s);
    SqueakSetBoxes(s, &sMoleyBoxData, HARM(s), &BOX(s));
    // (out of reach in the back or down a hole: untouchable, as Squeak Squad's: its hit and touch take nothing then);
    if (MODE(s) != MODE_FRONT || AWAY(s)) obj->base.flags |= 0x200;
    else obj->base.flags &= ~0x200;
}

/* MrsMoleyStart(); Her fight's start (and after a hit: it goes on, as KatAM's mini-bosses do): the first time, from the right front
 * hole; */
void MrsMoleyStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, -1, MrsMoleyMain);
    if (s->started) return;
    PHASE(s) = PHASE_ON;
    SAFE(s) = 0;
    LANDED(s) = FALSE;
    LAST_HP(s) = obj->unk80;
    SqueakSetState(s, MOLEY_START);
}

/* MrsMoleyInit(); Her box (mrsMoley_cmn.rca's small one), her bounds (KatAM's), the front holes' level HOLE_UP pixels over the floor
 * where she's placed, hidden till her fight (0x0212C68C's first frames: Squeak Squad's screen still fading in); */
void MrsMoleyInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -13, -25, 9, -4);
    ObjectSetBounds(&s->obj.base, -16, -40, 16, 0);
    HARM(s) = s->obj.base.unk68;
    SqueakStandOnFloor(&s->obj);
    FLOOR(s) = s->obj.base.y - (HOLE_UP << 8);
    MODE(s) = MODE_FRONT;
    AWAY(s) = TRUE;
    s->obj.base.flags |= 1 | 0x400;
    SqueakPlay(s, ANIM_GONE);
}

/* What she throws (0x021292EC, 0x02127A50), as SQUEAK_MOLEY_THING (the variant Squeak Squad's kind): thrown forward and up (PrmBB04Ml0's
 * speeds and gravity: a rock 2.6, 2.3, 0.2; a stone 3.0, 3.0, 0.16; a pebble 3.5, 5.0, 0.15; a bolt 0.9, 4.0, 0.15; a ring 3.2, 0.02,
 * 0.28 (it rolls); a bomb 2.6, 2.3, 0.2; a can 2.5, 1.5, 0.2; a saw 1.8, 1.0, 0.007; grit 0.5, 2.7 or 2.9, 0.5), the rock, stone,
 * pebble and bomb less or more by how far ahead Kirby is (within 32 pixels, beyond 64 or within it: 0x02128FD0, 0x02128DD8,
 * 0x02128BDC, 0x0212879C); bouncing on the ground (and off walls) at the speed thrown up, less each bounce by its own (a bomb not
 * at all, sliding 1.05 slower each: 2.3 less 5.0), forward less by its own each; grit bounces once and falls through the floor. It
 * flies 240 frames (a bolt 380; a bomb flashing its last 60), then bursts (0x020B48E0's type 6), as on touching Kirby (0x02129260)
 * or hit (0x021291C8: KatAM's defeat). Kirby swallows any but grit, a ring giving Wheel, a bomb Bomb, a can Fire and a saw Cutter
 * (Squeak Squad's abilities 10, 9, 1 and 7); grit is harmless (no box), out of reach. Its sound: 0x199 as it's thrown or flung up as grit (SqueakSfx); the screen's 2-frame
 * shake at each bounce (every frame a ring rolls: KatAM's shortest is 14 frames). */
static const s32 sThingSpeeds[][5] = {     // (forward, up, gravity, forward lost and up lost a bounce: PrmBB04Ml0's +0x64..+0x134);
    [THING_ROCK] = { 0x299A, 0x24CD, 0x334, 0xCD, 0 },
    [THING_STONE] = { 0x3000, 0x3000, 0x290, 0x52, 0 },
    [THING_PEBBLE] = { 0x3800, 0x5000, 0x267, 0x7B, 0 },
    [THING_BOLT] = { 0xE67, 0x4000, 0x267, 0, 0 },
    [THING_RING] = { 0x3334, 0x52, 0x47B, 0, 0 },
    [THING_BOMB] = { 0x299A, 0x24CD, 0x334, 0x10CD, 0x5000 },
    [THING_CAN] = { 0x2800, 0x1800, 0x334, 0x267, 0 },
    [THING_SAW] = { 0x1CCD, 0x1000, 0x1D, 0, 0 },
    [THING_GRIT] = { 0x800, 0x2B34, 0x800, 0, 0x1000 },
    [THING_GRIT2] = { 0x800, 0x2E67, 0x800, 0, 0x1B34 },
};
#define THING_LIFE 240
#define BOLT_LIFE 380
#define BOMB_FLASH 60
#define ANIM_BOMB_FLASH 10                  // (its OnlyAnims' last);
#define NEAR_KIRBY 0x20000                  // 32 pixels;
#define FAR_KIRBY 0x40000                   // 64;
#define GRIT_TURN 0x199                     // 0x021281C8: ways 1 and 4 0.1 slower, 3 and 6 0.1 faster;

static bool32 Grit2(struct SqueakObject *s) {
    return s->variant == THING_GRIT || s->variant == THING_GRIT2;
}

// ThingLaunch(); 0x02128FD0 and the others: its speeds, by how far ahead of it Kirby is;
static void ThingLaunch(struct SqueakObject *s) {
    const s32 *v = sThingSpeeds[s->variant];
    s32 ahead = Ahead(s, s->obj.kirby3->base.x);

    s->vx = v[0];
    s->vy = v[1];
    s->ay = -v[2];
    SqueakSfx(s, 0x199);
    switch (s->variant) {
    case THING_ROCK:
        if (ahead <= NEAR_KIRBY) s->vx -= 0x333, s->vy -= 0x333;
        else if (ahead >= FAR_KIRBY) s->vx -= 0x4CC, s->vy -= 0x199;
        break;
    case THING_STONE:
        if (ahead <= NEAR_KIRBY) s->vx -= 0x666, s->vy -= 0x333;
        else if (ahead <= FAR_KIRBY) s->vx -= 0x333, s->vy -= 0x199;
        break;
    case THING_PEBBLE:
        if (ahead <= NEAR_KIRBY) s->vx -= 0x999, s->vy -= 0x4CC;
        else if (ahead <= FAR_KIRBY) s->vx -= 0x666, s->vy -= 0x333;
        break;
    case THING_BOMB:
        if (ahead <= NEAR_KIRBY) s->vx -= 0x1333, s->vy -= 0x333;
        else if (ahead >= FAR_KIRBY) s->vx += 0x1333, s->vy += 0x333;
        break;
    }
}

// Burst(); 0x020B48E0(6): it bursts, gone;
static void Burst(struct SqueakObject *s) {
    SqueakSpawnEffect(s, SQUEAK_FX_BURST, 0, 0, FALSE);
    s->obj.base.flags |= 0x1000;
}

static bool32 Thing(struct SqueakObject *s) {
    const s32 *v = sThingSpeeds[s->variant];
    struct LevelInfo *level;
    bool32 landed;

    switch (s->state) {
    case 0:
        SqueakPlay(s, s->variant);
        if (Grit2(s)) {
            // (0x021281C8: its way, the spot's (Mrs. Moley's Grit()): 1-3 facing right, 4-6 left, 1 and 4 slower, 3 and 6 faster);
            u8 way = s->vars[4];

            s->obj.base.flags = (s->obj.base.flags & ~1) | (way >= 4 ? 1 : 0);
            s->vx = v[0] + (way == 1 || way == 4 ? -GRIT_TURN : way == 3 || way == 6 ? GRIT_TURN : 0);
            s->vy = v[1];
            s->ay = -v[2];
            SqueakSfx(s, 0x199);
        } else {
            ThingLaunch(s);
        }
        T_LIFE(s) = s->variant == THING_BOLT ? BOLT_LIFE : THING_LIFE;
        T_BOUNCES(s) = 0;
        T_FREE(s) = FALSE;
        SqueakSetState(s, 1);
        return TRUE;
    case 1:                 // 0x02127A50;
        if (T_FREE(s)) {
            // (grit through the floor: falling free, gone out of the camera's view);
            level = &gCurLevelInfo[s->obj.base.unk56];
            s->vy += s->ay;
            s->obj.base.x += SQUEAK_SPEED(s->obj.base.flags & 1 ? -s->vx : s->vx);
            s->obj.base.y -= SQUEAK_SPEED(s->vy);
            s->moved = TRUE;
            if (s->obj.base.y > level->viewportPosition.y + ((DISPLAY_HEIGHT + 32) << 8)) s->obj.base.flags |= 0x1000;
        } else {
            SqueakMove(s);
        }
        if (--T_LIFE(s) < 0) {
            Burst(s);
            break;
        }
        landed = !T_FREE(s) && (SqueakOnGround(s) || (s->terrain & 3));
        if (landed) {
            T_BOUNCES(s)++;
            if (Grit2(s)) {
                if (T_BOUNCES(s) < 2) {
                    s->vx -= v[3];
                    s->vy = v[1] - v[4] * T_BOUNCES(s);
                    s->ay = -v[2];
                } else {
                    T_FREE(s) = TRUE;
                }
            } else {
                s->vx -= v[3];
                s->vy = v[1] - v[4] * T_BOUNCES(s);
                s->ay = -v[2];
                if (s->vx < 0) s->vx = 0;
                if (s->vy <= 0) s->vy = 0;
            }
        }
        if (s->variant == THING_BOMB && T_LIFE(s) == BOMB_FLASH) SqueakPlay(s, ANIM_BOMB_FLASH);
        break;
    }
    return FALSE;
}

static void MoleyThingMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    obj->kirby3 = FindTargetKirby(&obj->base);
    if (!Grit2(s) && s->state == 1 && SqueakTouchingKirby(s)) {
        Burst(s);
        return;
    }
    SqueakRun(s, Thing);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
    if (!Grit2(s)) SqueakSetBoxes(s, &sThingBoxData, T_HARM(s), &T_BOX(s));
}

// MoleyThingStart(); Its start (and after a hit: KatAM's defeat, its HP 1); what Kirby gets swallowing it;
void MoleyThingStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;
    static const u8 sAbilities[] = {
        [THING_RING] = KIRBY_ABILITY_WHEEL, [THING_BOMB] = KIRBY_ABILITY_BOMB, [THING_CAN] = KIRBY_ABILITY_FIRE,
        [THING_SAW] = KIRBY_ABILITY_CUTTER,
    };

    ObjectSetFunc(obj, -1, MoleyThingMain);
    obj->kirbyAbility = s->variant < ARRAY_COUNT(sAbilities) ? sAbilities[s->variant] : 0;
    if (s->started) return;
    SqueakSetState(s, 0);
}

/* MoleyThingInit(); Its box (8 pixels round its middle), its bounds (KatAM's); Kirby's body doesn't hurt it (it bursts on him: KatAM's
 * unk5C 0x80), nor shows its HP bar (0x080365C8's flag 0x1000000); grit untouchable, not swallowed; */
void MoleyThingInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -6, -6, 6, 6);
    ObjectSetBounds(&s->obj.base, -6, -6, 6, 6);
    T_HARM(s) = s->obj.base.unk68;
    s->obj.base.unk5C |= 0x80;
    s->obj.base.flags |= 0x1000000;
    if (Grit2(s)) {
        s->obj.base.unk5C |= 0x20;
        s->obj.base.flags |= 0x200;
        s->obj.base.unk68 = 0;
    }
}

/* Her dirt (0x0212A404), as SQUEAK_MOLEY_DIRT (the variant: Squeak Squad's kind | its way << 2): the petals' burst where she's stuck
 * (0x0212A31C, 0x0212A2FC: _ext00's animation 8, gone at its end); a petal (0x0212A034, 0x02129D20: 9 or 10; way 0, falling from her
 * as she leaps, 0.18 or 0.22 forward, 0.006 or 0.003 gravity, 180 or 240 frames; ways 1-3 knocked off by a hit, facing right, 4-6
 * left: 0.9 forward and 0.8 up, -0.6 and 0.7, 0.5 and 0.5, their own gravity and slowing, 40 frames; gone on the ground); her
 * shadow (0x02129C30, 0x02129B74, 0x02129B38: 11 under Kirby 120 frames, then 12 under her till she lands, 85 frames at most);
 * harmless, untouchable; */
#define DIRT_TIME_A 180
#define DIRT_TIME_B 240
#define DIRT_TIME 40
#define SHADOW_TRACK 120
#define SHADOW_WAIT 85
#define D_ANIM_BURST 0
#define D_ANIM_PETAL 1
#define D_ANIM_SHADOW 3
#define D_ANIM_SHADOW_BIG 4

static const s32 sPetalWays[2][4][4] = {   // (kind 1, 2; way 0, 1/4, 2/5, 3/6: forward, up, gravity, slowing);
    { { 0x2E1, 0, 0x18, 0 }, { 0xE66, 0xCCC, 0xCC, 8 }, { -0x999, 0xB33, 0x49, 4 }, { 0x800, 0x800, 0x51, 2 } },
    { { 0x385, 0, 0xC, 0 }, { 0xE66, 0xCCC, 0xA3, 8 }, { -0x999, 0xB33, 0x28, 4 }, { 0x800, 0x800, 0x3D, 2 } },
};

static struct SqueakObject *MoleyOf(struct SqueakObject *d) {
    struct Object *parent = d->obj.base.parent;

    // (an object's header kind is 1: KatAM's Kirbys 0, attacks 2, effects 3);
    if (parent && parent->base.header.kind == 1 && !(parent->base.flags & 0x1000) && SqueakIsSqueak(parent)
        && ((struct SqueakObject *)parent)->enemy == SQUEAK_MRS_MOLEY)
        return (struct SqueakObject *)parent;
    return NULL;
}

static bool32 DirtState(struct SqueakObject *d) {
    u8 kind = d->variant & 3, way = d->variant >> 2;
    struct SqueakObject *moley = MoleyOf(d);
    const s32 *p;

    switch (d->state) {
    case 0:
        if (kind == DIRT_BURST) {
            SqueakPlay(d, D_ANIM_BURST);
            SqueakSetState(d, 1);
        } else if (kind == DIRT_SHADOW) {
            SqueakPlay(d, D_ANIM_SHADOW);
            D_LIFE(d) = SHADOW_WAIT;
            D_TIMER(d) = 0;
            SqueakSetState(d, 3);
        } else {
            p = sPetalWays[kind - 1][way == 0 ? 0 : (way - 1) % 3 + 1];
            if (way) d->obj.base.flags = (d->obj.base.flags & ~1) | (way >= 4 ? 1 : 0);
            d->vx = p[0];
            d->vy = p[1];
            d->ay = -p[2];
            d->fx = p[3];
            D_LIFE(d) = way ? DIRT_TIME : kind == DIRT_PETAL ? DIRT_TIME_A : DIRT_TIME_B;
            SqueakPlay(d, kind);
            SqueakSetState(d, 2);
        }
        return TRUE;
    case 1:                 // 0x0212A2FC;
        if (d->animEnded) d->obj.base.flags |= 0x1000;
        break;
    case 2:                 // 0x0212A034's, 0x02129D20's;
        SqueakMove(d);
        if (SqueakOnGround(d) || --D_LIFE(d) < 0) d->obj.base.flags |= 0x1000;
        break;
    case 3:                 // 0x02129B74: under Kirby;
        d->obj.base.x = d->obj.kirby3->base.x;
        if (++D_TIMER(d) > SHADOW_TRACK) {
            SqueakPlay(d, D_ANIM_SHADOW_BIG);
            D_TIMER(d) = 0;
            SqueakSetState(d, 4);
        }
        break;
    case 4:                 // 0x02129B38: under her, till she lands;
        if (moley == NULL) {
            d->obj.base.flags |= 0x1000;
            break;
        }
        d->obj.base.x = moley->obj.base.x;
        if (LANDED(moley)) D_LIFE(d) = 0;
        if (--D_LIFE(d) < 0) d->obj.base.flags |= 0x1000;
        break;
    }
    return FALSE;
}

static void MoleyDirtMain(struct Object *obj) {
    struct SqueakObject *d = (struct SqueakObject *)obj;

    obj->kirby3 = FindTargetKirby(&obj->base);
    SqueakRun(d, DirtState);
    SqueakFinishFrame(d);
    SqueakAnimate(d);
}

void MoleyDirtStart(struct Object *obj) {
    ObjectSetFunc(obj, -1, MoleyDirtMain);
    obj->base.flags |= 0x200;
    obj->base.unk68 = 0;
    if (((struct SqueakObject *)obj)->started) return;
    SqueakSetState((struct SqueakObject *)obj, 0);
}

void MoleyDirtInit(struct SqueakObject *d) {
    sub_0803E2B0(&d->obj.base, -4, -4, 4, 4);
    ObjectSetBounds(&d->obj.base, -4, -4, 4, 4);
    d->obj.base.unk5C |= 0x20 | 0x80;
    d->obj.base.flags |= 0x200 | 0x1000000;
}
