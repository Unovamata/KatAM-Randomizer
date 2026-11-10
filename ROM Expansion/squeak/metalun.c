/* Metalun; ported from Kirby: Squeak Squad (USA), enemy 0x2C (its body 0x020526AC, vtable 0x021699E4; brain 0x0205155C), as
 * SQUEAK_METALUN. Values from param/prmCE29Ml0.kgb (movement; MlG differs only in the other mode's walk), prmCE29Pl0.kgb (brain) and
 * prmCE29Sl0.kgb (HP 50); speeds 20.12 a frame, Y up; its boxes from enemy/metalum.rca (its animations from the .rca's second). A
 * heavy metal walker: it lurches along, and near Kirby it crouches and charges in heavy stomping lunges that shake the screen.
 * • Brain (the spawn's first parameter byte, KatAM's template unk12):
 *   0 and any but 1 and 2 (0x02051490): it faces Kirby (past 1 pixel) and stands (0x02051400) till he's within 96 pixels across and
 *     64 up or down: it faces him and presses B one frame (0x020513C4), then waits for four of its body's animation ends (0x0205137C,
 *     0x02051360, 0x02051344, 0x02051308: at a wall its pad would turn, but the next state stands it: 0x020512E0) and looks again;
 *   1 (0x020512C0): it faces Kirby and walks (0x0205124C) till he's within the same: B, the same four waits (0x02051224,
 *     0x020511DC, 0x020511C0, 0x020511A4, 0x02051178), it faces him again (0x02051150) and walks on;
 *   2 (0x02051130): it faces Kirby, then stands (0x02051118).
 * • Body (gravity 0.1 a frame, falls no faster than 2.4): it stands (anim 1: 0x020524E4, 0x02052420) or walks (anim 2, 80 frames:
 *   0x02052340, 0x02052230) in lurches, 4 pixels on its walk's frames 29, 37, 69 and 75, still otherwise (its count started over
 *   after 75), starting over at the animation's loop; off the ground it falls (anim 1: 0x02051F84, 0x02051EA0; forward keeps it going
 *   at 0.25: 0x0205258C; landed, a thud, 0x14A); A jumps (2.5 up, anim 3: 0x020520FC, 0x02052090; its brains never press it, nor in
 *   this mode its body heed it). B crouches (anim 4, 29 frames: 0x02051DDC, 0x02051CA8) and charges through anim 5 twice (156 frames
 *   each: 0x02051A4C, 0x020517F0): on its frames 0, 25, 51, 77, 103 and 129 it lurches 2 pixels, 5 or 6 frames on 8 more with a
 *   stomp (0x14B) and the screen shaking on the ground (Squeak Squad's 0x02035FC4(…, 3, 4, 2), 4 frames: here KatAM's shortest,
 *   RequestScreenShake 1, 14 frames, as Bio Spark's), turning at walls; in the air it drifts at 0.6; then it stands up (anim 6,
 *   28 frames: 0x02051738) once landed.
 * • Boxes (metalum.rca, per animation frame, as uja.c's): its body's hit box (8 pixels behind to 16 ahead, 2 to 26 up), its touch
 *   box (8 behind to 9 ahead, 7 to 24 up) in it.
 * Its sounds: 0x14A the thud, 0x14B the stomp. Not ported: its ability (Squeak Squad's Metal: KatAM has none), its other mode (the brain's +8 not 3,
 * which its brain's constructor 0x0205155C never leaves: A jumps and B stops the charge; MlG's walk), the enemy base class's bump
 * into Kirby (its vtable +0x48 and +0x50 states: logged knocked back 2.0, slowing 0.15 a frame), Squeak Squad's contact damage (8
 * of 36: KatAM's 1). */
#include "../squeak.h"
#include "main.h"

void RequestScreenShake(u16, struct ObjectBase *);

// Animations (metalum.cea);
enum { ANIM_STILL, ANIM_STAND, ANIM_WALK, ANIM_JUMP, ANIM_CROUCH, ANIM_CHARGE, ANIM_STAND_UP };

