/* Star; Squeak Squad's star a mid-boss knocks out (0x0212389C, 0x021236A0: its variants 1 and 2; effect/efc_cmn0's animations 11-13,
 * here its 0-2), as SQUEAK_STAR: harmless, Kirby inhales it (it gives nothing) and spits it back as a star, what he fights a mid-boss
 * with when he has no power. Variant SQUEAK_STAR_AHEAD flies off forward and up, SQUEAK_STAR_BEHIND back and up (3.0 and 3.0: 0x0212344C,
 * 0x02123358) through everything as it appears (its animation 0), then hangs there blinking (animation 1) 100 frames (0x02123268,
 * 0x02123218) and fades (animation 2, 0x02123184, 0x02123160). Variants SQUEAK_STAR_SWORD_AHEAD and _BEHIND (kinds 3 and 4, Meta
 * Knight's: 0x021236A0's brain 0x0203A9C4(16)) as those, a Sword when inhaled. Not ported: Squeak Squad doesn't hold it in the stage
 * (here SqueakMove's hold).
 * Variant SQUEAK_STAR_SPIT (Squeak Squad's kind 0: 0x021235BC, 0x021234F0; efc_cmn0's animation 5, here its 3, spinning) is the star
 * King Dedede spits the Kirby he swallowed out in (squeak/dedede.c, SqueakStarCarry()): 4.0 a frame forward through everything (its
 * flags 0x100, 0x200), the Kirbys inside it moving with it; out of the arena (Squeak Squad's 256 pixels from the area's left: here
 * KatAM's 240 of the camera's view, which the fight holds) he lets them out where it is (0x020E56EC: hurt, thrown back and up; here
 * KatAM's grab script, as the NiDL spit star's: enemies/spit_star.c), and it flies on till it's out of the camera's view. */
#include "../squeak.h"
#include "kirby.h"
#include "constants/kirby.h"

enum { ANIM_APPEAR, ANIM_BLINK, ANIM_FADE, ANIM_SPIT };
enum { STATE_START, STATE_APPEAR, STATE_BLINK, STATE_FADE, STATE_SPIT };

#define SPEED 0x3000
#define BLINK 100
#define SPIT_SPEED 0x4000

#define COUNT(s) ((s)->vars[0])
#define KIRBYS(s) ((s)->vars[1])    // The Kirbys inside the spat star (a bit each);

/* The grab's scripts (bosses/dedede.c's, enemies/spit_star.c's: x ahead, y, priority, frames, Kirby's animation, flags: 0x400 held,
 * 0x40 hidden, 0x10 KatAM's hurt): inside it, hidden; then out back and up, hurt. A Kirby's script restarts when its holder's unk83
 * (its frame) changes; */
static const struct Kirby_110 sInside[] = {
    { 0, 0, -1, 1, 0x4A, 0x440 },
    { 0, 0, 0, 0, 0, 0x500 },
};
static const struct Kirby_110 sOut[] = {
    { -8, 0x30, 0, 1, 0, 0x410 },
    { 0, 0, 0, 0, 0, 0x500 },
};

/* SqueakStarCarry(); The Kirbys a boss swallowed (a bit each: grabbed by it, held hidden) go into the spat star, facing its way; */
void SqueakStarCarry(struct SqueakObject *s, u8 kirbys) {
    u8 i;

    KIRBYS(s) = kirbys;
    for (i = 0; i < gNumKirbys; i++) {
        struct Kirby *kirby = &gKirbys[i];

        if (!(kirbys >> i & 1) || !kirby->unk110) continue;
        kirby->base.unk6C = &s->obj;
        kirby->unk110 = sInside;
        // (a new holder: sub_0804E60C starts the script again when the holder's unk83 isn't the last one it saw);
        kirby->base.counter = ~s->obj.unk83;
        kirby->base.flags = (kirby->base.flags & ~1) | (s->obj.base.flags & 1);
    }
}

// Release(); 0x020E56EC: the Kirbys out of the spat star, hurt (their scripts restart as its frame changes);
static void Release(struct SqueakObject *s) {
    u8 i;

    for (i = 0; i < gNumKirbys; i++)
        if ((KIRBYS(s) >> i & 1) && gKirbys[i].unk110 == sInside && gKirbys[i].base.unk6C == &s->obj) gKirbys[i].unk110 = sOut;
    KIRBYS(s) = 0;
}

// ViewX(); Its spot across the camera's view (pixels from its left);
static s32 ViewX(struct SqueakObject *s) {
    return (s->obj.base.x - gCurLevelInfo[s->obj.base.unk56].viewportPosition.x) >> 8;
}

