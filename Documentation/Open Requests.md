# Open requests

What the owner has asked for that is still to do: only what's open. When an item is finished, take it out (the pull request that
finishes it says how it was checked). The design rules for doors, mirrors and hubs are in
[Map and Door Design Rules](Map%20and%20Door%20Design%20Rules.md).

## 1. Squeak Squad rooms: tiles, blocks, looks
- **Layering** (the owner: "Some interactible blocks have some layering issues, therefore they show behind the stage.").
- **Palettes** (the owner: "There's a lot of color palette problems with enemies and stages.").
- **0x5E5's camera** (now room 0x5DE; the owner: "Room 0x5E5 camera moves without me"): not reproduced yet.

## 2. Squeak Squad objects
- **The Ability Scroll chests' icon** (the owner: "Ability scroll chests do not display the proper icon they should be
  displaying."): their popup shows a KatAM map item in the parchment palette; it should be the DS's scroll.
- **What an Ability Scroll does**: the scrolls are owned and saved, they don't do anything yet.
- **The chase-only chests**: the 5 Ability Scroll chests the DS brings out only in the chase are there from the start; the chase's
  other chests (five of them the Vitality halves) aren't placed.
- **The candles and the Light ability** (the owner: "Add to the pending list candle and light ability interaction", then: "the
  light ability should light up Squeak Squad sections that require the candle if NiDL expansion is present"): with the NiDL ROM
  loaded, NiDL's Light ability (from Cool Spook) lights the Squeak Squad dark rooms' parts the candles light, as Fire does on the DS.

## 3. Squeak Squad's final sequence (the owner: "Complete the final sequence.")
- "When going from phase to phase." (the sentence ends there): the phases now go on by themselves (no mirror), Dark Daroach's
  room once his defeat has played out and the ride's landing room 30 frames after Kirby lands; what else the owner meant here is
  still to ask.
- "Color palette and maybe object errors in the first Squeak Squad final boss fight.": checked against the DS's own files, the
  striped frames when Dark Daroach warps (`dancho00`'s 5-7) and his red bombs are the DS's (Dark Daroach's palette); his hit flash
  is still KatAM's, not the DS's `dan_dmg` palettes. Which errors the owner saw is still to ask.
- "Massive amounts of lag whenever the Squeak Squad boss invokes stars.": measured in the emulator, his beam (4 seconds) runs at
  half speed: the sound mixer takes about a fifth of a frame more while it plays, and its 15 sprite pieces the rest. His idle,
  bombs and warps keep full speed. Not fixed yet.

## 4. Dark Nebula
- "massive lag when summoning stars. You can use the game's existing stars"
- "fire attack looks very off and it's very laggy"
- "When using its ice attack, it doesn't add the ice platforms at the bottom."
- "Dark Nebula's sprite looks off."
- "Dark Nebula never activates its electric attack.": not reproduced. In a 6000-frame run in the emulator he took his electric
  form twice and used its wave and bolts, the bolts flying out and the electric cage round the screen's edges (screenshots).
  Still to ask whether this was an older build or his electric form simply didn't come up in that fight.

## 5. Hubs and the Map Viewer
- "The fountain of dreams hub doesn't have the base of the master sword.": the hub has KatAM's Master Sword stand object (0x98,
  `FinaleSwordAt`), which draws only the sword; Central Circle's base is part of its room's tiles, which the hub (NiDL's tiles)
  doesn't have. To do: bring Central Circle's pedestal tiles into that hub's tileset under the stand.
- "Squeak Squad's bosses are not showing in the Map Viewer app."
- "The squeak squad final boss sequence is not within this same line in the map viewer."

## 6. Polish the Squeak Squad hubs
- The owner: "Add a pending tasks to the list that we need to thoroughly polish the Squeak Squad hubs." The Squeak Squad hub (the
  Central Circle copy, its grey door, its mirror and doors' places and looks, still provisional) and the 8 world switch rooms (their
  provisional tiles, statues and pedestals): a thorough pass on looks and layout, the owner's picks first (the rulebook's frozen
  designs stay as they are unless the owner says).

## Waiting on the owner
- **Bohboh's lava**: Squeak Squad's stage draws a lava pool 128 pixels across the middle of his arena, and Bohboh brings in an
  invisible object over it that hurts whoever touches it (and makes the lava's "floor" its dives and Kirby's throw end in). KatAM's
  rooms have no lava, so that hurting strip is invisible on the floor's middle, as ported 1:1. It can stay, be made harmless (kept
  only as the line its dives and throws end in), or be shown somehow.
