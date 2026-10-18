/* Needle; NiDL's (from Needlous): B makes Kirby stick his needles out (10 frames), and keep them out as long as it's held; then he
 * pulls them in (10 frames). In the air he falls, needles all around, and keeps them out on landing if B is still held. The needles
 * hurt what they touch (DAMAGE: NiDL's 3 was too strong), once per REHIT_FRAMES (NiDL's enemies die in one hit; KatAM's would
 * otherwise be hurt every 3 frames); Kirby can still be hurt meanwhile (NiDL's guard made him untouchable); */
#include "../abilities.h"
#include "constants/songs.h"

#define OUT_FRAMES 10
#define IN_FRAMES 10
#define REHIT_FRAMES 30
#define DAMAGE 2

static void NeedleOut(struct Kirby *);
static void NeedleAir(struct Kirby *);
static void NeedleIn(struct Kirby *);

static bool32 NeedleSpiked(struct Kirby *kirby) {
    return kirby->stateFn == NeedleOut || kirby->stateFn == NeedleAir;
}

static void NeedleGround(struct Kirby *kirby) {
    NidlMotion(kirby, NEEDLE_MOTION_OUT);
    kirby->base.counter = 0;
    kirby->stateFn = NeedleOut;
}

// NeedleStart(); The B button;
void NeedleStart(struct Kirby *kirby) {
    kirby->idleTimer = 0;
    kirby->base.flags &= ~2;
    kirby->base.flags &= ~0x40;
    if (kirby->movementState & 0x20)
        kirby->base.flags |= 1;
    if (kirby->movementState & 0x10)
        kirby->base.flags &= ~1;
    PlaySfx(&kirby->base, SE_ABILITY_STONE_TRANSFORM);
    if (kirby->base.flags & 0x20) {
        NidlMotion(kirby, NEEDLE_MOTION_AIR);
        kirby->base.counter = 0;
        kirby->stateFn = NeedleAir;
        NidlAttackCreate(kirby, NeedleSpiked, 0x30002003, DAMAGE, FALSE, -28, -38, 28, 26)->rehit = REHIT_FRAMES;
    } else {
        kirby->base.xspeed = 0;
        NeedleGround(kirby);
        NidlAttackCreate(kirby, NeedleSpiked, 0x30002003, DAMAGE, FALSE, -28, -36, 28, 10)->rehit = REHIT_FRAMES;
    }
}

static void NeedleOut(struct Kirby *kirby) {
    if (NidlAbilityInWater(kirby)) return;
    NidlAbilityGravity(kirby);
    if (kirby->base.counter < OUT_FRAMES) ++kirby->base.counter;
    else if (!(kirby->movementState & 2)) {
        NidlMotion(kirby, NEEDLE_MOTION_IN);
        kirby->base.counter = 0;
        kirby->stateFn = NeedleIn;
    }
}

static void NeedleAir(struct Kirby *kirby) {
    if (NidlAbilityInWater(kirby)) return;
    NidlAbilityGravity(kirby);
    if (kirby->base.counter < OUT_FRAMES) ++kirby->base.counter;
    if (kirby->base.unk62 & 4 && kirby->base.yspeed <= 0) {
        kirby->base.xspeed = 0;
        if (kirby->movementState & 2) {
            // Landed with B held: the ground needles, already out;
            kirby->stateFn = NeedleOut;
            kirby->animationIndex = NIDL_MOTION_SLOT + NEEDLE_MOTION_OUT;
            kirby->base.counter = OUT_FRAMES;
        } else {
            NidlMotion(kirby, NEEDLE_MOTION_IN);
            kirby->base.counter = 0;
            kirby->stateFn = NeedleIn;
        }
    }
}

static void NeedleIn(struct Kirby *kirby) {
    if (NidlAbilityInWater(kirby)) return;
    NidlAbilityGravity(kirby);
    if (++kirby->base.counter >= IN_FRAMES) NidlAbilityEnd(kirby);
}
