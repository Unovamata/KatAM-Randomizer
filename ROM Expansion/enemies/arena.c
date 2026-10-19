/* Arena; NiDL's mini-boss fights keep Kirby on their screen: the camera stops scrolling there and the screen's edges are walls.
 * KatAM's arenas are rooms a screen big, so its mini-bosses do neither, and NiDL's are open. KatAMExpansionRooms.cs puts one of
 * these before every mini-boss in a NiDL room (behavior NIDL_ARENA << 4, the fight's screen as its template's area, as the
 * mini-boss's): unseen and untouchable, once a Kirby is on that screen while a mini-boss (types 0x38-0x52, KatAM's or ported) is
 * alive in the room, it holds the camera there as KatAM's scroll locks do (scroll_lock.c, type 0x77: sub_0811C7D0 and sub_0811C8BC,
 * which eases the camera in) and keeps every Kirby in the room on it, until none is left; then it gives the camera back
 * (ScrollLockDestroy) and goes away. Its graphics are Broom Hatter's (never shown).
 * The doors of its fight (its screen, and the cells Kirby goes to from it within ARENA_REACH cells across, ARENA_REACH_Y up or
 * down: KatAMExpansionRooms.cs, ArenaCells()) are built shut (BuildSolidity()): their cells
 * aren't doors (air, water), only their door records (kind 3, KatAM's door collision 0xFF, 0xFD in water) say they are, and their
 * mirrors (subtype 1) hide with them, as KatAM's hidden mirrors do. Once no mini-boss is alive in the room, it opens them: each
 * door record there whose cell has no other record (secret doors' blocks, Mirras' cells) gets its collision; so they show only
 * once the fight is won, whenever Kirby comes in (shut once it began, they showed in a long room before it came on screen).
 * A boss can make its screen sink (ArenaSink(): the Power Orb's, NiDL's fall through space), the camera with it; */
#include "../nidl.h"
#include "constants/object_types.h"

/* The camera's bounds (unk_S32Vec2_7C, unk_S32Vec2_84) are what it may show: the arena's area, the screen, so the camera stays put
 * (as KatAM's scroll locks, 15 x 10 cells); Kirby's (unkA0-unkAC) are that and 39 pixels more below (the room's own: 8, 8,
 * width - 8, height - 1), so he falls off its pits. Kirby stays 8 pixels inside its sides; */
#define BELOW 39
#define WALL 8
#define ARENA_REACH 4
#define ARENA_REACH_Y 2

struct Arena {
    struct NidlObject n;
    bool8 active;
    u8 locked;          // The players whose camera it holds (bit per player);
    struct S32Vec2 saved7C, saved84;
    s32 savedA0, savedA4, savedA8, savedAC;
    bool8 opened;       // It opened the doors (once);
    s32 sink;           // How far its screen has sunk (8.8, ArenaSink());
    s32 lockedTop;      // The top it last held the cameras at (a sinking boss moves `sink` after it, the frame it's beaten too);
};

// The camera's bounds: its template's area (Kirby's, BELOW more);
#define ARENA_LEFT(a) ((a->n.obj.objTemplate->x + a->n.obj.objTemplate->unk1A) << 8)
#define ARENA_TOP(a) (((a->n.obj.objTemplate->y + a->n.obj.objTemplate->unk1C) << 8) + a->sink)
#define ARENA_RIGHT(a) ((a->n.obj.objTemplate->x + a->n.obj.objTemplate->unk1A + a->n.obj.objTemplate->unk1E) << 8)
#define ARENA_BOTTOM(a) (((a->n.obj.objTemplate->y + a->n.obj.objTemplate->unk1C + a->n.obj.objTemplate->unk20) << 8) + a->sink)

typedef char ArenaFits[sizeof(struct Arena) <= NIDL_OBJECT_SIZE ? 1 : -1];

