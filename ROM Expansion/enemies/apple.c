/* Apple; Whispy Woods' apples (bosses/whispy_woods.c), ported from Kirby: Nightmare in Dream Land (USA), object type 0x7D (spawn
 * 0x080B3050, states 0x0874C21C), as NIDL_APPLE: no room has them, Whispy Woods makes them (behavior NIDL_SPECIAL << 4 | 1). Being
 * a type 0x31 object, Kirby inhales it (it gives nothing) and spits it back as a star.
 * NiDL script (16.16 speeds, Y down; KatAM's are 8.8, Y up):
 * • State 0 (0x080B312C): facing Kirby, it flickers in the branches (frame 4 and none, 4 frames each, 6 times), then falls (gravity
 *   0x2500, 4.0 at most) spinning (0x080B3010: frames 4, 6... 18 of its table, for 15, 8 and then 4 frames each).
 * • Landing, state 1 (0x080B31A0): facing Kirby, it bounces at him (1.0, 2.0 up, gravity 0x1500); landing again, state 2
 *   (0x080B3214): 1.25, 1.0 up, gravity 0xC00; then state 3 (0x080B328C): it rolls on in little hops (1.8, 0.25 up, gravity 0x700)
 *   until it's off the screen (KatAM's despawn, as the enemies'). It spins every 4 frames (8 in states 2 and 3) once it bounces;
 * Its frames are Whispy Woods' 18-37 (its own table, 0x08754308), numbered from 0 here. Its hurt frame is its 4; */
#include "../nidl.h"

#define FRAME_FLICKER 4
#define FLICKERS 6
#define FLICKER_TIME 4

enum { STATE_FLICKER, STATE_FALL, STATE_BOUNCE_1, STATE_BOUNCE_2, STATE_ROLL };

// Per state: speed, height, gravity, frames per spin frame (state 0's falls spin 15, 8, then 4);
struct AppleBounce {
    s16 speed;
    s16 height;
    s16 gravity;
    u8 spin;
};
static const struct AppleBounce sBounces[] = {
    [STATE_FALL] = { 0, 0, 0x25, 4 },
    [STATE_BOUNCE_1] = { 0x100, 0x200, 0x15, 4 },
    [STATE_BOUNCE_2] = { 0x140, 0x100, 0x0C, 8 },
    [STATE_ROLL] = { 0x1CD, 0x40, 0x07, 8 },
};
#define MAX_FALL 0x400

struct Apple {
    struct NidlObject n;
    u8 state;
    u8 timer;
    u8 count;
    u8 spin;            // Its spinning frame (0x0874C24C: 4, 6... 18);
    u8 spinTimer;
    u8 spinSteps;       // Spin frames shown in state 0 (its first two are longer);
    bool8 airborne;
};

typedef char sAppleFits[sizeof(struct Apple) <= NIDL_OBJECT_SIZE ? 1 : -1];

// Spin(); 0x080B3010: the next spinning frame;
static void Spin(struct Apple *a) {
    u8 time;

    if (a->spinTimer && --a->spinTimer) return;
    a->spin = (a->spin + 1) & 7;
    NidlSetFrame(&a->n, 4 + a->spin * 2);
    time = sBounces[a->state].spin;
    if (a->state == STATE_FALL && a->spinSteps < 2) time = a->spinSteps++ ? 8 : 15;
    a->spinTimer = time;
}

static void Bounce(struct Apple *a, u8 state) {
    struct Object *obj = &a->n.obj;

    a->state = state;
    a->airborne = FALSE;
    NidlFaceKirby(obj);
    obj->base.xspeed = (obj->base.flags & 1) ? -sBounces[state].speed : sBounces[state].speed;
    obj->base.yspeed = sBounces[state].height;
}

static void AppleMain(struct Object *obj) {
    struct Apple *a = (struct Apple *)obj;

    NidlUpdate(&a->n);
    obj->base.flags |= 4;
    obj->kirby3 = FindTargetKirby(&obj->base);

    if (a->state == STATE_FLICKER) {
        obj->base.xspeed = obj->base.yspeed = 0;
        if (--a->timer) return;
        a->timer = FLICKER_TIME;
        // Shown and hidden in turn (NiDL's frame 4 and none);
        obj->base.flags ^= 0x400;
        if (++a->count < FLICKERS * 2) return;
        obj->base.flags &= ~0x400;
        a->state = STATE_FALL;
        a->airborne = TRUE;
        a->spin = 7;
        a->spinTimer = 0;
        a->spinSteps = 0;
    }

    obj->base.yspeed -= sBounces[a->state].gravity;
    if (obj->base.yspeed < -MAX_FALL) obj->base.yspeed = -MAX_FALL;
    Spin(a);
    if (!(obj->base.unk62 & 4)) {
        a->airborne = TRUE;
        return;
    }
    if (!a->airborne || obj->base.yspeed > 0) return;
    Bounce(a, a->state == STATE_FALL ? STATE_BOUNCE_1 : a->state == STATE_BOUNCE_1 ? STATE_BOUNCE_2 : STATE_ROLL);
}

// AppleStart(); Behavior start, also after a hit (it falls on from where it is);
void AppleStart(struct Object *obj) {
    struct Apple *a = (struct Apple *)obj;

    ObjectSetFunc(obj, -1, AppleMain);
    if (a->state == STATE_FLICKER) {
        a->timer = FLICKER_TIME;
        a->count = 0;
        NidlFaceKirby(obj);
        NidlSetFrame(&a->n, FRAME_FLICKER);
    }
}

// AppleInit(); Its boxes (16x16);
void AppleInit(struct NidlObject *n) {
    struct Apple *a = (struct Apple *)n;
    struct Object *obj = &n->obj;

    a->state = STATE_FLICKER;
    sub_0803E2B0(&obj->base, -6, -6, 6, 8);
    ObjectSetBounds(&obj->base, -7, -7, 7, 8);
}
