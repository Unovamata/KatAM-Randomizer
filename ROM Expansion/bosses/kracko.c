/* Kracko; ported from Kirby: Nightmare in Dream Land (USA), boss object type 0x41 (spawn 0x080A7D2C), as NiDL boss NIDL_KRACKO, Grape
 * Garden's (room 0x1D1 here: its cloud tower, NiDL's, 256 x 2304, its pixels NiDL's). NiDL's, from its code (two parts, 0x08749150:
 * its eye's, 0x080A7D98, two states, each a script, 0x08749158, and a function every frame after them, 0x08749160; its cloud's,
 * 0x080A87C8, seven states, 0x08749168, 0x08749184; its globals at 0x02007D00), checked frame by frame against NiDL's recorded fight:
 * see "The fight" below.
 * NiDL's objects are Actors here (one array, run in NiDL's order: Turns()): it, its child (type 0xC3: its eye's four orbs, then its
 * cloud), its lightning's bolts (0xC5-0xC7), each drawn as a sprite of its own (EffectOnly frames: KatAMExpansion.cs' Kracko()),
 * at NiDL's depths; the boss object KatAM knows (hit, its HP bar) is its eye. Its Waddle Does are
 * KatAM's own (as KatAM's Kracko's), its lightning's bolts attack objects. Beaten, its NiDL frame for it (DEFEAT_FRAME) through
 * KatAM's defeat (its explosion; NiDL's: 0x0806A344) and its shard.
 * Its frames are NiDL's 0x087543E8 (0-36: its eye 0-11, its cloud 12-14, its puffs 15-23, its bolts 24-36), each a sprite: its
 * cloud is two (its top's palette, its bottom's); */
#include "../nidl.h"
#include "code_0806F780.h"
#include "task.h"
#include "main.h"
#include "kirby.h"
#include "functions.h"
#include "constants/songs.h"

void sub_0803D280(u8, u8);
void ArenaGo(struct Object *);
void sub_08055D9C(struct Kirby *);
extern struct Task *gUnk_03000010;       // The HUD's task (its data: +0x0A the HP bar's length, +0x1C the object it shows);

/* gNidlKracko; Filled in by KatAMExpansion.cs (WriteKrackoFight()): the first KatAM frame of each of its groups (FX_*: its eye, NiDL's
 * 0-11; its cloud's top, 12-14; its bottom, 12-14; its puffs and orbs, 15-23; its bolts, 24-36), the VRAM tiles of each group's
 * biggest frame, its three palettes' colours (NiDL's 0, its cloud's top; 1, its bottom, the orbs, the puffs; 2, its eye, the bolts) and
 * its hurt flash's (NiDL's, the 48 after its descriptor: USA 0x082FD438); */
struct NidlKracko {
    u8 fx[5];
    u8 tiles[5];
    u16 colors[3][16];
    u16 flash[48];
};

const struct NidlKracko gNidlKracko = { { 0 } };

enum { FX_EYE, FX_TOP, FX_BOTTOM, FX_PUFF, FX_BOLT };
enum { PAL_TOP, PAL_BOTTOM, PAL_EYE, PALS };
#define KEY_TOP 0x2000              // Its palettes' keys (| its anim ID; its eye's is its anim ID, as its object's);
#define KEY_BOTTOM 0x4000

#define FRAME_HIDDEN -1
#define NO_MAX 0x7FFFFFFF
#define PC_END 0xFFFF
#define NONE -1
#define KIRBY_DY 4                  // (NiDL's Kirby stands 4 pixels higher: KatAM's Kirby's spot, NiDL's: KirbyY());
#define GUARD_TIME 31               // Frames it takes no hits after one (NiDL's +0x75; KatAM's 0x8000: its attacks pass it by);
#define HURT_TIME 13                // NiDL's hurt (0x0806619C, 13): frozen, jittering, flashing;
#define DEFEAT_FRAME 0              // Its frame beaten (0x0806A344);
#define CLOUD_FRAMES 3              // Its child's frames 0-2 its cloud (NiDL's 12-14, two sprites), 3-11 its puffs and orbs (15-23);
#define BOLT_END 12                 // The last bolt's frame (NiDL's 36: its cloud's top's palette);
#define ORDER_EYE 0x18              // Sprite orders (NiDL's depths, +0x42: its eye 10, its cloud, orbs and bolts 12, the last bolt 10);
#define ORDER_TOP 0x1A
#define ORDER_BOTTOM 0x1B
#define SCREEN_W 240
#define SCREEN_H 160
#define LOCK_TOP 16                 // NiDL's camera at the tower's top (0x08026A80: 16-176, its x 8-248 as all the way up);
#define LOCK_FROM 69                // It locks there as its camera's top comes to this (0x08026A80);
#define RISE_AT 0x16F               // Its eye rises to the tower's top as NiDL's camera's top comes to this (0x080A8038);
#define RISE_ABOVE 62               // and stays this far above the screen (0x080A860C);
#define ALL_UP_Y 151                // until every Kirby is up there (0x080A860C: 229 its camera's bottom, 151 the Kirbys);
#define DARK_STEP 8                 // NiDL's darkening (type 0xAE, 0x08065C14: its backgrounds' colours to black, a step a frame,
#define DARKEST 128                 // of 256);
#define DOOS 2                      // Its Waddle Does out at once at most (NiDL's g+8: another while there's one or none);
#define TOP_SPAWN_Y 9               // The cell row a Kirby beaten up top comes back in (KatAM's spawn: y = row * 16 - 7; 137, over the
                                    // tower's top, where Kirbys stand at 152: Respawns());

/* NiDL's objects (Actor.kind): it, then its children (NiDL's slots on); */
enum { K_NONE, K_KRACKO, K_CHILD, K_BOLT, K_BOLT2, K_BOLT3 };
#define ACTORS 12
#define A_KRACKO 0

/* Sounds (NiDL's own: KatAM's nearest); */
#define SFX_CRACKLE SE_KRACKO_LIGHTNING_ATTACK  // (0x228: its lightning, every 10 frames);
#define SFX_SWOOP SE_KRACKO_SWEEP_ATTACK        // (0x229: its swoop);
#define SFX_DOO SE_BASIC_ENEMY_JUMP             // (0x1FA: a Waddle Doo dropped);

/* Its moves: speeds (16.16; across, mirrored by its facing where its script has it so: VelX()) and how long; */
struct Move {
    s32 vx, vy;
    u8 wait;
};

#define F(v) ((s32)((v) * 0x10000))
// Its eye's lunge (0x080A7E44): a jiggle (raw speeds), its loops (six of a frame a step, two of two), out of it;
static const struct Move sJiggle[10] = {
    { 0, F(-1), 2 }, { F(-1), 0, 2 }, { 0, F(1), 4 }, { F(1), 0, 4 }, { 0, F(-1), 4 }, { F(-1), 0, 4 }, { 0, F(1), 4 }, { F(1), 0, 4 },
    { 0, F(-1), 4 }, { F(-0.75), F(0.25), 4 },
};
static const struct Move sLoopFast[4] = { { 0, F(0.75), 1 }, { F(0.75), 0, 1 }, { 0, F(-0.75), 1 }, { F(-0.75), 0, 1 } };
static const struct Move sLoopSlow[4] = { { 0, F(1), 2 }, { F(1), 0, 2 }, { 0, F(-1), 2 }, { F(-1), 0, 2 } };
static const struct Move sLoopOut[3] = { { 0, F(0.75), 1 }, { F(0.75), 0, 1 }, { 0, F(1), 1 } };
// Its lightning (0x080A8D1C, mirrored): back and down, still; then across (sWaver), then back up;
static const struct Move sLightningIn[4] = { { F(-2), F(1), 16 }, { F(-1), F(1), 12 }, { F(-0.5), F(0.5), 8 }, { 0, 0, 16 } };
static const s32 sWaver[4] = { F(-0.5), F(0.125), F(0.5), F(-0.125) };
static const struct Move sLightningOut[4] = { { F(1.125), 0, 4 }, { F(-0.5), F(-0.5), 8 }, { F(-1), F(-1), 12 }, { F(-2), F(-1), 16 } };
// Its swoop (0x080A8FDC, mirrored; its sound as the fifth starts);
static const struct Move sSwoop[30] = {
    { F(2), F(-1), 10 }, { F(1), F(-0.5), 10 }, { F(0.5), F(-0.125), 10 }, { 0, 0, 10 }, { F(-4), F(0.5), 8 }, { F(-3), F(1), 8 },
    { F(-2), F(2), 8 }, { F(-1), F(2), 4 }, { F(-0.5), F(3), 4 }, { F(0.5), F(3), 4 }, { F(1), F(2), 4 }, { F(2), F(2), 8 },
    { F(3), F(1), 8 }, { F(4), F(0.5), 8 }, { F(6), 0, 10 }, { F(4), F(-2), 8 }, { F(3), F(-4), 4 }, { F(2), F(-6), 4 },
    { F(1), F(-4), 4 }, { F(-1), F(-4), 4 }, { F(-2), F(-2), 4 }, { F(-4), F(-1), 4 }, { F(-3), F(-0.5), 8 }, { F(-2), F(0.5), 4 },
    { F(-1), F(1), 4 }, { F(-0.5), F(2), 4 }, { F(0.5), F(1), 4 }, { F(1), F(1), 4 }, { F(1), F(0.5), 4 }, { F(0.125), 0, 6 },
};
#define SWOOP_SOUND 4
// Its bob (0x087490E8: 8 frames a step; 6 sweeping);
static const s32 sBob[6] = { F(0.5), F(1), F(0.5), F(-0.5), F(-1), F(-0.5) };
#undef F

