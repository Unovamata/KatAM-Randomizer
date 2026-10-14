/* Needlous; ported from Kirby: Nightmare in Dream Land (USA), object type 0x2C (spawn 0x0807BD20), as NiDL enemy NIDL_NEEDLOUS.
 * NiDL scripts of variant 0 (0x0807C080; 16.16 speeds, Y down; KatAM's are 8.8, Y up); states 0x08740AC8, per frame 0x08740AE0:
 * • Start (0x0807BD7C): faces Kirby, then walks one time in 4 (rand(4) == 0), curls up the others.
 * • Walk (0x0807BF0C, 0x0807C138): faces Kirby and walks at 0.5 (frames 0x08740A98), for 240 frames at most (0x08740A80), or until
 *   Kirby has been level with it (9 pixels) for 60 (0x08740A88); after rolling, it walks 150 frames first. Walls turn it around.
 * • Curl (0x0807BF74): hops 3.0 (gravity 0x2500, falling at 3.0 at most) on frames 7 and 8, then spins (0x08740AAC); landing it
 *   bounces 2.0 (0x0807BFD0), then 1.0 (0x0807C000), then rolls (0x0807C030, 0x0807C2E0) at 3.0 (0x08740A90) for 60 frames,
 *   stopping 16 frames at walls and rolling back; then it walks again.
 * • Variant 1 (0x0807C350, 0x0807C3F4): walks in place.
 * NiDL's water state (0x0807C1D0) isn't ported; */
#include "../nidl.h"

// Frames: NiDL's 4 to 16 (walking 4-6, hopping 7-8, spinning 11-16), numbered from 0. Its hurt frame is NiDL's 3;
#define FRAME(nidl) ((nidl) - 4)

#define WALK_SPEED 0x80
#define ROLL_SPEED 0x300
#define GRAVITY 0x25
#define MAX_FALL 0x300
#define WALK_TIME 240
#define LEVEL_TIME 60
#define LEVEL_RANGE 9
#define REST_TIME 150
#define ROLL_TIME 60
#define WALL_TIME 16

static const s16 sWalk[] = { FRAME(4), 4, FRAME(5), 4, FRAME(6), 6, FRAME(5), 2, NIDL_LOOP };
static const s16 sHop[] = { FRAME(7), 4, FRAME(8), 4, NIDL_END };
static const s16 sSpin[] = { FRAME(14), 3, FRAME(15), 2, FRAME(11), 3, FRAME(13), 3, FRAME(16), 2, FRAME(12), 3, NIDL_LOOP };
static const s16 sBounces[] = { 0x300, 0x200, 0x100 };

enum { STATE_WALK, STATE_CURL, STATE_ROLL, STATE_IN_PLACE };

struct Needlous {
    struct NidlObject n;
    u8 state;
    u8 bounce;
    u8 levelTimer;
    u8 wallTimer;
    u8 rest;
    u8 timer;
};

static void Walk(struct Needlous *nd) {
    struct Object *obj = &nd->n.obj;

    nd->state = STATE_WALK;
    nd->timer = WALK_TIME;
    nd->levelTimer = LEVEL_TIME;
    NidlFaceKirby(obj);
    obj->base.xspeed = (obj->base.flags & 1) ? -WALK_SPEED : WALK_SPEED;
    NidlPlay(&nd->n, sWalk);
}

static void Curl(struct Needlous *nd) {
    struct Object *obj = &nd->n.obj;

    nd->state = STATE_CURL;
    nd->bounce = 0;
    NidlFaceKirby(obj);
    obj->base.xspeed = 0;
    obj->base.yspeed = sBounces[0];
    NidlPlay(&nd->n, sHop);
}

// Blocked(); Moving into a wall (it barely moved);
static bool8 Blocked(struct Object *obj) {
    return (obj->base.unk62 & 3) && obj->base.xspeed && abs(obj->base.x - obj->base.unk48) < abs(obj->base.xspeed) / 2;
}

static void Roll(struct Needlous *nd) {
    struct Object *obj = &nd->n.obj;

    obj->base.xspeed = (obj->base.flags & 1) ? -ROLL_SPEED : ROLL_SPEED;
}

static void NeedlousMain(struct Object *obj) {
    struct Needlous *nd = (struct Needlous *)obj;
    s32 dy;

    NidlUpdate(&nd->n);
    obj->base.flags |= 4;
    NidlAnimate(&nd->n);

    switch (nd->state) {
    case STATE_WALK:
        NidlFall(obj);
        if (Blocked(obj)) {
            obj->base.flags ^= 1;
            obj->base.xspeed = -obj->base.xspeed;
        }
        if (nd->rest) {
            nd->rest--;
            break;
        }
        dy = (obj->kirby3->base.y - obj->base.y) >> 8;
        if (dy < -LEVEL_RANGE || dy > LEVEL_RANGE)
            nd->levelTimer = LEVEL_TIME;
        else if (!--nd->levelTimer)
            Curl(nd);
        if (nd->state == STATE_WALK && !--nd->timer) Curl(nd);
        break;
    case STATE_CURL:
        if (!nd->n.sequence) NidlPlay(&nd->n, sSpin);
        obj->base.yspeed -= GRAVITY;
        if (obj->base.yspeed < -MAX_FALL) obj->base.yspeed = -MAX_FALL;
        if ((obj->base.unk62 & 4) && obj->base.yspeed <= 0) {
            if (++nd->bounce < ARRAY_COUNT(sBounces)) {
                obj->base.yspeed = sBounces[nd->bounce];
            } else {
                obj->base.yspeed = 0;
                nd->state = STATE_ROLL;
                nd->timer = ROLL_TIME;
                nd->wallTimer = 0;
                Roll(nd);
            }
        }
        break;
    case STATE_ROLL:
        NidlFall(obj);
        if (nd->wallTimer) {
            // Stopped by a wall: rolls back afterwards;
            if (!--nd->wallTimer) {
                obj->base.flags ^= 1;
                Roll(nd);
            }
        } else if (Blocked(obj)) {
            obj->base.xspeed = 0;
            nd->wallTimer = WALL_TIME;
        } else if (!--nd->timer) {
            nd->rest = REST_TIME;
            Walk(nd);
        }
        break;
    case STATE_IN_PLACE:
        NidlFall(obj);
        break;
    }
}

// NeedlousStart(); Behavior start, also called by the engine after Needlous recovers from a hit;
void NeedlousStart(struct Object *obj) {
    struct Needlous *nd = (struct Needlous *)obj;

    ObjectSetFunc(obj, 0, NeedlousMain);
    obj->base.yspeed = 0;
    nd->rest = 0;
    if (nd->n.variant == 1) {
        nd->state = STATE_IN_PLACE;
        obj->base.xspeed = 0;
        NidlFaceKirby(obj);
        NidlPlay(&nd->n, sWalk);
    } else if (Rand16() % 4 == 0) {
        Walk(nd);
    } else {
        Curl(nd);
    }
}

// NeedlousInit(); Its box, on the floor;
void NeedlousInit(struct NidlObject *n) {
    struct Object *obj = &n->obj;

    sub_0803E2B0(&obj->base, -6, -5, 6, 7);
    ObjectSetBounds(&obj->base, -7, -6, 7, 9);
    obj->base.unk4C = obj->base.y = ((obj->base.y + (obj->base.unk3F * 0x100)) & 0xfffff000) - (obj->base.unk3F * 0x100) - 1;
}
