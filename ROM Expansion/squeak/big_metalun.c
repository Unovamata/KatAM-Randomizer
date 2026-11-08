/* Big Metalun; ported from Kirby: Squeak Squad (USA), mid-boss 0x34 (its body 0x0210C4F8, vtable 0x02180FD8; brain 0x0210A940), as
 * SQUEAK_BIG_METALUN, a mid-boss on mini-boss type 0x41 (squeak.h's SQUEAK_MIDBOSS). Values from param/prmMB10Ml0.kgb (movement:
 * 0x667, gravity 0x2B9, fall 0x399A, 60, the daze 180, -0x1000, 0x2667, 0x52), prmMB10Pl0.kgb (brain) and prmMB10Sl0.kgb (HP 300);
 * speeds 20.12 a frame, Y up; its box from enemy/bigmetalum.rca (one for all its frames). A big metal walker: it stomps along and
 * curls into a ball that rolls across the arena till it bonks on its edge.
 * • Its fight starts as a mini-boss's (the framework's wait, camera lock, HP bar and music); then, as Squeak Squad's once its stage
 *   lets it go (0x0210BD54: its music, 0x0210BCE0), it waits 75 frames, untouchable (+0x218, flag 2), falling if in the air; its
 *   brain runs meanwhile, as Squeak Squad's does once the mid-boss is on the screen (0x0210C244).
 * • Brain (Squeak Squad's random 0-99 for each pick; one state a frame): it faces Kirby (0x0210A868) and rests 60, 120 or 180 frames
 *   (30, 50, 20 in 100: 0x0210A7F8, 0x0210A7D0), walks 1, 2 or 4 steps (30, 50, 20: 0x0210A754; its `else if < 80` never picks 3),
 *   turning at walls (0x0210A708), and once they're done and its body stands, presses B (0x0210A6F0, 0x0210A6D0) and waits up to 180
 *   frames or a wall (0x0210A688); its body standing again, it turns and faces Kirby (0x0210A650: it turns round, then faces him, so
 *   with Kirby ahead it turns away), and 30 in 100 charges again after 60 frames (0x0210A5D0, 0x0210A5A8), else starts over.
 * • Body: it stands (anim 1: 0x0210BC40, 0x0210BB4C), B charging, forward walking, back turning it round; off the ground it falls
 *   (anim 7, gravity 0x2B9, no faster than 3.6: 0x0210B7F0, 0x0210B718; Squeak Squad's air control is the other mode's only), lands
 *   (anim 7 again, 0x0210B6B8) and stands. Its walk (anim 2: 0x0210BA88, 0x0210B8F0) moves it only on its steps' frames, 142 a step:
 *   1 pixel on frames 0 and 8, 3 on 20, 2 on 30, a stomp on 38 (the screen shaking 12 frames: KatAM's 14-frame shake, 1), 2 on 79,
 *   91, 101 and 109 (with a stomp on 109); its steps counted at 142. Its charge (0x0210B5F8: B): it stops (anim 9), sparkles (anim 8),
 *   curls up (anim 3), sits still as a ball (anim 4) 30, 60 or 120 frames (20, 60, 20 in 100: 0x0210B330, 0x0210B238), then rolls
 *   (anim 5) at 4.0 (0x0210B0B4) till it reaches the limits its stage keeps it in the way it rolls (here KatAM's mini-boss's:
 *   AtEdge()), a wall or 1000 frames, and bonks (anim 6; the screen shaking 30 frames: KatAM's longest plain one, 3, 24 frames),
 *   dazed 180 frames (0x0210AFD4), then stands up (anim 7, 0x0210AF58; a 15-frame shake: KatAM's 1, 14).
 * • Hit, it doesn't flinch (0x0210BF18: a flash in its _dmg palette, a 36-frame shake, 36 frames safe): KatAM's mini-bosses don't
 *   either (sub_0809AF38: KatAM's flash and its shortest shake); its defeat is KatAM's mini-boss's (to be inhaled).
 * Not ported: its sounds (0x121, 0x154, 0x16A, 0x16C, 0x16E, 0x130, 0x10D), the DS's rumble (0x0221C748), its _dmg palette flash and
 * Squeak Squad's hit shakes (KatAM's), its defeat's hop (0x0210AE44: KatAM's defeat instead), its ability (none: Metal), the other
 * mode (air control, 0x0210BEB4). */
#include "../squeak.h"

void RequestScreenShake(u16, struct ObjectBase *);

