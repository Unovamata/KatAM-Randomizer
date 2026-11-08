/* Star; Squeak Squad's star a mid-boss knocks out (0x0212389C, 0x021236A0: its variants 1 and 2; effect/efc_cmn0's animations 11-13,
 * here its 0-2), as SQUEAK_STAR: harmless, Kirby inhales it (it gives nothing) and spits it back as a star, what he fights a mid-boss
 * with when he has no power. Variant SQUEAK_STAR_AHEAD flies off forward and up, SQUEAK_STAR_BEHIND back and up (3.0 and 3.0: 0x0212344C,
 * 0x02123358) through everything as it appears (its animation 0), then hangs there blinking (animation 1) 100 frames (0x02123268,
 * 0x02123218) and fades (animation 2, 0x02123184, 0x02123160). Not ported: Squeak Squad doesn't hold it in the stage (here
 * SqueakMove's hold); */
#include "../squeak.h"

enum { ANIM_APPEAR, ANIM_BLINK, ANIM_FADE };
enum { STATE_START, STATE_APPEAR, STATE_BLINK, STATE_FADE };

#define SPEED 0x3000
#define BLINK 100

#define COUNT(s) ((s)->vars[0])

static bool32 Star(struct SqueakObject *s) {
    switch (s->state) {
    case STATE_START:
        s->vx = s->variant == SQUEAK_STAR_BEHIND ? -SPEED : SPEED;
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
    }
    return FALSE;
}

static void SqueakStarMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    SqueakRun(s, Star);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
}

// SqueakStarStart(); Its start (and after a hit: it goes on), through everything, harmless;
void SqueakStarStart(struct Object *obj) {
    ObjectSetFunc(obj, -1, SqueakStarMain);
    SqueakThroughTerrain((struct SqueakObject *)obj, TRUE);
    obj->base.unk68 = 0;
    if (!((struct SqueakObject *)obj)->started) SqueakSetState((struct SqueakObject *)obj, STATE_START);
}

// SqueakStarInit(); Its box (8 pixels round its middle);
void SqueakStarInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -8, -8, 8, 8);
    ObjectSetBounds(&s->obj.base, -8, -8, 8, 8);
}
