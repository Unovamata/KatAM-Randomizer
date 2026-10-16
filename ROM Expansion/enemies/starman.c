/* Starman; ported from Kirby: Nightmare in Dream Land (USA), object type 0x1F (spawn 0x08080DD0), as NiDL enemy NIDL_STARMAN.
 * NiDL scripts of variant 0 (0x08080E10; 16.16 speeds, Y down; KatAM's are 8.8, Y up):
 * • 0x08080EA4: faces Kirby and walks (1.0, table 0x087414F8; frames 0x08081814) until Kirby is within 47 pixels (0x08080EDC).
 * • 0x08080F34: crouches (frame 11 for 16 frames, 12 for 6), then takes off towards Kirby: 2.0 forward, 2.0 up, steering 0x1900
 *   a frame towards Kirby (at most 2.0, 0x08080FA4), flying (frames 16-18, every 6 frames; 13 when slow) until it's 9 pixels
 *   above the screen.
 * • 0x08081084: hides there 78 frames (0x08741500), following Kirby sideways but for the last 30 (0x080810C4).
 * • 0x08081140: dives (frame 20) at 2.0, gravity 0x1500, at 3.0 at most; landing it bounces up 2.0 (frame 13, then 14-15 every
 *   4 frames) and lands again. The first time it stands (frame 13) for 60 frames and walks again; the second time:
 * • 0x08081274: crouches and flies away: 0.4 forward, rising 0x1900 faster every frame (frame 13).
 * • Variant 1, 0x08081408 (states 0x08741584, per frame 0x08741590): walks at 0.5 facing Kirby (0x0808148C); after a while (216
 *   frames the first time, then 27 to 216 at random, table 0x0874150C) it crouches (frame 11 for 16 frames, 12 for 6) and jumps
 *   2.0 (0x08081508: gravity 0x1000, at 3.0 at most, frame 13), falls (frames 14 and 15, 4 frames each, 0x0808159C) and walks again.
 * • Variant 2, 0x08081614 (0x0808168C, 0x080816E8): flies at 0.75 (0x0874151C), flapping (frames 16-18, 6 frames each), bobbing
 *   (Y speeds 0, 0.5, 1.0, 0.5, 0, -0.5, -1.0, -0.5 down, table 0x08741524, 6 frames each, 8 when it bobs towards Kirby's height
 *   and Kirby is more than 10 pixels up or down); after 6 bobs it rises away (0x1900 faster a frame).
 * • Variant 3, 0x08081774 (0x080817F8): faces Kirby and walks in place.
 * NiDL's water state (0x080812F0) isn't ported; */
#include "../nidl.h"

// Frames: NiDL's 4 to 20 (walking 4-10, crouching 11-12, flying 13-18, diving 20), numbered from 0. Its hurt frame is NiDL's 3;
#define FRAME(nidl) ((nidl) - 4)

#define WALK_SPEED 0x100
#define TAKEOFF_SPEED 0x200
#define STEER 0x19
#define NEAR 47
#define HIDE_TIME 78
#define AIM_TIME 30
#define DIVE_SPEED 0x200
#define GRAVITY 0x15
#define MAX_FALL 0x300
#define AWAY_SPEED 0x66

static const s16 sWalk[] = {
    FRAME(4), 4, FRAME(8), 6, FRAME(10), 10, FRAME(9), 6, FRAME(4), 4, FRAME(5), 6, FRAME(6), 10, FRAME(7), 6, NIDL_LOOP,
};
static const s16 sCrouch[] = { FRAME(11), 16, FRAME(12), 6, NIDL_END };
static const s16 sFly[] = { FRAME(16), 6, FRAME(17), 6, FRAME(18), 6, NIDL_LOOP };
static const s16 sTumble[] = { FRAME(14), 4, FRAME(15), 4, NIDL_LOOP };

