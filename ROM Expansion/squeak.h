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
    SQUEAK_BUN,
    SQUEAK_METALUN,
    SQUEAK_BIG_METALUN,         // A mid-boss (on a mini-boss type, see below);
    SQUEAK_GAO_GAO,             // A mid-boss;
    SQUEAK_STAR,                // The star a mid-boss knocks out (effect/efc_cmn0's; its variants SQUEAK_STAR_*);
    SQUEAK_TEDHAUN,             // A mid-boss;
    SQUEAK_TEDHAUN_GHOST,       // Its little ghosts (drawn with its graphics);
    SQUEAK_BUBOO,               // A mid-boss;
    SQUEAK_BUBOO_BUBBLE,        // Its bubbles (blown and dropped by it, drawn with its graphics);
    SQUEAK_SQUEAKER,            // The Squeak Squad's Squeakers (their colors the variant);
    SQUEAK_SQUEAKER_BOMB,       // Their bombs (thrown by them, drawn with their graphics; the variant the big one);
    SQUEAK_SPINNI,              // The Squeak Squad's Spinni (the variant 1: no change of pace);
    SQUEAK_SPINNI_THROWN,       // What it throws (its shurikens, its slash, its claws knocked off: the variant; drawn with its graphics);
    SQUEAK_DOC,                 // The Squeak Squad's Doc, in his flying saucer;
    SQUEAK_DOC_THING,           // What it makes (its bolts, bomb, missile, its saucer's lid, its empty saucer: the variant);
    SQUEAK_STORO,               // The Squeak Squad's strongman, with his mallet;
    SQUEAK_DAROACH,             // The Squeak Squad's leader in a stage's scene (variant bit 0: facing right);
    SQUEAK_KING_DEDEDE,         // A boss (on a mini-boss type, as the mid-bosses);
    SQUEAK_DEDEDE_PUFF,         // The air he lets out after floating (drawn with his graphics);
    SQUEAK_MRS_MOLEY,           // A boss (on a mini-boss type, as the mid-bosses);
    SQUEAK_MOLEY_THING,         // What she throws (Squeak Squad's kind the variant: rocks, a bolt, a ring, a bomb, a can, a saw, pebbles);
    SQUEAK_MOLEY_DIRT,          // Her dirt (Squeak Squad's kind the variant's low bits: the petals' burst, petals, her shadow; their way 4 on);
    SQUEAK_MECHA_KRACKO,        // A boss (on a mini-boss type, as the mid-bosses);
    SQUEAK_MECHA_KRACKO_EYE,    // Its eye (drawn with its graphics, over it, looking at Kirby);
    SQUEAK_YADGAINE,            // A boss (on a mini-boss type, as the mid-bosses);
    SQUEAK_YADGAINE_LEG,        // Its legs (drawn with its graphics; the variant 0 its back one, 1 its front one);
    SQUEAK_YADGAINE_THING,      // What it throws and knocks down (Squeak Squad's kind the variant: its shot, blocks, rocks);
    SQUEAK_YADGAINE_GORDO,      // A gordo falling in its fight (enemy/gordo);
    SQUEAK_YADGAINE_DAROACH,    // Daroach bringing it in (drawn with his graphics);
    SQUEAK_BOHBOH,              // A boss (on a mini-boss type, as the mid-bosses);
    SQUEAK_BOHBOH_THING,        // What it brings in (the variant: its fire, a fire from the top, the lava's hurt);
    SQUEAK_DAROACH_BOSS,        // A boss (on a mini-boss type, as the mid-bosses): Daroach, enemy 0x3D;
    SQUEAK_DAROACH_BOSS_THING,  // What he brings in (the variant: his stars, his bomb, his beam's reach);
    SQUEAK_DARK_DAROACH,        // Enemy 0x3D with its parameter 1 (his other moves, dancho01's colors), as Daroach;
    SQUEAK_DARK_DAROACH_THING,
    SQUEAK_META_KNIGHT,         // A boss (on a mini-boss type, as the mid-bosses): Meta Knight, enemy 0x37;
    SQUEAK_META_KNIGHT_THING,   // What he brings in (the variant): his tornado;
    SQUEAK_META_KNIGHT_BEAM,    // His beam, his wave, the tornado's sparkles (and his sword's reach, drawing nothing): each drawn
    SQUEAK_META_KNIGHT_WAVE,    // with its own frames (VRAM the size of its own, not the tornado's);
    SQUEAK_META_KNIGHT_SPARKLE,
    SQUEAK_DARK_NEBULA,         // A boss (on a mini-boss type, as the mid-bosses): Dark Nebula, enemies 0x46 (his coming) and 0x3F;
    SQUEAK_DARK_NEBULA_EYE,     // His eye (Squeak Squad's +0x1F8), its twinkle, his flames (+0x1FC): each drawn by a sprite of its own;
    SQUEAK_DARK_NEBULA_TWINKLE,
    SQUEAK_DARK_NEBULA_AURA,
    SQUEAK_DARK_NEBULA_SHOT,    // His stars, sparks and flames (and his cage's hurting sides, drawing nothing);
    SQUEAK_DARK_NEBULA_FIREBALL,
    SQUEAK_DARK_NEBULA_FIRE,    // His fireball's burst and its fire wall;
    SQUEAK_DARK_NEBULA_ICE,     // His ice pillar;
    SQUEAK_DARK_NEBULA_BOLT,    // His bolts and their cage;
    SQUEAK_DARK_NEBULA_BURN,    // The floor his flames set burning;
    SQUEAK_DARK_STAR,           // Enemy 0x3E: the dark star Kirby chases before Dark Nebula;
    SQUEAK_PLATINUM_CHEST,      // Graphics only: Squeak Squad's big chest, drawn by KatAM's big chest in its rooms (SqueakAnimFor());
    SQUEAK_SWITCH,              // Graphics only: Squeak Squad's small star switch (squeak/gimmicks.c), up and pressed;
    SQUEAK_SHUTTER,             // Graphics only: the shutters (squeak/gimmicks.c), a stone pillar upright, turned 90 and 270 degrees,
    SQUEAK_SHUTTER_90,          // and spikes up and down; each its every cell, the DS's animations 0 shut, 1 opening, 2 open, 3 shutting;
    SQUEAK_SHUTTER_270,
    SQUEAK_NEEDLE,
    SQUEAK_NEEDLE_180,
    SQUEAK_FIRE,                // Graphics only: the fire columns (squeak/gimmicks.c), up, down, and turned 270 and 90 degrees;
    SQUEAK_FIRE_180,
    SQUEAK_FIRE_270,
    SQUEAK_FIRE_90,
    SQUEAK_GRASS,               // Graphics only: the grass tufts (squeak/gimmicks.c), A, B and C, each small and large, and the barrel;
    SQUEAK_GRASS_LAST = SQUEAK_GRASS + 5,
    SQUEAK_BARREL,
    SQUEAK_ICICLE,              // Graphics only: the icicles (squeak/gimmicks.c);
    SQUEAK_ROCK,                // Graphics only: the falling rocks (squeak/gimmicks.c);
    SQUEAK_FLOOR,               // Graphics only: the moving platforms (squeak/gimmicks.c);
    SQUEAK_THUNDER,             // Graphics only: the thunder clouds (squeak/gimmicks.c);
    SQUEAK_BOULDER,             // Graphics only: the rolling boulders (squeak/gimmicks.c), its roll animation 3;
    SQUEAK_CANDLE,              // Graphics only: the candles (squeak/gimmicks.c), 0 lit, 1 out;
    SQUEAK_ENEMY_COUNT,
};

