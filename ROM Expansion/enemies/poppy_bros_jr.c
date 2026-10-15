/* Poppy Bros. Jr.; ported from Kirby: Nightmare in Dream Land (USA), object types 0x26, 0x27 and 0x28 (spawns 0x080819B4,
 * 0x08081F60, 0x08082008), as NiDL enemy NIDL_POPPY_BROS_JR. The three types share their graphics and palette.
 * NiDL scripts (16.16 speeds, Y down; KatAM's are 8.8, Y up):
 * • Type 0x26, variants 0x087415B8 (0 and 1: 0x080819F4, states 0x087415C4, per frame 0x087415D0; 2: 0x08081D24):
 *   - Hop (0x08081AAC): frame 7 for 4, then 0.5 forward (0x087415AC) and 0.8 up (0xFFFF3300) with no gravity for 6 frames
 *     (then 0x1500, falling at 3.0 at most, 0x08081B5C), frames 5, 4, 5 for 4, 12, 4, then 6 until it lands. Landing
 *     (0x08081E64) counts the hops: every other one it turns around one time in 3, for one hop. Walls turn it around
 *     (0x08081F38). (In rooms where 0x03005554 is set, it walks instead, 0x08081BC4: not ported.)
 *   - Variant 1 starts as variant 0 jumping in (0x08081C78): 30 frames without moving, then 2.0 up (gravity 0x1500; frames 6,
 *     5, 4, 5 for 3, 4, 12, 4, then 6) and hops once it lands. It's what jumps off a defeated ball (below).
 *   - Variant 2 (0x08081DB4) hops in place.
 * • Types 0x27 (a ball) and 0x28 (a bomb), variants 0x08741604 (0: 0x080820B8, states 0x08741610; 1: the ball or bomb alone,
 *   0x080822B0, 0x08741618; 2: 0x0808248C, in place):
 *   - Variant 0 rolls the ball forward at 0.5 (0x08082270), which spins (frames 9-16, every 8, 0x080825EC) with Poppy Bros. Jr.
 *     hopping on top of it (a second sprite 16 pixels up, 0x08082554: frame 7 for 4, then 1.5 up with gravity 0x1800, frames 6,
 *     5, 4, 5 for 2, 2, 16, 6, then 6 until it's back on the ball). Walls turn it around (0x080826BC).
 *   - Inhaled (obj+0x7C 3 or 4, 0x08082108), the ball or bomb comes off (category 0, type 0x27 or 0x28, variant 1) and Kirby
 *     swallows Poppy Bros. Jr. alone (frame table 0x087528D0). Defeated (obj+0x7C 1), Poppy Bros. Jr. jumps off: type 0x26,
 *     variant 1, 14 pixels up.
 *   - The ball or bomb alone (0x08082338) bounces away from Kirby at 0.5 (0x0806421C), 2.0 up then 1.0 up (gravity 0x4000,
 *     0x2000; 0x087415F4, 0x087415FC), spinning every 4 frames (0x08082458), then stops; the bomb explodes (category 6, 2).
 * • Water defeats them all (0x08081F50, 0x080826C8).
 * Here the rider is an effect object drawing the enemy's own frames; the bomb's explosion is KatAM's small one (Foley's); */
#include "../nidl.h"
#include "code_0806F780.h"
#include "task.h"

// Frames: NiDL's 4 to 7 (Poppy Bros. Jr.), then the ball's 9 to 16 (type 0x27) and the bomb's 9 to 16 (type 0x28). Its hurt frame
// is NiDL's 3;
#define FRAME(nidl) ((nidl) - 4)
#define FRAME_BALL 4
#define FRAME_BOMB 12
#define BALL_FRAMES 8

enum {
    VARIANT_HOP, VARIANT_JUMP_IN, VARIANT_HOP_IN_PLACE, VARIANT_BALL, VARIANT_BOMB, VARIANT_BALL_IN_PLACE, VARIANT_BOMB_IN_PLACE,
    VARIANT_THROWN_BALL, VARIANT_THROWN_BOMB,
};

