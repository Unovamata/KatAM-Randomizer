/* Meta-Knights; ported from Kirby: Nightmare in Dream Land (USA), mini-boss object types 0x39 (Meta Knight, spawn 0x0809BBD8) and
 * 0x3A (his knights, 0x0809C404), as NiDL mini-boss NIDL_META_KNIGHTS. Every object here is of type 0x42 (a KatAM mini-boss type):
 * • Meta Knight, the template's object, is unseen (in NiDL he only swoops by at the start) and untouchable, and holds the HP bar. He
 *   runs one of NiDL's line-ups (the template's variant: 0x08745CFC's 0, 4, 8, 12 and 16): four lists that run side by side, each
 *   calling in its next knight once the last one is gone (0x0809BC1C), or waiting (30 to 240 frames), or warping one in (0x0809F680:
 *   his warp effect, drawn from NiDL's common tiles, is KatAM's puff of smoke here). The knights fade in (here they blink). His HP is
 *   what the knights have left (each KNIGHT_HP, those still to come too), out of MAX_HP (the HP bar's, as the other mini-boss's);
 *   once every list is done and every knight gone, he goes down where the last one did (KatAM's own defeat).
 * • The knights (type 0x3A's kinds 0-3, 0x08745CEC): Axe, Javelin, Mace and Trident Knight, each with its own graphics and palette.
 *   They die like KatAM's enemies (with base.unkC & 0x800), not like mini-bosses; KatAM's hurt frames show each one's own.
 * NiDL scripts (16.16 speeds, Y down; KatAM's are 8.8, Y up; gravity 0x2500, falling at 3.0 at most):
 * • Axe Knight (0x08747AA4; frames 0-34): variant 0 (0x0809C4D0) faces Kirby and backs off at 0.5; within 39 pixels (16 up or down)
 *   it slashes, dashing at 4.0, 3.0, 2.0 (0x0809C74C); every 90 frames it throws its axe (0x0809C880, the axe: 0x0809CF60, 3.5
 *   ahead), jumping first (3.0 up, one time in 3 when Kirby is above, 0x0809C984) or after (one time in 3 when Kirby is near,
 *   0x0809CA10). Variant 1 (0x0809CAB0) slashes on the spot over and over; 2 and 3 (0x0809CC24, 0x0809CD8C) walk in, slowing down.
 * • Javelin Knight (0x08747ADC; frames 0-57): its jet pack hops it about (0x0809D8F8: 0.5, 1.0 or 0.5 back, 2.0 or 3.0 up, gravity
 *   1/16); landed, it throws a javelin (0x0809D4A0, the javelin: 0x0809DA1C, 2.0 ahead, 3.5 or 1.0 up, gravity 0.09) or blasts off
 *   (0x0809D56C: 4.0 up, gravity 1/8). Variant 1 (0x0809D7A4) jets back and forth at 2.0.
 * • Mace Knight (0x08747BCC; frames 0-38): variant 0 (0x0809DCBC) stands and attacks (0x0809E04C): it twirls its mace overhead 4
 *   times, then throws it (0x0809E320) along the floor and reels it back; variant 1 (0x0809DD7C) steps forward at 0.25 between
 *   attacks; variant 2 (0x0809DF54) runs at 1.375 twirling it.
 * • Trident Knight (0x08747BF4; frames 0-62): variant 0 (0x0809E874) walks at 1.0, backing off at 0.75 within 63 pixels; every 60
 *   frames (one time in 2) it throws its trident (0x0809EB14, the trident: 0x0809F37C, 2.0 ahead and 2.0 up), jumping first when
 *   Kirby is far (0x0809EB7C); within 31 pixels at a wall it hops at Kirby (0x0809EAB8). Variant 1 (0x0809EDDC) walks in, then
 *   stands and throws (1.0, 1.625, 2.125, then 0.5 ahead, 0x08747C28); 2 and 3 walk in; */
#include "../nidl.h"
#include "code_0806F780.h"
#include "task.h"

void sub_0809D060(struct Object *);    // KatAM's mini-boss defeat (object.c, static);

#define KNIGHT_HP 6
#define MAX_HP 30           // Meta Knight's (the HP bar's full);
#define GRAVITY 0x25
#define MAX_FALL 0x300
#define FLOOR_Y 153         // NiDL's y of the knights on the floor;
#define VIEW_MARGIN 24      // The knights turn back that close to the edges of the screen;
#define WARP_TIME 21
#define FADE_TIME 18        // 0x0809F9DC;

enum { AXE, JAVELIN, MACE, TRIDENT, WAIT, END };

static const u8 sFirstFrame[] = { 0, 35, 93, 132 };     // Each knight's first frame;
static const u16 sWaits[] = { 30, 60, 120, 180, 240 };   // WAIT 0-4 (0x0809F658); WAIT 5 is the warp effect;

// One line-up entry (0x0809BC1C): knight (or WAIT, END), its variant and where it shows up (NiDL's screen coordinates);
struct Entry {
    u8 kind;
    u8 variant;
    s16 x;
    s16 y;
};

