/* Slippy; ported from Kirby: Nightmare in Dream Land (USA), object type 0x22 (spawn 0x08089C58), as NiDL enemy NIDL_SLIPPY.
 * NiDL scripts on land (16.16 speeds, Y down; KatAM's are 8.8, Y up):
 * • 0x08089DA8: sits facing Kirby (frame 4) for 60 frames, then decides (0x0808A7F4, p = 15).
 * • 0x08089EC8: looks around 3 times (frame 4 for 4 frames, then turning around, or blinking with frame 5, at random for the
 *   3 of them, for 16), then decides (p = 11 or 15, every other time).
 * • Deciding (0x0808A84C): r = random 0-p; the choice is the first of 6 thresholds r doesn't pass, from one table one time,
 *   the other the next (0x08742856: 3, 4, 8, 9, 11, 16; 0x0874285C: 1, 2, 9, 13, 15, 16). Choices: jump low, jump high, jump
 *   low forward (1.0), jump high forward, jump low forward turning around, look around.
 * • Low jump 0x0808A048: -5.0, gravity 0.5. High jump 0x0808A0AC: -6.0, gravity 0.25. Frame 8 going up, 6 for 2 frames at the
 *   top, then 5; landing it sits again.
 * NiDL scripts in water (states 4-10; it starts swimming when it's put in water, 0x08089C98):
 * • 0x0808A298 (6): drifts at 0.5 the way it heads for 26 frames (0x08742822; 3 times in 4, 30 more), paddles (frames 4, 5, 6, 5
 *   and 9: 3, 3, 4, 2, 3 frames), then kicks, one of 3 at random:
 * • 0x0808A3C4 (7): straight towards Kirby (0x0808A880: the main way, 8 ways in all, table 0x08742862) at 3.0, 2.0, 1.0 (8 frames
 *   each, frame 7, 0x08742824);
 * • 0x0808A4D0 (8): frame 9 for 10 frames (0x08742830), then the other way towards Kirby at 2.25, 2.0, 1.0, 0.5 (0x08742834);
 *   once more one time in 3;
 * • 0x0808A610 (9): frame 9 for 10 frames, then diagonally towards Kirby at 3.2, 2.83, 1.41, 0.71 (0x08742844); then drifting 2
 *   times in 3, else 8.
 * • Out of the water (8 pixels above it isn't water, 0x0808A36C) it leaps out (0x0808A110: bobbing 4 frames each at 2.0 down, 1.0
 *   down, 1.0 up, 2.0 up, then 5.0 up, 1.0 forward towards Kirby, gravity 0.25, frames 8, 6 for 2, 5) and sits when it lands.
 *   Jumping into water (0x0808A204, 5) it sinks for 40 frames (frame 5) heading diagonally down, then drifts.
 * • Variant 1 (0x0808A7A8): sits facing Kirby (frame 4) and never does anything else; */
#include "../nidl.h"

// Frames: NiDL's 4 (sitting), 5 (blinking, falling), 6 (top of a jump), 8 (jumping), 7 (swimming), 9 (paddling). Its hurt frame is
// NiDL's 3;
enum { FRAME_SIT, FRAME_BLINK, FRAME_TOP, FRAME_JUMP, FRAME_SWIM, FRAME_PADDLE };

static const u8 sThresholds[2][6] = { { 3, 4, 8, 9, 11, 16 }, { 1, 2, 9, 13, 15, 16 } };  // 0x08742856, 0x0874285C;
static const u8 sLookP[2] = { 11, 15 };                                                     // 0x08742854;

enum { CHOICE_JUMP, CHOICE_JUMP_HIGH, CHOICE_JUMP_FORWARD, CHOICE_JUMP_HIGH_FORWARD, CHOICE_JUMP_BACK, CHOICE_LOOK };
enum { STATE_SIT, STATE_LOOK, STATE_JUMP, STATE_DRIFT, STATE_KICK_WAIT, STATE_KICK, STATE_SINK, STATE_BOB, STATE_LEAP, STATE_STILL };

