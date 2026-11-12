/* scrolls.c: Squeak Squad's Ability Scrolls' upgrades on KatAM's abilities. A scroll is owned in the save's scroll bits (rooms.c's
 * SqueakScrollOwned(), squeak.h), numbered as the DS's (its HasScroll, 0x02020AD8, asked with a scroll's number by each ability's
 * code: Fire 0, Ice 1, Spark 2, Beam 3, Tornado 4, Parasol 5, Hammer 6, Cupid 7, Cutter 8, Laser 9, Sword 10, Wheel 11, Hi-Jump 12,
 * UFO 13, Sleep 14, Bomb 15, Ninja 16, Fighter 17, Throw 18, Magic 19, Animal 20, Bubble 21, Metal 22). The owner: "For KatAM existing
 * abilities, the scroll will upgrade them with the DS moveset 1:1": what the DS's scroll adds to its move, KatAM's move gets, with the
 * DS's numbers (SQUEAK_SPEED: its 20.12 speeds in KatAM's 24.8). Every Kirby has them (the DS has one Kirby: the helpers share the
 * player's scrolls). KatAMExpansionScrolls.cs hooks KatAM's code into these:
 * • Fire and Ice (the DS's flame and ice breath, 0x0202FDD0 and 0x02030F6C: each puff of breath it shoots gets, with the scroll, its
 *   prmKirbyFire/Ice.kgb words 3 and 4, 1.5 pixels a frame, up with Up held, down with Down): KatAM's breath's puffs take their height
 *   speed from Kirby's base.counter as they start again (sub_080737D8, the breath's hit box, and sub_080730E0, sub_080732E4,
 *   sub_080733D8, its flames), which KatAM's fire breath state (sub_0805D268) moves up to 1 pixel a frame up and 3/4 down, a step a
 *   frame, while Up or Down is held, and keeps at 0 for ice. ScrollBreath runs that state (its pointer, set as the breath starts, is
 *   this) and then, with the scroll, puts the DS's speed in at once: KatAM's ice breath bends as the DS's, its fire breath as far and
 *   as fast as the DS's; */
#include "global.h"
#include "kirby.h"
#include "constants/kirby.h"
#include "squeak.h"

// The scrolls' numbers (the DS's);
enum {
    SCROLL_FIRE,
    SCROLL_ICE,
};

// SCROLL_BREATH_AIM; prmKirbyFire.kgb's and prmKirbyIce.kgb's words 3 and 4 (6144, -6144): a breath's puff's height speed with Up or
// Down held;
#define SCROLL_BREATH_AIM SQUEAK_SPEED(6144)

extern void sub_0805D268(struct Kirby *kirby);

// ScrollBreath(); KatAM's fire and ice breath state (sub_0805D268), then the DS's Fire and Ice scrolls' aim (see above);
void ScrollBreath(struct Kirby *kirby) {
    sub_0805D268(kirby);
    if (kirby->stateFn != ScrollBreath) return;
    if (!(kirby->ability == KIRBY_ABILITY_FIRE ? SqueakScrollOwned(SCROLL_FIRE)
          : kirby->ability == KIRBY_ABILITY_ICE && SqueakScrollOwned(SCROLL_ICE))) return;
    if (kirby->movementState & 0x40) kirby->base.counter = SCROLL_BREATH_AIM;
    else if (kirby->movementState & 0x80) kirby->base.counter = -SCROLL_BREATH_AIM;
    else kirby->base.counter = 0;
}
