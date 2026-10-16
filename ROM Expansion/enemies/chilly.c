/* Chilly; ported from Kirby: Nightmare in Dream Land (USA), object type 0x0E (spawn 0x08084CD4), as NiDL enemy NIDL_CHILLY.
 * NiDL scripts of variant 0 (0x08084D14; 16.16 speeds, Y down; KatAM's are 8.8, Y up); states 0x08741FC0, per frame 0x08741FD4:
 * • Start: state 0 two times in 4, else 2 or 3.
 * • 0x08084DC0 (0): faces Kirby and wiggles (frames 4, 5, 6, 7, 6, 5, 4 for 1, 2, 2, 8, 2, 2, 1), turning around after each
 *   one, 4 times; then turns around once more and:
 * • 0x08084E9C (1): turns around and wiggles again until it faces Kirby with Kirby within 32 pixels up or down (0x08063DF4 is
 *   the way to Kirby, 0x08063D2C dy to it); then:
 * • 0x08084F78 (2): lunges at Kirby 4 times: frame 4 for 4, 5 for 2, then 3.0 (frame 5, 2), 2.0 (6, 2), 1.0 (6, 2), 0.5 (6, 4,
 *   then 7 for 18), stops (0x08006214) and settles (7, 6, 5, 4 for 3, 6, 5, 4); walls turn it around (0x0808542C). Then:
 * • 0x08085180 (3): shakes (1.0 back, forward, back for 1, 2, 1 frames) crouched (frame 10) 15 times, then freezes: spawns the
 *   freeze object (0x08064B28; type 0x68, 0x0808CB70: an icy ring flickering on two frames for 240 frames, its sound every 5,
 *   gone early if Chilly is interrupted) and shakes 64 times more with its arms up (frame 11); then state 0.
 * • Variant 1, 0x080852CC: faces Kirby and lunges in place forever (4, 5, 6, 7, 6, 5, 4 for 4, 4, 8, 21, 6, 5, 4).
 * • Water defeats variant 0, not variant 1 (0x08085404).
 * Here the freeze is a ring of KatAM's Pengy's ice (0x080BB080, copied below with its own heading), bursting out of Chilly
 * every 5 frames for 240 frames; it hurts like Pengy's ice and makes its sound. Chilly gives Ice (NiDL's Freeze isn't in KatAM); */
#include "../nidl.h"
#include "code_0806F780.h"
#include "constants/songs.h"
#include "task.h"

// Frames: NiDL's 4 to 11 (wiggling and lunging 4-7, crouched 10, arms up 11), numbered from 0. Its hurt frame is NiDL's 3;
#define FRAME(nidl) ((nidl) - 4)

static const s16 sWiggle[] = {
    FRAME(4), 1, FRAME(5), 2, FRAME(6), 2, FRAME(7), 8, FRAME(6), 2, FRAME(5), 2, FRAME(4), 1, NIDL_END,
};
#define WIGGLE_TIME (1 + 2 + 2 + 8 + 2 + 2 + 1)
#define WIGGLES 4
#define FACING_RANGE 32

// The lunge, step by step: a frame, its speed (0: stopped) and how long;
static const struct { u8 frame; u8 time; s16 speed; } sLunge[] = {
    { FRAME(4), 4, 0 }, { FRAME(5), 2, 0 }, { FRAME(5), 2, 0x300 }, { FRAME(6), 2, 0x200 }, { FRAME(6), 2, 0x100 },
    { FRAME(6), 4, 0x80 }, { FRAME(7), 18, 0x80 }, { FRAME(7), 3, 0 }, { FRAME(6), 6, 0 }, { FRAME(5), 5, 0 }, { FRAME(4), 4, 0 },
};
#define LUNGES 4

static const s16 sLungeInPlace[] = {
    FRAME(4), 4, FRAME(5), 4, FRAME(6), 8, FRAME(7), 21, FRAME(6), 6, FRAME(5), 5, FRAME(4), 4, NIDL_LOOP,
};

#define SHAKE_SPEED 0x100
#define CROUCH_SHAKES 15
#define FREEZE_SHAKES 64
#define FREEZE_TIME 240
#define ICE_EVERY 5
#define ICE_SPEED 0x380

enum { STATE_WIGGLE, STATE_WATCH, STATE_LUNGE, STATE_CROUCH, STATE_FREEZE, STATE_IN_PLACE };