// Animations (bigmetalum.cea);
enum { ANIM_STILL, ANIM_STAND, ANIM_WALK, ANIM_CURL, ANIM_BALL, ANIM_ROLL, ANIM_BONK, ANIM_LAND, ANIM_SPARKLE, ANIM_CHARGE };

// prmMB10Ml0.kgb;
#define GRAVITY 0x2B9
#define MAX_FALL -0x399A
#define DAZE 180
#define ROLL_SPEED 0x4000
#define INTRO_GRAVITY 0x2B8
#define INTRO_FALL -0x2666
#define INTRO 75
#define WALK_STEP 142
#define ROLL_TIME 1000
// prmMB10Pl0.kgb and the brain's own;
#define FACE_DISTANCE 0x1000
#define CHARGE_WAIT 180
#define AGAIN_WAIT 60
/* KatAM's screen shakes (RequestScreenShake 1-3: 14, 18, 24 frames; 4-6 shake on till sub_080700D8 stops them, 5 was taken for a
 * 12-frame one and shook the screen till the fight's end): 14 frames, 24 (its longest timed one), 14; */
#define SHAKE_STOMP 1
#define SHAKE_BONK 3
#define SHAKE_STAND 1

enum {
    BODY_START, BODY_STAND, BODY_INTRO, BODY_WALK_ENTRY, BODY_WALK, BODY_FALL_ENTRY, BODY_FALL, BODY_LAND, BODY_CHARGE_ENTRY,
    BODY_CHARGE, BODY_SPARKLE, BODY_CURL, BODY_BALL, BODY_ROLL, BODY_DAZED, BODY_STAND_UP,
};
enum {
    BRAIN_FACE, BRAIN_REST_PICK, BRAIN_REST, BRAIN_WALK_PICK, BRAIN_WALK, BRAIN_CHARGE_READY, BRAIN_CHARGE, BRAIN_CHARGING,
    BRAIN_SETTLE, BRAIN_SETTLED, BRAIN_AGAIN, BRAIN_AGAIN_WAIT,
};

#define BUSY(s) ((s)->vars[0])          // 0x204: 0 standing, 1 walking, 4 charging;
#define STEPS(s) ((s)->vars[1])         // 0x208: the walk's steps;
#define COUNT(s) ((s)->vars[2])         // 0x20C: the walk's frames, the ball's, the daze's, the intro's;
#define BRAIN_TIMER(s) ((s)->vars[3])   // The brain's 0x98;
#define WALK_COUNT(s) ((s)->vars[4])    // The brain's 0x9C: the steps to walk;
#define BOX(s) (*(u8 *)&(s)->vars[5])   // This frame's box (one);
#define HARM(s) ((s)->vars[6])          // KatAM's unk68 from InitObject: its touch's harm;

// bigmetalum.rca's box (one for all its frames; boxes.py);
static const struct SqueakBox sBigMetalunBoxes[] = {
    { { -19, -39, 23, 0 }, TRUE, 0 },
};
static const struct SqueakBoxFrame sBigMetalunAnim0[] = { { 1, 0 } };
static const struct SqueakBoxFrame sBigMetalunAnim1[] = { { 108, 0 } };
static const struct SqueakBoxFrame sBigMetalunAnim2[] = { { 142, 0 } };
static const struct SqueakBoxFrame sBigMetalunAnim3[] = { { 7, 0 } };
static const struct SqueakBoxFrame sBigMetalunAnim4[] = { { 1, 0 } };
static const struct SqueakBoxFrame sBigMetalunAnim5[] = { { 24, 0 } };
static const struct SqueakBoxFrame sBigMetalunAnim6[] = { { 8, 0 } };
static const struct SqueakBoxFrame sBigMetalunAnim7[] = { { 7, 0 } };
static const struct SqueakBoxFrame sBigMetalunAnim8[] = { { 5, 0 } };
static const struct SqueakBoxFrame sBigMetalunAnim9[] = { { 3, 0 } };
static const struct SqueakBoxFrame sBigMetalunAnim10[] = { { 1, 0 } };
static const struct SqueakBoxAnim sBigMetalunBoxAnims[] = {
    { sBigMetalunAnim0, 1 }, { sBigMetalunAnim1, 1 }, { sBigMetalunAnim2, 1 }, { sBigMetalunAnim3, 1 }, { sBigMetalunAnim4, 1 },
    { sBigMetalunAnim5, 1 }, { sBigMetalunAnim6, 1 }, { sBigMetalunAnim7, 1 }, { sBigMetalunAnim8, 1 }, { sBigMetalunAnim9, 1 },
    { sBigMetalunAnim10, 1 },
};
static const struct SqueakBoxes sBigMetalunBoxData = { sBigMetalunBoxes, sBigMetalunBoxAnims, 11 };

