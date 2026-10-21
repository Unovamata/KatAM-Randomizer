/* Heavy Mole; ported from Kirby: Nightmare in Dream Land (USA), boss object type 0x3E (spawn 0x080AD4B8), as NiDL boss
 * NIDL_HEAVY_MOLE, Yogurt Yard's (room 0x24C here). NiDL's Heavy Mole digs through a tunnel as the screen scrolls along with it; KatAM's
 * rooms don't scroll by themselves, so here it holds the right of its screen (the arena's), digging up and down. Its body (0x08753E3C's
 * 0-7, treads turning) has two arms, spiked balls on rods from its top and bottom, which reach out at Kirby (26-61: six ways, each
 * spinning) and which it fires off now and then, a ball with an exhaust flying at Kirby (64-85, the same six ways) and coming back.
 * The body, the descriptor's first palette, is this object; the arms, its second, are attack objects of their own (MoleArm). Its frames
 * (KatAMExpansion.cs): 0-7 the body, 8-43 the arms (six ways of six), 44-67 the fired balls (six ways of four). NiDL draws it facing
 * right; it faces left here, at Kirby. Touching it hurts, as do its arms. As it digs, stars (enemies/star.c) fall from the ceiling now
 * and then: Kirby inhales them and spits them at it (KatAM's bosses take only the stronger attacks; NiDL's has dirt blocks for that); */
#include "../nidl.h"
#include "code_0806F780.h"
#include "task.h"
#include "constants/songs.h"

#define FRAME_ARM 8         // + way * 6 + spin (0-3);
#define FRAME_SHOT 44       // + way * 4 + spin;
#define ARM_TILES 32
#define WAYS 6
#define DIG_RANGE 24        // Up and down from its spot;
#define DIG_SPEED 0x40
#define REACH_TIME 70
#define SHOT_SPEED 0x300
#define SHOT_TIME 50        // Out, then back;
#define ARM_Y 20            // The arms' roots, above and below its middle;
#define STAR_TIME 150
#define STAR_SPEED 0x100

/* Ways (facing right, as NiDL's frames: mirrored here): 0 right, 1 up-right, 2 up, 3 up-left, 4 left, 5 down-right; the ball's middle
 * from the arm's root in each (NiDL's frames: the rod's far end); */
static const s8 sBallX[WAYS] = { 44, 28, 0, -28, -41, 30 };
static const s8 sBallY[WAYS] = { -5, -25, -31, -25, -5, 0 };
// Which way an arm reaches (top arm, bottom arm) by where Kirby is: 0 above, 1 level, 2 below;
static const u8 sReach[3][2] = { { 2, 1 }, { 1, 0 }, { 0, 5 } };

enum { ARM_REST, ARM_REACH, ARM_FIRED };

struct HeavyMole {
    struct NidlObject n;
    u16 timer;
    u8 anim;
    bool8 started;
    s16 dig;            // Its height from its spot;
    s8 digDir;
    u8 armState[2];
    u8 armWay[2];
    u16 armTimer[2];
    s32 spotY;
    u8 nextArm;
    s16 shotX[2], shotY[2];  // A fired ball's speed;
    u16 starTimer;
};

typedef char sHeavyMoleFits[sizeof(struct HeavyMole) <= NIDL_OBJECT_SIZE ? 1 : -1];

/* MoleArm(); An arm (unk4: 0 top, 1 bottom), an attack object held at its root as it reaches (the ball at the rod's end hurts), or
 * flying at Kirby when fired (its own way), back to its root after SHOT_TIME; */
