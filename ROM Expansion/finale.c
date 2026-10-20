/* Finale; The Fountain of Dreams, the finale before KatAM's final boss (see "Documentation/Map and Door Design Rules.md"): Central
 * Circle's and the NiDL hub's Dimension Mirrors lead into the Power Orb's room once KatAM's 8 shards are found (KatAMExpansionRooms.cs,
 * FinaleMirrors()); its two bosses leave no shard:
 * • the Power Orb beaten (nidl_boss.c's NidlBossDefeat), NiDL's end of its fight (bosses/power_orb.c's PowerOrbEnd()): it flies
 *   off, and a warp star (KatAM's own, object 0x84) comes where it was (NidlFinaleStarAt()), to Nightmare's room; it waits for the
 *   helpers, as KatAM's (warpstar.c's NidlRideRev), the helpers flying to it (abilities/star_rod.c's AI(), NidlFinaleStarSpot()), but
 *   only until every Kirby is on it (NidlFinaleAllAboard()), and they all ride it there, NiDL's ride (up off the screen, the screen
 *   rising to the moon, the orb and the star flying into it: PowerOrbEnd() again), keeping the Star Rod:
 *   NiDL's Nightmare is fought with it, on the ground (abilities/star_rod.c; NidlFinaleKeep(): never lost in his room);
 * • Nightmare beaten, NiDL's defeat (bosses/nightmare.c's NightmareEnd()), then every Kirby in his room is flown out (NidlEndingFlight():
 *   the fade to white), his ability before the Star Rod given back, to NiDL's ending down from the moon in the Power Orb's room
 *   (ending.c: held there, NidlFinaleHold()), then to the Fountain of Dreams hub (his room's way on, gNidlShardExits'), where its
 *   Amazing Mirror leads to KatAM's final boss;
 * • the Power Orb's room's cloud bands drift sideways, NiDL's line scroll (NidlCloudsStart(), see there), in its fight and the ending;
 *   Nightmare's room's background drifts left (DriftMain());
 * gNidlFinale and gNidlCloudSpeeds filled in by KatAMExpansion.cs; */
#include "nidl.h"
#include "abilities.h"
#include "data.h"
#include "main.h"
#include "constants/object_types.h"

struct NidlFinale {
    u16 orbRoom;        // the Power Orb's room;
    u16 rideRoom;       // where its warp star takes the Kirbys: Nightmare's room,
    u8 rideX, rideY;    // and the cell they land in + 1, as a door's;
    u8 flight;          // the star's flight (warpstar.c; its first step NIDL_RIDE_FINALE, bosses/power_orb.c);
    u8 pad;
    const struct Unk_08353510 *ending;  // the ending's flight out of Nightmare's room (NidlEndingFlight()), NULL none;
};

#define ENDING_STEPS 7  // (the ending's flight's steps: KatAMExpansionRooms.cs, WarpFlights());

const struct NidlFinale gNidlFinale = { 0 };

void sub_080531B4(struct Kirby *, const struct Unk_08353510 *);   // a flight's start (kirby.c);
void sub_080534D0(struct Kirby *);                                 // a flight's step;

// PowerOrbBeaten(); The Power Orb's end (NidlBossDefeat()): NiDL's (bosses/power_orb.c);
void PowerOrbBeaten(struct Object *orb) {
    PowerOrbEnd(orb);
}

// NidlFinaleStarAt(); The Power Orb's warp star at (x, y) (8.8; PowerOrbEnd());
void NidlFinaleStarAt(u8 player, s32 x, s32 y) {
    CreateObjTemplateAndObj(player, 1, 0x24, x >> 8, y >> 8, 0, 0x1F, 0, 0, OBJ_WARP_STAR, gNidlFinale.flight,
                            0, 0, 0, gNidlFinale.rideX, gNidlFinale.rideY, gNidlFinale.rideRoom, 0, 0, 0, 0, 0, 0);
}

// NightmareBeaten(); Nightmare's end (NidlBossDefeat()): NiDL's (bosses/nightmare.c), then the Kirbys are flown out (NidlEndingFlight());
void NightmareBeaten(struct Object *nightmare) {
    NightmareEnd(nightmare);
}

