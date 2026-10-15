/* Cappy; ported from Kirby: Nightmare in Dream Land (USA), object type 0x10 (spawn 0x0807E530), as NiDL enemy NIDL_CAPPY.
 * NiDL scripts (16.16 speeds, Y down; KatAM's are 8.8, Y up):
 * • Variant 0, 0x0807E640: faces Kirby and walks forward at 0.25 hopping (Y speed -1.0, gravity 1/16), its sprite turning
 *   around on every landing; after 40 hops it faces Kirby again.
 * • Variant 1, 0x0807E6D4 (0x0807E768): the capless Cappy (frame table 0x0875237C): walks and hops like variant 0, but its sprite
 *   alternates frames 4 and 6 on every landing instead.
 * • Inhaled (obj+0x7C == 3), variant 0 loses its cap (0x0807E5BC): it spawns a capless Cappy (0x08064FC4(0, 8, 1, ...):
 *   category 0, type 0x10, variant 1) starting in state 1 (0x0807E814), which faces Kirby and jumps back (0.5 back, 2.0 up,
 *   gravity 1/8, frames 4, 5, 6, 5 for 3, 2, 3, 2) and hops on once it lands (0x0807E884). Kirby swallows the cap (frame 1).
 * • Variant 2, 0x0807E950: hops in place, turning around on every landing; */
#include "../nidl.h"

// Frames: NiDL's 4 (standing), the capless Cappy's 4, 6 and 5, and NiDL's 1 (the cap). Its hurt frame is NiDL's 3;
#define FRAME_STAND 0
#define FRAME_CAPLESS 1
#define FRAME_CAPLESS_2 2
#define FRAME_CAPLESS_3 3
#define FRAME_CAP 4

static const s16 sJumpBack[] = { FRAME_CAPLESS, 3, FRAME_CAPLESS_3, 2, FRAME_CAPLESS_2, 3, FRAME_CAPLESS_3, 2, NIDL_LOOP };
#define JUMP_BACK_SPEED 0x80
#define JUMP_BACK_HEIGHT 0x200
#define JUMP_BACK_GRAVITY 0x20

#define WALK_SPEED 0x40
#define HOP_SPEED 0x100
#define GRAVITY 0x10
#define MAX_FALL 0x300
#define HOPS 40

struct Cappy {
    struct NidlObject n;
    u8 hops;
    u8 airborne;
    bool8 jumpingBack;
    bool8 capOff;
    struct ObjectTemplate capless;  // The template of the capless Cappy it spawned as (the engine keeps a pointer to it);
};

// The struct must fit the type's objects;
typedef char sCappyFits[sizeof(struct Cappy) <= NIDL_OBJECT_SIZE ? 1 : -1];

// Walk(); 0x0807E652: faces Kirby and walks towards it (in place for variant 2);
static void Walk(struct Cappy *cappy) {
    struct Object *obj = &cappy->n.obj;

    NidlFaceKirby(obj);
    obj->base.xspeed = cappy->n.variant == 2 ? 0 : (obj->base.flags & 1) ? -WALK_SPEED : WALK_SPEED;
    cappy->hops = HOPS;
}

static void Hop(struct Cappy *cappy) {
    cappy->n.obj.base.yspeed = HOP_SPEED;
    cappy->airborne = TRUE;
}

