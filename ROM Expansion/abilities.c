/* NiDL abilities; What KatAM needs to know about abilities 27-34 (see abilities.h), and their HUD portraits; */
#include "abilities.h"
#include "main.h"
#include "data.h"
#include "code_0806F780.h"
#include "constants/songs.h"

void KirbyEnterWater(struct Kirby *);
void sub_0805B1B8(struct Kirby *);
void sub_08054950(struct Kirby *);
void sub_08034C9C(u8);
void sub_08035E28(u8);

const struct NidlAbilityTables gNidlAbilityTables = {
    .base = {
        [NIDL_ABILITY_NEEDLE - NIDL_FIRST_ABILITY] = KIRBY_ABILITY_NORMAL,
        [NIDL_ABILITY_FREEZE - NIDL_FIRST_ABILITY] = KIRBY_ABILITY_NORMAL,
        [NIDL_ABILITY_LIGHT - NIDL_FIRST_ABILITY] = KIRBY_ABILITY_NORMAL,
        [NIDL_ABILITY_BALL - NIDL_FIRST_ABILITY] = KIRBY_ABILITY_NORMAL,
        [NIDL_ABILITY_HI_JUMP - NIDL_FIRST_ABILITY] = KIRBY_ABILITY_NORMAL,
        [NIDL_ABILITY_MIKE - NIDL_FIRST_ABILITY] = KIRBY_ABILITY_NORMAL,
        [NIDL_ABILITY_BACKDROP - NIDL_FIRST_ABILITY] = KIRBY_ABILITY_THROW,
        [NIDL_ABILITY_STAR_ROD - NIDL_FIRST_ABILITY] = KIRBY_ABILITY_NORMAL,
    },
    .landB = {
        [NIDL_ABILITY_NEEDLE - NIDL_FIRST_ABILITY] = NeedleStart,
        [NIDL_ABILITY_FREEZE - NIDL_FIRST_ABILITY] = FreezeStart,
        [NIDL_ABILITY_LIGHT - NIDL_FIRST_ABILITY] = LightStart,
        [NIDL_ABILITY_BALL - NIDL_FIRST_ABILITY] = BallStart,
        [NIDL_ABILITY_HI_JUMP - NIDL_FIRST_ABILITY] = HiJumpStart,
        [NIDL_ABILITY_MIKE - NIDL_FIRST_ABILITY] = MikeStart,
    },
    .setup = {
        [NIDL_ABILITY_NEEDLE - NIDL_FIRST_ABILITY] = NidlAbilitySetup,
        [NIDL_ABILITY_FREEZE - NIDL_FIRST_ABILITY] = NidlAbilitySetup,
        [NIDL_ABILITY_LIGHT - NIDL_FIRST_ABILITY] = NidlAbilitySetup,
        [NIDL_ABILITY_BALL - NIDL_FIRST_ABILITY] = NidlAbilitySetup,
        [NIDL_ABILITY_HI_JUMP - NIDL_FIRST_ABILITY] = NidlAbilitySetup,
        [NIDL_ABILITY_MIKE - NIDL_FIRST_ABILITY] = NidlAbilitySetup,
        [NIDL_ABILITY_STAR_ROD - NIDL_FIRST_ABILITY] = NidlAbilitySetup,
    },
};

/* NidlAbilitySetup(); gUnk_083509D0's (on getting the ability): points the player's motion slots to the ability's motions (the rest
 * are Normal's, which losing the ability restores, like for KatAM's abilities); */
void NidlAbilitySetup(struct Kirby *kirby) {
    struct AnimInfo *dst = gUnk_02021590[kirby->base.unk56] + NIDL_MOTION_SLOT;
    const struct AnimInfo *src = gNidlAbilityData.motions[kirby->ability - NIDL_FIRST_ABILITY];
    u8 i;

    for (i = 0; i < NIDL_MOTION_SLOTS; i++) {
        if (!src[i].animId) continue;
        dst[i].animId = src[i].animId;
        dst[i].variant = src[i].variant;
    }
    if (kirby->ability == NIDL_ABILITY_MIKE) NIDL_MIKE_USES(kirby) = 0;
}

/* NidlAbilityPortrait(); sub_080356AC (detoured): queues the portrait `id` (an ability's, or a special one) for `vram` and loads its
 * palette into BG palette `palette`. The new abilities' come from gNidlAbilityData; the special ones are back at their place for
 * the original; */
