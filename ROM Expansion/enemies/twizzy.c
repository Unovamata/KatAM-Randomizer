/* Twizzy; ported from Kirby: Nightmare in Dream Land (USA), object type 0x18 (spawn 0x0808705C), as NiDL enemy NIDL_TWIZZY.
 * NiDL has 10 variants (table 0x08742570); these are ported (16.16 speeds, Y down; KatAM's are 8.8, Y up):
 * • Variant 0, 0x0808713C: flies forward at 0.75 swooping: it starts at 2.54 towards Kirby's height, accelerating back by 0x1900,
 *   and every 40 frames the swoop turns around (0x08087210). After 4 swoops up it keeps climbing away. It flaps every 8 frames
 *   (every 4 while accelerating upwards).
 * • Variant 1, 0x080872E0: the same, swooping at 1.93 (accelerating back by 0x1300; tables 0x087425C0, 0x087425C8) forever; halfway
 *   through each swoop (0x080873B0), if Kirby is more than 32 pixels above or below, it keeps going towards Kirby's height
 *   (accelerating back by 0xD00 only) or turns sooner (0x2000).
 * • Variant 2, 0x08087458: ambushes from the top of the room, above Kirby (at Kirby's x when Kirby is within 31 pixels vertically,
 *   else 72 or 40 pixels behind, 56 or 88 ahead, at random; table 0x087425DC), falling (gravity 0x2500, at 3.0 at most) until it's
 *   15 pixels above Kirby's height (0x08087724); then it loops (0x0808752C), backwards and down, then up and forward, in steps of 4
 *   frames (0x087425F0) with the gravity still on, and goes on as variant 0.
 * • Variant 3, 0x08087790: flies diagonally at 1.0 (0x08742600), towards Kirby's side and height (0x0806443C gives the direction,
 *   0x0806421C the speeds), bouncing off walls, floors and ceilings (0x08087848, 0x08086F54), flapping fast for 20 frames after
 *   every bounce.
 * • Variant 4, 0x0808795C (0x080879D4, 0x08087A98): follows Kirby for 384 frames, facing it: every 8 frames it heads towards Kirby
 *   again (0x080645A4: 8 directions), accelerating by 0x3300 a frame up to 1.0 (0x08742614, 0x08742624); it flaps fast going up,
 *   holds its wings going down. Then it turns away and flies off at 0.4, rising 0x1900 faster every frame.
 * • Variant 5, 0x08087B8C: sits until Kirby is 63 pixels away or less, then takes off (0x08087C48: -3.0 for 4 frames, -2.0 for 8,
 *   -1.0 for 16, -0.5 for 8, then drifts down at 0.5), hovers 48 frames, faces Kirby and dashes at 1.0, 2.0, then 3.0 (0x08087D20).
 *   It can't be hit while taking off.
 * • Variant 6, 0x08087E2C: on the ground: sits (frame 6) 24 frames, faces Kirby, sits 24 more (0x08087EB4), then hops straight up
 *   (0x08087F18: 2.0, gravity 0x800, falling at 3.0 at most), flapping 4 times, and flutters back down flapping (0x08087F8C).
 * • Variant 7, 0x08087FCC: on the ground: sits 15 frames facing Kirby (0x08088054), then at random (of 8) hops in place (2), hops
 *   1.0 (2) or 2.0 forward (1) (0x080881A0: 1.5 up, gravity 0x4000, turning around half the time), or sits on for 32 (1) or 64
 *   frames (2). When Kirby comes within 50 pixels sideways and 64 up or down while it sits (0x080880FC, 0x0874269C, 0x087426A4),
 *   it takes off (0x0808827C: 3.0 up, gravity 0x1500, flapping) and goes on as variant 4.
 * • Variant 8, 0x08088360 (0x080883DC, 0x08088478): hovers in place, bobbing (Y speeds 1.0 up, 0.5 up, 0.5 down, 1.0 down, 0.5
 *   down, 0, 0.5 up, 8 frames each, flapping with them), facing Kirby every 8 frames.
 * • Variant 9, 0x08088498: sits on the ground (frame 6) facing Kirby, and never does anything else; */
#include "../nidl.h"

// Frames: NiDL's 4, 5 (flapping) and 6 (sitting). Its hurt frame is NiDL's 3;
enum { FRAME_WINGS_UP, FRAME_WINGS_DOWN, FRAME_SIT };

