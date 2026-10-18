/* Spit star; the star King Dedede spits the Kirbys he swallowed out in (bosses/dedede.c), ported from Kirby: Nightmare in Dream Land
 * (USA), object type 0x70 (spawn 0x080A498C, states 0x087489B4), as NIDL_SPIT_STAR: no room has them, Dedede makes them (behavior
 * NIDL_SPECIAL << 4 | 3). Its frames are NiDL's shared effects' (0x0874C44C: 4 the star, 35-37 its burst; 45 the sparkles of
 * Dedede's inhale, bosses/dedede.c draws them), numbered from 0 here.
 * NiDL's (state 1, 0x080A4AC4): it flies on at 4 pixels a frame along the floor with Kirby hidden inside, until a wall stops it; then
 * it lets him out (0x080A49CC), hurt (1 HP), bouncing up and back off the wall, and bursts. Here the Kirbys ride in it through KatAM's
 * grab (it holds them hidden, as Dedede did: SpitStarCarry()), thrown out back and up (sub_0804EA18, KatAM's hurt: 1 HP); it can't be
 * touched; */
#include "../nidl.h"
#include "kirby.h"
#include "constants/songs.h"

#define SPEED 0x400
#define BURST_TIME 6

enum { FRAME_STAR, FRAME_BURST };

struct SpitStar {
    struct NidlObject n;
    u8 kirbys;      // The Kirbys inside it (a bit each);
    u8 burst;       // Frames since it hit the wall (0: still flying);
    bool8 left;
};

typedef char sSpitStarFits[sizeof(struct SpitStar) <= NIDL_OBJECT_SIZE ? 1 : -1];

/* The grab's scripts (see bosses/dedede.c): inside it, hidden; then out back and up (x * 0x10 ahead of where they face, the star's
 * way, y * 0x10 up, hurt). A Kirby's script restarts when its holder's unk83 (its frame) changes: the star's changes as it bursts; */
static const struct Kirby_110 sInside[] = {
    { 0, 0, -1, 1, 0x4A, 0x440 },
    { 0, 0, 0, 0, 0, 0x500 },
};
static const struct Kirby_110 sOut[] = {
    { -8, 0x30, 0, 1, 0, 0x410 },
    { 0, 0, 0, 0, 0, 0x500 },
};

// SpitStarCarry(); The Kirbys Dedede swallowed (a bit each) go into the star, which flies off their way;
void SpitStarCarry(struct Object *obj, u8 kirbys, bool8 left) {
    struct SpitStar *s = (struct SpitStar *)obj;
    u8 i;

    s->kirbys = kirbys;
    s->left = left;
    if (left) obj->base.flags |= 1;
    else obj->base.flags &= ~1;
    for (i = 0; i < gNumKirbys; i++) {
        struct Kirby *kirby = &gKirbys[i];

        if (!(kirbys >> i & 1) || !kirby->unk110) continue;
        kirby->base.unk6C = obj;
        kirby->unk110 = sInside;
        // (a new holder: sub_0804E60C starts the script again when the holder's unk83 isn't the last one it saw);
        kirby->base.counter = ~obj->unk83;
        if (left) kirby->base.flags |= 1;
        else kirby->base.flags &= ~1;
    }
}

// Release(); The Kirbys out of it, off the wall (their scripts restart as its frame changes);
static void Release(struct SpitStar *s) {
    struct Object *obj = &s->n.obj;
    u8 i;

    for (i = 0; i < gNumKirbys; i++)
        if ((s->kirbys >> i & 1) && gKirbys[i].unk110 == sInside && gKirbys[i].base.unk6C == obj) gKirbys[i].unk110 = sOut;
    PlaySfx(&obj->base, SE_KIRBY_STAR_DESTROY);
}

static void SpitStarMain(struct Object *obj) {
    struct SpitStar *s = (struct SpitStar *)obj;
    s32 x = (obj->base.x >> 8) - (gCurLevelInfo[obj->base.unk56].viewportPosition.x >> 8);

    NidlUpdate(&s->n);
    obj->base.flags |= 4;
    obj->kirby3 = FindTargetKirby(&obj->base);
    obj->base.yspeed = 0;
    if (!s->burst) {
        obj->base.xspeed = s->left ? -SPEED : SPEED;
        // (a wall, or the side of the screen: the arena holds the camera on Dedede's);
        if ((obj->base.unk62 & 3) || (s->left && x < 12) || (!s->left && x > 228)) {
            obj->base.xspeed = 0;
            s->burst = 1;
            Release(s);
            NidlSetFrame(&s->n, FRAME_BURST);
            return;
        }
        NidlSetFrame(&s->n, FRAME_STAR);
        return;
    }
    obj->base.xspeed = 0;
    if (++s->burst > 3 * BURST_TIME) {
        obj->base.flags |= 0x1000;
        return;
    }
    NidlSetFrame(&s->n, FRAME_BURST + (s->burst - 1) / BURST_TIME);
}

// SpitStarStart(); Behavior start;
void SpitStarStart(struct Object *obj) {
    ObjectSetFunc(obj, -1, SpitStarMain);
}

// SpitStarInit(); Its boxes (16x16, on the floor), untouchable (as a boss before its fight);
void SpitStarInit(struct NidlObject *n) {
    struct SpitStar *s = (struct SpitStar *)n;
    struct Object *obj = &n->obj;

    s->kirbys = 0;
    s->burst = 0;
    s->left = FALSE;
    obj->base.unkC |= 1;
    obj->base.flags |= 0x2000000 | 0x200 | 0x200000;
    sub_0803E2B0(&obj->base, -8, -8, 8, 8);
    ObjectSetBounds(&obj->base, -8, -8, 8, 8);
}
