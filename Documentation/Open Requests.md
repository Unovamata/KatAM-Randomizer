# Open requests

What the owner has asked for that is still to do: only what's open. When an item is finished, take it out (the pull request that
finishes it says how it was checked). The design rules for doors, mirrors and hubs are in
[Map and Door Design Rules](Map%20and%20Door%20Design%20Rules.md).

## 1. Squeak Squad rooms: tiles, blocks, looks
- **Palettes** (the owner: "There's a lot of color palette problems with enemies and stages."). Fixed so far: the three candle
  rooms' black terrain (4-3-7, 4-3-8, 8-1-6: their colors are their `PL` palettes). The owner's way: a full sweep of every Squeak
  Squad enemy and stage against the DS, with a debug mode that flies Kirby through the levels fast, ignoring collisions (to stress
  the levels as well).

## 2. Squeak Squad objects
- **What an Ability Scroll does**: the scrolls are owned and saved, they don't do anything yet. The owner: "For KatAM existing
  abilities, the scroll will upgrade them with the DS moveset 1:1. However, for non existing abilities you must port the base
  ability 1:1 and then the upgrade of it 1:1". So: each scroll's ability's DS upgrade, 1:1, for the abilities KatAM has (Fire, Ice,
  Spark, Beam, Bomb, Cutter, Hammer, Parasol, Sleep, Sword, Throw, Tornado, Wheel); the abilities it lacks (Animal, Bubble, Ghost,
  Metal, Ninja and the rest the scrolls name) ported 1:1 first, then their upgrades. The owner's order: KatAM's abilities first. The DS's scroll check isn't found yet (no
  constant search finds it: the ability code must compute the treasure's number).

## 3. Polish the Squeak Squad hubs
- The owner: "Add a pending tasks to the list that we need to thoroughly polish the Squeak Squad hubs." The Squeak Squad hub (the
  Central Circle copy, its grey door, its mirror and doors' places and looks, still provisional) and the 8 world switch rooms (their
  provisional tiles, statues and pedestals): a thorough pass on looks and layout, the owner's picks first (the rulebook's frozen
  designs stay as they are unless the owner says). The owner's pick: a pass of my own, then before/after screenshots.
