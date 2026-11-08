/* Squeak Squad enemies; Characters ported from Kirby: Squeak Squad (USA) all share object type 0x2C (OBJ_EMPTY_2C, KatAM's last free
 * regular enemy type; NiDL's have 0x31), and the template's behavior byte picks which one (SQUEAK_*), its unk12 byte the variant
 * (Squeak Squad's spawn records' parameter bytes, see KatAMExpansionSqueak.cs). What the game reads per type is shared, but an
 * object's frame isn't the type's: the game's object drawing (sub_0809A7A4) is patched to ask SqueakAnimFor() which anim and variant
 * to show, and each Squeak Squad character has an anim of its own whose variant is the object's frame index (unk83), so each can
 * have up to 256 frames:
 * • Variants 0-2: its hurt frame (the type's hurt indexes, 0 and 1, and 2, a defeated mini-boss's, sub_0809D1E0), then one variant
 *   per cell of its .cea (SqueakSetCell), in Squeak Squad's order.
 * • Palettes: the anim ID is the palette key (NidlLoadPalette), so each character has its own palette slot.
 * • Animations: Squeak Squad's own, (cell, frames, offset) steps, played by SqueakPlay/SqueakAnimate as Squeak Squad does; the
 *   offsets turn with the character, as Squeak Squad's (they're its x as it faces right). A character with an .mca also has
 *   composite animations (SqueakPlayComposite): compositions of its animations played together, each part its own sprite.
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
    SQUEAK_GUSSA,
    SQUEAK_CRIMP,
    SQUEAK_CRIMP_ROCK,          // Crimp's rock (spawned by it, drawn with its graphics);
    SQUEAK_MAIGA,
    SQUEAK_CRET,
    SQUEAK_CRET_BALL,           // Cret's ball (dropped by it, drawn with its graphics);
    SQUEAK_UJA,
    SQUEAK_BIO_SPARK,
    SQUEAK_BIO_SPARK_KUNAI,     // Bio Spark's kunai and leaf (thrown and left by it, drawn with its graphics);
    SQUEAK_BIO_SPARK_LEAF,
    SQUEAK_STARMAN,
    SQUEAK_BUBBLE_HEAD,
    SQUEAK_BUBBLE_HEAD_BUBBLE,  // Bubble Head's bubbles (blown by it, drawn with its graphics);
    SQUEAK_GAW_GAW,
    SQUEAK_ACCHI,
    SQUEAK_ACCHI_FIRE,          // Acchi's fire (spat by it, drawn with its graphics);
    SQUEAK_ENEMY_COUNT,
};

// Variants before the cells' (see above);
#define SQUEAK_HURT_VARIANTS 3

// Room every ported character gets (SqueakObject and its own fields);
#define SQUEAK_OBJECT_SIZE 0x168

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

/* A composite animation's part (.mca): the character's animation it plays, at what time (absolute: the frames since the composite
 * animation started; else its time within it), where (pixels added to the position as it faces right, y down); */
struct SqueakPart {
    u16 anim;
    bool8 absolute;
    u8 unused;
    s16 x;
    s16 y;
};

// A composition: its parts, drawn together (the first on top, the character's own sprite; the others SqueakPartFx effects);
struct SqueakComposition {
    u8 parts;
    u8 unused[3];
    const struct SqueakPart *part;
};

// A composite animation's step: the composition, for how many frames;
struct SqueakCompositeStep {
    u16 composition;
    u16 frames;
};

// A composite animation: its steps, and whether it loops (once over, it holds its last step);
struct SqueakComposite {
    u8 steps;
    bool8 loops;
    u16 unused;
    const struct SqueakCompositeStep *step;
};