// prmCE29Ml0.kgb;
#define WALK_SPEED 0x400
#define JUMP_SPEED 0x2800
#define GRAVITY 0x19A
// prmCE29Pl0.kgb;
#define NEAR_X 0x60000
#define NEAR_Y 0x40000
// The code's;
#define MAX_FALL -0x2666
#define FACE_DISTANCE 0x1000
#define STEP_SPEED 0x4000               // The walk's lurches (its frames 29, 37, 69 and 75), its count started over after 75;
#define STEP_LAST 75
#define LUNGE_SPEED 0x2000              // The charge's lurches: 2.0, then 8.0 with a stomp, on its frames below;
#define STOMP_SPEED 0x8000
#define CHARGE_FRAMES 0x9C
#define DRIFT_SPEED 0x99A               // In the air, charging;
#define SHAKE 1

enum {
    BODY_START, BODY_STAND, BODY_WALK_ENTRY, BODY_WALK, BODY_JUMP_ENTRY, BODY_RISE, BODY_FALL_ENTRY, BODY_FALL, BODY_CROUCH_ENTRY,
    BODY_CROUCH, BODY_CHARGE, BODY_CHARGE_2, BODY_STAND_UP,
};
enum {
    BRAIN_FACE, BRAIN_LOOK, BRAIN_LET_GO, BRAIN_WAIT_1, BRAIN_WAIT_2, BRAIN_WAIT_3, BRAIN_WAIT_4, BRAIN_RESET, BRAIN_WALK_FACE,
    BRAIN_WALK, BRAIN_WALK_LET_GO, BRAIN_WALK_WAIT_1, BRAIN_WALK_WAIT_2, BRAIN_WALK_WAIT_3, BRAIN_WALK_WAIT_4, BRAIN_WALK_RESET,
    BRAIN_STILL_FACE, BRAIN_STILL,
};

#define STEP(s) ((s)->vars[0])          // 0x1F8: the walk's frames;
#define COUNT(s) ((s)->vars[1])         // 0x1FC: the charge's frames;
#define HARM(s) ((s)->vars[2])          // KatAM's unk68 from InitObject: its touch's harm;
#define BOX(s) (*(u8 *)&(s)->vars[3])   // This frame's box: sMetalunBoxes' index;

// metalum.rca's boxes (its animations from the .rca's second; boxes.py);
static const struct SqueakBox sMetalunBoxes[] = {
    { { -8, -26, 16, -2 }, TRUE, 0 },
    { { -9, -26, 16, -2 }, TRUE, 0 },
    { { -8, -26, 16, -2 }, FALSE, 0 },
};
static const struct SqueakBoxFrame sMetalunAnim0[] = { { 1, 0 } };
static const struct SqueakBoxFrame sMetalunAnim1[] = { { 60, 0 } };
static const struct SqueakBoxFrame sMetalunAnim2[] = { { 80, 0 } };
static const struct SqueakBoxFrame sMetalunAnim3[] = { { 1, 1 } };
static const struct SqueakBoxFrame sMetalunAnim4[] = { { 29, 0 } };
static const struct SqueakBoxFrame sMetalunAnim5[] = { { 156, 0 } };
static const struct SqueakBoxFrame sMetalunAnim6[] = { { 28, 0 } };
static const struct SqueakBoxFrame sMetalunAnim7[] = { { 1, 2 } };
static const struct SqueakBoxFrame sMetalunAnim8[] = { { 1, 2 } };
static const struct SqueakBoxAnim sMetalunBoxAnims[] = {
    { sMetalunAnim0, 1 },
    { sMetalunAnim1, 1 },
    { sMetalunAnim2, 1 },
    { sMetalunAnim3, 1 },
    { sMetalunAnim4, 1 },
    { sMetalunAnim5, 1 },
    { sMetalunAnim6, 1 },
    { sMetalunAnim7, 1 },
    { sMetalunAnim8, 1 },
};
static const struct SqueakBoxes sMetalunBoxData = { sMetalunBoxes, sMetalunBoxAnims, 9 };

