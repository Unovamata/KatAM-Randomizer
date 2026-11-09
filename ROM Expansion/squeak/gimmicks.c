/* squeak/gimmicks.c: Squeak Squad's room gimmicks (section 10 of a room's file, `yakumono`; "Squeak Squad Notes.md", "The gimmicks
 * talk to each other") in its rooms in KatAM (KatAMExpansionSqueakRooms.cs). On the DS a gimmick sends a message to others by their
 * index in the room's gimmick list (0x0221CE2C: the targets' indices, how many, the message's kind); each takes it or not by its
 * state (its vtable's slot 16) and acts on it (slot 17). Here:
 * • the messages are a byte per gimmick in each loaded room's slot (gCurLevelInfo[].unk65E: SqueakMailOf(), rooms.c), cleared
 *   when a room is loaded in it, the message's kind + 1 till its gimmick takes it: one sent to a gimmick not made yet (KatAM makes
 *   a room's objects as they come into view) waits for it (on the DS it's dropped: its gimmicks are made as the camera nears them);
 * • the gimmicks with no look (a trigger area 0x15, a delay relay 0x16, a switch block group 0x10) are run by a task of the room's
 *   (SqueakLogicMain(), made with the room: SqueakGimmicksLoaded()), from its list (gSqueakGimmickRooms, KatAMExpansionSqueakRooms.cs);
 * • the ones with a look are KatAM objects on the hammer peg's type (0x8D: its spawn is SqueakCreateGimmick, KatAMExpansionChests.cs'
 *   HookSqueakSwitches(); KatAM's own pegs made by the game's, gSqueakSwitch.create), told apart by their template's unk11: the small
 *   star switch (gimmick 0) and the shutters (gimmick 2); */
#include "global.h"
#include "object.h"
#include "kirby.h"
#include "functions.h"
#include "task.h"
#include "data.h"
#include "level.h"
#include "gba/macro.h"
#include "constants/object_types.h"
#include "malloc_vram.h"
#include "constants/songs.h"
#include "code_0800E0E4.h"
#include "random.h"
#include "../squeak.h"

/* The rooms' gimmick lists (KatAMExpansionSqueakRooms.cs, SqueakGimmickLists()): per Squeak Squad room (from SQUEAK_FIRST_ROOM) its
 * list or NULL (no gimmick a task runs); a list is a count, then 24 bytes a gimmick: its type, its index in the room's list, its
 * spot (x, y: pixels, y down) and its parameters (the DS's table entry); */
struct SqueakGimmickEntry {
    u8 type;
    u8 index;
    u16 x;
    u16 y;
    u8 par[18];
};

struct SqueakGimmickList {
    u8 count;
    u8 unused[3];
    struct SqueakGimmickEntry entry[0];
};

struct SqueakGimmickRooms {
    const struct SqueakGimmickList *const *list;
    u32 count;
};

const struct SqueakGimmickRooms gSqueakGimmickRooms = { NULL, 0 };

// The hammer peg's spawn (KatAM's own pegs): the game's CreateHammerPeg, kept by KatAMExpansionChests.cs;
struct SqueakSwitch {
    void *(*create)(struct ObjectTemplate *, u8);
};

const struct SqueakSwitch gSqueakSwitch = { NULL };

#define GIMMICK_TRIGGER 0x15
#define GIMMICK_RELAY 0x16
#define GIMMICK_RANDOM_RELAY 0x17
#define RANDOM_RELAY_TARGETS 8
#define GIMMICK_BLOCKS 0x10
#define RELAY_TARGETS 5
// (a message's kinds: a switch's 0, a trigger area's 2; a relay with its parameter's bit 7 takes kind 4 only, sending 2);
#define MESSAGE_SWITCH 0
#define MESSAGE_AREA 2
#define MESSAGE_EVENT 4

// SqueakSendMail(); A message of `kind` for gimmick `index` of the room in `slot` (it waits till that gimmick takes it);
void SqueakSendMail(u8 slot, u8 index, u8 kind) {
    if (index < SQUEAK_MAIL_GIMMICKS) SqueakMailOf(slot)->box[index] = kind + 1;
}

// TakeMail(); The message waiting for gimmick `index` (its kind + 1), taken; 0 none;
static u8 TakeMail(u8 slot, u8 index) {
    struct SqueakMail *mail = SqueakMailOf(slot);
    u8 kind;

    if (index >= SQUEAK_MAIL_GIMMICKS) return 0;
    kind = mail->box[index];
    mail->box[index] = 0;
    return kind;
}

// SlotOf(); The room slot of the room an object is in (its player's level info's: KatAM's own objects look it up so);
#define SlotOf(base) (gCurLevelInfo[(base)->unk56].unk65E)

/* The room's task: the trigger areas (0x15), relays (0x16) and switch block groups (0x10). Its ObjectBase is what KatAM's chain
 * breaker (sub_0800E0E4) reads of the object breaking a chain (its room, its player's level info) and whose spot it starts from; */
struct SqueakLogic {
    struct ObjectBase base;
    const struct SqueakGimmickList *list;
    u16 room;
    u8 slot;
    u8 generation;
    u8 inside[SQUEAK_MAIL_GIMMICKS];            // A trigger area: a body in it last frame;
    u8 relayKind[SQUEAK_MAIL_GIMMICKS];         // A relay: the kind it sends + 1 while it runs;
    s16 relayTimer[SQUEAK_MAIL_GIMMICKS][RELAY_TARGETS];
};

/* Inside(); A Kirby in the room touching a trigger area: its box from its spot, its parameter's low 7 bits blocks right, the next 7
 * down (0x0221E5A4: x 0 to w, y -h to 0, blocks of 16 pixels, in the DS's world, whose y goes up), against his own bounds
 * (unk3C..unk3F, pixels from his spot); */
static bool32 Inside(const struct SqueakLogic *l, const struct SqueakGimmickEntry *e) {
    u16 p = e->par[0] | e->par[1] << 8;
    s32 x0 = e->x, x1 = e->x + (p & 0x7F) * 16, y0 = e->y, y1 = e->y + (p >> 7 & 0x7F) * 16;
    u8 i;

    for (i = 0; i < gNumKirbys; i++) {
        struct Kirby *k = &gKirbys[i];
        s32 kx = k->base.x >> 8, ky = k->base.y >> 8;

        if (k->base.roomId != l->room) continue;
        if (kx + k->base.unk3E > x0 && kx + k->base.unk3C < x1 && ky + k->base.unk3F > y0 && ky + k->base.unk3D < y1) return TRUE;
    }
    return FALSE;
}

/* Trigger(); A trigger area (0x0221E544, its states 0x0221E520 and 0x0221E4EC): a body moving into it sends its targets (its
 * parameter's bits 14-15 how many, its bytes 2-4 their indices) a message of kind 2 once; again only after it's been left; */
static void Trigger(struct SqueakLogic *l, const struct SqueakGimmickEntry *e) {
    bool32 inside = Inside(l, e);
    u8 count = e->par[1] >> 6, t;

    if (inside && !l->inside[e->index])
        for (t = 0; t < count && t < 3; t++) SqueakSendMail(l->slot, e->par[2 + t], MESSAGE_AREA);
    l->inside[e->index] = inside;
}

