/* Paint Roller; ported from Kirby: Nightmare in Dream Land (USA), boss object type 0x3C (spawn 0x080ACEA8), as NiDL boss
 * NIDL_PAINT_ROLLER, Ice Cream Island's (room 0x104 here, its four easels the room's own tiles). NiDL's, from its code (3 states, each a
 * script, 0x08749D10, and a function every frame after them, 0x080AD0F8; his moves keyframe tables, 0x08749D4C on; his paintings type
 * 0x74, 0x080B0CC8, its eight scripts 0x0874B540; his hurt NiDL's shared 0x0806619C), checked frame by frame against NiDL's recorded
 * fight (his states, frames, spots and speeds, his children's): see "The fight" below.
 * NiDL's objects are Actors here (one array, NiDL's object slots 32 on, run in NiDL's order: Turns()): him, his paintings (type 0x74: painted, then
 * come to life, each a KatAM enemy too, enemies/painting.c, that Kirby inhales for its ability), the lightning his cloud drops (0x75),
 * his skating's dust (0x8F, 0x90), his hit stars (0x8E) and NiDL's puff where a painting goes (the bomb's blast), each drawn as a sprite
 * of its own (EffectOnly frames: KatAMExpansion.cs' PaintRoller()), at NiDL's depths; the boss object KatAM knows (hit, its HP bar) is
 * him. Beaten, his NiDL frame for it (DEFEAT_FRAME) through KatAM's defeat (its explosion; NiDL's: frozen 29 frames, then eight stars,
 * 0x0806B12C) and his shard.
 * His frames are NiDL's streamed frames (0x08753994, 4-61), with two palettes each: his body (the first palette's pieces) and the
 * second's, in front of it or behind (NiDL draws them in its pieces' order: three sprites, gNidlPaintRoller's front and back); */
#include "../nidl.h"
#include "code_0806F780.h"
#include "task.h"
#include "main.h"
#include "kirby.h"
#include "functions.h"
#include "constants/songs.h"

void sub_0803D280(u8, u8);

/* gNidlPaintRoller; Filled in by KatAMExpansion.cs (WritePaintRollerFight()): per NiDL frame (0-61) the KatAM frame of his second
 * palette's pieces in front of his body and behind it (FRAME_NONE: none; his body is BODY()), his effects' first frames (FX_*), the
 * VRAM tiles of each group's biggest frame (G_*), his two palettes' colours and his hurt flash's, both palettes' (NiDL's, the 32 after
 * his descriptor: USA 0x082DFFA8); */
struct NidlPaintRoller {
    u8 front[62];
    u8 back[62];
    u8 fx[8];
    u8 tiles[8];
    u16 body[16], trim[16], flash[32];
};

const struct NidlPaintRoller gNidlPaintRoller = { { 0 } };

#define BODY(f) (1 + (f) - 4)       // (KatAM's frame 0 is his object's own, never shown: NiDL's 18);
#define FRAME_HIDDEN -1
#define FRAME_NONE 0xFF
#define NO_MAX 0x7FFFFFFF
#define PC_END 0xFFFF
#define LOOP -3                     // (NiDL's animations' ends: back to the start, or held);
#define END -2
#define NONE -1
/* His effects' first frames (gNidlPaintRoller.fx: his painting being painted, 0x08753BA4's 0-3; his cloud's lightning, 0x08753B88's 0-6;
 * NiDL's shared dust 0x0874CB90's 0-5, hit stars 0x0874CBC8's 0-1, puff 0x0874CA1C's 0-22); */
enum { FX_PAINTING, FX_LIGHTNING, FX_DUST, FX_HIT, FX_PUFF };
enum { G_BODY, G_FRONT, G_BACK, G_FX };
#define NIDL_SCREEN_X 8             // NiDL's camera in his room (held there; KatAM shows the room as NiDL's);
#define NIDL_SCREEN_Y 16
#define OFF_SCREEN 64               // A painting this far past NiDL's screen is gone (0x08069B44);
#define KIRBY_DY 4                  // (NiDL's Kirby stands 4 pixels higher: KatAM's Kirby's spot, NiDL's: KirbyY());
#define INTRO_TIME 128              // His intro (0x080664E0): NiDL's HP bar filling, KatAM's own fill meanwhile;
#define GUARD_TIME 31               // Frames he takes no hits after one (NiDL's +0x75; KatAM's 0x8000: its attacks pass him by);
#define HIT_HALVES 3                // A hit takes half again his HP (nidl_boss.c's NidlBossHitMore(): 60 on the HP bar, down as 40);
#define HURT_TIME 13                // NiDL's hurt (0x0806619C, 13): frozen, jittering, flashing;
#define PAINT_TIME 60               // A painting shown on its easel before it comes to life (each painting's script);
#define PARASOL_TIME 256            // The parasol's time, then gone (0x080B14FC: its +0x2C from -32);
#define DEFEAT_FRAME 40             // His frame beaten (NiDL's 0x0806B1A8: its 0, the same image as its 40);
#define PAINT_HALF 7                // A painting's terrain box (NiDL's 0x0874BF50: 7 either side, 7 up, 6 down);
#define PAINT_HEAD 7
#define PAINT_FEET 6
#define ORDER_PUFF 0x0E             // Sprite orders (NiDL's priorities, +0x42: a painting's puff and the bomb's blast 4, in front of
#define ORDER_PAINT 0x17            // Kirby, 7 (KatAM's 0x0F); his paintings 9 (their enemies' too: DrivePainting()) and his effects 10
#define ORDER_FX 0x18               // in front of him, 11; his second palette's pieces in front of his body or behind it);
#define ORDER_FRONT 0x19
#define ORDER_BODY 0x1A
#define ORDER_BACK 0x1B
#define KEY_TRIM 0x2000             // His palettes' keys (| his anim ID; his body's is his anim ID);
#define KEY_COMMON 0x4000

enum { PAL_BODY, PAL_TRIM, PAL_COMMON, PAL_PAINT, PALS };

/* NiDL's objects (Actor.kind): him, then his children (NiDL's slots 33 on); a painting, once painted, the kind of its painting (K_CAR on,
 * NiDL's 0-7: 0x0874B540); */
enum {
    K_NONE, K_ROLLER, K_PAINTING, K_CAR, K_KIRBY, K_WADDLE_DEE, K_MIC, K_BALL, K_BOMB, K_CLOUD, K_PARASOL, K_LIGHTNING, K_SKATE_DUST,
    K_PUFF_DUST, K_HIT, K_PUFF, K_BLAST,
};
#define PAINTED(kind) ((kind) >= K_CAR && (kind) <= K_PARASOL)
#define ACTORS 24
#define A_ROLLER 0

/* Sounds (NiDL's own: KatAM's nearest); */
#define SFX_NONE 0
#define SFX_JUMP 1                  // (0x221: his jumps);
#define SFX_BRUSH 2                 // (0x20D: painting);
#define SFX_THUNDER SE_KRACKO_LIGHTNING_ATTACK  // (0x222: his cloud's lightning);
#define SFX_BLAST SE_OBJECT_BOMB_EXPLODE        // (0xBD: the bomb);
static const u16 sSounds[3] = { 0, SE_BASIC_ENEMY_JUMP, SE_PRANK_PAINT };

/* His moves (NiDL's 0x080AD278, g+4 the move): a step a keyframe, the last first (+0x30 counting down): its frame, its speed across
 * (mirrored by his facing), down, its sound and how long (0x08749D4C, 0x08749D70, 0x08749D94, 0x08749DB8, 0x08749DDC); 0 his skate
 * off his easel, 1-6 to another (sRoutes), 7 up to it, 8 painting and back; */
