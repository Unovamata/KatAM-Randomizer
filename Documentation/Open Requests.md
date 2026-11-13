# Open requests

What the owner has asked for that is still to do: only what's open. When an item is finished, take it out (the pull request that
finishes it says how it was checked). The design rules for doors, mirrors and hubs are in
[Map and Door Design Rules](Map%20and%20Door%20Design%20Rules.md).

## 1. Squeak Squad: lag
- The owner: "Some squeak lairs lag a TON, we need to fix that." (Current build, USA, 900-frame fights with Sword: lairs 0x412 and
  0x42C 12-14 frames over the GBA's time, 0x41E 1, Daroach's 0x4AB 1; most of a heavy frame KatAM's own object updates; the
  Squeaks' chest search 4-5% of it. Which lair wanted.)
- The owner: "Daroach ray causes lag, we need to reduce it."
- The owner: "Dark nebula stars are still lagging the game, we need to solve that."
- The owner: "Squeak lairs are very prone to lag and garbage tiles." (Garbage tiles not reproduced: six lairs' fights, every
  sprite's VRAM its own, the heap at most 338 of 512. Room 0x49C's garbled sprites were the gimmicks' VRAM, fixed.)

## 2. Squeak Squad: the ability room, the hub, the boss rooms
- The owner: "Second phase of Mecha Kracko doesn't change the room's structure like it does in Squeak Squad."

## 3. Squeak Squad: rooms and objects
- The owner: "Crimps do nothing in-game. They can only attach to walls but I have seen some even walking around on ledges or not
  stiching to walls."
- The owner: "Room 0x44a heavy palette errors." (the owner, after: "0x44A did not randomize enemies, move kirby around ignoring
  collisions so see what I'm talking about") Not reproduced in an expansion-only build, USA: Kirby moved by RAM through the whole
  room (walls ignored, a snake 32 pixels a step, 175 steps) and the same path on the DS (its 2-3-7), 43 shots side by side, the
  tiles, far view and enemies in the same colors; 6 of KatAM's 7 enemy palette slots used. A sweep of every Squeak Squad room the
  same way: 8 load an 8th palette with all 7 in use on the screen (0x43E, 0x458, 0x464, 0x4EE, 0x4FA, 0x534, 0x591, 0x598: their
  enemies' kinds and gimmicks' looks), where KatAM's last slot is shared and a sprite shows in another's colors; 0x44A isn't one.
  A screenshot, or the build's seed and settings, wanted.
- The owner: "Room 0x5DB 2 pedestals on top of one another." (Not reproduced in an expansion-only build: one random pedestal,
  object 0x96, at 344, 80; the seed and settings wanted.)
- The owner: "Room 0x5E0 garbled tiles for a single frame." (Not reproduced, USA: every frame of coming in by 0x5E1's mirror and
  of the dark star's first 900 frames checked; only its flashes and the screen's shake at Dark Nebula's coming; a 2400-frame fight
  (no sprite VRAM short, 244 of 512 at most) and 2400 frames from his HP forced to 0 through his defeat, every frame compared with
  its neighbours: none unlike both. When in the fight it shows is wanted.)
- The owner: "Can't get inside barrels and the barrel destruction animation looks off. Room 0x420 feels empty, like something's
  missing. Also, double door clash."
- The owner: "Squeaks have no hitbox and don't try to hit Kirby at all." (Current build, USA: Kirby put on a Squeaker in lairs
  0x412 and 0x42C loses 1 HP, the hit the Squeaker's; which room and what they should do wanted.)

## 4. New abilities' sprites
- The owner: "Some sprites for the new abilities are absolutely bugged Needle and bubble to name some examples, wrong layering,
  wrong facing, among other problems." (with a screenshot of Ninja Kirby in a Squeak Squad room, a shuriken beside him)

## 5. Squeak Squad: abilities and Animal's digging
- The owner: "Squeak Squad Minibosses do not give their supposed ability." (Big Metalun, Gao Gao and Buboo now hold Metal, Animal
  and Bubble; Tedhaun's Ghost isn't in the ROM Expansion. Not yet seen end to end in the emulator: beating one and inhaling it.)
- The owner: "They way tiles get destroyed when diggable when using animal is not how Squeak Squad draws them."
- The owner: "Cannot dig up with animal."
- The owner: "When items are inside dirt like Tomatos, Lives, and such, they go through the ground instead of staying inside the
  dirt."
- The owner: "Some animal attacks are not digging the dirt."

## 6. Squeak Squad: bosses
- The owner: "Dark Nebula's ice platforms don't act as ice."
- The owner: "Dark Nebula fire attack doesn't have the white area in the center." (Current build, USA: his fireball's fire wall, forced
  by RAM in room 0x5E0, shows its white middle; which attack, or a screenshot, wanted.)
- The owner: "Dark Nebula's electric attack has missing tiles." (Current build, USA: his bolts and cage, forced by RAM, draw whole;
  the cage's top is at the view's top edge (KatAM's view 32 pixels shorter than the DS's, his room's camera still); the heap at most
  426 of 512 tiles then. A screenshot wanted.)