static const s16 sFlapFast[] = { FRAME_WINGS_UP, 4, FRAME_WINGS_DOWN, 4, NIDL_LOOP };  // 0x08742598;
static const s16 sFlapSlow[] = { FRAME_WINGS_UP, 8, FRAME_WINGS_DOWN, 8, NIDL_LOOP };  // 0x087425A4;

#define FLY_SPEED 0xC0
#define SWOOP_SPEED 0x28A
#define SWOOP_ACCEL 0x19
#define FOLLOW_SPEED 0x1EE
#define FOLLOW_ACCEL 0x13
#define FOLLOW_RANGE 32
#define SWOOP_TIME 40
#define SWOOPS 4
#define MAX_CLIMB 0x400
#define SIT_RANGE 63

// Takeoff of variant 5: Y speed and frames to keep it (0x08087C48);
static const struct { s16 speed; u8 time; } sTakeoff[] = { { 0x300, 4 }, { 0x200, 8 }, { 0x100, 16 }, { 0x80, 8 } };

// The loop of variant 2 (0x0808752C): speeds (X forward, Y up), 4 frames each, the last one 16;
static const s16 sLoop[][2] = {
    { -0x100, -0x400 }, { -0x200, -0x300 }, { -0x300, -0x200 }, { -0x200, 0x80 }, { -0x100, 0x100 }, { -0x80, 0x200 },
    { 0x100, 0x300 }, { 0x140, 0x200 }, { 0x140, 0x100 }, { 0x140, 0x80 }, { 0x140, -0x80 },
};
static const s16 sAmbushOffsets[] = { -72, -40, 56, 88 };  // 0x087425DC;

#define AMBUSH_GRAVITY 0x25
#define AMBUSH_MAX_FALL 0x300
#define AMBUSH_NEAR 31
#define AMBUSH_LEVEL 15
#define LOOP_STEP 4
#define DIAGONAL_SPEED 0xB5     // 1.0 at 45 degrees;
#define BOUNCE_FLAP_TIME 20
#define FOLLOW_TIME 384
#define CHASE_ACCEL 0x33
#define CHASE_SPEED 0x100
#define CHASE_AIM 8
#define LEAVE_SPEED 0x66
#define LEAVE_ACCEL 0x19
#define HOP_SPEED 0x200
#define HOP_GRAVITY 0x8
#define HOP_MAX_FALL 0x300
#define HOP_SIT_TIME 24
#define HOP_FLAPS 4
#define PERCH_SIT_TIME 15
#define PERCH_HOP_SPEED 0x180
#define PERCH_GRAVITY 0x40
#define PERCH_NEAR_X 50
#define PERCH_NEAR_Y 64
#define PERCH_TAKEOFF 0x300

enum { STATE_SWOOP, STATE_SIT, STATE_TAKEOFF, STATE_HOVER, STATE_DASH, STATE_DROP, STATE_LOOP, STATE_BOUNCE, STATE_CHASE, STATE_LEAVE, STATE_HOP_SIT, STATE_HOP, STATE_PERCH, STATE_PERCH_HOP, STATE_PERCH_WAIT, STATE_PERCH_TAKEOFF, STATE_BOB, STATE_STILL };

struct Twizzy {
    struct NidlObject n;
    s16 accel;
    u8 state;
    u8 timer;
    u8 step;
    s8 swoops;
    u8 placed;
    u16 chaseTimer;
    s8 aimX, aimY;      // Where it heads for variant 4: -1, 0, 1 (Y: 1 is up);
};

static void Face(struct Object *obj, s16 speed) {
    NidlFaceKirby(obj);
    obj->base.xspeed = (obj->base.flags & 1) ? -speed : speed;
}

static void SetFlap(struct Twizzy *tw) {
    NidlPlay(&tw->n, tw->accel > 0 ? sFlapFast : sFlapSlow);
}

// SwoopSpeed(), SwoopAccel(); Variant 1's swoops are slower;
static s16 SwoopSpeed(struct Twizzy *tw) { return tw->n.variant == 1 ? FOLLOW_SPEED : SWOOP_SPEED; }
static s16 SwoopAccel(struct Twizzy *tw) { return tw->n.variant == 1 ? FOLLOW_ACCEL : SWOOP_ACCEL; }

