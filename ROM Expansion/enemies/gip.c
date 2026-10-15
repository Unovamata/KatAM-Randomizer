/* Gip; ported from Kirby: Nightmare in Dream Land (USA), object type 0x2F (spawn 0x0808BDB4), as NiDL enemy NIDL_GIP.
 * NiDL scripts of variant 0 (0x0808BDF4; 16.16 speeds, Y down; KatAM's are 8.8, Y up): states 0x08742948, per frame (by group)
 * 0x08742978, handlers 0x08742DEC (landed: walk; left the floor: 7; walls: 6 from walking, 3 from 9 and 10; ceiling: 10 from 4):
 * • Start: on a wall within 12 pixels (0x0808C82C; probes 0x0808CA00 right, 0x0808CAB8 left) it clings to it (3), else walks.
 * • 0x0808BEA0 (0-2, walk): 1.0 forward (frames 6, 7, 4, 5 for 8, 6, 8, 6); every 16 frames it faces Kirby, or turns its back to it
 *   within 63 pixels (0x0808BF1C); after 120 frames it throws a star (11). Walking into a wall it climbs it (6); off a ledge, 7.
 * • 0x0808BFC4 (3, cling): frame 8 for 29, then picks (0x0808C8BC: weights 0x08742998 by Kirby's octant, 0x0808A84C) flapping up
 *   (4), gliding down (5), dropping off (9), jumping at Kirby (10) or clinging on: with Kirby mostly above or below on the right
 *   40, 40, 10, 10, 0 in 100, else 20, 40, 20, 10, 10.
 * • 0x0808C02C (4): flaps up 3 times (1.0 up: frames 8, 9, 10 for 3, 3, 4; 0.5: 11, 12 for 3; 0.25: 11, 10 for 3; stopped: 9 for
 *   6), then picks again; past the top of the wall it steps onto it (8: 1.0 forward, frame 4, 10 frames, then walks).
 * • 0x0808C260 (5): glides down 3 times (1.0 down: frames 16, 13 for 4, 6; 0.5: 16, 13 for 4, 2; 0.25: 13, 16 for 4, 2; stopped:
 *   16, 13 for 2, 4), then picks again; past the bottom of the wall it drops off (9).
 * • 0x0808C0FC (6): climbs fast, 3 times (2.0, 1.0, 0.5 up, every 2 frames, frames 8-12, 11, 10, 9), then clings.
 * • 0x0808C32C (7): turns around and slides down fast (2.0, then 3 times 2.0, 1.0, 0.5, 0.25 for 4 each, frames 16, 13), then
 *   clings; past the bottom of the wall it drops off.
 * • 0x0808C4E4 (9): turns around and drops, 0.5 forward (frame 4, gravity 0x1500), and walks once it lands.
 * • 0x0808C53C (10): facing Kirby (it can't jump into the wall) it clings on; else it shakes (1.0 back and forth for 2 frames,
 *   8 times; frame 8), turns around and jumps at Kirby (0x0808C980: 2.0 forward, up to 3.0 up, gravity 0x2000; frames 6, 7).
 * • 0x0808C610 (11): faces Kirby, frame 14 for 16, throws a star (sound 0xE9, not ported; 0x0808C934: type 0x89, 0x0808D218, 6 pixels ahead
 *   and up: 1.5 forward, 3.0 up, gravity 0x1500, frames 4-7 of 0x08752C18 for 4 each), frames 15, 14, 7 for 12, 8, 8; then walks.
 * • Variant 1 (0x0808C6AC): faces Kirby and walks in place.
 * Here the star is a KatAM attack object drawing the enemy's own frames, gone when it hits something or after 2 seconds; */
#include "../nidl.h"
#include "code_0806F780.h"
#include "task.h"

// Frames: NiDL's 4 to 16 (walking 4-7, clinging and flapping 8-12, gliding 13 and 16, throwing 14 and 15), then the star's 4-7
// (0x08752C18). Its hurt frame is NiDL's 3;
#define FRAME(nidl) ((nidl) - 4)
#define FRAME_STAR 13
#define STAR_FRAMES 4

