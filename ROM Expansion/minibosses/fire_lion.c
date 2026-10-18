/* Fire Lion; ported from Kirby: Nightmare in Dream Land (USA), mini-boss object type 0x37 (spawn 0x080957BC), as NiDL mini-boss
 * NIDL_FIRE_LION. Its frames (0-51 of 0x08753718) are streamed frames (see KatAMExpansion.cs); its fire bits (52-56) are drawn from
 * its descriptor's tiles. Timings, heights and speeds traced (16.16 speeds, Y down; KatAM's are 8.8, Y up):
 * • It idles (frames 4-7, 8 each) for 90 frames, then picks one of 5 moves at random, and after each attack hops back (23, 24 and
 *   25 for 8, 8, 16; 62 pixels back, 14 up):
 * • Roaring, it hops in place 3 times (13, 14, 15: 38 pixels up).
 * • Rolling into a fireball (17, 44, 45), it rolls at Kirby at 3.0 for 48 frames (46, 49, 50, 51, 3 each), then slows down (47, 48).
 * • Pouncing, it crouches (17-20) and leaps at Kirby (36, 37, 38: 48 pixels up), landing in a burst of flames (40-43, twice).
 * • Breathing fire, it walks up (8-11 at 1.5), leaps forward (13-15) and breathes fire 3 times (26-29).
 * • Blazing, it walks up and its mane flares (34, then 30-33, 6 times).
 * The flames it breathes and bursts into are attack objects drawing its fire bits; */
#include "../nidl.h"
#include "code_0806F780.h"
#include "task.h"

// Frames: NiDL's 0-56. Its hurt frame is NiDL's 0, drawn 12 pixels lower (HurtShift);
// Its frames are drawn this much lower than NiDL's (HurtShift in KatAMExpansion.cs: they fit the hurt frame's tiles), so its boxes too;
#define DRAWN_LOWER 12
#define FRAME_IDLE 4
#define FRAME_FIRE 53          // 53-56;

#define GRAVITY 0x24
#define MAX_FALL 0x300
#define DASH_SPEED 0x300
#define WALK_SPEED 0x180
#define FLAME_SPEED 0x280
#define FLAME_TIME 20
#define FLAME_TILES 4

enum { STATE_IDLE, STATE_ROAR, STATE_DASH, STATE_POUNCE, STATE_BREATH, STATE_BLAZE, STATE_HOP_BACK };

/* One step of a move: frame, frames, then what changes when it starts: horizontal speed (forward; NO_SPEED keeps it), a leap
 * (height, gravity; 0: none) and what it does (STEP_*); */
struct Step {
    u8 frame;
    u8 time;
    s16 speed;
    s16 height;
    s16 gravity;
    u8 action;
};
#define NO_SPEED 0x7FFF
enum { STEP_NONE, STEP_LAND, STEP_FLAME, STEP_BURST, STEP_END };