/* Mid-bosses: on the mini-boss types NiDL's use (0x40-0x42, which KatAM treats as mini-bosses everywhere: HP bar, music, camera,
 * defeat), told from NiDL's by the template's behavior byte, SQUEAK_MIDBOSS | the character (NiDL's are under 0x60); its unk12 byte
 * the variant. Created by CreateSqueakMidboss (the mini-bosses' spawn hands them over), they wait facing Kirby till he's in the
 * template's area, then the camera locks there, the HP bar shows and the mini-boss music plays, as KatAM's own mini-bosses do; */
#define SQUEAK_MIDBOSS 0x80
/* SQUEAK_STAR's variants: the star a boss spits Kirby out in (Squeak Squad's kind 0: King Dedede's; SqueakStarCarry() puts the Kirbys
 * he swallowed in it), and flying off forward or back (kinds 1 and 2, 0x0212389C); kinds 3 and 4 as 1 and 2, a Sword when inhaled
 * (0x0203A9C4's 16: Meta Knight's); */
enum { SQUEAK_STAR_SPIT, SQUEAK_STAR_AHEAD, SQUEAK_STAR_BEHIND, SQUEAK_STAR_SWORD_AHEAD, SQUEAK_STAR_SWORD_BEHIND };
#define SqueakIsMidboss(obj) (NidlIsMiniboss(obj) && (obj)->objTemplate->subtype1 >= SQUEAK_MIDBOSS)
// SqueakIsSqueak(); An object that's a Squeak Squad character (SqueakObject): on its own type or a mid-boss;
#define SqueakIsSqueak(obj) ((obj)->type == SQUEAK_TYPE || SqueakIsMidboss(obj))

