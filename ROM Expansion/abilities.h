/* NiDL abilities; Copy abilities ported from Kirby: Nightmare in Dream Land, as KatAM abilities 27-33 (KatAM has 0-26):
 * • Ability field: KatAM keeps the ability Kirby is getting (transitioningAbility) in 5 bits, with 3 flags above. It's widened to 6
 *   bits (NIDL_ABILITY_MASK): the "random" flag moves from 0x20 to 0x40, and the 0x40 flag ("several things in the mouth", which makes
 *   the spat star bigger) to the unused unkF1. KatAMExpansion.cs patches every "& 0x1F" read of it to "& 0x3F", the random flag's
 *   tests and sets and the spat star's read; swallowing (sub_080547C4) is replaced by NidlSwallow;
 * • Ability tables: KatAMExpansion.cs copies every table KatAM indexes by ability with 27 entries (B button on land and in water,
 *   setup, hat palette, hats, the CPU helpers' AI) to the expansion with 32, and points the game's code to the copies. The new
 *   entries come from gNidlAbilityTables, or from the base ability's entries where it has none.
 * • HUD: the portrait table (gUnk_08D60B64/gUnk_08D60BE4, sub_080356AC) already has 32 entries, but 27-31 are special
 *   portraits (0x1C-0x1F: hurt and such, set by sub_08088640). Both functions are detoured (KatAMExpansion.cs writes a jump to
 *   the C function at their start and a trampoline that runs the original): sub_08088640 moves the special portraits to
 *   0x2C-0x2F, and NidlAbilityPortrait draws the new abilities' portraits and moves those back for the original;
 * • Graphics (hats, portraits) are built from the NiDL ROM by KatAMExpansion.cs, see gNidlData.abilities; */
#ifndef GUARD_ABILITIES_H
#define GUARD_ABILITIES_H

#include "nidl.h"
#include "constants/kirby.h"

enum {
    NIDL_ABILITY_NEEDLE = KIRBY_ABILITY_COUNT,
    NIDL_ABILITY_FREEZE,
    NIDL_ABILITY_LIGHT,
    NIDL_ABILITY_BALL,
    NIDL_ABILITY_HI_JUMP,
    NIDL_ABILITY_MIKE,
    NIDL_ABILITY_BACKDROP,
    NIDL_ABILITY_END,
};

// transitioningAbility's layout (see above);
#define NIDL_ABILITY_MASK 0x3F
#define NIDL_ABILITY_CHANGE_RANDOM 0x40
#define NIDL_ABILITY_CHANGE_IS_ABILITY_STAR 0x80
#define NIDL_ABILITY_SEVERAL 0x40         // In unkF1: several things swallowed at once (KatAM's KIRBY_ABILITY_CHANGE_UNKNOWN);

// What sub_080547C4's callers pass instead of an ability: a random one (object type 0xCE), none (special objects, sub_0809C48C);
#define NIDL_SWALLOW_RANDOM 0x3B
#define NIDL_SWALLOW_NONE 0x3C

#define NIDL_FIRST_ABILITY NIDL_ABILITY_NEEDLE
#define NIDL_ABILITY_COUNT (NIDL_ABILITY_END - NIDL_FIRST_ABILITY)

// Special HUD portraits (sub_08088640's, 0x1C-0x1F) are moved up by this;
#define NIDL_SPECIAL_PORTRAIT_SHIFT 0x10

// NidlAbilityTables; The new abilities' entries in KatAM's ability tables (NULL: the base ability's), read by KatAMExpansion.cs;
struct NidlAbilityTables {
    u8 base[NIDL_ABILITY_COUNT];                        // Ability whose entries the new one copies;
    void (*landB[NIDL_ABILITY_COUNT])(struct Kirby *);  // gUnk_0834C2AC: B on land (or in the air);
    void (*waterB[NIDL_ABILITY_COUNT])(struct Kirby *); // gUnk_0834C318: B in water;
    void (*setup[NIDL_ABILITY_COUNT])(struct Kirby *);  // gUnk_083509D0: on getting it (body animations);
};

extern const struct NidlAbilityTables gNidlAbilityTables;