/* Relay(); A delay relay (0x02222A50: its parameter's first byte's low 7 bits how many targets, the next 5 bytes their indices, then
 * a u16 each, the frames before its message): a message (kinds 0-2; with the first byte's bit 7 only kind 4, sent on as 2) while it
 * isn't running starts it (0x022229B0, 0x02222930): each target's wait, a target with none sent the message at once; then a frame
 * at a time (0x022228C0) each sent its message as its wait runs out, till all are; */
static void Relay(struct SqueakLogic *l, const struct SqueakGimmickEntry *e) {
    u8 count = e->par[0] & 0x7F, t, kind;
    s16 *timer = l->relayTimer[e->index];
    bool32 running = FALSE;

    if (count > RELAY_TARGETS) count = RELAY_TARGETS;
    if (l->relayKind[e->index] == 0) {
        kind = TakeMail(l->slot, e->index);
        if (kind == 0) return;
        kind--;
        if ((e->par[0] & 0x80) ? kind != MESSAGE_EVENT : kind > MESSAGE_AREA) return;
        if (kind == MESSAGE_EVENT) kind = MESSAGE_AREA;
        l->relayKind[e->index] = kind + 1;
        for (t = 0; t < count; t++) {
            timer[t] = (s16)(e->par[6 + t * 2] | e->par[7 + t * 2] << 8);
            if (timer[t] <= 0) {
                timer[t] = 0;
                SqueakSendMail(l->slot, e->par[1 + t], kind);
            }
        }
        return;
    }
    kind = l->relayKind[e->index] - 1;
    for (t = 0; t < count; t++) {
        if (timer[t] <= 0) continue;
        if (--timer[t] <= 0) SqueakSendMail(l->slot, e->par[1 + t], kind);
        else running = TRUE;
    }
    if (!running) l->relayKind[e->index] = 0;
}

/* RandomRelay(); A random relay (0x17: ctor 0x02224384, vtable 0x02229948; its parameter's u16s the least and most frames it waits,
 * then up to 8 targets' indices, 0xFF the end): it waits a random number of frames between the two (state 1, 0x022241A4), sends its
 * targets its message, and waits again; a message (kinds 0-2, 4 kept as 2: 0x022242CC) sets the kind it sends. 2-5-5's and 5-3-4's
 * rain their rocks so. Its wait in relayTimer[i][0], the kind it sends + 1 in relayKind[i] (none yet: a trigger area's kind, 2); */
static void RandomRelay(struct SqueakLogic *l, const struct SqueakGimmickEntry *e) {
    s16 *wait = &l->relayTimer[e->index][0];
    u16 least = e->par[0] | e->par[1] << 8, most = e->par[2] | e->par[3] << 8;
    u8 kind = TakeMail(l->slot, e->index), t;

    if (kind != 0 && kind - 1 <= MESSAGE_EVENT) l->relayKind[e->index] = (kind - 1 == MESSAGE_EVENT ? MESSAGE_AREA : kind - 1) + 1;
    if (*wait > 1) {
        --*wait;
        return;
    }
    // (its wait over: its targets sent its message; on its first frame, 0, only its wait set);
    if (*wait == 1) {
        kind = l->relayKind[e->index] ? l->relayKind[e->index] - 1 : MESSAGE_AREA;
        for (t = 0; t < RANDOM_RELAY_TARGETS && e->par[4 + t] != 0xFF; t++) SqueakSendMail(l->slot, e->par[4 + t], kind);
    }
    *wait = least + (most >= least ? Rand16() % (most - least + 1) : 0);
    if (*wait < 1) *wait = 1;
}

/* Blocks(); A switch block group (0x02223900): a message clears its blocks. In KatAM they're a chain (KatAMExpansionSqueakRooms.cs:
 * 0xF2 blocks, 0xF3 links), broken from the group's spot as KatAM's lever walls break theirs: its cell's special block record
 * (sub_08001408), then KatAM's chain breaker from there (sub_0800E0E4); */
static void Blocks(struct SqueakLogic *l, const struct SqueakGimmickEntry *e) {
    u8 x = e->x >> 4, y = e->y >> 4;

    if (TakeMail(l->slot, e->index) == 0) return;
    l->base.x = e->x << 8;
    l->base.y = e->y << 8;
    sub_08001408(l->base.unk56, sub_080025AC(l->base.unk56, x, y), NULL, NULL);
    sub_0800E0E4((struct Object *)&l->base, x, y);
}

static void SqueakLogicMain(void) {
    struct SqueakLogic *l = TaskGetStructPtr(gCurTask);
    struct SqueakMail *mail = SqueakMailOf(l->slot);
    u8 p, i;

    // (gone once its slot holds another room, or this one loaded again, or no Kirby's in it);
    if (mail->room != l->room || mail->generation != l->generation) {
        TaskDestroy(gCurTask);
        return;
    }
    for (p = 0; p < gNumKirbys; p++)
        if (gCurLevelInfo[p].currentRoom == l->room) break;
    if (p == gNumKirbys) {
        TaskDestroy(gCurTask);
        return;
    }
    // (frozen as the room's objects are: the pause screen, a cutscene; Macro_0810B1F4);
    if (gUnk_03000510.unk4 & ((1 << p) | 0x10)) return;
    l->base.unk56 = p;
    for (i = 0; i < l->list->count; i++) {
        const struct SqueakGimmickEntry *e = &l->list->entry[i];

        if (e->index >= SQUEAK_MAIL_GIMMICKS) continue;
        switch (e->type) {
        case GIMMICK_TRIGGER: Trigger(l, e); break;
        case GIMMICK_RELAY: Relay(l, e); break;
        case GIMMICK_RANDOM_RELAY: RandomRelay(l, e); break;
        case GIMMICK_BLOCKS: Blocks(l, e); break;
        }
    }
}

/* SqueakGimmicksLoaded(); A room just loaded in `slot` by `playerId` (rooms.c's BigFillLevelInfo, the room not loaded in another
 * player's slot): its messages cleared, and a Squeak Squad room with gimmicks a task runs gets its task; */
void SqueakGimmicksLoaded(u8 playerId, u16 roomId, u8 slot) {
    struct SqueakMail *mail = SqueakMailOf(slot);
    const struct SqueakGimmickList *list;
    struct SqueakLogic *l;
    struct Task *task;
    u8 i;

    mail->room = roomId;
    mail->generation++;
    for (i = 0; i < SQUEAK_MAIL_GIMMICKS; i++) mail->box[i] = 0;
    if (roomId < SQUEAK_FIRST_ROOM || roomId - SQUEAK_FIRST_ROOM >= gSqueakGimmickRooms.count) return;
    list = gSqueakGimmickRooms.list[roomId - SQUEAK_FIRST_ROOM];
    if (list == NULL) return;
    task = TaskCreate(SqueakLogicMain, sizeof(struct SqueakLogic), 0x1000, TASK_USE_EWRAM, NULL);
    l = TaskGetStructPtr(task);
    CpuFill16(0, l, sizeof(struct SqueakLogic));
    l->list = list;
    l->room = roomId;
    l->slot = slot;
    l->generation = mail->generation;
    l->base.roomId = roomId;
    l->base.unk56 = playerId;
}