static const struct Step sIdle[] = {
    { 4, 8, 0, 0, 0, 0 }, { 5, 8, NO_SPEED, 0, 0, 0 }, { 6, 8, NO_SPEED, 0, 0, 0 }, { 7, 8, NO_SPEED, 0, 0, 0 },
    { 4, 8, NO_SPEED, 0, 0, 0 }, { 5, 8, NO_SPEED, 0, 0, 0 }, { 6, 8, NO_SPEED, 0, 0, 0 }, { 7, 8, NO_SPEED, 0, 0, 0 },
    { 4, 8, NO_SPEED, 0, 0, 0 }, { 5, 8, NO_SPEED, 0, 0, 0 }, { 6, 8, NO_SPEED, 0, 0, 0 }, { 7, 2, NO_SPEED, 0, 0, STEP_END },
};
static const struct Step sRoar[] = {
    { 12, 6, 0, 0, 0, 0 }, { 16, 8, NO_SPEED, 0, 0, 0 },
    { 13, 16, NO_SPEED, 0x450, 0x40, 0 }, { 14, 4, NO_SPEED, 0, 0, 0 }, { 15, 1, NO_SPEED, 0, 0, STEP_LAND },
    { 13, 16, NO_SPEED, 0x450, 0x40, 0 }, { 14, 4, NO_SPEED, 0, 0, 0 }, { 15, 1, NO_SPEED, 0, 0, STEP_LAND },
    { 13, 16, NO_SPEED, 0x450, 0x40, 0 }, { 14, 4, NO_SPEED, 0, 0, 0 }, { 15, 1, NO_SPEED, 0, 0, STEP_LAND },
    { 17, 8, NO_SPEED, 0, 0, 0 }, { 18, 8, NO_SPEED, 0, 0, 0 }, { 6, 8, NO_SPEED, 0, 0, 0 }, { 7, 8, NO_SPEED, 0, 0, STEP_END },
};
static const struct Step sDash[] = {
    { 17, 1, 0, 0, 0, 0 }, { 44, 2, NO_SPEED, 0, 0, 0 }, { 45, 14, NO_SPEED, 0, 0, 0 },
    { 46, 3, DASH_SPEED, 0, 0, 0 }, { 49, 3, NO_SPEED, 0, 0, 0 }, { 50, 3, NO_SPEED, 0, 0, 0 }, { 51, 3, NO_SPEED, 0, 0, 0 },
    { 46, 3, NO_SPEED, 0, 0, 0 }, { 49, 3, NO_SPEED, 0, 0, 0 }, { 50, 3, NO_SPEED, 0, 0, 0 }, { 51, 3, NO_SPEED, 0, 0, 0 },
    { 46, 3, NO_SPEED, 0, 0, 0 }, { 49, 3, NO_SPEED, 0, 0, 0 }, { 50, 3, NO_SPEED, 0, 0, 0 }, { 51, 3, NO_SPEED, 0, 0, 0 },
    { 46, 3, NO_SPEED, 0, 0, 0 }, { 49, 3, NO_SPEED, 0, 0, 0 }, { 50, 3, NO_SPEED, 0, 0, 0 }, { 51, 3, NO_SPEED, 0, 0, 0 },
    { 47, 8, 0x240, 0, 0, 0 }, { 48, 8, 0x160, 0, 0, 0 }, { 14, 4, 0, 0, 0, STEP_END },
};
static const struct Step sPounce[] = {
    { 17, 8, 0, 0, 0, 0 }, { 18, 8, NO_SPEED, 0, 0, 0 }, { 19, 8, NO_SPEED, 0, 0, 0 }, { 20, 8, NO_SPEED, 0, 0, 0 },
    { 36, 28, 0x1E8, 0x3B0, 0x24, 0 }, { 37, 8, 0, 0, 0, 0 }, { 38, 1, NO_SPEED, 0, 0, STEP_LAND },
    { 40, 8, NO_SPEED, 0, 0, STEP_BURST }, { 41, 8, NO_SPEED, 0, 0, 0 }, { 42, 8, NO_SPEED, 0, 0, 0 }, { 43, 8, NO_SPEED, 0, 0, 0 },
    { 40, 8, NO_SPEED, 0, 0, 0 }, { 41, 8, NO_SPEED, 0, 0, 0 }, { 42, 8, NO_SPEED, 0, 0, 0 }, { 43, 8, NO_SPEED, 0, 0, 0 },
    { 40, 2, NO_SPEED, 0, 0, 0 }, { 38, 4, NO_SPEED, 0, 0, 0 }, { 36, 8, NO_SPEED, 0, 0, 0 }, { 17, 8, NO_SPEED, 0, 0, STEP_END },
};
static const struct Step sBreath[] = {
    { 4, 5, 0, 0, 0, 0 }, { 5, 5, NO_SPEED, 0, 0, 0 }, { 6, 5, NO_SPEED, 0, 0, 0 }, { 7, 5, NO_SPEED, 0, 0, 0 },
    { 8, 5, WALK_SPEED, 0, 0, 0 }, { 9, 5, NO_SPEED, 0, 0, 0 }, { 10, 5, NO_SPEED, 0, 0, 0 }, { 11, 5, NO_SPEED, 0, 0, 0 },
    { 8, 9, 0, 0, 0, 0 }, { 17, 8, NO_SPEED, 0, 0, 0 }, { 18, 8, NO_SPEED, 0, 0, 0 }, { 19, 8, NO_SPEED, 0, 0, 0 },
    { 13, 20, 0x150, 0x230, 0x1B, 0 }, { 14, 4, NO_SPEED, 0, 0, 0 }, { 15, 1, NO_SPEED, 0, 0, STEP_LAND }, { 4, 8, 0, 0, 0, 0 },
    { 26, 4, NO_SPEED, 0, 0, STEP_FLAME }, { 27, 1, NO_SPEED, 0, 0, 0 }, { 28, 1, NO_SPEED, 0, 0, 0 }, { 29, 4, NO_SPEED, 0, 0, 0 },
    { 26, 4, NO_SPEED, 0, 0, STEP_FLAME }, { 27, 1, NO_SPEED, 0, 0, 0 }, { 28, 1, NO_SPEED, 0, 0, 0 }, { 29, 4, NO_SPEED, 0, 0, 0 },
    { 26, 4, NO_SPEED, 0, 0, STEP_FLAME }, { 27, 1, NO_SPEED, 0, 0, 0 }, { 28, 1, NO_SPEED, 0, 0, 0 }, { 29, 4, NO_SPEED, 0, 0, STEP_END },
};
static const struct Step sBlaze[] = {
    { 8, 5, WALK_SPEED, 0, 0, 0 }, { 9, 5, NO_SPEED, 0, 0, 0 }, { 10, 5, NO_SPEED, 0, 0, 0 }, { 11, 5, NO_SPEED, 0, 0, 0 },
    { 8, 14, 0, 0, 0, 0 }, { 9, 5, WALK_SPEED, 0, 0, 0 }, { 10, 5, NO_SPEED, 0, 0, 0 }, { 11, 5, NO_SPEED, 0, 0, 0 },
    { 8, 14, 0, 0, 0, 0 }, { 34, 17, NO_SPEED, 0, 0, 0 },
    { 30, 4, NO_SPEED, 0, 0, STEP_BURST }, { 31, 4, NO_SPEED, 0, 0, 0 }, { 32, 4, NO_SPEED, 0, 0, 0 }, { 33, 4, NO_SPEED, 0, 0, 0 },
    { 30, 4, NO_SPEED, 0, 0, 0 }, { 31, 4, NO_SPEED, 0, 0, 0 }, { 32, 4, NO_SPEED, 0, 0, 0 }, { 33, 4, NO_SPEED, 0, 0, 0 },
    { 30, 4, NO_SPEED, 0, 0, STEP_BURST }, { 31, 4, NO_SPEED, 0, 0, 0 }, { 32, 4, NO_SPEED, 0, 0, 0 }, { 33, 4, NO_SPEED, 0, 0, 0 },
    { 30, 4, NO_SPEED, 0, 0, 0 }, { 31, 4, NO_SPEED, 0, 0, 0 }, { 32, 4, NO_SPEED, 0, 0, 0 }, { 33, 4, NO_SPEED, 0, 0, 0 },
    { 30, 4, NO_SPEED, 0, 0, STEP_BURST }, { 31, 4, NO_SPEED, 0, 0, 0 }, { 32, 4, NO_SPEED, 0, 0, 0 }, { 33, 4, NO_SPEED, 0, 0, 0 },
    { 30, 4, NO_SPEED, 0, 0, 0 }, { 31, 4, NO_SPEED, 0, 0, 0 }, { 32, 4, NO_SPEED, 0, 0, 0 }, { 33, 4, NO_SPEED, 0, 0, STEP_END },
};
static const struct Step sHopBack[] = {
    { 23, 8, -0x1F0, 0x1C0, 0x1C, 0 }, { 24, 8, NO_SPEED, 0, 0, 0 }, { 25, 1, NO_SPEED, 0, 0, STEP_LAND }, { 12, 1, 0, 0, 0, STEP_END },
};

