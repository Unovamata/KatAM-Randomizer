/* Cappy; ported from Kirby: Nightmare in Dream Land (USA), object type 0x10 (spawn 0x0807E530), as NiDL enemy NIDL_CAPPY.
 * NiDL scripts (16.16 speeds, Y down; KatAM's are 8.8, Y up):
 * • Variant 0, 0x0807E640: faces Kirby and walks forward at 0.25 hopping (Y speed -1.0, gravity 1/16), its sprite turning
 *   around on every landing; after 40 hops it faces Kirby again.
 * • Variant 2, 0x0807E950: hops in place, turning around on every landing.
 * • Variant 1 is the capless Cappy NiDL spawns when Kirby inhales the cap (0x0807E814); it isn't ported; */
#include "../nidl.h"

// Frames: NiDL's 4 (standing). Its hurt frame is NiDL's 3;
#define FRAME_STAND 0

#define WALK_SPEED 0x40
#define HOP_SPEED 0x100
#define GRAVITY 0x10
#define MAX_FALL 0x300
#define HOPS 40

struct Cappy {
    struct NidlObject n;
    u8 hops;
    u8 airborne;
};

// Walk(); 0x0807E652: faces Kirby and walks towards it (in place for variant 2);
static void Walk(struct Cappy *cappy) {
    struct Object *obj = &cappy->n.obj;

    NidlFaceKirby(obj);
    obj->base.xspeed = cappy->n.variant == 2 ? 0 : (obj->base.flags & 1) ? -WALK_SPEED : WALK_SPEED;
    cappy->hops = HOPS;
}

static void Hop(struct Cappy *cappy) {
    cappy->n.obj.base.yspeed = HOP_SPEED;
    cappy->airborne = TRUE;
}

static void CappyMain(struct Object *obj) {
    struct Cappy *cappy = (struct Cappy *)obj;

    NidlUpdate(&cappy->n);
    obj->base.flags |= 4;

    obj->base.yspeed -= GRAVITY;
    if (obj->base.yspeed < -MAX_FALL) obj->base.yspeed = -MAX_FALL;

    if (!(obj->base.unk62 & 4) || obj->base.yspeed > 0) return;

    // Landed: the sprite turns around (not the walk), and it hops again;
    if (cappy->airborne) {
        cappy->airborne = FALSE;
        obj->base.flags ^= 1;
        if (cappy->n.variant != 2 && cappy->hops-- == 0) {
            Walk(cappy);
        }
    }
    Hop(cappy);
}

// CappyStart(); Behavior start, also called by the engine after Cappy recovers from a hit;
void CappyStart(struct Object *obj) {
    struct Cappy *cappy = (struct Cappy *)obj;

    ObjectSetFunc(obj, 0, CappyMain);
    NidlSetFrame(&cappy->n, FRAME_STAND);
    Walk(cappy);
    Hop(cappy);
}

// CappyInit(); Its box, standing on the floor;
void CappyInit(struct NidlObject *n) {
    struct Object *obj = &n->obj;

    sub_0803E2B0(&obj->base, -5, -4, 5, 7);
    ObjectSetBounds(&obj->base, -6, -5, 6, 9);
    obj->base.unk4C = obj->base.y = ((obj->base.y + (obj->base.unk3F * 0x100)) & 0xfffff000) - (obj->base.unk3F * 0x100) - 1;
}
