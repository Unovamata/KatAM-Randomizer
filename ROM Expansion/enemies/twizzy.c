/* Twizzy; ported from Kirby: Nightmare in Dream Land (USA), object type 0x18 (spawn 0x0808705C), as NiDL enemy NIDL_TWIZZY.
 * NiDL has 10 variants (table 0x08742570); these are ported (16.16 speeds, Y down; KatAM's are 8.8, Y up):
 * • Variant 0, 0x0808713C: flies forward at 0.75 swooping: it starts at 2.54 towards Kirby's height, accelerating back by 0x1900,
 *   and every 40 frames the swoop turns around (0x08087210). After 4 swoops up it keeps climbing away. It flaps every 8 frames
 *   (every 4 while accelerating upwards).
 * • Variant 5, 0x08087B8C: sits until Kirby is 63 pixels away or less, then takes off (0x08087C48: -3.0 for 4 frames, -2.0 for 8,
 *   -1.0 for 16, -0.5 for 8, then drifts down at 0.5), hovers 48 frames, faces Kirby and dashes at 1.0, 2.0, then 3.0 (0x08087D20).
 *   It can't be hit while taking off; */
#include "../nidl.h"

// Frames: NiDL's 4, 5 (flapping) and 6 (sitting). Its hurt frame is NiDL's 3;
enum { FRAME_WINGS_UP, FRAME_WINGS_DOWN, FRAME_SIT };

static const s16 sFlapFast[] = { FRAME_WINGS_UP, 4, FRAME_WINGS_DOWN, 4, NIDL_LOOP };  // 0x08742598;
static const s16 sFlapSlow[] = { FRAME_WINGS_UP, 8, FRAME_WINGS_DOWN, 8, NIDL_LOOP };  // 0x087425A4;

#define FLY_SPEED 0xC0
#define SWOOP_SPEED 0x28A
#define SWOOP_ACCEL 0x19
#define SWOOP_TIME 40
#define SWOOPS 4
#define MAX_CLIMB 0x400
#define SIT_RANGE 63

// Takeoff of variant 5: Y speed and frames to keep it (0x08087C48);
static const struct { s16 speed; u8 time; } sTakeoff[] = { { 0x300, 4 }, { 0x200, 8 }, { 0x100, 16 }, { 0x80, 8 } };

enum { STATE_SWOOP, STATE_SIT, STATE_TAKEOFF, STATE_HOVER, STATE_DASH };

struct Twizzy {
    struct NidlObject n;
    s16 accel;
    u8 state;
    u8 timer;
    u8 step;
    s8 swoops;
};

static void Face(struct Object *obj, s16 speed) {
    NidlFaceKirby(obj);
    obj->base.xspeed = (obj->base.flags & 1) ? -speed : speed;
}

static void SetFlap(struct Twizzy *tw) {
    NidlPlay(&tw->n, tw->accel > 0 ? sFlapFast : sFlapSlow);
}

// StartSwoop(); 0x0808713C: towards Kirby's height, accelerating back;
static void StartSwoop(struct Twizzy *tw) {
    struct Object *obj = &tw->n.obj;

    tw->state = STATE_SWOOP;
    Face(obj, FLY_SPEED);
    if (obj->base.y < obj->kirby3->base.y) {
        obj->base.yspeed = -SWOOP_SPEED;
        tw->accel = SWOOP_ACCEL;
    } else {
        obj->base.yspeed = SWOOP_SPEED;
        tw->accel = -SWOOP_ACCEL;
    }
    tw->swoops = SWOOPS;
    tw->timer = SWOOP_TIME;
    SetFlap(tw);
}

// Swoop(); 0x08087210: every 40 frames the swoop turns around, until the last one up;
static void Swoop(struct Twizzy *tw) {
    struct Object *obj = &tw->n.obj;

    obj->base.yspeed += tw->accel;
    if (obj->base.yspeed > MAX_CLIMB) obj->base.yspeed = MAX_CLIMB;
    if (!tw->timer || --tw->timer) return;

    // Accelerating upwards (swooping down): swoop up, unless it was the last; else swoop down;
    if (tw->accel > 0) {
        if (--tw->swoops < 0) return;
        obj->base.yspeed = SWOOP_SPEED;
        tw->accel = -SWOOP_ACCEL;
    } else {
        tw->swoops--;
        obj->base.yspeed = -SWOOP_SPEED;
        tw->accel = SWOOP_ACCEL;
    }
    tw->timer = SWOOP_TIME;
    SetFlap(tw);
}

