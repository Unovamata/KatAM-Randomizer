/* WarpStar; The ported NiDL warp stars' rides (KatAMExpansionRooms.cs, WarpFlights()): each is KatAM's own warp star (object 0x84)
 * flying a flight of its own (its subtype: NIDL_RIDE_FLIGHT and after), NiDL's ride step by step: the star's moves frame by frame,
 * as NiDL's star makes them, through each room NiDL's ride goes through (a sky it crosses, the room it lands in), down to where
 * Kirby hops off. Four of KatAM's routines are detoured here, and one task swapped (KatAMExpansionRooms.cs, WarpRideDetours()):
 * • the warp star's rev (sub_0800C270): KatAM's star waits 300 frames for the helpers before it's off, NiDL's goes at once: a
 *   ported star's (its subtype NIDL_RIDE_FLIGHT and after) skips the wait;
 * • a flight's room change (sub_08055D9C, a flight step flagged 0x40): KatAM's flights change room once, to the star's room; a
 *   step marked NIDL_RIDE_ROOM (its +8) takes Kirby to the room in its +0xA instead, at the cell in its +0 and +2 (the step after
 *   it moves him to where NiDL's star comes in), so a ride goes on through several rooms;
 * • a flight's step (sub_080534D0): a ride flies Kirby's tumble off the star as NiDL's (the orb where the star went, then KatAM's
 *   tumbling and bouncing animations) to where he stands, and ends there (its end step marked NIDL_RIDE_HOP, not 0x80: KatAM's
 *   hop-off would throw him off again): the flight's ending, KatAM's plain one (sub_080534D0: back to standing or falling), left as
 *   KatAM's hop-off leaves him (the flight's flags off), facing the end step's way;
 * • Kirby's drawing (sub_0803F46C), and the star he rides (sub_0808EEF4's task, its function swapped for NidlRideStar): where a
 *   ride's step has a size (its +0xA, NiDL's: the star flying off into the distance), both are drawn at that size around the
 *   star's middle, as NiDL draws its star and rider (all their pieces scaled by one affine matrix, each moved in toward the
 *   middle); */
#include "global.h"
#include "kirby.h"
#include "warp_star.h"
#include "functions.h"
#include "main.h"
#include "task.h"

void sub_080534D0(struct Kirby *);  // a flight's step (kirby.c);
void sub_0808EEF4(void);            // the star Kirby rides, following him (code_0806F780.c, sub_0808EDB8's task);
bool32 NidlFinaleStar(struct Object *);    // the Power Orb's warp star (finale.c);
void NidlFinaleRoomChange(struct Kirby *);  // a Kirby it takes away (finale.c);

#define NIDL_RIDE_FLIGHT 6    // the first warp star subtype (flight) of the ported warp stars (KatAMExpansionRooms.cs, WarpFlight);
#define NIDL_RIDE_ROOM 0xFE   // a flight step's +8 on a room change (+9 0x40) to the room in its +0xA;
#define NIDL_RIDE_REV 0x12C   // how long KatAM's warp star revs (sub_0800C270: launched once its count passes this);
#define NIDL_RIDE_HOP 0xFD    // a flight's end step's +8: a ride's end, Kirby standing where he tumbled to;
#define NIDL_RIDE_MATRIX 7    // the affine matrices of a drawn ride: 7, then 15, 23, 31 for pieces flipped (KatAM's own use 0);

// NidlRide; Filled in by KatAMExpansionRooms.cs: the trampolines running the detoured functions;
struct NidlRide {
    void (*rev)(struct WarpStar *);
    void (*roomChange)(struct Kirby *);
    void (*step)(struct Kirby *);
    void (*draw)(struct Kirby *);
};

const struct NidlRide gNidlRide = { NULL, NULL, NULL, NULL };

// Room for the trampolines KatAMExpansionRooms.cs writes;
const u32 gNidlRideTrampolines[24] = { 0 };

// (OAM's sizes by shape and size, attr0 >> 14 and attr1 >> 14: width, height);
static const u8 sNidlRidePieces[3][4][2] = {
    { { 8, 8 }, { 16, 16 }, { 32, 32 }, { 64, 64 } },
    { { 16, 8 }, { 32, 8 }, { 32, 16 }, { 64, 32 } },
    { { 8, 16 }, { 8, 32 }, { 16, 32 }, { 32, 64 } },
};

// NidlRideRev(); A warp star revving with Kirby on it (sub_0800C270): a ported one's off at once, as NiDL's (but the Power Orb's,
// which waits for the helpers, as KatAM's: every Kirby rides it to Nightmare's room, finale.c);
void NidlRideRev(struct WarpStar *ws) {
    if (ws->unk0.unkB4 >= NIDL_RIDE_FLIGHT && ws->unkBC < NIDL_RIDE_REV && !NidlFinaleStar(&ws->unk0.obj))
        ws->unkBC = NIDL_RIDE_REV;
    gNidlRide.rev(ws);
}