// Its hover's spots on the screen, by its side (0x087490E4), and the lunge's turn landed (0x087490A8);
static const u8 sHoverX[2] = { 72, 168 };
#define HOVER_Y 48
static const s16 sLungeTurn[4] = { 16, 32, -16, -32 };
// Its dash's pause by how far Kirby is (0x087490A4);
static const u8 sPause[3] = { 32, 36, 40 };
/* Its next attack (0x087490B4: by where Kirby is, one of eight; 3 sweep, 4 lightning, 5 swoop), never the lightning twice (0x087490DC),
 * Kirby's zone by its side (0x087490B0, 0x087490B2); */
static const u8 sNext[5][8] = {
    { 3, 3, 3, 5, 5, 5, 5, 5 }, { 3, 3, 3, 3, 3, 5, 5, 5 }, { 3, 3, 5, 5, 5, 4, 4, 4 }, { 3, 3, 3, 3, 5, 4, 4, 4 }, { 5, 5, 5, 5, 4, 4, 4, 4 },
};
static const u8 sNotLightning[2] = { 3, 5 };
static const u8 sZoneLeft[2] = { 4, 3 };
static const u8 sZoneRight[2] = { 3, 4 };
// Its child: the orbs' frames (0x087491A0) and their steps by its mode (0x087491A8); its cloud's frames (0x087491BC), steps (0x087491C4,
// 0x087491CC by its mode: 0x087491D4); its forming's puffs (0x080A99A0);
static const u8 sOrbs[4] = { 8, 9, 10, 11 };
static const u8 sOrbSteps[5] = { 1, 6, 2, 1, 1 };
static const u8 sCloud[4] = { 0, 1, 2, 1 };
static const u8 sCloudSlow[4] = { 16, 12, 24, 4 };
static const u8 sCloudFast[4] = { 2, 2, 2, 2 };
// Its bolts (0x087491E4: by its frame, how far across the first two stand from where they're spawned; 0x087491FC, the next's; the last's,
// 0x08749214);
static const s8 sBoltX[12] = { 8, -8, -8, 8, 0, 0, -8, 8, 0, 0, 0, 0 };
static const s8 sBoltNext[12] = { 8, -8, -8, 8, 8, -8, 0, 0, 0, 0, 0, 0 };
static const s8 sBoltEnd[3] = { 0, 7, -7 };
// Its hurt's jitter (0x0873E184, 0x0873E16C);
static const u8 sJitterOf[14] = { 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4 };
static const s8 sJitter[6][2] = { { 2, 1 }, { -2, -1 }, { -2, 0 }, { 2, 1 }, { 2, -1 }, { -2, 0 } };
/* NiDL's sine (0x0872FA30: 640 entries, 256 one; 512 a circle, 0 right, 128 down: the cosine 128 on), for its speeds at an angle
 * (0x0806421C); */
static const s16 sSine[640] = {
#include "shine_bright_sine.h"
};

/* Actor; One of NiDL's objects (its fields, by their NiDL offsets): a script (WAIT() resumes it, PC_END: over), its move (NiDL's
 * 0x080656B4) and its frame's function each frame, in NiDL's order (Turns()); its sprites; */
struct Actor {
    u8 kind;
    u8 state, next;                     // (+0x15, +0x14);
    u8 script;                          // The state whose script is running;
    bool8 restart;                      // Its next state starts as it's next run (NiDL's 0x08006148);
    bool8 moving;                       // It moves (+0: NiDL's 0x080656B4; its child's none);
    u16 pc, wait;
    s8 facing, flip;                    // (+0x43; its frame's mirroring);
    s16 frame;                          // (+0x3C; FRAME_HIDDEN none);
    s32 x, y;                           // 16.16 (+0x4C);
    s16 ix, iy;                         // Its pixel (+0x48);
    s32 vx, vy, ax, ay, maxX, maxY;     // (+0x54 on);
    s32 v18, v1C, v20, v24, v28, v2C, v30, v34;     // (+0x18 on);
    s16 loop, loop2, loop3;             // (+0x6C, +0x6E, +0x70);
    u8 step;                            // Its script's step in a move table;
    struct EffectObject *sprite, *sprite2;
    struct ObjectBase *attack;          // Its box hurting Kirby (a bolt's);
};

struct Kracko {
    struct NidlObject n;
    struct Fight *f;
    bool8 started;
};

typedef char sKrackoFits[sizeof(struct Kracko) <= NIDL_OBJECT_SIZE ? 1 : -1];

/* Fight; The fight's state (a task of its own), NiDL's objects and its globals (0x02007D00); */
struct Fight {
    struct Object *obj;                 // (NULL: gone, KrackoGone());
    struct Actor a[ACTORS];
    struct EffectObject *eye;           // Its eye's sprite (its child's are its own);
    struct ObjectBase *eyeBox;          // Its eye's box hurting Kirby (an attack object: Body());
    u16 seed;                           // NiDL's random numbers' (0x03000FB4);
    s16 hp;                             // KatAM's object's HP last frame (less now: hit);
    bool8 cloud;                        // Its cloud's part (0x080A87C8 on; its eye's before);
    bool8 cloudInit;                    // Its next start its cloud's (0x080A87C8);
    u8 box;                             // Its box (BOX_*);
    bool8 over;                         // Beaten (its defeat under way: KatAM's explosion);
    bool8 hooked;                       // Its hurt running (its own script held);
    u8 guard;                           // Frames it takes no hits yet;
    bool8 flashing;                     // Its hurt flash on;
    u8 hurt;                            // (0x02006190 + 0xC: its hurt's countdown);
    s16 hurtX, hurtY, hurtFrame;        // Where it was hit, its frame then;
    s32 mode;                           // (g+0: its child's mode; | 0x80, the orbs again);
    s32 saved;                          // (g+4 hurt: its mode before);
    s32 speed;                          // (g+4: its dash's speed);
    s32 count;                          // (g+8: its dash's countdown, -1 dashing);
    s32 lunge;                          // (g+0xC: 1 its lunge at a Kirby landed: to his spot; 0 in the air);
    s32 angle;                          // (g+0x10: its way, 512 a circle);
    struct Kirby *kirby;                // (g+0x14: the Kirby closest across);
    bool8 landed;                       // (g+0x18: his +0x7A);
    bool8 released;                     // The arena told to lock the camera at the tower's top (0x08026A80);
    u8 dark;                            // How dark its backgrounds are (0-DARKEST);
    s8 darkening;                       // 1 darker, -1 back, 0 neither;
    bool8 darkened;                     // It changed the screen's blending (and restores it);
    u16 bldCnt, bldY;
    u16 frameCount;                     // (0x03001EA4: the flashes' clock);
    u8 unk56;
    u16 room;
    u8 pal[PALS];
    struct Object *doos[DOOS];          // Its Waddle Does, while out;
    u8 queue[ACTORS * 2];               // This frame's turns (NiDL's lists: by group, then those spawned meanwhile) and their groups;
    u8 groups[ACTORS * 2];
    u8 queued;
    bool8 listing;
    u8 respawns;                        // The players coming back on the tower's top (bit per player: Respawns());
    struct S16Vec2 spawns[4];           // and their own spawns (KatAM's: where they came in), given back after;
};

enum { BOX_NONE, BOX_EYE, BOX_FORMING, BOX_CLOUD };

/* The scripts: WAIT() resumes them that many frames on; STOP_SCRIPT() ends them (NiDL's endScript); the variables they keep across
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

/* KirbyX(), KirbyY(); A Kirby's spot as NiDL's reads it, after his move this frame (NiDL moves the Kirbys before it, KatAM after: his
 * speed added; his y KIRBY_DY up, as NiDL has it); */
static s32 KirbyX(struct Kirby *k) {
    return (k->base.x + k->base.xspeed) >> 8;
}

static s32 KirbyY(struct Kirby *k) {
    return ((k->base.y - k->base.yspeed) >> 8) - KIRBY_DY;
}

static bool32 InRoom(struct Fight *f, struct Kirby *k) {
    return k->base.roomId == f->room && k->hp > 0;
}

// Landed(); NiDL's +0x7A: on the floor (KatAM's unk62 4, traced);
static bool32 Landed(struct Kirby *k) {
    return (k->base.unk62 & 4) != 0;
}

/* Floating(); NiDL's 0x03002170 + 4 14 (his player's: puffed up, traced): KatAM's puffed up Kirby's animations, flying up and floating
 * (31-33, traced: sub_08047814, sub_080488E0); */
static bool32 Floating(struct Kirby *k) {
    return k->animationIndex >= 31 && k->animationIndex <= 33;
}

// Closest(); The Kirby closest to `a` across (NiDL's 0x08063B38), or NULL;
static struct Kirby *Closest(struct Fight *f, struct Actor *a) {
    struct Kirby *best = NULL;
    s32 d, bestD = 0;
    u8 i;

    for (i = 0; i < gNumKirbys; i++) {
        struct Kirby *k = &gKirbys[i];

        if (!InRoom(f, k)) continue;
        d = abs(KirbyX(k) - a->ix);
        if (best == NULL || d < bestD) {
            best = k;
            bestD = d;
        }
    }
    return best;
}

// FaceKirby(); 0x08063E14: towards the closest Kirby (right when level);
static void FaceKirby(struct Fight *f, struct Actor *a) {
    struct Kirby *k = Closest(f, a);

    if (k) a->facing = KirbyX(k) - a->ix >= 0 ? 1 : -1;
}

/* ArcTan2(); The GBA BIOS's (NiDL's 0x080CFA50, swi 0xA: 0-0xFFFF, a circle); */
u16 ArcTan2(s16 x, s16 y);

/* KirbyAngle(); The way to the closest Kirby from `a` (NiDL's 0x08064314: ArcTan2's angle, >> shift; 0 none); */
static u16 KirbyAngle(struct Fight *f, struct Actor *a, u8 shift) {
    struct Kirby *k = Closest(f, a);

    return k ? ArcTan2(KirbyX(k) - a->ix, KirbyY(k) - a->iy) >> shift : 0;
}

