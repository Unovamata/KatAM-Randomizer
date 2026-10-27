/* Meta Knight; ported from Kirby: Nightmare in Dream Land (USA), boss object type 0x3D (spawn 0x080A55EC), as NiDL boss
 * NIDL_META_KNIGHT, Orange Ocean's (room 0x29D here). NiDL's, from its code (24 states, each a script, 0x08748EBC, and a function every
 * frame after it, 0x08748F1C; its main 0x080A56B0), checked frame by frame against NiDL's recorded fight (its states, frames, spots and
 * speeds): his intro (state 0), his duel (see "His duel" below) and his defeat (MetaKnightEnd(), as KatAM's explosion bursts him:
 * NiDL's unmasked Meta Knight, type 0x3D's second object, 0x080A73B4). NiDL throws Kirby a sword before the duel and as he leaves;
 * here Kirby fights with his own powers (no sword thrown), and his slashes and his thrust into the ground knock out stars that Kirby,
 * with none, inhales and spits back at him (Star(): not NiDL's, as King Dedede's);
 * His graphics (KatAMExpansion.cs, MetaKnight()): NiDL's streamed frames (0x08753BB4: 0-9 caped, 10-20 drawing Galaxia, 21-59 his
 * defeat, 60-122 his duel), drawn with two palettes (his body's; Galaxia's and its slashes'); KatAM's sprites have one: three sprites
 * (his body, Galaxia's pieces in front of it, behind it: gNidlMetaKnightFight), his effects NiDL's own frames (0x08753DA0's: his cape,
 * the spark, his mask; NiDL's shared dust, 0x0874CB90 and 0x0874C828); his object's own sprite (type 0x43's frame 0: NiDL's 4,
 * caped) shows only till his intro starts; */
#include "../nidl.h"
#include "code_0806F780.h"
#include "task.h"
#include "main.h"
#include "functions.h"
#include "constants/songs.h"

/* gNidlMetaKnightFight; Filled in by KatAMExpansion.cs (WriteMetaKnightFight()): per NiDL frame (0-122) the KatAM frame of Galaxia's
 * pieces drawn in front of his body and behind it (FRAME_NONE: none; his body is BODY()), his effects' first frames, the VRAM tiles
 * of his sprites' biggest frames, his body's colors and his hurt flash's (NiDL's, USA 0x082F427C); */
struct NidlMetaKnightFight {
    u8 front[123];
    u8 back[123];
    u8 cape, spark, mask, dust, landing;    // 0x08753DA0's 0-3; 13-15, 36-38; 26-34; 0x0874CB90's 0-3, 6-9; 0x0874C828's 0-23;
    u8 tiles[4];                            // His body's, in front, behind; the effects';
    u16 body[16], flash[16];
};

const struct NidlMetaKnightFight gNidlMetaKnightFight = { { 0 } };

#define NIDL_FRAMES 123
#define BODY(f) (1 + (f))           // (KatAM's frame 0 is type 0x43's: NiDL's 4, caped);
#define FRAME_HIDDEN -1
#define FRAME_NONE 0xFF
#define NO_MAX 0x7FFF
#define PC_END 0xFFFF
#define PARTS 3
#define PART_BODY 0
#define PART_FRONT 1
#define PART_BACK 2
#define FRONT_ORDER 0x18            // Sprite priorities (NiDL's: his effects 3 and his dust 10 in front of him, 11, his cape 12 behind);
#define SWORD_ORDER 0x19
#define BODY_ORDER 0x1A
#define BACK_ORDER 0x1B
#define CAPE_ORDER 0x1C
#define KEY_SWORD 0x2000            // His palettes' keys (| his anim ID: his body's is his anim ID, as his object's);
#define KEY_SPARK 0x4000
#define KEY_DUST 0x6000
#define SWORD_SPOT 120              // Where NiDL's sword stands (its screen's x, one player: 0x08748D28), and how near Kirby takes it;
#define SWORD_REACH 9
#define EDGE 111                    // The screen's edges for him (NiDL's 0x080227A4: its middle's x +-111, its top 72 up from its middle);
#define TOP 72
#define HURT_TIME 13                // NiDL's hurt (0x0806619C): frozen, jittering, flashing; the room's layers mosaic;
#define MOSAIC_LAYERS 0x6           // (KatAM's room layers: BG1 and BG2, gBgCntRegs; not the HUD's);
#define REG_MOSAIC_ADDR 0x0400004C
#define FLY_OFF_Y 10                // His defeat: flying off up to the room's y 10 (NiDL's);
#define STARS 4                     // His stars at once (Star()), their frames (enemies/star.c's: NiDL's shared 0x0874CCBC's 4-13 from its
#define STAR_FRAMES 6               // 6, King Dedede's), how far ahead of him and down they come out;
#define STAR_AHEAD 32
#define STAR_DOWN 4
#define WALL_X 16                   // (none past 16 in from the screen's sides: out of Kirby's reach, in the walls);

enum { PAL_BODY, PAL_SWORD, PAL_SPARK, PAL_DUST, PALS };
enum { FX_CAPE, FX_SPARK, FX_DUST, FX_LANDING, FX_LANDING_LEFT, FX_LANDING_RIGHT, FX_MASK, FX_MASK_HALF };

/* NiDL's tables (0x08748D28 on): his walk's and run's frame times (61-68: NiDL reads on past them, his frames 69 and 70 as he walks on
 * from a guard); */
static const u8 sWalkWaits[10] = { 2, 10, 5, 5, 2, 10, 5, 5, 2, 6 };
static const u8 sRunWaits[10] = { 2, 6, 3, 3, 2, 6, 3, 3, 18, 0 };
// Near Kirby walking (state 1): by whether Kirby's on the ground (0x08748DA8);
static const u8 sNearMoves[2][8] = { { 18, 18, 18, 18, 3, 3, 3, 3 }, { 7, 7, 7, 7, 11, 11, 11, 3 } };
// His walk's countdown out: stopped at the screen's edge (0x08748DC8), else by whether Kirby's on the ground (0x08748DE8);
static const u8 sEdgeMoves[16] = { 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 7, 7, 7, 7, 3, 3 };
static const u8 sWalkMoves[2][16] = { { 4, 4, 4, 6, 6, 6, 6, 6, 6, 6, 12, 12, 12, 3, 3, 3 }, { 4, 4, 4, 4, 11, 11, 11, 11, 11, 11, 7, 7, 7, 7, 7, 7 } };
// Falling from a leap (states 8, 9), by where Kirby is: above him, near, ahead, behind (0x08748E28; 0: none, on to the ground);
static const u8 sAirMoves[4][4] = { { 17, 17, 13, 13 }, { 15, 15, 15, 0 }, { 13, 13, 0, 0 }, { 15, 13, 0, 0 } };
// Where Kirby was, reached (state 6): Kirby near his height or not (0x08748E48, 0x08748E68);
static const u8 sReachMoves[2][16] = {
    { 3, 3, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18 }, { 3, 3, 3, 3, 3, 14, 14, 14, 14, 14, 14, 14, 18, 18, 18, 18 },
};
// Kirby reached running (state 7): his HP over half, under (0x08748E88, 0x08748E98);
static const u8 sRunMoves[2][8] = { { 19, 19, 19, 19, 20, 21, 21, 21 }, { 19, 20, 20, 20, 20, 21, 2, 2 } };
// His side of Kirby by turns (0x08748D60), the slashes' lunges (0x08748D44; the third's, 0x08748D4C, 0x08748D50), knocked back guarding
// (0x08748D78, 0x08748D80), his guard's chances in 16 by his HP's quarter (0x08748D74), free of an inhale (0x08748EA8);
static const s16 sSides[3] = { -64, 64, -64 };
static const s16 sLunge[2] = { 0x200, 0x100 };
static const u8 sThirdSteps[3] = { 10, 18, 26 };
static const s16 sThird[4] = { 0x200, 0x180, 0xC0, 0 };
static const u8 sGuardSteps[6] = { 8, 16, 17, 18, 19, 20 };
static const s16 sGuardSpeeds[6] = { -0x100, -0x200, 0x200, -0x200, 0x200, -0x200 };
static const u8 sGuardChances[4] = { 5, 4, 3, 2 };
static const u8 sFreeMoves[2] = { 7, 11 };
// Hurt (0x0806627C): his jitter's steps by its countdown (0x0873E184, 0x0873E16C), the room's mosaic (0x08748EAC);
static const u8 sJitterOf[14] = { 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4 };
static const s8 sJitter[6][2] = { { 2, 1 }, { -2, -1 }, { -2, 0 }, { 2, 1 }, { 2, -1 }, { -2, 0 } };
static const u8 sMosaic[7] = { 0, 0, 0, 0, 0, 0x22, 0x44 };

/* Galaxia's box (NiDL's companion, type 0xB8: per frame 88-121, 0x08748F8C, its spot from his, 0x08749014 and 0x08749058; left, top,
 * right, bottom, facing right): his slashes, his up-slash, his thrust down, his spin; */
struct SwordBox {
    s8 x, y, box[4];
};
static const struct SwordBox sSwordBoxes[34] = {
    [92 - 88] = { 0, 0, { 16, -16, 48, 16 } }, [93 - 88] = { 0, 0, { 16, -16, 48, 16 } },
    [100 - 88] = { 10, -28, { -6, -6, 6, 6 } }, [101 - 88] = { 10, -28, { -6, -6, 6, 6 } }, [102 - 88] = { 4, -24, { -12, -7, 12, 7 } },
    [103 - 88] = { 4, -44, { -4, -4, 4, 20 } }, [104 - 88] = { 4, -32, { -6, -6, 6, 6 } },
    [108 - 88] = { 0, 0, { 16, -16, 48, 16 } },
    [112 - 88] = { 6, 24, { -6, -6, 6, 6 } },
    [114 - 88] = { 0, -16, { -12, -6, 12, 6 } }, [115 - 88] = { 12, -12, { -10, -10, 10, 10 } }, [116 - 88] = { 16, 0, { -6, -12, 6, 12 } },
    [117 - 88] = { 12, 12, { -10, -10, 10, 10 } }, [118 - 88] = { 0, 16, { -12, -6, 12, 6 } }, [119 - 88] = { -12, 12, { -10, -10, 10, 10 } },
    [120 - 88] = { -16, 0, { -6, -12, 6, 12 } }, [121 - 88] = { -12, -12, { -10, -10, 10, 10 } },
};

struct MetaKnight {
    struct NidlObject n;
    struct Fight *f;
    bool8 started;
};

typedef char sMetaKnightFits[sizeof(struct MetaKnight) <= NIDL_OBJECT_SIZE ? 1 : -1];