#define HOP_SPEED 0x80
#define HOP_HEIGHT 0xCD
#define HOP_FLOAT 6
#define JUMP_IN_WAIT 30
#define JUMP_IN_HEIGHT 0x200
#define ROLL_SPEED 0x80
#define RIDER_HOP 0x180
#define RIDER_GRAVITY 0x18
#define RIDER_UP 16
#define RIDER_TILES 16
#define SPIN_STEP 8
#define THROWN_SPIN_STEP 4
#define THROWN_SPEED 0x80

// The hop's frames (after the crouch), and the jump in's;
static const s16 sHop[] = { FRAME(5), 4, FRAME(4), 12, FRAME(5), 4, FRAME(6), 1, NIDL_END };
static const s16 sJumpIn[] = { FRAME(6), 3, FRAME(5), 4, FRAME(4), 12, FRAME(5), 4, FRAME(6), 1, NIDL_END };
// The rider's frames from the hop off the ball on, with how long; then 6 until it lands;
static const s16 sRiderHop[] = { FRAME(6), 2, FRAME(5), 2, FRAME(4), 16, FRAME(5), 6, FRAME(6), 1, NIDL_END };
static const s16 sThrownBounces[] = { 0x200, 0x100 };
static const s16 sThrownGravity[] = { 0x40, 0x20 };

enum { STATE_CROUCH, STATE_HOP, STATE_JUMP_IN_WAIT, STATE_JUMP_IN, STATE_ROLL, STATE_THROWN, STATE_STOPPED };

struct PoppyBrosJr {
    struct NidlObject n;
    u8 state;
    u8 timer;
    u8 hops;
    bool8 turned;
    u8 spin;
    u8 spinTimer;
    u8 bounce;
    bool8 spawned;          // The ball came off, or Poppy Bros. Jr. jumped off;
    struct EffectObject *rider;
    u8 riderFrame;
    u8 riderPalette;        // Poppy Bros. Jr.'s palette (the ball and bomb use NiDL's palette 15);
    u8 riderStep;
    u8 riderTimer;
    bool8 riderUp;
    s16 riderY;             // 8.8, negative is up;
    s16 riderSpeed;
    struct ObjectTemplate spawn;    // The template of what it spawned as (the engine keeps a pointer to it);
};

typedef char sPoppyFits[sizeof(struct PoppyBrosJr) <= NIDL_OBJECT_SIZE ? 1 : -1];

static bool8 Rides(struct PoppyBrosJr *p) {
    return p->n.variant >= VARIANT_BALL && p->n.variant <= VARIANT_BOMB_IN_PLACE;
}

static bool8 Bomb(struct PoppyBrosJr *p) {
    return p->n.variant == VARIANT_BOMB || p->n.variant == VARIANT_BOMB_IN_PLACE || p->n.variant == VARIANT_THROWN_BOMB;
}

/* RiderMain(); Poppy Bros. Jr. on top of the ball: an effect object drawing its parent's frame riderFrame, RIDER_UP pixels up
 * and riderY higher; hidden while the parent is hurt or inhaled, gone with it; */
static void RiderMain(void) {
    struct Sprite sprite;
    struct EffectObject *r = TaskGetStructPtr(gCurTask);
    struct PoppyBrosJr *p = r->parent;
    struct Object *obj;

    if (r->flags & 0x1000) {
        TaskDestroy(gCurTask);
        return;
    }
    obj = &p->n.obj;
    if ((obj->base.flags & 0x1000) || p->rider != r) {
        r->flags |= 0x1000;
        return;
    }
    r->x = obj->base.x;
    r->y = obj->base.y - RIDER_UP * 0x100 + p->riderY;
    r->flags = (r->flags & ~1) | (obj->base.flags & 1);
    if (obj->unk83 < 2 || (obj->base.unkC & 0x400) || (obj->base.flags & 0x400))
        r->flags |= 0x400;
    else
        r->flags &= ~0x400;
    if (Macro_0810B1F4(&obj->base) && !(r->flags & 0x2000)) {
        EffectObjectDisplaySprite(r);
        return;
    }
    Macro_08107BA8_4(r, &r->sprite, &sprite, RIDER_TILES, &r->sprite);
    r->sprite.variant = p->riderFrame;
    r->sprite.palId = p->riderPalette;
    EffectObjectPostUpdate(r);
}