// SqueakGraphics; Where a character's graphics went (KatAMExpansionSqueak.cs's SqueakGraphicsSize: 20 bytes);
struct SqueakGraphics {
    u16 animId;                 // Its anim ID (and palette key);
    u8 cells;                   // Its cells (variants SQUEAK_HURT_VARIANTS on);
    u8 anims;                   // Its animations;
    const struct SqueakAnim *anim;
    u8 compositions;            // Its .mca's compositions and composite animations (SqueakPlayComposite), if it has one;
    u8 composites;
    u16 tiles;                  // The most tiles one of its frames takes (a part's VRAM);
    const struct SqueakComposition *composition;
    const struct SqueakComposite *composite;
};

// Squeak Squad's effects the characters show (KatAMExpansionSqueak.cs's SqueakEffectDefinitions: an animation of effect/efc_cmn*);
enum {
    SQUEAK_FX_DUST,             // 0x02021900's type 3 (efc_cmn0's animation 19): a puff of dust;
    SQUEAK_FX_SPLASH_IN,        // The stage's splashes (SqueakSplash): into the water (m_efc's animation 4), out of it (5);
    SQUEAK_FX_SPLASH_OUT,
    SQUEAK_FX_BURST,            // 0x020B48E0's type 6 (efc_cmn1's animation 1): a thing bursting (Crimp's rock);
    SQUEAK_FX_STARS,            // 0x020B48E0's type 3 (efc_cmn0's animation 7): a star burst (Bio Spark's kunai stuck in a wall);
    SQUEAK_FX_COUNT,
};

// SqueakEffect; An effect: its anim (its frames: the cells its animation shows), the most tiles a frame takes, its animation;
struct SqueakEffect {
    u16 animId;
    u16 tiles;
    struct SqueakAnim anim;
};

// SqueakData; Filled in by KatAMExpansionSqueak.cs once the graphics are built;
struct SqueakData {
    const u8 *names;            // One 64x8 HP bar name (0x100 bytes) per character;
    const struct SqueakEffect *effects;
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
    bool8 animEnded;            // Squeak Squad's flag 8: a held animation got through; a looping one started over this frame;
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
    u8 composite;               // The composite animation playing (SqueakPlayComposite), SQUEAK_NO_COMPOSITE while a plain one plays;
    void *follower;             // The effect following it (SqueakSpawnEffect's `follow`), let go when it's destroyed;
    void *parts;                // The composition's other parts (SqueakPartFx, a chain: SqueakAnimate places them);
    u32 compositeTime;          // The frames since the composite animation started;
    s32 vars[16];               // The character's own;
    struct ObjectTemplate spawn;    // Its template, when another character spawned it (SqueakSpawn);
};

// SqueakTouches()' sides (the stage's vtable +0x18's: 1 up, 2 down, 4 left, 8 right);
enum { SQUEAK_UP = 1, SQUEAK_DOWN = 2, SQUEAK_LEFT = 4, SQUEAK_RIGHT = 8 };

#define SQUEAK_NO_COMPOSITE 0xFF

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

/* Boxes per animation frame (a character's .rca, by Squeak Squad's LoadBoxes, 0x02014010; KatAM's units: pixels from its spot, x
 * ahead, y down; the tables made by boxes.py): the box KatAM's collisions use (sub_0803E2B0's: where it's hit, where its touch
 * hurts), whether that hurts, its grabbing boxes (tag 0x101 kind 6); each animation's frames (time, box); */
struct SqueakBox {
    s8 box[4];
    bool8 harms;
    u8 grabs;
    s8 grab[3][4];
};
struct SqueakBoxFrame {
    u8 frames;
    u8 box;
};
struct SqueakBoxAnim {
    const struct SqueakBoxFrame *frame;
    u8 count;
};
struct SqueakBoxes {
    const struct SqueakBox *box;
    const struct SqueakBoxAnim *anim;
    u8 anims;
};

