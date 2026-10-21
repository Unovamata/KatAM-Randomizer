/* NiDL enemies; Enemies ported from Kirby: Nightmare in Dream Land (USA) all share object type 0x31 (the only free regular
 * enemy type: the game treats types 0x00-0x37 as enemies all over), and the template's behavior byte picks which one:
 * high nibble = enemy (NIDL_*), low nibble = its variant. What the game reads per type is shared, so:
 * • Animations: one AnimInfo table for the type; every enemy has its own range in it (animBase + frame), and its own anim ID,
 *   which is also its palette key, so each enemy gets its own palette slot.
 * • Hurt frames: the game picks the type's hurt indexes (0 and 1), which show a generic 32x32 frame drawn from the last
 *   16 tiles of the object's VRAM; each enemy uploads its own hurt frame there (see NidlUploadHurtTiles).
 * • HP bar names: the game's name copies are patched to call NidlCopyName, which looks up the enemy's own name.
 * • Water: enemies that swim in NiDL swim here too (NidlIsWater, NidlSetHeading); the others sink like KatAM's.
 * • Gravity: the engine's (0x0809A990) is turned off, like KatAM's flying enemies do; each enemy moves up and down like in
 *   NiDL, and the ones that just stand or walk fall with NidlFall.
 * The tables filled by KatAMExpansion.cs (animations, hurt tiles, names) are in gNidlData; */
#ifndef GUARD_NIDL_H
#define GUARD_NIDL_H

#include "object.h"
#include "kirby.h"
#include "random.h"
#include "functions.h"
#include "task.h"

#define NIDL_TYPE OBJ_EMPTY_31
/* Mini-bosses ported from NiDL share object types 0x40-0x42 the same way (see nidl_miniboss.c): the game treats them as mini-bosses.
 * An object's frame index is a byte, so they're spread over the three, each with its own AnimInfo table (KatAMExpansion.cs decides
 * which mini-boss goes where); */
#define NIDL_MINIBOSS_TYPE OBJ_EMPTY_40
#define NIDL_MINIBOSS_TYPE_2 OBJ_EMPTY_41
#define NIDL_MINIBOSS_TYPE_3 OBJ_EMPTY_42
#define NidlIsMiniboss(obj) ((obj)->type == NIDL_MINIBOSS_TYPE || (obj)->type == NIDL_MINIBOSS_TYPE_2 || (obj)->type == NIDL_MINIBOSS_TYPE_3)
/* Bosses ported from NiDL share object type 0x43 (OBJ_EMPTY_43, which KatAM treats as a boss everywhere: the boss music, its HP bar,
 * its defeat, explosion and gUnk_08350E58's handler, here NidlBossDefeat: the mirror shard) the same way (see nidl_boss.c), with the
 * mini-bosses' graphics and hurt frame (64x96); */
#define NIDL_BOSS_TYPE OBJ_EMPTY_43
// (and type 0x44, OBJ_EMPTY_44, as much a boss to KatAM: the bosses that don't fit in type 0x43's 256 frames, KatAMExpansion.cs's BossTypeOf);
#define NIDL_BOSS_TYPE_2 OBJ_EMPTY_44
#define NidlIsBoss(obj) ((obj)->type == NIDL_BOSS_TYPE || (obj)->type == NIDL_BOSS_TYPE_2)
/* NidlOnFloor(); A mini-boss's spot from its template (NiDL's: the middle of the cell it stands in) on the floor under that cell:
 * its box's bottom (unk3F) on the cell's bottom. The enemies' snap (to the cell edge under the box's bottom) put the big ones
 * (Bugzzy, Mr. Tick-Tock, Fire Lion: bottoms past 23 pixels) a cell lower: in the ground, or through a floor a cell thick; */
#define NidlOnFloor(obj) ((obj)->base.unk4C = (obj)->base.y = (((obj)->base.y & ~0xFFF) + 0x1000) - ((obj)->base.unk3F * 0x100) - 1)