static const s16 sWalk[] = { FRAME(6), 8, FRAME(7), 6, FRAME(4), 8, FRAME(5), 6, NIDL_LOOP };
static const s16 sJump[] = { FRAME(6), 4, FRAME(7), 4, NIDL_LOOP };

// The wall moves, step by step: a frame, how long, the Y speed it sets (up; NO_SPEED keeps it);
#define NO_SPEED 0x7FFF
struct Step { u8 frame; u8 time; s16 speed; };
static const struct Step sFlapUp[] = {
    { FRAME(8), 3, 0x100 }, { FRAME(9), 3, NO_SPEED }, { FRAME(10), 4, NO_SPEED }, { FRAME(11), 3, 0x80 }, { FRAME(12), 3, NO_SPEED },
    { FRAME(11), 3, 0x40 }, { FRAME(10), 3, NO_SPEED }, { FRAME(9), 6, 0 }, { 0, 0, 0 },
};
static const struct Step sGlideDown[] = {
    { FRAME(16), 4, -0x100 }, { FRAME(13), 6, NO_SPEED }, { FRAME(16), 4, -0x80 }, { FRAME(13), 2, NO_SPEED }, { FRAME(13), 4, -0x40 },
    { FRAME(16), 2, NO_SPEED }, { FRAME(16), 2, 0 }, { FRAME(13), 4, NO_SPEED }, { 0, 0, 0 },
};
static const struct Step sClimb[] = {
    { FRAME(8), 2, 0x200 }, { FRAME(9), 2, NO_SPEED }, { FRAME(10), 2, 0x100 }, { FRAME(11), 2, NO_SPEED }, { FRAME(12), 2, 0x80 },
    { FRAME(11), 2, NO_SPEED }, { FRAME(10), 2, 0x40 }, { FRAME(9), 2, NO_SPEED }, { 0, 0, 0 },
};
static const struct Step sSlide[] = {
    { FRAME(16), 4, -0x200 }, { FRAME(13), 4, -0x100 }, { FRAME(16), 4, -0x80 }, { FRAME(13), 4, -0x40 }, { 0, 0, 0 },
};
static const struct Step sThrow[] = { { FRAME(14), 16, 0 }, { FRAME(15), 12, 0 }, { FRAME(14), 8, 0 }, { FRAME(7), 8, 0 }, { 0, 0, 0 } };
#define MOVES 3

// The picks from a wall (0x08742998): flap up, glide down, drop off, jump at Kirby, cling on; Kirby mostly above or below on the
// right (octants 0, 7), or not;
static const u8 sPicks[2][5] = { { 40, 80, 90, 100, 100 }, { 20, 60, 80, 90, 100 } };

#define WALK_SPEED 0x100
#define WALK_TIME 120
#define FACE_EVERY 16
#define TOO_CLOSE 63
#define WALL_RANGE 12
#define FEET 9
#define CLING_TIME 29
#define TOP_TIME 10
#define DROP_SPEED 0x80
#define SHAKE_SPEED 0x100
#define SHAKES 8
#define JUMP_SPEED 0x200
#define JUMP_HEIGHT 0x300
#define JUMP_GRAVITY 0x20
#define STAR_SPEED 0x180
#define STAR_HEIGHT 0x300
#define STAR_TIME 120

enum {
    STATE_WALK, STATE_CLING, STATE_FLAP_UP, STATE_GLIDE_DOWN, STATE_CLIMB, STATE_SLIDE, STATE_TOP, STATE_DROP, STATE_SHAKE,
    STATE_JUMP, STATE_THROW, STATE_IN_PLACE,
};

struct Gip {
    struct NidlObject n;
    u8 state;
    u8 timer;
    u8 step;
    u8 count;
    u8 walkTimer;
    u8 faceTimer;
    const struct Step *steps;
};

static void SetState(struct Gip *g, u8 state);

/* StarMain(); Gip's star: an attack object like KatAM's Pengy's ice (0x080BB290), drawing the enemy's own frames; gone when it
 * hits something or after STAR_TIME frames; */