// Roll(); Squeak Squad's random 0-99 (its LCG at 0x01FF8120: here KatAM's);
static s32 Roll(void) {
    return Rand16() * 100 >> 16;
}

// Gravity(); 0x0210AB10: on the ground, still; else falling GRAVITY a frame, no faster than MAX_FALL;
static void Gravity(struct SqueakObject *s) {
    if (SqueakOnGround(s)) {
        s->vy = s->ay = 0;
    } else if (!SqueakCapFall(s, MAX_FALL)) {
        s->ay = -GRAVITY;
    }
}

// Charge(); 0x0210AA88: B charges;
static bool32 Charge(struct SqueakObject *s) {
    if (!s->padB) return FALSE;
    SqueakSetState(s, BODY_CHARGE_ENTRY);
    return TRUE;
}

// WalkSpeed(); 0x0210B8F0's: its walk's speed on that frame of its step (and the stomps);
static s32 WalkSpeed(struct SqueakObject *s) {
    switch (COUNT(s)) {
    case 0:
    case 8:
        return 0x1000;
    case 20:
        return 0x3000;
    case 30:
    case 79:
    case 91:
    case 101:
        return 0x2000;
    case 38:
        RequestScreenShake(SHAKE_STOMP, &s->obj.base);
        return 0;
    case 109:
        RequestScreenShake(SHAKE_STOMP, &s->obj.base);
        return 0x2000;
    default:
        return 0;
    }
}

/* AtEdge(); 0x0210B0B4's: its middle at the limits its stage keeps characters in, the way it rolls (held there: logged, the stage's
 * limits, the camera's +0x5C and +0x64, 16 and 1584 in a 1600-pixel room, not the screen's); here the limits KatAM keeps its
 * mini-bosses in (ObjXSomething(), on the mini-boss's area from its camera lock or else the room's: its box 8 pixels inside them),
 * which hold it there anyway; */
static bool32 AtEdge(struct SqueakObject *s) {
    struct Object *obj = &s->obj;
    s32 left = (obj->unkA4 + 8 - obj->base.unk3C) << 8, right = (obj->unkA8 - 8 - obj->base.unk3E) << 8;

    if (obj->unkA8 <= obj->unkA4) return FALSE;
    if (!(obj->base.flags & 1) && obj->base.x >= right) {
        obj->base.x = right;
        return TRUE;
    }
    if ((obj->base.flags & 1) && obj->base.x <= left) {
        obj->base.x = left;
        return TRUE;
    }
    return FALSE;
}