// Variants before the cells' (see above);
#define SQUEAK_HURT_VARIANTS 3

/* Squeak Squad's chests (KatAMExpansionSqueakRooms.cs, chest.c): KatAM's small and big chests with this treasure index (template
 * unk11); a big one shows SQUEAK_PLATINUM_CHEST's variants (closed, open); */
#define SQUEAK_CHEST 0x9F
// (a fixed cannon's template unk11, chest.c's SqueakCreateCannon; the frames it waits with a Kirby in it, as Squeak Squad's);
#define SQUEAK_CANNON 0x9E
#define SQUEAK_CANNON_TURNING 0x9D
// (a switch's and a shutter's template unk11, on object 0x8D: squeak/gimmicks.c's SqueakCreateSwitch);
#define SQUEAK_SWITCH_OBJECT 0x9C
#define SQUEAK_SHUTTER_OBJECT 0x9B
#define SQUEAK_FIRE_OBJECT 0x9A
#define SQUEAK_GRASS_OBJECT 0x99
#define SQUEAK_BARREL_OBJECT 0x98
#define SQUEAK_BLADES_OBJECT 0x97
#define SQUEAK_ICICLE_OBJECT 0x96
#define SQUEAK_ROCK_OBJECT 0x95
#define SQUEAK_PLATFORM_OBJECT 0x94
#define SQUEAK_CLOUD_OBJECT 0x93
#define SQUEAK_ROPE_OBJECT 0x92
#define SQUEAK_BOULDER_OBJECT 0x91
#define SQUEAK_CANDLE_OBJECT 0x90
/* The gimmicks' messages (squeak/gimmicks.c): per loaded room's slot (gCurLevelInfo[].unk65E) its room, how many times a room was
 * loaded in it, and a byte per gimmick of the room's list, the kind + 1 of the message waiting for it (rooms.c keeps them after the
 * EWRAM heap: SqueakMailOf()); */
#define SQUEAK_MAIL_GIMMICKS 60
struct SqueakMail {
    u16 room;
    u8 generation;
    u8 unused;
    u8 box[SQUEAK_MAIL_GIMMICKS];
};
struct SqueakMail *SqueakMailOf(u8 slot);
void SqueakGimmicksLoaded(u8 playerId, u16 roomId, u8 slot);
void SqueakSendMail(u8 slot, u8 index, u8 kind);
u32 SqueakGimmickAnim(struct Object *obj);
#define SQUEAK_CANNON_WAIT 35
// (how long a cannon's shot flies at most, frames: Squeak Squad's carries Kirby on until he hits something or a cannon catches him);
#define SQUEAK_CANNON_FLIGHT 150
// (the first of the Squeak Squad rooms' ids, KatAMExpansionSqueakRooms.cs' SqueakFirstRoom: chest.c's SqueakCannonArrival);
#define SQUEAK_FIRST_ROOM 0x400
/* (a world's boss, in its boss room: its template's unk11 is the mirror shard it leaves + 1, KatAMExpansionSqueakRooms.cs; beaten, as
 * it goes, the shard comes, or a Maxim Tomato, as NiDL's bosses', nidl_boss.c's NidlBossShard(): the way out to the Squeak Squad hub,
 * gNidlShardExits; SqueakDestroy()); */
#define SqueakBossShard(obj) ((obj)->objTemplate->unk11)
/* (its unk14: where the shard comes, x in the room, pixels: the boss room's chest's, where the DS drops its reward, centred on its
 * art (King Dedede's machine, 12 pixels right of the room's middle); 0 the screen's middle, as NiDL's (nidl_boss.c's SHARD_X, Y);
 * the boss room's chest (its template's unk14 SQUEAK_CHEST_AFTER_BOSS) shows only once its boss is gone, as the DS drops it
 * then: chest.c's SqueakCreateChest); */
