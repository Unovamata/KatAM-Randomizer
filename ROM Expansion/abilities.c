/* NiDL abilities; What KatAM needs to know about abilities 27-31 (see abilities.h), and their HUD portraits; */
#include "abilities.h"
#include "main.h"

const struct NidlAbilityTables gNidlAbilityTables = {
    .base = {
        [NIDL_ABILITY_NEEDLE - NIDL_FIRST_ABILITY] = KIRBY_ABILITY_NORMAL,
        [NIDL_ABILITY_FREEZE - NIDL_FIRST_ABILITY] = KIRBY_ABILITY_NORMAL,
        [NIDL_ABILITY_LIGHT - NIDL_FIRST_ABILITY] = KIRBY_ABILITY_NORMAL,
        [NIDL_ABILITY_BALL - NIDL_FIRST_ABILITY] = KIRBY_ABILITY_NORMAL,
        [NIDL_ABILITY_HI_JUMP - NIDL_FIRST_ABILITY] = KIRBY_ABILITY_NORMAL,
    },
};

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
