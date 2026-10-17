/* Bugzzy; ported from Kirby: Nightmare in Dream Land (USA), mini-boss object type 0x36 (spawn 0x08091F08), as NiDL mini-boss
 * NIDL_BUGZZY. Its frames (0-63 of 0x087535FC) are streamed frames (see KatAMExpansion.cs).
 * NiDL script (13 states, 0x08743AE0; timings and speeds traced; 16.16 speeds, Y down; KatAM's are 8.8, Y up):
 * • State 1: walks (frames 4-16), then charges, jumps or calls for help.
 * • State 3, the charge: backs up a little (17, 18), raises its pincers (19-23) and dashes at 2.25 (24 and 26); touching Kirby, it
 *   grabs it (state 10).
 * • States 6 and 8, the jump: crouches (60, 59), leaps at Kirby (3.75 up, 1.0 forward; 52 and 61 rising, 58 and 57 falling);
 *   landing on Kirby, it grabs it.
 * • State 10, the suplex: holds Kirby (36, 37), jumps back with it over its head (45, 46, 49, 47) and slams it behind itself (48),
 *   which hurts Kirby; then gets up (state 11: 48, 50, 43).
 * • State 2: calls for help (29-33): a Bronto Burt or a Waddle Doo comes (NiDL types 0x14, 0x11; KatAM's own here).
 * Kirby is held like KatAM's Phan Phan holds it (0x080CE94C): Kirby follows a table of positions and poses relative to the holder
 * (kirby->unk110), restarting it whenever the holder's frame changes; Bugzzy points it at the entry for its current frame; */
#include "../nidl.h"
#include "code_0806F780.h"
#include "task.h"

void sub_0804E09C(struct Kirby *);

// Frames: NiDL's 0-63. Its hurt frame is NiDL's 3;
#define FRAME_WALK 4
#define FRAME_CALL 29
#define FRAME_GRAB 36
#define FRAME_SLAM 48

#define GRAVITY 0x24
#define MAX_FALL 0x300
#define WALK_SPEED 0x40
#define DASH_SPEED 0x240
#define DASH_TIME 44
#define JUMP_HEIGHT 0x3C0
#define JUMP_SPEED 0x100
#define SUPLEX_HEIGHT 0x2C0
#define SUPLEX_SPEED 0x180
#define SUPLEX_SLOW 0x80

enum { STATE_WALK, STATE_CHARGE, STATE_DASH, STATE_STOP, STATE_JUMP_CROUCH, STATE_JUMP, STATE_CALL, STATE_GRAB, STATE_SUPLEX, STATE_GET_UP };

static const s16 sWalk[] = { 4, 7, 5, 7, 6, 7, 7, 7, 8, 7, 16, 7, 15, 7, 14, 7, 13, 7, 12, 7, 11, 7, 10, 7, 9, 7, NIDL_LOOP };
static const s16 sCharge[] = { 17, 2, 18, 2, 17, 2, 18, 2, 17, 2, 18, 2, 19, 2, 20, 2, 21, 2, 22, 2, 21, 2, 22, 2, 21, 2, 22, 2, 23, 12, NIDL_END };
static const s16 sDash[] = { 24, 2, 26, 2, NIDL_LOOP };
static const s16 sCrouch[] = { 60, 4, 59, 10, NIDL_END };
static const s16 sRise[] = { 52, 2, 61, 2, NIDL_LOOP };
static const s16 sFall[] = { 58, 2, 57, 2, NIDL_LOOP };
static const s16 sCall[] = { 29, 12, 30, 8, 31, 5, 32, 5, 33, 28, 32, 4, 31, 4, NIDL_END };
static const s16 sGrab[] = { 36, 5, 37, 8, NIDL_END };
static const s16 sSuplex[] = { 45, 4, 46, 4, 49, 4, 47, 4, 48, 1, NIDL_END };
static const s16 sGetUp[] = { 48, 14, 50, 8, 43, 16, NIDL_END };
#define CHARGE_TIME 42
#define CALL_TIME 66
#define GRAB_TIME 13
#define GET_UP_TIME 38

