/* Skip; The randomizer's "Skip the tutorial" option: a new save starts past the tutorial (rooms 0x323-0x325) and the cutscene after
 * it, in Central Circle, with what the tutorial's big chest holds. A new save is a cleared one: ClearSaveBuffer zeroes the world
 * props of a blank, erased or unreadable file, then runs sbClearFunctions[1] (sbWorldPropsClear), which KatAMExpansion.cs points
 * here. gAIKirbyState AI_KIRBY_STATE_NORMAL is what the game sets once that cutscene ends (sub_0814AF04): from then on a new game,
 * a respawn and the help screens are the game's after the tutorial (sub_080332BC starts Kirby in Central Circle, sub_08002C98's
 * tables); the chest's item is given as opening it does (sub_0800AFC8: maps, sprays, music, an HP up; its treasure index marked,
 * so the collection counts it, and a mirror shard chest's, 0x80 + n, is the shard). The demos clear too and set the state anyway.
 * This file works without the rest of the expansion: the option copies the code even when no NiDL ROM is loaded; */
#include "global.h"
#include "data.h"
#include "treasures.h"

// NidlSkip; Filled in by KatAMExpansion.cs: the original clear, the tutorial big chest's item (template subtype1) and index (unk11);
struct NidlSkip {
    void (*clear)(void);
    u8 item;
    u8 index;
};

const struct NidlSkip gNidlSkip = { NULL, 0, 0 };

void NidlSkipClear(void) {
    u8 item = gNidlSkip.item;

    gNidlSkip.clear();
    gAIKirbyState = AI_KIRBY_STATE_NORMAL;
    gTreasures.chestFields[gNidlSkip.index >> 3] |= 1 << (gNidlSkip.index & 7);
    if (item == 6)
        ++gTreasures.vitalityField;
    else if (item >= 0xA && item <= 0x13)
        gTreasures.bigChestField |= 1 << (item - 0xA);
    else if (item >= 0x14 && item <= 0x21)
        gTreasures.sprayPaintField |= 1 << (item - 0x14);
    else if (item >= 0x28 && item <= 0x32)
        gTreasures.musicPlayerAndSheetsField |= 1 << (item - 0x28);
}