// The camera (NiDL's 0x03002158: KatAM's, its Kirbys' room's player's; the screen 240 x 160);
static s32 CamLeft(struct Fight *f) {
    return gCurLevelInfo[f->unk56].viewportPosition.x >> 8;
}

static s32 CamTop(struct Fight *f) {
    return gCurLevelInfo[f->unk56].viewportPosition.y >> 8;
}

static void Stop(struct Actor *a) {
    a->vx = a->vy = a->ax = a->ay = 0;
    a->maxX = a->maxY = NO_MAX;
}

// StopY(); 0x08006244;
static void StopY(struct Actor *a) {
    a->vy = a->ay = 0;
    a->maxY = NO_MAX;
}

// SetFrame(); NiDL's 0x08006338: its frame, mirrored by its facing (NiDL's other frame changes keep its mirroring);
static void SetFrame(struct Actor *a, s16 frame) {
    a->frame = frame;
    a->flip = a->facing;
}

// VelX(); NiDL's 0x080061C0: x speed for facing right, mirrored otherwise; then y;
static void Vel(struct Actor *a, s32 vx, s32 vy) {
    a->vx = a->facing == 1 ? vx : -vx;
    a->vy = vy;
}

static void SetX(struct Actor *a, s32 x) {
    a->ix = x;
    a->x = x << 16;
}

static void SetY(struct Actor *a, s32 y) {
    a->iy = y;
    a->y = y << 16;
}

