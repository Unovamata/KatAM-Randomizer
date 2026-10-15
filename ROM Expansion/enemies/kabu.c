/* Kabu; ported from Kirby: Nightmare in Dream Land (USA), object type 0x15 (spawn 0x0807EFFC), as NiDL enemy NIDL_KABU.
 * NiDL scripts of variant 0 (16.16 speeds, Y down; KatAM's are 8.8, Y up):
 * • 0x0807F0C4: sits (frame 12) for 15 frames, spins in place 1 to 3 times (frames 4 to 11, 5 frames each), sits for 5 more,
 *   then picks a jump (weights 2, 2, 2, 1, 1 out of 8, table 0x0874131C), never the same one twice in a row.
 * • 0x0807F218: faces Kirby and jumps; the 5 jumps (tables 0x08741328, 0x0874133C, 0x08741350): in place 3.0, forward 1.0 3.0,
 *   backwards 1.0 3.0, in place 4.5, forward 1.0 4.5 (gravity 1/8, falling at 6.0 at most), spinning every 4, 2, 8, 8 or 4
 *   frames. Landing (0x0807F2B4) it sits again.
 * • Variant 1, 0x0807F380 (states 0x087413A8, per frame 0x087413B4): sits rocking (frames 4-10, 8 frames each, 0x0807F42C) until
 *   Kirby is within 63 pixels (0x0807F488); then (0x0807F4DC) spins for 48 frames fading out (a mask per 8 frames, 0x08741357),
 *   is gone for 60, and comes back 32-48 pixels to a side of Kirby and up to 64 above, where there's room (tables 0x08741367,
 *   0x0874136D; up to 60 tries, 0x0807F6A8), spinning 48 frames fading in; it falls (0x0807F78C: gravity 0x2000, at 6.0 at most,
 *   spinning every 4 frames), sits 10 frames (0x08741365, frame 12), then slides at Kirby at 3.0 spinning until it stops, and
 *   vanishes again; */
#include "../nidl.h"

// Frames: NiDL's 4 to 12 (spinning 4-11, sitting 12), numbered from 0. Its hurt frame is NiDL's 3;
#define FRAME(nidl) ((nidl) - 4)
#define FRAME_SIT FRAME(12)

static const u8 sWeights[] = { 2, 2, 2, 1, 1 };                     // 0x0874131C;
static const s16 sJumpX[] = { 0, 0x100, -0x100, 0, 0x100 };         // 0x08741328;
static const s16 sJumpY[] = { 0x300, 0x300, 0x300, 0x480, 0x480 };  // 0x0874133C;
static const u8 sSpinTime[] = { 4, 2, 8, 8, 4 };                    // 0x08741350;

#define GRAVITY 0x20
#define MAX_FALL 0x600
#define SPIN_TIME 5

// Variant 1;
#define ROCK_TIME 8
#define WARP_RANGE 63
#define FADE_TIME 48
#define GONE_TIME 60
#define WARP_TRIES 60
#define WARP_SIT_TIME 10
#define SLIDE_SPEED 0x300
#define SLIDE_FRICTION 0x10
static const u8 sFade[] = { 0x7F, 0x77, 0x57, 0x55, 0x15, 0x11, 0x01, 0x01, 0x11, 0x15, 0x55, 0x57, 0x77, 0x7F };  // 0x08741357;
static const s8 sWarpX[] = { 32, 40, 48, -32, -40, -48 };                    // 0x08741367;
static const s8 sWarpY[] = { 0, -8, -16, -24, -32, -40, -48, -56, -64 };   // 0x0874136D;

enum { STATE_SIT, STATE_SPIN, STATE_WAIT, STATE_JUMP, STATE_ROCK, STATE_FADE_OUT, STATE_GONE, STATE_FADE_IN, STATE_DROP, STATE_LAND, STATE_SLIDE };

struct Kabu {
    struct NidlObject n;
    s8 lastJump;
    u8 state;
    u8 timer;
    u8 spins;
    u8 spinFrame;
    u8 spinTime;
    u8 airborne;
    u8 fade;
};

static void Sit(struct Kabu *k, u8 time) {
    k->state = STATE_SIT;
    k->timer = time;
    k->n.obj.base.xspeed = 0;
    NidlSetFrame(&k->n, FRAME_SIT);
}

