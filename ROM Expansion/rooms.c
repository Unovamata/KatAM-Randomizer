/* rooms.c: rooms bigger than KatAM's 1950 blocks (16x16 pixels each), for the Nightmare in Dream Land rooms (KatAMExpansionRooms.cs).
 * Each room loaded at once (up to 4: the player's and the CPU helpers', one slot each, gCurLevelInfo[].unk65E) has its collision
 * (gUnk_02024ED0[4][1950]), its CPU helpers' zones (gUnk_02026D60[4][1954]) and its tilemap (gUnk_02028EE0[4], 1950 * 4 entries) in
 * EWRAM. Here each slot has room for BIG_BLOCKS blocks: slots 0 and 1 in those arrays (they hold two of them), slots 2 and 3 at the
 * end of the EWRAM heap (gEwramHeap), which KatAMExpansionRooms.cs shrinks to NIDL_HEAP_SIZE (EwramInitHeap's 0x20080: the game
 * uses a few KB of it). These replace the game's functions that use those arrays (the same code, from the decompilation, with
 * BigCollision, BigZones and BigTilemap for them): FillLevelInfo (loads a room into its slot), GetCollisionTile and the other
 * collision reads, sub_0800255C (sets a block), sub_080025AC and sub_08002624 (a block's special block), sub_08001408 (a special
 * block changes the tilemap and collision), sub_080015A8 (reads a block's tilemap) and sub_080026A8 (the zones).
 * Each ported room has a doors list of its own (gUnk_08D640A4, from NIDL_DOORS_FIRST: a mirror looks its door up there every frame,
 * and one list for them all made NiDL's rooms lag), and KatAM keeps which of a list's doors were used (and whether its room was seen,
 * bit 15) in gVisitedDoors, a u16 per list, NIDL_DOORS_FIRST of them: the ported rooms' are kept here instead, after the heap
 * (NIDL_VISITED, not saved: their mirrors show as new again after a reset), by the functions reading and setting those bits
 * (sub_08002A5C, sub_08002A80, sub_08002AD0, sub_08002B30, sub_08002BA8, sub_08002C18); */
#include "global.h"
#include "data.h"
#include "functions.h"
#include "level.h"
#include "main.h"
#include "task.h"
#include "bg.h"
#include "gba/macro.h"
#include "malloc_ewram.h"
#include "multi_08030C94.h"

#define BIG_BLOCKS 3584
#define BIG_ZONES (BIG_BLOCKS + 4)
#define BIG_TILEMAP (BIG_BLOCKS * 8)
#define NIDL_HEAP_SIZE 0xE478   // 0x20080 - NIDL_VISITED_SIZE - 2 * (BIG_TILEMAP + BIG_BLOCKS + BIG_ZONES): KatAMExpansionRooms.cs's NidlHeapSize;
#define NIDL_VISITED_SIZE 0x400
#define NIDL_DOORS_FIRST 0x120  // ARRAY_COUNT(gVisitedDoors): KatAMExpansionRooms.cs's FirstDoorsList;
#define NIDL_VISITED_MAGIC 0x4E494456

void LoadLevelGfx(u8 playerId, const u16 **arg1, const u16 **arg2);

#define NIDL_VISITED ((u32 *)((u8 *)gEwramHeap + NIDL_HEAP_SIZE))
#define BIG_AREA ((u8 *)gEwramHeap + NIDL_HEAP_SIZE + NIDL_VISITED_SIZE)

// Visited(); A room's doors list's visited bits (see above): KatAM's, or the ported rooms' here (cleared the first time: a magic word
// first);
static u16 *Visited(u16 roomId) {
    u16 list = gRoomProps[roomId].doorsIdx;
    u32 *area = NIDL_VISITED;

    if (list < NIDL_DOORS_FIRST) return &gVisitedDoors[list];
    if (area[0] != NIDL_VISITED_MAGIC) {
        u32 i;
        for (i = 1; i < NIDL_VISITED_SIZE / 4; i++) area[i] = 0;
        area[0] = NIDL_VISITED_MAGIC;
    }
    list -= NIDL_DOORS_FIRST;
    if (list >= NIDL_VISITED_SIZE / 2 - 2) list = NIDL_VISITED_SIZE / 2 - 3;
    return (u16 *)(area + 1) + list;
}

