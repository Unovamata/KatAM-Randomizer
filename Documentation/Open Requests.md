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
