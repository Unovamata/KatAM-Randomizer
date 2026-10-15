/* Light; NiDL's (from Cool Spook), measured in NiDL: B makes Kirby hover and hold up a light (28 frames), which then shines (65
 * frames); then the ability is gone, used up. NiDL's Light only lights dark rooms; KatAM has none, so its flash hurts every enemy on
 * the screen instead (a weak Crash), and the screen flashes white (when the room isn't blending already); */
#include "../abilities.h"
#include "constants/songs.h"

#define RAISE_FRAMES 28
#define SHINE_FRAMES 65
#define FLASH_FRAMES 20

// The flash's brightness (BLDY) per frame;
static const u8 sFlash[FLASH_FRAMES] = { 3, 6, 9, 12, 12, 11, 11, 10, 9, 9, 8, 7, 6, 6, 5, 4, 3, 2, 2, 1 };

void sub_08034C9C(u8);
void sub_08035E28(u8);

static void LightShine(struct Kirby *);

static bool32 LightFlashing(struct Kirby *kirby) {
    return kirby->stateFn == LightShine && kirby->base.counter < RAISE_FRAMES + FLASH_FRAMES;
}

// LightStart(); The B button;
void LightStart(struct Kirby *kirby) {
    NidlMotion(kirby, LIGHT_MOTION_SHINE);
    kirby->base.counter = 0;
    kirby->idleTimer = 0;
    kirby->base.xspeed = 0;
    kirby->base.yspeed = 0;
    kirby->base.flags &= ~2;
    kirby->base.flags |= 0x40;     // Hovers;
    if (kirby->movementState & 0x20)
        kirby->base.flags |= 1;
    if (kirby->movementState & 0x10)
        kirby->base.flags &= ~1;
    kirby->stateFn = LightShine;
}

/* LightFlash; The screen brightens and fades back (a task of its own, so it ends whatever happens to Kirby); */
struct LightFlash {
    u16 timer;
};

static void LightFlashMain(void) {
    struct LightFlash *flash = TaskGetStructPtr(gCurTask);
    u16 t = flash->timer++;

    if (t >= FLASH_FRAMES) {
        gBldRegs.bldCnt = 0;
        gBldRegs.bldY = 0;
        TaskDestroy(gCurTask);
        return;
    }
    gBldRegs.bldCnt = BLDCNT_TGT1_ALL | BLDCNT_EFFECT_LIGHTEN;
    gBldRegs.bldY = sFlash[t];
}

static void LightFlashCreate(struct Kirby *kirby) {
    struct Task *task;

    if (gLocalPlayerId != kirby->base.unk56 || gBldRegs.bldCnt != 0) return;
    task = TaskCreate(LightFlashMain, sizeof(struct LightFlash), 0x3500, TASK_USE_IWRAM, NULL);
    ((struct LightFlash *)TaskGetStructPtr(task))->timer = 0;
}

static void LightShine(struct Kirby *kirby) {
    if (NidlAbilityInWater(kirby)) return;
    kirby->base.xspeed = 0;
    kirby->base.yspeed = 0;
    if (++kirby->base.counter == RAISE_FRAMES) {
        PlaySfx(&kirby->base, SE_ABILITY_MASTER_WAVE_CHARGED);
        NidlAttackCreate(kirby, LightFlashing, 0x20000403, 4, FALSE, -120, -80, 120, 80);
        LightFlashCreate(kirby);
    }
    if (kirby->base.counter < RAISE_FRAMES + SHINE_FRAMES) return;

    // Used up (like Crash, sub_08068724);
    kirby->ability = KIRBY_ABILITY_NORMAL;
    if (gLocalPlayerId == kirby->base.unk56) {
        sub_08035E28(0);
        sub_08034C9C(2);
    }
    sub_0806F260(kirby);
    NidlAbilityEnd(kirby);
}