static void CreateRider(struct PoppyBrosJr *p) {
    struct Object *obj = &p->n.obj;
    struct Task *t = TaskCreate(RiderMain, sizeof(struct EffectObject), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct EffectObject *r = TaskGetStructPtr(t);

    ClearEffectObject(r);
    r->header.kind = 3;
    r->x = obj->base.x;
    r->y = obj->base.y - RIDER_UP * 0x100;
    r->parent = p;
    r->roomId = obj->base.roomId;
    r->flags |= obj->base.flags & 1;
    if (Macro_0810B1F4(&obj->base)) r->flags |= 0x2000;
    EffectObjectInitSprite(r, &r->sprite, RIDER_TILES, gNidlData.enemies[p->n.enemy].animId, p->riderFrame, obj->base.sprite.unk14 >> 6);
    r->sprite.palId = p->riderPalette;
    p->rider = r;
}

// Spawn(); Another Poppy Bros. Jr. (variant) where this one is, dy pixels up, keeping its own template copy;
static struct PoppyBrosJr *Spawn(struct Object *obj, u8 variant, s32 dy) {
    struct ObjectTemplate template;
    struct PoppyBrosJr *spawned;

    if (!obj->objTemplate) return NULL;
    // Its room slot stays its own (31: none), so defeating the new one doesn't count twice;
    template = *obj->objTemplate;
    template.x = obj->base.x >> 8;
    template.y = (obj->base.y >> 8) - dy;
    template.subtype1 = (NIDL_POPPY_BROS_JR << 4) | variant;
    template.unk2 = 0;
    template.unk3 = 31;
    spawned = CreateNidlEnemy(&template, obj->base.unk56);
    spawned->spawn = template;
    spawned->n.obj.objTemplate = &spawned->spawn;
    spawned->n.obj.base.y = obj->base.y - dy * 0x100;
    return spawned;
}

/* RiderWatch(); The rider's pre-callback (obj->unk7C, called every frame even while inhaled or defeated): inhaled, the ball or
 * bomb comes off (0x08082108); defeated, Poppy Bros. Jr. jumps off; */
static void RiderWatch(struct Object *obj) {
    struct PoppyBrosJr *p = (struct PoppyBrosJr *)obj;

    if (p->spawned) return;
    if (obj->base.unkC & 0x400) {
        p->spawned = TRUE;
        Spawn(obj, Bomb(p) ? VARIANT_THROWN_BOMB : VARIANT_THROWN_BALL, 0);
    } else if (obj->unk80 <= 0) {
        p->spawned = TRUE;
        Spawn(obj, VARIANT_JUMP_IN, 14);
    }
}

static void Crouch(struct PoppyBrosJr *p) {
    struct Object *obj = &p->n.obj;

    p->state = STATE_CROUCH;
    p->timer = 4;
    obj->base.xspeed = 0;
    p->n.sequence = NULL;
    NidlSetFrame(&p->n, FRAME(7));
}

// Hop(); 0x08081AAC: forward (in place for variant 2), floating 6 frames before it falls;
static void Hop(struct PoppyBrosJr *p) {
    struct Object *obj = &p->n.obj;
    s16 speed = p->n.variant == VARIANT_HOP_IN_PLACE ? 0 : HOP_SPEED;

    p->state = STATE_HOP;
    p->timer = HOP_FLOAT;
    obj->base.xspeed = (obj->base.flags & 1) ? -speed : speed;
    obj->base.yspeed = HOP_HEIGHT;
    NidlPlay(&p->n, sHop);
}

// Landed(); 0x08081E64: every other hop, one time in 3, the next one goes the other way (and the one after back);
static void Landed(struct PoppyBrosJr *p) {
    struct Object *obj = &p->n.obj;

    if (p->turned) {
        p->turned = FALSE;
        obj->base.flags ^= 1;
    } else if (!(++p->hops & 1) && Rand16() % 3 == 0) {
        p->turned = TRUE;
        obj->base.flags ^= 1;
    }
    Crouch(p);
}

// RiderHop(); 0x08082554 and 0x080825EC: Poppy Bros. Jr. hopping on the ball;
static void RiderHop(struct PoppyBrosJr *p) {
    if (!p->riderUp) {
        // Crouched on the ball (frame 7 for 4), then off;
        if (--p->riderTimer) return;
        p->riderUp = TRUE;
        p->riderSpeed = -RIDER_HOP;
        p->riderStep = 0;
        p->riderTimer = sRiderHop[1];
        p->riderFrame = sRiderHop[0];
        return;
    }
    p->riderSpeed += RIDER_GRAVITY;
    p->riderY += p->riderSpeed;
    if (sRiderHop[p->riderStep + 2] != NIDL_END && !--p->riderTimer) {
        p->riderStep += 2;
        p->riderFrame = sRiderHop[p->riderStep];
        p->riderTimer = sRiderHop[p->riderStep + 1];
    }
    if (p->riderY >= 0) {
        p->riderY = 0;
        p->riderUp = FALSE;
        p->riderTimer = 4;
        p->riderFrame = FRAME(7);
    }
}

// Spin(); The ball's (or bomb's) frames, one step every `step` frames;
static void Spin(struct PoppyBrosJr *p, u8 step) {
    if (--p->spinTimer) return;
    p->spinTimer = step;
    if (++p->spin == BALL_FRAMES) p->spin = 0;
    NidlSetFrame(&p->n, (Bomb(p) ? FRAME_BOMB : FRAME_BALL) + p->spin);
}

static void Bounce(struct PoppyBrosJr *p) {
    p->n.obj.base.yspeed = sThrownBounces[p->bounce];
}

static void PoppyBrosJrMain(struct Object *obj) {
    struct PoppyBrosJr *p = (struct PoppyBrosJr *)obj;

    NidlUpdate(&p->n);
    obj->base.flags |= 4;

    switch (p->state) {
    case STATE_CROUCH:
        NidlFall(obj);
        if (!--p->timer) Hop(p);
        break;
    case STATE_HOP:
    case STATE_JUMP_IN:
        NidlAnimate(&p->n);
        if (p->timer) p->timer--;
        else NidlFall(obj);
        // Walls turn it around (0x08081F38);
        if ((obj->base.unk62 & 3) && obj->base.xspeed) {
            obj->base.flags ^= 1;
            obj->base.xspeed = -obj->base.xspeed;
            p->hops = 0;
            p->turned = FALSE;
        }
        if ((obj->base.unk62 & 4) && obj->base.yspeed <= 0 && !p->timer) {
            obj->base.yspeed = 0;
            if (p->state == STATE_JUMP_IN) Crouch(p);
            else Landed(p);
        }
        break;
    case STATE_JUMP_IN_WAIT:
        // 0x08081CE0: no physics for 30 frames;
        obj->base.xspeed = obj->base.yspeed = 0;
        if (--p->timer) break;
        p->state = STATE_JUMP_IN;
        obj->base.yspeed = JUMP_IN_HEIGHT;
        NidlPlay(&p->n, sJumpIn);
        break;
    case STATE_ROLL:
        NidlFall(obj);
        if ((obj->base.unk62 & 3) && obj->base.xspeed) {
            obj->base.flags ^= 1;
            obj->base.xspeed = -obj->base.xspeed;
        }
        Spin(p, SPIN_STEP);
        RiderHop(p);
        if (!p->rider) CreateRider(p);
        break;
    case STATE_THROWN:
        // 0x08082338: two bounces away from Kirby, spinning, then it stops (the bomb explodes);
        obj->base.yspeed -= sThrownGravity[p->bounce];
        if (obj->base.yspeed < -0x300) obj->base.yspeed = -0x300;
        Spin(p, THROWN_SPIN_STEP);
        if (!(obj->base.unk62 & 4) || obj->base.yspeed > 0) break;
        if (++p->bounce < ARRAY_COUNT(sThrownBounces)) {
            Bounce(p);
            break;
        }
        obj->base.xspeed = obj->base.yspeed = 0;
        p->state = STATE_STOPPED;
        if (Bomb(p)) {
            obj->base.flags |= 0x1000;
            sub_08073D2C(&obj->base);
        }
        break;
    case STATE_STOPPED:
        NidlFall(obj);
        break;
    }
}

// PoppyBrosJrStart(); Behavior start, also called by the engine after it recovers from a hit;
void PoppyBrosJrStart(struct Object *obj) {
    struct PoppyBrosJr *p = (struct PoppyBrosJr *)obj;

    ObjectSetFunc(obj, 0, PoppyBrosJrMain);
    obj->base.yspeed = 0;
    // The ball and the bomb: NiDL's palette 15 (0x08081FC4 sets it in obj+0x40, keeping Poppy Bros. Jr.'s for the rider), a second
    // palette its ball frames load;
    p->riderPalette = NidlLoadPalette(&p->n, gNidlData.enemies[p->n.enemy].animId, 0);
    if (Rides(p) || p->n.variant == VARIANT_THROWN_BALL || p->n.variant == VARIANT_THROWN_BOMB)
        obj->base.sprite.palId = NidlLoadPalette(&p->n, gNidlData.enemies[p->n.enemy].animId | NIDL_SECOND_PALETTE, FRAME_BALL);
    switch (p->n.variant) {
    case VARIANT_JUMP_IN:
        if (p->state == STATE_JUMP_IN_WAIT || p->state == STATE_JUMP_IN) return;
        // Fallthrough;
    case VARIANT_HOP:
    case VARIANT_HOP_IN_PLACE:
        Crouch(p);
        break;
    case VARIANT_THROWN_BALL:
    case VARIANT_THROWN_BOMB:
        if (p->state != STATE_THROWN) p->state = STATE_STOPPED;
        obj->base.xspeed = 0;
        break;
    default:
        // Rolling the ball: forward at 0.5 (in place for variants 5 and 6);
        p->state = STATE_ROLL;
        obj->base.xspeed = p->n.variant == VARIANT_BALL || p->n.variant == VARIANT_BOMB ? ((obj->base.flags & 1) ? -ROLL_SPEED : ROLL_SPEED) : 0;
        break;
    }
}

// PoppyBrosJrInit(); Its box (the rider's covers Poppy Bros. Jr. on top), on the floor;
void PoppyBrosJrInit(struct NidlObject *n) {
    struct PoppyBrosJr *p = (struct PoppyBrosJr *)n;
    struct Object *obj = &n->obj;

    p->spawned = FALSE;
    p->rider = NULL;
    p->hops = 0;
    p->turned = FALSE;
    p->spin = 0;
    p->spinTimer = 1;
    p->bounce = 0;
    p->riderY = 0;
    p->riderUp = FALSE;
    p->riderTimer = 4;
    p->riderFrame = FRAME(7);
    NidlFaceKirby(obj);
    if (Rides(p)) {
        sub_0803E2B0(&obj->base, -6, -22, 6, 7);
        ObjectSetBounds(&obj->base, -7, -7, 7, 8);
        obj->unk7C = RiderWatch;
        p->state = STATE_ROLL;
    } else {
        sub_0803E2B0(&obj->base, -5, -5, 5, 7);
        ObjectSetBounds(&obj->base, -6, -6, 6, 9);
        p->state = STATE_CROUCH;
    }
    if (n->variant == VARIANT_JUMP_IN) {
        // 0x080819F4: variant 1 waits 30 frames, then jumps in;
        p->state = STATE_JUMP_IN_WAIT;
        p->timer = JUMP_IN_WAIT;
        NidlSetFrame(n, FRAME(6));
        return;
    }
    if (n->variant == VARIANT_THROWN_BALL || n->variant == VARIANT_THROWN_BOMB) {
        // 0x08082338: away from Kirby;
        obj->base.flags ^= 1;
        p->state = STATE_THROWN;
        obj->base.xspeed = (obj->base.flags & 1) ? -THROWN_SPEED : THROWN_SPEED;
        Bounce(p);
        NidlSetFrame(n, (Bomb(p) ? FRAME_BOMB : FRAME_BALL));
        return;
    }
    if (Rides(p)) NidlSetFrame(n, Bomb(p) ? FRAME_BOMB : FRAME_BALL);
    obj->base.unk4C = obj->base.y = ((obj->base.y + (obj->base.unk3F * 0x100)) & 0xfffff000) - (obj->base.unk3F * 0x100) - 1;
}