/* Fight; His duel's state (a task of its own, NIDL_OBJECT_SIZE has no room), NiDL's object's fields and its globals' (0x02007D00 on); */
struct Fight {
    struct Object *obj;                 // (NULL: gone, MetaKnightGone());
    struct EffectObject *parts[PARTS];  // His sprites;
    struct ObjectBase *sword;           // Galaxia's box;
    u16 pc, wait;                       // His state's script and its wait;
    u8 state, next;                     // (+0x15, +0x14);
    bool8 restart;                      // His next state starts this frame;
    s8 facing;                          // (+0x43: 1 right, -1 left);
    s8 flip;                            // His sprites' (+0x3E's flip: his facing as his frame was last set, SetFrame());
    s16 frame;                          // His frame (NiDL's, FRAME_HIDDEN: none; +0x3C);
    s16 vx, vy, ax, ay, maxX, maxY;     // 8.8 (+0x54 on: NiDL's 16.16 >> 8, all whole there);
    s16 timer;                          // His walk's countdown (+0x1C);
    s16 offset;                         // His spot from Kirby's x walking (+0x18);
    u8 walk;                            // 1: held on 65, 2: running (+0x20);
    u8 step;                            // His walk's: to his spot, by Kirby, stopped (+0x24);
    u8 guard;                           // His guard's chances (+0x28: 0x80 in a move, 1-3 more);
    s8 boxSet;                          // His box (+0x30: 0, 1 hit; 2 and -1 not);
    u8 walkFlags;                       // 1: stopped at the screen's edge, 2: walking his way, 4: no move on the way (+0x34);
    s16 loop, loop2;                    // (+0x6C, +0x6E);
    bool8 landed;                       // On the ground (+0x7A);
    s32 g14, g18, g1C;                  // (0x02007D14-1C: what each state keeps there);
    struct Kirby *target, *lastTarget;  // The Kirby he walks by (0x02007D14 in state 1; +0x2C), the one inhaling him (0x02007D00);
    u16 seed;                           // NiDL's random numbers' (0x03000FB4);
    s16 hp;                             // His HP last frame (less now: hit);
    s16 maxHp;                          // (0x02007D0C);
    u8 hurt;                            // His hurt's countdown (0x02006190 + 0xC; 0: not hurt);
    bool8 flash;                        // His hurt flash this frame;
    s16 hurtFrame;
    s32 hurtX, hurtY;                   // Where he was hit;
    u8 walls;                           // The room's walls touching him last frame;
    s32 floorY;                         // The floor's y under him (his defeat lands there);
    u8 pal[PALS];
    struct FightStar {
        struct Object *obj;             // (NULL: none; KatAM's star enemy, enemies/star.c: inhaled, it's Kirby's);
        s32 x, y;                       // 8.8;
        s16 vx, vy;
        u8 step, wait;                  // Its step (sStarSteps) and that step's frames left;
    } stars[STARS];
};

// Rand(); NiDL's (0x08002EE8): 0 to n - 1;
static u16 Rand(struct Fight *f, u16 n) {
    f->seed = (f->seed * 61 + 0x579) & 0xFFF;
    return f->seed * n >> 12;
}

/* ScreenX(), ScreenY(); NiDL's screen in his room (its camera: (8, 8), the room's top left, there all along), which his moves go by
 * (his spot, the screen's edges, his defeat's spots): his room is NiDL's, so he moves where NiDL's does; KatAM's camera shows the
 * room 16 pixels lower (its top row hidden: KatAMExpansionRooms.cs' CameraTop()); */
#define NIDL_SCREEN_X 8
#define NIDL_SCREEN_Y 8

static s32 ScreenX(struct Object *obj) {
    return NIDL_SCREEN_X;
}

static s32 ScreenY(struct Object *obj) {
    return NIDL_SCREEN_Y;
}

static s32 X(struct Fight *f) { return f->obj->base.x >> 8; }
static s32 Y(struct Fight *f) { return f->obj->base.y >> 8; }

/* KirbyX(), KirbyY(); A Kirby's spot as NiDL's reads it, after his move this frame (NiDL moves the Kirbys before him, KatAM after: his
 * speed added; his y KIRBY_DY up, as NiDL has it: KatAM's is lower, 7 pixels over the floor standing, NiDL's 11); */
#define KIRBY_DY 4

static s32 KirbyX(struct Kirby *k) {
    return (k->base.x + k->base.xspeed) >> 8;
}

static s32 KirbyY(struct Kirby *k) {
    return ((k->base.y - k->base.yspeed) >> 8) - KIRBY_DY;
}

/* KirbyKeys(); A Kirby's D-pad this frame, as NiDL's reads it (0x03000F98): KatAM takes it into movementState as the Kirby moves,
 * after him (sub_0803F324): the player's keys, alone; else the last frame's; */
static u16 KirbyKeys(struct Kirby *k) {
    if (!(gUnk_0203AD10 & 0x12) && k->base.unk56 == 0) return gHeldKeys;
    return k->movementState;
}

// Closest(); The Kirby closest to him across (NiDL's 0x08063B38: in his room, alive), or NULL;
static struct Kirby *Closest(struct Fight *f) {
    struct Kirby *best = NULL;
    s32 d, bestD = 0;
    u8 i;

    for (i = 0; i < gNumKirbys; i++) {
        struct Kirby *k = &gKirbys[i];

        if (k->base.roomId != f->obj->base.roomId || k->hp <= 0) continue;
        d = abs(KirbyX(k) - X(f));
        if (best == NULL || d < bestD) {
            best = k;
            bestD = d;
        }
    }
    return best;
}

// DxKirby(), DyKirby(); The closest Kirby from him (0x08063CD0, 0x08063D2C; 0: none);
static s32 DxKirby(struct Fight *f) {
    struct Kirby *k = Closest(f);

    return k ? KirbyX(k) - X(f) : 0;
}

static s32 DyKirby(struct Fight *f) {
    struct Kirby *k = Closest(f);

    return k ? KirbyY(k) - Y(f) : 0;
}

// FaceKirby(); 0x08063E14: towards the closest Kirby (right when level);
static void FaceKirby(struct Fight *f) {
    struct Kirby *k = Closest(f);

    if (k) f->facing = KirbyX(k) - X(f) >= 0 ? 1 : -1;
}

// OnGround(); A Kirby's NiDL +0x7A: 1 on the ground, 0 in the air;
static u8 OnGround(struct Kirby *k) {
    return (k->base.unk62 & 4) ? 1 : 0;
}

static void Stop(struct Fight *f) {
    f->vx = f->vy = f->ax = f->ay = 0;
    f->maxX = f->maxY = NO_MAX;
}

static void StopY(struct Fight *f) {
    f->vy = f->ay = 0;
    f->maxY = NO_MAX;
}

// SetFrame(); NiDL's 0x08006338: his frame, his sprites facing his way (NiDL's other frame changes leave them as they were);
static void SetFrame(struct Fight *f, s16 frame) {
    f->frame = frame;
    f->flip = f->facing;
}

// VelX(); NiDL's 0x080061C0: x speed for facing right, mirrored otherwise (its change kept);
static void VelX(struct Fight *f, s16 vx) {
    f->vx = f->facing == 1 ? vx : -vx;
}

// VelY(); NiDL's 0x0800622C: y speed, its change and its cap;
static void VelY(struct Fight *f, s16 vy, s16 ay, s16 maxY) {
    f->vy = vy;
    f->ay = ay;
    f->maxY = maxY;
}

// TurnAround(); 0x08063FF4: facing (his sprites too: his frame set again) and moving the other way;
static void TurnAround(struct Fight *f) {
    f->facing = -f->facing;
    SetFrame(f, f->frame);
    f->vx = -f->vx;
    f->ax = -f->ax;
}

/* Edges(); NiDL's 0x080227A4: kept on the screen (its edges, EDGE, TOP: put back there), which edges he's at (1 left, 2 right, 4 the
 * top); */
static u8 Edges(struct Fight *f) {
    struct Object *obj = f->obj;
    s32 middle = ScreenX(obj) + DISPLAY_WIDTH / 2, top = ScreenY(obj) + DISPLAY_HEIGHT / 2 - TOP;
    u8 edges = 0;

    if (X(f) < middle - EDGE) {
        obj->base.x = (middle - EDGE) << 8;
        edges = 1;
    } else if (X(f) > middle + EDGE) {
        obj->base.x = (middle + EDGE) << 8;
        edges = 2;
    }
    if (Y(f) < top) {
        obj->base.y = top << 8;
        edges |= 4;
    }
    return edges;
}

// AtEdge(); 0x080A6F74: kept on the screen (Edges()), and moving into its edge;
static bool32 AtEdge(struct Fight *f) {
    u8 edges = Edges(f);

    return ((edges & 1) && f->vx < 0) || ((edges & 2) && f->vx > 0);
}

/* Walled(); 0x080A6F38: anything in the room's tile left or right of (x, y) (pixels; NiDL's 16x16 tiles, 0x08021BB4: the room's outer
 * ring counts, as outside it); */
static bool32 Walled(struct Fight *f, s32 x, s32 y) {
    struct Object *obj = f->obj;
    struct LevelInfo *level = &gCurLevelInfo[obj->base.unk56];
    s32 tx = x >> 4, ty = y >> 4, d;

    for (d = -1; d <= 1; d += 2) {
        if (tx + d <= 0 || tx + d >= level->roomWidth / 16 - 1 || ty <= 0 || ty >= level->roomHeight / 16 - 1) return TRUE;
        if (!(gCollisionAttributes[GetCollisionTile(obj->base.unk56, tx + d, ty)] & 1)) return TRUE;
    }
    return FALSE;
}

// HpQuarter(); 0x080A6E1C: his HP's quarter (0 the lowest);
static u8 HpQuarter(struct Fight *f) {
    s16 hp = f->obj->unk80;

    if (hp < f->maxHp / 4) return 0;
    if (hp < f->maxHp / 2) return 1;
    if (hp < f->maxHp / 4 * 3) return 2;
    return 3;
}

// Countdown(); His walk's countdown (0x080A6E58): longer under half his HP;
static void Countdown(struct Fight *f) {
    if (f->obj->unk80 >= f->maxHp / 2) f->timer = Rand(f, 54) + 48;
    else f->timer = Rand(f, 84) + 96;
}

static void Effect(struct Fight *f, u8 kind, s32 x, s32 y, s8 facing);
static void Star(struct Fight *f, s32 x, s32 y, s8 facing);
static void SlashStar(struct Fight *f);
static void ReleaseStars(struct Fight *f);

/* The scripts: WAIT() resumes them that many frames on; SET_STATE() ends them (NiDL's setState and endScript), the state starting as
 * the frame functions start it (Begin()); the variables they keep across waits are the Fight's; */
#define SCRIPT switch (f->pc) { case 0:
#define WAIT(n) do { f->wait = (n); f->pc = __LINE__; return; case __LINE__:; } while (0)
#define SET_STATE(s) do { f->next = (s); f->pc = PC_END; return; } while (0)
#define SCRIPT_END } f->pc = PC_END

/* Begin(); His next state starts (the frame after: NiDL's startScript); a move (0x080A7168) can be guarded in (Hit(): 0x80 in guard); */
static void Begin(struct Fight *f, bool32 move) {
    if (move) f->guard |= 0x80;
    f->restart = TRUE;
}

/* Walk(); His walk (0x080A6E98, states 1 and 6, for ever): frames 61-68 forward or back as he moves, each its time walking (the run's
 * running), held on 65 when stopped there; loop2 set once it's going (each call after the first, a step: its wait's over); */
static void Walk(struct Fight *f) {
    s32 i;

    if (f->loop2 && !(f->walk & 1)) {
        if ((f->facing == 1 && f->vx >= 0) || (f->facing == -1 && f->vx <= 0)) {
            if (++f->frame > 68) f->frame = 61;
        } else {
            if (--f->frame <= 60) f->frame = 68;
        }
    }
    f->loop2 = 1;
    i = f->frame - 61;
    if (i < 0 || i >= 10) i = 0;
    f->wait = (f->walk & 2) ? sRunWaits[i] : sWalkWaits[i];
}

/* Choose(); His walk's countdown out (0x080A7190): stopped at the screen's edge, a leap or a run; else by whether Kirby's on the
 * ground; */
static void Choose(struct Fight *f) {
    struct Kirby *k = Closest(f);

    if (f->walkFlags & 1) f->next = sEdgeMoves[Rand(f, 16)];
    else f->next = sWalkMoves[k ? OnGround(k) : 0][Rand(f, 16)];
    f->walkFlags = 0;
    Begin(f, TRUE);
}