/* The small star switch (gimmick 0): on the DS a body moving through it or an attack presses it (its state 0's messages, 0x02225458),
 * and it sends its targets a message of kind 0 (0x022252F0: its parameter's first byte's high nibble how many, then their indices).
 * Here its template's unk14 and unk16 are those 4 bytes; a Kirby touching it presses it, and it stays down (the DS's rises again
 * once its targets are done, and sends again when pressed again: KatAM's stays down till Kirby comes back to the room, as the
 * shutters it opened stay open then). Drawn with Squeak Squad's switch_small (SQUEAK_SWITCH: its up frame, variant 0, then its
 * pressed one, 1: squeak.c's SqueakAnimFor()); */
#define SQUEAK_SWITCH_ARMED 30

static void SqueakSwitchDown(struct Object *obj) {
}

static void SqueakSwitchUp(struct Object *obj) {
    u8 par[4], i;

    // (armed SQUEAK_SWITCH_ARMED frames after it's made: as the room comes up the Kirbys pass where it stands, 3-4-8's pressing then);
    if (obj->unk85 < SQUEAK_SWITCH_ARMED) {
        obj->unk85++;
        return;
    }
    for (i = 0; i < gNumKirbys; i++) {
        s32 dx = gKirbys[i].base.x - obj->base.x, dy = gKirbys[i].base.y - obj->base.y;
        // (positions in 1/256 pixels: a Kirby within 12 pixels of it sideways, his middle up to 24 pixels over its spot or 8 under);
        if (gKirbys[i].base.roomId != obj->base.roomId || dx <= -0xC00 || dx >= 0xC00 || dy <= -0x1800 || dy >= 0x800) continue;
        par[0] = obj->objTemplate->unk14;
        par[1] = obj->objTemplate->unk14 >> 8;
        par[2] = obj->objTemplate->unk16;
        par[3] = obj->objTemplate->unk16 >> 8;
        for (i = 0; i < par[0] >> 4 && i < 3; i++) SqueakSendMail(SlotOf(&obj->base), par[1 + i], MESSAGE_SWITCH);
        PlaySfx(&obj->base, SE_SWITCH_ACTIVATE);
        obj->unk83 = 1;
        obj->unk78 = SqueakSwitchDown;
        return;
    }
}

static void *SqueakCreateButton(struct ObjectTemplate *template, u8 playerId) {
    struct Task *task = TaskCreate(ObjectMain, sizeof(struct Object), 0x1000, TASK_USE_IWRAM, ObjectDestroy);
    struct Object *obj = TaskGetStructPtr(task);

    InitObject(obj, template, playerId);
    obj->base.flags |= 0x18B00;
    obj->base.unk68 &= ~7;
    ObjectSetBounds(&obj->base, -8, -8, 8, 8);
    obj->unk83 = 0;
    obj->unk85 = 0;   // (frames since it was made: the hammer peg's own count, which the game's object code leaves alone);
    ObjectInitSprite(obj);
    obj->base.sprite.palId = SqueakLoadPalette(gSqueakData.enemies[SQUEAK_SWITCH].animId, SQUEAK_HURT_VARIANTS, obj->base.roomId);
    obj->unk78 = SqueakSwitchUp;
    return obj;
}

/* The shutters (gimmick 2, 0x02224EFC): its parameter's low 3 bits its look (0x02227EA4: 0-2 a stone pillar, upright, turned 90 and
 * 270 degrees; 3-6 spikes, at 0, 180, 270 and 90), bit 3 whether it starts open, the rest the frames it stays changed before it
 * changes back (0: it stays). Its box (the ctor's cases, blocks of 16 pixels around its spot): the pillar 1 by 2, turned 2 by 1;
 * the spikes 2 by 1 and a pixel. Its states (vtable 0x02229B24's slots 3-8): 0 waiting; a message (kinds 0 or 2, in states 0 and
 * 4: 0x02224EA4) starts 1: its change's animation (0x02227E7E: shut, 1 opening; open, 3 shutting) and sound; 2 when it's over, the
 * other held (0x02227E80), its wait set, 3 for good (no wait) or 4 counting it down (a message then sets it again: 0x02224E70); at 0
 * the change back (0x02227E82) and 5, back to 0 when it's over. The pillar shut is a wall (checked on the DS: 1-5-1's stops Kirby
 * at its side); the spikes up hurt (checked: 3-6-6's knock Kirby back). Here its cells are KatAM's solid or spike blocks while it's shut
 * (ShutterSetBox()); drawn with SQUEAK_SHUTTER's looks (the DS's animations, its cells as the variants; facing right,
 * as the DS draws them); the DS's quake as a stone shutter moves is left out (the owner: the screen shakes too much); */
/* A gimmick's look (the shutters', the fire columns'): its graphics (SQUEAK_SHUTTER...), the DS animation playing, its step, the
 * frames left on it, whether it's over (a non-looping one holds its last step); right after its Object, as SqueakGimmickAnim()
 * reads it; */
struct SqueakGimmickSprite {
    u8 look;
    u8 anim;
    u8 step;
    u8 frames;
    bool8 ended;
};

struct SqueakGimmickObject {
    struct Object obj;
    struct SqueakGimmickSprite spr;
};

static void GimmickPlay(struct Object *obj, struct SqueakGimmickSprite *spr, u8 anim) {
    const struct SqueakAnim *a = &gSqueakData.enemies[spr->look].anim[anim];

    spr->anim = anim;
    spr->step = 0;
    spr->frames = a->step[0].frames;
    spr->ended = a->steps <= 1 && !a->loops;
    obj->unk83 = SQUEAK_HURT_VARIANTS + a->step[0].cell;
}

static void GimmickAnimate(struct Object *obj, struct SqueakGimmickSprite *spr) {
    const struct SqueakAnim *a = &gSqueakData.enemies[spr->look].anim[spr->anim];

    if (spr->frames > 1) {
        spr->frames--;
        return;
    }
    if (spr->step + 1 < a->steps) spr->step++;
    else if (a->loops) spr->step = 0;
    else {
        spr->ended = TRUE;
        return;
    }
    spr->frames = a->step[spr->step].frames;
    obj->unk83 = SQUEAK_HURT_VARIANTS + a->step[spr->step].cell;
}

/* GimmickInit(); An object a gimmick on the hammer peg's type: still, staying when off the screen, harmless, facing right (Squeak
 * Squad's look as the DS draws it, its frames' canvas mirrored), its sprite as many tiles as its look's frames take and its look's
 * palette; */
static void GimmickInit(struct Object *obj, struct SqueakGimmickSprite *spr, u8 look) {
    spr->look = look;
    obj->base.flags |= 0x2000000 | 0x8000 | 0x800 | 0x100 | 0x40 | 1;
    obj->base.unkC |= 1;
    obj->base.unk68 &= ~7;
    obj->base.unk5C &= ~7;
    ObjectInitSprite(obj);
    if (SqueakVramOk(obj->base.sprite.tilesVram)) VramFree(obj->base.sprite.tilesVram);
    obj->base.sprite.tilesVram = VramMalloc(gSqueakData.enemies[look].tiles);
    obj->base.sprite.palId = SqueakLoadPalette(gSqueakData.enemies[look].animId, SQUEAK_HURT_VARIANTS, obj->base.roomId);
}