// Enemies (the behavior byte's high nibble);
enum {
    NIDL_BROOM_HATTER,
    NIDL_CAPPY,
    NIDL_TWIZZY,
    NIDL_BUBBLES,
    NIDL_SLIPPY,
    NIDL_KABU,
    NIDL_COCONUT,
    NIDL_CONER,
    NIDL_STARMAN,
    NIDL_NEEDLOUS,
    NIDL_BLADE_KNIGHT,
    NIDL_CHILLY,
    NIDL_COOL_SPOOK,
    NIDL_POPPY_BROS_JR,
    NIDL_GIP,
    NIDL_APPLE,     // Whispy Woods' apples (enemies/apple.c), behavior NIDL_SPECIAL << 4 | 1: no room has them;
    NIDL_STAR,      // Mr. Shine's falling stars (enemies/star.c), behavior NIDL_SPECIAL << 4 | 2: no room has them;
    NIDL_SPIT_STAR, // The star King Dedede spits the Kirbys he swallowed out in (enemies/spit_star.c), behavior NIDL_SPECIAL << 4 | 3;
    NIDL_ENEMY_COUNT,
};
/* The behavior's high nibble 15 (NIDL_SPECIAL) holds what isn't a room's enemy, by its low nibble: 0 a mini-boss fight's walls in
 * NiDL rooms (NIDL_ARENA, enemies/arena.c, with Broom Hatter's graphics, never shown), 1 an apple (NIDL_APPLE), 2 a falling star
 * (NIDL_STAR), 3 King Dedede's spit star (NIDL_SPIT_STAR); */
#define NIDL_SPECIAL 15
#define NIDL_ARENA NIDL_ENEMY_COUNT

// Mini-bosses (type 0x40's behavior byte's high nibble);
enum {
    NIDL_POPPY_BROS_SR,
    NIDL_MR_TICK_TOCK,
    NIDL_GRAND_WHEELIE,
    NIDL_BUGZZY,
    NIDL_FIRE_LION,
    NIDL_META_KNIGHTS,
    NIDL_MINIBOSS_COUNT,
};

// Bosses (type 0x43's behavior byte's high nibble; its low nibble is the mirror shard it leaves, 0-7);
enum {
    NIDL_WHISPY_WOODS,
    NIDL_KRACKO,
    NIDL_SHINE_BRIGHT,
    NIDL_META_KNIGHT,
    NIDL_PAINT_ROLLER,
    NIDL_HEAVY_MOLE,
    NIDL_KING_DEDEDE,
    NIDL_POWER_ORB,
    NIDL_NIGHTMARE,
    NIDL_BOSS_COUNT,
};

// Room every ported enemy gets (NidlObject and its own fields);
#define NIDL_OBJECT_SIZE 0x100

#define NIDL_HURT_FRAME 0
#define NIDL_HURT_TILES 16
// A mini-boss's hurt frame is 64x96 (centered), type 0x42's (the Meta-Knights, several at once) 32x64 (KatAMExpansion.cs's HurtOf);
#define NIDL_MINIBOSS_HURT_TILES 96
#define NIDL_KNIGHT_HURT_TILES 32

// NidlObject; Every ported enemy starts with it;
struct NidlObject {
    struct Object obj;
    u8 enemy;
    u8 variant;
    u16 animBase;
    u32 hurtVram;   // VRAM the hurt frame was uploaded to;
    const s16 *sequence;
    u8 sequenceStep;
    u8 sequenceTimer;
    bool8 activated;    // Mini-bosses: Kirby came in range (the fight started);
    u8 hurt;            // Which of its hurt frames (mini-bosses with several, the Meta-Knights; 0 for the others);
};

/* Frame sequences, like NiDL's (0x080640DC plays one, 0x080640FC steps it): pairs of (frame, frames to show it), then
 * NIDL_LOOP (back to the start) or NIDL_END (holds the last frame); */
#define NIDL_LOOP -3
#define NIDL_END -2

// NidlEnemy; What the code knows about each enemy;
struct NidlEnemy {
    void (*init)(struct NidlObject *);  // Bounds and position, after InitObject;
    void (*start)(struct Object *);     // Behavior start, also after recovering from a hit;
    s16 hp;
    u16 ability;
};

// NidlMiniboss; What the code knows about each mini-boss;
struct NidlMiniboss {
    void (*init)(struct NidlObject *);  // Bounds and position, after InitObject;
    void (*start)(struct Object *);     // Behavior start once the fight starts, also after recovering from a hit;
    s16 hp;
    u16 ability;
    s8 hurtShift;   // Pixels its frames and hurt frame are drawn lower (to fit the hurt frame; the same as KatAMExpansion.cs's); its boxes are as much lower;
    u8 type;        // Its object type (NIDL_MINIBOSS_TYPE or _2; the same as KatAMExpansion.cs's);
};