static const struct Step *const sMoves[] = { sIdle, sRoar, sDash, sPounce, sBreath, sBlaze, sHopBack };

struct FireLion {
    struct NidlObject n;
    u8 state;
    u8 step;
    u8 timer;
    bool8 airborne;
    bool8 waiting;      // A STEP_LAND step waits for it to land;
    s16 gravity;
    u8 firePalette;     // Its descriptor's palette (its fire bits);
};

typedef char sFireLionFits[sizeof(struct FireLion) <= NIDL_OBJECT_SIZE ? 1 : -1];

static void Forward(struct Object *obj, s16 speed) {
    obj->base.xspeed = (obj->base.flags & 1) ? -speed : speed;
}

static bool8 Landed(struct FireLion *f) {
    struct Object *obj = &f->n.obj;

    if (!(obj->base.unk62 & 4)) f->airborne = TRUE;
    return f->airborne && (obj->base.unk62 & 4) && obj->base.yspeed <= 0;
}

/* FlameMain(); A flame (its fire bits, 53-56): an attack object flying straight for FLAME_TIME frames; */
static void FlameMain(void) {
    struct Sprite sprite;
    struct ObjectBase *flame = TaskGetStructPtr(gCurTask);

    if (ObjectPreUpdate(flame)) return;
    flame->flags |= 4;
    if (++flame->counter > FLAME_TIME) {
        flame->flags |= 0x1000;
        return;
    }
    if (!(flame->flags & 0x200)) SetPointerSomething(flame);
    flame->x += flame->xspeed;
    flame->y -= flame->yspeed;
    Macro_08107BA8_4(flame, &flame->sprite, &sprite, FLAME_TILES, &flame->sprite);
    flame->sprite.variant = FRAME_FIRE + ((flame->counter >> 2) & 3);
    flame->sprite.palId = flame->filler6[0];
    sub_0806F8BC(flame);
}