struct Step {
    s32 vx, vy;
    u8 frame, wait, sound;
};
static const struct Step sMove0[16] = {
    { 0x20000, 0, 12, 10, SFX_NONE }, { 0x18000, 0, 48, 2, SFX_NONE }, { 0x18000, 0, 10, 2, SFX_NONE },
    { 0x18000, 0, 46, 2, SFX_NONE }, { 0x18000, 0, 8, 2, SFX_NONE }, { 0x1A000, 0, 44, 2, SFX_NONE },
    { 0x1A000, 0, 6, 2, SFX_NONE }, { 0x1A000, 0, 42, 2, SFX_NONE }, { 0x1A000, 0, 4, 2, SFX_NONE }, { 0x20000, 0, 4, 3, SFX_NONE },
    { 0x20000, 0, 42, 2, SFX_NONE }, { 0x20000, 0, 6, 1, SFX_NONE }, { 0x20000, 0, 44, 1, SFX_NONE },
    { 0x20000, 0, 8, 1, SFX_NONE }, { 0x20000, 0, 52, 2, SFX_NONE }, { 0x20000, 0, 14, 2, SFX_NONE },
};
static const struct Step sMove1[45] = {
    { 0, 0, 16, 1, SFX_NONE }, { -0x8000, 0, 16, 7, SFX_NONE }, { -0x8000, 0, 54, 2, SFX_NONE }, { -0x8000, 0, 14, 2, SFX_NONE },
    { -0x8000, 0, 52, 2, SFX_NONE }, { -0x8000, 0, 8, 2, SFX_NONE }, { -0x8000, 0, 46, 2, SFX_NONE },
    { -0x8000, 0, 10, 2, SFX_NONE }, { -0x8000, 0, 48, 2, SFX_NONE }, { -0x8000, 0, 12, 3, SFX_NONE },
    { -0x10000, 0, 12, 6, SFX_NONE }, { -0xA000, 0, 12, 10, SFX_NONE }, { -0x10000, 0x40000, 12, 3, SFX_NONE },
    { -0x14000, 0x20000, 12, 6, SFX_NONE }, { -0x16000, 0x10000, 58, 2, SFX_NONE }, { -0x20000, 0x10000, 56, 2, SFX_NONE },
    { -0x20000, 0x10000, 60, 1, SFX_NONE }, { -0x20000, 0x10000, 61, 1, SFX_NONE }, { -0x20000, 0x8000, 57, 2, SFX_NONE },
    { -0x20000, 0x8000, 59, 2, SFX_NONE }, { -0x20000, 0x8000, 13, 2, SFX_NONE }, { -0x20000, -0x8000, 49, 2, SFX_NONE },
    { -0x20000, -0x8000, 11, 2, SFX_NONE }, { -0x20000, -0x8000, 47, 2, SFX_NONE }, { -0x20000, -0x10000, 9, 2, SFX_NONE },
    { -0x20000, -0x14000, 45, 2, SFX_NONE }, { -0x20000, -0x14000, 7, 2, SFX_NONE }, { -0x20000, -0x24000, 43, 2, SFX_NONE },
    { -0x20000, -0x24000, 5, 2, SFX_NONE }, { -0x20000, -0x24000, 51, 2, SFX_NONE }, { -0x20000, -0x24000, 50, 2, SFX_NONE },
    { -0x20000, -0x34000, 4, 2, SFX_NONE }, { -0x20000, -0x34000, 42, 2, SFX_NONE }, { -0x20000, -0x34000, 6, 2, SFX_NONE },
    { -0x20000, -0x34000, 44, 2, SFX_NONE }, { -0x20000, -0x50000, 8, 2, SFX_JUMP }, { 0, 0, 40, 4, SFX_NONE },
    { 0, 0, 18, 12, SFX_NONE }, { 0x20000, -0x38000, 20, 3, SFX_NONE }, { 0x20000, -0x38000, 38, 11, SFX_JUMP },
    { 0x10000, 0, 8, 2, SFX_NONE }, { 0x10000, 0, 46, 2, SFX_NONE }, { 0xC000, 0, 10, 2, SFX_NONE }, { 0xC000, 0, 12, 6, SFX_NONE },
    { 0x14000, 0, 12, 8, SFX_NONE },
};
static const struct Step sMove2[39] = {
    { 0, 0, 17, 1, SFX_NONE }, { 0x8000, 0, 17, 8, SFX_NONE }, { 0xE000, 0, 17, 8, SFX_NONE }, { 0x10000, 0, 55, 2, SFX_NONE },
    { 0x10000, 0, 15, 2, SFX_NONE }, { 0x10000, 0, 53, 2, SFX_NONE }, { 0x10000, 0, 9, 2, SFX_NONE },
    { 0x10000, 0, 45, 2, SFX_NONE }, { 0x10000, 0, 7, 2, SFX_NONE }, { 0x10000, 0, 43, 2, SFX_NONE },
    { 0x10000, 0, 5, 2, SFX_NONE }, { 0x10000, 0, 51, 2, SFX_NONE }, { 0x10000, 0, 50, 2, SFX_NONE },
    { 0x10000, 0, 4, 2, SFX_NONE }, { 0x10000, 0, 42, 2, SFX_NONE }, { 0x10000, 0, 6, 2, SFX_NONE },
    { 0x20000, 0, 44, 2, SFX_NONE }, { 0x20000, 0, 8, 2, SFX_NONE }, { 0x20000, 0, 46, 2, SFX_NONE },
    { 0x20000, 0, 10, 2, SFX_NONE }, { 0x20000, 0, 48, 2, SFX_NONE }, { 0x20000, 0, 12, 13, SFX_NONE },
    { 0x14000, 0, 12, 2, SFX_NONE }, { 0x14000, 0, 48, 2, SFX_NONE }, { 0x14000, 0, 10, 2, SFX_NONE },
    { 0x14000, 0, 46, 2, SFX_NONE }, { 0x14000, 0, 8, 2, SFX_NONE }, { 0x14000, 0, 44, 2, SFX_NONE },
    { 0x14000, 0, 6, 2, SFX_NONE }, { 0x14000, 0, 42, 2, SFX_NONE }, { 0x14000, 0, 4, 8, SFX_NONE }, { 0x20000, 0, 4, 4, SFX_NONE },
    { 0x20000, 0, 42, 2, SFX_NONE }, { 0x20000, 0, 6, 2, SFX_NONE }, { 0x10000, 0, 44, 2, SFX_NONE },
    { 0x10000, 0, 8, 2, SFX_NONE }, { 0x10000, 0, 46, 2, SFX_NONE }, { 0x10000, 0, 10, 2, SFX_NONE },
    { 0x10000, 0, 48, 2, SFX_NONE },
};
static const struct Step sMove3[32] = {
    { 0, 0, 17, 1, SFX_NONE }, { 0x8000, 0, 17, 22, SFX_NONE }, { 0x8000, 0, 17, 8, SFX_NONE }, { 0x10000, 0, 17, 16, SFX_NONE },
    { 0x20000, 0, 17, 8, SFX_NONE }, { 0x20000, 0, 17, 4, SFX_NONE }, { 0x18000, 0, 17, 3, SFX_NONE },
    { 0x18000, 0, 17, 3, SFX_NONE }, { 0x10000, 0x40000, 17, 1, SFX_NONE }, { 0x10000, 0x40000, 55, 2, SFX_NONE },
    { 0x10000, 0x20000, 15, 2, SFX_NONE }, { 0x10000, 0x20000, 53, 2, SFX_NONE }, { 0x10000, 0x20000, 9, 2, SFX_NONE },
    { 0x10000, 0x10000, 45, 2, SFX_NONE }, { 0x10000, 0x10000, 7, 2, SFX_NONE }, { 0x10000, 0x10000, 43, 2, SFX_NONE },
    { 0x10000, 0x8000, 5, 2, SFX_NONE }, { 0x10000, 0x8000, 51, 2, SFX_NONE }, { 0x10000, 0x8000, 50, 2, SFX_NONE },
    { 0x10000, -0x8000, 4, 2, SFX_NONE }, { 0x10000, -0x8000, 42, 2, SFX_NONE }, { 0x10000, -0x8000, 6, 2, SFX_NONE },
    { 0x10000, -0x10000, 44, 2, SFX_NONE }, { 0x10000, -0x10000, 8, 2, SFX_NONE }, { 0x10000, -0x10000, 40, 2, SFX_NONE },
    { 0x10000, -0x20000, 38, 8, SFX_NONE }, { 0x10000, -0x30000, 38, 8, SFX_NONE }, { 0x10000, -0x40000, 38, 16, SFX_JUMP },
    { 0x14000, 0, 8, 2, SFX_NONE }, { 0x14000, 0, 46, 2, SFX_NONE }, { 0x14000, 0, 10, 2, SFX_NONE },
    { 0x14000, 0, 48, 2, SFX_NONE },
};
static const struct Step sMove4[45] = {
    { -0x8000, 0, 16, 8, SFX_NONE }, { -0x10000, 0, 16, 9, SFX_NONE }, { -0x14000, 0, 16, 8, SFX_NONE },
    { -0x20000, 0, 16, 8, SFX_NONE }, { -0x20000, 0, 54, 2, SFX_NONE }, { -0x20000, 0, 14, 2, SFX_NONE },
    { -0x20000, 0, 52, 2, SFX_NONE }, { -0x20000, 0, 8, 2, SFX_NONE }, { -0x20000, 0, 44, 2, SFX_NONE },
    { -0x20000, 0, 6, 2, SFX_NONE }, { -0x20000, 0, 42, 2, SFX_NONE }, { -0x20000, 0, 4, 2, SFX_NONE },
    { -0x20000, 0, 50, 2, SFX_NONE }, { -0x20000, 0, 51, 2, SFX_NONE }, { -0x20000, 0, 5, 2, SFX_NONE },
    { -0x20000, 0, 43, 2, SFX_NONE }, { -0x20000, 0, 7, 2, SFX_NONE }, { -0x20000, 0, 45, 2, SFX_NONE },
    { -0x20000, 0, 9, 2, SFX_NONE }, { -0x20000, 0, 47, 2, SFX_NONE }, { -0x20000, 0, 11, 2, SFX_NONE },
    { -0x20000, 0, 49, 2, SFX_NONE }, { -0x20000, 0, 13, 22, SFX_NONE }, { -0x20000, 0, 49, 2, SFX_NONE },
    { 0x8000, 0, 11, 2, SFX_NONE }, { 0, 0, 47, 2, SFX_NONE }, { 0x10000, 0x20000, 9, 6, SFX_NONE },
    { 0x10000, 0x10000, 9, 6, SFX_NONE }, { 0x14000, -0x10000, 9, 4, SFX_NONE }, { 0x14000, -0x10000, 39, 2, SFX_NONE },
    { 0x14000, -0x20000, 39, 2, SFX_NONE }, { 0x14000, -0x20000, 38, 4, SFX_NONE }, { 0, 0, 8, 1, SFX_NONE },
    { 0, 0, 44, 1, SFX_NONE }, { 0x20000, 0x40000, 6, 1, SFX_NONE }, { 0x20000, 0x40000, 42, 1, SFX_NONE },
    { 0x20000, 0x40000, 4, 13, SFX_NONE }, { 0x20000, 0x20000, 4, 2, SFX_NONE }, { 0x20000, 0x20000, 42, 2, SFX_NONE },
    { 0x20000, 0x20000, 6, 2, SFX_NONE }, { 0x20000, 0x10000, 44, 2, SFX_NONE }, { 0x20000, 0x10000, 8, 2, SFX_NONE },
    { 0x20000, 0x10000, 46, 2, SFX_NONE }, { 0x20000, 0x8000, 10, 2, SFX_NONE }, { 0x20000, 0x8000, 48, 2, SFX_NONE },
};
static const struct Step sMove5[25] = {
    { 0, 0, 17, 4, SFX_NONE }, { 0x8000, 0, 17, 8, SFX_NONE }, { 0x8000, 0, 17, 8, SFX_NONE }, { 0x8000, 0, 17, 8, SFX_NONE },
    { 0x8000, 0, 17, 8, SFX_NONE }, { 0x10000, 0, 17, 4, SFX_NONE }, { 0x10000, 0, 55, 2, SFX_NONE },
    { 0x10000, 0, 15, 2, SFX_NONE }, { 0x20000, 0, 9, 2, SFX_NONE }, { 0x20000, 0, 45, 2, SFX_NONE },
    { 0x20000, 0, 7, 2, SFX_NONE }, { 0x20000, 0, 43, 2, SFX_NONE }, { 0x28000, 0, 5, 2, SFX_NONE },
    { 0x28000, 0, 51, 2, SFX_NONE }, { 0x30000, 0, 50, 2, SFX_NONE }, { 0x38000, 0, 4, 6, SFX_NONE },
    { 0x20000, 0x40000, 4, 15, SFX_NONE }, { 0x20000, 0x20000, 4, 2, SFX_NONE }, { 0x20000, 0x20000, 42, 2, SFX_NONE },
    { 0x20000, 0x20000, 6, 2, SFX_NONE }, { 0x20000, 0x10000, 44, 2, SFX_NONE }, { 0x20000, 0x10000, 8, 2, SFX_NONE },
    { 0x20000, 0x10000, 46, 2, SFX_NONE }, { 0x20000, 0x8000, 10, 2, SFX_NONE }, { 0x20000, 0x8000, 48, 2, SFX_NONE },
};
static const struct Step sMove6[22] = {
    { 0x8000, 0, 17, 16, SFX_NONE }, { 0x10000, 0, 17, 16, SFX_NONE }, { 0x1C000, 0, 17, 6, SFX_NONE },
    { 0x2C000, 0, 17, 6, SFX_NONE }, { 0x40000, 0, 55, 2, SFX_NONE }, { 0x40000, 0, 15, 2, SFX_NONE },
    { 0x20000, 0x40000, 9, 2, SFX_NONE }, { 0x20000, 0x40000, 45, 2, SFX_NONE }, { 0x20000, 0x20000, 7, 2, SFX_NONE },
    { 0x20000, 0x20000, 43, 2, SFX_NONE }, { 0x20000, 0x10000, 5, 2, SFX_NONE }, { 0x20000, 0x10000, 51, 2, SFX_NONE },
    { 0x20000, 0x8000, 50, 2, SFX_NONE }, { 0x20000, 0x8000, 4, 2, SFX_NONE }, { 0x20000, -0x8000, 42, 2, SFX_NONE },
    { 0x20000, -0x8000, 6, 2, SFX_NONE }, { 0x20000, -0x10000, 44, 2, SFX_NONE }, { 0x20000, -0x10000, 8, 2, SFX_NONE },
    { 0x20000, -0x20000, 46, 2, SFX_NONE }, { 0x20000, -0x20000, 10, 2, SFX_NONE }, { 0x20000, -0x40000, 38, 4, SFX_JUMP },
    { 0x20000, 0, 8, 4, SFX_NONE },
};
static const struct Step sMove7[10] = {
    { 0, 0, 4, 32, SFX_NONE }, { 0x2000, 0, 4, 8, SFX_NONE }, { 0x8000, 0, 42, 4, SFX_NONE }, { 0x8000, 0, 6, 4, SFX_NONE },
    { 0x8000, 0, 44, 4, SFX_NONE }, { 0x8000, 0, 8, 4, SFX_NONE }, { 0x2000, 0, 52, 2, SFX_NONE }, { 0x2000, 0, 14, 2, SFX_NONE },
    { 0x2000, 0, 54, 2, SFX_NONE }, { 0x2000, 0, 16, 2, SFX_NONE },
};
static const struct Step sMove8[27] = {
    { -0x2000, 0, 16, 8, SFX_NONE }, { -0x8000, 0, 16, 2, SFX_NONE }, { -0x8000, 0, 54, 2, SFX_NONE },
    { -0x8000, 0, 14, 2, SFX_NONE }, { -0x8000, 0, 52, 2, SFX_NONE }, { -0x8000, 0, 8, 2, SFX_NONE },
    { -0x8000, 0, 44, 2, SFX_NONE }, { -0x8000, 0, 6, 2, SFX_NONE }, { -0x8000, 0, 42, 2, SFX_NONE },
    { -0x2000, 0, 4, 8, SFX_NONE }, { 0, 0, 4, 29, SFX_NONE }, { 0, 0, 28, 2, SFX_NONE }, { 0, 0, 36, 2, SFX_NONE },
    { 0, 0, 34, 2, SFX_NONE }, { 0, 0, 32, 2, SFX_NONE }, { 0, 0, 30, 2, SFX_NONE }, { 0, 0, 28, 2, SFX_NONE },
    { 0, 0, 36, 2, SFX_NONE }, { 0, 0, 34, 2, SFX_NONE }, { 0, 0, 32, 2, SFX_NONE }, { 0, 0, 30, 2, SFX_NONE },
    { 0, 0, 28, 2, SFX_NONE }, { 0, 0, 36, 2, SFX_NONE }, { 0, 0, 34, 2, SFX_NONE }, { 0, 0, 32, 2, SFX_NONE },
    { 0, 0, 30, 2, SFX_NONE }, { 0, 0, 28, 2, SFX_BRUSH },
};
static const struct Step *const sMoves[9] = { sMove0, sMove1, sMove2, sMove3, sMove4, sMove5, sMove6, sMove7, sMove8 };
static const u8 sSteps[9] = { 16, 45, 39, 32, 45, 25, 22, 10, 27 };

