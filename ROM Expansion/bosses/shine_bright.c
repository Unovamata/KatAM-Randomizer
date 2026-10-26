/* Mr. Shine & Mr. Bright; ported from Kirby: Nightmare in Dream Land (USA), boss object type 0x3F (spawn 0x080A19CC, its four
 * objects 0x087484D4), as NiDL boss NIDL_SHINE_BRIGHT, Butter Building's (room 0x164 here). NiDL's, from its code: the controller
 * (type 0x3F's first object, 0x080A3114: 4 states), Mr. Shine (0x080A3298) and Mr. Bright (0x080A3C28: 18 states each, a script
 * each, 0x08748644 and 0x087486D4, and a function every frame after it, 0x0874868C and 0x0874871C), Mr. Bright's flames (0x080A472C),
 * their shots (type 0x7F, 0x080A4C84: the crescent, the fireball, the falling star, the beam), the beam's sparkles (type 0xC1,
 * 0x080A54E4) and sparks (0x8B, 0x08067120), the dash and fire trails (0xA7, 0xA8: 0x0806E6F8, 0x0806E808), the screen darkening
 * under the beam (0xAE, 0x080657F8) and the sky's fade (0x080275CC), checked frame by frame against NiDL's recorded fight (their
 * states, frames, spots and speeds); see "The fight" below. NiDL's objects are Actors here (one array, NiDL's object slots 32 on, in
 * NiDL's order), each drawn as a sprite of its own (EffectOnly frames: KatAMExpansion.cs' ShineAndBright()); the boss object KatAM
 * knows (hit, its HP bar) is the one on the ground (NiDL's: its box the one Kirby can hit; the HP bar its HP, the controller's); */
#include "../nidl.h"
#include "code_0806F780.h"
#include "task.h"
#include "main.h"
#include "functions.h"
#include "constants/songs.h"

/* gNidlShineBright; Filled in by KatAMExpansion.cs (WriteShineBrightFight()): the VRAM tiles of each group of their frames' biggest
 * (ShineBrightFrames' groups, G_*), NiDL's colours (Mr. Shine's glow, the four palettes it goes round, his hurt flash's, Mr.
 * Bright's, Mr. Bright's own: C_*) and the room's sky by day, at sunset and by night (KatAM's BG palettes 6-13: SKY_*); */
struct NidlShineBright {
    u8 tiles[16];
    u16 colors[7][16];
    u16 sky[3][128];
};

const struct NidlShineBright gNidlShineBright = { { 0 } };

// Their frames (KatAMExpansion.cs' ShineBrightFrames, in its order; frame 0 the object's own, never shown);
#define FRAME_SHINE 1               // NiDL's 0x08754210's 4-19 (SHINE());
#define FRAME_CRESCENT 17           // 0x08754260's 4-7;
#define FRAME_SHOT 21               // 0x08754280's 0-3;
#define FRAME_BEAM 25               // 0x08754290's 0-5;
#define FRAME_BRIGHT 31             // 0x0875412C's 4-20 (BRIGHT());
#define FRAME_FLAMES 48             // 0x0875412C's 21-26 (0x08754180's), 33-38 (0x087541B0's);
#define FRAME_FLAMES_BACK 60        // 0x087541E0's 0-5;
#define FRAME_TRAIL 66              // 0x0874CCA4's 0, 2, 4;
#define FRAME_FIRE 69               // 0x0874C718's 0, 2, ..., 18;
#define FRAME_SPARK 79              // 0x0874CCBC's 4-13;
#define FRAME_PUFF 89               // 0x0874C9D8's 0-16;
#define SHINE(f) (FRAME_SHINE + (f) - 4)
#define BRIGHT(f) (FRAME_BRIGHT + (f) - 4)
enum { G_SHINE, G_CRESCENT, G_SHOT, G_BEAM, G_BRIGHT, G_FLAMES, G_FLAMES_BACK, G_TRAIL, G_FIRE, G_SPARK, G_PUFF };
enum { C_GLOW, C_SHINE_HURT = 4, C_BRIGHT_HURT, C_BRIGHT };
enum { SKY_DAY, SKY_SUNSET, SKY_NIGHT };

#define FRAME_HIDDEN -1
#define NO_MAX 0x7FFFFFFF
#define PC_END 0xFFFF
#define LOOP -3                     // (NiDL's animations' ends: back to the start, or held);
#define END -2
#define NIDL_SCREEN_X 8             // NiDL's camera in their room (its top left, there all along; KatAM shows the room as NiDL's);
#define NIDL_SCREEN_Y 8
#define EDGE 111                    // The screen's edges for them (NiDL's 0x080227A4: its middle's x +-111, its top 72 up from its middle);
#define TOP 72
#define FLOOR_Y 144                 // Their room's floor (flat), their feet (NiDL's terrain boxes: 0x08748908, 0x08748910);
#define SHINE_FEET 19
#define BRIGHT_FEET 16
#define OFF_SCREEN 64               // Their shots gone this far off the screen (NiDL's: its objects');
#define KIRBY_DY 4                  // (NiDL's Kirby stands 4 pixels higher: KatAM's Kirby's spot, NiDL's: KirbyY());
#define HURT_TIME 13                // NiDL's hurt (0x0806619C): frozen, jittering, flashing; the room's layers mosaic;
#define MOSAIC_LAYERS 0x6
#define REG_MOSAIC_ADDR 0x0400004C
#define INTRO_TIME 128              // The controller's intro (0x080A19EC): the HP bar filling for each, KatAM's own fill meanwhile;
#define ORDER(p) (0x0F + (p))       // KatAM's sprite order for NiDL's priority p (+0x42: the lower in front);
#define KEY_BRIGHT 0x2000           // Their palettes' keys (| the anim ID; Mr. Shine's is the anim ID: his, the crescent's, the star's
#define KEY_BEAM 0x4000             // blink, NiDL's slot 8);
#define KEY_COMMON 0x6000
#define KEY_SHOT 0x8000

// Their palettes' slots (KatAM's);
enum { PAL_SHINE, PAL_BRIGHT, PAL_SHOT, PAL_BEAM, PAL_COMMON, PALS };

// NiDL's objects (Actor.kind) and their slots (NiDL's 32 on: the controller, Mr. Shine, Mr. Bright, his flames, then the rest);
enum { K_NONE, K_CONTROLLER, K_SHINE, K_BRIGHT, K_FLAMES, K_CRESCENT, K_SHOT, K_BEAM, K_PART, K_SPARK, K_TRAIL, K_FIRE, K_STAR, K_PUFF };
enum { A_CONTROLLER, A_SHINE, A_BRIGHT, A_FLAMES, A_CHILDREN };
#define ACTORS 31
#define NONE -1

// Their boxes (NiDL's sub +0x48: 0x0874883C and 0x087488AC on the ground, hit and touching; 0x08748858 and 0x087488C8 in the sky,
// neither; none in the intro);
enum { BOX_NONE, BOX_GROUND, BOX_SKY };

/* NiDL's tables (0x087484E4 on): the moves by HP (0x087484EC: over half, under), their states (0x087484E4), Mr. Shine's steps
 * pacing (0x0874851C, 0x0874852C), Mr. Bright's bounce (0x08748604, 0x08748614); */
static const s32 sMoveWeights[8] = { 3, 2, 3, 0, 2, 1, 1, 4 };
static const u16 sMoveStates[4] = { 14, 10, 11, 9 };
static const s32 sPaceSteps[4] = { 12, 8, 4, 0 };
static const s32 sPaceSpeeds[4] = { 0, 0x20000, 0x10000, 0 };
static const s32 sBounceSteps[4] = { 36, 24, 12, 0 };
static const s32 sBounceSpeeds[4] = { -0xC000, -0x8000, 0xC000, 0x8000 };

// Their animations ((frame, frames shown) pairs, then LOOP or END: NiDL's 0x0874853C on, 0x08748A00);
static const s16 sShineDashRight[] = { 4, 2, 5, 2, 6, 2, 7, 2, 8, 2, 9, 2, LOOP };
static const s16 sShineDashLeft[] = { 8, 2, 7, 2, 6, 2, 5, 2, 4, 2, 9, 2, LOOP };
static const s16 sBrightIdle[] = { 4, 8, 5, 8, 6, 8, 5, 8, LOOP };
static const s16 sBrightSun[] = { 16, 6, 19, 6, 17, 6, 18, 6, LOOP };
static const s16 sBrightSunFast[] = { 16, 2, 19, 2, 17, 2, 18, 2, LOOP };
static const s16 sBrightHop[] = { 9, 12, 7, 12, 8, 12, END };
static const s16 sBrightBigHop[] = { 9, 12, 8, 12, 7, 24, END };
static const s16 sBrightBack[] = { 8, 12, 7, 12, LOOP };
static const s16 sBrightDizzy[] = { 15, 12, LOOP };
static const s16 sBrightShot[] = { 14, 12, LOOP };
static const s16 sBrightUp[] = { 8, 12, 13, 24, 11, 12, END };
static const s16 sBrightLeave[] = { 11, 12, LOOP };
static const s16 sCrescentRight[] = { 4, 2, 5, 2, 6, 2, 7, 2, LOOP };
static const s16 sCrescentLeft[] = { 7, 2, 6, 2, 5, 2, 4, 2, LOOP };

// The falling stars' spots across (0x08748A38: by how many are left) and speeds (0x08748A40); the beam's sparkles' (0x08748A80, 0x08748A94);
static const s16 sStarSpots[4] = { 48, 160, 96, 208 };
static const s32 sStarSpeeds[5] = { -0x18000, -0xC000, 0, 0xC000, 0x18000 };
static const s32 sSparkleSpotsA[5] = { -0x80000, -0x40000, 0, 0x40000, 0x80000 };
static const s32 sSparkleSpotsB[7] = { 0xC0000, 0x80000, 0x40000, 0, -0x40000, -0x80000, -0xC0000 };

// Hurt (0x0806627C): the jitter's steps by its countdown (0x0873E184, 0x0873E16C), the room's mosaic (as Meta Knight's);
static const u8 sJitterOf[14] = { 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4 };
static const s8 sJitter[6][2] = { { 2, 1 }, { -2, -1 }, { -2, 0 }, { 2, 1 }, { 2, -1 }, { -2, 0 } };
static const u8 sMosaic[7] = { 0, 0, 0, 0, 0, 0x22, 0x44 };

/* NiDL's sine (0x0872FA30: 640 entries, 256 one; 512 a circle, 0 right, 128 down: the cosine 128 on), for its aims (0x0806421C); */
static const s16 sSine[640] = {
#include "shine_bright_sine.h"
};

/* Actor; One of NiDL's objects (its fields, by their NiDL offsets): a script (WAIT() resumes it, PC_END: over), its move (NiDL's
 * 0x080656B4, or on its parent: 0x080059FC) and its state's function each frame, in NiDL's order (its slot's); its sprite; */
struct Actor {
    u8 kind, variant, extra;            // (K_*; +0x73, +0x74);
    u8 state, next;                     // (+0x15, +0x14);
    u8 script;                          // The state whose script is running (+0x14 as it started);
    bool8 restart;                      // Its next state starts as it's next run (NiDL's startScript);
    bool8 fresh;                        // Made this frame (its script ran then);
    bool8 relative;                     // Its spot is from its parent's (0x080059FC);
    u16 pc, wait;
    s8 facing, flip;                    // (+0x43; its frame's mirroring, +0x3E: its facing as its frame was last set);
    s16 frame;                          // (+0x3C; NiDL's: FRAME_HIDDEN none);
    u8 prio;                            // (+0x42);
    s8 parent;                          // (+0x44: its parent's Actor);
    s8 child;                           // (+0x46);
    s32 x, y;                           // 16.16 (+0x4C; relative: from its parent);
    s16 ix, iy;                         // Its pixel (+0x48);
    s32 vx, vy, ax, ay, maxX, maxY;     // (+0x54 on);
    s32 v18, v1C, v20, v24, v28, v2C, v30, v34;
    s16 loop;                           // (+0x6C);
    bool8 frozen;                       // (+0x6E: its state's function not run);
    bool8 warn;                         // (+0x70: flashing);
    bool8 landed;                       // (+0x7A);
    s16 hp;                             // (+0x78);
    const s16 *anim;                    // Its animation (sub +0x2C: NULL none; its step +9; mode 1: its frame only, +8);
    u8 animStep, animMode;
    u8 box;                             // (sub +0x48: BOX_*);
    u8 prevState;                       // (sub +0x1C: the state before setState's);
    bool8 held;                         // Its palette held by its flash (sub +0xA's bit 0);
    u8 hurt;                            // Its hurt's countdown (0: not hurt);
    s32 hurtX, hurtY;
    s16 age;                            // (the puff's frames);
    u8 group;                           // Its sprite's tiles' group (G_*);
    struct EffectObject *sprite;
    struct ObjectBase *attack;          // Its box hurting Kirby (an attack object);
    struct Object *enemy;               // The falling star's KatAM enemy (enemies/star.c);
};

struct ShineBright {
    struct NidlObject n;
    struct Fight *f;
    bool8 started;
};

typedef char sShineBrightFits[sizeof(struct ShineBright) <= NIDL_OBJECT_SIZE ? 1 : -1];

/* Fight; The fight's state (a task of its own), NiDL's objects and globals (0x02007D00: g*, 0x02006040: the beam's children and the
 * fire trails, sky height); */
struct Fight {
    struct Object *obj;                 // (NULL: gone, ShineBrightGone());
    struct Actor a[ACTORS];
    s32 gSwap;                          // (+0xC: 1 asked, 3 the one on the ground going up);
    s32 gSkyAttack;                     // (+0x10);
    s32 gBeaten;                        // (+0x14: how many beaten);
    s32 gOn;                            // (+0x18: their boxes on);
    s32 gWait;                          // (+0x1C: in the sky waiting; at the end, how many reached the middle);
    s32 gHalf;                          // (+0x20: half their HP);
    bool8 gShineDown;                   // (+8: Mr. Shine on the ground);
    s8 kids[4];                         // (0x02006040's +0, +4, +8, +0xC: the beam and its sparkles; the fire trails);
    s32 skyHeight;                      // (0x02006040 + 0x10);
    bool8 beamSound;                    // (0x02006040 + 0x18: its loop on);
    u16 seed;                           // NiDL's random numbers' (0x03000FB4);
    s16 hp;                             // KatAM's object's HP last frame (less now: hit);
    s8 ground;                          // The Actor KatAM's object is (the one Kirby can hit), or NONE;
    struct Kirby *target;               // (NiDL's 0x030023D4: the Kirby last found; the sky's chase's);
    bool8 hit;                          // Hit this frame: its state's function not run (0x080692FC);
    bool8 over;                         // Beaten (its defeat under way: KatAM's explosion);
    u16 frameCount;                     // (0x03001EA4: the flashes' clock);
    s32 unk68;                          // KatAM's object's attack (its killing blow clears it: given back);
    // The sky (0x080275CC: 0x02006098 {cur | 0x80 fading, from, to, queued, dir}) and the darkening (type 0xAE, 4: 0x080657F8);
    s8 skyCur, skyFrom, skyTo, skyQueued, skyDir;
    bool8 skyFading;
    s16 skyT;
    u8 darkMode;                        // 0 darkening, 1 lightening, 2 back at once, 3 held; DARK_OFF none;
    s16 dark, darkStep, darkMax;
    u16 backup[224];                    // (0x02005E10: KatAM's BG palettes 0-13, NiDL's 2-15);
    const u16 *shineColors;             // Mr. Shine's slot's colours (NiDL's slot 8: the crescent's, the star's blink's);
    const u16 *shineFlash;              // His hurt's flash over them (NULL: none);
    const u16 *brightColors;            // Mr. Bright's slot's (his flames');
    u8 pal[PALS];
    u16 room;                           // Their room (KatAM's object's);
    u8 freeze;                          // Frames the room's held still yet (Beaten(): KatAM's sub_08033540, as NiDL's), 0 none;
    u8 player;                          // (whose freeze: KatAM's object's unk56);
};