// Gravity(); 0x020516E0: on the ground, still; else falling GRAVITY a frame, no faster than MAX_FALL;
static void Gravity(struct SqueakObject *s) {
    if (SqueakOnGround(s)) {
        s->vy = s->ay = 0;
    } else if (!SqueakCapFall(s, MAX_FALL)) {
        s->ay = -GRAVITY;
    }
}

// AirControl(); 0x0205258C: back turns it round; forward, it goes at WALK_SPEED (no cap); none, it stops across;
static void AirControl(struct SqueakObject *s) {
    if (s->padDir < 0) SqueakTurnAround(s);
    if (s->padDir > 0) {
        s->vx = WALK_SPEED;
        s->maxVx = 0;
    } else if (s->padDir == 0) {
        s->vx = 0;
    }
}

// Crouch(); 0x02051650: B crouches for the charge;
static bool32 Crouch(struct SqueakObject *s) {
    if (s->padB) {
        SqueakSetState(s, BODY_CROUCH_ENTRY);
        return TRUE;
    }
    return FALSE;
}

/* Lunge(); 0x02051A4C's and 0x020517F0's: its lurches on the charge's frames (2.0, then 8.0 with a stomp and the screen shaking on
 * the ground; every one but the first), turning at walls; its count runs over the animation's frames; */
static void Lunge(struct SqueakObject *s) {
    bool8 stomp = TRUE;

    if (SqueakWallAhead(s)) SqueakTurnAround(s);
    switch (COUNT(s)) {
    case 0:
        stomp = FALSE;
        // fall through
    case 25:
    case 51:
    case 77:
    case 103:
    case 129:
        s->vx = LUNGE_SPEED;
        break;
    case 5:
    case 31:
    case 57:
    case 83:
    case 109:
    case 135:
        s->vx = STOMP_SPEED;
        break;
    default:
        s->vx = 0;
        stomp = FALSE;
        break;
    }
    if (stomp && SqueakOnGround(s)) {
        SqueakSfx(s, 0x14B);
        RequestScreenShake(SHAKE, &s->obj.base);
    }
    COUNT(s) = COUNT(s) < CHARGE_FRAMES ? COUNT(s) + 1 : 0;
}