// StartSwoop(); 0x0808713C, 0x080872E0: towards Kirby's height, accelerating back;
static void StartSwoop(struct Twizzy *tw) {
    struct Object *obj = &tw->n.obj;

    tw->state = STATE_SWOOP;
    Face(obj, FLY_SPEED);
    if (obj->base.y < obj->kirby3->base.y) {
        obj->base.yspeed = -SwoopSpeed(tw);
        tw->accel = SwoopAccel(tw);
    } else {
        obj->base.yspeed = SwoopSpeed(tw);
        tw->accel = -SwoopAccel(tw);
    }
    tw->swoops = SWOOPS;
    tw->timer = SWOOP_TIME;
    SetFlap(tw);
}

// Follow(); 0x080873B0: halfway through a swoop, towards Kirby's height when it's far above or below;
static void Follow(struct Twizzy *tw) {
    struct Object *obj = &tw->n.obj;
    s32 dy = (obj->kirby3->base.y - obj->base.y) >> 8;

    if (dy >= -FOLLOW_RANGE && dy <= FOLLOW_RANGE) return;
    if (tw->accel > 0)
        tw->accel = dy < 0 ? 0x20 : 0xD;      // Swooping down: Kirby above turns it sooner;
    else
        tw->accel = dy < 0 ? -0xD : -0x20;    // Swooping up: Kirby above keeps it going;
}

