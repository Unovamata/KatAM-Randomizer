/* Falling star; Mr. Shine's from the sky (bosses/shine_bright.c), ported from Kirby: Nightmare in Dream Land (USA) (its frames
 * 0x08754210's 38-43, Mr. Shine & Mr. Bright's third palette), as NIDL_STAR: no room has them, Mr. Shine makes them (behavior
 * NIDL_SPECIAL << 4 | 2) and sets their speed. Being a type 0x31 object, Kirby inhales one (it gives nothing) and spits it back as a
 * star, as in NiDL: what Kirby hits the boss with when he has no power (KatAM's bosses take only the stronger attacks). It flies
 * straight on, spinning, through anything, until it's off the screen (KatAM's despawn, as the enemies'); */
#include "../nidl.h"

#define SPIN_TIME 4
#define SPIN_FRAMES 6

static void StarMain(struct Object *obj) {
    struct NidlObject *n = (struct NidlObject *)obj;

    NidlUpdate(n);
    obj->base.flags |= 4;
    obj->kirby3 = FindTargetKirby(&obj->base);
    NidlSetFrame(n, (++obj->base.counter / SPIN_TIME) % SPIN_FRAMES);
}

// StarStart(); Behavior start, also after a hit (it flies on as it is);
void StarStart(struct Object *obj) {
    ObjectSetFunc(obj, -1, StarMain);
}

// StarInit(); Its boxes (16x16), through anything;
void StarInit(struct NidlObject *n) {
    struct Object *obj = &n->obj;

    obj->base.flags |= 0x100;
    obj->base.counter = 0;
    sub_0803E2B0(&obj->base, -6, -6, 6, 6);
    ObjectSetBounds(&obj->base, -6, -6, 6, 6);
}
