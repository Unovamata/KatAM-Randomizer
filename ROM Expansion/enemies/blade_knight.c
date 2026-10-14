/* Blade Knight; ported from Kirby: Nightmare in Dream Land (USA), object type 0x25 (spawn 0x0807AF98, the same scripts as Sword
 * Knight, type 0x24), as NiDL enemy NIDL_BLADE_KNIGHT. Gives Sword when inhaled.
 * NiDL scripts of variant 0 (0x0807B300; 16.16 speeds; KatAM's are 8.8); states 0x087409FC, per frame 0x08740A1C:
 * • 0x0807B3F8: faces Kirby and walks at 0.5 (frames 0x0874099C), stopping at ledges (0x0807B088), until Kirby is within 63 pixels
 *   (0x0807B430). Then (0x0807B294) one time in 4 it raises its sword (frame 15, 16 frames) and slashes down (0x0807B4F0), else it
 *   lowers it (frame 16) and slashes up (0x0807B5D8).
 * • The slashes lunge forward (0x0807B0B4: 4.0, 3.0, 2.0, 1.0, then stopped, 3 frames each; table 0x087409E4) and hurt in front
 *   of it on their second frame (0x0807B1E4). Down: frames 14, 12, 11, 10, 16; up: 10, 11, 12, 14, 15 (2, 2, 1, 2 and 8 frames).
 *   After a slash (0x0807B16C, 0x0807B1A8) it slashes the other way once, one time in 4, else
 * • 0x0807B66C: backs off at 0.5, facing Kirby, for 90 frames, and walks again.
 * The slash's hitbox is KatAM's Sword Knight's (0x080B351C), with its sound. NiDL's swing at things thrown at it (0x0807B6E8),
 * its variants 1 and 2 and its water state aren't ported; */
#include "../nidl.h"
#include "constants/songs.h"

// Frames: NiDL's 4 to 16 (walking 4-9, slashing 10-16), numbered from 0. Its hurt frame is NiDL's 3;
#define FRAME(nidl) ((nidl) - 4)

#define WALK_SPEED 0x80
#define NEAR 63
#define BACK_OFF_TIME 90
#define LUNGE_STEP 3

static const s16 sWalk[] = {
    FRAME(5), 6, FRAME(9), 8, FRAME(7), 5, FRAME(8), 11, FRAME(7), 6, FRAME(9), 8, FRAME(5), 5, FRAME(4), 11, NIDL_LOOP,
};
static const s16 sSlashDown[] = { FRAME(14), 2, FRAME(12), 2, FRAME(11), 1, FRAME(10), 2, FRAME(16), 8, NIDL_END };
static const s16 sSlashUp[] = { FRAME(10), 2, FRAME(11), 2, FRAME(12), 1, FRAME(14), 2, FRAME(15), 8, NIDL_END };
static const s16 sLunge[] = { 0x400, 0x300, 0x200, 0x100, 0 };

#define SLASH_TIME (2 + 2 + 1 + 2 + 8)

enum { STATE_WALK, STATE_RAISE, STATE_LOWER, STATE_SLASH_DOWN, STATE_SLASH_UP, STATE_BACK_OFF };

struct BladeKnight {
    struct NidlObject n;
    u8 state;
    u8 timer;
    u8 combo;
    u8 lunge;
    u8 lungeTimer;
    u8 blocked;
};

static void SetState(struct BladeKnight *bk, u8 state);

/* SlashMain(); KatAM's Sword Knight's slash (0x080B3694): a hitbox 0x1A pixels in front of the knight, for 3 frames; */
static void SlashMain(void) {
    struct ObjectBase *slash = TaskGetStructPtr(gCurTask);
    struct Object *obj = slash->parent;

    if (slash->flags & 0x1000) {
        TaskDestroy(gCurTask);
    } else if (obj->base.flags & 0x1000) {
        slash->flags |= 0x1000;
    } else {
        slash->unk56 = obj->base.unk56;
        slash->x = obj->base.x;
        slash->y = obj->base.y;
        if (!Macro_0810B1F4(&obj->base)) {
            if (++slash->counter > 3) {
                slash->flags |= 0x1000;
            } else {
                if (obj->base.flags & 1) {
                    slash->x -= 0x1A00;
                    slash->flags |= 1;
                } else {
                    slash->x += 0x1A00;
                    slash->flags &= ~1;
                }
                if (!(slash->flags & 0x200)) SetPointerSomething(slash);
            }
        }
    }
}

// Slash(); KatAM's Sword Knight's (0x080B351C);
static void Slash(struct Object *obj) {
    struct Task *t = TaskCreate(SlashMain, sizeof(struct ObjectBase), 0x3500, TASK_USE_EWRAM, NULL);
    struct ObjectBase *slash = TaskGetStructPtr(t);
    u32 flags = 0x50500400;

    ClearObjectBase(slash);
    slash->header.kind = 2;
    slash->x = obj->base.x;
    slash->y = obj->base.y;
    slash->parent = obj;
    slash->counter = 0;
    slash->roomId = obj->base.roomId;
    slash->unk56 = obj->base.unk56;
    slash->unk63 = 1;
    slash->unk68 = 0x20000043;
    slash->unk5C |= 0x80000;
    if (obj->base.flags & 1) {
        slash->x -= 0x1A00;
        flags |= 1;
    } else {
        slash->x += 0x1A00;
    }
    if (Macro_0810B1F4(slash)) flags |= 0x2000;
    slash->flags = flags;
    sub_0803E2B0(slash, -0xF, -0xA, 0xA, 0xA);
    PlaySfx(slash, SE_SWORD_KNIGHT_SLASH_ATTACK);
}