#define DARK_OFF 0xFF
#define BEATEN_FREEZE 27            // (NiDL's game held still after a killing blow's frame: the room, Kirby too; FightMain() lets it go);

/* The scripts: WAIT() resumes them that many frames on; END_SCRIPT() ends them (NiDL's endScript); the variables they keep across
 * waits are the Actor's; */
#define SCRIPT switch (a->pc) { case 0:
#define WAIT(n) do { a->wait = (n); a->pc = __LINE__; return; case __LINE__:; } while (0)
#define END_SCRIPT() do { a->pc = PC_END; return; } while (0)
#define SCRIPT_END } a->pc = PC_END

// Rand(); NiDL's (0x08002EE8): 0 to n - 1;
static u16 Rand(struct Fight *f, u16 n) {
    f->seed = (f->seed * 61 + 0x579) & 0xFFF;
    return f->seed * n >> 12;
}

/* KirbyX(), KirbyY(); A Kirby's spot as NiDL's reads it, after his move this frame (NiDL moves the Kirbys before them, KatAM after:
 * his speed added; his y KIRBY_DY up, as NiDL has it); */
static s32 KirbyX(struct Kirby *k) {
    return (k->base.x + k->base.xspeed) >> 8;
}

static s32 KirbyY(struct Kirby *k) {
    return ((k->base.y - k->base.yspeed) >> 8) - KIRBY_DY;
}

