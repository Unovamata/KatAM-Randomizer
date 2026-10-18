/* Grand Wheelie; ported from Kirby: Nightmare in Dream Land (USA), mini-boss object type 0x33 (spawn 0x080941AC), as NiDL
 * mini-boss NIDL_GRAND_WHEELIE. Its frames (0-55 of 0x08752F74) are streamed frames (see KatAMExpansion.cs), played by its own
 * animation tables (0x080956C8: pairs of frame and frames, -3 loops, -2 holds).
 * NiDL script (11 states, 0x08744444; 16.16 speeds, Y down; KatAM's are 8.8, Y up; gravity 0x2500, falling at 3.0 at most):
 * • State 2 (0x080946B0): revs in place for a while, rolling slowly (0x0874433C) or rocking on its tire (0x087443A8, 1.0 back and
 *   forth every 2 frames), then rolls (state 4) three times in 4, else hops at Kirby (state 3: 0x08744360).
 * • State 4 (0x08094930): faces Kirby, revs (0x08744384) for 45 frames and rolls at it (1.5; 2.625, 0.9, 1.57 for variants 1-3,
 *   0x087444A4). Heading at Kirby, once it's within 63 pixels it may skid and turn back (state 7, one time in 3); a wall knocks it
 *   back (state 9: a quake, 1.0 back and 3.0 up, tumbling 0x08744408, then 0.5 back and 2.0 up, spinning 0x0874441C); past
 *   Kirby, it skids (0x087443D0) and rolls back.
 * • Every 4th decision (0x08094810), state 8: two hops in place (3.0, 1.5 up), spinning (0x087443E4), then it spits out a small
 *   Wheelie (here KatAM's own Wheelie enemy, which gives Wheel too).
 * • States 0, 1 (it rolls in) and 10 (defeated) are KatAM's own here; */
#include "../nidl.h"
#include "code_0806F780.h"
#include "task.h"

// Frames: NiDL's 0-55 (its small Wheelies' 56-66 aren't used). Its hurt frame is NiDL's 35;
#define FRAME_FRONT 4

#define GRAVITY 0x25
#define MAX_FALL 0x300
#define REV_TIME 45
#define TURN_RANGE 63
#define SKID_TIME 60
#define SKID_SLOWDOWN 6

enum { STATE_REV, STATE_HOP, STATE_ROLL_REV, STATE_ROLL, STATE_SKID, STATE_TURN, STATE_BUMP, STATE_SPIT };

// NiDL's animation tables;
static const s16 sRollSlow[] = { 18, 3, 10, 4, 18, 3, 12, 4, 18, 3, 14, 4, 18, 3, 16, 4, NIDL_LOOP };     // 0x0874433C;
static const s16 sRollMedium[] = { 18, 2, 10, 3, 18, 2, 12, 3, 18, 2, 14, 3, 18, 2, 16, 3, NIDL_LOOP };   // 0x08744360;
static const s16 sRollFast[] = { 18, 1, 10, 2, 18, 1, 12, 2, 18, 1, 14, 2, 18, 1, 16, 2, NIDL_LOOP };     // 0x08744384;
static const s16 sRock[] = { 4, 3, 8, 2, 6, 3, 8, 3, NIDL_LOOP };                                         // 0x087443A8;
static const s16 sSkid[] = { 27, 16, 31, 4, 29, 4, 21, 4, NIDL_END };                                     // 0x087443D0;
static const s16 sSpin[] = { 36, 4, 41, 4, 51, 2, 49, 2, 47, 4, 42, 4, 54, 2, 48, 2, NIDL_LOOP };        // 0x087443E4;
static const s16 sTumble[] = { 32, 8, 35, 8, 27, 8, 53, 8, NIDL_END };                                    // 0x08744408;

static const s16 sRollSpeeds[] = { 0x180, 0x2A0, 0xE6, 0x192 };     // 0x087444A4;

