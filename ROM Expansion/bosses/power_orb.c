/* Nightmare's Power Orb; ported from Kirby: Nightmare in Dream Land (USA), boss object type 0x42 (spawn 0x080AE3BC), as NiDL boss
 * NIDL_POWER_ORB, the Fountain of Dreams' first (room 0x003 here: the fall through space). NiDL fights it in a shooter: Kirby rides
 * the Warp Star as the screen scrolls, the orb bounces around and fires stars; KatAM's Kirby can't ride it, so the fight is on the
 * room's floor, at its bottom, the orb bouncing around the sky above Kirby. Its frames are plain ones (0x087544B4's 0-7, spinning,
 * mini-boss descriptor 37). It fires stars at Kirby now and then (enemies/star.c: Kirby inhales them and spits them back at it, KatAM's
 * bosses take only the stronger attacks), three at once every third time. It flies through anything. Touching it hurts; */
#include "../nidl.h"
#include "constants/songs.h"

#define SPEED_X 0x180
#define SPEED_Y 0x100
#define SHOT_TIME 80
#define STAR_SPEED 0x200
// Where it bounces (screen pixels: the sky above Kirby);
#define LEFT 24
#define RIGHT 216
#define TOP 20
#define BOTTOM 96

struct PowerOrb {
    struct NidlObject n;
    u16 timer;
    u8 anim;
    u8 volley;
};

typedef char sPowerOrbFits[sizeof(struct PowerOrb) <= NIDL_OBJECT_SIZE ? 1 : -1];

// Fire(); A star at Kirby, turned by `turn` (a quarter of its speed sideways per step), gone with it (sub_0809F840);
static void Fire(struct PowerOrb *o, s32 turn) {
    struct Object *obj = &o->n.obj, *star;
    s32 dx = (obj->kirby3->base.x - obj->base.x) >> 8, dy = (obj->kirby3->base.y - obj->base.y) >> 8, d, xs, ys;

    d = (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy);
    if (d < 1) d = 1;
    xs = dx * STAR_SPEED / d;
    ys = -dy * STAR_SPEED / d;
    star = CreateObjTemplateAndObj(obj->base.unk56, 1, 0x24, obj->base.x >> 8, obj->base.y >> 8, 0, 0x1F, 0, 0, NIDL_TYPE,
                                   NIDL_SPECIAL << 4 | 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    star->base.xspeed = xs - ys * turn / 4;
    star->base.yspeed = ys + xs * turn / 4;
    star->base.parent = obj;
    star->unk7C = sub_0809F840;
}

static void PowerOrbMain(struct Object *obj) {
    struct PowerOrb *o = (struct PowerOrb *)obj;
    struct LevelInfo *level = &gCurLevelInfo[obj->base.unk56];
    s32 x = (obj->base.x - level->viewportPosition.x) >> 8, y = (obj->base.y - level->viewportPosition.y) >> 8;

    NidlUpdate(&o->n);
    obj->base.flags |= 4;
    obj->kirby3 = FindTargetKirby(&obj->base);
    // The HP bar goes to whatever is hit (its stars too); it takes it back;
    sub_08034E14(obj);
    o->anim++;

    // Bouncing off the sky's sides;
    if ((x <= LEFT && obj->base.xspeed < 0) || (x >= RIGHT && obj->base.xspeed > 0)) obj->base.xspeed = -obj->base.xspeed;
    if ((y <= TOP && obj->base.yspeed > 0) || (y >= BOTTOM && obj->base.yspeed < 0)) obj->base.yspeed = -obj->base.yspeed;

    if (++o->timer >= SHOT_TIME) {
        o->timer = 0;
        Fire(o, 0);
        if (++o->volley >= 3) {
            o->volley = 0;
            Fire(o, 1);
            Fire(o, -1);
        }
        PlaySfx(&obj->base, SE_KIRBY_AIRSHOT);
    }
    NidlSetFrame(&o->n, (o->anim >> 2) & 7);
}

// PowerOrbStart(); The fight starts: off it goes;
void PowerOrbStart(struct Object *obj) {
    ObjectSetFunc(obj, -1, PowerOrbMain);
    if (!obj->base.xspeed && !obj->base.yspeed) {
        obj->base.xspeed = -SPEED_X;
        obj->base.yspeed = SPEED_Y;
    }
}

// PowerOrbInit(); Its boxes (32x32, through anything);
void PowerOrbInit(struct NidlObject *n) {
    struct PowerOrb *o = (struct PowerOrb *)n;
    struct Object *obj = &n->obj;

    o->timer = SHOT_TIME / 2;
    o->anim = 0;
    o->volley = 0;
    obj->base.xspeed = obj->base.yspeed = 0;
    obj->base.flags |= 0x100;
    obj->base.unk5C = (obj->base.unk5C & ~7) | 3 | 0xA0;
    sub_0803E2B0(&obj->base, -16, -16, 16, 16);
    ObjectSetBounds(&obj->base, -16, -16, 16, 16);
}