// Polar(); 0x0806421C: its speeds at `angle` (512 a circle, 0 right, clockwise), `speed` (256 a pixel a frame);
static void Polar(struct Actor *a, s32 angle, s32 speed) {
    a->vx = sSine[(angle & 0x1FF) + 128] * speed;
    a->vy = sSine[angle & 0x1FF] * speed;
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

/* Group(); An Actor's group (NiDL's +0x12, its type's: it 3, its child and bolts 4; each frame, NiDL runs its objects by group, then by
 * slot: Turns()); */
static u8 Group(const struct Actor *a) {
    return a->kind == K_KRACKO ? 3 : 4;
}

static void Queue(struct Fight *f, s8 i) {
    if (f->queued >= ARRAY_COUNT(f->queue)) return;
    f->groups[f->queued] = Group(&f->a[i]);
    f->queue[f->queued++] = i;
}

static s8 Spawn(struct Fight *f, u8 kind, s32 x, s32 y) {
    struct Actor *a;
    s8 i;

    for (i = A_KRACKO + 1; i < ACTORS && f->a[i].kind != K_NONE; i++) ;
    if (i >= ACTORS) return NONE;
    a = &f->a[i];
    CpuFill32(0, a, sizeof(struct Actor));
    a->kind = kind;
    a->facing = a->flip = 1;
    a->frame = FRAME_HIDDEN;
    a->moving = TRUE;
    SetX(a, x);
    SetY(a, y);
    Stop(a);
    if (f->listing) Queue(f, i);
    return i;
}

static void Release(struct EffectObject **e) {
    if (*e == NULL) return;
    (*e)->flags |= 0x1000;
    *e = NULL;
}

static void Destroy(struct Fight *f, s8 i) {
    struct Actor *a;

    if (i <= A_KRACKO || i >= ACTORS) return;
    a = &f->a[i];
    if (a->kind == K_NONE) return;
    Release(&a->sprite);
    Release(&a->sprite2);
    if (a->attack) a->attack->flags |= 0x1000;
    a->attack = NULL;
    a->kind = K_NONE;
}

static s8 FindKind(struct Fight *f, u8 kind) {
    s8 i;

    for (i = A_KRACKO + 1; i < ACTORS; i++)
        if (f->a[i].kind == kind) return i;
    return NONE;
}

/* Motion(); NiDL's 0x080656B4: its speed plus its change, capped (across either way, falling), then its spot plus its speed; */
static void Motion(struct Actor *a) {
    a->vx += a->ax;
    a->vy += a->ay;
    if (a->maxX != NO_MAX) {
        if (a->vx > 0 && a->vx > a->maxX) a->vx = a->maxX;
        else if (a->vx <= 0 && a->vx < -a->maxX) a->vx = -a->maxX;
    }
    if (a->maxY != NO_MAX && a->vy > 0 && a->vy > a->maxY) a->vy = a->maxY;
    a->x += a->vx;
    a->y += a->vy;
    a->ix = a->x >> 16;
    a->iy = a->y >> 16;
}

/* Home(); 0x08064680 (0x080645A4: by its eighth of a circle to the closest Kirby, 0x08064314 0: ArcTan2's >> 13, 0 right, clockwise):
 * faster that way (0x08064460: by `a`, to `max`, or `max` past it), slower across (0x0806453C: by `a` to none, NiDL's: past it), back
 * (0x080644C0); */
static void Faster(s32 *v, s32 a, s32 max) {
    if (*v < 0 || *v < max) *v += a;
    else *v = max;
}

static void Slower(s32 *v, s32 a, s32 max) {
    if (*v > 0 || abs(*v) < max) *v -= a;
    else *v = -max;
}

static void Damp(s32 *v, s32 a) {
    if (*v < 0) *v += a;
    else if (*v > 0) *v -= a;
}

static void Home(struct Actor *a, u8 way, s32 accel, s32 max) {
    switch (way) {
    case 0: Faster(&a->vx, accel, max); Damp(&a->vy, accel); break;
    case 1: Faster(&a->vx, accel, max); Faster(&a->vy, accel, max); break;
    case 2: Damp(&a->vx, accel); Faster(&a->vy, accel, max); break;
    case 3: Slower(&a->vx, accel, max); Faster(&a->vy, accel, max); break;
    case 4: Slower(&a->vx, accel, max); Damp(&a->vy, accel); break;
    case 5: Slower(&a->vx, accel, max); Slower(&a->vy, accel, max); break;
    case 6: Damp(&a->vx, accel); Slower(&a->vy, accel, max); break;
    case 7: Faster(&a->vx, accel, max); Slower(&a->vy, accel, max); break;
    }
}

/* Its eye (0x080A7D98 on); */

// Way(); 0x080A9738: its eye's frame, the way to the closest Kirby (4 right, then each eighth of a circle clockwise);
static void Way(struct Fight *f, struct Actor *a) {
    a->frame = (((KirbyAngle(f, a, 7) + 32) & 0x1FF) >> 6) + 4;
}

// KeepNear(); 0x080A932C: no more than 32 above or below the screen, 16 off its sides;
static void KeepNear(struct Fight *f, struct Actor *a) {
    s32 top = CamTop(f), left = CamLeft(f);

    if (a->iy < top - 32) SetY(a, top - 32);
    else if (a->iy > top + SCREEN_H + 32) SetY(a, top + SCREEN_H + 32);
    if (a->ix < left - 16) SetX(a, left - 16);
    else if (a->ix > left + SCREEN_W + 16) SetX(a, left + SCREEN_W + 16);
}

// AboveClose(); 0x080A93EC: the Kirby above it, 15 across at most;
static bool32 AboveClose(struct Fight *f, struct Actor *a) {
    struct Kirby *k = f->kirby;

    return k && KirbyY(k) - a->iy < 0 && abs(KirbyX(k) - a->ix) <= 15;
}

/* Lunge(); 0x080A9434: still; at the Kirby: his spot (+0x18, +0x20), its way back from him in 32nds (a dash away first); at him landed,
 * to his spot (lowered to his tile's bottom), a little to one side; */
static void Lunge(struct Fight *f, struct Actor *a) {
    struct Kirby *k = f->kirby;

    Stop(a);
    a->v2C = 0;
    a->v30 = 32;
    f->mode = 0x83;
    a->v18 = KirbyX(k);
    a->v20 = KirbyY(k);
    f->angle = (KirbyAngle(f, a, 7) + 0x100) & 0x1F0;
    if (Landed(k)) {
        f->lunge = 1;
        a->v20 |= 15;
        f->angle = (f->angle + sLungeTurn[Rand(f, 4)]) & 0x1F0;
    } else {
        f->lunge = 0;
    }
    a->v28 = 4;
}

/* Drift(); 0x080A94D4: towards the Kirby along one axis (+0x18: 0 across, 1 up or down; mostly the one he's less far on), 1.25 a frame
 * (+0x20 his side of it that way); */
static void Drift(struct Fight *f, struct Actor *a) {
    struct Kirby *k = f->kirby;
    s32 right = KirbyX(k) - a->ix >= 0, below = KirbyY(k) - a->iy >= 0;

    a->v18 = below;
    if (Rand(f, 4)) a->v18 ^= 1;
    if (!a->v18) {
        f->angle = right ? 0 : 0x100;
        a->v20 = right;
    } else {
        f->angle = below ? 0x80 : 0x180;
        a->v20 = below;
    }
    Polar(a, f->angle, 0x140);
}

// Flip(); 0x080A95DC: the Kirby past it that way (its other axis then, his side of it on that one);
static bool32 Flip(struct Fight *f, struct Actor *a) {
    struct Kirby *k = f->kirby;
    s32 right = KirbyX(k) - a->ix >= 0, below = KirbyY(k) - a->iy >= 0;

    if (!a->v18) {
        if (a->v20 == right) return FALSE;
        a->v18 = 1;
        a->v20 = below;
    } else {
        if (a->v20 == below) return FALSE;
        a->v18 = 0;
        a->v20 = right;
    }
    return TRUE;
}

// Turn(); 0x080A96DC: its way a 32nd of a circle nearer the Kirby's;
static void Turn(struct Fight *f, struct Actor *a) {
    s32 to = KirbyAngle(f, a, 7);
    s32 d1 = (f->angle - (to - 16)) & 0x1FF, d2 = (f->angle - (to + 16)) & 0x1FF;

    f->angle = (d1 <= d2 ? f->angle + 16 : f->angle - 16) & 0x1FF;
}

// HomeCheck(); 0x080A96A4: the Kirby in the air 120 frames, after him (3) while he is;
static void HomeCheck(struct Fight *f, struct Actor *a) {
    if (!(a->v34 & 0x8000)) return;
    if (!f->landed) {
        f->mode = 2;
        a->v24 = 4;
        a->v28 = 3;
    } else {
        a->v34 = 0;
    }
}

// Inside(); 0x08063EB0: the Kirby strictly within `r` of it;
static bool32 Inside(struct Fight *f, struct Actor *a, s32 r) {
    s32 x = KirbyX(f->kirby), y = KirbyY(f->kirby);

    return x > a->ix - r && x < a->ix + r && y > a->iy - r && y < a->iy + r;
}

/* EyeScript0(); Its eye's lunge's end (0x080A7E44): at +0x28 10 (its lunge there: EyeFrame0()), a jiggle, then its loops, out of them,
 * still, the orbs fast (0x82) and slow again (0x81); +0x28 9; */
static void EyeScript0(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 0;
    for (;;) {
        do WAIT(1); while (a->v28 != 10);
        Stop(a);
        for (a->step = 0; a->step < ARRAY_COUNT(sJiggle); a->step++) {
            a->vx = sJiggle[a->step].vx;
            a->vy = sJiggle[a->step].vy;
            WAIT(sJiggle[a->step].wait);
        }
        // (0x08030DB8 0x08749B84: a look at the terrain about it, its answer unused: not ported);
        for (a->loop = 0; a->loop <= 5; a->loop++)
            for (a->step = 0; a->step < ARRAY_COUNT(sLoopFast); a->step++) {
                a->vx = sLoopFast[a->step].vx;
                a->vy = sLoopFast[a->step].vy;
                WAIT(sLoopFast[a->step].wait);
            }
        for (a->loop = 0; a->loop <= 1; a->loop++)
            for (a->step = 0; a->step < ARRAY_COUNT(sLoopSlow); a->step++) {
                a->vx = sLoopSlow[a->step].vx;
                a->vy = sLoopSlow[a->step].vy;
                WAIT(sLoopSlow[a->step].wait);
            }
        for (a->step = 0; a->step < ARRAY_COUNT(sLoopOut); a->step++) {
            a->vx = sLoopOut[a->step].vx;
            a->vy = sLoopOut[a->step].vy;
            WAIT(sLoopOut[a->step].wait);
        }
        Stop(a);
        WAIT(16);
        f->mode = 0x82;
        WAIT(8);
        f->mode = 0x81;
        a->v28 = 9;
    }
    SCRIPT_END;
}

// EyeScript1(); Its eye rising (0x080A85E4): up, faster and faster (0.125 a frame more);
static void EyeScript1(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 1;
    a->vy = 0;
    a->ay = -0x2000;
    a->v28 = 0;
    STOP_SCRIPT();
    SCRIPT_END;
}

/* Timer(); Its lunge's clock (0x080A8106: +0x30 counting, every 16 frames from 32 on a chance; the Kirby right above it, waiting
 * (0x8000) till he isn't); */
static void Timer(struct Fight *f, struct Actor *a) {
    s32 r;

    if (a->v30 & 0x8000) {
        if (!AboveClose(f, a)) Lunge(f, a);
        return;
    }
    if (++a->v30 <= 31 || (a->v30 & 15)) return;
    r = Rand(f, 32);
    if (a->v30 == 0x80) {
        if (f->landed || r > 16) Lunge(f, a);
        else a->v30 = 32;
    } else if (f->landed ? r <= 1 : r <= 0) {
        if (AboveClose(f, a)) a->v30 = 0x8000;
        else Lunge(f, a);
    }
}

/* EyeFrame0(); Its eye's every frame (0x080A8038): up to the tower's top as the camera comes to it (1); else its moves by +0x28: 0 still,
 * 1 drifting at Kirby along one axis, 2 turning at him, 3 after him in the air, 4 waiting, 5 a dash away, slowing, 6 then at him (his
 * spot landed: 8; in the air, 7 slowing again), 9 after its loops, 10 its loops (its script); */
static void EyeFrame0(struct Fight *f, struct Actor *a) {
    s32 n;

    if (CamTop(f) <= RISE_AT) {
        Stop(a);
        SetState(a, 1);
        Restart(a);
        return;
    }
    f->kirby = Closest(f, a);
    if (f->kirby == NULL) return;
    f->landed = Landed(f->kirby);
    if (a->v28 < 8 || a->v28 > 10) KeepNear(f, a);
    if (!(a->v34 & 0x8000)) {
        if (f->landed) a->v34 = 0;
        else if (++a->v34 > 0x77) a->v34 = 0x8000;
    }
    if (a->v28 >= 0 && a->v28 <= 2) {
        if (a->v28 != 0 && ++a->v2C > 0x3F) {
            a->v2C = 0;
            if (!Rand(f, 4)) a->vx = a->vy = a->v28 = 0;
        }
        Timer(f, a);
    }
    switch (a->v28) {
    case 0:
        if (++a->v2C > 32) {
            a->v2C = 0;
            f->mode = 0x81;
            a->v1C = 4;
            a->v28 = 1;
            Drift(f, a);
        }
        HomeCheck(f, a);
        break;
    case 1:
        if (Flip(f, a)) {
            a->v28 = 2;
            a->v1C = 4;
            Turn(f, a);
            Polar(a, f->angle, 0x140);
        }
        HomeCheck(f, a);
        break;
    case 2:
        Flip(f, a);
        if (!--a->v1C) {
            a->v1C = 4;
            Turn(f, a);
            Polar(a, f->angle, 0x140);
        }
        HomeCheck(f, a);
        break;
    case 3:
        if (f->landed) {
            if (Rand(f, 8) <= 2) {
                Lunge(f, a);
            } else {
                a->vx = a->vy = 0;
                a->v34 = a->v30 = a->v2C = a->v28 = 0;
            }
        } else if (!--a->v24) {
            a->v24 = 4;
            Home(a, KirbyAngle(f, a, 13), 0x3000, 0x18000);
        }
        break;
    case 4:
        if (!--a->v30) {
            a->v28 = 5;
            f->speed = 0x400;
            f->count = -1;
        }
        break;
    case 5:
        if (f->count == -1) {
            Polar(a, f->angle, f->speed);
            f->speed -= 0x40;
            if (!f->speed) {
                a->vx = a->vy = 0;
                f->count = 16;
            }
        } else if (!--f->count) {
            n = 0;
            if (!Inside(f, a, 32)) {
                n++;
                if (!Inside(f, a, 64)) n++;
            }
            f->count = sPause[n];
            f->angle = (f->angle + 0x100) & 0x1FF;
            a->v28 = 6;
        }
        break;
    case 6:
        if (--f->count) {
            f->speed += 24;
            if (f->speed > 0x300) f->speed = 0x300;
            Polar(a, f->angle, f->speed);
        } else {
            f->count = -1;
            a->v1C = 4;
            a->v28 = f->lunge + 7;
        }
        break;
    case 7:
        if (f->count != -1) {
            if (f->count == 24) f->mode = 0x82;
            else if (f->count == 16) f->mode = 0x81;
            if (--f->count) break;
            HomeCheck(f, a);
            if (a->v34 & 0x8000) break;
            if (Rand(f, 16) <= 2) Lunge(f, a);
            else a->v30 = a->v2C = a->v28 = 0;
        } else {
            f->speed -= 30;
            if (f->speed < 0) {
                a->vx = a->vy = 0;
                f->count = 32;
            } else if (!--a->v1C) {
                a->v1C = 4;
                Turn(f, a);
                Polar(a, f->angle, f->speed);
            }
        }
        break;
    case 8:
        if (abs(a->ix - a->v18) <= 2 && abs(a->iy - a->v20) <= 2) {
            a->vx = a->vy = 0;
            a->v28 = 10;
        } else {
            f->angle = (((ArcTan2(a->v18 - a->ix, a->v20 - a->iy) >> 7) + 32) & 0x1FF) >> 6;
            Home(a, f->angle, 0x3000, 0x18000);
        }
        break;
    case 9:
        HomeCheck(f, a);
        if (!(a->v34 & 0x8000)) a->v30 = a->v2C = a->v28 = 0;
        break;
    }
}

/* ArenaLock(); 0x08026A80 (its +0x76 6): the camera's top at LOCK_FROM, the arena told to hold it at the tower's top (as NiDL's lock,
 * 16-176); whether it's there; */
static bool32 ArenaLock(struct Fight *f) {
    if (!f->released && CamTop(f) <= LOCK_FROM) {
        f->released = TRUE;
        ArenaGo(f->obj);
    }
    return f->released && CamTop(f) <= LOCK_TOP;
}

/* EyeFrame1(); Its eye rising (0x080A860C): off the screen's top, still there, RISE_ABOVE over it (its orbs gone, 4), till the camera is
 * held at the tower's top and every Kirby up there; then over the side across from the closest one, its cloud next (3); */
static void EyeFrame1(struct Fight *f, struct Actor *a) {
    struct Kirby *k;
    s32 sx;
    u8 i;

    switch (a->v28) {
    case 0:
        if (a->iy < CamTop(f) - RISE_ABOVE) {
            StopY(a);
            f->mode = 4;
            a->v28++;
        }
        ArenaLock(f);
        break;
    case 1:
        SetY(a, CamTop(f) - RISE_ABOVE);
        if (ArenaLock(f)) a->v28++;
        break;
    case 2:
        for (i = 0; i < gNumKirbys; i++)
            if (InRoom(f, &gKirbys[i]) && KirbyY(&gKirbys[i]) > ALL_UP_Y) break;
        if (i >= gNumKirbys) {
            k = Closest(f, a);
            sx = k ? KirbyX(k) - CamLeft(f) : 0;
            if (sx == 128) SetX(a, CamLeft(f) + sHoverX[Rand(f, 2)]);
            SetX(a, CamLeft(f) + (sx <= 127 ? sHoverX[1] : sHoverX[0]));
            a->v28++;
        }
        SetY(a, CamTop(f) - RISE_ABOVE);
        break;
    case 3:
        f->cloudInit = TRUE;
        Restart(a);
        break;
    }
}

/* Its cloud (0x080A87C8 on); */

/* ShowBar(); The HP bar (NiDL's 0x08066544, after the cloud's first 8 frames: none before; HideBar(): KatAM's taken back, as the fight
 * starts and after anything shows it, its eye untouchable meanwhile: Body()); */
static void ShowBar(struct Fight *f) {
    sub_08034E14(f->obj);
}

static void HideBar(struct Object *obj) {
    u8 *hud = gUnk_03000010 ? TaskGetStructPtr(gUnk_03000010) : NULL;

    if (hud && *(void **)(hud + 0x1C) == obj) {
        hud[0xA] = 0;
        *(void **)(hud + 0x1C) = NULL;
        sub_08034FA8(NULL);     // (the HUD's corner back to the area's name, as sub_08034E14 does letting go);
    }
}

/* CloudScript0(); Its cloud forming (0x080A8878): coming down from over the screen, the HP bar shown, its puffs (its child), still on its
 * hover's height; its box hurts Kirby, then takes hits (BOX_FORMING, BOX_CLOUD); then its choice (1); */
static void CloudScript0(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 0;
    SetY(a, CamTop(f) - 16);
    a->vy = 0x8000;
    WAIT(8);
    ShowBar(f);
    f->box = BOX_FORMING;
    f->mode = 0;
    WAIT(76);
    for (a->loop = 0; a->loop <= 10; a->loop++) {
        a->vy = 0x8000;
        WAIT(3);
        a->vy = 0;
        WAIT(1);
        a->vy = 0x8000;
        WAIT(1);
    }
    a->vy = 0;
    SetY(a, CamTop(f) + HOVER_Y);
    f->mode = 0;
    // (NiDL waits for its HP bar full, already: 0x0200AFF8; KatAM's fills on its own);
    f->box = BOX_CLOUD;
    SetState(a, 1);
    STOP_SCRIPT();
    SCRIPT_END;
}

/* Pick(); Its next attack (0x080A8A00): by where the closest Kirby is (floating high up: the swoop; else his zone, sNext); never the
 * lightning twice in a row; */
static void Pick(struct Fight *f, struct Actor *a) {
    struct Kirby *k = Closest(f, a);
    s32 kx = k ? KirbyX(k) : 0, ky = k ? KirbyY(k) : 0;    // (in the room, NiDL's 0x08063D4C);
    u8 zone, next;

    a->v28++;
    if (k && Floating(k)) {
        if (ky <= 95) {
            SetState(a, 5);
            a->v2C = 3;
            return;
        }
        zone = a->v30 == 0 ? kx > 71 : kx <= 168;
    } else if (kx <= 71) {
        zone = sZoneLeft[a->v30];
    } else if (kx > 168) {
        zone = sZoneRight[a->v30];
    } else {
        zone = 2;
    }
    next = sNext[zone][Rand(f, 8)];
    if (next == 4 && a->v18 == 4) next = sNotLightning[Rand(f, 2)];
    SetState(a, next);
    a->v18 = next;
    a->v2C = 3;
}

static u8 DoosOut(struct Fight *f);

/* CloudScript1(); Its choice (0x080A8970), by what it did (+0x2C): 0 after forming, the lightning, then the swoop; 1 after its bob, a Waddle
 * Doo half the time (always after 5 attacks; never with two out), else its next attack (Pick()); 2 after an attack, back on its hover
 * on its side, its bob; 3 waiting for the next to start; */
static void CloudScript1(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 1;
    for (;;) {
        switch (a->v2C) {
        case 0:
            if (a->v18 == 0) {
                a->v18 = 4;
                SetState(a, 4);
            } else {
                a->v18 = 5;
                SetState(a, 5);
                a->v2C = 3;
            }
            break;
        case 1:
            if (a->v28 & 0x80) {
                a->v28 = 0;
                Pick(f, a);
            } else if ((Rand(f, 2) || a->v28 > 4) && DoosOut(f) <= 1) {
                a->v28 = 0x80;
                a->v2C = 3;
                SetState(a, 6);
            } else {
                Pick(f, a);
            }
            break;
        case 2:
            SetState(a, 2);
            SetX(a, CamLeft(f) + sHoverX[a->v30]);
            SetY(a, CamTop(f) + HOVER_Y);
            a->v2C = 3;
            break;
        }
        WAIT(1);
    }
    SCRIPT_END;
}

// CloudScript2(); Its bob (0x080A8B6C), 8 frames a step; then its choice (1, after its bob);
static void CloudScript2(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 2;
    Stop(a);
    for (a->loop = 0; a->loop <= 5; a->loop++) {
        a->vy = sBob[a->loop];
        WAIT(8);
    }
    a->v2C = 1;
    SetState(a, 1);
    STOP_SCRIPT();
    SCRIPT_END;
}

// CloudScript3(); Its sweep (0x080A8BF4): across to its other side at 1.25, bobbing (6 frames a step), till there (CloudFrame3());
static void CloudScript3(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 3;
    Stop(a);
    if (a->ix - CamLeft(f) <= 127) {
        a->v30 = 0;
        a->vx = 0x14000;
    } else {
        a->v30 = 1;
        a->vx = -0x14000;
    }
    a->v24 = 0;
    for (;;)
        for (a->loop = 0; a->loop <= 5; a->loop++) {
            a->vy = sBob[a->loop];
            WAIT(6);
        }
    SCRIPT_END;
}

/* CloudScript4(); Its lightning (0x080A8D1C): the screen darker (0x080657F8), back and down a little, across (1.25, wavering), a bolt every
 * 4 frames (0xC5, 32 under it), a crackle every 10; the screen back (0x080657CC), up to its other side's hover; */
static void CloudScript4(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 4;
    f->mode = 1;
    Stop(a);
    f->darkening = 1;
    a->facing = a->v30 == 0 ? 1 : -1;
    for (a->step = 0; a->step < ARRAY_COUNT(sLightningIn); a->step++) {
        Vel(a, sLightningIn[a->step].vx, sLightningIn[a->step].vy);
        WAIT(sLightningIn[a->step].wait);
    }
    a->v24 = 1;
    Vel(a, 0x14000, a->vy);
    for (a->loop = 0; a->loop <= 8; a->loop++)
        for (a->loop2 = 0; a->loop2 <= 3; a->loop2++) {
            Spawn(f, K_BOLT, a->ix, a->iy + 32);
            a->vy = sWaver[a->loop2];
            for (a->loop3 = 0; a->loop3 <= 3; a->loop3++) {
                if (!--a->v24) {
                    Sound(f, SFX_CRACKLE);
                    a->v24 = 10;
                }
                WAIT(1);
            }
        }
    Spawn(f, K_BOLT, a->ix, a->iy + 32);
    a->vy = 0;
    WAIT(6);
    f->darkening = -1;
    for (a->step = 0; a->step < ARRAY_COUNT(sLightningOut); a->step++) {
        Vel(a, sLightningOut[a->step].vx, sLightningOut[a->step].vy);
        WAIT(sLightningOut[a->step].wait);
    }
    a->vx = a->vy = 0;
    f->mode = 0;
    a->v30 ^= 1;
    if (a->v2C) a->v2C = 2;
    SetState(a, 1);
    STOP_SCRIPT();
    SCRIPT_END;
}

// CloudScript5(); Its swoop (0x080A8FDC): down to the floor in a loop and back up across, to its other side;
static void CloudScript5(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 5;
    f->mode = 1;
    a->facing = a->v30 == 0 ? 1 : -1;
    for (a->step = 0; a->step < ARRAY_COUNT(sSwoop); a->step++) {
        if (a->step == SWOOP_SOUND) Sound(f, SFX_SWOOP);
        Vel(a, sSwoop[a->step].vx, sSwoop[a->step].vy);
        WAIT(sSwoop[a->step].wait);
    }
    f->mode = 0;
    if (a->v2C) a->v2C = 2;
    a->v30 ^= 1;
    SetState(a, 1);
    STOP_SCRIPT();
    SCRIPT_END;
}

/* DoosOut(); Its Waddle Does still about (NiDL counts them: g+8); Doo(): one dropped from under it (KatAM's, its own till Kracko goes,
 * as KatAM's Kracko's: sub_080DE064); */
static u8 DoosOut(struct Fight *f) {
    u8 i, n = 0;

    for (i = 0; i < DOOS; i++) {
        struct Object *doo = f->doos[i];

        if (doo && doo->base.header.kind && doo->type == OBJ_WADDLE_DOO && doo->base.parent == f->obj && !(doo->base.flags & 0x1000)
            && doo->unk80 > 0)
            n++;
        else
            f->doos[i] = NULL;
    }
    return n;
}

static void Doo(struct Fight *f, struct Actor *a) {
    struct Object *doo;
    u8 i;

    DoosOut(f);
    for (i = 0; i < DOOS && f->doos[i]; i++) ;
    if (i >= DOOS) return;
    doo = f->doos[i] = CreateObjTemplateAndObj(f->unk56, 1, 0x24, a->ix, a->iy + 16, 0, 0x1F, 0, 0, OBJ_WADDLE_DOO, 0, 0, 1, 0, 0, 0, 0, 0,
                                               0, 0, 0, 0, 0);
    doo->base.parent = f->obj;
    doo->unk7C = sub_0809F840;
}

// CloudScript6(); A Waddle Doo (0x080A8F40): still 36 frames, one dropped 16 under it; then its choice (2: back on its hover);
static void CloudScript6(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 6;
    Stop(a);
    f->mode = 1;
    WAIT(36);
    Doo(f, a);
    Sound(f, SFX_DOO);
    f->mode = 0;
    a->v2C = 2;
    SetState(a, 1);
    STOP_SCRIPT();
    SCRIPT_END;
}

/* CloudFrame(); Its cloud's every frame (0x080A8850: its eye's frame, then its state's: 0x08749184): its next state started (its sweep's:
 * at its other side, 0x080A8C84: there, its choice next, 2); */
static void CloudFrame(struct Fight *f, struct Actor *a) {
    s32 sx = a->ix - CamLeft(f);

    Way(f, a);
    if (a->state == 3) {
        if (a->v30 == 0 ? sx > 167 : sx <= 72) {
            SetX(a, CamLeft(f) + sHoverX[a->v30 == 0]);
            a->v24++;
        }
        if (a->v24) {
            a->vx = a->vy = 0;
            a->v2C = 2;
            a->v30 ^= 1;
            SetState(a, 1);
            Restart(a);
        }
        return;
    }
    if (a->next != a->state) Restart(a);
}

static void (*const sEyeScripts[2])(struct Fight *, struct Actor *) = { EyeScript0, EyeScript1 };
static void (*const sCloudScripts[7])(struct Fight *, struct Actor *) = {
    CloudScript0, CloudScript1, CloudScript2, CloudScript3, CloudScript4, CloudScript5, CloudScript6,
};

/* CloudInit(); Its cloud's start (0x080A87C8): its side (its screen x over 127: the right), its forming (0) as it's run; */
static void CloudInit(struct Fight *f, struct Actor *a) {
    f->cloud = TRUE;
    f->box = BOX_EYE;
    a->v28 = a->v2C = a->v34 = a->v18 = 0;
    a->v30 = a->ix - CamLeft(f) > 127;
    SetState(a, 0);
}

/* KrackoFrame(); Its frame's function (+4): its eye's (0x080A7E10: its frame, its state's; its box only in state 0) or its cloud's; */
static void KrackoFrame(struct Fight *f, struct Actor *a) {
    if (f->cloud) {
        CloudFrame(f, a);
        return;
    }
    Way(f, a);
    if (a->state == 0) EyeFrame0(f, a);
    else EyeFrame1(f, a);
    f->box = a->state == 0 ? BOX_EYE : BOX_NONE;
}

/* CloudStep(); Its cloud's frame (0x080A99A0's last loop): its frames sCloud, slow or fast by its mode (its mode changed, the step
 * from then by the new one); */
static void CloudStep(struct Fight *f, struct Actor *a) {
    if (a->v28 != f->mode) {
        a->v28 = f->mode;
        a->v30 = (f->mode == 1 || f->mode == 2 ? sCloudFast : sCloudSlow)[a->v2C];
    }
    if (!--a->v30) {
        if (++a->v2C > 3) a->v2C = 0;
        a->frame = sCloud[a->v2C];
        a->v30 = (a->v28 == 1 || a->v28 == 2 ? sCloudFast : sCloudSlow)[a->v2C];
    }
}

/* Its child (type 0xC3, 0x080A983C): its eye's orbs (frames sOrbs, each the steps of its mode: f->mode), facing Kirby as its mode's 0x80
 * says; hidden for mode 4; its mode 0, its cloud (0x080A99A0): its puffs forming it, then its frames (CloudStep(); as each starts, where
 * it was kept, +0x18 on, for a hit: Hit()); where Kracko is (0x080A9910, 0x080A9BA0); */
static void ChildScript(struct Fight *f, struct Actor *a) {
    SCRIPT;
    // (the orbs);
    FaceKirby(f, a);
    a->v28 = f->mode & 15;
    a->v2C = 0;
    a->v30 = sOrbSteps[a->v28];
    a->frame = sOrbs[0];
    while (f->mode != 4) {
        if (a->v28 != (f->mode & 15)) {
            a->v28 = f->mode & 15;
            a->v30 = sOrbSteps[a->v28];
        }
        if (!--a->v30) {
            if (++a->v2C > 3) a->v2C = 0;
            a->frame = sOrbs[a->v2C];
            a->v30 = sOrbSteps[a->v28];
        }
        WAIT(1);
    }
    a->frame = FRAME_HIDDEN;
    STOP_SCRIPT();
    // (its cloud forming: restarted here, ChildFrame(); facing right, its mirroring the orbs' last, as NiDL's);
    case 1:
    a->facing = 1;
    a->frame = 10;
    WAIT(6);
    a->frame++;
    WAIT(6);
    for (a->frame = 8; a->frame < 11; a->frame++) WAIT(6);
    WAIT(6);
    for (a->loop = 0; a->loop <= 2; a->loop++)
        for (a->frame = 8; a->frame <= 11; a->frame++) WAIT(2);
    for (a->loop = 0; a->loop <= 3; a->loop++)
        for (a->frame = 8; a->frame <= 11; a->frame++) WAIT(1);
    for (a->loop = 0; a->loop <= 6; a->loop++) {
        a->frame = 0;
        WAIT(2);
        a->frame = 1;
        WAIT(1);
        a->frame = 2;
        WAIT(1);
        a->frame = 1;
        WAIT(1);
    }
    a->v28 = f->mode;
    a->v2C = 0;
    a->frame = sCloud[0];
    a->v30 = (f->mode == 1 || f->mode == 2 ? sCloudFast : sCloudSlow)[0];
    a->v24 = TRUE;
    for (;;) {
        a->v18 = a->v2C;
        a->v1C = a->v30;
        a->v20 = a->frame;
        CloudStep(f, a);
        WAIT(1);
    }
    SCRIPT_END;
}

/* ChildFrame(); Where Kracko is (its pixel); its orbs facing Kirby again (its mode's 0x80, then off); its mode 0, its cloud (its script
 * restarted there); */
static void ChildFrame(struct Fight *f, struct Actor *a) {
    struct Actor *k = &f->a[A_KRACKO];

    a->ix = k->ix;
    a->iy = k->iy;
    if (a->v34) return;
    if (f->mode & 0x80) {
        FaceKirby(f, a);
        a->flip = a->facing;
        f->mode &= ~0x80;
    }
    if (f->mode == 0) {
        a->v34 = 1;
        a->pc = 1;
        a->wait = 0;
    }
}

/* BoltScript(); A bolt (0xC5, 0xC6: 0x080A9C28, 0x080A9CF0, a frame by chance; the first a little to one side; each spawning the next
 * under it, 16 then 12; 0xC7, 0x080A9DA4, its end, frame BOLT_END), 2 frames, gone; */
static void BoltScript(struct Fight *f, struct Actor *a) {
    SCRIPT;
    if (a->kind == K_BOLT3) {
        SetX(a, a->ix + sBoltEnd[Rand(f, 3)]);
        a->frame = BOLT_END;
    } else {
        a->frame = Rand(f, 12);
        if (a->kind == K_BOLT) SetX(a, a->ix + Rand(f, 16) + sBoltX[a->frame] - 8);
        else SetX(a, a->ix + sBoltX[a->frame]);
        Spawn(f, a->kind + 1, a->ix + sBoltNext[a->frame], a->iy + (a->kind == K_BOLT ? 16 : 12));
    }
    WAIT(2);
    Destroy(f, a - f->a);
    return;
    SCRIPT_END;
}

static void RunChild(struct Fight *f, struct Actor *a) {
    switch (a->kind) {
    case K_CHILD: ChildScript(f, a); break;
    case K_BOLT: case K_BOLT2: case K_BOLT3: BoltScript(f, a); break;
    default: a->pc = PC_END; break;
    }
}

/* Hurt (NiDL's 0x0806619C, 13: its handler 0x080A9760, each frame 0x080A9794, after every object's turn: KrackoMain()): its own script
 * held (its move, its frame's function; the room, the Kirbys, its children go on), flashing in its hurt colours (its three palettes,
 * the clock's 2 frames in 4: Colors()), jittering (sJitter), its eye spinning (frames 0-7: every other frame one on), its child's mode
 * 2 (its cloud fast); out, back where it was, its mode back; no hit stars (NiDL's handler makes none, checked against a recorded hit); */
static void HurtStep(struct Fight *f) {
    struct Actor *a = &f->a[A_KRACKO];
    u8 i = sJitterOf[f->hurt];

    a->ix += sJitter[i][0];
    a->iy += sJitter[i][1];
    if (f->frameCount & 1) a->frame = (a->frame + 1) & 7;
    if (--f->hurt) return;
    a->ix = f->hurtX;
    a->iy = f->hurtY;
    a->frame = f->hurtFrame;
    f->hooked = FALSE;
    f->flashing = FALSE;
    f->mode = f->saved;
}

/* Hit(); A hit took HP off it (KatAM's, between frames; NiDL's in its frame's function, its hurt's first step after it: KatAM's a frame
 * later, so two steps this frame, this one and its own turn's; its cloud's last frame again, by its new mode, as NiDL's child had it):
 * no more for GUARD_TIME frames; */
static void Hit(struct Fight *f) {
    struct Actor *a = &f->a[A_KRACKO];
    s8 i;

    f->guard = GUARD_TIME;
    if (f->hooked) return;
    f->hooked = TRUE;
    f->flashing = TRUE;
    f->hurtX = a->ix;
    f->hurtY = a->iy;
    f->hurtFrame = a->frame;
    f->hurt = HURT_TIME;
    f->saved = f->mode;
    f->mode = 2;
    i = FindKind(f, K_CHILD);
    if (i != NONE && f->a[i].v24) {
        struct Actor *c = &f->a[i];

        c->v2C = c->v18;
        c->v30 = c->v1C;
        c->frame = c->v20;
        CloudStep(f, c);
    }
    HurtStep(f);
}

/* Process(); An Actor's frame (in NiDL's order: Turns()): its script (its next state's, restarted), its move, its frame's function; it
 * hurt, none (its hurt's step after the turns); */
static void BoltFrame(struct Fight *f, struct Actor *a);

static void Process(struct Fight *f, s8 i) {
    struct Actor *a = &f->a[i];

    if (a->kind == K_NONE) return;
    if (a->kind == K_KRACKO) {
        if (f->hooked) return;
        if (a->restart) {
            a->restart = FALSE;
            if (f->cloudInit) {
                f->cloudInit = FALSE;
                CloudInit(f, a);
            }
            a->script = a->next;
            a->pc = 0;
            a->wait = 0;
        }
        if (a->pc != PC_END && (!a->wait || !--a->wait)) {
            if (f->cloud) sCloudScripts[a->script < 7 ? a->script : 1](f, a);
            else sEyeScripts[a->script & 1](f, a);
        }
        Motion(a);
        KrackoFrame(f, a);
        return;
    }
    if (a->pc != PC_END && (!a->wait || !--a->wait)) RunChild(f, a);
    if (a->kind == K_NONE) return;
    if (a->kind == K_CHILD) ChildFrame(f, a);
    else Motion(a), BoltFrame(f, a);
}

/* Turns(); The Actors' frames in NiDL's order (0x08005228): by group (Group(): its, its child's and bolts'), each by slot; then those
 * spawned meanwhile (NiDL's never run in the turns they're spawned in), by group, and so on (its bolts' chain); */
static void Turns(struct Fight *f) {
    u8 g, q, from, to;
    s8 i;

    f->queued = 0;
    for (g = 3; g <= 4; g++)
        for (i = 0; i < ACTORS; i++)
            if (f->a[i].kind != K_NONE && Group(&f->a[i]) == g) Queue(f, i);
    f->listing = TRUE;
    for (from = 0; from < f->queued; from = to) {
        to = f->queued;
        for (g = 3; g <= 4; g++)
            for (q = from; q < to; q++)
                if (f->groups[q] == g) Process(f, f->queue[q]);
    }
    f->listing = FALSE;
}

/* BoltFrame(); 0x080A9E14: a bolt's box (-7, -7 to 7, 7: an attack object, as King Dedede's hammer: NewAttack()), while Kracko is
 * unbeaten; */
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

/* NewAttack(); A box hurting Kirby (kind 2, the fight its parent: KatAM's HUD never shows it, nor a Kirby it touches), an Object's size
 * and zeroed past its ObjectBase (KatAM's HUD reads an HP there: none); */
static struct ObjectBase *NewAttack(struct Fight *f, s8 left, s8 top, s8 right, s8 bottom) {
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
    sub_0803E2B0(b, left, top, right, bottom);
    return b;
}

static void BoltFrame(struct Fight *f, struct Actor *a) {
    struct ObjectBase *b;

    if (f->over) {
        Destroy(f, a - f->a);
        return;
    }
    if (a->attack == NULL) a->attack = NewAttack(f, -7, -7, 7, 7);
    b = a->attack;
    b->x = a->ix << 8;
    b->y = a->iy << 8;
    b->flags &= ~0x200;
}

/* Body(); Its box on KatAM's object, where it is: its eye's (NiDL's 0x08749704: -10, -10 to 10, 10, hurting Kirby, taking no hits: an
 * attack object of its own, KatAM's object untouchable then, as KatAM's HUD shows what a Kirby touches, NiDL's no HP bar before its
 * cloud), none as its eye rises and before its cloud forms, its forming cloud's (0x0874973C: -32, -20 to 32, 20, no hits), its cloud's
 * (0x08749720, the same, hits; its other, 0x08749758, inside it); */
static void Body(struct Fight *f) {
    struct Object *obj = f->obj;
    struct Actor *a = &f->a[A_KRACKO];

    obj->base.x = a->ix << 8;
    obj->base.y = a->iy << 8;
    if (f->box == BOX_EYE) sub_0803E2B0(&obj->base, -10, -10, 10, 10);
    else sub_0803E2B0(&obj->base, -32, -20, 32, 20);
    if (f->over || f->box == BOX_NONE || f->box == BOX_EYE) obj->base.flags |= 0x200;
    else obj->base.flags &= ~0x200;
    if (f->guard || f->box != BOX_CLOUD) obj->base.flags |= 0x8000;
    else obj->base.flags &= ~0x8000;
    if (f->box == BOX_EYE && !f->over) {
        if (f->eyeBox == NULL) f->eyeBox = NewAttack(f, -10, -10, 10, 10);
        f->eyeBox->x = obj->base.x;
        f->eyeBox->y = obj->base.y;
        f->eyeBox->flags &= ~0x200;
    } else if (f->eyeBox) {
        f->eyeBox->flags |= 0x1000;
        f->eyeBox = NULL;
    }
}

/* Darken(); Its backgrounds darker while it strikes lightning (NiDL's 0x080657F8: their colours to black, DARK_STEP of 256 a frame to
 * DARKEST; KatAM's BLDCNT darken on the backgrounds, BLDY its sixteenths), then back (0x080657CC) as they were; at once beaten
 * (0x080657A4); */
static void Darken(struct Fight *f) {
    if (f->darkening > 0) {
        if (!f->darkened) {
            f->bldCnt = gBldRegs.bldCnt;
            f->bldY = gBldRegs.bldY;
            f->darkened = TRUE;
        }
        f->dark += DARK_STEP;
        if (f->dark > DARKEST) f->dark = DARKEST;
    } else if (f->darkened) {
        f->dark = f->darkening < 0 && f->dark > DARK_STEP ? f->dark - DARK_STEP : 0;
        if (!f->dark) {
            gBldRegs.bldCnt = f->bldCnt;
            gBldRegs.bldY = f->bldY;
            f->darkened = FALSE;
            f->darkening = 0;
            return;
        }
    } else {
        return;
    }
    gBldRegs.bldCnt = BLDCNT_EFFECT_DARKEN | BLDCNT_TGT1_BG0 | BLDCNT_TGT1_BG1 | BLDCNT_TGT1_BG2 | BLDCNT_TGT1_BG3 | BLDCNT_TGT1_BD;
    gBldRegs.bldY = f->dark >> 4;
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
    EffectObjectInitSprite(e, &e->sprite, tiles, gNidlData.bosses[NIDL_KRACKO].animId, frame, order);
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
    u16 animId = gNidlData.bosses[NIDL_KRACKO].animId;
    const u8 *fx = gNidlKracko.fx;

    if (!f->pal[PAL_EYE]) f->pal[PAL_EYE] = Palette(f->room, animId, fx[FX_EYE]);
    if (!f->pal[PAL_TOP]) f->pal[PAL_TOP] = Palette(f->room, animId | KEY_TOP, fx[FX_TOP]);
    if (!f->pal[PAL_BOTTOM]) f->pal[PAL_BOTTOM] = Palette(f->room, animId | KEY_BOTTOM, fx[FX_BOTTOM]);
}

// Colors(); Its palettes this frame: its hurt flash's colours (NiDL's 0x080663F4: the clock's 2 frames in 4, all three), else its own;
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
    for (p = PAL_TOP; p <= PAL_EYE; p++) SetColors(f->pal[p], flash ? gNidlKracko.flash + p * 16 : gNidlKracko.colors[p]);
}