struct SqueakShutter {
    struct Object obj;
    struct SqueakGimmickSprite spr;
    u8 state;
    u8 index;
    u16 wait;
    u16 par;
    bool8 solid;                // (a pillar: its cells solid now, their own values kept);
    u8 kept[4];
};

enum { SHUTTER_WAIT, SHUTTER_CHANGE, SHUTTER_CHANGING, SHUTTER_HELD, SHUTTER_COUNT, SHUTTER_BACK };
#define ANIM_SHUT 0
#define ANIM_OPEN 2
// (the DS's 0x02227E7C: per its bit 3 the animations it starts with, changes with, holds, changes back with);
static const u8 sShutterAnims[8] = { 0, 2, 1, 3, 2, 0, 3, 1 };
static const u8 sShutterLooks[7] = { SQUEAK_SHUTTER, SQUEAK_SHUTTER_90, SQUEAK_SHUTTER_270, SQUEAK_NEEDLE, SQUEAK_NEEDLE_180,
                                     SQUEAK_NEEDLE, SQUEAK_NEEDLE };
// (per look its box, pixels around its spot);
static const s8 sShutterBoxes[7][4] = { { -8, -16, 8, 16 }, { -16, -8, 16, 8 }, { -16, -8, 16, 8 }, { -17, -8, 16, 8 },
                                        { -16, -8, 17, 8 }, { -8, -16, 8, 17 }, { -8, -17, 8, 16 } };

#define IsSpikes(s) (((s)->par & 7) >= 3)
#define SQUEAK_SOLID 0x0D   // (KatAM's solid block and spike block: KatAMExpansionSqueakRooms.cs's KatAMSolid, KatAMSpikes);
#define SQUEAK_SPIKES 0x1C

/* ShutterBlocks(); Whether it blocks or hurts now: shut (the pillar, the spikes up), and changing: a pillar from the start of its
 * shutting to the end of its opening, the spikes till they're half down, and from half up as they rise; */
static bool32 ShutterBlocks(const struct SqueakShutter *s) {
    const struct SqueakAnim *a = &gSqueakData.enemies[s->spr.look].anim[s->spr.anim];

    switch (s->spr.anim) {
    case ANIM_SHUT: return TRUE;
    case ANIM_OPEN: return FALSE;
    case 1: return IsSpikes(s) ? s->spr.step < a->steps / 2 : !s->spr.ended;
    default: return IsSpikes(s) ? s->spr.step >= a->steps / 2 : TRUE;
    }
}

/* ShutterSetBox(); While it's shut its cells (the ones its box covers, a pixel in from each side: 1 by 2 or 2 by 1, a pillar between
 * two columns 2 by 2) are a wall in the room's collision, KatAM's solid block (0x0D) for the pillar, its spike block (0x1C: it hurts
 * and knocks Kirby back, as the DS's spikes do) for the spikes; their own values back once it's open. (KatAM's solid objects, its
 * sliding door's flags and box, didn't stop Kirby at a pillar, and Gordo's harm didn't hurt him on the spikes, the hammer peg's type
 * not being an enemy's: tried in 1-5-1 and 3-6-6, he walked through both); */
static void ShutterSetBox(struct SqueakShutter *s) {
    struct ObjectBase *base = &s->obj.base;
    bool32 blocks = ShutterBlocks(s);
    const s8 *box = sShutterBoxes[s->par & 7];
    s32 x0, x1, y0, y1, x, y;
    u8 n = 0;

    if (blocks == s->solid) return;
    x0 = ((base->x >> 8) + box[0] + 1) >> 4;
    x1 = ((base->x >> 8) + box[2] - 2) >> 4;
    y0 = ((base->y >> 8) + box[1] + 1) >> 4;
    y1 = ((base->y >> 8) + box[3] - 2) >> 4;
    for (y = y0; y <= y1; y++)
        for (x = x0; x <= x1 && n < 4; x++, n++) {
            if (x < 0 || y < 0) continue;
            if (blocks) s->kept[n] = sub_0800255C(base->unk56, x, y, IsSpikes(s) ? SQUEAK_SPIKES : SQUEAK_SOLID);
            else sub_0800255C(base->unk56, x, y, s->kept[n]);
        }
    s->solid = blocks;
}

static void ShutterSound(struct SqueakShutter *s) {
    PlaySfx(&s->obj.base, SE_VERT_SLIDING_DOOR_OPEN_CLOSE);
}

static void SqueakShutterMain(struct Object *obj) {
    struct SqueakShutter *s = (struct SqueakShutter *)obj;
    u8 bit3 = s->par >> 3 & 1, mail;

    GimmickAnimate(obj, &s->spr);
    // (a message taken in any state: one it doesn't take, changing, is dropped, as on the DS);
    mail = TakeMail(SlotOf(&obj->base), s->index);
    switch (s->state) {
    case SHUTTER_WAIT:
        if (mail == MESSAGE_SWITCH + 1 || mail == MESSAGE_AREA + 1) s->state = SHUTTER_CHANGE;
        break;
    case SHUTTER_CHANGE:
        GimmickPlay(obj, &s->spr, sShutterAnims[2 + bit3]);
        ShutterSound(s);
        s->state = SHUTTER_CHANGING;
        break;
    case SHUTTER_CHANGING:
        if (!s->spr.ended) break;
        s->wait = s->par >> 4;
        GimmickPlay(obj, &s->spr, sShutterAnims[4 + bit3]);
        s->state = s->wait == 0 ? SHUTTER_HELD : SHUTTER_COUNT;
        break;
    case SHUTTER_HELD:
        break;
    case SHUTTER_COUNT:
        if (mail == MESSAGE_SWITCH + 1 || mail == MESSAGE_AREA + 1) s->wait = s->par >> 4;
        else if (s->wait > 0) s->wait--;
        if (s->wait > 0) break;
        GimmickPlay(obj, &s->spr, sShutterAnims[6 + bit3]);
        ShutterSound(s);
        s->state = SHUTTER_BACK;
        break;
    case SHUTTER_BACK:
        if (!s->spr.ended) break;
        GimmickPlay(obj, &s->spr, sShutterAnims[bit3]);
        s->state = SHUTTER_WAIT;
        break;
    }
    ShutterSetBox(s);
}

static void *SqueakCreateShutter(struct ObjectTemplate *template, u8 playerId) {
    struct Task *task = TaskCreate(ObjectMain, sizeof(struct SqueakShutter), 0x1000, TASK_USE_EWRAM, ObjectDestroy);
    struct SqueakShutter *s = TaskGetStructPtr(task);
    struct Object *obj = &s->obj;
    u8 look;

    InitObject(obj, template, playerId);
    s->index = template->unk12;
    s->par = template->unk14;
    look = s->par & 7;
    s->state = SHUTTER_WAIT;
    s->solid = FALSE;
    GimmickInit(obj, &s->spr, sShutterLooks[look]);
    ObjectSetBounds(&obj->base, sShutterBoxes[look][0], sShutterBoxes[look][1], sShutterBoxes[look][2], sShutterBoxes[look][3]);
    GimmickPlay(obj, &s->spr, sShutterAnims[s->par >> 3 & 1]);
    ShutterSetBox(s);
    obj->unk78 = SqueakShutterMain;
    return obj;
}

