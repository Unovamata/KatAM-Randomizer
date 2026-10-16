/* Arena; NiDL's mini-boss fights keep Kirby on their screen: the camera stops scrolling there and the screen's edges are walls.
 * KatAM's arenas are rooms a screen big, so its mini-bosses do neither, and NiDL's are open. KatAMExpansionRooms.cs puts one of
 * these before every mini-boss in a NiDL room (behavior NIDL_ARENA << 4, the fight's screen as its template's area, as the
 * mini-boss's): unseen and untouchable, once a Kirby is on that screen while a mini-boss (types 0x38-0x52, KatAM's or ported) is
 * alive in the room, it holds the camera there as KatAM's scroll locks do (scroll_lock.c, type 0x77: sub_0811C7D0 and sub_0811C8BC,
 * which eases the camera in) and keeps every Kirby in the room on it, until none is left; then it gives the camera back
 * (ScrollLockDestroy) and goes away. Its graphics are Broom Hatter's (never shown); */
#include "../nidl.h"
#include "constants/object_types.h"

/* The camera's bounds are what it may show, and 39 pixels more below (the room's own: 8, 8, width - 8, height - 1, as it never
 * shows a room's bottom 40); the arena's area is the screen, so the camera stays put. Kirby stays 8 pixels inside it; */
#define BELOW 39
#define WALL 8

struct Arena {
    struct NidlObject n;
    bool8 active;
    u8 locked;          // The players whose camera it holds (bit per player);
    s32 left, top, right, bottom;
    struct S32Vec2 saved7C, saved84;
    s32 savedA0, savedA4, savedA8, savedAC;
};

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

    l->unk_S32Vec2_7C.x = l->unkA0 = a->left;
    l->unk_S32Vec2_7C.y = l->unkA4 = a->top;
    l->unk_S32Vec2_84.x = l->unkA8 = a->right;
    l->unk_S32Vec2_84.y = l->unkAC = a->bottom;
    if (a->locked & (1 << i)) return;
    a->locked |= 1 << i;
    l->unk1EC = 0;
    if (a->right - a->left >= 0xF000) {
        if (l->unk_S32Vec2_6C.x <= l->viewportPosition.x - 0x800) l->unk_S32Vec2_6C.x = l->viewportPosition.x - 0x800;
        if (l->unk_S32Vec2_74.x >= l->viewportPosition.x + 0xF800) l->unk_S32Vec2_74.x = l->viewportPosition.x + 0xF800;
    }
    if (a->bottom - a->top >= 0xA000) {
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

        if (!(a->locked & (1 << i)) || l->unk_S32Vec2_7C.x != a->left || l->unk_S32Vec2_7C.y != a->top) continue;
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
        a->left = (area->x + area->unk1A) << 8;
        a->top = (area->y + area->unk1C) << 8;
        a->right = (area->x + area->unk1A + area->unk1E) << 8;
        a->bottom = (area->y + area->unk1C + area->unk20 + BELOW) << 8;
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

// ArenaStart(); Behavior start (also the engine's after a hit, which it never takes);
void ArenaStart(struct Object *obj) {
    ObjectSetFunc(obj, 0, ArenaMain);
    obj->kirbyAbility = 0;
    obj->base.flags |= 0x200 | 0x400;
}

// ArenaInit(); Where its template is;
void ArenaInit(struct NidlObject *n) {
    struct Arena *a = (struct Arena *)n;

    a->active = FALSE;
    a->locked = 0;
}
