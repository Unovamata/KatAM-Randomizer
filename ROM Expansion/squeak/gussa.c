/* Gussa; ported from Kirby: Squeak Squad (USA), enemy 0x0B (its body 0x0206E364, brain 0x0206C3D0), as SQUEAK_GUSSA. Values from
 * param/PrmNE12Ml0.kgb (movement; MlG, the other mode's, only jumps lower), PrmNE12Pl0.kgb (brain) and PrmNE12Sl0.kgb (HP 10);
 * speeds 20.12 a frame, Y up. A fish: its moves in the water are composite animations (gussa.mca: body, tail and fin, each its own
 * animation; SqueakPlayComposite).
 * • Brain: the spawn record's first parameter byte (KatAM's template unk12) picks it (0x0206C3D0); each first notes where it is and
 *   turns to face Kirby (0x0206E534, past 1 pixel):
 *   1: lines up with Kirby while he's within 96 pixels across (swimming up or down till within 2 pixels of his height), then dashes
 *      (B) forward for 300 frames or till a wall, rests 200 frames, faces him and dashes again, for good;
 *   0 (and any other): lines up and dashes the same, then swims back to where it started (up a wall in its way, or down from a
 *      ceiling), faces Kirby and lines up again.
 * • Body: in the water it floats (composite 0) or swims at 0.8 the way its pad says (0 across, 1 up, 2 down, and diagonally, 0.8
 *   each way); B dashes (composite 3) at 1.3, up or down at 0.4 as the pad says (turning round, it swings back at 0.2 a frame); a wall
 *   ahead stops the dash: it bonks (anims 15, 16), springs back at 3.0 slowing 0.2 a frame, then floats. Out of the water (the swims
 *   and the dash) it jumps out at 4.0 up, falling 0x19A a frame, no faster than 2.4 (anim 1), its pad turning it round or moving it
 *   on at 0.8; it splashes going out and falling back in (SqueakSplash). On land it lies (composite 0) or wriggles on at 0.4 while
 *   its pad says so; A would jump (no brain of its presses it).
 * Its sound: 0x130 the bonk (0x0206C75C). Not ported: the other mode (the brain's +8 not 3; never so for these brains): floating sinks it
 * (0x52 a frame, to 0.2) and leaves the water by falling; */
#include "../squeak.h"

// Animations (gussa.cea; the composite ones are gussa.mca's);
enum { ANIM_STILL, ANIM_FALL, ANIM_BONK = 15, ANIM_BONK_BACK, ANIM_HURT, ANIM_HURT_2 };
enum { MCA_FLOAT, MCA_UP, MCA_DOWN, MCA_DASH };

// PrmNE12Ml0.kgb;
#define LAND_SPEED 0x667
#define AIR_SPEED 0xCCD
#define JUMP_SPEED 0x4000
#define GRAVITY 0x19A
#define MAX_FALL -0x2667
#define SWIM_SPEED 0xCCD
#define DASH_SPEED 0x14CD
#define DASH_RISE 0x667
#define DASH_SWING 0x334
#define BONK_SPEED 0x3000
#define BONK_DECEL 0x334
// PrmNE12Pl0.kgb;
#define LINE_UP 0x2000
#define DASH_RANGE 0x60000
#define DASH_RANGE_1 0x60000
#define DASH_TIME 300
#define REST_TIME 200
#define FACE_DISTANCE 0x1000

/* Body states: each a Squeak Squad state function; the entries (_ENTRY) set things up and chain to the state that runs every frame
 * (SqueakRun: a state returning TRUE runs the next one this same frame, as Squeak Squad's dispatcher does); */