// The line-ups (0x08745CFC's 0, 4, 8, 12 and 16), four lists each;
static const struct Entry sLineup0_0[] = { { AXE, 2, 264, 57 }, { MACE, 1, 264, 105 }, { MACE, 1, 264, 57 }, { AXE, 3, 264, 105 }, { TRIDENT, 3, 264, 57 }, { END, 0, 0, 0 } };
static const struct Entry sLineup0_1[] = { { AXE, 3, 264, 153 }, { WAIT, 5, 40, 153 }, { MACE, 1, 40, 153 }, { WAIT, 2, 119, 105 }, { WAIT, 5, 248, 153 }, { MACE, 1, 248, 153 }, { MACE, 2, 8, 153 }, { END, 0, 0, 0 } };
static const struct Entry sLineup0_2[] = { { WAIT, 4, 119, 105 }, { TRIDENT, 1, 8, 41 }, { WAIT, 2, 119, 105 }, { TRIDENT, 2, 8, 41 }, { END, 0, 0, 0 } };
static const struct Entry sLineup1_0[] = { { AXE, 2, 264, 105 }, { AXE, 0, 264, 57 }, { WAIT, 5, 168, 25 }, { MACE, 0, 168, 25 }, { END, 0, 0, 0 } };
static const struct Entry sLineup1_1[] = { { AXE, 2, 8, 89 }, { AXE, 0, 8, 137 }, { WAIT, 5, 120, 9 }, { MACE, 0, 120, 9 }, { END, 0, 0, 0 } };
static const struct Entry sLineup1_2[] = { { WAIT, 0, 200, 41 }, { WAIT, 5, 200, 41 }, { JAVELIN, 0, 200, 41 }, { WAIT, 1, 56, 57 }, { WAIT, 5, 56, 57 }, { JAVELIN, 0, 56, 57 }, { END, 0, 0, 0 } };
static const struct Entry sLineup2_0[] = { { AXE, 0, 8, 105 }, { WAIT, 5, 264, 41 }, { TRIDENT, 1, 248, 41 }, { AXE, 2, 264, 105 }, { END, 0, 0, 0 } };
static const struct Entry sLineup2_1[] = { { MACE, 1, 264, 105 }, { WAIT, 5, 24, 41 }, { TRIDENT, 1, 24, 41 }, { MACE, 1, 8, 105 }, { END, 0, 0, 0 } };
static const struct Entry sLineup2_2[] = { { WAIT, 2, 119, 119 }, { WAIT, 5, 144, 57 }, { JAVELIN, 0, 144, 57 }, { WAIT, 5, 144, 153 }, { JAVELIN, 1, 144, 153 }, { WAIT, 5, 144, 1 }, { MACE, 0, 144, 1 }, { END, 0, 0, 0 } };
static const struct Entry sLineup3_0[] = { { AXE, 1, 264, 41 }, { WAIT, 1, 119, 119 }, { WAIT, 5, 56, 41 }, { AXE, 1, 56, 41 }, { WAIT, 0, 119, 119 }, { WAIT, 5, 200, 41 }, { AXE, 2, 200, 41 }, { END, 0, 0, 0 } };
static const struct Entry sLineup3_1[] = { { AXE, 1, 8, 89 }, { WAIT, 1, 119, 119 }, { WAIT, 5, 248, 89 }, { AXE, 1, 248, 89 }, { WAIT, 0, 119, 119 }, { WAIT, 5, 72, 89 }, { AXE, 2, 72, 89 }, { TRIDENT, 1, 8, 89 }, { END, 0, 0, 0 } };
static const struct Entry sLineup3_2[] = { { AXE, 1, 264, 137 }, { WAIT, 1, 119, 119 }, { WAIT, 5, 56, 137 }, { AXE, 1, 56, 137 }, { WAIT, 0, 119, 119 }, { WAIT, 5, 200, 137 }, { AXE, 2, 200, 137 }, { WAIT, 5, 248, 41 }, { TRIDENT, 1, 248, 41 }, { END, 0, 0, 0 } };
static const struct Entry sLineup4_0[] = { { WAIT, 5, 136, 89 }, { MACE, 0, 136, 89 }, { WAIT, 2, 119, 119 }, { WAIT, 5, 152, 41 }, { TRIDENT, 0, 152, 41 }, { WAIT, 2, 119, 119 }, { WAIT, 5, 152, 89 }, { MACE, 0, 152, 89 }, { END, 0, 0, 0 } };
static const struct Entry sLineup4_1[] = { { WAIT, 5, 216, 153 }, { MACE, 1, 216, 153 }, { AXE, 2, 8, 105 }, { AXE, 2, 264, 105 }, { WAIT, 5, 40, 105 }, { JAVELIN, 0, 40, 105 }, { END, 0, 0, 0 } };
static const struct Entry sLineup4_2[] = { { WAIT, 5, 72, 153 }, { MACE, 1, 72, 153 }, { AXE, 2, 264, 57 }, { AXE, 2, 8, 57 }, { WAIT, 5, 216, 57 }, { TRIDENT, 1, 216, 57 }, { END, 0, 0, 0 } };
static const struct Entry sLineup4_3[] = { { WAIT, 3, 119, 119 }, { WAIT, 5, 40, 153 }, { JAVELIN, 1, 40, 153 }, { END, 0, 0, 0 } };
static const struct Entry sNone[] = { { END, 0, 0, 0 } };

static const struct Entry *const sLineups[][4] = {
    { sLineup0_0, sLineup0_1, sLineup0_2, sNone },
    { sLineup1_0, sLineup1_1, sLineup1_2, sNone },
    { sLineup2_0, sLineup2_1, sLineup2_2, sNone },
    { sLineup3_0, sLineup3_1, sLineup3_2, sNone },
    { sLineup4_0, sLineup4_1, sLineup4_2, sLineup4_3 },
};

/* One step of a knight's move: frame (the knight's own), frames (0: the next step right away), then what changes when it starts:
 * horizontal speed (forward; NO_SPEED keeps it), a leap (height, gravity; 0: none) and what it does (STEP_*); */
struct Step {
    u8 frame;
    u8 time;
    s16 speed;
    s16 height;
    s16 gravity;
    u8 action;
};
#define NO_SPEED 0x7FFF
enum { STEP_NONE, STEP_LAND, STEP_FACE, STEP_THROW, STEP_SLASH, STEP_TWIRL, STEP_SWING, STEP_JET, STEP_LOOP, STEP_END };
#define S(frame, time) { frame, time, NO_SPEED, 0, 0, STEP_NONE }
#define SV(frame, time, speed) { frame, time, speed, 0, 0, STEP_NONE }
#define SA(frame, time, action) { frame, time, NO_SPEED, 0, 0, action }
#define SJ(frame, time, speed, height, gravity) { frame, time, speed, height, gravity, STEP_NONE }
#define LAND(frame) { frame, 1, NO_SPEED, 0, 0, STEP_LAND }
#define LOOP { 0, 0, NO_SPEED, 0, 0, STEP_LOOP }
#define DONE { 0, 0, NO_SPEED, 0, 0, STEP_END }

enum { ST_WALK, ST_SLASH, ST_THROW, ST_JUMP_THROW, ST_THROW_JUMP, ST_GUARD, ST_ENTER, ST_IDLE, ST_STEP, ST_RUN, ST_MACE, ST_HOP,
       ST_LEDGE, ST_JET, ST_FLY, ST_TURN };

// Axe Knight;
static const struct Step sAxeWalk[] = {   // 0x0809C5A4;
    S(4, 10), S(5, 8), S(6, 6), S(7, 4), S(8, 4), S(9, 10), S(8, 4), S(7, 4), S(6, 6), S(5, 8), LOOP,
};
static const struct Step sAxeSlash[] = {  // 0x0809C74C;
    SA(25, 0, STEP_FACE), SV(25, 10, 0), S(26, 2), { 22, 2, 0x400, 0, 0, STEP_SLASH }, SV(23, 2, 0x300), SV(24, 2, 0x200),
    SV(17, 16, 0), S(24, 2), S(23, 1), S(22, 2), S(26, 2), S(25, 10), DONE,
};
static const struct Step sAxeGuard[] = {  // 0x0809CAB0;
    SV(25, 10, 0), S(26, 2), { 22, 2, 0x400, 0, 0, STEP_SLASH }, SV(23, 2, 0x300), SV(24, 2, 0x200),
    SV(17, 16, 0), S(24, 2), S(23, 1), S(22, 2), S(26, 2), S(25, 10), LOOP,
};
static const struct Step sAxeThrow[] = {  // 0x0809C880;
    SA(5, 0, STEP_FACE), SV(5, 2, 0), S(16, 3), S(12, 3), S(11, 10), S(13, 2), SA(14, 60, STEP_THROW), DONE,
};
static const struct Step sAxeJumpThrow[] = {  // 0x0809C984;
    SA(6, 0, STEP_FACE), SJ(6, 2, 0, 0x300, GRAVITY), S(16, 2), S(12, 2), S(11, 11), S(12, 1), S(13, 2), SA(14, 1, STEP_THROW), LAND(14), DONE,
};
static const struct Step sAxeThrowJump[] = {  // 0x0809CA10;
    SA(6, 0, STEP_FACE), SV(6, 2, 0), S(16, 2), S(12, 2), S(11, 11), S(12, 1), S(13, 2), SA(14, 26, STEP_THROW),
    SJ(14, 12, NO_SPEED, 0x300, GRAVITY), LAND(14), DONE,
};
static const struct Step sAxeEnter[] = {  // 0x0809CC24;
    SV(4, 9, 0x200), S(5, 5), S(6, 5), S(7, 5), SV(8, 5, 0x100), S(9, 7), SV(9, 3, 0x80), S(8, 5), SV(7, 5, 0x40), S(6, 3), DONE,
};