// Swimming;
static const u16 sHeadings[3][8] = {                // 0x08742862: per Kirby's octant (NidlKirbyOctant);
    { 128, 0, 256, 128, 384, 256, 0, 384 },         // The main way;
    { 0, 128, 128, 256, 256, 384, 384, 0 },         // The other way;
    { 64, 64, 192, 192, 320, 320, 448, 448 },       // Diagonally;
};
static const s16 sKickSpeeds[3][4] = { { 0x300, 0x200, 0x100, 0 }, { 0x240, 0x200, 0x100, 0x80 }, { 0x333, 0x2D4, 0x16A, 0xB5 } };
static const s16 sPaddle[] = { FRAME_SIT, 3, FRAME_BLINK, 3, FRAME_TOP, 4, FRAME_BLINK, 2, FRAME_PADDLE, 3, NIDL_END };
static const s16 sBob[] = { -0x200, -0x100, 0x100, 0x200 };      // 0x0808A110 (Y up);
#define DRIFT_SPEED 0x80
#define DRIFT_TIME 26
#define KICK_STEP 8
#define KICK_WAIT 10
#define SINK_TIME 40

#define SPEED 0x100
#define MAX_FALL 0x400
#define SIT_TIME 60
#define LOOKS 3

struct Slippy {
    struct NidlObject n;
    s16 gravity;
    u8 state;
    u8 timer;
    u8 decisions;
    u8 looks;
    u8 lookAround;  // Turning around (else blinking) this time;
    u8 airborne;
    u8 kick;        // 0, 1, 2: kicks 7, 8, 9;
    u8 step;
    u16 heading;
};

// Decide(); 0x0808A7F4: one of the 6 choices (p = 0 takes it from 0x08742854);
static u8 Decide(struct Slippy *s, u8 p) {
    u8 table, r, choice;

    s->decisions++;
    if (!p) p = sLookP[s->decisions & 1];
    table = (s->decisions & 1) ? 1 : 0;
    r = Rand16() % (p + 1);
    for (choice = 0; sThresholds[table][choice] < r; choice++);
    return choice;
}

static void Sit(struct Slippy *s) {
    s->state = STATE_SIT;
    s->timer = SIT_TIME;
    s->n.obj.base.xspeed = 0;
    NidlFaceKirby(&s->n.obj);
    NidlSetFrame(&s->n, FRAME_SIT);
}

static void Look(struct Slippy *s) {
    s->state = STATE_LOOK;
    s->looks = 0;
    s->lookAround = Rand16() % 3 != 0;
    s->timer = 4;
    NidlSetFrame(&s->n, FRAME_SIT);
}

static void Jump(struct Slippy *s, u8 choice) {
    struct Object *obj = &s->n.obj;
    bool8 high = choice == CHOICE_JUMP_HIGH || choice == CHOICE_JUMP_HIGH_FORWARD;

    s->state = STATE_JUMP;
    s->airborne = FALSE;
    obj->base.yspeed = high ? 0x600 : 0x500;
    s->gravity = high ? 0x40 : 0x80;
    if (choice == CHOICE_JUMP_BACK) obj->base.flags ^= 1;
    if (choice >= CHOICE_JUMP_FORWARD) obj->base.xspeed = (obj->base.flags & 1) ? -SPEED : SPEED;
    NidlSetFrame(&s->n, FRAME_JUMP);
}

static void Act(struct Slippy *s, u8 choice) {
    if (choice == CHOICE_LOOK)
        Look(s);
    else
        Jump(s, choice);
}

// Drift(); 0x0808A298;
static void Drift(struct Slippy *s) {
    struct Object *obj = &s->n.obj;

    s->state = STATE_DRIFT;
    s->timer = DRIFT_TIME + (Rand16() % 4 ? 30 : 0);
    s->step = 0;
    NidlSetHeading(obj, s->heading, DRIFT_SPEED);
    s->n.sequence = NULL;
    NidlSetFrame(&s->n, FRAME_SWIM);
}