static bool32 Body(struct SqueakObject *s) {
    switch (s->state) {
    case BODY_START:        // 0x020524E4 (vtable +0x3C);
        SqueakPlay(s, ANIM_STAND);
        SqueakStop(s);
        SqueakSetState(s, BODY_STAND);
        return TRUE;
    case BODY_STAND:        // 0x02052420;
        if (Crouch(s)) return TRUE;
        if (s->padDir != 0) {
            SqueakSetState(s, BODY_WALK_ENTRY);
            return TRUE;
        }
        SqueakMove(s);
        if (!SqueakOnGround(s)) SqueakSetState(s, BODY_FALL_ENTRY);
        break;
    case BODY_WALK_ENTRY:   // 0x02052340;
        SqueakPlay(s, ANIM_WALK);
        SqueakStop(s);
        s->vx = WALK_SPEED;
        STEP(s) = 0;
        SqueakSetState(s, BODY_WALK);
        return TRUE;
    case BODY_WALK:         // 0x02052230: its lurches; its animation's loop over, it stands (and walks again);
        s->vx = STEP(s) == 29 || STEP(s) == 37 || STEP(s) == 69 || STEP(s) == STEP_LAST ? STEP_SPEED : 0;
        if (STEP(s) > STEP_LAST - 1) STEP(s) = 0;
        if (Crouch(s)) return TRUE;
        if (s->animEnded) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        if (s->padDir < 0) SqueakTurnAround(s);
        SqueakMove(s);
        if (!SqueakOnGround(s)) {
            SqueakSetState(s, BODY_FALL_ENTRY);
            break;
        }
        STEP(s)++;
        break;
    case BODY_JUMP_ENTRY:   // 0x020520FC;
        SqueakPlay(s, ANIM_JUMP);
        s->vy = s->maxVx = s->maxVy = s->ax = s->ay = s->fx = s->fy = 0;
        s->vy = JUMP_SPEED;
        s->ay = -GRAVITY;
        SqueakSetState(s, BODY_RISE);
        return TRUE;
    case BODY_RISE:         // 0x02052090;
        AirControl(s);
        SqueakMove(s);
        if (s->vy < 0) SqueakSetState(s, BODY_FALL_ENTRY);
        break;
    case BODY_FALL_ENTRY:   // 0x02051F84: its speeds as they were;
        SqueakPlay(s, ANIM_STAND);
        s->maxVx = s->maxVy = s->ax = s->ay = s->fx = s->fy = 0;
        s->ay = -GRAVITY;
        SqueakSetState(s, BODY_FALL);
        return TRUE;
    case BODY_FALL:         // 0x02051EA0: landed, it walks on if it was going across;
        AirControl(s);
        SqueakCapFall(s, MAX_FALL);
        SqueakMove(s);
        if (SqueakOnGround(s)) {
            SqueakSfx(s, 0x14A);
            SqueakSetState(s, s->vx == 0 ? BODY_START : BODY_WALK_ENTRY);
            s->vy = s->ay = s->ax = 0;
            return TRUE;
        }
        break;
    case BODY_CROUCH_ENTRY: // 0x02051DDC;
        SqueakPlay(s, ANIM_CROUCH);
        SqueakStop(s);
        COUNT(s) = 0;
        if (s->padDir < 0) SqueakTurnAround(s);
        SqueakSetState(s, BODY_CROUCH);
        return TRUE;
    case BODY_CROUCH:       // 0x02051CA8;
        if (s->animEnded) {
            SqueakPlay(s, ANIM_CHARGE);
            SqueakStop(s);
            COUNT(s) = 0;
            SqueakSetState(s, BODY_CHARGE);
            return TRUE;
        }
        Gravity(s);
        SqueakMove(s);
        break;
    case BODY_CHARGE:       // 0x02051A4C, 0x020517F0: twice through anim 5, then it stands up;
    case BODY_CHARGE_2:
        Lunge(s);
        if (s->animEnded) {
            if (s->state == BODY_CHARGE) {
                SqueakPlay(s, ANIM_CHARGE);
                SqueakStop(s);
                SqueakSetState(s, BODY_CHARGE_2);
            } else {
                SqueakStop(s);
                SqueakPlay(s, ANIM_STAND_UP);
                SqueakSetState(s, BODY_STAND_UP);
            }
            COUNT(s) = 0;
            return TRUE;
        }
        Gravity(s);
        if (!SqueakOnGround(s)) s->vx = DRIFT_SPEED;
        SqueakMove(s);
        break;
    case BODY_STAND_UP:     // 0x02051738: landed and its animation over;
        if (s->vy == 0 && s->animEnded) {
            COUNT(s) = 0;
            SqueakStop(s);
            SqueakSetState(s, SqueakOnGround(s) ? BODY_START : BODY_FALL_ENTRY);
            return TRUE;
        }
        Gravity(s);
        SqueakMove(s);
        break;
    }
    return FALSE;
}

// Stand(); The brain's 0x02052860 and 0x02052868: no direction, its pad's up and down and A none;
static void Stand(struct SqueakObject *s) {
    s->padDir = s->padY = 0;
    s->padA = 0;
}