static void CappyMain(struct Object *obj) {
    struct Cappy *cappy = (struct Cappy *)obj;

    NidlUpdate(&cappy->n);
    obj->base.flags |= 4;

    // 0x0807E814: jumping back without its cap, until it lands;
    if (cappy->jumpingBack) {
        NidlAnimate(&cappy->n);
        obj->base.yspeed -= JUMP_BACK_GRAVITY;
        if (obj->base.yspeed < -MAX_FALL) obj->base.yspeed = -MAX_FALL;
        if (!(obj->base.unk62 & 4) || obj->base.yspeed > 0) return;
        cappy->jumpingBack = FALSE;
        cappy->n.sequence = NULL;
        NidlSetFrame(&cappy->n, FRAME_CAPLESS);
        obj->base.yspeed = 0;
        Walk(cappy);
        Hop(cappy);
        return;
    }

    obj->base.yspeed -= GRAVITY;
    if (obj->base.yspeed < -MAX_FALL) obj->base.yspeed = -MAX_FALL;

    if (!(obj->base.unk62 & 4) || obj->base.yspeed > 0) return;

    // Landed: the sprite turns around (not the walk), and it hops again;
    if (cappy->airborne) {
        cappy->airborne = FALSE;
        if (cappy->n.variant == 1)
            NidlSetFrame(&cappy->n, obj->unk83 == cappy->n.animBase + FRAME_CAPLESS ? FRAME_CAPLESS_2 : FRAME_CAPLESS);
        else
            obj->base.flags ^= 1;
        if (cappy->n.variant != 2 && cappy->hops-- == 0) {
            Walk(cappy);
        }
    }
    Hop(cappy);
}

// CappyStart(); Behavior start, also called by the engine after Cappy recovers from a hit;
void CappyStart(struct Object *obj) {
    struct Cappy *cappy = (struct Cappy *)obj;

    ObjectSetFunc(obj, 0, CappyMain);
    cappy->jumpingBack = FALSE;
    cappy->n.sequence = NULL;
    NidlSetFrame(&cappy->n, cappy->n.variant == 1 ? FRAME_CAPLESS : FRAME_STAND);
    Walk(cappy);
    Hop(cappy);
}

// JumpBack(); 0x0807E814: the capless Cappy that just lost its cap;
static void JumpBack(struct Cappy *cappy) {
    struct Object *obj = &cappy->n.obj;

    cappy->jumpingBack = TRUE;
    NidlFaceKirby(obj);
    obj->base.xspeed = (obj->base.flags & 1) ? JUMP_BACK_SPEED : -JUMP_BACK_SPEED;
    obj->base.yspeed = JUMP_BACK_HEIGHT;
    NidlPlay(&cappy->n, sJumpBack);
}

/* CappyWatch(); Variant 0's pre-callback (obj->unk7C, which the engine calls every frame, even while inhaled): once Kirby starts
 * inhaling it (the engine sets unkC & 0x400), the cap is all that's swallowed and a capless Cappy springs out (0x0807E5BC); */
static void CappyWatch(struct Object *obj) {
    struct Cappy *cappy = (struct Cappy *)obj, *capless;
    struct ObjectTemplate template;

    if (!(obj->base.unkC & 0x400)) return;
    NidlSetFrame(&cappy->n, FRAME_CAP);
    if (cappy->capOff || !obj->objTemplate) return;
    cappy->capOff = TRUE;

    // Its own room slot stays its own (31: none), so defeating the capless one doesn't count twice;
    template = *obj->objTemplate;
    template.x = obj->base.x >> 8;
    template.y = obj->base.y >> 8;
    template.subtype1 = (NIDL_CAPPY << 4) | 1;
    template.unk2 = 0;
    template.unk3 = 31;
    capless = CreateNidlEnemy(&template, obj->base.unk56);
    capless->capless = template;
    capless->n.obj.objTemplate = &capless->capless;
    capless->n.obj.base.y = obj->base.y;
    JumpBack(capless);
}

// CappyInit(); Its box, standing on the floor;
void CappyInit(struct NidlObject *n) {
    struct Object *obj = &n->obj;

    sub_0803E2B0(&obj->base, -5, -4, 5, 7);
    ObjectSetBounds(&obj->base, -6, -5, 6, 9);
    ((struct Cappy *)n)->capOff = FALSE;
    if (n->variant == 0) obj->unk7C = CappyWatch;
    obj->base.unk4C = obj->base.y = ((obj->base.y + (obj->base.unk3F * 0x100)) & 0xfffff000) - (obj->base.unk3F * 0x100) - 1;
}