enum {
    BODY_START, BODY_LAND, BODY_WRIGGLE_ENTRY, BODY_WRIGGLE, BODY_JUMP_ENTRY, BODY_JUMP, BODY_FALL_ENTRY, BODY_FALL, BODY_FLOAT_ENTRY,
    BODY_FLOAT, BODY_SWIM_ENTRY, BODY_SWIM, BODY_SWIM_UP_ENTRY, BODY_SWIM_UP, BODY_SWIM_DOWN_ENTRY, BODY_SWIM_DOWN,
    BODY_SWIM_UP_FWD_ENTRY, BODY_SWIM_UP_FWD, BODY_SWIM_DOWN_FWD_ENTRY, BODY_SWIM_DOWN_FWD, BODY_DASH_ENTRY, BODY_DASH,
    BODY_BONK_ENTRY, BODY_BONK, BODY_BONK_BACK, BODY_RECOIL,
};
// Brain states (one each frame);
enum {
    BRAIN_HOME, BRAIN_LINE_UP, BRAIN_DASH, BRAIN_RETURN, BRAIN_FACE, BRAIN_HOME_1, BRAIN_LINE_UP_1, BRAIN_DASH_1, BRAIN_REST_1,
};

// The brain's own fields;
#define BRAIN_TIMER(s) ((s)->vars[0])   // 0xA0;
#define CLIMB(s) ((s)->vars[1])         // 0xA4: on its way back, a wall ahead: 1 swims up it, -1 down it;
#define HOME_X(s) ((s)->vars[2])        // 0x98, 0x9C: where it started;
#define HOME_Y(s) ((s)->vars[3])

// OnGround(); The body's 0x84 (its distance down to the floor) under 1;
static bool32 OnGround(struct SqueakObject *s) {
    return SqueakOnGround(s);
}

/* Swims (0x0206C9F4, 0x0206CCEC, 0x0206CFE0, 0x0206D2BC, 0x0206D5A0): out of the water it jumps (splashing); B dashes; else the pad
 * picks the swim (up/down, across); `self` is the state asking, which goes on; */
static bool32 SwimChecks(struct SqueakObject *s, u8 self) {
    u8 next;

    if (!SqueakInWater(s)) {
        SqueakSetState(s, BODY_JUMP_ENTRY);
        SqueakSplash(s, TRUE);
        return TRUE;
    }
    if (s->padB) {
        SqueakSetState(s, BODY_DASH_ENTRY);
        return TRUE;
    }
    if (s->padY == 0) next = s->padDir == 0 ? BODY_FLOAT_ENTRY : BODY_SWIM_ENTRY;
    else if (s->padY > 0) next = s->padDir == 0 ? BODY_SWIM_UP_ENTRY : BODY_SWIM_UP_FWD_ENTRY;
    else next = s->padDir == 0 ? BODY_SWIM_DOWN_ENTRY : BODY_SWIM_DOWN_FWD_ENTRY;
    if (next == self) return FALSE;
    SqueakSetState(s, next);
    return TRUE;
}

// Swim(); A swim's every frame: its pad turns it round;
static bool32 Swim(struct SqueakObject *s, u8 entry) {
    if (SwimChecks(s, entry)) return TRUE;
    if (s->padDir < 0) SqueakTurnAround(s);
    SqueakMove(s);
    return FALSE;
}

// AirControl(); 0x0206E250: in the air, its pad turns it round, forward moves it on at 0.8, nothing stops it across;
static void AirControl(struct SqueakObject *s) {
    if (s->padDir < 0) SqueakTurnAround(s);
    if (s->padDir > 0) {
        s->vx = AIR_SPEED;
        s->maxVx = 0;
    }
}

// Landed(); vtable +0x80 (0x0206BE8C), after each move of a fall: in the water it floats, splashing;
static bool32 Landed(struct SqueakObject *s) {
    if (SqueakInWater(s)) {
        SqueakSetState(s, BODY_FLOAT_ENTRY);
        SqueakSplash(s, FALSE);
        return TRUE;
    }
    return FALSE;
}

// Stop(); 0x02023468, and the speeds' caps, accelerations and friction (the jump's and fall's entries);
static void Still(struct SqueakObject *s) {
    s->maxVx = s->maxVy = s->ax = s->ay = s->fx = s->fy = 0;
}

