/* worldmap.c: the world map's doors for the Nightmare in Dream Land levels (see KatAMExpansionWorldMap.cs).
 * KatAM's world map (pause_world_map.c) has 15 doors, one per big switch: pressing a big switch runs gUnk_0834BD94[its template's
 * +0x14 + 9] (WorldMapUnlock*: CreateWorldMap(door)), which draws the path from the door to where it leads, dot by dot, and then
 * sets the door's world slot (WorldMapUnlockSave). Doors 16-23 are the NiDL levels': KatAMExpansionWorldMap.cs copies their data
 * (gNidlWorldMapDoors) into copies of the door tables, adds NidlWorldMapUnlock* to gUnk_0834BD94, and makes these functions
 * replace KatAM's:
 * - NidlWorldMapUnlockSave: the slot of every door (KatAM's switch, then the NiDL doors');
 * - NidlWorldMapGetCoorFromRoom: where Kirby is drawn on the map: in a NiDL level (area 11 + level), on its door; in the NiDL hub
 *   (gNidlHubRoom, set by KatAMExpansionRooms.cs), on Central Circle;
 * - NidlWorldMapDrawKirbys: in the pause menu, the unlocked NiDL doors (the map's door tile) and their paths (the dots of the
 *   unlocking animation: KatAM's paths are drawn in the map's tilemap, which has none for them) before the Kirbys;
 * - NidlGetKirbyRoomFlagIndex: the pause menu takes the NiDL levels' rooms and the NiDL hub (no area map) as the tutorial's: Select
 *   switches between the help and the world map once it is owned; */
#include "global.h"
#include "data.h"
#include "functions.h"
#include "kirby.h"
#include "save.h"
#include "sprite.h"
#include "pause_world_map.h"
#include "multi_08030C94.h"
#include "pause_area_map.h"

#define NIDL_WORLDMAP_FIRST_DOOR 16
#define NIDL_WORLDMAP_DOORS 8
#define NIDL_WORLDMAP_MAX_DOTS 20
#define NIDL_AREA_BASE 11
#define WORLDMAP_DOT_SPRITE_MAGIC 0x4E494D50
#define CENTRAL_CIRCLE_ROOM 0x321

u16 gNidlHubRoom = 0xFFFF;

struct NidlWorldMapDoor {
    u8 x, y;          // the NiDL level's door;
    u8 destX, destY;  // where its path leads: a KatAM door, another NiDL door, or Central Circle (112, 80);
    u8 slot;          // its world slot (the level's big switch's);
    u8 dotCount;
    u16 tilemapOffset;  // its door tile in the map's tilemap (BG screen 24);
    struct { u8 x, y; } dots[NIDL_WORLDMAP_MAX_DOTS];  // 4 bytes each (agbcc pads them), as KatAM's struct WorldMapDotCoor;
};

/* The doors, clear of KatAM's paths, doors and the map's captions, leading where the level's switch room's revealed mirror does
 * (KatAMExpansionRooms.cs' LevelLinks): Vegetable Valley right of Rainbow Route East, Ice Cream Island right of the Moonlight Mansion door,
 * Butter Building up from Rainbow Resort, Grape Garden left of Peppermint Palace East, Yogurt Yard left of Cabbage Cavern West, Orange
 * Ocean right of Cabbage Cavern East and the Fountain of Dreams below it, Rainbow Resort straight from Central Circle; their dots are
 * filled in by KatAMExpansionWorldMap.cs; */
struct NidlWorldMapDoor gNidlWorldMapDoors[NIDL_WORLDMAP_DOORS] = {
    { 204, 72, 168, 72, 0x13 + 1, 0, 0 },
    { 208, 100, 152, 88, 0x13 + 2, 0, 0 },
    { 176, 16, 152, 40, 0x13 + 3, 0, 0 },
    { 24, 72, 88, 56, 0x13 + 4, 0, 0 },
    { 24, 104, 80, 120, 0x13 + 5, 0, 0 },
    { 228, 120, 168, 120, 0x13 + 6, 0, 0 },
    { 152, 40, 112, 80, 0x13 + 7, 0, 0 },
    { 162, 136, 168, 120, 0x13 + 8, 0, 0 },
};