u32 BigSub_08002A5C(u16 roomId)
{
    return *Visited(roomId) >> 0xF;
}

void BigSub_08002A80(u16 roomId)
{
    *Visited(roomId) |= 0x8000;
}

u8 BigSub_08002AD0(u16 roomId)
{
    u8 count = 0;
    u8 doors = gUnk_08D640A4[gRoomProps[roomId].doorsIdx]->unk04;
    u16 visited = *Visited(roomId);
    u16 i;

    for (i = 0; i < doors; i++) {
        if ((visited >> i) & 1)
            count++;
    }
    return count;
}

void BigSub_08002B30(u16 roomId, s16 x, s16 y)
{
    struct Unk_08930E00_1 *entry = gUnk_08D640A4[gRoomProps[roomId].doorsIdx]->unk00.pat1;
    u16 i;

    for (i = 0; ; i++) {
        if (entry->unk00 == 1 && entry->unk04 == x && entry->unk06 == y) {
            *Visited(roomId) |= 1 << i;
            return;
        }
        entry = (void *)entry + entry->unk01;
    }
}

u32 BigSub_08002BA8(u16 roomId, u16 room, u8 x, u8 y)
{
    struct Unk_08930E00_2 *entry = gUnk_08D640A4[gRoomProps[roomId].doorsIdx]->unk00.pat2;
    u16 i;

    for (i = 0; ; i++) {
        if (entry->unk00 == 2 && entry->unk04 == room && entry->unk06 == x && entry->unk07 == y)
            return *Visited(roomId) >> i & 1;
        entry = (void *)entry + entry->unk01;
    }
}

void BigSub_08002C18(u16 roomId, u16 room, u8 x, u8 y)
{
    struct Unk_08930E00_2 *entry = gUnk_08D640A4[gRoomProps[roomId].doorsIdx]->unk00.pat2;
    u16 i;

    for (i = 0; ; i++) {
        if (entry->unk00 == 2 && entry->unk04 == room && entry->unk06 == x && entry->unk07 == y) {
            *Visited(roomId) |= 1 << i;
            return;
        }
        entry = (void *)entry + entry->unk01;
    }
}

static inline u16 *BigTilemap(u8 slot) {
    return (u16 *)(slot < 2 ? (u8 *)gUnk_02028EE0 + slot * BIG_TILEMAP : BIG_AREA + (slot - 2) * BIG_TILEMAP);
}

static inline u8 *BigCollision(u8 slot) {
    return slot < 2 ? (u8 *)gUnk_02024ED0 + slot * BIG_BLOCKS : BIG_AREA + 2 * BIG_TILEMAP + (slot - 2) * BIG_BLOCKS;
}

static inline u8 *BigZones(u8 slot) {
    return slot < 2 ? (u8 *)gUnk_02026D60 + slot * BIG_ZONES : BIG_AREA + 2 * BIG_TILEMAP + 2 * BIG_BLOCKS + (slot - 2) * BIG_ZONES;
}