/* NidlEndingFlight(); Every Kirby in Nightmare's room (and still up) flown out where he is, his ability before the Star Rod back: the
 * ending's flight (KatAMExpansionRooms.cs, WarpFlights()), its fade to white, NiDL's ending down from the moon in the Power Orb's
 * room (ending.c), then the room his way on leads to (the Fountain of Dreams hub); without it, the dance out (NidlDanceOut()); */
void NidlEndingFlight(u16 room) {
    u8 i;

    if (gNidlFinale.ending == NULL) {
        NidlDanceOut(room);
        return;
    }
    for (i = 0; i < gNumKirbys; i++) {
        struct Kirby *kirby = &gKirbys[i];

        if (kirby->base.roomId != room || kirby->hp <= 0 || kirby->stateFn == sub_080534D0) continue;
        StarRodRestore(kirby);
        sub_080531B4(kirby, gNidlFinale.ending);
    }
}

// NidlOnEnding(); Whether a Kirby's flying the ending's flight (on one of its steps);
bool32 NidlOnEnding(struct Kirby *kirby) {
    return gNidlFinale.ending != NULL && kirby->stateFn == sub_080534D0 && (const struct Unk_08353510 *)kirby->unk114 >= gNidlFinale.ending
        && (const struct Unk_08353510 *)kirby->unk114 < gNidlFinale.ending + ENDING_STEPS;
}

// NidlFinaleStar(); Whether a warp star is the Power Orb's (it waits for the helpers, as KatAM's: warpstar.c);
bool32 NidlFinaleStar(struct Object *star) {
    return gNidlFinale.orbRoom && star->base.roomId == gNidlFinale.orbRoom;
}

// NidlFinaleAllAboard(); Whether every Kirby still in a warp star's room (and not knocked out) rides it, `aboard` its riders (its
// +0xB5: warpstar.c: then the Power Orb's is off, not waiting out KatAM's 300 frames);
bool32 NidlFinaleAllAboard(struct Object *star, u8 aboard) {
    u8 i;

    for (i = 0; i < gNumKirbys; i++)
        if (gKirbys[i].base.roomId == star->base.roomId && gKirbys[i].hp > 0 && !((aboard >> i) & 1)) return FALSE;
    return TRUE;
}

// NidlFinaleStarSpot(); The Power Orb's warp star in a room (abilities/star_rod.c: the helpers fly to it): its spot (8.8); FALSE: none;
bool32 NidlFinaleStarSpot(u16 room, s32 *x, s32 *y) {
    struct Task *t = &gTasks[0];
    u8 n;

    if (!gNidlFinale.orbRoom || room != gNidlFinale.orbRoom) return FALSE;
    for (n = 0; n < MAX_TASK_NUM && t->next; n++) {
        struct Object *obj;

        t = (struct Task *)(IWRAM_START + t->next);
        obj = TaskGetStructPtr(t);
        if (obj->base.header.kind != 1 || obj->type != OBJ_WARP_STAR || obj->base.roomId != room || (obj->base.flags & 0x1000)) continue;
        *x = obj->base.x;
        *y = obj->base.y;
        return TRUE;
    }
    return FALSE;
}

/* NidlFinaleHoldCamera(); The screen held at (x, y) (8.8, the room's: its top left) for every player in a room, as an arena's lock,
 * right on it, the Kirbys' bounds a screen above and below it (the finale's rides: bosses/power_orb.c's end, Nightmare's intro); */
void NidlFinaleHoldCamera(u16 room, s32 x, s32 y) {
    u8 i;

    for (i = 0; i < gNumKirbys; i++) {
        struct LevelInfo *l = &gCurLevelInfo[i];

        if (gKirbys[i].base.roomId != room) continue;
        l->unk_S32Vec2_7C.x = l->unk_S32Vec2_6C.x = l->unkA0 = l->unk90 = x;
        l->unk_S32Vec2_7C.y = l->unk_S32Vec2_6C.y = y;
        l->unk_S32Vec2_84.x = l->unk_S32Vec2_74.x = l->unkA8 = l->unk98 = x + (DISPLAY_WIDTH << 8);
        l->unk_S32Vec2_84.y = l->unk_S32Vec2_74.y = y + (DISPLAY_HEIGHT << 8);
        l->unkA4 = l->unk94 = y - (DISPLAY_HEIGHT << 8);
        l->unkAC = l->unk9C = y + (DISPLAY_HEIGHT << 9);
    }
}