/* DrawActor(); A child's sprites: its cloud (two: its top's palette, its bottom's), its puffs and orbs, a bolt; */
static void DrawActor(struct Fight *f, struct Actor *a) {
    const u8 *fx = gNidlKracko.fx, *tiles = gNidlKracko.tiles;
    u8 child = tiles[FX_TOP] > tiles[FX_PUFF] ? tiles[FX_TOP] : tiles[FX_PUFF];

    if (a->frame < 0 || !Watched(f)) {
        Hide(a->sprite);
        Hide(a->sprite2);
        return;
    }
    switch (a->kind) {
    case K_CHILD:
        if (a->frame < CLOUD_FRAMES) {
            Show(f, &a->sprite, child, a->ix, a->iy, fx[FX_TOP] + a->frame, PAL_TOP, a->flip, ORDER_TOP);
            Show(f, &a->sprite2, tiles[FX_BOTTOM], a->ix, a->iy, fx[FX_BOTTOM] + a->frame, PAL_BOTTOM, a->flip, ORDER_BOTTOM);
        } else {
            Hide(a->sprite2);
            Show(f, &a->sprite, child, a->ix, a->iy, fx[FX_PUFF] + a->frame - CLOUD_FRAMES, PAL_BOTTOM, a->flip, ORDER_TOP);
        }
        break;
    case K_BOLT: case K_BOLT2: case K_BOLT3:
        Show(f, &a->sprite, tiles[FX_BOLT], a->ix, a->iy, fx[FX_BOLT] + a->frame, a->frame == BOLT_END ? PAL_TOP : PAL_EYE, 1,
             a->kind == K_BOLT3 ? ORDER_EYE : ORDER_TOP);
        break;
    }
}

