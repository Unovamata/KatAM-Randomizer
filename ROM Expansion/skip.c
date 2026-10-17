/* Skip; The randomizer's "Skip the tutorial" option: a save starts as a continued one does, in Central Circle where loading a save puts
 * Kirby, with no opening cutscene, no tutorial (rooms 0x323-0x325) and no cutscene after it, with what the tutorial's big chest holds.
 * What decides it all is gAIKirbyState, saved with the world props: below AI_KIRBY_STATE_NORMAL (300) a game starts in the tutorial
 * room with Dark Meta Knight's intro (room 0x323's cutscene trigger), at 200 in Central Circle with the mirror cutscene (room
 * 0x321's), at 300 at the continue spot (sub_08002C98's tables), with neither. A new and a continued game start the same way:
 * sub_08142624 loads the file and runs sub_08123FD4, the single-player start; so does every respawn table after it (sub_08002C98).
 * Two hooks set the state:
 * • NidlSkipClear: ClearSaveBuffer, run on a blank, erased or unreadable file, runs sbClearFunctions[1] (sbWorldPropsClear), which
 *   KatAMExpansion.cs points here;
 * • NidlSkipStart: sub_08123FD4, detoured here (its trampoline in gNidlSkipTrampolines), for the files no clear touched: one saved by
 *   a ROM without the option (vanilla, an older build, an emulator's .sav kept under the ROM's name) or saved mid-tutorial; the
 *   state set, and the file saved, before the start reads it.
 * The chest's item is given as opening it does (sub_0800AFC8: maps, sprays, music, an HP up; its treasure index marked, so the
 * collection counts it, and a mirror shard chest's, 0x80 + n, is the shard), once: not when its index is marked already. The demos
 * clear too and set the state anyway. Multiplayer starts elsewhere (sub_080329C8), from the file's own state. This file works
 * without the rest of the expansion: the option copies the code even when no NiDL ROM is loaded; */
#include "global.h"
#include "data.h"
#include "treasures.h"
#include "save.h"

// NidlSkip; Filled in by KatAMExpansion.cs: the original clear, the tutorial big chest's item (template subtype1) and index (unk11), the
// original start's trampoline;
struct NidlSkip {
    void (*clear)(void);
    u8 item;
    u8 index;
    void (*start)(void);
};

const struct NidlSkip gNidlSkip = { NULL, 0, 0, NULL };

// Room for the start's trampoline KatAMExpansion.cs writes (so the option needs no other free space);
const u32 gNidlSkipTrampolines[8] = { 0 };

// NidlSkipGive(); The state past the tutorial and its cutscenes, and the chest's item unless it's been given;
static void NidlSkipGive(void) {
    u8 item = gNidlSkip.item, index = gNidlSkip.index;

    gAIKirbyState = AI_KIRBY_STATE_NORMAL;
    if (gTreasures.chestFields[index >> 3] & (1 << (index & 7)))
        return;
    gTreasures.chestFields[index >> 3] |= 1 << (index & 7);
    if (item == 6)
        ++gTreasures.vitalityField;
    else if (item >= 0xA && item <= 0x13)
        gTreasures.bigChestField |= 1 << (item - 0xA);
    else if (item >= 0x14 && item <= 0x21)
        gTreasures.sprayPaintField |= 1 << (item - 0x14);
    else if (item >= 0x28 && item <= 0x32)
        gTreasures.musicPlayerAndSheetsField |= 1 << (item - 0x28);
}

void NidlSkipClear(void) {
    gNidlSkip.clear();
    NidlSkipGive();
}

void NidlSkipStart(void) {
    if (gAIKirbyState < AI_KIRBY_STATE_NORMAL) {
        NidlSkipGive();
        UpdateSaveBufferByOffset(SAVE_BUFFER_TYPE_WORLD_PROPS, gSaveID > 2 ? 0 : gSaveID);
    }
    gNidlSkip.start();
}