// Swoop(); 0x08087210: every 40 frames the swoop turns around, until the last one up (variant 1: forever, following Kirby);
static void Swoop(struct Twizzy *tw) {
    struct Object *obj = &tw->n.obj;

    obj->base.yspeed += tw->accel;
    if (tw->n.variant == 1) {
        if (tw->timer == SWOOP_TIME / 2) Follow(tw);
        if (--tw->timer) return;
        obj->base.yspeed = tw->accel > 0 ? FOLLOW_SPEED : -FOLLOW_SPEED;
        tw->accel = tw->accel > 0 ? -FOLLOW_ACCEL : FOLLOW_ACCEL;
        tw->timer = SWOOP_TIME;
        SetFlap(tw);
        return;
    }
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

// Ambush(); 0x08087458: to the top of the room, above Kirby;
static void Ambush(struct Twizzy *tw) {
    struct Object *obj = &tw->n.obj;
    s32 dy = (obj->kirby3->base.y - obj->base.y) >> 8;

    obj->base.x = obj->kirby3->base.x;
    if (dy > AMBUSH_NEAR) obj->base.x += sAmbushOffsets[Rand16() % ARRAY_COUNT(sAmbushOffsets)] * 0x100;
    obj->base.y = gCurLevelInfo[obj->base.unk56].levelMinPosition.y;
    obj->base.flags |= 0x100;   // Through the ceiling until it's in the room;
    tw->state = STATE_DROP;
    obj->base.xspeed = obj->base.yspeed = 0;
    NidlFaceKirby(obj);
    tw->n.sequence = NULL;
    NidlSetFrame(&tw->n, FRAME_WINGS_UP);
}

// Loop(); 0x0808752C: the next step of the loop, then variant 0's swoops;
static void Loop(struct Twizzy *tw) {
    struct Object *obj = &tw->n.obj;

    obj->base.yspeed -= AMBUSH_GRAVITY;
    if (tw->timer && --tw->timer) return;
    if (tw->step == ARRAY_COUNT(sLoop)) {
        tw->n.variant = 0;
        StartSwoop(tw);
        return;
    }
    if (tw->step == 3) NidlPlay(&tw->n, sFlapFast);
    if (tw->step == 10) NidlPlay(&tw->n, sFlapSlow);
    obj->base.xspeed = (obj->base.flags & 1) ? -sLoop[tw->step][0] : sLoop[tw->step][0];
    obj->base.yspeed = sLoop[tw->step][1];
    tw->timer = tw->step == ARRAY_COUNT(sLoop) - 1 ? 16 : LOOP_STEP;
    tw->step++;
}

// StartBounce(); 0x08087790: diagonally towards Kirby;
static void StartBounce(struct Twizzy *tw) {
    struct Object *obj = &tw->n.obj;

    tw->state = STATE_BOUNCE;
    NidlFaceKirby(obj);
    obj->base.xspeed = (obj->base.flags & 1) ? -DIAGONAL_SPEED : DIAGONAL_SPEED;
    obj->base.yspeed = obj->base.y < obj->kirby3->base.y ? -DIAGONAL_SPEED : DIAGONAL_SPEED;
    tw->timer = 0;
    NidlPlay(&tw->n, sFlapSlow);
}

// Bounce(); 0x08087848: off walls, floors and ceilings, flapping fast for a while;
static void Bounce(struct Twizzy *tw) {
    struct Object *obj = &tw->n.obj;
    bool8 bounced = FALSE;

    if ((obj->base.unk62 & 3) && obj->base.x == obj->base.unk48) {
        obj->base.xspeed = -obj->base.xspeed;
        obj->base.flags ^= 1;
        bounced = TRUE;
    }
    if (((obj->base.unk62 & 4) && obj->base.yspeed < 0) || ((obj->base.unk62 & 8) && obj->base.yspeed > 0)) {
        obj->base.yspeed = -obj->base.yspeed;
        bounced = TRUE;
    }
    if (bounced) {
        tw->timer = BOUNCE_FLAP_TIME;
        NidlPlay(&tw->n, sFlapFast);
    } else if (tw->timer && !--tw->timer) {
        NidlPlay(&tw->n, sFlapSlow);
    }
}

// Approach(); A speed accelerating towards a direction, up to the chase speed;
static s16 Approach(s16 speed, s8 aim) {
    if (aim > 0 && (speed += CHASE_ACCEL) > CHASE_SPEED) speed = CHASE_SPEED;
    if (aim < 0 && (speed -= CHASE_ACCEL) < -CHASE_SPEED) speed = -CHASE_SPEED;
    return speed;
}

// Chase(); 0x08087A98: towards Kirby, aiming again every 8 frames, then away;
static void Chase(struct Twizzy *tw) {
    struct Object *obj = &tw->n.obj;
    s32 dx, dy;
    s8 aimY;

    if (!tw->chaseTimer--) {
        // 0x08087A56: turns away from Kirby and flies off rising;
        tw->state = STATE_LEAVE;
        NidlFaceKirby(obj);
        obj->base.flags ^= 1;
        obj->base.xspeed = (obj->base.flags & 1) ? -LEAVE_SPEED : LEAVE_SPEED;
        NidlPlay(&tw->n, sFlapFast);
        return;
    }
    NidlFaceKirby(obj);
    if (!tw->timer || !--tw->timer) {
        tw->timer = CHASE_AIM;
        dx = (obj->kirby3->base.x - obj->base.x) >> 8;
        dy = (obj->kirby3->base.y - obj->base.y) >> 8;
        tw->aimX = dx > 4 ? 1 : dx < -4 ? -1 : 0;
        aimY = dy > 4 ? -1 : dy < -4 ? 1 : 0;
        if (aimY != tw->aimY || !tw->n.sequence) {
            // 0x080879D4: flapping fast going up, wings held going down;
            if (aimY > 0)
                NidlPlay(&tw->n, sFlapFast);
            else if (aimY < 0) {
                tw->n.sequence = NULL;
                NidlSetFrame(&tw->n, FRAME_WINGS_UP);
            } else
                NidlPlay(&tw->n, sFlapSlow);
        }
        tw->aimY = aimY;
    }
    obj->base.xspeed = Approach(obj->base.xspeed, tw->aimX);
    obj->base.yspeed = Approach(obj->base.yspeed, tw->aimY);
}

// HopSit(); 0x08087EB4: sits, facing Kirby halfway, then hops;
static void HopSit(struct Twizzy *tw) {
    struct Object *obj = &tw->n.obj;

    NidlFall(obj);
    if (--tw->timer) return;
    if (!tw->step) {
        tw->step = 1;
        tw->timer = HOP_SIT_TIME;
        NidlFaceKirby(obj);
        return;
    }
    // 0x08087F18: up, flapping 4 times (32 frames), then fluttering down;
    tw->state = STATE_HOP;
    obj->base.yspeed = HOP_SPEED;
    tw->timer = HOP_FLAPS * 8;
    NidlPlay(&tw->n, sFlapFast);
}

static void StartHopSit(struct Twizzy *tw) {
    tw->state = STATE_HOP_SIT;
    tw->step = 0;
    tw->timer = HOP_SIT_TIME;
    tw->n.obj.base.xspeed = 0;
    tw->n.sequence = NULL;
    NidlSetFrame(&tw->n, FRAME_SIT);
}

// Hop(); 0x08087F18, 0x08087F8C: the hop, then fluttering down until it lands;
static void Hop(struct Twizzy *tw) {
    struct Object *obj = &tw->n.obj;

    obj->base.yspeed -= HOP_GRAVITY;
    if (obj->base.yspeed < -HOP_MAX_FALL) obj->base.yspeed = -HOP_MAX_FALL;
    if (tw->timer && !--tw->timer) {
        tw->n.sequence = NULL;
        NidlSetFrame(&tw->n, FRAME_WINGS_UP);
    }
    if (obj->base.yspeed <= 0 && !tw->timer && !tw->n.sequence) NidlPlay(&tw->n, sFlapFast);
    if ((obj->base.unk62 & 4) && obj->base.yspeed <= 0) {
        obj->base.yspeed = 0;
        StartHopSit(tw);
    }
}

// Perch(); 0x08088054: sits facing Kirby;
static void Perch(struct Twizzy *tw) {
    struct Object *obj = &tw->n.obj;

    tw->state = STATE_PERCH;
    tw->timer = PERCH_SIT_TIME;
    obj->base.xspeed = 0;
    NidlFaceKirby(obj);
    tw->n.sequence = NULL;
    NidlSetFrame(&tw->n, FRAME_SIT);
}

// PerchChoose(); 0x0808807C: a hop or a wait, at random;
static void PerchChoose(struct Twizzy *tw) {
    struct Object *obj = &tw->n.obj;
    static const s16 sSpeeds[] = { 0, 0, 0x100, 0x100, 0x200 };
    u8 r = Rand16() % 8;

    NidlFaceKirby(obj);
    if (r < ARRAY_COUNT(sSpeeds)) {
        // 0x080881A0: turning around half the time;
        if (Rand16() & 1) obj->base.flags ^= 1;
        obj->base.xspeed = (obj->base.flags & 1) ? -sSpeeds[r] : sSpeeds[r];
        obj->base.yspeed = PERCH_HOP_SPEED;
        tw->state = STATE_PERCH_HOP;
    } else {
        tw->state = STATE_PERCH_WAIT;
        tw->timer = r == 5 ? 32 : 64;
    }
}

static void TwizzyPerch(struct Twizzy *tw) {
    struct Object *obj = &tw->n.obj;
    s32 dx = (obj->kirby3->base.x - obj->base.x) >> 8, dy = (obj->kirby3->base.y - obj->base.y) >> 8;

    switch (tw->state) {
    case STATE_PERCH:
        NidlFall(obj);
        // 0x080880FC: Kirby near: takes off (0x0808827C);
        if (dx > -PERCH_NEAR_X && dx < PERCH_NEAR_X && dy > -PERCH_NEAR_Y && dy < PERCH_NEAR_Y) {
            tw->state = STATE_PERCH_TAKEOFF;
            obj->base.xspeed = 0;
            obj->base.yspeed = PERCH_TAKEOFF;
            NidlFaceKirby(obj);
            NidlPlay(&tw->n, sFlapFast);
        } else if (!--tw->timer) {
            PerchChoose(tw);
        }
        break;
    case STATE_PERCH_HOP:
        obj->base.yspeed -= PERCH_GRAVITY;
        if (obj->base.yspeed < -HOP_MAX_FALL) obj->base.yspeed = -HOP_MAX_FALL;
        if ((obj->base.unk62 & 4) && obj->base.yspeed <= 0) {
            obj->base.yspeed = 0;
            Perch(tw);
        }
        break;
    case STATE_PERCH_WAIT:
        NidlFall(obj);
        if (!--tw->timer) Perch(tw);
        break;
    case STATE_PERCH_TAKEOFF:
        obj->base.yspeed -= 0x15;
        if (obj->base.yspeed <= 0) {
            // 0x080882D6: variant 4 from here;
            tw->n.variant = 4;
            tw->state = STATE_CHASE;
            tw->chaseTimer = FOLLOW_TIME;
            tw->timer = 0;
            tw->aimY = 2;
        }
        break;
    }
}

// The bob of variant 8 (0x080883DC): Y speeds and frames, 8 frames each;
static const s16 sBob[][2] = {
    { 0x100, FRAME_WINGS_DOWN }, { 0x80, FRAME_WINGS_UP }, { -0x80, FRAME_WINGS_UP }, { -0x100, FRAME_WINGS_DOWN },
    { -0x80, FRAME_WINGS_UP }, { 0, FRAME_WINGS_DOWN }, { 0x80, FRAME_WINGS_UP },
};

// Bob(); 0x080883DC, 0x08088478;
static void Bob(struct Twizzy *tw) {
    struct Object *obj = &tw->n.obj;

    if (tw->timer && --tw->timer) return;
    tw->timer = 8;
    NidlFaceKirby(obj);
    obj->base.yspeed = sBob[tw->step][0];
    NidlSetFrame(&tw->n, sBob[tw->step][1]);
    if (++tw->step >= ARRAY_COUNT(sBob)) tw->step = 0;
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
    case STATE_DROP:
        if ((obj->base.flags & 0x100) && !NidlIsFloor(obj, obj->base.x, obj->base.y)) obj->base.flags &= ~0x100;
        obj->base.yspeed -= AMBUSH_GRAVITY;
        if (obj->base.yspeed < -AMBUSH_MAX_FALL) obj->base.yspeed = -AMBUSH_MAX_FALL;
        if (((obj->kirby3->base.y - obj->base.y) >> 8) <= AMBUSH_LEVEL) {
            obj->base.flags &= ~0x100;
            tw->state = STATE_LOOP;
            tw->step = 0;
            tw->timer = 0;
            NidlPlay(&tw->n, sFlapSlow);
            Loop(tw);
        }
        break;
    case STATE_LOOP:
        Loop(tw);
        break;
    case STATE_BOUNCE:
        Bounce(tw);
        break;
    case STATE_CHASE:
        Chase(tw);
        break;
    case STATE_HOP_SIT:
        HopSit(tw);
        break;
    case STATE_HOP:
        Hop(tw);
        break;
    case STATE_PERCH:
    case STATE_PERCH_HOP:
    case STATE_PERCH_WAIT:
    case STATE_PERCH_TAKEOFF:
        TwizzyPerch(tw);
        break;
    case STATE_BOB:
        Bob(tw);
        break;
    case STATE_STILL:
        NidlFall(obj);
        break;
    case STATE_LEAVE:
        if ((obj->base.yspeed += LEAVE_ACCEL) > MAX_CLIMB) obj->base.yspeed = MAX_CLIMB;
        break;
    }
}

