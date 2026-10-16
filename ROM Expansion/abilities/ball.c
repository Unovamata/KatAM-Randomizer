/* Ball; NiDL's (from Bubbles), measured in NiDL: B makes Kirby hop (up to 20 pixels) and curl into a ball, which bounces once (about
 * 9 pixels) on landing and then rests, squashing. B again launches it (up to 37 pixels); it ends on landing. It can be steered left and
 * right (up to 1.87 pixels a frame) while in the air, and hurts what it touches, Kirby unhurt meanwhile; */
#include "../abilities.h"
#include "constants/songs.h"

#define HOP_SPEED 0x270
#define BOUNCE_SPEED 0x1A0
#define LAUNCH_SPEED 0x350
#define STEER_SPEED 0x1E0
#define STEER_ACCELERATION 0x18
#define SQUASH_FRAMES 4

static void BallHop(struct Kirby *);
static void BallBounce(struct Kirby *);
static void BallSquash(struct Kirby *);
static void BallRest(struct Kirby *);
static void BallLaunch(struct Kirby *);

static bool32 BallBall(struct Kirby *kirby) {
    return kirby->stateFn == BallHop || kirby->stateFn == BallBounce || kirby->stateFn == BallSquash
        || kirby->stateFn == BallRest || kirby->stateFn == BallLaunch;
}

// BallPressed(); B pressed this frame (unk11A: the buttons pressed this frame);
static bool32 BallPressed(struct Kirby *kirby) {
    return kirby->unk11A & 2;
}

static void BallSteer(struct Kirby *kirby) {
    s16 target = 0;

    if (kirby->movementState & 0x20) {
        target = -STEER_SPEED;
        kirby->base.flags |= 1;
    } else if (kirby->movementState & 0x10) {
        target = STEER_SPEED;
        kirby->base.flags &= ~1;
    }
    if (kirby->base.xspeed < target) {
        kirby->base.xspeed += STEER_ACCELERATION;
        if (kirby->base.xspeed > target) kirby->base.xspeed = target;
    } else if (kirby->base.xspeed > target) {
        kirby->base.xspeed -= STEER_ACCELERATION;
        if (kirby->base.xspeed < target) kirby->base.xspeed = target;
    }
    if (kirby->base.unk62 & 1)
        kirby->base.xspeed = 0;
}

static void BallFall(struct Kirby *kirby) {
    kirby->base.yspeed -= 38;
    if (kirby->base.yspeed < -0x280)
        kirby->base.yspeed = -0x280;
}

static bool32 BallLanded(struct Kirby *kirby) {
    return kirby->base.unk62 & 4 && kirby->base.yspeed <= 0;
}

static void BallJump(struct Kirby *kirby, s16 speed) {
    kirby->base.yspeed = speed;
    kirby->base.flags |= 0x20;
    kirby->base.unk62 &= ~4;
}

// BallStart(); The B button;
void BallStart(struct Kirby *kirby) {
    NidlMotion(kirby, BALL_MOTION_HOP);
    kirby->base.counter = 0;
    kirby->idleTimer = 0;
    kirby->base.flags &= ~2;
    kirby->base.flags &= ~0x40;
    if (kirby->movementState & 0x20)
        kirby->base.flags |= 1;
    if (kirby->movementState & 0x10)
        kirby->base.flags &= ~1;
    BallJump(kirby, HOP_SPEED);
    PlaySfx(&kirby->base, SE_KIRBY_BOUNCE);
    NidlAttackCreate(kirby, BallBall, 0x30000043, 3, TRUE, -10, -10, 10, 8);
    kirby->stateFn = BallHop;
}

// BallHop(); The hop into a ball, then the fall, until it lands: a bounce;
static void BallHop(struct Kirby *kirby) {
    if (NidlAbilityInWater(kirby)) return;
    BallSteer(kirby);
    BallFall(kirby);
    if (kirby->base.yspeed < 0 && kirby->animationIndex == NIDL_MOTION_SLOT + BALL_MOTION_HOP && kirby->base.flags & 2)
        NidlMotion(kirby, BALL_MOTION_FALL);
    if (++kirby->base.counter > 2 && BallLanded(kirby)) {
        NidlMotion(kirby, BALL_MOTION_ROLL);
        kirby->base.counter = 0;
        BallJump(kirby, BOUNCE_SPEED);
        PlaySfx(&kirby->base, SE_KIRBY_STAR_BOUNCE);
        kirby->stateFn = BallBounce;
    }
}

static void BallBounce(struct Kirby *kirby) {
    if (NidlAbilityInWater(kirby)) return;
    BallSteer(kirby);
    BallFall(kirby);
    if (++kirby->base.counter > 2 && BallLanded(kirby)) {
        NidlMotion(kirby, BALL_MOTION_SQUASH);
        kirby->base.counter = 0;
        kirby->base.xspeed = 0;
        kirby->stateFn = BallSquash;
    }
}

static void BallSquash(struct Kirby *kirby) {
    if (NidlAbilityInWater(kirby)) return;
    if (++kirby->base.counter >= SQUASH_FRAMES) {
        NidlMotion(kirby, BALL_MOTION_REST);
        kirby->base.counter = 0;
        kirby->stateFn = BallRest;
    }
}

// BallRest(); Resting until B launches it (or it rolls off a ledge);
static void BallRest(struct Kirby *kirby) {
    if (NidlAbilityInWater(kirby)) return;
    if (!(kirby->base.unk62 & 4)) {
        NidlMotion(kirby, BALL_MOTION_FALL);
        kirby->base.counter = 0;
        kirby->stateFn = BallBounce;
        return;
    }
    kirby->base.xspeed = 0;
    if (BallPressed(kirby)) {
        NidlMotion(kirby, BALL_MOTION_LAUNCH);
        kirby->base.counter = 0;
        BallJump(kirby, LAUNCH_SPEED);
        PlaySfx(&kirby->base, SE_ABILITY_WHEEL_TRANSFORM);
        kirby->stateFn = BallLaunch;
    }
}

static void BallLaunch(struct Kirby *kirby) {
    if (NidlAbilityInWater(kirby)) return;
    BallSteer(kirby);
    BallFall(kirby);
    if (++kirby->base.counter > 2 && BallLanded(kirby)) {
        kirby->base.flags &= ~0x20;
        NidlAbilityEnd(kirby);
    }
}