static bool32 Body(struct SqueakObject *s) {
    switch (s->state) {
    case BODY_START:        // 0x0206E174 (vtable +0x3C: its start): in the water it floats, else it lies;
        if (SqueakInWater(s)) {
            SqueakSetState(s, BODY_FLOAT_ENTRY);
            return TRUE;
        }
        SqueakPlayComposite(s, MCA_FLOAT);
        SqueakStop(s);
        SqueakSetState(s, BODY_LAND);
        return TRUE;
    case BODY_LAND:         // 0x0206E0A4;
        if (s->padA) {
            SqueakSetState(s, BODY_JUMP_ENTRY);
            return TRUE;
        }
        if (s->padDir != 0 || s->padY != 0) {
            SqueakSetState(s, BODY_WRIGGLE_ENTRY);
            return TRUE;
        }
        SqueakMove(s);
        if (!OnGround(s)) SqueakSetState(s, BODY_FALL_ENTRY);
        break;
    case BODY_WRIGGLE_ENTRY:    // 0x0206DFCC;
        SqueakStop(s);
        s->vx = LAND_SPEED;
        SqueakPlayComposite(s, MCA_FLOAT);
        SqueakSetState(s, BODY_WRIGGLE);
        return TRUE;
    case BODY_WRIGGLE:      // 0x0206DECC;
        if (s->padA) {
            SqueakSetState(s, BODY_JUMP_ENTRY);
            return TRUE;
        }
        if (s->padDir == 0 && s->padY == 0) {
            SqueakSetState(s, BODY_START);
            return TRUE;
        }
        s->vx = LAND_SPEED;
        if (s->padDir < 0) SqueakTurnAround(s);
        SqueakMove(s);
        if (!OnGround(s)) SqueakSetState(s, BODY_FALL_ENTRY);
        break;
    case BODY_JUMP_ENTRY:   // 0x0206DD98: out of the water;
        SqueakPlayComposite(s, MCA_UP);
        Still(s);
        s->vy = JUMP_SPEED;
        s->ay = -GRAVITY;
        SqueakSetState(s, BODY_JUMP);
        return TRUE;
    case BODY_JUMP:         // 0x0206DD3C;
        AirControl(s);
        SqueakMove(s);
        if (s->vy < 0) SqueakSetState(s, BODY_FALL_ENTRY);
        break;
    case BODY_FALL_ENTRY:   // 0x0206DC28;
        SqueakPlay(s, ANIM_FALL);
        Still(s);
        s->ay = -GRAVITY;
        SqueakSetState(s, BODY_FALL);
        return TRUE;
    case BODY_FALL:         // 0x0206DB3C: back in the water it floats; on the ground, starts over (lies);
        if (s->padA) {
            SqueakSetState(s, BODY_JUMP_ENTRY);
            return TRUE;
        }
        AirControl(s);
        SqueakCapFall(s, MAX_FALL);
        SqueakMove(s);
        if (Landed(s)) SqueakStop(s);
        else if (OnGround(s)) SqueakSetState(s, BODY_START);
        break;
    case BODY_FLOAT_ENTRY:  // 0x0206DA78;
        SqueakPlayComposite(s, MCA_FLOAT);
        SqueakStop(s);
        SqueakSetState(s, BODY_FLOAT);
        return TRUE;
    case BODY_FLOAT:        // 0x0206D880 (it doesn't check for the water);
        if (s->padB) {
            SqueakSetState(s, BODY_DASH_ENTRY);
            return TRUE;
        }
        if (s->padY != 0 || s->padDir != 0) {
            if (s->padY == 0) SqueakSetState(s, BODY_SWIM_ENTRY);
            else if (s->padY > 0) SqueakSetState(s, s->padDir == 0 ? BODY_SWIM_UP_ENTRY : BODY_SWIM_UP_FWD_ENTRY);
            else SqueakSetState(s, s->padDir == 0 ? BODY_SWIM_DOWN_ENTRY : BODY_SWIM_DOWN_FWD_ENTRY);
            return TRUE;
        }
        SqueakMove(s);
        break;
    case BODY_SWIM_ENTRY:   // 0x0206D214;
        SqueakPlayComposite(s, MCA_FLOAT);
        SqueakStop(s);
        SqueakSetState(s, BODY_SWIM);
        return TRUE;
    case BODY_SWIM:         // 0x0206CFE0;
        if (SwimChecks(s, BODY_SWIM_ENTRY)) return TRUE;
        s->vx = SWIM_SPEED;
        if (s->padDir < 0) SqueakTurnAround(s);
        SqueakMove(s);
        break;
    case BODY_SWIM_UP_ENTRY:    // 0x0206D7A8;
        SqueakPlayComposite(s, MCA_UP);
        SqueakStop(s);
        s->vy = SWIM_SPEED;
        SqueakSetState(s, BODY_SWIM_UP);
        return TRUE;
    case BODY_SWIM_UP:      // 0x0206D5A0;
        return Swim(s, BODY_SWIM_UP_ENTRY);
    case BODY_SWIM_DOWN_ENTRY:  // 0x0206D4C4;
        SqueakPlayComposite(s, MCA_DOWN);
        SqueakStop(s);
        s->vy = -SWIM_SPEED;
        SqueakSetState(s, BODY_SWIM_DOWN);
        return TRUE;
    case BODY_SWIM_DOWN:    // 0x0206D2BC;
        return Swim(s, BODY_SWIM_DOWN_ENTRY);
    case BODY_SWIM_UP_FWD_ENTRY:    // 0x0206CF38;
        SqueakPlayComposite(s, MCA_UP);
        SqueakStop(s);
        SqueakSetState(s, BODY_SWIM_UP_FWD);
        return TRUE;
    case BODY_SWIM_UP_FWD:  // 0x0206CCEC;
        if (SwimChecks(s, BODY_SWIM_UP_FWD_ENTRY)) return TRUE;
        s->vx = s->vy = SWIM_SPEED;
        if (s->padDir < 0) SqueakTurnAround(s);
        SqueakMove(s);
        break;
    case BODY_SWIM_DOWN_FWD_ENTRY:  // 0x0206CC44;
        SqueakPlayComposite(s, MCA_DOWN);
        SqueakStop(s);
        SqueakSetState(s, BODY_SWIM_DOWN_FWD);
        return TRUE;
    case BODY_SWIM_DOWN_FWD:    // 0x0206C9F4;
        if (SwimChecks(s, BODY_SWIM_DOWN_FWD_ENTRY)) return TRUE;
        s->vx = SWIM_SPEED;
        s->vy = -SWIM_SPEED;
        if (s->padDir < 0) SqueakTurnAround(s);
        SqueakMove(s);
        break;
    case BODY_DASH_ENTRY:   // 0x0206C91C;
        SqueakPlayComposite(s, MCA_DASH);
        SqueakStop(s);
        s->vx = DASH_SPEED;
        SqueakSetState(s, BODY_DASH);
        return TRUE;
    case BODY_DASH:         // 0x0206C798: out of the water it jumps (splashing); a wall ahead bonks it; B let go, it floats;
        if (!SqueakInWater(s)) {
            SqueakSetState(s, BODY_JUMP_ENTRY);
            SqueakSplash(s, TRUE);
            return TRUE;
        }
        if (SqueakWallAhead(s)) {
            SqueakSetState(s, BODY_BONK_ENTRY);
            return TRUE;
        }
        if (!s->padB) {
            SqueakSetState(s, BODY_FLOAT_ENTRY);
            return TRUE;
        }
        s->vy = s->padY > 0 ? DASH_RISE : s->padY < 0 ? -DASH_RISE : 0;
        if (s->vx > DASH_SPEED) {
            s->vx = DASH_SPEED;
            s->ax = 0;
        }
        if (s->padDir < 0) {
            SqueakTurnAround(s);
            s->vx = -DASH_SPEED;
            s->ax = DASH_SWING;
        }
        SqueakMove(s);
        break;
    case BODY_BONK_ENTRY:   // 0x0206C6E4;
        SqueakPlay(s, ANIM_BONK);
        SqueakStop(s);
        SqueakSfx(s, 0x130);
        SqueakSetState(s, BODY_BONK);
        return TRUE;
    case BODY_BONK:         // 0x0206C62C;
        if (s->animEnded) {
            SqueakPlay(s, ANIM_BONK_BACK);
            SqueakSetState(s, BODY_BONK_BACK);
            return TRUE;
        }
        SqueakMove(s);
        break;
    case BODY_BONK_BACK:    // 0x0206C528: it springs back;
        if (s->animEnded) {
            SqueakPlay(s, ANIM_FALL);
            s->vx = -BONK_SPEED;
            s->ax = BONK_DECEL;
            SqueakSetState(s, BODY_RECOIL);
            return TRUE;
        }
        SqueakMove(s);
        break;
    case BODY_RECOIL:       // 0x0206C4C8: till it stops, then it floats;
        if (s->vx >= 0) {
            SqueakStop(s);
            SqueakSetState(s, BODY_FLOAT_ENTRY);
            return TRUE;
        }
        SqueakMove(s);
        break;
    }
    return FALSE;
}