// InView(); 0x02209D00's margins 1 (24 pixels above, 56 below and either side): its spot in the camera's view;
static bool32 InView(struct SqueakObject *s) {
    struct LevelInfo *level = &gCurLevelInfo[s->obj.base.unk56];
    s32 x = s->obj.base.x - level->viewportPosition.x - (DISPLAY_WIDTH / 2 << 8);
    s32 y = s->obj.base.y - level->viewportPosition.y - (DISPLAY_HEIGHT / 2 << 8);

    return x > -((DISPLAY_WIDTH / 2 + 56) << 8) && x < (DISPLAY_WIDTH / 2 + 56) << 8 && y > -((DISPLAY_HEIGHT / 2 + 24) << 8)
           && y < (DISPLAY_HEIGHT / 2 + 56) << 8;
}

// MoveFree(); The spat star's move: through everything and not held in the stage (its flag 0x20000), so it leaves the arena;
static void MoveFree(struct SqueakObject *s) {
    struct ObjectBase *base = &s->obj.base;

    base->xspeed = SQUEAK_SPEED(base->flags & 1 ? -s->vx : s->vx);
    base->yspeed = SQUEAK_SPEED(s->vy);
    base->unk48 = base->x;
    base->unk4C = base->y;
    base->x += base->xspeed;
    base->y -= base->yspeed;
    base->unk62 = 0;
    s->terrain = 0;
    s->moved = TRUE;
}

static bool32 Star(struct SqueakObject *s) {
    switch (s->state) {
    case STATE_START:
        if (s->variant == SQUEAK_STAR_SPIT) {   // 0x021235BC;
            s->vx = SPIT_SPEED;
            s->vy = 0;
            SqueakPlay(s, ANIM_SPIT);
            SqueakSetState(s, STATE_SPIT);
            return TRUE;
        }
        s->vx = s->variant == SQUEAK_STAR_BEHIND || s->variant == SQUEAK_STAR_SWORD_BEHIND ? -SPEED : SPEED;
        s->vy = SPEED;
        SqueakPlay(s, ANIM_APPEAR);
        SqueakSetState(s, STATE_APPEAR);
        return TRUE;
    case STATE_APPEAR:
        if (s->animEnded) {
            COUNT(s) = BLINK;
            SqueakStop(s);
            SqueakPlay(s, ANIM_BLINK);
            SqueakSetState(s, STATE_BLINK);
            return TRUE;
        }
        SqueakMove(s);
        break;
    case STATE_BLINK:
        if (COUNT(s) < 1) {
            SqueakPlay(s, ANIM_FADE);
            SqueakSetState(s, STATE_FADE);
            return TRUE;
        }
        COUNT(s)--;
        SqueakMove(s);
        break;
    case STATE_FADE:
        if (s->animEnded) s->obj.base.flags |= 0x1000;
        SqueakMove(s);
        break;
    case STATE_SPIT:        // 0x021234F0: out of the arena, the Kirbys out; out of the view, gone;
        if (KIRBYS(s) && (ViewX(s) < 0 || ViewX(s) > DISPLAY_WIDTH)) Release(s);
        if (!InView(s)) {
            Release(s);
            s->obj.base.flags |= 0x1000;
            break;
        }
        MoveFree(s);
        break;
    }
    return FALSE;
}

static void SqueakStarMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    SqueakRun(s, Star);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
}

// SqueakStarStart(); Its start (and after a hit: it goes on), through everything, harmless; the spat star untouchable;
void SqueakStarStart(struct Object *obj) {
    ObjectSetFunc(obj, -1, SqueakStarMain);
    // (kinds 3 and 4: a Sword, 0x0203A9C4's 16; the others nothing, 0x0203A82C's 0);
    if (((struct SqueakObject *)obj)->variant >= SQUEAK_STAR_SWORD_AHEAD) obj->kirbyAbility = KIRBY_ABILITY_SWORD;
    SqueakThroughTerrain((struct SqueakObject *)obj, TRUE);
    obj->base.unk68 = 0;
    if (((struct SqueakObject *)obj)->variant == SQUEAK_STAR_SPIT) obj->base.flags |= 0x200;
    if (!((struct SqueakObject *)obj)->started) SqueakSetState((struct SqueakObject *)obj, STATE_START);
}

/* SqueakStarInit(); Its box (8 pixels round its middle); Kirby's body doesn't hurt it, nor shows its HP bar (0x080365C8's flag
 * 0x1000000), as Doc's and Spinni's things: Squeak Squad's attack things go through him; */
void SqueakStarInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -8, -8, 8, 8);
    ObjectSetBounds(&s->obj.base, -8, -8, 8, 8);
    s->obj.base.unk5C |= 0x80;
    s->obj.base.flags |= 0x1000000;
}