// OpenDoors(); The doors of the fight (ArenaCells()' box: those outside its cells weren't shut), for the room's player: their
// records' collision, where their cell has no other record (the special records, levelInfo->unk1E0: kind, collision, x, y, length);
static void OpenDoors(struct Arena *a) {
    const struct ObjectTemplate *area = a->n.obj.objTemplate;
    u8 player = a->n.obj.base.unk56;
    s32 left = ((area->x + area->unk1A) >> 4) - ARENA_REACH, top = ((area->y + area->unk1C) >> 4) - ARENA_REACH_Y;
    s32 right = ((area->x + area->unk1A + area->unk1E - 1) >> 4) + ARENA_REACH, bottom = ((area->y + area->unk1C + area->unk20 - 1) >> 4) + ARENA_REACH_Y;
    const u8 *record, *other;

    for (record = (const u8 *)gCurLevelInfo[player].unk1E0.pat1; record[4] != 0; record += record[4]) {
        if (record[0] != 3 || record[2] < left || record[2] > right || record[3] < top || record[3] > bottom) continue;
        for (other = (const u8 *)gCurLevelInfo[player].unk1E0.pat1; other[4] != 0; other += other[4])
            if (other != record && other[2] == record[2] && other[3] == record[3]) break;
        if (other[4] == 0 && GetCollisionTile(player, record[2], record[3]) != record[1]) sub_0800255C(player, record[2], record[3], record[1]);
    }
}

// MinibossAlive(); A mini-boss with HP left in the room, found walking the task list (objects are ObjectBase kind 1, InitObject);
static bool8 MinibossAlive(struct Object *arena) {
    struct Task *t = &gTasks[0];
    u8 n;

    for (n = 0; n < MAX_TASK_NUM && t->next; n++) {
        struct Object *other;

        t = (struct Task *)(IWRAM_START + t->next);
        other = TaskGetStructPtr(t);
        if (other->base.header.kind != 1 || !ObjType38To52(other)) continue;
        if (other->base.roomId == arena->base.roomId && !(other->base.flags & 0x1000) && other->unk80 > 0) return TRUE;
    }
    return FALSE;
}

// LockCamera(); sub_0811C7D0 without its flag 0x10 (which snaps the camera there), then sub_0811C8BC, the first time;
static void LockCamera(struct Arena *a, u8 i) {
    struct LevelInfo *l = &gCurLevelInfo[i];

    l->unk_S32Vec2_7C.x = l->unkA0 = ARENA_LEFT(a);
    l->unk_S32Vec2_7C.y = l->unkA4 = a->lockedTop = ARENA_TOP(a);
    l->unk_S32Vec2_84.x = l->unkA8 = ARENA_RIGHT(a);
    l->unk_S32Vec2_84.y = ARENA_BOTTOM(a);
    l->unkAC = ARENA_BOTTOM(a) + (BELOW << 8);
    if (a->locked & (1 << i)) return;
    a->locked |= 1 << i;
    l->unk1EC = 0;
    if (ARENA_RIGHT(a) - ARENA_LEFT(a) >= 0xF000) {
        if (l->unk_S32Vec2_6C.x <= l->viewportPosition.x - 0x800) l->unk_S32Vec2_6C.x = l->viewportPosition.x - 0x800;
        if (l->unk_S32Vec2_74.x >= l->viewportPosition.x + 0xF800) l->unk_S32Vec2_74.x = l->viewportPosition.x + 0xF800;
    }
    if (ARENA_BOTTOM(a) - ARENA_TOP(a) >= 0xA000) {
        if (l->unk_S32Vec2_6C.y <= l->viewportPosition.y - 0x800) l->unk_S32Vec2_6C.y = l->viewportPosition.y - 0x800;
        if (l->unk_S32Vec2_74.y >= l->viewportPosition.y + 0xA800) l->unk_S32Vec2_74.y = l->viewportPosition.y + 0xA800;
    }
    if (l->unk90 <= l->viewportPosition.x - 0x800) l->unk90 = l->viewportPosition.x - 0x800;
    if (l->unk94 <= l->viewportPosition.y - 0x800) l->unk94 = l->viewportPosition.y - 0x800;
    if (l->unk98 >= l->viewportPosition.x + 0xF800) l->unk98 = l->viewportPosition.x + 0xF800;
    if (l->unk9C >= l->viewportPosition.y + 0xF800) l->unk9C = l->viewportPosition.y + 0xF800;
}

