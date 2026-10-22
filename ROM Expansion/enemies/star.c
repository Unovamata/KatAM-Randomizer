/* Falling star; Mr. Shine's from the sky (bosses/shine_bright.c), ported from Kirby: Nightmare in Dream Land (USA) (its frames
 * 0x087542A8's 4, 5, the blink, and NiDL's shared star 0x0874C44C's 4-7), as NIDL_STAR: no room has them, Mr. Shine's fight makes
 * them (behavior NIDL_SPECIAL << 4 | 2) and moves them as NiDL's (its blink, its fall, its spin, gone off the screen: its frames, its
 * palette); King Dedede's fight the stars he knocks out (bosses/dedede.c: its frames 6-15, NiDL's shared 0x0874CCBC's 4-13). Being a
 * type 0x31 object, Kirby inhales one (it gives nothing) and spits it back as a star, as in NiDL: what Kirby hits the boss with when he
 * has no power (KatAM's bosses take only the stronger attacks). Let go (the fight over), or one nothing drives (Heavy Mole's:
 * StarFree()), it flies on at its speed, spinning, through anything, until it's off the screen (KatAM's despawn, as the enemies'); */
#include "../nidl.h"

#define SPIN_TIME 3
#define SPIN_FIRST 2
#define SPIN_LAST 5

static void StarMain(struct Object *obj) {
    struct NidlObject *n = (struct NidlObject *)obj;
    u8 k = obj->unk83 - n->animBase;

    NidlUpdate(n);
    obj->base.flags |= 4;
    obj->kirby3 = FindTargetKirby(&obj->base);
    // (Mr. Shine's fight moves it, its frames: counter 0; let go, 1 on: it spins its way, NiDL's 0x08064BCC: its frames 2-5 round
    // every 3 frames);
    if (!obj->base.counter || ++obj->base.counter <= SPIN_TIME) return;
    obj->base.counter = 1;
    if (k < SPIN_FIRST || k > SPIN_LAST) k = SPIN_FIRST;
    else if (obj->base.flags & 1) k = k == SPIN_LAST ? SPIN_FIRST : k + 1;
    else k = k == SPIN_FIRST ? SPIN_LAST : k - 1;
    NidlSetFrame(n, k);
}

// StarStart(); Behavior start, also after a hit (it flies on as it is);
void StarStart(struct Object *obj) {
    ObjectSetFunc(obj, -1, StarMain);
}

// StarFree(); A star nothing drives (Heavy Mole's): spinning its way from the start, moving at its speed;
void StarFree(struct Object *obj) {
    obj->base.counter = 1;
    NidlSetFrame((struct NidlObject *)obj, SPIN_FIRST);
}

// StarInit(); Its boxes (NiDL's: 10x10), through anything;
void StarInit(struct NidlObject *n) {
    struct Object *obj = &n->obj;

    obj->base.flags |= 0x100;
    obj->base.counter = 0;
    sub_0803E2B0(&obj->base, -5, -5, 5, 5);
    ObjectSetBounds(&obj->base, -5, -5, 5, 5);
}
