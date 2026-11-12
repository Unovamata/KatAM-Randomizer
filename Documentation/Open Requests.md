# Open requests

What the owner has asked for that is still to do: only what's open. When an item is finished, take it out (the pull request that
finishes it says how it was checked). The design rules for doors, mirrors and hubs are in
[Map and Door Design Rules](Map%20and%20Door%20Design%20Rules.md).

## 1. Squeak Squad objects
- **What an Ability Scroll does**: the scrolls are owned and saved, they don't do anything yet. The owner: "For KatAM existing
  abilities, the scroll will upgrade them with the DS moveset 1:1. However, for non existing abilities you must port the base
  ability 1:1 and then the upgrade of it 1:1". So: each scroll's ability's DS upgrade, 1:1, for the abilities KatAM has (Fire, Ice,
  Spark, Beam, Bomb, Cutter, Hammer, Parasol, Sleep, Sword, Throw, Tornado, Wheel); the abilities it lacks (Animal, Bubble, Ghost,
  Metal, Ninja and the rest the scrolls name) ported 1:1 first, then their upgrades. The owner's order: KatAM's abilities first.
  Where the DS upgrade builds on a DS move KatAM's ability lacks (Spark's charge levels), the owner: "Keep KatAM's base as it is,
  however, with the upgraded abilities that the scrolls give, we will then implement the quirks the base ability has plus the upgraded
  ones. The bases are only applied if they still applied to the upgraded ability." So: no scroll, KatAM's ability unchanged; with it,
  KatAM's ability gains the DS ability's own quirks and its scroll's, a DS base quirk only where the upgrade doesn't change it.
  The DS's scroll check is HasScroll (0x02020AD8), asked by each ability's code (`ROM Expansion/scrolls.c` lists the numbers). The
  23 scrolls are Fire, Ice, Spark, Beam, Tornado, Parasol, Hammer, Cupid, Cutter, Laser, Sword, Wheel, Hi-Jump, UFO, Sleep, Bomb,
  Ninja, Fighter, Throw, Magic, Animal, Bubble, Metal: the ones KatAM (or the NiDL port) has also count as KatAM's (Laser, UFO,
  Fighter, Magic, Hi-Jump too). Done: Fire, Ice, Spark, Beam, Tornado, Parasol, Sleep. Next: the rest of KatAM's (Hammer, Cupid,
  Cutter, Laser, Sword, Wheel, Hi-Jump, UFO, Bomb, Fighter, Throw, Magic), then Ninja, Animal, Bubble, Metal (KatAM has no such
  ability: each ported 1:1 first).

## 2. Polish the Squeak Squad hubs
- The owner: "Add a pending tasks to the list that we need to thoroughly polish the Squeak Squad hubs." The Squeak Squad hub (the
  Central Circle copy, its grey door, its mirror and doors' places and looks, still provisional) and the 8 world switch rooms (their
  provisional tiles, statues and pedestals): a thorough pass on looks and layout, the owner's picks first (the rulebook's frozen
  designs stay as they are unless the owner says). The owner's pick: a pass of my own, then before/after screenshots.
