/* Bubbles; ported from Kirby: Nightmare in Dream Land (USA), object type 0x1E (spawn 0x08089180), as NiDL enemy NIDL_BUBBLES.
 * NiDL scripts (16.16 speeds, Y down; KatAM's are 8.8, Y up):
 * • Variant 0, 0x0808924C: bounces forward at 1.0, each bounce -3.0, -4.0 or -5.0 at random (table 0x0874276C, gravity 1/8,
 *   falling at 3.0 at most), spinning (16 steps, 0x08089808 and its table 0x08742778) every 4, 8 or never, by the bounce.
 *   Landing it squashes (0x08089544: frame 18 for 13 frames, then 16 or 14 for 3), against a wall it squashes and turns around
 *   (0x08089334: 21 for 12, then 20 or 21 for 3), against a ceiling it squashes too (0x08089460: 17, then 15 or 13).
 * • Variant 1, 0x08089680: bounces in place (frames 4 for 20, 14 for 2, 18 for 4, 14 for 2); */
#include "../nidl.h"

// Frames: NiDL's 4 to 21, numbered from 0 (NiDL frame - 4). Its hurt frame is NiDL's 0;
#define FRAME(nidl) ((nidl) - 4)

// 0x08742778: frame and flip of each of the 16 spinning steps;
static const u8 sSpin[16][2] = {
    { 8, 0 }, { 9, 0 }, { 10, 0 }, { 11, 0 }, { 12, 0 }, { 11, 1 }, { 10, 1 }, { 9, 1 },
    { 8, 1 }, { 7, 1 }, { 6, 1 }, { 5, 1 }, { 4, 0 }, { 5, 0 }, { 6, 0 }, { 7, 0 },
};

static const s16 sBounceSpeed[] = { 0x300, 0x400, 0x500 };   // 0x0874276C;
static const u8 sSpinTime[] = { 4, 8, 0 };

// Variant 1 (0x08089680);
static const s16 sInPlace[] = { FRAME(4), 20, FRAME(14), 2, FRAME(18), 4, FRAME(14), 2, NIDL_LOOP };

#define SPEED 0x100
#define GRAVITY 0x20
#define MAX_FALL 0x300

enum { STATE_BOUNCE, STATE_SQUASH, STATE_IN_PLACE };

struct Bubbles {
    struct NidlObject n;
    s16 xspeed;         // Kept while squashed;
    s16 yspeed;
    u8 state;
    u8 spin;            // Spinning step (0-15);
    u8 spinTime;
    u8 spinTimer;
    u8 squashTimer;
    u8 squashFrames[2]; // Squashed, then about to leave;
    u8 airborne;
    u8 wallTimer;       // Frames before walls count again, after turning around;
};

// ShowSpin(); 0x08089808: the spinning step's frame and flip (NiDL's flip = facing left, as KatAM's sprites do unflipped);
static void ShowSpin(struct Bubbles *b) {
    NidlSetFrame(&b->n, FRAME(sSpin[b->spin][0]));
    if (sSpin[b->spin][1])
        b->n.obj.base.flags |= 1;
    else
        b->n.obj.base.flags &= ~1;
}

// Bounce(); 0x0808924C: a new bounce, at random;
static void Bounce(struct Bubbles *b) {
    u8 height = Rand16() % 3;

    b->state = STATE_BOUNCE;
    b->n.obj.base.yspeed = sBounceSpeed[height];
    b->spinTime = sSpinTime[height];
    b->spinTimer = b->spinTime;
    b->airborne = TRUE;
    ShowSpin(b);
}

// Squash(); Stops and shows a squashed frame, then another one for 3 frames, by the spinning step;
static void Squash(struct Bubbles *b, u8 squashed, u8 time, u8 front, u8 back) {
    struct Object *obj = &b->n.obj;

    b->xspeed = obj->base.xspeed;
    b->yspeed = obj->base.yspeed;
    obj->base.xspeed = obj->base.yspeed = 0;
    b->state = STATE_SQUASH;
    b->squashFrames[0] = squashed;
    b->squashFrames[1] = b->spin <= 7 ? front : back;
    b->squashTimer = time + 3;
    NidlSetFrame(&b->n, FRAME(squashed));
}

