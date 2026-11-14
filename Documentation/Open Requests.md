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
- The owner: "Can't get inside barrels and the barrel destruction animation looks off. Room 0x420 feels empty, like something's
  missing. Also, double door clash." (the owner, after: only empty barrels open, as the DS's; what 0x420 misses: its enemies and the
  Squeakers' chase)
- The owner: "Squeaks have no hitbox and don't try to hit Kirby at all." (the owner, after: in the chase rooms; in the lairs they
  hurt Kirby and throw bombs)

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
