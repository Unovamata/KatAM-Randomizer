/* NiDL abilities; Copy abilities ported from Kirby: Nightmare in Dream Land, as KatAM abilities 27-33 (KatAM has 0-26), and its Star Rod
 * (34), which the finale's fights give, the Power Orb's and Nightmare's (abilities/star_rod.c), and its stand in the NiDL hub
 * (abilities/star_rod_stand.c):
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
    NIDL_ABILITY_STAR_ROD,
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
 * NIDL_MOTION_SLOT on (KatAM's copy abilities' attacks, 103-118, which only their own states play; not 74-89, KatAM's tumble, which
 * a warp star's hop-off and cutscenes play for every ability), which NidlAbilitySetup points to them. Numbered per ability, as in
 * Motions there; */
#define NIDL_MOTION_SLOT 0x67
#define NIDL_MOTION_SLOTS 16

enum { NEEDLE_MOTION_OUT, NEEDLE_MOTION_IN, NEEDLE_MOTION_AIR };
enum { FREEZE_MOTION_EDGE, FREEZE_MOTION_BREATH };
enum { LIGHT_MOTION_SHINE };
enum { BALL_MOTION_HOP, BALL_MOTION_FALL, BALL_MOTION_SQUASH, BALL_MOTION_REST, BALL_MOTION_LAUNCH, BALL_MOTION_ROLL };
enum { HI_JUMP_MOTION_CROUCH, HI_JUMP_MOTION_RISE };    // (the rise: NiDL's five sets by his speed sideways, RISE + 0-4);
enum { MIKE_MOTION_1, MIKE_MOTION_2, MIKE_MOTION_3, MIKE_MOTION_1_END, MIKE_MOTION_2_END, MIKE_MOTION_3_END };
enum { STAR_ROD_MOTION_HOLD, STAR_ROD_MOTION_SHOOT, STAR_ROD_MOTION_RIGHT, STAR_ROD_MOTION_LEFT, STAR_ROD_MOTION_UP, STAR_ROD_MOTION_DOWN,
       STAR_ROD_MOTION_RIGHT_END, STAR_ROD_MOTION_LEFT_END, STAR_ROD_MOTION_UP_END, STAR_ROD_MOTION_DOWN_END, STAR_ROD_MOTION_HURT,
       STAR_ROD_MOTION_INTRO, STAR_ROD_MOTION_SWING, STAR_ROD_MOTION_TUMBLE, STAR_ROD_MOTION_FALLEN, STAR_ROD_MOTION_SPIN };
// KatAM's jump's animation indexes (its rise, its flip: held till he lands when its anim loops), the Star Rod's spin's too;
#define KIRBY_JUMP_RISE 15
#define KIRBY_JUMP_FLIP 16

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
    void (*displaySpriteOriginal)(struct Sprite *);
    struct {
        u16 anim, first, count, plain;                  // The Star Rod's anims (its hat's, its motions', its stand's): its frames first on
    } glows[3];                                         // (the motions' anim is every ability's: the rod's are the last), then their
                                                        // glowing versions, count frames on (see NidlDisplaySprite); from `plain` on
                                                        // (0: none), its own colors, no glow (the motions': the jump's spin);
    const u16 *rodPalettes;                             // Its hat's palette bright, halfway, and its own (16 colors each);
    const u16 *hiJumpFlash;                             // The Hi-Jump's glow: the body's colors (1-11 of 16), then its hat's (16);
    u16 hiJumpFx;                                       // Its effects' anim (KatAMExpansion.cs, HiJumpEffects);
};

extern const struct NidlAbilityData gNidlAbilityData;

// The HUD portraits' size (sub_080356AC's: abilities and the special portraits up to 0x1B; the rest are smaller);
#define PORTRAIT_SIZE 0x3C0

void NidlAbilityPortrait(u32 vram, u8 palette, u8 id);

void NidlAbilityHudTimer(struct Kirby *, u8 portrait, u16 frames);
void NidlDisplaySprite(struct Sprite *);
void NidlAbilitySetup(struct Kirby *);
void NidlSwallow(struct Kirby *, u8 ability);
void NidlKirbyHit(struct Kirby *);

/* NidlAttack; An attack box that follows Kirby while `alive` says so (his state is the ability's), making him unhurt (`guard`) meanwhile;
 * or, with `frames` set, stays where it was made for that many frames (an effect's: Hi-Jump's burst). `effect` is the hit's kind (unk68,
 * like gUnk_082DDFC8's: 0x200 freezes, 0x2000 cuts...), `damage` its strength (unk63); */
struct NidlAttack {
    struct ObjectBase base;
    bool32 (*alive)(struct Kirby *);
    bool8 guard;
    u8 rehit;           // Frames until it can hit again (2 by default, as Burning's);
    u8 frames;          // Frames left where it was made (0: following Kirby, as `alive` says);
    u32 kirbyUnk68;     // Kirby's base.unk68 before the guard, put back after;
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
/* The ability Kirby had before the Power Orb's fight gave him the Star Rod (his padding byte after NIDL_MIKE_USES), | STAR_ROD_FROM_FIGHT,
 * given back after the shard's dance; without the flag (0: none), the Star Rod is his own (the hub's stand's, abilities/star_rod_stand.c,
 * or a saved game's), kept; */
#define STAR_ROD_SAVED(kirby) (((u8 *)(kirby))[0x1A7])
#define STAR_ROD_FROM_FIGHT 0x80

// Star Rod (abilities/star_rod.c): the Power Orb's fight (bosses/power_orb.c) flies every Kirby in its room with it; Nightmare's
// (bosses/nightmare.c) is fought with it on the ground (StarRodSwingStart: B; StarRodKeep: never lost there);
void StarRodHeld(struct Kirby *);
void NidlKirbyStack(void);
void StarRodSwingStart(struct Kirby *);
void StarRodKeep(struct Kirby *);
void StarRodAim(struct Kirby *);
void StarRodForget(void *fight);
bool32 StarRodCanFly(struct Kirby *);
void StarRodStart(struct Kirby *, struct Object *orb);
void StarRodRestore(struct Kirby *);
void StarRodEnd(struct Kirby *);
void StarRodFightOver(struct Object *orb);
s32 StarRodSink(u8 player, s32 *lastTop);
void StarRodHit(struct Kirby *, s32 x);
bool32 StarRodTouches(struct Kirby *, s32 x, s32 y, s32 halfWidth, s32 halfHeight);
u8 StarRodHits(u16 room, s32 x, s32 y, s32 reach);
// (and the NiDL hub's stand, object 0x9A's spawn: abilities/star_rod_stand.c);
void *NidlCreateRodStand(struct ObjectTemplate *, u8 playerId);

// Abilities (abilities/*.c): B on land;
void NeedleStart(struct Kirby *);
void FreezeStart(struct Kirby *);
void LightStart(struct Kirby *);
void BallStart(struct Kirby *);
void HiJumpStart(struct Kirby *);
void MikeStart(struct Kirby *);

#endif