/* UpSlash(); His up-slash (0x080A7080, its steps in loop2): Galaxia raised (98, 99), a hop up slashing (100-104), then down; TRUE:
 * waiting; */
static bool32 UpSlash(struct Fight *f) {
    static const u8 steps[11][2] = {
        { 98, 6 }, { 99, 2 }, { 98, 2 }, { 99, 4 }, { 100, 1 }, { 101, 1 }, { 102, 1 }, { 103, 2 }, { 104, 5 }, { 101, 2 }, { 100, 2 },
    };
    u8 step = f->loop2;

    if (step >= 11) return FALSE;
    if (step == 0 || step == 9) SetFrame(f, steps[step][0]);
    else f->frame = steps[step][0];
    f->wait = steps[step][1];
    if (step == 4) {
        PlaySfx(&f->obj->base, SE_DMK_SWORD_SLASH_ATTACK);
        f->landed = FALSE;
        f->vy = -0x400;
        f->ay = 0x80;
    } else if (step == 6 || step == 10) {
        StopY(f);
    } else if (step == 9) {
        f->vy = 0x400;
        f->ay = -0x80;
    }
    f->loop2++;
    return TRUE;
}

/* His states (NiDL's 24, 0x08748EBC); */

// State0(); His intro (0x080A57D4): caped on his spot until a Kirby reaches where NiDL's sword stands, then his cape off and Galaxia
// drawn, the spark, a shiver, and he leaps down at Kirby (state 9);
static void State0(struct Fight *f) {
    struct Object *obj = f->obj;
    u8 i;

    SCRIPT;
    f->state = 0;
    obj->base.x = (ScreenX(obj) + 216) << 8;
    obj->base.y = (ScreenY(obj) + 44) << 8;
    f->facing = 1;
    SetFrame(f, 4);
    // (NiDL's: until every player takes the sword thrown to him; here, as one reaches where it stands);
    for (;;) {
        for (i = 0; i < gNumKirbys; i++) {
            struct Kirby *k = &gKirbys[i];

            if (k->base.roomId == obj->base.roomId && k->hp > 0 && abs(KirbyX(k) - ScreenX(obj) - SWORD_SPOT) <= SWORD_REACH) break;
        }
        if (i < gNumKirbys) break;
        WAIT(1);
    }
    WAIT(1);
    f->boxSet = 2;
    f->frame = 5;
    WAIT(3);
    f->frame = 6;
    WAIT(3);
    f->frame = 7;
    WAIT(1);
    f->frame = 8;
    WAIT(2);
    f->frame = 9;
    WAIT(3);
    Effect(f, FX_CAPE, obj->base.x + 8 * 0x100, obj->base.y, 1);
    f->frame = 10;
    WAIT(3);
    f->frame = 11;
    WAIT(3);
    f->frame = 12;
    WAIT(18);
    f->frame = 13;
    WAIT(3);
    f->frame = 14;
    WAIT(1);
    f->frame = 15;
    WAIT(1);
    f->frame = 16;
    WAIT(1);
    f->frame = 17;
    WAIT(1);
    f->frame = 18;
    WAIT(2);
    f->frame = 19;
    WAIT(2);
    f->frame = 20;
    WAIT(3);
    Effect(f, FX_SPARK, obj->base.x - 32 * 0x100, obj->base.y, 1);
    for (f->loop = 0; f->loop < 3; f->loop++) {
        f->vy = 0x200;
        f->ay = -0x100;
        WAIT(3);
    }
    StopY(f);
    WAIT(20);
    f->facing = -1;
    SetFrame(f, 60);
    WAIT(10);
    f->g14 = 0x180;
    f->g18 = 0;
    SET_STATE(9);
    SCRIPT_END;
}

// State1(); Walking at Kirby to his spot beside him (0x080A5AA0; Frame1()), for ever;
static void State1(struct Fight *f) {
    if (f->pc == 0) {
        f->state = 1;
        f->guard &= ~0x80;
        f->boxSet = 0;
        f->target = Closest(f);
        // (another Kirby the closest: his walk starts over);
        if (f->lastTarget != f->target) {
            f->walk = 0;
            f->step = 0;
            f->lastTarget = f->target;
        }
        f->pc = 1;
        f->loop2 = 0;
    }
    Walk(f);
}

// State2(); To the other side of Kirby (0x080A5DD0; not walking his way: the side he faces), walking (state 1);
static void State2(struct Fight *f) {
    SCRIPT;
    f->state = 2;
    if (!(f->walkFlags & 2)) f->offset = f->facing == 1 ? 64 : -64;
    Countdown(f);
    f->walk = 0;
    f->step = 0;
    f->walkFlags &= 2;
    SetFrame(f, 61);
    SET_STATE(1);
    SCRIPT_END;
}

// State3(); To the other side of Kirby, walking his way (0x080A5E60);
static void State3(struct Fight *f) {
    SCRIPT;
    f->state = 3;
    f->offset = -f->offset;
    Countdown(f);
    f->walk = 0;
    f->step = 0;
    f->walkFlags = 2;
    SetFrame(f, 61);
    SET_STATE(1);
    SCRIPT_END;
}

// State4(); Walking on 48 frames (0x080A5ECC);
static void State4(struct Fight *f) {
    SCRIPT;
    f->state = 4;
    f->timer = 48;
    SET_STATE(1);
    SCRIPT_END;
}

// State5(); Back to his side of Kirby, no move on the way (0x080A5F20);
static void State5(struct Fight *f) {
    SCRIPT;
    f->state = 5;
    f->walkFlags = 4;
    FaceKirby(f);
    f->offset = f->facing == 1 ? -64 : 64;
    Countdown(f);
    f->walk = 0;
    f->step = 0;
    SetFrame(f, 61);
    SET_STATE(1);
    SCRIPT_END;
}

// State6(); Walking to where Kirby is (0x080A5FAC; Frame6()), for ever;
static void State6(struct Fight *f) {
    struct Kirby *k;

    if (f->pc == 0) {
        f->state = 6;
        f->g18 = f->offset < 0 ? 1 : 0;
        k = Closest(f);
        f->g1C = k ? KirbyX(k) : X(f);
        f->walk = 0;
        FaceKirby(f);
        VelX(f, 0x100);
        SetFrame(f, 61);
        f->pc = 1;
        f->loop2 = 0;
    }
    Walk(f);
}

// State7(); Running at Kirby (0x080A60D8; Frame7()): 82-87, 2 frames each, for ever;
static void State7(struct Fight *f) {
    SCRIPT;
    f->state = 7;
    FaceKirby(f);
    VelX(f, 0x1C0);
    for (;;) {
        SetFrame(f, 82);
        WAIT(2);
        for (f->loop = 0; f->loop <= 4; f->loop++) {
            f->frame++;
            WAIT(2);
        }
    }
    SCRIPT_END;
}

// Leap(); A leap (0x080A6280, 0x080A61B4: states 8 and 9), g14 ahead, `up` up: crouched (72) 20 frames, rising through 73-80, then
// till he lands (AirMove(): a move falling, g18's or one by where Kirby is), crouched again (state 10);
static void Leap(struct Fight *f, u8 state, s16 up) {
    SCRIPT;
    f->state = state;
    if (state == 9) f->boxSet = 0;
    f->g1C = 0;
    f->landed = FALSE;
    VelY(f, up, 0x15, 0x300);
    VelX(f, f->g14);
    SetFrame(f, 72);
    WAIT(20);
    for (f->loop = 0; f->loop <= 6; f->loop++) {
        f->frame++;
        WAIT(2);
    }
    f->frame++;
    while (!f->landed) WAIT(1);
    Stop(f);
    SET_STATE(10);
    SCRIPT_END;
}

static void State8(struct Fight *f) {
    Leap(f, 8, -0x330);
}

static void State9(struct Fight *f) {
    Leap(f, 9, -0x260);
}

// State10(); Landed, crouched (81) 12 frames, standing (60) 56 (0x080A634C), then to the other side of Kirby (state 2);
static void State10(struct Fight *f) {
    SCRIPT;
    f->state = 10;
    Stop(f);
    f->boxSet = 0;
    SetFrame(f, 81);
    WAIT(12);
    SetFrame(f, 60);
    WAIT(56);
    SET_STATE(2);
    SCRIPT_END;
}

// State11(), State12(); A leap at Kirby, straight up (0x080A63A4, 0x080A63F0: state 8);
static void State11(struct Fight *f) {
    SCRIPT;
    f->state = 11;
    FaceKirby(f);
    f->g14 = 0x180;
    f->g18 = 0;
    SET_STATE(8);
    SCRIPT_END;
}

static void State12(struct Fight *f) {
    SCRIPT;
    f->state = 12;
    FaceKirby(f);
    f->g14 = 0;
    f->g18 = 0;
    SET_STATE(8);
    SCRIPT_END;
}

// State13(); His spin in the air (0x080A6438): twice (four times high on the screen) 114-121, a frame each, then 122, 80 till he lands
// (Frame());
static void State13(struct Fight *f) {
    struct Object *obj = f->obj;

    SCRIPT;
    f->state = 13;
    f->ay = 0x48;
    f->maxY = 0x400;
    f->g1C = Y(f) - ScreenY(obj) <= 43 ? 4 : 2;
    for (f->loop = 0; f->loop < f->g1C; f->loop++) {
        PlaySfx(&obj->base, SE_DMK_TORNADO);
        SetFrame(f, 114);
        WAIT(1);
        for (f->loop2 = 0; f->loop2 <= 6; f->loop2++) {
            f->frame++;
            WAIT(1);
        }
    }
    SetFrame(f, 122);
    WAIT(2);
    SetFrame(f, 80);
    SCRIPT_END;
}

// State14(), State16(); A leap at Kirby, then his up-slash (17), his thrust down (15) falling (0x080A6650, 0x080A6544);
static void State14(struct Fight *f) {
    SCRIPT;
    f->state = 14;
    FaceKirby(f);
    f->g14 = 0x180;
    f->g18 = 17;
    SET_STATE(8);
    SCRIPT_END;
}

static void State16(struct Fight *f) {
    SCRIPT;
    f->state = 16;
    FaceKirby(f);
    f->g14 = 0x180;
    f->g18 = 15;
    SET_STATE(8);
    SCRIPT_END;
}

// State15(); His thrust down (0x080A658C): 111, 112 down to the ground, the dust there, 113 12 frames, standing (60) 56, then to the
// other side of Kirby (state 2);
static void State15(struct Fight *f) {
    struct Object *obj = f->obj;

    SCRIPT;
    f->state = 15;
    Stop(f);
    SetFrame(f, 111);
    WAIT(2);
    f->frame = 112;
    VelY(f, 0x200, 0x48, 0x400);
    while (!f->landed) WAIT(1);
    Stop(f);
    PlaySfx(&obj->base, SE_INTRO_CUTSCENE_DMK_LANDING);
    Effect(f, FX_LANDING, obj->base.x, obj->base.y + 3 * 0x100, 1);
    // (a star either way, as King Dedede's landing: not NiDL's);
    Star(f, X(f) + STAR_AHEAD, Y(f) + STAR_DOWN, 1);
    Star(f, X(f) - STAR_AHEAD, Y(f) + STAR_DOWN, -1);
    f->boxSet = 0;
    f->frame = 113;
    WAIT(12);
    SetFrame(f, 60);
    WAIT(56);
    SET_STATE(2);
    SCRIPT_END;
}