struct GrandWheelie {
    struct NidlObject n;
    u8 state;
    u8 count;           // Decisions (every 4th it spits a Wheelie);
    u8 step;
    bool8 airborne;
    bool8 checked;      // It already decided whether to turn back as it neared Kirby;
    bool8 rocking;
    u16 timer;
    s16 speed;
};

typedef char sGrandWheelieFits[sizeof(struct GrandWheelie) <= NIDL_OBJECT_SIZE ? 1 : -1];

static void SetState(struct GrandWheelie *g, u8 state);

static void Forward(struct Object *obj, s16 speed) {
    obj->base.xspeed = (obj->base.flags & 1) ? -speed : speed;
}

static bool8 Landed(struct GrandWheelie *g) {
    struct Object *obj = &g->n.obj;

    if (!(obj->base.unk62 & 4)) g->airborne = TRUE;
    return g->airborne && (obj->base.unk62 & 4) && obj->base.yspeed <= 0;
}

static void Leap(struct GrandWheelie *g, s16 height) {
    g->n.obj.base.yspeed = height;
    g->airborne = FALSE;
}

// KirbyAhead(); Kirby is the way it faces;
static bool8 KirbyAhead(struct Object *obj) {
    return (obj->kirby3->base.x < obj->base.x) == ((obj->base.flags & 1) != 0);
}

static s32 KirbyDistance(struct Object *obj) {
    return abs(obj->kirby3->base.x - obj->base.x) >> 8;
}

