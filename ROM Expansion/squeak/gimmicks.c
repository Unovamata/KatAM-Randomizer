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
#include "code_0806F780.h"
#include "constants/kirby.h"
#include "../abilities.h"
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

/* The chase rooms of KatAM's own (the randomizer's "Add Chase Sequences to KatAM", KatAMExpansionChases.cs's AddKatamChases()):
 * per room its gimmick list (its chase's wall and Squeaks), by its id; */
struct SqueakChaseRoom {
    u16 room;
    u16 unused;
    const struct SqueakGimmickList *list;
};

struct SqueakChaseRooms {
    const struct SqueakChaseRoom *rooms;
    u32 count;
};

const struct SqueakChaseRooms gSqueakChaseRooms = { NULL, 0 };

// ListOf(); A room's gimmick list (a Squeak Squad room's, a KatAM chase room's), NULL none;
static const struct SqueakGimmickList *ListOf(u16 roomId) {
    u32 i;

    if (roomId >= SQUEAK_FIRST_ROOM) return roomId - SQUEAK_FIRST_ROOM < gSqueakGimmickRooms.count ? gSqueakGimmickRooms.list[roomId - SQUEAK_FIRST_ROOM] : NULL;
    for (i = 0; i < gSqueakChaseRooms.count; i++)
        if (gSqueakChaseRooms.rooms[i].room == roomId) return gSqueakChaseRooms.rooms[i].list;
    return NULL;
}

/* ChaseKey(), SetChaseKey(); The chase on (its key, 0 none): kept in the rooms' mail slots' spare bytes (rooms.c's SqueakMailOf: not
 * saved, nothing else uses them), slot 0's and its complement in slot 1's (read as none if they don't match: what else uses that memory
 * while no room is loaded, the Collection Room); */
static u8 ChaseKey(void) {
    u8 key = SqueakMailOf(0)->unused;

    return (u8)~SqueakMailOf(1)->unused == key ? key : 0;
}

static void SetChaseKey(u8 key) {
    SqueakMailOf(0)->unused = key;
    SqueakMailOf(1)->unused = ~key;
}

/* SqueakChaseRoom(); The player's Kirby (player 0) going into a room (rooms.c's BigFillLevelInfo): a room of another chase's key (or
 * none) ends the chase on; */
void SqueakChaseRoom(u8 playerId, u16 roomId) {
    const struct SqueakGimmickList *list;
    u8 key = ChaseKey();

    if (playerId != 0 || key == 0) return;
    list = ListOf(roomId);
    if (list == NULL || list->unused[0] != key) SetChaseKey(0);
}

// The hammer peg's spawn (KatAM's own pegs): the game's CreateHammerPeg, kept by KatAMExpansionChests.cs;
struct SqueakSwitch {
    void *(*create)(struct ObjectTemplate *, u8);
};

const struct SqueakSwitch gSqueakSwitch = { NULL };

#define GIMMICK_TRIGGER 0x15
#define GIMMICK_RELAY 0x16
#define GIMMICK_RANDOM_RELAY 0x17
#define GIMMICK_QUAKE 0x13
#define QUAKE_REACH 64
#define QUAKE_DONE 2
#define RANDOM_RELAY_TARGETS 8
#define GIMMICK_BLOCKS 0x10
#define GIMMICK_CAMERA 0x14
#define GIMMICK_EMITTER 0x19
#define EMITTER_WAIT 100
#define GIMMICK_STREAKS 0x18
#define GIMMICK_LIGHT 0x12
#define GIMMICK_SOUND 0x1A
#define GIMMICK_CHASE 0xF0          // A chase's wall (squeak.h, "The chases"): its x the chest's, the room's whole height;
#define GIMMICK_CHASE_SPAWN 0xF1    // A Squeak the chase brings: at its spot (KatAM's template spot), par[0] the character, par[1] its variant;
#define GIMMICK_CHASE_CHEST 0xF6    // A chest the chase brings (the DS's chase-only chests): at its spot, par[0] its item, par[1] its number, par[2-3] its mark (unk16), par[4] its half flag, par[5] its type;
#define GIMMICK_CANDLE_LIGHT 0xF7    // A room's candles' light (KatAMExpansionSqueakRooms.cs: in the rooms with candles): CandleLight();
#define CHASE_NEAR 64               // (a chase's Squeak comes out once its spot is this near the screen, as KatAM's enemies as the camera nears them);
#define GIMMICK_LAIR_EXIT 0xF2      // A Squeak Lair's way out (KatAMExpansionSqueakRooms.cs): its door at its spot's cell, opened once no mini-boss is left;
#define GIMMICK_CURRENTS 0xF3       // A room's currents (KatAMExpansionSqueakRooms.cs's SqueakCurrents()): par[0-3] its grid's address;
#define GIMMICK_CRUMBLES 0xF4       // A room's walk-once blocks (KatAMExpansionSqueakRooms.cs's SqueakCrumbles()): par[0-3] their list's address;
#define CRUMBLES 8                  // (walk-once blocks breaking at once in a room);
#define GIMMICK_PHASE_WARP 0xF5     // The finale's way on from a fight (KatAMExpansionSqueakLevels.cs' SqueakPhaseWarps): par[0-1] the room, par[2-3] the cell + 1, par[4] 1 after the fight;
#define LAIR_EXIT_WAIT 30           // (the room's objects made by then: KatAM makes those on screen as the room comes up);
#define LAIR_EXIT_OPEN 0xFF
#define FAR_VIEW_FIRST 96           // (the far view's colors: KatAM's BG palettes 6-13, KatAMExpansionSqueakRooms.cs's SqueakBackground);
#define FAR_VIEW_COLORS 128
#define LIGHT_STEPS 16
#define LIGHT_DIM 16                // (the start: its colors at LIGHT_DIM / LIGHT_STEPS, the DS's, full: Light());
#define STREAKS_WAIT 10
#define STREAKS_NEAR 32
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

/* The room's task: the trigger areas (0x15), relays (0x16, 0x17), quake triggers (0x13), camera areas (0x14), effect emitters (0x19)
 * and switch block groups (0x10). Its ObjectBase is what KatAM's chain
 * breaker (sub_0800E0E4) reads of the object breaking a chain (its room, its player's level info) and whose spot it starts from; */
// A walk-once block breaking (Crumbles()): its cell, the frames left before it goes, how many hidden cells under it go after it;
struct SqueakCrumble {
    u8 x, y, timer, left;
};