struct Chilly {
    struct NidlObject n;
    u8 state;
    u8 count;
    u8 step;
    u8 timer;
    u8 shake;
    u8 iceTimer;
    u8 iceAngle;
    u16 freezeTimer;
};

static void SetState(struct Chilly *c, u8 state);

/* IceMain(); KatAM's Pengy's ice (0x080BB290): flies for 10 frames; */
static void IceMain(void) {
    struct Sprite sprite;
    struct ObjectBase *ice = TaskGetStructPtr(gCurTask);

    if (ObjectPreUpdate(ice)) return;
    ice->flags |= 4;
    if (++ice->counter > 10) {
        ice->flags |= 0x1000;
        return;
    }
    if (!(ice->flags & 0x200)) SetPointerSomething(ice);
    if (!(ice->flags & 0x800)) {
        ice->x += ice->xspeed;
        ice->y -= ice->yspeed;
    }
    Macro_08107BA8_4(ice, &ice->sprite, &sprite, 0x12, &ice->sprite);
    sub_0806F8BC(ice);
}

/* Ice(); KatAM's Pengy's (0x080BB080), from Chilly's middle towards one of 8 headings (NiDL angles / 64: 0 right, 2 down); */
static void Ice(struct Object *obj, u8 heading) {
    static const s16 sCos[8] = { 256, 181, 0, -181, -256, -181, 0, 181 };
    struct Task *t = TaskCreate(IceMain, sizeof(struct ObjectBase), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct ObjectBase *ice = TaskGetStructPtr(t);

    ClearObjectBase(ice);
    ice->header.kind = 2;
    ice->x = obj->base.x;
    ice->y = obj->base.y;
    ice->parent = obj;
    ice->counter = 0;
    ice->roomId = obj->base.roomId;
    ice->unk56 = obj->base.unk56;
    if (Macro_0810B1F4(ice)) ice->flags |= 0x2000;
    ice->unk63 = 1;
    ice->flags |= 0x10004140;
    ice->unk68 = 0x20000203;
    ice->unk5C |= 0x80000;
    ice->xspeed = sCos[heading & 7] * ICE_SPEED / 256;
    ice->yspeed = -sCos[(heading + 6) & 7] * ICE_SPEED / 256;
    if (ice->xspeed < 0) ice->flags |= 1;
    sub_0803E2B0(ice, -2, -2, 2, 2);
    ObjectSetBounds(ice, 0, 0, 0, 0);
    ObjectBaseInitSprite(ice, &ice->sprite, 0x12, 0x334, 0, 0xc);
    PlaySfx(ice, SE_PENGY_ICE_ATTACK);
}

// LungeStep(); The current step of the lunge;
static void LungeStep(struct Chilly *c) {
    struct Object *obj = &c->n.obj;

    NidlSetFrame(&c->n, sLunge[c->step].frame);
    c->timer = sLunge[c->step].time;
    obj->base.xspeed = (obj->base.flags & 1) ? -sLunge[c->step].speed : sLunge[c->step].speed;
}

static void Wiggle(struct Chilly *c) {
    NidlPlay(&c->n, sWiggle);
    c->timer = WIGGLE_TIME;
}

// KirbyInFront(); 0x08084E9C: it faces Kirby, and Kirby is within 32 pixels up or down;
static bool8 KirbyInFront(struct Object *obj) {
    s32 dx = obj->kirby3->base.x - obj->base.x, dy = (obj->kirby3->base.y - obj->base.y) >> 8;

    return (dx < 0) == ((obj->base.flags & 1) != 0) && dy >= -FACING_RANGE && dy <= FACING_RANGE;
}

static void SetState(struct Chilly *c, u8 state) {
    struct Object *obj = &c->n.obj;

    c->state = state;
    c->count = 0;
    obj->base.xspeed = 0;
    switch (state) {
    case STATE_WIGGLE:
        NidlFaceKirby(obj);
        Wiggle(c);
        break;
    case STATE_WATCH:
        obj->base.flags ^= 1;
        Wiggle(c);
        break;
    case STATE_LUNGE:
        NidlFaceKirby(obj);
        c->step = 0;
        LungeStep(c);
        break;
    case STATE_CROUCH:
    case STATE_FREEZE:
        c->n.sequence = NULL;
        NidlSetFrame(&c->n, state == STATE_CROUCH ? FRAME(10) : FRAME(11));
        c->shake = 0;
        c->freezeTimer = FREEZE_TIME;
        c->iceTimer = 0;
        break;
    }
}

// Shake(); 0x08085180: 1.0 back, forward for 2 frames, back again; true after the 4 frames;
static bool8 Shake(struct Chilly *c) {
    struct Object *obj = &c->n.obj;
    s16 speed = c->shake == 1 || c->shake == 2 ? SHAKE_SPEED : -SHAKE_SPEED;

    obj->base.xspeed = (obj->base.flags & 1) ? -speed : speed;
    if (++c->shake < 4) return FALSE;
    c->shake = 0;
    return TRUE;
}

static void ChillyMain(struct Object *obj) {
    struct Chilly *c = (struct Chilly *)obj;

    NidlUpdate(&c->n);
    obj->base.flags |= 4;
    NidlFall(obj);
    NidlAnimate(&c->n);

    switch (c->state) {
    case STATE_WIGGLE:
        if (--c->timer) break;
        obj->base.flags ^= 1;
        if (++c->count < WIGGLES) {
            Wiggle(c);
        } else {
            obj->base.flags ^= 1;
            SetState(c, STATE_WATCH);
        }
        break;
    case STATE_WATCH:
        if (--c->timer) break;
        if (KirbyInFront(obj)) {
            SetState(c, STATE_LUNGE);
        } else {
            obj->base.flags ^= 1;
            Wiggle(c);
        }
        break;
    case STATE_LUNGE:
        // 0x0808542C: walls turn it around;
        if ((obj->base.unk62 & 3) && obj->base.xspeed) {
            obj->base.flags ^= 1;
            obj->base.xspeed = -obj->base.xspeed;
        }
        if (--c->timer) break;
        if (++c->step < ARRAY_COUNT(sLunge)) {
            LungeStep(c);
        } else if (++c->count < LUNGES) {
            NidlFaceKirby(obj);
            c->step = 0;
            LungeStep(c);
        } else {
            NidlFaceKirby(obj);
            SetState(c, STATE_CROUCH);
        }
        break;
    case STATE_CROUCH:
        if (Shake(c) && ++c->count == CROUCH_SHAKES) SetState(c, STATE_FREEZE);
        break;
    case STATE_FREEZE:
        if (c->freezeTimer) {
            c->freezeTimer--;
            if (!c->iceTimer--) {
                c->iceTimer = ICE_EVERY - 1;
                Ice(obj, c->iceAngle);
                Ice(obj, c->iceAngle + 4);
                c->iceAngle++;
            }
        }
        if (Shake(c) && ++c->count == FREEZE_SHAKES) SetState(c, STATE_WIGGLE);
        break;
    }
}

// ChillyStart(); Behavior start, also called by the engine after Chilly recovers from a hit (0x08084D14);
void ChillyStart(struct Object *obj) {
    struct Chilly *c = (struct Chilly *)obj;

    ObjectSetFunc(obj, 0, ChillyMain);
    obj->base.yspeed = 0;
    if (c->n.variant == 1) {
        c->state = STATE_IN_PLACE;
        obj->base.xspeed = 0;
        NidlFaceKirby(obj);
        NidlPlay(&c->n, sLungeInPlace);
        return;
    }
    switch (Rand16() % 4) {
    case 2:
        SetState(c, STATE_LUNGE);
        break;
    case 3:
        SetState(c, STATE_CROUCH);
        break;
    default:
        SetState(c, STATE_WIGGLE);
        break;
    }
}

// ChillyInit(); Its box, on the floor;
void ChillyInit(struct NidlObject *n) {
    struct Object *obj = &n->obj;

    sub_0803E2B0(&obj->base, -6, -5, 6, 7);
    ObjectSetBounds(&obj->base, -7, -6, 7, 9);
    // Water defeats variant 0 like KatAM's enemies, but not variant 1 (0x08085404);
    if (n->variant == 1) NIDL_SWIMS(obj);
    ((struct Chilly *)n)->iceAngle = 0;
    obj->base.unk4C = obj->base.y = ((obj->base.y + (obj->base.unk3F * 0x100)) & 0xfffff000) - (obj->base.unk3F * 0x100) - 1;
}
