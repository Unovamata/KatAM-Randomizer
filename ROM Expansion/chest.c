/* Chest; Mirror shards in chests (the randomizer's "In chests" option for the mirror shards it adds): a chest holding the item
 * NIDL_CHEST_SHARD + n (an item id KatAM doesn't use) holds mirror shard n. Opening one (sub_0800AFC8, detoured here by
 * KatAMExpansionChests.cs; other chests run the original) collects the shard, so dying before touching it loses nothing, and lets
 * out of the chest the game's own shard object (subtype n, as the boss of shard n drops it), which Kirby takes as a boss's: the
 * shard cutscene, then out of the level. The randomizer gives these chests the treasure index (template unk11) 0x80 + n: HasChest
 * reads past chestFields into gTreasures.shardField there, so the chest is open exactly when shard n is owned, wherever it came
 * from, and NumChestsCollected (the first 0x80) doesn't count it. They aren't in the rooms' 1E8 chest lists (so the pause map
 * doesn't show them): this doesn't mark them there (sub_08002B30 would never find them). This file works without the rest of the
 * expansion: the option copies the code even when no NiDL ROM is loaded; */
#include "global.h"
#include "chest.h"
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

// NidlChestOpen(); A Kirby opened the chest (chest.c, sub_0800AFC8);
void NidlChestOpen(struct Chest *chest) {
    u16 shard = chest->unkE0 - NIDL_CHEST_SHARD;

    if (shard >= 8) {
        gNidlChest.open(chest);
        return;
    }
    chest->obj.unk83 = 1;
    CollectShard(shard);
    PlaySfx(&chest->obj.base, SE_CHEST_OPEN);
    if (!(gUnk_0203AD10 & 0x10)) {
        if (!(gUnk_0203AD10 & 2) || gLocalPlayerId == gUnk_0203AD24)
            UpdateSaveBufferByOffset(SAVE_BUFFER_TYPE_WORLD_PROPS, gSaveID > 2 ? 0 : gSaveID);
        else
            sub_08031CE4(8);
    }
    CreateObjTemplateAndObj(chest->obj.base.unk56, 1, 0x24, chest->obj.base.x >> 8, (chest->obj.base.y >> 8) - NIDL_CHEST_RISE, 0, 0x1F,
                            0, 0, OBJ_MIRROR_SHARD, shard, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    // As the item popups do once their item is given (sub_080029F4: lever walls count these);
    gUnk_02023510[gCurLevelInfo[chest->obj.base.unk56].unk65E]++;
    chest->obj.unk78 = NidlChestOpened;
}