static void KrackoMain(struct Object *obj);
static void LetGo(struct Fight *f);
static void Respawns(struct Fight *f);

/* FightMain(); The fight's task: its sprites and its children's, its palettes, its backgrounds' darkening; as KatAM's explosion takes it,
 * its bolts gone (its frame held, its cloud with it); gone with it; */
static void FightMain(void) {
    struct Fight *f = TaskGetStructPtr(gCurTask);
    struct Object *obj = f->obj;
    struct Actor *a = &f->a[A_KRACKO];
    u8 i;

    if (obj == NULL || (obj->base.flags & 0x1000)) {
        f->over = TRUE;
        Respawns(f);
        LetGo(f);
        f->darkening = 0;
        Darken(f);
        Release(&f->eye);
        if (obj) ((struct Kracko *)obj)->f = NULL;
        TaskDestroy(gCurTask);
        return;
    }
    f->frameCount++;
    Palettes(f);
    if (obj->unk78 != KrackoMain && !f->over) {
        // (its killing blow: KatAM's defeat, its freeze and explosion; NiDL's 0x080A97D8, 0x0806A344);
        f->over = TRUE;
        f->hooked = FALSE;
        f->flashing = FALSE;
        if (f->hurt) {
            a->ix = f->hurtX;
            a->iy = f->hurtY;
            f->hurt = 0;
        }
        for (i = A_KRACKO + 1; i < ACTORS; i++)
            if (f->a[i].kind != K_CHILD) Destroy(f, i);
        Stop(a);
        SetFrame(a, DEFEAT_FRAME);
        f->darkening = 0;
    }
    Darken(f);
    Colors(f, f->flashing && (f->frameCount & 2));
    if (!Watched(f) || a->frame < 0) Hide(f->eye);
    else Show(f, &f->eye, gNidlKracko.tiles[FX_EYE], a->ix, a->iy, gNidlKracko.fx[FX_EYE] + a->frame, PAL_EYE, a->flip, ORDER_EYE);
    for (i = A_KRACKO + 1; i < ACTORS; i++) {
        struct Actor *c = &f->a[i];

        if (c->kind != K_NONE) DrawActor(f, c);
    }
}