void NidlAbilityPortrait(u32 vram, u8 palette, u8 id) {
    const struct NidlAbilityData *data = &gNidlAbilityData;

    if (id >= NIDL_FIRST_ABILITY && id < NIDL_ABILITY_END && data->portraits[id - NIDL_FIRST_ABILITY].tiles) {
        sub_0803D21C(data->portraits[id - NIDL_FIRST_ABILITY].palette, palette << 4, 0x10);
        gMainFlags |= 1;
        gUnk_03002EC0[gUnk_030039A4].unk0 = (u32)data->portraits[id - NIDL_FIRST_ABILITY].tiles;
        gUnk_03002EC0[gUnk_030039A4].unk4 = vram;
        gUnk_03002EC0[gUnk_030039A4].unk8 = PORTRAIT_SIZE;
        gUnk_030039A4 = (gUnk_030039A4 + 1) & 0x3F;
        return;
    }
    if (id >= NIDL_FIRST_ABILITY + NIDL_SPECIAL_PORTRAIT_SHIFT) id -= NIDL_SPECIAL_PORTRAIT_SHIFT;
    data->portraitOriginal(vram, palette, id);
}

// NidlAbilityHudTimer(); sub_08088640 (detoured): shows a special portrait for a while; they're moved past the abilities;
void NidlAbilityHudTimer(struct Kirby *kirby, u8 portrait, u16 frames) {
    if (portrait >= NIDL_FIRST_ABILITY) portrait += NIDL_SPECIAL_PORTRAIT_SHIFT;
    gNidlAbilityData.hudTimerOriginal(kirby, portrait, frames);
}

// NidlAttackMain(); Follows Kirby, hitting every few frames (Burning's, sub_08079714), until his state isn't the ability's;
static void NidlAttackMain(void) {
    struct NidlAttack *attack = TaskGetStructPtr(gCurTask);
    struct Kirby *kirby = attack->base.parent;

    if (ObjectPreUpdate(&attack->base)) return;
    if (!attack->alive(kirby)) {
        if (attack->guard) {
            kirby->base.flags &= ~0x8000;
            kirby->base.unk68 = attack->kirbyUnk68;
        }
        attack->base.flags |= 0x1000;
        return;
    }
    SetPointerSomething(&attack->base);
    if (!(kirby->base.flags & 0x800000) && ++attack->base.counter > attack->rehit) {
        attack->base.flags &= ~0x40000;
        attack->base.counter = 0;
    }
    attack->base.x = kirby->base.x;
    attack->base.y = kirby->base.y;
    attack->base.flags = (attack->base.flags & ~1) | (kirby->base.flags & 1);
    if (attack->guard) kirby->base.flags |= 0x8000;
}

// NidlAttackCreate(); See abilities.h (Burning's attack box, sub_0807958C);
struct NidlAttack *NidlAttackCreate(struct Kirby *kirby, bool32 (*alive)(struct Kirby *), u32 effect, u8 damage, bool8 guard,
                                    s8 left, s8 top, s8 right, s8 bottom) {
    struct Task *task = TaskCreate(NidlAttackMain, sizeof(struct NidlAttack), 0x3500, TASK_USE_IWRAM, ObjectBaseDestroy);
    struct NidlAttack *attack = TaskGetStructPtr(task);

    ClearObjectBase(&attack->base);
    attack->alive = alive;
    attack->guard = guard;
    attack->rehit = 2;
    attack->kirbyUnk68 = kirby->base.unk68;
    attack->base.header.kind = 2;
    attack->base.x = kirby->base.x;
    attack->base.y = kirby->base.y;
    attack->base.parent = kirby;
    attack->base.counter = 0;
    attack->base.roomId = kirby->base.roomId;
    attack->base.unk56 = kirby->base.unk56;
    if (Macro_0810B1F4(&attack->base))
        attack->base.flags |= 0x2000;
    attack->base.unk64 = 0;
    attack->base.unk66 = 0x80;
    attack->base.unk63 = damage;
    attack->base.flags |= 0xA0000000;
    attack->base.flags |= 0x400;
    attack->base.flags |= 0x2000000;
    attack->base.unk68 |= effect;
    if (kirby->base.flags & 1)
        attack->base.flags |= 1;
    if (guard) {
        kirby->base.flags |= 0x8000;
        kirby->base.unk68 = 0x80;
    }
    sub_0803E2B0(&attack->base, left, top, right, bottom);
    ObjectSetBounds(&attack->base, left, top, right, bottom);
    SetPointerSomething(&attack->base);
    return attack;
}

// NidlAbilityInWater(); Into the water (out of the ability's state), like KatAM's abilities;
bool32 NidlAbilityInWater(struct Kirby *kirby) {
    if (!(kirby->base.unk58 & 2)) return FALSE;
    kirby->base.flags &= ~0x40;
    KirbyEnterWater(kirby);
    KirbyStartWaterMovement(kirby);
    return TRUE;
}

// NidlAbilityGravity(); Falling (unless on the ground) and slowing down sideways, like KatAM's attacks (sub_0805D164);
void NidlAbilityGravity(struct Kirby *kirby) {
    if (!(kirby->base.unk62 & 4)) {
        kirby->base.yspeed -= 38;
        if (kirby->base.yspeed < -0x280)
            kirby->base.yspeed = -0x280;
    }
    sub_0805B1B8(kirby);
    if (kirby->base.unk62 & 1)
        kirby->base.xspeed = 0;
}

