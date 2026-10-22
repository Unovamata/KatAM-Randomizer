/* King Dedede; ported from Kirby: Nightmare in Dream Land (USA), boss object type 0x3B (spawn 0x0809FC44), as NiDL boss
 * NIDL_KING_DEDEDE, Rainbow Resort's (room 0x313 here, the ring on the Fountain of Dreams). NiDL's, from its code (11 states, each a
 * script, 0x0874844C, and a function every frame after it, 0x08748478; his main 0x080A0A38; his hit 0x0809FE10, his hurt 0x080A0A84),
 * checked frame by frame against NiDL's recorded fight (his states, frames, spots and speeds, his children's): see "The fight" below.
 * NiDL's objects are Actors here (one array, NiDL's object slots 32 on, in NiDL's order): him, his hammer's box (type 0xB6), his
 * inhale's box (0xB7), the stars he knocks out (0x8B: KatAM's star enemies, enemies/star.c, that Kirby inhales), his dust (0x92, 0x93,
 * 0x90, 0xB2), his breath (0x71) and his hit stars (0x8E), each drawn as a sprite of its own (EffectOnly frames: KatAMExpansion.cs'
 * KingDedede()); the boss object KatAM knows (hit, its HP bar) is him. His defeat (DededeEnd(), as KatAM's explosion bursts him: NiDL's
 * type 0x3B's second object, 0x080A150C): down to the floor, a slide back, dazed; then his shard;
 * His frames are NiDL's streamed frames (0x087538E0, 4-43), with two palettes each: his body (the first palette's pieces) and the
 * second's, in front of it or behind (NiDL draws them in its pieces' order: three sprites, gNidlKingDedede's front and back); */
#include "../nidl.h"
#include "code_0806F780.h"
#include "task.h"
#include "main.h"
#include "kirby.h"
#include "functions.h"
#include "constants/songs.h"

void sub_0804E09C(struct Kirby *);
void sub_0803D280(u8, u8);

/* gNidlKingDedede; Filled in by KatAMExpansion.cs (WriteKingDededeFight()): per NiDL frame (0-43) the KatAM frame of his second
 * palette's pieces in front of his body and behind it (FRAME_NONE: none; his body is BODY()), his effects' first frames (FX_*), the
 * VRAM tiles of each group's biggest frame (G_*), his body's colours and his hurt flash's (NiDL's, USA 0x082D8638); */
struct NidlKingDedede {
    u8 front[44];
    u8 back[44];
    u8 fx[8];
    u8 tiles[8];
    u16 body[16], flash[16];
};

const struct NidlKingDedede gNidlKingDedede = { { 0 } };

#define BODY(f) (1 + (f) - 4)       // (KatAM's frame 0 is his object's own, never shown: NiDL's 4);
#define FRAME_HIDDEN -1
#define FRAME_NONE 0xFF
#define NO_MAX 0x7FFFFFFF
#define PC_END 0xFFFF
#define LOOP -3                     // (NiDL's animations' ends: back to the start, or held);
#define END -2
#define NONE -1
// His effects' first frames (gNidlKingDedede.fx: NiDL's shared dust 0x0874CB90's 0-9, 0x0874C500's 0, the hit stars 0x0874CBC8's 0-1,
// his breath 0x08753990's 0, his inhale's sparkle 0x0874C44C's 45);
enum { FX_DUST, FX_SMALL_DUST, FX_HIT, FX_BREATH, FX_SPARKLE };
enum { G_BODY, G_FRONT, G_BACK, G_FX };
#define NIDL_SCREEN_X 24            // NiDL's camera in his room (from the fight's start on, held there; KatAM shows the room as NiDL's);
#define NIDL_SCREEN_Y 168
#define MIDDLE_X (NIDL_SCREEN_X + DISPLAY_WIDTH / 2)
#define REACH 87                    // His walk's spot this far from the screen's middle at most (0x080A0358: 0x03005628, 144 both);
#define HALF_WIDTH 16               // His terrain box (NiDL's 0x08748900: 16 either side, his head 24 up, his feet 32 down; the
#define HEAD 24                     // screen's sides 24 from him: its camera held, NiDL's fight keeps him on it, his walls);
#define FEET 32
#define SIDE 24
#define KIRBY_DY 4                  // (NiDL's Kirby stands 4 pixels higher: KatAM's Kirby's spot, NiDL's: KirbyY());
#define INTRO_TIME 128              // His intro (state 0, 0x080664E0): NiDL's HP bar filling, KatAM's own fill meanwhile;
#define GUARD_TIME 31               // Frames he takes no hits after one (NiDL's +0x75; KatAM's 0x8000: its attacks pass him by);
#define PULL_TIME 6                 // A Kirby caught drawn into his mouth (NiDL's Kirby's own code; here KatAM's grab: DrawIn());
#define MOUTH_X 23
#define MOUTH_Y 10
#define ORDER_FX 0x18               // Sprite orders (NiDL's priorities: his effects 9 and 10 in front of him, 11; his second palette's
#define ORDER_FRONT 0x19            // pieces in front of his body or behind it);
#define ORDER_BODY 0x1A
#define ORDER_BACK 0x1B
#define KEY_TRIM 0x2000             // His palettes' keys (| his anim ID; his body's is his anim ID);
#define KEY_COMMON 0x4000
#define KEY_BREATH 0x6000
#define STAR_FRAMES 6               // (enemies/star.c's frames: 0x0874CCBC's 4-13 from its 6: NiDL's shared star of type 0x8B);

enum { PAL_BODY, PAL_TRIM, PAL_COMMON, PAL_BREATH, PALS };

// NiDL's objects (Actor.kind): him, then his children (NiDL's slots 33 on);
enum { K_NONE, K_DEDEDE, K_HAMMER, K_CATCH, K_STAR, K_DUST, K_SLIDE_DUST, K_PUFF_DUST, K_SMALL_DUST, K_BREATH, K_HIT };
#define ACTORS 24
#define A_DEDEDE 0

/* NiDL's tables (0x08748268 on): his stand animations (0x08748384, angry 0x08748398) and their times (0x08748374), his rise puffed up
 * (0x087483B8), his breath's push back (0x08748410), his swings' times (0x08748420: read 3 on for the fourth swing, past their
 * end, as NiDL does), his hurt's jitter (0x08748430), his moves after each state (0x087482A8), his inhale's sparkles' spots
 * (0x08748268, 0x08748288), his breath's speeds (0x087489C0), his daze (0x087484A4); */
static const s16 sStand[] = { 4, 16, 5, 14, 6, 12, 7, 14, LOOP };
static const s16 sStandAngry[] = { 4, 10, 5, 8, 6, 6, 7, 8, LOOP };
static const s16 sStandTimes[6] = { 60, 50, 40, 50, 40, 30 };
static const s32 sRise[6][2] = { { 0, 8 }, { -0x30000, 4 }, { -0x20000, 8 }, { -0x10000, 8 }, { -0x8000, 8 }, { 0x8000, 8 } };
static const s32 sPushBack[4] = { -0x20000, -0x10000, -0x8000, 0x8000 };
static const u16 sSwings[8] = { 4, 1, 25, 4, 2, 1, 15, 2 };
static const s16 sJitter[14] = { 1, 1, -1, 2, 1, 2, -1, 2, 1, 2, -1, 1, 0, 4 };
static const s8 sSparkleX[16] = { 15, 20, 25, 30, 35, 40, 44, 44, 44, 44, 40, 35, 30, 25, 20, 15 };
static const s8 sSparkleY[16] = { -16, -16, -14, -14, -12, -12, -12, -5, 5, 12, 16, 16, 16, 16, 16, 16 };
static const s32 sBreath[5] = { 0x40000, 0x20000, 0x10000, 0x8000, 0x2000 };
static const s16 sDaze[] = { 11, 10, 12, 4, 11, 50, 12, 4, 11, 6, 12, 6, 11, 60, LOOP };
// (each state's next moves: a {state, the walk's move} by chance of 8, by the state he was in, sMoves[]: none after 0, 2, 10);
static const u8 sMovesA[8][2] = { { 2, 8 }, { 2, 7 }, { 2, 3 }, { 2, 7 }, { 2, 8 }, { 2, 3 }, { 2, 7 }, { 2, 8 } };
static const u8 sMovesB[8][2] = { { 2, 7 }, { 6, 11 }, { 2, 8 }, { 2, 7 }, { 6, 11 }, { 2, 7 }, { 2, 8 }, { 2, 7 } };
static const u8 sMovesC[8][2] = { { 2, 7 }, { 6, 11 }, { 2, 8 }, { 2, 3 }, { 2, 7 }, { 2, 3 }, { 6, 11 }, { 2, 7 } };
static const u8 sMovesD[8][2] = { { 2, 8 }, { 2, 3 }, { 2, 8 }, { 2, 3 }, { 6, 11 }, { 2, 8 }, { 6, 11 }, { 2, 3 } };
static const u8 sMovesE[8][2] = { { 6, 11 }, { 2, 3 }, { 2, 7 }, { 2, 3 }, { 2, 7 }, { 6, 11 }, { 2, 3 }, { 2, 7 } };
static const u8 (*const sMoves[11])[2] = { NULL, sMovesA, NULL, sMovesB, sMovesC, sMovesC, sMovesA, sMovesD, sMovesE, sMovesE, NULL };
// Hit stars' jitter about him (0x0873EB30, 0x0873EB38);
static const s8 sHitX[8] = { -8, 0, 4, -16, 4, -8, -16, 4 };
static const s8 sHitY[8] = { -16, -20, 0, -16, 4, -16, -16, 0 };

/* Actor; One of NiDL's objects (its fields, by their NiDL offsets): a script (WAIT() resumes it, PC_END: over), its move (NiDL's
 * 0x080656B4) and its state's function each frame, in NiDL's order (its slot's); its sprite; */
struct Actor {
    u8 kind, variant;
    u8 state, next;                     // (+0x15, +0x14);
    u8 script;                          // The state whose script is running;
    bool8 restart;                      // Its next state starts as it's next run (NiDL's 0x08006148);
    u16 pc, wait;
    s8 facing, flip;                    // (+0x43; its frame's mirroring, +0x3E: its facing as its frame was last set);
    s16 frame;                          // (+0x3C; NiDL's: FRAME_HIDDEN none);
    s8 parent;                          // (+0x44: its parent's Actor);
    s8 child;                           // (+0x46: NONE none);
    s32 x, y;                           // 16.16 (+0x4C);
    s16 ix, iy;                         // Its pixel (+0x48);
    s16 dx;                             // Its pixel's nudge across (NiDL's 0x080063AC: there till the next);
    s32 vx, vy, ax, ay, maxX, maxY;     // (+0x54 on);
    s32 v18, v1C, v20, v24, v28, v2C, v30, v34;
    s16 loop;                           // (+0x6C);
    bool8 landed;                       // (+0x7A);
    const s16 *anim;                    // Its animation (ext +0x2C: NULL none; its step +9);
    u8 animStep;
    u8 prevState;                       // (ext +0x1C: the state before setState's);
    struct EffectObject *sprite;
    struct ObjectBase *attack;          // Its box hurting Kirby (an attack object);
    struct Object *enemy;               // The star's KatAM enemy (enemies/star.c);
};