// NidlFinaleRideSize(); The size the finale's riders are drawn at (warpstar.c): what its rides say (bosses/power_orb.c's end in the
// Power Orb's room, or the ending's there, ending.c; bosses/nightmare.c's intro in Nightmare's);
u32 NidlFinaleRideSize(u16 room) {
    if (gNidlFinale.orbRoom && room == gNidlFinale.orbRoom) return NidlEndingOn(room) ? NidlEndingRideSize(room) : PowerOrbRideSize(room);
    if (gNidlFinale.rideRoom && room == gNidlFinale.rideRoom) return NightmareRideSize(room);
    return 0;
}

// NidlFinaleHold(); A Kirby held on the finale's ride (warpstar.c, every frame): in Nightmare's room, his intro takes them; on the
// ending's flight in the Power Orb's room, the ending (ending.c; the Power Orb's own ride there its end moves, bosses/power_orb.c);
void NidlFinaleHold(struct Kirby *kirby) {
    if (gNidlFinale.rideRoom && kirby->base.roomId == gNidlFinale.rideRoom) NightmareIntroRider(kirby);
    else if (gNidlFinale.orbRoom && kirby->base.roomId == gNidlFinale.orbRoom && NidlOnEnding(kirby)) NidlEndingRider(kirby);
}

// NidlFinaleKeep(); Nightmare alive (bosses/nightmare.c): every Kirby in his room keeps the Star Rod (StarRodKeep()), and the helpers
// swing it at him (StarRodAim());
void NidlFinaleKeep(struct Object *nightmare) {
    u8 i;

    for (i = 0; i < gNumKirbys; i++)
        if (gKirbys[i].base.roomId == nightmare->base.roomId) {
            StarRodKeep(&gKirbys[i]);
            StarRodAim(&gKirbys[i]);
        }
}

/* The Power Orb's room's cloud bands drift sideways, each at its own speed, as NiDL's (its room's line scroll, 0x080B6B08, armed every
 * VBlank by 0x080B60A8: an HBlank DMA of each line's BG2 and BG3 horizontal scroll): a line's background row (the room's: its
 * background's drawn at the camera, bosses/power_orb.c) gives its speed, gNidlCloudSpeeds (NiDL's table, a byte a row of 8 pixels:
 * 0 the starry sky, then 1-8 half pixels a frame down the cloud bands; KatAMExpansionRooms.cs copies it from the player's NiDL ROM),
 * times the frames since the room came in (0-511, then 0 again: every band's back where it was, 256 pixels on), halved; NiDL's
 * ending down from the moon (ending.c, here too) runs it backwards (0x080B6D04: its moon room's background is the orb room's
 * mirrored, so its bands drift right). KatAM has the same HBlank DMA (main.c's VBlankIntr): a task after the level's (its camera
 * and background scroll, priority 0x7FFF) fills the table the next VBlank shows (gUnk_03002484) with BG0's (the room's background)
 * horizontal scroll, as Sonic Advance's line effects do. KatAM draws a background too big for VRAM a screen at a time, but every
 * row whole (it wraps a row round, sub_08153184), and NiDL's is 256 pixels wide: a line's scroll shows its row as NiDL's; */
#define CLOUD_ROWS 144      // (KatAMExpansionRooms.cs's CloudRows: 1152 pixels, the room's 936 and more);
#define CLOUD_PERIOD 512
#define CLOUD_PRIORITY 0x8000

const u8 gNidlCloudSpeeds[CLOUD_ROWS] = { 0 };

struct Clouds {
    u16 t;          // Frames since the room came in (0-511);
    bool8 ending;   // The ending's (its Kirbys on its flight, all through it: NidlOnEnding());
    u8 sky;         // The rows of starry sky over the first band (NidlCloudsStart());
};

/* (the game's heaviest frames, the room coming in and the fight, have no time to spare: a row of 8 lines at a time, the same scroll,
 * and none while the screen's all sky, every line's scroll BG0's own, as the fight's first half: no table, no HBlank DMA); */