enum { STATE_WALK, STATE_CROUCH, STATE_TAKEOFF, STATE_HIDE, STATE_DIVE, STATE_BOUNCE, STATE_REST, STATE_LEAVE_CROUCH, STATE_LEAVE,
       STATE_STROLL, STATE_HOP_CROUCH, STATE_HOP, STATE_BOB, STATE_RISE };

// Variant 2;
static const s16 sBob[] = { 0, -0x80, -0x100, -0x80, 0, 0x80, 0x100, 0x80 };   // 0x08741524 (Y up);
#define BOB_SPEED 0xC0
#define BOB_STEP 6
#define BOB_COUNT 6

// Variant 1: frames between hops (0x0874150C);
static const u8 sHopDelays[] = { 27, 54, 54, 81, 81, 81, 108, 108, 135, 135, 162, 162, 162, 189, 189, 216 };
#define HOP_SPEED 0x200
#define HOP_GRAVITY 0x10

struct Starman {
    struct NidlObject n;
    u8 state;
    u8 timer;
    u8 dives;
    u8 strollTime;
    u8 bobStep;
    u8 bobs;
};

// SetState(); Also what it looks like;
static void SetState(struct Starman *s, u8 state) {
    struct Object *obj = &s->n.obj;

    s->state = state;
    switch (state) {
    case STATE_STROLL:
    case STATE_WALK:
        NidlFaceKirby(obj);
        obj->base.xspeed = (obj->base.flags & 1) ? -WALK_SPEED : WALK_SPEED;
        NidlPlay(&s->n, sWalk);
        break;
    case STATE_CROUCH:
    case STATE_LEAVE_CROUCH:
        obj->base.xspeed = 0;
        s->timer = 16 + 6;
        NidlPlay(&s->n, sCrouch);
        break;
    case STATE_TAKEOFF:
        NidlFaceKirby(obj);
        obj->base.xspeed = (obj->base.flags & 1) ? -TAKEOFF_SPEED : TAKEOFF_SPEED;
        obj->base.yspeed = TAKEOFF_SPEED;
        obj->base.flags |= 0x2000000;   // Not despawned off screen;
        NidlPlay(&s->n, sFly);
        break;
    case STATE_HIDE:
        obj->base.xspeed = obj->base.yspeed = 0;
        obj->base.flags |= 0x400 | 0x100;   // Not drawn, through walls;
        s->timer = HIDE_TIME;
        s->n.sequence = NULL;
        break;
    case STATE_DIVE:
        obj->base.flags &= ~0x400;
        obj->base.yspeed = -DIVE_SPEED;
        NidlSetFrame(&s->n, FRAME(20));
        break;
    case STATE_BOUNCE:
        obj->base.yspeed = DIVE_SPEED;
        NidlSetFrame(&s->n, FRAME(13));
        break;
    case STATE_REST:
        obj->base.xspeed = obj->base.yspeed = 0;
        obj->base.flags &= ~0x2000000;
        s->timer = 60;
        NidlSetFrame(&s->n, FRAME(13));
        break;
    case STATE_LEAVE:
        // Flies away from where Kirby is (0x08063DF4), through walls, and goes once off screen;
        obj->base.flags = (obj->base.flags | 0x100) & ~0x2000000;
        if (obj->base.x > obj->kirby3->base.x)
            obj->base.flags &= ~1;
        else
            obj->base.flags |= 1;
        obj->base.xspeed = (obj->base.flags & 1) ? -AWAY_SPEED : AWAY_SPEED;
        obj->base.yspeed = 0;
        NidlSetFrame(&s->n, FRAME(13));
        break;
    }
}