static void MoleArmMain(void) {
    struct Sprite sprite;
    struct ObjectBase *arm = TaskGetStructPtr(gCurTask);
    struct HeavyMole *m = arm->parent;
    struct Object *obj = &m->n.obj;
    u8 which = arm->filler6[1], way = m->armWay[which], state = m->armState[which];
    s32 rootX = obj->base.x, rootY = obj->base.y + (which ? ARM_Y : -ARM_Y) * 0x100;
    bool8 left = obj->base.flags & 1;

    Macro_08107BA8_4(arm, &arm->sprite, &sprite, ARM_TILES, &arm->sprite);
    if (arm->roomId != 0xFFFF && (obj->base.flags & 0x1000)) {
        arm->roomId = 0xFFFF;
        arm->flags |= 0x1000;
        return;
    }
    arm->sprite.palId = arm->filler6[0];
    if (ObjectPreUpdate(arm)) return;
    arm->flags = (arm->flags & ~1) | (left ? 1 : 0);
    if (state == ARM_FIRED) {
        // Flying: out at Kirby, then back to its root;
        if (!arm->counter) {
            arm->xspeed = m->shotX[which];
            arm->yspeed = m->shotY[which];
        }
        arm->counter++;
        if (arm->counter < SHOT_TIME) {
            arm->x += arm->xspeed;
            arm->y -= arm->yspeed;
        } else {
            arm->x += (rootX - arm->x) / 8;
            arm->y += (rootY - arm->y) / 8;
            if (arm->counter > SHOT_TIME + 40) m->armState[which] = ARM_REST;
        }
        arm->sprite.variant = FRAME_SHOT + way * 4 + ((arm->counter >> 2) & 3);
        sub_0803E2B0(arm, -8, -8, 8, 8);
    } else {
        // At its root (the sprite's origin), the rod reaching out its way (a short reach at rest);
        arm->x = rootX;
        arm->y = rootY;
        arm->counter = 0;
        arm->sprite.variant = FRAME_ARM + way * 6 + ((m->anim >> 2) & 3);
        sub_0803E2B0(arm, (left ? -sBallX[way] : sBallX[way]) - 8, sBallY[way] - 8, (left ? -sBallX[way] : sBallX[way]) + 8, sBallY[way] + 8);
    }
    SetPointerSomething(arm);
    sub_0806F8BC(arm);
}

static void MoleArm(struct HeavyMole *m, u8 which) {
    struct Object *obj = &m->n.obj;
    struct Task *t = TaskCreate(MoleArmMain, sizeof(struct ObjectBase), 0x3500, TASK_USE_EWRAM, ObjectBaseDestroy);
    struct ObjectBase *arm = TaskGetStructPtr(t);

    ClearObjectBase(arm);
    arm->header.kind = 2;
    arm->x = obj->base.x;
    arm->y = obj->base.y;
    arm->parent = obj;
    arm->counter = 0;
    arm->roomId = obj->base.roomId;
    arm->unk56 = obj->base.unk56;
    if (Macro_0810B1F4(arm)) arm->flags |= 0x2000;
    arm->unk63 = 2;
    arm->flags |= 0x10000140 | 0x2000000;
    arm->unk68 = 0x20000043;
    arm->unk5C |= 0x80000;
    arm->filler6[0] = NidlLoadPalette(&m->n, NidlGraphicsOf(&m->n)->animId | NIDL_SECOND_PALETTE, FRAME_ARM);   // Its palette, in the padding;
    arm->filler6[1] = which;
    sub_0803E2B0(arm, 0, 0, 0, 0);
    ObjectSetBounds(arm, 0, 0, 0, 0);
    // Behind the body (a higher priority: its is 0x6C0 >> 6);
    ObjectBaseInitSprite(arm, &arm->sprite, ARM_TILES, NidlGraphicsOf(&m->n)->animId, FRAME_ARM, 0x1C);
    arm->sprite.palId = arm->filler6[0];
    m->armState[which] = ARM_REST;
    m->armWay[which] = which ? 5 : 1;
}

// Fire(); An arm's ball flies off at Kirby (its arm object takes the speed on its first flying frame);
static void Fire(struct HeavyMole *m, u8 which) {
    struct Object *obj = &m->n.obj;
    s32 dx = (obj->kirby3->base.x - obj->base.x) >> 8, dy = (obj->kirby3->base.y - obj->base.y) / 0x100 - (which ? ARM_Y : -ARM_Y), d;

    d = (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy);
    if (d < 1) d = 1;
    m->armState[which] = ARM_FIRED;
    m->shotX[which] = dx * SHOT_SPEED / d;
    m->shotY[which] = -dy * SHOT_SPEED / d;
    PlaySfx(&obj->base, SE_KIRBY_DASH);
}