void BigSub_08001408(u8 playerId, union LevelInfo_1E0 arg1, void *arg2, u8 *arg3)
{
    u16 var0 = gCurLevelInfo[playerId].roomWidth >> 3;
    u16 currentRoom = gCurLevelInfo[playerId].currentRoom;
    u16 i;

    if (arg1.pat1->unk00 == 1) {
        struct Unk_0888562C *var1 = arg1.pat1;
        u16 *var2 = arg2;

        u16 *var3 = BigTilemap(gCurLevelInfo[playerId].unk65E)
                  + ((var1->unk03 << 1) * var0 + var1->unk02 * 2);

        u8 *var4;

        if (arg2) {
            var2[0] = var3[0];
            var2[1] = var3[1];
            var2[2] = var3[var0];
            var2[3] = var3[var0 + 1];
        }

        var3[0]        = var1->unk08[0];
        var3[1]        = var1->unk08[1];
        var3[var0]     = var1->unk08[2];
        var3[var0 + 1] = var1->unk08[3];

        var4 = BigCollision(gCurLevelInfo[playerId].unk65E)
             + (var1->unk03 * (var0 / 2) + var1->unk02);

        if (arg3)
            arg3[0] = var4[0];

        var4[0] = var1->unk10;
    }
    else {
        struct Unk_0888562C_2 *var5 = arg1.pat2;
        u8 *var6 = arg2;

        u8 *var7 = ((u8 *)BigTilemap(gCurLevelInfo[playerId].unk65E))
                 + ((var5->unk03 << 1) * var0 + var5->unk02 * 2);

        u8 *var8;

        if (arg2) {
            var6[0] = var7[0];
            var6[1] = var7[1];
            var6[2] = var7[var0];
            var6[3] = var7[var0 + 1];
        }

        var7[0]        = var5->unk08[0];
        var7[1]        = var5->unk08[1];
        var7[var0]     = var5->unk08[2];
        var7[var0 + 1] = var5->unk08[3];

        var8 = BigCollision(gCurLevelInfo[playerId].unk65E)
             + (var5->unk03 * (var0 / 2) + var5->unk02);

        if (arg3)
            arg3[0] = var8[0];

        var8[0] = var5->unk0C;
    }

    for (i = 0; i < gNumKirbys; i++) {
        if (gCurLevelInfo[i].currentRoom != currentRoom)
            continue;

        gCurLevelInfo[i].unkC0[1].prevScrollY = 0x7FFF;
        gCurLevelInfo[i].unkC0[1].prevScrollX = 0x7FFF;
    }
}

void BigSub_080015A8(u8 playerId, u16 arg1, u16 arg2, void *arg3)
{
    u32 var0 = gCurLevelInfo[playerId].roomWidth / 8;
    u16 unk1C = gForegroundTilemaps[gRoomProps[gCurLevelInfo[playerId].currentRoom].mapDataIdx]->unk1C;

    if (unk1C == 0) {
        u16 *var1 = arg3;
        u16 *var2 = BigTilemap(gCurLevelInfo[playerId].unk65E) + ((arg2 << 1) * var0 + arg1 * 2);

        var1[0] = var2[0];
        var1[1] = var2[1];
        var1[2] = var2[var0];
        var1[3] = var2[var0 + 1];
    }
    else {
        u8 *var3 = arg3;
        u8 *var4 = ((u8 *)BigTilemap(gCurLevelInfo[playerId].unk65E)) + ((arg2 << 1) * var0 + arg1 * 2);

        var3[0] = var4[0];
        var3[1] = var4[1];
        var3[2] = var4[var0];
        var3[3] = var4[var0 + 1];
    }
}

u8 BigGetCollisionTile(u8 playerId, u16 x, u16 y)
{
    struct LevelInfo *var0 = gCurLevelInfo + playerId;

    return BigCollision(var0->unk65E)[
        var0->roomWidth / 16 * y + x
    ];
}

u8 BigSub_08002434(struct LevelInfo *arg0, u16 x, u16 y)
{
    return BigCollision(arg0->unk65E)[
        arg0->roomWidth / 16 * y + x
    ];
}

u8 BigSub_08002470(u8 playerId, u16 x, u16 y)
{
    struct LevelInfo *var0 = gCurLevelInfo + playerId;

    u16 roomWidth = var0->roomWidth / 16;
    u16 roomHeight = var0->roomHeight / 16;

    // TODO: UB
    if ((s16)x < 0) {
        x = 0;
    }
    else if (roomWidth <= x) {
        x = roomWidth - 1;
    }

    if ((s16)y < 0) {
        y = 0;
    }
    else if (roomHeight <= y) {
        y = roomHeight - 1;
    }

    return BigCollision(var0->unk65E)[y * roomWidth + x];
}