#define SQUEAK_SHARD_Y 72
#define SQUEAK_CHEST_AFTER_BOSS 1
// (a Vitality half's chest: its template's unk14's high byte; chest.c's NidlChestOpen, two make one Vitality);
#define SQUEAK_CHEST_HALF 1
/* (Squeak Squad's spray paints not in KatAM, KatAMExpansionSprays.cs: owned in gTreasures.sprayPaintField's bits SQUEAK_SPRAY_BIT up, Kirby colors
 * SQUEAK_SPRAY_COLOR up; a chest holding one: its template's unk16, the spray's number + 1, its item KatAM's spray of the closest
 * color, which NidlChestOpen gives back); */
#define SQUEAK_SPRAYS 5
#define SQUEAK_SPRAY_BIT 14
#define SQUEAK_SPRAY_COLOR 16
/* (the expansion's areas' maps, KatAMExpansionMaps.cs: the NiDL levels' then the Squeak Squad worlds', owned in gTreasures.bigChestField's
 * bits AREA_MAP_BIT up (KatAM's 10 maps take bits 0-9); a chest holding one: its template's unk16 CHEST_MARK_MAP << 8 | the map's
 * number + 1, its item one of KatAM's maps 1-9 (sub_0800AFC8's 0xB-0x13), which NidlChestOpen gives back; the popup the map's); */
#define AREA_MAPS 16
#define AREA_MAP_BIT 10
#define CHEST_MARK_MAP 1
bool32 SqueakBossAlive(u16 room);
#define SQUEAK_CHEST_CLOSED SQUEAK_HURT_VARIANTS

// Room every ported character gets (SqueakObject and its own fields);
#define SQUEAK_OBJECT_SIZE 0x1A0

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
    u8 tiles;                   // The tiles its sprite takes (the most a frame of its animation needs);
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

/* SqueakPaletteKey; A palette's slot (SqueakLoadPalette): its key and the frame of the key's anim it's loaded from; sprites in the
 * same colors share one (KatAMExpansionSqueak.cs's SqueakPaletteKey: KatAM keeps 7 palettes for a room's enemies); */
struct SqueakPaletteKey {
    u16 key;
    u16 variant;
};

// SqueakGraphics; Where a character's graphics went (KatAMExpansionSqueak.cs's SqueakGraphicsSize: 24 bytes);
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
    const struct SqueakPaletteKey *palette;     // Per animation its palette, its cells in different ones (Doc's things); else NULL;
};

// Squeak Squad's effects the characters show (KatAMExpansionSqueak.cs's SqueakEffectDefinitions: an animation of effect/efc_cmn*);
enum {
    SQUEAK_FX_DUST,             // 0x02021900's type 3 (efc_cmn0's animation 19): a puff of dust;
    SQUEAK_FX_SPLASH_IN,        // The stage's splashes (SqueakSplash): into the water (m_efc's animation 4), out of it (5);
    SQUEAK_FX_SPLASH_OUT,
    SQUEAK_FX_BURST,            // 0x020B48E0's type 6 (efc_cmn1's animation 1): a thing bursting (Crimp's rock);
    SQUEAK_FX_STARS,            // 0x020B48E0's type 3 (efc_cmn0's animation 7): a star burst (Bio Spark's kunai stuck in a wall);
    SQUEAK_FX_LAND,             // 0x020219C4's types 1 and 2 (efc_cmn0's animations 23 and 22): a mid-boss's landing puffs;
    SQUEAK_FX_LAND_BIG,
    SQUEAK_FX_DOC_BALL,         // Doc's composite animations' other parts (doc_at1's animations 4, 5, 11, 12, 18, 26);
    SQUEAK_FX_DOC_RINGS,
    SQUEAK_FX_DOC_BALL_LOW,
    SQUEAK_FX_DOC_RINGS_LOW,
    SQUEAK_FX_DOC_BOMB,
    SQUEAK_FX_DOC_SMOKE,
    SQUEAK_FX_YADGAINE_SHELL,   // Yadgaine's shell (its stage's picture: KatAMExpansionSqueak.cs's SqueakShell, at three quarters of its size);
    SQUEAK_FX_COUNT,
};

// SqueakEffect; An effect: its anim (its frames: the cells its animation shows), the most tiles a frame takes, its animation, its palette;
struct SqueakEffect {
    u16 animId;
    u16 tiles;
    struct SqueakAnim anim;
    struct SqueakPaletteKey palette;
};

// SqueakData; Filled in by KatAMExpansionSqueak.cs once the graphics are built;
/* SqueakLayer; A character's layer (KatAMExpansionSqueak.cs's Layers: the pieces of its cells in other palettes than its own, a
 * weapon in its hands, Doc's saucer): an anim of its own (its own palette), its frames' most tiles, per cell its frame + 1 (0: none; | 0x8000: drawn
 * in front of the character, its first piece on the DS being one of them), its palette; a character without one: anim ID 0; */