/* Debris(); A star falling from the top of the screen, left of it (an inhalable one: enemies/star.c), gone with it (as KatAM's Kracko's
 * Waddle Does: sub_0809F840); */
static void Debris(struct HeavyMole *m) {
    struct Object *obj = &m->n.obj, *star;
    s32 left = gCurLevelInfo[obj->base.unk56].viewportPosition.x >> 8, top = gCurLevelInfo[obj->base.unk56].viewportPosition.y >> 8;

    star = CreateObjTemplateAndObj(obj->base.unk56, 1, 0x24, left + 24 + Rand16() % 128, top + 8, 0, 0x1F, 0, 0, NIDL_TYPE,
                                   NIDL_SPECIAL << 4 | 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    star->base.xspeed = 0;
    star->base.yspeed = -STAR_SPEED;
    star->base.parent = obj;
    star->unk7C = sub_0809F840;
    StarFree(star);
}

// Way(); The way (of the six) toward Kirby for an arm;
static u8 Way(struct HeavyMole *m, u8 which) {
    struct Object *obj = &m->n.obj;
    s32 dy = (obj->kirby3->base.y - obj->base.y) >> 8;

    return sReach[dy < -24 ? 0 : dy > 24 ? 2 : 1][which];
}

static void HeavyMoleMain(struct Object *obj) {
    struct HeavyMole *m = (struct HeavyMole *)obj;
    u8 which;

    NidlUpdate(&m->n);
    obj->base.flags |= 4;
    obj->kirby3 = FindTargetKirby(&obj->base);
    // The HP bar goes to whatever is hit; it takes it back;
    sub_08034E14(obj);
    m->anim++;
    NidlFaceKirby(obj);

    // Digging up and down (NiDL's tunnel);
    m->dig += m->digDir;
    if (m->dig > DIG_RANGE * 4 || m->dig < -DIG_RANGE * 4) m->digDir = -m->digDir;
    obj->base.y = m->spotY + (m->dig / 4) * 0x100;
    obj->base.xspeed = obj->base.yspeed = 0;

    if (++m->starTimer >= STAR_TIME) {
        m->starTimer = 0;
        Debris(m);
    }

    // Its arms: one reaches out at Kirby in turn, now and then fired off;
    if (++m->timer >= REACH_TIME) {
        m->timer = 0;
        which = m->nextArm;
        m->nextArm ^= 1;
        if (m->armState[which] == ARM_REST) {
            m->armWay[which] = Way(m, which);
            if (!(Rand16() % 3)) Fire(m, which);
            else m->armState[which] = ARM_REACH;
            PlaySfx(&obj->base, SE_BASIC_ENEMY_JUMP);
        }
    }
    for (which = 0; which < 2; which++) {
        if (m->armState[which] == ARM_REACH && ++m->armTimer[which] > REACH_TIME / 2) {
            m->armTimer[which] = 0;
            m->armState[which] = ARM_REST;
            m->armWay[which] = which ? 5 : 1;
        }
    }
    NidlSetFrame(&m->n, (m->anim >> 2) & 7);
}

// HeavyMoleStart(); The fight starts: its arms;
void HeavyMoleStart(struct Object *obj) {
    struct HeavyMole *m = (struct HeavyMole *)obj;

    ObjectSetFunc(obj, -1, HeavyMoleMain);
    if (m->started) return;
    m->started = TRUE;
    m->spotY = obj->base.y;
    MoleArm(m, 0);
    MoleArm(m, 1);
}

// HeavyMoleInit(); Its boxes (its body), through the dirt (it digs);
void HeavyMoleInit(struct NidlObject *n) {
    struct HeavyMole *m = (struct HeavyMole *)n;
    struct Object *obj = &n->obj;

    m->started = FALSE;
    m->timer = 0;
    m->anim = 0;
    m->dig = 0;
    m->digDir = 1;
    m->nextArm = 0;
    m->starTimer = STAR_TIME - 30;
    m->armTimer[0] = m->armTimer[1] = 0;
    obj->base.flags |= 0x100;
    obj->base.unk5C = (obj->base.unk5C & ~7) | 3 | 0xA0;
    sub_0803E2B0(&obj->base, -24, -20, 24, 20);
    ObjectSetBounds(&obj->base, -8, -8, 8, 8);
}
