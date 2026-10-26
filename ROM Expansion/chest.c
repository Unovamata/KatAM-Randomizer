/* Chest; Mirror shards in chests (the randomizer's "In chests" option for the mirror shards it adds): a chest holding the item
 * NIDL_CHEST_SHARD + n (an item id KatAM doesn't use) holds mirror shard n. Opening one (sub_0800AFC8, detoured here by
 * KatAMExpansionChests.cs; other chests run the original) collects the shard, so dying before touching it loses nothing, and lets
 * out of the chest the game's own shard object (subtype n, as the boss of shard n drops it), which Kirby takes as a boss's: the
 * shard cutscene. Opened once shard n is owned (from a boss, the field, another chest), it lets out a Maxim Tomato instead, as an
 * owned field shard is one. Each such chest has a treasure index (template unk11) of its own, 0x88 + k (KatAMItems.cs, ShardChestIndex):
 * HasChest reads it, as this marks it, past chestFields, in gTreasures.shardField's bits 8-23, which nothing else uses
 * (NumShardsCollected counts bits 0-7, NumChestsCollected the first 0x80 indices), so the chest shows open once it's been opened,
 * and only then. They aren't in the rooms' 1E8 chest lists (so the pause map doesn't show them): this doesn't mark them there
 * (sub_08002B30 would never find them). This file works without the rest of the expansion: the option copies the code even when no
 * NiDL ROM is loaded; */
#include "global.h"
#include "chest.h"
#include "bonus.h"
#include "shard.h"
#include "random.h"
#include "code_0806F780.h"
#include "object.h"
#include "kirby.h"
#include "functions.h"
#include "treasures.h"
#include "save.h"
#include "multi_08030C94.h"
#include "constants/object_types.h"
#include "constants/songs.h"

#define NIDL_CHEST_SHARD 0x40
#define NIDL_CHEST_RISE 0x18    // how far above the chest's floor the shard floats (pixels);
#define NIDL_SHARD_STAYS 0x8000 // template unk22: a shard that isn't a boss's (a chest's): after its cutscene Kirby stays (NidlShardDone);

// NidlChest; Filled in by KatAMExpansionChests.cs: the trampoline running the detoured function;
struct NidlChest {
    void (*open)(struct Chest *);
};

const struct NidlChest gNidlChest = { NULL };

// Room for the trampoline KatAMExpansionChests.cs writes (so the option needs no other free space);
const u32 gNidlChestTrampolines[8] = { 0 };

// NidlChestOpened(); Open and emptied, as the game's own chests once their item is out (sub_0800BD9C);
static void NidlChestOpened(struct Chest *chest) {
    chest->obj.base.flags |= 4;
}

