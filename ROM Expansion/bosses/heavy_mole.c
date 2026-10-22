/* Heavy Mole; ported from Kirby: Nightmare in Dream Land (USA), boss object type 0x3E (spawn 0x080AD4B8), as NiDL boss NIDL_HEAVY_MOLE,
 * Yogurt Yard's (room 0x24C here: NiDL's 4-6-0, 1024 x 512, all dirt but the pocket Kirby comes in at, its pixels NiDL's:
 * KatAMExpansionRooms.cs' HeavyMoleRoom). NiDL's, from its code: it (its treads' script, its move and the camera every frame, 0x080AD650,
 * its attacks' steps after them, 0x080AD710; its globals at 0x02007D00), its children (its arms, 0x79 on top and 0x7A below, six states
 * each, 0x0874B5E4 and 0x0874B5FC; its drill 0xBE, its hatch 0xBD, its eye 0xBF, its dirt clods 0xC0), its missiles (0x7B, 0x7C), its
 * hit stars (0x8E) and the dirt's debris (0xEC), checked against NiDL's recorded fight: see "The fight" below.
 * NiDL's objects are Actors here (one array, run in NiDL's order: Turns(), then Follows()), each drawn as a sprite of its own (EffectOnly
 * frames: KatAMExpansion.cs' HeavyMole()), at NiDL's depths; the boss object KatAM knows (hit, its HP bar) is it. The screen is NiDL's,
 * scrolling with it (its arena's, enemies/arena.c: ArenaScroll()), 512 pixels back every time it passes 767 (Wrap()), as NiDL's; it and
 * its arms dig through the dirt (KatAM's ability blocks, 0xF4, Kirby doesn't inhale: Dig()). Its missiles are what Kirby inhales and
 * spits back at it (Inhaled()), as NiDL's; one that hits him bursts (Burst()).
 * Its frames are NiDL's 0x08753E3C (its body 0-7, its drill 8-10, its eye 11-13, its hatch 14-16, its clods 17-19, its arms 20-155:
 * their 0-125, their rods 126-135), its missiles' (0x087540AC, 0x087540EC), NiDL's shared hit stars (0x0874CBC8), dirt debris
 * (0x0874CD54) and burst (0x0874C9D8); */
#include "../nidl.h"
#include "../abilities.h"
#include "code_0806F780.h"
#include "task.h"
#include "main.h"
#include "kirby.h"
#include "object.h"
#include "functions.h"
#include "constants/songs.h"

void sub_0803D280(u8, u8);
void ArenaScroll(struct Object *, s32, s32, s32);

/* gNidlHeavyMole; Filled in by KatAMExpansion.cs (WriteHeavyMoleFight()): the first KatAM frame of each of its groups (G_*: its body,
 * NiDL's 0-7; its drill, 8-10; its eye, 11-13; its hatch, 14-16; its clods, 17-19; its arms, 20-155; its missile 0x7B's third palette's
 * pieces, 0-15 of its table; its fourth's; its missile 0x7C, 0-15 of its table; the hit stars; the debris; the burst), the VRAM tiles of each
 * group's biggest frame, its four palettes' colours (NiDL's 0, its body, eye and hatch; 1, its drill and arms; 2 and 3, its missiles; 3,
 * its clods) and its hurt flash's (NiDL's, the 32 after its descriptor: USA 0x082F65D4); */
#define GROUPS 12
struct NidlHeavyMole {
    u8 fx[GROUPS];
    u8 tiles[GROUPS];
    u16 colors[4][16];
    u16 flash[32];
};

const struct NidlHeavyMole gNidlHeavyMole = { { 0 } };

enum { G_BODY, G_DRILL, G_EYE, G_HATCH, G_CLODS, G_ARMS, G_MISSILE, G_MISSILE_B, G_MISSILE2, G_HIT, G_DEBRIS, G_BURST };
enum { PAL_BODY, PAL_ARMS, PAL_MISSILE, PAL_CLODS, PAL_COMMON, PALS };
#define KEY_ARMS 0x2000             // Its palettes' keys (| its anim ID; its body's is its anim ID, as its object's; never 0x1000, which
#define KEY_MISSILE 0x4000          // KatAM's sub_0803DF24 counts its users with);
#define KEY_CLODS 0x6000
#define KEY_COMMON 0x8000

#define FRAME_HIDDEN -1
#define PC_END 0xFFFF
#define NONE -1
#define GUARD_TIME 31               // Frames it takes no hits after one (NiDL's +0x75; KatAM's 0x8000: its attacks pass it by);
#define HURT_TIME 23                // NiDL's hurt (0x0806619C, 23): jittering, flashing, its steps held (0x080ADB58);
#define SCREEN_W 240
#define SCREEN_H 160
#define START_CX 128                // NiDL's camera's middle as the fight starts (8, 181 its top left: traced), its spot on the screen
#define START_CY 261                // (+0x4C: 176, 56 right of its middle) and its height (the camera's middle);
#define START_X 176
#define CAM_TOP 88                  // The camera's middle's bounds up and down (0x03005628: its height's 90 inside them, Edge()'s);
#define CAM_BOTTOM 424
#define ROOM_W 1024                 // NiDL's room (0x03005620: 64 x 32 cells);
#define ROOM_H 512
#define CELLS_W (ROOM_W / 16)
#define CELLS_H (ROOM_H / 16)
#define WRAP_AFTER 767              // The camera's left past it, everything 512 back (NiDL's 0x08026308: every x & 0x1FF);
#define DIRT 0xF4                   // Its dirt's collision (KatAM's ability blocks: KatAMExpansionRooms.cs' DirtBlock);
#define MIRROR_FROM 40              // A cell broken this far across also is at its x & 31 (NiDL's 0x08031EBC: the screen back there);
#define PUSH 0x980                  // Kirby kept 9.5 pixels inside the screen's sides (NiDL's, traced: sb/hmIdle);
#define ALIVE_MARGIN 64             // A missile gone this far off the screen (0x08065160);
#define INHALE_PULL 42              // A missile in a Kirby's inhale pulled in faster and faster, caught this near (nidl.c's NidlInhaled(),
#define INHALE_CATCH 0x1200         // KatAM's 0x0809C48C);
#define KIRBY_GROW_Y 2              // Its boxes hurting Kirby this much taller each way: NiDL's Kirby's box (his spot, 4 above KatAM's, to
                                    // 10 down: shine_bright.c's fit) past KatAM's standing Kirby's (2 up to 4 down); across both 4 either
                                    // side (NiDL's: 3 to 5, by its missile's burst in sb/hmIdle);
#define ORDER_CLODS 0x15            // Sprite orders (NiDL's depths, +0x42: its drill and clods 7, behind Kirby; its bottom arm 8, its
#define ORDER_BOTTOM 0x16           // hatch and eye 9, it 10, its top arm, rods and missiles 11, the debris 12; the hit stars 10);
#define ORDER_EYE 0x17
#define ORDER_MOLE 0x18
#define ORDER_TOP 0x19
#define ORDER_DEBRIS 0x1A

/* NiDL's objects (Actor.kind): it, then its children (NiDL's slots on); */
enum { K_NONE, K_MOLE, K_TOP, K_BOTTOM, K_DRILL, K_HATCH, K_EYE, K_CLODS, K_MISSILE, K_MISSILE2, K_HIT, K_DEBRIS, K_BURST };
#define ACTORS 32
#define A_MOLE 0
#define SPARE_DEBRIS 4

/* Sounds (NiDL's own: KatAM's with the same notes, but the hatch's, whose end differs); */
#define SFX_HATCH SE_BOMBAR_MISSILE_TELEGRAPH   // (0x20C: its hatch opening);
#define SFX_FIRE SE_BOMBAR_MISSILE_ATTACK       // (0x20B: a missile fired);
#define SFX_BREAK SE_BLOCK_BREAK                // (0x9F: a cell of dirt broken);
#define SFX_BURST SE_OBJECT_BULLET_DESPAWN      // (0xD4: a missile bursting on Kirby);

#define F(v) ((s32)((v) * 0x10000))
/* Its steps (0x0874AAF4 on): its treads' frames' waits by their speed; its script (by turns: back to its spot, then an attack's steps
 * picked by Pick()), each step's frames and its speeds' row (+ its level: by its HP), the speeds (across, up or down; across 16.16 on
 * the screen); back to its spot (its speed and its spot across by its level), the camera's speed; the attacks' chances by its level
 * and the last one, their steps' choices (masks) and starts; how near the camera's bounds it turns back (Edge()); */
static const u8 sTreadWait[3] = { 8, 4, 2 };
static const u8 sScript[47] = {
    0, 1, 2, 4, 0, 1, 3, 5, 0, 1, 6, 8, 0, 1, 7, 9, 0, 1, 10, 12, 0, 1, 11, 13, 0, 1, 14, 19, 16, 0, 1, 15, 18, 17, 0, 1, 16, 19, 14, 0, 1,
    17, 18, 15, 0, 1, 0,
};
static const u16 sDur[21] = { 0, 0, 240, 180, 240, 120, 120, 90, 240, 180, 120, 90, 240, 180, 120, 120, 120, 120, 240, 240, 0 };
static const u8 sBase[20] = { 0, 0, 3, 3, 0, 0, 9, 9, 15, 15, 12, 12, 18, 18, 21, 24, 27, 30, 6, 3 };
static const s32 sXs[33] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, F(0.125), F(0.1875), F(0.3125), F(-0.125), F(-0.1875), F(-0.3125), F(0.0625), F(0.09375), F(0.15625),
    F(-0.0625), F(-0.09375), F(-0.15625), F(0.125), F(0.1875), F(0.3125), F(0.125), F(0.1875), F(0.3125), F(-0.125), F(-0.1875),
    F(-0.3125), F(-0.125), F(-0.1875), F(-0.3125),
};
static const s32 sYs[33] = {
    0, 0, 0, F(0.25), F(0.375), F(0.625), F(-0.25), F(-0.375), F(-0.625), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, F(-0.25), F(-0.375),
    F(-0.625), F(0.25), F(0.375), F(0.625), F(-0.25), F(-0.375), F(-0.625), F(0.25), F(0.375), F(0.625),
};
static const s32 sMoveSpeed[3] = { F(0.125), F(0.1875), F(0.3125) };
static const s16 sTarget[3] = { 208, 168, 128 };
static const s32 sCamSpeed[3] = { F(0.25), F(0.3125), F(0.375) };
static const u8 sChances[12] = { 192, 63, 0, 0, 96, 64, 64, 31, 64, 32, 32, 127 };
static const u8 sMasks[4] = { 1, 1, 1, 3 };
static const u8 sStarts[10] = { 2, 6, 10, 14, 18, 22, 26, 31, 36, 41 };
static const u16 sLims[6] = { 60, 45, 90, 67, 150, 112 };
// Its clods (0x0874ACFA): up or down from it by their side; by their step (2, 1, 0) and side, their frames and speeds; their waits;
static const s16 sClodY[2] = { 12, -12 };
static const u16 sClodFrame[6] = { 19, 19, 17, 18, 17, 18 };
static const s32 sClodVX[3] = { F(-1), F(-2), F(-3) };
static const s32 sClodVY[6] = { F(0.5), F(-0.5), F(1), F(-1), F(6), F(-6) };
static const u8 sClodWait[3] = { 2, 2, 2 };
/* Its arms (0x0874B824 on): resting (by its being the bottom one and its level), their spin's waits by its speed, spinning up, their
 * next attack, aiming (two sweeps' speeds), the sweeps' waits by the sweep and their speed, the jab's time, frames and waits (its steps
 * counting down), the punch's speeds and waits (its steps counting down; up or down by the arm); */
