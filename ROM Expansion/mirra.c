/* Mirra; KatAM's Mirras (object 0x34, mirra.c in the decompilation) fall, and open their door where they are: KatAM turns the engine's
 * gravity off for a Mirra (its start, sub_080B1174: flags 0x40, and 0x800, which keeps it from moving at all), so one stood in the air
 * once what it stood on broke, and opened its door (sub_080B11C0: the special block record of its template's cell) where it started.
 * Object 0x34's spawn (CreateMirra) becomes NidlCreateMirra, which turns its gravity back on, and sub_080B11C0 jumps to NidlMirraOpen
 * (KatAMExpansion.cs' HookMirras()), which opens the door of the cell the Mirra is in, or of the one it's falling to: the ROM Expansion
 * builds a door there for each cell a ported Mirra can land in (KatAMExpansionRooms.cs' MirraLandings()). KatAM's own Mirras stand on
 * solid ground, so they never fall: theirs stays KatAM's door; */
#include "global.h"
#include "object.h"
#include "functions.h"

#define BLOCK_RECORD 1
#define DOOR_RECORD 3
#define SOLID 0x200

// NidlMirra; Filled in by KatAMExpansion.cs: the Mirra's own spawn;
struct NidlMirra {
    void *(*create)(struct ObjectTemplate *, u8);
};

const struct NidlMirra gNidlMirra = { NULL };

// NidlCreateMirra(); Object 0x34's spawn: KatAM's Mirra, with the engine's gravity on (nothing turns it off again before it breaks);
void *NidlCreateMirra(struct ObjectTemplate *template, u8 playerId) {
    struct Object *mirra = gNidlMirra.create(template, playerId);

    mirra->base.flags &= ~(0x40 | 0x800);
    return mirra;
}

/* MirraDoor(); The special block record that makes the cell (x, y) a door, if there's one: a block record of the cell as it is now
 * whose new value is a door record's of that cell (KatAM finds a record by its cell and the cell's value, sub_080025AC; the cell read
 * through GetCollisionTile, which the ROM Expansion's bigger rooms replace: rooms.c); */
static bool32 MirraDoor(u8 playerId, u8 x, u8 y, union LevelInfo_1E0 *found) {
    u8 value = GetCollisionTile(playerId, x, y);
    struct Unk_0888562C *block, *door;
    struct LevelInfo *level = &gCurLevelInfo[playerId];

    for (block = level->unk1E0.pat1; block->unk04; block = (void *)block + block->unk04) {
        if (block->unk00 != BLOCK_RECORD || block->unk01 != value || block->unk02 != x || block->unk03 != y) continue;
        for (door = level->unk1E0.pat1; door->unk04; door = (void *)door + door->unk04) {
            if (door->unk00 == DOOR_RECORD && door->unk01 == block->unk10 && door->unk02 == x && door->unk03 == y) {
                found->pat1 = block;
                return TRUE;
            }
        }
    }
    return FALSE;
}

/* NidlMirraOpen(); sub_080B11C0, when a Mirra breaks (and when one already broken is made again): its encounter marked, as KatAM's, and
 * the door opened of the cell it's in, or of the first one below it that has a door (where it's falling to; nothing solid between), or
 * else of its template's cell, as KatAM's; */
void NidlMirraOpen(struct Object *mirra) {
    struct LevelInfo *level = &gCurLevelInfo[mirra->base.unk56];
    union LevelInfo_1E0 record;
    s32 x = mirra->base.x >> 12, y = mirra->base.y >> 12;

    if (mirra->objTemplate->unk14 != 0) gMirraEncounters |= 1 << (mirra->objTemplate->unk14 - 1);
    if (x >= 0 && x < level->roomWidth / 16) {
        for (y = y < 0 ? 0 : y; y < level->roomHeight / 16; y++) {
            if (gCollisionAttributes[GetCollisionTile(mirra->base.unk56, x, y)] & SOLID) break;
            if (MirraDoor(mirra->base.unk56, x, y, &record)) {
                sub_08001408(mirra->base.unk56, record, NULL, NULL);
                return;
            }
        }
    }
    sub_08001408(mirra->base.unk56, sub_080025AC(mirra->base.unk56, mirra->objTemplate->x >> 4, mirra->objTemplate->y >> 4), NULL, NULL);
}