/* His easels (0x0874AAD0: his facing at each, 0x0874AAD4 and 0x0874AADC his spot; 0x0874AAE4 and 0x0874AAEC its painting's): 0 and 1
 * on the left (the floor's, the ledge's), 2 and 3 on the right; */
static const s8 sEaselFacing[4] = { 1, 1, -1, -1 };
static const s16 sEaselX[4] = { 23, 23, 231, 231 };
static const s16 sEaselY[4] = { 145, 65, 145, 65 };
static const s16 sPaintX[4] = { 60, 60, 196, 196 };
static const s16 sPaintY[4] = { 124, 44, 124, 44 };
// From an easel to another (0x08749D1C: its route, by from * 4 + to), each route's move (0x08749D2C) and way (0x08749D38);
static const s8 sRoutes[16] = { -1, 0, 1, 2, 3, -1, 4, 5, 7, 8, -1, 6, 10, 11, 9, -1 };
static const u8 sRouteMoves[12] = { 1, 2, 3, 4, 5, 6, 1, 2, 3, 4, 5, 6 };
static const s8 sRouteWays[12] = { -1, 1, -1, -1, -1, 1, -1, 1, -1, -1, -1, 1 };
// His intro (0x08749CEC) and his stand (0x080AD08C);
static const s16 sIntro[] = { 20, 4, 22, 3, 24, 4, 26, 11, 24, 4, 22, 3, 20, 4, 18, 11, LOOP };
static const u8 sStand[8][2] = { { 20, 4 }, { 22, 3 }, { 24, 4 }, { 26, 11 }, { 24, 4 }, { 22, 3 }, { 20, 4 }, { 18, 11 } };
/* His paintings: each one's chance (0x0874B5A4: of 255, his HP under its half, then at it or over; the last two painted none),
 * its blink as he paints it (0x080B0CC8), its first frame in its enemy (enemies/painting.c; KatAMExpansion.cs' PaintingA(),
 * PaintingB()); */
static const u8 sChances[16] = { 11, 43, 21, 32, 31, 33, 42, 42, 74, 0, 64, 11, 31, 12, 31, 32 };
static const u8 sPaintingBase[8] = { 0, 0, 3, 7, 11, 4, 7, 10 };
// The bomb's hop (0x0874B560), its fuse (0x0874B568), the cloud (0x0874B574), the parasol (0x0874B590), NiDL's puff (0x0873E640);
static const s16 sBombHop[] = { 4, 1, END };
static const s16 sBombFuse[] = { 5, 2, 4, 3, LOOP };
static const s16 sCloud[] = { 5, 8, 5, 2, 6, 2, 5, 2, 6, 2, 4, 56, LOOP };
static const s16 sParasol[] = { 4, 6, 6, 6, 8, 6, 7, 6, LOOP };
static const u8 sPuff[23] = { 0, 12, 1, 13, 2, 14, 3, 15, 4, 16, 5, 17, 6, 18, 7, 19, 8, 20, 9, 21, 10, 22, 11 };
// The cloud's bob (0x080B1264);
static const s32 sCloudBob[6] = { -0x8000, -0x10000, -0x8000, 0x8000, 0x10000, 0x8000 };
// Hit stars' jitter about him (0x0873EB30, 0x0873EB38);
static const s8 sHitX[8] = { -8, 0, 4, -16, 4, -8, -16, 4 };
static const s8 sHitY[8] = { -16, -20, 0, -16, 4, -16, -16, 0 };
// Hurt (0x0806627C): his jitter's steps by its countdown (0x0873E184, 0x0873E16C);
static const u8 sJitterOf[14] = { 5, 4, 3, 2, 1, 0, 5, 4, 3, 2, 1, 0, 5, 4 };
static const s8 sJitter[6][2] = { { 2, 1 }, { -2, -1 }, { -2, 0 }, { 2, 1 }, { 2, -1 }, { -2, 0 } };
/* NiDL's sine (0x0872FA30: 640 entries, 256 one; 512 a circle, 0 right, 128 down: the cosine 128 on), for the baseball's aim
 * (0x0806421C); */
static const s16 sSine[640] = {
#include "shine_bright_sine.h"
};

/* Actor; One of NiDL's objects (its fields, by their NiDL offsets): a script (WAIT() resumes it, PC_END: over), its move (NiDL's
 * 0x080656B4) and its frame's function each frame, in NiDL's order (Turns()); its sprite; */
struct Actor {
    u8 kind, variant;
    u8 state, next;                     // (+0x15, +0x14);
    u8 script;                          // The state whose script is running (+0x28);
    bool8 restart;                      // Its next state starts as it's next run (NiDL's 0x08006148);
    u16 pc, wait;
    s8 facing, flip;                    // (+0x43; its frame's mirroring: its facing as its frame was last set);
    s16 frame;                          // (+0x3C; NiDL's: FRAME_HIDDEN none);
    s8 parent;                          // (+0x44: its parent's Actor);
    s8 child;                           // (+0x46: NONE none);
    s32 x, y;                           // 16.16 (+0x4C);
    s16 ix, iy;                         // Its pixel (+0x48);
    s32 vx, vy, ax, ay, maxX, maxY;     // (+0x54 on);
    s32 v18, v1C, v20, v2C, v30;        // (+0x18 on);
    s16 loop;                           // (+0x6C);
    bool8 landed;                       // (+0x7A);
    bool8 live;                         // Its frame's function set (+4: a painting come to life, its box on);
    const s16 *anim;                    // Its animation (+0x28 of the paintings: NULL none; its step);
    u8 animStep;
    s16 animWait;
    struct EffectObject *sprite;
    struct ObjectBase *attack;          // Its box hurting Kirby (an attack object: the lightning, the blast);
    struct Object *enemy;               // A painting's KatAM enemy (enemies/painting.c);
};