/* LineUp(); 0x0206C270, 0x0206BFAC: Kirby within `range` across, it swims up or down to his height (no direction across), and once
 * within 2 pixels of it dashes (TRUE); */
static bool32 LineUp(struct SqueakObject *s, s32 range) {
    s32 kx = SQ_X(s->obj.kirby3), ky = SQ_Y(s->obj.kirby3), x = SQ_X(&s->obj), y = SQ_Y(&s->obj);

    s->padDir = 0;
    if (kx < x + range && kx > x - range) {
        if (ky < y + LINE_UP && ky > y - LINE_UP) {
            BRAIN_TIMER(s) = DASH_TIME;
            return TRUE;
        }
        if (y < ky) s->padY = 1;
        else if (ky < y) s->padY = -1;
    }
    return FALSE;
}

// Toward(); The direction to x across: 1 forward, -1 turn round;
static s32 Toward(struct SqueakObject *s, s32 x) {
    s32 here = SQ_X(&s->obj);

    if (here < x) return s->obj.base.flags & 1 ? -1 : 1;
    return s->obj.base.flags & 1 ? 1 : -1;
}

static void Brain(struct SqueakObject *s) {
    s32 x = SQ_X(&s->obj), y = SQ_Y(&s->obj);
    s8 home;

    switch (s->brain) {
    case BRAIN_HOME:        // 0x0206C2F0;
        HOME_X(s) = x;
        HOME_Y(s) = y;
        SqueakFaceKirby(s, FACE_DISTANCE);
        s->brain = BRAIN_LINE_UP;
        break;
    case BRAIN_LINE_UP:     // 0x0206C270;
        if (LineUp(s, DASH_RANGE)) s->brain = BRAIN_DASH;
        break;
    case BRAIN_DASH:        // 0x0206C210: forward, B, for 300 frames or till a wall;
        s->padDir = 1;
        s->padY = 0;
        s->padB = 1;
        if (SqueakWallAhead(s)) BRAIN_TIMER(s) = 0;
        if (BRAIN_TIMER(s) < 1) {
            s->padB = 0;
            s->brain = BRAIN_RETURN;
        } else {
            BRAIN_TIMER(s)--;
        }
        CLIMB(s) = 0;
        break;
    case BRAIN_RETURN:      // 0x0206C080: back to where it started, within 2 pixels each way; a wall ahead, up it (down from a ceiling);
        home = 0;
        if (HOME_Y(s) > y - LINE_UP && HOME_Y(s) < y + LINE_UP) {
            s->padY = 0;
            home = 1;
        } else if (y < HOME_Y(s)) {
            s->padY = 1;
        } else if (HOME_Y(s) < y) {
            s->padY = -1;
        }
        if (HOME_X(s) > x - LINE_UP && HOME_X(s) < x + LINE_UP) {
            s->padDir = 0;
            home++;
        } else if (x != HOME_X(s)) {
            s->padDir = Toward(s, HOME_X(s));
        }
        // (on a ceiling or the ground: there, "no direction" is forward);
        if (SqueakCeiling(s) || OnGround(s)) {
            if (HOME_X(s) > x - LINE_UP && HOME_X(s) < x + LINE_UP) {
                s->padDir = 1;
                home++;
            } else if (x != HOME_X(s)) {
                s->padDir = Toward(s, HOME_X(s));
            }
        }
        if (!SqueakWallAhead(s)) {
            CLIMB(s) = 0;
        } else {
            if (CLIMB(s) == 0) CLIMB(s) = 1;
            if (SqueakCeiling(s)) CLIMB(s) = -1;
            else if (OnGround(s)) CLIMB(s) = 1;
            if (CLIMB(s) == 1) s->padY = 1;
            else if (CLIMB(s) == -1) s->padY = -1;
        }
        if (home > 1) s->brain = BRAIN_FACE;
        break;
    case BRAIN_FACE:        // 0x0206C060;
        SqueakFaceKirby(s, FACE_DISTANCE);
        s->brain = BRAIN_LINE_UP;
        break;
    case BRAIN_HOME_1:      // 0x0206C02C;
        HOME_X(s) = x;
        HOME_Y(s) = y;
        SqueakFaceKirby(s, FACE_DISTANCE);
        s->brain = BRAIN_LINE_UP_1;
        break;
    case BRAIN_LINE_UP_1:   // 0x0206BFAC;
        if (LineUp(s, DASH_RANGE_1)) s->brain = BRAIN_DASH_1;
        break;
    case BRAIN_DASH_1:      // 0x0206BF48;
        s->padDir = 1;
        s->padY = 0;
        s->padB = 1;
        if (SqueakWallAhead(s)) BRAIN_TIMER(s) = 0;
        if (BRAIN_TIMER(s) < 1) {
            BRAIN_TIMER(s) = REST_TIME;
            s->padDir = 0;
            s->padY = 0;
            s->padB = 0;
            s->brain = BRAIN_REST_1;
        } else {
            BRAIN_TIMER(s)--;
        }
        break;
    case BRAIN_REST_1:      // 0x0206BF08;
        if (BRAIN_TIMER(s) < 1) {
            SqueakFaceKirby(s, FACE_DISTANCE);
            BRAIN_TIMER(s) = DASH_TIME;
            s->brain = BRAIN_DASH_1;
        } else {
            BRAIN_TIMER(s)--;
        }
        break;
    }
}

static void GussaMain(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    Brain(s);
    SqueakRun(s, Body);
    SqueakFinishFrame(s);
    SqueakAnimate(s);
}

void GussaStart(struct Object *obj) {
    struct SqueakObject *s = (struct SqueakObject *)obj;

    ObjectSetFunc(obj, 0, GussaMain);
    // (after a hit only the body starts over, at vtable +0x3C; the brain goes on; 0x0206C3D0: variant 1 its own, any other the first);
    if (!s->started) s->brain = s->variant == 1 ? BRAIN_HOME_1 : BRAIN_HOME;
    SqueakSetState(s, BODY_START);
}

/* GussaInit(); Its box (gussa.rca's for its float and swims, box 4: 6 pixels behind to 10 ahead, 2 to 17 up; as the others', from
 * its feet), faces left (the body's constructor sets flag 4), and whether it starts in the water (its start asks); */
void GussaInit(struct SqueakObject *s) {
    sub_0803E2B0(&s->obj.base, -6, -15, 10, 0);
    ObjectSetBounds(&s->obj.base, -6, -15, 10, 0);
    s->obj.base.flags |= 1;
    if (NidlIsWater(&s->obj, 0)) s->water = 2;
}