// KatAM's doors' slots (pause_world_map.c's WorldMapUnlockSave);
static const u8 sKatamDoorSlots[NIDL_WORLDMAP_FIRST_DOOR] = { 0, 0x2, 0x1, 0x6, 0x5, 0x9, 0xa, 0xd, 0xf, 0x7, 0x8, 0xb, 0xc, 0x3, 0x4, 0xe };

static u8 NidlDoorSlot(s32 door) {
    if (door > 0 && door < NIDL_WORLDMAP_FIRST_DOOR) return sKatamDoorSlots[door];
    if (door >= NIDL_WORLDMAP_FIRST_DOOR && door < NIDL_WORLDMAP_FIRST_DOOR + NIDL_WORLDMAP_DOORS) return gNidlWorldMapDoors[door - NIDL_WORLDMAP_FIRST_DOOR].slot;
    return 0;
}

void NidlWorldMapUnlockSave(s8 unlockedDoorId) {
    u8 slot = NidlDoorSlot(unlockedDoorId);

    if (slot) *GetStateSlot(STATE_SLOT_WORLD, slot, 0) = 1;
    if (!(gUnk_0203AD10 & 0x10)) {
        if (gUnk_0203AD10 & 0x2) {
            if (gLocalPlayerId == gUnk_0203AD24)
                UpdateSaveBufferByOffset(SAVE_BUFFER_TYPE_WORLD_PROPS, gSaveID > 2 ? 0 : gSaveID);
            else
                sub_08031CE4(0x8);
        }
        else
            UpdateSaveBufferByOffset(SAVE_BUFFER_TYPE_WORLD_PROPS, gSaveID > 2 ? 0 : gSaveID);
    }
}

// pause_world_map.c's, with the NiDL levels' areas;
struct NidlKirbysCoorByRoom { u32 roomId; u8 coor[4][2]; };
extern const struct NidlKirbysCoorByRoom *const gWorldMapKirbysCoorsByArea[];

u16 NidlWorldMapGetCoorFromRoom(u16 roomId, u8 playerId) {
    u16 roomIdx;
    u32 coor = 0;
    s32 area, areaId = AREA_TUTORIAL;

    if (roomId == gNidlHubRoom) roomId = CENTRAL_CIRCLE_ROOM;
    area = gUnk_08D6CD0C[roomId]->unk46;

    if (area >= NIDL_AREA_BASE && area < NIDL_AREA_BASE + NIDL_WORLDMAP_DOORS)
        return gNidlWorldMapDoors[area - NIDL_AREA_BASE].x << 8 | gNidlWorldMapDoors[area - NIDL_AREA_BASE].y;

    switch (area) {
    case 0:  areaId = AREA_RAINBOW_ROUTE; break;
    case 1:  areaId = AREA_MOONLIGHT_MANSION; break;
    case 4:  areaId = AREA_CARROT_CASTLE; break;
    case 6:  areaId = AREA_PEPPERMINT_PALACE; break;
    case 5:  areaId = AREA_OLIVE_OCEAN; break;
    case 2:  areaId = AREA_CABBAGE_CAVERN; break;
    case 7:  areaId = AREA_RADISH_RUINS; break;
    case 3:  areaId = AREA_MUSTARD_MOUNTAIN; break;
    case 8:  areaId = AREA_CANDY_CONSTELLATION; break;
    case 9:  areaId = AREA_TUTORIAL; break;
    case 10: areaId = AREA_DIMENSION_MIRROR; break;
    }

    for (roomIdx = 0; gWorldMapKirbysCoorsByArea[areaId][roomIdx].roomId != 0; roomIdx++) {
        if (gWorldMapKirbysCoorsByArea[areaId][roomIdx].roomId == roomId)
            coor = gWorldMapKirbysCoorsByArea[areaId][roomIdx].coor[playerId & 3][0] << 8 | gWorldMapKirbysCoorsByArea[areaId][roomIdx].coor[playerId & 3][1];
    }
    return coor;
}