struct SqueakLogic {
    struct ObjectBase base;
    const struct SqueakGimmickList *list;
    u16 room;
    u8 slot;
    u8 generation;
    u8 inside[SQUEAK_MAIL_GIMMICKS];            // A trigger area: a body in it last frame;
    u8 relayKind[SQUEAK_MAIL_GIMMICKS];         // A relay: the kind it sends + 1 while it runs;
    s16 relayTimer[SQUEAK_MAIL_GIMMICKS][RELAY_TARGETS];
    u8 cameraArea[4];                           // Per Kirby, the camera area holding his view (its index + 1), 0 none;
    struct S32Vec2 cameraWas[4][2];             // and his view's bounds' targets before it (unk_S32Vec2_7C, unk_S32Vec2_84);
    u8 lightState;                              // The far view's light (gimmick 0x12, Light()): its state, its step shown, its
    u8 lightStep;                               // frames so far,
    u16 lightCount;
    u16 farView[FAR_VIEW_COLORS];               // and the far view's colors as loaded;
    u8 candleState;                             // The candles' light (CandleLight()): whether it has the far view's colors, whether a
    u8 candleLit;                               // candle was lit last frame, the fade's frames so far, from and to which light, and the
    u16 candleCount;                            // light shown (CANDLE_FULL full);
    u16 candleFrom, candleTo, candleLevel;
    struct SqueakCrumble crumble[CRUMBLES];     // The walk-once blocks breaking (Crumbles());
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

/* CameraArea(); A camera area (0x14, ctor 0x02224B88, vtable 0x02229A7C): its parameter's u16s at +0 its bounds (x0, y0, x1, y1, pixels),
 * its u16 at +8 its box (bits 0-6 blocks right, 7-13 down, as a trigger area's), bits 14-15 how many targets (indices from +0xA), +0xD
 * bit 0 that it holds the camera. A body moving into the box (0x02224AA0) sends its targets kind 2 and holds the camera's view inside
 * the bounds (one DS screen, 256 x 192, in nearly all of the 58: the goal games' and boss rooms' screens, 2-3-8's two). Here the
 * Kirby's view's bounds (KatAM eases his view to them: unk_S32Vec2_7C, unk_S32Vec2_84) are the area's, the GBA's smaller screen
 * moving inside them; let go (his room's bounds again) once he's out of the bounds, so he's never off the screen (the DS keeps them:
 * its rooms close behind him). Not in a room whose fight's arena holds the camera (the converter leaves those out); */
static void CameraArea(struct SqueakLogic *l, const struct SqueakGimmickEntry *e) {
    s32 x0 = e->par[0] | e->par[1] << 8, y0 = e->par[2] | e->par[3] << 8, x1 = e->par[4] | e->par[5] << 8, y1 = e->par[6] | e->par[7] << 8;
    u16 box = e->par[8] | e->par[9] << 8;
    s32 bx0 = e->x, bx1 = e->x + (box & 0x7F) * 16, by0 = e->y, by1 = e->y + (box >> 7 & 0x7F) * 16;
    u8 i, t;

    // (the GBA's screen fitting: bounds narrower or lower than it widened round their middle);
    if (x1 - x0 < DISPLAY_WIDTH) x0 = (x0 + x1 - DISPLAY_WIDTH) / 2, x1 = x0 + DISPLAY_WIDTH;
    if (y1 - y0 < DISPLAY_HEIGHT) y0 = (y0 + y1 - DISPLAY_HEIGHT) / 2, y1 = y0 + DISPLAY_HEIGHT;
    for (i = 0; i < gNumKirbys && i < 4; i++) {
        struct Kirby *k = &gKirbys[i];
        struct LevelInfo *level = &gCurLevelInfo[i];
        s32 kx = k->base.x >> 8, ky = k->base.y >> 8;
        bool32 inBox = k->base.roomId == l->room && kx + k->base.unk3E > bx0 && kx + k->base.unk3C < bx1 && ky + k->base.unk3F > by0
                       && ky + k->base.unk3D < by1;

        if (l->cameraArea[i] == e->index + 1) {
            // (out of the bounds, or of the room: his room's view again);
            if (k->base.roomId != l->room || kx < x0 || kx > x1 || ky < y0 || ky > y1) {
                level->unk_S32Vec2_7C = l->cameraWas[i][0];
                level->unk_S32Vec2_84 = l->cameraWas[i][1];
                l->cameraArea[i] = 0;
                continue;
            }
        } else {
            if (!inBox) continue;
            if (l->cameraArea[i] == 0) {
                l->cameraWas[i][0] = level->unk_S32Vec2_7C;
                l->cameraWas[i][1] = level->unk_S32Vec2_84;
            }
            for (t = 0; t < (box >> 14) && t < 3; t++) SqueakSendMail(l->slot, e->par[10 + t], MESSAGE_AREA);
            if (!(e->par[13] & 1)) continue;
            l->cameraArea[i] = e->index + 1;
        }
        level->unk_S32Vec2_7C.x = x0 << 8;
        level->unk_S32Vec2_7C.y = y0 << 8;
        level->unk_S32Vec2_84.x = x1 << 8;
        level->unk_S32Vec2_84.y = y1 << 8;
    }
}

/* Emitter(); An effect emitter (0x19, ctor 0x02225E9C, vtable 0x02229D30, its state 0x02225D14): 171 in 24 rooms, its parameter's
 * first byte's low nibble its look (0x02229CBC: 0 yakumono/bubble, a water bubble, 51 of them; 1 yakumono/spark, a lava spark, 120;
 * 2 unused), always its animation 0 (0x02227F90). Each time its wait (+0x5C, from 0) runs out it plays that once at its spot (an
 * effect: 0x0221F89C, kind 6, not moving), if its spot is in the camera's view (0x0221FA50), and waits again 100, 200, 300 or 400
 * frames (the random's top 2 bits + 1, times 100), seen or not. Here the local player's view; its wait the relay timer's first; */
static void Emitter(struct SqueakLogic *l, const struct SqueakGimmickEntry *e) {
    s16 *wait = &l->relayTimer[e->index][0];
    const struct LevelInfo *level = &gCurLevelInfo[gLocalPlayerId];
    s32 vx = level->viewportPosition.x >> 8, vy = level->viewportPosition.y >> 8;

    if (*wait > 0) {
        (*wait)--;
        return;
    }
    if (gKirbys[gLocalPlayerId].base.roomId == l->room && e->x >= vx && e->x <= vx + DISPLAY_WIDTH && e->y >= vy && e->y <= vy + DISPLAY_HEIGHT)
        SqueakEffectInRoom(l->room, (e->par[0] & 15) == 0 ? SQUEAK_FX_WATER_BUBBLE : SQUEAK_FX_LAVA_SPARK, e->x << 8, e->y << 8);
    *wait = ((Rand16() >> 14) + 1) * EMITTER_WAIT;
}

/* SoundGimmick(); The sound gimmick (0x1A, ctor 0x0222684C, vtable 0x02229E34, its state 0x022267AC; one, in 4-3-2): when its wait
 * (+0x60) runs out it plays its sound (its first byte's low nibble picks it from 0x02227FB4: the only one, 0x32) at its spot, then
 * waits its parameter's second u16 in frames; its second byte how many times (0: on and on); heard here while the player's Kirby is
 * in its room (KatAMExpansionSqueakSound.cs: the DS's sounds); */
static void SoundGimmick(struct SqueakLogic *l, const struct SqueakGimmickEntry *e) {
    static const u16 sSounds[] = { 0x32 };
    s16 *wait = &l->relayTimer[e->index][0], *played = &l->relayTimer[e->index][1];
    u8 times = e->par[1];

    if (*wait > 0) {
        (*wait)--;
        return;
    }
    if (times != 0 && *played >= times) return;
    if (gKirbys[gLocalPlayerId].base.roomId == l->room) m4aSongNumStart(SQUEAK_SE(sSounds[(e->par[0] & 15) % ARRAY_COUNT(sSounds)]));
    (*played)++;
    *wait = e->par[2] | e->par[3] << 8;
}

/* Streaks(); A streak emitter (0x18, ctor 0x02226384, vtable 0x02229DD4, its state 0x02225F80): 33 in 18 rooms (Ice Island's snow,
 * 3-4-1's and 7-2-3's wind, 8-1-9's and 8-2-5's falling streams). Its parameter: a u16, bits 0-3 its look (0x02229D78: 0
 * yakumono/stream, 1 yakumono/snow), bits 4-6 the side of its box it comes from (0 the top, 1 the bottom, 2 the left, 3 the right),
 * bits 7-9 its speed; then its box, a byte cells across and one down. Every STREAKS_WAIT frames (+0x60), if its box is near the
 * camera's view (0x0221FA50: a box 64 pixels wider and taller than the screen), a streak (an effect, 0x0221F89C kind 6) at a random
 * spot along that side (a cell's middle across the top or bottom, down the left or right), one of the look's 3 animations at random
 * (0x02227F9C: the stream's 0, 1, 2, the snow's 0, 1, 0), moving the way the side faces at the speed its constructor's table gives
 * (0x0222C75C: the stream 4, 6 or 8 pixels a frame, the snow always 1 pixel down). Both looks' animations loop: the DS's effects end
 * them; here a streak ends once it's been on the screen and left it, or after SQUEAK_DRIFT_LIFE frames (squeak.c's
 * SqueakDriftInRoom); near the view: its box within STREAKS_NEAR pixels of it; */
static void Streaks(struct SqueakLogic *l, const struct SqueakGimmickEntry *e) {
    static const u8 sAnims[2][3] = { { 0, 1, 2 }, { 0, 1, 0 } };
    s16 *wait = &l->relayTimer[e->index][0];
    const struct LevelInfo *level = &gCurLevelInfo[gLocalPlayerId];
    s32 vx = level->viewportPosition.x >> 8, vy = level->viewportPosition.y >> 8;
    u16 p = e->par[0] | e->par[1] << 8;
    u8 look = (p & 15) != 0, side = p >> 4 & 7, speed = p >> 7 & 7, across = e->par[2], down = e->par[3];
    s32 x0 = e->x, y0 = e->y, x1 = e->x + across * 16, y1 = e->y + down * 16, x, y, dx = 0, dy = 0;

    if (*wait > 0) {
        (*wait)--;
        return;
    }
    *wait = STREAKS_WAIT;
    if (gKirbys[gLocalPlayerId].base.roomId != l->room || side > 3) return;
    if (x1 < vx - STREAKS_NEAR || x0 > vx + DISPLAY_WIDTH + STREAKS_NEAR || y1 < vy - STREAKS_NEAR || y0 > vy + DISPLAY_HEIGHT + STREAKS_NEAR) return;
    if (side <= 1) {
        x = x0 + (across ? Rand16() % across : 0) * 16 + 8;
        y = side == 0 ? y0 : y1;
    } else {
        x = side == 2 ? x0 : x1;
        y = y0 + (down ? Rand16() % down : 0) * 16 + 8;
    }
    if (look == 1) dy = 1;
    else {
        s32 v = 4 + (speed > 2 ? 2 : speed) * 2;
        switch (side) {
        case 0: dy = v; break;
        case 1: dy = -v; break;
        case 2: dx = v; break;
        default: dx = -v; break;
        }
    }
    SqueakDriftInRoom(l->room, (look ? SQUEAK_FX_SNOW : SQUEAK_FX_STREAM) + sAnims[look][Rand16() % 3], x << 8, y << 8, dx << 8, dy << 8);
}

/* Light(); The far view's light (gimmick 0x12, ctor 0x022202E4, vtable 0x0222913C; in 2-3-1, 2-8-1 and 7-3-1, where Kirby starts the
 * stage): its parameter's bytes 0-1 its box (cells across, down from its spot, as a trigger area's), its u16 at +2 the frames, bytes
 * 5-7 a color (all three 255: full light). A body moving into its box (or a switch's message) calls the far view's fade (the stage's
 * +0xAC, its vtable +0x10: 0x0221A75C) with those frames: the DS's far view there is a list of 17 pictures, its colors blended from
 * a start palette to its own in 16 steps (0x0221A84C: (start x (16 - k) + own x k) / 16 a channel), the room showing the first till
 * then and stepping through them (k = frames so far x 16 / frames, 0x0221A774) to its own: that list is only built from a room's
 * `PL` palette file of type 2 (map/pa_NN.pal: 256 colors, the boss rooms'; its start the colors in use, 0x0221AD54's), so in these
 * three rooms, which have none, the fade does nothing. The gimmick also fades the far view's light (the stage's +0xA8, 0x0221992C,
 * the gimmick's 0x02220224): from its color as the room starts, the room's light record's bytes 3-5 / 255 (read in the DS emulator:
 * 255, 255, 255 in 2-3-1 and 7-3-1, full; 2-8-1's stage didn't boot that way) to the gimmick's (bytes 5-7: 255, 255, 255 in all
 * three) over its frames: full to full, nothing dims on the DS. Here the far view (KatAM's BG palettes 6-13) starts at its colors
 * times LIGHT_DIM / 16, the DS's start (full; it was half, a stand-in, before the DS's was traced), and steps up the same way; */
extern u16 gUnk_02022120[0x100];    // The BG palettes as loaded (KatAM's palette effects put them back from it);
static u16 LightBlend(u16 c, u8 k) {
    u16 out = 0;
    u8 shift;

    for (shift = 0; shift <= 10; shift += 5) {
        u16 v = c >> shift & 31, dim = v * LIGHT_DIM / LIGHT_STEPS;

        out |= ((dim * (LIGHT_STEPS - k) + v * k) / LIGHT_STEPS) << shift;
    }
    return out;
}

static void LightShow(struct SqueakLogic *l, u8 k) {
    u16 i;

    for (i = 0; i < FAR_VIEW_COLORS; i++) gBgPalette[FAR_VIEW_FIRST + i] = gUnk_02022120[FAR_VIEW_FIRST + i] = LightBlend(l->farView[i], k);
    l->lightStep = k;
    gMainFlags |= MAIN_FLAG_BG_PALETTE_SYNC_ENABLE;
}

/* LightKept(); The far view's colors as loaded, again if the room's loading put them back after Light() took them (it loads them
 * after the room's task first runs): its colors as last shown, else taken again; */
static void LightKept(struct SqueakLogic *l) {
    u16 i;

    for (i = 0; i < FAR_VIEW_COLORS; i++)
        if (gUnk_02022120[FAR_VIEW_FIRST + i] != LightBlend(l->farView[i], l->lightStep)) break;
    if (i == FAR_VIEW_COLORS) return;
    for (i = 0; i < FAR_VIEW_COLORS; i++) l->farView[i] = gUnk_02022120[FAR_VIEW_FIRST + i];
    LightShow(l, l->lightStep);
}

static void Light(struct SqueakLogic *l, const struct SqueakGimmickEntry *e) {
    u16 frames = e->par[2] | e->par[3] << 8, i;
    s32 x0 = e->x, x1 = e->x + e->par[0] * 16, y0 = e->y, y1 = e->y + e->par[1] * 16;
    u8 kind = TakeMail(l->slot, e->index);

    if (l->lightState == 3) return;
    if (l->lightState == 0) {
        for (i = 0; i < FAR_VIEW_COLORS; i++) l->farView[i] = gUnk_02022120[FAR_VIEW_FIRST + i];
        LightShow(l, 0);
        l->lightState = 1;
    }
    LightKept(l);
    switch (l->lightState) {
    case 1:
        if (kind == MESSAGE_SWITCH + 1 || kind == MESSAGE_SWITCH + 2) l->lightState = 2;
        for (i = 0; i < gNumKirbys && l->lightState == 1; i++) {
            struct Kirby *k = &gKirbys[i];
            s32 kx = k->base.x >> 8, ky = k->base.y >> 8;

            if (k->base.roomId == l->room && kx + k->base.unk3E > x0 && kx + k->base.unk3C < x1 && ky + k->base.unk3F > y0 && ky + k->base.unk3D < y1)
                l->lightState = 2;
        }
        break;
    case 2:
        l->lightCount++;
        if (frames == 0 || l->lightCount >= frames) {
            LightShow(l, LIGHT_STEPS);
            l->lightState = 3;
        } else if (l->lightCount * LIGHT_STEPS / frames != (l->lightCount - 1) * LIGHT_STEPS / frames) {
            LightShow(l, l->lightCount * LIGHT_STEPS / frames);
        }
        break;
    }
}

/* CandleLight(); The far view's light in a room with candles (the DS's candle, 0x02223588 lit and 0x02223514 out): the first candle lit
 * while none is turns the far view's light (the stage's +0xA8) up to full (0x1000) over 30 frames, the last one going out down to 0x404
 * (a quarter) over 30; the room starts full (its light record's 255, 255, 255: read in the DS emulator in 4-3-7, loaded with its own
 * stage entry). Also the DS fades the room's 3D far view's palette, which only a kind 2 `PL` palette fills: these rooms' `PL` palette is
 * their BG palette (KatAMExpansionSqueakRooms.cs's SqueakRoomTileset()), so that one's the plain fade, which does nothing. Here the
 * far view's colors (KatAM's BG palettes 6-13) times the light, as Light() does with its own; */
#define CANDLE_FULL 0x1000
#define CANDLE_DIM 0x404
#define CANDLE_FADE 30
static u8 CandlesLit(u16 room);

static u16 CandleBlend(u16 c, u16 level) {
    u16 out = 0;
    u8 shift;

    for (shift = 0; shift <= 10; shift += 5) out |= ((c >> shift & 31) * level >> 12) << shift;
    return out;
}

static void CandleShow(struct SqueakLogic *l, u16 level) {
    u16 i;

    for (i = 0; i < FAR_VIEW_COLORS; i++) gBgPalette[FAR_VIEW_FIRST + i] = gUnk_02022120[FAR_VIEW_FIRST + i] = CandleBlend(l->farView[i], level);
    l->candleLevel = level;
    gMainFlags |= MAIN_FLAG_BG_PALETTE_SYNC_ENABLE;
}

static void CandleLight(struct SqueakLogic *l, const struct SqueakGimmickEntry *e) {
    u8 lit = CandlesLit(l->room) != 0;
    u16 i;

    if (l->candleState == 0) {
        for (i = 0; i < FAR_VIEW_COLORS; i++) l->farView[i] = gUnk_02022120[FAR_VIEW_FIRST + i];
        l->candleLevel = l->candleTo = CANDLE_FULL;
        l->candleState = 1;
    }
    // (the far view's colors again if the room's loading put them back after they were taken, as LightKept());
    for (i = 0; i < FAR_VIEW_COLORS; i++)
        if (gUnk_02022120[FAR_VIEW_FIRST + i] != CandleBlend(l->farView[i], l->candleLevel)) break;
    if (i < FAR_VIEW_COLORS) {
        for (i = 0; i < FAR_VIEW_COLORS; i++) l->farView[i] = gUnk_02022120[FAR_VIEW_FIRST + i];
        if (l->candleLevel != CANDLE_FULL) CandleShow(l, l->candleLevel);
    }
    if (lit != l->candleLit) {
        l->candleLit = lit;
        l->candleFrom = l->candleLevel;
        l->candleTo = lit ? CANDLE_FULL : CANDLE_DIM;
        l->candleCount = 0;
    }
    if (l->candleLevel != l->candleTo) {
        l->candleCount++;
        CandleShow(l, l->candleCount >= CANDLE_FADE ? l->candleTo
                                                     : l->candleFrom + ((s32)l->candleTo - l->candleFrom) * l->candleCount / CANDLE_FADE);
    }
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

/* MinibossFighting(); Whether a mini-boss (types 0x38-0x52) is alive in the room with a Kirby inside its fight's area (its template's
 * +0x1A-0x21, which KatAM's mini-bosses wait for): the random relays rain their rocks only then (2-5-5's and 5-3-4's, Big Metalun's
 * rooms: from the room's first frame they fell before the fight, and on after it; the owner, "Big Metalun spawns infinite rocks even
 * before the battle starts"); */
static bool32 MinibossFighting(u16 room) {
    struct Task *t = &gTasks[0];
    u8 n, i;

    for (n = 0; n < MAX_TASK_NUM && t->next; n++) {
        struct Object *o;
        const struct ObjectTemplate *a;

        t = (struct Task *)(IWRAM_START + t->next);
        o = TaskGetStructPtr(t);
        if (o->base.header.kind != 1 || !ObjType38To52(o) || o->base.roomId != room || o->base.flags & 0x1000 || o->unk80 <= 0
            || (a = o->objTemplate) == NULL) continue;
        for (i = 0; i < gNumKirbys; i++) {
            s32 x = gKirbys[i].base.x >> 8, y = gKirbys[i].base.y >> 8;

            if (gKirbys[i].base.roomId == room && x >= a->x + a->unk1A && x < a->x + a->unk1A + a->unk1E && y >= a->y + a->unk1C
                && y < a->y + a->unk1C + a->unk20) return TRUE;
        }
    }
    return FALSE;
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
    // (not before the room's mini-boss fight, nor after it: MinibossFighting(); its wait starts over then);
    if (!MinibossFighting(l->room)) {
        *wait = 0;
        return;
    }
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

/* Quake(); A quake trigger (0x13: ctor 0x022240E8, vtable 0x022298F8): its parameter's second u16 its box (bits 0-6 blocks right, 7-13
 * down, as a trigger area's), bits 14-15 how many targets, its bytes 4 on their indices; a body moving into it (message 5) or a switch's
 * message (kinds 0 and 1, 0x022240A8) sets it off once (state 1, 0x02224034): the screen quakes (its first u16's low 4 bits the kind,
 * the rest the frames: KatAM's timed shake nearest its length, QuakeShake(); the owner, of the quakes left out: "I want them") and its
 * targets get a message of kind 2. The bodies that set them off on the DS are its rolling boulders (4-3-2's roll down its shaft into
 * one): a Kirby within QUAKE_REACH pixels of its box does here too (4-3-2's at the end of the corridor the boulders take); */
// QuakeShake(); KatAM's timed screen shake (RequestScreenShake 1-3: 14, 18, 24 frames) nearest a quake of `frames`, the longest for more;
void RequestScreenShake(u16, struct ObjectBase *);
static void QuakeShake(u16 frames, struct ObjectBase *by) {
    RequestScreenShake(frames <= 16 ? 1 : frames <= 21 ? 2 : 3, by);
}

static void Quake(struct SqueakLogic *l, const struct SqueakGimmickEntry *e) {
    u16 p = e->par[2] | e->par[3] << 8, quake = e->par[0] | e->par[1] << 8;
    struct ObjectBase *by = &gKirbys[0].base;
    s32 x0 = e->x - QUAKE_REACH, x1 = e->x + (p & 0x7F) * 16 + QUAKE_REACH, y0 = e->y - QUAKE_REACH, y1 = e->y + (p >> 7 & 0x7F) * 16 + QUAKE_REACH;
    u8 count = p >> 14, kind = TakeMail(l->slot, e->index), i, t;
    bool32 near = kind == MESSAGE_SWITCH + 1 || kind == MESSAGE_SWITCH + 2;

    if (l->inside[e->index] == QUAKE_DONE) return;
    for (i = 0; i < gNumKirbys && !near; i++) {
        struct Kirby *k = &gKirbys[i];
        s32 kx = k->base.x >> 8, ky = k->base.y >> 8;

        if (k->base.roomId == l->room && kx > x0 && kx < x1 && ky > y0 && ky < y1) near = TRUE, by = &k->base;
    }
    if (!near) return;
    QuakeShake(quake >> 4, by);
    PlaySfx(by, SQUEAK_SE(0x68)); // (the DS's 0x68);
    for (t = 0; t < count; t++) SqueakSendMail(l->slot, e->par[4 + t], MESSAGE_AREA);
    l->inside[e->index] = QUAKE_DONE;
}

static const u8 *CellRecord(u8 p, u8 x, u8 y);

/* Blocks(); A switch block group (0x02223900): a message clears its blocks. In KatAM they're a chain (KatAMExpansionSqueakRooms.cs:
 * 0xF2 blocks, 0xF3 links), broken from the group's spot as KatAM's lever walls break theirs: its cell's special block record
 * (sub_08001408), then KatAM's chain breaker from there (sub_0800E0E4). Only while its cell still holds the record's block: a
 * group already cleared takes no more (a trigger area sends again each time a body comes back into it, and KatAM's own search,
 * sub_080025AC, never ends without a record: 4-6-3's bottom left trigger, sent twice, hung the game once its floor was gone); */
static void Blocks(struct SqueakLogic *l, const struct SqueakGimmickEntry *e) {
    u8 x = e->x >> 4, y = e->y >> 4;
    const u8 *record;

    if (TakeMail(l->slot, e->index) == 0) return;
    if ((record = CellRecord(l->base.unk56, x, y)) == NULL) return;
    l->base.x = e->x << 8;
    l->base.y = e->y << 8;
    sub_08001408(l->base.unk56, (struct Unk_0888562C *)record, NULL, NULL);
    sub_0800E0E4((struct Object *)&l->base, x, y);
}

/* ChaseWall(); A chase's wall: a Kirby in the room going past its x (either way, any height: the owner's "the height of the wall will
 * be of the entire room") starts the room's chase (its key, the list's), unless it's on; its byte (inside) a bit per Kirby, his side
 * last frame (bits 0-3), and whether he was in the room then (bits 4-7); */
static void ChaseWall(struct SqueakLogic *l, const struct SqueakGimmickEntry *e) {
    u8 *seen = &l->inside[e->index], key = l->list->unused[0], i;

    for (i = 0; i < gNumKirbys; i++) {
        u8 bit = 1 << i;
        bool8 right;

        if (gKirbys[i].base.roomId != l->room || gCurLevelInfo[i].currentRoom != l->room) {
            *seen &= ~(bit | bit << 4);
            continue;
        }
        right = (gKirbys[i].base.x >> 8) >= e->x;
        if ((*seen & bit << 4) && !!(*seen & bit) != right && key != 0 && ChaseKey() != key) SetChaseKey(key);
        *seen = (*seen & ~bit) | (right ? bit : 0) | bit << 4;
    }
}

/* ChaseSpawn(); A chase's Squeak: once its room's chase is on and its spot nears the screen (CHASE_NEAR), it comes out (type 0x2C, its
 * character and variant, SQUEAK_CHASE_SPAWN: squeak.c makes it a chase's), once while the room's loaded (its byte); */
static void ChaseSpawn(struct SqueakLogic *l, const struct SqueakGimmickEntry *e) {
    u8 *done = &l->inside[e->index];
    struct LevelInfo *level = &gCurLevelInfo[l->base.unk56];
    s32 vx = level->viewportPosition.x >> 8, vy = level->viewportPosition.y >> 8;

    if (*done || ChaseKey() == 0 || ChaseKey() != l->list->unused[0]) return;
    if (e->x < vx - CHASE_NEAR || e->x > vx + DISPLAY_WIDTH + CHASE_NEAR || e->y < vy - CHASE_NEAR || e->y > vy + DISPLAY_HEIGHT + CHASE_NEAR) return;
    *done = 1;
    CreateObjTemplateAndObj(l->base.unk56, 1, 0x24, e->x, e->y, 0, 0x1F, 0, 0, SQUEAK_TYPE, e->par[0], 0, 0, 0, 0, 0, 0, 0, 0,
                            e->par[1] | SQUEAK_CHASE_SPAWN, 0, 0, 0);
}

/* ChaseChest(); A chest the chase brings (the owner: "the 5 Ability Scroll chests the DS brings out only in the chase"; the DS's chase
 * chests, their objects' flag 4, KatAMExpansionSqueakRooms.cs): not in the room until its chase is on, then made at its spot as the
 * room's other chests (KatAM's chest, chest.c's SqueakCreateChest's fields: SQUEAK_CHEST in unk11, its number unk12, the half flag
 * unk14's high byte, the mark unk16, as LetOutChest()); once while the room's loaded (its byte): an opened one stays opened (its bit); */
static void ChaseChest(struct SqueakLogic *l, const struct SqueakGimmickEntry *e) {
    u8 *done = &l->inside[e->index];

    if (*done || ChaseKey() == 0 || ChaseKey() != l->list->unused[0]) return;
    *done = 1;
    CreateObjTemplateAndObj(l->base.unk56, 1, 0x24, e->x, e->y, 0, 0x1F, 0, 0, e->par[5], e->par[0], 0, 0, 0, 0, 0, 0, 0,
                            SQUEAK_CHEST, e->par[1], e->par[4] << 8, e->par[2] | e->par[3] << 8, 0);
}

/* LairExit(); A Squeak Lair's way out (the rulebook's rule 8; the owner: "Do make sure that these exits only appear after defeating the
 * miniboss only. These can have any miniboss in game"): built shut, its door record alone, its mirror (KatAM's one-way door) hidden
 * (KatAMExpansionSqueakRooms.cs). LAIR_EXIT_WAIT frames after the room comes up, once no mini-boss with HP is left in it (types
 * 0x38-0x52: KatAM's, NiDL's and Squeak Squad's, whichever the randomizer put there; one beaten before isn't made again), its record's
 * collision is written (as enemies/arena.c's OpenDoors()), and its mirror shows, as KatAM's hidden ones do; once (its byte); */
static bool8 MinibossLeft(u16 room) {
    struct Task *t = &gTasks[0];
    u8 n;

    for (n = 0; n < MAX_TASK_NUM && t->next; n++) {
        struct Object *other;

        t = (struct Task *)(IWRAM_START + t->next);
        other = TaskGetStructPtr(t);
        if (other->base.header.kind != 1 || !ObjType38To52(other)) continue;
        if (other->base.roomId == room && !(other->base.flags & 0x1000) && other->unk80 > 0) return TRUE;
    }
    return FALSE;
}

// MinibossThere(); Whether a mini-boss is still in the room at all, beaten or not (its defeat playing out: Dark Daroach's own);
static bool8 MinibossThere(u16 room) {
    struct Task *t = &gTasks[0];
    u8 n;

    for (n = 0; n < MAX_TASK_NUM && t->next; n++) {
        struct Object *other;

        t = (struct Task *)(IWRAM_START + t->next);
        other = TaskGetStructPtr(t);
        if (other->base.header.kind != 1 || !ObjType38To52(other)) continue;
        if (other->base.roomId == room && !(other->base.flags & 0x1000)) return TRUE;
    }
    return FALSE;
}

static void LairExit(struct SqueakLogic *l, const struct SqueakGimmickEntry *e) {
    u8 *state = &l->inside[e->index], x = e->x >> 4, y = e->y >> 4, p = l->base.unk56;
    const u8 *record;

    if (*state == LAIR_EXIT_OPEN) return;
    if (*state < LAIR_EXIT_WAIT) {
        (*state)++;
        return;
    }
    if (MinibossLeft(l->room)) return;
    *state = LAIR_EXIT_OPEN;
    for (record = (const u8 *)gCurLevelInfo[p].unk1E0.pat1; record[4] != 0; record += record[4])
        if (record[0] == 3 && record[2] == x && record[3] == y && GetCollisionTile(p, x, y) != record[1]) sub_0800255C(p, x, y, record[1]);
}

/* Currents(); The room's currents (the DS's collision ids 0x10-0x3C, on water or air; its grid a byte a cell, 0x80 | way << 2 |
 * strength): a Kirby whose middle is in one is moved its way 0.5, 1 or 2 pixels a frame (strength 0, 1, 2; way 0 up, 1 right, 2
 * down, 3 left), as Squeak Squad's (measured in the DS emulator: Kirby standing in 1-1-1, its cells rewritten to each id, on air
 * and on water, moved by exactly those, whatever he did), unless a wall stands just that way; going up, his fall is held (KatAM's
 * gravity would win over the weaker ones: on the DS the strongest lifts him); */
#define CURRENT_WAY(v) ((v) >> 2 & 3)
#define CURRENT_PUSH(v) (0x80 << ((v) & 3))
static bool32 CurrentWall(u8 p, s32 x, s32 y) {
    return (gCollisionAttributes[GetCollisionTile(p, x >> 12, y >> 12)] & 0x200) != 0;
}
static void Currents(struct SqueakLogic *l, const struct SqueakGimmickEntry *e) {
    const u8 *grid = (const u8 *)(e->par[0] | e->par[1] << 8 | e->par[2] << 16 | e->par[3] << 24);
    u8 i;

    for (i = 0; i < gNumKirbys; i++) {
        struct Kirby *k = &gKirbys[i];
        struct LevelInfo *level = &gCurLevelInfo[i];
        s32 w = level->roomWidth >> 4, h = level->roomHeight >> 4, cx = k->base.x >> 12, cy = k->base.y >> 12, push;
        u8 v;

        if (k->base.roomId != l->room || level->currentRoom != l->room || cx < 0 || cy < 0 || cx >= w || cy >= h) continue;
        // (Metal Kirby too heavy for them: the DS's flag 0x800000, abilities/squeak_moves.c);
        if (k->ability == SQUEAK_ABILITY_METAL) continue;
        v = grid[cy * w + cx];
        if (!(v & 0x80)) continue;
        push = CURRENT_PUSH(v);
        switch (CURRENT_WAY(v)) {
        case 0:
            if (CurrentWall(i, k->base.x, k->base.y - 0xC00 - push)) break;
            k->base.y -= push;
            if (k->base.yspeed < 0) k->base.yspeed = 0;
            break;
        case 1:
            if (!CurrentWall(i, k->base.x + 0x800 + push, k->base.y)) k->base.x += push;
            break;
        case 2:
            if (!CurrentWall(i, k->base.x, k->base.y + 0x800 + push)) k->base.y += push;
            break;
        case 3:
            if (!CurrentWall(i, k->base.x - 0x800 - push, k->base.y)) k->base.x -= push;
            break;
        }
    }
}

/* Crumbles(); The room's walk-once blocks (the DS's blocks grid's block 3 on air over a solid cell, KatAM's star blocks here: rows of
 * cracked blocks, 1-6-6's castle walkway, 2-6-7's leafy bridge; the owner: "Blocks that you can only walk once and they get destroyed
 * after do not work"): the one under a Kirby standing on it (under his middle) breaks CRUMBLE_WAIT frames after he lands on it, then
 * the hidden cells under it (the DS's 0x400 cells over solid ground: 3-6-2's second row, 3-2-3's and 3-1-3's shafts) one after
 * another, CRUMBLE_CHAIN frames apart (measured in the DS emulator, 3-6-2: Kirby put on its first one at x 968 falls 12 frames later,
 * the cell under it breaks 7 frames after; put at x 961, over its edge, only the cell under his middle breaks and he stays on the
 * one beside it). Its list: u16 a count, then 4 bytes a block: x, y (cells), how many hidden cells under it, 0; */
#define CRUMBLE_WAIT 11            // (KatAM sees him standing a frame after he lands: 12 frames in all, as the DS);
#define CRUMBLE_CHAIN 7
#define CRUMBLE_STAR 0xF0           // (KatAMExpansionSqueakRooms.cs' KatAMStarBlock);
static void CrumbleBreak(u8 p, u8 x, u8 y) {
    const u8 *record;

    if (GetCollisionTile(p, x, y) == CRUMBLE_STAR && (record = CellRecord(p, x, y)) != NULL)
        sub_08001408(p, (struct Unk_0888562C *)record, NULL, NULL);
}
static void Crumbles(struct SqueakLogic *l, const struct SqueakGimmickEntry *e) {
    const u8 *list = (const u8 *)(e->par[0] | e->par[1] << 8 | e->par[2] << 16 | e->par[3] << 24), *c;
    u16 count = list[0] | list[1] << 8, j;
    u8 i, k, p = l->base.unk56;

    for (i = 0; i < gNumKirbys; i++) {
        struct Kirby *kirby = &gKirbys[i];
        s32 cx = kirby->base.x >> 12, cy = (kirby->base.y + 0x900) >> 12;

        if (kirby->base.roomId != l->room || gCurLevelInfo[i].currentRoom != l->room || !(kirby->base.unk62 & 4) || cx < 0 || cy < 0) continue;
        for (j = 0, c = list + 2; j < count; j++, c += 4) {
            if (c[0] != cx || c[1] != cy) continue;
            if (GetCollisionTile(p, cx, cy) != CRUMBLE_STAR) break;
            for (k = 0; k < CRUMBLES; k++)
                if (l->crumble[k].timer != 0 && l->crumble[k].x == cx && l->crumble[k].y == cy) break;
            if (k < CRUMBLES) break;
            for (k = 0; k < CRUMBLES; k++)
                if (l->crumble[k].timer == 0) {
                    l->crumble[k].x = cx;
                    l->crumble[k].y = cy;
                    l->crumble[k].timer = CRUMBLE_WAIT;
                    l->crumble[k].left = c[2];
                    break;
                }
            break;
        }
    }
    for (k = 0; k < CRUMBLES; k++) {
        struct SqueakCrumble *b = &l->crumble[k];

        if (b->timer == 0 || --b->timer != 0) continue;
        CrumbleBreak(p, b->x, b->y);
        if (b->left != 0) {
            b->y++;
            b->left--;
            b->timer = CRUMBLE_CHAIN;
        }
    }
}

/* PhaseWarp(); Squeak Squad's final stage (8-4) goes from part to part on its own, as the DS's stage switches (the owner: "Why is there
 * a door in daroach's first phase fight [...] the warp should send me to the next phase automatically", "the mirror to go to the next
 * phase is literally right there"): from Dark Daroach's room once he's beaten (LAIR_EXIT_WAIT frames after the room comes up, its
 * objects made, then once no mini-boss with HP is left in it, as soon as none is there at all: his defeat played out, its end the
 * DS's stage switch, squeak/daroach_boss.c's DEFEAT_END; counted PHASE_STEP frames a count, PHASE_DEFEAT_MAX counts at most: a
 * defeat that leaves its mini-boss there), into the dark star's chase; from where the ride after the chase lands (a8s4s5, whose
 * coming the port plays in Dark Nebula's own room) into his fight, PHASE_ARRIVAL frames after Kirby comes in. Every Kirby in the room
 * goes, as through a door (KatAM's room change, sub_08055920(): the one the shard cutscene ends in), onto the cell. Its byte: the
 * frames counted (after a fight: LAIR_EXIT_WAIT while the fight's on, then the counts from there), PHASE_GONE once gone; */
#define PHASE_STEP 4
#define PHASE_DEFEAT_MAX 200
#define PHASE_ARRIVAL 30
#define PHASE_GONE 0xFF
void sub_08055920(struct Kirby *);  // KatAM's room change (kirby.c): to kirby->roomId, onto spawnLocation;
void sub_080534D0(struct Kirby *);  // a flight's step (kirby.c): a warp star's ride;
static void PhaseWarp(struct SqueakLogic *l, const struct SqueakGimmickEntry *e) {
    u8 *state = &l->inside[e->index], i;
    u16 to = e->par[0] | e->par[1] << 8;

    if (*state == PHASE_GONE) return;
    if (e->par[4]) {
        if (*state < LAIR_EXIT_WAIT) { (*state)++; return; }
        if (*state == LAIR_EXIT_WAIT && MinibossLeft(l->room)) return;
        if (MinibossThere(l->room) && *state < LAIR_EXIT_WAIT + PHASE_DEFEAT_MAX) {
            if ((gFrameCount & (PHASE_STEP - 1)) == 0) (*state)++;
            return;
        }
    } else if (*state < PHASE_ARRIVAL) {
        // (counted once the ride that brought Kirby in has set him down: its flight, sub_080534D0, ended);
        if (gKirbys[0].base.roomId == l->room && gKirbys[0].stateFn != sub_080534D0) (*state)++;
        return;
    }
    *state = PHASE_GONE;
    for (i = 0; i < gNumKirbys; i++) {
        struct Kirby *kirby = &gKirbys[i];

        if (kirby->base.roomId != l->room || kirby->hp <= 0) continue;
        kirby->roomId = to;
        kirby->spawnLocation.x = e->par[2];
        kirby->spawnLocation.y = e->par[3];
        sub_08055920(kirby);
    }
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
        case GIMMICK_QUAKE: Quake(l, e); break;
        case GIMMICK_CAMERA: CameraArea(l, e); break;
        case GIMMICK_EMITTER: Emitter(l, e); break;
        case GIMMICK_STREAKS: Streaks(l, e); break;
        case GIMMICK_LIGHT: Light(l, e); break;
        case GIMMICK_SOUND: SoundGimmick(l, e); break;
        case GIMMICK_CHASE: ChaseWall(l, e); break;
        case GIMMICK_CHASE_SPAWN: ChaseSpawn(l, e); break;
        case GIMMICK_CHASE_CHEST: ChaseChest(l, e); break;
        case GIMMICK_CANDLE_LIGHT: CandleLight(l, e); break;
        case GIMMICK_LAIR_EXIT: LairExit(l, e); break;
        case GIMMICK_CURRENTS: Currents(l, e); break;
        case GIMMICK_CRUMBLES: Crumbles(l, e); break;
        case GIMMICK_PHASE_WARP: PhaseWarp(l, e); break;
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
    list = ListOf(roomId);
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

/* The small star switch (gimmick 0, 0x022254AC, vtable 0x02229B84): on the DS a body moving into it or an attack presses it (its
 * slot 16, 0x02225458, takes those messages in its states 0 and 3), and it sends its targets a message of kind 0 (0x022252F0: its
 * parameter's first byte's high nibble how many, then their indices). Its states (slots 3-7): 0 up; 1 pressed (its sound, a short
 * quake); 2 going down, then its message sent; 3 down, a body or an attack sending it again (0x02225434: a timed shutter's count
 * starts over), and rising once its targets made (at least one) are all back at rest, their state 0 (0x02225298, 0x0222524C):
 * a timed shutter shut again, so the switch can be pressed again (the owner: "Door switches should always reset"; a shutter that
 * stays open keeps it down for good, as on the DS); 4 coming up, then 0. Here its template's unk14 and unk16 are those 4 bytes; a
 * Kirby moving into it presses it, and so does an attack on it, a thrown one too (the owner: "If I throw a knife at them, they do
 * not activate which this doesn't happen in KatAM": the item holders' kind, HOLDER_KIND, and Struck(); not inhaled). Drawn with
 * Squeak Squad's switch_small (SQUEAK_SWITCH: its up frame, variant 0, then its pressed one, 1: squeak.c's SqueakAnimFor()); */
#define SQUEAK_SWITCH_ARMED 30
#define SQUEAK_SWITCH_DOWN 30       // (frames it's down before it may rise: its targets have taken its message by then);

struct SqueakButton {
    struct Object obj;
    bool8 touching;                 // (a Kirby touching it last frame: one moving into it presses it, standing on it doesn't again);
    u8 down;                        // (frames it's been down, up to SQUEAK_SWITCH_DOWN);
};

static bool32 Struck(struct Object *obj);
static bool32 SwitchTargetsRest(struct Object *obj);
static void SqueakSwitchUp(struct Object *obj);

// (a Kirby in its room within 12 pixels of it sideways, his middle up to 24 pixels over its spot or 8 under; positions in 1/256 pixels);
static bool32 SwitchTouched(struct Object *obj) {
    u8 i;

    for (i = 0; i < gNumKirbys; i++) {
        s32 dx = gKirbys[i].base.x - obj->base.x, dy = gKirbys[i].base.y - obj->base.y;

        if (gKirbys[i].base.roomId == obj->base.roomId && dx > -0xC00 && dx < 0xC00 && dy > -0x1800 && dy < 0x800) return TRUE;
    }
    return FALSE;
}

// (its message to its targets, the DS's 0x022252F0);
static void SwitchSend(struct Object *obj) {
    u8 par[4], i;

    par[0] = obj->objTemplate->unk14;
    par[1] = obj->objTemplate->unk14 >> 8;
    par[2] = obj->objTemplate->unk16;
    par[3] = obj->objTemplate->unk16 >> 8;
    for (i = 0; i < par[0] >> 4 && i < 3; i++) SqueakSendMail(SlotOf(&obj->base), par[1 + i], MESSAGE_SWITCH);
}

// (down: a Kirby moving into it or an attack sends its message again; up again once its targets rest, SqueakSwitchUp());
static void SqueakSwitchDown(struct Object *obj) {
    struct SqueakButton *b = (struct SqueakButton *)obj;
    bool32 touched = SwitchTouched(obj);

    if ((touched && !b->touching) || Struck(obj)) SwitchSend(obj);
    b->touching = touched;
    if (b->down < SQUEAK_SWITCH_DOWN) {
        b->down++;
        return;
    }
    if (!SwitchTargetsRest(obj)) return;
    obj->unk83 = 0;
    obj->unk78 = SqueakSwitchUp;
}

// (pressed: its targets messaged, its sound, down);
static void SqueakSwitchPress(struct Object *obj) {
    struct SqueakButton *b = (struct SqueakButton *)obj;

    SwitchSend(obj);
    PlaySfx(&obj->base, SE_SWITCH_ACTIVATE);
    obj->unk83 = 1;
    b->down = 0;
    obj->unk78 = SqueakSwitchDown;
}

static void SqueakSwitchUp(struct Object *obj) {
    struct SqueakButton *b = (struct SqueakButton *)obj;
    bool32 touched = SwitchTouched(obj);

    // (armed SQUEAK_SWITCH_ARMED frames after it's made: as the room comes up the Kirbys pass where it stands, 3-4-8's pressing then);
    if (obj->unk85 < SQUEAK_SWITCH_ARMED) {
        obj->unk85++;
        b->touching = touched;
        Struck(obj);
        return;
    }
    // (a Kirby moving into it; an attack on it: Kirby's, a thrown one, a helper's);
    if ((touched && !b->touching) || Struck(obj)) SqueakSwitchPress(obj);
    b->touching = touched;
}

static void *SqueakCreateButton(struct ObjectTemplate *template, u8 playerId) {
    struct Task *task = TaskCreate(ObjectMain, sizeof(struct SqueakButton), 0x1000, TASK_USE_IWRAM, ObjectDestroy);
    struct SqueakButton *b = TaskGetStructPtr(task);
    struct Object *obj = &b->obj;

    InitObject(obj, template, playerId);
    obj->base.flags |= 0x8900;
    // (attacks reach it as they do the item holders, harmless and deaf to a Kirby's body; not inhaled);
    obj->base.unk68 = 0;
    obj->base.unk5C = (obj->base.unk5C & ~7) | 1 | 0x80 | 0x20;
    ObjectSetBounds(&obj->base, -8, -8, 8, 8);
    sub_0803E2B0(&obj->base, -8, -8, 8, 8);
    obj->unk83 = 0;
    obj->unk85 = 0;   // (frames since it was made: the hammer peg's own count, which the game's object code leaves alone);
    b->touching = FALSE;
    b->down = 0;
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
 * as the DS draws them); a stone shutter's change shakes the screen (ShutterSound()); */
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
 * and knocks Kirby back, as the DS's spikes do) for the spikes; their own values back once it's open: the room's, from its template
 * (+0x16 on, KatAMExpansionSqueakRooms.cs), not read off the room as it shuts (made again over its own shut cells, it kept those). (KatAM's solid objects, its
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
            sub_0800255C(base->unk56, x, y, blocks ? (IsSpikes(s) ? SQUEAK_SPIKES : SQUEAK_SOLID) : s->kept[n]);
        }
    s->solid = blocks;
}

/* ShutterSound(); Its change's sound, and a stone pillar's quake (the DS's 0x02035FC4 with kind 2 for 22 frames, on its change and
 * its change back, 0x02224E3C and 0x02224D0C: KatAM's timed shake nearest it, QuakeShake(); the owner: "Stone shutters should shake
 * the screen like KatAM does for consistency sake"); */
#define SHUTTER_QUAKE_FRAMES 22
static void ShutterSound(struct SqueakShutter *s) {
    PlaySfx(&s->obj.base, SQUEAK_SE(IsSpikes(s) ? 0x2D : 0x14B)); // (the DS's: 0x02224D1C, 0x02224D28);
    if (!IsSpikes(s)) QuakeShake(SHUTTER_QUAKE_FRAMES, &s->obj.base);
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
    s->kept[0] = template->unk16;
    s->kept[1] = template->unk16 >> 8;
    s->kept[2] = template->unk18;
    s->kept[3] = template->unk18 >> 8;
    GimmickInit(obj, &s->spr, sShutterLooks[look]);
    // (not inhaled: the owner, "You can swallow stone doors which should not happen");
    obj->base.unk5C |= 0x20;
    ObjectSetBounds(&obj->base, sShutterBoxes[look][0], sShutterBoxes[look][1], sShutterBoxes[look][2], sShutterBoxes[look][3]);
    GimmickPlay(obj, &s->spr, sShutterAnims[s->par >> 3 & 1]);
    ShutterSetBox(s);
    obj->unk78 = SqueakShutterMain;
    return obj;
}

/* SwitchTargetsRest(); Whether a switch's targets made are all back at rest (the DS's 0x02225298 and 0x0222524C: at least one made,
 * none out of its state 0): the shutters among them, a message they haven't taken yet keeping them busy (a block group, cleared for
 * good, and a target not made yet aren't counted: a switch with only those stays down, as on the DS); */
static bool32 SwitchTargetsRest(struct Object *obj) {
    struct SqueakMail *mail = SqueakMailOf(SlotOf(&obj->base));
    struct Task *t = &gTasks[0];
    u8 count = (obj->objTemplate->unk14 & 0xFF) >> 4, made = 0, targets[3], i, n;

    targets[0] = obj->objTemplate->unk14 >> 8;
    targets[1] = obj->objTemplate->unk16;
    targets[2] = obj->objTemplate->unk16 >> 8;
    if (count > 3) count = 3;
    for (i = 0; i < count; i++)
        if (targets[i] < SQUEAK_MAIL_GIMMICKS && mail->box[targets[i]] != 0) return FALSE;
    for (n = 0; n < MAX_TASK_NUM && t->next; n++) {
        struct Object *o;

        t = (struct Task *)(IWRAM_START + t->next);
        o = TaskGetStructPtr(t);
        if (o->base.header.kind != 1 || o->base.roomId != obj->base.roomId || o->unk78 != SqueakShutterMain) continue;
        for (i = 0; i < count; i++) {
            if (((struct SqueakShutter *)o)->index != targets[i]) continue;
            if (((struct SqueakShutter *)o)->state != SHUTTER_WAIT) return FALSE;
            made++;
        }
    }
    return made > 0;
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
        PlaySfx(&obj->base, SQUEAK_SE(0x69)); // (the DS's 0x69);
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

/* (a chest it holds: the converter's chest template's fields, its item over its type in unk14, its mark and number in unk16, its
 * own number and its half flag in unk18 (chest.c's SqueakCreateChest: SQUEAK_CHEST in unk11, its number unk12, the half flag
 * unk14's high byte); its bottom on the holder's: KatAM's CreateChest() stands a chest's bottom on the cell edge at or under its
 * template's y + half its height, the converter's chests' template y their floor less 16 (a big one) or 8 (a small one));
 * 2-1-3's, 4-3-8's, 5-5-4's and 7-4-5's barrels and 4-1-6's grass hold one; */
#define HoldsChest(obj) (((obj)->objTemplate->unk14 & 0xFF) == OBJ_SMALL_CHEST || ((obj)->objTemplate->unk14 & 0xFF) == OBJ_BIG_CHEST)
static void LetOutChest(struct Object *obj) {
    const struct ObjectTemplate *t = obj->objTemplate;
    u8 type = t->unk14;
    s32 y = (obj->base.y >> 8) + (t->unk11 == SQUEAK_BARREL_OBJECT ? 32 : 0) - (type == OBJ_BIG_CHEST ? 16 : 8);

    CreateObjTemplateAndObj(obj->base.unk56, 1, 0x24, obj->base.x >> 8, y, 0, 0x1F, 0, 0, type, t->unk14 >> 8, 0, 0, 0, 0, 0, 0, 0,
                            SQUEAK_CHEST, t->unk18 & 0xFF, t->unk18 & 0xFF00, t->unk16, 0);
}

static void LetOut(struct Object *obj) {
    s32 x = obj->base.x >> 8, y = obj->base.y >> 8;
    u8 type = obj->objTemplate->unk14, behavior = obj->objTemplate->unk14 >> 8;
    struct Object *out;

    if (!type) return;
    if (HoldsChest(obj)) {
        LetOutChest(obj);
        return;
    }
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
            PlaySfx(&obj->base, SQUEAK_SE(0x59)); // (the DS's 0x59, its cut);
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
        // (one that held a chest stays cut: grown back, it would let out the chest again);
        if (!(obj->objTemplate->unk14 & 0xFF) || HoldsChest(obj)) break;
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
 * world); one holding something starts shut, an empty one open; an attack bursts it (message 0xB, sound 0x51), letting out what it
 * holds, and it's gone. An open one hides Kirby (its states, vtable 0x02229594's: 1-2 a Kirby got in, the lid shutting then shut;
 * 6-7 he pressed a way, the barrel shaking then shut; 3-4 he pressed A, the lid opening, he's out, then open; 5 none gets in for
 * 0x1E frames after): a Kirby on the floor in front of it pressing Down gets in, as he would into KatAM's cannon (sub_08051C40:
 * hidden, out of reach of anything, held at the object in his unk6C: here the barrel's bottom, BarrelHold); A lets him out, hopping
 * up into KatAM's own fall (sub_08044EA8: seen again, his collisions back, gravity), a jump's speed up. One with a Kirby in isn't
 * burst. Only empty ones open: 8-1-4's two; */
struct SqueakBarrel {
    struct Object obj;
    struct SqueakGimmickSprite spr;
    bool8 burst;
    u8 rider;   // the Kirby in it + 1, 0 none;
    u8 wait;    // frames before a Kirby gets in again;
    u8 timer;   // frames left of the lid's move or the shake (animations 1, 3, 4, which loop in their files);
    u8 down;    // the Kirbys holding Down last frame (a bit each): a Kirby's own new-press bits (unk11A) are taken by his crouch
                // before the barrel's turn when he stands;
};
#define BARREL_WAIT 0x1E
#define BARREL_HOP 0x300
#define BARREL_REACH 12       // a Kirby's middle within this of its middle, across;
#define BARREL_KEY_DOWN 0x80
#define BARREL_KEY_A 1
#define BARREL_KEYS_WAY 0xF0

extern bool8 sub_08051C40(struct Kirby *);
extern void sub_08044EA8(struct Kirby *);

// (an animation's length in frames, once through; at most 60);
static u8 BarrelAnimLength(struct SqueakGimmickSprite *spr, u8 anim) {
    const struct SqueakAnim *a = &gSqueakData.enemies[spr->look].anim[anim];
    u16 n = 0;
    u8 i;

    for (i = 0; i < a->steps; i++) n += a->step[i].frames;
    return n > 60 ? 60 : n;
}

static void BarrelPlay(struct SqueakBarrel *b, u8 anim) {
    GimmickPlay(&b->obj, &b->spr, anim);
    b->timer = BarrelAnimLength(&b->spr, anim);
}

// (a Kirby in a barrel: at its bottom, under the lid, as KatAM's cannon holds him, sub_0805C018);
static void BarrelHold(struct Kirby *kirby) {
    struct ObjectBase *barrel = kirby->base.unk6C;

    kirby->base.x = barrel->x;
    kirby->base.y = barrel->y + 0x1800;
}

static void BarrelLetOutKirby(struct Object *obj, struct Kirby *kirby) {
    kirby->base.x = obj->base.x;
    kirby->base.y = obj->base.y + 0x1000;
    sub_08044EA8(kirby);
    kirby->base.yspeed = BARREL_HOP;
}

static void BarrelRide(struct SqueakBarrel *b) {
    struct Object *obj = &b->obj;
    struct Kirby *kirby = &gKirbys[b->rider - 1];

    // (a Kirby the game took elsewhere: hurt, a room left, the end of the stage);
    if (kirby->stateFn != BarrelHold || kirby->base.roomId != obj->base.roomId) {
        b->rider = 0;
        GimmickPlay(obj, &b->spr, 0);
        return;
    }
    if (b->timer) b->timer--;
    if ((b->spr.anim == 1 || b->spr.anim == 4) && (b->spr.ended || !b->timer)) GimmickPlay(obj, &b->spr, 2);
    if (kirby->unk11A & BARREL_KEY_A) {
        BarrelLetOutKirby(obj, kirby);
        b->rider = 0;
        BarrelPlay(b, 3);
        return;
    }
    if (kirby->movementState & 0x30) {
        if (kirby->movementState & 0x20) kirby->base.flags |= 1;
        else kirby->base.flags &= ~1;
    }
    if (kirby->unk11A & BARREL_KEYS_WAY && b->spr.anim == 2) BarrelPlay(b, 4);
}

static void BarrelTakeKirby(struct SqueakBarrel *b) {
    struct Object *obj = &b->obj;
    u8 i;

    for (i = 0; i < gNumHumanPlayers; i++) {
        struct Kirby *kirby = &gKirbys[i];
        s32 dx = (kirby->base.x - obj->base.x) >> 8, dy = (kirby->base.y - obj->base.y) >> 8;

        bool32 pressed = kirby->movementState & BARREL_KEY_DOWN && !(b->down & (1 << i));

        if (kirby->base.roomId != obj->base.roomId || !pressed || !(kirby->base.unk62 & 4)) continue;
        if (dx < -BARREL_REACH || dx > BARREL_REACH || dy < 8 || dy > 40) continue;
        if (!sub_08051C40(kirby)) continue;
        kirby->base.unk6C = &obj->base;
        kirby->stateFn = BarrelHold;
        b->rider = i + 1;
        BarrelPlay(b, 1);
        return;
    }
}

static void SqueakBarrelMain(struct Object *obj) {
    struct SqueakBarrel *b = (struct SqueakBarrel *)obj;
    u8 i, down = 0;

    if (gUnk_03000510.unk4 & ((1 << obj->base.unk56) | 0x10)) return;
    GimmickAnimate(obj, &b->spr);
    if (b->burst) {
        if (b->spr.ended) obj->base.flags |= 0x1000;
        return;
    }
    for (i = 0; i < gNumHumanPlayers; i++)
        if (gKirbys[i].movementState & BARREL_KEY_DOWN) down |= 1 << i;
    if (b->rider) {
        obj->base.flags &= ~HOLDER_HIT;
        BarrelRide(b);
        if (!b->rider) b->wait = BARREL_WAIT;
        b->down = down;
        return;
    }
    if (Struck(obj)) {
        LetOut(obj);
        PlaySfx(&obj->base, SQUEAK_SE(0x51)); // (the DS's 0x51);
        GimmickPlay(obj, &b->spr, 5);
        b->burst = TRUE;
        obj->base.unk5C &= ~7;
        return;
    }
    if (obj->objTemplate->unk14 & 0xFF) return;
    if (b->timer) b->timer--;
    if (b->spr.anim == 3 && (b->spr.ended || !b->timer)) GimmickPlay(obj, &b->spr, 0);
    if (b->wait) {
        b->wait--;
        b->down = down;
        return;
    }
    if (b->spr.anim == 0) BarrelTakeKirby(b);
    b->down = down;
}

static void *SqueakCreateBarrel(struct ObjectTemplate *template, u8 playerId) {
    struct Task *task = TaskCreate(ObjectMain, sizeof(struct SqueakBarrel), 0x1000, TASK_USE_EWRAM, ObjectDestroy);
    struct SqueakBarrel *b = TaskGetStructPtr(task);
    struct Object *obj = &b->obj;

    InitObject(obj, template, playerId);
    HolderInit(obj, &b->spr, SQUEAK_BARREL, -16, 0, 16, 32);
    b->burst = FALSE;
    b->rider = 0;
    b->wait = 0;
    b->timer = 0;
    b->down = 0;
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
            PlaySfx(&obj->base, SQUEAK_SE(0x54)); // (the DS's 0x54, its shatter: 0x022235E2);
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
            PlaySfx(&obj->base, SQUEAK_SE(0x4B)); // (the DS's 0x4B, its break: 0x02221898);
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

/* The moving platforms (gimmick 4, 0x02223D78, vtable 0x02229898; graphics yakumono/floor, look 0 its animation 0 and look 1 its 3).
 * Its first word (the template's unk14): bits 0-3 its look; 4-6 how it starts (0 and 1 at once, the DS's state 1; 2 when a Kirby
 * stands on it, message 6; 3 on a switch's or an area's message, 0-2); 7-9 what it does after its last leg (0 stops; 1 shakes, falls
 * and is gone, its states 4-7: the shake 0x02227024's +2, -4, +3, -2, +1 pixels, the fall 0x02227304's 0.2 a frame; 2 goes back to
 * its start in one leg, again and again; 3 goes back along its legs, again and again: the constructor adds those legs); 10-11 its
 * speed (0x02227DB0: 1, 2, 4 pixels a frame); 12-14 its legs (3 at most in the game: the template's unk16, unk18, unk1A, each two
 * signed bytes, cells across and up), each a straight run at its speed. Its box (-1.5, -0.25)-(1.5, 0.5) cells: 48 pixels wide, its
 * top 8 pixels over its spot.
 * The carrier: KatAM has no moving floor (its solid objects are terrain cells), so the platform holds its riders itself, as a Kirby
 * landing on terrain is held (Macro_080063B0): its task runs before the Kirbys' (priority 0x1000 to their 0x3000), moves, then
 * carries each Kirby on it the same distance, puts his feet (his y + his box's bottom, unk3F) on its top and marks him on the floor
 * (unk62's 4, his fall speed 0), so his own update that frame walks, ducks or jumps from it as from the ground; a Kirby is on it when
 * he was last frame and hasn't jumped or walked off, or when his feet came down through its top since his last update. The room's
 * other bodies on the terrain (the DS's floors hold every body: the owner, "Entities go through the movable platforms, which doesn't
 * happen in Squeak Squad") ride it the same way (PlatformCarryObjects(): KatAM's objects that collide with the terrain, run after
 * it too); it isn't inhaled (unk5C 0x20: the owner swallowed one); */
struct SqueakPlatform {
    struct Object obj;
    struct SqueakGimmickSprite spr;
    u8 index;
    u8 state;
    u8 leg;
    u8 legs;
    u8 riders;
    u8 speed;
    u16 frame;
    u16 frames;
    u16 timer;
    s8 path[6][2];
    s32 fromX, fromY;
    s32 vx, vy;
};
enum { PLATFORM_WAIT, PLATFORM_MOVE, PLATFORM_STOP, PLATFORM_SHAKE, PLATFORM_FALL, PLATFORM_GONE };
#define PLATFORM_HALF 24
#define PLATFORM_TOP 8
#define PLATFORM_FALL_GRAVITY 0x33
#define PLATFORM_FALL_MOST 160
#define PLATFORM_START_STEP 2
#define PLATFORM_START_MAIL 3
#define PLATFORM_END_STOP 0
#define PLATFORM_END_BREAK 1
#define PLATFORM_END_RETURN 2
static const s8 sPlatformShake[13] = { 2, 2, 2, -2, -2, -2, 1, 1, 1, -1, -1, 0, 0 };
static const u8 sPlatformSpeeds[4] = { 1, 2, 4, 4 };

// (the next leg, or what it does at the end: FALSE when it stops or breaks);
static bool32 PlatformNextLeg(struct SqueakPlatform *p) {
    s32 x, y, ax, ay;

    if (p->leg >= p->legs) {
        u8 end = p->obj.objTemplate->unk14 >> 7 & 7;

        // (a rope floor: no legs, it breaks; 0x022219C4's states 1-3);
        if (end == PLATFORM_END_BREAK) {
            p->state = PLATFORM_SHAKE;
            p->timer = 0;
            return FALSE;
        }
        if (end == PLATFORM_END_STOP || p->legs == 0) {
            p->state = PLATFORM_STOP;
            return FALSE;
        }
        p->leg = 0;
    }
    x = p->path[p->leg][0] * 16;
    y = -p->path[p->leg][1] * 16;
    p->leg++;
    ax = x < 0 ? -x : x;
    ay = y < 0 ? -y : y;
    // (its length in pixels, near enough: the longer side and 3/8 of the shorter);
    p->frames = ((ax > ay ? ax + ay * 3 / 8 : ay + ax * 3 / 8) + p->speed - 1) / p->speed;
    if (p->frames == 0) p->frames = 1;
    p->frame = 0;
    p->fromX = p->obj.base.x;
    p->fromY = p->obj.base.y;
    p->vx = x << 8;
    p->vy = y << 8;
    return TRUE;
}

/* (the room's other bodies standing on it: KatAM's objects (the tasks' structs of header kind 1) in its room that collide with the
 * terrain (not flags 0x100) and move (not 0x800), not going (0x1000), not rising (an object's yspeed is up), over it, whose feet were
 * on or over its top last frame (unk4C, their y before their last move) and are at it now (a rider's at its old top, which it may
 * have left by a step either way, a lander's through it): carried with it, their feet on its top, on the floor (unk62's 4), their
 * fall stopped; walked off its end, they fall); */
static void PlatformCarryObjects(struct SqueakPlatform *p, s32 oldTop, s32 dx) {
    struct Task *t = &gTasks[0];
    s32 top = p->obj.base.y - (PLATFORM_TOP << 8), low = oldTop < top ? oldTop : top, high = oldTop > top ? oldTop : top;
    u8 n;

    for (n = 0; n < MAX_TASK_NUM && t->next; n++) {
        struct Object *o;
        s32 ax, foot, before;
        bool32 chest;

        t = (struct Task *)(IWRAM_START + t->next);
        o = TaskGetStructPtr(t);
        if (o == &p->obj || o->base.header.kind != 1 || o->base.roomId != p->obj.base.roomId) continue;
        chest = o->type == OBJ_SMALL_CHEST || o->type == OBJ_BIG_CHEST;
        // (a still object, flags 0x800, but a chest standing on it: the owner, "Some chests on top of movable platforms are not
        // moving with the platforms");
        if (o->base.flags & (0x100 | 0x1000) || o->base.yspeed > 0) continue;
        if (o->base.flags & 0x800 && !chest) continue;
        // (a Squeak Squad body moves itself, squeak.c's SqueakMove(): its own speed, up positive; its landing is there too);
        if (SqueakIsSqueak(o) && ((struct SqueakObject *)o)->vy > 0) continue;
        ax = (o->base.x - p->obj.base.x + dx) >> 8;
        if (ax < 0) ax = -ax;
        if (ax > PLATFORM_HALF + 4) continue;
        foot = o->base.y + (o->base.unk3F << 8);
        before = o->base.unk4C + (o->base.unk3F << 8);
        /* (a chest, its bottom on the cell edge at or under the platform's top where KatAM's CreateChest() put it, up to 12 pixels
         * under it: held there from its first frame, before it falls through, KatAM's chests falling as any object); */
        if (chest) {
            if (foot < oldTop - (4 << 8) || foot > oldTop + (12 << 8)) continue;
        } else if (before > oldTop + (2 << 8) || foot < low - (4 << 8) || foot > high + (16 << 8)) continue;
        o->base.x += dx;
        o->base.y = top - (o->base.unk3F << 8);
        o->base.unk62 |= 4;
        o->base.yspeed = 0;
    }
}

// (whether a Kirby stands on it now: on it last frame and still over it, or his feet came down through its top);
static void PlatformCarry(struct SqueakPlatform *p, s32 oldTop, s32 dx, s32 dy) {
    s32 top = p->obj.base.y - (PLATFORM_TOP << 8);
    u8 i, riders = 0;

    for (i = 0; i < gNumKirbys; i++) {
        struct Kirby *k = &gKirbys[i];
        s32 ax = (k->base.x - p->obj.base.x + dx) >> 8, foot = k->base.y + (k->base.unk3F << 8);

        /* (one riding it stays on while an attack holds him still: KatAM's 0x800 and 0x2000000 midway through a sword swing, with
         * which he was left in the air as it moved on: the owner, "If an attack is used on up and down movable platforms, you just
         * go through it"; one not on it isn't taken on in such a state); */
        if (k->base.roomId != p->obj.base.roomId || k->hp <= 0 || k->base.flags & 0x100) continue;
        if (!(p->riders & (1 << i)) && k->base.flags & (0x800 | 0x2000000)) continue;
        if (ax < 0) ax = -ax;
        if (k->base.yspeed > 0 || ax > PLATFORM_HALF + 4) continue;
        if (p->riders & (1 << i)) {
            if (foot < oldTop - (4 << 8) || foot > oldTop + (8 << 8)) continue;
        } else {
            s32 before = k->base.unk4C + (k->base.unk3F << 8);

            if (before > oldTop + (2 << 8) || foot < top - (2 << 8) || foot > top + (16 << 8)) continue;
        }
        k->base.x += dx;
        k->base.y = top - (k->base.unk3F << 8);
        k->base.unk62 |= 4;
        k->base.yspeed = 0;
        k->movementOverride.y = 0;
        if (!(k->base.flags & 0x40)) k->base.flags &= ~0x1060;
        riders |= 1 << i;
    }
    p->riders = riders;
    PlatformCarryObjects(p, oldTop, dx);
}

static void SqueakPlatformMain(struct Object *obj) {
    struct SqueakPlatform *p = (struct SqueakPlatform *)obj;
    s32 x = obj->base.x, y = obj->base.y, oldTop = y - (PLATFORM_TOP << 8);
    u8 kind;

    if (gUnk_03000510.unk4 & ((1 << obj->base.unk56) | 0x10)) return;
    kind = TakeMail(SlotOf(&obj->base), p->index);
    switch (p->state) {
    case PLATFORM_WAIT:
        if ((obj->objTemplate->unk14 >> 4 & 7) == PLATFORM_START_STEP ? p->riders != 0 : kind != 0 && kind <= MESSAGE_AREA + 1)
            if (PlatformNextLeg(p)) p->state = PLATFORM_MOVE;
        break;
    case PLATFORM_MOVE:
        if (p->frame >= p->frames && !PlatformNextLeg(p)) break;
        p->frame++;
        obj->base.x = p->fromX + p->vx * p->frame / p->frames;
        obj->base.y = p->fromY + p->vy * p->frame / p->frames;
        break;
    case PLATFORM_SHAKE:
        obj->base.x += sPlatformShake[p->timer] << 8;
        if (++p->timer < ARRAY_COUNT(sPlatformShake)) break;
        p->state = PLATFORM_FALL;
        p->timer = 0;
        obj->base.yspeed = 0;
        break;
    case PLATFORM_FALL:
        obj->base.yspeed += PLATFORM_FALL_GRAVITY;
        obj->base.y += obj->base.yspeed;
        // (it breaks where it lands, the DS's state 3: a puff, sound 0x4B; or once fallen out of the room);
        if (GetCollisionTile(obj->base.unk56, obj->base.x >> 12, ((obj->base.y >> 8) + 4) >> 4) - 1u < 0x1F)
            PlaySfx(&obj->base, SQUEAK_SE(0x4B)); // (the DS's 0x4B);
        else if (++p->timer < PLATFORM_FALL_MOST && obj->base.y >> 8 < gCurLevelInfo[obj->base.unk56].roomHeight + 32) break;
        obj->base.flags |= 0x400;
        p->state = PLATFORM_GONE;
        p->riders = 0;
        return;
    case PLATFORM_GONE:
        return;
    }
    PlatformCarry(p, oldTop, obj->base.x - x, obj->base.y - y);
}

/* SqueakPlatformLanding(); A Squeak Squad body's move (squeak.c's SqueakMove(), after KatAM's terrain pass) coming down onto a platform
 * or a thunder cloud of its room (its feet on or over its top before the move, `before` its y then, and at it or a step through it
 * now: the platform may have moved since), or standing on one: its feet put on its top and it's on the floor (unk62's 4), as
 * on the terrain; the platform carries it on as it moves (PlatformCarryObjects()); */
static void SqueakCloudMain(struct Object *obj);
bool32 SqueakPlatformLanding(struct ObjectBase *b, s32 before) {
    struct Task *t = &gTasks[0];
    u8 n;

    for (n = 0; n < MAX_TASK_NUM && t->next; n++) {
        struct Object *o;
        s32 ax, top, foot;

        t = (struct Task *)(IWRAM_START + t->next);
        o = TaskGetStructPtr(t);
        if (o->base.header.kind != 1 || o->base.roomId != b->roomId || (o->unk78 != SqueakPlatformMain && o->unk78 != SqueakCloudMain)) continue;
        if (((struct SqueakPlatform *)o)->state == PLATFORM_GONE) continue;
        ax = (b->x - o->base.x) >> 8;
        if (ax < 0) ax = -ax;
        if (ax > PLATFORM_HALF + 4) continue;
        top = o->base.y - (PLATFORM_TOP << 8);
        foot = b->y + (b->unk3F << 8);
        if (before + (b->unk3F << 8) > top + (4 << 8) || foot < top - (6 << 8) || foot > top + (16 << 8)) continue;
        b->y = top - (b->unk3F << 8);
        b->unk62 |= 4;
        return TRUE;
    }
    return FALSE;
}

// (its way back: in one leg, or along its legs);
static void PlatformReturnLegs(struct SqueakPlatform *p, u16 word, u8 n) {
    u8 i;

    switch (word >> 7 & 7) {
    case PLATFORM_END_RETURN: {
        s32 sx = 0, sy = 0;

        for (i = 0; i < n; i++) sx += p->path[i][0], sy += p->path[i][1];
        p->path[p->legs][0] = -sx;
        p->path[p->legs][1] = -sy;
        p->legs++;
        break;
    }
    case PLATFORM_END_RETURN + 1:
        for (i = n; i-- > 0;) {
            p->path[p->legs][0] = -p->path[i][0];
            p->path[p->legs][1] = -p->path[i][1];
            p->legs++;
        }
        break;
    }
}

static void *SqueakCreatePlatform(struct ObjectTemplate *template, u8 playerId) {
    struct Task *task = TaskCreate(ObjectMain, sizeof(struct SqueakPlatform), 0x1000, TASK_USE_EWRAM, ObjectDestroy);
    struct SqueakPlatform *p = TaskGetStructPtr(task);
    struct Object *obj = &p->obj;
    u16 word = template->unk14;
    const u16 *legs = &template->unk16;
    u8 i, n = word >> 12 & 7;

    InitObject(obj, template, playerId);
    p->index = template->unk12;
    GimmickInit(obj, &p->spr, SQUEAK_FLOOR);
    ObjectSetBounds(&obj->base, -PLATFORM_HALF, -PLATFORM_TOP, PLATFORM_HALF, 4);
    obj->base.unk5C |= 0x20;
    p->speed = sPlatformSpeeds[word >> 10 & 3];
    if (n > 3) n = 3;
    for (i = 0; i < n; i++) {
        p->path[i][0] = (s8)legs[i];
        p->path[i][1] = (s8)(legs[i] >> 8);
    }
    p->legs = n;
    PlatformReturnLegs(p, word, n);
    p->state = PLATFORM_WAIT;
    if ((word >> 4 & 7) <= 1 && PlatformNextLeg(p)) p->state = PLATFORM_MOVE;
    GimmickPlay(obj, &p->spr, (word & 15) == 1 ? 3 : 0);
    obj->unk78 = SqueakPlatformMain;
    return obj;
}

/* The thunder clouds (gimmick 5, ctor 0x02220C00, vtable 0x02229230; yakumono/thunder_cloud): a moving platform's first word and
 * legs (its legs from its 6th byte; the same straight runs, 0x022270A4, at the speeds of its table 0x02227BA0), so here a moving
 * platform with the cloud's look. The DS plays only its .mca's 4 composites (its look's table 0x02227BAC: 0x3000000-3), here
 * animations 14-17 (KatAMExpansionSqueak.cs's SqueakFlattenComposites): 14 resting (147 frames, looping), 15 the zap (its green
 * burst and sparks, 12 frames, looping), 16 the shake before it breaks, 17 breaking up. Its zaps (its +0x5C phases 0-6): an attack
 * on it (message 0xB, 0x02220ADC to 0x02220B0C) sets phase 1 while it rests, or 2 while it zaps (zapping on); as each loop of its
 * animation ends (0x02220A24): phase 1 starts the zap (harm kind 0x0B, strength 8: +0x48, +0x4C) and 2, phases 2-5 go on to the
 * next (the thunder, sound 0x11F, on 3 and 5), phase 6 back to rest and 0: a zap is 5 of its loops, 60 frames, starting once the
 * resting loop it was struck in ends. It has no timer of its own: it rests until struck (its +0x60 counter is only reset there).
 * The zap hurts in its burst's box, ±40 pixels (yakumono/thunder_cloud.rca's attack boxes, ±40 and ±42), riders too (a plain hit).
 * Its break (a platform's end 1: states 4-6, 0x02220854-0x022207EC): 30 frames of animation 16 where it is, then 17 (sound 0x29)
 * and it's gone, riders dropped; */
struct SqueakCloud {
    struct SqueakPlatform p;
    u8 phase;
    u16 clock;
};
#define CLOUD_REST_ANIM 14
#define CLOUD_ZAP_ANIM 15
#define CLOUD_SHAKE_ANIM 16
#define CLOUD_BREAK_ANIM 17
#define CLOUD_ZAP_HALF 40
#define CLOUD_ZAP_LAST 6
#define CLOUD_SHAKE_FRAMES 30
enum { CLOUD_SOLID, CLOUD_SHAKING, CLOUD_BREAKING };

// (whether the animation's loop ends with this frame: its last step's last frame);
static bool32 GimmickLoopEnds(struct SqueakGimmickSprite *spr) {
    const struct SqueakAnim *a = &gSqueakData.enemies[spr->look].anim[spr->anim];

    return a->loops && spr->frames <= 1 && spr->step + 1 >= a->steps;
}

static void CloudBox(struct Object *obj, bool32 zap) {
    if (zap) sub_0803E2B0(&obj->base, -CLOUD_ZAP_HALF, -CLOUD_ZAP_HALF, CLOUD_ZAP_HALF, CLOUD_ZAP_HALF);
    else sub_0803E2B0(&obj->base, -PLATFORM_HALF, -PLATFORM_TOP - 4, PLATFORM_HALF, 8);
}

static void SqueakCloudMain(struct Object *obj) {
    struct SqueakCloud *c = (struct SqueakCloud *)obj;
    bool32 loopEnds;

    if (gUnk_03000510.unk4 & ((1 << obj->base.unk56) | 0x10)) return;
    // (its break: shaking where it is, then breaking up, gone with its animation);
    if (c->p.state == PLATFORM_SHAKE || c->p.state == PLATFORM_FALL) {
        obj->base.unk68 &= ~(7 | 0x80);
        if (c->clock == 0) {
            CloudBox(obj, FALSE);
            GimmickPlay(obj, &c->p.spr, CLOUD_SHAKE_ANIM);
        }
        GimmickAnimate(obj, &c->p.spr);
        if (++c->clock == CLOUD_SHAKE_FRAMES) {
            GimmickPlay(obj, &c->p.spr, CLOUD_BREAK_ANIM);
            PlaySfx(&obj->base, SQUEAK_SE(0x29)); // (the DS's 0x29);
            c->p.riders = 0;
            c->p.state = PLATFORM_FALL;
        }
        if (c->clock < CLOUD_SHAKE_FRAMES) {
            // (it holds its riders where it is while it shakes);
            s32 top = obj->base.y - (PLATFORM_TOP << 8);
            PlatformCarry(&c->p, top, 0, 0);
        } else if (c->p.spr.ended) {
            obj->base.flags |= 0x400;
            c->p.state = PLATFORM_GONE;
        }
        return;
    }
    if (c->p.state == PLATFORM_GONE) return;

    // (struck: its zap after the loop it's in, or a zap going on again);
    if (Struck(obj)) c->phase = c->phase <= 1 ? 1 : 2;
    loopEnds = GimmickLoopEnds(&c->p.spr);
    GimmickAnimate(obj, &c->p.spr);
    if (loopEnds && c->phase != 0) {
        if (c->phase == 1) {
            GimmickPlay(obj, &c->p.spr, CLOUD_ZAP_ANIM);
            CloudBox(obj, TRUE);
            c->phase = 2;
        } else if (c->phase < CLOUD_ZAP_LAST) {
            c->phase++;
            if (c->phase == 3 || c->phase == 5) PlaySfx(&obj->base, SQUEAK_SE(0x11F)); // (the DS's 0x11F);
        } else {
            GimmickPlay(obj, &c->p.spr, CLOUD_REST_ANIM);
            CloudBox(obj, FALSE);
            c->phase = 0;
        }
    }
    obj->base.unk68 &= ~(7 | 0x80);
    if (c->phase >= 2) obj->base.unk68 |= 3 | 0x80;
    SqueakPlatformMain(obj);
    // (a cloud's break is its own: above);
    if (c->p.state == PLATFORM_SHAKE) c->clock = 0;
}

static void *SqueakCreateCloud(struct ObjectTemplate *template, u8 playerId) {
    struct Task *task = TaskCreate(ObjectMain, sizeof(struct SqueakCloud), 0x1000, TASK_USE_EWRAM, ObjectDestroy);
    struct SqueakCloud *c = TaskGetStructPtr(task);
    struct Object *obj = &c->p.obj;
    u16 word = template->unk14;
    const u16 *legs = &template->unk16;
    u8 i, n = word >> 12 & 7;

    InitObject(obj, template, playerId);
    c->p.index = template->unk12;
    GimmickInit(obj, &c->p.spr, SQUEAK_THUNDER);
    // (attacks reach it, as the grass tufts and ropes: Struck());
    obj->base.unk5C = (obj->base.unk5C & ~7) | HOLDER_KIND | 0x20;
    ObjectSetBounds(&obj->base, -PLATFORM_HALF, -PLATFORM_TOP, PLATFORM_HALF, 4);
    CloudBox(obj, FALSE);
    c->p.speed = sPlatformSpeeds[word >> 10 & 3];
    if (n > 3) n = 3;
    for (i = 0; i < n; i++) {
        c->p.path[i][0] = (s8)legs[i];
        c->p.path[i][1] = (s8)(legs[i] >> 8);
    }
    c->p.legs = n;
    PlatformReturnLegs(&c->p, word, n);
    c->p.state = PLATFORM_WAIT;
    if ((word >> 4 & 7) <= 1 && PlatformNextLeg(&c->p)) c->p.state = PLATFORM_MOVE;
    c->phase = 0;
    c->clock = 0;
    GimmickPlay(obj, &c->p.spr, CLOUD_REST_ANIM);
    obj->unk78 = SqueakCloudMain;
    return obj;
}

// (the Kirby behind an attack: a Kirby itself, or what one let go, its parent);
static struct Kirby *AttackKirby(struct ObjectBase *by) {
    if (by == NULL) return NULL;
    if ((u32)by - (u32)gKirbys < 4 * sizeof(struct Kirby)) return (struct Kirby *)by;
    by = by->parent;
    if (by != NULL && (u32)by - (u32)gKirbys < 4 * sizeof(struct Kirby)) return (struct Kirby *)by;
    return NULL;
}

/* The ropes (gimmick 1, ctor 0x022249C8, vtable 0x02229A2C): no sprite (the rope is the terrain's: its front layer's blocks 0x260-0x262
 * over the blocks grid's rope cells, ids 0x40-0x42 under top byte 0x38; a KatAM ladder here), an area 4 pixels wide (table
 * 0x02229A20 by its first byte's low nibble: pixels 6-9 of its cell) and its second byte's pixels long, down from its spot. A
 * cutting attack on it (message 0xB with the attack's flag 8, its check 0x02224960) cuts it (state 1, 0x022248A0: sound 0x48 or 0x49
 * by its look) and messages its targets (its first byte's high nibble how many, from its third byte): the floor it holds (gimmick
 * 3), 8-1-8's rock relay; and the rope goes (the stage's 0x022164C0: from the cut, up then down, each cell whose id is a rope's gets
 * a short fade, 0x022124F8's tiles 0x10-0x14, and is cleared, 0x02215830, one further out each frame). Here (the template's unk14,
 * unk16, unk18 its 6 bytes) an object hidden, its box over the rope's cells (2-14 pixels across), taking any attack as the grass tufts
 * do (Struck()), but cutting only as the DS's does: a Kirby's with a blade, Cutter, Sword or Master, or Needle (AttackKirby(); not his
 * inhale or a plain hit, which dropped its floor here before); once; not inhaled (unk5C 0x20); its cells go from the cut (the attacker's row, in the rope) out,
 * ROPE_CLEAR_STEP frames apart, each way till a cell isn't a ladder: each one's block record (KatAMExpansionSqueakRooms.cs's SqueakRopeCells(): the cell drawn without
 * its front layer, air) applied as a broken block's, so its look goes and Kirby can't climb it; */
struct SqueakRope {
    struct Object obj;
    struct SqueakGimmickSprite spr;
    u8 ways;                    // (cut, its cells going: ROPE_UP, ROPE_DOWN the ways that still have some);
    u8 step;                    // (how far from the cut its cells have gone; with ways 0, cut and gone);
    u8 row;                     // (the cut's row);
    u8 wait;
};
#define ROPE_TARGETS 3
#define ROPE_CLEAR_STEP 2
#define ROPE_UP 1
#define ROPE_DOWN 2
#define ROPE_LADDER_TOP 0x20
#define ROPE_LADDER 0x21

// (a cell's block record of type 1 matching its collision now, or NULL: KatAM's own search, sub_080025AC, never ends without one);
static const u8 *CellRecord(u8 p, u8 x, u8 y) {
    const u8 *record;
    u8 value = GetCollisionTile(p, x, y);

    for (record = (const u8 *)gCurLevelInfo[p].unk1E0.pat1; record[4] != 0; record += record[4])
        if (record[0] == 1 && record[1] == value && record[2] == x && record[3] == y) return record;
    return NULL;
}

// (the rope's cell in a row, if it's one still standing, a ladder with its record: cleared; FALSE where the rope ends);
static bool32 RopeClear(struct Object *obj, s32 y) {
    u8 p = obj->base.unk56, x = obj->base.x >> 12, value;
    const u8 *record;

    if (y < 0 || y >= gCurLevelInfo[p].roomHeight / 16) return FALSE;
    value = GetCollisionTile(p, x, y);
    if ((value != ROPE_LADDER_TOP && value != ROPE_LADDER) || (record = CellRecord(p, x, y)) == NULL) return FALSE;
    sub_08001408(p, (struct Unk_0888562C *)record, NULL, NULL);
    return TRUE;
}

// (a cutting attack, the DS's flag 8: a Kirby's with a blade, or with Needle (the owner: "Needle counts as a cutting ability."));
static bool32 RopeCutter(struct Kirby *k) {
    u8 ability = k != NULL ? k->ability & NIDL_ABILITY_MASK : KIRBY_ABILITY_NORMAL;

    return ability == KIRBY_ABILITY_CUTTER || ability == KIRBY_ABILITY_SWORD || ability == KIRBY_ABILITY_MASTER
        || ability == NIDL_ABILITY_NEEDLE;
}

static void SqueakRopeMain(struct Object *obj) {
    struct SqueakRope *r = (struct SqueakRope *)obj;
    u8 par[6], i;
    s32 top, bottom, row;

    if (gUnk_03000510.unk4 & ((1 << obj->base.unk56) | 0x10)) return;
    // (cut: its cells go, the cut's first, then one further out each way each ROPE_CLEAR_STEP frames, till neither way has any);
    if (r->ways) {
        if (++r->wait < ROPE_CLEAR_STEP) return;
        r->wait = 0;
        if (r->ways & ROPE_UP && !RopeClear(obj, (s32)r->row - r->step)) r->ways &= ~ROPE_UP;
        if (r->ways & ROPE_DOWN && r->step > 0 && !RopeClear(obj, (s32)r->row + r->step)) r->ways &= ~ROPE_DOWN;
        r->step++;
        return;
    }
    if (r->step || !Struck(obj) || !RopeCutter(AttackKirby(obj->base.unk6C))) return;
    par[0] = obj->objTemplate->unk14;
    par[1] = obj->objTemplate->unk14 >> 8;
    par[2] = obj->objTemplate->unk16;
    par[3] = obj->objTemplate->unk16 >> 8;
    par[4] = obj->objTemplate->unk18;
    par[5] = obj->objTemplate->unk18 >> 8;
    for (i = 0; i < par[0] >> 4 && i < ROPE_TARGETS; i++)
        if (par[2 + i] != 0xFF) SqueakSendMail(SlotOf(&obj->base), par[2 + i], MESSAGE_SWITCH + 1);
    PlaySfx(&obj->base, SQUEAK_SE((obj->objTemplate->unk14 & 15) < 2 ? 0x48 : 0x49)); // (the DS's, by its look: 0x022248CE);
    obj->base.unk5C &= ~7;
    // (where it was cut: the attacker's row, kept in the rope's);
    top = obj->base.y >> 12;
    bottom = ((obj->base.y >> 8) + (par[1] > 0 ? par[1] : 16) - 1) >> 4;
    row = obj->base.unk6C != NULL ? ((struct ObjectBase *)obj->base.unk6C)->y >> 12 : (top + bottom) / 2;
    r->row = row < top ? top : row > bottom ? bottom : row;
    r->ways = ROPE_UP | ROPE_DOWN;
    r->step = 0;
    r->wait = ROPE_CLEAR_STEP - 1;
}

static void *SqueakCreateRope(struct ObjectTemplate *template, u8 playerId) {
    struct Task *task = TaskCreate(ObjectMain, sizeof(struct SqueakRope), 0x1000, TASK_USE_EWRAM, ObjectDestroy);
    struct SqueakRope *r = TaskGetStructPtr(task);
    struct Object *obj = &r->obj;
    s16 length = template->unk14 >> 8;

    InitObject(obj, template, playerId);
    HolderInit(obj, &r->spr, SQUEAK_FLOOR, 2, 0, 14, length > 0 ? length : 16);
    obj->base.unk5C |= 0x20;
    obj->base.flags |= 0x400;
    r->ways = 0;
    r->step = 0;
    r->row = 0;
    r->wait = 0;
    obj->unk78 = SqueakRopeMain;
    return obj;
}

/* The rolling boulders (gimmick 6, ctor 0x0221EE04, vtable 0x02228FE8; yakumono/ball_rock: animation 0 the rock, a 64-pixel ball, 1 and
 * 2 its pieces spinning): its spot its middle, its box ±2 cells (0x0221EE04's -0x20000-0x20000). Its first byte: bits 0-3 its look (one,
 * 0x02227AA0); 4-5 how it starts (0 rolling at once, the constructor's state 1; else waiting, state 0, for a message: 2-4-1's quake
 * triggers, 4-3-2's relays, 4-6-7's trigger areas); 6-7 its speed (0x02227AB4: 0x2F frames to 2 pixels a frame, 0x40 to 3, 0x61 to 4,
 * a ramp, 0x02226CE4). Rolling (state 2, 0x0221EAE0) it's a body of the DS's physics on the terrain: down its slopes, a quake as it
 * lands, and on a wall (its body's flags 0x400, 0x800) a bigger quake, sound 0x4F and it breaks (0x0221E7D8: its pieces); it hurts
 * (0x0221ED6C: harm kind 0x0E, strength 8). Here: KatAM's own terrain collision (ObjectMain's sub_0809D8C8, its box), gravity,
 * rolling down the slope under it (the ground lower on one side than the other), on along flat ground the way it went (none yet: it
 * stays, as the DS's body at rest: 2-4-2's, 4-3-2's and 8-2-4's sit in pockets 4 cells wide till their floor goes), its speed ramped
 * up as the DS's; turned as it rolls (animation 3, SqueakRolled(): its step by how far it went);
 * on a wall it breaks (the block-break sound, a puff, a piece flying off spinning, animation 1); it hurts as the other gimmicks do (a
 * plain hit). Its quakes (the owner, of those left out: "I want them"): landing after a fall (the DS's 0x02035FC4(4, 16, 2): a 16-frame
 * quake, KatAM's 14-frame shake, 1), and bursting on a wall (the DS's bigger 7: KatAM's longest timed shake, 24 frames, 3); a fall is
 * BOULDER_FALL_FRAMES frames in the air or more (going over a slope's bumps isn't one); */
struct SqueakBoulder {
    struct Object obj;
    struct SqueakGimmickSprite spr;
    u8 index;
    u8 state;
    s8 way;
    u8 air;
    u16 timer;
    u32 rolled;
};
#define BOULDER_FALL_FRAMES 8
#define BOULDER_LAND_SHAKE 1
#define BOULDER_BURST_SHAKE 3
#define BOULDER_WAIT 0
#define BOULDER_ROLL 1
#define BOULDER_PIECE 2
#define BOULDER_GONE 3
#define BOULDER_RADIUS 28
#define BOULDER_GRAVITY 0x30
#define BOULDER_TOP_FALL 0x600
#define BOULDER_ROLL_ANIM 3
#define BOULDER_PIECE_ANIM 1
#define BOULDER_STEPS 16
// (a step of its turn: the ball's round, 2π × 30 pixels, over 16, in 1/256 pixels);
#define BOULDER_STEP_LENGTH 0xBC0
#define BOULDER_PIECE_FRAMES 48
static const u16 sBoulderRamp[4][2] = { { 0x2F, 0x200 }, { 0x40, 0x300 }, { 0x61, 0x400 }, { 0x61, 0x400 } };

// (the rows down from its middle to the first with ground under x, 6 at most);
static u8 GroundRows(struct Object *obj, s32 x) {
    s32 y = obj->base.y;
    u8 r;

    if (x < gCurLevelInfo[obj->base.unk56].levelMinPosition.x || x > gCurLevelInfo[obj->base.unk56].levelMaxPosition.x) return 0;
    for (r = 0; r < 6 && y <= gCurLevelInfo[obj->base.unk56].levelMaxPosition.y; r++, y += 0x1000)
        if (NidlIsFloor(obj, x, y)) return r;
    return r;
}

/* (-1 or 1 the side whose ground is lower, 0 flat: looked at 16 pixels either side, inside its round, so a wall it touches isn't
 * taken for ground higher up, and it rolls on into it); */
#define BOULDER_PROBE 16
static s8 Downhill(struct Object *obj) {
    u8 left = GroundRows(obj, obj->base.x - (BOULDER_PROBE << 8)), right = GroundRows(obj, obj->base.x + (BOULDER_PROBE << 8));

    return left > right ? -1 : right > left ? 1 : 0;
}

static void BoulderTurn(struct Object *obj, struct SqueakBoulder *b) {
    const struct SqueakAnim *a = &gSqueakData.enemies[b->spr.look].anim[BOULDER_ROLL_ANIM];
    u32 step = b->rolled / BOULDER_STEP_LENGTH % BOULDER_STEPS;

    // (animation 3's steps turn it the DS's way, drawn as KatAM draws the gimmicks: anticlockwise; rolling right, clockwise: backwards);
    if (b->way > 0) step = (BOULDER_STEPS - step) % BOULDER_STEPS;
    obj->unk83 = SQUEAK_HURT_VARIANTS + a->step[step].cell;
}

/* BoulderBreak(); The star blocks in front of it, across its height, broken (their records: sub_08001408): the block groups nothing
 * else sets off, KatAM's star blocks in the converter (KatAMExpansionSqueakRooms.cs' "loose" groups), which on the DS the boulder
 * breaks as it runs into them (4-3-2's wall, 4-6-7's floors over its shaft; the owner: "Destroyed tiles by boulders are still
 * walkable": the boulder burst on them and they stayed); */
#define SQUEAK_STAR_BLOCK 0xF0      // (KatAMExpansionSqueakRooms.cs' KatAMStarBlock);
static void BoulderBreak(struct Object *obj, s8 way) {
    u8 p = obj->base.unk56;
    s32 cx = ((obj->base.x >> 8) + way * (BOULDER_RADIUS + 4)) >> 4, cy;
    s32 y0 = ((obj->base.y >> 8) - BOULDER_RADIUS + 2) >> 4, y1 = ((obj->base.y >> 8) + BOULDER_RADIUS - 2) >> 4;
    const u8 *record;

    if (cx < 0 || way == 0) return;
    for (cy = y0 < 0 ? 0 : y0; cy <= y1; cy++)
        if (GetCollisionTile(p, cx, cy) == SQUEAK_STAR_BLOCK && (record = CellRecord(p, cx, cy)) != NULL)
            sub_08001408(p, (struct Unk_0888562C *)record, NULL, NULL);
}

static void SqueakBoulderMain(struct Object *obj) {
    struct SqueakBoulder *b = (struct SqueakBoulder *)obj;
    u16 frames = sBoulderRamp[obj->objTemplate->unk14 >> 6 & 3][0], top = sBoulderRamp[obj->objTemplate->unk14 >> 6 & 3][1];
    u8 kind;
    s8 slope;

    if (gUnk_03000510.unk4 & ((1 << obj->base.unk56) | 0x10)) return;
    kind = TakeMail(SlotOf(&obj->base), b->index);
    obj->base.unk68 &= ~(7 | 0x80);
    switch (b->state) {
    case BOULDER_WAIT:
        if (kind == 0 || kind > MESSAGE_AREA + 1) break;
        b->state = BOULDER_ROLL;
        b->timer = 0;
        obj->base.flags &= ~(0x800 | 0x100);
        break;
    case BOULDER_ROLL:
        obj->base.unk68 |= 3 | 0x80;
        // (on a wall it ran into: it breaks);
        if (obj->base.xspeed != 0 && (obj->base.unk62 & 3)) {
            BoulderBreak(obj, b->way);
            PlaySfx(&obj->base, SQUEAK_SE(0x4F)); // (the DS's 0x4F);
            RequestScreenShake(BOULDER_BURST_SHAKE, &obj->base);
            CreateEffectObject(&obj->base, 0, 0x292, 0);
            GimmickPlay(obj, &b->spr, BOULDER_PIECE_ANIM);
            obj->base.flags |= 0x100;
            obj->base.xspeed = -b->way * 0x100;
            obj->base.yspeed = 0x300;
            b->state = BOULDER_PIECE;
            b->timer = 0;
            break;
        }
        if (!(obj->base.unk62 & 4)) {
            obj->base.yspeed -= BOULDER_GRAVITY;
            if (obj->base.yspeed < -BOULDER_TOP_FALL) obj->base.yspeed = -BOULDER_TOP_FALL;
            if (b->air < 0xFF) b->air++;
        } else {
            // (landed after a fall: a quake);
            if (b->air >= BOULDER_FALL_FRAMES) RequestScreenShake(BOULDER_LAND_SHAKE, &obj->base);
            b->air = 0;
            // (on flat ground it keeps the way it went; one that hasn't gone yet stays, as the DS's body at rest: 2-4-2's in their pockets);
            if ((slope = Downhill(obj)) != 0) b->way = slope;
            if (b->way != 0 && b->timer < frames) b->timer++;
        }
        obj->base.xspeed = b->way * (s32)(top * b->timer / frames);
        // (on the ground pressed down as far as it goes across, so it keeps to a slope going down: the terrain lifts it back);
        if (obj->base.unk62 & 4) obj->base.yspeed = -(top * b->timer / frames) - BOULDER_GRAVITY;
        b->rolled += obj->base.xspeed < 0 ? -obj->base.xspeed : obj->base.xspeed;
        BoulderTurn(obj, b);
        break;
    case BOULDER_PIECE:
        GimmickAnimate(obj, &b->spr);
        obj->base.yspeed -= BOULDER_GRAVITY;
        if (++b->timer < BOULDER_PIECE_FRAMES) break;
        obj->base.flags |= 0x400 | 0x800;
        obj->base.xspeed = obj->base.yspeed = 0;
        b->state = BOULDER_GONE;
        break;
    }
}

static void *SqueakCreateBoulder(struct ObjectTemplate *template, u8 playerId) {
    struct Task *task = TaskCreate(ObjectMain, sizeof(struct SqueakBoulder), 0x1000, TASK_USE_EWRAM, ObjectDestroy);
    struct SqueakBoulder *b = TaskGetStructPtr(task);
    struct Object *obj = &b->obj;

    InitObject(obj, template, playerId);
    b->index = template->unk12;
    GimmickInit(obj, &b->spr, SQUEAK_BOULDER);
    obj->base.unk5C |= 0x20;
    ObjectSetBounds(&obj->base, -BOULDER_RADIUS, -BOULDER_RADIUS, BOULDER_RADIUS, BOULDER_RADIUS + 2);
    sub_0803E2B0(&obj->base, -BOULDER_RADIUS + 2, -BOULDER_RADIUS + 2, BOULDER_RADIUS - 2, BOULDER_RADIUS);
    b->way = 0;
    b->rolled = 0;
    b->timer = 0;
    b->air = 0;
    b->state = BOULDER_WAIT;
    // (bits 4-5 0: rolling at once, the terrain its own);
    if (!(template->unk14 & 0x30)) {
        b->state = BOULDER_ROLL;
        obj->base.flags &= ~(0x800 | 0x100);
    }
    BoulderTurn(obj, b);
    obj->unk78 = SqueakBoulderMain;
    return obj;
}

/* The candles (gimmick 0xA, ctor 0x022237A4, vtable 0x022297E8; yakumono/candle, its look the parameter's low 4 bits, 0x02227D9C: one,
 * animation 0 lit, its flame flickering, 1 out): 4-3-7's, 4-3-8's and 8-1-6's. Its box ±0.5 cells across, 1.5 up from
 * its spot. A burning attack on it (the message's flags 0x901, 0x022236A0) lights it (state 1) for the parameter's bits 4-15 frames
 * (400-540 in the rooms; again while lit: as long again), a blowing one (flag 2) puts it out with a puff; out (0x02223514), once none of
 * the room's is lit, the far view's light dims (CandleLight()). Here: lit by an attack of a Kirby with Fire or Burning (his own, or
 * one he let go: its parent), or with NiDL's Light (its flash, with the NiDL ROM: the owner, "the light ability should light up Squeak
 * Squad sections that require the candle if NiDL expansion is present"), for its frames, then out; lit, it isn't culled off screen
 * (the DS's candles count down wherever the camera is), so the room's light follows it; */
struct SqueakCandle {
    struct Object obj;
    struct SqueakGimmickSprite spr;
    u16 timer;
};
#define CANDLE_LIT_ANIM 0
#define CANDLE_OUT_ANIM 1

static void SqueakCandleMain(struct Object *obj) {
    struct SqueakCandle *c = (struct SqueakCandle *)obj;
    struct ObjectBase *by = obj->base.unk6C;
    struct Kirby *k;

    if (gUnk_03000510.unk4 & ((1 << obj->base.unk56) | 0x10)) return;
    GimmickAnimate(obj, &c->spr);
    if (Struck(obj) && (k = AttackKirby(by)) != NULL
        && ((k->ability & NIDL_ABILITY_MASK) == KIRBY_ABILITY_FIRE || (k->ability & NIDL_ABILITY_MASK) == KIRBY_ABILITY_BURNING
            || (k->ability & NIDL_ABILITY_MASK) == NIDL_ABILITY_LIGHT)) {
        if (c->timer == 0) {
            GimmickPlay(obj, &c->spr, CANDLE_LIT_ANIM);
            PlaySfx(&obj->base, SE_ABILITY_FIRE_ATTACK);
            obj->base.flags |= 0x2000000;
        }
        c->timer = obj->objTemplate->unk14 >> 4;
    }
    if (c->timer != 0 && --c->timer == 0) {
        GimmickPlay(obj, &c->spr, CANDLE_OUT_ANIM);
        obj->base.flags &= ~0x2000000;
    }
}

// CandlesLit(); The room's candles lit (CandleLight());
static u8 CandlesLit(u16 room) {
    struct Task *t = &gTasks[0];
    u8 n, count = 0;

    for (n = 0; n < MAX_TASK_NUM && t->next; n++) {
        struct Object *other;

        t = (struct Task *)(IWRAM_START + t->next);
        other = TaskGetStructPtr(t);
        if (other->base.header.kind != 1 || other->unk78 != SqueakCandleMain) continue;
        if (other->base.roomId == room && ((struct SqueakCandle *)other)->timer != 0) count++;
    }
    return count;
}

static void *SqueakCreateCandle(struct ObjectTemplate *template, u8 playerId) {
    struct Task *task = TaskCreate(ObjectMain, sizeof(struct SqueakCandle), 0x1000, TASK_USE_EWRAM, ObjectDestroy);
    struct SqueakCandle *c = TaskGetStructPtr(task);
    struct Object *obj = &c->obj;

    InitObject(obj, template, playerId);
    HolderInit(obj, &c->spr, SQUEAK_CANDLE, -8, -24, 8, 0);
    c->timer = 0;
    GimmickPlay(obj, &c->spr, CANDLE_OUT_ANIM);
    obj->unk78 = SqueakCandleMain;
    return obj;
}

/* The pedestals of the Squeak Squad ability room (KatAMExpansionSqueakLevels.cs's BuildSqueakAbilityRoom(): its .doe's items, kinds
 * 0x17 on, Squeak Squad's ability the kind less 0x16), drawn as the DS draws them (item/copy, the pedestal item's 0x0203A8A0 and its
 * table 0x021582D8: ability n's pedestal): the stand by the object (SQUEAK_PEDESTAL, with the icon's pieces in the stand's palette),
 * the rest of the icon by a sprite of its own over it (SQUEAK_PEDESTAL_ICON: the DS draws each piece in its own palette, 1-3 an icon's,
 * more colors than one of KatAM's palettes holds). A Kirby with no ability touching it takes KatAM's ability it holds, as KatAM's
 * ability statues (sub_080AA588), and it stays; one KatAM lacks is only shown. Its template: unk12 the DS's ability (its look's
 * animation unk12 - 1), unk14 KatAM's ability (0: none), unk16 1 when its icon has pieces of its own (the icon's sprite); */
struct SqueakPedestalIcon;
struct SqueakPedestal {
    struct Object obj;
    struct SqueakGimmickSprite spr;
    struct SqueakPedestalIcon *icon;
};
struct SqueakPedestalIcon {
    struct EffectObject e;
    struct SqueakPedestal *owner;
};
// (its box, pixels from its spot: the DS's frames, the stand 8 under it, the icon 16 over the stand);
#define PEDESTAL_LEFT -8
#define PEDESTAL_TOP -20
#define PEDESTAL_RIGHT 8
#define PEDESTAL_BOTTOM 4

// SqueakPedestalIconDestroy(); The icon's end (its own, or the room's): its pedestal lets go of it;
static void SqueakPedestalIconDestroy(struct Task *t) {
    struct SqueakPedestalIcon *i = TaskGetStructPtr(t);

    if (i->owner && i->owner->icon == i) i->owner->icon = NULL;
    ObjectBaseDestroy(t);
}

// SqueakPedestalIconMain(); Draws the icon where its pedestal is, in front of it (none while the pedestal isn't drawn);
static void SqueakPedestalIconMain(void) {
    struct SqueakPedestalIcon *i = TaskGetStructPtr(gCurTask);
    struct SqueakPedestal *p = i->owner;

    if ((i->e.flags & 0x1000) || p == NULL) {
        TaskDestroy(gCurTask);
        return;
    }
    if (p->obj.base.flags & 0x400) return;
    i->e.x = p->obj.base.x;
    i->e.y = p->obj.base.y;
    i->e.sprite.unk14 = p->obj.base.sprite.unk14 - 0x40;
    // (KatAM flips a frame when flag 1 is set: as the pedestal, facing right);
    i->e.flags = (i->e.flags & ~0x401) | 4 | (p->obj.base.flags & 1 ? 0 : 1);
    i->e.objBase54 = p->obj.base.objBase54;
    i->e.objBase55 = p->obj.base.objBase55;
    EffectObjectPostUpdate(&i->e);
}

// PedestalShowIcon(); Its icon's sprite, made once the local player is in its room (its palette loaded then, as the layers');
static void PedestalShowIcon(struct SqueakPedestal *p) {
    const struct SqueakGraphics *g = &gSqueakData.enemies[SQUEAK_PEDESTAL_ICON];
    u8 anim = p->obj.objTemplate->unk12 - 1;
    struct SqueakPedestalIcon *i;
    struct Task *t;

    if (anim >= g->anims || gKirbys[gLocalPlayerId].base.roomId != p->obj.base.roomId) return;
    t = TaskCreate(SqueakPedestalIconMain, sizeof(struct SqueakPedestalIcon), 0x3500, TASK_USE_EWRAM, SqueakPedestalIconDestroy);
    if (t == NULL) return;
    i = TaskGetStructPtr(t);
    ClearEffectObject(&i->e);
    i->e.header.kind = 3;
    i->e.roomId = p->obj.base.roomId;
    i->e.parent = NULL;
    i->e.x = p->obj.base.x;
    i->e.y = p->obj.base.y;
    i->owner = p;
    EffectObjectInitSprite(&i->e, &i->e.sprite, g->tiles, g->animId, SQUEAK_HURT_VARIANTS + g->anim[anim].step[0].cell, 0x1A);
    i->e.sprite.palId = g->palette != NULL ? SqueakLoadPalette(g->palette[anim].key, g->palette[anim].variant, p->obj.base.roomId)
                                           : SqueakLoadPalette(g->animId, SQUEAK_HURT_VARIANTS + g->anim[anim].step[0].cell, p->obj.base.roomId);
    p->icon = i;
}

// SqueakPedestalMain(); Each frame: its icon shown; a Kirby touching it with no ability, free (sub_080AA588's checks), takes its ability;
static void SqueakPedestalMain(struct Object *obj) {
    struct SqueakPedestal *p = (struct SqueakPedestal *)obj;
    u8 ability = obj->objTemplate->unk14;
    struct Kirby *kirby;

    if (p->icon == NULL && (obj->objTemplate->unk16 & 0xFF) != 0) PedestalShowIcon(p);
    if (gUnk_03000510.unk4 & ((1 << obj->base.unk56) | 0x10)) return;
    if (ability == KIRBY_ABILITY_NORMAL || !(obj->base.flags & 0x40000) || !obj->base.unk6C) return;
    kirby = (struct Kirby *)obj->base.unk6C;
    if (kirby->base.header.kind != 0 || kirby->ability != KIRBY_ABILITY_NORMAL || kirby->hp <= 0 || kirby->animationIndex == 0x27
        || kirby->animationIndex > 0x7A || kirby->unk110 || (kirby->base.flags & 0x3800B00)) return;
    kirby->transitioningAbility = ability;
    sub_08054C0C(kirby);
    kirby->animationIndex = 0xF;
}

// SqueakPedestalDestroy(); Its end: its icon goes with it;
static void SqueakPedestalDestroy(struct Task *t) {
    struct SqueakPedestal *p = TaskGetStructPtr(t);

    if (p->icon != NULL) p->icon->owner = NULL;
    ObjectDestroy(t);
}

/* SqueakCreatePedestal(); A pedestal: still, harmless, staying when off the screen (GimmickInit()), touched as KatAM's ability statues
 * are (CreateAbilityStatue's collision); */
static void *SqueakCreatePedestal(struct ObjectTemplate *template, u8 playerId) {
    struct Task *task = TaskCreate(ObjectMain, sizeof(struct SqueakPedestal), 0x1000, TASK_USE_EWRAM, SqueakPedestalDestroy);
    struct SqueakPedestal *p = TaskGetStructPtr(task);
    struct Object *obj = &p->obj;

    InitObject(obj, template, playerId);
    GimmickInit(obj, &p->spr, SQUEAK_PEDESTAL);
    p->icon = NULL;
    obj->base.unk68 &= ~7;
    obj->base.unk5C |= ~7;
    obj->base.unk5C &= ~0x80;
    obj->base.unkC |= 1 | 4;
    obj->base.flags |= 0x200000 | 0x10000 | 0x800 | 0x8000 | 0x40 | 0x100 | 0x400000;
    sub_0803E2B0(&obj->base, PEDESTAL_LEFT, PEDESTAL_TOP, PEDESTAL_RIGHT, PEDESTAL_BOTTOM);
    ObjectSetBounds(&obj->base, PEDESTAL_LEFT, PEDESTAL_TOP, PEDESTAL_RIGHT, PEDESTAL_BOTTOM);
    if (template->unk12 >= 1 && template->unk12 <= gSqueakData.enemies[SQUEAK_PEDESTAL].anims) GimmickPlay(obj, &p->spr, template->unk12 - 1);
    obj->unk78 = SqueakPedestalMain;
    return obj;
}

// SqueakGimmickAnim(); A gimmick's look and cell as squeak.c's SqueakAnimFor() draws it (0: the object isn't one);
u32 SqueakGimmickAnim(struct Object *obj) {
    if (obj->type != OBJ_HAMMER_PEG) return 0;
    if (obj->objTemplate->unk11 == SQUEAK_SWITCH_OBJECT)
        return gSqueakData.enemies[SQUEAK_SWITCH].animId | (SQUEAK_HURT_VARIANTS + (obj->unk83 & 1)) << 16;
    if (obj->objTemplate->unk11 >= SQUEAK_PEDESTAL_OBJECT && obj->objTemplate->unk11 <= SQUEAK_SHUTTER_OBJECT)
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
    case SQUEAK_PLATFORM_OBJECT: return SqueakCreatePlatform(template, playerId);
    case SQUEAK_CLOUD_OBJECT: return SqueakCreateCloud(template, playerId);
    case SQUEAK_ROPE_OBJECT: return SqueakCreateRope(template, playerId);
    case SQUEAK_BOULDER_OBJECT: return SqueakCreateBoulder(template, playerId);
    case SQUEAK_CANDLE_OBJECT: return SqueakCreateCandle(template, playerId);
    case SQUEAK_PEDESTAL_OBJECT: return SqueakCreatePedestal(template, playerId);
    }
    return gSqueakSwitch.create(template, playerId);
}