// Javelin Knight;
static const struct Step sJavelinHop[] = {    // 0x0809D2A4;
    S(4, 4), S(5, 4), S(6, 4), S(7, 4), S(8, 4), S(9, 4), S(10, 4), S(11, 4), LOOP,
};
static const struct Step sJavelinTurn[] = {   // 0x0809D6DC;
    SA(25, 0, STEP_FACE), SV(25, 2, 0), S(8, 2), S(25, 2), S(8, 2), S(25, 2), S(8, 2), S(25, 2), S(8, 2), DONE,
};
static const struct Step sJavelinThrow[] = {  // 0x0809D4A0;
    SJ(4, 6, 0, 0x200, 0x10), S(5, 6), S(6, 4), SA(6, 2, STEP_THROW), S(7, 6), S(8, 6), S(9, 6), S(10, 6), S(11, 6), S(4, 6), S(5, 6),
    S(6, 6), LAND(6), DONE,
};
static const struct Step sJavelinJet[] = {    // 0x0809D56C;
    { 17, 2, 0, 0x400, 0x20, STEP_JET }, S(18, 2), S(19, 2), S(20, 2), S(21, 2), S(22, 2), S(23, 2), S(24, 2),
    SA(17, 2, STEP_JET), S(18, 2), S(19, 2), S(20, 2), S(21, 2), S(22, 2), S(23, 2), S(24, 2), LAND(17), DONE,
};
static const struct Step sJavelinFly[] = {    // 0x0809D7A4;
    SA(17, 2, STEP_JET), S(18, 2), S(19, 2), S(20, 2), S(21, 2), S(22, 2), S(23, 2), S(24, 2), LOOP,
};

// Mace Knight;
static const struct Step sMaceIdle[] = {  // 0x0809DCBC;
    SV(9, 8, 0), S(10, 12), S(11, 8), S(10, 12), DONE,
};
static const struct Step sMaceStep[] = {  // 0x0809DE54;
    SV(6, 3, 0x40), S(5, 5), S(4, 6), S(26, 5), S(6, 3), S(7, 5), S(8, 6), S(27, 5), DONE,
};
static const struct Step sMaceRun[] = {   // 0x0809DF54;
    SA(15, 3, STEP_TWIRL), S(13, 4), S(12, 4), S(14, 3), S(16, 4), S(12, 4), LOOP,
};
static const struct Step sMaceAttack[] = {    // 0x0809E04C;
    SV(28, 1, 0), S(29, 2), S(30, 2), S(31, 2), S(32, 2), S(33, 3), S(34, 16),
    SA(15, 3, STEP_TWIRL), S(13, 4), S(12, 4), S(14, 3), S(16, 4), S(12, 4), S(15, 3), S(13, 4), S(12, 4), S(14, 3), S(16, 4), S(12, 4),
    S(15, 3), S(13, 4), S(12, 4), S(14, 3), S(16, 4), S(12, 4), S(15, 3), S(13, 4), S(12, 4), S(14, 3), S(16, 4), S(12, 4),
    SA(17, 3, STEP_FACE), S(18, 3), SA(24, 60, STEP_SWING), S(25, 4), SV(21, 2, -0x400), SV(21, 2, -0x200), SV(20, 2, 0x200),
    SV(20, 2, 0x400), SV(20, 10, 0), DONE,
};

// Trident Knight;
static const struct Step sTridentWalk[] = {   // 0x0809E914;
    S(6, 3), S(9, 3), S(4, 4), S(5, 2), S(6, 3), S(10, 3), S(8, 4), S(7, 2), LOOP,
};
static const struct Step sTridentBack[] = {
    S(6, 3), S(7, 2), S(8, 4), S(10, 3), S(6, 3), S(5, 2), S(4, 4), S(9, 3), LOOP,
};
static const struct Step sTridentHop[] = {    // 0x0809EAB8;
    SA(29, 0, STEP_FACE), SJ(29, 18, 0x100, 0x300, GRAVITY), LAND(21), DONE,
};
static const struct Step sTridentThrow[] = {  // 0x0809EB14;
    SA(11, 0, STEP_FACE), SV(11, 8, 0), S(12, 3), S(13, 4), S(14, 14), SA(15, 3, STEP_THROW), S(27, 1), S(28, 1), S(16, 10), DONE,
};
static const struct Step sTridentJumpThrow[] = {  // 0x0809EB7C;
    SA(11, 0, STEP_FACE), SJ(11, 6, 0, 0x300, GRAVITY), S(12, 3), S(13, 3), S(14, 19), SA(15, 5, STEP_THROW), LAND(16), DONE,
};
static const struct Step sTridentEnter[] = {  // 0x0809EFC8;
    SV(5, 2, 0x200), S(6, 1), S(10, 2), S(8, 3), S(7, 2), S(6, 1), S(9, 2), S(4, 3), S(5, 2), S(6, 2), S(22, 4),
    SV(18, 10, 0x100), SV(21, 8, 0x80), SV(21, 8, 0x40), DONE,
};
static const struct Step sTridentLedge[] = {  // 0x0809EDDC, once it walked in;
    SV(23, 10, 0), S(24, 10), S(25, 10), S(23, 10), S(24, 10), S(25, 10), SA(11, 8, STEP_FACE), S(12, 3), S(13, 4), S(14, 14),
    SA(15, 3, STEP_THROW), S(27, 1), S(28, 1), S(16, 10), LOOP,
};

// The weapons: Axe Knight's axe (31-34), Javelin Knight's javelin (49-57), Mace Knight's mace (35-38), Trident Knight's trident (51-62);
static const u8 sAxeFrames[] = { 31, 34, 33, 32 };
static const s16 sTridentSpeeds[][2] = { { 0x200, 0x200 }, { 0x100, 0x200 }, { 0x1A0, 0x200 }, { 0x220, 0x200 }, { 0x80, 0x100 } };   // 0x08747C6C;
static const u8 sLedgeThrows[] = { 1, 2, 3, 1, 2, 3, 4 };    // 0x08747C28;
static const s16 sHopSpeeds[] = { 0x80, 0x80, 0x100, -0x80 };       // 0x08747B58;
static const s16 sHopHeights[] = { 0x200, 0x200, 0x300, 0x200, 0x200, 0x300, 0x200, 0x300 };    // 0x08747B68;
#define WEAPON_TILES 16
#define WEAPON_TIME 150

