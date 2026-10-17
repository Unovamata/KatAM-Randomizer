/* Paint Roller; ported from Kirby: Nightmare in Dream Land (USA), boss object type 0x3C (spawn 0x080ACEA8), as NiDL boss
 * NIDL_PAINT_ROLLER, Ice Cream Island's (room 0x104 here, from NiDL's spot (24, 40)). The room's four easels are its own tiles; NiDL's
 * Paint Roller skates up to one, paints on it (strokes drawn into the background) and what he painted comes to life: an enemy Kirby
 * can inhale (Waddle Doo, a parasol...), a Maxim Tomato or an Invincibility Candy now and then. Here the painting is a sparkle on the
 * easel and a KatAM enemy or food out of it (gone with him, as KatAM's Kracko's Waddle Does: sub_0809F840); he paints the two easels on
 * the floor (NiDL's (60, 124) and (196, 124), 0x0874AAD4). His frames are streamed frames with two palettes each (0x08753994), so his
 * object draws the first palette's pieces and a sprite follows it with the second's (PaintPart). Its frames (KatAMExpansion.cs): 0-40
 * NiDL's 0-40 (the first palette's pieces), 41-81 the same (the second's): 0-3 standing, 4-7 turning to the easel, 16-19 skating,
 * 28-37 painting (from behind), 38-39 jumping, 10-11 landing.
 * What he does (NiDL's moves, by its pictures): skates to an easel, paints, steps back as the painting comes out, then skates across
 * the room at Kirby, or jumps at him; touching him hurts; */
#include "../nidl.h"
#include "code_0806F780.h"
#include "task.h"
#include "constants/songs.h"

#define FRAMES 41               // The second palette's pieces follow the first's;
#define PART_TILES 64
#define SKATE_SPEED 0x200
#define DASH_SPEED 0x380
#define JUMP_SPEED 0x180
#define JUMP_UP 0x480
#define GRAVITY 0x30
#define MAX_FALL 0x500
#define PAINT_TIME 72
#define PAINTINGS 2             // At most out at once;

enum { STATE_IDLE, STATE_SKATE, STATE_TURN, STATE_PAINT, STATE_BACK, STATE_DASH, STATE_JUMP };

// The easels he paints (NiDL's room positions, the room's here too: 0x0874AAD4's floor ones);
static const s16 sEaselX[] = { 60, 196 };
#define EASEL_Y 124

// What comes out of a painting (KatAM's): enemies to inhale (Beam, Fire, Cutter, Spark, Parasol's Waddle Dee...), food now and then;
static const u8 sPaintings[] = {
    OBJ_WADDLE_DEE_1, OBJ_WADDLE_DOO, OBJ_BRONTO_BURT, OBJ_HOT_HEAD, OBJ_SIR_KIBBLE, OBJ_SPARKY, OBJ_WADDLE_DOO, OBJ_TOMATO,
};

struct PaintRoller {
    struct NidlObject n;
    u8 state;
    u16 timer;
    u8 frame;
    u8 anim;
    u8 easel;
    bool8 started;
    bool8 airborne;
    struct Object *paintings[PAINTINGS];
};

typedef char sPaintRollerFits[sizeof(struct PaintRoller) <= NIDL_OBJECT_SIZE ? 1 : -1];

// PaintPart(); The second palette's pieces of his frame, following him (as KatAM's Kracko's eye: sub_080DD9B4);
static void PaintPartMain(void) {
    struct Sprite sprite;
    struct EffectObject *part = TaskGetStructPtr(gCurTask);
    struct PaintRoller *p = part->parent;
    struct Object *obj = &p->n.obj;
    u8 frame = p->frame + FRAMES;

    if (part->flags & 0x1000) {
        TaskDestroy(gCurTask);
        return;
    }
    if (obj->base.flags & 0x1000) {
        part->flags |= 0x1000;
        return;
    }
    Macro_08107BA8_4(part, &part->sprite, &sprite, PART_TILES, &part->sprite);
    if (!part->sprite.palId) part->sprite.palId = NidlLoadPalette(&p->n, NidlGraphicsOf(&p->n)->animId | NIDL_SECOND_PALETTE, frame);
    if (Macro_0810B1F4(&obj->base) && !(part->flags & 0x2000)) {
        EffectObjectDisplaySprite(part);
        return;
    }
    part->flags |= 4;
    part->flags = (part->flags & ~0x401) | (obj->base.flags & 0x401);
    part->x = obj->base.x;
    part->y = obj->base.y;
    part->objBase54 = obj->base.objBase54;
    part->objBase55 = obj->base.objBase55;
    part->sprite.variant = frame;
    EffectObjectPostUpdate(part);
}