// Kick(); 0x0808A3C4, 0x0808A4D0, 0x0808A610: 1 and 2 wait paddling first;
static void Kick(struct Slippy *s, u8 kick) {
    struct Object *obj = &s->n.obj;

    s->kick = kick;
    s->step = 0;
    obj->base.xspeed = obj->base.yspeed = 0;
    if (kick) {
        s->state = STATE_KICK_WAIT;
        s->timer = KICK_WAIT;
        NidlSetFrame(&s->n, FRAME_PADDLE);
    } else {
        s->state = STATE_KICK;
        s->timer = 1;
    }
}

// Leap(); 0x0808A110: out of the water;
static void Leap(struct Slippy *s) {
    struct Object *obj = &s->n.obj;

    s->state = STATE_BOB;
    s->step = 0;
    s->timer = 1;
    NidlFaceKirby(obj);
    obj->base.xspeed = 0;
    s->n.sequence = NULL;
    NidlSetFrame(&s->n, FRAME_BLINK);
}

// Swim(); The swimming states; false when it isn't in one;
static bool8 Swim(struct Slippy *s) {
    struct Object *obj = &s->n.obj;

    switch (s->state) {
    case STATE_DRIFT:
    case STATE_KICK_WAIT:
    case STATE_KICK:
        // 0x0808A36C: out of the water, it leaps (NiDL looks 8 pixels up; KatAM's swimmers stop in the surface's 16 pixel tile);
        if (!NidlIsWater(obj, -16)) {
            Leap(s);
            return TRUE;
        }
        break;
    }

    switch (s->state) {
    case STATE_DRIFT:
        NidlAnimate(&s->n);
        if (s->timer && --s->timer) break;
        if (!s->step++) {
            NidlPlay(&s->n, sPaddle);
            s->timer = 3 + 3 + 4 + 2 + 3;
        } else {
            Kick(s, Rand16() % 3);
        }
        break;
    case STATE_KICK_WAIT:
        if (--s->timer) break;
        s->state = STATE_KICK;
        s->timer = 1;
        break;
    case STATE_KICK:
        if (--s->timer) break;
        if (s->step == 0) {
            s->heading = sHeadings[s->kick][NidlKirbyOctant(obj)];
            NidlSetFrame(&s->n, FRAME_SWIM);
        }
        if (s->step < 4 && sKickSpeeds[s->kick][s->step]) {
            NidlSetHeading(obj, s->heading, sKickSpeeds[s->kick][s->step++]);
            s->timer = KICK_STEP;
            break;
        }
        // Then: 7 drifts; 8 kicks again one time in 3; 9 drifts 2 times in 3, else kicks like 8;
        if (s->kick == 1 && Rand16() % 3 == 0) Kick(s, 1);
        else if (s->kick == 2 && Rand16() % 3 == 0) Kick(s, 1);
        else Drift(s);
        break;
    case STATE_SINK:
        // 0x0808A204: sinking diagonally, then drifting;
        obj->base.yspeed = -DRIFT_SPEED;
        if (!--s->timer) Drift(s);
        break;
    case STATE_BOB:
        if (--s->timer) break;
        if (s->step < ARRAY_COUNT(sBob)) {
            obj->base.yspeed = sBob[s->step++];
            s->timer = 4;
            break;
        }
        NidlFaceKirby(obj);
        s->state = STATE_LEAP;
        s->airborne = FALSE;
        obj->base.yspeed = 0x500;
        s->gravity = 0x40;
        obj->base.xspeed = (obj->base.flags & 1) ? -SPEED : SPEED;
        NidlSetFrame(&s->n, FRAME_JUMP);
        break;
    case STATE_LEAP:
        s->state = STATE_JUMP;
        return FALSE;
    case STATE_STILL:
        NidlFall(obj);
        return TRUE;
    default:
        return FALSE;
    }
    if (s->state != STATE_BOB) NidlKeepInLevel(obj, 9);
    return TRUE;
}

