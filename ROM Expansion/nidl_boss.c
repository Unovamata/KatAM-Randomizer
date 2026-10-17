/* NiDL bosses; Object types 0x43 and 0x44 (OBJ_EMPTY_43 and 44, which KatAM already treats as a boss everywhere: the boss music, its HP bar, a hit
 * only flashes it, its defeat: it freezes, explodes and goes, then gUnk_08350E58[type - 0x43] runs, here NidlBossDefeat): creates the
 * boss the template's behavior byte asks for (high nibble = boss, NIDL_* in nidl.h; low nibble = the mirror shard it leaves, 0-7),
 * and starts the fight as the mini-bosses do (nidl_miniboss.c): it waits, untouchable, until Kirby is in the template's area (the
 * boss's screen, held by an arena, enemies/arena.c, until it's beaten), then the HP bar shows and the boss music plays. Graphics are
 * the mini-bosses' (one AnimInfo table for the type, its own palettes and anim ID, a 64x96 hurt frame); */
#include "nidl.h"
#include "constants/kirby.h"
#include "treasures.h"
#include "bonus.h"

static const struct NidlBoss sNidlBosses[NIDL_BOSS_COUNT] = {
    [NIDL_WHISPY_WOODS] = { WhispyWoodsInit, WhispyWoodsStart, 60 },
    [NIDL_KRACKO] = { KrackoInit, KrackoStart, 60 },
    [NIDL_SHINE_BRIGHT] = { ShineBrightInit, ShineBrightStart, 60 },
    [NIDL_META_KNIGHT] = { MetaKnightInit, MetaKnightStart, 60 },
    [NIDL_PAINT_ROLLER] = { PaintRollerInit, PaintRollerStart, 60 },
    [NIDL_HEAVY_MOLE] = { HeavyMoleInit, HeavyMoleStart, 60 },
    [NIDL_KING_DEDEDE] = { KingDededeInit, KingDededeStart, 60 },
    [NIDL_POWER_ORB] = { PowerOrbInit, PowerOrbStart, 60 },
    [NIDL_NIGHTMARE] = { NightmareInit, NightmareStart, 60 },
};

// Where a boss leaves its shard: the middle of its screen (the camera's, KatAM's 240x160), a little up;
#define SHARD_X 120
#define SHARD_Y 72

void *CreateNidlBoss(struct ObjectTemplate *template, u8 playerId) {
    struct Task *task = TaskCreate(ObjectMain, NIDL_OBJECT_SIZE, 0x1000, TASK_USE_EWRAM, NidlDestroy);
    struct NidlObject *n = TaskGetStructPtr(task);
    struct Object *obj = &n->obj;
    u8 boss = template->subtype1 >> 4;

    if (boss >= NIDL_BOSS_COUNT || !gNidlData.bosses[boss].animId) boss = 0;
    InitObject(obj, template, playerId);
    n->enemy = boss;
    n->variant = template->subtype1 & 0xF;
    n->animBase = gNidlData.bosses[boss].animBase;
    n->hurtVram = 0;
    n->sequence = NULL;
    n->activated = FALSE;
    n->hurt = 0;
    obj->unk80 = sNidlBosses[boss].hp;
    // As the mini-bosses: water doesn't defeat it, untouchable until the fight starts;
    obj->base.unkC |= 1;
    obj->base.flags |= 0x2000000 | 0x200 | 0x200000;
    sNidlBosses[boss].init(n);
    NidlSetFrame(n, 0);
    ObjectInitSprite(obj);
    obj->base.sprite.unk14 = 0x6C0;
    NidlInitPalette(n);
    NidlUploadHurtTiles(n);
    NidlBossStart(obj);
    return n;
}

/* NidlBossWait(); Before the fight: until Kirby is in the template's area, then the HP bar shows and the boss music plays
 * (Macro_08100F18: gUnk_08352D80[0x43 - 0x38], the boss music); */
static void NidlBossWait(struct Object *obj) {
    struct NidlObject *n = (struct NidlObject *)obj;

    NidlUpdate(n);
    obj->base.flags |= 4;
    obj->kirby3 = FindTargetKirby(&obj->base);
    if ((obj->kirby3->base.unkC & 0x8000) || obj->base.roomId != obj->kirby3->base.roomId) return;
    if (!Macro_08039430_2(&obj->kirby3->base, obj)) return;
    Macro_081003EC(obj, &obj->kirby3->base);
    obj->base.flags &= ~0x200;
    Macro_08100F18(obj);
    n->activated = TRUE;
    sNidlBosses[n->enemy].start(obj);
}