// LetGo(); Its children gone (their sprites, the bolts' boxes), its eye's box;
static void LetGo(struct Fight *f) {
    s8 i;

    for (i = A_KRACKO + 1; i < ACTORS; i++) Destroy(f, i);
    if (f->eyeBox) f->eyeBox->flags |= 0x1000;
    f->eyeBox = NULL;
}

/* Respawns(); A player beaten in the fight on the tower's top (the camera locked there) while another Kirby is still up in the room:
 * KatAM keeps the room, the fight with it, and brings him back where he came in (his spawn: sub_08055920, then the room change,
 * sub_08055D9C), the tower's bottom, out of the fight's reach (its moves are the screen's, the screen his camera's: it went down
 * with him). His spawn is the tower's top where he was till he's back, then his own again; beaten alone, KatAM loads the room again
 * and the fight starts over from the bottom, as NiDL's stage (his own spawn, untouched); */
static void Respawns(struct Fight *f) {
    u8 i, j;

    for (i = 0; i < gNumHumanPlayers; i++) {
        struct Kirby *k = &gKirbys[i];
        bool32 others = FALSE;

        if (f->released && !f->over && k->roomId == f->room && (k->hp <= 0 || k->stateFn == sub_08055D9C)) {
            for (j = 0; j < gNumKirbys; j++)
                if (j != i && InRoom(f, &gKirbys[j]) && gKirbys[j].stateFn != sub_08055D9C) others = TRUE;
            if (others) {
                if (!(f->respawns & (1 << i))) {
                    s32 x = KirbyX(k);

                    f->respawns |= 1 << i;
                    f->spawns[i] = k->spawnLocation;
                    k->spawnLocation.x = ((x < 24 ? 24 : x > 216 ? 216 : x) + 8) >> 4;
                    k->spawnLocation.y = TOP_SPAWN_Y;
                }
                continue;
            }
        }
        if (f->respawns & (1 << i)) {
            k->spawnLocation = f->spawns[i];
            f->respawns &= ~(1 << i);
        }
    }
}

