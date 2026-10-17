/* NiDL bosses; Object type 0x43 (OBJ_EMPTY_43, which KatAM already treats as a boss everywhere: the boss music, its HP bar, a hit
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
 * (the behavior's low nibble) in the middle of the screen, which Kirby takes as a boss's (the shard cutscene, then the Dimension
 * Mirror); a Maxim Tomato instead when the shard is owned already (KatAM's bosses fought again leave one, sub_08088A38); */
void NidlBossDefeat(struct Object *obj) {
    struct NidlObject *n = (struct NidlObject *)obj;
    struct LevelInfo *level = &gCurLevelInfo[obj->base.unk56];
    s32 x = (level->viewportPosition.x >> 8) + SHARD_X, y = (level->viewportPosition.y >> 8) + SHARD_Y;
    u8 shard = n->variant & 7;

    if (HasShard(shard)) {
        BonusCreateTomatoAt(obj, x, y);
        return;
    }
    CreateObjTemplateAndObj(obj->base.unk56, 1, 0x24, x, y, 0, 0x1F, 0, 0, OBJ_MIRROR_SHARD, shard, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}