struct KingDedede {
    struct NidlObject n;
    struct Fight *f;
    bool8 started;
};

typedef char sKingDededeFits[sizeof(struct KingDedede) <= NIDL_OBJECT_SIZE ? 1 : -1];

/* Fight; The fight's state (a task of its own), NiDL's objects and globals (0x02007D00: his inhale's catch, 0x02006040: its
 * sparkles, 0x02006190: his hurt); */
struct Fight {
    struct Object *obj;                 // (NULL: gone, KingDededeGone());
    struct Actor a[ACTORS];
    struct EffectObject *parts[3];      // His body, his second palette's pieces in front, behind;
    u16 seed;                           // NiDL's random numbers' (0x03000FB4);
    s16 hp;                             // KatAM's object's HP last frame (less now: hit);
    s16 angry;                          // Under this, angry (+0x70: his HP * 85 / 256);
    bool8 hit;                          // Hit this frame: his state's function not run (0x080692FC);
    bool8 over;                         // Beaten (his defeat under way: KatAM's explosion);
    bool8 hooked;                       // His hurt running (+8: 0x080A0A84, his own script held: +0x13 7);
    u8 guard;                           // Frames he takes no hits yet (+0x75: 31 from a hit's);
    bool8 flashing;                     // His hurt flash on (0x02006190 + 0x1C);
    s32 hurtLoops, hurtStep, hurtWait;  // (0x02006190 + 0xC, 0x10, 0x14);
    s16 hurtX, hurtY, hurtFrame;        // Where he was hit, his frame then (0x02006190: back as his hurt ends);
    s32 caught;                         // (0x02007D00: -1 none, 0 a Kirby being drawn in, 1 swallowed);
    s8 catchKirby;                      // (0x02007D20: the Kirby caught, NONE);
    u8 swallowed;                       // The Kirbys inside him (a bit each: KatAM's grab);
    u8 pull;                            // Frames of drawing them in yet;
    s32 sparkX[3], sparkY[3], sparkV[3]; // (0x02006040: his inhale's sparkles, from him; 0: none);
    struct EffectObject *sparkles[3];
    u16 frameCount;                     // (0x03001EA4: the flashes' clock);
    u8 unk56;                           // (the room's level: its terrain, Solid());
    u16 room;
    u8 pal[PALS];
    s16 lastX, lastY;                   // His pixel as the frame starts (his hit stars': HitFrame());
};

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

// RandOffset(); NiDL's 0x080064AC (and 0x080064DC across, mirrored by `a`'s facing): lo + step * (one of n);
static s32 RandOffset(struct Fight *f, struct Actor *a, s16 lo, u8 step, u8 n) {
    s32 v = lo + step * ((Rand(f, 256) * n) >> 8);

    return a && a->facing != 1 ? -v : v;
}

/* KirbyX(), KirbyY(); A Kirby's spot as NiDL's reads it, after his move this frame (NiDL moves the Kirbys before him, KatAM after:
 * his speed added; his y KIRBY_DY up, as NiDL has it); */
static s32 KirbyX(struct Kirby *k) {
    return (k->base.x + k->base.xspeed) >> 8;
}

static s32 KirbyY(struct Kirby *k) {
    return ((k->base.y - k->base.yspeed) >> 8) - KIRBY_DY;
}