// pause_world_map.c's struct WorldMap (its filler holds the dots' sprite);
struct NidlWorldMapKirby {
    struct Sprite kirby;
    struct Sprite abilityAccessory;
    u16 flags;
};
struct NidlWorldMap {
    u8 bg[0x40];
    struct NidlWorldMapKirby worldmapKirbys[4];
    struct Sprite dot;
    u8 filler1B8[0x34];
    u32 dotMagic;
    u8 rest[0x24];
};

void NidlWorldMapDrawKirbys(struct NidlWorldMap *worldmap) {
    s32 i, d;

    for (i = 0; i < NIDL_WORLDMAP_DOORS; i++) {
        const struct NidlWorldMapDoor *door = &gNidlWorldMapDoors[i];
        if (!*GetStateSlot(STATE_SLOT_WORLD, door->slot, 0)) continue;

        // Its door tile (as WorldMapSetTileDoorVisited's), then its path's dots;
        if (door->tilemapOffset) *((u16 *)BG_SCREEN_ADDR(24) + door->tilemapOffset) = 0x5;
        if (worldmap->dotMagic != WORLDMAP_DOT_SPRITE_MAGIC) {
            SpriteInitNoFunc(&worldmap->dot, (u32)OBJ_VRAM0 + 0x2000, 0x480, 0x373, 3, 0, 0xff, 0x10, 8, door->x, door->y, 0xc1000);
            worldmap->dotMagic = WORLDMAP_DOT_SPRITE_MAGIC;
        }
        sub_08155128(&worldmap->dot);
        for (d = 0; d < door->dotCount; d++) {
            worldmap->dot.x = door->dots[d].x;
            worldmap->dot.y = door->dots[d].y;
            sub_081564D8(&worldmap->dot);
        }
    }

    for (i = 0; i < 4; i++) {
        struct NidlWorldMapKirby *kirby = worldmap->worldmapKirbys + i;
        if (kirby->flags & 1) continue;
        sub_08155128(&kirby->kirby);
        sub_081564D8(&kirby->kirby);
        if (!(kirby->flags & 2)) {
            sub_08155128(&kirby->abilityAccessory);
            sub_081564D8(&kirby->abilityAccessory);
        }
    }
}

// pause_area_map.c's GetKirbyRoomFlagIndex, then the original (its trampoline, set by KatAMExpansionWorldMap.cs);
u32 (*gNidlRoomFlagIndexOriginal)(u32) = NULL;

u32 NidlGetKirbyRoomFlagIndex(u32 playerId) {
    u16 roomId = gKirbys[playerId].base.roomId;
    s32 area = gUnk_08D6CD0C[roomId]->unk46;

    if (roomId == gNidlHubRoom) return KIRBY_IN_TUTORIAL_ROOM;
    if (area >= NIDL_AREA_BASE && area < NIDL_AREA_BASE + NIDL_WORLDMAP_DOORS) return KIRBY_IN_TUTORIAL_ROOM;
    return gNidlRoomFlagIndexOriginal(playerId);
}

// What the NiDL big switches run (gUnk_0834BD94, after KatAM's 35 entries);
void NidlWorldMapUnlock0(void) { CreateWorldMap(NIDL_WORLDMAP_FIRST_DOOR + 0); }
void NidlWorldMapUnlock1(void) { CreateWorldMap(NIDL_WORLDMAP_FIRST_DOOR + 1); }
void NidlWorldMapUnlock2(void) { CreateWorldMap(NIDL_WORLDMAP_FIRST_DOOR + 2); }
void NidlWorldMapUnlock3(void) { CreateWorldMap(NIDL_WORLDMAP_FIRST_DOOR + 3); }
void NidlWorldMapUnlock4(void) { CreateWorldMap(NIDL_WORLDMAP_FIRST_DOOR + 4); }
void NidlWorldMapUnlock5(void) { CreateWorldMap(NIDL_WORLDMAP_FIRST_DOOR + 5); }
void NidlWorldMapUnlock6(void) { CreateWorldMap(NIDL_WORLDMAP_FIRST_DOOR + 6); }
void NidlWorldMapUnlock7(void) { CreateWorldMap(NIDL_WORLDMAP_FIRST_DOOR + 7); }
