/* Daroach (in a stage); ported from Kirby: Squeak Squad (USA), the Squeak Squad's leader in a stage's scene, enemy 0x45 (its body
 * 0x02134DA8, vtable 0x02184B14: +0x3C its start 0x02134C74, +0x34 per frame 0x02134ECC, +0x2C and +0x30 its hit handlers
 * 0x02134D38 and 0x02134D34: nothing), as SQUEAK_DAROACH. Values from param/prmDB04Ml0.kgb (60, 20, 60, -2.0, 2.0, 0, 0); speeds
 * 20.12 a frame, Y up. Drawn with enemy/demo_dancho (animations 1-5; its .rca has no boxes: nothing hits him, his touch hurts
 * nobody), made by 0x020A51A8 (not the enemy factory's table). His boss fight (enemy 0x3D, dancho00, dancho01, prmBB07) is another
 * character.
 * • His scene (recorded on the DS): he takes Kirby's controls (0x02012F94: here KatAM's cutscene flag, Kirby's 0x1000000, which
 *   zeroes his input) and hovers 60 frames (animation 1, bobbing), swirls his cape (2), holds it 20 frames (3) and points (4); then
 *   he gives Kirby his controls back (0x02012F68) and flies off backward and up (2.0 and 2.0 a frame, animation 5) through
 *   everything and out of the stage (flags 0x100, 0x20000), 60 frames or till he's out of the camera's view (0x02209D00's margins 1:
 *   56 pixels either side and below, 24 above), gone for the rest of the visit (0x0220A2B4: his spawn record's state 4; here the
 *   room's spawned bit left set, as KatAM's room load clears it).
 * • Facing: Squeak Squad's spawn byte 9 (0: left, its flag 4; else right); here the variant's bit 0.
 * Not ported: the stage's mode 3 (0x02209560's +0x75: Kirby keeps his controls, and at the end 0x0220A2A4 restarts the stage's
 * first spawn record), which KatAM has no counterpart for. */
#include "../squeak.h"

// Animations (demo_dancho.cea's 1-5: KatAMExpansionSqueak.cs's OnlyAnims);
enum { ANIM_HOVER, ANIM_SWIRL, ANIM_HOLD, ANIM_POINT, ANIM_FLY };

enum { STATE_START, STATE_HOVER, STATE_SWIRL, STATE_HOLD, STATE_POINT, STATE_FLY_ENTRY, STATE_FLY };

// prmDB04Ml0.kgb (+0x234);
#define HOVER_TIME 60           // +0x00;
#define HOLD_TIME 20            // +0x04;
#define FLY_TIME 60             // +0x08;
#define FLY_X (-0x2000)         // +0x0C..+0x18: its flight (backward, up; no acceleration);
#define FLY_Y 0x2000
#define FLY_AX 0
#define FLY_AY 0

#define COUNT(s) ((s)->vars[0])
#define LOCKED(s) ((s)->vars[1])    // The Kirbys whose controls it took (a bit each);

// Lock(); 0x02012F94: every Kirby in its room (and up) without controls (the cutscenes' flag, unless one already has it);
static void Lock(struct SqueakObject *s) {
    u8 i;

    LOCKED(s) = 0;
    for (i = 0; i < 4; i++) {
        struct Kirby *kirby = &gKirbys[i];

        if (kirby->base.roomId != s->obj.base.roomId || kirby->hp <= 0 || (kirby->base.flags & 0x1000000)) continue;
        kirby->base.flags |= 0x1000000;
        LOCKED(s) |= 1 << i;
    }
}

// Unlock(); 0x02012F68: the controls it took given back;
static void Unlock(struct SqueakObject *s) {
    u8 i;

    for (i = 0; i < 4; i++)
        if (LOCKED(s) >> i & 1) gKirbys[i].base.flags &= ~0x1000000;
    LOCKED(s) = 0;
}

/* MoveFree(); 0x02013D18 with its flags 0x100 and 0x20000: through everything (the terrain's not checked) and not held in the
 * stage; */