// NidlRideRoomChange(); A room change (sub_08055D9C); on a ride's (its flight step marked), to the step's room and cell (and out of
// the Power Orb's room, the Star Rod given back: finale.c);
void NidlRideRoomChange(struct Kirby *kirby) {
    const struct Unk_08353510 *step = kirby->unk114;
    NidlFinaleRoomChange(kirby);
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

// NidlRideSize(); The size Kirby's drawn at on a ride's step (its +0xA: NiDL's matrix pa, 256 / the size in 8.8), or 0 for his own;
static u32 NidlRideSize(struct Kirby *kirby) {
    const struct Unk_08353510 *step = kirby->unk114;
    u32 pa;

    if (kirby->stateFn != sub_080534D0 || step == NULL) return 0;
    if (step->unk9 & 0x40 && step->unk8 == NIDL_RIDE_ROOM) step--;  // (a room change's +0xA is its room: the size before it);
    pa = ((const u16 *)step)[5];
    return pa > 0x100 && pa < 0x8000 ? pa : 0;
}

/* NidlRideShrink(); The OAM pieces drawn since `first` (gUnk_030031C0, up to gUnk_030024F0) at the size of `pa` around (x, y), the
 * star's middle on screen: each piece scaled by an affine matrix (its flips are its matrix's: attr1 bits 12, 13 become its bits 3,
 * 4) and its middle moved in toward (x, y) by as much, as NiDL draws its star and rider; */
static void NidlRideShrink(u32 first, s32 x, s32 y, u32 pa) {
    u32 i, size = 0, bit, rest = 0x10000;

    if (first >= gUnk_030024F0) return;
    // (the size, 8.8: 0x10000 / pa, under 0x100 as pa is over it);
    for (bit = 0x80; bit; bit >>= 1)
        if (rest >= pa * bit) {
            rest -= pa * bit;
            size |= bit;
        }
    for (i = first; i < gUnk_030024F0; i++) {
        OamData *oam = &gUnk_030031C0[i];
        u32 attr0 = oam->all.attr0, attr1 = oam->all.attr1;
        s32 w, h, px, py;

        if (attr0 & 0x300 || attr0 >> 14 > 2) continue;  // (affine already, or hidden);
        w = sNidlRidePieces[attr0 >> 14][attr1 >> 14][0];
        h = sNidlRidePieces[attr0 >> 14][attr1 >> 14][1];
        px = attr1 & 0x1FF;
        if (px >= 256) px -= 512;
        py = attr0 & 0xFF;
        if (py >= 160) py -= 256;
        // (its middle, in half pixels, moved in toward the star's);
        px = x * 2 + (((px * 2 + w - x * 2) * (s32)size) >> 8);
        py = y * 2 + (((py * 2 + h - y * 2) * (s32)size) >> 8);
        oam->all.attr0 = (attr0 & ~0x3FF) | 0x100 | (((py - h) >> 1) & 0xFF);
        oam->all.attr1 = (attr1 & ~0xFFF) | (NIDL_RIDE_MATRIX << 9) | (((px - w) >> 1) & 0x1FF);
    }
    for (i = 0; i < 4; i++) {
        u16 *matrix = &gOamBuffer[(NIDL_RIDE_MATRIX + i * 8) * 16 + 3];
        matrix[0] = i & 1 ? -pa : pa;
        matrix[4] = 0;
        matrix[8] = 0;
        matrix[12] = i & 2 ? -pa : pa;
    }
}

// NidlRideDraw(); Kirby's drawing (sub_0803F46C); on a ride's step with a size, at that size (see above);
void NidlRideDraw(struct Kirby *kirby) {
    u32 first = gUnk_030024F0, pa = NidlRideSize(kirby);

    gNidlRide.draw(kirby);
    if (pa) NidlRideShrink(first, kirby->base.sprite.x, kirby->base.sprite.y, pa);
}

// NidlRideStar(); The task of the star Kirby rides (sub_0808EEF4, which draws it where he is); its size his (see above);
void NidlRideStar(void) {
    struct EffectObject *star = TaskGetStructPtr(gCurTask);
    struct Kirby *kirby = star->parent;
    u32 first = gUnk_030024F0, pa = kirby != NULL ? NidlRideSize(kirby) : 0;

    sub_0808EEF4();
    if (pa) NidlRideShrink(first, star->sprite.x, star->sprite.y, pa);
}