static void Takeoff(struct Twizzy *tw) {
    struct Object *obj = &tw->n.obj;

    if (tw->timer && --tw->timer) return;
    if (tw->step == ARRAY_COUNT(sTakeoff)) {
        // Hovering, drifting down at 0.5 (0x08087D20), then the dash;
        tw->state = STATE_HOVER;
        obj->base.yspeed = -0x80;
        tw->timer = 48;
        return;
    }
    if (tw->step == 2) NidlPlay(&tw->n, sFlapSlow);
    obj->base.yspeed = sTakeoff[tw->step].speed;
    tw->timer = sTakeoff[tw->step].time;
    tw->step++;
}

static void Hover(struct Twizzy *tw) {
    struct Object *obj = &tw->n.obj;

    if (tw->timer && --tw->timer) return;
    switch (tw->step++) {
    case 4:
        obj->base.yspeed = 0;
        Face(obj, 0x100);
        tw->timer = 10;
        break;
    case 5:
        obj->base.xspeed *= 2;
        NidlPlay(&tw->n, sFlapFast);
        tw->timer = 10;
        break;
    default:
        tw->state = STATE_DASH;
        obj->base.xspeed = obj->base.xspeed / 2 * 3;
        break;
    }
}

static void TwizzyMain(struct Object *obj) {
    struct Twizzy *tw = (struct Twizzy *)obj;
    s32 distance;

    NidlUpdate(&tw->n);
    NidlAnimate(&tw->n);
    obj->base.flags |= 4;

    switch (tw->state) {
    case STATE_SWOOP:
        Swoop(tw);
        break;
    case STATE_SIT:
        distance = (obj->kirby3->base.x - obj->base.x) >> 8;
        if (distance < 0) distance = -distance;
        if (distance <= SIT_RANGE) {
            // 0x08087C48;
            tw->state = STATE_TAKEOFF;
            tw->step = 0;
            tw->timer = 0;
            NidlPlay(&tw->n, sFlapFast);
            Takeoff(tw);
        }
        break;
    case STATE_TAKEOFF:
        Takeoff(tw);
        break;
    case STATE_HOVER:
        Hover(tw);
        break;
    }
}

// TwizzyStart(); Behavior start, also called by the engine after Twizzy recovers from a hit: once flying, it dashes at Kirby;
void TwizzyStart(struct Object *obj) {
    struct Twizzy *tw = (struct Twizzy *)obj;

    ObjectSetFunc(obj, 0, TwizzyMain);
    obj->base.yspeed = 0;

    if (tw->n.variant != 5) {
        StartSwoop(tw);
    } else if (tw->state == STATE_SIT) {
        obj->base.xspeed = 0;
        NidlFaceKirby(obj);
        NidlSetFrame(&tw->n, FRAME_SIT);
        tw->n.sequence = NULL;
    } else {
        tw->state = STATE_HOVER;
        tw->step = 4;
        tw->timer = 1;
        NidlPlay(&tw->n, sFlapSlow);
    }
}

// TwizzyInit(); Its box; sitting ones stand on the floor;
void TwizzyInit(struct NidlObject *n) {
    struct Twizzy *tw = (struct Twizzy *)n;
    struct Object *obj = &n->obj;

    sub_0803E2B0(&obj->base, -5, -4, 5, 7);
    ObjectSetBounds(&obj->base, -6, -5, 6, 9);
    tw->state = STATE_SWOOP;
    if (n->variant == 5) {
        tw->state = STATE_SIT;
        obj->base.unk4C = obj->base.y = ((obj->base.y + (obj->base.unk3F * 0x100)) & 0xfffff000) - (obj->base.unk3F * 0x100) - 1;
    }
}