// Spit(); A small Wheelie (KatAM's), ahead of it;
static void Spit(struct GrandWheelie *g) {
    struct Object *obj = &g->n.obj;
    struct Object *wheelie = CreateObjTemplateAndObj(obj->base.unk56, 1, 0x24, (obj->base.x >> 8) + ((obj->base.flags & 1) ? -16 : 16),
                                                     obj->base.y >> 8, 0, 0x1F, 0, 0, OBJ_WHEELIE, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    if (wheelie && (obj->base.flags & 1)) wheelie->base.flags |= 1;
}

static void SetState(struct GrandWheelie *g, u8 state) {
    struct Object *obj = &g->n.obj;

    g->state = state;
    g->step = 0;
    switch (state) {
    case STATE_REV:
        // 0x08094810: every 4th decision it spits a Wheelie;
        g->count = (g->count + 1) & 3;
        if (g->count == 3) {
            SetState(g, STATE_SPIT);
            return;
        }
        obj->base.xspeed = 0;
        g->rocking = Rand16() & 1;
        NidlPlay(&g->n, g->rocking ? sRock : sRollSlow);
        g->timer = 30 + (Rand16() & 31);
        break;
    case STATE_HOP:
        NidlFaceKirby(obj);
        obj->base.xspeed = 0;
        NidlPlay(&g->n, sRollMedium);
        Leap(g, 0x180);
        break;
    case STATE_ROLL_REV:
        NidlFaceKirby(obj);
        obj->base.xspeed = 0;
        NidlPlay(&g->n, sRollFast);
        g->timer = REV_TIME;
        PlaySfx(&obj->base, SE_MINIBOSS_RUN);
        break;
    case STATE_ROLL:
        g->speed = sRollSpeeds[g->n.variant & 3];
        Forward(obj, g->speed);
        g->checked = FALSE;
        break;
    case STATE_SKID:
        NidlPlay(&g->n, sSkid);
        g->timer = SKID_TIME;
        PlaySfx(&obj->base, SE_MINIBOSS_RUN);
        break;
    case STATE_BUMP:
        RequestScreenShake(3, &obj->base);
        PlaySfx(&obj->base, SE_BOSS_GROUND_POUND_ATTACK);
        Forward(obj, -0x100);
        Leap(g, 0x300);
        NidlPlay(&g->n, sTumble);
        break;
    case STATE_SPIT:
        obj->base.xspeed = 0;
        NidlPlay(&g->n, sSpin);
        Leap(g, 0x300);
        break;
    }
}

static void GrandWheelieMain(struct Object *obj) {
    struct GrandWheelie *g = (struct GrandWheelie *)obj;

    NidlUpdate(&g->n);
    NidlAnimate(&g->n);
    obj->base.flags |= 4;
    obj->base.yspeed -= GRAVITY;
    if (obj->base.yspeed < -MAX_FALL) obj->base.yspeed = -MAX_FALL;

    switch (g->state) {
    case STATE_REV:
        // Rocking, it rocks back and forth (1.0 every 2 frames);
        if (g->rocking) obj->base.xspeed = (g->timer & 2) ? 0x100 : -0x100;
        if (--g->timer) break;
        SetState(g, (Rand16() & 3) ? STATE_ROLL_REV : STATE_HOP);
        break;
    case STATE_HOP:
        if (Landed(g)) SetState(g, STATE_ROLL_REV);
        break;
    case STATE_ROLL_REV:
        if (!--g->timer) SetState(g, STATE_ROLL);
        break;
    case STATE_ROLL:
        if ((obj->base.unk62 & 3) && abs(obj->base.x - obj->base.unk48) < g->speed / 2) {
            SetState(g, STATE_BUMP);
            break;
        }
        if (KirbyAhead(obj)) {
            // Nearing Kirby, it may skid and turn back, one time in 3;
            if (!g->checked && KirbyDistance(obj) <= TURN_RANGE) {
                g->checked = TRUE;
                if ((Rand16() & 0xFF) < 0x55) SetState(g, STATE_SKID);
            }
        } else if (KirbyDistance(obj) > TURN_RANGE) {
            SetState(g, STATE_SKID);
        }
        break;
    case STATE_SKID:
        if (g->speed > SKID_SLOWDOWN) g->speed -= SKID_SLOWDOWN;
        else g->speed = 0;
        Forward(obj, g->speed);
        if (--g->timer) break;
        obj->base.flags ^= 1;
        NidlPlay(&g->n, sRollFast);
        SetState(g, STATE_ROLL);
        break;
    case STATE_BUMP:
        // Two bounces back: 3.0 up tumbling, then 2.0 up spinning; then 30 frames;
        if (g->step == 2) {
            if (!--g->timer) SetState(g, STATE_REV);
            break;
        }
        if (!Landed(g)) break;
        if (g->step++ == 0) {
            Forward(obj, -0x80);
            Leap(g, 0x200);
            NidlPlay(&g->n, sSpin);
        } else {
            obj->base.xspeed = 0;
            NidlPlay(&g->n, sRollSlow);
            g->timer = 30;
        }
        break;
    case STATE_SPIT:
        // Hops (3.0, 1.5 up), then 16 frames, spits a Wheelie and 20 more;
        if (g->step == 2) {
            if (--g->timer == 20) {
                PlaySfx(&obj->base, SE_PRANK_THROW_ITEM);
                Spit(g);
            }
            if (!g->timer) SetState(g, STATE_REV);
            break;
        }
        if (!Landed(g)) break;
        if (g->step++ == 0) {
            Leap(g, 0x180);
        } else {
            NidlPlay(&g->n, sRollSlow);
            g->timer = 36;
        }
        break;
    }
}

// GrandWheelieStart(); The fight starts, and again after a hit;
void GrandWheelieStart(struct Object *obj) {
    struct GrandWheelie *g = (struct GrandWheelie *)obj;

    ObjectSetFunc(obj, 0, GrandWheelieMain);
    obj->base.yspeed = 0;
    SetState(g, STATE_REV);
}

// GrandWheelieInit(); Its box, on the floor: its terrain box ends where its wheel is drawn (15 below its spot; 18 floated it 3 pixels);
void GrandWheelieInit(struct NidlObject *n) {
    struct GrandWheelie *g = (struct GrandWheelie *)n;
    struct Object *obj = &n->obj;

    g->count = 0;
    NidlSetFrame(n, FRAME_FRONT);
    sub_0803E2B0(&obj->base, -16, -16, 16, 15);
    ObjectSetBounds(&obj->base, -18, -16, 18, 17);
    NidlOnFloor(obj);
}