// TwizzyStart(); Behavior start, also called by the engine after Twizzy recovers from a hit: once flying, it dashes at Kirby;
void TwizzyStart(struct Object *obj) {
    struct Twizzy *tw = (struct Twizzy *)obj;

    ObjectSetFunc(obj, 0, TwizzyMain);
    obj->base.yspeed = 0;

    if (tw->n.variant == 2 && !tw->placed) {
        tw->placed = TRUE;
        Ambush(tw);
    } else if (tw->n.variant == 3) {
        StartBounce(tw);
    } else if (tw->n.variant == 6) {
        StartHopSit(tw);
    } else if (tw->n.variant == 7) {
        Perch(tw);
    } else if (tw->n.variant == 9) {
        tw->state = STATE_STILL;
        obj->base.xspeed = 0;
        NidlFaceKirby(obj);
        tw->n.sequence = NULL;
        NidlSetFrame(&tw->n, FRAME_SIT);
    } else if (tw->n.variant == 8) {
        tw->state = STATE_BOB;
        tw->step = 0;
        tw->timer = 0;
        obj->base.xspeed = 0;
        tw->n.sequence = NULL;
    } else if (tw->n.variant == 4) {
        if (tw->state != STATE_LEAVE) {
            tw->state = STATE_CHASE;
            tw->chaseTimer = FOLLOW_TIME;
            tw->timer = 0;
            tw->aimY = 2;
            obj->base.xspeed = 0;
        } else {
            obj->base.xspeed = (obj->base.flags & 1) ? -LEAVE_SPEED : LEAVE_SPEED;
            NidlPlay(&tw->n, sFlapFast);
        }
    } else if (tw->n.variant != 5) {
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
    tw->placed = FALSE;
    if (n->variant == 5 || n->variant == 6 || n->variant == 7 || n->variant == 9) {
        tw->state = STATE_SIT;
        obj->base.unk4C = obj->base.y = ((obj->base.y + (obj->base.unk3F * 0x100)) & 0xfffff000) - (obj->base.unk3F * 0x100) - 1;
    }
}