static const u8 sRestTime[6] = { 60, 60, 120, 120, 120, 240 };
static const u8 sSpinWait[4] = { 8, 4, 2, 1 };
static const u8 sSpinTime[3] = { 90, 90, 45 };
static const u8 sNextState[4] = { 2, 3, 4, 5 };
static const u8 sAimTime[3] = { 30, 30, 15 };
static const u8 sFastAimTime[3] = { 15, 15, 7 };
static const u8 sSweepWait[4] = { 4, 2, 12, 6 };
static const u8 sJabTime[3] = { 90, 90, 40 };
static const u8 sJabTop[24] = { 60, 30, 24, 18, 12, 6, 36, 0, 42, 12, 42, 0, 48, 18, 48, 0, 54, 24, 54, 0, 60, 30, 60, 0 };
static const u8 sJabBottom[24] = { 120, 90, 84, 78, 72, 66, 96, 0, 102, 72, 102, 0, 108, 78, 108, 0, 114, 84, 114, 0, 120, 90, 120, 0 };
static const u8 sJabWait[24] = { 1, 2, 2, 2, 2, 10, 1, 2, 1, 4, 1, 2, 1, 4, 1, 2, 1, 4, 1, 2, 1, 4, 1, 2 };
static const s32 sPunchX[11] = { F(-3), F(-4), F(-2), F(-1), F(1), F(2), F(4), F(6), F(3), 0, F(-2) };
static const s32 sPunchY[11] = { 0, F(0.25), F(1), F(1.5), F(-4), F(-2), F(-1), 0, 0, 0, 0 };
static const u8 sPunchWait[11] = { 3, 3, 3, 3, 1, 1, 2, 2, 5, 4, 4 };
/* Their frames' looks (0x0874B63E: 0 resting, 1-5 the top one out each way, 6-10 aiming, 11-20 the bottom one's), their boxes (each a
 * square about a spot: hurting Kirby, 0x0874B6D4, and digging, 0x0874B77C, the same), how many (0x0874B6BC); the bottom one's rod by its
 * look (0x0874B614, at its depth 11: none -1); */
static const u8 sFrameLook[136] = {
    0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12,
    13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 17, 17, 17, 17, 17, 17,
    18, 18, 18, 18, 18, 18, 19, 19, 19, 19, 19, 19, 20, 20, 20, 20, 20, 20, 1, 2, 2, 2, 2, 2, 1, 1, 1, 1,
};
static const s8 sArmBox[21][2][3] = {
    { { 0, 0, 6 }, { 0, 0, 0 } }, { { 36, -5, 6 }, { 12, 4, 3 } }, { { 19, -17, 6 }, { 6, -1, 3 } }, { { 0, -23, 6 }, { -1, -1, 3 } },
    { { -20, -17, 6 }, { -8, -1, 3 } }, { { -33, -5, 6 }, { -10, 4, 3 } }, { { 22, 0, 6 }, { 0, 0, 0 } }, { { 10, -6, 6 }, { 0, 0, 0 } },
    { { 0, -11, 6 }, { 0, 0, 0 } }, { { -12, -7, 6 }, { 0, 0, 0 } }, { { -20, 0, 6 }, { 0, 0, 0 } }, { { 36, 5, 6 }, { 12, -3, 3 } },
    { { 20, 20, 6 }, { 6, 2, 3 } }, { { 0, 28, 6 }, { -1, 3, 3 } }, { { -19, 20, 6 }, { -8, 1, 3 } }, { { -33, 5, 6 }, { -13, -3, 3 } },
    { { 19, -1, 6 }, { 0, 0, 0 } }, { { 13, 11, 6 }, { 0, 0, 0 } }, { { 0, 10, 6 }, { 0, 0, 0 } }, { { -11, 10, 6 }, { 0, 0, 0 } },
    { { -21, 0, 6 }, { 0, 0, 0 } },
};
static const u8 sBoxCount[21] = { 1, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1 };
static const s16 sRod[21] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135 };
#define ARM_ROOT_X 8                // Their roots: 8 left of its middle, 24 above and below it;
#define ARM_ROOT_Y 24
#define REST_TIME 120               // Their spin's slowing (+0x28's low half);
#undef F
// Its hurt's jitter (0x0873E184 by its countdown, 0x0873E16C);
static const u8 sJitterOf[24] = { 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0 };
static const s8 sJitter[6][2] = { { 2, 1 }, { -2, -1 }, { -2, 0 }, { 2, 1 }, { 2, -1 }, { -2, 0 } };
/* The dirt's crust (NiDL's 0x08031F3C, drawing an open cell): each of a cell's quarters (top left, top right, bottom left, bottom right)
 * by the cells about it (a 3 x 3 grid, it in the middle, 4: dirt or open; outside the room open), those beside it up or down (1),
 * across (2), both (3), else the one at its corner (4) or none (0) (0x0873A6D4: Tiles()), its tile (0x080D71A0: palettes down by 2,
 * as KatAMExpansionRooms.cs' ShiftPalette()); */
static const u16 sCrustTiles[5][4] = {
    { 0x000, 0x000, 0x000, 0x000 }, { 0x102, 0x103, 0x112, 0x113 }, { 0x104, 0x105, 0x114, 0x115 }, { 0x10A, 0x109, 0x11C, 0x11F },
    { 0x106, 0x107, 0x116, 0x117 },
};
// Hit stars' jitter about it (0x0873EB30, 0x0873EB38);
static const s8 sHitX[8] = { -8, 0, 4, -16, 4, -8, -16, 4 };
static const s8 sHitY[8] = { -16, -20, 0, -16, 4, -16, -16, 0 };

/* Actor; One of NiDL's objects (its fields, by their NiDL offsets): a script (WAIT() resumes it, PC_END: parked), its move and its frame's
 * functions in NiDL's order (Turns(), Follows()); its sprites and its boxes hurting Kirby; */
struct Actor {
    u8 kind;
    u8 state, next;                     // (+0x15, +0x14);
    u8 script;                          // The state whose script is running;
    bool8 restart;                      // Its next state starts as it's next run (NiDL's 0x08006148);
    bool8 hold;                         // Its script held (NiDL's skip mask 1: its hatch's and eye's while it's hurt);
    u16 pc, wait;
    s8 facing, flip;                    // (+0x43; its frame's mirroring);
    s16 frame;                          // (+0x3C; FRAME_HIDDEN none);
    s32 x, y;                           // 16.16 (+0x4C; its arms', from their root; its own, across, on the screen);
    s16 ix, iy;                         // Its pixel (+0x48);
    s32 vx, vy;                         // (+0x54, +0x58);
    s32 v28, v2C, v30, v34;             // (+0x28 on);
    s16 loop, loop2;                    // (+0x6C, +0x6E);
    struct EffectObject *sprite, *sprite2;
    struct ObjectBase *attack, *attack2;
    struct Kirby *inhaler;              // The Kirby pulling it in (a missile: Inhaled());
};

struct HeavyMole {
    struct NidlObject n;
    struct Fight *f;
    bool8 started;
};

typedef char sHeavyMoleFits[sizeof(struct HeavyMole) <= NIDL_OBJECT_SIZE ? 1 : -1];

/* Fight; The fight's state (a task of its own), NiDL's objects, its globals (0x02007D00) and the camera; */
struct Fight {
    struct Object *obj;                 // (NULL: gone, HeavyMoleGone());
    struct Actor a[ACTORS];
    struct EffectObject *body;          // Its sprite (its children's are their own);
    u16 seed;                           // NiDL's random numbers' (0x03000FB4);
    s16 hp;                             // KatAM's object's HP last frame (less now: hit);
    s32 g[10];                          // (g+0 its last attack, 1 its script's step, 2 its treads' speed (now << 16, before), 3 its
                                        // level, 4 its missiles' turn, 5 its clods' side, 6 their wait, 7 its hatch's countdown, 9 its
                                        // HP at first);
    s32 camX16;                         // (+0x1C: the camera's middle across, its fraction kept);
    s32 camSpeed;                       // (+0x18);
    s16 camCX, camCY;                   // NiDL's camera's middle (0x03002398, 0x03001F00);
    s16 camLeft, camTop;                // NiDL's camera (0x03002158) as the frame started (the dig's, the missiles');
    s16 savedX, savedY;                 // Its pixel as its move left it (0x02006190: its hurt's end puts it back);
    s16 lastX, lastY;                   // Its pixel as the frame started (its hit stars');
    bool8 over;                         // Beaten (its defeat under way: KatAM's explosion);
    bool8 flashing;                     // Its hurt flash on;
    bool8 fresh;                        // Hit this frame (its move and steps still run: NiDL's hit comes in its steps);
    u8 guard;                           // Frames it takes no hits yet;
    u8 hurt;                            // (0x02006190 + 0xC: its hurt's countdown);
    s16 hurtFrame;                      // Its frame as it was hit;
    u16 frameCount;                     // (0x03001EA4: its arms' frames, the flashes' clock);
    u8 unk56;
    u16 room;
    u8 pal[PALS];
    u8 dirt[CELLS_W * CELLS_H / 8];     // The room's dirt (a bit a cell: the crust's, Scan()'s);
    u8 queue[ACTORS * 2];               // This frame's turns (NiDL's lists: by group, then those spawned meanwhile) and their groups;
    u8 groups[ACTORS * 2];
    u8 queued;
    bool8 listing;
    struct EffectObject *spare[SPARE_DEBRIS];   // Debris sprites kept hidden for the next debris (making them lags: Destroy());
    u8 spares;
};

/* The scripts: WAIT() resumes them that many frames on; STOP_SCRIPT() parks them (NiDL's 0x08006138); the variables they keep across
 * waits are the Actor's; */
#define SCRIPT switch (a->pc) { case 0:
#define WAIT(n) do { a->wait = (n); a->pc = __LINE__; return; case __LINE__:; } while (0)
#define STOP_SCRIPT() do { a->pc = PC_END; return; } while (0)
#define SCRIPT_END } a->pc = PC_END

// Rand(); NiDL's (0x08002EE8): 0 to n - 1;
static u16 Rand(struct Fight *f, u16 n) {
    f->seed = (f->seed * 61 + 0x579) & 0xFFF;
    return f->seed * n >> 12;
}

static void Stop(struct Actor *a) {
    a->vx = a->vy = 0;
}

// SetFrame(); NiDL's 0x08006338: its frame, mirrored by its facing (NiDL's other frame changes keep its mirroring);
static void SetFrame(struct Actor *a, s16 frame) {
    a->frame = frame;
    a->flip = a->facing;
}

/* SetState(); NiDL's 0x0806395C: its next state (+0x14); Restart(): it starts as it's next run (0x08006148); */
static void SetState(struct Actor *a, u8 state) {
    a->next = state;
}

static void Restart(struct Actor *a) {
    a->restart = TRUE;
}

static void Sound(struct Fight *f, u16 sfx) {
    PlaySfx(&f->obj->base, sfx);
}

/* Their objects: Spawn() takes NiDL's lowest free slot (0x08005904), at (x, y), its script run as it's next run (its turn: Turns());
 * the spawn's facing right; */
static void Destroy(struct Fight *f, s8 i);

/* Group(); An Actor's group (NiDL's master table's, 0x0872FF34: the hit stars 1; its bottom arm and its missiles 2; it 3; its top arm,
 * its other children and the debris 4; each frame, NiDL runs its objects by group, then by slot: Turns()); */
