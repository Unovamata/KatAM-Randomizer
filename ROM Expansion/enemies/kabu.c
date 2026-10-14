/* Kabu; ported from Kirby: Nightmare in Dream Land (USA), object type 0x15 (spawn 0x0807EFFC), as NiDL enemy NIDL_KABU.
 * NiDL scripts of variant 0 (16.16 speeds, Y down; KatAM's are 8.8, Y up):
 * • 0x0807F0C4: sits (frame 12) for 15 frames, spins in place 1 to 3 times (frames 4 to 11, 5 frames each), sits for 5 more,
 *   then picks a jump (weights 2, 2, 2, 1, 1 out of 8, table 0x0874131C), never the same one twice in a row.
 * • 0x0807F218: faces Kirby and jumps; the 5 jumps (tables 0x08741328, 0x0874133C, 0x08741350): in place 3.0, forward 1.0 3.0,
 *   backwards 1.0 3.0, in place 4.5, forward 1.0 4.5 (gravity 1/8, falling at 6.0 at most), spinning every 4, 2, 8, 8 or 4
 *   frames. Landing (0x0807F2B4) it sits again.
 * NiDL's variants 1 to 3 aren't ported; */
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

enum { STATE_SIT, STATE_SPIN, STATE_WAIT, STATE_JUMP };

struct Kabu {
    struct NidlObject n;
    s8 lastJump;
    u8 state;
    u8 timer;
    u8 spins;
    u8 spinFrame;
    u8 spinTime;
    u8 airborne;
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

static void KabuMain(struct Object *obj) {
    struct Kabu *k = (struct Kabu *)obj;

    NidlUpdate(&k->n);
    obj->base.flags |= 4;

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