// NidlBoss; What the code knows about each boss;
struct NidlBoss {
    void (*init)(struct NidlObject *);  // Bounds and position, after InitObject;
    void (*start)(struct Object *);     // Behavior start once the fight starts;
    s16 hp;
};

// NidlGraphics; Where an enemy's or mini-boss's graphics went;
struct NidlGraphics {
    u16 animBase;                       // First AnimInfo of the enemy in the type's table;
    u16 animId;                         // Its anim ID (and palette key);
    const u8 *hurtTiles;                // Its hurt frame (32x32, 16 tiles; mini-bosses 64x96, 96 tiles; NidlHurtTiles), or frames;
};

// NidlData; Filled in by KatAMExpansion.cs once the graphics are built;
struct NidlData {
    const u8 *names;                    // One 64x8 HP bar name (0x100 bytes) per enemy;
    struct NidlGraphics enemies[NIDL_ENEMY_COUNT];
    const u8 *minibossNames;            // The same per mini-boss;
    struct NidlGraphics minibosses[NIDL_MINIBOSS_COUNT];
    const u8 *bossNames;                // The same per boss;
    struct NidlGraphics bosses[NIDL_BOSS_COUNT];
};

extern const struct NidlData gNidlData;

void *CreateNidlEnemy(struct ObjectTemplate *, u8 playerId);
void *CreateNidlMiniboss(struct ObjectTemplate *, u8 playerId);
void *CreateNidlBoss(struct ObjectTemplate *, u8 playerId);
void NidlBossStart(struct Object *);
void NidlBossDefeat(struct Object *);
// A boss rematch's Maxim Tomato's gone bit kind (template unk2; rooms.c's BigSub_08001678 calls NidlExitTomato with it, nidl_boss.c);
#define NIDL_EXIT_KIND 4
void NidlExitTomato(u16 slot, u32 eaten);
void NidlDanceOut(u16 room);
// The finale (finale.c): the Power Orb's and Nightmare's ends, the Power Orb's warp star;
void PowerOrbBeaten(struct Object *orb);
void NidlFinaleStarAt(u8 player, s32 x, s32 y);
void PowerOrbEnd(struct Object *orb);
void NightmareBeaten(struct Object *nightmare);
void NightmareEnd(struct Object *nightmare);
void NidlEndingFlight(u16 room);
bool32 NidlOnEnding(struct Kirby *kirby);
bool32 NidlFinaleFlight(struct Kirby *kirby);
// The ending down from the moon (ending.c);
void NidlEndingRider(struct Kirby *kirby);
u32 NidlEndingRideSize(u16 room);
bool32 NidlEndingOn(u16 room);
void NidlFinaleKeep(struct Object *nightmare);
bool32 NidlFinaleStarSpot(u16 room, s32 *x, s32 *y);
bool32 NidlFinaleStar(struct Object *star);
void NidlFinaleHoldCamera(u16 room, s32 x, s32 y);
u32 PowerOrbRideSize(u16 room);
void NidlCloudsStart(u16 room);
u32 NightmareRideSize(u16 room);
bool32 NightmareIntroOn(u16 room);
bool32 NightmareIntroHolds(u16 room);
bool32 NightmareBlocks(u16 room, s32 x, s32 y);
bool32 NightmareTarget(u16 room, s32 x, s32 y, bool8 *left, s32 *gap);
void NightmareGone(struct Object *nightmare);
void NightmareIntroRider(struct Kirby *kirby);
const struct NidlGraphics *NidlGraphicsOf(struct NidlObject *);
u32 NidlHurtTiles(struct Object *);
void NidlUploadHurtTiles(struct NidlObject *);
void NidlInitPalette(struct NidlObject *);
void NidlSetFrame(struct NidlObject *, u8 frame);
u8 NidlLoadPalette(struct NidlObject *, u16 key, u8 frame);
#define NIDL_SECOND_PALETTE 0x2000
#define NIDL_THIRD_PALETTE 0x4000
void NidlPlay(struct NidlObject *, const s16 *sequence);
void NidlAnimate(struct NidlObject *);
void NidlUpdate(struct NidlObject *);
void NidlFall(struct Object *);
void NidlFaceKirby(struct Object *);
bool32 NidlIsFloor(struct Object *, s32 x, s32 y);
void NidlStopAtLedge(struct Object *);
bool32 NidlIsWater(struct Object *, s32 dy);
void NidlKeepInLevel(struct Object *, s32 bottom);
bool32 NidlAtLevelBottom(struct Object *, s32 bottom);
void NidlSetHeading(struct Object *, u16 angle, s16 speed);
u8 NidlKirbyOctant(struct Object *);