static u8 Group(const struct Actor *a) {
    switch (a->kind) {
    case K_HIT: return 1;
    case K_BOTTOM: case K_MISSILE: case K_MISSILE2: case K_BURST: return 2;
    case K_MOLE: return 3;
    }
    return 4;
}

static void Queue(struct Fight *f, s8 i) {
    if (f->queued >= ARRAY_COUNT(f->queue)) return;
    f->groups[f->queued] = Group(&f->a[i]);
    f->queue[f->queued++] = i;
}

static s8 Spawn(struct Fight *f, u8 kind, s32 x, s32 y) {
    struct Actor *a;
    s8 i;

    for (i = A_MOLE + 1; i < ACTORS && f->a[i].kind != K_NONE; i++) ;
    if (i >= ACTORS) return NONE;
    a = &f->a[i];
    CpuFill32(0, a, sizeof(struct Actor));
    a->kind = kind;
    a->facing = a->flip = 1;
    a->frame = FRAME_HIDDEN;
    a->x = x << 16;
    a->y = y << 16;
    a->ix = x;
    a->iy = y;
    if (f->listing) Queue(f, i);
    return i;
}

static void Release(struct EffectObject **e) {
    if (*e == NULL) return;
    (*e)->flags |= 0x1000;
    *e = NULL;
}

static void LetGoBox(struct ObjectBase **b) {
    if (*b == NULL) return;
    (*b)->flags |= 0x1000;
    *b = NULL;
}

static void Hide(struct EffectObject *e);

static void Destroy(struct Fight *f, s8 i) {
    struct Actor *a;

    if (i <= A_MOLE || i >= ACTORS) return;
    a = &f->a[i];
    if (a->kind == K_NONE) return;
    // (a debris' sprite kept for the next: making one, its task, tiles and all, a cell broken, lags);
    if (a->kind == K_DEBRIS && a->sprite && f->spares < SPARE_DEBRIS) {
        Hide(a->sprite);
        f->spare[f->spares++] = a->sprite;
        a->sprite = NULL;
    }
    Release(&a->sprite);
    Release(&a->sprite2);
    LetGoBox(&a->attack);
    LetGoBox(&a->attack2);
    if (a->inhaler && !a->inhaler->base.header.kind && a->inhaler->inhaling) a->inhaler->inhaling--;
    a->inhaler = NULL;
    a->kind = K_NONE;
}

// Motion(); NiDL's 0x080059D8 (0x080059A0: its speed added, none changing it here; then its pixel);
static void Motion(struct Actor *a) {
    a->x += a->vx;
    a->y += a->vy;
    a->ix = a->x >> 16;
    a->iy = a->y >> 16;
}

/* Its boxes hurting Kirby: attack objects (as King Dedede's hammer: NiDL's 0x08068E04), kind 2, the fight their parent (KatAM's HUD never
 * shows them, nor a Kirby they touch), an Object's size and zeroed past their ObjectBase (KatAM's HUD reads an HP there: none); */
static void AttackMain(void) {
    struct ObjectBase *b = TaskGetStructPtr(gCurTask);

    if (b->flags & 0x1000) {
        TaskDestroy(gCurTask);
        return;
    }
    if (ObjectPreUpdate(b)) return;
    b->flags |= 4 | 0x400;
    if (!(b->flags & 0x200)) SetPointerSomething(b);
}

static struct ObjectBase *NewAttack(struct Fight *f) {
    struct Task *t = TaskCreate(AttackMain, sizeof(struct Object), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct ObjectBase *b = TaskGetStructPtr(t);

    CpuFill32(0, b, sizeof(struct Object));
    ClearObjectBase(b);
    b->header.kind = 2;
    b->parent = f;
    b->roomId = f->room;
    b->unk56 = f->unk56;
    b->unk63 = 1;
    b->flags |= 0x10000140 | 0x600;
    b->unk68 = 0x20000043;
    b->unk5C |= 0x80000;
    ObjectSetBounds(b, 0, 0, 0, 0);
    sub_0803E2B0(b, 0, 0, 0, 0);
    return b;
}

// Box(); A box hurting Kirby at (x, y), (left, top) to (right, bottom) about it, as NiDL's Kirby's box meets it (made the first time);
static void Box(struct Fight *f, struct ObjectBase **b, s32 x, s32 y, s8 left, s8 top, s8 right, s8 bottom) {
    if (*b == NULL) *b = NewAttack(f);
    (*b)->x = x << 8;
    (*b)->y = y << 8;
    sub_0803E2B0(*b, left, top - KIRBY_GROW_Y, right, bottom + KIRBY_GROW_Y);
    (*b)->flags &= ~0x200;
}

static void NoBox(struct ObjectBase *b) {
    if (b) b->flags |= 0x200;
}

/* The dirt: IsDirt() its cells (KatAM's: the room's collision), Dirt() as the fight last saw them (Scan()); Tiles() draws an open cell's
 * crust into the room's tilemap by the cells about it (sCrustOf), Crust() its and the open cells' about it (NiDL's: the quarters facing
 * it: the 5 x 5 about it read once, by rows: Row()), the screen's tiles drawn again; */
static bool32 IsDirt(struct Fight *f, s32 x, s32 y) {
    return NidlCollision(f->unk56)[y * CELLS_W + x] == DIRT;
}

static bool32 Dirt(struct Fight *f, s32 x, s32 y) {
    if (x < 0 || y < 0 || x >= CELLS_W || y >= CELLS_H) return FALSE;
    return (f->dirt[(y * CELLS_W + x) >> 3] >> (x & 7)) & 1;
}

static void SetDirt(struct Fight *f, s32 x, s32 y, bool32 on) {
    if (on) f->dirt[(y * CELLS_W + x) >> 3] |= 1 << (x & 7);
    else f->dirt[(y * CELLS_W + x) >> 3] &= ~(1 << (x & 7));
}

// Row(); The dirt of 5 cells across from x (bits: 0 past the room's sides);
static u32 Row(struct Fight *f, s32 x, s32 y) {
    const u8 *d;
    s32 k = x >> 3;

    if (y < 0 || y >= CELLS_H) return 0;
    d = f->dirt + y * (CELLS_W / 8);
    return (((k >= 0 ? d[k] : 0) | (k + 1 < CELLS_W / 8 ? d[k + 1] : 0) << 8) >> (x & 7)) & 0x1F;
}

// Tiles(); about: the 3 x 3 about it (bit y * 3 + x): each quarter by the cells up or down of it and across, else at its corner;
#define QUARTER(side, across, corner) ((side) + 2 * (across) ? (side) + 2 * (across) : 4 * (corner))
static void Tiles(u16 *map, u16 w, s32 cx, s32 cy, u32 about) {
    u32 up = (about >> 1) & 1, left = (about >> 3) & 1, right = (about >> 5) & 1, down = (about >> 7) & 1;

    map += cy * 2 * w + cx * 2;
    map[0] = sCrustTiles[QUARTER(up, left, about & 1)][0];
    map[1] = sCrustTiles[QUARTER(up, right, (about >> 2) & 1)][1];
    map[w] = sCrustTiles[QUARTER(down, left, (about >> 6) & 1)][2];
    map[w + 1] = sCrustTiles[QUARTER(down, right, (about >> 8) & 1)][3];
}

static void Redraw(struct Fight *f) {
    u8 i;

    for (i = 0; i < gNumKirbys; i++) {
        if (gCurLevelInfo[i].currentRoom != f->room) continue;
        gCurLevelInfo[i].unkC0[1].prevScrollX = 0x7FFF;
        gCurLevelInfo[i].unkC0[1].prevScrollY = 0x7FFF;
    }
}

static void Crust(struct Fight *f, s32 cx, s32 cy) {
    u16 *map = NidlTilemap(f->unk56), w = gCurLevelInfo[f->unk56].roomWidth >> 3;
    u32 r[5];
    s32 x, y, i, j;

    for (j = 0; j < 5; j++) r[j] = Row(f, cx - 2, cy + j - 2);
    for (j = 1; j <= 3; j++)
        for (i = 1; i <= 3; i++) {
            x = cx + i - 2;
            y = cy + j - 2;
            if (x < 0 || y < 0 || x >= CELLS_W || y >= CELLS_H || ((r[j] >> i) & 1)) continue;
            Tiles(map, w, x, y, ((r[j - 1] >> (i - 1)) & 7) | ((r[j] >> (i - 1)) & 7) << 3 | ((r[j + 1] >> (i - 1)) & 7) << 6);
        }
    Redraw(f);
}

/* Open() breaks a cell (its special block's record), its crust; Break() as NiDL's 0x08031374: its debris (its sound: Dig()'s), and one
 * MIRROR_FROM cells across or more broken at its x & 31 too (0x08031EBC: the screen back there after Wrap()); Dig() breaks the cells a
 * box touches (NiDL's 0x0803097C: its spot kept in the room, its cells too) near the camera (0x08031310: their middles inside it and 16
 * more, but below), their sound; Scan() finds the cells on the screen something else broke (Kirby: NiDL's breaks all come this way), their crust,
 * their x & 31 too; */
static void Open(struct Fight *f, s32 x, s32 y) {
    if (!IsDirt(f, x, y)) return;
    sub_08001408(f->unk56, sub_080025AC(f->unk56, x, y), NULL, NULL);
    SetDirt(f, x, y, FALSE);
    Crust(f, x, y);
}

static bool32 Break(struct Fight *f, s32 x, s32 y) {
    if (!IsDirt(f, x, y)) return FALSE;
    Open(f, x, y);
    Spawn(f, K_DEBRIS, x * 16 + 8, y * 16 + 8);
    if (x >= MIRROR_FROM) Open(f, x & 31, y);
    return TRUE;
}

// (4 cells' collision by their dirt bits: dirt or empty; the bytes of those of them on the screen);
#define DIRT_WORD(n, v) (((n) & 1 ? (v) : 0) | ((n) & 2 ? (v) << 8 : 0) | ((n) & 4 ? (v) << 16 : 0) | ((n) & 8 ? (u32)(v) << 24 : 0))
#define DIRT_WORDS(v) { DIRT_WORD(0, v), DIRT_WORD(1, v), DIRT_WORD(2, v), DIRT_WORD(3, v), DIRT_WORD(4, v), DIRT_WORD(5, v), \
    DIRT_WORD(6, v), DIRT_WORD(7, v), DIRT_WORD(8, v), DIRT_WORD(9, v), DIRT_WORD(10, v), DIRT_WORD(11, v), DIRT_WORD(12, v), \
    DIRT_WORD(13, v), DIRT_WORD(14, v), DIRT_WORD(15, v) }
static const u32 sDirtWords[16] = DIRT_WORDS(DIRT);
static const u32 sCellBytes[16] = DIRT_WORDS(0xFF);

