/* Calm; The randomizer's "Reduce flashing" option: KatAM brightens the whole screen white (BLDCNT's lighten effect, BLDY 0-16) for
 * its flashes (Kracko's lightning, bombs, Dark Mind, Crash...), often in one frame and off again a few later. Each frame the game
 * copies gBldRegs to the hardware (UpdateScreenDma, UpdateScreenCpuSet, the VBlank's copies: KatAMExpansion.cs detours both here);
 * these copy it with the brightness the screen may show instead, which follows the game's at most CALM_STEP a frame on the way up
 * (a fade is no faster, so fades look the same) and at once on the way down, then put the game's own back for its code to read.
 * What the screen shows is kept in gBldRegs' padding (the struct is 6 bytes, the symbol takes 8). This file works without the rest of
 * the expansion: the option copies the code even when no NiDL ROM is loaded; */
#include "global.h"
#include "main.h"

#define CALM_STEP 1
#define CALM_EFFECT (3 << 6)      // BLDCNT's effect bits;
#define CALM_SHOWN (*((u8 *)&gBldRegs + 6))

// NidlCalm; Filled in by KatAMExpansion.cs: the trampolines running the detoured functions;
struct NidlCalm {
    void (*updateScreenDma)(void);
    void (*updateScreenCpuSet)(void);
};

const struct NidlCalm gNidlCalm = { NULL, NULL };

// Room for the trampolines KatAMExpansion.cs writes (it allocates them from here, so the option needs no other free space);
const u32 gNidlCalmTrampolines[16] = { 0 };

// Calm(); Runs the game's copy with the brightness the screen may show;
static void Calm(void (*original)(void)) {
    u16 own = gBldRegs.bldY;
    bool32 lighten = (gBldRegs.bldCnt & CALM_EFFECT) == BLDCNT_EFFECT_LIGHTEN;
    u8 target = lighten ? min(own & 0x1F, 16) : 0;

    CALM_SHOWN = target > CALM_SHOWN + CALM_STEP ? CALM_SHOWN + CALM_STEP : target;
    if (lighten) gBldRegs.bldY = CALM_SHOWN;
    original();
    gBldRegs.bldY = own;
}

void NidlCalmUpdateScreenDma(void) {
    Calm(gNidlCalm.updateScreenDma);
}

void NidlCalmUpdateScreenCpuSet(void) {
    Calm(gNidlCalm.updateScreenCpuSet);
}