// NidlBossStart(); The type's behavior start;
void NidlBossStart(struct Object *obj) {
    struct NidlObject *n = (struct NidlObject *)obj;

    obj->kirbyAbility = 0;
    // The engine's gravity off, as the enemies' (see NidlStart);
    obj->base.flags = (obj->base.flags | 0x40) & ~0x20;
    if (n->activated) {
        sNidlBosses[n->enemy].start(obj);
        return;
    }
    ObjectSetFunc(obj, 0, NidlBossWait);
}

/* NidlBossDefeat(); gUnk_08350E58's entry for type 0x43, run as it explodes (as KatAM's bosses, sub_08087390...): its mirror shard
 * (the behavior's low nibble) in the middle of the screen, which Kirby takes as a boss's (the shard cutscene, then where
 * gNidlShardExits says for the room: the boss stage's way on, its room having no door out; KatAM's: Central Circle); a Maxim
 * Tomato instead when the shard is owned already (KatAM's bosses fought again leave one, sub_08088A38), and the shard again in a
 * room with no door out (one gNidlShardExits has); */
static const struct NidlShardExit *NidlShardExitOf(u16 room);

void NidlBossDefeat(struct Object *obj) {
    struct NidlObject *n = (struct NidlObject *)obj;
    struct LevelInfo *level = &gCurLevelInfo[obj->base.unk56];
    s32 x = (level->viewportPosition.x >> 8) + SHARD_X, y = (level->viewportPosition.y >> 8) + SHARD_Y;
    u8 shard = n->variant & 7;

    if (HasShard(shard)) {
        BonusCreateTomatoAt(obj, x, y);
        // (and the shard again, under it, in a room with no door out: taking it is still the way on);
        if (NidlShardExitOf(obj->base.roomId))
            CreateObjTemplateAndObj(obj->base.unk56, 1, 0x24, x, y + 0x18, 0, 0x1F, 0, 0, OBJ_MIRROR_SHARD, shard, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        return;
    }
    CreateObjTemplateAndObj(obj->base.unk56, 1, 0x24, x, y, 0, 0x1F, 0, 0, OBJ_MIRROR_SHARD, shard, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

/* gNidlShardExits; Where Kirby goes once a boss's shard cutscene ends (KatAM: Central Circle, sub_0804C614's `roomId = 0x321`, which
 * KatAMExpansion.cs replaces with a call to NidlShardExit()): from a ported boss's room (each part), where its boss stage goes on (the
 * next room, the next level's first): a boss room has no door out, the shard is the way on (KatAMExpansionRooms.cs' bossExitsOf);
 * filled in by KatAMExpansion.cs, ended by a room 0; */
#define NIDL_SHARD_EXITS 24
struct NidlShardExit {
    u16 room, exitRoom;
    u8 x, y;    // the landing cell + 1, as a door's;
    u16 pad;
};

const struct NidlShardExit gNidlShardExits[NIDL_SHARD_EXITS] = { { 0 } };

// NidlShardExitOf(); A room's entry in gNidlShardExits, or NULL;
static const struct NidlShardExit *NidlShardExitOf(u16 room) {
    const struct NidlShardExit *exit;

    for (exit = gNidlShardExits; exit < gNidlShardExits + NIDL_SHARD_EXITS && exit->room; exit++)
        if (exit->room == room) return exit;
    return NULL;
}

// NidlShardExit(); The room and spot Kirby comes back to after the shard cutscene (sub_0804C614);
void NidlShardExit(struct Kirby *kirby) {
    const struct NidlShardExit *exit = NidlShardExitOf(kirby->base.roomId);

    if (exit) {
        kirby->roomId = exit->exitRoom;
        kirby->spawnLocation.x = exit->x;
        kirby->spawnLocation.y = exit->y;
        return;
    }
    kirby->roomId = 0x321;
    kirby->spawnLocation.x = kirby->unkF2;
    kirby->spawnLocation.y = kirby->unkF3;
}