void *CreateSqueakEnemy(struct ObjectTemplate *, u8 playerId);
void SqueakStart(struct Object *);
u32 SqueakAnimFor(struct Object *);
const u8 *SqueakName(struct Object *);
s16 SqueakMaxHp(struct Object *);
void SqueakSetCell(struct SqueakObject *, u8 cell);
void SqueakPlay(struct SqueakObject *, u8 anim);
void SqueakPlayComposite(struct SqueakObject *, u8 composite);
void SqueakAnimate(struct SqueakObject *);
void SqueakSetState(struct SqueakObject *, u8 state);
void SqueakRun(struct SqueakObject *, bool32 (*state)(struct SqueakObject *));
void SqueakStandOnFloor(struct Object *);

// Squeak Squad's engine (its enemies' shared functions);
void SqueakMove(struct SqueakObject *);
void SqueakThroughTerrain(struct SqueakObject *, bool32 through);
u32 SqueakAnimTime(struct SqueakObject *);
void SqueakFinishFrame(struct SqueakObject *);
void SqueakStop(struct SqueakObject *);
bool32 SqueakCapFall(struct SqueakObject *, s32 cap);
bool32 SqueakSinkTo(struct SqueakObject *, s32 accel, s32 cap);
void SqueakTurnAround(struct SqueakObject *);
bool32 SqueakOnGround(struct SqueakObject *);
bool32 SqueakWallAhead(struct SqueakObject *);
bool32 SqueakCeiling(struct SqueakObject *);
bool32 SqueakInWater(struct SqueakObject *);
void SqueakFaceKirby(struct SqueakObject *, s32 distance);
bool32 SqueakKirbyWithin(struct SqueakObject *, s32 dx, s32 dy);
void SqueakSpawnEffect(struct SqueakObject *, u8 effect, s32 dx, s32 dy, bool8 follow);
void SqueakSplash(struct SqueakObject *, bool8 out);
bool32 SqueakTouches(struct SqueakObject *, u8 side, s32 dy, bool8 both);
struct SqueakObject *SqueakSpawn(struct SqueakObject *, u8 enemy, u8 variant, s32 dx, s32 dy);
bool32 SqueakInLevel(struct SqueakObject *);
void SqueakSetBoxes(struct SqueakObject *, const struct SqueakBoxes *, s16 harm, u8 *index);
bool32 SqueakKirbyFree(struct Kirby *);
struct Kirby *SqueakGrabbing(struct SqueakObject *, const struct SqueakBox *);
struct Kirby *SqueakTouchingKirby(struct SqueakObject *);

// The characters (squeak/*.c);
void PeraraInit(struct SqueakObject *);
void PeraraStart(struct Object *);
void GoldWaddleDeeInit(struct SqueakObject *);
void GoldWaddleDeeStart(struct Object *);
void GussaInit(struct SqueakObject *);
void GussaStart(struct Object *);
void CrimpInit(struct SqueakObject *);
void CrimpStart(struct Object *);
void CrimpRockInit(struct SqueakObject *);
void CrimpRockStart(struct Object *);
void MaigaInit(struct SqueakObject *);
void MaigaStart(struct Object *);
void CretInit(struct SqueakObject *);
void CretStart(struct Object *);
void CretBallInit(struct SqueakObject *);
void CretBallStart(struct Object *);
void UjaInit(struct SqueakObject *);
void UjaStart(struct Object *);
void BioSparkInit(struct SqueakObject *);
void BioSparkStart(struct Object *);
void BioSparkKunaiInit(struct SqueakObject *);
void BioSparkKunaiStart(struct Object *);
void BioSparkLeafInit(struct SqueakObject *);
void BioSparkLeafStart(struct Object *);
void SqueakStarmanInit(struct SqueakObject *);
void SqueakStarmanStart(struct Object *);
void BubbleHeadInit(struct SqueakObject *);
void BubbleHeadStart(struct Object *);
void BubbleHeadBubbleInit(struct SqueakObject *);
void BubbleHeadBubbleStart(struct Object *);
void GawGawInit(struct SqueakObject *);
void GawGawStart(struct Object *);
void AcchiInit(struct SqueakObject *);
void AcchiStart(struct Object *);
void AcchiFireInit(struct SqueakObject *);
void AcchiFireStart(struct Object *);

#endif