#define SQUEAK_LAYERS 2

struct SqueakLayer {
    u16 animId;
    u16 tiles;
    const u16 *cell;            // Per cell its layer frame + 1 (0: none), | 0x8000 drawn in front;
    struct SqueakPaletteKey palette;
};

struct SqueakData {
    const u8 *names;            // One 64x8 HP bar name (0x100 bytes) per character;
    const struct SqueakEffect *effects;
    struct SqueakGraphics enemies[SQUEAK_ENEMY_COUNT];
    const struct SqueakLayer *layers;   // SQUEAK_LAYERS per character;
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
    void *layer[2];             // Its layers' sprites (SqueakShowLayer), while its cell has one;
    u32 compositeTime;          // The frames since the composite animation started;
    s32 vars[27];               // The character's own;
    struct ObjectTemplate spawn;    // Its template, when another character spawned it (SqueakSpawn);
    bool8 activated;            // A mid-boss: its fight has started;
    u8 level;                   // Its KatAM hit level (unk5C's 7) while SqueakUntouchable holds it at 6, | 0x80; 0 when not held;
    s16 maxHp;                  // Its HP bar's full HP, when not its table's (0: the table's: SqueakMaxHp), and the HP under the bar's
    s16 hpBase;                 // empty (a boss's next form's, held back: Mecha-Kracko's);
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
    bool8 midboss;              // A mid-boss: created on a mini-boss type only (CreateSqueakMidboss);
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
    u32 untouchable[2];         // The animations (bits: 0-31, 32-63) whose .rca hit boxes are flagged 0x8000: no attack takes it (0x0201322C);
};
// SQUEAK_ANIM_LO(), SQUEAK_ANIM_HI(); An animation's bit in SqueakBoxes' untouchable words;
#define SQUEAK_ANIM_LO(a) ((a) < 32 ? 1u << ((a) & 31) : 0)
#define SQUEAK_ANIM_HI(a) ((a) >= 32 ? 1u << ((a) & 31) : 0)