/* Water: KatAM sinks (kills) any enemy in water unless base.unkC & 1, so the ones that swim set it (NIDL_SWIMS). Angles are
 * NiDL's: 512 a circle, 0 right, 128 down; */
#define NIDL_SWIMS(obj) ((obj)->base.unkC |= 1)

// Enemies;
void BroomHatterInit(struct NidlObject *);
void BroomHatterStart(struct Object *);
void CappyInit(struct NidlObject *);
void CappyStart(struct Object *);
void TwizzyInit(struct NidlObject *);
void TwizzyStart(struct Object *);
void BubblesInit(struct NidlObject *);
void BubblesStart(struct Object *);
void SlippyInit(struct NidlObject *);
void SlippyStart(struct Object *);
void KabuInit(struct NidlObject *);
void KabuStart(struct Object *);
void CoconutInit(struct NidlObject *);
void CoconutStart(struct Object *);
void ConerInit(struct NidlObject *);
void ConerStart(struct Object *);
void StarmanInit(struct NidlObject *);
void StarmanStart(struct Object *);
void NeedlousInit(struct NidlObject *);
void NeedlousStart(struct Object *);
void ArenaInit(struct NidlObject *);
void ArenaStart(struct Object *);
void BladeKnightInit(struct NidlObject *);
void BladeKnightStart(struct Object *);
void ChillyInit(struct NidlObject *);
void ChillyStart(struct Object *);
void CoolSpookInit(struct NidlObject *);
void CoolSpookStart(struct Object *);
void PoppyBrosJrInit(struct NidlObject *);
void PoppyBrosJrStart(struct Object *);
void GipInit(struct NidlObject *);
void GipStart(struct Object *);
void AppleInit(struct NidlObject *);
void AppleStart(struct Object *);
void StarInit(struct NidlObject *);
void StarStart(struct Object *);
void SpitStarInit(struct NidlObject *);
void SpitStarStart(struct Object *);
void SpitStarCarry(struct Object *star, u8 kirbys, bool8 left);

// Mini-bosses;
void PoppyBrosSrInit(struct NidlObject *);
void PoppyBrosSrStart(struct Object *);
void MrTickTockInit(struct NidlObject *);
void MrTickTockStart(struct Object *);
void GrandWheelieInit(struct NidlObject *);
void GrandWheelieStart(struct Object *);
void BugzzyInit(struct NidlObject *);
void BugzzyStart(struct Object *);
void FireLionInit(struct NidlObject *);
void FireLionStart(struct Object *);
void MetaKnightsInit(struct NidlObject *);
void MetaKnightsStart(struct Object *);
void MetaKnightsDestroy(struct Task *);

// Bosses;
void WhispyWoodsInit(struct NidlObject *);
void WhispyWoodsStart(struct Object *);
void KrackoInit(struct NidlObject *);
void KrackoStart(struct Object *);
void ShineBrightInit(struct NidlObject *);
void ShineBrightStart(struct Object *);
void MetaKnightInit(struct NidlObject *);
void MetaKnightStart(struct Object *);
void PaintRollerInit(struct NidlObject *);
void PaintRollerStart(struct Object *);
void HeavyMoleInit(struct NidlObject *);
void HeavyMoleStart(struct Object *);
void KingDededeInit(struct NidlObject *);
void KingDededeStart(struct Object *);
void PowerOrbInit(struct NidlObject *);
void PowerOrbStart(struct Object *);
void NightmareInit(struct NidlObject *);
void NightmareStart(struct Object *);

void NidlDestroy(struct Task *);
bool32 NidlIsMetaKnight(struct ObjectTemplate *);

#endif