/* The fire columns (gimmick 7, 0x02220094, vtable 0x022290EC; yakumono/fire_long: animations 0 its pilot flame, 1 flaring up, 2 the
 * column, 3 dying down): its parameter's first u16: its look's turn (0x02227B30: 0 up, 1 at 180 degrees, 2 at 270, 3 at 90) in
 * bits 0-3, the frames it burns in bits 4-15 (0: for good); its second: its mode in bits 0-3 (0 waiting out, 1 waiting lit: a
 * message, kinds 0 or 2, lights it or puts it out, 0x0221FF14; 2 burning by itself), the frames it waits out in bits 4-15 (0: for
 * good). Its states (slots 3-7): 0 waiting for a message; 1 out (0x0221FE88: its wait, then flaring up); 2 flaring (0x0221FE24:
 * then the column, sound 0x69); 3 lit (0x0221FDC4: its time, then dying down); 4 dying (0x0221FD68: then its pilot flame, and by
 * its mode, 0x02227B08, 0 waiting or 1 out). Its box (the constructor): a column 1 by 4 blocks around its spot (turned, 4 by 1).
 * In KatAM it hurts as KatAM's flame column does (object 0x7C: its unk68, 3 and 0x100) while it burns, its sound KatAM's enemies'
 * fire attack; */
struct SqueakFire {
    struct Object obj;
    struct SqueakGimmickSprite spr;
    u8 state;
    u8 index;
    u16 count;
    u16 burn;
    u16 out;
    u8 mode;
    u8 unused;
};

enum { FIRE_WAIT, FIRE_OUT, FIRE_FLARE, FIRE_LIT, FIRE_DYING };
#define ANIM_PILOT 0
#define ANIM_FLARE 1
#define ANIM_COLUMN 2
#define ANIM_DIE 3
static const u8 sFireLooks[4] = { SQUEAK_FIRE, SQUEAK_FIRE_180, SQUEAK_FIRE_270, SQUEAK_FIRE_90 };
static const s8 sFireBoxes[4][4] = { { -8, -32, 8, 32 }, { -8, -32, 8, 32 }, { -32, -8, 32, 8 }, { -32, -8, 32, 8 } };
static const u8 sFireAfterDying[4] = { FIRE_WAIT, FIRE_OUT, FIRE_OUT, FIRE_LIT };

// FireBurns(); Whether it hurts now: the column, the end of its flaring, the start of its dying;
static bool32 FireBurns(const struct SqueakFire *f) {
    switch (f->spr.anim) {
    case ANIM_COLUMN: return TRUE;
    case ANIM_FLARE: return f->spr.step >= 1;
    case ANIM_DIE: return f->spr.step == 0;
    }
    return FALSE;
}

static void SqueakFireMain(struct Object *obj) {
    struct SqueakFire *f = (struct SqueakFire *)obj;
    u8 mail;

    GimmickAnimate(obj, &f->spr);
    mail = TakeMail(SlotOf(&obj->base), f->index);
    switch (f->state) {
    case FIRE_WAIT:
        if (mail != MESSAGE_SWITCH + 1 && mail != MESSAGE_AREA + 1) break;
        f->count = 0;
        if (f->mode == 0) {
            GimmickPlay(obj, &f->spr, ANIM_FLARE);
            f->state = FIRE_FLARE;
        } else if (f->mode == 1) {
            GimmickPlay(obj, &f->spr, ANIM_DIE);
            f->state = FIRE_DYING;
        }
        break;
    case FIRE_OUT:
        if (f->out == 0 || ++f->count < f->out) break;
        f->count = 0;
        GimmickPlay(obj, &f->spr, ANIM_FLARE);
        f->state = FIRE_FLARE;
        break;
    case FIRE_FLARE:
        if (!f->spr.ended) break;
        GimmickPlay(obj, &f->spr, ANIM_COLUMN);
        PlaySfx(&obj->base, SE_BASIC_ENEMY_FIRE_ATTACK);
        f->state = FIRE_LIT;
        break;
    case FIRE_LIT:
        if (f->burn == 0 || ++f->count < f->burn) break;
        f->count = 0;
        GimmickPlay(obj, &f->spr, ANIM_DIE);
        f->state = FIRE_DYING;
        break;
    case FIRE_DYING:
        if (!f->spr.ended) break;
        GimmickPlay(obj, &f->spr, ANIM_PILOT);
        f->state = sFireAfterDying[f->mode & 3];
        break;
    }
    obj->base.unk68 &= ~(7 | 0x100);
    if (FireBurns(f)) obj->base.unk68 |= 3 | 0x100;
}

static void *SqueakCreateFire(struct ObjectTemplate *template, u8 playerId) {
    struct Task *task = TaskCreate(ObjectMain, sizeof(struct SqueakFire), 0x1000, TASK_USE_EWRAM, ObjectDestroy);
    struct SqueakFire *f = TaskGetStructPtr(task);
    struct Object *obj = &f->obj;
    u8 look = template->unk14 & 3;

    InitObject(obj, template, playerId);
    f->index = template->unk12;
    f->burn = template->unk14 >> 4;
    f->out = template->unk16 >> 4;
    f->mode = template->unk16 & 15;
    f->count = 0;
    GimmickInit(obj, &f->spr, sFireLooks[look]);
    obj->base.unk5C |= 0x20;
    ObjectSetBounds(&obj->base, sFireBoxes[look][0], sFireBoxes[look][1], sFireBoxes[look][2], sFireBoxes[look][3]);
    sub_0803E2B0(&obj->base, sFireBoxes[look][0], sFireBoxes[look][1], sFireBoxes[look][2], sFireBoxes[look][3]);
    // (its start, 0x02227AF8 and 0x02227AFC by its mode: 0 out waiting, 1 lit waiting, 2 out burning by itself);
    if (f->mode == 1) {
        f->state = FIRE_WAIT;
        GimmickPlay(obj, &f->spr, ANIM_COLUMN);
    } else {
        f->state = f->mode == 2 ? FIRE_OUT : FIRE_WAIT;
        GimmickPlay(obj, &f->spr, ANIM_PILOT);
    }
    obj->unk78 = SqueakFireMain;
    return obj;
}

/* The item holders: the grass tufts (gimmick 0xE: ctor 0x02222078, vtable 0x02229500) and barrels (0xF: 0x0222277C, 0x02229594).
 * What one hides (a record of its room's item or enemy list, the DS letting it out as it's cut or burst: the converter's
 * SqueakHeld(), which leaves it out of the room's objects) is its template's unk14: the KatAM object's type, its behavior byte
 * over it (an ability KatAM's ability star, object 0xA3, its subtype the ability: inhaled, it gives it), unk16 its variant (a
 * Squeak Squad character's unk12). An attack is told by KatAM's collision pass: an attack reaching an object marks it (flags
 * 0x40000, the attacker in unk6C: code_08032E98's pairs, 0x080368B2) before asking whether it hurts it, which an object with
 * flags 0x8000 never is; these are KatAM's enemies' kind (unk5C's low bits 1) for the attacks to reach them, harmless (unk68 0),
 * and deaf to a Kirby's body (its unk68's 0x80, which a touch defeating a weak enemy is: unk5C's 0x80, as a beaten enemy's,
 * masks it out); */