static void Scan(struct Fight *f) {
    const u8 *col = NidlCollision(f->unk56), *c, *d;
    s32 x, y, gx, x0 = (f->camLeft >> 4) - 1, y0 = (f->camTop >> 4) - 1, x1 = x0 + SCREEN_W / 16 + 2, y1 = y0 + SCREEN_H / 16 + 2;
    bool32 words = !((u32)col & 3);
    u32 m, n;

    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 >= CELLS_W) x1 = CELLS_W - 1;
    if (y1 >= CELLS_H) y1 = CELLS_H - 1;
    for (y = y0; y <= y1; y++) {
        c = col + y * CELLS_W;
        d = f->dirt + y * (CELLS_W / 8);
        // (by fours: none of those on the screen dirt as last seen, or each as last seen still, dirt or empty; else one by one);
        for (gx = x0 & ~3; gx <= x1; gx += 4) {
            m = 0xF;
            if (gx < x0) m &= 0xF << (x0 - gx);
            if (gx + 3 > x1) m &= 0xF >> (gx + 3 - x1);
            n = (d[gx >> 3] >> (gx & 7)) & m;
            if (!n || (words && !((*(const u32 *)(c + gx) ^ sDirtWords[n]) & sCellBytes[m]))) continue;
            for (x = gx; x < gx + 4; x++) {
                if (!((m >> (x - gx)) & 1) || !((d[x >> 3] >> (x & 7)) & 1) || c[x] == DIRT) continue;
                SetDirt(f, x, y, FALSE);
                Crust(f, x, y);
                if (x >= MIRROR_FROM) Open(f, x & 31, y);
            }
        }
    }
}

static void Dig(struct Fight *f, s32 px, s32 py, s8 top, s8 bottom, s8 left, s8 right) {
    s32 x0, x1, y0, y1, x, y, cx, cy;
    s32 minX = f->camLeft - 16 < 0 ? 0 : f->camLeft - 16, maxX = f->camLeft + SCREEN_W + 16 > ROOM_W ? ROOM_W : f->camLeft + SCREEN_W + 16;
    s32 minY = f->camTop - 16 < 0 ? 0 : f->camTop - 16;
    bool32 broke = FALSE;

    if (px < 0) px = 0;
    else if (px > ROOM_W) px = ROOM_W - 1;
    if (py < 0) py = 0;
    else if (py > ROOM_H) py = ROOM_H - 1;
    x0 = px + left < 0 ? 0 : (px + left) >> 4;
    x1 = (px + right) >> 4;
    y0 = py + top < 0 ? 0 : (py + top) >> 4;
    y1 = (py + bottom) >> 4;
    if (x1 >= ROOM_W / 16) x1 = ROOM_W / 16 - 1;
    if (y1 >= ROOM_H / 16) y1 = ROOM_H / 16 - 1;
    for (y = y0; y <= y1; y++)
        for (x = x0; x <= x1; x++) {
            cx = x * 16 + 8;
            cy = y * 16 + 8;
            if (cx > minX && cx < maxX && cy > minY && Break(f, x, y)) broke = TRUE;
        }
    // (its sound once: each cell's restarting it the same);
    if (broke) Sound(f, SFX_BREAK);
}

// Solid(); NiDL's 0x08021A40: its pixel in a solid cell (dirt: a missile 0x7C isn't fired from inside it);
static bool32 Solid(struct Fight *f, s32 x, s32 y) {
    if (x < 0 || y < 0 || x >= ROOM_W || y >= ROOM_H) return FALSE;
    return IsDirt(f, x >> 4, y >> 4);
}

/* It (0x080AD4B8 on); */

/* MoleScript(); Its treads (frames 4-7, faster as it goes faster: sTreadWait); */
static void MoleScript(struct Fight *f, struct Actor *a) {
    SCRIPT;
    for (;;)
        for (a->loop = 4; a->loop <= 7; a->loop++) {
            SetFrame(a, a->loop);
            WAIT(sTreadWait[f->g[2] >> 16]);
        }
    SCRIPT_END;
}

/* MoleMotion(); 0x080AD650: its speed (not hurt), its height kept 90 inside the camera's bounds; the camera's middle across on by its
 * speed (it rides with it), up or down its height; its pixel (its spot across is on the screen); */
static void MoleMotion(struct Fight *f, struct Actor *a) {
    if (!f->hurt || f->fresh) {
        a->x += a->vx;
        a->y += a->vy;
    }
    if ((a->y >> 16) < CAM_TOP + 90) a->y = ((CAM_TOP + 90) << 16) + 0x8000;
    if ((a->y >> 16) >= CAM_BOTTOM - 90) a->y = ((CAM_BOTTOM - 90) << 16) - 0x8000;
    f->camX16 = (f->camX16 & 0xFFFF) + (f->camCX << 16) + f->camSpeed;
    f->camCX = f->camX16 >> 16;
    f->camCY = a->y >> 16;
    a->ix = f->camCX + (a->x >> 16) - SCREEN_W / 2;
    a->iy = a->y >> 16;
    f->savedX = a->ix;
    f->savedY = a->iy;
}

/* Tread(); 0x080AD788 (every 120 frames): its treads' speed one up going right, one down going left, else back to the middle one (or on
 * from it to the other end from where it was); */
static void Tread(struct Fight *f, struct Actor *a) {
    s32 s = f->g[2] >> 16, t;

    if (a->vx > 0) t = s + 1 > 2 ? 2 : s + 1;
    else if (a->vx < 0) t = s - 1 < 0 ? 0 : s - 1;
    else if (s != 1) t = 1;
    else t = (f->g[2] & 0xFF) == 0 ? 2 : 0;
    f->g[2] = s | t << 16;
}

// Level(); 0x080AD9DC: its level by its HP (below half 2, below five sixths 1);
static void Level(struct Fight *f) {
    s32 hp = f->obj->unk80, max = f->g[9];

    if (hp < max >> 1) f->g[3] = 2;
    else if (hp < max * 5 / 6 + 1) f->g[3] = 1;
    else f->g[3] = 0;
}

/* Pick(); 0x080ADA20: its next attack by chance (never the last one again), its steps' start (one of its choices by the same chance); */
static void Pick(struct Fight *f) {
    const u8 *c = &sChances[f->g[3] * 4];
    s32 r = Rand(f, 255 - c[f->g[0]]), sum = 0, i;

    for (i = 0; i < 3; i++) {
        if (i == f->g[0]) continue;
        sum += c[i];
        if (sum >= r) break;
    }
    f->g[0] = i;
    f->g[1] = sStarts[i * 2 + (r & sMasks[i])];
}

// Edge(); 0x080ADAF8: 1 its height too near the camera's top bound, 2 its bottom (3 both);
static u8 Edge(struct Fight *f, struct Actor *a, u8 n) {
    s32 y = a->y >> 16, lim = sLims[f->g[3] * 2 + n];
    u8 r = 0;

    if (y < CAM_TOP + lim) r = 1;
    if (y > CAM_BOTTOM - lim) r |= 2;
    return r;
}

/* Step(); 0x080AD7F0: its script's next step: its frames and speeds (its level's row); 0 back to its spot across (its level and the
 * camera's speed by its HP); 1 there, its next attack (Pick()); 2, 3 up or down, away from the camera's bound it's near (either way
 * by chance); 4, 5 still; the hatch counting down at 0, 1, 4 and 5; */
static void Step(struct Fight *f, struct Actor *a) {
    u8 s = sScript[f->g[1]], r;
    s32 i = sBase[s] + f->g[3], v, d;

    a->v28 = sDur[s];
    a->vx = sXs[i];
    a->vy = sYs[i];
    switch (s) {
    case 0:
        Level(f);
        f->camSpeed = sCamSpeed[f->g[3]];
        v = sMoveSpeed[f->g[3]];
        d = (sTarget[f->g[3]] << 16) - a->x;
        a->v28 = (d < 0 ? -d : d) / v;
        if ((a->x >> 16) > sTarget[f->g[3]]) v = -v;
        a->vx = v;
        f->g[1]++;
        if (f->g[7] > 0) f->g[7]--;
        break;
    case 1:
        Stop(a);
        a->x = (sTarget[f->g[3]] << 16) + 0x8000;
        Pick(f);
        if (f->g[7] > 0) f->g[7]--;
        break;
    case 2: case 3:
        r = Edge(f, a, s == 3);
        if (r == 2 || ((r == 0 || r == 3) && Rand(f, 2))) a->vy = -a->vy;
        f->g[1]++;
        break;
    case 4: case 5:
        Stop(a);
        if (f->g[7] > 0) f->g[7]--;
        f->g[1]++;
        break;
    default:
        f->g[1]++;
        break;
    }
}

/* MoleTimers(); Its treads' speed every 120 frames (Tread()), its clods' wait (their side the next, 2, 1, 0, as it runs out); */
static void MoleTimers(struct Fight *f, struct Actor *a) {
    if (--a->v2C == 0) {
        Tread(f, a);
        a->v2C = 120;
    }
    if (f->g[6] > 0 && --f->g[6] <= 0 && --f->g[5] < 0) f->g[5] = 2;
}

// MoleFrame(); 0x080AD710 (not hurt): it digs (-18, -12 to 14, 12: 0x0874B538), its steps (Step()), its timers;
static void MoleFrame(struct Fight *f, struct Actor *a) {
    Dig(f, a->ix, a->iy, -12, 12, -18, 14);
    if (--a->v28 <= 0) Step(f, a);
    MoleTimers(f, a);
}

/* HurtStep(); Its hurt's step (NiDL's 0x080662D8, after the turns: Follows()): its flash (Colors()), its jitter, then 0x080ADB90: it
 * digs, its timers; out, back where its move left it, its frame as it was hit; */
static void HurtStep(struct Fight *f, struct Actor *a) {
    u8 i = sJitterOf[f->hurt];

    a->ix += sJitter[i][0];
    a->iy += sJitter[i][1];
    f->hurt--;
    Dig(f, a->ix, a->iy, -12, 12, -18, 14);
    MoleTimers(f, a);
    if (f->hurt) return;
    a->ix = f->savedX;
    a->iy = f->savedY;
    a->frame = f->hurtFrame;
    f->flashing = FALSE;
}

/* Its children; */

/* Follow(); 0x080ADD48 (its drill's, hatch's, eye's: after the turns): on its pixel; its hatch's and eye's scripts held while it's hurt,
 * its eye's frame 13 then; */
static void Follow(struct Fight *f, struct Actor *a) {
    struct Actor *m = &f->a[A_MOLE];

    a->x = m->ix << 16;
    a->ix = m->ix;
    a->y = m->iy << 16;
    a->iy = m->iy;
    if (a->kind == K_DRILL) return;
    a->hold = f->hurt != 0;
    if (a->hold && a->kind == K_EYE) a->frame = 13;
}

// DrillScript(); 0x080ADEFC: its drill turning (frames 8-10, as fast as its treads);
static void DrillScript(struct Fight *f, struct Actor *a) {
    SCRIPT;
    for (;;)
        for (a->loop = 8; a->loop <= 10; a->loop++) {
            a->frame = a->loop;
            WAIT(sTreadWait[f->g[2] >> 16]);
        }
    SCRIPT_END;
}

/* Fire(); 0x080ADDF8: a missile from its hatch, 0x7B or (one in four the first time, one in three after, never twice) 0x7C (none from
 * inside the dirt: 0x08021A40); */
static void Fire(struct Fight *f, struct Actor *a) {
    s32 r;

    Sound(f, SFX_FIRE);
    if (f->g[4] == 0) r = Rand(f, 4);
    else if (f->g[4] == 1) {
        r = 1;
        f->g[4] = 2;
    } else {
        r = Rand(f, 3);
    }
    if (r == 0) {
        f->g[4] = 1;
        if (!Solid(f, a->ix, a->iy)) Spawn(f, K_MISSILE2, a->ix, a->iy);
    } else {
        Spawn(f, K_MISSILE, a->ix, a->iy);
    }
}