// Lunge(); 0x0807B0B4: forward, slower every 3 frames, unless it stopped at a ledge;
static void Lunge(struct BladeKnight *bk) {
    struct Object *obj = &bk->n.obj;
    s16 speed;

    if (bk->blocked || --bk->lungeTimer) return;
    bk->lungeTimer = LUNGE_STEP;
    if (bk->lunge >= ARRAY_COUNT(sLunge)) return;
    speed = sLunge[bk->lunge++];
    obj->base.xspeed = (obj->base.flags & 1) ? -speed : speed;
}

// AfterSlash(); 0x0807B16C, 0x0807B1A8: the other slash once, one time in 4, else backs off;
static void AfterSlash(struct BladeKnight *bk, u8 other) {
    if (bk->combo || Rand16() % 4) {
        SetState(bk, STATE_BACK_OFF);
    } else {
        bk->combo++;
        SetState(bk, other);
    }
}

static void SetState(struct BladeKnight *bk, u8 state) {
    struct Object *obj = &bk->n.obj;

    bk->state = state;
    switch (state) {
    case STATE_WALK:
    case STATE_BACK_OFF:
        NidlFaceKirby(obj);
        obj->base.xspeed = (obj->base.flags & 1) ? -WALK_SPEED : WALK_SPEED;
        if (state == STATE_BACK_OFF) {
            obj->base.xspeed = -obj->base.xspeed;
            bk->timer = BACK_OFF_TIME;
        }
        bk->blocked = FALSE;
        NidlPlay(&bk->n, sWalk);
        break;
    case STATE_RAISE:
    case STATE_LOWER:
        obj->base.xspeed = 0;
        bk->timer = 16;
        bk->n.sequence = NULL;
        NidlSetFrame(&bk->n, state == STATE_RAISE ? FRAME(15) : FRAME(16));
        break;
    case STATE_SLASH_DOWN:
    case STATE_SLASH_UP:
        obj->base.xspeed = 0;
        bk->lunge = 0;
        bk->lungeTimer = 1;
        bk->timer = SLASH_TIME;
        NidlPlay(&bk->n, state == STATE_SLASH_DOWN ? sSlashDown : sSlashUp);
        break;
    }
}

static void BladeKnightMain(struct Object *obj) {
    struct BladeKnight *bk = (struct BladeKnight *)obj;

    NidlUpdate(&bk->n);
    NidlFall(obj);
    obj->base.flags |= 4;
    NidlAnimate(&bk->n);

    // 0x0807B088: stops at ledges (and lunges no more);
    if (obj->base.xspeed) {
        NidlStopAtLedge(obj);
        if (!obj->base.xspeed) bk->blocked = TRUE;
    }

    switch (bk->state) {
    case STATE_WALK:
        if (abs((obj->kirby3->base.x - obj->base.x) >> 8) <= NEAR) {
            NidlFaceKirby(obj);
            bk->combo = 0;
            SetState(bk, Rand16() % 4 == 0 ? STATE_RAISE : STATE_LOWER);
        }
        break;
    case STATE_RAISE:
    case STATE_LOWER:
        if (!--bk->timer) SetState(bk, bk->state == STATE_RAISE ? STATE_SLASH_DOWN : STATE_SLASH_UP);
        break;
    case STATE_SLASH_DOWN:
    case STATE_SLASH_UP:
        Lunge(bk);
        if (--bk->timer == SLASH_TIME - 2) Slash(obj);
        if (!bk->timer) AfterSlash(bk, bk->state == STATE_SLASH_DOWN ? STATE_SLASH_UP : STATE_SLASH_DOWN);
        break;
    case STATE_BACK_OFF:
        if (!--bk->timer) SetState(bk, STATE_WALK);
        break;
    }
}

// BladeKnightStart(); Behavior start, also called by the engine after Blade Knight recovers from a hit;
void BladeKnightStart(struct Object *obj) {
    struct BladeKnight *bk = (struct BladeKnight *)obj;

    ObjectSetFunc(obj, 0, BladeKnightMain);
    obj->base.yspeed = 0;
    SetState(bk, STATE_WALK);
}

// BladeKnightInit(); Its box (KatAM's Sword Knight's), on the floor;
void BladeKnightInit(struct NidlObject *n) {
    struct Object *obj = &n->obj;

    sub_0803E2B0(&obj->base, -5, -5, 5, 6);
    ObjectSetBounds(&obj->base, -6, -6, 6, 9);
    obj->base.unk4C = obj->base.y = ((obj->base.y + (obj->base.unk3F * 0x100)) & 0xfffff000) - (obj->base.unk3F * 0x100) - 1;
}