static bool32 Body(struct SqueakObject *s) {
    switch (s->state) {
    case BODY_START:        // 0x0210BC40 (vtable +0x3C);
        BUSY(s) = 0;
        SqueakPlay(s, ANIM_STAND);
        SqueakStop(s);
        SqueakSetState(s, BODY_STAND);
        return TRUE;
    case BODY_STAND:        // 0x0210BB4C;
        if (Charge(s)) return TRUE;
        if (s->padDir > 0) {
            SqueakSetState(s, BODY_WALK_ENTRY);
            return TRUE;
        }
        if (s->padDir < 0) SqueakTurnAround(s);
        SqueakMove(s);
        if (!SqueakOnGround(s)) SqueakSetState(s, BODY_FALL_ENTRY);
        break;
    case BODY_INTRO:        // 0x0210BCE0: untouchable, falling, INTRO frames;
        if (COUNT(s) < 1) {
            s->obj.base.flags &= ~0x200;
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        COUNT(s)--;
        SqueakCapFall(s, INTRO_FALL);
        SqueakMove(s);
        break;
    case BODY_WALK_ENTRY:   // 0x0210BA88;
        BUSY(s) = 1;
        SqueakPlay(s, ANIM_WALK);
        SqueakStop(s);
        COUNT(s) = STEPS(s) = 0;
        SqueakSetState(s, BODY_WALK);
        return TRUE;
    case BODY_WALK:         // 0x0210B8F0: its steps' moves, stomps and count;
        if (Charge(s)) return TRUE;
        if (s->padDir == 0) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        s->vx = WalkSpeed(s);
        if (++COUNT(s) == WALK_STEP) {
            COUNT(s) = 0;
            STEPS(s)++;
        }
        if (s->padDir < 0) SqueakTurnAround(s);
        SqueakMove(s);
        if (!SqueakOnGround(s)) SqueakSetState(s, BODY_FALL_ENTRY);
        break;
    case BODY_FALL_ENTRY:   // 0x0210B7F0: its speed across as it was;
        SqueakPlay(s, ANIM_LAND);
        s->maxVx = s->maxVy = s->ax = s->ay = s->fx = s->fy = s->vy = 0;
        s->ay = -GRAVITY;
        SqueakSetState(s, BODY_FALL);
        return TRUE;
    case BODY_FALL:         // 0x0210B718;
        SqueakCapFall(s, MAX_FALL);
        SqueakMove(s);
        if (SqueakOnGround(s)) {
            SqueakStop(s);
            SqueakPlay(s, ANIM_LAND);
            SqueakSetState(s, BODY_LAND);
        }
        break;
    case BODY_LAND:         // 0x0210B6B8: its animation over, it stands;
        if (s->animEnded) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        Gravity(s);
        SqueakMove(s);
        break;
    case BODY_CHARGE_ENTRY: // 0x0210B5F8;
        BUSY(s) = 4;
        SqueakStop(s);
        SqueakPlay(s, ANIM_CHARGE);
        SqueakSetState(s, BODY_CHARGE);
        return TRUE;
    case BODY_CHARGE:       // 0x0210B52C, 0x0210B460: each animation over, the next (it sparkles, it curls up);
    case BODY_SPARKLE:
        if (s->animEnded) {
            SqueakPlay(s, s->state == BODY_CHARGE ? ANIM_SPARKLE : ANIM_CURL);
            SqueakSetState(s, s->state + 1);
            return TRUE;
        }
        Gravity(s);
        SqueakMove(s);
        break;
    case BODY_CURL:         // 0x0210B330: curled up, a ball, still how long;
        if (s->animEnded) {
            s32 roll = Roll();

            COUNT(s) = roll < 20 ? 30 : roll < 80 ? 60 : 120;
            SqueakPlay(s, ANIM_BALL);
            SqueakSetState(s, BODY_BALL);
            return TRUE;
        }
        Gravity(s);
        SqueakMove(s);
        break;
    case BODY_BALL:         // 0x0210B238;
        if (COUNT(s) < 1) {
            COUNT(s) = ROLL_TIME;
            SqueakPlay(s, ANIM_ROLL);
            SqueakSetState(s, BODY_ROLL);
            return TRUE;
        }
        COUNT(s)--;
        Gravity(s);
        SqueakMove(s);
        break;
    case BODY_ROLL:         // 0x0210B0B4: at ROLL_SPEED till the arena's edge, a wall or ROLL_TIME frames;
        s->vx = ROLL_SPEED;
        if (!AtEdge(s) && !SqueakWallAhead(s) && COUNT(s) > 0) {
            COUNT(s)--;
            Gravity(s);
            SqueakMove(s);
            break;
        }
        COUNT(s) = DAZE;
        SqueakStop(s);
        SqueakPlay(s, ANIM_BONK);
        RequestScreenShake(SHAKE_BONK, &s->obj.base);
        SqueakSetState(s, BODY_DAZED);
        return TRUE;
    case BODY_DAZED:        // 0x0210AFD4;
        if (COUNT(s) < 1) {
            SqueakPlay(s, ANIM_LAND);
            SqueakSetState(s, BODY_STAND_UP);
            return TRUE;
        }
        COUNT(s)--;
        Gravity(s);
        SqueakMove(s);
        break;
    case BODY_STAND_UP:     // 0x0210AF58: its animation over, it stands;
        if (s->animEnded) {
            RequestScreenShake(SHAKE_STAND, &s->obj.base);
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        Gravity(s);
        SqueakMove(s);
        break;
    }
    return FALSE;
}

// Brain(); Its pad (0x0210A868 and the states it leads to), one state a frame;
static void Brain(struct SqueakObject *s) {
    s32 roll;

    switch (s->brain) {
    case BRAIN_FACE:        // 0x0210A868;
        SqueakFaceKirby(s, FACE_DISTANCE);
        BRAIN_TIMER(s) = 0;
        s->brain = BRAIN_REST_PICK;
        break;
    case BRAIN_REST_PICK:   // 0x0210A7F8;
        roll = Roll();
        s->padDir = s->padY = 0;
        BRAIN_TIMER(s) = roll < 30 ? 60 : roll < 80 ? 120 : 180;
        s->brain = BRAIN_REST;
        break;
    case BRAIN_REST:        // 0x0210A7D0;
        if (BRAIN_TIMER(s) < 1) s->brain = BRAIN_WALK_PICK;
        BRAIN_TIMER(s)--;
        break;
    case BRAIN_WALK_PICK:   // 0x0210A754;
        roll = Roll();
        s->padDir = 1;
        s->padY = 0;
        WALK_COUNT(s) = roll < 30 ? 1 : roll < 80 ? 2 : 4;
        s->brain = BRAIN_WALK;
        break;
    case BRAIN_WALK:        // 0x0210A708: its steps walked and its body standing, the charge; a wall ahead turns it round;
        if (STEPS(s) >= WALK_COUNT(s)) {
            s->padDir = s->padY = 0;
            if (BUSY(s) == 0) s->brain = BRAIN_CHARGE_READY;
        }
        if (SqueakWallAhead(s)) {
            s->padDir = -1;
            s->padY = 0;
        }
        break;
    case BRAIN_CHARGE_READY:    // 0x0210A6F0;
        s->padDir = s->padY = 0;
        s->brain = BRAIN_CHARGE;
        break;
    case BRAIN_CHARGE:      // 0x0210A6D0;
        s->padB = 1;
        BRAIN_TIMER(s) = CHARGE_WAIT;
        s->brain = BRAIN_CHARGING;
        break;
    case BRAIN_CHARGING:    // 0x0210A688;
        s->padB = 0;
        if (!SqueakWallAhead(s) && BRAIN_TIMER(s) > 0) {
            BRAIN_TIMER(s)--;
            break;
        }
        s->brain = BRAIN_SETTLE;
        break;
    case BRAIN_SETTLE:      // 0x0210A650: its body standing, it turns round, then faces Kirby;
        s->padDir = s->padY = 0;
        if (BUSY(s) == 0) {
            s->padDir = -1;
            s->padY = 0;
            SqueakFaceKirby(s, FACE_DISTANCE);
            s->brain = BRAIN_SETTLED;
        }
        break;
    case BRAIN_SETTLED:     // 0x0210A638;
        s->padDir = s->padY = 0;
        s->brain = BRAIN_AGAIN;
        break;
    case BRAIN_AGAIN:       // 0x0210A5D0;
        roll = Roll();
        s->padDir = s->padY = 0;
        if (roll < 30) {
            BRAIN_TIMER(s) = AGAIN_WAIT;
            s->brain = BRAIN_AGAIN_WAIT;
        } else {
            s->brain = BRAIN_FACE;
        }
        break;
    case BRAIN_AGAIN_WAIT:  // 0x0210A5A8;
        if (BRAIN_TIMER(s) < 1) s->brain = BRAIN_CHARGE_READY;
        BRAIN_TIMER(s)--;
        break;
    }
}

static void BigMetalunMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    SqueakMidbossUpdate(s);
    Brain(s);
    SqueakRun(s, Body);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
    SqueakSetBoxes(s, &sBigMetalunBoxData, HARM(s), &BOX(s));
}

/* BigMetalunStart(); Its fight's start (and after a hit: it goes on, as KatAM's mini-bosses do): the first time, its intro (0x0210BDE8,
 * 0x0210BD54: untouchable, falling at 0x2B8 a frame to 2.4, INTRO frames), then its body's start; */
void BigMetalunStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, -1, BigMetalunMain);
    if (s->started) return;
    SqueakPlay(s, ANIM_STAND);
    SqueakStop(s);
    s->ay = -INTRO_GRAVITY;
    COUNT(s) = INTRO;
    s->obj.base.flags |= 0x200;
    SqueakSetState(s, BODY_INTRO);
}

/* BigMetalunInit(); Its box (bigmetalum.rca's: 19 pixels behind to 23 ahead, up to 39 up), its bounds (KatAM's), facing left,
 * standing; */
void BigMetalunInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -19, -39, 23, 0);
    ObjectSetBounds(&s->obj.base, -16, -36, 16, 0);
    HARM(s) = s->obj.base.unk68;
    s->obj.base.flags |= 1;
    SqueakStandOnFloor(&s->obj);
    SqueakPlay(s, ANIM_STAND);
}