static void CloudsMain(void) {
    struct Clouds *clouds = TaskGetStructPtr(gCurTask);
    struct Unk_08002E48 *view = TaskGetStructPtr(gUnk_02023354);
    struct LevelInfo *level = &gCurLevelInfo[view->unk0];
    u16 *line = (u16 *)gUnk_03002484, *end = line + DISPLAY_HEIGHT;
    s32 t = clouds->t, y = level->unkC0[2].scrollY, x = gBgScrollRegs[0][0];
    u8 i;

    if (level->currentRoom != gNidlFinale.orbRoom) {
        TaskDestroy(gCurTask);
        return;
    }
    for (i = 0; i < gNumKirbys && !clouds->ending; i++)
        clouds->ending = gKirbys[i].base.roomId == level->currentRoom && NidlOnEnding(&gKirbys[i]);
    if (clouds->ending) t = CLOUD_PERIOD - t;
    if (++clouds->t >= CLOUD_PERIOD) clouds->t = 0;
    if ((y + DISPLAY_HEIGHT - 1) >> 3 < clouds->sky) return;
    y >>= 3;
    for (line -= level->unkC0[2].scrollY & 7; line < end; y++) {
        u16 *rowEnd = line + 8, scroll = x + ((y < CLOUD_ROWS ? gNidlCloudSpeeds[y] : 0) * t >> 1);

        if (line < (u16 *)gUnk_03002484) line = (u16 *)gUnk_03002484;
        if (rowEnd > end) rowEnd = end;
        if (rowEnd - line == 8) {
            line[0] = line[1] = line[2] = line[3] = line[4] = line[5] = line[6] = line[7] = scroll;
            line = rowEnd;
        } else {
            while (line < rowEnd) *line++ = scroll;
        }
    }
    gMainFlags |= 4;
    gUnk_030036C8 = REG_ADDR_BG0HOFS;
    gUnk_030039A0 = sizeof(u16);
}

/* Nightmare's room's background drifts left as NiDL's, on and on (its room's line scroll, 0x080B6C40: every line's horizontal scroll
 * 4 pixels a frame on, from 0 as the room came in, round its 512 pixels, BG3's whole width, every 128 frames): KatAM's background
 * scroll (sub_080019F8, the level's task) adds an offset of the room's (LevelInfo +0x3C, 0 as a room comes in) and draws the columns
 * it brings in, the background repeating past its edge (KatAMExpansionRooms.cs: NiDL's, 512 pixels wide); a task after the level's
 * sets it for the next frame; */
#define DRIFT_SPEED 4
#define DRIFT_PERIOD 128

struct Drift {
    u16 t;          // Frames since the room came in (0-127);
};

static void DriftMain(void) {
    struct Drift *drift = TaskGetStructPtr(gCurTask);
    struct Unk_08002E48 *view = TaskGetStructPtr(gUnk_02023354);
    struct LevelInfo *level = &gCurLevelInfo[view->unk0];

    if (level->currentRoom != gNidlFinale.rideRoom) {
        TaskDestroy(gCurTask);
        return;
    }
    if (++drift->t >= DRIFT_PERIOD) drift->t = 0;
    level->unk3C = drift->t * DRIFT_SPEED << 8;
}

// NidlCloudsStart(); A room's come in on the screen (rooms.c's BigFillLevelInfo()): the Power Orb's, its clouds from the start;
// Nightmare's, its drift;
void NidlCloudsStart(u16 room) {
    struct Task *t = &gTasks[0];
    struct Clouds *clouds;
    u8 n;

    if (gNidlFinale.rideRoom && room == gNidlFinale.rideRoom) {
        for (n = 0; n < MAX_TASK_NUM && t->next; n++) {
            t = (struct Task *)(IWRAM_START + t->next);
            if (t->main == DriftMain) break;
        }
        if (t->main != DriftMain) t = TaskCreate(DriftMain, sizeof(struct Drift), CLOUD_PRIORITY, TASK_USE_IWRAM, NULL);
        ((struct Drift *)TaskGetStructPtr(t))->t = 0;
        return;
    }
    if (!gNidlFinale.orbRoom || room != gNidlFinale.orbRoom) return;
    for (n = 0; n < MAX_TASK_NUM && t->next; n++) {
        t = (struct Task *)(IWRAM_START + t->next);
        if (t->main == CloudsMain) break;
    }
    if (t->main != CloudsMain) t = TaskCreate(CloudsMain, sizeof(struct Clouds), CLOUD_PRIORITY, TASK_USE_IWRAM, NULL);
    clouds = TaskGetStructPtr(t);
    clouds->t = 0;
    clouds->ending = FALSE;
    for (clouds->sky = 0; clouds->sky < CLOUD_ROWS && gNidlCloudSpeeds[clouds->sky] == 0; clouds->sky++);
}