/* Kirby held: one entry per frame of the hold (x ahead of Bugzzy and y down, in pixels; sprite order; frames; Kirby's animation;
 * flags: 0x400 keeps it held whatever Bugzzy's frame), each followed by an end the hold never reaches; */
static const struct Kirby_110 sHold[][2] = {
    { { 20, -6, 1, 0xFF, 0x57, 0x400 }, { 0, 0, 0, 1, 0, 0x400 } },     // 36, 37: in its pincers;
    { { 14, -26, -1, 0xFF, 0x52, 0x400 }, { 0, 0, 0, 1, 0, 0x400 } },   // 45: lifted;
    { { 0, -38, -1, 0xFF, 0x54, 0x400 }, { 0, 0, 0, 1, 0, 0x400 } },    // 46: over its head;
    { { -14, -30, -1, 0xFF, 0x55, 0x400 }, { 0, 0, 0, 1, 0, 0x400 } },  // 49;
    { { -22, -14, -1, 0xFF, 0x53, 0x400 }, { 0, 0, 0, 1, 0, 0x400 } },  // 47;
    { { -26, 4, -1, 0xFF, 0x56, 0x400 }, { 0, 0, 0, 1, 0, 0x400 } },    // 48: slammed behind it;
};
// Slammed: thrown back hurt (0x10: 0x0804EA18), or let go when Bugzzy is hit (2: 0x0804F32C);
static const struct Kirby_110 sSlam[] = { { -0x20, 0x20, 0, 1, 0, 0x10 }, { 0, 0, 0, 1, 0, 0 } };
static const struct Kirby_110 sLetGo[] = { { -0x10, 0x20, 0, 1, 0, 2 }, { 0, 0, 0, 1, 0, 0 } };

struct Bugzzy {
    struct NidlObject n;
    u8 state;
    u8 hold;            // Entry of sHold Kirby follows;
    bool8 airborne;
    bool8 callBronto;   // The next call brings a Bronto Burt (else a Waddle Doo);
    u8 next;            // What follows the walk;
    u16 timer;
    struct Kirby *held;
    struct Object *helper;
};

typedef char sBugzzyFits[sizeof(struct Bugzzy) <= NIDL_OBJECT_SIZE ? 1 : -1];

static void SetState(struct Bugzzy *b, u8 state);

static void Forward(struct Object *obj, s16 speed) {
    obj->base.xspeed = (obj->base.flags & 1) ? -speed : speed;
}

static bool8 Landed(struct Bugzzy *b) {
    struct Object *obj = &b->n.obj;

    if (!(obj->base.unk62 & 4)) b->airborne = TRUE;
    return b->airborne && (obj->base.unk62 & 4) && obj->base.yspeed <= 0;
}

// CanGrab(); Kirby is in its pincers' reach (ahead, or under it when it lands; before their boxes touch) and can be grabbed (as Phan Phan's, 0x080CE94C);
static bool8 CanGrab(struct Object *obj, struct Kirby *kirby) {
    s32 dx = (kirby->base.x - obj->base.x) >> 8, dy = (kirby->base.y - obj->base.y) >> 8;

    if (obj->base.flags & 1) dx = -dx;
    if (dx < -24 || dx > 40 || dy < -28 || dy > 40) return FALSE;
    return !kirby->base.header.kind && kirby->hp > 0 && kirby->animationIndex != 0x27 && kirby->animationIndex <= 0x7A
        && !kirby->unk110 && !(kirby->base.flags & 0x3800B00) && kirby->base.roomId == obj->base.roomId;
}

// Hold(); Kirby follows the entry for Bugzzy's next frame;
static void Hold(struct Bugzzy *b, u8 entry) {
    if (b->held) b->held->unk110 = sHold[entry];
}

static void Grab(struct Bugzzy *b, struct Kirby *kirby) {
    struct Object *obj = &b->n.obj;

    b->held = kirby;
    kirby->base.unk6C = obj;
    kirby->unk110 = sHold[0];
    obj->base.unk6C = kirby;
    kirby->base.flags |= 0x40000;
    sub_0804E09C(kirby);
    PlaySfx(&obj->base, SE_MINIBOSS_GRAB);
    SetState(b, STATE_GRAB);
}

