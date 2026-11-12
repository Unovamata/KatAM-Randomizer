/* gNidlData; Left blank here and filled in by KatAMExpansion.cs (see nidl.h). It's in its own file so the code reading it
 * can't assume these zeros; */
#include "nidl.h"

const struct NidlData gNidlData = { 0 };

#include "abilities.h"

const struct NidlAbilityData gNidlAbilityData = { 0 };
const struct AnimInfo *const gSqueakAbilityBodies[SQUEAK_ABILITY_COUNT] = { 0 };
const u16 gSqueakAbilityLooks[SQUEAK_LOOK_COUNT][SQUEAK_LOOK_LAYERS] = { 0 };

#include "title.h"

const struct NidlTitle gNidlTitle = { 0 };

#include "squeak.h"

const struct SqueakData gSqueakData = { 0 };