static bool32 InRoom(struct Fight *f, struct Kirby *k) {
    return k->base.roomId == f->obj->base.roomId && k->hp > 0;
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

// KirbyRight(); 0x08064398: the closest Kirby to the right (4), left (8), level or none (0);
static u8 KirbyRight(struct Fight *f, struct Actor *a) {
    struct Kirby *k = Closest(f, a);
    s32 dx = k ? KirbyX(k) - a->ix : 0;

    return dx == 0 ? 0 : dx > 0 ? 4 : 8;
}

/* KirbyIn(); 0x080647FC (y) and 0x08064758 (x): the Kirby (nearest of them to `a`, 0x08063BF4) whose spot on the screen is in [lo,
 * hi), or NULL (NiDL keeps its index, 0x030023D4: `last`, kept as it was when there's none); */
static struct Kirby *KirbyIn(struct Fight *f, struct Actor *a, s32 lo, s32 hi, bool32 across, struct Kirby **last) {
    struct Kirby *best = NULL;
    s32 d, bestD = 0, p;
    u8 i;

    for (i = 0; i < gNumKirbys; i++) {
        struct Kirby *k = &gKirbys[i];

        if (!InRoom(f, k)) continue;
        p = across ? KirbyX(k) - NIDL_SCREEN_X : KirbyY(k) - NIDL_SCREEN_Y;
        if (p < lo || p >= hi) continue;
        d = (KirbyX(k) - a->ix) * (KirbyX(k) - a->ix) + (KirbyY(k) - a->iy) * (KirbyY(k) - a->iy);
        if (best == NULL || bestD > d) {
            best = k;
            bestD = d;
        }
    }
    *last = best;
    return best;
}

/* ArcTan2(); The GBA BIOS's (NiDL's 0x080CFA50, swi 0xA: 0-0xFFFF, a circle); */
u16 ArcTan2(s16 x, s16 y);

/* Aim(); The speed at `speed` (256 a pixel a frame) from `a` towards a Kirby (NiDL's 0x080642FC with 3, 0x0806421C: ArcTan2's angle,
 * 512 a circle, its sine); */
static void Aim(struct Actor *a, struct Kirby *k, s16 speed, s32 *vx, s32 *vy) {
    s32 dx = (k ? KirbyX(k) : a->ix) - a->ix, dy = (k ? KirbyY(k) : a->iy) - a->iy;
    u16 angle = ArcTan2(dx, dy) >> 7;

    *vx = sSine[angle + 128] * speed;
    *vy = sSine[angle] * speed;
}

// AimAt(); The same at an Actor's spot (0x080642FC on the controller);
static void AimAt(struct Actor *a, struct Actor *to, s16 speed) {
    u16 angle = ArcTan2(to->ix - a->ix, to->iy - a->iy) >> 7;

    a->vx = sSine[angle + 128] * speed;
    a->vy = sSine[angle] * speed;
}

static void Stop(struct Actor *a) {
    a->vx = a->vy = a->ax = a->ay = 0;
    a->maxX = a->maxY = NO_MAX;
}

// SetFrame(); NiDL's 0x08006338: its frame, mirrored by its facing (NiDL's other frame changes keep its mirroring);
static void SetFrame(struct Actor *a, s16 frame) {
    a->frame = frame;
    a->flip = a->facing;
}

// VelX(); NiDL's 0x080061C0: x speed for facing right, mirrored otherwise;
static void VelX(struct Actor *a, s32 vx) {
    a->vx = a->facing == 1 ? vx : -vx;
}

// VelY(); NiDL's 0x0800622C: y speed, its change and its cap;
static void VelY(struct Actor *a, s32 vy, s32 ay, s32 maxY) {
    a->vy = vy;
    a->ay = ay;
    a->maxY = maxY;
}

// TurnAround(); 0x08063FF4: facing (its frame mirrored again) and moving the other way;
static void TurnAround(struct Actor *a) {
    a->facing = -a->facing;
    SetFrame(a, a->frame);
    a->vx = -a->vx;
    a->ax = -a->ax;
}

/* Edges(); NiDL's 0x080227A4: kept on the screen (put back there, its pixel), which edges it's at (1 left, 2 right, 4 the top); */
static u8 Edges(struct Actor *a) {
    s32 middle = NIDL_SCREEN_X + DISPLAY_WIDTH / 2, top = NIDL_SCREEN_Y + DISPLAY_HEIGHT / 2 - TOP;
    u8 edges = 0;

    if (a->ix < middle - EDGE) {
        a->ix = middle - EDGE;
        a->x = a->ix << 16;
        edges = 1;
    } else if (a->ix > middle + EDGE) {
        a->ix = middle + EDGE;
        a->x = a->ix << 16;
        edges = 2;
    }
    if (a->iy < top) {
        a->iy = top;
        a->y = a->iy << 16;
        edges |= 4;
    }
    return edges;
}

// AtSide(); 0x080A2390: at the screen's left or right;
static bool32 AtSide(struct Actor *a) {
    return (Edges(a) & 3) != 0;
}

// ScreenY(); Its pixel's y on NiDL's screen;
static s32 ScreenY(struct Actor *a) {
    return a->iy - NIDL_SCREEN_Y;
}

static void SetY(struct Actor *a, s32 y) {
    a->iy = y;
    a->y = y << 16;
}

/* Animations (sub +0x2C): Play() (0x080640DC: its frame mirrored by its facing at each step) and PlayFrames() (0x080640A8: its frames
 * only) step it at once and give the step's frames; Step() (0x080640FC); AnimTick() (0x08064188: the frames left of the step, the
 * next once out) and AnimTickFacing() (0x0806415C: facing Kirby at each step); */
static s16 Step(struct Actor *a) {
    const s16 *e = &a->anim[a->animStep * 2];

    if (e[0] == LOOP) {
        a->animStep = 0;
        e = a->anim;
    } else if (e[0] == END) {
        a->anim = NULL;
        a->animStep++;
        return END;
    }
    if (a->animMode) a->frame = e[0];
    else SetFrame(a, e[0]);
    a->animStep++;
    return e[1];
}

static s16 Play(struct Actor *a, const s16 *anim) {
    a->anim = anim;
    a->animStep = 0;
    a->animMode = 0;
    return Step(a);
}

static s16 PlayFrames(struct Actor *a, const s16 *anim) {
    a->anim = anim;
    a->animStep = 0;
    a->animMode = 1;
    return Step(a);
}

static s32 AnimTick(struct Actor *a, s32 t) {
    if (a->anim == NULL) return t;
    if (t <= 0) t = Step(a);
    return t - 1;
}

static s32 AnimTickFacing(struct Fight *f, struct Actor *a, s32 t) {
    if (a->anim == NULL) return t;
    if (t <= 0) {
        FaceKirby(f, a);
        t = Step(a);
    }
    return t - 1;
}

/* SetState(); NiDL's 0x0806395C: its next state (+0x14; the state it's in kept, sub +0x1C); Restart(): it starts as the Actor's next
 * run (0x08006148: the same frame, the Actor after the one restarting it; else the next); */
static void SetState(struct Actor *a, u8 state) {
    a->prevState = a->next;
    a->next = state;
}

static void Restart(struct Actor *a) {
    a->restart = TRUE;
}

static void SetRestart(struct Actor *a, u8 state) {
    SetState(a, state);
    Restart(a);
}

/* Their objects: Spawn() takes NiDL's lowest free slot (0x08005904) for a child of `parent`; a shot (0x08064A78: NiDL's projectiles,
 * the sparks) starts its script there and then (fresh: not again this frame), an effect (0x08064C1C) as it's next run; */
static void Destroy(struct Fight *f, s8 i);
static void ChildScript(struct Fight *f, struct Actor *a);

static void Make(struct Fight *f, s8 i, u8 kind, s8 parent, u8 variant, u8 extra, s32 x, s32 y) {
    struct Actor *a = &f->a[i], *p = &f->a[parent];

    a->kind = kind;
    a->variant = variant;
    a->extra = extra;
    a->state = a->next = 0;
    a->restart = FALSE;
    a->fresh = FALSE;
    a->relative = FALSE;
    a->pc = a->wait = 0;
    a->facing = p->facing;
    a->flip = 1;
    a->frame = FRAME_HIDDEN;
    a->prio = 9;
    a->parent = parent;
    a->child = NONE;
    a->ix = x;
    a->iy = y;
    a->x = x << 16;
    a->y = y << 16;
    Stop(a);
    a->v18 = a->v1C = a->v20 = a->v24 = a->v28 = a->v2C = a->v30 = a->v34 = 0;
    a->loop = 0;
    a->frozen = a->warn = a->landed = FALSE;
    a->hp = 2;
    a->anim = NULL;
    a->animStep = a->animMode = 0;
    a->box = BOX_NONE;
    a->prevState = 0;
    a->held = FALSE;
    a->hurt = 0;
    a->age = 0;
    a->group = G_PUFF;
    a->sprite = NULL;
    a->attack = NULL;
    a->enemy = NULL;
    a->script = 0;
}

static s8 Spawn(struct Fight *f, u8 kind, s8 parent, u8 variant, u8 extra, s32 x, s32 y, bool32 now) {
    s8 i;

    for (i = A_CHILDREN; i < ACTORS && f->a[i].kind != K_NONE; i++) ;
    if (i >= ACTORS) return NONE;
    Make(f, i, kind, parent, variant, extra, x, y);
    if (now) {
        ChildScript(f, &f->a[i]);
        f->a[i].fresh = TRUE;
    }
    return i;
}

/* Shoot(); A shot (0x08064B5C): `ahead` pixels ahead of `a` (by its facing), `down` down; not in a wall (NiDL's 0x08021A40: the
 * room's solid tiles; outside it, none); */
static bool32 Solid(struct Fight *f, s32 x, s32 y) {
    struct Object *obj = f->obj;
    struct LevelInfo *level = &gCurLevelInfo[obj->base.unk56];

    if (x < 0 || y < 0 || x >= level->roomWidth || y >= level->roomHeight) return FALSE;
    return !(gCollisionAttributes[GetCollisionTile(obj->base.unk56, x >> 4, y >> 4)] & 1);
}

static s8 Shoot(struct Fight *f, s8 from, u8 kind, u8 variant, u8 extra, s32 ahead, s32 down, bool32 notInWall) {
    struct Actor *a = &f->a[from];
    s32 x = a->ix + ahead * a->facing, y = a->iy + down;

    if (notInWall && Solid(f, x, y)) return NONE;
    return Spawn(f, kind, from, variant, extra, x, y, TRUE);
}

/* Their sounds (NiDL's own: KatAM's nearest); */
#define SFX_DASH SE_KRACKO_SWEEP_ATTACK         // (0x1F4);
#define SFX_WALL SE_MINIBOSS_PRE_EXPLODE        // (0x1F7: the wall's thud);
#define SFX_BEATEN SE_MINIBOSS_PRE_EXPLODE      // (0x1FE);
#define SFX_CRESCENT SE_KIRBY_STARSHOT_1        // (0x226: the crescent, a falling star);
#define SFX_SHOT SE_KIRBY_STARSHOT_2            // (0x227);
#define SFX_BEAM SE_DARK_MIND_LASER_BEAM        // (0x216, looping while it shines);
#define SFX_STAR SE_KIRBY_STAR_RELEASE          // (0x213-0x215: the falling stars');

static void Sound(struct Fight *f, u16 sfx) {
    PlaySfx(&f->obj->base, sfx);
}

static void SkyTo(struct Fight *f, s8 sky);
static void Darken(struct Fight *f, s16 step, s16 max);
static void Lighten(struct Fight *f, s16 step);
static void Undarken(struct Fight *f);

/* Their shared moves (NiDL's 0x080A1DD4 on; `a` Mr. Shine or Mr. Bright); */

// Swap(); 0x080A1DD4: the one in the sky asked to come down (gSwap 1, none beaten), the one on the ground goes up (state 2);
static void Swap(struct Fight *f, struct Actor *a) {
    if (f->gBeaten == 0 && f->gSwap) {
        f->gSwap = 3;
        SetState(a, 2);
    }
}

// Landed(); 0x080A1DF8 (their floor's): down from the sky (state 8), on the ground (state 1);
static void Landed(struct Fight *f, struct Actor *a) {
    if (a->next == 8) SetRestart(a, 1);
}

// ClearWarn(); 0x080A2C90: its palette its own again, no warning flash;
static void ClearWarn(struct Actor *a) {
    a->held = FALSE;
    a->warn = FALSE;
}

static void BeatenEffects(struct Fight *f, struct Actor *a);
static void DestroyTrails(struct Fight *f);

/* Beaten(); 0x080A1EC4: its HP out: the room held still (BEATEN_FREEZE), a shake, its burst (BeatenEffects()), its trail gone (Mr.
 * Bright's fire trails too); the first up into the sky (state 2), the second still (state 15), once the room moves again */
void sub_08033540(u8);
void sub_080335B4(u8);

static bool32 BeatenCheck(struct Fight *f, struct Actor *a);

static void Beaten(struct Fight *f, struct Actor *a) {
    struct Actor *b;

    // (everything held still a while first, KatAM's freeze: its objects, the Kirbys; NiDL's 0x0806D4E4(1, 0));
    f->player = f->obj->base.unk56;
    f->freeze = BEATEN_FREEZE;
    sub_08033540(f->player);
    NidlShake(4, &f->obj->base);
    BeatenEffects(f, a);
    Sound(f, SFX_BEATEN);
    if (a->child != NONE) {
        Destroy(f, a->child);
        a->child = NONE;
    }
    if (a->kind == K_BRIGHT) DestroyTrails(f);
    SetState(a, f->gBeaten ? 15 : 2);
    a->v20++;
    f->gBeaten++;
    Restart(a);
    Stop(a);
    /* (NiDL's hit lands in the beaten one's own turn, its slot's: the other, after it, still sees it that frame, its state's check
     * in the sky, BeatenCheck(); KatAM's comes between frames, after both: Mr. Shine's beaten, Mr. Bright's check now); */
    b = &f->a[A_BRIGHT];
    if (a->kind == K_SHINE && b->state >= 3 && b->state <= 7 && !b->frozen) BeatenCheck(f, b);
}

// LeaveSky(); 0x080A2030: down out of the sky (high up, falling at once; else a hop up first);
static void LeaveSky(struct Fight *f, struct Actor *a) {
    if (ScreenY(a) <= 31) VelY(a, 0x10000, 0x1500, 0x60000);
    else VelY(a, -0x30000, 0x1500, 0x60000);
    ClearWarn(a);
    a->prio = 10;
}

/* UpToSky(); 0x080A2090: up into the sky (its height there 16-39 down the screen), facing the screen's middle; the other on the
 * ground; the day (Mr. Bright up) or the night (Mr. Shine); */
static void UpToSky(struct Fight *f, struct Actor *a) {
    f->skyHeight = 16 + Rand(f, 24);
    a->facing = a->ix - NIDL_SCREEN_X <= 99 ? 1 : -1;
    if (a->kind == K_BRIGHT) {
        f->gShineDown = TRUE;
        a->box = BOX_SKY;
        SkyTo(f, SKY_DAY);
    } else {
        f->gShineDown = FALSE;
        a->box = BOX_SKY;
        SkyTo(f, SKY_NIGHT);
    }
    f->gSkyAttack = 0;
    a->landed = FALSE;
    Stop(a);
    VelX(a, 0x8000);
    VelY(a, -0x60000, 0x2500, 0x30000);
    ClearWarn(a);
    a->prio = 12;
}

// KeepBelowTop(); 0x080A2164: kept 17 pixels under the screen's top;
static void KeepBelowTop(struct Actor *a) {
    if (ScreenY(a) <= 15) {
        SetY(a, NIDL_SCREEN_Y + 17);
        a->vy = 0;
    }
}

// Rise(); 0x080A21A0: up into the sky, then down to its height there (the sky, state 3);
static void Rise(struct Fight *f, struct Actor *a) {
    s32 sy = ScreenY(a);

    if (a->vy >= 0 && sy > f->skyHeight) {
        SetY(a, NIDL_SCREEN_Y + f->skyHeight);
        Stop(a);
        SetRestart(a, 3);
    } else if (sy <= 16) {
        SetY(a, NIDL_SCREEN_Y + 17);
        a->vy = 0;
    }
}

/* BeatenCheck(); 0x080A2224: both beaten, still (state 15); one beaten, the other down from the sky (state 8); TRUE: its state's
 * changed; */
static bool32 BeatenCheck(struct Fight *f, struct Actor *a) {
    if (f->gBeaten == 0) return FALSE;
    if (f->gBeaten == 2 && a->next != 16) {
        SetRestart(a, 15);
        return TRUE;
    }
    if (a->v20 != 0) return FALSE;
    SetRestart(a, 8);
    return TRUE;
}

// Below(); 0x080A2274: a Kirby high up on the screen (16-63 down it), chased (state 6);
static void Below(struct Fight *f, struct Actor *a, struct Kirby **target) {
    if (KirbyIn(f, a, 16, 0x40, FALSE, target)) SetRestart(a, 6);
}

// HoldTime(); 0x080A22B0: its wait on the ground, shorter under half its HP;
static u16 HoldTime(struct Fight *f, struct Actor *a) {
    return a->hp >= f->gHalf ? 120 : 60;
}

/* NextMove(); 0x080A22D4: a move by chance (sMoveWeights: by its HP over half or under; never the last one), only hops while the
 * other attacks from the sky (gSkyAttack); */
static void NextMove(struct Fight *f, struct Actor *a) {
    s32 k = a->hp >= f->gHalf ? 4 : 0, r, acc = 0, i = 3;

    r = Rand(f, 8 - sMoveWeights[k + a->v24]);
    if (a->v24 != 3 && (acc = sMoveWeights[k + 3]) >= r) {
        i = 3;
    } else {
        for (i = 2; i >= 0; i--) {
            if (a->v24 == i) continue;
            acc += sMoveWeights[i + k];
            if (acc >= r) break;
        }
    }
    if (f->gSkyAttack && !(i & 1)) i = Rand(f, 2) ? i + 1 : i - 1;
    i &= 3;
    a->v24 = i;
    SetState(a, sMoveStates[i]);
}

// HopWay(); 0x080A23C0: facing Kirby, its hop his way (v30: 1 right, -1 left), now and then the other;
static void HopWay(struct Fight *f, struct Actor *a) {
    FaceKirby(f, a);
    a->v30 = Rand(f, 4) ? a->facing : -a->facing;
}

// AttackOver(); 0x080A2400: its attack from the sky over: waiting there (state 7; beaten: state 3);
static void AttackOver(struct Fight *f, struct Actor *a) {
    f->gSkyAttack = 0;
    if (a->v20) {
        SetRestart(a, 3);
        return;
    }
    f->gWait = 0;
    a->v30 = 300;
    SetRestart(a, 7);
}

static void SwapDown(struct Fight *f, struct Actor *a);

// AskSwap(); 0x080A2448: done waiting, the other asked to come up (gSwap 1); else its attack (state 4);
static void AskSwap(struct Fight *f, struct Actor *a) {
    if (f->gWait == 0) {
        if (f->gSwap == 0) f->gSwap = 1;
        SwapDown(f, a);
    } else {
        SetRestart(a, 4);
    }
}

/* Chasing(); 0x080A24F8: high on the screen (16-63 down it), not at its side, a Kirby high up too (16-63) and across it (16-207);
 * else kept 17-64 down it; */
static bool32 Chasing(struct Fight *f, struct Actor *a, struct Kirby **target) {
    struct Kirby *k;
    s32 sy = ScreenY(a);

    if ((u32)(sy - 16) <= 0x2F) {
        if (AtSide(a) || !KirbyIn(f, a, 16, 0x40, FALSE, &k) || !KirbyIn(f, a, 16, 0xD0, TRUE, &k)) return FALSE;
        *target = k;
        return TRUE;
    }
    if (sy <= 15) a->iy = NIDL_SCREEN_Y + 17;
    if (a->iy - NIDL_SCREEN_Y > 63) a->iy = NIDL_SCREEN_Y + 64;
    a->y = a->iy << 16;
    return FALSE;
}

// Chase(); 0x080A2488: after a Kirby high up, slowly (a quarter of a pixel a frame); lost, waiting (state 7) or back (state 3);
static void Chase(struct Fight *f, struct Actor *a, struct Kirby **target) {
    if (Chasing(f, a, target)) {
        Aim(a, *target, 0x40, &a->vx, &a->vy);
        return;
    }
    SetRestart(a, f->gWait == 0 ? 7 : 3);
}

// SwapDown(); 0x080A2590: the one on the ground on its way up (gSwap 3): down from the sky (state 8);
static void SwapDown(struct Fight *f, struct Actor *a) {
    if (f->gSwap >> 1 & 1) {
        f->gSwap = 0;
        SetRestart(a, 8);
    }
}

// SkyWait(); 0x080A25C4 (state 3): waiting in the sky, then its attack (state 4); a Kirby high up chased;
static void SkyWait(struct Fight *f, struct Actor *a, struct Kirby **target) {
    if (BeatenCheck(f, a)) return;
    if (--a->v30 <= 0) SetRestart(a, 4);
    else Below(f, a, target);
}

// SkyChase(); 0x080A2608 (state 6): chasing, till its time's out (AskSwap());
static void SkyChase(struct Fight *f, struct Actor *a, struct Kirby **target) {
    if (BeatenCheck(f, a)) return;
    if (--a->v30 <= 0) AskSwap(f, a);
    else Chase(f, a, target);
}

// SkyIdle(); 0x080A263C (state 7): waiting in the sky till the swap (asking it once its time's out); a Kirby high up chased;
static void SkyIdle(struct Fight *f, struct Actor *a, struct Kirby **target) {
    if (BeatenCheck(f, a)) return;
    if (f->gSwap == 0) {
        if (--a->v30 > 0) {
            Below(f, a, target);
            return;
        }
        f->gSwap++;
    }
    SwapDown(f, a);
}

/* ToMiddle(); 0x080A268C: to the controller (both beaten), stopped across and down within `d` of it (v24: each stopped); both
 * stopped, one more there (gWait); */
static void ToMiddle(struct Fight *f, struct Actor *a, s32 d) {
    struct Actor *c = &f->a[A_CONTROLLER];

    if (a->vx != 0 && abs(c->ix - a->ix) <= d) {
        a->vx = 0;
        a->v24++;
    }
    if (a->vy != 0 && abs(c->iy - a->iy) <= d) {
        a->vy = 0;
        a->v24++;
    }
    if (a->v24 == 2) f->gWait++;
}

// ToMiddleStart(); 0x080A2754: on its way to the controller, a pixel a frame (a still way counting as reached);
static void ToMiddleStart(struct Fight *f, struct Actor *a) {
    Stop(a);
    a->v24 = 0;
    a->frozen = FALSE;
    AimAt(a, &f->a[A_CONTROLLER], 0x100);
    if (a->vx == 0) a->v24++;
    if (a->vy == 0) a->v24++;
}

// Pace(); 0x080A28D0 (Mr. Shine's steps, state 9): faster by its step's countdown (v30: sPaceSteps, sPaceSpeeds);
static void Pace(struct Actor *a) {
    s32 i;

    a->v30--;
    for (i = 0; i < 3 && sPaceSteps[i] > a->v30; i++) ;
    VelX(a, sPaceSpeeds[i]);
}

static void Crescent(struct Fight *f) {
    Shoot(f, A_SHINE, K_CRESCENT, 0, 0, 24, 0, TRUE);
    Sound(f, SFX_CRESCENT);
}

static void FallingStar(struct Fight *f, u8 left);

// Star(); 0x080A2954: a falling star (one fewer left), the next in 45 frames;
static void Star(struct Fight *f, struct Actor *a) {
    a->v34--;
    a->v30 = 45;
    FallingStar(f, a->v34);
}

// Moon(); 0x080A2994: the moon facing its way (its frames 4 and 8, its mirroring as it was); beaten, 19;
static void Moon(struct Actor *a) {
    a->frame = a->facing == -1 ? 4 : 8;
    if (a->v20) SetFrame(a, 19);
}

// ShineSky(); 0x080A29CC (state 3): waiting 300 frames there (not after a chase);
static void ShineSky(struct Fight *f, struct Actor *a) {
    Stop(a);
    Moon(a);
    if (a->prevState != 6) a->v30 = 300;
    f->gWait = 1;
}

// Bounce(); 0x080A2A94 (Mr. Bright on the ground): bobbing up and down, 48 frames round (sBounceSteps, sBounceSpeeds);
static void Bounce(struct Actor *a) {
    s32 i;

    if (--a->v28 < 0) a->v28 = 47;
    a->landed = FALSE;
    for (i = 0; i < 3 && sBounceSteps[i] > a->v28; i++) ;
    a->vy = sBounceSpeeds[i];
}

static void Shot(struct Fight *f) {
    Shoot(f, A_BRIGHT, K_SHOT, 1, 0, 24, 0, TRUE);
    Sound(f, SFX_SHOT);
}

// Beam(); 0x080A2CE8: the beam, 64 pixels under Mr. Bright, its sound looping while it shines; its three sparkles (0x080A2D38);
static void Beam(struct Fight *f) {
    f->kids[0] = Spawn(f, K_BEAM, A_BRIGHT, 3, 0, 0, 0x40, TRUE);
    Sound(f, SFX_BEAM);
    f->beamSound = TRUE;
}

static void Sparkles(struct Fight *f) {
    f->kids[3] = Spawn(f, K_PART, A_BRIGHT, 0, 0, 0, 0x60, FALSE);
    f->kids[1] = Spawn(f, K_PART, A_BRIGHT, 1, 0, 0, 0x10, FALSE);
    f->kids[2] = Spawn(f, K_PART, A_BRIGHT, 2, 0, 0, 8, FALSE);
}

// AtBeamSpot(); 0x080A2DE0: still; else stopped across at its spot (v18), down at the sky's height there (its pixel's);
static bool32 AtBeamSpot(struct Fight *f, struct Actor *a) {
    if (a->vx == 0 && a->vy == 0) return TRUE;
    if (a->facing == 1 ? a->ix - NIDL_SCREEN_X >= a->v18 : a->ix - NIDL_SCREEN_X <= a->v18) a->vx = 0;
    if (a->vy >= 0 ? a->iy >= f->skyHeight : a->iy <= f->skyHeight) a->vy = 0;
    return FALSE;
}

// Sun(); 0x080A2E88: the sun (its anim; fast: shining); beaten, 20;
static void Sun(struct Actor *a, bool32 fast) {
    Stop(a);
    if (fast) {
        a->v1C = PlayFrames(a, sBrightSunFast);
    } else if (a->v20) {
        a->anim = NULL;
        SetFrame(a, 20);
    } else {
        a->v1C = PlayFrames(a, sBrightSun);
    }
}

// BrightSky(); 0x080A2EDC (state 3): the sun waiting 300 frames there (not after a chase);
static void BrightSky(struct Fight *f, struct Actor *a) {
    Stop(a);
    if (a->v20) {
        a->anim = NULL;
        SetFrame(a, 20);
    } else {
        a->v1C = PlayFrames(a, sBrightSun);
    }
    if (a->prevState != 6) a->v30 = 300;
    f->gWait = 1;
}

/* ToBeam(); 0x080A2F38 (state 4): flashing, shining, over the closest Kirby (its spot kept 16-208 across the screen: v18), 48 down
 * the room, a pixel a frame each way; */
static void ToBeam(struct Fight *f, struct Actor *a) {
    struct Kirby *k;
    s32 kx;
    u8 side;

    Stop(a);
    f->gSkyAttack = 1;
    a->warn = TRUE;
    Sun(a, TRUE);
    k = Closest(f, a);
    kx = k ? KirbyX(k) - NIDL_SCREEN_X : 0;
    if (kx <= 15) kx = 16;
    if (kx > 0xCF) kx = 0xD0;
    a->v18 = kx;
    f->skyHeight = 48;
    side = k ? (KirbyX(k) - a->ix == 0 ? 0 : KirbyX(k) - a->ix > 0 ? 4 : 8) : 0;
    if (side == 4) {
        a->facing = 1;
        VelX(a, 0x10000);
    } else if (side == 8) {
        a->facing = -1;
        VelX(a, 0x10000);
    }
    a->vy = f->skyHeight < a->iy ? -0x10000 : 0x10000;
}

// Follow(); 0x080A301C (the beam's last 96 frames): after the closest Kirby, slowly;
static void Follow(struct Fight *f, struct Actor *a) {
    u8 side = KirbyRight(f, a);

    if (side == 4) {
        a->facing = 1;
        a->vx = 0x4C00;
    } else if (side == 8) {
        a->facing = -1;
        VelX(a, 0x4C00);
    }
}

// BeamOver(); 0x080A306C: the beam and its sparkles gone, its sound stopped;
static void BeamOver(struct Fight *f) {
    u8 i;

    for (i = 0; i < 4; i++) {
        if (f->kids[i] > 0) Destroy(f, f->kids[i]);
        f->kids[i] = NONE;
    }
    if (f->beamSound) {
        m4aSongNumStop(SFX_BEAM);
        f->beamSound = FALSE;
    }
}

// DestroyTrails(); 0x080A30D0: Mr. Bright's fire trails gone (0x02006040's +0, +4, +8);
static void DestroyTrails(struct Fight *f) {
    u8 i;

    for (i = 0; i < 3; i++) {
        if (f->kids[i] > 0) Destroy(f, f->kids[i]);
        f->kids[i] = NONE;
    }
}

static s8 DashTrail(struct Fight *f);
static s8 FireTrail(struct Fight *f);

/* Landing(); Waiting for it to land (+0x7A's bit 0); */
#define UNTIL_LANDED() while (!a->landed) WAIT(1)

/* Mr. Shine's states (0x08748644: their scripts; Actor.script the one running, its first line its state, +0x15); */

// Shine0(); 0x080A3348: falling to the ground (the fight's start);
static void Shine0(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 0;
    SetFrame(a, 10);
    a->landed = FALSE;
    VelY(a, 0, 0x1500, 0x30000);
    UNTIL_LANDED();
    Stop(a);
    SCRIPT_END;
}

// Shine1(); 0x080A33DC: on the ground (its box Kirby's), the swap or a move;
static void Shine1(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 1;
    Stop(a);
    Swap(f, a);
    a->v1C = 8;
    a->box = BOX_GROUND;
    SetFrame(a, 10);
    WAIT(HoldTime(f, a));
    NextMove(f, a);
    SCRIPT_END;
}

// Shine2(); 0x080A346C: up into the sky (Rise());
static void Shine2(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 2;
    SetFrame(a, 13);
    UpToSky(f, a);
    SCRIPT_END;
}

// Shine3(); 0x080A349C: the moon waiting in the sky (SkyWait());
static void Shine3(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 3;
    ShineSky(f, a);
    SCRIPT_END;
}

// Shine4(); 0x080A34C8: flashing 60 frames, then its stars (5);
static void Shine4(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 4;
    Stop(a);
    f->gSkyAttack = 1;
    a->warn = TRUE;
    Moon(a);
    WAIT(60);
    SetState(a, 5);
    SCRIPT_END;
}

// Shine5(); 0x080A352C: eight falling stars, 45 frames apart;
static void Shine5(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 5;
    Stop(a);
    a->v30 = 45;
    a->v34 = 8;
    Moon(a);
    SCRIPT_END;
}

// Shine6(), Shine7(); 0x080A3588, 0x080A35B8: chasing a Kirby high up; waiting for the swap;
static void Shine6(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 6;
    Stop(a);
    Moon(a);
    SCRIPT_END;
}

static void Shine7(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 7;
    Stop(a);
    Moon(a);
    SCRIPT_END;
}

// Shine8(); 0x080A35E8: down from the sky (to the ground: state 1, Landed());
static void Shine8(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 8;
    Stop(a);
    SetFrame(a, 12);
    WAIT(4);
    a->frame++;
    LeaveSky(f, a);
    SCRIPT_END;
}

// Shine9(); 0x080A3628: four steps at Kirby (Pace());
static void Shine9(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 9;
    FaceKirby(f, a);
    Stop(a);
    for (a->loop = 0; a->loop < 4; a->loop++) {
        a->v30 = 20;
        SetFrame(a, 10);
        WAIT(10);
        a->frame++;
        WAIT(10);
    }
    Stop(a);
    SetState(a, 1);
    SCRIPT_END;
}

// Shine10(); 0x080A36C8: a hop, a pixel a frame his way (or not), 6 up;
static void Shine10(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 10;
    Stop(a);
    HopWay(f, a);
    SetFrame(a, 10);
    WAIT(10);
    a->frame++;
    WAIT(5);
    a->landed = FALSE;
    SetFrame(a, 13);
    a->vx = a->v30 << 16;
    VelY(a, -0x60000, 0x4000, 0x60000);
    UNTIL_LANDED();
    Stop(a);
    SetFrame(a, 11);
    WAIT(5);
    SetState(a, 1);
    SCRIPT_END;
}

// Shine11(); 0x080A37AC: a jump back, flashing, then the dash (12);
static void Shine11(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 11;
    Stop(a);
    FaceKirby(f, a);
    a->warn = TRUE;
    a->landed = FALSE;
    SetFrame(a, 13);
    VelX(a, -0x10000);
    VelY(a, -0x20000, 0x1500, 0x30000);
    UNTIL_LANDED();
    Stop(a);
    SetFrame(a, 12);
    WAIT(10);
    SetState(a, 12);
    SCRIPT_END;
}

// Shine12(); 0x080A387C: the dash, rolling (his frames unmirrored: each way its own), his trail behind him, till the screen's side;
static void Shine12(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 12;
    Stop(a);
    Sound(f, SFX_DASH);
    VelX(a, 0x48000);
    a->child = DashTrail(f);
    a->flip = 1;
    a->v1C = PlayFrames(a, a->facing == 1 ? sShineDashRight : sShineDashLeft);
    SCRIPT_END;
}

// Shine13(); 0x080A3954: the wall hit, knocked back, a shake;
static void Shine13(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 13;
    Stop(a);
    Sound(f, SFX_WALL);
    NidlShake(4, &f->obj->base);
    a->landed = FALSE;
    SetFrame(a, 14);
    VelX(a, -0x8000);
    VelY(a, -0x20000, 0x1500, 0x30000);
    UNTIL_LANDED();
    Stop(a);
    SetFrame(a, 12);
    WAIT(5);
    SetFrame(a, 10);
    WAIT(5);
    SetFrame(a, 12);
    WAIT(5);
    SetState(a, 1);
    SCRIPT_END;
}

// Shine14(); 0x080A3A40: the crescent: a lunge, the throw, back;
static void Shine14(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 14;
    Stop(a);
    FaceKirby(f, a);
    SetFrame(a, 11);
    WAIT(15);
    SetFrame(a, 15);
    WAIT(20);
    a->frame++;
    VelX(a, 0x40000);
    WAIT(3);
    a->frame++;
    VelX(a, 0x20000);
    WAIT(3);
    VelX(a, 0x10000);
    WAIT(3);
    Crescent(f);
    a->frame++;
    VelX(a, -0x10000);
    WAIT(3);
    VelX(a, -0x20000);
    WAIT(3);
    VelX(a, -0x40000);
    WAIT(3);
    Stop(a);
    WAIT(20);
    SetState(a, 1);
    SCRIPT_END;
}

// Shine15(); 0x080A3B4C: beaten, the second: still, flashing;
static void Shine15(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 15;
    Stop(a);
    a->warn = TRUE;
    SetFrame(a, 19);
    SCRIPT_END;
}

// Shine16(), Shine17(); 0x080A3B7C, 0x080A3BC0: to the controller (both beaten: ToMiddle()), then shivering there;
static void Shine16(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 16;
    ToMiddleStart(f, a);
    SetFrame(a, 19);
    SCRIPT_END;
}

static void Shine17(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 17;
    ToMiddleStart(f, a);
    SetFrame(a, 19);
    for (;;) {
        WAIT(4);
        a->vx = -a->vx;
        a->vy = -a->vy;
        WAIT(2);
        a->vx = -a->vx;
        a->vy = -a->vy;
    }
    SCRIPT_END;
}

static void (*const sShineScripts[18])(struct Fight *, struct Actor *) = {
    Shine0, Shine1, Shine2, Shine3, Shine4, Shine5, Shine6, Shine7, Shine8,
    Shine9, Shine10, Shine11, Shine12, Shine13, Shine14, Shine15, Shine16, Shine17,
};

/* ShineFrame(); Mr. Shine's state's function each frame (0x0874868C), after his script and his move; */
static void ShineFrame(struct Fight *f, struct Actor *a) {
    switch (a->state) {
    case 0:
        if (a->next != 0) {
            a->v24 = 3;
            a->v20 = 0;
            a->child = NONE;
            Restart(a);
        }
        break;
    case 1:
        if (a->v1C <= 0) {
            FaceKirby(f, a);
            SetFrame(a, 10);
            a->v1C = 8;
        }
        a->v1C--;
        if (a->next != 1) Restart(a);
        break;
    case 2:
        Rise(f, a);
        break;
    case 3:
        SkyWait(f, a, &f->target);
        break;
    case 4:
        if (!BeatenCheck(f, a) && a->next != 4) {
            ClearWarn(a);
            Restart(a);
        }
        break;
    case 5:
        if (BeatenCheck(f, a)) break;
        if (--a->v30 > 0) break;
        if (a->v34 > 0) Star(f, a);
        else AttackOver(f, a);
        break;
    case 6:
        SkyChase(f, a, &f->target);
        break;
    case 7:
        SkyIdle(f, a, &f->target);
        break;
    case 8:
        KeepBelowTop(a);
        break;
    case 9:
        if (a->next != 9) {
            Restart(a);
            break;
        }
        Pace(a);
        if (AtSide(a)) TurnAround(a);
        break;
    case 10:
        if (a->next != 10) {
            Restart(a);
        } else if (AtSide(a)) {
            a->vx = -a->vx;
            a->v30 = -a->v30;
        }
        break;
    case 11: case 14:
        if (a->next != a->state) Restart(a);
        else if (AtSide(a)) a->vx = 0;
        break;
    case 12:
        if (AtSide(a)) {
            a->vx = 0;
            ClearWarn(a);
            Destroy(f, a->child);
            a->child = NONE;
            SetRestart(a, 13);
        } else {
            a->v1C = AnimTick(a, a->v1C);
        }
        break;
    case 13:
        if (a->next != 13) Restart(a);
        break;
    case 16: case 17:
        if (a->v24 != 2) ToMiddle(f, a, a->state == 16 ? 8 : 3);
        break;
    }
}

/* Mr. Bright's states (0x087486D4); */

// Bright0(); 0x080A3CD8: falling to the ground (the fight's start);
static void Bright0(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 0;
    a->v1C = Play(a, sBrightIdle);
    a->landed = FALSE;
    VelY(a, 0, 0x1500, 0x30000);
    UNTIL_LANDED();
    Stop(a);
    SCRIPT_END;
}

// Bright1(); 0x080A3D84: on the ground, bobbing (Bounce()), the swap or a move (as the bob comes round);
static void Bright1(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 1;
    Stop(a);
    Swap(f, a);
    a->box = BOX_GROUND;
    a->v1C = Play(a, sBrightIdle);
    a->v28 = 48;
    WAIT(HoldTime(f, a));
    NextMove(f, a);
    SCRIPT_END;
}

// Bright2(); 0x080A3E10: up into the sky (Rise());
static void Bright2(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 2;
    UpToSky(f, a);
    a->v1C = Play(a, sBrightUp);
    SCRIPT_END;
}

// Bright3(); 0x080A3E60: the sun waiting in the sky (SkyWait());
static void Bright3(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 3;
    BrightSky(f, a);
    SCRIPT_END;
}

// Bright4(); 0x080A3EA0: over Kirby, shining (ToBeam()), 60 frames, then the beam (5);
static void Bright4(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 4;
    ToBeam(f, a);
    while (!AtBeamSpot(f, a)) WAIT(1);
    Stop(a);
    WAIT(60);
    SetState(a, 5);
    SCRIPT_END;
}

// Bright5(); 0x080A3F24: the beam, 120 frames (its last 96 after Kirby), the screen darkening;
static void Bright5(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 5;
    Stop(a);
    a->v34 = 120;
    Sun(a, TRUE);
    WAIT(6);
    Beam(f);
    Sparkles(f);
    Darken(f, 10, 0x50);
    SCRIPT_END;
}

// Bright6(), Bright7(); 0x080A3FB8, 0x080A3FF8: chasing a Kirby high up; waiting for the swap;
static void Bright6(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 6;
    Sun(a, FALSE);
    SCRIPT_END;
}

static void Bright7(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 7;
    Stop(a);
    Sun(a, FALSE);
    SCRIPT_END;
}

// Bright8(); 0x080A403C: down from the sky;
static void Bright8(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 8;
    Stop(a);
    a->v1C = PlayFrames(a, sBrightLeave);
    LeaveSky(f, a);
    SCRIPT_END;
}

/* HopStart(), HopEnd(); 0x080A2AF4, 0x080A2B14: crouched 12 frames facing Kirby (his hop's way: HopWay()); landed, 12; */
#define HOP_START() do { Stop(a); HopWay(f, a); a->anim = NULL; SetFrame(a, 4); WAIT(12); } while (0)
#define HOP_END() do { Stop(a); SetFrame(a, 10); WAIT(12); } while (0)

// Bright9(); 0x080A408C: two small hops at Kirby;
static void Bright9(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 9;
    HOP_START();
    for (a->loop = 0; a->loop <= 1; a->loop++) {
        a->landed = FALSE;
        VelX(a, 0x8000);
        VelY(a, -0x20000, 0x4000, 0x20000);
        a->v1C = Play(a, sBrightHop);
        UNTIL_LANDED();
        HOP_END();
    }
    Stop(a);
    SetState(a, 1);
    SCRIPT_END;
}

// Bright10(); 0x080A4180: a big hop, 1.125 pixels a frame his way (or not), 6 up;
static void Bright10(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 10;
    HOP_START();
    a->landed = FALSE;
    a->vx = a->v30 * 0x12000;
    VelY(a, -0x60000, 0x4000, 0x60000);
    a->v1C = Play(a, sBrightBigHop);
    UNTIL_LANDED();
    HOP_END();
    SetState(a, 1);
    SCRIPT_END;
}

// Bright11(); 0x080A4260: a jump back, flashing, then the fire dash (12);
static void Bright11(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 11;
    Stop(a);
    FaceKirby(f, a);
    a->warn = TRUE;
    a->anim = NULL;
    HOP_END();
    a->landed = FALSE;
    a->v1C = Play(a, sBrightBack);
    VelX(a, -0x10000);
    VelY(a, -0x20000, 0x1500, 0x30000);
    UNTIL_LANDED();
    Stop(a);
    SetState(a, 12);
    SCRIPT_END;
}

// Bright12(); 0x080A4350: the fire dash, four fire trails 3 frames apart, till the screen's side;
static void Bright12(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 12;
    Stop(a);
    a->child = FireTrail(f);
    f->kids[0] = f->kids[1] = f->kids[2] = NONE;
    Sound(f, SFX_DASH);
    VelX(a, 0x48000);
    SetFrame(a, 12);
    WAIT(3);
    f->kids[0] = FireTrail(f);
    WAIT(3);
    f->kids[1] = FireTrail(f);
    WAIT(3);
    f->kids[2] = FireTrail(f);
    SCRIPT_END;
}

// Bright13(); 0x080A4430: the wall hit, dizzy, knocked back, a shake;
static void Bright13(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 13;
    Stop(a);
    Sound(f, SFX_WALL);
    NidlShake(4, &f->obj->base);
    a->landed = FALSE;
    a->v1C = Play(a, sBrightDizzy);
    VelX(a, -0x8000);
    VelY(a, -0x20000, 0x1500, 0x30000);
    UNTIL_LANDED();
    Stop(a);
    SetState(a, 1);
    SCRIPT_END;
}

// Bright14(); 0x080A4508: the fireball: 64 frames, the punch, a lunge back and forth;
static void Bright14(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 14;
    Stop(a);
    FaceKirby(f, a);
    a->v1C = Play(a, sBrightShot);
    WAIT(64);
    Shot(f);
    VelX(a, -0x40000);
    WAIT(3);
    VelX(a, -0x20000);
    WAIT(3);
    VelX(a, -0x10000);
    WAIT(3);
    VelX(a, 0x10000);
    WAIT(3);
    VelX(a, 0x20000);
    WAIT(3);
    VelX(a, 0x40000);
    WAIT(3);
    Stop(a);
    WAIT(20);
    SetState(a, 1);
    SCRIPT_END;
}

// Bright15(); 0x080A4604: beaten, the second: still, flashing;
static void Bright15(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 15;
    Stop(a);
    a->warn = TRUE;
    SetFrame(a, 20);
    SCRIPT_END;
}

// Bright16(), Bright17(); 0x080A4634, 0x080A4678: as Mr. Shine's;
static void Bright16(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 16;
    ToMiddleStart(f, a);
    SetFrame(a, 20);
    SCRIPT_END;
}

static void Bright17(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 17;
    ToMiddleStart(f, a);
    SetFrame(a, 20);
    for (;;) {
        WAIT(4);
        a->vx = -a->vx;
        a->vy = -a->vy;
        WAIT(2);
        a->vx = -a->vx;
        a->vy = -a->vy;
    }
    SCRIPT_END;
}

static void (*const sBrightScripts[18])(struct Fight *, struct Actor *) = {
    Bright0, Bright1, Bright2, Bright3, Bright4, Bright5, Bright6, Bright7, Bright8,
    Bright9, Bright10, Bright11, Bright12, Bright13, Bright14, Bright15, Bright16, Bright17,
};

/* BrightFrame(); Mr. Bright's state's function each frame (0x0874871C); */
static void BrightFrame(struct Fight *f, struct Actor *a) {
    switch (a->state) {
    case 0:
        if (a->next != 0) {
            a->v24 = 3;
            a->v20 = 0;
            a->child = NONE;
            Restart(a);
        }
        a->v1C = AnimTickFacing(f, a, a->v1C);
        break;
    case 1:
        a->v1C = AnimTickFacing(f, a, a->v1C);
        Bounce(a);
        if (a->next != 1 && a->v28 == 47) Restart(a);
        break;
    case 2:
        a->v1C = AnimTick(a, a->v1C);
        Rise(f, a);
        break;
    case 3:
        a->v1C = AnimTick(a, a->v1C);
        SkyWait(f, a, &f->target);
        break;
    case 4:
        a->v1C = AnimTick(a, a->v1C);
        if (!BeatenCheck(f, a) && a->next != 4) {
            ClearWarn(a);
            Restart(a);
        }
        break;
    case 5:
        a->v1C = AnimTick(a, a->v1C);
        if (BeatenCheck(f, a)) {
            Undarken(f);
            BeamOver(f);
            break;
        }
        if (a->v34 <= 95) Follow(f, a);
        if (a->v34 <= 0) {
            AttackOver(f, a);
            Lighten(f, 16);
            BeamOver(f);
        }
        a->v34--;
        break;
    case 6:
        a->v1C = AnimTick(a, a->v1C);
        SkyChase(f, a, &f->target);
        break;
    case 7:
        a->v1C = AnimTick(a, a->v1C);
        SkyIdle(f, a, &f->target);
        break;
    case 8:
        a->v1C = AnimTick(a, a->v1C);
        KeepBelowTop(a);
        break;
    case 9:
        if (!a->landed) a->v1C = AnimTick(a, a->v1C);
        if (a->next != 9) Restart(a);
        else if (AtSide(a)) TurnAround(a);
        break;
    case 10:
        if (!a->landed) a->v1C = AnimTick(a, a->v1C);
        if (a->next != 10) {
            Restart(a);
        } else if (AtSide(a)) {
            a->vx = -a->vx;
            a->v30 = -a->v30;
        }
        break;
    case 11:
        if (!a->landed) a->v1C = AnimTick(a, a->v1C);
        if (a->next != 11) Restart(a);
        else if (AtSide(a)) a->vx = 0;
        break;
    case 12:
        if (AtSide(a)) {
            a->vx = 0;
            ClearWarn(a);
            Destroy(f, a->child);
            a->child = NONE;
            DestroyTrails(f);
            SetRestart(a, 13);
        }
        break;
    case 13:
        if (!a->landed) a->v1C = AnimTick(a, a->v1C);
        if (a->next != 13) Restart(a);
        break;
    case 14:
        a->v1C = AnimTick(a, a->v1C);
        if (a->next != 14) Restart(a);
        else if (AtSide(a)) a->vx = 0;
        break;
    case 16: case 17:
        if (a->v24 != 2) ToMiddle(f, a, a->state == 16 ? 8 : 3);
        break;
    }
}

/* The controller (type 0x3F's first object, 0x080A3114: on NiDL's spot, (208, 104) in their room; its HP the HP bar's); */
static void Defeat(struct Fight *f);

// Middle(); 0x080A1C1C: the controller between the two;
static void Middle(struct Fight *f) {
    struct Actor *c = &f->a[A_CONTROLLER];

    c->ix = (f->a[A_SHINE].ix + f->a[A_BRIGHT].ix) >> 1;
    c->iy = (f->a[A_SHINE].iy + f->a[A_BRIGHT].iy) >> 1;
    c->x = c->ix << 16;
    c->y = c->iy << 16;
}

// Both(); 0x080A1BD8: both their next states, at once (the frame they're run);
static void Both(struct Fight *f, u8 shine, u8 bright) {
    SetRestart(&f->a[A_SHINE], shine);
    SetRestart(&f->a[A_BRIGHT], bright);
}

// Controller0(); 0x080A3184: the intro (0x080A19EC: the HP bar filling, KatAM's; then their boxes on);
static void Controller0(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 0;
    WAIT(INTRO_TIME);
    f->a[A_SHINE].box = BOX_GROUND;
    f->a[A_BRIGHT].box = BOX_GROUND;
    SetState(a, 1);
    SCRIPT_END;
}

static void Controller1(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 1;
    SCRIPT_END;
}

static void Controller2(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 2;
    SCRIPT_END;
}

static void Controller3(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 3;
    SCRIPT_END;
}

static void (*const sControllerScripts[4])(struct Fight *, struct Actor *) = { Controller0, Controller1, Controller2, Controller3 };

/* ControllerLate(); Its state 1's own (its +8, 0x080A31F4: run after every object's frame, not in its slot's turn): both beaten
 * (0x080A1C90), both to the middle (their next frame), the sunset again; else its HP the one's on the ground; */
static void ControllerLate(struct Fight *f) {
    struct Actor *a = &f->a[A_CONTROLLER];

    if (a->kind != K_CONTROLLER || a->state != 1) return;
    if (f->gBeaten == 2) {
        f->obj->unk80 = 0;
        f->gWait = f->gOn = 0;
        Middle(f);
        SetRestart(a, 2);
        BeamOver(f);
        Both(f, 16, 16);
        f->a[A_SHINE].frozen = f->a[A_BRIGHT].frozen = TRUE;
        SkyTo(f, SKY_SUNSET);
        a->state = 0xFF;
        return;
    }
    f->obj->unk80 = f->a[f->gShineDown ? A_SHINE : A_BRIGHT].hp;
}

/* ControllerFrame(); 0x08748634: the intro over, one up into the sky (by chance: Mr. Shine, the night; or Mr. Bright, the day:
 * 0x080A1B94); both at the middle, shivering there (0x080A1D2C), then the defeat (0x080A1D84); state 1's, ControllerLate(); */
static void ControllerFrame(struct Fight *f, struct Actor *a) {
    switch (a->state) {
    case 0:
        if (a->next == 0) break;
        Restart(a);
        if (Rand(f, 2)) {
            f->gShineDown = FALSE;
            SkyTo(f, SKY_NIGHT);
            Both(f, 2, 1);
        } else {
            f->gShineDown = TRUE;
            SkyTo(f, SKY_DAY);
            Both(f, 1, 2);
        }
        break;
    case 2:
        if (f->gWait != 2) break;
        f->gWait = 0;
        SetRestart(a, 3);
        Both(f, 17, 17);
        f->a[A_SHINE].frozen = f->a[A_BRIGHT].frozen = TRUE;
        break;
    case 3:
        if (f->gWait == 2) Defeat(f);
        break;
    }
}

/* The flames (type 0x3F's fourth object, 0x080A472C: on Mr. Bright, his body; 12 frames 16 frames round, the table by his frame:
 * crouched (12) or up (13) its own; hidden as the sun, beaten; his facing as each round starts; his palette, unless he holds it); */
static void FlamesScript(struct Fight *f, struct Actor *a) {
    static const u8 waits[12] = { 1, 2, 1, 1, 1, 2, 1, 1, 2, 1, 2, 1 };

    SCRIPT;
    for (;;) {
        SetFrame(a, 0);
        for (a->loop = 0; a->loop < 12; a->loop++) {
            if (a->loop) a->frame++;
            WAIT(waits[a->loop]);
        }
    }
    SCRIPT_END;
}

static void FlamesFrame(struct Fight *f, struct Actor *a) {
    struct Actor *b = &f->a[A_BRIGHT];

    a->v18 = b->frame == 12 ? 1 : b->frame == 13 ? 2 : 0;
    if (b->frame > 15) a->frame = FRAME_HIDDEN;
    a->facing = b->facing;
    a->held = b->held;
}

/* Their shots (type 0x7F: the crescent, the fireball, the falling star, the beam; 0x080A4C84), the beam's sparkles (0xC1) and sparks
 * (0x8B), the trails (0xA7, 0xA8); their scripts (the crescent's and the fireball's state 1: hitting a Kirby, a puff: 0x0806A344); */

// CrescentScript(); 0x080A4D88: 4 pixels a frame its way, spinning (its frames 4-7 by its way);
static void CrescentScript(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 0;
    a->landed = FALSE;
    VelX(a, 0x40000);
    a->v28 = PlayFrames(a, a->facing == 1 ? sCrescentRight : sCrescentLeft);
    SCRIPT_END;
}

// ShotScript(); 0x080A510C: 4 pixels a frame its way, its frames 1, 0, 2, 3;
static void ShotScript(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 0;
    a->landed = FALSE;
    VelX(a, 0x40000);
    for (;;) {
        SetFrame(a, 1);
        WAIT(1);
        SetFrame(a, 0);
        WAIT(1);
        SetFrame(a, 2);
        WAIT(1);
        a->frame++;
        WAIT(1);
    }
    SCRIPT_END;
}

/* BeamSparks(); 0x080A523C: two sparks from the beam's foot, 16 either side of it, each off its own way (0x08067120: its way set
 * before its script's first frame, run as it's made); */
static void BeamSparks(struct Fight *f, struct Actor *a) {
    s8 i, d;

    for (d = 1; d >= -1; d -= 2) {
        i = Spawn(f, K_SPARK, a - f->a, 1, 0, a->ix + 16 * d, a->iy + 32, FALSE);
        if (i == NONE) continue;
        f->a[i].facing = d;
        ChildScript(f, &f->a[i]);
        f->a[i].fresh = TRUE;
    }
}

// BeamScript(); 0x080A528C: on Mr. Bright, 64 down; its sparks 2 frames on, and 84 after;
static void BeamScript(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->relative = TRUE;
    a->state = 0;
    Stop(a);
    a->landed = FALSE;
    a->v18 = a->v1C = 0;
    WAIT(2);
    BeamSparks(f, a);
    WAIT(84);
    BeamSparks(f, a);
    SCRIPT_END;
}

/* PartScript(); 0x080A5524, 0x080A556C, 0x080A55AC: the beam's sparkles (on Mr. Bright): its foot's (96 down) blinking (1, out 2,
 * 0, out 3); one falling 8 pixels a frame from 16 down, back up every 10 frames, across by turns (sSparkleSpotsA); one 16 a frame
 * from 8 down, every 5 (sSparkleSpotsB); */
static void PartScript(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->relative = TRUE;
    Stop(a);
    a->v2C = a->y;
    if (a->variant == 0) {
        for (;;) {
            a->frame = 1;
            WAIT(1);
            a->frame = FRAME_HIDDEN;
            WAIT(2);
            a->frame = 0;
            WAIT(1);
            a->frame = FRAME_HIDDEN;
            WAIT(3);
        }
    }
    a->frame = a->variant == 1 ? 4 : 5;
    a->vy = a->variant == 1 ? 0x80000 : 0x100000;
    for (;;) {
        for (a->loop = 0; a->loop < (a->variant == 1 ? 5 : 7); a->loop++) {
            a->y = a->v2C;
            a->x = a->variant == 1 ? sSparkleSpotsA[a->loop] : sSparkleSpotsB[a->loop];
            WAIT(a->variant == 1 ? 10 : 5);
        }
    }
    SCRIPT_END;
}

/* SparkScript(); 0x08067258: a spark off the beam's foot: 13, 12, 11 rising 1.5 a frame, 2 across; then slower (0.5 each way) and
 * still, 8 4 10 5 9 6 10 7 four times; then harmless, 11 12 13, gone; */
static void SparkScript(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 0;
    a->landed = FALSE;
    a->v24 = 2;
    VelX(a, 0x20000);
    a->vy = -0x18000;
    a->frame = 13;
    WAIT(2);
    a->frame--;
    WAIT(2);
    a->frame--;
    WAIT(2);
    VelX(a, 0x8000);
    a->vy = -0x8000;
    for (a->loop = 0; a->loop <= 3; a->loop++) {
        a->frame = 8;
        WAIT(1);
        a->frame = 4;
        WAIT(3);
        a->frame = 10;
        WAIT(1);
        a->frame = 5;
        WAIT(3);
        a->frame = 9;
        WAIT(1);
        a->frame = 6;
        WAIT(3);
        Stop(a);
        a->frame = 10;
        WAIT(1);
        a->frame = 7;
        WAIT(3);
    }
    a->v20 = 1;
    a->frame = 11;
    WAIT(2);
    a->frame++;
    WAIT(2);
    a->frame++;
    WAIT(1);
    Destroy(f, a - f->a);
    return;
    SCRIPT_END;
}

/* TrailScript(); 0x0806E73C: Mr. Shine's dash trail: 16 behind him, drifting back 3 a frame, its frames 0, 2, 4 (2, 3, 2 frames),
 * round; */
static void TrailScript(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->relative = TRUE;
    a->prio = 10;
    a->facing = f->a[a->parent].facing;
    for (;;) {
        a->x = (a->facing == 1 ? a->v24 : -a->v24) << 16;
        a->y = a->v20 << 16;
        VelX(a, -0x30000);
        SetFrame(a, 0);
        WAIT(2);
        SetFrame(a, 2);
        WAIT(3);
        SetFrame(a, 4);
        WAIT(2);
    }
    SCRIPT_END;
}

/* FireScript(); 0x0806E84C: a fire trail of Mr. Bright's dash: 24 behind him, +-8 each way by chance, drifting back 3 a frame, its
 * frames 0-18 (every other), 14 frames round; */
static void FireScript(struct Fight *f, struct Actor *a) {
    static const u8 waits[10] = { 2, 2, 1, 1, 1, 1, 1, 2, 2, 1 };
    s32 r;

    SCRIPT;
    a->relative = TRUE;
    a->prio = 10;
    a->facing = -f->a[a->parent].facing;
    for (;;) {
        r = (s16)(-8 + (Rand(f, 0x100) * 16 >> 8));
        a->x = (a->facing == 1 ? r : -r) + a->facing * a->v24;
        a->x <<= 16;
        r = (s16)(-8 + (Rand(f, 0x100) * 16 >> 8));
        a->y = (r + a->v20) << 16;
        VelX(a, 0x30000);
        SetFrame(a, 0);
        for (a->loop = 0; a->loop < 10; a->loop++) {
            if (a->loop) a->frame += 2;
            WAIT(waits[a->loop]);
        }
    }
    SCRIPT_END;
}

/* StarScript(); 0x080A4F40: a falling star (KatAM's enemy, enemies/star.c: Kirby can inhale it): high on the screen (16-47 down it),
 * across by how many are left (sStarSpots), blinking 16 frames (Mr. Shine's colours), then falling 3 or 4 a frame, across by chance
 * (sStarSpeeds; more his way when he's to its right), spinning; */
static void StarScript(struct Fight *f, struct Actor *a) {
    s32 r;

    SCRIPT;
    a->state = 0;
    Stop(a);
    a->landed = FALSE;
    a->v18 = 0;
    SetY(a, NIDL_SCREEN_Y + (s16)(16 + Rand(f, 0x20)));
    r = Rand(f, 3);
    a->v28 = (3 + (a->extra & 1)) << 16;
    // (Kirby's side of it as it was made, at Mr. Shine: NiDL's pixel spot, +0x48, isn't moved with it till it moves);
    if (KirbyRight(f, a) == 4) r += 2;
    a->v2C = sStarSpeeds[r];
    a->ix = NIDL_SCREEN_X + sStarSpots[a->extra & 3];
    a->x = a->ix << 16;
    for (a->loop = 0; a->loop <= 1; a->loop++) {
        a->frame = 4;
        WAIT(4);
        a->frame++;
        WAIT(4);
    }
    a->v34 = 1;
    a->vx = a->v2C;
    a->vy = a->v28;
    Sound(f, SFX_STAR);
    Rand(f, 3);
    a->frame = 4;
    a->v18 = 1;
    a->v24 = 2;
    SCRIPT_END;
}

// PuffScript(); State 1 (0x080A4DF4, 0x080A5168: 0x0806A344): its puff where it is, still, then gone (PuffStep());
static void PuffScript(struct Fight *f, struct Actor *a) {
    // (the falling star's: KatAM's enemy gone, the fight's sprite its puff);
    if (a->enemy && !(a->enemy->base.flags & 0x1000) && !(a->enemy->base.unkC & 0x400)) a->enemy->base.flags |= 0x1000;
    a->enemy = NULL;
    a->kind = K_PUFF;
    a->age = 0;
    a->frame = 0;
    a->pc = PC_END;
    Stop(a);
}

static void ChildScript(struct Fight *f, struct Actor *a) {
    if (a->script == 1 && (a->kind == K_CRESCENT || a->kind == K_SHOT || a->kind == K_STAR)) {
        PuffScript(f, a);
        return;
    }
    switch (a->kind) {
    case K_CRESCENT: CrescentScript(f, a); break;
    case K_SHOT: ShotScript(f, a); break;
    case K_BEAM: BeamScript(f, a); break;
    case K_PART: PartScript(f, a); break;
    case K_SPARK: SparkScript(f, a); break;
    case K_TRAIL: TrailScript(f, a); break;
    case K_FIRE: FireScript(f, a); break;
    case K_STAR: StarScript(f, a); break;
    case K_FLAMES: FlamesScript(f, a); break;
    }
}

/* Their objects' frames: Destroy() (NiDL's 0x08063F24: gone, its sprite and its box with it), the move (0x080656B4: its speed plus
 * its change, capped across either way and falling; or on its parent, 0x080059FC), the bosses' floor (their terrain, 0x080651B4); */
static void Release(struct EffectObject **e) {
    if (*e == NULL) return;
    (*e)->flags |= 0x1000;
    *e = NULL;
}

static void Destroy(struct Fight *f, s8 i) {
    struct Actor *a;

    if (i < A_CHILDREN || i >= ACTORS) return;
    a = &f->a[i];
    if (a->kind == K_NONE) return;
    Release(&a->sprite);
    if (a->attack) a->attack->flags |= 0x1000;
    a->attack = NULL;
    if (a->enemy && !(a->enemy->base.flags & 0x1000) && !(a->enemy->base.unkC & 0x400)) a->enemy->base.flags |= 0x1000;
    a->enemy = NULL;
    a->kind = K_NONE;
}

static void Motion(struct Fight *f, struct Actor *a) {
    struct Actor *p;
    s32 feet;

    if (a->kind == K_CONTROLLER) return;
    a->vx += a->ax;
    a->vy += a->ay;
    if (a->maxX != NO_MAX) {
        if (a->vx > 0 && a->vx > a->maxX) a->vx = a->maxX;
        else if (a->vx <= 0 && a->vx < -a->maxX) a->vx = -a->maxX;
    }
    if (a->maxY != NO_MAX && a->vy > 0 && a->vy > a->maxY) a->vy = a->maxY;
    a->x += a->vx;
    a->y += a->vy;
    if (a->relative) {
        p = &f->a[a->parent];
        a->ix = (a->x + p->x) >> 16;
        a->iy = (a->y + p->y) >> 16;
        return;
    }
    a->ix = a->x >> 16;
    a->iy = a->y >> 16;
    if (a->kind != K_SHINE && a->kind != K_BRIGHT) return;
    // (landing once its feet are in the floor, falling: in the middle of the pixel over it; Landed());
    feet = a->kind == K_SHINE ? SHINE_FEET : BRIGHT_FEET;
    if (a->vy >= 0 && a->iy + feet >= FLOOR_Y) {
        a->iy = FLOOR_Y - feet - 1;
        a->y = (a->iy << 16) | 0x8000;
        a->vy = 0;
        a->landed = TRUE;
        Landed(f, a);
    }
}

/* HitsKirby(); The crescent's, the fireball's or the falling star's box on a Kirby (NiDL's 0x08068E04: its box listed, Kirby's own
 * code finds it, the shot's puff next, state 1; even as he's knocked back): NiDL's Kirby's box, where NiDL has him (KirbyX(), KirbyY()):
 * 7 either side of him, his spot down to his feet (fitted to the recordings: every puff and near miss by a standing or knocked back
 * Kirby; KatAM's own box is smaller and lower); */
#define KIRBY_BOX_W 7
#define KIRBY_BOX_B 10

static bool32 HitsKirby(struct Fight *f, struct Actor *a, s32 w, s32 h) {
    u8 i;

    for (i = 0; i < gNumKirbys; i++) {
        struct Kirby *k = &gKirbys[i];
        s32 kx, ky;

        if (!InRoom(f, k) || (k->base.flags & 0x200)) continue;
        kx = KirbyX(k);
        ky = KirbyY(k);
        if (a->ix - w <= kx + KIRBY_BOX_W && a->ix + w >= kx - KIRBY_BOX_W && a->iy - h <= ky + KIRBY_BOX_B && a->iy + h >= ky)
            return TRUE;
    }
    return FALSE;
}

// Spin(); 0x08064BCC: the falling star's spin: its frame on by its facing every 3 frames (4-7 round);
static void Spin(struct Actor *a) {
    if (a->v24 > 0) {
        a->v24--;
        return;
    }
    if (a->facing == 1) {
        if (++a->frame > 7) a->frame = 4;
    } else if (--a->frame <= 3) {
        a->frame = 7;
    }
    a->v24 = 2;
}

/* PuffStep(); A puff (0x0806A344: the shared effect's 0x0874C9D8, 0-16): its first frame twice, out four after its eleventh, gone after
 * its seventeenth (as Nightmare's); */
static void PuffStep(struct Fight *f, struct Actor *a) {
    s16 k = a->age++;

    if (k >= 22) {
        Destroy(f, a - f->a);
        return;
    }
    a->frame = k < 2 ? 0 : k < 12 ? k - 1 : k < 16 ? FRAME_HIDDEN : k - 5;
}

/* OffScreen(); A shot gone off the screen (OFF_SCREEN past it); */
static bool32 OffScreen(struct Actor *a) {
    return a->ix < NIDL_SCREEN_X - OFF_SCREEN || a->ix >= NIDL_SCREEN_X + DISPLAY_WIDTH + OFF_SCREEN
        || a->iy < NIDL_SCREEN_Y - OFF_SCREEN || a->iy >= NIDL_SCREEN_Y + DISPLAY_HEIGHT + OFF_SCREEN;
}

static void ShinePalette(struct Fight *f, struct Actor *a);
static void BrightPalette(struct Fight *f, struct Actor *a);

/* Frame(); An Actor's function each frame (NiDL's +4), after its script and its move; */
static void Frame(struct Fight *f, struct Actor *a) {
    s8 i = a - f->a;

    switch (a->kind) {
    case K_CONTROLLER:
        ControllerFrame(f, a);
        return;
    case K_SHINE: case K_BRIGHT:
        // (not frozen, not hit this frame: 0x080692FC; their ends' (16, 17) theirs; their palettes);
        if (a->state >= 16 || (!a->frozen && !(f->hit && f->ground == i))) {
            if (a->kind == K_SHINE) ShineFrame(f, a);
            else BrightFrame(f, a);
        }
        if (a->kind == K_SHINE) ShinePalette(f, a);
        else BrightPalette(f, a);
        return;
    case K_FLAMES:
        FlamesFrame(f, a);
        if (!a->held) f->brightColors = gNidlShineBright.colors[C_BRIGHT];
        return;
    case K_CRESCENT:
        if (HitsKirby(f, a, 8, 6)) SetRestart(a, 1);
        else a->v28 = AnimTick(a, a->v28);
        break;
    case K_SHOT:
        if (HitsKirby(f, a, 10, 10)) SetRestart(a, 1);
        break;
    case K_BEAM:
        // (0x080A52C8: its frames 2 and 3, 2 frames each);
        if (a->v18 <= 0) {
            a->frame = a->v1C ? 3 : 2;
            a->v18 = 2;
            a->v1C ^= 1;
        }
        a->v18--;
        return;
    case K_TRAIL: case K_FIRE:
        // (0x0806E7C0: gone with the one dashing);
        if (f->a[a->parent].kind != K_SHINE && f->a[a->parent].kind != K_BRIGHT) Destroy(f, i);
        return;
    case K_STAR:
        if (HitsKirby(f, a, 5, 5)) SetRestart(a, 1);
        else if (a->v18) Spin(a);
        break;
    case K_PUFF:
        PuffStep(f, a);
        break;
    default:
        return;
    }
    if (a->kind != K_NONE && OffScreen(a)) Destroy(f, i);
}

static void RunScript(struct Fight *f, struct Actor *a) {
    switch (a->kind) {
    case K_CONTROLLER: sControllerScripts[a->script & 3](f, a); break;
    case K_SHINE: if (a->script < 18) sShineScripts[a->script](f, a); break;
    case K_BRIGHT: if (a->script < 18) sBrightScripts[a->script](f, a); break;
    default: ChildScript(f, a); break;
    }
}

static void HurtStep(struct Fight *f, struct Actor *a);

/* Process(); An Actor's frame (in NiDL's order, its slot's): hurt, its hurt's step only; else its script (its next state's, restarted;
 * not the frame it's made: it ran then), its move, its frame's function; */
static void Process(struct Fight *f, s8 i) {
    struct Actor *a = &f->a[i];

    if (a->kind == K_NONE) return;
    if (a->hurt) {
        HurtStep(f, a);
        return;
    }
    if (a->fresh) {
        a->fresh = FALSE;
    } else {
        if (a->restart) {
            a->restart = FALSE;
            a->script = a->next;
            a->pc = 0;
            a->wait = 0;
        }
        if (a->pc != PC_END && (!a->wait || !--a->wait)) RunScript(f, a);
        if (a->kind == K_NONE) return;
    }
    Motion(f, a);
    Frame(f, a);
}

/* Their palettes (KatAM's slots for NiDL's): Mr. Shine's glow going round its four every 12 frames (0x080A2814), flashing with his
 * hurt colours warning (2 frames in 4) or beaten (4 in 8); Mr. Bright's his flames' (their streamed frame's, 0x080A4814), his
 * flashing his hurt's (0x080A2BC4); their hurt's flash (2 frames in 4, by its countdown: 0x0806619C); */
static void ShinePalette(struct Fight *f, struct Actor *a) {
    if (a->warn || a->v20) {
        f->shineColors = (f->frameCount & (a->v20 ? 4 : 2)) ? gNidlShineBright.colors[C_GLOW] : gNidlShineBright.colors[C_SHINE_HURT];
        return;
    }
    if (a->v2C <= 0) {
        f->shineColors = gNidlShineBright.colors[C_GLOW + a->v28];
        a->v2C = 12;
        a->v28 = (a->v28 + 1) & 3;
    }
    a->v2C--;
}

static void BrightPalette(struct Fight *f, struct Actor *a) {
    if (!a->warn && !a->v20) return;
    a->held = TRUE;
    f->brightColors = (f->frameCount & (a->v20 ? 4 : 2)) ? gNidlShineBright.colors[C_BRIGHT] : gNidlShineBright.colors[C_BRIGHT_HURT];
}

void sub_0803D280(u8, u8);
extern u16 gUnk_02022120[0x100];    // The BG palettes as loaded (KatAM's palette effects put them back from it);

static bool32 Watched(struct Fight *f) {
    return gKirbys[gLocalPlayerId].base.roomId == f->room;
}

// Colors(); A palette's slot's colours (`pal`), written when they've changed;
static void Colors(struct Fight *f, u8 pal, const u16 *colors) {
    u16 *slot;
    u8 i;

    if (!Watched(f) || !f->pal[pal] || colors == NULL) return;
    slot = &gObjPalette[f->pal[pal] * 16];
    for (i = 1; i < 16 && slot[i] == colors[i]; i++) ;
    if (i == 16) return;
    for (i = 0; i < 16; i++) slot[i] = colors[i];
    sub_0803D280(f->pal[pal] * 16, 16);
    gMainFlags |= MAIN_FLAG_OBJ_PALETTE_SYNC_ENABLE;
}

/* Blend(); NiDL's 0x08003014: each of a colour's components `t` 256ths of its way from `a` to `b`; */
static u16 Blend(u16 a, u16 b, s32 t) {
    s32 r = (a & 0x1F) + ((((b & 0x1F) - (a & 0x1F)) * t) >> 8);
    s32 g = (a & 0x3E0) + ((((b & 0x3E0) - (a & 0x3E0)) * t) >> 8);
    s32 c = (a & 0x7C00) + ((((b & 0x7C00) - (a & 0x7C00)) * t) >> 8);

    return (r & 0x1F) | (g & 0x3E0) | (c & 0x7C00);
}

/* The sky (0x080275CC: the room's background's colours, KatAM's BG palettes 6-13: day, sunset, night; a fade to a neighbour 16
 * frames, day to night through the sunset, the next queued; 0x0802DA8C: 16 256ths a frame); */
#define SKY_COLORS 96

static void SkyColors(struct Fight *f) {
    const u16 *from = gNidlShineBright.sky[f->skyFrom], *to = gNidlShineBright.sky[f->skyTo];
    u8 i;

    if (!Watched(f)) return;
    for (i = 0; i < 128; i++) gBgPalette[SKY_COLORS + i] = gUnk_02022120[SKY_COLORS + i] = Blend(from[i], to[i], f->skyT);
    gMainFlags |= MAIN_FLAG_BG_PALETTE_SYNC_ENABLE;
}

static void SkyTo(struct Fight *f, s8 n) {
    s8 near, queued;

    if (f->skyFading) {
        if (f->skyDir == 1) {
            if (f->skyTo == n) {
                f->skyQueued = NONE;
            } else if (f->skyFrom == n) {
                f->skyDir = -f->skyDir;
                f->skyQueued = NONE;
            } else {
                if (n < f->skyFrom) f->skyDir = -f->skyDir;
                f->skyQueued = n;
            }
        } else {
            if (f->skyFrom == n) {
                f->skyQueued = NONE;
            } else if (f->skyTo == n) {
                f->skyDir = -f->skyDir;
                f->skyQueued = NONE;
            } else {
                if (f->skyTo < n) f->skyDir = -f->skyDir;
                f->skyQueued = n;
            }
        }
        return;
    }
    if (f->skyCur == n) return;
    if (f->skyCur == SKY_SUNSET || n == SKY_SUNSET) {
        near = n;
        queued = NONE;
    } else {
        near = SKY_SUNSET;
        queued = n;
    }
    if (f->skyCur < n) {
        f->skyFrom = f->skyCur;
        f->skyTo = near;
        f->skyDir = 1;
        f->skyT = 0;
    } else {
        f->skyFrom = near;
        f->skyTo = f->skyCur;
        f->skyDir = -1;
        f->skyT = 256;
    }
    f->skyQueued = queued;
    f->skyFading = TRUE;
}

static void SkyStep(struct Fight *f) {
    s8 q;

    if (!f->skyFading) return;
    if (f->skyDir > 0) {
        f->skyT += 16;
        if (f->skyT > 256) f->skyT = 256;
    } else {
        f->skyT -= 16;
        if (f->skyT < 0) f->skyT = 0;
    }
    SkyColors(f);
    if (f->skyDir > 0 && f->skyT == 256) {
        if (f->skyQueued != NONE) {
            q = f->skyQueued;
            f->skyFrom = f->skyTo;
            f->skyTo = q;
            f->skyQueued = NONE;
            f->skyT = 0;
            f->skyCur = f->skyFrom;
        } else {
            f->skyCur = f->skyTo;
            f->skyFading = FALSE;
        }
    } else if (f->skyDir < 0 && f->skyT == 0) {
        if (f->skyQueued != NONE) {
            q = f->skyQueued;
            f->skyTo = f->skyFrom;
            f->skyFrom = q;
            f->skyQueued = NONE;
            f->skyT = 256;
            f->skyCur = f->skyTo;
        } else {
            f->skyCur = f->skyFrom;
            f->skyFading = FALSE;
        }
    }
}

/* The darkening under the beam (type 0xAE's fifth, 0x080657F8, 0x08065C14: the room's colours, KatAM's BG palettes 0-13, from what
 * they were then towards black, 10 256ths a frame to 80; back 16 a frame (0x080657CC); back at once (0x080657A4)); */
static void DarkColors(struct Fight *f) {
    // (Blend()'s, to black: each component c to c + (-c * t >> 8), the same for the three, so one table of 32 a frame: no lag);
    u16 ramp[32];
    const u16 *from = f->backup;
    u16 *to = gBgPalette;
    s32 i, t = f->dark;
    u32 c;

    if (!Watched(f)) return;
    for (i = 0; i < 32; i++) ramp[i] = i + ((-i * t) >> 8);
    for (i = 224; i > 0; i--) {
        c = *from++;
        *to++ = ramp[c & 0x1F] | ramp[(c >> 5) & 0x1F] << 5 | ramp[c >> 10 & 0x1F] << 10;
    }
    gMainFlags |= MAIN_FLAG_BG_PALETTE_SYNC_ENABLE;
}

static void Darken(struct Fight *f, s16 step, s16 max) {
    u8 i;

    if (f->darkMode == DARK_OFF) f->dark = 0;
    f->darkStep = step;
    f->darkMode = 0;
    f->darkMax = max;
    for (i = 0; i < 224; i++) f->backup[i] = gBgPalette[i];
}

static void Lighten(struct Fight *f, s16 step) {
    if (f->darkMode == DARK_OFF) return;
    f->darkStep = step;
    f->darkMode = 1;
}

static void Undarken(struct Fight *f) {
    if (f->darkMode != DARK_OFF) f->darkMode = 2;
}

static void DarkStep(struct Fight *f) {
    switch (f->darkMode) {
    case 0:
        f->dark += f->darkStep;
        if (f->dark > f->darkMax) {
            f->dark = f->darkMax;
            f->darkMode = 3;
        }
        DarkColors(f);
        break;
    case 1:
        f->dark -= f->darkStep;
        if (f->dark < 0) {
            f->dark = 0;
            f->darkMode = 3;
        }
        DarkColors(f);
        break;
    case 2:
        f->dark = 0;
        DarkColors(f);
        f->darkMode = 3;
        break;
    case 3:
        // (held: kept as it is, KatAM's own effects putting the room's colours back aside);
        if (f->dark > 0 && gBgPalette[1] != Blend(f->backup[1], 0, f->dark)) DarkColors(f);
        break;
    }
}

/* Mosaic(); The room's layers mosaic (NiDL's hurt: as Meta Knight's); */
static void Mosaic(u8 level, bool32 on) {
    u8 i;

    *(vu16 *)REG_MOSAIC_ADDR = level;
    for (i = 0; i < 4; i++) {
        if (!(MOSAIC_LAYERS & (1 << i))) continue;
        if (on) gBgCntRegs[i] |= 0x40;
        else gBgCntRegs[i] &= ~0x40;
    }
}

/* HurtStep(); A step of a boss's hurt (NiDL's 0x080662D8, their handler 0x080A1FC8, 0x080A1F90: by its countdown), frozen: flashing
 * in its hurt colours 2 frames in 4, jittering (sJitter), the room's layers mosaic (sMosaic); out, back where it was hit; */
static void HurtStep(struct Fight *f, struct Actor *a) {
    u8 i = sJitterOf[a->hurt];
    const u16 *own = a->kind == K_SHINE ? f->shineColors : gNidlShineBright.colors[C_BRIGHT];
    const u16 *hurt = gNidlShineBright.colors[a->kind == K_SHINE ? C_SHINE_HURT : C_BRIGHT_HURT];

    a->x += sJitter[i][0] << 16;
    a->y += sJitter[i][1] << 16;
    a->ix = a->x >> 16;
    a->iy = a->y >> 16;
    a->hurt--;
    if (a->kind == K_SHINE) f->shineFlash = (a->hurt & 2) ? NULL : hurt;
    else f->brightColors = (a->hurt & 2) ? own : hurt;
    if (a->kind == K_BRIGHT) a->held = TRUE;
    if (Watched(f)) Mosaic(sMosaic[a->hurt >> 1], a->hurt != 0);
    if (a->hurt == 0) {
        a->x = a->hurtX;
        a->y = a->hurtY;
        a->ix = a->x >> 16;
        a->iy = a->y >> 16;
        f->shineFlash = NULL;
        if (a->kind == K_BRIGHT) {
            a->held = a->warn || a->v20;
            f->brightColors = own;
        }
    }
}

/* Hit(); A hit took HP off KatAM's object (the one on the ground: f->ground; KatAM's, between frames): its HP (twice the hit's, NiDL's
 * 30 each on the HP bar's 60); out, beaten (Beaten());
 * else hurt (its first step as the hit's frame ends, KatAM's hit a frame later: two steps this frame); its state's function not run
 * this frame (0x080692FC); */
static void Hit(struct Fight *f) {
    struct Object *obj = f->obj;
    struct Actor *a;

    // (none on the ground, hurt, or beaten already (rising yet): its HP back);
    if (f->ground == NONE || f->a[f->ground].hurt || f->a[f->ground].v20) {
        obj->unk80 = f->hp;
        return;
    }
    a = &f->a[f->ground];
    // (NiDL's 30 HP each, kept on the HP bar's 60 as the other bosses' (nidl_boss.c): a hit takes its HP twice over);
    a->hp = obj->unk80 = f->hp - (f->hp - obj->unk80) * 2;
    f->hit = TRUE;
    if (a->hp <= 0) {
        a->hp = 0;
        Beaten(f, a);
        return;
    }
    a->hurtX = a->x;
    a->hurtY = a->y;
    a->hurt = HURT_TIME;
    HurtStep(f, a);
}

/* Their sprites (KatAM's: their tiles, NiDL's frames as KatAMExpansion.cs lays them out; mirrored by NiDL's mirroring) and their
 * palettes' slots (loaded under their keys, as their frames have them, the local player in the room); */
static void SpriteMain(void) {
    struct EffectObject *e = TaskGetStructPtr(gCurTask);

    if (e->flags & 0x1000) TaskDestroy(gCurTask);
}

static struct EffectObject *Sprite(struct Fight *f, u16 tiles, u8 frame, u8 order) {
    struct Task *t = TaskCreate(SpriteMain, sizeof(struct EffectObject), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct EffectObject *e = TaskGetStructPtr(t);

    ClearEffectObject(e);
    e->header.kind = 3;
    e->roomId = f->obj->base.roomId;
    e->parent = NULL;
    e->flags |= 0x400;
    EffectObjectInitSprite(e, &e->sprite, tiles, gNidlData.bosses[NIDL_SHINE_BRIGHT].animId, frame, order);
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

static u8 Palette(struct Fight *f, u16 key, u8 frame) {
    u8 palId = sub_0803DF24(key);

    if (palId == 0xFF) {
        if (!Watched(f)) return 0;
        sub_0803DFAC(key, frame);
        palId = sub_0803DF24(key);
    }
    return palId == 0xFF ? 0 : palId;
}

static void Palettes(struct Fight *f) {
    u16 animId = gNidlData.bosses[NIDL_SHINE_BRIGHT].animId;

    if (!f->pal[PAL_SHINE]) f->pal[PAL_SHINE] = Palette(f, animId, FRAME_SHINE);
    if (!f->pal[PAL_BRIGHT]) f->pal[PAL_BRIGHT] = Palette(f, animId | KEY_BRIGHT, FRAME_BRIGHT);
    if (!f->pal[PAL_SHOT]) f->pal[PAL_SHOT] = Palette(f, animId | KEY_SHOT, FRAME_SHOT);
    if (!f->pal[PAL_BEAM]) f->pal[PAL_BEAM] = Palette(f, animId | KEY_BEAM, FRAME_BEAM);
    if (!f->pal[PAL_COMMON]) f->pal[PAL_COMMON] = Palette(f, animId | KEY_COMMON, FRAME_PUFF);
    Colors(f, PAL_SHINE, f->shineFlash ? f->shineFlash : f->shineColors);
    Colors(f, PAL_BRIGHT, f->brightColors);
}

/* Look(); An Actor's KatAM frame (FRAME_HIDDEN none), its tiles' group and palette; */
static s16 Look(struct Fight *f, struct Actor *a, u8 *group, u8 *pal) {
    s16 k = a->frame;

    if (k < 0) return FRAME_HIDDEN;
    switch (a->kind) {
    case K_SHINE:
        *group = G_SHINE;
        *pal = PAL_SHINE;
        return SHINE(k);
    case K_BRIGHT:
        *group = G_BRIGHT;
        *pal = PAL_BRIGHT;
        return BRIGHT(k);
    case K_FLAMES:
        *group = G_FLAMES;
        *pal = PAL_BRIGHT;
        return (a->v18 == 2 ? FRAME_FLAMES_BACK : FRAME_FLAMES + 6 * a->v18) + k % 6;
    case K_CRESCENT:
        *group = G_CRESCENT;
        *pal = PAL_SHINE;
        return FRAME_CRESCENT + k - 4;
    case K_SHOT:
        *group = G_SHOT;
        *pal = PAL_SHOT;
        return FRAME_SHOT + k;
    case K_BEAM: case K_PART:
        *group = G_BEAM;
        *pal = PAL_BEAM;
        return FRAME_BEAM + k;
    case K_SPARK:
        *group = G_SPARK;
        *pal = PAL_COMMON;
        return FRAME_SPARK + k - 4;
    case K_TRAIL:
        *group = G_TRAIL;
        *pal = PAL_COMMON;
        return FRAME_TRAIL + k / 2;
    case K_FIRE:
        *group = G_FIRE;
        *pal = PAL_COMMON;
        return FRAME_FIRE + k / 2;
    case K_PUFF:
        *group = G_PUFF;
        *pal = PAL_COMMON;
        return FRAME_PUFF + k;
    }
    return FRAME_HIDDEN;
}

static u16 Tiles(u8 group) {
    if (group == G_FLAMES && gNidlShineBright.tiles[G_FLAMES_BACK] > gNidlShineBright.tiles[G_FLAMES])
        return gNidlShineBright.tiles[G_FLAMES_BACK];
    return gNidlShineBright.tiles[group];
}

/* DrawActor(); An Actor's sprite at its pixel (its own: a new one as its frames' tiles change, the shot's puff); its order by its
 * priority (the flames behind Mr. Bright's face at his own); */
static void DrawActor(struct Fight *f, struct Actor *a) {
    u8 group = 0, pal = 0, order = ORDER(a->prio);
    s16 k = Look(f, a, &group, &pal);

    if (a->kind == K_FLAMES && f->a[A_BRIGHT].prio == a->prio) order++;
    if (k == FRAME_HIDDEN) {
        if (a->sprite) a->sprite->flags |= 0x400;
        return;
    }
    if (a->sprite && a->group != group) Release(&a->sprite);
    if (a->sprite == NULL) {
        a->sprite = Sprite(f, Tiles(group), k, order);
        a->group = group;
    }
    Draw(a->sprite, a->ix << 8, a->iy << 8, k, f->pal[pal], a->flip, order);
}

/* Their boxes hurting Kirby (attack objects, as Meta Knight's Galaxia: NiDL's 0x08068E04): the crescent's (-8, -6 to 8, 6), the
 * fireball's (+-10), the beam's (-18, -42 to 18, 42), the sparks' (+-5, till their last frames); */
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

static void Attack(struct Fight *f, struct Actor *a) {
    struct Object *obj = f->obj;
    struct ObjectBase *b;
    s8 box[4];
    bool32 on;

    switch (a->kind) {
    case K_CRESCENT: box[0] = -8; box[1] = -6; box[2] = 8; box[3] = 6; on = TRUE; break;
    case K_SHOT: box[0] = box[1] = -10; box[2] = box[3] = 10; on = TRUE; break;
    case K_BEAM: box[0] = -18; box[1] = -42; box[2] = 18; box[3] = 42; on = TRUE; break;
    case K_SPARK: box[0] = box[1] = -5; box[2] = box[3] = 5; on = !a->v20; break;
    default: on = FALSE; break;
    }
    if (!on || f->over) {
        if (a->attack) a->attack->flags |= 0x1000;
        a->attack = NULL;
        return;
    }
    if (a->attack == NULL) {
        struct Task *t = TaskCreate(AttackMain, sizeof(struct ObjectBase), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);

        b = a->attack = TaskGetStructPtr(t);
        ClearObjectBase(b);
        b->header.kind = 2;
        b->parent = f;
        b->roomId = obj->base.roomId;
        b->unk56 = obj->base.unk56;
        b->unk63 = 1;
        b->flags |= 0x10000140 | 0x600;
        b->unk68 = 0x20000043;
        b->unk5C |= 0x80000;
        ObjectSetBounds(b, 0, 0, 0, 0);
        sub_0803E2B0(b, box[0], box[1], box[2], box[3]);
    }
    b = a->attack;
    b->x = a->ix << 8;
    b->y = a->iy << 8;
    b->flags &= ~0x200;
}

/* FallingStar(); 0x080A2954's star: an Actor (StarScript()) and KatAM's enemy it moves (enemies/star.c: inhaled, it's Kirby's; let go,
 * it falls on as it was going), its frames (the blink's 0, 1, Mr. Shine's colours; the star's 2-5, NiDL's shared effects'); */
static void FallingStar(struct Fight *f, u8 left) {
    struct Object *obj = f->obj, *star;
    struct Actor *a;
    s8 i = Spawn(f, K_STAR, A_SHINE, 2, left, f->a[A_SHINE].ix, f->a[A_SHINE].iy, TRUE);

    if (i == NONE) return;
    a = &f->a[i];
    star = CreateObjTemplateAndObj(obj->base.unk56, 1, 0x24, a->ix, a->iy, 0, 0x1F, 0, 0, NIDL_TYPE, NIDL_SPECIAL << 4 | 2, 0, 0, 0, 0, 0,
                                   0, 0, 0, 0, 0, 0, 0);
    // (KatAM's despawn off the screen not its while it's NiDL's: OffScreen()'s, further off; let go, KatAM's again, LetGo());
    if (star) star->base.flags |= 0x2000000;
    a->enemy = star;
}

static void DriveStar(struct Fight *f, struct Actor *a) {
    struct Object *star = a->enemy;

    if (star == NULL) return;
    if ((star->base.flags & 0x1000) || (star->base.unkC & 0x400)) {
        // (gone, or Kirby's: inhaled; the Actor goes, NiDL's star with it);
        a->enemy = NULL;
        Destroy(f, a - f->a);
        return;
    }
    star->base.x = a->ix << 8;
    star->base.y = a->iy << 8;
    star->base.xspeed = star->base.yspeed = 0;
    NidlSetFrame((struct NidlObject *)star, a->v34 ? 2 + a->frame - 4 : a->frame - 4);
    star->base.sprite.palId = f->pal[a->v34 ? PAL_COMMON : PAL_SHINE];
    star->base.flags = (star->base.flags & ~1) | (a->flip == -1 ? 0 : 1);
}

static void LetGo(struct Actor *a) {
    struct Object *star = a->enemy;

    if (star == NULL || (star->base.flags & 0x1000) || (star->base.unkC & 0x400)) return;
    star->base.xspeed = a->vx >> 8;
    star->base.yspeed = -(a->vy >> 8);
    star->base.counter = 1;
    star->base.flags &= ~0x2000000;
    a->enemy = NULL;
}

// DashTrail(), FireTrail(); 0x0806E6F8 (-16, 7) and 0x0806E808 (24, 5): their trails (effects: their scripts next run);
static s8 DashTrail(struct Fight *f) {
    s8 i = Spawn(f, K_TRAIL, A_SHINE, 0, 0, 0, 0, FALSE);

    if (i != NONE) {
        f->a[i].v24 = -16;
        f->a[i].v20 = 7;
    }
    return i;
}

static s8 FireTrail(struct Fight *f) {
    s8 i = Spawn(f, K_FIRE, A_BRIGHT, 0, 0, 0, 0, FALSE);

    if (i != NONE) {
        f->a[i].v24 = 24;
        f->a[i].v20 = 5;
    }
    return i;
}

/* BeatenEffects(); 0x080A1EC4's burst (NiDL's effects 0x95 and 0xA2): KatAM's hit stars where it is; */
static void BeatenEffects(struct Fight *f, struct Actor *a) {
    struct Object *obj = f->obj;
    s32 x = obj->base.x, y = obj->base.y;

    obj->base.x = a->ix << 8;
    obj->base.y = a->iy << 8;
    CreateEffectObject(&obj->base, 0, 0x299, 0);
    obj->base.x = x;
    obj->base.y = y;
}

/* Defeat(); 0x080A1D84 (both beaten, shivering at the middle): KatAM's boss defeat (as its killing blow's: the music, the explosion
 * there; NidlBossDefeat(): the shard); they shiver on till it bursts them (ShineBrightMain(): untouchable, where they are); */
static void Defeat(struct Fight *f) {
    struct Object *obj = f->obj;
    struct Actor *c = &f->a[A_CONTROLLER];

    if (f->over) return;
    f->over = TRUE;
    obj->unk80 = 0;
    obj->base.x = c->ix << 8;
    obj->base.y = c->iy << 8;
    obj->base.flags |= 0x200;
    sub_08086C48(obj);
    PlaySfx(&obj->base, SE_MINIBOSS_PRE_EXPLODE);
    sub_08088528(obj);
}

/* Ground(); KatAM's object is the one on the ground (its box: hit, touching hurts; Mr. Shine first when he's the one down there:
 * the intro's last frame has both); none (both in the sky, their boxes off, beaten): untouchable, where it was; */
static void Ground(struct Fight *f) {
    struct Object *obj = f->obj;
    s8 first = f->gShineDown ? A_SHINE : A_BRIGHT, second = f->gShineDown ? A_BRIGHT : A_SHINE;

    if (f->a[first].box == BOX_GROUND) f->ground = first;
    else if (f->a[second].box == BOX_GROUND) f->ground = second;
    else f->ground = NONE;
    if (f->ground == NONE || !f->gOn || f->over) {
        obj->base.flags |= 0x200;
        return;
    }
    obj->base.x = f->a[f->ground].ix << 8;
    obj->base.y = f->a[f->ground].iy << 8;
    obj->base.flags &= ~0x200;
}

static void ShineBrightMain(struct Object *obj);

/* PreMain(); KatAM's object's own before its Main (unk7C, after its hits): a killing blow (the one on the ground beaten, KatAM's object
 * at 0: unkC 0x800, sub_0809AF38) froze it, its attack off; the fight goes on (Hit(): beaten), its Main and attack given back; */
static void PreMain(struct Object *obj) {
    struct ShineBright *s = (struct ShineBright *)obj;
    struct Fight *f = s->f;

    if (f == NULL || f->over) return;
    if (obj->unk78 != ShineBrightMain) {
        obj->unk78 = ShineBrightMain;
        obj->base.flags &= ~0x200;
        obj->base.unk68 = f->unk68;
    }
    // (the room held still after one's beaten, NiDL's game: no more hits, a shot still on it its HP not taken; Ground() after);
    if (f->freeze) {
        obj->unk80 = f->hp;
        obj->base.flags |= 0x200;
    }
}

/* ShineBrightMain(); KatAM's object's Main (the one on the ground): its hits first (Hit()), then each of NiDL's objects in its slot's
 * order (Process()), the darkening, the sky (their objects after: slots 40, 62), their boxes and the falling stars, KatAM's object
 * where the one on the ground is; */
static void ShineBrightMain(struct Object *obj) {
    struct ShineBright *s = (struct ShineBright *)obj;
    struct Fight *f = s->f;
    s8 i;

    NidlUpdate(&s->n);
    obj->base.flags |= 4 | 0x400;
    obj->kirby3 = FindTargetKirby(&obj->base);
    // The HP bar goes to whatever is hit; it takes it back;
    sub_08034E14(obj);
    if (f == NULL) return;
    obj->base.xspeed = obj->base.yspeed = 0;
    f->hit = FALSE;
    if (!f->over && obj->unk80 < f->hp) {
        NidlBossHitShake(obj);
        Hit(f);
    }
    // (a killing blow: the room held still from this frame on, as NiDL's from the end of its hit's frame);
    if (f->freeze) {
        Ground(f);
        f->hp = obj->unk80;
        return;
    }
    for (i = 0; i < ACTORS; i++) Process(f, i);
    DarkStep(f);
    SkyStep(f);
    ControllerLate(f);
    for (i = A_CHILDREN; i < ACTORS; i++) {
        struct Actor *a = &f->a[i];

        if (a->kind == K_NONE) continue;
        Attack(f, a);
        if (a->kind == K_STAR) DriveStar(f, a);
    }
    Ground(f);
    if (f->over) obj->unk80 = 0;
    f->hp = obj->unk80;
    f->frameCount++;
}

/* FightMain(); The fight's task: their sprites (all but the controller's and the stars': KatAM's enemies, enemies/star.c), their
 * palettes; gone with KatAM's object (its defeat's burst, or the room left): the stars let go, the sprites and boxes gone, the beam's
 * sound, the mosaic off; */
static void FightMain(void) {
    struct Fight *f = TaskGetStructPtr(gCurTask);
    struct Object *obj = f->obj;
    u8 i;

    if (obj == NULL || (obj->base.flags & 0x1000)) {
        for (i = 0; i < ACTORS; i++) {
            struct Actor *a = &f->a[i];

            if (a->kind == K_STAR) LetGo(a);
            a->enemy = NULL;
            Release(&a->sprite);
            if (a->attack) a->attack->flags |= 0x1000;
            a->attack = NULL;
        }
        if (f->beamSound) m4aSongNumStop(SFX_BEAM);
        if (f->freeze) sub_080335B4(f->player);
        if ((f->a[A_SHINE].hurt || f->a[A_BRIGHT].hurt) && Watched(f)) Mosaic(0, FALSE);
        if (obj) ((struct ShineBright *)obj)->f = NULL;
        TaskDestroy(gCurTask);
        return;
    }
    // (KatAM's objects held still, the fight's task not: it lets them go, its frames done: ShineBrightMain() runs on the next);
    if (f->freeze && !--f->freeze) sub_080335B4(f->player);
    Palettes(f);
    for (i = A_SHINE; i < ACTORS; i++) {
        struct Actor *a = &f->a[i];

        if (a->kind != K_NONE && a->kind != K_STAR) DrawActor(f, a);
    }
}

// ShineBrightGone(); KatAM's object's gone (nidl.c's NidlDestroy()): the fight's task lets go of it;
void ShineBrightGone(struct Object *obj) {
    struct ShineBright *s = (struct ShineBright *)obj;

    if (s->f) s->f->obj = NULL;
    s->f = NULL;
}

/* ShineBrightStart(); The fight starts (NidlBossStart(): Kirby in its room), its task with it: NiDL's 0x080A1AD0: the controller (KatAM's
 * object's spot: NiDL's room object's, 208, 104), Mr. Shine (192, 48 on NiDL's screen) and Mr. Bright (32, 64) falling to the floor,
 * Mr. Bright's flames on him; their boxes on (+0x18), half their HP (+0x20); the room's sunset; */
void ShineBrightStart(struct Object *obj) {
    struct ShineBright *s = (struct ShineBright *)obj;
    struct Task *t;
    struct Fight *f;
    u8 i;

    ObjectSetFunc(obj, -1, ShineBrightMain);
    if (s->started) return;
    s->started = TRUE;
    t = TaskCreate(FightMain, sizeof(struct Fight), 0x3500, TASK_USE_EWRAM, NULL);
    f = s->f = TaskGetStructPtr(t);
    CpuFill32(0, f, sizeof(struct Fight));
    f->obj = obj;
    f->room = obj->base.roomId;
    f->seed = Rand16() & 0xFFF;
    for (i = 0; i < 4; i++) f->kids[i] = NONE;
    f->ground = NONE;
    f->hp = obj->unk80;
    f->skyCur = SKY_SUNSET;
    f->skyQueued = NONE;
    f->darkMode = DARK_OFF;
    f->shineColors = gNidlShineBright.colors[C_GLOW];
    f->brightColors = gNidlShineBright.colors[C_BRIGHT];
    Make(f, A_CONTROLLER, K_CONTROLLER, A_CONTROLLER, 0, 0, obj->base.x >> 8, obj->base.y >> 8);
    Make(f, A_SHINE, K_SHINE, A_CONTROLLER, 1, 0, NIDL_SCREEN_X + 192, NIDL_SCREEN_Y + 48);
    Make(f, A_BRIGHT, K_BRIGHT, A_CONTROLLER, 2, 0, NIDL_SCREEN_X + 32, NIDL_SCREEN_Y + 64);
    Make(f, A_FLAMES, K_FLAMES, A_BRIGHT, 3, 0, 0, 0);
    for (i = A_CONTROLLER; i <= A_FLAMES; i++) {
        struct Actor *a = &f->a[i];

        a->prio = 11;
        a->restart = TRUE;
    }
    for (i = A_SHINE; i <= A_BRIGHT; i++) {
        struct Actor *a = &f->a[i];

        FaceKirby(f, a);
        a->hp = obj->unk80;
    }
    f->a[A_FLAMES].relative = TRUE;
    f->gOn = 1;
    f->gHalf = obj->unk80 >> 1;
    // (their sprites are the fight's task's: KatAM's object's never shown, its VRAM theirs; unkC 0x200 keeps KatAM from giving it VRAM
    // again, flag 8 from animating it; 0x800: its killing blow only freezes it, PreMain());
    obj->base.unkC |= 0x200 | 0x800;
    if (obj->base.sprite.tilesVram >= 0x6014000) VramFree(obj->base.sprite.tilesVram);
    obj->base.sprite.tilesVram = 0;
    obj->base.flags |= 8 | 0x200;
    f->unk68 = obj->base.unk68;
    obj->unk7C = PreMain;
}

// ShineBrightInit(); Hidden (the fight's task draws them), through the room's terrain (placed where they are); its box NiDL's on the
// ground (-16, -16 to 16, 16);
void ShineBrightInit(struct NidlObject *n) {
    struct ShineBright *s = (struct ShineBright *)n;
    struct Object *obj = &n->obj;

    s->f = NULL;
    s->started = FALSE;
    obj->base.flags |= 0x400 | 0x100;
    obj->base.unk5C = (obj->base.unk5C & ~7) | 3 | 0xA0;
    sub_0803E2B0(&obj->base, -16, -16, 16, 16);
    ObjectSetBounds(&obj->base, -8, -8, 8, 8);
}