// NidlChestOpen(); A Kirby opened the chest (chest.c, sub_0800AFC8): its own index marked, its shard, or a Maxim Tomato once it's owned;
void NidlChestOpen(struct Chest *chest) {
    u16 shard = chest->unkE0 - NIDL_CHEST_SHARD;
    bool32 owned;

    if (shard >= 8) {
        gNidlChest.open(chest);
        return;
    }
    chest->obj.unk83 = 1;
    owned = HasShard(shard);
    gTreasures.chestFields[chest->unkE2 >> 3] |= 1 << (chest->unkE2 & 7);
    CollectShard(shard);
    PlaySfx(&chest->obj.base, owned ? SE_CHEST_COLLECT_ITEM : SE_CHEST_OPEN);
    if (!(gUnk_0203AD10 & 0x10)) {
        if (!(gUnk_0203AD10 & 2) || gLocalPlayerId == gUnk_0203AD24)
            UpdateSaveBufferByOffset(SAVE_BUFFER_TYPE_WORLD_PROPS, gSaveID > 2 ? 0 : gSaveID);
        else
            sub_08031CE4(8);
    }
    if (owned)
        CreateObjTemplateAndObj(chest->obj.base.unk56, 1, 0x24, chest->obj.base.x >> 8, (chest->obj.base.y >> 8) - NIDL_CHEST_RISE, 0, 0x1F,
                                0, 0, OBJ_TOMATO, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    else
        CreateObjTemplateAndObj(chest->obj.base.unk56, 1, 0x24, chest->obj.base.x >> 8, (chest->obj.base.y >> 8) - NIDL_CHEST_RISE, 0, 0x1F,
                                0, 0, OBJ_MIRROR_SHARD, shard, 0, 0, NIDL_SHARD_STAYS, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    // As the item popups do once their item is given (sub_080029F4: lever walls count these);
    gUnk_02023510[gCurLevelInfo[chest->obj.base.unk56].unk65E]++;
    chest->obj.unk78 = NidlChestOpened;
}

/* Field shards: the shards the randomizer puts in rooms instead of in chests (object 0x65, subtype n, from the room's own object
 * list) spawn every time the room is entered, owned or not (CreateShard never asks), and touching one plays the shard cutscene and
 * throws Kirby out of the level, even for a shard he has: one on the way through a room blocks it. Its spawn (gSpawnFuncTable1[0x65],
 * pointed here by KatAMExpansionChests.cs) makes an owned one a Maxim Tomato instead, as a boss fought again leaves. Only the rooms'
 * shards (their templates in the ROM): the ones the game makes as it goes (a boss's, a chest's, which collects its shard first, a NiDL
 * boss room's way out) and the Dimension Mirror (subtype 8) stay as they are; */
/* Bosses' shards: the randomizer's "Bosses give random shards" sets which shard each of KatAM's bosses leaves (bossShards, by the
 * shard it leaves in the game; KatAMExpansionChests.cs also changes the shard number each boss's defeat asks HasShard about, so an
 * owned one still leaves a tomato). A boss's shard is made in its room (the boss rooms, by the game's shard: gUnk_0835105C's), its
 * template's subtype the game's: it's made as the game's, then given the new one (unkD4, the cutscene and the shard it collects;
 * unk83, the piece drawn); the template keeps the game's, which places the victory dance in that room (gUnk_08352DBE). The NiDL
 * bosses' shards are in their templates; */
struct NidlShard {
    void *(*create)(struct ObjectTemplate *, u8);
    u8 bossShards[8];
};

const struct NidlShard gNidlShard = { NULL, { 0, 1, 2, 3, 4, 5, 6, 7 } };

static const u16 sBossRooms[8] = { 0x144, 0x2C6, 0x1A1, 0x336, 0xDB, 0x204, 0x2E2, 0x81 };

/* NidlFieldTomato(); A Maxim Tomato from the shard's own template, as CreateBonus makes a room's (subtype2 0: on the floor), the type
 * changed before its sprite is made; eaten, it's gone until Kirby comes back to the room, as the room's food (the template's
 * slot, unk2/unk3, the shard's); */
static struct Object *NidlFieldTomato(struct ObjectTemplate *template, u8 playerId) {
    struct Task *task = TaskCreate(ObjectMain, sizeof(struct Object), 0x2F9C, TASK_USE_EWRAM, ObjectDestroy);
    struct Object *obj = TaskGetStructPtr(task);

    InitObject(obj, template, playerId);
    obj->type = OBJ_TOMATO;
    obj->unk78 = gUnk_08351648[OBJ_TOMATO].unk10;
    obj->unk80 = gUnk_08351648[OBJ_TOMATO].unk4;
    obj->kirbyAbility = gUnk_08351648[OBJ_TOMATO].kirbyAbility;
    obj->base.flags |= 0x10000000;
    obj->base.unkC |= 1;
    obj->base.flags |= 0x10000;
    obj->base.flags |= 0x200000;
    obj->base.flags |= 0x400000;
    obj->base.unk68 &= ~7;
    obj->base.unk5C |= ~0xA7;
    obj->base.flags |= 1;
    obj->base.unkC |= 0x100;
    sub_0803E2B0(&obj->base, -5, 1 - 7, 5, 5);
    ObjectSetBounds(&obj->base, -6, -7, 6, 7);
    obj->base.y = ((obj->base.y + (obj->base.unk3F << 8) + 0xFFF) & 0xFFFFF000) - (obj->base.unk3F << 8) - 1;
    obj->base.unk4C = obj->base.y;
    ObjectInitSprite(obj);
    obj->base.sprite.unk14 = 0x700;
    sub_08123780(obj);
    if (!(template->unk22 & 4))
        sub_0809513C(&obj->base, 0, 0);
    if (template->unk22 & 8) {
        obj->base.flags |= 0x40;
        obj->base.flags |= 0x100;
    }
    return obj;
}

void *NidlCreateShard(struct ObjectTemplate *template, u8 playerId) {
    struct Shard *shard;
    u8 n = template->subtype1;

    if ((u32)template >= 0x08000000 && n < 8 && HasShard(n))
        return NidlFieldTomato(template, playerId);
    shard = gNidlShard.create(template, playerId);
    if ((u32)template < 0x08000000 && n < 8 && shard->obj.base.roomId == sBossRooms[n]) {
        shard->unkD4 = gNidlShard.bossShards[n];
        shard->obj.unk83 = gNidlShard.bossShards[n];
    }
    return shard;
}

void sub_0801BD68(struct Shard *);

#define DANCE_MARGIN 72 // (pixels: four dancers stand 64 either side of the spot, sub_0804BD98's gUnk_0834C384)

/* FloorRow(); The first row at or below `row` in column `x` with ground in it (any tile but an empty one: gCollisionAttributes
 * bit 0), from the top of the ground there if `row` is in it, or -1 if the column has none down to the room's bottom; */
static s32 FloorRow(u8 playerId, s32 x, s32 row) {
    struct LevelInfo *level = &gCurLevelInfo[playerId];
    s32 rows = level->roomHeight / 16;

    if (x < 0 || x >= level->roomWidth / 16 || row < 0) return -1;
    if (row < rows && !(gCollisionAttributes[GetCollisionTile(playerId, x, row)] & 1)) {
        while (row > 0 && !(gCollisionAttributes[GetCollisionTile(playerId, x, row - 1)] & 1)) row--;
        return row;
    }
    for (; row < rows; row++)
        if (!(gCollisionAttributes[GetCollisionTile(playerId, x, row)] & 1)) return row;
    return -1;
}

/* NidlShardDance(); A NiDL boss's shard after its cutscene (its room not one of KatAM's boss rooms): sub_0801BD68's victory dance,
 * each Kirby in the room once, on the ground under the shard, each dancer on the ground under him (KatAM's spot is its table's for
 * the shard, gUnk_08352DBE: made for that shard's KatAM boss room, it can be in the ground or past the room's edge of a NiDL boss's,
 * where Kirby died: Meta Knight's with shard 5, (46, 17), past its corner); none under the shard, KatAM's dance; */
static void NidlShardDance(struct Shard *shard) {
    struct LevelInfo *level = &gCurLevelInfo[shard->obj.base.unk56];
    u16 room = shard->obj.base.roomId, i;
    u8 order[4], count = 0;
    s32 x = shard->unkD8, row, own;

    if (level->roomWidth >= 2 * DANCE_MARGIN) {
        if (x < DANCE_MARGIN) x = DANCE_MARGIN;
        if (x > level->roomWidth - DANCE_MARGIN) x = level->roomWidth - DANCE_MARGIN;
    }
    row = FloorRow(shard->obj.base.unk56, x >> 4, shard->unkDA >> 4);
    if (row < 0) {
        sub_0801BD68(shard);
        return;
    }
    for (i = 0; i < 4; i++)
        if (gKirbys[i].base.roomId == room) order[i] = count++;
    for (i = 0; i < 4; i++) {
        struct Kirby *kirby = &gKirbys[i];

        if (kirby->base.roomId != room || (shard->unkE0 >> i) & 1) continue;
        // (sub_0804BD98 stands him at the spot's x + his place in the row - 128, on the ground of row `row`);
        sub_0804BD98(kirby, order[i], count, shard->unkDC, (x >> 4) - 8, row);
        own = FloorRow(shard->obj.base.unk56, kirby->base.x >> 12, row);
        if (own >= 0) kirby->base.y = own * 0x1000 - 0x800;
        shard->unkE0 |= 1 << i;
    }
}

/* NidlShardDone(); The shard's step once its cutscene ends (sub_0801C558, which jumps here: replaced whole, its work done here): a
 * boss's shard starts the victory dance (sub_0801BD68, as sub_0801C558 does; a NiDL boss's, NidlShardDance()), which ends with Kirby
 * taken out of the level (to Central Circle, or the NiDL hub for a NiDL boss); a shard that isn't a boss's (a room's own, its
 * template in the ROM, or a chest's, NIDL_SHARD_STAYS) is simply gone, its shard collected by the cutscene, and Kirby plays on where
 * he is: no dance, no trip out; */
void NidlShardDone(struct Shard *shard) {
    struct ObjectTemplate *template = shard->obj.objTemplate;
    void (*dance)(struct Shard *) = NidlShardDance;
    u8 i;

    if ((u32)template >= 0x08000000 || (template->unk22 & NIDL_SHARD_STAYS)) {
        shard->obj.base.flags |= 0x1000;
        return;
    }
    shard->unkDC = (Rand16() & 1) + 2;
    shard->unkE0 = 0;
    for (i = 0; i < 8; i++)
        if (shard->obj.base.roomId == sBossRooms[i]) dance = sub_0801BD68;
    shard->obj.unk78 = dance;
    dance(shard);
}