static void PaintPart(struct PaintRoller *p) {
    struct Object *obj = &p->n.obj;
    struct Task *t = TaskCreate(PaintPartMain, sizeof(struct EffectObject), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct EffectObject *part = TaskGetStructPtr(t);

    ClearEffectObject(part);
    part->header.kind = 3;
    part->x = obj->base.x;
    part->y = obj->base.y;
    part->parent = obj;
    part->roomId = obj->base.roomId;
    // Behind his first palette's pieces (its priority higher: his is 0x6C0 >> 6);
    EffectObjectInitSprite(part, &part->sprite, PART_TILES, NidlGraphicsOf(&p->n)->animId, FRAMES, 0x1C);
    part->sprite.palId = NidlLoadPalette(&p->n, NidlGraphicsOf(&p->n)->animId | NIDL_SECOND_PALETTE, FRAMES);
}

// PaintingsOut(); How many of his paintings are still around;
static u8 PaintingsOut(struct PaintRoller *p) {
    u8 i, out = 0;

    for (i = 0; i < PAINTINGS; i++) {
        struct Object *o = p->paintings[i];

        if (!o) continue;
        // (a new object may take a gone one's memory: one slot per object);
        if (o->base.header.kind && o->base.parent == &p->n.obj && !(o->base.flags & 0x1000) && (i == 0 || o != p->paintings[0])) out++;
        else p->paintings[i] = NULL;
    }
    return out;
}

// Paint(); The painting comes to life: a sparkle on the easel and what he painted out of it;
static void Paint(struct PaintRoller *p) {
    struct Object *obj = &p->n.obj;
    s32 x = sEaselX[p->easel], y = EASEL_Y;
    struct EffectObject *sparkle = CreateEffectObject(&obj->base, 0, 0x2A3, 0);
    struct Object *painting;
    u8 i, type = sPaintings[Rand16() % ARRAY_COUNT(sPaintings)];

    sparkle->x = x * 0x100;
    sparkle->y = y * 0x100;
    PaintingsOut(p);
    for (i = 0; i < PAINTINGS && p->paintings[i]; i++) ;
    if (i == PAINTINGS) return;
    painting = CreateObjTemplateAndObj(obj->base.unk56, 1, 0x24, x, y, 0, 0x1F, 0, 0, type, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    painting->base.parent = obj;
    painting->unk7C = sub_0809F840;
    for (i = 0; i < PAINTINGS; i++)
        if (p->paintings[i] == painting) p->paintings[i] = NULL;
    for (i = 0; i < PAINTINGS && p->paintings[i]; i++) ;
    p->paintings[i] = painting;
    PlaySfx(&obj->base, SE_KIRBY_AIRSHOT);
}

static void Set(struct PaintRoller *p, u8 state) {
    p->state = state;
    p->timer = 0;
}

static void Jump(struct PaintRoller *p) {
    struct Object *obj = &p->n.obj;

    NidlFaceKirby(obj);
    obj->base.xspeed = (obj->base.flags & 1) ? -JUMP_SPEED : JUMP_SPEED;
    obj->base.yspeed = JUMP_UP;
    p->airborne = TRUE;
    Set(p, STATE_JUMP);
    PlaySfx(&obj->base, SE_BASIC_ENEMY_JUMP);
}

static void Update(struct PaintRoller *p) {
    struct Object *obj = &p->n.obj;
    s32 x = obj->base.x >> 8, target;

    switch (p->state) {
    case STATE_IDLE:
        p->frame = (p->anim >> 3) & 3;
        NidlFaceKirby(obj);
        if (++p->timer < 30) break;
        if (PaintingsOut(p) < PAINTINGS && (Rand16() & 3)) {
            // The easel farther from Kirby (NiDL's goes to one and the other);
            p->easel = (obj->kirby3->base.x >> 8) < (sEaselX[0] + sEaselX[1]) / 2;
            Set(p, STATE_SKATE);
        } else if (Rand16() & 1) {
            NidlFaceKirby(obj);
            Set(p, STATE_DASH);
            PlaySfx(&obj->base, SE_KIRBY_DASH);
        } else {
            Jump(p);
        }
        break;
    case STATE_SKATE:
        target = sEaselX[p->easel];
        p->frame = 16 + ((p->anim >> 2) & 3);
        if (x < target - 4) {
            obj->base.flags &= ~1;
            obj->base.xspeed = SKATE_SPEED;
        } else if (x > target + 4) {
            obj->base.flags |= 1;
            obj->base.xspeed = -SKATE_SPEED;
        } else {
            obj->base.xspeed = 0;
            Set(p, STATE_TURN);
        }
        if (++p->timer > 180) {
            obj->base.xspeed = 0;
            Set(p, STATE_TURN);
        }
        break;
    case STATE_TURN:
        p->frame = 4 + ((p->timer >> 2) & 1);
        if (++p->timer >= 8) Set(p, STATE_PAINT);
        break;
    case STATE_PAINT:
        p->frame = 28 + ((p->timer >> 2) % 10);
        if (!(p->timer & 15)) PlaySfx(&obj->base, SE_ABILITY_SPARK_ATTACK);
        if (++p->timer >= PAINT_TIME) {
            Paint(p);
            Set(p, STATE_BACK);
        }
        break;
    case STATE_BACK:
        p->frame = 6 + ((p->timer >> 2) & 1);
        if (++p->timer >= 12) Set(p, STATE_IDLE);
        break;
    case STATE_DASH:
        // Across the room at Kirby, until a wall;
        p->frame = 20 + ((p->anim >> 2) & 3);
        obj->base.xspeed = (obj->base.flags & 1) ? -DASH_SPEED : DASH_SPEED;
        // (the screen's sides: the arena holds Kirby there, not him);
        x -= gCurLevelInfo[obj->base.unk56].viewportPosition.x >> 8;
        if ((obj->base.unk62 & 3) || ++p->timer > 90 || (obj->base.flags & 1 ? x < 20 : x > 220)) {
            obj->base.xspeed = 0;
            obj->base.flags ^= 1;
            Set(p, STATE_IDLE);
        }
        break;
    case STATE_JUMP:
        p->frame = obj->base.yspeed > 0 ? 38 + ((p->anim >> 3) & 1) : 10 + ((p->anim >> 3) & 1);
        obj->base.yspeed -= GRAVITY;
        if (obj->base.yspeed < -MAX_FALL) obj->base.yspeed = -MAX_FALL;
        if (obj->base.yspeed <= 0 && (obj->base.unk62 & 4)) {
            obj->base.xspeed = obj->base.yspeed = 0;
            p->airborne = FALSE;
            Set(p, STATE_IDLE);
        }
        break;
    }
}

static void PaintRollerMain(struct Object *obj) {
    struct PaintRoller *p = (struct PaintRoller *)obj;

    NidlUpdate(&p->n);
    obj->base.flags |= 4;
    obj->kirby3 = FindTargetKirby(&obj->base);
    // The HP bar goes to whatever is hit (his paintings too); he takes it back;
    sub_08034E14(obj);
    p->anim++;
    if (!p->airborne) NidlFall(obj);
    Update(p);
    NidlSetFrame(&p->n, p->frame);
}

// PaintRollerStart(); The fight starts;
void PaintRollerStart(struct Object *obj) {
    struct PaintRoller *p = (struct PaintRoller *)obj;

    ObjectSetFunc(obj, -1, PaintRollerMain);
    if (p->started) return;
    p->started = TRUE;
    Set(p, STATE_IDLE);
    PaintPart(p);
}

// PaintRollerInit(); His boxes (his body, 18 pixels down to his skates);
void PaintRollerInit(struct NidlObject *n) {
    struct PaintRoller *p = (struct PaintRoller *)n;
    struct Object *obj = &n->obj;
    u8 i;

    p->started = FALSE;
    p->airborne = FALSE;
    p->frame = 0;
    p->anim = 0;
    for (i = 0; i < PAINTINGS; i++) p->paintings[i] = NULL;
    obj->base.unk5C = (obj->base.unk5C & ~7) | 3 | 0xA0;
    sub_0803E2B0(&obj->base, -14, -12, 14, 18);
    ObjectSetBounds(&obj->base, -10, -10, 10, 18);
}
