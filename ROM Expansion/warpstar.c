/* WarpStar; The ported NiDL warp stars' rides (KatAMExpansionRooms.cs, WarpFlights()): each is KatAM's own warp star (object 0x84)
 * flying a flight of its own (its subtype: NIDL_RIDE_FLIGHT and after), NiDL's ride step by step: the star's moves frame by frame,
 * as NiDL's star makes them, through each room NiDL's ride goes through (a sky it crosses, the room it lands in), down to where
 * Kirby hops off. Four of KatAM's routines are detoured here, and one task swapped (KatAMExpansionRooms.cs, WarpRideDetours()):
 * • the warp star's rev (sub_0800C270): KatAM's star waits 300 frames for the helpers before it's off, NiDL's goes at once: a
 *   ported star's (its subtype NIDL_RIDE_FLIGHT and after) skips the wait;
 * • a flight's room change (sub_08055D9C, a flight step flagged 0x40): KatAM's flights change room once, to the star's room; a
 *   step marked NIDL_RIDE_ROOM (its +8) takes Kirby to the room in its +0xA instead, at the cell in its +0 and +2 (the step after
 *   it moves him to where NiDL's star comes in), so a ride goes on through several rooms; one where he comes in standing (its
 *   animation not the rider's: the ending's, into the Fountain of Dreams hub) lets his star go there, quietly (KatAM's goes with a
 *   sound once he's off it, sub_0808EEF4);
 * • a flight's step (sub_080534D0): a ride flies Kirby's tumble off the star as NiDL's (the orb where the star went, then KatAM's
 *   tumbling and bouncing animations) to where he stands, and ends there (its end step marked NIDL_RIDE_HOP, not 0x80: KatAM's
 *   hop-off would throw him off again): the flight's ending, KatAM's plain one (sub_080534D0: back to standing or falling), left as
 *   KatAM's hop-off leaves him (the flight's flags off), facing the end step's way;
 * • Kirby's drawing (sub_0803F46C), and the star he rides (sub_0808EEF4's task, its function swapped for NidlRideStar): where a
 *   ride's step has a size (its +0xA, NiDL's: the star flying off into the distance), both are drawn at that size around the
 *   star's middle, as NiDL draws its star and rider (all their pieces scaled by one affine matrix, each moved in toward the
 *   middle);
 * The Power Orb's star (finale.c) flies a ride of its own: its flight's first step (NIDL_RIDE_FINALE) holds the Kirbys on it while
 * the orb's end (bosses/power_orb.c's PowerOrbEnd()) moves them, NiDL's ride up off the screen and into the moon, smaller and smaller
 * (that step and the fade's after it ask their size: NIDL_RIDE_ASK, finale.c's NidlFinaleRideSize()), hidden at last (Kirby's flag
 * 0x400, the star's: the orb's end draws a dot there), until it moves them on to the fade (hidden too: its flag 4); after the room
 * change another such step holds them in Nightmare's room, where his intro moves them (bosses/nightmare.c, NidlFinaleHold()); */
#include "global.h"
#include "kirby.h"
#include "warp_star.h"
#include "functions.h"
#include "main.h"
#include "task.h"

void sub_080534D0(struct Kirby *);  // a flight's step (kirby.c);
void sub_0808EEF4(void);            // the star Kirby rides, following him (code_0806F780.c, sub_0808EDB8's task);
void NidlRideStar(void);
bool32 NidlFinaleStar(struct Object *);    // the Power Orb's warp star (finale.c);
bool32 NidlFinaleAllAboard(struct Object *, u8);
u32 NidlFinaleRideSize(u16 room);         // the finale's rides' size, and a Kirby held on one (finale.c);
void NidlFinaleHold(struct Kirby *);
bool32 NidlFinaleFlight(struct Kirby *);  // the finale's flights, without the pause (finale.c);