// State17(); His up-slash in the air (0x080A6698), then falling, 80, till he lands (Frame());
static void State17(struct Fight *f) {
    if (f->pc == 0) {
        f->state = 17;
        f->loop2 = 0;
        f->pc = 1;
    }
    if (UpSlash(f)) return;
    VelY(f, 0, 0x15, 0x300);
    SetFrame(f, 80);
    f->pc = PC_END;
}

// State18(); His up-slash on the ground (0x080A6710), then to the other side of Kirby (state 2);
static void State18(struct Fight *f) {
    if (f->pc == 0) {
        f->state = 18;
        Stop(f);
        f->loop2 = 0;
        f->pc = 1;
    }
    if (UpSlash(f)) return;
    f->next = 2;
    f->pc = PC_END;
}

// Dust(); A slash: the dust behind him (NiDL's type 0x91), a star ahead (SlashStar(): not NiDL's);
static void Dust(struct Fight *f) {
    struct Object *obj = f->obj;

    PlaySfx(&obj->base, SE_DMK_SWORD_SLASH_ATTACK);
    Effect(f, FX_DUST, obj->base.x - f->facing * 8 * 0x100, obj->base.y + 8 * 0x100, f->facing);
    SlashStar(f);
}

// State19(); A slash (0x080A6754): 88 12 frames, 89-96 a frame each, 97 24, standing (60) 28 (lunging: Lunge()), then back to his
// side (state 5);
static void State19(struct Fight *f) {
    SCRIPT;
    f->state = 19;
    Stop(f);
    f->g1C = -1;
    SetFrame(f, 88);
    WAIT(12);
    Dust(f);
    for (f->loop = 0; f->loop <= 7; f->loop++) {
        f->frame++;
        WAIT(1);
    }
    f->boxSet = 0;
    f->frame++;
    WAIT(24);
    SetFrame(f, 60);
    WAIT(28);
    SET_STATE(5);
    SCRIPT_END;
}

// State20(); Two slashes (0x080A6868): 88 4 frames, 89-96, 97 8, 96-89, 88 10, then the third (state 21);
static void State20(struct Fight *f) {
    SCRIPT;
    f->state = 20;
    Stop(f);
    f->g1C = -1;
    SetFrame(f, 88);
    WAIT(4);
    Dust(f);
    for (f->loop = 0; f->loop <= 7; f->loop++) {
        f->frame++;
        WAIT(1);
    }
    f->frame++;
    WAIT(4);
    SetFrame(f, 97);
    WAIT(4);
    Dust(f);
    for (f->loop = 0; f->loop <= 7; f->loop++) {
        f->frame--;
        WAIT(1);
    }
    SetFrame(f, 88);
    WAIT(10);
    SET_STATE(21);
    SCRIPT_END;
}

// State21(); The third slash (0x080A69E4): 105 3 frames, 106 4, 107-109 2 each, 110 20, standing (60) 48, then back to his side
// (state 5);
static void State21(struct Fight *f) {
    SCRIPT;
    f->state = 21;
    Stop(f);
    f->g1C = -1;
    SetFrame(f, 105);
    WAIT(3);
    f->frame = 106;
    WAIT(4);
    Dust(f);
    f->frame = 107;
    WAIT(2);
    f->frame = 108;
    WAIT(2);
    f->frame = 109;
    WAIT(2);
    f->frame = 110;
    WAIT(20);
    f->boxSet = 0;
    SetFrame(f, 60);
    WAIT(48);
    SET_STATE(5);
    SCRIPT_END;
}

// State22(); A hit guarded (0x080A6B3C): the spark, knocked back (Frame()); in the air 70 and 71 by turns 16 frames falling, else 69;
static void State22(struct Fight *f) {
    struct Object *obj = f->obj;

    SCRIPT;
    f->state = 22;
    f->g18 = f->landed;
    f->g1C = 20;
    FaceKirby(f);
    PlaySfx(&obj->base, SE_DMK_PARRY);
    if (!f->landed) {
        Effect(f, FX_SPARK, obj->base.x + f->facing * 4 * 0x100, obj->base.y - 4 * 0x100, 1);
        f->ay = 0x15;
        f->maxY = 0x300;
        for (f->loop = 0; f->loop <= 3; f->loop++) {
            SetFrame(f, 70);
            WAIT(2);
            f->frame = 71;
            WAIT(2);
        }
    } else {
        Effect(f, FX_SPARK, obj->base.x + f->facing * 16 * 0x100, obj->base.y + 4 * 0x100, 1);
        Stop(f);
        SetFrame(f, 69);
    }
    SCRIPT_END;
}

// State23(); Inhaled (0x080A6D08): holding out (69) 60 frames while the Kirby inhales him (Frame()), then free: a run or a leap;
static void State23(struct Fight *f) {
    struct Object *obj = f->obj;

    SCRIPT;
    f->state = 23;
    Stop(f);
    f->g18 = f->landed;
    if (!f->landed) {
        f->ay = 0x15;
        f->maxY = 0x300;
    }
    f->g1C = 60;
    FaceKirby(f);
    PlaySfx(&obj->base, SE_DMK_PARRY);
    SetFrame(f, 69);
    SCRIPT_END;
}

static void (*const sStates[24])(struct Fight *) = {
    State0, State1, State2, State3, State4, State5, State6, State7, State8, State9, State10, State11,
    State12, State13, State14, State15, State16, State17, State18, State19, State20, State21, State22, State23,
};

/* His states' functions each frame, after his script and his move (0x08748F1C); */

// Frame1(); Walking by Kirby (0x080A5AF4): to his spot beside him (running far off), then keeping it as Kirby moves (with Kirby's
// steps, his D-pad's: 15 one way and he walks again), stopped when he's at a wall or Kirby is still; near Kirby, a move; his countdown
// out, another (Choose());
static void Frame1(struct Fight *f) {
    struct Object *obj = f->obj;
    struct Kirby *k = f->target;
    s32 dx, kx;
    u16 keys;

    if (--f->timer == 0) {
        Choose(f);
        return;
    }
    if (k == NULL) return;
    kx = KirbyX(k);
    dx = kx + f->offset - X(f);
    switch (f->step) {
    case 0:
        if (f->walkFlags & 2) f->facing = dx < 0 ? -1 : 1;
        else FaceKirby(f);
        if (abs(dx) <= 1) {
            f->step = 1;
            f->vx = 0;
            f->g1C = 0;
        } else if (abs(dx) <= 27) {
            f->walk &= ~2;
            f->vx = dx > 0 ? 0x100 : -0x100;
        } else {
            f->walk |= 2;
            f->vx = dx > 0 ? 0x200 : -0x200;
        }
        f->g18 = kx;
        break;
    case 1:
        f->walkFlags &= ~6;
        if (Walled(f, X(f), Y(f))) {
            f->step = 2;
            f->vx = 0;
            f->g18 = kx;
            break;
        }
        obj->base.x = (kx + f->offset) << 8;
        FaceKirby(f);
        keys = KirbyKeys(k);
        if ((keys & 0x30) && kx != f->g18) {
            if (keys & 0x10) {
                if (f->g1C < 0) f->g1C = 0;
                f->g1C++;
            } else {
                if (f->g1C > 0) f->g1C = 0;
                f->g1C--;
            }
            if (abs(f->g1C) == 15) f->step = 0;
        } else if (k->base.xspeed == 0) {
            f->step = 2;
            f->vx = 0;
        }
        f->g18 = kx;
        break;
    case 2:
        FaceKirby(f);
        if (kx != f->g18 && !Walled(f, kx + f->offset, Y(f))) {
            f->step = 0;
            f->walk &= ~1;
            break;
        }
        if (f->frame == 65) f->walk |= 1;
        break;
    }
    if (AtEdge(f)) {
        f->vx = 0;
        f->step = 2;
        if (!(f->walkFlags & 1)) {
            f->walkFlags |= 1;
            f->timer = Rand(f, 30) + 30;
        }
    }
    // (his sprites facing his way: NiDL's 0x08006304, each frame walking by Kirby);
    f->flip = f->facing;
    if (!(f->walkFlags & 6) && abs(DxKirby(f)) <= 31) {
        f->next = sNearMoves[OnGround(k)][Rand(f, 8)];
        Begin(f, TRUE);
    }
}

// Frame6(); Where Kirby was, reached (0x080A6020: within 5): a move by Kirby's height, his side of Kirby next (by turns: state 3's);
static void Frame6(struct Fight *f) {
    Edges(f);
    if (abs(X(f) - f->g1C) > 5) return;
    f->next = sReachMoves[abs(DyKirby(f)) <= 47 ? 0 : 1][Rand(f, 16)];
    if (f->next == 3) f->g18++;
    f->offset = sSides[f->g18];
    f->walkFlags = 2;
    Begin(f, TRUE);
}

// Frame7(); Kirby reached running (0x080A6130: within 43): one slash, two or three, by his HP;
static void Frame7(struct Fight *f) {
    Edges(f);
    if (abs(DxKirby(f)) > 43) return;
    f->walkFlags = 0;
    f->next = sRunMoves[f->obj->unk80 >= f->maxHp / 2 ? 0 : 1][Rand(f, 8)];
    Begin(f, TRUE);
}

/* AirMove(); Falling from a leap (0x080A6FB4): back from the screen's edge; once falling, g18's move, or one by where Kirby is
 * (sAirMoves); */
static void AirMove(struct Fight *f) {
    s32 dx;
    u8 row;

    if (AtEdge(f)) TurnAround(f);
    if (f->g1C != 0 || f->landed || f->vy <= 0) return;
    if (f->g18 == 0) {
        f->walkFlags = 0;
        f->g1C++;
        dx = DxKirby(f);
        if (DyKirby(f) < 0) row = 0;
        else if (abs(dx) <= 15) row = 1;
        else if ((dx > 0 && f->facing == 1) || (dx < 0 && f->facing == -1)) row = 2;
        else row = 3;
        f->g18 = sAirMoves[row][Rand(f, 4)];
        if (f->g18 == 0) return;
    }
    f->next = f->g18;
}

// Lunge(); A slash's lunge (states 19, 20: 0x080A680C): 2 pixels a frame 8 frames, 1 8, then still;
static void Lunge(struct Fight *f) {
    Edges(f);
    if (f->g1C > 15) return;
    if (++f->g1C == 16) f->vx = 0;
    else VelX(f, sLunge[f->g1C >> 3]);
}

/* Frame(); His state's function each frame (0x08748F1C), after his script and his move: kept on the screen, a new state started as
 * NiDL's does (Begin()); */
static void Frame(struct Fight *f) {
    u8 i;

    switch (f->state) {
    case 0:
        if (f->next != 0) Begin(f, FALSE);
        return;
    case 1:
        Frame1(f);
        return;
    case 2: case 3: case 4: case 5:
        Edges(f);
        if (f->next != f->state) Begin(f, FALSE);
        return;
    case 6:
        Frame6(f);
        return;
    case 7:
        Frame7(f);
        return;
    case 8: case 9:
        AirMove(f);
        break;
    case 13: case 17:
        if (AtEdge(f)) TurnAround(f);
        if (f->landed) {
            Stop(f);
            f->next = 10;
            Begin(f, TRUE);
        }
        return;
    case 15:
        if (AtEdge(f)) TurnAround(f);
        break;
    case 18:
        Edges(f);
        break;
    case 19: case 20:
        Lunge(f);
        break;
    case 21:
        Edges(f);
        if (f->g1C <= 25) {
            f->g1C++;
            for (i = 0; i < 3 && f->g1C >= sThirdSteps[i]; i++) ;
            VelX(f, sThird[i]);
        }
        break;
    case 22:
        Edges(f);
        if (f->g1C == 0) {
            f->boxSet = 0;
            if (!f->landed) return;
            Stop(f);
            if (f->g18 == 0) {
                f->next = 10;
                Begin(f, FALSE);
            } else {
                f->step = 0;
                Choose(f);
            }
            return;
        }
        f->g1C--;
        for (i = 0; i < 6 && f->g1C >= sGuardSteps[i]; i++) ;
        VelX(f, sGuardSpeeds[i]);
        if (f->landed) {
            StopY(f);
            f->g18 = f->landed;
            SetFrame(f, 69);
        }
        return;
    case 23:
        Edges(f);
        if (f->target && NidlInhaler(&f->obj->base) == f->target) {
            if (!f->landed) return;
            if (f->g18 == 0) {
                StopY(f);
                f->g18 = f->landed;
            }
            if (--f->g1C == 0) {
                f->boxSet = 1;
                f->next = sFreeMoves[Rand(f, 2)];
                Begin(f, FALSE);
            }
            return;
        }
        if (f->landed) {
            if (f->g18 == 0) {
                StopY(f);
                f->next = 10;
            } else {
                f->next = 2;
            }
            Begin(f, FALSE);
        } else {
            SetFrame(f, 70);
        }
        return;
    default:
        break;
    }
    if (f->next != f->state) Begin(f, TRUE);
}

