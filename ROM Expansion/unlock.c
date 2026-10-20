/* Unlock; The randomizer's "Unlock the Ability Room" option: Central Circle's door to the ability room (0x3CA; the NiDL hub has a copy,
 * KatAMExpansionHub.cs) is a hub mirror object (0x8C, special_hub_mirror.c) of world slot ABILITY_ROOM_SLOT, hidden under its wall
 * until that slot's set: KatAM sets it once all 15 big switches are pressed (pause_world_map.c's WorldMapUnlockMain, then the door's
 * scene, sub_0801DA58). The hub mirror's init (SpecialHubMirrorInit, which waits for its slot) is swapped for NidlHubMirrorInit
 * (KatAMExpansionCalm.cs), which sets that slot first for that door, so it's open the first time Kirby sees it, on a new save or an
 * old one (saved with the world props as KatAM saves them). This file works without the rest of the expansion: the option copies the
 * code even when no NiDL ROM is loaded; */
#include "global.h"
#include "object.h"
#include "functions.h"

#define ABILITY_ROOM_SLOT 0x12

// NidlUnlock; Filled in by KatAMExpansion.cs: the hub mirror's own init;
struct NidlUnlock {
    void (*hubMirrorInit)(struct Object *);
};

const struct NidlUnlock gNidlUnlock = { NULL };

// NidlHubMirrorInit(); A hub mirror's init (its object's unk78 until its slot's set): the ability room's slot set first, then KatAM's;
void NidlHubMirrorInit(struct Object *mirror) {
    if (mirror->objTemplate->unk4 == ABILITY_ROOM_SLOT) *GetStateSlot(STATE_SLOT_WORLD, ABILITY_ROOM_SLOT, 0) = 1;
    gNidlUnlock.hubMirrorInit(mirror);
}