/* HatchScript(); 0x080ADCA4: shut (14) while it counts down (g+0x1C), then open (15, 16), a missile (Fire()), shut again; */
static void HatchScript(struct Fight *f, struct Actor *a) {
    SCRIPT;
    for (;;) {
        a->frame = 14;
        while (f->g[7] > 0) WAIT(1);
        Sound(f, SFX_HATCH);
        a->frame = 15;
        WAIT(4);
        a->frame = 16;
        WAIT(48);
        Fire(f, a);
        WAIT(64);
        a->frame = 15;
        WAIT(4);
        a->frame = 14;
        WAIT(2);
        f->g[7] = 3;
    }
    SCRIPT_END;
}

// EyeScript(); 0x080AE038: hidden 97 frames, a blink (11, 12, 11);
static void EyeScript(struct Fight *f, struct Actor *a) {
    SCRIPT;
    for (;;) {
        while (a->v28 >= 0) {
            a->v28--;
            a->frame = FRAME_HIDDEN;
            WAIT(1);
        }
        a->frame = 11;
        WAIT(4);
        a->frame = 12;
        WAIT(4);
        a->frame = 11;
        WAIT(4);
        a->v28 = 96;
    }
    SCRIPT_END;
}

/* ClodsScript(); 0x080AE1F0: once its hatch counts down and their wait (g+0x18) is out, 20 times while it counts down, flung from in
 * front of it up or down (by g+0x14's side, else by turns: three steps, slower each), then a wait of 240; */
static void ClodsScript(struct Fight *f, struct Actor *a) {
    struct Actor *m = &f->a[A_MOLE];
    s32 n, side;

    SCRIPT;
    for (;;) {
        Stop(a);
        a->frame = FRAME_HIDDEN;
        while (f->g[7] <= 0) WAIT(1);
        while (f->g[6] > 0) WAIT(1);
        a->v2C = 20;
        while (f->g[7] > 0) {
            side = f->g[5] ? f->g[5] : (a->v2C & 1) + 1;
            a->v28 = side >> 1;
            a->x = (m->ix - 24) << 16;
            a->y = (m->iy + sClodY[a->v28]) << 16;
            for (a->v30 = 3; a->v30 > 0; ) {
                n = --a->v30;
                a->frame = sClodFrame[n * 2 + a->v28];
                a->vx = sClodVX[n];
                a->vy = sClodVY[n * 2 + a->v28];
                WAIT(sClodWait[a->v30]);
            }
            if (--a->v2C <= 0) break;
        }
        f->g[6] = 240;
    }
    SCRIPT_END;
}

/* Its arms (0x080B1710 on: six states, each a script, 0x0874B5E4, and a function after the turns, 0x0874B5FC: ArmFollow()): spinning at
 * rest, faster as it goes (+0x28: its speed above, its timer below); spinning up; then by chance sweeping out across from one end (two
 * speeds), a jab out and back, a punch away and back; */
#define TOP(a) ((a)->facing == -1)
#define HI(a) ((s16)((a)->v28 >> 16))

// Spin(); Their resting spin's frames (3, 2, 1, 0 on top, 0, 1, 2, 3 below), a step;
static s16 Spin(struct Actor *a, u8 step) {
    return TOP(a) ? 3 - step : step;
}

/* ArmRest(); 0x080B1A1C: its spin no faster than 2, a rest by its level (the bottom one's longer), spinning; */
static void ArmRest(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 0;
    if (HI(a) > 2) a->v28 = (a->v28 & 0xFFFF) + 0x20000;
    a->v2C = sRestTime[(a->facing == 1) * 2 + f->g[3]];
    for (;;)
        for (a->loop = 0; a->loop < 4; a->loop++) {
            a->frame = Spin(a, a->loop);
            WAIT(sSpinWait[HI(a)]);
        }
    SCRIPT_END;
}

/* ArmSpinUp(); 0x080B1C04: spinning, its fastest (3) a frame a step (4, 2 or 1, 5, 0 or 3); */
static void ArmSpinUp(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 1;
    for (;;) {
        if (HI(a) == 3) {
            a->frame = 4;
            WAIT(1);
            a->frame = TOP(a) ? 2 : 1;
            WAIT(1);
            a->frame = 5;
            WAIT(1);
            a->frame = TOP(a) ? 0 : 3;
            WAIT(1);
        } else {
            for (a->loop = 0; a->loop < 4; a->loop++) {
                a->frame = Spin(a, a->loop);
                WAIT(sSpinWait[HI(a)]);
            }
        }
    }
    SCRIPT_END;
}

/* ArmSweep(); 0x080B1D98 (state 2), 0x080B1EF8 (3, faster): a way (+0x34: 0-4) by chance, aiming; then out (+0x30: its speed, its sign
 * the way: from the ends inwards, else by chance) to the end that way, a step at a time (0x080B2058: sSweepWait), and back across to
 * the other; resting again; */
static void ArmSweep(struct Fight *f, struct Actor *a) {
    s32 speed = a->script == 3 ? 2 : 1;

    SCRIPT;
    a->state = a->script;
    a->v34 = Rand(f, 5);
    a->v30 = 0;
    WAIT(a->script == 3 ? sFastAimTime[f->g[3]] : sAimTime[f->g[3]]);
    a->v30 = speed;
    if (a->v34 == 4) a->v30 = -a->v30;
    else if (a->v34 != 0 && Rand(f, 2)) a->v30 = -a->v30;
    for (a->loop = 0; a->loop < 2; a->loop++) {
        WAIT(sSweepWait[a->loop * 2 + speed - 1]);
        if (a->v30 > 0) {
            while (a->v34 <= 3) {
                a->v34++;
                WAIT(sSweepWait[a->loop * 2 + speed - 1]);
            }
        } else {
            while (a->v34 > 0) {
                a->v34--;
                WAIT(sSweepWait[a->loop * 2 + speed - 1]);
            }
        }
        if (a->loop == 0) a->v30 = -a->v30;
    }
    a->v30 = 0;
    WAIT(a->script == 3 ? 1 : 2);
    SetState(a, 0);
    STOP_SCRIPT();
    SCRIPT_END;
}

/* ArmJab(); 0x080B21A0: out and back (its frames' bases, sJabTop or sJabBottom, counting down), no longer than its level's time (+0x30:
 * ArmFollow()); */
static void ArmJab(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 4;
    a->v2C = 24;
    a->v30 = sJabTime[f->g[3]];
    do {
        a->v2C--;
        a->v34 = TOP(a) ? sJabTop[a->v2C] : sJabBottom[a->v2C];
        WAIT(sJabWait[a->v2C]);
    } while (a->v2C > 0);
    SetState(a, 0);
    STOP_SCRIPT();
    SCRIPT_END;
}

/* ArmPunch(); 0x080B20D4: flung away from its root and back (sPunchX, sPunchY, counting down: up for the top one), back on its root; */
static void ArmPunch(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 5;
    a->v2C = 11;
    a->v34 = 0;
    do {
        a->v2C--;
        a->vx = sPunchX[a->v2C];
        a->vy = sPunchY[a->v2C] * a->facing;
        WAIT(sPunchWait[a->v2C]);
    } while (a->v2C > 0);
    Stop(a);
    a->x = a->y = 0;
    SetState(a, 0);
    STOP_SCRIPT();
    SCRIPT_END;
}

static void (*const sArmStates[6])(struct Fight *, struct Actor *) = {
    ArmRest, ArmSpinUp, ArmSweep, ArmSweep, ArmJab, ArmPunch,
};

/* Their frames by the clock (as it goes: odd frames 4 or 5, even 0-3 of each look's six) from a base; */
static s16 Turning(struct Fight *f, s16 base) {
    return f->frameCount & 1 ? base + 4 + ((f->frameCount >> 1) & 1) : base + (f->frameCount & 3);
}

/* ArmFollow(); Each frame after the turns (0x080B1910): its state's function (0x0874B5FC: resting, its spin slower or faster every 16
 * frames as it moves, then spinning up, then the next attack by chance; sweeping, jabbing, punching, its frame by its way; out of its
 * state, the next one's script as it's next run), on its root (its speed moving it from there: 0x080B17D0, 0x080B1830), its boxes
 * (0x080B199C: hurting Kirby, digging); */
static void ArmBoxes(struct Fight *f, struct Actor *a);

static void ArmFollow(struct Fight *f, struct Actor *a) {
    struct Actor *m = &f->a[A_MOLE];
    s32 v = a->v28;

    switch (a->state) {
    case 0:
        if (v & 0xFFFF) {
            a->v28 = --v;
            if (!(v & 15)) {
                if (m->vx > 0 || (m->vx == 0 && m->vy != 0)) {
                    if ((v >> 16) <= 1) a->v28 = v + 0x10000;
                } else if (m->vx < 0 && (v >> 16) != 0) {
                    a->v28 = v - 0x10000;
                }
            }
        } else {
            if (v & 0xFFFF0000) a->v28 = v - 0x10000;
            a->v28 += REST_TIME;
        }
        if (--a->v2C == 0) {
            a->v2C = sSpinTime[f->g[3]];
            SetState(a, 1);
            Restart(a);
        }
        break;
    case 1:
        if (--a->v2C == 0) {
            SetState(a, sNextState[Rand(f, 4)]);
            Restart(a);
        }
        v = a->v28;
        if (v & 0xFFFF) {
            a->v28 = --v;
            if (!(v & 15) && (v >> 16) <= 2) a->v28 = v + 0x10000;
        } else {
            a->v28 = v + REST_TIME;
        }
        break;
    case 2: case 3:
        if (a->next != a->state) {
            Restart(a);
            break;
        }
        a->frame = Turning(f, (a->v30 ? (TOP(a) ? 6 : 66) : (TOP(a) ? 36 : 96)) + a->v34 * 6);
        break;
    case 4:
        if (--a->v30 < 0) SetState(a, 0);
        if (a->next != 4) {
            Restart(a);
            break;
        }
        a->frame = Turning(f, a->v34);
        break;
    case 5:
        if (a->next != 5) Restart(a);
        a->frame = Turning(f, 0);
        break;
    }
    a->x += a->vx;
    a->y += a->vy;
    a->ix = m->ix + (a->x >> 16) - ARM_ROOT_X;
    a->iy = m->iy + (a->y >> 16) + (TOP(a) ? -ARM_ROOT_Y : ARM_ROOT_Y);
    ArmBoxes(f, a);
}

static void ArmBoxes(struct Fight *f, struct Actor *a) {
    u8 k, n, j;
    struct ObjectBase **b;

    if (a->frame < 0 || a->frame >= (s16)ARRAY_COUNT(sFrameLook)) return;
    k = sFrameLook[a->frame];
    n = sBoxCount[k];
    for (j = 0; j < 2; j++) {
        const s8 *box = sArmBox[k][j];

        b = j ? &a->attack2 : &a->attack;
        if (j >= n) {
            NoBox(*b);
            continue;
        }
        Box(f, b, a->ix + box[0], a->iy + box[1], -box[2], -box[2], box[2], box[2]);
        Dig(f, a->ix + box[0], a->iy + box[1], -box[2], box[2], -box[2], box[2]);
    }
}

/* Its missiles (0x080B22F8, 0x080B2418): flying left (2 a frame), turning (4-7, 0x7C's 12-15) a while (1-32 turns), a look about (8, 13,
 * 10, 7; 0x7C's 8, 5, 10, 15), turning again; their boxes (-6, -3 to 6, 3: 0x0874BF34), digging (-8, -6 to 8, 6: 0x0874C108); gone 64
 * off the screen (0x08065160); */
