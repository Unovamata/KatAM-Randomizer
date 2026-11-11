# Open requests

What the owner has asked for that is still to do: only what's open. When an item is finished, take it out (the pull request that
finishes it says how it was checked). The design rules for doors, mirrors and hubs are in
[Map and Door Design Rules](Map%20and%20Door%20Design%20Rules.md).

## 1. Squeak Squad rooms: tiles, blocks, looks
- **Animated tiles, water currents included** (the owner: "Some tiles are not animated. Please ensure you port all the animated
  tiles."; "Currents in water don't have any effect on the player."; "Fix animated tiles in general, that's where currents fit
  in."): every tile animation Squeak Squad's rooms have, and the water currents pushing Kirby.
- **Walk-once blocks** (the owner: "Blocks that you can only walk once and they get destroyed after do not work"; "Walk-once
  blocks, check first image": that image didn't come through, only one image arrived, the invincibility screenshot).
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

## 3. Squeak hideouts (lairs)
- **A mini-boss's attack stuck after a cancel** (the owner: "One of the minibosses in the Squeak hideouts activated an attack and I
  canceled, however the attack got locked in there."; "check the second image attached": that image didn't come through).

## 4. Squeak Squad's final sequence (the owner: "Complete the final sequence.")
- "I can pretty much invalidate the first fight of the final sequence [...] the mirror to go to the next phase is literally right
  there."
- "Why is there a door in daroach's first phase fight [...] the warp should send me to the next phase automatically."
- "Daroach when beaten on the first phase doesn't trigger the expected animation when it dies."
- "When going from phase to phase."
- "Color palette and maybe object errors in the first Squeak Squad final boss fight."
- "Massive amounts of lag whenever the Squeak Squad boss invokes stars."
- "The 1st phase of the Squeak Squad boss goes out of the screen, this is really prevalent with bosses in general"
- "The ray of light summoned from Daroach's wand attack is cutoff in the middle".

## 5. Dark Nebula
- "massive lag when summoning stars. You can use the game's existing stars"
- "fire attack looks very off and it's very laggy"
- "When using its ice attack, it doesn't add the ice platforms at the bottom."
- "Dark Nebula's sprite looks off."
- "Dark Nebula never activates its electric attack."
- "Kirby's dance is off centered in Dark Nebula's room."

## 6. Hubs and the Map Viewer
- "The fountain of dreams hub doesn't have the base of the master sword."
- "Squeak Squad's bosses are not showing in the Map Viewer app."
- "The squeak squad final boss sequence is not within this same line in the map viewer."

## 7. Polish the Squeak Squad hubs
- The owner: "Add a pending tasks to the list that we need to thoroughly polish the Squeak Squad hubs." The Squeak Squad hub (the
  Central Circle copy, its grey door, its mirror and doors' places and looks, still provisional) and the 8 world switch rooms (their
  provisional tiles, statues and pedestals): a thorough pass on looks and layout, the owner's picks first (the rulebook's frozen
  designs stay as they are unless the owner says).

## Waiting on the owner
- **Bohboh's lava**: Squeak Squad's stage draws a lava pool 128 pixels across the middle of his arena, and Bohboh brings in an
  invisible object over it that hurts whoever touches it (and makes the lava's "floor" its dives and Kirby's throw end in). KatAM's
  rooms have no lava, so that hurting strip is invisible on the floor's middle, as ported 1:1. It can stay, be made harmless (kept
  only as the line its dives and throws end in), or be shown somehow.
