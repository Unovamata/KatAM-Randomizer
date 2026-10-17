# Open requests

Everything the owner has asked for that is still open, in detail, plus the finished items that set a rule to keep. When an item
is done, move it to "Done" and say how it was checked. The design rules for doors, mirrors and hubs are in
[Map and Door Design Rules](Map%20and%20Door%20Design%20Rules.md).

## Still open

### 1. Save door visits, and show them in game from the save
- Which doors Kirby has been through (and which chest shards he's taken) must be kept in the save file, for KatAM's rooms and
  the NiDL rooms alike. Today the NiDL rooms' door visits aren't saved: they share one slot of the game's visited-doors table.
- What the game shows must follow the save: load a save and the pause screen's maps show the doors and rooms that save visited.
  This is one of the owner's biggest complaints.

### 2. Consumable chests in the NiDL rooms
- Add chests holding consumables (food, Maxim Tomatoes, 1UPs, candy, batteries and the like) to the NiDL rooms, the way KatAM uses
  them: as a reward for exploring (a dead end, a secret room, behind breakable blocks, the end of a branch), for a challenge (a hard
  jump, a room full of enemies) or as a surprise.
- Never in a boss room, a switch room or a mini-boss's fight area.
- They then count as consumable chests for the randomizer (the shards in chests option may replace them).

### 3. Master Sword toggle
- A randomizer option: the Master Sword in the hub is unlocked from the start of the game.

### 4. NiDL Meta Knight fight keeps Kirby's own abilities
- The fight starts with whatever abilities the player has. Don't give a sword, and don't add a sword pedestal.

### 5. Bosses as faithful 1:1 recreations of NiDL
- **Heavy Mole**: rework the fight to match NiDL's, which needs the boss rooms to support scrolling (the screen moves through the
  dirt with the mole).
- **King Dedede**: he must inhale Kirby (and the helper Kirbys), swallow them and spit them out, as in NiDL.
- **Nightmare's Power Orb**: port the Star Rod flying sequence, for the player and for all the helper Kirbys.
- **Every boss** must play like NiDL's. Whispy Woods' face is done (see below); check the others against NiDL.

## Done (rules to keep)

- **Mirror shard options**: field shards (Unchanged, Random, Custom 1-5 copies of the 8, lying in rooms) and **Add shards in
  chests** (off by default, 1-3 copies of the 8, each in a chest that held a consumable, never a treasure) work separately or both
  at once. Off means no shards in chests. Shard chests are left out of the pause screen's chest lists. Checked: with 1 field copy and
  3 chest copies, 8 field shards and 24 shard chests (3 of each shard), 17 consumable chests left; a converted chest opened in the
  emulator and gave its shard; USA, European and Japanese builds.
- **NiDL boss shards send Kirby back to the NiDL hub**, onto that level's door, the way KatAM's go to Central Circle. The Power Orb's
  alone goes on to Nightmare's room (the rest of NiDL's final fight). NiDL's empty walk-out rooms after a boss (4 of them) are left
  out of the port, since nothing leads there any more. Checked: the shard exit table, the post-shard state run in the emulator from
  Shine & Bright's room (Kirby arrived in the NiDL hub at Butter Building's door), and the logic checks (all 280 rooms reached, no
  traps) on all three versions.
- **Map viewer, unrandomized ROMs**: KatAM's map stays exactly the pause screen's (`MapVanilla`), and the NiDL levels go around it,
  never in its gaps. Checked: no NiDL level room inside KatAM's map (only the NiDL hub, beside Central Circle, as before).
- **Bosses give random shards** is a toggle, off by default: KatAM's 8 bosses get the 8 shards shuffled, then the NiDL bosses
  spread the 8 evenly. Tested: Kracko's new shard dropped, was collected, and its rematch left a tomato (HP zeroed in RAM, not a
  real fight).
- **Boss rooms have no doors out.** The shard is the only way out. A boss room is the end of a branch, and any one-way door that was
  in one was moved elsewhere. The logic checks confirm it.
- **Hubs** have at most 2 regular doors; hub doors are counted apart. The world map and the interconnected hub links stay as
  approved.
- **Skip the tutorial** (Miscellaneous > Game): a game starts as a continued save does, in Central Circle, with no intro, tutorial
  or mirror cutscene, and the tutorial chest's item given once, even with a save made by another ROM.
- **Debug click-to-warp** in the Map Viewer (Debug menu): clicking a room moves Kirby there. Debug only: remove it before release.
- **Whispy Woods' face** is drawn as NiDL draws it.
- **Warp stars** float where NiDL puts them; **cannons** sit on their fuse exactly as KatAM's do (slightly off-centre: the fuse
  enters the cannon's left half).
- **Settings window**: Load and Save Settings on their own rows; Skip the tutorial in the Miscellaneous tab.
- **Map viewer, randomized ROMs**: whole clusters, no 2-4 room islands, levels packed tight.
