/* Hi-Jump; NiDL's (from Starman), measured in NiDL: B makes Kirby crouch for 4 frames, then rocket straight up at 4.16 pixels a
 * frame (0x428), steerable sideways up to 2.0 (0x200), hurting what he touches and unhurt, until he hits a ceiling or has gone
 * about 6.5 tiles (25 frames); then he falls like after a jump. On land or in the air; */
#include "../abilities.h"
#include "constants/songs.h"

#define CROUCH_FRAMES 4
#define RISE_FRAMES 25
#define RISE_SPEED 0x428
#define STEER_SPEED 0x200
#define STEER_ACCELERATION 0x20

static void HiJumpCrouch(struct Kirby *);
static void HiJumpRise(struct Kirby *);

static bool32 HiJumpRising(struct Kirby *kirby) {
    return kirby->stateFn == HiJumpRise;
}

// HiJumpStart(); The B button;
void HiJumpStart(struct Kirby *kirby) {
    kirby->animationIndex = 53;
    kirby->base.counter = 0;
    kirby->idleTimer = 0;
    kirby->base.xspeed = 0;
    kirby->base.yspeed = 0;
    kirby->base.flags &= ~2;
    kirby->base.flags |= 0x40;     // No gravity;
    kirby->base.flags &= ~0x20;
    if (kirby->movementState & 0x20)
        kirby->base.flags |= 1;
    if (kirby->movementState & 0x10)
        kirby->base.flags &= ~1;
    kirby->stateFn = HiJumpCrouch;
}

static void HiJumpCrouch(struct Kirby *kirby) {
    if (NidlAbilityInWater(kirby)) return;
    if (++kirby->base.counter < CROUCH_FRAMES) return;

    NidlMotion(kirby, HI_JUMP_MOTION_RISE);
    kirby->base.counter = 0;
    kirby->base.yspeed = RISE_SPEED;
    kirby->base.flags |= 0x20;
    kirby->base.unk62 &= ~(8 | 4);
    PlaySfx(&kirby->base, SE_ABILITY_SWORD_UPSLASH);
    NidlAttackCreate(kirby, HiJumpRising, 0x30000043, 2, TRUE, -10, -12, 10, 8);
    kirby->stateFn = HiJumpRise;
}

static void HiJumpRise(struct Kirby *kirby) {
    s16 target = 0;

    if (NidlAbilityInWater(kirby)) return;
    if (kirby->base.unk62 & 8 || ++kirby->base.counter > RISE_FRAMES) {
        kirby->base.unk62 &= ~8;
        kirby->base.yspeed = 0;    // Falls right away, like in NiDL (KatAM's gravity is NiDL's, 0x26 a frame);
        kirby->base.flags &= ~0x40;
        sub_08044EA8(kirby);
        return;
    }

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
    kirby->base.yspeed = RISE_SPEED;
    kirby->base.flags |= 0x40 | 0x20;  // No gravity, and in the air (not snapped to the floor);
}