static bool32 InRoom(struct Fight *f, struct Kirby *k) {
    return k->base.roomId == f->room && k->hp > 0;
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

// DxKirby(), DyKirby(); The closest Kirby from `a` (0x08063CD0, 0x08063D18; 0: none);
static s32 DxKirby(struct Fight *f, struct Actor *a) {
    struct Kirby *k = Closest(f, a);

    return k ? KirbyX(k) - a->ix : 0;
}

static s32 DyKirby(struct Fight *f, struct Actor *a) {
    struct Kirby *k = Closest(f, a);

    return k ? KirbyY(k) - a->iy : 0;
}

// FaceKirby(); 0x08063E14: towards the closest Kirby (right when level);
static void FaceKirby(struct Fight *f, struct Actor *a) {
    struct Kirby *k = Closest(f, a);

    if (k) a->facing = KirbyX(k) - a->ix >= 0 ? 1 : -1;
}

// KirbyLeft(); 0x08064398 (8): the closest Kirby to the left; KirbyAbove(); 0x08064358 (2): above;
static bool32 KirbyLeft(struct Fight *f, struct Actor *a) {
    return DxKirby(f, a) < 0;
}

static bool32 KirbyAbove(struct Fight *f, struct Actor *a) {
    return DyKirby(f, a) < 0;
}

static void Stop(struct Actor *a) {
    a->vx = a->vy = a->ax = a->ay = 0;
    a->maxX = a->maxY = NO_MAX;
}

static void StopX(struct Actor *a) {
    a->vx = a->ax = 0;
    a->maxX = NO_MAX;
}

static void StopY(struct Actor *a) {
    a->vy = a->ay = 0;
    a->maxY = NO_MAX;
}

// SetFrame(); NiDL's 0x08006338: its frame, mirrored by its facing (NiDL's other frame changes keep its mirroring);
static void SetFrame(struct Actor *a, s16 frame) {
    a->frame = frame;
    a->flip = a->facing;
}

// SetFrameBy(); 0x0800617C: its frame facing right, the next facing left (NiDL's dust: its frames by pairs), unmirrored;
static void SetFrameBy(struct Actor *a, s16 frame) {
    a->frame = a->facing == 1 ? frame : frame | 1;
}

// VelX(); NiDL's 0x080061C0: x speed (and change, unless VEL_KEEP) for facing right, mirrored otherwise;
#define VEL_KEEP 0x5A5A5A5A

static void VelX(struct Actor *a, s32 vx, s32 ax) {
    if (vx != VEL_KEEP) a->vx = a->facing == 1 ? vx : -vx;
    if (ax != VEL_KEEP) a->ax = a->facing == 1 ? ax : -ax;
}

// VelY(); NiDL's 0x0800622C: y speed, its change and its cap;
static void VelY(struct Actor *a, s32 vy, s32 ay, s32 maxY) {
    a->vy = vy;
    a->ay = ay;
    a->maxY = maxY < 0 ? -maxY : maxY;
}

static void SetX(struct Actor *a, s32 x) {
    a->ix = x;
    a->x = x << 16;
}

static void SetY(struct Actor *a, s32 y) {
    a->iy = y;
    a->y = y << 16;
}

/* Animations (ext +0x2C): Play() (0x080640DC: its frame mirrored by its facing at each step) steps it at once and gives the step's
 * frames; Step(); AnimTick() (0x08064188: the frames left of the step, the next once out) and AnimTickFacing() (0x0806415C: facing
 * Kirby at each step); */
static s16 Step(struct Actor *a) {
    const s16 *e = &a->anim[a->animStep * 2];

    if (e[0] == LOOP) {
        a->animStep = 0;
        e = a->anim;
    } else if (e[0] == END) {
        a->anim = NULL;
        return END;
    }
    SetFrame(a, e[0]);
    a->animStep++;
    return e[1];
}

static s16 Play(struct Actor *a, const s16 *anim) {
    a->anim = anim;
    a->animStep = 0;
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

/* SetState(); NiDL's 0x0806395C: his next state (+0x14; the one he was set to, ext +0x1C); Restart(): it starts as he's next run
 * (0x08006148 with 0x080A0B10: facing Kirby first); */
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

/* Their sounds (NiDL's own: KatAM's nearest; his steps, 0x201, have none); */
#define SFX_THUD SE_KING_GOLEM_HAND_SLAM        // (0x1F7: landing, the hammer's slam on the ground);
#define SFX_LANDING SE_BONKERS_HAMMER_ATTACK    // (0x1F8: down from his big jump);
#define SFX_EXHALE SE_KIRBY_AIRSHOT             // (0x21A);
#define SFX_INHALE SE_KIRBY_INHALE              // (0x21B);
#define SFX_GULP SE_GOBBLER_EAT_KIRBY           // (0x21C);
#define SFX_SPIT SE_FROSTY_SPIT_KIRBY           // (0x21D);
#define SFX_JUMP SE_BASIC_ENEMY_JUMP            // (0x21E);
#define SFX_PUFF SE_KIRBY_FLOAT_START           // (0x21F: puffing up);
#define SFX_SWING SE_ABILITY_HAMMER_ATTACK      // (0x220);

static void Sound(struct Fight *f, u16 sfx) {
    PlaySfx(&f->obj->base, sfx);
}

// Shake(); NiDL's 0x080261D4: the screen shakes (KatAM's);
static void Shake(struct Fight *f, u16 n) {
    RequestScreenShake(n, &f->obj->base);
}

/* Their objects: Spawn() takes NiDL's lowest free slot (0x08005904), a child of `parent` at its spot or at (x, y) (0x08064CDC: `dx`
 * ahead of its parent by its facing, `dy` down), its script run as it's next run (in its slot's turn: this frame, past its parent's);
 * the spawn's facing its parent's; */
static void Destroy(struct Fight *f, s8 i);

static s8 Spawn(struct Fight *f, u8 kind, s8 parent, u8 variant, s32 x, s32 y) {
    struct Actor *a, *p = &f->a[parent];
    s8 i;

    for (i = A_DEDEDE + 1; i < ACTORS && f->a[i].kind != K_NONE; i++) ;
    if (i >= ACTORS) return NONE;
    a = &f->a[i];
    CpuFill32(0, a, sizeof(struct Actor));
    a->kind = kind;
    a->variant = variant;
    a->facing = p->facing;
    a->flip = 1;
    a->frame = FRAME_HIDDEN;
    a->parent = parent;
    a->child = NONE;
    SetX(a, x);
    SetY(a, y);
    Stop(a);
    return i;
}

static s8 SpawnAt(struct Fight *f, u8 kind, s8 parent, u8 variant, s16 dx, s16 dy) {
    struct Actor *p = &f->a[parent];

    return Spawn(f, kind, parent, variant, p->ix + (p->facing == 1 ? dx : -dx), p->iy + dy);
}

static void Release(struct EffectObject **e) {
    if (*e == NULL) return;
    (*e)->flags |= 0x1000;
    *e = NULL;
}

static void Destroy(struct Fight *f, s8 i) {
    struct Actor *a;

    if (i <= A_DEDEDE || i >= ACTORS) return;
    a = &f->a[i];
    if (a->kind == K_NONE) return;
    Release(&a->sprite);
    if (a->attack) a->attack->flags |= 0x1000;
    a->attack = NULL;
    if (a->enemy && !(a->enemy->base.flags & 0x1000) && !(a->enemy->base.unkC & 0x400)) a->enemy->base.flags |= 0x1000;
    a->enemy = NULL;
    a->kind = K_NONE;
}

// FreeChild(); 0x08005654 on his +0x46;
static void FreeChild(struct Fight *f, struct Actor *a) {
    if (a->child != NONE) Destroy(f, a->child);
    a->child = NONE;
}

/* Solid(); NiDL's terrain: the room's solid tiles (its 16x16 ones; outside it, none); */
static bool32 Solid(struct Fight *f, s32 x, s32 y) {
    struct LevelInfo *level = &gCurLevelInfo[f->unk56];

    if (x < 0 || y < 0 || x >= level->roomWidth || y >= level->roomHeight) return FALSE;
    return !(gCollisionAttributes[GetCollisionTile(f->unk56, x >> 4, y >> 4)] & 1);
}

static void FloorHit(struct Fight *f, struct Actor *a);
static void CeilingHit(struct Fight *f, struct Actor *a);

/* Motion(); NiDL's 0x080656B4: its speed plus its change, capped (across either way, falling), then its spot plus its speed; his
 * terrain (his box: HALF_WIDTH, HEAD, FEET): past the screen's side (SIDE: NiDL's 48 and 239, its camera held), put back on it (the
 * middle of his pixel), stopped (0x080A0588); into the floor falling, on it (landed: 0x080A0538), into the ceiling rising, under it
 * (0x080A0598); on the ground (landed) not rising, the floor within the tile under his feet: on it (NiDL's, traced: lifted 15 pixels
 * standing, back on the floor; 16, landed no more); */
static void Motion(struct Fight *f, struct Actor *a) {
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
    if (a->kind != K_DEDEDE) return;
    if (a->ix < NIDL_SCREEN_X + SIDE) {
        SetX(a, NIDL_SCREEN_X + SIDE);
        a->x |= 0x8000;
        a->vx = 0;
    } else if (a->ix > NIDL_SCREEN_X + DISPLAY_WIDTH - 1 - SIDE) {
        SetX(a, NIDL_SCREEN_X + DISPLAY_WIDTH - 1 - SIDE);
        a->x |= 0x8000;
        a->vx = 0;
    }
    if (a->vy >= 0 && Solid(f, a->ix, a->iy + FEET)) {
        SetY(a, ((a->iy + FEET) & ~15) - FEET - 1);
        a->y |= 0x8000;
        a->landed = TRUE;
        FloorHit(f, a);
    } else if (a->vy < 0 && Solid(f, a->ix, a->iy - HEAD)) {
        SetY(a, ((a->iy - HEAD) | 15) + 1 + HEAD);
        a->y |= 0x8000;
        CeilingHit(f, a);
    } else if (a->landed && a->vy >= 0) {
        if (Solid(f, a->ix, a->iy + FEET + 16)) {
            SetY(a, ((a->iy + FEET + 16) & ~15) - FEET - 1);
            a->y |= 0x8000;
        } else {
            a->landed = FALSE;
        }
    }
}

static void DustPair(struct Fight *f, struct Actor *a, s16 dx, s16 dx2, s16 dy);

// FloorHit(); 0x080A0538: his belly slide's down (state 3: no more falling); puffed up (4), a bounce, dust either way;
static void FloorHit(struct Fight *f, struct Actor *a) {
    if (a->next == 3) {
        StopY(a);
    } else if (a->next == 4) {
        a->v24 = 4;
        a->landed = FALSE;
        DustPair(f, a, 24, 8, 32);
        SetFrame(a, 23);
        VelY(a, -a->vy, -a->ay, a->maxY);
    }
}

// CeilingHit(); 0x080A0598: puffed up, a bounce;
static void CeilingHit(struct Fight *f, struct Actor *a) {
    if (a->next != 4) return;
    a->v24 = 4;
    SetFrame(a, 23);
    VelY(a, -a->vy, -a->ay, a->maxY);
}

/* His moves (NiDL's 0x080A0028 on); */

// OnScreen(); 0x080A060C: his pixel on NiDL's screen (a margin about it);
static bool32 OnScreen(struct Actor *a) {
    s32 sx = a->ix - NIDL_SCREEN_X, sy = a->iy - NIDL_SCREEN_Y;

    return sx >= -19 && sx <= 259 && sy > -20 && sy <= 179;
}

/* Stand(); 0x080A0658 (states 0 and 1): his stand animation (angry: faster) and how long (sStandTimes: by chance, shorter angry;
 * hit, none); off the screen, at once; */
static void Stand(struct Fight *f, struct Actor *a) {
    s16 r;

    if (!OnScreen(a)) {
        a->v24 = Play(a, sStandAngry);
        a->v20 = 1;
        return;
    }
    r = Rand(f, 3);
    a->v24 = Play(a, sStand);
    if (f->obj->unk80 < f->angry) {
        r += 3;
        a->v24 = Play(a, sStandAngry);
    }
    a->v20 = a->v2C ? 0 : sStandTimes[r];
    a->v2C = 0;
}

/* NextMove(); 0x080A06F0: every sixth, puffing up (state 4); else by chance by the state he was set to before (sMoves[]): a walk
 * to a move, his big jump; */
static void NextMove(struct Fight *f, struct Actor *a) {
    const u8 (*moves)[2];
    u16 r;

    if (++a->v18 == 6) {
        a->v18 = 0;
        SetRestart(a, 4);
        return;
    }
    moves = sMoves[a->prevState < 11 ? a->prevState : 0];
    r = Rand(f, 8);
    if (moves == NULL) moves = sMovesA;
    SetState(a, moves[r][0]);
    a->v1C = moves[r][1];
    Restart(a);
}

/* WalkTo(); 0x080A0358: his walk's end by the move it leads to (+0x34: how near; his inhale's spot, 16 short of Kirby, on the screen;
 * his hammer raised, its box); */
static void WalkTo(struct Fight *f, struct Actor *a) {
    struct Kirby *k;
    s32 kx;

    switch (a->v1C) {
    case 3:
        a->v34 = 46;
        break;
    case 7:
        a->child = Spawn(f, K_HAMMER, A_DEDEDE, 0, a->ix, a->iy);
        a->v34 = 52;
        break;
    case 8:
        k = Closest(f, a);
        kx = k ? KirbyX(k) : 0;
        a->v20 = kx;
        if (a->v20 < MIDDLE_X - REACH) a->v20 = MIDDLE_X - REACH;
        if (a->v20 > MIDDLE_X + REACH) a->v20 = MIDDLE_X + REACH;
        if (a->ix == kx) break;
        if (KirbyLeft(f, a)) a->v20 += 16;
        else a->v20 -= 16;
        a->v34 = 8;
        break;
    default:
        a->v34 = 0;
        break;
    }
    if (a->v20 < MIDDLE_X - REACH) a->v20 = MIDDLE_X - REACH;
    if (a->v20 > MIDDLE_X + REACH) a->v20 = MIDDLE_X + REACH;
}

/* HammerSwing(); 0x080A043C: his hammer's move: Kirby high up on the screen, a jump and a slam (+0x24 2); else a swing, every fourth
 * four; */
static void HammerSwing(struct Fight *f, struct Actor *a) {
    struct Kirby *k = Closest(f, a);

    a->v20 = 1;
    if ((k ? KirbyY(k) - NIDL_SCREEN_Y : 0) <= 111) {
        a->v24 = 2;
        return;
    }
    a->v30 = (a->v30 + 1) & 3;
    if (a->v30 == 0) {
        a->v24 = 1;
        a->v20 = 4;
    } else {
        a->v24 = 0;
    }
}

// JumpAim(); 0x080A0480: his big jump at Kirby (far off, or still his way once up: faster angry);
static void JumpAim(struct Fight *f, struct Actor *a, bool32 again) {
    s8 was;

    if (!again) {
        a->v24 = abs(DxKirby(f, a)) > 21;
    } else {
        was = a->facing;
        FaceKirby(f, a);
        a->v24 = was == a->facing;
    }
    if (a->v24 == 1) VelX(a, f->obj->unk80 < f->angry ? 0x22000 : 0x14000, VEL_KEEP);
}

static void Stars(struct Fight *f, struct Actor *a, bool32 pair);
static void Spit(struct Fight *f, struct Actor *a);
static void Catch(struct Fight *f, struct Actor *a);

/* His states (NiDL's 11, 0x0874844C); */

// State0(); His intro (0x080A0B30): standing while the HP bar fills (KatAM's), then a while more (Stand()) and to Kirby (State0's frame);
static void State0(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 0;
    Stop(a);
    a->v24 = Play(a, sStand);
    WAIT(INTRO_TIME);
    a->v2C = 0;
    Stand(f, a);
    SCRIPT_END;
}

// State1(); Standing (0x080A0BB4): a while (Stand()), then his next move (Frame1);
static void State1(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 1;
    Stop(a);
    a->landed = TRUE;
    Stand(f, a);
    SCRIPT_END;
}

/* State2(); His walk (0x080A0C08, 0x080A094C): 1 pixel a frame his way, a step bobbing 5 up and down (13-16; 30-33 his hammer
 * raised), to his move (Frame2); */
static void State2(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 2;
    Stop(a);
    WalkTo(f, a);
    a->v28 = a->v1C == 7 ? 30 : 13;
    for (;;) {
        a->landed = FALSE;
        WAIT(1);
        VelX(a, 0x10000, VEL_KEEP);
        a->vy = 0;
        SetFrame(a, a->v28);
        WAIT(4);
        a->landed = FALSE;
        a->vy = -0x8000;
        SetFrame(a, a->v28 + 1);
        WAIT(10);
        a->vy = 0x8000;
        WAIT(10);
        a->vy = 0;
        SetFrame(a, a->v28 + 2);
        WAIT(4);
        a->landed = FALSE;
        a->vy = -0x8000;
        SetFrame(a, a->v28 + 3);
        WAIT(10);
        a->vy = 0x8000;
        WAIT(10);
    }
    SCRIPT_END;
}

/* State3(); His belly slide (0x080A0CCC): a hop at Kirby (17), down on his belly (18, 19) sliding, slowing, the dust; */
static void State3(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 3;
    Stop(a);
    a->landed = FALSE;
    VelX(a, 0x20000, VEL_KEEP);
    VelY(a, -0x40000, 0x8000, 0x40000);
    SetFrame(a, 17);
    while (!a->landed) WAIT(1);
    Sound(f, SFX_THUD);
    Shake(f, 4);
    a->child = SpawnAt(f, K_SLIDE_DUST, A_DEDEDE, 0, 0, 0);
    if (a->child != NONE) {
        f->a[a->child].v24 = 24;
        f->a[a->child].v20 = 32;
    }
    SpawnAt(f, K_SMALL_DUST, A_DEDEDE, 0, -24, -8);
    VelX(a, 0x14000, VEL_KEEP);
    a->frame++;
    WAIT(10);
    VelX(a, 0x10000, VEL_KEEP);
    WAIT(10);
    VelX(a, 0x8000, VEL_KEEP);
    a->frame++;
    WAIT(10);
    VelX(a, 0x2000, VEL_KEEP);
    WAIT(10);
    Stop(a);
    SetState(a, 1);
    SCRIPT_END;
}

/* State4(); Puffing up (0x080A0E08): his mouth open (25), sucking air (26, 27, 4 times), puffed up (28) rising, then floating (22, 300
 * frames: Frame4) to Kirby's height and side, bouncing off the floor and the ceiling (23); */
static void State4(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 4;
    Stop(a);
    a->v20 = a->v24 = -2;
    SetFrame(a, 25);
    WAIT(20);
    for (a->loop = 0; a->loop <= 3; a->loop++) {
        SetFrame(a, 26);
        WAIT(10);
        a->frame++;
        WAIT(10);
    }
    Sound(f, SFX_PUFF);
    a->landed = FALSE;
    a->frame++;
    for (a->loop = 0; a->loop < 6; a->loop++) {
        a->vy = sRise[a->loop][0];
        WAIT(sRise[a->loop][1]);
    }
    Stop(a);
    SetFrame(a, 22);
    a->maxY = a->maxX = 0x18000;
    a->v20 = 300;
    for (;;) {
        WAIT(12);
        FaceKirby(f, a);
    }
    SCRIPT_END;
}

/* State5(); Letting the air out (0x080A0F7C): his breath (29) at Kirby, pushed back, then down to the floor (43), crouched (41); */
static void State5(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 5;
    Stop(a);
    a->landed = FALSE;
    SetFrame(a, 22);
    SetFrame(a, 29);
    Sound(f, SFX_EXHALE);
    a->child = SpawnAt(f, K_BREATH, A_DEDEDE, 0, 32, 16);
    for (a->loop = 0; a->loop < 4; a->loop++) {
        VelX(a, sPushBack[a->loop], VEL_KEEP);
        WAIT(2);
    }
    StopX(a);
    SetFrame(a, 43);
    VelY(a, 0, 0x2500, 0x30000);
    while (!a->landed) WAIT(1);
    Stop(a);
    SetFrame(a, 41);
    WAIT(10);
    SetState(a, 1);
    SCRIPT_END;
}

/* State6(); His big jump (0x080A1058): crouched (41), up (42) at Kirby far off, down (43), still his way at the top; landing, the
 * screen shakes, stars burst out either way; */
static void State6(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 6;
    Stop(a);
    SetFrame(a, 41);
    WAIT(24);
    a->landed = FALSE;
    JumpAim(f, a, FALSE);
    a->frame++;
    Sound(f, SFX_JUMP);
    VelY(a, -0x50000, 0x2800, 0x50000);
    while (a->vy < 0) WAIT(1);
    Stop(a);
    a->frame++;
    a->ay = 0x2800;
    a->maxY = 0x50000;
    if (a->v24) JumpAim(f, a, TRUE);
    while (!a->landed) WAIT(1);
    Stop(a);
    Sound(f, SFX_LANDING);
    Shake(f, 2);
    Stars(f, a, TRUE);
    SetFrame(a, 41);
    WAIT(34);
    SetState(a, 1);
    SCRIPT_END;
}

/* State7(); His hammer (0x080A1168): Kirby high up, a jump and a slam (0x080A0768: 37, 38 up, 36 35 34 down); else raised (39), swung
 * down (36, 35, the slam 34: the screen shakes, a star), every fourth swing four (33 back up between); */
static void State7(struct Fight *f, struct Actor *a) {
    const u16 *t;

    SCRIPT;
    a->state = 7;
    Stop(a);
    HammerSwing(f, a);
    if (a->v24 == 2) {
        SetFrame(a, 37);
        WAIT(24);
        a->landed = FALSE;
        a->frame++;
        Sound(f, SFX_JUMP);
        VelY(a, -0x50000, 0x2800, 0x50000);
        while (a->vy < 0) WAIT(1);
        Stop(a);
        Sound(f, SFX_SWING);
        SetFrame(a, 36);
        WAIT(4);
        a->frame--;
        WAIT(1);
        a->frame--;
        a->ay = 0x2800;
        a->maxY = 0x50000;
        while (!a->landed) WAIT(1);
        Stop(a);
        Sound(f, SFX_THUD);
        Stars(f, a, FALSE);
        Shake(f, 4);
        WAIT(20);
    } else {
        SetFrame(a, 39);
        WAIT(24);
        Sound(f, SFX_SWING);
        SetFrame(a, 36);
        t = &sSwings[a->v24 * 3];
        WAIT(t[0]);
        t = &sSwings[a->v24 * 3];
        a->frame--;
        WAIT(t[1]);
        t = &sSwings[a->v24 * 3];
        Shake(f, t[3]);
        Stars(f, a, FALSE);
        a->frame--;
        WAIT(t[2]);
        a->v20--;
        for (a->loop = 0; a->loop < a->v20; a->loop++) {
            a->frame--;
            WAIT(6);
            Sound(f, SFX_SWING);
            a->frame = 36;
            t = &sSwings[a->v24 * 3];
            WAIT(t[0]);
            t = &sSwings[a->v24 * 3];
            a->frame--;
            WAIT(t[1]);
            t = &sSwings[a->v24 * 3];
            a->frame--;
            WAIT(t[2]);
        }
    }
    SetState(a, 1);
    SCRIPT_END;
}

/* State8(); His inhale (0x080A11E0): his mouth opening (24, 25), then sucking (26, 27) up to 90 frames, the sparkles flying in
 * (Frame8); a Kirby caught (his inhale's box: Catch()) drawn in, he shuts his mouth puffed up (28), shaking, and spits him out (9); */
static void State8(struct Fight *f, struct Actor *a) {
    u8 i;

    SCRIPT;
    a->state = 8;
    Stop(a);
    a->v20 = 0;
    SetFrame(a, 24);
    WAIT(8);
    a->frame++;
    WAIT(20);
    // (0x080A0098: the sparkles, his inhale's box 38 ahead of him and 10 down);
    for (i = 0; i < 3; i++) f->sparkX[i] = f->sparkY[i] = f->sparkV[i] = 0;
    a->v20 = 1;
    a->v34 = 90;
    f->caught = -1;
    a->child = SpawnAt(f, K_CATCH, A_DEDEDE, 0, 38, 10);
    Sound(f, SFX_INHALE);
    while (f->caught != 1) {
        SetFrame(a, 26);
        WAIT(4);
        a->frame++;
        WAIT(6);
    }
    a->v20 = 0;
    FreeChild(f, a);
    Sound(f, SFX_GULP);
    SetFrame(a, 28);
    // (0x080063AC: his pixel nudged his way, there till the next: back 2, on 2);
    for (a->loop = 0; a->loop <= 3; a->loop++) {
        a->dx -= 2 * a->facing;
        WAIT(2);
        a->dx += 2 * a->facing;
        WAIT(2);
    }
    WAIT(14);
    SetState(a, 9);
    SCRIPT_END;
}

// State9(); His spit (0x080A13D4): turned to the screen's middle (29), the Kirbys he swallowed out in a star (Spit());
static void State9(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 9;
    Stop(a);
    a->facing = a->ix - NIDL_SCREEN_X <= 127 ? 1 : -1;
    SetFrame(a, 29);
    Spit(f, a);
    Sound(f, SFX_SPIT);
    WAIT(51);
    SetState(a, 1);
    SCRIPT_END;
}

// State10(); Down from a hit in the air (0x080A146C): falling (43), crouched (41); the state before kept for his next move;
static void State10(struct Fight *f, struct Actor *a) {
    u8 was;

    SCRIPT;
    a->state = 10;
    Stop(a);
    SetFrame(a, 43);
    VelY(a, 0, 0x2500, 0x30000);
    while (!a->landed) WAIT(1);
    Stop(a);
    SetFrame(a, 41);
    WAIT(10);
    was = a->prevState;
    SetState(a, 1);
    a->prevState = was;
    SCRIPT_END;
}

static void (*const sStates[11])(struct Fight *, struct Actor *) = {
    State0, State1, State2, State3, State4, State5, State6, State7, State8, State9, State10,
};

static void Sparkles(struct Fight *f, struct Actor *a);
static void DrawIn(struct Fight *f);

/* DededeFrame(); His state's function each frame (NiDL's 0x08748478, after his script and his move): */
static void DededeFrame(struct Fight *f, struct Actor *a) {
    s32 d;

    switch (a->state) {
    case 0:
        // (0x080A0B74: his stand animation; his while out, to Kirby for his belly slide);
        a->v24 = AnimTickFacing(f, a, a->v24);
        if (--a->v20 == 0) {
            a->v1C = 3;
            SetRestart(a, 2);
        }
        break;
    case 1:
        // (0x080A0BDC: his while out, his next move);
        a->v24 = AnimTickFacing(f, a, a->v24);
        if (a->v20 == 0) NextMove(f, a);
        else a->v20--;
        break;
    case 2:
        // (0x080A0C28: near enough to Kirby (his inhale's spot) his way, or Kirby behind him, his move);
        d = a->v1C == 8 ? a->v20 - a->ix : DxKirby(f, a);
        if ((d < 0 && a->facing != -1) || (d >= 0 && a->facing != 1)) d = 0;
        if (abs(d) <= a->v34 && a->v1C != 11) {
            a->landed = TRUE;
            SetRestart(a, a->v1C);
        }
        break;
    case 3: case 7:
        if (a->next != a->state) {
            FreeChild(f, a);
            Restart(a);
        }
        break;
    case 4:
        // (0x080A0EC8: his float's while; the bounce's frame back to 22; up or down to Kirby's height, to his side);
        if (a->v20 == -2) break;
        if (a->v20 == 0) {
            SetRestart(a, 5);
            break;
        }
        a->v20--;
        if (a->v24 != -2 && a->v24-- <= 0) {
            SetFrame(a, 22);
            a->v24 = -2;
        }
        a->ay = KirbyAbove(f, a) ? -0x500 : 0x500;
        a->ax = KirbyLeft(f, a) ? -0x800 : 0x800;
        SetFrame(a, a->frame);
        break;
    case 8:
        // (0x080A12E0: hit, flashing a while; sucking, the sparkles, a Kirby caught: his box gone; none in 90 frames, standing);
        if (f->flashing) {
            if (f->hurtWait <= 0) f->flashing = FALSE;
            else f->hurtWait--;
        }
        if (a->v20) {
            if (f->catchKirby != NONE) FreeChild(f, a);
            Sparkles(f, a);
            if (f->caught == -1) {
                if (a->v34 <= 0) {
                    FreeChild(f, a);
                    SetRestart(a, 1);
                } else {
                    a->v34--;
                }
            }
        }
        if (a->next != 8) {
            f->flashing = FALSE;
            Restart(a);
        }
        break;
    case 9:
        if (f->flashing) {
            if (f->hurtWait <= 0) f->flashing = FALSE;
            else f->hurtWait--;
        }
        if (a->next != 9) {
            if (f->hurtWait > 0) f->flashing = FALSE;
            Restart(a);
        }
        break;
    default:
        // (5, 6, 10: their next state, at once);
        if (a->next != a->state) Restart(a);
        break;
    }
}

/* His children's scripts (NiDL's: each its object's); */

// StarScript(); 0x08067258 (type 0x8B, 0): a star out of his landing or his slam: 13, 12, 11 rising 1.5, 2 across; slower (0.5),
// still, 8 4 10 5 9 6 10 7 four times; 11 12 13, gone;
static void StarScript(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 0;
    a->landed = FALSE;
    a->v24 = 2;
    VelX(a, 0x20000, VEL_KEEP);
    a->vy = -0x18000;
    a->frame = 13;
    WAIT(2);
    a->frame--;
    WAIT(2);
    a->frame--;
    WAIT(2);
    VelX(a, 0x8000, VEL_KEEP);
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

/* SlideDustScript(); 0x0806D22C (type 0x93): his belly slide's dust, three puffs from behind him (24 behind, 32 down, where he is
 * then), drifting back and up, its frames by pairs (0x0800617C); the first a puff of its own halfway (0x90); */
static void SlideDustScript(struct Fight *f, struct Actor *a) {
    struct Actor *p = &f->a[A_DEDEDE];
    s8 i;

    SCRIPT;
    SetX(a, p->ix - a->facing * a->v24);
    SetY(a, p->iy + a->v20);
    VelX(a, -0x30000, 0);
    a->vy = 0;
    a->ay = -0x2000;
    SetFrameBy(a, 0);
    WAIT(2);
    a->frame += 2;
    WAIT(2);
    a->frame += 2;
    WAIT(1);
    i = Spawn(f, K_PUFF_DUST, a - f->a, 0, a->ix, a->iy);
    if (i != NONE) f->a[i].facing = a->facing;
    SetFrameBy(a, 4);
    WAIT(1);
    a->frame -= 2;
    WAIT(2);
    a->frame -= 2;
    WAIT(1);
    a->frame = FRAME_HIDDEN;
    WAIT(1);
    Stop(a);
    p = &f->a[A_DEDEDE];
    SetX(a, p->ix - a->facing * a->v24);
    SetY(a, p->iy + a->v20);
    VelX(a, -0x24000, 0x1000);
    a->vy = -0x4000;
    a->ay = -0x2000;
    SetFrameBy(a, 0);
    WAIT(2);
    a->frame += 2;
    WAIT(2);
    a->frame += 2;
    WAIT(2);
    a->frame -= 2;
    WAIT(2);
    a->frame -= 2;
    WAIT(1);
    a->frame = FRAME_HIDDEN;
    WAIT(1);
    Stop(a);
    p = &f->a[A_DEDEDE];
    SetX(a, p->ix - a->facing * a->v24);
    SetY(a, p->iy + a->v20);
    VelX(a, -0x12000, 0x1800);
    a->vy = -0x4000;
    a->ay = -0x2000;
    SetFrameBy(a, 0);
    WAIT(2);
    a->frame += 2;
    WAIT(2);
    SetFrameBy(a, 6);
    WAIT(2);
    a->frame += 2;
    WAIT(1);
    Destroy(f, a - f->a);
    return;
    SCRIPT_END;
}

// PuffDustScript(); 0x0806CEB8 (type 0x90): a small puff near its parent (up to 8 behind and up, by chance), rising, slowing back;
static void PuffDustScript(struct Fight *f, struct Actor *a) {
    SCRIPT;
    SetX(a, a->ix + RandOffset(f, a, -8, 1, 8));
    SetY(a, a->iy + RandOffset(f, NULL, -8, 1, 8));
    VelX(a, VEL_KEEP, 0x4000);
    a->ay = -0x4000;
    SetFrameBy(a, 4);
    WAIT(2);
    a->frame -= 2;
    WAIT(2);
    a->frame -= 2;
    WAIT(1);
    Destroy(f, a - f->a);
    return;
    SCRIPT_END;
}

// DustScript(); 0x0806D148 (type 0x92): his bounce's dust off one way: 1, 3, 7, 9 (2 frames each, 1 the last), slowing, rising;
static void DustScript(struct Fight *f, struct Actor *a) {
    SCRIPT;
    if (a->facing != 1 && a->facing != -1) a->facing = 1;
    VelX(a, 0x24000, -0x1800);
    a->vy = -0x4000;
    a->ay = -0x2000;
    SetFrame(a, 1);
    WAIT(2);
    a->frame += 2;
    WAIT(2);
    SetFrame(a, 7);
    WAIT(2);
    a->frame += 2;
    WAIT(1);
    Destroy(f, a - f->a);
    return;
    SCRIPT_END;
}

// SmallDustScript(); 0x080A5324 (type 0xB2): his slide's dust off his back: 2 back and up 3 frames, still 6, gone;
static void SmallDustScript(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->frame = 0;
    VelX(a, -0x20000, VEL_KEEP);
    a->vy = -0x20000;
    WAIT(3);
    Stop(a);
    WAIT(6);
    Destroy(f, a - f->a);
    return;
    SCRIPT_END;
}

// BreathScript(); 0x080A4C3C (type 0x71): his breath, off his way, slowing (4, 2, 1, 0.5, 0.125 a frame, 6 frames each), gone;
static void BreathScript(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 0;
    a->landed = FALSE;
    SetFrame(a, 0);
    for (a->loop = 0; a->loop < 5; a->loop++) {
        VelX(a, sBreath[a->loop], VEL_KEEP);
        WAIT(6);
    }
    Destroy(f, a - f->a);
    return;
    SCRIPT_END;
}

// HitScript(); 0x0806CB64 (type 0x8E): his hit stars, 0 and 1 by turns (2 frames each) 8 times about him (HitFrame());
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

static void ChildScript(struct Fight *f, struct Actor *a) {
    switch (a->kind) {
    case K_STAR: StarScript(f, a); break;
    case K_SLIDE_DUST: SlideDustScript(f, a); break;
    case K_PUFF_DUST: PuffDustScript(f, a); break;
    case K_DUST: DustScript(f, a); break;
    case K_SMALL_DUST: SmallDustScript(f, a); break;
    case K_BREATH: BreathScript(f, a); break;
    case K_HIT: HitScript(f, a); break;
    default: a->pc = PC_END; break;
    }
}

/* DustPair(); 0x0806D08C: his bounce's dust, `dx` ahead his way and `dx2` behind the other, `dy` down; */
static void DustPair(struct Fight *f, struct Actor *a, s16 dx, s16 dx2, s16 dy) {
    s8 i = Spawn(f, K_DUST, A_DEDEDE, 0, a->ix + a->facing * dx, a->iy + dy);

    if (i != NONE) f->a[i].facing = a->facing;
    i = Spawn(f, K_DUST, A_DEDEDE, 0, a->ix - a->facing * dx2, a->iy + dy);
    if (i != NONE) f->a[i].facing = -a->facing;
}

/* Stars(); 0x080A02D4: his landing's pair (32 either side, each its way) or his slam's one (32 ahead, his way), 24 down (0x08067120:
 * type 0x8B, 0: StarScript(); KatAM's star enemy with it, enemies/star.c: inhaled, it's Kirby's); */
static void Star(struct Fight *f, s16 x, s16 y, s8 facing) {
    struct Object *obj = f->obj, *star;
    struct Actor *a;
    s8 i = Spawn(f, K_STAR, A_DEDEDE, 0, x, y);

    if (i == NONE) return;
    a = &f->a[i];
    if (facing) a->facing = facing;
    star = CreateObjTemplateAndObj(obj->base.unk56, 1, 0x24, x, y, 0, 0x1F, 0, 0, NIDL_TYPE, NIDL_SPECIAL << 4 | 2, 0, 0, 0, 0, 0, 0, 0,
                                   0, 0, 0, 0, 0);
    // (KatAM's despawn off the screen not its while the fight drives it);
    if (star) star->base.flags |= 0x2000000;
    a->enemy = star;
}

static void Stars(struct Fight *f, struct Actor *a, bool32 pair) {
    s16 y = a->iy + 24;

    if (pair) {
        Star(f, a->ix + 32, y, 1);
        Star(f, a->ix - 32, y, -1);
    } else {
        Star(f, a->ix + a->facing * 32, y, 0);
    }
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
    NidlSetFrame((struct NidlObject *)star, STAR_FRAMES + a->frame - 4);
    star->base.sprite.palId = f->pal[PAL_COMMON];
    star->base.flags = (star->base.flags & ~1) | (a->flip == -1 ? 0 : 1);
}

/* Catch(); His inhale's box (type 0xB7, 0x080A54A4: 0x08748C44, under 20 either side, under 14 up and down: NiDL's, traced, a Kirby 19
 * pixels from its middle caught, 20 not) on a Kirby's box (KatAM's: its half sizes, base.unk3A and unk3B): caught, into KatAM's grab
 * (as Gobbler's catch: sub_080E588C; the grab's start, sub_0804E09C, refuses a Kirby hurt, dead or busy, and clears its script then),
 * the player's and every helper there, drawn into his mouth (DrawIn()); */
#define CATCH_W 20
#define CATCH_H 14

/* The grab's scripts (struct Kirby_110, kirby.c's sub_0804E60C: x ahead of him, y, priority, frames, Kirby's animation, flags: 0x400
 * held, 0x40 hidden, 0x200 left where he is, 0x100 back to the entry its frames count names): drawn into his mouth (DrawIn() moves
 * him), then hidden inside him. A Kirby's script restarts when its holder's unk83 (his frame) changes: one entry, held, looping; */
static const struct Kirby_110 sDrawnIn[] = {
    { 0, 0, -1, 1, 0x4A, 0x600 },
    { 0, 0, 0, 0, 0, 0x500 },
};
static const struct Kirby_110 sSwallowed[] = {
    { MOUTH_X, MOUTH_Y, -1, 1, 0x4A, 0x440 },
    { 0, 0, 0, 0, 0, 0x500 },
};

static void Catch(struct Fight *f, struct Actor *a) {
    struct Object *obj = f->obj;
    s32 dx, dy;
    u8 i;

    for (i = 0; i < gNumKirbys; i++) {
        struct Kirby *kirby = &gKirbys[i];

        // (not one untouchable, or blinking after a hit (KatAM's sub_080880AC: its collision class 6): NiDL's box checks pass it by);
        if (!InRoom(f, kirby) || kirby->unk110 || (kirby->base.flags & 0x200) || (kirby->base.unk5C & 7) == 6) continue;
        dx = KirbyX(kirby) - a->ix;
        dy = KirbyY(kirby) - a->iy;
        if (abs(dx) >= CATCH_W + kirby->base.unk3A || abs(dy) >= CATCH_H + kirby->base.unk3B) continue;
        kirby->unk110 = sDrawnIn;
        kirby->base.unk6C = obj;
        kirby->base.flags |= 0x40000;
        sub_0804E09C(kirby);
        if (!kirby->unk110) continue;
        f->swallowed |= 1 << i;
        if (f->catchKirby == NONE) f->catchKirby = i;
    }
    if (f->swallowed && f->caught == -1) {
        f->caught = 0;
        f->pull = PULL_TIME;
    }
}

// Inside(); Whether a Kirby he caught is still his (not let go: hurt again, or he was beaten);
static bool32 Inside(struct Fight *f, u8 i) {
    return (f->swallowed >> i & 1) && gKirbys[i].unk110 && gKirbys[i].base.unk6C == f->obj;
}

// DrawIn(); The Kirbys he caught drawn into his mouth (a frame of PULL_TIME), then hidden inside him: swallowed (0x02007D00 1);
static void DrawIn(struct Fight *f) {
    struct Actor *a = &f->a[A_DEDEDE];
    s32 mouthX = (a->ix + a->facing * MOUTH_X) << 8, mouthY = (a->iy + MOUTH_Y) << 8;
    // (256 / the frames left: no division in this code);
    static const u8 sStep[PULL_TIME + 1] = { 0, 0, 128, 85, 64, 51, 43 };
    u8 i;

    if (f->caught != 0) return;
    // (none inside him any more (KatAM's: hurt as he's drawn in): sucking again, his while on);
    for (i = 0; i < gNumKirbys && !Inside(f, i); i++) ;
    if (i == gNumKirbys) {
        f->caught = -1;
        f->catchKirby = NONE;
        f->swallowed = 0;
        return;
    }
    for (i = 0; i < gNumKirbys; i++) {
        struct Kirby *kirby = &gKirbys[i];

        if (!Inside(f, i)) continue;
        if (f->pull <= 1) {
            kirby->unk110 = sSwallowed;
            kirby->base.counter = ~f->obj->unk83;
            continue;
        }
        kirby->base.x += ((mouthX - kirby->base.x) * sStep[f->pull]) >> 8;
        kirby->base.y += ((mouthY - kirby->base.y) * sStep[f->pull]) >> 8;
    }
    if (f->pull <= 1) f->caught = 1;
    else f->pull--;
}

/* Spit(); 0x080A0028's star (type 0x70: enemies/spit_star.c), 32 ahead of him and 16 down, with the Kirbys inside him, off his way; */
static void Spit(struct Fight *f, struct Actor *a) {
    struct Object *obj = f->obj, *star;
    u8 i, kirbys = 0;

    for (i = 0; i < gNumKirbys; i++)
        if (Inside(f, i)) kirbys |= 1 << i;
    f->swallowed = 0;
    if (!kirbys) return;
    star = CreateObjTemplateAndObj(obj->base.unk56, 1, 0x24, a->ix + a->facing * 32, a->iy + 16, 0, 0x1F, 0, 0, NIDL_TYPE,
                                   NIDL_SPECIAL << 4 | 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    if (star == NULL) return;
    star->base.parent = obj;
    SpitStarCarry(star, kirbys, a->facing == -1);
}

/* Sparkles(); 0x080A00EC: his inhale's three sparkles, each from a spot ahead of him by chance (sSparkleX: 20 more out, sSparkleY),
 * drawn into his mouth faster and faster (0x6000 a frame), its height following in as it nears; in, from another; drawn 16 lower; */
static void Sparkles(struct Fight *f, struct Actor *a) {
    s32 r, ax, sh, dy;
    u8 i;

    for (i = 0; i < 3; i++) {
        if (f->sparkX[i] == 0) {
            f->sparkV[i] = 0;
            r = Rand(f, 16);
            f->sparkX[i] = (sSparkleX[r] + 20) << 16;
            if (a->facing != 1) f->sparkX[i] = -f->sparkX[i];
            f->sparkY[i] = sSparkleY[r] << 16;
        }
        if (abs(f->sparkX[i]) <= 28 << 16) {
            f->sparkX[i] = 0;
            continue;
        }
        f->sparkV[i] += f->sparkX[i] > 0 ? -0x6000 : 0x6000;
        f->sparkX[i] += f->sparkV[i];
        ax = abs(f->sparkX[i]);
        sh = (ax >> 20) + 2;
        dy = ((u32)(abs(f->sparkY[i]) & 0xFFFF0000)) >> sh;
        if (f->sparkY[i] > 0) dy = -dy;
        f->sparkY[i] += dy;
    }
}

/* Hurt (NiDL's 0x0809FCB4, its handler 0x0809FE10 and his hurt 0x080A0A84): his own script held (NiDL's 0x080055B0 7: his +0x13,
 * his script, his main, his per-frame function skipped; the room, the Kirbys, his children go on), him in his hurt frame (8)
 * flashing in his hurt colours (2 frames in 4, by the clock: Colors()), jittering across (sJitter: 1 pixel each way, 15 frames), his
 * hit stars; out, back where he was, in the frame his hit gave him; */
static void HurtStep(struct Fight *f) {
    struct Actor *a = &f->a[A_DEDEDE];

    if (f->hurtWait <= 0) {
        if (f->hurtStep <= 11) {
            f->hurtWait = sJitter[f->hurtStep + 3];
            f->hurtStep += 2;
        } else {
            f->hurtStep = 0;
            f->hurtWait = sJitter[1];
            if (--f->hurtLoops <= 0) {
                // (0x0809FD20);
                f->hooked = FALSE;
                f->flashing = FALSE;
                a->ix = f->hurtX;
                a->iy = f->hurtY;
                a->frame = f->hurtFrame;
                return;
            }
        }
    }
    f->hurtWait--;
    a->ix += sJitter[f->hurtStep];
}

static void HurtStart(struct Fight *f) {
    struct Actor *a = &f->a[A_DEDEDE];
    s8 i;

    Shake(f, 4);
    f->hooked = TRUE;
    f->hurtX = a->ix;
    f->hurtY = a->iy;
    f->hurtFrame = a->frame;
    f->flashing = TRUE;
    SetFrame(a, 8);
    f->hurtLoops = 1;
    f->hurtStep = -2;
    f->hurtWait = 0;
    i = Spawn(f, K_HIT, A_DEDEDE, 0, a->ix, a->iy);
    if (i != NONE) f->a[i].v18 = f->a[i].v1C = 0;
}

/* Hit(); A hit took HP off him (KatAM's, between frames; NiDL's from Kirby's turn, before his): no more for GUARD_TIME frames; hurt
 * (HurtStart(), its first step in his own slot's turn); by his state: standing or walking, standing again at once (+0x2C: no while; a walk's move his next's choice); in the air (his belly
 * slide's hop, puffed up, his slam's jump), down (10); his breath, his big jump: on; inhaling or spitting, not held: flashing 14
 * frames only; his state's function not run this frame (0x080692FC); */
static void Hit(struct Fight *f) {
    struct Actor *a = &f->a[A_DEDEDE];
    s16 frame;

    f->hit = TRUE;
    f->guard = GUARD_TIME;
    HurtStart(f);
    switch (a->next) {
    case 0:
        a->next = 1;
        // (fall through)
    case 1:
        a->v2C = 1;
        f->hurtFrame = 4;
        SetState(a, 1);
        Restart(a);
        break;
    case 2:
        a->v2C = 1;
        f->hurtFrame = 4;
        SetState(a, 1);
        a->prevState = a->v1C;
        Restart(a);
        break;
    case 3: case 4:
        if (!a->landed) {
            f->hurtFrame = 43;
            SetState(a, 10);
        } else {
            a->v2C = 1;
            f->hurtFrame = 4;
            SetState(a, 1);
        }
        Restart(a);
        break;
    case 7:
        if (a->v24 == 2) {
            f->hurtFrame = 43;
            SetState(a, 10);
        } else {
            a->v2C = 1;
            f->hurtFrame = 4;
            SetState(a, 1);
        }
        Restart(a);
        break;
    case 8: case 9:
        // (0x080055B0 0: his script goes on);
        f->hooked = FALSE;
        frame = f->hurtFrame;
        a->frame = frame;
        f->hurtWait = 14;
        return;
    }
}

/* Process(); An Actor's frame (in NiDL's order, its slot's): its script (its next state's, restarted: his facing Kirby first), its
 * move, its frame's function; */
static void HitFrame(struct Fight *f, struct Actor *a);
static void HammerFrame(struct Fight *f, struct Actor *a);

static void Process(struct Fight *f, s8 i) {
    struct Actor *a = &f->a[i];

    if (a->kind == K_NONE) return;
    if (a->kind == K_DEDEDE) {
        // (hurt: his script, his move, his frame's function held; his hurt's step after his children's, KingDededeMain());
        if (f->hooked) return;
        if (a->restart) {
            a->restart = FALSE;
            FaceKirby(f, a);
            a->script = a->next;
            a->pc = 0;
            a->wait = 0;
        }
        if (a->pc != PC_END && (!a->wait || !--a->wait)) sStates[a->script < 11 ? a->script : 1](f, a);
        Motion(f, a);
        if (!f->hit) DededeFrame(f, a);
        return;
    }
    if (a->pc != PC_END && (!a->wait || !--a->wait)) ChildScript(f, a);
    if (a->kind == K_NONE) return;
    if (a->kind != K_HIT) Motion(f, a);
    switch (a->kind) {
    case K_HIT:
        HitFrame(f, a);
        break;
    case K_HAMMER:
        HammerFrame(f, a);
        break;
    case K_CATCH:
        // (0x080A54A4: his box where it was made, 38 ahead of him and 10 down);
        Catch(f, a);
        break;
    }
}

/* HitFrame(); 0x0806CBD4: his hit stars on him (his pixel: jittering; as the frame started, before his own turn's move: NiDL's,
 * traced), about him by turns; */
static void HitFrame(struct Fight *f, struct Actor *a) {
    u8 k = a->loop & 7;

    a->ix = f->lastX + a->v18 + sHitX[k] * a->facing;
    a->iy = f->lastY + a->v1C + sHitY[k];
}

/* HammerFrame(); 0x080A53A8 (type 0xB6): his hammer's box by his frame (0x08748C0C, 0x08748C28): raised walking (30-33, behind his
 * head), coming down (35), on the ground (34); none else; */
static void HammerFrame(struct Fight *f, struct Actor *a) {
    struct Actor *p = &f->a[A_DEDEDE];
    s16 dx, dy;

    a->v18 = 0;
    if (p->frame >= 30 && p->frame <= 33) {
        dx = -20;
        dy = -30;
        a->v18 = 1;
    } else if (p->frame == 34) {
        dx = 40;
        dy = 16;
        a->v18 = 1;
    } else if (p->frame == 35) {
        dx = 40;
        dy = 8;
        a->v18 = 2;
    } else {
        return;
    }
    a->ix = p->ix + p->facing * dx;
    a->iy = p->iy + dy;
}

/* Their boxes hurting Kirby (attack objects, as Meta Knight's Galaxia: NiDL's 0x08068CF8, 0x08068E04): his hammer's (9 either way;
 * coming down, 10 across and 18 up and down), his breath's (8 either way); */
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
    case K_HAMMER:
        on = a->v18 != 0;
        if (a->v18 == 2) {
            box[0] = -10; box[1] = -18; box[2] = 10; box[3] = 18;
        } else {
            box[0] = box[1] = -9; box[2] = box[3] = 9;
        }
        break;
    case K_BREATH:
        box[0] = box[1] = -8; box[2] = box[3] = 8;
        on = TRUE;
        break;
    default:
        on = FALSE;
        break;
    }
    if (!on || f->over) {
        if (a->attack) a->attack->flags |= 0x200;
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
    }
    b = a->attack;
    sub_0803E2B0(b, box[0], box[1], box[2], box[3]);
    b->x = a->ix << 8;
    b->y = a->iy << 8;
    b->flags &= ~0x200;
}

/* His box (NiDL's 0x087487E8: -20, -14 to 24, 34 facing right) on KatAM's object, where he is: hit, touching hurts; */
static void Body(struct Fight *f) {
    struct Object *obj = f->obj;
    struct Actor *a = &f->a[A_DEDEDE];

    obj->base.x = a->ix << 8;
    obj->base.y = a->iy << 8;
    if (a->facing == 1) sub_0803E2B0(&obj->base, -20, -14, 24, 34);
    else sub_0803E2B0(&obj->base, -24, -14, 20, 34);
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
    EffectObjectInitSprite(e, &e->sprite, tiles, gNidlData.bosses[NIDL_KING_DEDEDE].animId, frame, order);
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

static u8 Palette(u16 room, u16 key, u8 frame) {
    u8 palId = sub_0803DF24(key);

    if (palId == 0xFF) {
        if (gKirbys[gLocalPlayerId].base.roomId != room) return 0;
        sub_0803DFAC(key, frame);
        palId = sub_0803DF24(key);
    }
    return palId == 0xFF ? 0 : palId;
}

static void Palettes(u16 room, u8 *pal) {
    u16 animId = gNidlData.bosses[NIDL_KING_DEDEDE].animId;

    if (!pal[PAL_BODY]) pal[PAL_BODY] = Palette(room, animId, BODY(4));
    if (!pal[PAL_TRIM]) pal[PAL_TRIM] = Palette(room, animId | KEY_TRIM, gNidlKingDedede.back[4]);
    if (!pal[PAL_COMMON]) pal[PAL_COMMON] = Palette(room, animId | KEY_COMMON, gNidlKingDedede.fx[FX_DUST]);
    if (!pal[PAL_BREATH]) pal[PAL_BREATH] = Palette(room, animId | KEY_BREATH, gNidlKingDedede.fx[FX_BREATH]);
}

// Colors(); His body's palette this frame: his hurt flash's colours (NiDL's 0x080663F4: the clock's 2 frames in 4), else his own;
static void Colors(u16 room, u8 palId, bool32 flash) {
    const u16 *colors = flash ? gNidlKingDedede.flash : gNidlKingDedede.body;
    u16 *slot;
    u8 i;

    if (gKirbys[gLocalPlayerId].base.roomId != room || !palId) return;
    slot = &gObjPalette[palId * 16];
    for (i = 1; i < 16 && slot[i] == colors[i]; i++) ;
    if (i == 16) return;
    for (i = 0; i < 16; i++) slot[i] = colors[i];
    sub_0803D280(palId * 16, 16);
    gMainFlags |= MAIN_FLAG_OBJ_PALETTE_SYNC_ENABLE;
}

// DrawDedede(); His three sprites at (x, y) (8.8) in NiDL's frame `frame`: his body, his second palette's pieces in front, behind;
static void DrawDedede(struct EffectObject **parts, u16 room, const u8 *pal, s32 x, s32 y, s16 frame, s8 flip) {
    static const u8 orders[3] = { ORDER_BODY, ORDER_FRONT, ORDER_BACK };
    static const u8 groups[3] = { G_BODY, G_FRONT, G_BACK };
    u8 i, k;

    for (i = 0; i < 3; i++) {
        if (frame < 4 || frame > 43) k = FRAME_NONE;
        else if (i == 0) k = BODY(frame);
        else k = i == 1 ? gNidlKingDedede.front[frame] : gNidlKingDedede.back[frame];
        if (k == FRAME_NONE) {
            Hide(parts[i]);
            continue;
        }
        if (parts[i] == NULL) parts[i] = Sprite(room, gNidlKingDedede.tiles[groups[i]], k, orders[i]);
        Draw(parts[i], x, y, k, pal[i == 0 ? PAL_BODY : PAL_TRIM], flip, orders[i]);
    }
}

/* Look(); A child's KatAM frame (FRAME_HIDDEN none) and palette; */
static s16 Look(struct Actor *a, u8 *pal) {
    const u8 *fx = gNidlKingDedede.fx;

    if (a->frame < 0) return FRAME_HIDDEN;
    *pal = PAL_COMMON;
    switch (a->kind) {
    case K_DUST: case K_SLIDE_DUST: case K_PUFF_DUST:
        return fx[FX_DUST] + a->frame;
    case K_SMALL_DUST:
        return fx[FX_SMALL_DUST];
    case K_HIT:
        return fx[FX_HIT] + (a->frame & 1);
    case K_BREATH:
        *pal = PAL_BREATH;
        return fx[FX_BREATH];
    }
    return FRAME_HIDDEN;
}

static void DrawActor(struct Fight *f, struct Actor *a) {
    u8 pal = 0;
    s16 k = Look(a, &pal);

    if (k == FRAME_HIDDEN || !Watched(f)) {
        Hide(a->sprite);
        return;
    }
    if (a->sprite == NULL) a->sprite = Sprite(f->room, gNidlKingDedede.tiles[G_FX], k, ORDER_FX);
    Draw(a->sprite, a->ix << 8, a->iy << 8, k, f->pal[pal], a->flip, ORDER_FX);
}

// DrawSparkles(); His inhale's sparkles (0x080A00EC: NiDL's shared effects' 45), 16 lower than their spots from him;
static void DrawSparkles(struct Fight *f) {
    struct Actor *a = &f->a[A_DEDEDE];
    bool32 on = a->state == 8 && a->v20 && !f->over && Watched(f);
    u8 i;

    for (i = 0; i < 3; i++) {
        if (!on || f->sparkX[i] == 0) {
            Hide(f->sparkles[i]);
            continue;
        }
        if (f->sparkles[i] == NULL) f->sparkles[i] = Sprite(f->room, gNidlKingDedede.tiles[G_FX], gNidlKingDedede.fx[FX_SPARKLE], ORDER_FX);
        Draw(f->sparkles[i], (a->ix + (f->sparkX[i] >> 16)) << 8, (a->iy + (f->sparkY[i] >> 16) + 16) << 8, gNidlKingDedede.fx[FX_SPARKLE],
             f->pal[PAL_COMMON], 1, ORDER_FX);
    }
}

static void KingDededeMain(struct Object *obj);
static void LetGo(struct Fight *f);

/* FightMain(); The fight's task: his sprites and his children's, his palettes; as KatAM's explosion takes him, in his defeat's frame (9), his children gone; gone with
 * him (his defeat goes on, DededeEnd()); */
static void FightMain(void) {
    struct Fight *f = TaskGetStructPtr(gCurTask);
    struct Object *obj = f->obj;
    struct Actor *a = &f->a[A_DEDEDE];
    u8 i;

    if (obj == NULL || (obj->base.flags & 0x1000)) {
        LetGo(f);
        for (i = 0; i < 3; i++) Release(&f->parts[i]);
        for (i = 0; i < 3; i++) Release(&f->sparkles[i]);
        if (obj) ((struct KingDedede *)obj)->f = NULL;
        TaskDestroy(gCurTask);
        return;
    }
    f->frameCount++;
    Palettes(f->room, f->pal);
    if (obj->unk78 != KingDededeMain && !f->over) {
        // (his killing blow: KatAM's defeat, its freeze and explosion; NiDL's 0x0809FD64);
        f->over = TRUE;
        f->hooked = FALSE;
        f->flashing = FALSE;
        LetGo(f);
        SetFrame(a, 9);
    }
    Colors(f->room, f->pal[PAL_BODY], f->flashing && (f->frameCount & 2));
    if (!Watched(f)) {
        for (i = 0; i < 3; i++) Hide(f->parts[i]);
    } else {
        DrawDedede(f->parts, f->room, f->pal, (a->ix + a->dx) << 8, a->iy << 8, a->frame, a->flip);
    }
    for (i = A_DEDEDE + 1; i < ACTORS; i++) {
        struct Actor *c = &f->a[i];

        if (c->kind != K_NONE && c->kind != K_STAR && c->kind != K_HAMMER && c->kind != K_CATCH) DrawActor(f, c);
    }
    DrawSparkles(f);
}

// LetGo(); His children gone (their boxes, the stars), the Kirbys inside him let go (KatAM's grab ends with him);
static void LetGo(struct Fight *f) {
    s8 i;

    for (i = A_DEDEDE + 1; i < ACTORS; i++) Destroy(f, i);
    f->a[A_DEDEDE].child = NONE;
}

/* The fight (KingDededeMain()): NiDL's main each frame (0x080A0A38): a hit taken first (Hit(): KatAM's come between frames); each of
 * NiDL's objects in its slot's order (Process(): hurt, his own script held), their boxes and the stars, KatAM's
 * object where he is (Body()). His states:
 * • 0: his intro, standing (4-7) while the HP bar fills; then a while more (60, 50 or 40 frames), and a walk to Kirby, to his belly
 *   slide;
 * • 1: standing a while (60, 50 or 40 frames by chance; 50, 40 or 30 angry, his HP under a third, standing faster), then a move by
 *   chance by the one before (sMoves[]), every sixth puffing up: a walk to Kirby (1 pixel a frame, bobbing) to his belly slide (3),
 *   his hammer (7) or his inhale (8, to 16 short of Kirby), or his big jump (6);
 * • 3: his belly slide: a hop at Kirby, on his belly sliding (1.25 pixels a frame slowing), the dust;
 * • 4: puffed up, floating to Kirby (300 frames), bouncing off the floor and the ceiling; then 5: his breath at Kirby, down;
 * • 6: his big jump at Kirby (5 pixels a frame up; 1.25 across far off, 2.125 angry), stars either way as he lands;
 * • 7: his hammer: raised, swung down, a star (every fourth, four swings); Kirby high up, a jump and a slam;
 * • 8: his inhale (90 frames), Kirby caught drawn in, puffed up, then 9: spat out in a star;
 * • 10: down from a hit in the air.
 * Hit (KatAM's attacks: KatAM's bosses take only the stronger ones), his own script holds 15 frames, jittering, flashing (the room, the
 * Kirbys, his stars go on), then he stands again (in the air, falls: 10); inhaling or spitting, he only flashes; no more hits for 31
 * frames. His box (NiDL's: -20, -14 to 24, 34) hurts Kirby as KatAM's
 * enemies do, his hammer's and his breath's too. 60 HP (NiDL's 100); */
static void KingDededeMain(struct Object *obj) {
    struct KingDedede *d = (struct KingDedede *)obj;
    struct Fight *f = d->f;
    s8 i;

    NidlUpdate(&d->n);
    obj->base.flags |= 4 | 0x400;
    obj->kirby3 = FindTargetKirby(&obj->base);
    // The HP bar goes to whatever is hit; he takes it back;
    sub_08034E14(obj);
    if (f == NULL) return;
    obj->base.xspeed = obj->base.yspeed = 0;
    f->hit = FALSE;
    f->lastX = f->a[A_DEDEDE].ix;
    f->lastY = f->a[A_DEDEDE].iy;
    if (f->guard && !--f->guard) obj->base.flags &= ~0x8000;
    if (!f->over && obj->unk80 < f->hp) {
        // (one through all the same: its HP back);
        if (f->guard) obj->unk80 = f->hp;
        else Hit(f);
    }
    f->hp = obj->unk80;
    if (f->guard) obj->base.flags |= 0x8000;
    for (i = 0; i < ACTORS; i++) Process(f, i);
    // (+8: 0x080A0A84, after every object's turn);
    if (f->hooked) HurtStep(f);
    DrawIn(f);
    for (i = A_DEDEDE + 1; i < ACTORS; i++) {
        struct Actor *a = &f->a[i];

        if (a->kind == K_NONE) continue;
        Attack(f, a);
        if (a->kind == K_STAR) DriveStar(f, a);
    }
    Body(f);
}

// KingDededeGone(); His object's gone (nidl.c's NidlDestroy()): the fight's task lets go of it;
void KingDededeGone(struct Object *obj) {
    struct KingDedede *d = (struct KingDedede *)obj;

    if (d->f) d->f->obj = NULL;
    d->f = NULL;
}

static void DededeEnd(struct Object *obj);

// KingDededeBeaten(); KatAM's explosion bursts him (nidl_boss.c's NidlBossDefeat()): his defeat, from where he was;
void KingDededeBeaten(struct Object *obj) {
    DededeEnd(obj);
}

/* KingDededeStart(); The fight starts (NidlBossStart(): Kirby in his area), its task with it: NiDL's 0x080A0A0C: facing Kirby, his
 * intro (state 0); */
void KingDededeStart(struct Object *obj) {
    struct KingDedede *d = (struct KingDedede *)obj;
    struct Task *t;
    struct Fight *f;
    struct Actor *a;

    ObjectSetFunc(obj, -1, KingDededeMain);
    if (d->started) return;
    d->started = TRUE;
    t = TaskCreate(FightMain, sizeof(struct Fight), 0x3500, TASK_USE_EWRAM, NULL);
    f = d->f = TaskGetStructPtr(t);
    CpuFill32(0, f, sizeof(struct Fight));
    f->obj = obj;
    f->room = obj->base.roomId;
    f->unk56 = obj->base.unk56;
    f->seed = Rand16() & 0xFFF;
    f->hp = obj->unk80;
    f->angry = obj->unk80 * 85 >> 8;
    f->caught = -1;
    f->catchKirby = NONE;
    a = &f->a[A_DEDEDE];
    a->kind = K_DEDEDE;
    a->facing = a->flip = -1;
    a->frame = 4;
    a->parent = A_DEDEDE;
    a->child = NONE;
    a->x = obj->base.x << 8;
    a->y = obj->base.y << 8;
    a->ix = a->x >> 16;
    a->iy = a->y >> 16;
    Stop(a);
    FaceKirby(f, a);
    SetState(a, 0);
    a->script = 0;
    // (his sprites are the fight's task's: KatAM's object's never shown, its VRAM theirs; unkC 0x200 keeps KatAM from giving it VRAM
    // again, flag 8 from animating it);
    obj->base.unkC |= 0x200;
    if (obj->base.sprite.tilesVram >= 0x6014000) VramFree(obj->base.sprite.tilesVram);
    obj->base.sprite.tilesVram = 0;
    obj->base.flags |= 8;
}

/* KingDededeInit(); On NiDL's spot (232, 296: on the floor there, 271.5, NiDL's terrain), hidden (the fight's task draws him), through
 * the room's terrain (his own: Motion()); his box; */
void KingDededeInit(struct NidlObject *n) {
    struct KingDedede *d = (struct KingDedede *)n;
    struct Object *obj = &n->obj;

    d->f = NULL;
    d->started = FALSE;
    obj->base.x = 232 << 8;
    obj->base.y = (271 << 8) | 0x80;    // (NiDL's x a whole pixel, his y in his pixel's middle, on the floor);
    obj->base.flags |= 0x400 | 0x100;
    obj->base.unk5C = (obj->base.unk5C & ~7) | 3 | 0xA0;
    sub_0803E2B0(&obj->base, -24, -14, 20, 34);
    ObjectSetBounds(&obj->base, -HALF_WIDTH, -HEAD, HALF_WIDTH, FEET);
}

/* His defeat (DededeEnd(), as KatAM's explosion bursts him: NiDL's type 0x3B's second object, 0x080A150C: its states 0x080A1590,
 * 0x080A18D4), traced frame by frame: beaten (9), facing Kirby, down to the floor; 16 frames on, the screen shakes, he slides his
 * way (10: 2, 1, 0.5 pixels a frame, 8 frames each), dazed (11, 12: sDaze); 120 frames from his landing on, and 84 more (NiDL's
 * story's start: not here), his shard (NidlBossShard()); dazed till Kirby leaves the room; */
struct DededeEnd {
    struct EffectObject e;              // (the task's own: never shown);
    struct EffectObject *parts[3];
    struct Actor a;                     // (his: its spot, speed, frame, animation; the room's terrain: Motion());
    struct Fight f;                     // (a fight's for Motion(), Closest(): its room, its level);
    u16 pc, wait;
    s32 timer;                          // (+0x20: 120, 0x080A1864);
    bool8 anim;                         // (+0x28: dazed);
    u8 unk56, shard;
    u8 pal[PALS];
};

#define END_SCRIPT switch (end->pc) { case 0:
#define END_WAIT(n) do { end->wait = (n); end->pc = __LINE__; return; case __LINE__:; } while (0)
#define END_SCRIPT_END } end->pc = PC_END

static void EndScript(struct DededeEnd *end) {
    struct Actor *a = &end->a;
    struct ObjectBase shake;

    END_SCRIPT;
    // (0x080A1590: down to the floor);
    Stop(a);
    FaceKirby(&end->f, a);
    SetFrame(a, 9);
    a->ay = 0x2800;
    a->maxY = 0x50000;
    while (!a->landed) END_WAIT(1);
    Stop(a);
    // (setState(1): its script the frame after);
    END_WAIT(1);
    // (0x080A18D4);
    end->timer = 120;
    END_WAIT(16);
    shake.roomId = end->e.roomId;
    RequestScreenShake(4, &shake);
    SetFrame(a, 10);
    a->v20 = 0x20000;
    for (a->loop = 0; a->loop < 3; a->loop++) {
        VelX(a, a->v20, VEL_KEEP);
        END_WAIT(8);
        a->v20 >>= 1;
    }
    Stop(a);
    a->v24 = Play(a, sDaze);
    end->anim = TRUE;
    while (end->timer > 0) END_WAIT(1);
    END_WAIT(24);
    END_WAIT(60);
    NidlBossShard(end->unk56, end->e.roomId, end->shard);
    for (;;) END_WAIT(1);
    END_SCRIPT_END;
}

static void DededeEndMain(void) {
    struct DededeEnd *end = TaskGetStructPtr(gCurTask);
    struct Actor *a = &end->a;
    u8 i;

    if (end->e.flags & 0x1000 || gKirbys[gLocalPlayerId].base.roomId != end->e.roomId) {
        for (i = 0; i < 3; i++) Release(&end->parts[i]);
        TaskDestroy(gCurTask);
        return;
    }
    if (end->pc != PC_END && (!end->wait || !--end->wait)) EndScript(end);
    Motion(&end->f, a);
    if (end->anim) a->v24 = AnimTick(a, a->v24);
    if (end->timer > 0) end->timer--;
    Palettes(end->e.roomId, end->pal);
    Colors(end->e.roomId, end->pal[PAL_BODY], FALSE);
    DrawDedede(end->parts, end->e.roomId, end->pal, a->ix << 8, a->iy << 8, a->frame, a->flip);
}

static void DededeEnd(struct Object *obj) {
    struct Task *t = TaskCreate(DededeEndMain, sizeof(struct DededeEnd), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct DededeEnd *end = TaskGetStructPtr(t);
    struct KingDedede *d = (struct KingDedede *)obj;
    struct Actor *a = &end->a;

    CpuFill32(0, end, sizeof(struct DededeEnd));
    ClearEffectObject(&end->e);
    end->e.header.kind = 3;
    end->e.roomId = obj->base.roomId;
    end->e.parent = NULL;
    end->e.flags |= 0x400;
    end->unk56 = obj->base.unk56;
    end->shard = d->n.variant & 7;
    end->f.room = obj->base.roomId;
    end->f.unk56 = obj->base.unk56;
    a->kind = K_DEDEDE;
    a->child = NONE;
    a->facing = a->flip = d->f ? d->f->a[A_DEDEDE].facing : -1;
    if (d->f) {
        a->x = d->f->a[A_DEDEDE].x;
        a->y = d->f->a[A_DEDEDE].y;
        a->landed = d->f->a[A_DEDEDE].landed;
    } else {
        a->x = obj->base.x << 8;
        a->y = obj->base.y << 8;
    }
    a->ix = a->x >> 16;
    a->iy = a->y >> 16;
    a->frame = 9;
    Stop(a);
}