static void MoveFree(struct SqueakObject *s) {
    struct ObjectBase *base = &s->obj.base;

    s->vx += s->ax;
    s->vy += s->ay;
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

// InView(); 0x02209D00's margins 1 (0x022275F4's: 24 pixels above, 56 below and either side): its spot in the camera's view;
static bool32 InView(struct SqueakObject *s) {
    struct LevelInfo *level = &gCurLevelInfo[s->obj.base.unk56];
    s32 x = s->obj.base.x - level->viewportPosition.x - (DISPLAY_WIDTH / 2 << 8);
    s32 y = s->obj.base.y - level->viewportPosition.y - (DISPLAY_HEIGHT / 2 << 8);

    return x > -((DISPLAY_WIDTH / 2 + 56) << 8) && x < (DISPLAY_WIDTH / 2 + 56) << 8 && y > -((DISPLAY_HEIGHT / 2 + 24) << 8)
           && y < (DISPLAY_HEIGHT / 2 + 56) << 8;
}

/* Gone(); 0x0220A2B4(..., 2): gone for the rest of the visit: its template a copy out of the room's list (SqueakSpawn's room slot
 * none), so KatAM's object end (ObjectDestroy) leaves the room's spawned bit set and it doesn't come back till the room is loaded
 * again; */
static void Gone(struct SqueakObject *s) {
    Unlock(s);
    if (s->obj.objTemplate != NULL && s->obj.objTemplate != &s->spawn) {
        s->spawn = *s->obj.objTemplate;
        s->spawn.unk2 = 0;
        s->spawn.unk3 = 31;
        s->obj.objTemplate = &s->spawn;
    }
    s->obj.base.flags |= 0x1000;
}

static bool32 Scene(struct SqueakObject *s) {
    switch (s->state) {
    case STATE_START:       // 0x02134C74: Kirby's controls taken, hovering;
        Lock(s);
        COUNT(s) = HOVER_TIME;
        SqueakPlay(s, ANIM_HOVER);
        SqueakSetState(s, STATE_HOVER);
        return TRUE;
    case STATE_HOVER:       // 0x02134C24: 60 frames, then the cape (0x02134B94);
        if (COUNT(s)-- < 1) {
            SqueakPlay(s, ANIM_SWIRL);
            SqueakSetState(s, STATE_SWIRL);
            return TRUE;
        }
        MoveFree(s);
        break;
    case STATE_SWIRL:       // 0x02134ADC: its animation through, held 20 frames;
        if (s->animEnded) {
            COUNT(s) = HOLD_TIME;
            SqueakPlay(s, ANIM_HOLD);
            SqueakSetState(s, STATE_HOLD);
            return TRUE;
        }
        MoveFree(s);
        break;
    case STATE_HOLD:        // 0x02134A30: then pointing;
        if (COUNT(s)-- < 1) {
            SqueakPlay(s, ANIM_POINT);
            SqueakSetState(s, STATE_POINT);
            return TRUE;
        }
        MoveFree(s);
        break;
    case STATE_POINT:       // 0x021349E8: its animation through, off;
        if (s->animEnded) {
            SqueakSetState(s, STATE_FLY_ENTRY);
            return TRUE;
        }
        MoveFree(s);
        break;
    case STATE_FLY_ENTRY:   // 0x021348F8: Kirby's controls back, flying off backward and up;
        Unlock(s);
        COUNT(s) = FLY_TIME;
        SqueakPlay(s, ANIM_FLY);
        s->vx = FLY_X;
        s->vy = FLY_Y;
        s->ax = FLY_AX;
        s->ay = FLY_AY;
        SqueakSetState(s, STATE_FLY);
        return TRUE;
    case STATE_FLY:         // 0x0213488C: 60 frames, or out of the camera's view: gone;
        if (COUNT(s)-- < 1 || !InView(s)) {
            Gone(s);
            break;
        }
        MoveFree(s);
        break;
    }
    return FALSE;
}

static void DaroachMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    SqueakRun(s, Scene);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
    obj->base.flags |= 0x200;
}

// DaroachGone(); Its end (squeak.c's SqueakDestroy: gone, or its room left mid-scene): the controls it took given back;
void DaroachGone(struct SqueakObject *s) {
    Unlock(s);
}

void DaroachStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, 0, DaroachMain);
    if (!s->started) SqueakSetState(s, STATE_START);
}

/* DaroachInit(); Facing as its spawn byte 9 (0x02134DA8: 0, left, its flag 4), here the variant's bit 0; no boxes (demo_dancho.rca
 * has none): untouchable (KatAM's flag 0x200), Kirby's body and inhale take nothing from it (unk5C 0x80, 0x20), its touch hurts
 * nobody; */
void DaroachInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -12, -24, 12, 0);
    ObjectSetBounds(&s->obj.base, -12, -24, 12, 0);
    if (!(s->variant & 1)) s->obj.base.flags |= 1;
    s->obj.base.flags |= 0x200;
    s->obj.base.unk5C |= 0x20 | 0x80;
    s->obj.base.unk68 = 0;
}
