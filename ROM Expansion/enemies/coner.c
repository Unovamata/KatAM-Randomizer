/* Coner; ported from Kirby: Nightmare in Dream Land (USA), object type 0x29 (spawn 0x0808FA10), as NiDL enemy NIDL_CONER.
 * NiDL scripts (16.16 speeds; KatAM's are 8.8):
 * • Variant 0 (0x0808FA50): faces Kirby, then scuttles forever (0x0808FAB4): 1.0 for 8 frames (frame 5), 0.5 for 8 (frame 6),
 *   0.25 for 16 (frame 5), stopped for 16 (frame 4); times from table 0x087432EC.
 * • Variant 1 (0x0808FBAC): faces Kirby and scuttles in place (0x0808FC00), same frames and times.
 * NiDL's water states (sinking 0x0808FB50, swimming 0x0808FB80) aren't ported; */
#include "../nidl.h"

// Frames: NiDL's 4 to 6 (standing 4, scuttling 5 and 6), numbered from 0. Its hurt frame is NiDL's 3;
#define FRAME(nidl) ((nidl) - 4)

// The scuttle: a frame, its speed and how long;
static const struct { u8 frame; u8 time; s16 speed; } sSteps[] = {
    { FRAME(5), 8, 0x100 },
    { FRAME(6), 8, 0x80 },
    { FRAME(5), 16, 0x40 },
    { FRAME(4), 16, 0 },
};

struct Coner {
    struct NidlObject n;
    u8 step;
    u8 timer;
};

static void Step(struct Coner *c) {
    struct Object *obj = &c->n.obj;
    s16 speed = c->n.variant == 1 ? 0 : sSteps[c->step].speed;

    NidlSetFrame(&c->n, sSteps[c->step].frame);
    c->timer = sSteps[c->step].time;
    obj->base.xspeed = (obj->base.flags & 1) ? -speed : speed;
}

static void ConerMain(struct Object *obj) {
    struct Coner *c = (struct Coner *)obj;

    NidlUpdate(&c->n);
    NidlFall(obj);
    obj->base.flags |= 4;

    if (!--c->timer) {
        if (++c->step >= ARRAY_COUNT(sSteps)) c->step = 0;
        Step(c);
    }
    NidlStopAtLedge(obj);
}

// ConerStart(); Behavior start, also called by the engine after Coner recovers from a hit;
void ConerStart(struct Object *obj) {
    struct Coner *c = (struct Coner *)obj;

    ObjectSetFunc(obj, 0, ConerMain);
    obj->base.yspeed = 0;
    NidlFaceKirby(obj);
    c->step = 0;
    Step(c);
}

// ConerInit(); Its box, on the floor;
void ConerInit(struct NidlObject *n) {
    struct Object *obj = &n->obj;

    sub_0803E2B0(&obj->base, -6, -4, 6, 7);
    ObjectSetBounds(&obj->base, -7, -5, 7, 9);
    obj->base.unk4C = obj->base.y = ((obj->base.y + (obj->base.unk3F * 0x100)) & 0xfffff000) - (obj->base.unk3F * 0x100) - 1;
}