u8 BigSub_080024F0(struct LevelInfo *arg0, u16 x, u16 y)
{
    u16 roomWidth = arg0->roomWidth / 16;
    u16 roomHeight = arg0->roomHeight / 16;

    // TODO: UB
    if ((s16)x < 0) {
        x = 0;
    }
    else if (roomWidth <= x) {
        x = roomWidth - 1;
    }

    if ((s16)y < 0) {
        y = 0;
    }
    else if (roomHeight <= y) {
        y = roomHeight - 1;
    }

    return BigCollision(arg0->unk65E)[y * roomWidth + x];
}

u8 BigSub_0800255C(u8 playerId, u16 x, u16 y, u8 value)
{
    struct LevelInfo *var0 = gCurLevelInfo + playerId;
    u32 var1 = var0->roomWidth / 16 * y + x;

    u8 old = BigCollision(var0->unk65E)[var1];
    BigCollision(var0->unk65E)[var1] = value;

    return old;
}

union LevelInfo_1E0 BigSub_080025AC(u8 playerId, u8 x, u8 y)
{
    struct LevelInfo *var0 = gCurLevelInfo + playerId;
    union LevelInfo_1E0 var1 = var0->unk1E0;
    u8 var2 = BigCollision(var0->unk65E)[y * (var0->roomWidth / 16) + x];

    for (;;) {
        if (
            var1.pat1->unk01 == var2 &&
            var1.pat1->unk02 == x &&
            var1.pat1->unk03 == y
        ) {
            return var1;
        }

        var1.pat1 = (void *) var1.pat1 + var1.pat1->unk04;
    }
}

union LevelInfo_1E0 BigSub_08002624(u8 playerId, u8 x, u8 y, u8 arg3)
{
    struct LevelInfo *var0 = gCurLevelInfo + playerId;
    union LevelInfo_1E0 var1 = var0->unk1E0;
    u8 var2 = BigCollision(var0->unk65E)[y * (var0->roomWidth / 16) + x];

    for (;;) {
        if (
            var1.pat1->unk01 == var2 &&
            var1.pat1->unk02 == x &&
            var1.pat1->unk03 == y &&
            --arg3 == 0xFF
        ) {
            return var1;
        }

        var1.pat1 = (void *) var1.pat1 + var1.pat1->unk04;
    }
}

u8 BigSub_080026A8(u8 playerId, u16 x, u16 y)
{
    struct LevelInfo *var0 = gCurLevelInfo + playerId;

    return BigZones(var0->unk65E)[
        var0->roomWidth / 16 * y + x
    ];
}