static void Spin(struct Bubbles *b) {
    if (!b->spinTime || --b->spinTimer) return;
    b->spinTimer = b->spinTime;
    b->spin = (b->spin + (b->xspeed > 0 ? 1 : 15)) & 15;
    ShowSpin(b);
}

static void BubblesMain(struct Object *obj) {
    struct Bubbles *b = (struct Bubbles *)obj;

    NidlUpdate(&b->n);
    NidlAnimate(&b->n);
    obj->base.flags |= 4;

    if (b->wallTimer) b->wallTimer--;

    switch (b->state) {
    case STATE_BOUNCE:
        b->xspeed = obj->base.xspeed;
        obj->base.yspeed -= GRAVITY;
        if (obj->base.yspeed < -MAX_FALL) obj->base.yspeed = -MAX_FALL;
        if (!(obj->base.unk62 & 4)) b->airborne = TRUE;

        if ((obj->base.unk62 & 4) && obj->base.yspeed <= 0 && b->airborne) {
            // Landing (0x08089544): it bounces again after;
            Squash(b, 18, 13, 16, 14);
            b->yspeed = 0;
            b->airborne = FALSE;
        } else if ((obj->base.unk62 & 3) && !b->wallTimer) {
            // A wall (0x08089334): turns around;
            Squash(b, 21, 12, 20, 21);
            b->xspeed = -b->xspeed;
            b->wallTimer = 16;
        } else if ((obj->base.unk62 & 8) && obj->base.yspeed > 0) {
            // A ceiling (0x08089460);
            Squash(b, 17, 12, 15, 13);
            b->yspeed = 0;
        } else {
            Spin(b);
        }
        break;
    case STATE_SQUASH:
        if (--b->squashTimer == 3) NidlSetFrame(&b->n, FRAME(b->squashFrames[1]));
        if (b->squashTimer) break;
        obj->base.xspeed = b->xspeed;
        obj->base.yspeed = b->yspeed;
        if (b->yspeed == 0 && (obj->base.unk62 & 4))
            Bounce(b);
        else {
            b->state = STATE_BOUNCE;
            ShowSpin(b);
        }
        break;
    }
}

// BubblesStart(); Behavior start, also called by the engine after Bubbles recovers from a hit;
void BubblesStart(struct Object *obj) {
    struct Bubbles *b = (struct Bubbles *)obj;

    ObjectSetFunc(obj, 0, BubblesMain);
    NidlFaceKirby(obj);

    if (b->n.variant == 1) {
        b->state = STATE_IN_PLACE;
        obj->base.xspeed = obj->base.yspeed = 0;
        NidlPlay(&b->n, sInPlace);
        return;
    }
    b->n.sequence = NULL;
    obj->base.xspeed = (obj->base.flags & 1) ? -SPEED : SPEED;
    b->xspeed = obj->base.xspeed;
    Bounce(b);
}

// BubblesInit(); Its box, on the floor;
void BubblesInit(struct NidlObject *n) {
    struct Bubbles *b = (struct Bubbles *)n;
    struct Object *obj = &n->obj;

    sub_0803E2B0(&obj->base, -6, -6, 6, 7);
    ObjectSetBounds(&obj->base, -7, -7, 7, 8);
    // Water defeats variant 0 like KatAM's enemies, but not variant 1 (0x0808976C);
    if (n->variant == 1) NIDL_SWIMS(obj);
    obj->base.unk4C = obj->base.y = ((obj->base.y + (obj->base.unk3F * 0x100)) & 0xfffff000) - (obj->base.unk3F * 0x100) - 1;
    b->spin = 12;
}