static void StarMain(void) {
    struct Sprite sprite;
    struct ObjectBase *star = TaskGetStructPtr(gCurTask);

    if (ObjectPreUpdate(star)) return;
    star->flags |= 4;
    if (++star->counter > STAR_TIME || (star->counter > 4 && NidlIsFloor((struct Object *)star, star->x, star->y))) {
        star->flags |= 0x1000;
        return;
    }
    if (!(star->flags & 0x200)) SetPointerSomething(star);
    star->yspeed -= 0x15;
    if (star->yspeed < -0x300) star->yspeed = -0x300;
    star->x += star->xspeed;
    star->y -= star->yspeed;
    Macro_08107BA8_4(star, &star->sprite, &sprite, 0x10, &star->sprite);
    star->sprite.variant = FRAME_STAR + (star->counter >> 2) % STAR_FRAMES;
    star->sprite.palId = star->filler6[0];
    sub_0806F8BC(star);
}

// Star(); 0x0808C934: 6 pixels ahead and up;
static void Star(struct Gip *g) {
    struct Object *obj = &g->n.obj;
    struct Task *t = TaskCreate(StarMain, sizeof(struct ObjectBase), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct ObjectBase *star = TaskGetStructPtr(t);

    ClearObjectBase(star);
    star->header.kind = 2;
    star->x = obj->base.x + ((obj->base.flags & 1) ? -0x600 : 0x600);
    star->y = obj->base.y - 0x600;
    star->parent = obj;
    star->counter = 0;
    star->roomId = obj->base.roomId;
    star->unk56 = obj->base.unk56;
    if (Macro_0810B1F4(star)) star->flags |= 0x2000;
    star->unk63 = 1;
    star->flags |= 0x10000140 | (obj->base.flags & 1);
    star->unk68 = 0x20000043;
    star->unk5C |= 0x80000;
    star->xspeed = (obj->base.flags & 1) ? -STAR_SPEED : STAR_SPEED;
    star->yspeed = STAR_HEIGHT;
    star->filler6[0] = obj->base.sprite.palId;    // Its palette, kept in the padding;
    sub_0803E2B0(star, -4, -4, 4, 4);
    ObjectSetBounds(star, 0, 0, 0, 0);
    ObjectBaseInitSprite(star, &star->sprite, 0x10, gNidlData.enemies[g->n.enemy].animId, FRAME_STAR, obj->base.sprite.unk14 >> 6);
    star->sprite.palId = obj->base.sprite.palId;
}

// WallAhead(); A wall within 12 pixels the way it faces, level with the bottom of its box (0x0808CA00, 0x0808CAB8, at its record's offset);
static bool8 WallAhead(struct Object *obj) {
    return NidlIsFloor(obj, obj->base.x + ((obj->base.flags & 1) ? -WALL_RANGE : WALL_RANGE) * 0x100, obj->base.y + FEET * 0x100);
}

// Blocked(); Walking into a wall (it barely moved; slopes don't stop it);
static bool8 Blocked(struct Object *obj) {
    return (obj->base.unk62 & 3) && obj->base.xspeed && abs(obj->base.x - obj->base.unk48) < abs(obj->base.xspeed) / 2;
}

static void PlayStep(struct Gip *g) {
    const struct Step *s = &g->steps[g->step];

    NidlSetFrame(&g->n, s->frame);
    g->timer = s->time;
    if (s->speed != NO_SPEED) g->n.obj.base.yspeed = s->speed;
}

static void StartSteps(struct Gip *g, const struct Step *steps) {
    g->steps = steps;
    g->step = 0;
    g->count = 0;
    g->n.sequence = NULL;
    PlayStep(g);
}

// Steps(); The next step when it's time; true once they're all done MOVES times (once for the throw);
static bool8 Steps(struct Gip *g, u8 times) {
    if (--g->timer) return FALSE;
    if (!g->steps[++g->step].time) {
        g->step = 0;
        if (++g->count == times) return TRUE;
    }
    PlayStep(g);
    return FALSE;
}

// Pick(); 0x0808C8BC: what to do on a wall;
static void Pick(struct Gip *g) {
    static const u8 sStates[] = { STATE_FLAP_UP, STATE_GLIDE_DOWN, STATE_DROP, STATE_SHAKE, STATE_CLING };
    u8 octant = NidlKirbyOctant(&g->n.obj), r = Rand16() % 101, i = 0;
    const u8 *picks = sPicks[octant == 0 || octant == 7 ? 0 : 1];

    while (i < 4 && picks[i] < r) i++;
    SetState(g, sStates[i]);
}

static void Walk(struct Gip *g) {
    struct Object *obj = &g->n.obj;

    obj->base.xspeed = (obj->base.flags & 1) ? -WALK_SPEED : WALK_SPEED;
}

static void SetState(struct Gip *g, u8 state) {
    struct Object *obj = &g->n.obj;

    g->state = state;
    switch (state) {
    case STATE_WALK:
        obj->base.yspeed = 0;
        g->walkTimer = WALK_TIME;
        g->faceTimer = FACE_EVERY;
        NidlFaceKirby(obj);
        Walk(g);
        NidlPlay(&g->n, sWalk);
        break;
    case STATE_CLING:
        obj->base.xspeed = obj->base.yspeed = 0;
        g->n.sequence = NULL;
        NidlSetFrame(&g->n, FRAME(8));
        g->timer = CLING_TIME;
        break;
    case STATE_FLAP_UP:
        obj->base.xspeed = 0;
        StartSteps(g, sFlapUp);
        break;
    case STATE_GLIDE_DOWN:
        obj->base.xspeed = 0;
        StartSteps(g, sGlideDown);
        break;
    case STATE_CLIMB:
        obj->base.xspeed = 0;
        StartSteps(g, sClimb);
        break;
    case STATE_SLIDE:
        obj->base.flags ^= 1;
        obj->base.xspeed = 0;
        StartSteps(g, sSlide);
        obj->base.yspeed = -0x200;
        break;
    case STATE_TOP:
        obj->base.yspeed = 0;
        Walk(g);
        g->n.sequence = NULL;
        NidlSetFrame(&g->n, FRAME(4));
        g->timer = TOP_TIME;
        break;
    case STATE_DROP:
        obj->base.flags ^= 1;
        obj->base.xspeed = (obj->base.flags & 1) ? -DROP_SPEED : DROP_SPEED;
        obj->base.yspeed = 0;
        g->n.sequence = NULL;
        NidlSetFrame(&g->n, FRAME(4));
        break;
    case STATE_SHAKE:
        obj->base.yspeed = 0;
        // 0x0808C53C: facing Kirby, it would jump into the wall;
        if (((obj->kirby3->base.x < obj->base.x) == ((obj->base.flags & 1) != 0))) {
            SetState(g, STATE_CLING);
            return;
        }
        g->n.sequence = NULL;
        NidlSetFrame(&g->n, FRAME(8));
        g->count = 0;
        g->timer = 2;
        obj->base.xspeed = (obj->base.flags & 1) ? SHAKE_SPEED : -SHAKE_SPEED;
        break;
    case STATE_JUMP:
        // 0x0808C980: towards Kirby, as high as it goes (the arc NiDL works out is nearly always capped at 3.0);
        obj->base.flags ^= 1;
        obj->base.xspeed = (obj->base.flags & 1) ? -JUMP_SPEED : JUMP_SPEED;
        obj->base.yspeed = JUMP_HEIGHT;
        NidlPlay(&g->n, sJump);
        break;
    case STATE_THROW:
        obj->base.xspeed = obj->base.yspeed = 0;
        NidlFaceKirby(obj);
        g->steps = sThrow;
        g->step = 0;
        g->count = 0;
        g->n.sequence = NULL;
        PlayStep(g);
        break;
    }
}

static void GipMain(struct Object *obj) {
    struct Gip *g = (struct Gip *)obj;
    s32 dx;

    NidlUpdate(&g->n);
    obj->base.flags |= 4;
    NidlAnimate(&g->n);

    switch (g->state) {
    case STATE_WALK:
        // Off a ledge it slides down its side (7), into a wall it climbs it (6);
        if (!(obj->base.unk62 & 4)) {
            SetState(g, STATE_SLIDE);
            break;
        }
        if (Blocked(obj)) {
            obj->base.xspeed = 0;
            SetState(g, STATE_CLIMB);
            break;
        }
        if (!--g->walkTimer) {
            SetState(g, STATE_THROW);
            break;
        }
        if (--g->faceTimer) break;
        g->faceTimer = FACE_EVERY;
        NidlFaceKirby(obj);
        dx = (obj->kirby3->base.x - obj->base.x) >> 8;
        if (dx >= -TOO_CLOSE && dx <= TOO_CLOSE) obj->base.flags ^= 1;
        Walk(g);
        break;
    case STATE_CLING:
        if (!--g->timer) Pick(g);
        break;
    case STATE_FLAP_UP:
    case STATE_CLIMB:
        // Past the top of the wall it steps onto it; a ceiling makes it jump at Kirby;
        if (!WallAhead(obj)) {
            SetState(g, STATE_TOP);
            break;
        }
        if ((obj->base.unk62 & 8) && g->state == STATE_FLAP_UP) {
            SetState(g, STATE_SHAKE);
            break;
        }
        if (Steps(g, MOVES)) {
            if (g->state == STATE_CLIMB) SetState(g, STATE_CLING);
            else Pick(g);
        }
        break;
    case STATE_GLIDE_DOWN:
    case STATE_SLIDE:
        // Past the bottom of the wall (or onto the floor) it drops off;
        if (!WallAhead(obj) || (obj->base.unk62 & 4)) {
            SetState(g, STATE_DROP);
            break;
        }
        if (Steps(g, MOVES)) {
            if (g->state == STATE_SLIDE) SetState(g, STATE_CLING);
            else Pick(g);
        }
        break;
    case STATE_TOP:
        NidlFall(obj);
        if (!--g->timer) SetState(g, STATE_WALK);
        break;
    case STATE_DROP:
    case STATE_JUMP:
        if (g->state == STATE_JUMP) {
            obj->base.yspeed -= JUMP_GRAVITY;
            if (obj->base.yspeed < -0x300) obj->base.yspeed = -0x300;
        } else {
            NidlFall(obj);
        }
        // Into a wall it clings to it; on the floor it walks;
        if (obj->base.unk62 & 3) {
            SetState(g, STATE_CLING);
        } else if ((obj->base.unk62 & 4) && obj->base.yspeed <= 0) {
            SetState(g, STATE_WALK);
        }
        break;
    case STATE_SHAKE:
        if (--g->timer) break;
        g->timer = 2;
        obj->base.xspeed = -obj->base.xspeed;
        if (++g->count == SHAKES * 2) SetState(g, STATE_JUMP);
        break;
    case STATE_THROW:
        if (g->step == 0 && g->timer == 1) Star(g);
        NidlFall(obj);
        if (Steps(g, 1)) SetState(g, STATE_WALK);
        break;
    case STATE_IN_PLACE:
        NidlFall(obj);
        break;
    }
}

// GipStart(); Behavior start, also called by the engine after Gip recovers from a hit;
void GipStart(struct Object *obj) {
    struct Gip *g = (struct Gip *)obj;

    ObjectSetFunc(obj, 0, GipMain);
    if (g->n.variant == 1) {
        g->state = STATE_IN_PLACE;
        obj->base.xspeed = obj->base.yspeed = 0;
        NidlFaceKirby(obj);
        NidlPlay(&g->n, sWalk);
        return;
    }
    obj->base.yspeed = 0;
    // 0x0808C82C: on a wall within 12 pixels it clings to it, facing it;
    obj->base.flags &= ~1;
    if (WallAhead(obj)) {
        SetState(g, STATE_CLING);
        return;
    }
    obj->base.flags |= 1;
    if (WallAhead(obj)) {
        SetState(g, STATE_CLING);
        return;
    }
    SetState(g, (obj->base.unk62 & 4) || NidlIsFloor(obj, obj->base.x, obj->base.y + 0x1000) ? STATE_WALK : STATE_DROP);
}

// GipInit(); Its box;
void GipInit(struct NidlObject *n) {
    struct Object *obj = &n->obj;

    sub_0803E2B0(&obj->base, -6, -5, 6, 6);
    ObjectSetBounds(&obj->base, -7, -6, 7, 8);
    ((struct Gip *)n)->state = STATE_WALK;
    obj->base.unk4C = obj->base.y = ((obj->base.y + (obj->base.unk3F * 0x100)) & 0xfffff000) - (obj->base.unk3F * 0x100) - 1;
}