/* Move(); His move (NiDL's 0x080656B4: his speed plus its change, capped (across either way, falling), then his spot plus his speed),
 * the room's floor and walls (KatAM's, his bounds: NiDL's terrain box) after it, on the ground or not (NiDL's +0x7A: set as he lands,
 * kept while he stands); landing once his whole pixel is into the floor, in the middle of his pixel, as NiDL's floor puts him (KatAM's
 * 1/256 of a pixel under the next); a wall met, NiDL's handler
 * (0x080A7334): walking, he stops by Kirby; in the air, he turns back; */
static void Move(struct Fight *f) {
    struct Object *obj = f->obj;
    s32 y;
    u8 frac, walls;

    f->vx += f->ax;
    f->vy += f->ay;
    if (f->maxX != NO_MAX) {
        if (f->vx > f->maxX) f->vx = f->maxX;
        if (f->vx < -f->maxX) f->vx = -f->maxX;
    }
    // (falling only: NiDL caps no rise);
    if (f->maxY != NO_MAX && f->vy > f->maxY) f->vy = f->maxY;
    obj->base.x += f->vx;
    obj->base.y += f->vy;
    obj->base.xspeed = obj->base.yspeed = 0;
    // (his intro on his spot: NiDL's ledge there, none of the room's floors or walls in his way);
    if (f->state == 0) return;
    // (checked as KatAM rests things on its floor, 1/256 of a pixel over it: on it, or into it as NiDL's whole pixels go);
    frac = obj->base.y & 0xFF;
    y = obj->base.y |= 0xFF;
    sub_0809D8C8(&obj->base);
    // (NiDL's landing: into the floor, falling; his flag stays while he's on it, off in the air or rising);
    if (obj->base.y < y && (obj->base.unk62 & 4) && f->vy >= 0) {
        obj->base.y = (obj->base.y & ~0xFF) | 0x80;
        f->landed = TRUE;
    } else {
        obj->base.y = (obj->base.y & ~0xFF) | frac;
        if (!(obj->base.unk62 & 4) || f->vy < 0) f->landed = FALSE;
    }
    if (f->landed) f->floorY = obj->base.y;
    walls = obj->base.unk62 & 3;
    if (walls && !f->walls) {
        if (f->state == 1) {
            f->step = 1;
            f->vx = 0;
        } else if (f->state == 8 || f->state == 13 || f->state == 15 || f->state == 17) {
            TurnAround(f);
        }
    }
    f->walls = walls;
}

/* Boxes(); His box (NiDL's 0x08749598, 0x087495B4, the same box: 0 and 1; 2 and none, 0x087495D0: not hit, not touching); */
static void Boxes(struct Fight *f) {
    struct Object *obj = f->obj;

    if (f->boxSet == 0 || f->boxSet == 1) {
        obj->base.flags &= ~0x200;
        sub_0803E2B0(&obj->base, -12, -10, 12, 12);
    } else {
        obj->base.flags |= 0x200;
    }
}

/* Mosaic(); The room's layers mosaic (NiDL's hurt: BG0 and BG1 there, KatAM's room layers here: MOSAIC_LAYERS) `level` (REG_MOSAIC's);
 * off; */
static void Mosaic(u8 level, bool32 on) {
    u8 i;

    *(vu16 *)REG_MOSAIC_ADDR = level;
    for (i = 0; i < 4; i++) {
        if (!(MOSAIC_LAYERS & (1 << i))) continue;
        if (on) gBgCntRegs[i] |= 0x40;
        else gBgCntRegs[i] &= ~0x40;
    }
}

/* Hurt(); A step of his hurt (NiDL's 0x080662D8, his handler 0x080A7260 and 0x080A72B0, by its countdown: NiDL's 0x02006190 + 0xC),
 * frozen: flashing in his hurt colors 2 frames in 4 (Colors()), jittering (sJitter), the room's layers mosaic (sMosaic); out, back
 * where he was hit, as he was, still frozen this frame; */
static void Hurt(struct Fight *f) {
    struct Object *obj = f->obj;
    u8 i = sJitterOf[f->hurt];

    f->flash = !(f->hurt & 2);
    obj->base.x += sJitter[i][0] * 0x100;
    obj->base.y += sJitter[i][1] * 0x100;
    f->hurt--;
    if (gKirbys[gLocalPlayerId].base.roomId == obj->base.roomId) Mosaic(sMosaic[f->hurt >> 1], f->hurt != 0);
    if (f->hurt == 0) {
        obj->base.x = f->hurtX;
        obj->base.y = f->hurtY;
        f->frame = f->hurtFrame;
    }
}

/* Hit(); A hit took HP off him (KatAM's, between frames; NiDL's checked after its main's collisions): in a move, he may guard it
 * (0x080A720C: a chance in 16 by his HP's quarter; its HP back, state 22 now); else hurt (NiDL's handler, 0x080A7260: its first step
 * as the hit's frame ends, KatAM's hit a frame later: Hurt() twice this frame). Hurt already, the HP's gone, nothing more (NiDL's hurt
 * runs instead of his main); his box off (Boxes()), none; */
static void Hit(struct Fight *f) {
    struct Object *obj = f->obj;
    bool32 guard = FALSE;

    if (f->hurt) return;
    // (his box off, NiDL's takes no hits: its HP back);
    if (f->boxSet != 0 && f->boxSet != 1) {
        obj->unk80 = f->hp;
        return;
    }
    if (f->guard & 0x80) f->guard++;
    if (f->guard & 3) {
        f->guard--;
        guard = Rand(f, 16) < sGuardChances[HpQuarter(f)];
    }
    if (guard) {
        obj->unk80 = f->hp;
        f->boxSet = 2;
        f->guard |= 0x80;
        f->state = f->next = 22;
        f->pc = 0;
        f->wait = 0;
        f->restart = FALSE;
        return;
    }
    f->guard |= 2;
    f->hurtX = obj->base.x;
    f->hurtY = obj->base.y;
    f->hurtFrame = f->frame;
    f->hurt = HURT_TIME;
    Hurt(f);
}

/* Colors(); His body's palette this frame: his hurt flash's colors 2 frames in 4 hurt (NiDL's, by the hurt's countdown), else his own;
 * in gObjPalette and the copy KatAM puts back after a flash; */
void sub_0803D280(u8, u8);

static void Colors(struct Fight *f) {
    const u16 *colors = f->flash ? gNidlMetaKnightFight.flash : gNidlMetaKnightFight.body;
    u16 *slot;
    u8 i;

    if (gKirbys[gLocalPlayerId].base.roomId != f->obj->base.roomId || !f->pal[PAL_BODY]) return;
    slot = &gObjPalette[f->pal[PAL_BODY] * 16];
    if (slot[1] == colors[1] && slot[6] == colors[6]) return;
    for (i = 0; i < 16; i++) slot[i] = colors[i];
    sub_0803D280(f->pal[PAL_BODY] * 16, 16);
    gMainFlags |= MAIN_FLAG_OBJ_PALETTE_SYNC_ENABLE;
}

// Palette(); A palette's slot, loaded under `key` as the frame `frame` of his anim has it, or 0 (the local player isn't in the room);
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
    u16 animId = gNidlData.bosses[NIDL_META_KNIGHT].animId, room = f->obj->base.roomId;

    if (!f->pal[PAL_BODY]) f->pal[PAL_BODY] = Palette(room, animId, 0);
    if (!f->pal[PAL_SWORD]) f->pal[PAL_SWORD] = Palette(room, animId | KEY_SWORD, gNidlMetaKnightFight.front[60]);
    if (!f->pal[PAL_SPARK]) f->pal[PAL_SPARK] = Palette(room, animId | KEY_SPARK, gNidlMetaKnightFight.spark);
    if (!f->pal[PAL_DUST]) f->pal[PAL_DUST] = Palette(room, animId | KEY_DUST, gNidlMetaKnightFight.dust);
}

static void SpriteMain(void) {
    struct EffectObject *e = TaskGetStructPtr(gCurTask);

    if (e->flags & 0x1000) TaskDestroy(gCurTask);
}