struct PaintRoller {
    struct NidlObject n;
    struct Fight *f;
    bool8 started;
};

typedef char sPaintRollerFits[sizeof(struct PaintRoller) <= NIDL_OBJECT_SIZE ? 1 : -1];

/* Fight; The fight's state (a task of its own), NiDL's objects and his globals (0x02007D00: his easel, his move, his paintings, his
 * way; 0x02006190: his hurt); */
struct Fight {
    struct Object *obj;                 // (NULL: gone, PaintRollerGone());
    struct Actor a[ACTORS];
    struct EffectObject *parts[3];      // His body, his second palette's pieces in front, behind;
    u16 seed;                           // NiDL's random numbers' (0x03000FB4);
    s16 hp;                             // KatAM's object's HP last frame (less now: hit);
    s16 half;                           // Under this, his HP under half (0x080AD37C: g+0x24 >> 1);
    bool8 over;                         // Beaten (his defeat under way: KatAM's explosion);
    bool8 hooked;                       // His hurt running (+8: 0x080662D8, his own script held: +0x13 7);
    u8 guard;                           // Frames he takes no hits yet (+0x75: 31 from a hit's);
    bool8 flashing;                     // His hurt flash on (0x02006190 + 0x14);
    u8 hurt;                            // (0x02006190 + 0xC: his hurt's countdown);
    s16 hurtX, hurtY, hurtFrame;        // Where he was hit, his frame then (0x02006190: back as his hurt ends);
    s32 easel;                          // (g+0: the easel he's at, or skating to);
    s32 move;                           // (g+4: his move, sMoves[]);
    s32 stands;                         // (g+8: one stand more every other painting);
    s32 painted;                        // (g+0xC: the last painting (its high half) and the one before);
    s32 way;                            // (g+0x10: his ways about the easels, one way or the other);
    s32 after;                          // (g+0x14: the state a hurt's end starts, -1 none);
    u16 frameCount;                     // (0x03001EA4: the flashes' clock);
    u8 unk56;                           // (the room's level: its terrain, Solid());
    u16 room;
    u8 pal[PALS];
    s16 lastX, lastY;                   // His pixel as the frame starts (his hit stars': HitFrame());
    u8 queue[ACTORS * 2];               // This frame's turns (NiDL's lists, 0x030024A0: Actors, by group, then those spawned meanwhile)
    u8 groups[ACTORS * 2];              // and their groups (+0x12);
    u8 queued;
    bool8 listing;                      // The turns under way (a spawn takes one at the end: Spawn());
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

static void Stop(struct Actor *a) {
    a->vx = a->vy = a->ax = a->ay = 0;
    a->maxX = a->maxY = NO_MAX;
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

static void SetX(struct Actor *a, s32 x) {
    a->ix = x;
    a->x = x << 16;
}

static void SetY(struct Actor *a, s32 y) {
    a->iy = y;
    a->y = y << 16;
}

/* Animations (+0x28 of the paintings: 0x080640DC starts one, its first frame mirrored by its facing; 0x08064188 steps it, the frames
 * left of the step, the next once out); */
static void Step(struct Actor *a) {
    const s16 *e = &a->anim[a->animStep * 2];

    if (e[0] == LOOP) {
        a->animStep = 0;
        e = a->anim;
    } else if (e[0] == END) {
        a->anim = NULL;
        return;
    }
    SetFrame(a, e[0]);
    a->animWait = e[1];
    a->animStep++;
}

static void Play(struct Actor *a, const s16 *anim) {
    a->anim = anim;
    a->animStep = 0;
    if (anim) Step(a);
}

static void AnimTick(struct Actor *a) {
    if (a->anim == NULL) return;
    if (a->animWait <= 0) Step(a);
    a->animWait--;
}

/* SetState(); NiDL's 0x0806395C: his next state (+0x14); Restart(): it starts as he's next run (0x08006148); */
static void SetState(struct Actor *a, u8 state) {
    a->next = state;
}

static void Restart(struct Actor *a) {
    a->restart = TRUE;
}

static void Sound(struct Fight *f, u16 sfx) {
    if (sfx) PlaySfx(&f->obj->base, sfx);
}

/* Their objects: Spawn() takes NiDL's lowest free slot (0x08005904), a child of `parent` at (x, y), its script run as it's next run
 * (its turn: Turns()); the spawn's facing its parent's; */
static void Destroy(struct Fight *f, s8 i);

/* Group(); An Actor's group (NiDL's +0x12): the effects 1 (the dust, the hit stars, the puffs), the paintings and the lightning 2, him 3
 * (each frame, NiDL runs its objects by group, then by slot: Turns()); */
static u8 Group(const struct Actor *a) {
    if (a->kind == K_ROLLER) return 3;
    if (a->kind == K_PAINTING || PAINTED(a->kind) || a->kind == K_LIGHTNING) return 2;
    return 1;
}

static void Queue(struct Fight *f, s8 i) {
    if (f->queued >= ARRAY_COUNT(f->queue)) return;
    f->groups[f->queued] = Group(&f->a[i]);
    f->queue[f->queued++] = i;
}

static s8 Spawn(struct Fight *f, u8 kind, s8 parent, s32 x, s32 y) {
    struct Actor *a, *p = &f->a[parent];
    s8 i;

    for (i = A_ROLLER + 1; i < ACTORS && f->a[i].kind != K_NONE; i++) ;
    if (i >= ACTORS) return NONE;
    a = &f->a[i];
    CpuFill32(0, a, sizeof(struct Actor));
    a->kind = kind;
    a->facing = p->facing;
    a->flip = 1;
    a->frame = FRAME_HIDDEN;
    a->parent = parent;
    a->child = NONE;
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

// LetEnemyGo(); A painting's KatAM enemy no more the fight's (inhaled, beaten); gone, unless `keep`;
static void LetEnemyGo(struct Actor *a, bool32 keep) {
    if (a->enemy && !keep && PaintingDriven(a->enemy)) a->enemy->base.flags |= 0x1000;
    a->enemy = NULL;
}

static void Destroy(struct Fight *f, s8 i) {
    struct Actor *a;

    if (i <= A_ROLLER || i >= ACTORS) return;
    a = &f->a[i];
    if (a->kind == K_NONE) return;
    Release(&a->sprite);
    if (a->attack) a->attack->flags |= 0x1000;
    a->attack = NULL;
    LetEnemyGo(a, FALSE);
    a->kind = K_NONE;
    if (f->a[A_ROLLER].child == i) f->a[A_ROLLER].child = NONE;
}

/* Solid(); NiDL's terrain: the room's solid tiles (its 16x16 ones; outside it, none); */
static bool32 Solid(struct Fight *f, s32 x, s32 y) {
    struct LevelInfo *level = &gCurLevelInfo[f->unk56];

    if (x < 0 || y < 0 || x >= level->roomWidth || y >= level->roomHeight) return FALSE;
    return !(gCollisionAttributes[GetCollisionTile(f->unk56, x >> 4, y >> 4)] & 1);
}

/* Motion(); NiDL's 0x080656B4 (his: 0x080059D8, the same): its speed plus its change, capped (across either way, falling), then its
 * spot plus its speed; */
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

// TurnAround(); 0x08063FF4 (a painting's wall): the other way, its frame mirrored with it, its speed and change across back;
static void TurnAround(struct Actor *a) {
    a->facing = -a->facing;
    SetFrame(a, a->frame);
    a->vx = -a->vx;
    a->ax = -a->ax;
}

/* Terrain(); A painting's (NiDL's 0x08068E04, its box 0x0874BF50: PAINT_HALF, PAINT_HEAD, PAINT_FEET; its handlers 0x0874BF58), traced:
 * into a wall its way, out of it (the middle of its pixel), turned around; into the floor falling (either corner), on it, landed, no more
 * falling (0x080B1588); into the ceiling rising, under it; landed with no floor under either corner, falling again (0x080B1594: 0x1500
 * down a frame, 3 pixels a frame at most); */
static void Terrain(struct Fight *f, struct Actor *a) {
    s32 feet = a->iy + PAINT_FEET, head = a->iy - PAINT_HEAD;

    if (a->vx < 0 && Solid(f, a->ix - PAINT_HALF, a->iy)) {
        SetX(a, ((a->ix - PAINT_HALF) | 15) + 1 + PAINT_HALF);
        a->x |= 0x8000;
        TurnAround(a);
    } else if (a->vx > 0 && Solid(f, a->ix + PAINT_HALF, a->iy)) {
        SetX(a, ((a->ix + PAINT_HALF) & ~15) - PAINT_HALF - 1);
        a->x |= 0x8000;
        TurnAround(a);
    }
    if (a->vy >= 0 && (Solid(f, a->ix - PAINT_HALF, feet) || Solid(f, a->ix + PAINT_HALF, feet))) {
        SetY(a, (feet & ~15) - PAINT_FEET - 1);
        a->y |= 0x8000;
        if (!a->landed) {
            a->landed = TRUE;
            StopY(a);
        }
    } else if (a->vy < 0 && (Solid(f, a->ix - PAINT_HALF, head) || Solid(f, a->ix + PAINT_HALF, head))) {
        SetY(a, (head | 15) + 1 + PAINT_HEAD);
        a->y |= 0x8000;
    } else if (a->landed && !Solid(f, a->ix - PAINT_HALF, feet + 1) && !Solid(f, a->ix + PAINT_HALF, feet + 1)) {
        a->landed = FALSE;
        a->ay = 0x1500;
        a->maxY = 0x30000;
    }
}

// OffScreen(); 0x08069B44: past NiDL's screen by OFF_SCREEN (a painting come to life, a lightning falling: gone);
static bool32 OffScreen(struct Actor *a) {
    s32 sx = a->ix - NIDL_SCREEN_X, sy = a->iy - NIDL_SCREEN_Y;

    return sx < 1 - OFF_SCREEN || sx > DISPLAY_WIDTH - 1 + OFF_SCREEN || sy < 1 - OFF_SCREEN || sy > DISPLAY_HEIGHT - 1 + OFF_SCREEN;
}

/* His moves (NiDL's 0x080AC800 on); */

// Place(); 0x080AD32C: on his easel's spot, facing its way;
static void Place(struct Fight *f, struct Actor *a) {
    a->facing = sEaselFacing[f->easel];
    a->x = sEaselX[f->easel] << 16;
    a->y = sEaselY[f->easel] << 16;
}

// HpHalf(); 0x080AD37C: his HP at its half or over;
static bool32 HpHalf(struct Fight *f) {
    return f->obj->unk80 >= f->half;
}

/* MoveStep(); 0x080AD278: his move's next step (its last first: +0x30 down): its frame, speeds, sound; how long; */
static u8 MoveStep(struct Fight *f, struct Actor *a) {
    const struct Step *s = &sMoves[f->move][--a->v30];

    SetFrame(a, s->frame);
    VelX(a, s->vx, VEL_KEEP);
    a->vy = s->vy;
    Sound(f, sSounds[s->sound]);
    return s->wait;
}

/* NextEasel(); 0x080AD170: the easel he skates to and its move: after two ways the same, the other side's (the floor's or the ledge's,
 * by chance); after two the other way, across; else one of the three by chance, his way (+0x10) counted: the same as the last, two; */
static void NextEasel(struct Fight *f, struct Actor *a) {
    s32 to = f->easel & 0xFF;
    s8 route;

    if (f->way > 1) {
        f->way = 0;
        to = ((to + 1) & 1) + Rand(f, 2) * 2;
        route = sRoutes[f->easel * 4 + to];
    } else if (f->way <= -2) {
        f->way = 0;
        to = (to + 2) & 3;
        route = sRoutes[f->easel * 4 + to];
    } else {
        to = (to + 1 + Rand(f, 3)) & 3;
        route = sRoutes[f->easel * 4 + to];
        if (f->way == sRouteWays[route]) f->way *= 2;
        else f->way = sRouteWays[route];
    }
    f->easel = to;
    f->move = sRouteMoves[route];
    a->v30 = sSteps[f->move];
}

static void Paint(struct Fight *f, struct Actor *a);

/* His states (NiDL's 3, 0x08749D10); */

// State0(); His intro (0x080ACEA8): on his easel (1, the ledge's left), his intro animation while the HP bar fills (KatAM's), then skating;
static void State0(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 0;
    Place(f, a);
    WAIT(1);
    Play(a, sIntro);
    WAIT(INTRO_TIME);
    Play(a, NULL);
    SetState(a, 1);
    STOP_SCRIPT();
    SCRIPT_END;
}

/* State1(); Skating (0x080ACF68): off his easel (move 0, his dust) and to another (NextEasel()), on its spot; his HP at its half or
 * over, a stand (a hurt then ends it: +0x14 2); then painting (2); */
static void State1(struct Fight *f, struct Actor *a) {
    s8 i;

    SCRIPT;
    a->state = 1;
    f->after = -1;
    Place(f, a);
    f->move = 0;
    a->v30 = sSteps[0];
    i = Spawn(f, K_SKATE_DUST, A_ROLLER, a->ix, a->iy);
    if (i != NONE) {
        f->a[i].v18 = 3;
        f->a[i].v1C = f->a[i].facing * 8;
        f->a[i].v20 = 10;
    }
    while (a->v30 > 0) WAIT(MoveStep(f, a));
    NextEasel(f, a);
    while (a->v30 > 0) WAIT(MoveStep(f, a));
    Stop(a);
    Place(f, a);
    if (HpHalf(f)) {
        f->after = 2;
        for (a->loop = 0; a->loop < 8; a->loop++) {
            SetFrame(a, sStand[a->loop][0]);
            WAIT(sStand[a->loop][1]);
        }
    }
    SetState(a, 2);
    STOP_SCRIPT();
    SCRIPT_END;
}

/* State2(); Painting (0x080ACFFC): up to his easel (move 7), a while (his HP at its half or over), his painting (Paint()), painting it
 * and back (move 8); his stands (none or one, by turns; two more his HP at its half or over: a hurt then ends them, +0x14 1); then
 * skating (1); */
static void State2(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->state = 2;
    f->after = -1;
    f->move = 7;
    a->v30 = sSteps[7];
    while (a->v30 > 0) WAIT(MoveStep(f, a));
    if (HpHalf(f)) WAIT(32);
    Paint(f, a);
    f->move = 8;
    a->v30 = sSteps[8];
    while (a->v30 > 0) WAIT(MoveStep(f, a));
    f->stands = (f->stands + 1) & 1;
    a->v30 = f->stands + (HpHalf(f) ? 2 : 0);
    f->after = 1;
    while (a->v30 > 0) {
        Stop(a);
        for (a->loop = 0; a->loop < 8; a->loop++) {
            SetFrame(a, sStand[a->loop][0]);
            WAIT(sStand[a->loop][1]);
        }
        a->v30--;
    }
    SetState(a, 1);
    STOP_SCRIPT();
    SCRIPT_END;
}

static void (*const sStates[3])(struct Fight *, struct Actor *) = { State0, State1, State2 };

// Paint(); 0x080AD3E8: his painting (type 0x74) on his easel's painting spot, his child;
static void Paint(struct Fight *f, struct Actor *a) {
    a->child = Spawn(f, K_PAINTING, A_ROLLER, sPaintX[f->easel], sPaintY[f->easel]);
}

/* RollerFrame(); 0x080AD0F8: his state set and not his script's, it starts as he's next run; */
static void RollerFrame(struct Fight *f, struct Actor *a) {
    AnimTick(a);
    if (a->script != a->next) Restart(a);
}

/* His paintings (type 0x74, 0x080B0CC8): painted (its blink), then which (Pick()) and to life (each one's script, 0x0874B540): shown
 * on the easel PAINT_TIME frames, then on (its frame's function: its box, its terrain, gone off the screen; PaintingFrame()); */

/* Pick(); 0x080B15C0: his painting by chance (sChances: by his HP's half; the last two painted none, but the last picked once the
 * draw's out on it: NiDL's), the last two painted kept (g+0xC: the newest in its high half); */
static u8 Pick(struct Fight *f) {
    u8 set = HpHalf(f) ? 8 : 0;
    s32 hi = f->painted >> 16, lo = f->painted & 0xFFFF, draw;
    s8 i;

    draw = Rand(f, 255 - sChances[set + hi] - sChances[set + lo]);
    for (i = 7;; i--) {
        if (i != hi && i != lo) draw -= sChances[set + i];
        if (draw <= 0 || i == 0) break;
    }
    f->painted = (s16)(f->painted >> 16) + (i << 16);
    return i;
}

static void ChildScript(struct Fight *f, struct Actor *a);

/* PaintingScript(); Being painted (0x080B0CC8): its blink (0x08753BA4's 0-1, then 2-3 three times, each 2 frames, hidden between), then
 * which (Pick()): its KatAM enemy (enemies/painting.c), untouchable yet; on as its painting at once (its script, this frame); */
static void PaintingScript(struct Fight *f, struct Actor *a) {
    u8 painting;

    SCRIPT;
    a->landed = FALSE;
    for (a->loop = 0; a->loop < 2; a->loop++) {
        SetFrame(a, 0);
        WAIT(2);
        SetFrame(a, FRAME_HIDDEN);
        WAIT(2);
        SetFrame(a, 1);
        WAIT(2);
        SetFrame(a, FRAME_HIDDEN);
        WAIT(2);
    }
    for (a->loop = 0; a->loop <= 2; a->loop++) {
        SetFrame(a, 2);
        WAIT(2);
        SetFrame(a, FRAME_HIDDEN);
        WAIT(2);
        SetFrame(a, 3);
        WAIT(2);
        SetFrame(a, FRAME_HIDDEN);
        WAIT(2);
    }
    painting = Pick(f);
    a->kind = K_CAR + painting;
    a->variant = painting;
    a->enemy = PaintingCreate(f->unk56, a->ix, a->iy, painting);
    a->pc = 0;
    a->wait = 0;
    ChildScript(f, a);
    return;
    SCRIPT_END;
}

/* WalkerScript(); The car, the Kirby, the Waddle Dee, the microphone (0x080B0DE4, 0x080B0E80, 0x080B0F04, 0x080B0F98): shown (4), then
 * at Kirby (3, 2, 2, 1 pixels a frame), falling (0x1500 a frame, 3 at most) till the floor; their steps (5-7; the car 5, 6 on the
 * floor, 4 off it); */
static const s32 sWalks[4] = { 0x30000, 0x20000, 0x20000, 0x10000 };
static const u8 sWalkSteps[2][4] = { { 6, 4, 6, 4 }, { 4, 4, 4, 4 } };

static void WalkerScript(struct Fight *f, struct Actor *a) {
    u8 w = a->kind == K_CAR ? 0 : a->kind == K_KIRBY ? 1 : a->kind == K_WADDLE_DEE ? 2 : 3;

    SCRIPT;
    SetFrame(a, 4);
    WAIT(PAINT_TIME);
    a->live = TRUE;
    FaceKirby(f, a);
    VelX(a, sWalks[w], VEL_KEEP);
    a->ay = 0x1500;
    a->maxY = 0x30000;
    for (;;) {
        if (a->kind == K_CAR) {
            if (a->landed) {
                SetFrame(a, 5);
                WAIT(4);
                SetFrame(a, 6);
                WAIT(4);
            } else {
                SetFrame(a, 4);
                WAIT(1);
            }
            continue;
        }
        for (a->loop = 0; a->loop < 4; a->loop++) {
            SetFrame(a, a->loop == 2 ? 7 : a->loop == 0 ? 5 : 6);
            WAIT(sWalkSteps[a->kind == K_KIRBY][a->loop]);
        }
    }
    SCRIPT_END;
}

/* BallScript(); The baseball (0x080B102C): shown, then at Kirby (his way then, 2 pixels a frame: 0x08064314 3, 0x0806421C), through
 * anything, spinning (4-7, 10 frames each); */
static void BallScript(struct Fight *f, struct Actor *a) {
    u16 angle;

    SCRIPT;
    SetFrame(a, 4);
    WAIT(PAINT_TIME);
    a->live = TRUE;
    angle = KirbyAngle(f, a, 7);
    a->vx = sSine[angle + 128] * 0x200;
    a->vy = sSine[angle] * 0x200;
    for (;;) {
        for (a->loop = 4; a->loop <= 7; a->loop++) {
            SetFrame(a, a->loop);
            WAIT(10);
        }
    }
    SCRIPT_END;
}

/* BombScript(); The bomb (0x080B10C8): shown, then two hops at Kirby (0.5 a frame across; down 0.5, 1, then 2 till the floor; up 2, 0.5,
 * down 0.5, 2 till the floor), its fuse (5, 4) 44 frames, a shake (6, 4, 5, 6, 4) and its blast (K_BLAST: NiDL's puff, its box hurting
 * Kirby, 0x0874BB98: -12, -12 to 12, 8; 0x0806D730); */
static void BombScript(struct Fight *f, struct Actor *a) {
    SCRIPT;
    SetFrame(a, 4);
    WAIT(PAINT_TIME);
    a->live = TRUE;
    Play(a, sBombHop);
    FaceKirby(f, a);
    VelX(a, 0x8000, VEL_KEEP);
    a->vy = 0x8000;
    WAIT(6);
    a->vy = 0x10000;
    WAIT(6);
    a->vy = 0x20000;
    while (!a->landed) WAIT(1);
    a->landed = FALSE;
    a->vy = -0x20000;
    WAIT(6);
    a->vy = -0x8000;
    WAIT(6);
    a->vy = 0x8000;
    WAIT(6);
    a->vy = 0x20000;
    while (!a->landed) WAIT(1);
    Stop(a);
    WAIT(10);
    Play(a, sBombFuse);
    WAIT(44);
    Play(a, NULL);
    SetFrame(a, 6);
    WAIT(2);
    SetFrame(a, 4);
    WAIT(3);
    SetFrame(a, 5);
    WAIT(2);
    SetFrame(a, 6);
    WAIT(2);
    SetFrame(a, 4);
    WAIT(3);
    Sound(f, SFX_BLAST);
    LetEnemyGo(a, FALSE);
    a->kind = K_BLAST;
    for (a->loop = 0; a->loop < 23; a->loop++) {
        a->frame = sPuff[a->loop];
        WAIT(1);
    }
    Destroy(f, a - f->a);
    return;
    SCRIPT_END;
}

/* CloudScript(); The cloud (0x080B1264): shown, then up (2, 1, 0.5 a frame), still, puffing (sCloud), at Kirby (1 a frame), bobbing up
 * and down (sCloudBob, 6 frames each, twice) and a lightning each time round (type 0x75: K_LIGHTNING, where it is); through anything; */
static void CloudScript(struct Fight *f, struct Actor *a) {
    s8 i;

    SCRIPT;
    SetFrame(a, 4);
    WAIT(PAINT_TIME);
    a->live = TRUE;
    a->vy = -0x20000;
    WAIT(4);
    a->vy = -0x10000;
    WAIT(8);
    a->vy = -0x8000;
    WAIT(8);
    Stop(a);
    WAIT(16);
    Play(a, sCloud);
    WAIT(16);
    FaceKirby(f, a);
    VelX(a, 0x10000, VEL_KEEP);
    for (;;) {
        Sound(f, SFX_THUNDER);
        i = Spawn(f, K_LIGHTNING, a - f->a, a->ix, a->iy);
        a->child = i;
        for (a->loop = 0; a->loop <= 1; a->loop++) {
            for (a->v18 = 0; a->v18 < 6; a->v18++) {
                a->vy = sCloudBob[a->v18];
                WAIT(6);
            }
        }
    }
    SCRIPT_END;
}

/* ParasolScript(); The parasol (0x080B14AC): shown, then turning (sParasol); its frame's function moves it (PaintingFrame()); */
static void ParasolScript(struct Fight *f, struct Actor *a) {
    SCRIPT;
    SetFrame(a, 4);
    WAIT(PAINT_TIME);
    a->live = TRUE;
    Play(a, sParasol);
    a->v2C = -32;
    STOP_SCRIPT();
    SCRIPT_END;
}

/* LightningScript(); The cloud's lightning (type 0x75, 0x080B13BC): down 4 pixels a frame (0, 1, 4 frames each) till the floor (its box
 * NiDL's: 4 either way, 0x0874BBB4, hurting Kirby; its terrain a painting's), then out (2-6, its box off), gone; */
static void LightningScript(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->landed = FALSE;
    a->live = TRUE;
    a->vy = 0x40000;
    while (!a->landed) {
        SetFrame(a, 0);
        WAIT(4);
        SetFrame(a, 1);
        WAIT(4);
    }
    a->live = FALSE;
    Stop(a);
    SetFrame(a, 2);
    WAIT(4);
    for (a->loop = 3; a->loop <= 6; a->loop++) {
        SetFrame(a, a->loop);
        WAIT(2);
    }
    Destroy(f, a - f->a);
    return;
    SCRIPT_END;
}

/* SkateDustScript(); 0x0806CD40 (type 0x8F): his skating's dust, three puffs (+0x18) behind him (+0x1C ahead, +0x20 down, where he is
 * then), drifting back and up, its frames by pairs (0x0800617C), a puff of its own halfway (0x90); */
static void SkateDustScript(struct Fight *f, struct Actor *a) {
    struct Actor *p;

    SCRIPT;
    while (a->v18 > 0) {
        p = &f->a[A_ROLLER];
        SetX(a, p->ix + a->v1C);
        SetY(a, p->iy + a->v20);
        a->ay = -0x2000;
        VelX(a, -0x30000, VEL_KEEP);
        SetFrameBy(a, 0);
        WAIT(2);
        a->frame += 2;
        WAIT(2);
        a->frame += 2;
        WAIT(1);
        Spawn(f, K_PUFF_DUST, a - f->a, a->ix, a->iy);
        WAIT(1);
        a->frame -= 2;
        WAIT(2);
        a->frame -= 2;
        WAIT(1);
        a->frame = FRAME_HIDDEN;
        WAIT(1);
        Stop(a);
        a->v18--;
    }
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

/* PuffScript(); A painting gone (NiDL's 0x0806A25C, its end 3: the parasol's time out, PaintingFrame()): its hurt frame a frame (its
 * enemy's: 0, DrivePainting(); its box off), then NiDL's puff where it is (sPuff, a frame each: 0x0806AC48), gone; */
static void PuffScript(struct Fight *f, struct Actor *a) {
    SCRIPT;
    a->live = FALSE;
    a->frame = 0;
    WAIT(1);
    LetEnemyGo(a, FALSE);
    for (a->loop = 0; a->loop < 23; a->loop++) {
        a->frame = sPuff[a->loop];
        WAIT(1);
    }
    Destroy(f, a - f->a);
    return;
    SCRIPT_END;
}

static void ChildScript(struct Fight *f, struct Actor *a) {
    switch (a->kind) {
    case K_PAINTING: PaintingScript(f, a); break;
    case K_CAR: case K_KIRBY: case K_WADDLE_DEE: case K_MIC: WalkerScript(f, a); break;
    case K_BALL: BallScript(f, a); break;
    case K_BOMB: case K_BLAST: BombScript(f, a); break;
    case K_CLOUD: CloudScript(f, a); break;
    case K_PARASOL: ParasolScript(f, a); break;
    case K_LIGHTNING: LightningScript(f, a); break;
    case K_SKATE_DUST: SkateDustScript(f, a); break;
    case K_PUFF_DUST: PuffDustScript(f, a); break;
    case K_HIT: HitScript(f, a); break;
    case K_PUFF: PuffScript(f, a); break;
    default: a->pc = PC_END; break;
    }
}

/* Home(); 0x080645A4 (0x4D00, 1.5 a frame at most: the parasol): its speeds towards the closest Kirby, by its eighth of a circle
 * (0x08064314 0: ArcTan2's >> 13, 0 right, clockwise): faster towards it (0x08064460: by `a`, to `max`, or `max` past it), slower
 * across (0x0806453C: by `a` to none, NiDL's: past it), back (0x080644C0); */
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

static void Home(struct Fight *f, struct Actor *a, s32 accel, s32 max) {
    switch (KirbyAngle(f, a, 13)) {
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

/* PaintingFrame(); A painting's frame's function, come to life (0x080B1564, 0x080B1578, 0x080B123C, 0x080B1398, 0x080B14FC): its
 * animation (the bomb, the cloud, the parasol), the parasol's way (every 16 frames: Home(); after PARASOL_TIME, gone: PuffScript(); its
 * box and the rest only from 32 frames on), its terrain (the car, the Kirby, the Waddle Dee, the microphone, the bomb, the
 * lightning), gone off the screen; */
static void PaintingFrame(struct Fight *f, struct Actor *a) {
    AnimTick(a);
    if (a->kind == K_PARASOL) {
        if (++a->v2C > PARASOL_TIME) {
            // (NiDL's 0x080639A4 0x0874BFD4, 0x0806A25C: its frame and its box kept this frame, the puff's script next: PuffScript());
            a->kind = K_PUFF;
            Stop(a);
            Play(a, NULL);
            a->pc = 0;
            a->wait = 0;
            return;
        }
        if (!(a->v2C & 15)) Home(f, a, 0x4D00, 0x18100);
        if (a->v2C <= 0) return;
    }
    if (a->kind != K_BALL && a->kind != K_CLOUD && a->kind != K_PARASOL) Terrain(f, a);
    if (OffScreen(a)) Destroy(f, a - f->a);
}

/* Hurt (NiDL's 0x0806619C, 13: his handler 0x080AD128, his hurt's end 0x080AD47C): his own script held (NiDL's 0x080055B0 7: his +0x13,
 * his script, his move, his frame's function skipped; the room, the Kirbys, his children go on), flashing in his hurt colours (both
 * palettes: the clock's 2 frames in 4: Colors()), jittering (sJitter), his hit stars; out, back where he was, his state +0x14 (a stand's:
 * on) as he's next run; */
static void HurtStep(struct Fight *f) {
    struct Actor *a = &f->a[A_ROLLER];
    u8 i = sJitterOf[f->hurt];

    a->ix += sJitter[i][0];
    a->iy += sJitter[i][1];
    if (--f->hurt) return;
    a->ix = f->hurtX;
    a->iy = f->hurtY;
    a->frame = f->hurtFrame;
    f->hooked = FALSE;
    f->flashing = FALSE;
    if (f->after >= 0) {
        SetState(a, f->after);
        Restart(a);
    }
}

/* Hit(); A hit took HP off him (KatAM's, between frames; NiDL's in his frame's function, his hurt's first step after it: KatAM's a frame
 * later, so two steps this frame, this one and his own turn's; his hit stars' first frame too): no more for GUARD_TIME frames; */
static void Hit(struct Fight *f) {
    struct Actor *a = &f->a[A_ROLLER];
    s8 i;

    f->guard = GUARD_TIME;
    if (f->hooked) return;
    i = Spawn(f, K_HIT, A_ROLLER, a->ix, a->iy);
    if (i != NONE) {
        f->a[i].v18 = f->a[i].v1C = 0;
        ChildScript(f, &f->a[i]);
    }
    f->hooked = TRUE;
    f->flashing = TRUE;
    f->hurtX = a->ix;
    f->hurtY = a->iy;
    f->hurtFrame = a->frame;
    f->hurt = HURT_TIME;
    HurtStep(f);
}

/* Process(); An Actor's frame (in NiDL's order: Turns()): its script (its next state's, restarted), its move, its frame's function;
 * him hurt, his hurt's step instead; */
static void HitFrame(struct Fight *f, struct Actor *a);

static void Process(struct Fight *f, s8 i) {
    struct Actor *a = &f->a[i];

    if (a->kind == K_NONE) return;
    if (a->kind == K_ROLLER) {
        if (f->hooked) {
            HurtStep(f);
            return;
        }
        if (a->restart) {
            a->restart = FALSE;
            a->script = a->next;
            a->pc = 0;
            a->wait = 0;
        }
        if (a->pc != PC_END && (!a->wait || !--a->wait)) sStates[a->script < 3 ? a->script : 1](f, a);
        Motion(a);
        RollerFrame(f, a);
        return;
    }
    if (a->pc != PC_END && (!a->wait || !--a->wait)) ChildScript(f, a);
    if (a->kind == K_NONE) return;
    if (a->kind != K_HIT) Motion(a);
    if (a->kind == K_HIT) HitFrame(f, a);
    else if ((PAINTED(a->kind) || a->kind == K_LIGHTNING || a->kind == K_BLAST) && a->live) PaintingFrame(f, a);
}

/* Turns(); The Actors' frames in NiDL's order (0x08005228): by group (Group(): the effects', the paintings', his), each by slot; then
 * those spawned meanwhile (NiDL's never run in the turns they're spawned in), by group, and so on (his skate dust, one of his children:
 * his pixel before his move but the frame it's spawned in, traced); */
static void Turns(struct Fight *f) {
    u8 g, q, from, to;
    s8 i;

    f->queued = 0;
    for (g = 1; g <= 3; g++)
        for (i = 0; i < ACTORS; i++)
            if (f->a[i].kind != K_NONE && Group(&f->a[i]) == g) Queue(f, i);
    f->listing = TRUE;
    for (from = 0; from < f->queued; from = to) {
        to = f->queued;
        for (g = 1; g <= 3; g++)
            for (q = from; q < to; q++)
                if (f->groups[q] == g) Process(f, f->queue[q]);
    }
    f->listing = FALSE;
}

/* HitFrame(); 0x0806CBD4: his hit stars on him (his pixel as the frame started: NiDL's, traced), about him by turns; */
static void HitFrame(struct Fight *f, struct Actor *a) {
    u8 k = a->loop & 7;

    a->ix = f->lastX + a->v18 + sHitX[k] * f->a[A_ROLLER].facing;
    a->iy = f->lastY + a->v1C + sHitY[k];
}

/* Their boxes hurting Kirby (attack objects, as King Dedede's hammer: NiDL's 0x08068E04): the lightning's falling (4 either way), the
 * blast's (-12, -12 to 12, 8); */
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
    case K_LIGHTNING:
        box[0] = box[1] = -4; box[2] = box[3] = 4;
        on = a->live;
        break;
    case K_BLAST:
        box[0] = box[1] = -12; box[2] = 12; box[3] = 8;
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

/* DrivePainting(); A painting's KatAM enemy where it is, in its frame (its painting's in its enemy: sPaintingBase; 0 its hurt frame),
 * untouchable till it comes to life (the parasol's, 32 frames more: its box); inhaled or beaten, the Actor goes (NiDL's painting with
 * it), the enemy KatAM's; */
static void DrivePainting(struct Fight *f, struct Actor *a) {
    struct Object *e = a->enemy;
    struct NidlObject *n = (struct NidlObject *)e;

    if (e == NULL) return;
    if (!PaintingDriven(e)) {
        LetEnemyGo(a, TRUE);
        Destroy(f, a - f->a);
        return;
    }
    e->base.x = a->ix << 8;
    e->base.y = a->iy << 8;
    e->base.xspeed = e->base.yspeed = 0;
    if (a->frame >= 4) NidlSetFrame(n, sPaintingBase[a->variant] + a->frame - 4);
    else e->unk83 = 0;
    e->base.flags = (e->base.flags & ~1) | (a->flip == -1 ? 0 : 1);
    e->base.sprite.unk14 = (e->base.sprite.unk14 & ~0x7C0) | (a->kind == K_PUFF || a->kind == K_BLAST ? ORDER_PUFF : ORDER_PAINT) << 6;
    if (a->live && (a->kind != K_PARASOL || a->v2C > 0) && !f->over) e->base.flags &= ~0x200;
    else e->base.flags |= 0x200;
}

/* His box (NiDL's 0x0874B3A8: -12, -12 to 8, 12 facing right) on KatAM's object, where he is: hit, touching hurts; */
static void Body(struct Fight *f) {
    struct Object *obj = f->obj;
    struct Actor *a = &f->a[A_ROLLER];

    obj->base.x = a->ix << 8;
    obj->base.y = a->iy << 8;
    if (a->facing == 1) sub_0803E2B0(&obj->base, -12, -12, 8, 12);
    else sub_0803E2B0(&obj->base, -8, -12, 12, 12);
    if (f->over) obj->base.flags |= 0x200;
    else obj->base.flags &= ~0x200;
}

/* Their sprites (KatAM's: their tiles, NiDL's frames as KatAMExpansion.cs lays them out; mirrored by NiDL's mirroring) and their
 * palettes' slots (loaded under their keys, as their frames have them, the local player in the room; his paintings' blink and his
 * lightning the first paintings enemy's: the descriptor's first palette); */
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
    EffectObjectInitSprite(e, &e->sprite, tiles, gNidlData.bosses[NIDL_PAINT_ROLLER].animId, frame, order);
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
    u16 animId = gNidlData.bosses[NIDL_PAINT_ROLLER].animId;

    if (!pal[PAL_BODY]) pal[PAL_BODY] = Palette(room, animId, BODY(18));
    if (!pal[PAL_TRIM]) pal[PAL_TRIM] = Palette(room, animId | KEY_TRIM, gNidlPaintRoller.front[18]);
    if (!pal[PAL_COMMON]) pal[PAL_COMMON] = Palette(room, animId | KEY_COMMON, gNidlPaintRoller.fx[FX_DUST]);
    if (!pal[PAL_PAINT]) pal[PAL_PAINT] = Palette(room, gNidlData.enemies[NIDL_PAINTING].animId, 0);
}

// Colors(); His palettes this frame: his hurt flash's colours (NiDL's 0x080663F4: the clock's 2 frames in 4, both palettes), else his own;
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

static void Colors(u16 room, const u8 *pal, bool32 flash) {
    if (gKirbys[gLocalPlayerId].base.roomId != room) return;
    SetColors(pal[PAL_BODY], flash ? gNidlPaintRoller.flash : gNidlPaintRoller.body);
    SetColors(pal[PAL_TRIM], flash ? gNidlPaintRoller.flash + 16 : gNidlPaintRoller.trim);
}

// DrawRoller(); His three sprites at (x, y) (8.8) in NiDL's frame `frame`: his body, his second palette's pieces in front, behind;
static void DrawRoller(struct EffectObject **parts, u16 room, const u8 *pal, s32 x, s32 y, s16 frame, s8 flip) {
    static const u8 orders[3] = { ORDER_BODY, ORDER_FRONT, ORDER_BACK };
    static const u8 groups[3] = { G_BODY, G_FRONT, G_BACK };
    u8 i, k;

    for (i = 0; i < 3; i++) {
        if (frame < 4 || frame > 61) k = FRAME_NONE;
        else if (i == 0) k = BODY(frame);
        else k = i == 1 ? gNidlPaintRoller.front[frame] : gNidlPaintRoller.back[frame];
        if (k == FRAME_NONE) {
            Hide(parts[i]);
            continue;
        }
        if (parts[i] == NULL) parts[i] = Sprite(room, gNidlPaintRoller.tiles[groups[i]], k, orders[i]);
        Draw(parts[i], x, y, k, pal[i == 0 ? PAL_BODY : PAL_TRIM], flip, orders[i]);
    }
}

/* Look(); A child's KatAM frame (FRAME_HIDDEN none: the paintings come to life draw themselves, their enemies) and palette; */
static s16 Look(struct Actor *a, u8 *pal) {
    const u8 *fx = gNidlPaintRoller.fx;

    if (a->frame < 0) return FRAME_HIDDEN;
    *pal = PAL_COMMON;
    switch (a->kind) {
    case K_PAINTING:
        *pal = PAL_PAINT;
        return fx[FX_PAINTING] + a->frame;
    case K_LIGHTNING:
        *pal = PAL_PAINT;
        return fx[FX_LIGHTNING] + a->frame;
    case K_SKATE_DUST: case K_PUFF_DUST:
        return fx[FX_DUST] + a->frame;
    case K_HIT:
        return fx[FX_HIT] + (a->frame & 1);
    case K_PUFF: case K_BLAST:
        return a->enemy ? FRAME_HIDDEN : fx[FX_PUFF] + a->frame;
    }
    return FRAME_HIDDEN;
}

static void DrawActor(struct Fight *f, struct Actor *a) {
    u8 pal = 0;
    s16 k = Look(a, &pal);
    u8 order = a->kind == K_PAINTING ? ORDER_PAINT : a->kind == K_PUFF || a->kind == K_BLAST ? ORDER_PUFF : ORDER_FX;

    if (k == FRAME_HIDDEN || !Watched(f)) {
        Hide(a->sprite);
        return;
    }
    if (a->sprite == NULL) a->sprite = Sprite(f->room, gNidlPaintRoller.tiles[G_FX], k, order);
    Draw(a->sprite, a->ix << 8, a->iy << 8, k, f->pal[pal], a->flip, order);
}

static void PaintRollerMain(struct Object *obj);
static void LetGo(struct Fight *f);

/* FightMain(); The fight's task: his sprites and his children's, his palettes; as KatAM's explosion takes him, his children gone (his
 * frame held); gone with him; */
static void FightMain(void) {
    struct Fight *f = TaskGetStructPtr(gCurTask);
    struct Object *obj = f->obj;
    struct Actor *a = &f->a[A_ROLLER];
    u8 i;

    if (obj == NULL || (obj->base.flags & 0x1000)) {
        LetGo(f);
        for (i = 0; i < 3; i++) Release(&f->parts[i]);
        if (obj) ((struct PaintRoller *)obj)->f = NULL;
        TaskDestroy(gCurTask);
        return;
    }
    f->frameCount++;
    Palettes(f->room, f->pal);
    if (obj->unk78 != PaintRollerMain && !f->over) {
        // (his killing blow: KatAM's defeat, its freeze and explosion; NiDL's 0x0806B12C);
        f->over = TRUE;
        f->hooked = FALSE;
        f->flashing = FALSE;
        if (f->hurt) {
            a->ix = f->hurtX;
            a->iy = f->hurtY;
            f->hurt = 0;
        }
        LetGo(f);
        Stop(a);
        SetFrame(a, DEFEAT_FRAME);
    }
    Colors(f->room, f->pal, f->flashing && (f->frameCount & 2));
    if (!Watched(f)) {
        for (i = 0; i < 3; i++) Hide(f->parts[i]);
    } else {
        DrawRoller(f->parts, f->room, f->pal, a->ix << 8, a->iy << 8, a->frame, a->flip);
    }
    for (i = A_ROLLER + 1; i < ACTORS; i++) {
        struct Actor *c = &f->a[i];

        if (c->kind != K_NONE) DrawActor(f, c);
    }
}

// LetGo(); His children gone (their boxes, the paintings' enemies);
static void LetGo(struct Fight *f) {
    s8 i;

    for (i = A_ROLLER + 1; i < ACTORS; i++) Destroy(f, i);
    f->a[A_ROLLER].child = NONE;
}

/* The fight (PaintRollerMain()): NiDL's each frame: a hit taken first (Hit(): KatAM's come between frames); each of NiDL's objects in its
 * order (Turns(): by group, then slot; hurt, his own script held), their boxes and the paintings' enemies, KatAM's object where he is (Body()).
 * His states:
 * • 0: his intro on the ledge's left easel (his frames 18-26) while the HP bar fills;
 * • 1: skating: off his easel and to another (by NiDL's random numbers, his ways about them counted: twice one way, the other side's;
 *   twice the other, across), his moves NiDL's keyframes (skating 2 pixels a frame, a jump onto a ledge or down from it, a loop); his
 *   HP at its half or over, a stand;
 * • 2: painting: up to the easel, his strokes, his painting (Pick(): by chance, by his HP's half, never the last two), back; a stand or
 *   none, by turns (two more, his HP at its half or over); then skating again.
 * His paintings, each a KatAM enemy Kirby inhales for its ability (enemies/painting.c), 60 frames on the easel, then: the car at Kirby (3
 * pixels a frame, Wheel), the Kirby (2) and the Waddle Dee (2, no ability), the microphone (1, Mike), the baseball at him (2, through
 * anything, Ball), the bomb (two hops, its fuse, its blast, Crash), the cloud (up, at Kirby, bobbing, its lightning every 72 frames,
 * Spark), the parasol (after Kirby, 256 frames, Parasol); NiDL's terrain for the walkers and the bomb; gone 64 pixels off NiDL's screen.
 * Hit (KatAM's attacks: KatAM's bosses take only the stronger ones), he freezes 13 frames, jittering, flashing in his hurt colours, his
 * hit stars; a stand's then over; no more hits for 31 frames. His box (NiDL's: -12, -12 to 8, 12) hurts Kirby as KatAM's enemies do,
 * the lightning's and the blast's too. 60 HP (NiDL's 60), a hit taking half again his HP (as 40: 4 spit stars, NidlBossHitMore()); */
static void PaintRollerMain(struct Object *obj) {
    struct PaintRoller *p = (struct PaintRoller *)obj;
    struct Fight *f = p->f;
    s8 i;

    NidlUpdate(&p->n);
    obj->base.flags |= 4 | 0x400;
    obj->kirby3 = FindTargetKirby(&obj->base);
    // The HP bar goes to whatever is hit; he takes it back;
    sub_08034E14(obj);
    if (f == NULL) return;
    obj->base.xspeed = obj->base.yspeed = 0;
    f->lastX = f->a[A_ROLLER].ix;
    f->lastY = f->a[A_ROLLER].iy;
    if (f->guard && !--f->guard) obj->base.flags &= ~0x8000;
    if (!f->over && obj->unk80 < f->hp) {
        // (one through all the same: its HP back; one that takes half again its HP out: KatAM's defeat, nidl_boss.c);
        if (f->guard) obj->unk80 = f->hp;
        else if (NidlBossHitMore(obj, f->hp, HIT_HALVES)) return;
        else Hit(f);
    }
    f->hp = obj->unk80;
    if (f->guard) obj->base.flags |= 0x8000;
    Turns(f);
    for (i = A_ROLLER + 1; i < ACTORS; i++) {
        struct Actor *a = &f->a[i];

        if (a->kind == K_NONE) continue;
        Attack(f, a);
        DrivePainting(f, a);
    }
    Body(f);
}

// PaintRollerGone(); His object's gone (nidl.c's NidlDestroy()): the fight's task lets go of it;
void PaintRollerGone(struct Object *obj) {
    struct PaintRoller *p = (struct PaintRoller *)obj;

    if (p->f) p->f->obj = NULL;
    p->f = NULL;
}

/* PaintRollerStart(); The fight starts (NidlBossStart(): Kirby in his area), its task with it: NiDL's 0x080ACEA8: his globals (on easel
 * 1, his last two paintings 1, his HP's half), his intro (state 0); */
void PaintRollerStart(struct Object *obj) {
    struct PaintRoller *p = (struct PaintRoller *)obj;
    struct Task *t;
    struct Fight *f;
    struct Actor *a;

    ObjectSetFunc(obj, -1, PaintRollerMain);
    if (p->started) return;
    p->started = TRUE;
    t = TaskCreate(FightMain, sizeof(struct Fight), 0x3500, TASK_USE_EWRAM, NULL);
    f = p->f = TaskGetStructPtr(t);
    CpuFill32(0, f, sizeof(struct Fight));
    f->obj = obj;
    f->room = obj->base.roomId;
    f->unk56 = obj->base.unk56;
    f->seed = Rand16() & 0xFFF;
    f->hp = obj->unk80;
    f->half = obj->unk80 >> 1;
    f->easel = 1;
    f->stands = 1;
    f->painted = 0x10001;
    f->way = 0;
    f->after = -1;
    a = &f->a[A_ROLLER];
    a->kind = K_ROLLER;
    a->facing = a->flip = 1;
    a->frame = FRAME_HIDDEN;
    a->parent = A_ROLLER;
    a->child = NONE;
    a->x = obj->base.x << 8;
    a->y = obj->base.y << 8;
    a->ix = a->x >> 16;
    a->iy = a->y >> 16;
    Stop(a);
    SetState(a, 0);
    a->script = 0;
    // (his sprites are the fight's task's: KatAM's object's never shown, its VRAM theirs; unkC 0x200 keeps KatAM from giving it VRAM
    // again, flag 8 from animating it);
    obj->base.unkC |= 0x200;
    if (obj->base.sprite.tilesVram >= 0x6014000) VramFree(obj->base.sprite.tilesVram);
    obj->base.sprite.tilesVram = 0;
    obj->base.flags |= 8;
}

/* PaintRollerInit(); On NiDL's first spot (his easel 1's: 23, 65), hidden (the fight's task draws him), through the room's terrain (his
 * moves are his own: Motion()); his box; */
void PaintRollerInit(struct NidlObject *n) {
    struct PaintRoller *p = (struct PaintRoller *)n;
    struct Object *obj = &n->obj;

    p->f = NULL;
    p->started = FALSE;
    obj->base.x = sEaselX[1] << 8;
    obj->base.y = sEaselY[1] << 8;
    obj->base.flags |= 0x400 | 0x100;
    obj->base.unk5C = (obj->base.unk5C & ~7) | 3 | 0xA0;
    sub_0803E2B0(&obj->base, -12, -12, 8, 12);
    ObjectSetBounds(&obj->base, -12, -12, 8, 12);
}