static void MissileScript(struct Fight *f, struct Actor *a) {
    s16 base = a->kind == K_MISSILE2 ? 12 : 4;

    SCRIPT;
    a->facing = -1;
    a->vx = -0x20000;
    a->loop2 = Rand(f, 32) + 1;
    for (a->loop = 0; a->loop < a->loop2; a->loop++) {
        SetFrame(a, base);
        WAIT(2);
        a->frame++;
        WAIT(2);
        a->frame++;
        WAIT(2);
        a->frame++;
        WAIT(2);
    }
    SetFrame(a, 8);
    WAIT(2);
    SetFrame(a, a->kind == K_MISSILE2 ? 5 : 13);
    WAIT(2);
    SetFrame(a, 10);
    WAIT(2);
    SetFrame(a, a->kind == K_MISSILE2 ? 15 : 7);
    WAIT(2);
    for (;;) {
        SetFrame(a, base);
        WAIT(2);
        a->frame++;
        WAIT(2);
        a->frame++;
        WAIT(2);
        a->frame++;
        WAIT(2);
    }
    SCRIPT_END;
}

static void MissileFrame(struct Fight *f, struct Actor *a) {
    Box(f, &a->attack, a->ix, a->iy, -6, -3, 6, 3);
    Dig(f, a->ix, a->iy, -6, 6, -8, 8);
}

static bool32 InView(struct Fight *f, struct Actor *a) {
    return a->ix > f->camLeft - ALIVE_MARGIN && a->ix < f->camLeft + SCREEN_W + ALIVE_MARGIN && a->iy > f->camTop - ALIVE_MARGIN
        && a->iy < f->camTop + SCREEN_H + ALIVE_MARGIN;
}

/* Touched(); Its box hurt a Kirby (KatAM's, marking it, at its frame's end; NiDL's Kirby finds it in his next turn); Burst(); NiDL's
 * reaction (0x0806A344, its descriptor 0x0874BFE0's: 0x0806B1A8): still where it is, its box gone, NiDL's shared burst, its sound;
 * BurstStep(); its frames (0x0874C9D8's 0-16): its first twice, out four after its eleventh, gone after its seventeenth; */
/* Inhaled(); A missile in a Kirby's inhale (nidl.c's NidlInhaler(), KatAM's box; NiDL's 0x7B and 0x7C too, the 0x7B in sb/hmI):
 * pulled in, harmless, faster and faster (as nidl.c's NidlInhaled()), swallowed: a mouthful that gives nothing, spat back as a star
 * (NiDL's takes 10 off it); he stops first, it's gone in a puff; TRUE while it's his; */
static bool32 Inhaled(struct Fight *f, struct Actor *a) {
    struct Kirby *k = a->inhaler;
    s32 x, y;

    if (k == NULL) {
        if (a->attack == NULL || (k = NidlInhaler(a->attack)) == NULL) return FALSE;
        if (!k->base.header.kind) k->inhaling++;
        a->inhaler = k;
        NoBox(a->attack);
        a->pc = PC_END;
        Stop(a);
        a->v28 = 0;
        a->v2C = (a->ix << 8) - k->base.x;
        a->v30 = (a->iy << 8) - k->base.y;
    }
    if (!NidlInhaling(k, TRUE)) {
        CreateEffectObject(a->attack, 0, 0x292, 0);
        Destroy(f, a - f->a);
        return TRUE;
    }
    a->v28 += INHALE_PULL;
    if (a->v2C > 0) a->v2C -= a->v28;
    else a->v2C += a->v28;
    a->v30 -= a->v30 / 8;
    x = k->base.x + a->v2C;
    y = k->base.y + a->v30;
    a->ix = x >> 8;
    a->iy = y >> 8;
    a->x = x << 8;
    a->y = y << 8;
    a->attack->x = x;
    a->attack->y = y;
    if (a->v28 > INHALE_PULL && abs(a->v2C) < INHALE_CATCH) {
        a->inhaler = NULL;
        NidlSwallow(k, KIRBY_ABILITY_NORMAL);
        Destroy(f, a - f->a);
    }
    return TRUE;
}

static bool32 Touched(struct ObjectBase *b) {
    return b && (b->flags & 0x40000) && b->unk6C && ((struct ObjectBase *)b->unk6C)->header.kind == 0;
}

static void Burst(struct Fight *f, struct Actor *a) {
    LetGoBox(&a->attack);
    Release(&a->sprite);
    Release(&a->sprite2);
    a->kind = K_BURST;
    a->loop = 0;
    a->pc = PC_END;
    Stop(a);
    Sound(f, SFX_BURST);
}

static void BurstStep(struct Fight *f, struct Actor *a) {
    s16 k = a->loop++;

    if (k >= 22) {
        Destroy(f, a - f->a);
        return;
    }
    a->frame = k < 2 ? 0 : k < 12 ? k - 1 : k < 16 ? FRAME_HIDDEN : k - 5;
}

// HitScript(); 0x0806CB64 (type 0x8E): its hit stars, 0 and 1 by turns (2 frames each) 8 times about it (HitFrame());
static void HitScript(struct Fight *f, struct Actor *a) {
    SCRIPT;
    for (a->loop = 0; a->loop <= 7; a->loop++) {
        a->frame = 0;
        WAIT(2);
        a->frame++;
        WAIT(2);
    }
    Destroy(f, a - f->a);
    return;
    SCRIPT_END;
}

// HitFrame(); 0x0806CBD4: on it (its pixel as the frame started), about it by turns;
static void HitFrame(struct Fight *f, struct Actor *a) {
    u8 k = a->loop & 7;

    a->ix = f->lastX + sHitX[k];
    a->iy = f->lastY + sHitY[k];
}

// DebrisScript(); 0x08030254 (type 0xEC): a broken cell's debris (0, 1, 2, 3, 4), gone;
static void DebrisScript(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->frame = 0;
    WAIT(2);
    a->frame = 1;
    WAIT(1);
    a->frame = 2;
    WAIT(1);
    a->frame = 3;
    WAIT(2);
    a->frame = 4;
    WAIT(1);
    Destroy(f, a - f->a);
    return;
    SCRIPT_END;
}

/* Process(); An Actor's turn (in NiDL's order: Turns()): its script (its next state's, restarted; none held), its move, its frame's
 * function; it hurt, its steps held (its move without its speed: MoleMotion()); */
static void Process(struct Fight *f, s8 i) {
    struct Actor *a = &f->a[i];

    if (a->kind == K_NONE) return;
    if ((a->kind == K_MISSILE || a->kind == K_MISSILE2) && Inhaled(f, a)) return;
    if ((a->kind == K_MISSILE || a->kind == K_MISSILE2) && Touched(a->attack)) Burst(f, a);
    if (a->restart) {
        a->restart = FALSE;
        a->script = a->next;
        a->pc = 0;
        a->wait = 0;
    }
    if (!a->hold && a->pc != PC_END && (!a->wait || !--a->wait)) {
        switch (a->kind) {
        case K_MOLE: MoleScript(f, a); break;
        case K_TOP: case K_BOTTOM: sArmStates[a->script < 6 ? a->script : 0](f, a); break;
        case K_DRILL: DrillScript(f, a); break;
        case K_HATCH: HatchScript(f, a); break;
        case K_EYE: EyeScript(f, a); break;
        case K_CLODS: ClodsScript(f, a); break;
        case K_MISSILE: case K_MISSILE2: MissileScript(f, a); break;
        case K_HIT: HitScript(f, a); break;
        case K_DEBRIS: DebrisScript(f, a); break;
        }
    }
    if (a->kind == K_NONE) return;
    switch (a->kind) {
    case K_MOLE:
        MoleMotion(f, a);
        if (!f->hurt || f->fresh) MoleFrame(f, a);
        break;
    case K_DRILL: case K_HATCH: case K_CLODS:
        Motion(a);
        break;
    case K_MISSILE: case K_MISSILE2:
        Motion(a);
        MissileFrame(f, a);
        break;
    case K_HIT:
        HitFrame(f, a);
        break;
    case K_BURST:
        BurstStep(f, a);
        break;
    }
}

/* Turns(); The Actors' turns in NiDL's order (0x08005228): by group (Group()), each by slot; then those spawned meanwhile (NiDL's never
 * run in the turns they're spawned in), by group, and so on; */
static void Turns(struct Fight *f) {
    const struct Actor *a;
    u8 g, q, from, to, n, of[ACTORS], at[5];
    s32 i;

    // (the first list sorted at once: each group counted, then each Actor put at its group's next);
    for (g = 0; g <= 4; g++) at[g] = 0;
    for (i = 0, a = f->a; i < ACTORS; i++, a++) {
        g = a->kind != K_NONE ? Group(a) : 0;
        of[i] = g;
        at[g]++;
    }
    for (g = 1, q = 0; g <= 4; g++) {
        n = at[g];
        at[g] = q;
        q += n;
    }
    f->queued = q;
    for (i = 0; i < ACTORS; i++) {
        if (!of[i]) continue;
        q = at[of[i]]++;
        f->groups[q] = of[i];
        f->queue[q] = i;
    }
    f->listing = TRUE;
    for (from = 0; from < f->queued; from = to) {
        to = f->queued;
        if (!from) {
            for (q = 0; q < to; q++) Process(f, f->queue[q]);
            continue;
        }
        for (g = 1; g <= 4; g++)
            for (q = from; q < to; q++)
                if (f->groups[q] == g) Process(f, f->queue[q]);
    }
    f->listing = FALSE;
}

/* Follows(); Their functions after the turns (NiDL's +8, by group then in their turns' order): its hurt's step (first: both its arms
 * on it shaken, its bottom one's group though before its, as recorded: sb/hmI); its arms' states, roots and boxes; its drill, hatch and
 * eye on it; */
static void Follows(struct Fight *f) {
    u8 g, q;

    if (f->hurt) {
        // (its frame kept as it was hit: its treads' this turn, NiDL's hit coming after them);
        if (f->fresh) f->hurtFrame = f->a[A_MOLE].frame;
        HurtStep(f, &f->a[A_MOLE]);
    }
    for (g = 1; g <= 4; g++)
        for (q = 0; q < f->queued; q++) {
            struct Actor *a = &f->a[f->queue[q]];

            if (f->groups[q] != g) continue;
            switch (a->kind) {
            case K_TOP: case K_BOTTOM:
                ArmFollow(f, a);
                break;
            case K_DRILL: case K_HATCH: case K_EYE:
                Follow(f, a);
                break;
            }
        }
}

/* Wrap(); NiDL's 0x08026308: the camera's left past 767, the camera and everything 512 back (NiDL's: every x & 0x1FF; the dirt there
 * the same: Break()'s mirror): the screen's cells (Keep()), its objects, the Kirbys and KatAM's objects in the room (Shift()), the
 * cameras (the screen's tiles drawn again); */
static void Shift(s32 *x) {
    if (*x >= 0) *x &= 0x1FFFF;
}

/* Keep(); The cells on the screen as they were 512 on (NiDL's screen tiles aren't drawn again: its screen's map is 512 across, the
 * same; the room's cells there have the first time through's holes too, kept off the screen's), their crust too; */