/* The mace's twirl (0x0809E670): its speed relative to the knight, 2 frames each (0: 3 frames), the second half behind the knight;
 * thrown (0x0809E320) it drops to the floor and rolls out and back (frames each); */
static const s8 sTwirl[] = { 8, 4, 0, -4, -8, -8, -4, 0, 4, 8 };
struct Roll {
    s16 speed;
    u8 time;
};
static const struct Roll sRoll[] = {
    { 0x300, 1 }, { 0x500, 1 }, { 0x300, 1 }, { 0x300, 10 }, { 0x200, 6 }, { 0x100, 6 }, { 0x80, 6 }, { 0, 12 },
    { -0x80, 6 }, { -0x100, 6 }, { -0x200, 6 }, { -0x300, 10 },
};

struct MetaKnight;

// Meta Knight (the template's object);
struct MetaKnights {
    struct NidlObject n;
    bool8 isKnight;                     // FALSE (the knights' is TRUE);
    u8 lastKind;
    s16 total;                          // The knights' HP, all told;
    const struct Entry *next[4];        // Each list's next entry;
    struct MetaKnight *knights[4];      // Each list's knight;
    u16 timer[4];                       // Each list's wait;
    s32 lastX;                          // Where the last knight went;
    s32 lastY;
};

// A knight;
struct MetaKnight {
    struct NidlObject n;
    bool8 isKnight;                     // TRUE;
    u8 kind;
    u8 state;
    u8 step;
    u8 timer;
    bool8 airborne;
    bool8 waiting;
    bool8 back;                         // Backing off (Axe Knight always does);
    u8 cooldown;
    u8 throws;
    u8 slot;
    u8 landings;
    u8 fade;
    s16 decide;
    s16 gravity;
    const struct Step *steps;
    struct MetaKnights *leader;
    struct Weapon *mace;
};

// A thrown weapon, or the mace (which stays with its knight);
struct Weapon {
    struct ObjectBase base;
    u8 kind;
    u8 palId;
    u8 mode;                            // The mace: twirling or thrown;
    u8 step;
    u8 timer;
    s16 gravity;
    s16 offsetX;
    s16 offsetY;
    struct MetaKnight *owner;
};
enum { MACE_TWIRL, MACE_THROWN };

typedef char sMetaKnightsFits[sizeof(struct MetaKnights) <= NIDL_OBJECT_SIZE ? 1 : -1];
typedef char sMetaKnightFits[sizeof(struct MetaKnight) <= NIDL_OBJECT_SIZE ? 1 : -1];

static void KnightSetState(struct MetaKnight *k, u8 state);
static void KnightMain(struct Object *obj);

static s32 ViewLeft(void) {
    return gCurLevelInfo[gLocalPlayerId].viewportPosition.x;
}

static s32 ViewTop(void) {
    return gCurLevelInfo[gLocalPlayerId].viewportPosition.y;
}

static void Forward(struct Object *obj, s16 speed) {
    obj->base.xspeed = (obj->base.flags & 1) ? -speed : speed;
}

static bool8 Landed(struct MetaKnight *k) {
    struct Object *obj = &k->n.obj;

    if (!(obj->base.unk62 & 4)) k->airborne = TRUE;
    return k->airborne && (obj->base.unk62 & 4) && obj->base.yspeed <= 0;
}

static s32 KirbyDx(struct Object *obj) {
    return (obj->kirby3->base.x - obj->base.x) >> 8;
}

static s32 KirbyDy(struct Object *obj) {
    return (obj->kirby3->base.y - obj->base.y) >> 8;
}

static u8 KnightPalette(struct MetaKnight *k) {
    return k->n.obj.base.sprite.palId;
}

/* WeaponMain(); A thrown axe, javelin or trident (an attack object, like the enemies' stars), falling as NiDL's, gone once it's
 * off the screen; */
static void WeaponMain(void) {
    struct Sprite sprite;
    struct Weapon *w = TaskGetStructPtr(gCurTask);
    struct ObjectBase *base = &w->base;
    s32 ax, ay;
    u8 frame;

    if (ObjectPreUpdate(base)) return;
    // (Kirby can inhale it, nidl.c: what he fights with when he has no power);
    if (NidlInhaled(base, WEAPON_TILES)) return;
    base->flags |= 4;
    if (++base->counter > WEAPON_TIME || base->y > ViewTop() + 0xC400) {
        base->flags |= 0x1000;
        return;
    }
    if (!(base->flags & 0x200)) SetPointerSomething(base);
    base->yspeed -= w->gravity;
    if (base->yspeed < -MAX_FALL) base->yspeed = -MAX_FALL;
    base->x += base->xspeed;
    base->y -= base->yspeed;
    ax = abs(base->xspeed);
    ay = abs(base->yspeed);
    switch (w->kind) {
    case AXE:   // 0x0809CF94;
        frame = sAxeFrames[(base->counter >> 1) & 3];
        break;
    case JAVELIN:   // 0x0809DA9C: by its heading;
        if ((ax >> 2) < (ay >> 2)) frame = base->yspeed > 0 ? 53 : 57;
        else if ((ax >> 1) < ay) frame = base->yspeed > 0 ? 54 : 56;
        else frame = 55;
        frame += sFirstFrame[JAVELIN];
        break;
    default:    // 0x0809F3E0: by its heading, flickering;
        if (base->yspeed > 0) frame = ax >= ay ? 59 : 55;
        else frame = ax >= ay ? 61 : 57;
        frame += sFirstFrame[TRIDENT] + ((base->counter >> 1) & 1);
        break;
    }
    Macro_08107BA8_4(base, &base->sprite, &sprite, WEAPON_TILES, &base->sprite);
    base->sprite.variant = frame;
    base->sprite.palId = w->palId;
    sub_0806F8BC(base);
}