#define HOLDER_KIND (1 | 0x80)
#define HOLDER_HIT 0x40000

/* (KatAM marks a Kirby's touch so too: a hit counts from an attack's object, or from a Kirby whose unk68 shows an attack besides
 * his body's 0x80, his strength the low bits: a slide, a dash); */
#define KIRBY_BODY_ATTACK (7 | 0x80)
static bool32 Struck(struct Object *obj) {
    struct ObjectBase *by = obj->base.unk6C;

    if (!(obj->base.flags & HOLDER_HIT)) return FALSE;
    obj->base.flags &= ~HOLDER_HIT;
    if (by == NULL) return FALSE;
    if ((u32)by - (u32)gKirbys < 4 * sizeof(struct Kirby)) return (by->unk68 & ~KIRBY_BODY_ATTACK) != 0;
    return TRUE;
}

// (a Kirby in the room whose middle is in its box, given in pixels from its spot, and, for `moving`, moving);
static bool32 KirbyIn(struct Object *obj, s16 x0, s16 y0, s16 x1, s16 y1, bool32 moving) {
    u8 i;

    for (i = 0; i < gNumKirbys; i++) {
        struct Kirby *k = &gKirbys[i];
        s32 dx = (k->base.x - obj->base.x) >> 8, dy = (k->base.y - obj->base.y) >> 8;

        if (k->base.roomId != obj->base.roomId || dx < x0 || dx > x1 || dy < y0 || dy > y1) continue;
        if (!moving || k->base.xspeed != 0 || k->base.yspeed != 0) return TRUE;
    }
    return FALSE;
}

static void LetOut(struct Object *obj) {
    s32 x = obj->base.x >> 8, y = obj->base.y >> 8;
    u8 type = obj->objTemplate->unk14, behavior = obj->objTemplate->unk14 >> 8;
    struct Object *out;

    if (!type) return;
    // (out of its middle: a grass tuft's spot is its feet, a barrel's its top);
    y += obj->objTemplate->unk11 == SQUEAK_BARREL_OBJECT ? 16 : -8;
    out = CreateObjTemplateAndObj(obj->base.unk56, 1, 0x24, x, y, 0, 0x1F, 0, 0, type, behavior, 0, 0, 0, 0, 0, 0, 0, 0,
                                  obj->objTemplate->unk16, 0, 0, 0);
    // (an ability star's bounces read a Kirby's state: the one nearest);
    if (type == OBJ_ABILITY_STAR_1) out->base.parent = FindTargetKirby(&obj->base);
}

static void HolderInit(struct Object *obj, struct SqueakGimmickSprite *spr, u8 look, s8 x0, s8 y0, s8 x1, s8 y1) {
    GimmickInit(obj, spr, look);
    obj->base.unk5C = (obj->base.unk5C & ~7) | HOLDER_KIND;
    obj->base.unk68 = 0;
    ObjectSetBounds(&obj->base, x0, y0, x1, y1);
    sub_0803E2B0(&obj->base, x0, y0, x1, y1);
}

/* The grass tuft: its look its template's unk12 (0x02227C28: grass_A_S, _A_L, _B_S, _B_L, _C_S, _C_L; even small, its box -16 to 16
 * across and 48 up from its spot, its feet; odd large, -32 to 32 and 56 up). A body moving through it rustles it (message 5: its
 * animation 1); an attack cuts it (0xB, `0x02221C64`): what it holds comes out, its blades fly (animation 5, an object of its own
 * here) and its stub goes (6); one that held something grows back 600 frames later (`0x02221F58`), with it again; */
struct SqueakGrass {
    struct Object obj;
    struct SqueakGimmickSprite spr;
    u8 state;
    u16 timer;
};
#define GRASS_STILL 0
#define GRASS_RUSTLE 1
#define GRASS_CUT 2
#define GRASS_GONE 3
#define GRASS_REGROW 600
#define GrassLarge(obj) ((obj)->objTemplate->unk12 & 1)
static const s8 sGrassBoxes[2][4] = { { -16, -48, 16, 0 }, { -32, -56, 32, 0 } };

static void SqueakBladesMain(struct Object *obj) {
    struct SqueakGimmickSprite *spr = &((struct SqueakGimmickObject *)obj)->spr;

    GimmickAnimate(obj, spr);
    if (spr->ended) obj->base.flags |= 0x1000;
}

static void SqueakGrassMain(struct Object *obj) {
    struct SqueakGrass *g = (struct SqueakGrass *)obj;
    const s8 *box = sGrassBoxes[GrassLarge(obj)];

    if (gUnk_03000510.unk4 & ((1 << obj->base.unk56) | 0x10)) return;
    GimmickAnimate(obj, &g->spr);
    switch (g->state) {
    case GRASS_STILL:
    case GRASS_RUSTLE:
        if (Struck(obj)) {
            LetOut(obj);
            CreateObjTemplateAndObj(obj->base.unk56, 1, 0x24, obj->base.x >> 8, obj->base.y >> 8, 0, 0x1F, 0, 0, OBJ_HAMMER_PEG, 0, 0,
                                    0, 0, 0, 0, 0, 0, SQUEAK_BLADES_OBJECT, obj->objTemplate->unk12, 0, 0, 0);
            PlaySfx(&obj->base, SE_ABILITY_CUTTER_ATTACK);
            GimmickPlay(obj, &g->spr, 6);
            g->state = GRASS_CUT;
            obj->base.unk5C &= ~7;
        } else if (g->state == GRASS_STILL && KirbyIn(obj, box[0], box[1], box[2], box[3], TRUE)) {
            GimmickPlay(obj, &g->spr, 1);
            g->state = GRASS_RUSTLE;
        } else if (g->state == GRASS_RUSTLE && g->spr.ended) {
            GimmickPlay(obj, &g->spr, 0);
            g->state = GRASS_STILL;
        }
        break;
    case GRASS_CUT:
        if (!g->spr.ended) break;
        obj->base.flags |= 0x400;
        g->state = GRASS_GONE;
        g->timer = 0;
        break;
    case GRASS_GONE:
        if (!(obj->objTemplate->unk14 & 0xFF)) break;
        if (++g->timer < GRASS_REGROW) break;
        obj->base.flags &= ~(0x400 | HOLDER_HIT);
        obj->base.unk5C = (obj->base.unk5C & ~7) | HOLDER_KIND;
        GimmickPlay(obj, &g->spr, 0);
        g->state = GRASS_STILL;
        break;
    }
}