static void Keep(struct Fight *f) {
    u16 *map = NidlTilemap(f->unk56);
    u8 *col = NidlCollision(f->unk56);
    u16 w = gCurLevelInfo[f->unk56].roomWidth >> 3;
    s32 x, y, i, x0 = f->camLeft >> 4, y0 = f->camTop < 0 ? 0 : f->camTop >> 4;
    s32 x1 = (f->camLeft + SCREEN_W - 1) >> 4, y1 = (f->camTop + SCREEN_H - 1) >> 4;

    if (y1 >= CELLS_H) y1 = CELLS_H - 1;
    for (y = y0; y <= y1; y++)
        for (x = x0; x <= x1; x++) {
            col[y * CELLS_W + x] = col[y * CELLS_W + x + 32];
            SetDirt(f, x, y, Dirt(f, x + 32, y));
            for (i = 0; i < 4; i++) map[(y * 2 + (i >> 1)) * w + x * 2 + (i & 1)] = map[(y * 2 + (i >> 1)) * w + x * 2 + 64 + (i & 1)];
        }
    Redraw(f);
}

static void Wrap(struct Fight *f) {
    struct Task *t = &gTasks[0];
    u8 n, i;

    if (f->camLeft <= WRAP_AFTER) return;
    f->camCX &= 0x1FF;
    f->camLeft &= 0x1FF;
    Keep(f);
    for (i = 0; i < ACTORS; i++) {
        f->a[i].x &= 0x01FFFFFF;
        f->a[i].ix &= 0x1FF;
    }
    f->savedX &= 0x1FF;
    f->lastX &= 0x1FF;
    for (i = 0; i < gNumKirbys; i++) {
        struct LevelInfo *l = &gCurLevelInfo[i];

        if (gKirbys[i].base.roomId == f->room) Shift(&gKirbys[i].base.x);
        if (l->currentRoom != f->room) continue;
        Shift(&l->viewportPosition.x);
        Shift(&l->viewportPositionUnmodified.x);
        l->unkC0[1].prevScrollX = 0x7FFF;
        l->unkC0[1].prevScrollY = 0x7FFF;
    }
    for (n = 0; n < MAX_TASK_NUM && t->next; n++) {
        struct ObjectBase *b;

        t = (struct Task *)(IWRAM_START + t->next);
        if (t->dtor != ObjectDestroy && t->dtor != ObjectBaseDestroy && t->dtor != NidlDestroy) continue;
        b = TaskGetStructPtr(t);
        if (b->header.kind == 3) {
            struct EffectObject *e = (struct EffectObject *)b;

            if (e->roomId == f->room) Shift(&e->x);
        } else if (b->header.kind != 0 && b->roomId == f->room) {
            Shift(&b->x);
        }
    }
}

/* Camera(); The arena's screen where NiDL's camera is next frame (KatAM sets its cameras as its frame starts, before its objects move,
 * from the arena's lock; the next frame's move: its speed, unless it's hurt, the camera's speed): Kirby kept PUSH inside its sides; */
static void Camera(struct Fight *f) {
    struct Actor *a = &f->a[A_MOLE];
    s32 x16 = (f->camX16 & 0xFFFF) + (f->camCX << 16) + f->camSpeed, y = f->hurt ? a->y : a->y + a->vy;

    if ((y >> 16) < CAM_TOP + 90) y = ((CAM_TOP + 90) << 16) + 0x8000;
    if ((y >> 16) >= CAM_BOTTOM - 90) y = ((CAM_BOTTOM - 90) << 16) - 0x8000;
    ArenaScroll(f->obj, ((x16 >> 16) - SCREEN_W / 2) << 8, ((y >> 16) - SCREEN_H / 2) << 8, PUSH);
}

/* Hit(); A hit took HP off it (KatAM's, between frames; NiDL's in its steps: 0x080ADB58): its hit stars, its hurt (its move and steps
 * still run this frame, as NiDL's: the hit comes in them), no more hits for GUARD_TIME frames; */
static void Hit(struct Fight *f) {
    struct Actor *a = &f->a[A_MOLE];

    f->guard = GUARD_TIME;
    f->obj->base.flags |= 0x8000;
    if (f->hurt) return;
    Spawn(f, K_HIT, a->ix, a->iy);
    f->hurt = HURT_TIME;
    f->fresh = TRUE;
    f->flashing = TRUE;
}

/* Body(); Its box on KatAM's object, where it is (NiDL's 0x0874B3E0: -20, -16 to 16, 16): hit, touching hurts; */
static void Body(struct Fight *f) {
    struct Object *obj = f->obj;
    struct Actor *a = &f->a[A_MOLE];

    obj->base.x = a->ix << 8;
    obj->base.y = a->iy << 8;
    sub_0803E2B0(&obj->base, -20, -16, 16, 16);
    if (f->over) obj->base.flags |= 0x200;
    else obj->base.flags &= ~0x200;
}

/* Their sprites (KatAM's: their tiles, NiDL's frames as KatAMExpansion.cs lays them out; mirrored by NiDL's mirroring) and their
 * palettes' slots (loaded under their keys, as their frames have them, the local player in the room); */
static bool32 Watched(struct Fight *f) {
    return gKirbys[gLocalPlayerId].base.roomId == f->room;
}

static void SpriteMain(void) {
    struct EffectObject *e = TaskGetStructPtr(gCurTask);

    if (e->flags & 0x1000) TaskDestroy(gCurTask);
}