void *CreateSqueakEnemy(struct ObjectTemplate *, u8 playerId);
void SqueakStart(struct Object *);
void *CreateSqueakMidboss(struct ObjectTemplate *, u8 playerId);
void SqueakMidbossUpdate(struct SqueakObject *);
u32 SqueakAnimFor(struct Object *);
const u8 *SqueakName(struct Object *);
s16 SqueakMaxHp(struct Object *);
s16 SqueakHpBase(struct Object *);
void SqueakSetCell(struct SqueakObject *, u8 cell);
// SqueakVramOk(); A sprite's VRAM was got (KatAM's VramMalloc gives ewram_end when it's full: the sprite would draw what's there);
#define SqueakVramOk(vram) ((u32)(vram) >= (u32)OBJ_VRAM0 && (u32)(vram) < (u32)OBJ_VRAM0 + OBJ_VRAM0_SIZE)
void SqueakUsePalette(struct SqueakObject *, u8 alternate);
u8 SqueakLoadPalette(u16 key, u8 variant, u16 roomId);
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
void SqueakHoldEffect(struct SqueakObject *, u8 effect, s32 dx, s32 dy);
void SqueakEndHeldEffect(struct SqueakObject *);
void SqueakSplash(struct SqueakObject *, bool8 out);
bool32 SqueakTouches(struct SqueakObject *, u8 side, s32 dy, bool8 both);
struct SqueakObject *SqueakSpawn(struct SqueakObject *, u8 enemy, u8 variant, s32 dx, s32 dy);
bool32 SqueakInLevel(struct SqueakObject *);
void SqueakSetBoxes(struct SqueakObject *, const struct SqueakBoxes *, s16 harm, u8 *index);
void SqueakUntouchable(struct SqueakObject *, bool32 untouchable);
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
void BunInit(struct SqueakObject *);
void BunStart(struct Object *);
void MetalunInit(struct SqueakObject *);
void MetalunStart(struct Object *);
void BigMetalunInit(struct SqueakObject *);
void BigMetalunStart(struct Object *);
void GaoGaoInit(struct SqueakObject *);
void GaoGaoStart(struct Object *);
void SqueakStarInit(struct SqueakObject *);
void SqueakStarStart(struct Object *);
void SqueakStarCarry(struct SqueakObject *, u8 kirbys);
void TedhaunInit(struct SqueakObject *);
void TedhaunStart(struct Object *);
void TedhaunGhostInit(struct SqueakObject *);
void TedhaunGhostStart(struct Object *);
void BubooInit(struct SqueakObject *);
void BubooStart(struct Object *);
void BubooBubbleInit(struct SqueakObject *);
void BubooBubbleStart(struct Object *);
// The Squeaks' chest thefts (squeak/squeakers.c);
struct Chest;
struct Chest *SqueakNearestChest(struct SqueakObject *, bool8 *any);
struct Chest *SqueakHeldChest(struct Chest **held);
void SqueakChestFeet(struct Chest *, s32 *x, s32 *y);
struct Chest *SqueakTouchedChest(struct SqueakObject *);
void SqueakCarryChest(struct SqueakObject *, struct Chest **held, s32 *sway, s32 dx, s32 dy);
void SqueakTakeChest(struct Chest **held, s32 *sway, struct Chest *);
void SqueakDropChest(struct SqueakObject *, struct Chest **held);
void SqueakerInit(struct SqueakObject *);
void SqueakerStart(struct Object *);
void SqueakerGone(struct SqueakObject *);
void SqueakerBombInit(struct SqueakObject *);
void SqueakerBombStart(struct Object *);
void SpinniInit(struct SqueakObject *);
void SpinniStart(struct Object *);
void SpinniGone(struct SqueakObject *);
void SpinniThrownInit(struct SqueakObject *);
void SpinniThrownStart(struct Object *);
void DocInit(struct SqueakObject *);
void DocStart(struct Object *);
void DocGone(struct SqueakObject *);
void DocThingInit(struct SqueakObject *);
void DocThingStart(struct Object *);
void StoroInit(struct SqueakObject *);
void StoroStart(struct Object *);
void StoroGone(struct SqueakObject *);
void DaroachInit(struct SqueakObject *);
void DaroachStart(struct Object *);
void DaroachGone(struct SqueakObject *);
void SqueakDededeInit(struct SqueakObject *);
void SqueakDededeStart(struct Object *);
void SqueakDededeGone(struct SqueakObject *);
void DededePuffInit(struct SqueakObject *);
void DededePuffStart(struct Object *);
void MrsMoleyInit(struct SqueakObject *);
void MrsMoleyStart(struct Object *);
void MoleyThingInit(struct SqueakObject *);
void MoleyThingStart(struct Object *);
void MoleyDirtInit(struct SqueakObject *);
void MoleyDirtStart(struct Object *);
void MechaKrackoInit(struct SqueakObject *);
void MechaKrackoStart(struct Object *);
void MechaKrackoEyeInit(struct SqueakObject *);
void MechaKrackoEyeStart(struct Object *);
void YadgaineInit(struct SqueakObject *);
void YadgaineStart(struct Object *);
void YadgaineLegInit(struct SqueakObject *);
void YadgaineLegStart(struct Object *);
void YadgaineThingInit(struct SqueakObject *);
void YadgaineThingStart(struct Object *);
void YadgaineGordoInit(struct SqueakObject *);
void YadgaineGordoStart(struct Object *);
void YadgaineDaroachInit(struct SqueakObject *);
void YadgaineDaroachStart(struct Object *);
void BohbohInit(struct SqueakObject *);
void BohbohStart(struct Object *);
void BohbohThingInit(struct SqueakObject *);
void BohbohThingStart(struct Object *);
void DaroachBossInit(struct SqueakObject *);
void DaroachBossStart(struct Object *);
void DaroachBossThingInit(struct SqueakObject *);
void DaroachBossThingStart(struct Object *);
void SqueakMetaKnightInit(struct SqueakObject *);
void SqueakMetaKnightStart(struct Object *);
void SqueakMetaKnightThingInit(struct SqueakObject *);
void SqueakMetaKnightThingStart(struct Object *);
void SqueakDarkNebulaInit(struct SqueakObject *);
void SqueakDarkNebulaStart(struct Object *);
void SqueakDarkNebulaThingInit(struct SqueakObject *);
void SqueakDarkNebulaThingStart(struct Object *);
void DarkNebulaGone(struct SqueakObject *);
void YadgaineGone(struct SqueakObject *);

// squeak/dark_star.c;
void SqueakDarkStarInit(struct SqueakObject *);
void SqueakDarkStarStart(struct Object *);

#endif