/* The fight (KrackoMain()): NiDL's each frame: a hit taken first (Hit(): KatAM's come between frames); each of NiDL's objects in its
 * order (Turns(): by group, then slot; hurt, its own script held), its hurt's step, the bolts' boxes, KatAM's object where it is
 * (Body()). Its two parts:
 * • Its eye, at the tower's bottom (NiDL's spot, 200, 2168, where Kirby comes in), four orbs about it, no HP bar, after the closest
 *   Kirby up the tower as he climbs, the camera following him (EyeFrame0()): still a while, drifting at him along one axis (1.25 a
 *   frame), turning at him; every 16 frames from 32 on, by chance (sooner with him landed), its lunge: a dash away (4 a frame, slowing),
 *   a pause (longer, him farther), then at him, faster: in the air, past him and slowing; landed, to his spot, then its loops there
 *   (EyeScript0()); him 120 frames in the air, homing in on him. It stays near the screen (32 above or below, 16 off its sides),
 *   touching it hurts, it takes no hits.
 * • As the camera's top comes to 367, it rises off the screen's top; as it comes to 69, the camera locks onto the tower's top (the
 *   arena, enemies/arena.c: ArenaGo()); there, with every Kirby up, over the side across from Kirby, its cloud forms (the HP bar
 *   shown), and the fight proper starts (Pick(), CloudScript1()): the lightning, then the swoop; then by turns its bob on its hover on
 *   its side, a Waddle Doo half the time (KatAM's, two out at most), its next attack by where Kirby is (floating high up: its swoop):
 *   its sweep across (1.25, bobbing), its lightning (the screen darker, a bolt every 4 frames across it), its swoop to the floor and
 *   back up. Hit (KatAM's attacks: KatAM's bosses take only the stronger ones), it freezes 13 frames, jittering, flashing in its hurt
 *   colours, its eye spinning (no hit stars: NiDL's makes none); no more hits for 31 frames. Touching it hurts (its box NiDL's: -32,
 *   -20 to 32, 20), its bolts too (-7, -7 to 7, 7 each). 60 HP for one player (NiDL's 60; nidl_boss.c); */
static void KrackoMain(struct Object *obj) {
    struct Kracko *k = (struct Kracko *)obj;
    struct Fight *f = k->f;
    s8 i;

    NidlUpdate(&k->n);
    obj->base.flags |= 4 | 0x400;
    obj->kirby3 = FindTargetKirby(&obj->base);
    if (f == NULL) return;
    Respawns(f);
    // The HP bar goes to whatever is hit; it takes it back, once its cloud shows it (ShowBar()); none before (HideBar());
    if (f->box == BOX_FORMING || f->box == BOX_CLOUD) sub_08034E14(obj);
    else HideBar(obj);
    obj->base.xspeed = obj->base.yspeed = 0;
    if (f->guard && !--f->guard) obj->base.flags &= ~0x8000;
    if (!f->over && obj->unk80 < f->hp) {
        // (one through all the same, or before its cloud takes hits: its HP back);
        if (f->guard || f->box != BOX_CLOUD) obj->unk80 = f->hp;
        else Hit(f);
    }
    f->hp = obj->unk80;
    Turns(f);
    if (f->hooked) HurtStep(f);
    for (i = A_KRACKO + 1; i < ACTORS; i++) {
        struct Actor *a = &f->a[i];

        if (a->attack && a->kind == K_NONE) a->attack->flags |= 0x1000;
    }
    Body(f);
}

// KrackoGone(); Its object's gone (nidl.c's NidlDestroy()): the fight's task lets go of it;
void KrackoGone(struct Object *obj) {
    struct Kracko *k = (struct Kracko *)obj;

    if (k->f) k->f->obj = NULL;
    k->f = NULL;
}

/* KrackoStart(); The fight starts (NidlBossStart(): Kirby in its area, the tower's bottom), its task with it: NiDL's 0x080A7D98: its eye,
 * its orbs (its child, 0xC3), its child's mode 1, its lunge (state 0); the HP bar KatAM showed taken back (NiDL's shows with its cloud);
 * the arena at the tower's top waiting for it (ArenaLock()); */
void KrackoStart(struct Object *obj) {
    struct Kracko *k = (struct Kracko *)obj;
    struct Task *t;
    struct Fight *f;
    struct Actor *a;
    s8 i;

    ObjectSetFunc(obj, -1, KrackoMain);
    if (k->started) return;
    k->started = TRUE;
    HideBar(obj);
    t = TaskCreate(FightMain, sizeof(struct Fight), 0x3500, TASK_USE_EWRAM, NULL);
    f = k->f = TaskGetStructPtr(t);
    CpuFill32(0, f, sizeof(struct Fight));
    f->obj = obj;
    f->room = obj->base.roomId;
    f->unk56 = obj->base.unk56;
    f->seed = Rand16() & 0xFFF;
    f->hp = obj->unk80;
    f->mode = 1;
    f->count = 0;
    f->box = BOX_EYE;
    a = &f->a[A_KRACKO];
    a->kind = K_KRACKO;
    a->facing = a->flip = 1;
    a->frame = 4;
    a->moving = TRUE;
    a->x = obj->base.x << 8;
    a->y = obj->base.y << 8;
    a->ix = a->x >> 16;
    a->iy = a->y >> 16;
    Stop(a);
    SetState(a, 0);
    a->script = 0;
    i = Spawn(f, K_CHILD, a->ix, a->iy);
    if (i != NONE) f->a[i].moving = FALSE;
    // (its sprites are the fight's task's: KatAM's object's never shown, its VRAM theirs; unkC 0x200 keeps KatAM from giving it VRAM
    // again, flag 8 from animating it);
    obj->base.unkC |= 0x200;
    if (obj->base.sprite.tilesVram >= 0x6014000) VramFree(obj->base.sprite.tilesVram);
    obj->base.sprite.tilesVram = 0;
    obj->base.flags |= 8;
}

/* KrackoInit(); On NiDL's spot (its template's: the tower's bottom), hidden (the fight's task draws it), through anything (its moves are
 * its own: Motion()); its eye's box; */
void KrackoInit(struct NidlObject *n) {
    struct Kracko *k = (struct Kracko *)n;
    struct Object *obj = &n->obj;

    k->f = NULL;
    k->started = FALSE;
    obj->base.flags |= 0x400 | 0x100;
    obj->base.unk5C = (obj->base.unk5C & ~7) | 3 | 0xA0;    // (not KatAM's Kracko's electric touch: NiDL's only hurts);
    sub_0803E2B0(&obj->base, -10, -10, 10, 10);
    ObjectSetBounds(&obj->base, -10, -10, 10, 10);
}
