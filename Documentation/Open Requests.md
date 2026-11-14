# Open requests

What the owner has asked for that is still to do: only what's open. When an item is finished, take it out (the pull request that
finishes it says how it was checked). The design rules for doors, mirrors and hubs are in
[Map and Door Design Rules](Map%20and%20Door%20Design%20Rules.md).

## 1. Squeak Squad: lag
- The owner: "Some squeak lairs lag a TON, we need to fix that." (Measured again, USA, every mini-boss room the expansion makes, 600
  frames each from Kirby's start at (40, 159) with Sword, B every 12 frames: 0x51E, 0x537 and 0x58A dropped 16 frames, the rest 0-3.
  Fixed since: Moving()'s 40 tests each frame for every Kirby without Animal or Ninja (the three now 4, the rest 0-2), Dark
  Nebula's and Daroach's sounds. Which lair still lags for the owner, and with what, wanted.)
- The owner: "Squeak lairs are very prone to lag and garbage tiles." (Garbage tiles not reproduced: six lairs' fights, every
  sprite's VRAM its own, the heap at most 338 of 512. Room 0x49C's garbled sprites were the gimmicks' VRAM, fixed.)