static void *SqueakCreateGrass(struct ObjectTemplate *template, u8 playerId) {
    struct Task *task = TaskCreate(ObjectMain, sizeof(struct SqueakGrass), 0x1000, TASK_USE_EWRAM, ObjectDestroy);
    struct SqueakGrass *g = TaskGetStructPtr(task);
    struct Object *obj = &g->obj;
    const s8 *box;

    InitObject(obj, template, playerId);
    box = sGrassBoxes[GrassLarge(obj)];
    HolderInit(obj, &g->spr, SQUEAK_GRASS + template->unk12 % 6, box[0], box[1], box[2], box[3]);
    g->state = GRASS_STILL;
    GimmickPlay(obj, &g->spr, 0);
    obj->unk78 = SqueakGrassMain;
    return obj;
}

// (its blades flying as it's cut: the grass's look's animation 5, gone as it ends);
static void *SqueakCreateBlades(struct ObjectTemplate *template, u8 playerId) {
    struct Task *task = TaskCreate(ObjectMain, sizeof(struct SqueakGimmickObject), 0x1000, TASK_USE_EWRAM, ObjectDestroy);
    struct SqueakGimmickObject *b = TaskGetStructPtr(task);

    InitObject(&b->obj, template, playerId);
    GimmickInit(&b->obj, &b->spr, SQUEAK_GRASS + template->unk12 % 6);
    GimmickPlay(&b->obj, &b->spr, 5);
    b->obj.unk78 = SqueakBladesMain;
    return &b->obj;
}

/* The barrel (yakumono/barrel: 0 open, its lid up with a down arrow, 1 the lid shutting, 2 shut, 3 the lid opening, 4 shaking,
 * 5 bursting): its spot its top, its box 16 to each side and 32 down (the constructor's -1.0, -2.0 to 1.0, 0 in the DS's y-up
 * world); one holding something starts shut, an empty one open (on the DS Kirby gets in with Down, out with A, shaking it as he
 * moves: KatAM has no such move, left out here); an attack bursts it (message 0xB, sound 0x51), letting out what it holds, and
 * it's gone; */
struct SqueakBarrel {
    struct Object obj;
    struct SqueakGimmickSprite spr;
    bool8 burst;
};

static void SqueakBarrelMain(struct Object *obj) {
    struct SqueakBarrel *b = (struct SqueakBarrel *)obj;

    if (gUnk_03000510.unk4 & ((1 << obj->base.unk56) | 0x10)) return;
    GimmickAnimate(obj, &b->spr);
    if (b->burst) {
        if (b->spr.ended) obj->base.flags |= 0x1000;
        return;
    }
    if (!Struck(obj)) return;
    LetOut(obj);
    PlaySfx(&obj->base, SE_BLOCK_BREAK);
    GimmickPlay(obj, &b->spr, 5);
    b->burst = TRUE;
    obj->base.unk5C &= ~7;
}

static void *SqueakCreateBarrel(struct ObjectTemplate *template, u8 playerId) {
    struct Task *task = TaskCreate(ObjectMain, sizeof(struct SqueakBarrel), 0x1000, TASK_USE_EWRAM, ObjectDestroy);
    struct SqueakBarrel *b = TaskGetStructPtr(task);
    struct Object *obj = &b->obj;

    InitObject(obj, template, playerId);
    HolderInit(obj, &b->spr, SQUEAK_BARREL, -16, 0, 16, 32);
    b->burst = FALSE;
    GimmickPlay(obj, &b->spr, template->unk14 & 0xFF ? 2 : 0);
    obj->unk78 = SqueakBarrelMain;
    return obj;
}

/* The icicle (gimmick 9: ctor 0x022233B0, vtable 0x02229740; yakumono/icicle, 0 hanging, 1 shaking, 2 falling, 3 shattering): it
 * waits for a body under it (state 0, 0x02223260: a box from the body up 12 blocks) and lets a falling icicle go (state 1,
 * 0x0222324C: an object of its own, 0x138 bytes, vtable 0x02229788, made again once the last is gone). Here one object: it
 * hangs over its spot (its tip: the DS draws it above), shakes ICICLE_SHAKE frames once a Kirby is under it (within 24 pixels across, 192 down), falls
 * (KatAM's gravity, hurting as it goes: a plain hit, an enemy's touch's kind 0x80; the fire column's 0x100 burnt Kirby), shatters
 * on the floor and hangs again ICICLE_BACK
 * frames on; */
struct SqueakIcicle {
    struct Object obj;
    struct SqueakGimmickSprite spr;
    u8 state;
    u16 timer;
};
#define ICICLE_HANG 0
#define ICICLE_SHAKE 1
#define ICICLE_FALL 2
#define ICICLE_SHATTER 3
#define ICICLE_GONE 4
#define ICICLE_SHAKE_FRAMES 30
#define ICICLE_BACK 180
#define ICICLE_GRAVITY 0x30
#define ICICLE_TOP_SPEED 0x500
#define ICICLE_FALL_MOST 512

static bool32 OnFloor(struct Object *obj) {
    u8 tile = GetCollisionTile(obj->base.unk56, obj->base.x >> 12, ((obj->base.y >> 8) + 2) >> 4);

    return tile != 0 && tile < 0x20;
}

static void SqueakIcicleMain(struct Object *obj) {
    struct SqueakIcicle *c = (struct SqueakIcicle *)obj;

    if (gUnk_03000510.unk4 & ((1 << obj->base.unk56) | 0x10)) return;
    GimmickAnimate(obj, &c->spr);
    obj->base.unk68 &= ~(7 | 0x80);
    switch (c->state) {
    case ICICLE_HANG:
        if (KirbyIn(obj, -24, -8, 24, 192, FALSE)) {
            GimmickPlay(obj, &c->spr, 1);
            c->state = ICICLE_SHAKE;
            c->timer = 0;
        }
        break;
    case ICICLE_SHAKE:
        if (++c->timer < ICICLE_SHAKE_FRAMES) break;
        GimmickPlay(obj, &c->spr, 2);
        c->state = ICICLE_FALL;
        obj->base.yspeed = 0;
        break;
    case ICICLE_FALL:
        obj->base.unk68 |= 3 | 0x80;
        if (obj->base.yspeed < ICICLE_TOP_SPEED) obj->base.yspeed += ICICLE_GRAVITY;
        obj->base.y += obj->base.yspeed;
        // (on a floor, or ICICLE_FALL_MOST pixels down: off the room's bottom, as in a pit);
        if (OnFloor(obj) || obj->base.y - (obj->objTemplate->y << 8) >= ICICLE_FALL_MOST << 8) {
            GimmickPlay(obj, &c->spr, 3);
            PlaySfx(&obj->base, SE_BLOCK_BREAK);
            c->state = ICICLE_SHATTER;
        }
        break;
    case ICICLE_SHATTER:
        if (!c->spr.ended) break;
        obj->base.flags |= 0x400;
        c->state = ICICLE_GONE;
        c->timer = 0;
        break;
    case ICICLE_GONE:
        if (++c->timer < ICICLE_BACK) break;
        obj->base.x = obj->objTemplate->x << 8;
        obj->base.y = obj->objTemplate->y << 8;
        obj->base.flags &= ~0x400;
        GimmickPlay(obj, &c->spr, 0);
        c->state = ICICLE_HANG;
        break;
    }
}

