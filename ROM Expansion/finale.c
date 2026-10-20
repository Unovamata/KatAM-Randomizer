/* Finale; The Fountain of Dreams, the finale before KatAM's final boss (see "Documentation/Map and Door Design Rules.md"): Central
 * Circle's and the NiDL hub's Dimension Mirrors lead into the Power Orb's room once KatAM's 8 shards are found (KatAMExpansionRooms.cs,
 * FinaleMirrors()); its two bosses leave no shard:
 * • the Power Orb beaten (nidl_boss.c's NidlBossDefeat), a warp star (KatAM's own, object 0x84) comes where it was, to Nightmare's
 *   room; it waits for the helpers, as KatAM's (warpstar.c's NidlRideRev), and every Kirby rides it there, keeping the Star Rod:
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
    u8 flight;          // the star's flight (warpstar.c);
    u8 pad;
};

const struct NidlFinale gNidlFinale = { 0 };

// PowerOrbBeaten(); The Power Orb's end (NidlBossDefeat()): its warp star where it was;
void PowerOrbBeaten(struct Object *orb) {
    CreateObjTemplateAndObj(orb->base.unk56, 1, 0x24, orb->base.x >> 8, orb->base.y >> 8, 0, 0x1F, 0, 0, OBJ_WARP_STAR, gNidlFinale.flight,
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

// NidlFinaleKeep(); Nightmare alive (bosses/nightmare.c): every Kirby in his room keeps the Star Rod (StarRodKeep());
void NidlFinaleKeep(struct Object *nightmare) {
    u8 i;

    for (i = 0; i < gNumKirbys; i++)
        if (gKirbys[i].base.roomId == nightmare->base.roomId) StarRodKeep(&gKirbys[i]);
}