// Release(); Lets Kirby go: slammed (hurt) or just dropped;
static void Release(struct Bugzzy *b, const struct Kirby_110 *how) {
    if (!b->held) return;
    if (b->held->unk110 && b->held->base.unk6C == &b->n.obj) b->held->unk110 = how;
    b->held = NULL;
}

static bool8 TryGrab(struct Bugzzy *b) {
    struct Object *obj = &b->n.obj;

    obj->kirby3 = FindTargetKirby(&obj->base);
    if (!CanGrab(obj, obj->kirby3)) return FALSE;
    Grab(b, obj->kirby3);
    return TRUE;
}

// Call(); A helper comes (once the last one is gone): a Bronto Burt or a Waddle Doo, in turn, ahead of it;
static void Call(struct Bugzzy *b) {
    struct Object *obj = &b->n.obj;

    if (b->helper && !(b->helper->base.flags & 0x1000) && b->helper->base.header.kind == 1) return;
    b->helper = CreateObjTemplateAndObj(obj->base.unk56, 1, 0x24, (obj->base.x >> 8) + ((obj->base.flags & 1) ? -32 : 32),
                                        (obj->base.y >> 8) - (b->callBronto ? 32 : 0), 0, 0x1F, 0, 0, b->callBronto ? OBJ_BRONTO_BURT : OBJ_WADDLE_DOO,
                                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    b->callBronto = !b->callBronto;
}

static void SetState(struct Bugzzy *b, u8 state) {
    struct Object *obj = &b->n.obj;

    b->state = state;
    switch (state) {
    case STATE_WALK:
        NidlFaceKirby(obj);
        Forward(obj, WALK_SPEED);
        NidlPlay(&b->n, sWalk);
        b->timer = 84 + (Rand16() & 15);
        break;
    case STATE_CHARGE:
        NidlFaceKirby(obj);
        Forward(obj, -0x80);
        NidlPlay(&b->n, sCharge);
        b->timer = CHARGE_TIME;
        break;
    case STATE_DASH:
        Forward(obj, DASH_SPEED);
        NidlPlay(&b->n, sDash);
        b->timer = DASH_TIME;
        PlaySfx(&obj->base, SE_MINIBOSS_RUN);
        break;
    case STATE_STOP:
        obj->base.xspeed = 0;
        b->n.sequence = NULL;
        NidlSetFrame(&b->n, 26);
        b->timer = 11;
        break;
    case STATE_JUMP_CROUCH:
        NidlFaceKirby(obj);
        obj->base.xspeed = 0;
        NidlPlay(&b->n, sCrouch);
        b->timer = 14;
        break;
    case STATE_JUMP:
        Forward(obj, JUMP_SPEED);
        obj->base.yspeed = JUMP_HEIGHT;
        b->airborne = FALSE;
        NidlPlay(&b->n, sRise);
        PlaySfx(&obj->base, SE_BONKERS_JUMP);
        break;
    case STATE_CALL:
        obj->base.xspeed = 0;
        NidlPlay(&b->n, sCall);
        b->timer = CALL_TIME;
        break;
    case STATE_GRAB:
        obj->base.xspeed = obj->base.yspeed = 0;
        NidlPlay(&b->n, sGrab);
        b->hold = 0;
        b->timer = GRAB_TIME;
        break;
    case STATE_SUPLEX:
        Forward(obj, -SUPLEX_SPEED);
        obj->base.yspeed = SUPLEX_HEIGHT;
        b->airborne = FALSE;
        NidlPlay(&b->n, sSuplex);
        b->hold = 1;
        Hold(b, 1);
        break;
    case STATE_GET_UP:
        obj->base.xspeed = 0;
        NidlPlay(&b->n, sGetUp);
        b->timer = GET_UP_TIME;
        break;
    }
    // What follows the walk (traced: charges about half the time, jumps or calls for help the rest);
    if (state == STATE_WALK) b->next = Rand16() & 3;
}

static void BugzzyMain(struct Object *obj) {
    struct Bugzzy *b = (struct Bugzzy *)obj;

    NidlUpdate(&b->n);
    NidlAnimate(&b->n);
    obj->base.flags |= 4;
    obj->base.yspeed -= GRAVITY;
    if (obj->base.yspeed < -MAX_FALL) obj->base.yspeed = -MAX_FALL;

    switch (b->state) {
    case STATE_WALK:
        NidlStopAtLedge(obj);
        if (--b->timer) break;
        switch (b->next) {
        case 0:
        case 1:
            SetState(b, STATE_CHARGE);
            break;
        case 2:
            SetState(b, STATE_JUMP_CROUCH);
            break;
        default:
            SetState(b, STATE_CALL);
            break;
        }
        break;
    case STATE_CHARGE:
        if (b->timer == CHARGE_TIME - 12) obj->base.xspeed = 0;
        if (!--b->timer) SetState(b, STATE_DASH);
        break;
    case STATE_DASH:
        if (TryGrab(b)) break;
        if (!--b->timer || (obj->base.unk62 & 3)) SetState(b, STATE_STOP);
        break;
    case STATE_STOP:
        if (!--b->timer) SetState(b, STATE_WALK);
        break;
    case STATE_JUMP_CROUCH:
        if (!--b->timer) SetState(b, STATE_JUMP);
        break;
    case STATE_JUMP:
        if (obj->base.yspeed < 0 && b->n.sequence == sRise) NidlPlay(&b->n, sFall);
        if (obj->base.yspeed < 0 && TryGrab(b)) break;
        if (Landed(b)) SetState(b, STATE_WALK);
        break;
    case STATE_CALL:
        if (b->timer == CALL_TIME - 33) Call(b);
        if (!--b->timer) SetState(b, STATE_WALK);
        break;
    case STATE_GRAB:
        if (!--b->timer) SetState(b, STATE_SUPLEX);
        break;
    case STATE_SUPLEX:
        if (b->hold < 5 && b->n.sequence) {
            // The hold entry follows its frames (45, 46, 49, 47, 48);
            static const u8 sFrames[] = { 45, 46, 49, 47, 48 };
            u8 i;

            for (i = 0; i < ARRAY_COUNT(sFrames); i++)
                if (obj->unk83 == b->n.animBase + sFrames[i]) b->hold = i + 1;
            Hold(b, b->hold);
        }
        // Traced: 1.5 back while it lifts Kirby, 0.5 once it's on its back (48);
        if (obj->unk83 == b->n.animBase + FRAME_SLAM) Forward(obj, -SUPLEX_SLOW);
        if (!Landed(b)) break;
        RequestScreenShake(3, &obj->base);
        PlaySfx(&obj->base, SE_BOSS_GROUND_POUND_ATTACK);
        Release(b, sSlam);
        SetState(b, STATE_GET_UP);
        break;
    case STATE_GET_UP:
        if (!--b->timer) SetState(b, STATE_WALK);
        break;
    }
}

// BugzzyStart(); The fight starts, and again after a hit (which makes it let Kirby go);
void BugzzyStart(struct Object *obj) {
    struct Bugzzy *b = (struct Bugzzy *)obj;

    ObjectSetFunc(obj, 0, BugzzyMain);
    Release(b, sLetGo);
    obj->base.yspeed = 0;
    SetState(b, STATE_WALK);
}

// BugzzyInit(); Its box, on the floor;
void BugzzyInit(struct NidlObject *n) {
    struct Bugzzy *b = (struct Bugzzy *)n;
    struct Object *obj = &n->obj;

    b->held = NULL;
    b->helper = NULL;
    b->callBronto = TRUE;
    NidlSetFrame(n, FRAME_WALK);
    sub_0803E2B0(&obj->base, -20, -18, 20, 22);
    ObjectSetBounds(&obj->base, -22, -18, 22, 24);
    NidlOnFloor(obj);
}