// Brain(); Its pad (0x02051490, 0x020512C0, 0x02051130 and the states they lead to), one state a frame;
static void Brain(struct SqueakObject *s) {
    switch (s->brain) {
    case BRAIN_FACE:        // 0x02051490;
        SqueakFaceKirby(s, FACE_DISTANCE);
        s->brain = BRAIN_LOOK;
        break;
    case BRAIN_LOOK:        // 0x02051400;
        Stand(s);
        if (SqueakKirbyWithin(s, NEAR_X, NEAR_Y)) {
            SqueakFaceKirby(s, FACE_DISTANCE);
            s->padB = 1;
            s->brain = BRAIN_LET_GO;
        }
        break;
    case BRAIN_LET_GO:      // 0x020513C4;
        Stand(s);
        s->padB = 0;
        s->brain = BRAIN_WAIT_1;
        break;
    case BRAIN_WAIT_1:      // 0x0205137C;
        if (s->animEnded) {
            Stand(s);
            s->padB = 0;
            s->brain = BRAIN_WAIT_2;
        }
        break;
    case BRAIN_WAIT_2:      // 0x02051360;
    case BRAIN_WAIT_3:      // 0x02051344;
        if (s->animEnded) s->brain++;
        break;
    case BRAIN_WAIT_4:      // 0x02051308: (at a wall its pad turns, the next state standing it);
        if (s->animEnded) {
            if (SqueakWallAhead(s)) s->padDir = s->padDir == 1 ? -1 : 1;
            s->brain = BRAIN_RESET;
        }
        break;
    case BRAIN_RESET:       // 0x020512E0;
        Stand(s);
        s->brain = BRAIN_LOOK;
        break;
    case BRAIN_WALK_FACE:   // 0x020512C0;
        SqueakFaceKirby(s, FACE_DISTANCE);
        s->brain = BRAIN_WALK;
        break;
    case BRAIN_WALK:        // 0x0205124C;
        s->padDir = 1;
        if (SqueakKirbyWithin(s, NEAR_X, NEAR_Y)) {
            s->padB = 1;
            s->brain = BRAIN_WALK_LET_GO;
        }
        break;
    case BRAIN_WALK_LET_GO: // 0x02051224;
        s->padB = 0;
        s->brain = BRAIN_WALK_WAIT_1;
        break;
    case BRAIN_WALK_WAIT_1: // 0x020511DC;
        if (s->animEnded) {
            Stand(s);
            s->padB = 0;
            s->brain = BRAIN_WALK_WAIT_2;
        }
        break;
    case BRAIN_WALK_WAIT_2: // 0x020511C0;
    case BRAIN_WALK_WAIT_3: // 0x020511A4;
        if (s->animEnded) s->brain++;
        break;
    case BRAIN_WALK_WAIT_4: // 0x02051178;
        if (s->animEnded) {
            SqueakFaceKirby(s, FACE_DISTANCE);
            s->brain = BRAIN_WALK_RESET;
        }
        break;
    case BRAIN_WALK_RESET:  // 0x02051150;
        Stand(s);
        s->brain = BRAIN_WALK;
        break;
    case BRAIN_STILL_FACE:  // 0x02051130;
        SqueakFaceKirby(s, FACE_DISTANCE);
        s->brain = BRAIN_STILL;
        break;
    case BRAIN_STILL:       // 0x02051118;
        Stand(s);
        break;
    }
}

static void MetalunMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    Brain(s);
    SqueakRun(s, Body);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
    SqueakSetBoxes(s, &sMetalunBoxData, HARM(s), &BOX(s));
}

void MetalunStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, 0, MetalunMain);
    SqueakThroughTerrain(s, FALSE);
    if (!s->started) {
        switch (s->variant) {
        case 1:
            s->brain = BRAIN_WALK_FACE;
            break;
        case 2:
            s->brain = BRAIN_STILL_FACE;
            break;
        default:
            s->brain = BRAIN_FACE;
            break;
        }
    }
    SqueakSetState(s, BODY_START);
}

/* MetalunInit(); Its box (metalum.rca's hit box, 8 pixels behind to 16 ahead, 2 to 26 up; its body box 6 either side, 1 to 19 up:
 * here from its feet, KatAM's bounds), facing left (0x020526AC); its hit box and harm set each frame (SqueakSetBoxes()); */
void MetalunInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -8, -26, 16, -2);
    ObjectSetBounds(&s->obj.base, -6, -19, 6, 0);
    HARM(s) = s->obj.base.unk68;
    s->obj.base.flags |= 1;
    SqueakStandOnFloor(&s->obj);
}