// Spin(); The next spinning frame, every spinTime frames; true after frame 11;
static bool8 Spin(struct Kabu *k) {
    if (--k->timer) return FALSE;
    k->timer = k->spinTime;
    if (++k->spinFrame > FRAME(11)) k->spinFrame = FRAME(4);
    NidlSetFrame(&k->n, k->spinFrame);
    return k->spinFrame == FRAME(4);
}

static void Jump(struct Kabu *k) {
    struct Object *obj = &k->n.obj;
    u8 jump, total;
    u8 r;

    do {
        r = Rand16() % 8;
        for (jump = 0, total = sWeights[0]; total <= r && jump < 4; total += sWeights[++jump]);
    } while (jump == k->lastJump);
    k->lastJump = jump;

    NidlFaceKirby(obj);
    obj->base.xspeed = (obj->base.flags & 1) ? -sJumpX[jump] : sJumpX[jump];
    obj->base.yspeed = sJumpY[jump];
    k->state = STATE_JUMP;
    k->airborne = FALSE;
    k->spinTime = sSpinTime[jump];
    k->timer = k->spinTime;
    k->spinFrame = FRAME(4);
    NidlSetFrame(&k->n, k->spinFrame);
}

// FadeSpin(); The spin while vanishing or coming back: every 4 frames, shown or hidden by a mask of 8 frames (0x0807F634);
static void FadeSpin(struct Kabu *k, u8 step) {
    struct Object *obj = &k->n.obj;

    if (!(k->timer % 4) && ++k->spinFrame > FRAME(11)) k->spinFrame = FRAME(4);
    NidlSetFrame(&k->n, k->spinFrame);
    if (sFade[step + (k->timer >> 3)] & (1 << (k->timer & 7)))
        obj->base.flags &= ~0x400;
    else
        obj->base.flags |= 0x400;
}

// Warp(); 0x0807F538: somewhere free next to Kirby (it turns around when it found none);
static void Warp(struct Kabu *k) {
    struct Object *obj = &k->n.obj;
    s32 x, y;
    u8 tries;

    for (tries = 0; tries < WARP_TRIES; tries++) {
        x = obj->kirby3->base.x + sWarpX[Rand16() % ARRAY_COUNT(sWarpX)] * 0x100;
        y = obj->kirby3->base.y + sWarpY[Rand16() % ARRAY_COUNT(sWarpY)] * 0x100;
        if (!NidlIsFloor(obj, x - 0x600, y - 0x700) && !NidlIsFloor(obj, x + 0x600, y - 0x700)
            && !NidlIsFloor(obj, x - 0x600, y + 0x600) && !NidlIsFloor(obj, x + 0x600, y + 0x600)) break;
    }
    if (tries == WARP_TRIES) obj->base.flags ^= 1;
    else obj->base.x = x, obj->base.y = y;
}