// Sink(); 0x0808A204: into the water from a jump;
static void Sink(struct Slippy *s) {
    struct Object *obj = &s->n.obj;

    s->state = STATE_SINK;
    s->timer = SINK_TIME;
    s->heading = (obj->base.flags & 1) ? 192 : 64;
    obj->base.xspeed = (obj->base.flags & 1) ? -DRIFT_SPEED : DRIFT_SPEED;
    NidlSetFrame(&s->n, FRAME_BLINK);
}

static void SlippyMain(struct Object *obj) {
    struct Slippy *s = (struct Slippy *)obj;

    NidlUpdate(&s->n);
    obj->base.flags |= 4;
    if (Swim(s)) return;
    if (s->state != STATE_JUMP) NidlFall(obj);

    switch (s->state) {
    case STATE_SIT:
        if (!--s->timer) Act(s, Decide(s, 15));
        break;
    case STATE_LOOK:
        // 4 frames sitting, then 16 turned around or blinking, 3 times;
        if (--s->timer) break;
        if (++s->looks == LOOKS * 2) {
            Act(s, Decide(s, 0));
        } else if (s->looks & 1) {
            if (s->lookAround)
                obj->base.flags ^= 1;
            else
                NidlSetFrame(&s->n, FRAME_BLINK);
            s->timer = 16;
        } else {
            NidlSetFrame(&s->n, FRAME_SIT);
            s->timer = 4;
        }
        break;
    case STATE_JUMP:
        obj->base.yspeed -= s->gravity;
        if (obj->base.yspeed < -MAX_FALL) obj->base.yspeed = -MAX_FALL;
        if (!(obj->base.unk62 & 4)) s->airborne = TRUE;
        if (obj->base.yspeed <= 0 && s->n.obj.unk83 == s->n.animBase + FRAME_JUMP) {
            NidlSetFrame(&s->n, FRAME_TOP);
            s->timer = 2;
        } else if (s->timer && !--s->timer) {
            NidlSetFrame(&s->n, FRAME_BLINK);
        }
        if (s->airborne && (obj->base.unk62 & 4) && obj->base.yspeed <= 0) {
            obj->base.yspeed = 0;
            Sit(s);
        } else if (s->airborne && obj->base.yspeed < 0 && NidlIsWater(obj, -8)) {
            Sink(s);
        }
        break;
    }
}

// SlippyStart(); Behavior start, also called by the engine after Slippy recovers from a hit;
void SlippyStart(struct Object *obj) {
    struct Slippy *s = (struct Slippy *)obj;

    ObjectSetFunc(obj, 0, SlippyMain);
    obj->base.yspeed = 0;
    if (s->n.variant == 1) {
        s->state = STATE_STILL;
        obj->base.xspeed = 0;
        NidlFaceKirby(obj);
        NidlSetFrame(&s->n, FRAME_SIT);
    } else if (NidlIsWater(obj, -8)) {
        // 0x08089C98: in water it swims, heading the way it faces;
        NidlFaceKirby(obj);
        s->heading = (obj->base.flags & 1) ? 256 : 0;
        Drift(s);
    } else {
        Sit(s);
    }
}

// SlippyInit(); Its box, on the floor;
void SlippyInit(struct NidlObject *n) {
    struct Object *obj = &n->obj;

    sub_0803E2B0(&obj->base, -6, -5, 6, 7);
    ObjectSetBounds(&obj->base, -7, -6, 7, 9);
    NIDL_SWIMS(obj);
    if (!NidlIsWater(obj, -8))
        obj->base.unk4C = obj->base.y = ((obj->base.y + (obj->base.unk3F * 0x100)) & 0xfffff000) - (obj->base.unk3F * 0x100) - 1;
}
