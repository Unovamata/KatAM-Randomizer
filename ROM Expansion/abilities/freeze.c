/* Freeze; NiDL's (from Chilly), measured in NiDL: B wraps Kirby in ice crystals where he stands, for as long as it's held (16 frames at
 * least): 2 frames of the start, the breath (8 frames a loop), 2 frames of the end. What touches the ice freezes (KatAM's Ice's hit),
 * and Kirby can still be hurt meanwhile (NiDL's guard made him untouchable); */
#include "../abilities.h"
#include "constants/songs.h"

#define EDGE_FRAMES 2
#define MIN_FRAMES 16

static void FreezeIn(struct Kirby *);
static void FreezeBreath(struct Kirby *);
static void FreezeOut(struct Kirby *);

static bool32 FreezeBreathing(struct Kirby *kirby) {
    return kirby->stateFn == FreezeBreath;
}

// FreezeStart(); The B button;
void FreezeStart(struct Kirby *kirby) {
    NidlMotion(kirby, FREEZE_MOTION_EDGE);
    kirby->base.counter = 0;
    kirby->idleTimer = 0;
    kirby->base.flags &= ~2;
    kirby->base.flags &= ~0x40;
    if (kirby->movementState & 0x20)
        kirby->base.flags |= 1;
    if (kirby->movementState & 0x10)
        kirby->base.flags &= ~1;
    kirby->stateFn = FreezeIn;
}

static void FreezeIn(struct Kirby *kirby) {
    if (NidlAbilityInWater(kirby)) return;
    NidlAbilityGravity(kirby);
    if (++kirby->base.counter < EDGE_FRAMES) return;

    NidlMotion(kirby, FREEZE_MOTION_BREATH);
    kirby->base.counter = 0;
    PlaySfx(&kirby->base, SE_ABILITY_ICE_ATTACK);
    NidlAttackCreate(kirby, FreezeBreathing, 0x20000203, 2, FALSE, -20, -32, 20, 24);
    kirby->stateFn = FreezeBreath;
}

static void FreezeBreath(struct Kirby *kirby) {
    if (NidlAbilityInWater(kirby)) return;
    NidlAbilityGravity(kirby);
    if (kirby->base.counter < MIN_FRAMES) ++kirby->base.counter;
    else if (!(kirby->movementState & 2)) {
        NidlMotion(kirby, FREEZE_MOTION_EDGE);
        kirby->base.counter = 0;
        kirby->stateFn = FreezeOut;
        return;
    }
    if (!(kirby->base.counter & 7)) PlaySfx(&kirby->base, SE_ABILITY_ICE_ATTACK);
}

static void FreezeOut(struct Kirby *kirby) {
    if (NidlAbilityInWater(kirby)) return;
    NidlAbilityGravity(kirby);
    if (++kirby->base.counter >= EDGE_FRAMES) NidlAbilityEnd(kirby);
}