#define NIDL_RIDE_FLIGHT 6    // the first warp star subtype (flight) of the ported warp stars (KatAMExpansionRooms.cs, WarpFlight);
#define NIDL_RIDE_ROOM 0xFE   // a flight step's +8 on a room change (+9 0x40) to the room in its +0xA;
#define NIDL_RIDE_REV 0x12C   // how long KatAM's warp star revs (sub_0800C270: launched once its count passes this);
#define NIDL_RIDE_HOP 0xFD    // a flight's end step's +8: a ride's end, Kirby standing where he tumbled to;
#define NIDL_RIDE_MATRIX 7    // the affine matrices of a drawn ride: 7, then 15, 23, 31 for pieces flipped (KatAM's own use 0);
#define NIDL_RIDE_FINALE 0xFC // a flight step's +8: the Power Orb's ride's first, Kirby held on it (bosses/power_orb.c moves him);
#define NIDL_RIDE_ASK 0xFFFF  // a flight step's +0xA: the size the finale's ride says (NidlFinaleRideSize());
#define NIDL_RIDE_HIDDEN 0x8000 // (NidlFinaleRideSize(): not shown: too small, the orb's end draws a dot there, or off the screen);
#define NIDL_RIDE_ALONE 0x4000  // (and: the Kirbys at their size, the star gone: Nightmare's intro, knocked off it);
#define NIDL_RIDE_SIZED(pa) ((pa) && (pa) != NIDL_RIDE_HIDDEN && (pa) != NIDL_RIDE_ALONE)
#define NIDL_RIDER 0x5A         // (Kirby's animations on a star: KatAM's 90, and 91; sub_0808EEF4 lets the star go on any other);

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
// which waits for the helpers, as KatAM's, until they're all on it: every Kirby rides it to Nightmare's room, finale.c);
void NidlRideRev(struct WarpStar *ws) {
    if (ws->unk0.unkB4 >= NIDL_RIDE_FLIGHT && ws->unkBC < NIDL_RIDE_REV
        && (!NidlFinaleStar(&ws->unk0.obj) || NidlFinaleAllAboard(&ws->unk0.obj, ws->unk0.unkB5)))
        ws->unkBC = NIDL_RIDE_REV;
    gNidlRide.rev(ws);
}

// NidlRideDropStar(); The star a Kirby rides let go (its task ends: sub_0808EEF4 destroys it, without its sound);
static void NidlRideDropStar(struct Kirby *kirby) {
    struct Task *t = &gTasks[0];
    u8 n;

    for (n = 0; n < MAX_TASK_NUM && t->next; n++) {
        struct EffectObject *star;

        t = (struct Task *)(IWRAM_START + t->next);
        if (t->main != NidlRideStar) continue;
        star = TaskGetStructPtr(t);
        if (star->parent == kirby) star->flags |= 0x1000;
    }
}

// NidlRideRoomChange(); A room change (sub_08055D9C); on a ride's (its flight step marked), to the step's room and cell (coming in
// standing, his star let go);
void NidlRideRoomChange(struct Kirby *kirby) {
    const struct Unk_08353510 *step = kirby->unk114;
    if (kirby->stateFn == sub_080534D0 && step != NULL && step->unk9 & 0x40 && step->unk8 == NIDL_RIDE_ROOM) {
        kirby->roomId = ((const u16 *)step)[5];
        kirby->spawnLocation.x = step->unk0;
        kirby->spawnLocation.y = step->unk2;
        if (step->unk6 != NIDL_RIDER && step->unk6 != NIDL_RIDER + 1) NidlRideDropStar(kirby);
    }
    gNidlRide.roomChange(kirby);
}

// NidlRideStep(); A flight's step (sub_080534D0); at a ride's end, Kirby left where he stands (see above), as KatAM's hop-off leaves
// him (sub_080534D0's flight end with the 0x80 flag);
void NidlRideStep(struct Kirby *kirby) {
    const struct Unk_08353510 *step = kirby->unk114;

    // (no pause on the finale's flights, its cutscenes: START taken before Kirby's update checks it, finale.c's NidlFinaleFlight());
    if (NidlFinaleFlight(kirby)) kirby->unk11A &= ~START_BUTTON;
    // (the Power Orb's ride: held, the orb's end moving him; hidden when it says, as a flight step's flag 4 hides him);
    if (step != NULL && step->unk8 == NIDL_RIDE_FINALE && !(step->unk9 & 0x40)) {
        kirby->base.flags |= 4;
        kirby->base.xspeed = kirby->base.yspeed = 0;
        NidlFinaleHold(kirby);
        if (NidlFinaleRideSize(kirby->base.roomId) == NIDL_RIDE_HIDDEN) kirby->base.flags |= 0x400;
        else kirby->base.flags &= ~0x400;
        return;
    }
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
    if (pa == NIDL_RIDE_ASK) return NidlFinaleRideSize(kirby->base.roomId);
    return pa > 0x100 && pa < 0x8000 ? pa : 0;
}