// UnlockCameras(); ScrollLockDestroy: the bounds from before, for the players whose camera it still holds;
static void UnlockCameras(struct Arena *a) {
    u8 i;

    for (i = 0; i < gNumKirbys; i++) {
        struct LevelInfo *l = &gCurLevelInfo[i];

        if (!(a->locked & (1 << i)) || l->unk_S32Vec2_7C.x != ARENA_LEFT(a) || l->unk_S32Vec2_7C.y != a->lockedTop) continue;
        l->unk_S32Vec2_7C = l->unk_S32Vec2_6C = a->saved7C;
        l->unk_S32Vec2_84 = l->unk_S32Vec2_74 = a->saved84;
        l->unkA0 = l->unk90 = a->savedA0;
        l->unkA4 = l->unk94 = a->savedA4;
        l->unkA8 = l->unk98 = a->savedA8;
        l->unkAC = l->unk9C = a->savedAC;
        l->unk1EC = 0;
    }
    a->locked = 0;
}

static void ArenaMain(struct Object *obj) {
    struct Arena *a = (struct Arena *)obj;
    const struct ObjectTemplate *area = obj->objTemplate;
    s32 left, right;
    u8 i;

    obj->base.flags |= 0x200 | 0x400;
    obj->base.xspeed = obj->base.yspeed = 0;

    if (!MinibossAlive(obj)) {
        if (!a->opened) {
            a->opened = TRUE;
            OpenDoors(a);
        }
        if (a->active) {
            UnlockCameras(a);
            obj->base.flags |= 0x1000;
        }
        return;
    }
    if (!a->active) {
        struct LevelInfo *l = &gCurLevelInfo[obj->base.unk56];

        for (i = 0; i < 4; i++)
            if (gKirbys[i].base.roomId == obj->base.roomId && Macro_08039430_2(&gKirbys[i].base, obj)) a->active = TRUE;
        if (!a->active) return;
        a->saved7C = l->unk_S32Vec2_7C;
        a->saved84 = l->unk_S32Vec2_84;
        a->savedA0 = l->unkA0;
        a->savedA4 = l->unkA4;
        a->savedA8 = l->unkA8;
        a->savedAC = l->unkAC;
    }

    left = (area->x + area->unk1A + WALL) << 8;
    right = (area->x + area->unk1A + area->unk1E - WALL) << 8;
    for (i = 0; i < 4; i++) {
        struct Kirby *kirby = &gKirbys[i];

        if (kirby->base.roomId != obj->base.roomId) continue;
        if (i < gNumKirbys) LockCamera(a, kirby->base.unk56);
        if (kirby->base.x < left) {
            kirby->base.x = left;
            if (kirby->base.xspeed < 0) kirby->base.xspeed = 0;
        } else if (kirby->base.x > right) {
            kirby->base.x = right;
            if (kirby->base.xspeed > 0) kirby->base.xspeed = 0;
        }
    }
}

// ArenaSink(); The arena in a boss's room sinks its screen to `sink` (8.8) below where it was;
void ArenaSink(struct Object *boss, s32 sink) {
    struct Task *t = &gTasks[0];
    u8 n;

    for (n = 0; n < MAX_TASK_NUM && t->next; n++) {
        struct Object *other;

        t = (struct Task *)(IWRAM_START + t->next);
        other = TaskGetStructPtr(t);
        if (other->base.header.kind != 1 || other->type != NIDL_TYPE || other->base.roomId != boss->base.roomId) continue;
        if (((struct NidlObject *)other)->enemy == NIDL_ARENA) ((struct Arena *)other)->sink = sink;
    }
}

// ArenaStart(); Behavior start (also the engine's after a hit, which it never takes); never culled off the screen (sub_0803D6B4: 128
// pixels up or down, the Power Orb's screen above Kirby as he came in), as the bosses;
void ArenaStart(struct Object *obj) {
    ObjectSetFunc(obj, 0, ArenaMain);
    obj->kirbyAbility = 0;
    obj->base.flags |= 0x200 | 0x400 | 0x2000000;
}

// ArenaInit(); Where its template is;
void ArenaInit(struct NidlObject *n) {
    struct Arena *a = (struct Arena *)n;

    a->active = FALSE;
    a->locked = 0;
    a->opened = FALSE;
    a->sink = 0;
    a->lockedTop = 0;
}