// Takeoff(); Steers towards Kirby until it's above the screen (or under a ceiling);
static void Takeoff(struct Starman *s) {
    struct Object *obj = &s->n.obj;
    s32 top = gCurLevelInfo[obj->base.unk56].viewportPosition.y - 9 * 0x100;
    s16 speed;

    if (obj->base.y < top || (obj->base.unk62 & 8)) {
        SetState(s, STATE_HIDE);
        return;
    }
    speed = obj->base.xspeed + (obj->kirby3->base.x < obj->base.x ? -STEER : STEER);
    if (speed > TAKEOFF_SPEED) speed = TAKEOFF_SPEED;
    if (speed < -TAKEOFF_SPEED) speed = -TAKEOFF_SPEED;
    obj->base.xspeed = speed;
    if (speed > 0)
        obj->base.flags &= ~1;
    else if (speed < 0)
        obj->base.flags |= 1;
    if (abs(speed) < 0x60)
        NidlSetFrame(&s->n, FRAME(13));
    else
        NidlAnimate(&s->n);
    obj->base.yspeed = TAKEOFF_SPEED;
}

// Fall(); Gravity while diving and bouncing; true when it lands;
static bool8 Fall(struct Object *obj) {
    obj->base.yspeed -= GRAVITY;
    if (obj->base.yspeed < -MAX_FALL) obj->base.yspeed = -MAX_FALL;
    return (obj->base.unk62 & 4) && obj->base.yspeed <= 0;
}

// Stroll(); Variant 1 (0x0808148C, 0x080814B4, 0x08081508, 0x0808159C);
static void Stroll(struct Starman *s) {
    struct Object *obj = &s->n.obj;

    switch (s->state) {
    case STATE_STROLL:
        NidlFall(obj);
        NidlAnimate(&s->n);
        NidlStopAtLedge(obj);
        if (--s->strollTime) break;
        s->state = STATE_HOP_CROUCH;
        s->timer = 16 + 6;
        obj->base.xspeed = 0;
        NidlPlay(&s->n, sCrouch);
        break;
    case STATE_HOP_CROUCH:
        NidlFall(obj);
        NidlAnimate(&s->n);
        if (--s->timer) break;
        s->state = STATE_HOP;
        obj->base.yspeed = HOP_SPEED;
        s->n.sequence = NULL;
        NidlSetFrame(&s->n, FRAME(13));
        break;
    case STATE_HOP:
        obj->base.yspeed -= HOP_GRAVITY;
        if (obj->base.yspeed < -MAX_FALL) obj->base.yspeed = -MAX_FALL;
        if (obj->base.yspeed <= 0 && !s->n.sequence) NidlPlay(&s->n, sTumble);
        NidlAnimate(&s->n);
        if ((obj->base.unk62 & 4) && obj->base.yspeed <= 0) {
            obj->base.yspeed = 0;
            SetState(s, STATE_STROLL);
            s->strollTime = sHopDelays[Rand16() % ARRAY_COUNT(sHopDelays)];
        }
        break;
    }
}

// Bob(); Variant 2 (0x080816E8);
static void Bob(struct Starman *s) {
    struct Object *obj = &s->n.obj;
    s32 dy;
    s16 speed;

    NidlAnimate(&s->n);
    if (s->state == STATE_RISE) {
        obj->base.yspeed += STEER;
        if (obj->base.yspeed > MAX_FALL) obj->base.yspeed = MAX_FALL;
        return;
    }
    if (--s->timer) return;
    if (++s->bobStep == ARRAY_COUNT(sBob)) {
        s->bobStep = 0;
        if (!--s->bobs) {
            s->state = STATE_RISE;
            return;
        }
    }
    speed = sBob[s->bobStep];
    dy = (obj->kirby3->base.y - obj->base.y) >> 8;
    // Longer steps towards Kirby's height when it's far up or down (Kirby below: down, which is negative);
    s->timer = (dy > 10 && speed < 0) || (dy < -10 && speed > 0) ? 8 : BOB_STEP;
    obj->base.yspeed = speed;
}

