/* Cool Spook; ported from Kirby: Nightmare in Dream Land (USA), object type 0x13 (spawn 0x0807EDB4), as NiDL enemy NIDL_COOL_SPOOK.
 * NiDL scripts (16.16 speeds, Y down; KatAM's are 8.8, Y up); variants 0x08741300:
 * • Spawn: faces left, flaps (frames 4, 5, 6, 7 for 12 each, 0x087412EC), no gravity.
 * • Variant 0 (0x0807EE14, 0x0807EE84): flies left at 0.5, bobbing (0x0807EF7C: Y speeds -1.0, -0.5, 0, 0.5, 1.0, 0.5, 0, -0.5 down,
 *   8 frames each).
 * • Variant 1 (0x0807EEC4, 0x0807EF48): faces Kirby and bobs in place.
 * Its per frame skips NiDL's common checks (0x080692FC): no handlers, so nothing stops it; here it floats through walls and
 * water; */
#include "../nidl.h"

// Frames: NiDL's 4 to 7 (flapping), numbered from 0. Its hurt frame is NiDL's 3;
#define FRAME(nidl) ((nidl) - 4)

static const s16 sFlap[] = { FRAME(4), 12, FRAME(5), 12, FRAME(6), 12, FRAME(7), 12, NIDL_LOOP };
static const s16 sBob[] = { 0x100, 0x80, 0, -0x80, -0x100, -0x80, 0, 0x80 };   // 0x0807EF7C (Y up);
#define BOB_STEP 8
#define SPEED 0x80

struct CoolSpook {
    struct NidlObject n;
    u8 bobStep;
    u8 timer;
};

static void CoolSpookMain(struct Object *obj) {
    struct CoolSpook *cs = (struct CoolSpook *)obj;

    NidlUpdate(&cs->n);
    obj->base.flags |= 4;
    NidlAnimate(&cs->n);
    if (--cs->timer) return;
    cs->timer = BOB_STEP;
    obj->base.yspeed = sBob[cs->bobStep];
    if (++cs->bobStep == ARRAY_COUNT(sBob)) cs->bobStep = 0;
}

// CoolSpookStart(); Behavior start, also called by the engine after Cool Spook recovers from a hit;
void CoolSpookStart(struct Object *obj) {
    struct CoolSpook *cs = (struct CoolSpook *)obj;

    ObjectSetFunc(obj, 0, CoolSpookMain);
    obj->base.flags |= 0x100;
    if (cs->n.variant == 1) {
        NidlFaceKirby(obj);
        obj->base.xspeed = 0;
    } else {
        obj->base.flags |= 1;
        obj->base.xspeed = -SPEED;
    }
    obj->base.yspeed = 0;
    cs->bobStep = 0;
    cs->timer = 1;
    NidlPlay(&cs->n, sFlap);
}

// CoolSpookInit(); Its box; it flies;
void CoolSpookInit(struct NidlObject *n) {
    struct Object *obj = &n->obj;

    sub_0803E2B0(&obj->base, -6, -6, 6, 6);
    ObjectSetBounds(&obj->base, -7, -7, 7, 7);
    NIDL_SWIMS(obj);
}
