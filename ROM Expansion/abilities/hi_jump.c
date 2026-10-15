/* Hi-Jump; NiDL's (from Starman), measured in NiDL: B makes Kirby crouch for 4 frames, then rocket straight up at 4.16 pixels a
 * frame (0x428), steerable sideways up to 2.0 (0x200), hurting what he touches and unhurt, until he hits a ceiling or has gone
 * about 6.5 tiles (25 frames); then he falls like after a jump. On land or in the air; */
#include "../abilities.h"
#include "code_0806F780.h"
#include "constants/songs.h"

#define CROUCH_FRAMES 4
#define RISE_FRAMES 25
#define RISE_SPEED 0x428
#define STEER_SPEED 0x200
#define STEER_ACCELERATION 0x20

void KirbyEnterWater(struct Kirby *);

static void HiJumpCrouch(struct Kirby *);
static void HiJumpRise(struct Kirby *);
static void HiJumpAttack(struct Kirby *);

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

static bool32 HiJumpInWater(struct Kirby *kirby) {
    if (!(kirby->base.unk58 & 2)) return FALSE;
    kirby->base.flags &= ~0x40;
    KirbyEnterWater(kirby);
    KirbyStartWaterMovement(kirby);
    return TRUE;
}

static void HiJumpCrouch(struct Kirby *kirby) {
    if (HiJumpInWater(kirby)) return;
    if (++kirby->base.counter < CROUCH_FRAMES) return;

    kirby->animationIndex = 52;
    kirby->base.counter = 0;
    kirby->base.yspeed = RISE_SPEED;
    kirby->base.flags |= 0x20;
    kirby->base.unk62 &= ~(8 | 4);
    PlaySfx(&kirby->base, SE_ABILITY_SWORD_UPSLASH);
    HiJumpAttack(kirby);
    kirby->stateFn = HiJumpRise;
}

static void HiJumpRise(struct Kirby *kirby) {
    s16 target = 0;

    if (HiJumpInWater(kirby)) return;
    if (kirby->base.unk62 & 8 || ++kirby->base.counter > RISE_FRAMES) {
        kirby->base.unk62 &= ~8;
        kirby->base.yspeed = 0;    // Falls right away, like in NiDL (KatAM's gravity is NiDL's, 0x26 a frame);
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

// HiJumpAttackMain(); The attack box follows Kirby while he rises, like Burning's (sub_08079714);
static void HiJumpAttackMain(void) {
    struct ObjectBase *attack = TaskGetStructPtr(gCurTask);
    struct Kirby *kirby = attack->parent;

    if (ObjectPreUpdate(attack)) return;
    if (kirby->stateFn != HiJumpRise) {
        kirby->base.flags &= ~0x8000;
        attack->flags |= 0x1000;
        return;
    }
    SetPointerSomething(attack);
    if (!(kirby->base.flags & 0x800000) && ++attack->counter > 2) {
        attack->flags &= ~0x40000;
        attack->counter = 0;
    }
    attack->x = kirby->base.x;
    attack->y = kirby->base.y;
}

// HiJumpAttack(); Kirby's attack box while rising, and his invulnerability (Burning's, sub_0807958C);
static void HiJumpAttack(struct Kirby *kirby) {
    struct Task *task = TaskCreate(HiJumpAttackMain, sizeof(struct ObjectBase), 0x3500, TASK_USE_IWRAM, ObjectBaseDestroy);
    struct ObjectBase *attack = TaskGetStructPtr(task);

    ClearObjectBase(attack);
    attack->header.kind = 2;
    attack->x = kirby->base.x;
    attack->y = kirby->base.y;
    attack->parent = kirby;
    attack->counter = 0;
    attack->roomId = kirby->base.roomId;
    attack->unk56 = kirby->base.unk56;
    if (Macro_0810B1F4(attack))
        attack->flags |= 0x2000;
    attack->unk64 = 0;
    attack->unk66 = 0x80;
    attack->unk63 = 2;
    attack->flags |= 0xA0000000;
    attack->flags |= 0x400;
    attack->flags |= 0x2000000;
    attack->unk68 |= 0x30001103;
    if (kirby->base.flags & 1)
        attack->flags |= 1;
    kirby->base.flags |= 0x8000;
    kirby->base.unk68 = 0x80;
    sub_0803E2B0(attack, -10, -12, 10, 8);
    ObjectSetBounds(attack, -10, -12, 10, 7);
    SetPointerSomething(attack);
}
