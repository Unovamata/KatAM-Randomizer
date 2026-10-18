/* WarpStar; The ported NiDL warp stars' rides (KatAMExpansionRooms.cs, WarpFlights()): each is KatAM's own warp star (object 0x84)
 * flying a flight of its own (its subtype: NIDL_RIDE_FLIGHT and after), NiDL's ride step by step: the star's moves frame by frame,
 * as NiDL's star makes them, through each room NiDL's ride goes through (a sky it crosses, the room it lands in), down to where
 * Kirby hops off. Two of KatAM's routines are detoured here (KatAMExpansionRooms.cs, WarpRideDetours()):
 * • the warp star's rev (sub_0800C270): KatAM's star waits 300 frames for the helpers before it's off, NiDL's goes at once: a
 *   ported star's (its subtype NIDL_RIDE_FLIGHT and after) skips the wait;
 * • a flight's room change (sub_08055D9C, a flight step flagged 0x40): KatAM's flights change room once, to the star's room; a
 *   step marked NIDL_RIDE_ROOM (its +8) takes Kirby to the room in its +0xA instead, at the cell in its +0 and +2 (the step after
 *   it moves him to where NiDL's star comes in), so a ride goes on through several rooms;
 * • a flight's step (sub_080534D0): a ride flies Kirby's tumble off the star as NiDL's (the orb where the star went, then KatAM's
 *   tumbling and bouncing animations) to where he stands, and ends there (its end step marked NIDL_RIDE_HOP, not 0x80: KatAM's
 *   hop-off would throw him off again): the flight's ending, KatAM's plain one (sub_080534D0: back to standing or falling), left as
 *   KatAM's hop-off leaves him (the flight's flags off), facing the end step's way; */
#include "global.h"
#include "kirby.h"
#include "warp_star.h"
#include "functions.h"

void sub_080534D0(struct Kirby *);  // a flight's step (kirby.c);

#define NIDL_RIDE_FLIGHT 6    // the first warp star subtype (flight) of the ported warp stars (KatAMExpansionRooms.cs, WarpFlight);
#define NIDL_RIDE_ROOM 0xFE   // a flight step's +8 on a room change (+9 0x40) to the room in its +0xA;
#define NIDL_RIDE_REV 0x12C   // how long KatAM's warp star revs (sub_0800C270: launched once its count passes this);
#define NIDL_RIDE_HOP 0xFD    // a flight's end step's +8: a ride's end, Kirby standing where he tumbled to;

// NidlRide; Filled in by KatAMExpansionRooms.cs: the trampolines running the detoured functions;
struct NidlRide {
    void (*rev)(struct WarpStar *);
    void (*roomChange)(struct Kirby *);
    void (*step)(struct Kirby *);
};

const struct NidlRide gNidlRide = { NULL, NULL, NULL };

// Room for the trampolines KatAMExpansionRooms.cs writes;
const u32 gNidlRideTrampolines[16] = { 0 };

// NidlRideRev(); A warp star revving with Kirby on it (sub_0800C270): a ported one's off at once, as NiDL's;
void NidlRideRev(struct WarpStar *ws) {
    if (ws->unk0.unkB4 >= NIDL_RIDE_FLIGHT && ws->unkBC < NIDL_RIDE_REV)
        ws->unkBC = NIDL_RIDE_REV;
    gNidlRide.rev(ws);
}

// NidlRideRoomChange(); A room change (sub_08055D9C); on a ride's (its flight step marked), to the step's room and cell;
void NidlRideRoomChange(struct Kirby *kirby) {
    const struct Unk_08353510 *step = kirby->unk114;
    if (kirby->stateFn == sub_080534D0 && step != NULL && step->unk9 & 0x40 && step->unk8 == NIDL_RIDE_ROOM) {
        kirby->roomId = ((const u16 *)step)[5];
        kirby->spawnLocation.x = step->unk0;
        kirby->spawnLocation.y = step->unk2;
    }
    gNidlRide.roomChange(kirby);
}

// NidlRideStep(); A flight's step (sub_080534D0); at a ride's end, Kirby left where he stands (see above), as KatAM's hop-off leaves
// him (sub_080534D0's flight end with the 0x80 flag);
void NidlRideStep(struct Kirby *kirby) {
    const struct Unk_08353510 *step = kirby->unk114;
    gNidlRide.step(kirby);
    if (step != NULL && kirby->unk114 == step + 1 && step[1].unk4 == 0 && step[1].unk8 == NIDL_RIDE_HOP && kirby->stateFn != sub_080534D0) {
        kirby->base.flags &= ~0x8000;
        kirby->unk114 = NULL;
        kirby->base.unkC &= ~0x2000;
        kirby->base.xspeed = 0;
        kirby->base.yspeed = 0;
        // (facing the end step's way: the flight's end set it back to where he faced before, unkC 0x800);
        if (step[1].unk9 & 1) {
            kirby->base.flags |= 1;
            kirby->base.unkC |= 0x800;
        } else {
            kirby->base.flags &= ~1;
            kirby->base.unkC &= ~0x800;
        }
    }
}