/* Motions; The new abilities' attack poses, NiDL's frames (KatAMExpansionMotions.cs builds them): played at animation indexes
 * NIDL_MOTION_SLOT on (copy abilities' own), which NidlAbilitySetup points to them. Numbered per ability, as in Motions there; */
#define NIDL_MOTION_SLOT 0x4A
#define NIDL_MOTION_SLOTS 16

enum { NEEDLE_MOTION_OUT, NEEDLE_MOTION_IN, NEEDLE_MOTION_AIR };
enum { FREEZE_MOTION_EDGE, FREEZE_MOTION_BREATH };
enum { LIGHT_MOTION_SHINE };
enum { BALL_MOTION_HOP, BALL_MOTION_FALL, BALL_MOTION_SQUASH, BALL_MOTION_REST, BALL_MOTION_LAUNCH, BALL_MOTION_ROLL };
enum { HI_JUMP_MOTION_RISE };
enum { MIKE_MOTION_1, MIKE_MOTION_2, MIKE_MOTION_3, MIKE_MOTION_1_END, MIKE_MOTION_2_END, MIKE_MOTION_3_END };

// NidlMotion(); Plays one of the ability's motions;
#define NidlMotion(kirby, motion) ((kirby)->animationIndex = NIDL_MOTION_SLOT + (motion))

// NidlAbilityData; Filled in by KatAMExpansion.cs (in nidl_data.c, like gNidlData);
struct NidlAbilityData {
    void (*portraitOriginal)(u32, u8, u8);              // Trampolines running the detoured functions;
    void (*hudTimerOriginal)(struct Kirby *, u8, u16);
    struct {
        const u16 *palette;                             // 16 colors;
        const u8 *tiles;                                // PORTRAIT_SIZE bytes, laid out like KatAM's portraits;
    } portraits[NIDL_ABILITY_COUNT];
    struct AnimInfo motions[NIDL_ABILITY_COUNT][NIDL_MOTION_SLOTS]; // Body animations of the motions (0: none), see NidlAbilitySetup;
};

extern const struct NidlAbilityData gNidlAbilityData;

// The HUD portraits' size (sub_080356AC's: abilities and the special portraits up to 0x1B; the rest are smaller);
#define PORTRAIT_SIZE 0x3C0

void NidlAbilityPortrait(u32 vram, u8 palette, u8 id);

void NidlAbilityHudTimer(struct Kirby *, u8 portrait, u16 frames);
void NidlAbilitySetup(struct Kirby *);
void NidlSwallow(struct Kirby *, u8 ability);

/* NidlAttack; An attack box that follows Kirby while `alive` says so (his state is the ability's), making him unhurt (`guard`) meanwhile.
 * `effect` is the hit's kind (unk68, like gUnk_082DDFC8's: 0x200 freezes, 0x2000 cuts...), `damage` its strength (unk63); */
struct NidlAttack {
    struct ObjectBase base;
    bool32 (*alive)(struct Kirby *);
    bool8 guard;
};

struct NidlAttack *NidlAttackCreate(struct Kirby *, bool32 (*alive)(struct Kirby *), u32 effect, u8 damage, bool8 guard,
                                    s8 left, s8 top, s8 right, s8 bottom);

// Helpers for the abilities' states: water, falling, landing, being used up, a white screen flash;
bool32 NidlAbilityInWater(struct Kirby *);
void NidlAbilityGravity(struct Kirby *);
void NidlAbilityEnd(struct Kirby *);
void NidlAbilityUsedUp(struct Kirby *);
void NidlFlash(struct Kirby *, const u8 *levels, u16 count);

// Mike's uses so far (Kirby's padding byte after unk1A5; reset by NidlAbilitySetup on getting Mike);
#define NIDL_MIKE_USES(kirby) (((u8 *)(kirby))[0x1A6])

// Abilities (abilities/*.c): B on land;
void NeedleStart(struct Kirby *);
void FreezeStart(struct Kirby *);
void LightStart(struct Kirby *);
void BallStart(struct Kirby *);
void HiJumpStart(struct Kirby *);
void MikeStart(struct Kirby *);

#endif
