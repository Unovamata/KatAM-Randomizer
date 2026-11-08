/* Squeak Squad enemies; Characters ported from Kirby: Squeak Squad (USA) all share object type 0x2C (OBJ_EMPTY_2C, KatAM's last free
 * regular enemy type; NiDL's have 0x31), and the template's behavior byte picks which one (SQUEAK_*), its unk12 byte the variant
 * (Squeak Squad's spawn records' parameter bytes, see KatAMExpansionSqueak.cs). What the game reads per type is shared, but an
 * object's frame isn't the type's: the game's object drawing (sub_0809A7A4) is patched to ask SqueakAnimFor() which anim and variant
 * to show, and each Squeak Squad character has an anim of its own whose variant is the object's frame index (unk83), so each can
 * have up to 256 frames:
 * • Variants 0-2: its hurt frame (the type's hurt indexes, 0 and 1, and 2, a defeated mini-boss's, sub_0809D1E0), then one variant
 *   per cell of its .cea (SqueakSetCell), in Squeak Squad's order.
 * • Palettes: the anim ID is the palette key (NidlLoadPalette), so each character has its own palette slot.
 * • Animations: Squeak Squad's own, (cell, frames, offset) steps, played by SqueakPlay/SqueakAnimate as Squeak Squad does.
 * • Speeds: Squeak Squad's are 20.12 fixed point a frame (0x1000 a pixel), KatAM's 24.8 (0x100): SQUEAK_SPEED converts. Both run
 *   at 60 frames a second.
 * The tables filled by KatAMExpansionSqueak.cs (anims, animations, names) are in gSqueakData; */
#ifndef GUARD_SQUEAK_H
#define GUARD_SQUEAK_H

#include "nidl.h"

#define SQUEAK_TYPE OBJ_EMPTY_2C

// Characters (the behavior byte);
enum {
    SQUEAK_PERARA,
    SQUEAK_GOLD_WADDLE_DEE,
    SQUEAK_ENEMY_COUNT,
};

// Variants before the cells' (see above);
#define SQUEAK_HURT_VARIANTS 3

// Room every ported character gets (SqueakObject and its own fields);
#define SQUEAK_OBJECT_SIZE 0x140

// SQUEAK_SPEED(); A Squeak Squad speed or position (20.12 fixed point) in KatAM's units (24.8);
#define SQUEAK_SPEED(value) ((value) >> 4)

// An animation step: the cell, how many frames it shows, and where it is drawn (x, y: pixels, y down; added to the position);
struct SqueakStep {
    u16 cell;
    u16 frames;
    s16 x;
    s16 y;
};

// An animation: its steps, and whether it loops (once over, it holds its last step);
struct SqueakAnim {
    u8 steps;
    bool8 loops;
    u16 unused;
    const struct SqueakStep *step;
};

// SqueakGraphics; Where a character's graphics went;
struct SqueakGraphics {
    u16 animId;                 // Its anim ID (and palette key);
    u8 cells;                   // Its cells (variants SQUEAK_HURT_VARIANTS on);
    u8 anims;                   // Its animations;
    const struct SqueakAnim *anim;
};

// SqueakData; Filled in by KatAMExpansionSqueak.cs once the graphics are built;
struct SqueakData {
    const u8 *names;            // One 64x8 HP bar name (0x100 bytes) per character;
    struct SqueakGraphics enemies[SQUEAK_ENEMY_COUNT];
};

extern const struct SqueakData gSqueakData;

/* SqueakObject; Every ported character starts with it: its animation and state (SqueakAnimate, SqueakRun), and Squeak Squad's
 * physics and pad, the fields the engine's helpers below use (the body's 0x8C-0xA8 and its brain's 0x14-0x20); */
struct SqueakObject {
    struct Object obj;
    u8 enemy;
    u8 variant;
    u8 anim;                    // The Squeak Squad animation playing, its step and the frames left on it;
    u8 step;
    u16 timer;
    bool8 animEnded;            // The animation got through once (Squeak Squad's flag 8; a looping one goes on);
    u8 state;                   // The character's own state, and its timer (Squeak Squad's: frames since the state started);
    u16 stateTimer;
    u8 brain;                   // Its brain's state (one run a frame);
    bool8 started;              // Its start ran once (it runs again after each hit: the body starts over, not the brain);
    s32 vx, vy;                 // Speed (forward, up), acceleration, the speeds' caps (0: none) and friction;
    s32 ax, ay;
    s32 maxVx, maxVy;
    s32 fx, fy;
    s32 padDir;                 // Its brain's pad: direction (1 forward, -1 turn around, 0 none), up/down, buttons A and B;
    s32 padY;
    s32 padA;
    s32 padB;
    bool8 moved;                // SqueakMove moved it this frame (SqueakFinishFrame);
    u8 terrain;                 // What its last move met (SqueakMove): KatAM's unk62 (floor 4, wall ahead 1, behind 2) and unk58 (2: water);
    u8 water;
    u8 unused;
    s32 vars[19];               // The character's own;
};

// The position in Squeak Squad's units (20.12, Y up);
#define SQ_X(obj) ((obj)->base.x << 4)
#define SQ_Y(obj) (-(obj)->base.y << 4)

// SqueakCharacter; What the code knows about each character;
struct SqueakCharacter {
    void (*init)(struct SqueakObject *);    // Bounds and position, after InitObject;
    void (*start)(struct Object *);         // Behavior start, also after recovering from a hit;
    u16 ability;
    bool8 swims;                // It has moves in water (KatAM's engine drowns the others, unkC 1 unset: sub_0809D710);
};

extern const s16 gSqueakEnemyHp[];

void *CreateSqueakEnemy(struct ObjectTemplate *, u8 playerId);
void SqueakStart(struct Object *);
u32 SqueakAnimFor(struct Object *);
const u8 *SqueakName(struct Object *);
s16 SqueakMaxHp(struct Object *);
void SqueakSetCell(struct SqueakObject *, u8 cell);
void SqueakPlay(struct SqueakObject *, u8 anim);
void SqueakAnimate(struct SqueakObject *);
void SqueakSetState(struct SqueakObject *, u8 state);
void SqueakRun(struct SqueakObject *, bool32 (*state)(struct SqueakObject *));
void SqueakStandOnFloor(struct Object *);

// Squeak Squad's engine (its enemies' shared functions);
void SqueakMove(struct SqueakObject *);
void SqueakFinishFrame(struct SqueakObject *);
void SqueakStop(struct SqueakObject *);
bool32 SqueakCapFall(struct SqueakObject *, s32 cap);
bool32 SqueakSinkTo(struct SqueakObject *, s32 accel, s32 cap);
void SqueakTurnAround(struct SqueakObject *);
bool32 SqueakOnGround(struct SqueakObject *);
bool32 SqueakWallAhead(struct SqueakObject *);
bool32 SqueakInWater(struct SqueakObject *);
void SqueakFaceKirby(struct SqueakObject *, s32 distance);
bool32 SqueakKirbyWithin(struct SqueakObject *, s32 dx, s32 dy);

// The characters (squeak/*.c);
void PeraraInit(struct SqueakObject *);
void PeraraStart(struct Object *);
void GoldWaddleDeeInit(struct SqueakObject *);
void GoldWaddleDeeStart(struct Object *);

#endif