static void StarmanMain(struct Object *obj) {
    struct Starman *s = (struct Starman *)obj;

    NidlUpdate(&s->n);
    obj->base.flags |= 4;

    if (s->n.variant == 1) {
        Stroll(s);
        return;
    }
    if (s->n.variant == 2) {
        Bob(s);
        return;
    }
    if (s->n.variant == 3) {
        NidlFall(obj);
        NidlAnimate(&s->n);
        return;
    }

    switch (s->state) {
    case STATE_WALK:
        NidlFall(obj);
        NidlAnimate(&s->n);
        if (abs((obj->kirby3->base.x - obj->base.x) >> 8) <= NEAR) SetState(s, STATE_CROUCH);
        break;
    case STATE_CROUCH:
    case STATE_LEAVE_CROUCH:
        NidlFall(obj);
        NidlAnimate(&s->n);
        if (!--s->timer) SetState(s, s->state == STATE_CROUCH ? STATE_TAKEOFF : STATE_LEAVE);
        break;
    case STATE_TAKEOFF:
        Takeoff(s);
        break;
    case STATE_HIDE:
        if (--s->timer > AIM_TIME) obj->base.x = obj->kirby3->base.x;
        obj->base.y = gCurLevelInfo[obj->base.unk56].viewportPosition.y - 9 * 0x100;
        if (!s->timer) SetState(s, STATE_DIVE);
        break;
    case STATE_DIVE:
        // Through walls until it's out of them (it may have hidden above a ceiling);
        if ((obj->base.flags & 0x100) && !NidlIsFloor(obj, obj->base.x, obj->base.y)) obj->base.flags &= ~0x100;
        if (Fall(obj)) SetState(s, STATE_BOUNCE);
        break;
    case STATE_BOUNCE:
        if (obj->base.yspeed <= 0 && !s->n.sequence) NidlPlay(&s->n, sTumble);
        NidlAnimate(&s->n);
        if (Fall(obj)) {
            s->n.sequence = NULL;
            if (s->dives++)
                SetState(s, STATE_LEAVE_CROUCH);
            else
                SetState(s, STATE_REST);
        }
        break;
    case STATE_REST:
        NidlFall(obj);
        if (!--s->timer) SetState(s, STATE_WALK);
        break;
    case STATE_LEAVE:
        obj->base.yspeed += STEER;
        if (obj->base.yspeed > MAX_FALL) obj->base.yspeed = MAX_FALL;
        break;
    }
}

// StarmanStart(); Behavior start, also called by the engine after Starman recovers from a hit (it walks again);
void StarmanStart(struct Object *obj) {
    struct Starman *s = (struct Starman *)obj;

    ObjectSetFunc(obj, 0, StarmanMain);
    obj->base.flags &= ~(0x400 | 0x100 | 0x2000000);
    obj->base.yspeed = 0;
    if (s->n.variant == 3) {
        NidlFaceKirby(obj);
        obj->base.xspeed = 0;
        NidlPlay(&s->n, sWalk);
        return;
    }
    if (s->n.variant == 2) {
        if (s->state != STATE_RISE) {
            s->state = STATE_BOB;
            s->bobStep = 0;
            s->bobs = BOB_COUNT;
            s->timer = BOB_STEP;
            NidlFaceKirby(obj);
        }
        obj->base.xspeed = (obj->base.flags & 1) ? -BOB_SPEED : BOB_SPEED;
        NidlPlay(&s->n, sFly);
        return;
    }
    if (s->n.variant == 1) {
        SetState(s, STATE_STROLL);
        if (!s->strollTime) s->strollTime = 216;
        return;
    }
    SetState(s, STATE_WALK);
}

// StarmanInit(); Its box, on the floor;
void StarmanInit(struct NidlObject *n) {
    struct Object *obj = &n->obj;

    ((struct Starman *)n)->dives = 0;
    ((struct Starman *)n)->strollTime = 0;
    sub_0803E2B0(&obj->base, -6, -6, 6, 8);
    ObjectSetBounds(&obj->base, -7, -7, 7, 9);
    ((struct Starman *)n)->state = 0;
    if (n->variant != 2)
        obj->base.unk4C = obj->base.y = ((obj->base.y + (obj->base.unk3F * 0x100)) & 0xfffff000) - (obj->base.unk3F * 0x100) - 1;
}