// Sprite(); A sprite of his (`tiles` tiles of VRAM, frame `frame` first): a task of its own (KatAM frees its VRAM with it);
static struct EffectObject *Sprite(u16 room, u16 tiles, u8 frame, u8 order) {
    struct Task *t = TaskCreate(SpriteMain, sizeof(struct EffectObject), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct EffectObject *e = TaskGetStructPtr(t);

    ClearEffectObject(e);
    e->header.kind = 3;
    e->roomId = room;
    e->parent = NULL;
    e->flags |= 0x400;
    EffectObjectInitSprite(e, &e->sprite, tiles, gNidlData.bosses[NIDL_META_KNIGHT].animId, frame, order);
    return e;
}

/* Draw(); A sprite of his at (x, y) (8.8), frame `frame`, palette `palId`, facing as NiDL draws it: its frames as they are facing right,
 * mirrored facing left (KatAMExpansion.cs builds them as NiDL has them, which KatAM draws so with flag 1); */
static void Draw(struct EffectObject *e, s32 x, s32 y, u8 frame, u8 palId, s8 facing) {
    e->x = x;
    e->y = y;
    e->sprite.variant = frame;
    e->sprite.palId = palId;
    e->flags = (e->flags & ~0x401) | 4 | (facing == -1 ? 0 : 1);
    e->objBase55 = -1;
    EffectObjectPostUpdate(e);
}

static void Hide(struct EffectObject *e) {
    if (e) e->flags |= 0x400;
}

static void Release(struct EffectObject **e) {
    if (*e == NULL) return;
    (*e)->flags |= 0x1000;
    *e = NULL;
}

static void MetaKnightMain(struct Object *obj);

/* DrawFight(); His three sprites at his spot: his body, Galaxia's pieces in front of it and behind it (gNidlMetaKnightFight's), for
 * NiDL's frame `frame`; */
static void DrawFight(struct Fight *f, s16 frame) {
    static const u8 orders[PARTS] = { BODY_ORDER, SWORD_ORDER, BACK_ORDER };
    struct Object *obj = f->obj;
    u8 i, k;

    for (i = 0; i < PARTS; i++) {
        if (frame < 0 || frame >= NIDL_FRAMES) k = FRAME_NONE;
        else if (i == PART_BODY) k = BODY(frame);
        else k = i == PART_FRONT ? gNidlMetaKnightFight.front[frame] : gNidlMetaKnightFight.back[frame];
        if (k == FRAME_NONE) {
            Hide(f->parts[i]);
            continue;
        }
        if (f->parts[i] == NULL) f->parts[i] = Sprite(obj->base.roomId, gNidlMetaKnightFight.tiles[i], k, orders[i]);
        Draw(f->parts[i], obj->base.x, obj->base.y, k, f->pal[i == PART_BODY ? PAL_BODY : PAL_SWORD], f->flip);
    }
}

static void MetaKnightEnd(struct Object *obj, s32 floorY);

/* FightMain(); His duel's task: his sprites drawn where he is (as KatAM's explosion takes him, NiDL's 0: masked, Galaxia gone), his
 * palettes; gone with him (his defeat goes on, MetaKnightEnd()); */
static void FightMain(void) {
    struct Fight *f = TaskGetStructPtr(gCurTask);
    struct Object *obj = f->obj;
    u8 i;

    if (obj == NULL || (obj->base.flags & 0x1000)) {
        for (i = 0; i < PARTS; i++) Release(&f->parts[i]);
        if (f->sword) f->sword->flags |= 0x1000;
        f->sword = NULL;
        ReleaseStars(f);
        if (obj) ((struct MetaKnight *)obj)->f = NULL;
        TaskDestroy(gCurTask);
        return;
    }
    Palettes(f);
    if (obj->unk78 != MetaKnightMain) {
        ReleaseStars(f);
        if (f->hurt && gKirbys[gLocalPlayerId].base.roomId == obj->base.roomId) Mosaic(0, FALSE);
        f->hurt = 0;
        f->flash = FALSE;
        Colors(f);
        DrawFight(f, 0);
        return;
    }
    Colors(f);
    DrawFight(f, f->frame);
}

/* SwordMain(); Galaxia's box (NiDL's companion, type 0xB8): an attack object on him (as Mr. Shine & Mr. Bright's), hurting Kirby in his
 * frames 88-121 that have one (sSwordBoxes, their spot from his: facing right, mirrored facing left); */
static void SwordMain(void) {
    struct ObjectBase *sword = TaskGetStructPtr(gCurTask);
    struct Fight *f = sword->parent;
    const struct SwordBox *box;
    struct Object *obj;

    if (sword->flags & 0x1000) {
        TaskDestroy(gCurTask);
        return;
    }
    if (ObjectPreUpdate(sword)) return;
    obj = f->obj;
    sword->flags |= 4 | 0x400;
    box = f->frame >= 88 && f->frame <= 121 ? &sSwordBoxes[f->frame - 88] : NULL;
    if (obj == NULL || obj->unk78 != MetaKnightMain || box == NULL || (!box->box[0] && !box->box[2])) {
        sword->flags |= 0x200;
        return;
    }
    sword->flags &= ~0x200;
    sword->x = obj->base.x + f->facing * box->x * 0x100;
    sword->y = obj->base.y + box->y * 0x100;
    if (f->facing == -1) sub_0803E2B0(sword, -box->box[2], box->box[1], -box->box[0], box->box[3]);
    else sub_0803E2B0(sword, box->box[0], box->box[1], box->box[2], box->box[3]);
    SetPointerSomething(sword);
}

static struct ObjectBase *Sword(struct Fight *f) {
    struct Object *obj = f->obj;
    struct Task *t = TaskCreate(SwordMain, sizeof(struct ObjectBase), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct ObjectBase *sword = TaskGetStructPtr(t);

    ClearObjectBase(sword);
    sword->header.kind = 2;
    sword->x = obj->base.x;
    sword->y = obj->base.y;
    sword->parent = f;
    sword->roomId = obj->base.roomId;
    sword->unk56 = obj->base.unk56;
    sword->unk63 = 1;
    sword->flags |= 0x10000140 | 0x600;
    sword->unk68 = 0x20000043;
    sword->unk5C |= 0x80000;
    ObjectSetBounds(sword, 0, 0, 0, 0);
    return sword;
}

/* His effects (NiDL's objects, each its frames and moves, drawn from his anim's effect frames): his cape (type 0xB9: off right as he
 * draws Galaxia, rising, blinking out), the spark (0xBC: as he draws it, as he guards), the dust behind his slashes (0x91, NiDL's
 * shared: off back, rising, its frames facing as it goes), his thrust's dust (0xA9: a puff, and two clouds off either way), his mask
 * (0xBA: up, then in two halves, 0xBB, flying off falling); */
struct Fx {
    struct EffectObject e;
    u8 kind, which, step, left;
    s32 x, y;                           // 8.8 (the room's);
    s16 vx, vy, ax, ay;
    s8 facing;
    s16 frame;                          // His anim's (FRAME_HIDDEN: none);
    u8 pal;
};

// Their steps (frame from their first, frames shown; FX_OFF hidden, FX_GONE their end);
#define FX_GONE 0xFE
#define FX_OFF 0xFF
static const u8 sCapeSteps[][2] = {
    { 0, 3 }, { 1, 3 }, { 2, 3 }, { 3, 3 }, { 0, 2 }, { FX_OFF, 1 }, { 1, 2 }, { FX_OFF, 1 }, { 2, 1 }, { FX_OFF, 1 }, { 2, 1 }, { FX_OFF, 1 },
    { 3, 1 }, { FX_OFF, 1 }, { 0, 1 }, { FX_OFF, 2 }, { 1, 1 }, { FX_OFF, 2 }, { FX_GONE, 0 },
};
static const u8 sSparkSteps[][2] = { { 0, 1 }, { 1, 1 }, { 2, 1 }, { 3, 1 }, { 4, 1 }, { 5, 1 }, { FX_GONE, 0 } };
static const u8 sDustSteps[][2] = { { 0, 2 }, { 2, 2 }, { 4, 2 }, { 6, 1 }, { FX_GONE, 0 } };
static const u8 sLandingSteps[][2] = { { 16, 1 }, { 17, 2 }, { 18, 2 }, { 19, 2 }, { 20, 2 }, { 21, 2 }, { 22, 2 }, { 23, 2 }, { FX_GONE, 0 } };
static const u8 sCloudSteps[][2] = { { 0, 1 }, { 2, 1 }, { 4, 1 }, { 4, 2 }, { 6, 2 }, { 8, 2 }, { 10, 2 }, { 12, 1 }, { 14, 1 }, { FX_GONE, 0 } };
static const u8 sMaskSteps[][2] = { { 0, 3 }, { FX_GONE, 0 } };

static void FxMain(void);

static void FxCreate(u16 room, u8 kind, s32 x, s32 y, s8 facing, u8 which, u8 pal) {
    struct Task *t = TaskCreate(FxMain, sizeof(struct Fx), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct Fx *fx = TaskGetStructPtr(t);
    const struct NidlMetaKnightFight *g = &gNidlMetaKnightFight;
    u8 first;

    if (kind == FX_SPARK) first = g->spark;
    else if (kind == FX_DUST) first = g->dust;
    else if (kind == FX_LANDING || kind == FX_LANDING_LEFT || kind == FX_LANDING_RIGHT) first = g->landing;
    else if (kind == FX_CAPE) first = g->cape;
    else first = g->mask;
    ClearEffectObject(&fx->e);
    fx->e.header.kind = 3;
    fx->e.roomId = room;
    fx->e.parent = NULL;
    fx->e.flags |= 0x400;
    EffectObjectInitSprite(&fx->e, &fx->e.sprite, g->tiles[3], gNidlData.bosses[NIDL_META_KNIGHT].animId, first,
                           kind == FX_CAPE ? CAPE_ORDER : FRONT_ORDER);
    fx->kind = kind;
    fx->which = which;
    fx->step = fx->left = 0;
    fx->x = x;
    fx->y = y;
    fx->vx = fx->vy = fx->ax = fx->ay = 0;
    fx->facing = facing;
    fx->frame = FRAME_HIDDEN;
    fx->pal = pal;
    switch (kind) {
    case FX_CAPE:
        fx->vx = 0x80;
        fx->vy = -0xC0;
        fx->ay = 0x10;
        break;
    case FX_DUST:
        fx->vx = facing == 1 ? -0x240 : 0x240;
        fx->ax = facing == 1 ? 0x18 : -0x18;
        fx->vy = -0x40;
        fx->ay = -0x20;
        break;
    case FX_LANDING_LEFT:
        fx->vx = -0x600;
        break;
    case FX_LANDING_RIGHT:
        fx->vx = 0x600;
        break;
    case FX_MASK:
        fx->vy = -0x100;
        fx->ay = -0x100;
        break;
    case FX_MASK_HALF:
        fx->vx = (which ? 0xC0 : -0xC0) * facing;
        fx->vy = -0x1F8;
        fx->ay = 0x18;
        break;
    }
}

/* FxStep(); An effect's next step: its frame for its frames, what it does then (the thrust's clouds, slowing; the mask's halves); FALSE:
 * done; */
static bool32 FxStep(struct Fx *fx) {
    const struct NidlMetaKnightFight *g = &gNidlMetaKnightFight;
    const u8 (*steps)[2];
    u8 first;

    switch (fx->kind) {
    case FX_CAPE:
        steps = sCapeSteps;
        first = g->cape;
        break;
    case FX_SPARK:
        steps = sSparkSteps;
        first = g->spark;
        break;
    case FX_DUST:
        steps = sDustSteps;
        first = g->dust + (fx->facing == -1 ? 1 : 0);
        break;
    case FX_LANDING:
        steps = sLandingSteps;
        first = g->landing;
        break;
    case FX_LANDING_LEFT: case FX_LANDING_RIGHT:
        steps = sCloudSteps;
        first = g->landing + (fx->kind == FX_LANDING_RIGHT ? 1 : 0);
        break;
    case FX_MASK:
        steps = sMaskSteps;
        first = g->mask;
        break;
    default:
        // (the halves: 31-34 or 27-30, 2 frames each, five times);
        if (fx->step >= 20) return FALSE;
        fx->frame = g->mask + (fx->which ? 1 : 5) + (fx->step & 3);
        fx->left = 2;
        fx->step++;
        return TRUE;
    }
    if (steps[fx->step][0] == FX_GONE) {
        if (fx->kind == FX_MASK) {
            FxCreate(fx->e.roomId, FX_MASK_HALF, fx->x, fx->y, fx->facing, 0, fx->pal);
            FxCreate(fx->e.roomId, FX_MASK_HALF, fx->x, fx->y, fx->facing, 1, fx->pal);
        }
        return FALSE;
    }
    fx->frame = steps[fx->step][0] == FX_OFF ? FRAME_HIDDEN : first + steps[fx->step][0];
    fx->left = steps[fx->step][1];
    if (fx->kind == FX_LANDING && fx->step == 0) {
        FxCreate(fx->e.roomId, FX_LANDING_LEFT, fx->x, fx->y, 1, 0, fx->pal);
        FxCreate(fx->e.roomId, FX_LANDING_RIGHT, fx->x, fx->y, 1, 0, fx->pal);
    }
    if ((fx->kind == FX_LANDING_LEFT || fx->kind == FX_LANDING_RIGHT) && fx->step == 3) {
        fx->vx = fx->kind == FX_LANDING_LEFT ? -0x200 : 0x200;
        fx->ax = fx->kind == FX_LANDING_LEFT ? 0x10 : -0x10;
    }
    fx->step++;
    return TRUE;
}

static void FxMain(void) {
    struct Fx *fx = TaskGetStructPtr(gCurTask);

    if (fx->e.flags & 0x1000) {
        TaskDestroy(gCurTask);
        return;
    }
    if (fx->left == 0 || --fx->left == 0) {
        if (!FxStep(fx)) {
            fx->e.flags |= 0x1000;
            return;
        }
    }
    fx->vx += fx->ax;
    fx->vy += fx->ay;
    fx->x += fx->vx;
    fx->y += fx->vy;
    if (fx->frame == FRAME_HIDDEN) fx->e.flags |= 0x400;
    else Draw(&fx->e, fx->x, fx->y, fx->frame, fx->pal, fx->kind == FX_MASK || fx->kind == FX_MASK_HALF ? fx->facing : 1);
}

// Effect(); One of his effects, at (x, y) (8.8), facing as it's spawned (NiDL's 0x08064C1C);
static void Effect(struct Fight *f, u8 kind, s32 x, s32 y, s8 facing) {
    u8 pal;

    if (gKirbys[gLocalPlayerId].base.roomId != f->obj->base.roomId) return;
    if (kind == FX_SPARK) pal = f->pal[PAL_SPARK];
    else if (kind == FX_DUST || kind == FX_LANDING) pal = f->pal[PAL_DUST];
    else pal = f->pal[PAL_BODY];
    FxCreate(f->obj->base.roomId, kind, x, y, facing, 0, pal);
}

/* His stars (not NiDL's: NiDL throws Kirby a sword for the duel; here Kirby fights with his own powers, the owner's, and with none he
 * had nothing to hit him with): his slashes and his thrust into the ground knock stars out, as King Dedede's landings and his
 * hammer do (bosses/dedede.c's Stars(): KatAM's star enemy, enemies/star.c, which Kirby inhales and spits back at him as a star),
 * moving as King Dedede's (NiDL's type 0x8B, 0x08067258): out 2 pixels a frame his way and up 1.5, 6 frames; then 0.5 and 0.5, 12
 * frames; still, twinkling, till its 75th frame, gone. Each step: its frame (King Dedede's star's: 4-13), its frames, its speed (1:
 * slower, 2: still); */
static const u8 sStarSteps[][3] = {
    { 13, 2, 0 }, { 12, 2, 0 }, { 11, 2, 0 },
    { 8, 1, 1 }, { 4, 3, 0 }, { 10, 1, 0 }, { 5, 3, 0 }, { 9, 1, 0 }, { 6, 3, 0 }, { 10, 1, 2 }, { 7, 3, 0 },
    { 8, 1, 0 }, { 4, 3, 0 }, { 10, 1, 0 }, { 5, 3, 0 }, { 9, 1, 0 }, { 6, 3, 0 }, { 10, 1, 0 }, { 7, 3, 0 },
    { 8, 1, 0 }, { 4, 3, 0 }, { 10, 1, 0 }, { 5, 3, 0 }, { 9, 1, 0 }, { 6, 3, 0 }, { 10, 1, 0 }, { 7, 3, 0 },
    { 8, 1, 0 }, { 4, 3, 0 }, { 10, 1, 0 }, { 5, 3, 0 }, { 9, 1, 0 }, { 6, 3, 0 }, { 10, 1, 0 }, { 7, 3, 0 },
    { 11, 2, 0 }, { 12, 2, 0 }, { 13, 1, 0 },
};
#define STAR_STEPS (sizeof(sStarSteps) / sizeof(sStarSteps[0]))

// StarSet(); A star's step `step` starts: its frame, its frames, its speed;
static void StarSet(struct Fight *f, struct FightStar *s, u8 step) {
    s->step = step;
    s->wait = sStarSteps[step][1];
    if (sStarSteps[step][2] == 1) {
        s->vx = s->vx < 0 ? -0x80 : 0x80;
        s->vy = -0x80;
    } else if (sStarSteps[step][2] == 2) {
        s->vx = s->vy = 0;
    }
    NidlSetFrame((struct NidlObject *)s->obj, STAR_FRAMES + sStarSteps[step][0] - 4);
}

// Star(); A star out of his attack at (x, y) (pixels), going `facing`'s way (none when all STARS are out, or past the screen's sides);
static void Star(struct Fight *f, s32 x, s32 y, s8 facing) {
    struct Object *obj = f->obj, *star;
    struct FightStar *s;
    u8 i;

    if (x < ScreenX(obj) + WALL_X || x > ScreenX(obj) + 240 - WALL_X) return;
    for (i = 0; i < STARS && f->stars[i].obj; i++) ;
    if (i >= STARS) return;
    s = &f->stars[i];
    star = CreateObjTemplateAndObj(obj->base.unk56, 1, 0x24, x, y, 0, 0x1F, 0, 0, NIDL_TYPE, NIDL_SPECIAL << 4 | 2, 0, 0, 0, 0, 0, 0, 0,
                                   0, 0, 0, 0, 0);
    if (star == NULL) return;
    // (KatAM's despawn off the screen not its while the fight drives it);
    star->base.flags |= 0x2000000;
    s->obj = star;
    s->x = x << 8;
    s->y = y << 8;
    s->vx = facing == -1 ? -0x200 : 0x200;
    s->vy = -0x180;
    StarSet(f, s, 0);
}

// SlashStar(); A slash's star, STAR_AHEAD ahead of him his way, on the floor;
static void SlashStar(struct Fight *f) {
    Star(f, X(f) + f->facing * STAR_AHEAD, Y(f) + STAR_DOWN, f->facing);
}

// DriveStars(); His stars each frame: gone or inhaled, let go; else their step, their move, their frame and palette (his dust's: NiDL's
// shared palette 15, as King Dedede's);
static void DriveStars(struct Fight *f) {
    u8 i;

    for (i = 0; i < STARS; i++) {
        struct FightStar *s = &f->stars[i];
        struct Object *star = s->obj;

        if (star == NULL) continue;
        if ((star->base.flags & 0x1000) || (star->base.unkC & 0x400)) {
            s->obj = NULL;
            continue;
        }
        if (s->wait && !--s->wait) {
            if (s->step + 1 >= STAR_STEPS) {
                star->base.flags |= 0x1000;
                s->obj = NULL;
                continue;
            }
            StarSet(f, s, s->step + 1);
        }
        s->x += s->vx;
        s->y += s->vy;
        star->base.x = s->x;
        star->base.y = s->y;
        star->base.xspeed = star->base.yspeed = 0;
        star->base.sprite.palId = f->pal[PAL_DUST];
        star->base.flags |= 1;
    }
}

// ReleaseStars(); His stars gone (his duel over), but those Kirby inhaled;
static void ReleaseStars(struct Fight *f) {
    u8 i;

    for (i = 0; i < STARS; i++) {
        struct Object *star = f->stars[i].obj;

        if (star && !(star->base.flags & 0x1000) && !(star->base.unkC & 0x400)) star->base.flags |= 0x1000;
        f->stars[i].obj = NULL;
    }
}

/* His duel (MetaKnightMain()): NiDL's main each frame (0x080A56B0): a hit taken first (Hit(): KatAM's come between frames); hurt, frozen
 * (Hurt()); else his script (WAIT() resumes it that many frames on), his move (Move()), his state's function (Frame()), his box
 * (Boxes()). His states:
 * • 1: walking at Kirby (frames 61-68; 2 pixels a frame far off, 1 near) to his spot 64 pixels beside him, then keeping it as Kirby
 *   steps (with Kirby's D-pad, KatAM's movementState), stopped when Kirby is (held on 65) or at a wall; within 31 pixels of Kirby, a
 *   move (sNearMoves, by whether Kirby's on the ground); his countdown out (48-101 frames, 96-179 under half his HP), another
 *   (sWalkMoves; sEdgeMoves stopped at the screen's edge);
 * • 2, 3, 5: to the other side of Kirby (2: the side he faces; 3: his own by turns; 5: back to his side, no move on the way), walking
 *   (1); 4: walking 48 frames more; 6: walking to where Kirby is, then a move by Kirby's height (sReachMoves);
 * • 7: running at Kirby (82-87, 1.75 pixels a frame), within 43 pixels one slash, two or three (sRunMoves, by his HP);
 * • 8, 9, 11, 12, 14, 16: leaps (crouched 20 frames, up 3.2 or 2.4 pixels a frame, 1.5 ahead or straight up), falling a move (13 his
 *   spin, 15 his thrust down, 17 his up-slash: sAirMoves, by where Kirby is), landed crouched (10);
 * • 13: his spin (114-121, twice, four times high up) falling; 15: his thrust down (111-113), the ground's dust, a star either way
 *   (Star()); 17, 18: his up-slash, in the air or not (98-104, a hop up);
 * • 19, 20, 21: one, two, three slashes, lunging (2 pixels a frame 8 frames, 1 8; the third 2, 1.5, 0.75), the dust behind him, a
 *   star ahead (SlashStar());
 * • 22: a hit guarded (in his moves: a chance in 16 by his HP's quarter, 2 at full HP to 5 under a quarter; its HP given back), the
 *   spark, knocked back; 23: inhaled, holding out 60 frames, then free (his box 1) running or leaping (sFreeMoves).
 * Hurt (not guarded), he freezes 13 frames, jittering, flashing in his hurt colors (Colors()), the room's layers mosaic (Hurt()). His
 * box (NiDL's: -12, -10 to 12, 12) hurts Kirby as KatAM's enemies do, his slashes Galaxia's (SwordMain()). 60 HP.
 * Not here: NiDL's hit kinds 6 (an inhale on his box 1, which only state 23 gives him: KatAM's bosses aren't inhaled) and 4 (on his box
 * 0: guarded, or his leap and thrust down, state 16), none of KatAM's attacks. NiDL's sounds are its own: KatAM's Dark Meta Knight's
 * play (his slashes, his spin, his guard, his thrust landing); */
static void MetaKnightMain(struct Object *obj) {
    struct MetaKnight *m = (struct MetaKnight *)obj;
    struct Fight *f = m->f;

    NidlUpdate(&m->n);
    obj->base.flags |= 4 | 0x400;
    obj->kirby3 = FindTargetKirby(&obj->base);
    // The HP bar goes to whatever is hit; he takes it back;
    sub_08034E14(obj);
    if (f == NULL) return;
    obj->base.xspeed = obj->base.yspeed = 0;
    f->flash = FALSE;
    if (obj->unk80 < f->hp) {
        NidlBossHitShake(obj);
        Hit(f);
    }
    f->hp = obj->unk80;
    if (f->hurt) {
        Hurt(f);
        Boxes(f);
        DriveStars(f);
        return;
    }
    if (f->restart) {
        f->restart = FALSE;
        f->state = f->next;
        f->pc = 0;
        f->wait = 0;
    }
    if (f->pc != PC_END && (!f->wait || !--f->wait)) sStates[f->state](f);
    Move(f);
    Frame(f);
    Boxes(f);
    DriveStars(f);
    // (Galaxia's box from his leap down on: NiDL's companion);
    if (f->sword == NULL && f->state != 0) f->sword = Sword(f);
}

// MetaKnightGone(); His object's gone (nidl.c's NidlDestroy()): his duel's task lets go of it;
void MetaKnightGone(struct Object *obj) {
    struct MetaKnight *m = (struct MetaKnight *)obj;

    if (m->f) m->f->obj = NULL;
    m->f = NULL;
}

// MetaKnightBeaten(); KatAM's explosion bursts him (nidl_boss.c's NidlBossDefeat()): his defeat, from where he was (to his floor);
void MetaKnightBeaten(struct Object *obj) {
    struct MetaKnight *m = (struct MetaKnight *)obj;

    MetaKnightEnd(obj, m->f ? m->f->floorY : obj->base.y);
}

// MetaKnightStart(); The duel starts (NidlBossStart(): Kirby in his area), his duel's task with it: his intro first (state 0);
void MetaKnightStart(struct Object *obj) {
    struct MetaKnight *m = (struct MetaKnight *)obj;
    struct Task *t;
    struct Fight *f;
    u8 i;

    ObjectSetFunc(obj, -1, MetaKnightMain);
    if (m->started) return;
    m->started = TRUE;
    t = TaskCreate(FightMain, sizeof(struct Fight), 0x3500, TASK_USE_EWRAM, NULL);
    f = m->f = TaskGetStructPtr(t);
    f->obj = obj;
    for (i = 0; i < PARTS; i++) f->parts[i] = NULL;
    f->sword = NULL;
    f->state = f->next = 0;
    f->pc = 0;
    f->wait = 0;
    f->restart = FALSE;
    f->facing = f->flip = 1;
    f->frame = 4;
    Stop(f);
    f->timer = f->offset = 0;
    f->walk = f->step = f->guard = f->walkFlags = 0;
    f->boxSet = -1;
    f->loop = f->loop2 = 0;
    f->landed = FALSE;
    f->g14 = f->g18 = f->g1C = 0;
    f->target = f->lastTarget = NULL;
    f->seed = Rand16() & 0xFFF;
    f->hp = f->maxHp = obj->unk80;
    f->hurt = 0;
    f->flash = FALSE;
    f->walls = 0;
    f->floorY = obj->base.y;
    for (i = 0; i < PALS; i++) f->pal[i] = 0;
    for (i = 0; i < STARS; i++) f->stars[i].obj = NULL;
    // (his own sprite's never shown again, his duel's task's are him: its VRAM theirs; unkC 0x200 keeps KatAM from giving it VRAM
    // again, flag 8 from animating it);
    obj->base.unkC |= 0x200;
    if (obj->base.sprite.tilesVram >= 0x6014000) VramFree(obj->base.sprite.tilesVram);
    obj->base.sprite.tilesVram = 0;
    obj->base.flags |= 8;
}

// MetaKnightInit(); On his spot (NiDL's: 216, 44 on its screen, his template's less 8, 4), caped (frame 0: NiDL's 4), his bounds
// (NiDL's terrain box);
void MetaKnightInit(struct NidlObject *n) {
    struct MetaKnight *m = (struct MetaKnight *)n;
    struct Object *obj = &n->obj;

    m->f = NULL;
    m->started = FALSE;
    obj->base.x -= 8 * 0x100;
    obj->base.y -= 4 * 0x100;
    obj->base.unk5C = (obj->base.unk5C & ~7) | 3 | 0xA0;
    sub_0803E2B0(&obj->base, -12, -10, 12, 12);
    ObjectSetBounds(&obj->base, -8, -6, 8, 10);
}

/* His defeat (MetaKnightEnd(), as KatAM's explosion bursts him: NiDL's type 0x3D's second object, 0x080A73B4: its states 0x080A7438,
 * 0x080A75C8), traced frame by frame: unmasked (NiDL's 21-59), he falls to the floor (his duel's last: NiDL's room's is flat), hops
 * 2 pixels a frame to the screen's middle (Kirby there: to the quarter on his own side), stands, his mask breaks (two halves flying
 * off), he hops twice, leaps, wraps himself in his cape, and flies off up the room 7 pixels a frame; then his shard (NidlBossShard():
 * NiDL's sword to Kirby and its level's end aren't here); */
#define END_HIDDEN -1

struct MetaKnightEnd {
    struct EffectObject e;              // (the task's own: never shown);
    struct EffectObject *parts[2];      // His body, Galaxia's pieces (NiDL's 23);
    s32 x, y, floorY;                   // 8.8;
    s16 vx, vy, ay, maxY;
    s8 facing;
    bool8 landed;                       // (his floor reached: MetaKnightEndMain());
    s16 frame;
    u16 pc, wait;
    s16 loop;
    s32 spot, target;                   // (+0x30, +0x28: on the screen);
    u8 unk56, shard, palBody, palSword;
    u16 room;
};

#define END_SCRIPT switch (end->pc) { case 0:
#define END_WAIT(n) do { end->wait = (n); end->pc = __LINE__; return; case __LINE__:; } while (0)
#define END_SCRIPT_END } end->pc = PC_END

static void EndScript(struct MetaKnightEnd *end) {
    struct Kirby *k = NULL;
    s32 d, bestD = 0, screenX = NIDL_SCREEN_X;
    u8 i;

    END_SCRIPT;
    // (0x080A7438: down to the floor);
    end->vx = end->vy = 0;
    end->ay = 0x15;
    end->maxY = 0x300;
    end->landed = FALSE;
    do END_WAIT(1); while (!end->landed);
    // (NiDL's sword to Kirby: not here) to the screen's middle, or with Kirby there the quarter on his side;
    end->spot = (end->x >> 8) - screenX;
    for (i = 0; i < gNumKirbys; i++) {
        if (gKirbys[i].base.roomId != end->room || gKirbys[i].hp <= 0) continue;
        d = abs(KirbyX(&gKirbys[i]) - (end->x >> 8));
        if (k == NULL || d < bestD) {
            k = &gKirbys[i];
            bestD = d;
        }
    }
    d = k ? KirbyX(k) - screenX : 0;
    if ((u32)(d - 0x58) <= 0x40) end->target = end->spot > 0x7F ? 0xB0 : 0x40;
    else end->target = 0x80;
    end->facing = end->spot < end->target ? 1 : -1;
    end->vx = end->facing * 0x200;
    end->frame = 21;
    d = abs(end->spot - end->target) >> 1;
    end->vy = -((d >> 1) << 5);
    end->ay = 0x20;
    if (end->vy == 0) end->vy = -0x100;
    end->landed = FALSE;
    do END_WAIT(1); while (!end->landed);
    // (NiDL's stop: his caps off too, his leap below falls to 4 pixels a frame);
    end->vx = end->vy = end->ay = 0;
    end->maxY = NO_MAX;
    END_WAIT(2);
    // (0x080A75C8: stands, his mask breaks);
    for (end->loop = 0; end->loop <= 1; end->loop++) {
        end->frame = 23;
        END_WAIT(2);
        end->frame = 21;
        END_WAIT(14);
    }
    if (gKirbys[gLocalPlayerId].base.roomId == end->room)
        FxCreate(end->room, FX_MASK, end->x - end->facing * 2 * 0x100, end->y - 0x100, end->facing, 0, end->palBody);
    end->frame = 24;
    END_WAIT(8);
    end->frame = 25;
    END_WAIT(4);
    end->vy = -0x300;
    end->ay = 0x100;
    end->frame = 26;
    END_WAIT(5);
    end->vy = -0x200;
    end->ay = 0x100;
    END_WAIT(3);
    end->vx = end->vy = end->ay = 0;
    end->frame = 27;
    END_WAIT(2);
    end->frame = 28;
    END_WAIT(10);
    end->vy = -0x440;
    end->ay = 0x40;
    end->frame = 29;
    END_WAIT(10);
    for (end->loop = 0; end->loop <= 6; end->loop++) {
        end->frame++;
        END_WAIT(2);
    }
    end->frame++;
    END_WAIT(9);
    end->vx = end->vy = end->ay = 0;
    end->frame = 38;
    END_WAIT(2);
    end->frame = 39;
    END_WAIT(3);
    end->frame = 40;
    END_WAIT(2);
    end->frame = 41;
    END_WAIT(2);
    end->frame = 42;
    END_WAIT(3);
    end->frame = 43;
    END_WAIT(2);
    end->frame = 44;
    END_WAIT(1);
    end->frame = 45;
    END_WAIT(1);
    end->frame = 46;
    END_WAIT(2);
    end->frame = 47;
    END_WAIT(2);
    end->frame = 48;
    END_WAIT(1);
    for (end->loop = 0; end->loop <= 7; end->loop++) {
        end->frame++;
        END_WAIT(2);
    }
    end->frame--;
    END_WAIT(2);
    end->frame = 57;
    END_WAIT(2);
    end->frame = 58;
    END_WAIT(1);
    // (off up the room);
    end->frame = 59;
    end->vy = -0x700;
    end->maxY = 0x7FFF;
    while ((end->y >> 8) > FLY_OFF_Y) END_WAIT(1);
    end->vy = 0;
    end->frame = END_HIDDEN;
    END_WAIT(62);
    END_SCRIPT_END;
}

static void MetaKnightEndMain(void) {
    struct MetaKnightEnd *end = TaskGetStructPtr(gCurTask);
    u16 animId = gNidlData.bosses[NIDL_META_KNIGHT].animId;
    u8 i, k;

    if (end->e.flags & 0x1000) {
        for (i = 0; i < 2; i++) Release(&end->parts[i]);
        TaskDestroy(gCurTask);
        return;
    }
    if (end->pc != PC_END && (!end->wait || !--end->wait)) EndScript(end);
    if (end->pc == PC_END) {
        for (i = 0; i < 2; i++) Release(&end->parts[i]);
        NidlBossShard(end->unk56, end->room, end->shard);
        end->e.flags |= 0x1000;
        return;
    }
    end->vy += end->ay;
    if (end->vy > end->maxY) end->vy = end->maxY;
    end->x += end->vx;
    end->y += end->vy;
    // (landing: NiDL's floor stops him once his whole pixel is past its own, in the middle of his pixel, Move());
    if (end->vy > 0 && (end->y >> 8) > (end->floorY >> 8)) {
        end->y = (end->floorY & ~0xFF) | 0x80;
        end->landed = TRUE;
    }
    if (gKirbys[gLocalPlayerId].base.roomId != end->room) return;
    if (!end->palBody) end->palBody = Palette(end->room, animId, 0);
    if (!end->palSword) end->palSword = Palette(end->room, animId | KEY_SWORD, gNidlMetaKnightFight.front[60]);
    for (i = 0; i < 2; i++) {
        if (end->frame < 0 || end->frame >= NIDL_FRAMES) k = FRAME_NONE;
        else k = i == 0 ? BODY(end->frame) : gNidlMetaKnightFight.front[end->frame];
        if (k == FRAME_NONE) {
            Hide(end->parts[i]);
            continue;
        }
        if (end->parts[i] == NULL) end->parts[i] = Sprite(end->room, gNidlMetaKnightFight.tiles[i], k, i ? SWORD_ORDER : BODY_ORDER);
        Draw(end->parts[i], end->x, end->y, k, i ? end->palSword : end->palBody, end->facing);
    }
}

static void MetaKnightEnd(struct Object *obj, s32 floorY) {
    struct Task *t = TaskCreate(MetaKnightEndMain, sizeof(struct MetaKnightEnd), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct MetaKnightEnd *end = TaskGetStructPtr(t);
    struct MetaKnight *m = (struct MetaKnight *)obj;

    ClearEffectObject(&end->e);
    end->e.header.kind = 3;
    end->e.roomId = obj->base.roomId;
    end->e.parent = NULL;
    end->e.flags |= 0x400;
    end->parts[0] = end->parts[1] = NULL;
    end->x = obj->base.x;
    // (on the floor, in his pixel's middle as NiDL's floor keeps him);
    end->y = m->f && m->f->landed ? (obj->base.y & ~0xFF) | 0x80 : obj->base.y;
    end->floorY = floorY;
    end->vx = end->vy = end->ay = 0;
    end->maxY = 0x300;
    end->facing = m->f ? m->f->facing : 1;
    end->landed = FALSE;
    end->frame = 0;
    end->pc = 0;
    end->wait = 0;
    end->loop = 0;
    end->unk56 = obj->base.unk56;
    end->shard = m->n.variant & 7;
    end->palBody = end->palSword = 0;
    end->room = obj->base.roomId;
}