// NidlAbilityEnd(); Back to standing, or falling (sub_0805D480);
void NidlAbilityEnd(struct Kirby *kirby) {
    kirby->base.flags &= ~0x40;
    if (kirby->base.flags & 0x20)
        sub_08044EA8(kirby);
    else
        sub_0803FE74(kirby);
}

/* NidlSwallow(); sub_080547C4 (detoured, never returning to it): something is swallowed, giving `ability` (| the ability star flag
 * for ability stars; NIDL_SWALLOW_RANDOM, NIDL_SWALLOW_NONE). KatAM's, with transitioningAbility's 6-bit layout; */
void NidlSwallow(struct Kirby *kirby, u8 ability) {
    PlaySfx(&kirby->base, SE_KIRBY_STUFFED_CHEEKS_UNUSED);
    if (ability != NIDL_SWALLOW_NONE) {
        if (!(kirby->base.flags & 0x80)) {
            kirby->base.flags |= 0x80;
            kirby->unkF1 = 0;
            sub_0806F260(kirby);
        } else {
            if ((kirby->transitioningAbility & NIDL_ABILITY_MASK) != KIRBY_ABILITY_NORMAL && ability
                && !(kirby->transitioningAbility & NIDL_ABILITY_CHANGE_IS_ABILITY_STAR))
                kirby->transitioningAbility |= NIDL_ABILITY_CHANGE_RANDOM;
            kirby->unkF1 = NIDL_ABILITY_SEVERAL;
        }
        if (ability & NIDL_ABILITY_CHANGE_IS_ABILITY_STAR) {
            if (!(kirby->transitioningAbility & NIDL_ABILITY_CHANGE_IS_ABILITY_STAR))
                kirby->transitioningAbility = (ability & NIDL_ABILITY_MASK) | NIDL_ABILITY_CHANGE_IS_ABILITY_STAR;
            else if ((ability & NIDL_ABILITY_MASK) == KIRBY_ABILITY_MASTER)
                gUnk_0203AD34 = 0;
        } else if (!(kirby->transitioningAbility & (NIDL_ABILITY_CHANGE_RANDOM | NIDL_ABILITY_CHANGE_IS_ABILITY_STAR))) {
            kirby->transitioningAbility |= ability;
            if (ability == NIDL_SWALLOW_RANDOM)
                kirby->transitioningAbility |= NIDL_ABILITY_CHANGE_RANDOM;
        } else if ((ability & NIDL_ABILITY_MASK) == KIRBY_ABILITY_MASTER)
            gUnk_0203AD34 = 0;
    }
    if (!kirby->inhaling || !--kirby->inhaling) {
        if (kirby->stateFn == sub_08054950)
            kirby->animationIndex = kirby->base.flags & 0x80 ? 30 : 37;
        sub_0808A3E0(&kirby->base);
        kirby->base.flags &= ~2;
    }
}

// NidlAbilityUsedUp(); The ability is gone (like Crash's, sub_08068724): back to Normal, the HUD's portrait too;
void NidlAbilityUsedUp(struct Kirby *kirby) {
    kirby->ability = KIRBY_ABILITY_NORMAL;
    if (gLocalPlayerId == kirby->base.unk56) {
        sub_08035E28(0);
        sub_08034C9C(2);
    }
    sub_0806F260(kirby);
}

/* NidlFlash; The screen brightening (BLDY, one level a frame), a task of its own so it ends whatever happens to Kirby. Only the
 * local player's, and only when the room isn't blending already; */
struct NidlFlash {
    const u8 *levels;
    u16 count;
    u16 timer;
};

static void NidlFlashMain(void) {
    struct NidlFlash *flash = TaskGetStructPtr(gCurTask);
    u16 t = flash->timer++;

    if (t >= flash->count) {
        gBldRegs.bldCnt = 0;
        gBldRegs.bldY = 0;
        TaskDestroy(gCurTask);
        return;
    }
    gBldRegs.bldCnt = BLDCNT_TGT1_ALL | BLDCNT_EFFECT_LIGHTEN;
    gBldRegs.bldY = flash->levels[t];
}

void NidlFlash(struct Kirby *kirby, const u8 *levels, u16 count) {
    struct NidlFlash *flash;

    if (gLocalPlayerId != kirby->base.unk56 || gBldRegs.bldCnt != 0) return;
    flash = TaskGetStructPtr(TaskCreate(NidlFlashMain, sizeof(struct NidlFlash), 0x3500, TASK_USE_IWRAM, NULL));
    flash->levels = levels;
    flash->count = count;
    flash->timer = 0;
}
