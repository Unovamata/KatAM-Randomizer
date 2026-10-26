# Open requests

What the owner has asked for that is still to do. When an item is finished, take it out. The design rules for doors, mirrors and
hubs are in [Map and Door Design Rules](Map%20and%20Door%20Design%20Rules.md).

## 5. Bosses as faithful 1:1 recreations of NiDL
- **The other bosses**: check each against NiDL and fix what differs.
- **Differences left in the recreated fights** (KatAM's own engine, not changed): KatAM sees a hit a frame after NiDL; each boss's
  defeat starts with KatAM's explosion instead of NiDL's flash and burst of stars (Meta Knight's knockout too, where NiDL holds him
  53 frames on a dark screen); KatAM's nearest sounds; the flashes, the Kracko eye's spin and the Star Rod's shine count from the
  fight's start (NiDL's from its power-on clock, so they differ from game to game there too); KatAM's Kirby's box reaches 2 pixels
  lower than NiDL's; KatAM's inhale reach and pull and Kirby's knock-back when hurt.
- **Meta Knight**: the thrust's screen shake as he lands (NiDL's `0x080261D4`; the owner: keep it, to be done later); NiDL's hit
  kinds 4 and 6 (states 16 and 23: inhaled, a hit on his box 0), which none of KatAM's attacks are, not ported.
- **Mr. Shine & Mr. Bright**: when one is beaten, KatAM's hit stars stand in for NiDL's effects (`0x95`, `0xA2`), and shots still in
  the air fall on instead of popping; NiDL's 31 frames without hits after a hurt and Kirby's box for their shots while he floats
  aren't modelled. Ask the owner: at their fight's camera the room's two back layers sit about 4 pixels further right than NiDL's,
  and its foreground colours show a step brighter (KatAM's fade in from white stops a step short).
- **King Dedede**: not traced: NiDL's Kirby's own box, whether NiDL takes a hit on the 31st frame after one, whether a blinking
  Kirby is caught. Ask the owner: KatAM's screen in his room sits 8 pixels left of and 16 above NiDL's (24, 168).
- **Paint Roller**: ask the owner whether NiDL's painting touch (knocked back 20 frames, a white burst, `0x98`) should be ported:
  they die KatAM's way at once now.
- **Heavy Mole**: its hatch's sound ends differently from NiDL's. Found while testing, ask the owner: when the mole digs up high,
  the camera goes up with it (NiDL's), and a Kirby who stays on the dirt floor below can walk along there under the screen, out
  of sight (the fight keeps him up to 39 pixels below it rather than beating him, as NiDL's bottom does).
- **Poppy Bros. Jr.**: the tomato or apple comes off a frame after KatAM's inhale takes the rider (NiDL's the same frame), at 0.5 a
  frame (NiDL's 0.496 to 0.498, from a small random angle); KatAM's Maxim Tomato, not NiDL's blinking one.

## 37. Mini-boss cameras and Mike
- **Mini-boss cameras in rooms bigger than a screen** (the owner: cameras always exactly NiDL's): the fight's screen is NiDL's,
  but the way there isn't yet: NiDL's camera follows Kirby with no easing, slides to the fight's screen 2 pixels a frame once the
  room's trigger object (type 4) comes on, and in the tall rooms 0x09B and 0x27C goes on following him up and down (the port holds
  it); 0x244 has two holds.
- **Mike's flash**: NiDL's screen goes yellow, the port's brightens to white.

## 34. The Fountain of Dreams finale: differences from NiDL
- **The ending**: the Kirbys ride in KatAM's pose (NiDL's Kirby holds the Star Rod up); KatAM's HUD stays on (in the Power Orb's
  room it covers the cloud band's bottom); the room's own music plays, not NiDL's ending music; the ending's background is NiDL's
  mirrored left to right; at Nightmare's defeat his flight off starts after KatAM's burst, not 52 frames after the hit as in NiDL.
- **The ride into the moon**: the star and the Kirbys are KatAM's warp star sprite (NiDL's star is four 16x16 pieces, with Kirby
  in his rod pose); the star's dot is 1-2 pixels off NiDL's.
- **Nightmare's intro**: Kirby's pose (NiDL's holds the Star Rod up), KatAM's HUD, NiDL's random twinkling stars.
- **Nightmare's defeat**: may show a wrong palette. Not reproduced (NiDL's defeat and five of ours compared frame by frame, all
  three versions). Needs the owner's details: when in the defeat, what it looked like (a screenshot if possible), which version.

## 45. The Star Rod: questions for the owner
- The rod isn't drawn in the poses its hat has no NiDL frame for (swimming, climbing a ladder and a few others:
  `KatAMExpansionHats.cs`' `HatFrames`); its stars fly through walls (as in the finale's fights); outside the fights the HUD card
  shows the rod's picture, as for any ability.
- Only the Star Rod's hat is drawn behind Kirby where NiDL draws it behind him; the other new abilities' hats have 1-4 such
  frames each and are left as they were. Say if they should follow too.
- Its stand unlocks with KatAM's ending (as the Master Sword's). Should beating Nightmare unlock it instead, and should the
  "Unlock the Master Sword" option unlock the rod too?

## 43. Kracko's stage's lag
- Not reproduced (1 lag frame in 3400 alone, 3 with the three helpers). Ask the owner when it lags (the climb, the cloud, the
  lightning), on which emulator or console, with helpers out or not, and on which seed.

## 51. NiDL rooms' backgrounds at a tall room's top
- The NiDL rooms' background rates come from the formula the NiDL hub's did (`BackgroundRates()`), which leaves out the 40 pixels
  KatAM's camera keeps above a room's bottom, so at a tall room's very top its background can start a little above its first row
  (the hub's garbled band). Only the hub was reported and changed: say if the rooms should be checked and changed as well.

## 10. Screen shake as KatAM's
- The owner: the screen shakes far too much. Mr. Shine & Mr. Bright's and King Dedede's endless shakes are fixed; anything else
  that shakes too much: tell where.
