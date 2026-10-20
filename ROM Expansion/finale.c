/* Finale; The Fountain of Dreams, the finale before KatAM's final boss (see "Documentation/Map and Door Design Rules.md"): Central
 * Circle's and the NiDL hub's Dimension Mirrors lead into the Power Orb's room once KatAM's 8 shards are found (KatAMExpansionRooms.cs,
 * FinaleMirrors()); its two bosses leave no shard:
 * • the Power Orb beaten (nidl_boss.c's NidlBossDefeat), NiDL's end of its fight (bosses/power_orb.c's PowerOrbEnd()): it flies
 *   off, and a warp star (KatAM's own, object 0x84) comes where it was (NidlFinaleStarAt()), to Nightmare's room; it waits for the
 *   helpers, as KatAM's (warpstar.c's NidlRideRev), the helpers flying to it (abilities/star_rod.c's AI(), NidlFinaleStarSpot()), but
 *   only until every Kirby is on it (NidlFinaleAllAboard()), and they all ride it there, NiDL's ride (up off the screen, the screen
 *   rising to the moon, the orb and the star flying into it: PowerOrbEnd() again), keeping the Star Rod:
 *   NiDL's Nightmare is fought with it, on the ground (abilities/star_rod.c; NidlFinaleKeep(): never lost in his room);
 * • Nightmare beaten, every Kirby in his room dances and leaves (as after a boss's shard, NidlDanceOut()), for the Fountain of
 *   Dreams hub (gNidlShardExits: his room's), his ability before the Star Rod given back (NidlShardExit()), where its Amazing Mirror
 *   leads to KatAM's final boss;
 * gNidlFinale filled in by KatAMExpansion.cs; */
#include "nidl.h"
#include "abilities.h"
#include "constants/object_types.h"

struct NidlFinale {
    u16 orbRoom;        // the Power Orb's room;
    u16 rideRoom;       // where its warp star takes the Kirbys: Nightmare's room,
    u8 rideX, rideY;    // and the cell they land in + 1, as a door's;
    u8 flight;          // the star's flight (warpstar.c; its first step NIDL_RIDE_FINALE, bosses/power_orb.c);
    u8 pad;
};

const struct NidlFinale gNidlFinale = { 0 };

// PowerOrbBeaten(); The Power Orb's end (NidlBossDefeat()): NiDL's (bosses/power_orb.c);
void PowerOrbBeaten(struct Object *orb) {
    PowerOrbEnd(orb);
}

// NidlFinaleStarAt(); The Power Orb's warp star at (x, y) (8.8; PowerOrbEnd());
void NidlFinaleStarAt(u8 player, s32 x, s32 y) {
    CreateObjTemplateAndObj(player, 1, 0x24, x >> 8, y >> 8, 0, 0x1F, 0, 0, OBJ_WARP_STAR, gNidlFinale.flight,
                            0, 0, 0, gNidlFinale.rideX, gNidlFinale.rideY, gNidlFinale.rideRoom, 0, 0, 0, 0, 0, 0);
}

// NightmareBeaten(); Nightmare's end (NidlBossDefeat()): the Kirbys dance and leave, for the Fountain of Dreams hub;
void NightmareBeaten(struct Object *nightmare) {
    NidlDanceOut(nightmare->base.roomId);
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

// NidlFinaleKeep(); Nightmare alive (bosses/nightmare.c): every Kirby in his room keeps the Star Rod (StarRodKeep());
void NidlFinaleKeep(struct Object *nightmare) {
    u8 i;

    for (i = 0; i < gNumKirbys; i++)
        if (gKirbys[i].base.roomId == nightmare->base.roomId) StarRodKeep(&gKirbys[i]);
}
