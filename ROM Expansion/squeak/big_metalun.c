/* Big Metalun; ported from Kirby: Squeak Squad (USA), mid-boss 0x34 (its body 0x0210C4F8, brain 0x0210A940), as SQUEAK_BIG_METALUN, a
 * mid-boss on mini-boss type 0x41 (squeak.h's SQUEAK_MIDBOSS). Values from param/prmMB10Ml0.kgb (movement), prmMB10Pl0.kgb (brain)
 * and prmMB10Sl0.kgb (HP 300); speeds 20.12 a frame, Y up; its box from enemy/bigmetalum.rca (one for all its frames).
 * (In progress: its body's states are being ported; it stands for now.) */
#include "../squeak.h"

// Animations (bigmetalum.cea);
enum { ANIM_STILL, ANIM_STAND };

static void BigMetalunMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    SqueakMidbossUpdate(s);
    obj->base.flags |= 4;
    SqueakAnimate(s);
}

void BigMetalunStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, -1, BigMetalunMain);
    SqueakPlay(s, ANIM_STAND);
}

/* BigMetalunInit(); Its box (bigmetalum.rca's: 19 pixels behind to 23 ahead, up to 39 up), its bounds (KatAM's), facing left,
 * standing; */
void BigMetalunInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -19, -39, 23, 0);
    ObjectSetBounds(&s->obj.base, -16, -36, 16, 0);
    s->obj.base.flags |= 1;
    SqueakStandOnFloor(&s->obj);
    SqueakPlay(s, ANIM_STAND);
}
