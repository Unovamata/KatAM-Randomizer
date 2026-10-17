/* Hi-Jump; NiDL's (from Starman), measured in NiDL: B makes Kirby crouch for 4 frames, then rocket straight up at 4.16 pixels a
 * frame (0x428), steerable sideways up to 2.0 (0x200), hurting what he touches (he can still be hurt: NiDL's guard made him
 * untouchable), until he hits a ceiling or has risen 23 frames; then he slows (sEaseSpeeds, a frame each: about 6.3 tiles
 * in all, as NiDL's) and falls like after a jump; a ceiling stops him at once. On land or in the air; */
#include "../abilities.h"
#include "constants/songs.h"

#define CROUCH_FRAMES 4
#define RISE_FRAMES 23
#define RISE_SPEED 0x428
#define STEER_SPEED 0x200
#define STEER_ACCELERATION 0x20

static const s16 sEaseSpeeds[] = { 0x3D9, 0x18E }; // NiDL's last steps up (3.85, 1.55 pixels);

static void HiJumpCrouch(struct Kirby *);
static void HiJumpRise(struct Kirby *);
static void HiJumpEase(struct Kirby *);
static void HiJumpFall(struct Kirby *);
static void HiJumpSteer(struct Kirby *);

static bool32 HiJumpRising(struct Kirby *kirby) {
    return kirby->stateFn == HiJumpRise || kirby->stateFn == HiJumpEase;
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
    NidlAttackCreate(kirby, HiJumpRising, 0x30000043, 2, FALSE, -10, -12, 10, 8);
    kirby->stateFn = HiJumpRise;
}

static void HiJumpRise(struct Kirby *kirby) {
    if (NidlAbilityInWater(kirby)) return;
    if (kirby->base.unk62 & 8) {
        HiJumpFall(kirby);
        return;
    }
    if (++kirby->base.counter > RISE_FRAMES) {
        kirby->base.counter = 0;
        kirby->stateFn = HiJumpEase;
        HiJumpEase(kirby);
        return;
    }
    HiJumpSteer(kirby);
    kirby->base.yspeed = RISE_SPEED;
    kirby->base.flags |= 0x40 | 0x20;  // No gravity, and in the air (not snapped to the floor);
}

// HiJumpEase(); The rise's last frames, slower each (sEaseSpeeds), then the fall;
static void HiJumpEase(struct Kirby *kirby) {
    if (NidlAbilityInWater(kirby)) return;
    if (kirby->base.unk62 & 8 || kirby->base.counter >= ARRAY_COUNT(sEaseSpeeds)) {
        HiJumpFall(kirby);
        return;
    }
    HiJumpSteer(kirby);
    kirby->base.yspeed = sEaseSpeeds[kirby->base.counter++];
    kirby->base.flags |= 0x40 | 0x20;
}

// HiJumpFall(); Falls from a standstill, as after a jump (KatAM's gravity is NiDL's, 0x26 a frame);
static void HiJumpFall(struct Kirby *kirby) {
    kirby->base.unk62 &= ~8;
    kirby->base.yspeed = 0;
    kirby->base.flags &= ~0x40;
    sub_08044EA8(kirby);
}

// HiJumpSteer(); Left and right while rising;
static void HiJumpSteer(struct Kirby *kirby) {
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
}
