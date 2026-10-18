#ifndef GUARD_NIDL_TITLE_H
#define GUARD_NIDL_TITLE_H

// title.h: Nightmare in Dream Land's title screen swarm (title.c), its data filled in by KatAMExpansion.cs (nidl_data.c);

#include "global.h"

#define NIDL_TITLE_MAX_KIRBYS 128

struct NidlTitle {
    u16 count;                   // Kirbys;
    u16 poses;                   // their 32x32 poses;
    const s16 *entries;          // per Kirby: script, delay, x, y;
    const u16 *const *scripts;   // NiDL's scripts, by index;
    const u8 *framePoses;        // per NiDL frame: its pose;
    const u16 *frameFlips;       // per NiDL frame: its flips (OAM attribute 1 bits 12-13);
    const u32 *poseTiles;        // 16 tiles per pose, in 1D order;
    const u16 *palette;          // NiDL's Kirby palette;
};

extern const struct NidlTitle gNidlTitle;

void CreateNidlTitle(u16 unused);

#endif