static struct EffectObject *Sprite(u16 room, u16 tiles, u8 frame, u8 order) {
    struct Task *t = TaskCreate(SpriteMain, sizeof(struct EffectObject), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct EffectObject *e = TaskGetStructPtr(t);

    ClearEffectObject(e);
    e->header.kind = 3;
    e->roomId = room;
    e->parent = NULL;
    e->flags |= 0x400;
    EffectObjectInitSprite(e, &e->sprite, tiles, gNidlData.bosses[NIDL_HEAVY_MOLE].animId, frame, order);
    return e;
}

static void Draw(struct EffectObject *e, s32 x, s32 y, u8 frame, u8 palId, s8 flip, u8 order) {
    e->x = x;
    e->y = y;
    e->sprite.variant = frame;
    e->sprite.palId = palId;
    e->sprite.unk14 = order << 6;
    e->flags = (e->flags & ~0x401) | 4 | (flip == -1 ? 0 : 1);
    e->objBase55 = -1;
    EffectObjectPostUpdate(e);
}

static void Hide(struct EffectObject *e) {
    if (e) e->flags |= 0x400;
}

static void Show(struct Fight *f, struct EffectObject **e, u8 tiles, s32 x, s32 y, u8 frame, u8 pal, s8 flip, u8 order) {
    if (*e == NULL) *e = Sprite(f->room, tiles, frame, order);
    Draw(*e, x << 8, y << 8, frame, f->pal[pal], flip, order);
}

static u8 Palette(u16 room, u16 key, u8 frame) {
    u8 palId = sub_0803DF24(key);

    if (palId == 0xFF) {
        if (gKirbys[gLocalPlayerId].base.roomId != room) return 0;
        sub_0803DFAC(key, frame);
        palId = sub_0803DF24(key);
    }
    return palId == 0xFF ? 0 : palId;
}

static void Palettes(struct Fight *f) {
    u16 animId = gNidlData.bosses[NIDL_HEAVY_MOLE].animId;
    const u8 *fx = gNidlHeavyMole.fx;

    if (!f->pal[PAL_BODY]) f->pal[PAL_BODY] = Palette(f->room, animId, fx[G_BODY]);
    if (!f->pal[PAL_ARMS]) f->pal[PAL_ARMS] = Palette(f->room, animId | KEY_ARMS, fx[G_DRILL]);
    if (!f->pal[PAL_MISSILE]) f->pal[PAL_MISSILE] = Palette(f->room, animId | KEY_MISSILE, fx[G_MISSILE]);
    if (!f->pal[PAL_CLODS]) f->pal[PAL_CLODS] = Palette(f->room, animId | KEY_CLODS, fx[G_CLODS]);
    if (!f->pal[PAL_COMMON]) f->pal[PAL_COMMON] = Palette(f->room, animId | KEY_COMMON, fx[G_HIT]);
}

// Colors(); Its palettes this frame: its hurt flash's colours (NiDL's 0x08066480: the clock's 2 frames in 4, its first two), else its own;
static void SetColors(u8 palId, const u16 *colors) {
    u16 *slot;
    u8 i;

    if (!palId) return;
    slot = &gObjPalette[palId * 16];
    for (i = 1; i < 16 && slot[i] == colors[i]; i++) ;
    if (i == 16) return;
    for (i = 0; i < 16; i++) slot[i] = colors[i];
    sub_0803D280(palId * 16, 16);
    gMainFlags |= MAIN_FLAG_OBJ_PALETTE_SYNC_ENABLE;
}

static void Colors(struct Fight *f, bool32 flash) {
    u8 p;

    if (!Watched(f)) return;
    for (p = PAL_BODY; p <= PAL_CLODS; p++)
        SetColors(f->pal[p], flash && p <= PAL_ARMS ? gNidlHeavyMole.flash + p * 16 : gNidlHeavyMole.colors[p]);
}

/* DrawActor(); An Actor's sprites: its body, its children (its bottom arm's rod a second sprite), its missiles (0x7B two: its third
 * palette's pieces, its fourth's), the hit stars, the debris; */
static void DrawActor(struct Fight *f, struct Actor *a) {
    const u8 *fx = gNidlHeavyMole.fx, *tiles = gNidlHeavyMole.tiles;
    s16 rod;

    if (a->frame < 0 || !Watched(f)) {
        Hide(a->sprite);
        Hide(a->sprite2);
        return;
    }
    switch (a->kind) {
    case K_TOP: case K_BOTTOM:
        Show(f, &a->sprite, tiles[G_ARMS], a->ix, a->iy, fx[G_ARMS] + a->frame, PAL_ARMS, 1, a->kind == K_TOP ? ORDER_TOP : ORDER_BOTTOM);
        rod = a->kind == K_BOTTOM && a->frame < (s16)ARRAY_COUNT(sFrameLook) ? sRod[sFrameLook[a->frame]] : -1;
        if (rod < 0) Hide(a->sprite2);
        else Show(f, &a->sprite2, tiles[G_ARMS], a->ix, a->iy, fx[G_ARMS] + rod, PAL_ARMS, 1, ORDER_TOP);
        break;
    case K_DRILL:
        Show(f, &a->sprite, tiles[G_DRILL], a->ix, a->iy, fx[G_DRILL] + a->frame - 8, PAL_ARMS, 1, ORDER_CLODS);
        break;
    case K_HATCH:
        Show(f, &a->sprite, tiles[G_HATCH], a->ix, a->iy, fx[G_HATCH] + a->frame - 14, PAL_BODY, 1, ORDER_EYE);
        break;
    case K_EYE:
        Show(f, &a->sprite, tiles[G_EYE], a->ix, a->iy, fx[G_EYE] + a->frame - 11, PAL_BODY, 1, ORDER_EYE);
        break;
    case K_CLODS:
        Show(f, &a->sprite, tiles[G_CLODS], a->ix, a->iy, fx[G_CLODS] + a->frame - 17, PAL_CLODS, 1, ORDER_CLODS);
        break;
    case K_MISSILE:
        Show(f, &a->sprite, tiles[G_MISSILE], a->ix, a->iy, fx[G_MISSILE] + a->frame, PAL_MISSILE, a->flip, ORDER_TOP);
        Show(f, &a->sprite2, tiles[G_MISSILE_B], a->ix, a->iy, fx[G_MISSILE_B] + a->frame, PAL_CLODS, a->flip, ORDER_TOP);
        break;
    case K_MISSILE2:
        Show(f, &a->sprite, tiles[G_MISSILE2], a->ix, a->iy, fx[G_MISSILE2] + a->frame, PAL_CLODS, a->flip, ORDER_TOP);
        break;
    case K_HIT:
        Show(f, &a->sprite, tiles[G_HIT], a->ix, a->iy, fx[G_HIT] + (a->frame & 1), PAL_COMMON, 1, ORDER_MOLE);
        break;
    case K_DEBRIS:
        if (a->sprite == NULL && f->spares) a->sprite = f->spare[--f->spares];
        Show(f, &a->sprite, tiles[G_DEBRIS], a->ix, a->iy, fx[G_DEBRIS] + a->frame, PAL_COMMON, 1, ORDER_DEBRIS);
        break;
    case K_BURST:
        Show(f, &a->sprite, tiles[G_BURST], a->ix, a->iy, fx[G_BURST] + a->frame, PAL_COMMON, 1, ORDER_MOLE);
        break;
    }
}

static void HeavyMoleMain(struct Object *obj);

// LetGo(); Its children gone (their sprites, their boxes);
static void LetGo(struct Fight *f) {
    s8 i;

    for (i = A_MOLE + 1; i < ACTORS; i++) Destroy(f, i);
    while (f->spares) Release(&f->spare[--f->spares]);
}

/* FightMain(); The fight's task: its sprites and its children's, its palettes; as KatAM's explosion takes it, its missiles, clods, hit
 * stars and debris gone (its frame held, its other children with it, as NiDL's: 0x080B2294, 0x080ADE98); gone with it; */
static void FightMain(void) {
    struct Fight *f = TaskGetStructPtr(gCurTask);
    struct Object *obj = f->obj;
    struct Actor *a = &f->a[A_MOLE];
    u8 i;

    if (obj == NULL || (obj->base.flags & 0x1000)) {
        LetGo(f);
        Release(&f->body);
        if (obj) ((struct HeavyMole *)obj)->f = NULL;
        TaskDestroy(gCurTask);
        return;
    }
    Palettes(f);
    if (obj->unk78 != HeavyMoleMain && !f->over) {
        // (its killing blow: KatAM's defeat, its freeze and explosion; NiDL's 0x080ADBF0, 0x0806A344);
        f->over = TRUE;
        f->flashing = FALSE;
        if (f->hurt) {
            a->ix = f->savedX;
            a->iy = f->savedY;
            f->hurt = 0;
        }
        for (i = A_MOLE + 1; i < ACTORS; i++) {
            struct Actor *c = &f->a[i];

            if (c->kind == K_EYE) c->frame = 13;
            NoBox(c->attack);
            NoBox(c->attack2);
            if (c->kind == K_MISSILE || c->kind == K_MISSILE2 || c->kind == K_CLODS || c->kind == K_HIT || c->kind == K_DEBRIS
                || c->kind == K_BURST)
                Destroy(f, i);
        }
    }
    Colors(f, f->flashing && (f->frameCount & 2));
    if (!Watched(f) || a->frame < 0) Hide(f->body);
    else Show(f, &f->body, gNidlHeavyMole.tiles[G_BODY], a->ix, a->iy, gNidlHeavyMole.fx[G_BODY] + a->frame, PAL_BODY, 1, ORDER_MOLE);
    for (i = A_MOLE + 1; i < ACTORS; i++) {
        struct Actor *c = &f->a[i];

        if (c->kind != K_NONE) DrawActor(f, c);
    }
}

/* The fight (HeavyMoleMain()): NiDL's each frame: a hit taken first (Hit(): KatAM's come between frames); each of NiDL's objects in its
 * order (Turns(): by group, then slot; its steps held while it's hurt), their functions after (Follows(): its hurt's step, its arms', its
 * children on it), the camera as it left it (back 512 past 767: Wrap()), its missiles gone off the screen, the arena's screen on NiDL's
 * camera (ArenaScroll(): Kirby kept on it), KatAM's object where it is (Body()):
 * • It holds the right of the screen (NiDL's spot 176 on it, 128-208 as its HP goes down), digging (-18, -12 to 14, 12) as the screen
 *   scrolls right with it (from 120 frames in: a quarter to three eighths of a pixel a frame by its HP), the screen up and down with it
 *   (its middle its height): by turns back to its spot, then an attack by chance (Pick()): up or down, still, back and forth, a zigzag
 *   (its speeds by its HP); its treads and drill turning faster as it goes right (Tread()). Its hatch counts its steps down from 3,
 *   then opens and fires a missile (flying left: 0x7C one in three or four, never twice); meanwhile its dirt clods fly out in front of it.
 *   Its arms spin, faster as it moves; then by chance one sweeps out (aiming, then across, slower or faster), jabs or punches; their
 *   balls hurt Kirby (and dig), as do its missiles (which dig too) and touching it.
 * • Hit (KatAM's attacks: KatAM's bosses take only the stronger ones), it flashes in its hurt colours, jittering, its hit stars about it,
 *   its steps held 23 frames (the screen goes on, it with it), its hatch and eye too (its eye shut); no more hits for 31 frames. 60 HP
 *   (NiDL's 60).
 * • The screen pushes Kirby along (9.5 inside its sides, as NiDL's: traced); he falls off its bottom (the arena's). Every 512 pixels the
 *   room's (1024 x 512: KatAM's own) the screen goes back to where it was (NiDL's), the dirt there dug as it was (Break()), Kirby, it
 *   and everything with it; */
static void HeavyMoleMain(struct Object *obj) {
    struct HeavyMole *m = (struct HeavyMole *)obj;
    struct Fight *f = m->f;
    s8 i;

    NidlUpdate(&m->n);
    obj->base.flags |= 4 | 0x400;
    obj->kirby3 = FindTargetKirby(&obj->base);
    if (f == NULL) return;
    // (the HP bar goes to whatever is hit; it takes it back);
    sub_08034E14(obj);
    obj->base.xspeed = obj->base.yspeed = 0;
    f->frameCount++;
    if (f->guard && !--f->guard) obj->base.flags &= ~0x8000;
    if (!f->over && obj->unk80 < f->hp) {
        // (one through all the same: its HP back);
        if (f->guard) obj->unk80 = f->hp;
        else Hit(f);
    }
    f->hp = obj->unk80;
    f->lastX = f->a[A_MOLE].ix;
    f->lastY = f->a[A_MOLE].iy;
    Turns(f);
    Follows(f);
    f->fresh = FALSE;
    f->camLeft = f->camCX - SCREEN_W / 2;
    f->camTop = f->camCY - SCREEN_H / 2;
    Wrap(f);
    for (i = A_MOLE + 1; i < ACTORS; i++) {
        struct Actor *a = &f->a[i];

        if ((a->kind == K_MISSILE || a->kind == K_MISSILE2 || a->kind == K_HIT) && a->frame >= 0 && !InView(f, a)) Destroy(f, i);
    }
    Scan(f);
    Camera(f);
    Body(f);
}

// HeavyMoleGone(); Its object's gone (nidl.c's NidlDestroy()): the fight's task lets go of it;
void HeavyMoleGone(struct Object *obj) {
    struct HeavyMole *m = (struct HeavyMole *)obj;

    if (m->f) m->f->obj = NULL;
    m->f = NULL;
}

/* HeavyMoleStart(); The fight starts (NidlBossStart(): Kirby on its screen, come in at the pocket), its task with it: NiDL's 0x080AD4B8:
 * it on NiDL's spot (the camera's then: START_CX, START_CY), its steps from 120 frames on, its globals (its treads' speed the fastest,
 * its clods' wait 240, its hatch's countdown 3, its HP), its children (its arms, drill, hatch, eye, clods); */
void HeavyMoleStart(struct Object *obj) {
    struct HeavyMole *m = (struct HeavyMole *)obj;
    static const u8 sChildren[6] = { K_TOP, K_BOTTOM, K_DRILL, K_HATCH, K_EYE, K_CLODS };
    struct Task *t;
    struct Fight *f;
    struct Actor *a;
    s32 x, y;
    s8 i;
    u8 c;

    ObjectSetFunc(obj, -1, HeavyMoleMain);
    if (m->started) return;
    m->started = TRUE;
    t = TaskCreate(FightMain, sizeof(struct Fight), 0x3500, TASK_USE_EWRAM, NULL);
    f = m->f = TaskGetStructPtr(t);
    CpuFill32(0, f, sizeof(struct Fight));
    f->obj = obj;
    f->room = obj->base.roomId;
    f->unk56 = obj->base.unk56;
    f->seed = Rand16() & 0xFFF;
    f->hp = obj->unk80;
    f->g[2] = 0x20000;
    f->g[6] = 240;
    f->g[7] = 3;
    f->g[9] = obj->unk80;
    f->camCX = START_CX;
    f->camCY = START_CY;
    f->camX16 = START_CX << 16;
    f->camLeft = START_CX - SCREEN_W / 2;
    f->camTop = START_CY - SCREEN_H / 2;
    a = &f->a[A_MOLE];
    a->kind = K_MOLE;
    a->facing = a->flip = 1;
    a->x = START_X << 16;
    a->y = START_CY << 16;
    a->ix = f->savedX = START_CX + START_X - SCREEN_W / 2;
    a->iy = f->savedY = START_CY;
    a->v28 = a->v2C = 120;
    a->frame = 4;
    // (the room's dirt as it is, the crust about its pocket, NiDL's room's load: 0x08027F44);
    for (y = 0; y < CELLS_H; y++)
        for (x = 0; x < CELLS_W; x++) SetDirt(f, x, y, IsDirt(f, x, y));
    for (y = 0; y < CELLS_H; y++)
        for (x = 0; x < CELLS_W; x++) {
            if (Dirt(f, x, y)) continue;
            Tiles(NidlTilemap(f->unk56), gCurLevelInfo[f->unk56].roomWidth >> 3, x, y,
                  (Row(f, x - 1, y - 1) & 7) | (Row(f, x - 1, y) & 7) << 3 | (Row(f, x - 1, y + 1) & 7) << 6);
        }
    Redraw(f);
    for (c = 0; c < ARRAY_COUNT(sChildren); c++) {
        i = Spawn(f, sChildren[c], a->ix, a->iy);
        if (i == NONE) continue;
        if (sChildren[c] == K_TOP) f->a[i].facing = f->a[i].flip = -1;
        if (sChildren[c] == K_TOP || sChildren[c] == K_BOTTOM) {
            f->a[i].x = f->a[i].y = 0;
            f->a[i].v28 = 0x10000 + REST_TIME;
            f->a[i].frame = 0;
        }
        if (sChildren[c] == K_EYE) f->a[i].v28 = 96;
    }
    // (its sprites are the fight's task's: KatAM's object's never shown, its VRAM theirs; unkC 0x200 keeps KatAM from giving it VRAM
    // again, flag 8 from animating it);
    obj->base.unkC |= 0x200;
    if (obj->base.sprite.tilesVram >= 0x6014000) VramFree(obj->base.sprite.tilesVram);
    obj->base.sprite.tilesVram = 0;
    obj->base.flags |= 8;
    Camera(f);
    Body(f);
}

/* HeavyMoleInit(); On its template's spot (the fight puts it on NiDL's), hidden (the fight's task draws it), through anything (its moves
 * are its own: MoleMotion()); its box; */
void HeavyMoleInit(struct NidlObject *n) {
    struct HeavyMole *m = (struct HeavyMole *)n;
    struct Object *obj = &n->obj;

    m->f = NULL;
    m->started = FALSE;
    obj->base.flags |= 0x400 | 0x100;
    obj->base.unk5C = (obj->base.unk5C & ~7) | 3 | 0xA0;
    sub_0803E2B0(&obj->base, -20, -16, 16, 16);
    ObjectSetBounds(&obj->base, -20, -16, 16, 16);
}