static void *SqueakCreateIcicle(struct ObjectTemplate *template, u8 playerId) {
    struct Task *task = TaskCreate(ObjectMain, sizeof(struct SqueakIcicle), 0x1000, TASK_USE_EWRAM, ObjectDestroy);
    struct SqueakIcicle *c = TaskGetStructPtr(task);
    struct Object *obj = &c->obj;

    InitObject(obj, template, playerId);
    GimmickInit(obj, &c->spr, SQUEAK_ICICLE);
    obj->base.unk5C |= 0x20;
    ObjectSetBounds(&obj->base, -8, -32, 8, 0);
    sub_0803E2B0(&obj->base, -6, -28, 6, -2);
    c->state = ICICLE_HANG;
    GimmickPlay(obj, &c->spr, 0);
    obj->unk78 = SqueakIcicleMain;
    return obj;
}

/* The falling rock (gimmick 8: ctor 0x02221740, vtable 0x02229348; yakumono/rock, 0 appearing, 1 falling and spinning, 2 breaking):
 * it waits (state 0, 0x0221E664: nothing) for a message (kinds 0-2, 0x022216F4: a switch, a trigger area, a relay, the cascades
 * of 8-1-8 and 4-6-8 the relays' waits), then lets a rock fall from its spot (state 1, 0x02221688: an object of its own, 0x138
 * bytes, vtable 0x02229390, its look by the parameter's bits 0-2, 0x02227BD0: rock's animations 0-2) and waits again. Here one
 * object: hidden till a message, then its delay (the parameter's bits 4-15, frames), it appears (ROCK_APPEAR frames), falls with gravity spinning and hurting (a plain hit), breaks
 * on the floor and hides again; */
struct SqueakRock {
    struct Object obj;
    struct SqueakGimmickSprite spr;
    u8 index;
    u8 state;
    u16 timer;
};
#define ROCK_WAIT 0
#define ROCK_APPEAR 1
#define ROCK_FALL 2
#define ROCK_BREAK 3
#define ROCK_DELAY 4
#define ROCK_APPEAR_FRAMES 16

static void SqueakRockMain(struct Object *obj) {
    struct SqueakRock *r = (struct SqueakRock *)obj;
    u8 kind;

    if (gUnk_03000510.unk4 & ((1 << obj->base.unk56) | 0x10)) return;
    GimmickAnimate(obj, &r->spr);
    obj->base.unk68 &= ~(7 | 0x80);
    kind = TakeMail(SlotOf(&obj->base), r->index);
    switch (r->state) {
    case ROCK_WAIT:
        if (kind == 0 || kind > MESSAGE_AREA + 1) break;
        // (its parameter's bits 4-15 the frames before it comes: 8-1-8's five at 10, 30, 60, 100 and 120);
        r->timer = obj->objTemplate->unk14 >> 4;
        r->state = ROCK_DELAY;
        // fallthrough
    case ROCK_DELAY:
        if (r->timer != 0 && --r->timer != 0) break;
        obj->base.x = obj->objTemplate->x << 8;
        obj->base.y = obj->objTemplate->y << 8;
        obj->base.flags &= ~0x400;
        GimmickPlay(obj, &r->spr, 0);
        r->state = ROCK_APPEAR;
        r->timer = 0;
        break;
    case ROCK_APPEAR:
        if (++r->timer < ROCK_APPEAR_FRAMES) break;
        GimmickPlay(obj, &r->spr, 1);
        obj->base.yspeed = 0;
        r->state = ROCK_FALL;
        break;
    case ROCK_FALL:
        obj->base.unk68 |= 3 | 0x80;
        if (obj->base.yspeed < ICICLE_TOP_SPEED) obj->base.yspeed += ICICLE_GRAVITY;
        obj->base.y += obj->base.yspeed;
        if (GetCollisionTile(obj->base.unk56, obj->base.x >> 12, ((obj->base.y >> 8) + 8) >> 4) - 1u < 0x1F
            || obj->base.y - (obj->objTemplate->y << 8) >= ICICLE_FALL_MOST << 8) {
            GimmickPlay(obj, &r->spr, 2);
            PlaySfx(&obj->base, SE_BLOCK_BREAK);
            r->state = ROCK_BREAK;
            r->timer = 0;
        }
        break;
    case ROCK_BREAK:
        if (++r->timer < 30) break;
        obj->base.flags |= 0x400;
        r->state = ROCK_WAIT;
        break;
    }
}

static void *SqueakCreateRock(struct ObjectTemplate *template, u8 playerId) {
    struct Task *task = TaskCreate(ObjectMain, sizeof(struct SqueakRock), 0x1000, TASK_USE_EWRAM, ObjectDestroy);
    struct SqueakRock *r = TaskGetStructPtr(task);
    struct Object *obj = &r->obj;

    InitObject(obj, template, playerId);
    r->index = template->unk12;
    GimmickInit(obj, &r->spr, SQUEAK_ROCK);
    obj->base.unk5C |= 0x20;
    ObjectSetBounds(&obj->base, -8, -8, 8, 8);
    sub_0803E2B0(&obj->base, -7, -7, 7, 7);
    obj->base.flags |= 0x400;
    r->state = ROCK_WAIT;
    GimmickPlay(obj, &r->spr, 0);
    obj->unk78 = SqueakRockMain;
    return obj;
}

// SqueakGimmickAnim(); A gimmick's look and cell as squeak.c's SqueakAnimFor() draws it (0: the object isn't one);
u32 SqueakGimmickAnim(struct Object *obj) {
    if (obj->type != OBJ_HAMMER_PEG) return 0;
    if (obj->objTemplate->unk11 == SQUEAK_SWITCH_OBJECT)
        return gSqueakData.enemies[SQUEAK_SWITCH].animId | (SQUEAK_HURT_VARIANTS + (obj->unk83 & 1)) << 16;
    if (obj->objTemplate->unk11 >= SQUEAK_ROCK_OBJECT && obj->objTemplate->unk11 <= SQUEAK_SHUTTER_OBJECT)
        return gSqueakData.enemies[((struct SqueakGimmickObject *)obj)->spr.look].animId | obj->unk83 << 16;
    return 0;
}

// SqueakCreateSwitch(); Object 0x8D's spawn: a Squeak Squad gimmick by its template's unk11, else KatAM's hammer peg;
void *SqueakCreateSwitch(struct ObjectTemplate *template, u8 playerId) {
    switch (template->unk11) {
    case SQUEAK_SWITCH_OBJECT: return SqueakCreateButton(template, playerId);
    case SQUEAK_SHUTTER_OBJECT: return SqueakCreateShutter(template, playerId);
    case SQUEAK_FIRE_OBJECT: return SqueakCreateFire(template, playerId);
    case SQUEAK_GRASS_OBJECT: return SqueakCreateGrass(template, playerId);
    case SQUEAK_BARREL_OBJECT: return SqueakCreateBarrel(template, playerId);
    case SQUEAK_BLADES_OBJECT: return SqueakCreateBlades(template, playerId);
    case SQUEAK_ICICLE_OBJECT: return SqueakCreateIcicle(template, playerId);
    case SQUEAK_ROCK_OBJECT: return SqueakCreateRock(template, playerId);
    }
    return gSqueakSwitch.create(template, playerId);
}