static struct Weapon *Weapon(struct MetaKnight *k, void (*main)(void), s16 dx, s16 dy, u8 frame) {
    struct Object *obj = &k->n.obj;
    struct Task *t = TaskCreate(main, sizeof(struct Weapon), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct Weapon *w = TaskGetStructPtr(t);
    struct ObjectBase *base = &w->base;
    bool8 left = obj->base.flags & 1;

    ClearObjectBase(base);
    base->header.kind = 2;
    base->x = obj->base.x + (left ? -dx : dx) * 0x100;
    base->y = obj->base.y + dy * 0x100;
    base->parent = obj;
    base->counter = 0;
    base->roomId = obj->base.roomId;
    base->unk56 = obj->base.unk56;
    if (Macro_0810B1F4(base)) base->flags |= 0x2000;
    base->unk63 = 1;
    base->flags |= 0x10000140 | left;
    base->unk68 = 0x20000043;
    base->unk5C |= 0x80000;
    w->kind = k->kind;
    w->palId = KnightPalette(k);
    w->gravity = GRAVITY;
    w->owner = k;
    sub_0803E2B0(base, -6, -6, 6, 6);
    ObjectSetBounds(base, 0, 0, 0, 0);
    ObjectBaseInitSprite(base, &base->sprite, WEAPON_TILES, NidlGraphicsOf(&k->n)->animId, frame, obj->base.sprite.unk14 >> 6);
    base->sprite.palId = w->palId;
    return w;
}

// Throw(); The knight's weapon (0x0809CF04, 0x0809D944, 0x0809F29C);
static void Throw(struct MetaKnight *k) {
    struct Object *obj = &k->n.obj;
    struct Weapon *w;
    s16 xspeed, yspeed;

    switch (k->kind) {
    case AXE:
        w = Weapon(k, WeaponMain, 20, 0, sAxeFrames[0]);
        xspeed = 0x380;
        yspeed = 0x25;
        break;
    case JAVELIN:
        w = Weapon(k, WeaponMain, 0, 0, sFirstFrame[JAVELIN] + 55);
        xspeed = 0x200;
        yspeed = KirbyDy(obj) < -16 ? 0x380 : 0x100;
        w->gravity = 0x17;
        break;
    default:
        w = Weapon(k, WeaponMain, 12, -20, sFirstFrame[TRIDENT] + 59);
        if (k->n.variant == 1) {
            xspeed = sTridentSpeeds[sLedgeThrows[k->throws]][0];
            yspeed = sTridentSpeeds[sLedgeThrows[k->throws]][1];
            if (++k->throws >= ARRAY_COUNT(sLedgeThrows)) k->throws = 0;
        } else {
            xspeed = sTridentSpeeds[0][0];
            yspeed = sTridentSpeeds[0][1];
        }
        break;
    }
    w->base.xspeed = (obj->base.flags & 1) ? -xspeed : xspeed;
    w->base.yspeed = yspeed;
    PlaySfx(&obj->base, SE_COOKIN_THROW_PAN);
}

/* MaceMain(); Mace Knight's mace, on its chain: twirling over its head, or thrown along the floor and reeled back (an attack object
 * that stays with its knight); */
static void MaceMain(void) {
    struct Sprite sprite;
    struct Weapon *w = TaskGetStructPtr(gCurTask);
    struct ObjectBase *base = &w->base;
    struct MetaKnight *k = w->owner;
    struct Object *obj;
    s16 speed;

    if (ObjectPreUpdate(base)) return;
    // Gone with its knight, or once its knight is hit or does something else;
    if (!k || k->mace != w || k->n.obj.unk78 != (void *)KnightMain || (k->state != ST_MACE && k->state != ST_RUN)) {
        if (k && k->mace == w) k->mace = NULL;
        base->flags |= 0x1000;
        return;
    }
    obj = &k->n.obj;
    base->flags |= 4;
    ++base->counter;
    if (w->mode == MACE_TWIRL) {
        // 2 frames a step, 3 for the stops;
        if (!w->timer--) {
            if (++w->step >= ARRAY_COUNT(sTwirl)) w->step = 0;
            w->timer = sTwirl[w->step] ? 1 : 2;
        }
        w->offsetX += sTwirl[w->step];
        w->offsetY = -16;
    } else {
        // Down to the floor, then out and back;
        if (w->offsetY < 4) w->offsetY += 6;
        if (!w->timer--) {
            if (++w->step >= ARRAY_COUNT(sRoll)) {
                k->mace = NULL;
                base->flags |= 0x1000;
                return;
            }
            w->timer = sRoll[w->step].time - 1;
        }
        speed = sRoll[w->step].speed;
        w->offsetX += speed >> 8;
        if (speed & 0x80) w->offsetX += (base->counter & 1) ? (speed < 0 ? -1 : 1) : 0;
    }
    base->x = obj->base.x + ((obj->base.flags & 1) ? -w->offsetX : w->offsetX) * 0x100;
    base->y = obj->base.y + w->offsetY * 0x100;
    if (!(base->flags & 0x200)) SetPointerSomething(base);
    Macro_08107BA8_4(base, &base->sprite, &sprite, WEAPON_TILES, &base->sprite);
    base->sprite.variant = sFirstFrame[MACE] + 35 + ((base->counter >> 1) & 3);
    base->sprite.palId = w->palId;
    sub_0806F8BC(base);
}

static void Mace(struct MetaKnight *k, u8 mode) {
    struct Weapon *w = k->mace;

    if (!w) {
        w = k->mace = Weapon(k, MaceMain, 0, -16, sFirstFrame[MACE] + 35);
        w->offsetX = 0;
        w->offsetY = -16;
    }
    w->mode = mode;
    w->step = 0;
    w->timer = 1;
    if (mode == MACE_THROWN) {
        w->offsetX = -16;
        w->timer = 0;
        PlaySfx(&k->n.obj.base, SE_COOKIN_THROW_PAN);
    }
}

/* SlashMain(); Axe Knight's slash: a hitbox 0x1A pixels in front of it for 3 frames (as Blade Knight's, KatAM's Sword Knight's
 * 0x080B3694); */
static void SlashMain(void) {
    struct ObjectBase *slash = TaskGetStructPtr(gCurTask);
    struct Object *obj = slash->parent;

    if (slash->flags & 0x1000) {
        TaskDestroy(gCurTask);
    } else if (obj->base.flags & 0x1000) {
        slash->flags |= 0x1000;
    } else {
        slash->unk56 = obj->base.unk56;
        slash->x = obj->base.x;
        slash->y = obj->base.y;
        if (!Macro_0810B1F4(&obj->base)) {
            if (++slash->counter > 3) {
                slash->flags |= 0x1000;
            } else {
                if (obj->base.flags & 1) {
                    slash->x -= 0x1A00;
                    slash->flags |= 1;
                } else {
                    slash->x += 0x1A00;
                    slash->flags &= ~1;
                }
                if (!(slash->flags & 0x200)) SetPointerSomething(slash);
            }
        }
    }
}

static void Slash(struct Object *obj) {
    struct Task *t = TaskCreate(SlashMain, sizeof(struct ObjectBase), 0x3500, TASK_USE_EWRAM, NULL);
    struct ObjectBase *slash = TaskGetStructPtr(t);
    u32 flags = 0x50500400;

    ClearObjectBase(slash);
    slash->header.kind = 2;
    slash->x = obj->base.x;
    slash->y = obj->base.y;
    slash->parent = obj;
    slash->counter = 0;
    slash->roomId = obj->base.roomId;
    slash->unk56 = obj->base.unk56;
    slash->unk63 = 1;
    slash->unk68 = 0x20000043;
    slash->unk5C |= 0x80000;
    if (obj->base.flags & 1) {
        slash->x -= 0x1A00;
        flags |= 1;
    } else {
        slash->x += 0x1A00;
    }
    if (Macro_0810B1F4(slash)) flags |= 0x2000;
    slash->flags = flags;
    sub_0803E2B0(slash, -0x10, -0xC, 0x10, 0xC);
    PlaySfx(slash, SE_HEAVY_KNIGHT_SLASH_ATTACK);
}

static const struct Step *KnightSteps(struct MetaKnight *k, u8 state) {
    switch (state) {
    case ST_WALK: return k->kind == AXE ? sAxeWalk : k->back ? sTridentBack : sTridentWalk;
    case ST_SLASH: return sAxeSlash;
    case ST_GUARD: return sAxeGuard;
    case ST_THROW: return k->kind == AXE ? sAxeThrow : k->kind == JAVELIN ? sJavelinThrow : sTridentThrow;
    case ST_JUMP_THROW: return k->kind == AXE ? sAxeJumpThrow : sTridentJumpThrow;
    case ST_THROW_JUMP: return sAxeThrowJump;
    case ST_ENTER: return k->kind == AXE ? sAxeEnter : sTridentEnter;
    case ST_IDLE: return sMaceIdle;
    case ST_STEP: return sMaceStep;
    case ST_RUN: return sMaceRun;
    case ST_MACE: return sMaceAttack;
    case ST_HOP: return k->kind == TRIDENT ? sTridentHop : sJavelinHop;
    case ST_LEDGE: return sTridentLedge;
    case ST_JET: return sJavelinJet;
    case ST_FLY: return sJavelinFly;
    default: return sJavelinTurn;
    }
}

static void StartStep(struct MetaKnight *k) {
    struct Object *obj = &k->n.obj;
    const struct Step *s;

    for (;;) {
        s = &k->steps[k->step];
        if (s->action == STEP_LOOP) {
            k->step = 0;
            continue;
        }
        if (s->action == STEP_END) return;
        NidlSetFrame(&k->n, sFirstFrame[k->kind] + s->frame);
        k->timer = s->time;
        k->waiting = s->action == STEP_LAND;
        if (s->speed != NO_SPEED) Forward(obj, s->speed);
        if (s->height) {
            obj->base.yspeed = s->height;
            k->gravity = s->gravity;
            k->airborne = FALSE;
        }
        switch (s->action) {
        case STEP_FACE:
            NidlFaceKirby(obj);
            break;
        case STEP_THROW:
            Throw(k);
            break;
        case STEP_SLASH:
            Slash(obj);
            break;
        case STEP_TWIRL:
            if (!k->mace || k->mace->mode != MACE_TWIRL) {
                Mace(k, MACE_TWIRL);
                PlaySfx(&obj->base, SE_TWISTER_FLYING);
            }
            break;
        case STEP_SWING:
            Mace(k, MACE_THROWN);
            break;
        case STEP_JET:
            PlaySfx(&obj->base, SE_BRONTO_BURT_FLY);
            break;
        }
        if (k->timer) return;
        k->step++;
    }
}

// JavelinHop(); 0x0809D8F8: a jet pack hop, ahead (or a little back) and up;
static void JavelinHop(struct MetaKnight *k) {
    struct Object *obj = &k->n.obj;
    u16 r = Rand16() & 7;

    Forward(obj, sHopSpeeds[r >> 1]);
    obj->base.yspeed = sHopHeights[r];
    k->gravity = 0x10;
    k->airborne = FALSE;
}

static void KnightSetState(struct MetaKnight *k, u8 state) {
    struct Object *obj = &k->n.obj;

    k->state = state;
    k->step = 0;
    k->waiting = FALSE;
    if (state != ST_FLY) k->gravity = GRAVITY;
    switch (state) {
    case ST_WALK:
        NidlFaceKirby(obj);
        if (k->kind == AXE) {
            // 0x0809C570: facing Kirby, backing off;
            Forward(obj, -0x80);
            k->decide = 90;
        } else {
            // 0x0809E874;
            Forward(obj, k->back ? -0xC0 : 0x100);
        }
        break;
    case ST_HOP:
        if (k->kind == JAVELIN) JavelinHop(k);
        break;
    case ST_RUN:
        Forward(obj, 0x160);
        break;
    case ST_FLY:
        Forward(obj, 0x200);
        obj->base.yspeed = 0;
        k->gravity = 0;
        break;
    }
    k->steps = KnightSteps(k, state);
    StartStep(k);
}

// KnightNext(); What a knight does once a move ends;
static void KnightNext(struct MetaKnight *k) {
    struct Object *obj = &k->n.obj;

    obj->base.xspeed = 0;
    switch (k->kind) {
    case AXE:
        KnightSetState(k, ST_WALK);
        break;
    case JAVELIN:
        if (k->state == ST_TURN) KnightSetState(k, abs(KirbyDx(obj)) > 39 ? ST_THROW : ST_JET);
        else KnightSetState(k, ST_HOP);
        break;
    case MACE:
        if (k->state == ST_MACE) KnightSetState(k, k->n.variant == 1 ? ST_STEP : ST_IDLE);
        else KnightSetState(k, ST_MACE);
        break;
    default:
        if (k->state == ST_THROW) {
            NidlFaceKirby(obj);
            k->back = TRUE;
        }
        KnightSetState(k, ST_WALK);
        break;
    }
}

// AtEdge(); Heading off the screen (0x0809F994);
static bool8 AtEdge(struct Object *obj) {
    s32 x = obj->base.x - ViewLeft();

    return (x < VIEW_MARGIN * 0x100 && obj->base.xspeed < 0) || (x > (240 - VIEW_MARGIN) * 0x100 && obj->base.xspeed > 0);
}

// AxeWalk(); 0x0809C638: what Axe Knight does while it backs off;
static void AxeWalk(struct MetaKnight *k) {
    struct Object *obj = &k->n.obj;
    s32 dx = abs(KirbyDx(obj)), dy = KirbyDy(obj);

    if (k->cooldown) {
        k->cooldown--;
        return;
    }
    if (dx <= 39 && abs(dy) <= 16) {
        k->cooldown = 90;
        KnightSetState(k, ST_SLASH);
        return;
    }
    if (--k->decide > 0) return;
    k->cooldown = 90;
    if (dy < 0 && (Rand16() & 0xFF) < 0x55) {
        KnightSetState(k, ST_JUMP_THROW);
    } else if (dy > 47) {
        k->decide = 90;
        k->cooldown = 0;
    } else if (dx > 59 || (Rand16() & 0xFF) >= 0x55) {
        KnightSetState(k, ST_THROW);
    } else {
        KnightSetState(k, ST_THROW_JUMP);
    }
}

// TridentWalk(); 0x0809EA08: what Trident Knight does while it walks;
static void TridentWalk(struct MetaKnight *k) {
    struct Object *obj = &k->n.obj;
    s32 dx = abs(KirbyDx(obj));

    if (AtEdge(obj)) {
        // 0x0809EC84: an attack, or it turns back;
        if (k->decide <= 59 && dx <= 63) {
            KnightSetState(k, dx <= 31 ? ST_HOP : ST_THROW);
        } else {
            // Back the other way, facing Kirby: backing off if that's away from Kirby;
            NidlFaceKirby(obj);
            obj->base.xspeed = -obj->base.xspeed;
            k->back = (obj->base.xspeed > 0) == ((obj->base.flags & 1) != 0);
            k->steps = KnightSteps(k, ST_WALK);
            k->step = 0;
            StartStep(k);
        }
        return;
    }
    if (--k->decide > 59) return;
    if (k->decide <= 0) {
        // 0x0809ED08;
        k->decide = 60;
        if (Rand16() & 1) KnightSetState(k, dx > 63 ? ST_JUMP_THROW : ST_THROW);
        return;
    }
    // Close to Kirby, it backs off;
    if ((k->decide & 7) == 7 && !k->back && dx <= 63) {
        k->back = TRUE;
        KnightSetState(k, ST_WALK);
    }
}

static void KnightMain(struct Object *obj) {
    struct MetaKnight *k = (struct MetaKnight *)obj;

    NidlUpdate(&k->n);
    obj->kirby3 = FindTargetKirby(&obj->base);
    obj->base.flags |= 4;
    // (fading in, blinking: untouchable, as NiDL's, till it's all there);
    if (k->fade) {
        k->fade--;
        obj->base.flags = (k->fade & 2) ? obj->base.flags | 0x400 : obj->base.flags & ~0x400;
        obj->base.flags = k->fade ? obj->base.flags | 0x200 : obj->base.flags & ~(0x200 | 0x400);
    }
    obj->base.yspeed -= k->gravity;
    if (obj->base.yspeed < -MAX_FALL) obj->base.yspeed = -MAX_FALL;

    switch (k->state) {
    case ST_WALK:
        if (k->kind == AXE) {
            if (AtEdge(obj)) obj->base.xspeed = -obj->base.xspeed;
            AxeWalk(k);
        } else {
            TridentWalk(k);
        }
        if (k->state != ST_WALK) return;
        break;
    case ST_HOP:
        if (k->kind == JAVELIN) {
            if (AtEdge(obj)) obj->base.xspeed = -obj->base.xspeed;
            // Landed, every other time it throws or blasts off (after turning to Kirby), else it hops again;
            if (Landed(k)) {
                obj->base.xspeed = 0;
                if (++k->landings & 1) KnightSetState(k, ST_TURN);
                else JavelinHop(k);
                return;
            }
        }
        break;
    case ST_GUARD:
    case ST_STEP:
    case ST_RUN:
    case ST_FLY:
    case ST_JET:
    case ST_THROW:
        if (AtEdge(obj)) {
            obj->base.xspeed = -obj->base.xspeed;
            if (k->state == ST_RUN || k->state == ST_FLY) obj->base.flags ^= 1;
        }
        break;
    }

    if (k->waiting) {
        if (!Landed(k)) return;
        k->waiting = FALSE;
        k->gravity = GRAVITY;
        k->timer = 1;
    }
    if (k->timer && --k->timer) return;
    if (k->steps[k->step].action == STEP_END) {
        KnightNext(k);
        return;
    }
    k->step++;
    if (k->steps[k->step].action == STEP_END) {
        KnightNext(k);
        return;
    }
    StartStep(k);
}

// KnightStart(); It shows up (and again after a hit);
static void KnightStart(struct MetaKnight *k) {
    struct Object *obj = &k->n.obj;
    u8 v = k->n.variant;

    ObjectSetFunc(obj, 0, KnightMain);
    // Its own palette (each knight's frames load theirs);
    obj->base.sprite.palId = NidlLoadPalette(&k->n, NidlGraphicsOf(&k->n)->animId | k->kind << 13, sFirstFrame[k->kind]);
    obj->base.flags &= ~0x200;
    obj->base.yspeed = 0;
    k->gravity = GRAVITY;
    k->airborne = TRUE;
    switch (k->kind) {
    case AXE:
        // Its entrance only once (variants 2 and 3: it walks in);
        if (v == 1) KnightSetState(k, ST_GUARD);
        else if (v >= 2 && !k->landings) KnightSetState(k, ST_ENTER);
        else KnightSetState(k, ST_WALK);
        break;
    case JAVELIN:
        KnightSetState(k, v == 1 ? ST_FLY : ST_HOP);
        break;
    case MACE:
        KnightSetState(k, v == 2 ? ST_RUN : v == 1 ? ST_STEP : ST_IDLE);
        break;
    default:
        if (v >= 1 && !k->landings) KnightSetState(k, ST_ENTER);
        else KnightSetState(k, v == 1 ? ST_LEDGE : ST_WALK);
        break;
    }
    k->landings = 1;
}

/* Warp(); Where a knight warps in next (0x0809F680): KatAM's puff of smoke (an effect object stays where it's made, so Meta Knight,
 * unseen, makes it from there); */
static void Warp(struct MetaKnights *m, s32 x, s32 y) {
    struct Object *obj = &m->n.obj;
    s32 ox = obj->base.x, oy = obj->base.y;

    obj->base.x = x;
    obj->base.y = y;
    CreateEffectObject(&obj->base, 0, 0x292, 0);
    PlaySfx(&obj->base, SE_ABILITY_MAGIC_META_KNIGHT);
    obj->base.x = ox;
    obj->base.y = oy;
}

// Where an entry's NiDL screen coordinates are: across the screen, and up from Meta Knight (on the floor);
static s32 EntryX(struct Entry const *e) {
    s32 x = e->x;

    if (x < 16) x = 16;
    if (x > 224) x = 224;
    return ViewLeft() + x * 0x100;
}

static s32 EntryY(struct MetaKnights *m, struct Entry const *e) {
    s32 y = m->n.obj.base.y + (e->y - FLOOR_Y) * 0x100;

    if (y < ViewTop() + 0x1800) y = ViewTop() + 0x1800;
    return y;
}

// CallKnight(); 0x0809BC1C: a knight of the entry's kind and variant (a type 0x42 object too: CreateNidlMiniboss);
static void CallKnight(struct MetaKnights *m, u8 slot, struct Entry const *e) {
    struct Object *obj = &m->n.obj;
    struct MetaKnight *k = CreateObjTemplateAndObj(obj->base.unk56, 1, 0x24, EntryX(e) >> 8, EntryY(m, e) >> 8, 0, 0x1F, 0, 0,
                                                   NIDL_MINIBOSS_TYPE_3, NIDL_META_KNIGHTS << 4 | e->variant, 0, e->kind + 1, 0, 0, 0, 0, 0,
                                                   0, 0, 0, 0, 0);

    if (!k) return;
    k->leader = m;
    k->slot = slot;
    k->n.obj.base.parent = m;
    // The fight's area (Macro_081003EC's, NidlUpdate keeps the knights in it);
    k->n.obj.unkA4 = obj->unkA4;
    k->n.obj.unkA6 = obj->unkA6;
    k->n.obj.unkA8 = obj->unkA8;
    k->n.obj.unkAA = obj->unkAA;
    m->knights[slot] = k;
}

// Divide(); n / d (the code has no division routine to call);
static u32 Divide(u32 n, u32 d) {
    u32 q = 0, bit = 1;

    if (!d) return 0;
    while (d < n && !(d & 0x80000000)) {
        d <<= 1;
        bit <<= 1;
    }
    while (bit) {
        if (n >= d) {
            n -= d;
            q |= bit;
        }
        d >>= 1;
        bit >>= 1;
    }
    return q;
}

// KnightsLeft(); What's left of the knights: the ones there and the ones to come;
static s16 KnightsLeft(struct MetaKnights *m) {
    s16 hp = 0;
    u8 i;
    const struct Entry *e;

    for (i = 0; i < 4; i++) {
        if (m->knights[i] && m->knights[i]->n.obj.unk80 > 0) hp += m->knights[i]->n.obj.unk80;
        for (e = m->next[i]; e->kind != END; e++)
            if (e->kind < WAIT) hp += KNIGHT_HP;
    }
    return hp;
}

static void MetaKnightsMain(struct Object *obj) {
    struct MetaKnights *m = (struct MetaKnights *)obj;
    const struct Entry *e;
    bool8 done = TRUE;
    u8 i;

    NidlUpdate(&m->n);
    obj->base.flags |= 4 | 0x400 | 0x200;
    obj->base.xspeed = 0;
    NidlFall(obj);
    for (i = 0; i < 4; i++) {
        if (m->knights[i]) {
            done = FALSE;
            continue;
        }
        if (m->timer[i]) {
            m->timer[i]--;
            done = FALSE;
            continue;
        }
        e = m->next[i];
        if (e->kind == END) continue;
        done = FALSE;
        m->next[i]++;
        if (e->kind == WAIT) {
            if (e->variant < ARRAY_COUNT(sWaits)) {
                m->timer[i] = sWaits[e->variant];
            } else {
                Warp(m, EntryX(e), EntryY(m, e));
                m->timer[i] = WARP_TIME;
            }
        } else {
            CallKnight(m, i, e);
        }
    }
    /* His HP (the HP bar): what's left of them, out of MAX_HP. The bar goes to any mini-boss that's hit (0x08034E14), knights too:
     * he takes it back; */
    sub_08034E14(obj);
    obj->unk80 = Divide(KnightsLeft(m) * MAX_HP + m->total - 1, m->total);
    if (!done) {
        if (obj->unk80 < 1) obj->unk80 = 1;
        return;
    }

    // All gone: Meta Knight goes down where the last one did, as KatAM's mini-bosses do (with its hurt frame);
    obj->unk80 = 0;
    obj->base.x = m->lastX;
    obj->base.y = m->lastY;
    obj->base.flags &= ~(0x400 | 0x200);
    m->n.hurt = m->lastKind;
    m->n.hurtVram = 0;
    NidlUploadHurtTiles(&m->n);
    obj->base.sprite.palId = NidlLoadPalette(&m->n, NidlGraphicsOf(&m->n)->animId | m->lastKind << 13, sFirstFrame[m->lastKind]);
    sub_08086C48(obj);
    sub_0809D060(obj);
}

// NidlIsMetaKnight(); A knight Meta Knight called in (its template, CallKnight's: the knight's kind + 1 in subtype2);
bool32 NidlIsMetaKnight(struct ObjectTemplate *template) {
    return template->unk2 == 0 && template->unk3 == 0x1F && template->subtype2;
}

// MetaKnightsStart(); The fight starts (Meta Knight), a knight shows up, and again after a hit;
void MetaKnightsStart(struct Object *obj) {
    struct MetaKnights *m = (struct MetaKnights *)obj;

    if (m->isKnight) {
        KnightStart((struct MetaKnight *)obj);
        return;
    }
    ObjectSetFunc(obj, 0, MetaKnightsMain);
    obj->base.flags |= 0x400 | 0x200;
}

// MetaKnightsInit(); Meta Knight (on the floor, unseen, his line-up) or a knight (its graphics, palette and hurt frame); their box's
// bottom where the knights' frames' feet are, 9 pixels down (at 12 they stood 2 to 5 above the floor);
void MetaKnightsInit(struct NidlObject *n) {
    struct Object *obj = &n->obj;
    struct ObjectTemplate *template = obj->objTemplate;
    struct MetaKnights *m = (struct MetaKnights *)n;
    struct MetaKnight *k = (struct MetaKnight *)n;
    u8 i;

    ObjectSetBounds(&obj->base, -8, -12, 8, 9);
    if (NidlIsMetaKnight(template)) {
        k->isKnight = TRUE;
        k->kind = (template->subtype2 - 1) & 3;
        k->n.variant &= 3;
        k->n.hurt = k->kind;
        k->n.activated = TRUE;
        k->leader = NULL;
        k->mace = NULL;
        k->back = FALSE;
        k->cooldown = 0;
        k->decide = 60;
        k->throws = 0;
        k->landings = 0;
        k->fade = FADE_TIME;
        k->state = ST_WALK;
        k->steps = sAxeWalk;
        k->step = 0;
        k->timer = 1;
        obj->unk80 = KNIGHT_HP;
        // They die as KatAM's enemies do, not as mini-bosses (object.c's 0x0809AF38), and Kirby can inhale them as an enemy (KatAM's
        // weight 1, not a mini-boss's 3 with its "not inhaled" bit, 0x20, and its 0x108000), as NiDL's: what he fights the others
        // with when he has no power (they give nothing, as NiDL's: their variant data's byte 12, 0x08747D5C...);
        obj->base.unkC |= 0x800;
        obj->base.unk5C = (obj->base.unk5C & ~(7 | 0x20 | 0x108000)) | 1;
        sub_0803E2B0(&obj->base, -10, -10, 10, 9);
        NidlSetFrame(n, sFirstFrame[k->kind] + 4);
        return;
    }
    m->isKnight = FALSE;
    m->lastKind = AXE;
    if (n->variant >= ARRAY_COUNT(sLineups)) n->variant = 0;
    for (i = 0; i < 4; i++) {
        m->next[i] = sLineups[n->variant][i];
        m->knights[i] = NULL;
        m->timer[i] = 0;
    }
    obj->base.flags |= 0x400;
    sub_0803E2B0(&obj->base, 0, 0, 0, 0);
    NidlOnFloor(obj);
    m->lastX = obj->base.x;
    m->lastY = obj->base.y;
    m->total = KnightsLeft(m);
    obj->unk80 = MAX_HP;
}

/* MetaKnightsDestroy(); Any of them goes (CreateNidlMiniboss's task destructor): a knight leaves its list free (and Meta Knight learns
 * where it went), Meta Knight lets his knights know; */
void MetaKnightsDestroy(struct Task *task) {
    struct MetaKnights *m = TaskGetStructPtr(task);
    struct MetaKnight *k = (struct MetaKnight *)m;
    u8 i;

    if (k->isKnight) {
        if (k->mace) k->mace->owner = NULL;
        if (k->leader && k->leader->knights[k->slot] == k) {
            k->leader->knights[k->slot] = NULL;
            k->leader->lastX = k->n.obj.base.x;
            k->leader->lastY = k->n.obj.base.y;
            k->leader->lastKind = k->kind;
        }
    } else {
        for (i = 0; i < 4; i++)
            if (m->knights[i]) m->knights[i]->leader = NULL;
    }
    NidlDestroy(task);
}