void BigFillLevelInfo(u8 playerId, u16 roomId, const u16 **arg2, const u16 **arg3)
{
    struct Unk_08002E48 *var0 = TaskGetStructPtr(gUnk_02023354), *var1 = var0;
    struct LevelInfo *levelInfo = gCurLevelInfo + playerId;
    struct Background *bg2 = &levelInfo->unkC0[2];
    struct Background *bg0 = &levelInfo->unkC0[0];
    u16 prevRoom = levelInfo->currentRoom;
    u8 i = 0;

    if (prevRoom != 0xFFFF)
        sub_08003028(playerId, playerId == var1->unk0);

    levelInfo->unk5FA = levelInfo->currentRoom;
    levelInfo->currentRoom = roomId;

    if (roomId != 0xFFFF) {
        s32 x, y;

        CpuCopy16(gForegroundTilemaps[gRoomProps[roomId].mapDataIdx], levelInfo->unk180, sizeof(struct RoomTiledBG));
        CpuCopy16(gRoomTiledBGs[gRoomProps[roomId].backgroundIdx], levelInfo->unk180 + 1, sizeof(struct RoomTiledBG));

        if (gRoomProps[roomId].objectList2Idx != 0xFFFF)
            CpuCopy16(gLevelObjLists[gRoomProps[roomId].objectList2Idx].unk, levelInfo->unk180 + 2, sizeof(struct RoomTiledBG));
        else
            CpuFill16(0xFFFF, levelInfo->unk180 + 2, sizeof(struct RoomTiledBG));

        levelInfo->roomWidth = levelInfo->unk180[0].width * 8;
        levelInfo->roomHeight = levelInfo->unk180[0].height * 8;
        levelInfo->levelMinPosition.x = 0x800;
        levelInfo->levelMinPosition.y = 0x800;
        levelInfo->levelMaxPosition.x = x = (levelInfo->roomWidth - 8) * 0x100;
        levelInfo->levelMaxPosition.y = y = (levelInfo->roomHeight - 2) * 0x100;
        levelInfo->unk58 = 0x800;
        levelInfo->unk5C = 0x800;
        levelInfo->unk60 = x;
        levelInfo->unk64 = y;
        levelInfo->unk68 = 0x200;
        levelInfo->unk_S32Vec2_6C.x = 0x800;
        levelInfo->unk_S32Vec2_6C.y = 0x800;
        levelInfo->unk_S32Vec2_74.x = x;
        levelInfo->unk_S32Vec2_74.y = (levelInfo->roomHeight - 0x28) * 0x100;
        levelInfo->unk_S32Vec2_7C.x = 0x800;
        levelInfo->unk_S32Vec2_7C.y = 0x800;
        levelInfo->unk_S32Vec2_84.x = levelInfo->unk_S32Vec2_74.x;
        levelInfo->unk_S32Vec2_84.y = levelInfo->unk_S32Vec2_74.y;
        levelInfo->unk8C = 0x200;
        levelInfo->unk90 = levelInfo->levelMinPosition.x;
        levelInfo->unk94 = levelInfo->levelMinPosition.y;
        levelInfo->unk98 = levelInfo->levelMaxPosition.x;
        levelInfo->unk9C = (levelInfo->roomHeight - 1) * 0x100;
        levelInfo->unkA0 = levelInfo->unk90;
        levelInfo->unkA4 = levelInfo->unk94;
        levelInfo->unkA8 = levelInfo->unk98;
        levelInfo->unkAC = levelInfo->unk9C;
        levelInfo->unkB0 = 0x200;
        levelInfo->unk8 = 0;
        levelInfo->viewportPosition.x = 0;
        levelInfo->viewportPosition.y = 0;
        levelInfo->viewportPositionUnmodified.x = 0;
        levelInfo->viewportPositionUnmodified.y = 0;
        levelInfo->unk1C = 0;
        levelInfo->unk20 = 0;
        levelInfo->altViewport_24.x = 0;
        levelInfo->altViewport_24.y = (levelInfo->unk180[2].height * 8 - 0xA0) * 0x100;
        levelInfo->unk2C = 0;
        levelInfo->unk30 = 0;
        levelInfo->altViewport_34.x = gRoomProps[roomId].unk0A;
        levelInfo->altViewport_34.y = (levelInfo->unk180[1].height * 8 - ({ gRoomProps[roomId].unk0C + 0xA0; })) * 0x100;
        levelInfo->unk3C = 0;
        levelInfo->unk40 = 0;
        levelInfo->viewportModX_44 = 0;
        levelInfo->viewportModY_46 = 0;
        levelInfo->unk1E0 = gSolidityMaps[gRoomProps[roomId].solidityMapIdx]->unk4;
        levelInfo->objlistPtr = gLevelObjLists[gRoomProps[levelInfo->currentRoom].objectListIdx].obj;
        levelInfo->unk1E8 = gUnk_08D63C28[gRoomProps[levelInfo->currentRoom].unk22];
        // The extra scope releases the CpuFill temporaries' stack slots for reuse
        // by the fills further down; without it the frame grows from 0x30 to 0x38.
        {
            CpuFill32(0, levelInfo->unk5FC, sizeof(levelInfo->unk5FC));
            CpuFill32(0, levelInfo->unk63C, sizeof(levelInfo->unk63C));
        }

        if (levelInfo->unk180[1].width <= 0x20 && levelInfo->unk180[1].height <= 0x20) {
            bg2->unk26 = levelInfo->unk180[1].width;
            bg2->unk28 = levelInfo->unk180[1].height;
            bg2->unk2E &= ~0x20;
        }
        else {
            bg2->unk26 = 0x1F;
            bg2->unk28 = 0x15;
            bg2->unk2E |= 0x20;
        }

        bg2->scrollX = 0;
        bg2->scrollY = levelInfo->unk180[1].height * 8 - 0xA0;
        bg2->unk18 = 0;
        bg2->unk1A = 0;
        bg2->prevScrollX = 0x7FFF;
        bg2->prevScrollY = 0x7FFF;

        if (gRoomProps[roomId].priorityFlags & 4) {
            levelInfo->unkB8 = gRoomProps[roomId].unk0E;
            levelInfo->unkBA = gRoomProps[roomId].unk10;
        }
        else {
            s32 x2 = (levelInfo->unk180[1].width * 8 - 0xF0) * 0x100;
            s32 y2 = (levelInfo->unk180[1].height * 8 - 0xA0) * 0x100;

            if (x2 > 0) {
                if (gRoomProps[roomId].unk0E == -1)
                    levelInfo->unkB8 = x2 / 0x870;
                else
                    levelInfo->unkB8 = gRoomProps[roomId].unk0E;
            }
            else {
                levelInfo->unkB8 = 0;
            }

            if (y2 > 0) {
                if (gRoomProps[roomId].unk10 == -1)
                    levelInfo->unkBA = y2 / 0x5A0;
                else
                    levelInfo->unkBA = gRoomProps[roomId].unk10;
            }
            else {
                levelInfo->unkBA = 0;
            }
        }

        if (levelInfo->unk180[2].width == 0x20 && levelInfo->unk180[2].height == 0x20) {
            bg0->unk26 = 0x20;
            bg0->unk28 = 0x20;
            bg0->unk2E &= ~0x20;
        }
        else {
            bg0->unk26 = 0x1F;
            bg0->unk28 = 0x15;
            bg0->unk2E |= 0x20;
        }

        bg0->scrollX = 0;
        bg0->scrollY = levelInfo->unk180[2].height * 8 - 0xA0;
        bg0->unk18 = 0;
        bg0->unk1A = 0;
        bg0->prevScrollX = 0x7FFF;
        bg0->prevScrollY = 0x7FFF;
        levelInfo->unk_S16Vec2_B4.x = gRoomProps[roomId].unkXmod_06;
        levelInfo->unk_S16Vec2_B4.y = gRoomProps[roomId].unkXmod_08;
    }

    if (prevRoom != 0xFFFF) {
        for (i = 0; i < gNumKirbys; i++) {
            if (i == playerId)
                continue;
            if (gCurLevelInfo[i].currentRoom == prevRoom)
                break;
        }

        if (i < gNumKirbys) {
            while (levelInfo->unk1F0.first) {
                struct Unk_02023720 *node = sub_08002958(i);

                node->unk08 = levelInfo->unk1F0.first->unk08;
                sub_080028CC(playerId, levelInfo->unk1F0.first);
                sub_08002918(i, node);
            }
        }
        else {
            while (levelInfo->unk1F0.first)
                sub_080028CC(playerId, levelInfo->unk1F0.first);
        }
    }

    if (roomId != 0xFFFF) {
        for (i = 0; i < gNumKirbys; i++) {
            if (i == playerId)
                continue;
            if (gCurLevelInfo[i].currentRoom == roomId)
                break;
        }

        if (i < gNumKirbys) {
            levelInfo->unk65E = gCurLevelInfo[i].unk65E;
            levelInfo->unk180[0].tilemap = BigTilemap(levelInfo->unk65E);
        }
        else {
            u8 mask = 0;

            for (i = 0; i < gNumKirbys; i++) {
                if (i == playerId)
                    continue;
                if (gCurLevelInfo[i].currentRoom == 0xFFFF)
                    continue;
                mask |= 1 << gCurLevelInfo[i].unk65E;
            }

            for (i = 0; i < 4; i++) {
                if (!((mask >> i) & 1)) {
                    levelInfo->unk65E = i;
                    break;
                }
            }

            RLUnCompWram(gSolidityMaps[gRoomProps[roomId].solidityMapIdx]->unk0, BigCollision(levelInfo->unk65E));
            RLUnCompWram(gUnk_08D63C28[gRoomProps[roomId].unk22]->unk10, BigZones(levelInfo->unk65E));
            LZ77UnCompWram((const u32 *)levelInfo->unk180[0].tilemap, BigTilemap(levelInfo->unk65E));
            levelInfo->unk180[0].tilemap = BigTilemap(levelInfo->unk65E);
            CpuFill32(0, gUnk_02023358[levelInfo->unk65E], sizeof(gUnk_02023358[0]));
            CpuFill32(0, gUnk_02023388[levelInfo->unk65E], sizeof(gUnk_02023388[0]));
            gUnk_02023508[levelInfo->unk65E] = 0;
            gUnk_02023510[levelInfo->unk65E] = 0;
            gUnk_02026D50[levelInfo->unk65E] = 1;

            if (prevRoom == roomId)
                gUnk_02026D50[levelInfo->unk65E] |= 4;

            gUnk_02023518[levelInfo->unk65E] = gRoomProps[roomId].songIdx;

            {
                u8 j;

                for (j = 0; j < 2; j++)
                    gUnk_02023520[levelInfo->unk65E][j] |= 0xFFFF;
            }
        }
    }

    if (roomId != 0xFFFF) {
        levelInfo->unkC0[1].prevScrollX = 0x7FFF;
        levelInfo->unkC0[1].prevScrollY = 0x7FFF;
        levelInfo->unkC0[1].unk1A = 0;
        levelInfo->unkC0[1].unk18 = 0;
        levelInfo->unkC0[1].scrollX = 0;
        levelInfo->unkC0[1].scrollY = 0;
        levelInfo->unkC0[2].prevScrollX = 0x7FFF;
        levelInfo->unkC0[2].prevScrollY = 0x7FFF;
        levelInfo->unkC0[2].unk18 = 0;
        levelInfo->unkC0[2].unk1A = 0;
        levelInfo->unkC0[2].scrollX = 0;
        levelInfo->unkC0[2].scrollY = 0;
        levelInfo->unkC0[0].prevScrollX = 0x7FFF;
        levelInfo->unkC0[0].prevScrollY = 0x7FFF;
        levelInfo->unkC0[0].unk18 = 0;
        levelInfo->unkC0[0].unk1A = 0;
        levelInfo->unkC0[0].scrollX = 0;
        levelInfo->unkC0[0].scrollY = 0;
        levelInfo->unk1EC = 1;
        levelInfo->unk662 = 0x180;
        levelInfo->unk664 = 0x180;

        if (playerId == var1->unk0)
            LoadLevelGfx(playerId, arg2, arg3);

        sub_08001D18(playerId);
        levelInfo->viewportPositionUnmodified.x = levelInfo->viewportPosition.x;
        levelInfo->viewportPositionUnmodified.y = levelInfo->viewportPosition.y;
        sub_08002EC4(playerId, playerId == var1->unk0);
    }

    if ((gUnk_0203AD10 & 2) && playerId < gNumHumanPlayers && gLocalPlayerId != i)
        sub_08031CE4(8);
}