static void Flame(struct FireLion *f, s32 dx, s32 dy, s16 xspeed, s16 yspeed) {
    struct Object *obj = &f->n.obj;
    struct Task *t = TaskCreate(FlameMain, sizeof(struct ObjectBase), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct ObjectBase *flame = TaskGetStructPtr(t);
    bool8 left = obj->base.flags & 1;

    ClearObjectBase(flame);
    flame->header.kind = 2;
    flame->x = obj->base.x + (left ? -dx : dx) * 0x100;
    flame->y = obj->base.y + dy * 0x100;
    flame->parent = obj;
    flame->counter = 0;
    flame->roomId = obj->base.roomId;
    flame->unk56 = obj->base.unk56;
    if (Macro_0810B1F4(flame)) flame->flags |= 0x2000;
    flame->unk63 = 1;
    flame->flags |= 0x10000140 | (left ^ (xspeed < 0));
    flame->unk68 = 0x20000043;
    flame->unk5C |= 0x80000;
    flame->xspeed = left ? -xspeed : xspeed;
    flame->yspeed = yspeed;
    flame->filler6[0] = f->firePalette;
    sub_0803E2B0(flame, -5, -5 + DRAWN_LOWER, 5, 5 + DRAWN_LOWER);
    ObjectSetBounds(flame, 0, 0, 0, 0);
    ObjectBaseInitSprite(flame, &flame->sprite, FLAME_TILES, NidlGraphicsOf(&f->n)->animId, FRAME_FIRE, obj->base.sprite.unk14 >> 6);
    flame->sprite.palId = f->firePalette;
}

static void SetState(struct FireLion *f, u8 state);

static void StartStep(struct FireLion *f) {
    struct Object *obj = &f->n.obj;
    const struct Step *s = &sMoves[f->state][f->step];

    NidlSetFrame(&f->n, s->frame);
    f->timer = s->time;
    f->waiting = s->action == STEP_LAND;
    if (s->speed != NO_SPEED) Forward(obj, s->speed);
    if (s->height) {
        obj->base.yspeed = s->height;
        f->gravity = s->gravity;
        f->airborne = FALSE;
    }
    switch (s->action) {
    case STEP_FLAME:
        PlaySfx(&obj->base, SE_PRANK_THROW_ITEM);
        Flame(f, 28, -24, FLAME_SPEED, 0);
        Flame(f, 28, -24, FLAME_SPEED, 0x80);
        break;
    case STEP_BURST:
        PlaySfx(&obj->base, SE_BOSS_GROUND_POUND_ATTACK);
        Flame(f, 12, -8, 0x200, 0x100);
        Flame(f, -12, -8, -0x200, 0x100);
        break;
    }
}

static void SetState(struct FireLion *f, u8 state) {
    f->state = state;
    f->step = 0;
    if (state != STATE_HOP_BACK) NidlFaceKirby(&f->n.obj);
    StartStep(f);
}

static void FireLionMain(struct Object *obj) {
    struct FireLion *f = (struct FireLion *)obj;
    const struct Step *s;

    NidlUpdate(&f->n);
    obj->base.flags |= 4;
    obj->base.yspeed -= f->gravity;
    if (obj->base.yspeed < -MAX_FALL) obj->base.yspeed = -MAX_FALL;
    // Rolling into a wall ends the roll;
    if (f->state == STATE_DASH && (obj->base.unk62 & 3) && f->step >= 3 && f->step < 19) {
        f->step = 19;
        StartStep(f);
    }

    if (f->waiting) {
        if (!Landed(f)) return;
        f->waiting = FALSE;
        f->gravity = GRAVITY;
        f->timer = 1;
    }
    if (--f->timer) return;
    s = &sMoves[f->state][f->step];
    if (s->action == STEP_END) {
        obj->base.xspeed = 0;
        if (f->state == STATE_IDLE) SetState(f, STATE_ROAR + (Rand16() & 0xFF) * 5 / 256);
        else if (f->state == STATE_HOP_BACK || f->state == STATE_ROAR) SetState(f, STATE_IDLE);
        else SetState(f, STATE_HOP_BACK);
        return;
    }
    f->step++;
    StartStep(f);
}

// FireLionStart(); The fight starts, and again after a hit;
void FireLionStart(struct Object *obj) {
    struct FireLion *f = (struct FireLion *)obj;

    ObjectSetFunc(obj, 0, FireLionMain);
    f->firePalette = NidlLoadPalette(&f->n, NidlGraphicsOf(&f->n)->animId | NIDL_SECOND_PALETTE, FRAME_FIRE);
    f->gravity = GRAVITY;
    obj->base.yspeed = 0;
    SetState(f, STATE_IDLE);
}

// FireLionInit(); Its boxes (DRAWN_LOWER pixels lower than NiDL's, as its frames are), on the floor;
void FireLionInit(struct NidlObject *n) {
    struct FireLion *f = (struct FireLion *)n;
    struct Object *obj = &n->obj;

    f->gravity = GRAVITY;
    f->waiting = FALSE;
    NidlSetFrame(n, FRAME_IDLE);
    sub_0803E2B0(&obj->base, -24, -40 + DRAWN_LOWER, 24, 12 + DRAWN_LOWER);
    ObjectSetBounds(&obj->base, -24, -40 + DRAWN_LOWER, 24, 13 + DRAWN_LOWER);
    NidlOnFloor(obj);
}