// KabuWarp(); Variant 1;
static void KabuWarp(struct Kabu *k) {
    struct Object *obj = &k->n.obj;
    s32 dx = (obj->kirby3->base.x - obj->base.x) >> 8;

    switch (k->state) {
    case STATE_ROCK:
        NidlFall(obj);
        if (dx >= -WARP_RANGE && dx <= WARP_RANGE) {
            k->state = STATE_FADE_OUT;
            k->timer = FADE_TIME;
            k->spinFrame = FRAME(4);
            break;
        }
        if (--k->timer) break;
        k->timer = ROCK_TIME;
        if (++k->spinFrame > FRAME(10)) k->spinFrame = FRAME(4);
        NidlSetFrame(&k->n, k->spinFrame);
        break;
    case STATE_FADE_OUT:
        NidlFall(obj);
        FadeSpin(k, 0);
        if (--k->timer) break;
        obj->base.flags |= 0x400 | 0x100;   // Gone: not drawn, not touching anything;
        obj->base.flags |= 0x200;
        k->state = STATE_GONE;
        k->timer = GONE_TIME;
        break;
    case STATE_GONE:
        obj->base.xspeed = obj->base.yspeed = 0;
        if (--k->timer) break;
        Warp(k);
        NidlFaceKirby(obj);
        obj->base.flags &= ~(0x100 | 0x200);
        k->state = STATE_FADE_IN;
        k->timer = FADE_TIME;
        break;
    case STATE_FADE_IN:
        FadeSpin(k, 7 - 1);
        if (--k->timer) break;
        obj->base.flags &= ~0x400;
        k->state = STATE_DROP;
        k->timer = 4;
        break;
    case STATE_DROP:
        obj->base.yspeed -= GRAVITY;
        if (obj->base.yspeed < -MAX_FALL) obj->base.yspeed = -MAX_FALL;
        Spin(k);
        if ((obj->base.unk62 & 4) && obj->base.yspeed <= 0) {
            obj->base.yspeed = 0;
            k->state = STATE_LAND;
            k->timer = WARP_SIT_TIME;
            NidlSetFrame(&k->n, FRAME_SIT);
        }
        break;
    case STATE_LAND:
        NidlFall(obj);
        if (--k->timer) break;
        k->state = STATE_SLIDE;
        NidlFaceKirby(obj);
        obj->base.xspeed = (obj->base.flags & 1) ? -SLIDE_SPEED : SLIDE_SPEED;
        k->spinTime = 1;
        k->timer = 1;
        break;
    case STATE_SLIDE:
        NidlFall(obj);
        Spin(k);
        if (obj->base.xspeed > 0) obj->base.xspeed = obj->base.xspeed > SLIDE_FRICTION ? obj->base.xspeed - SLIDE_FRICTION : 0;
        if (obj->base.xspeed < 0) obj->base.xspeed = obj->base.xspeed < -SLIDE_FRICTION ? obj->base.xspeed + SLIDE_FRICTION : 0;
        if (!obj->base.xspeed || (obj->base.unk62 & 3)) {
            obj->base.xspeed = 0;
            k->state = STATE_FADE_OUT;
            k->timer = FADE_TIME;
        }
        break;
    }
}

static void KabuMain(struct Object *obj) {
    struct Kabu *k = (struct Kabu *)obj;

    NidlUpdate(&k->n);
    obj->base.flags |= 4;

    if (k->n.variant == 1) {
        KabuWarp(k);
        return;
    }
    if (k->state != STATE_JUMP) NidlFall(obj);

    switch (k->state) {
    case STATE_SIT:
        if (--k->timer) break;
        k->state = STATE_SPIN;
        k->spins = Rand16() % 3 + 1;
        k->spinTime = SPIN_TIME;
        k->timer = SPIN_TIME;
        k->spinFrame = FRAME(4);
        NidlSetFrame(&k->n, k->spinFrame);
        break;
    case STATE_SPIN:
        if (Spin(k) && !--k->spins) {
            Sit(k, SPIN_TIME);
            k->state = STATE_WAIT;
        }
        break;
    case STATE_WAIT:
        if (!--k->timer) Jump(k);
        break;
    case STATE_JUMP:
        Spin(k);
        obj->base.yspeed -= GRAVITY;
        if (obj->base.yspeed < -MAX_FALL) obj->base.yspeed = -MAX_FALL;
        if (!(obj->base.unk62 & 4)) k->airborne = TRUE;
        if (k->airborne && (obj->base.unk62 & 4) && obj->base.yspeed <= 0) {
            obj->base.yspeed = 0;
            Sit(k, 15);
        }
        break;
    }
}

// KabuStart(); Behavior start, also called by the engine after Kabu recovers from a hit;
void KabuStart(struct Object *obj) {
    struct Kabu *k = (struct Kabu *)obj;

    ObjectSetFunc(obj, 0, KabuMain);
    obj->base.yspeed = 0;
    obj->base.flags &= ~(0x400 | 0x100 | 0x200);
    if (k->n.variant == 1) {
        k->state = STATE_ROCK;
        k->timer = ROCK_TIME;
        k->spinFrame = FRAME(4);
        obj->base.xspeed = 0;
        NidlSetFrame(&k->n, k->spinFrame);
        return;
    }
    Sit(k, 1);
}

// KabuInit(); Its box, on the floor;
void KabuInit(struct NidlObject *n) {
    struct Kabu *k = (struct Kabu *)n;
    struct Object *obj = &n->obj;

    sub_0803E2B0(&obj->base, -6, -7, 6, 7);
    ObjectSetBounds(&obj->base, -7, -8, 7, 9);
    obj->base.unk4C = obj->base.y = ((obj->base.y + (obj->base.unk3F * 0x100)) & 0xfffff000) - (obj->base.unk3F * 0x100) - 1;
    k->lastJump = -1;
}
