/* Coconut; ported from Kirby: Nightmare in Dream Land (USA), object type 0x17 (spawn 0x0808E8D4), as NiDL enemy NIDL_COCONUT.
 * NiDL scripts (16.16 speeds, Y down; KatAM's are 8.8, Y up):
 * • Variants 0 and 1 (0x0808E914, facing right or left): hang (frame 5) until Kirby is within 7 pixels sideways (0x0808E9D4),
 *   then fall (0x0808EA00) with gravity 0x1500, at 3.0 at most (tables 0x08743214, 0x0874321C: KatAM's own gravity, so
 *   NidlFall), tumbling every 2 frames and turning around 4 times per tumble. Landing (0x0808EB10) they explode (0x0806A344).
 * • Variant 2 (0x0808EB24): rocks on the ground (frames 4, 5, 6, 5, 6 frames each) and doesn't explode;
 * The explosion is KatAM's own small one (0x08073D2C, also used by Foley and Scarfy); */
#include "../nidl.h"
#include "code_0806F780.h"

// Frames: NiDL's 4 to 15 (rocking 4-6, hanging 5, tumbling 7-15), numbered from 0. Its hurt frame is NiDL's 3;
#define FRAME(nidl) ((nidl) - 4)

#define DROP_RANGE 7
#define TUMBLE_TIME 2

static const s16 sRock[] = { FRAME(4), 6, FRAME(5), 6, FRAME(6), 6, FRAME(5), 6, NIDL_LOOP };

// The tumble (0x0808EA00): a frame, and whether it turns around first;
static const u8 sTumble[][2] = {
    { FRAME(7), 0 }, { FRAME(8), 0 }, { FRAME(9), 0 }, { FRAME(8), 1 }, { FRAME(10), 0 }, { FRAME(11), 0 },
    { FRAME(12), 0 }, { FRAME(13), 1 }, { FRAME(14), 0 }, { FRAME(15), 0 }, { FRAME(14), 1 }, { FRAME(13), 0 },
    { FRAME(12), 1 }, { FRAME(11), 0 }, { FRAME(10), 0 }, { FRAME(9), 0 },
};

enum { STATE_HANG, STATE_FALL, STATE_ROCK };

struct Coconut {
    struct NidlObject n;
    u8 state;
    u8 step;
    u8 timer;
};

// Tumble(); The next tumbling frame, every TUMBLE_TIME frames;
static void Tumble(struct Coconut *c) {
    if (c->timer && --c->timer) return;
    c->timer = TUMBLE_TIME;
    if (sTumble[c->step][1]) c->n.obj.base.flags ^= 1;
    NidlSetFrame(&c->n, sTumble[c->step][0]);
    if (++c->step >= ARRAY_COUNT(sTumble)) c->step = 0;
}

static void CoconutMain(struct Object *obj) {
    struct Coconut *c = (struct Coconut *)obj;
    s32 dx;

    NidlUpdate(&c->n);
    obj->base.flags |= 4;

    switch (c->state) {
    case STATE_HANG:
        dx = (obj->kirby3->base.x - obj->base.x) >> 8;
        if (dx >= -DROP_RANGE && dx <= DROP_RANGE) {
            c->state = STATE_FALL;
            c->step = 0;
            c->timer = 0;
            Tumble(c);
        }
        break;
    case STATE_FALL:
        Tumble(c);
        obj->base.xspeed = 0;
        NidlFall(obj);
        if (obj->base.unk62 & 4) {
            obj->base.xspeed = obj->base.yspeed = 0;
            obj->base.flags |= 0x1000;
            sub_08073D2C(&obj->base);
        }
        break;
    case STATE_ROCK:
        NidlFall(obj);
        NidlAnimate(&c->n);
        break;
    }
}

// CoconutStart(); Behavior start, also called by the engine after Coconut recovers from a hit (it keeps falling if it was);
void CoconutStart(struct Object *obj) {
    struct Coconut *c = (struct Coconut *)obj;

    ObjectSetFunc(obj, 0, CoconutMain);
    obj->base.xspeed = 0;
    if (c->state == STATE_ROCK) {
        obj->base.yspeed = 0;
        NidlPlay(&c->n, sRock);
    } else if (c->state == STATE_HANG) {
        obj->base.yspeed = 0;
        NidlSetFrame(&c->n, FRAME(5));
    }
}

// CoconutInit(); Its box; variant 2 sits on the floor, the others hang where they are;
void CoconutInit(struct NidlObject *n) {
    struct Coconut *c = (struct Coconut *)n;
    struct Object *obj = &n->obj;

    sub_0803E2B0(&obj->base, -6, -6, 6, 6);
    ObjectSetBounds(&obj->base, -7, -7, 7, 7);
    c->step = 0;
    c->timer = 0;
    if (n->variant == 2) {
        c->state = STATE_ROCK;
        obj->base.unk4C = obj->base.y = ((obj->base.y + (obj->base.unk3F * 0x100)) & 0xfffff000) - (obj->base.unk3F * 0x100) - 1;
    } else {
        c->state = STATE_HANG;
        if (n->variant == 1)
            obj->base.flags |= 1;
        else
            obj->base.flags &= ~1;
    }
}