/* Shrink(); The OAM pieces drawn since `first` (gUnk_030031C0, up to gUnk_030024F0) at the size of `pa` around (x, y), the
 * sprite's spot on screen: each piece scaled by an affine matrix (`matrix`, then +8, +16, +24 for its flips: attr1 bits 12, 13
 * become its bits 3, 4) and moved in toward (x, y): its middle by as much (NidlShrink(): as NiDL draws its star and rider on its
 * rides), or as NiDL's own sprites place theirs (NidlShrinkPieces(), 0x08001E38: its corner's offset scaled, half a pixel toward
 * the spot, less half of how much it shrinks); */
static void Shrink(u32 first, s32 x, s32 y, u32 pa, u32 matrix, bool32 pieces) {
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
        if (pieces) {
            px = (px - x) * (s32)size;
            py = (py - y) * (s32)size;
            px = x + ((px >= 0 ? px - 0x80 : px + 0x80) >> 8) - (w / 2 - ((w / 2 * size) >> 8));
            py = y + ((py >= 0 ? py - 0x80 : py + 0x80) >> 8) - (h / 2 - ((h / 2 * size) >> 8));
        } else {
            // (its middle, in half pixels, moved in toward the star's);
            px = (x * 2 + (((px * 2 + w - x * 2) * (s32)size) >> 8) - w) >> 1;
            py = (y * 2 + (((py * 2 + h - y * 2) * (s32)size) >> 8) - h) >> 1;
        }
        oam->all.attr0 = (attr0 & ~0x3FF) | 0x100 | (py & 0xFF);
        oam->all.attr1 = (attr1 & ~0xFFF) | (matrix << 9) | (px & 0x1FF);
    }
    for (i = 0; i < 4; i++) {
        u16 *m = &gOamBuffer[(matrix + i * 8) * 16 + 3];
        m[0] = i & 1 ? -pa : pa;
        m[4] = 0;
        m[8] = 0;
        m[12] = i & 2 ? -pa : pa;
    }
}

// NidlShrink(); Shrink(), its pieces' middles moved in (see above): the rides' star and rider;
void NidlShrink(u32 first, s32 x, s32 y, u32 pa, u32 matrix) {
    Shrink(first, x, y, pa, matrix, FALSE);
}

// NidlShrinkPieces(); Shrink(), its pieces placed as NiDL's sprites' (see above): the Power Orb's and its star's flights into the moon;
void NidlShrinkPieces(u32 first, s32 x, s32 y, u32 pa, u32 matrix) {
    Shrink(first, x, y, pa, matrix, TRUE);
}

// NidlRideDraw(); Kirby's drawing (sub_0803F46C); on a ride's step with a size, at that size (see above);
void NidlRideDraw(struct Kirby *kirby) {
    u32 first = gUnk_030024F0, pa = NidlRideSize(kirby);

    gNidlRide.draw(kirby);
    if (NIDL_RIDE_SIZED(pa)) NidlShrink(first, kirby->base.sprite.x, kirby->base.sprite.y, pa, NIDL_RIDE_MATRIX);
}

// NidlRideStar(); The task of the star Kirby rides (sub_0808EEF4, which draws it where he is); its size his (see above);
void NidlRideStar(void) {
    struct EffectObject *star = TaskGetStructPtr(gCurTask);
    struct Kirby *kirby = star->parent;
    u32 first = gUnk_030024F0, pa = kirby != NULL ? NidlRideSize(kirby) : 0;

    // (hidden: EffectObjectPostUpdate draws it only without its flag 0x400; the Power Orb's ride's, shown again);
    if (pa == NIDL_RIDE_HIDDEN || pa == NIDL_RIDE_ALONE) star->flags |= 0x400;
    else if (kirby != NULL && kirby->stateFn == sub_080534D0 && kirby->unk114 != NULL && ((const u16 *)kirby->unk114)[5] == NIDL_RIDE_ASK)
        star->flags &= ~0x400;
    sub_0808EEF4();
    if (NIDL_RIDE_SIZED(pa)) NidlShrink(first, star->sprite.x, star->sprite.y, pa, NIDL_RIDE_MATRIX);
}
