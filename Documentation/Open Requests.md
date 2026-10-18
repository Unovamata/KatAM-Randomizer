# Open requests

Everything the owner has asked for that is still open, in detail, plus the finished items that set a rule to keep. When an item
is done, move it to "Done" and say how it was checked. The design rules for doors, mirrors and hubs are in
[Map and Door Design Rules](Map%20and%20Door%20Design%20Rules.md).

## Still open

### 1. Title screen: NiDL enemies in the parade
- When the ROM Expansion is on, the random title parade can include its ported NiDL enemies. The parade has its own character
  table, so they need new entries there pointing at the expansion's sprites, within the intro's 16-palette limit.

### 2. Save door visits, and show them in game from the save
- Which doors Kirby has been through (and which chest shards he's taken) must be kept in the save file, for KatAM's rooms and
  the NiDL rooms alike. Today the NiDL rooms' door visits aren't saved: they share one slot of the game's visited-doors table.
- What the game shows must follow the save: load a save and the pause screen's maps show the doors and rooms that save visited.
  This is one of the owner's biggest complaints.

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

### 10. Screen shake as KatAM's
- The screen shakes far too much; something broke along the way. Match KatAM's screen shake.
- Not reproduced yet; needs the owner's details (which room, what was happening). Checked so far: KatAM's shake itself runs the
  same in NiDL rooms as in KatAM's (a strength-2 shake written into its task in RAM: the same 16 frames of 2 pixels in 0x0F0 and in
  Central Circle); the expansion's code asks for a shake only when Grand Wheelie hits a wall and when Bugzzy slams Kirby, once each
  (strength 3, as KatAM's mini-bosses); NiDL's Shotzos are KatAM's Shotzo (0x35), which doesn't shake, not its 8-way cannon (which
  shakes on every shot); 600 frames of the 0x164 boss fight shook 4 times, each when Kirby was hit (62 frames). KatAM shakes on every
  hit on an enemy (1), every enemy defeated and every hit on a boss (3), so randomized mini-bosses in enemy spots ("Include
  Minibosses") shake the screen as often as they do in KatAM.

### 20. Room 0x147: falling doesn't kill Kirby
- The owner asked whether that's expected. It isn't: the room has no fall-out exit (its only ways out are its two doors), so a fall
  should cost a life. To look into.

## Done (rules to keep)

- **Warp star rides through NiDL's skies** (item 8, rooms 0x2F9 and 0x280): Kirby arrived on the sky room's bottom row and its edge
  exit sent him straight on, a white flash. A warp star into a sky room now has a flight of its own (`SkyFlight()`): KatAM's flight
  up, then the star carries Kirby across the sky in an arc, trailing its sparkles, and out at the bottom into the next room (its
  bottom exit: the top row is out of Kirby's reach in KatAM). Tested in the emulator (USA, new games started beside the stars, not
  playthroughs): from 0x2EB, the star arced across 0x2F9 left to right and Kirby arrived in 0x2EC; from 0x27E, it rose through
  0x280 to near its top, came down the other side and Kirby arrived in 0x27F; started with the helpers on the star, the pink,
  yellow and green Kirbys rode it together and all arrived in 0x2EC. It's KatAM's own star and flight, drawn for these rooms, not
  NiDL's ride animation (NiDL's star scrolls the sky past Kirby; KatAM's camera follows him). Logic and chest checks pass on all
  three versions, with and without dropping through thin floors.

- **Goal rooms drawn beside each room leading to them** (item 7): the map viewer splits each goal game (0x3D4-0x3D6: 9, 14 and 12
  rooms' goal mirrors lead to them) into a copy beside each of those rooms (`RomMap.SplitGoalRooms()`), in that room's NiDL level
  when it's a NiDL room; arriving in a goal game marks the copy beside the room Kirby came from (`ResolveRoom()`), and the debug warp
  and the room caption use the real room. Checked with the viewer's layout code outside the app (renders of an unrandomized and a
  mirror-randomized expanded USA ROM): 35 copies, each 1 cell (48 pixels) from its room, at most 4 where the map is crowded; the long
  lines to the goal rooms are gone; the unrandomized ROM is still recognised as KatAM's original map and its 263 area map rooms are
  exactly where they were. Not tried in the app with the emulator connected, and the app itself wasn't built here (no Windows
  desktop SDK in this container): its two edits, in `TrackerForm.cs` and `MapView.cs`, are single calls to `RomMap`.

- **NiDL rooms spaced as KatAM's on the map viewer** (item 13, rulebook section 11): KatAM's rooms are never nearer each other than
  24 pixels edge to edge (32 most often); NiDL's were 16 (8 by a big room). The levels are now laid out on a 48-pixel grid (6 tiles),
  linked rooms a step apart (32 pixels between two small rooms), no room nearer another than 24 (`MapExpansion.LayoutLevel()`).
  Measured on the unrandomized expanded USA ROM: nearest-room gaps 24 (31 rooms, the big ones) and 32 (211), as KatAM's 24 and 32;
  the map grew from 2976x3216 to 3336x3576 and stays square. On a mirror-randomized ROM two rooms sit 3 cells from their level's
  nearest room (a goal copy kept off a crowded corner, and 0x1B1). Not tried in the app.

- **NiDL backgrounds in their own colours** (item 19, 0x1B9): Grape Garden 2's sky (rooms 0x1B9-0x1BE) has a 7-palette palette that
  NiDL loads from palette 9 (its map uses 9-15); it was loaded from 8, so every tile had its neighbour palette's colours (the blocky,
  patched sky). A shorter palette now ends at palette 15. Found with it: rooms sharing a background got the first room's colours,
  while eleven of NiDL's backgrounds have a second colouring (Orange Ocean's sea at sunset is Ice Cream Island's, 0x27B and 0x0F0;
  the others: 0x164, 0x274-0x277, 0x234, 0x25F and its stage, 0x160, 0x2D5, 0x2D6, 0x275/0x27A, 0x230/0x231, 0x2D4/0x2D9); each
  colouring is now its own palette, sharing the tiles and map. Tested in the emulator (USA, new games started in each room, not
  playthroughs): 0x1B9's sky is NiDL's clouds, 0x27B's sea is orange at sunset, 0x0F0's stays blue. The other recoloured rooms
  weren't looked at one by one. Logic and chest checks pass on all three versions, with and without dropping through thin floors.

- **No empty dead ends** (item 12, room 0x298): a dead end with nothing in it at all (no enemy, item or boss) always gets a chest,
  over its level's count (0x298 and 0x150 got one: 30 NiDL chests). Tested in the emulator (USA, a new game started in 0x298, not a
  playthrough): the chest stood at (136, 136); Kirby stopped on it and pressed Up, it opened, gave its Meat and its treasure bit (103)
  was set. Logic and chest checks pass on all three versions, with and without dropping through thin floors.

- **Batteries in the NiDL rooms** (item 18): KatAM's battery (object `0x62`) at rewarding spots, made with the rooms
  (`PlanChests()`): the farthest floor behind breakable blocks, else the farthest of a dead end, a battery per 8 rooms of a level
  (2 to 5; 19 in all), one a room at most, never in a chest's room, a boss room, a switch room or a mini-boss's fight area; taken once
  per visit, as the rooms' items. Tested in the emulator (USA, a new game started in 0x236 beside it, the phone battery set to 1 by
  RAM; not a playthrough): Kirby walked into it, the battery went to 2 and it was gone. Coming back to its room wasn't tried for the
  battery itself (it uses the items' once-per-visit bit, tested for item 17). Logic checks pass on all three versions, with and
  without dropping through thin floors.

- **A mini-boss's room is one-way in and one-way out** (item 11, 0x29B): the doors into a room with a mini-boss and its own doors are
  one-way, their landings moved off the doors (to a floor nearby, else off the doors only within twice the reach, before anywhere in
  the room: one landed Kirby cut off, 0x29A's into 0x224). 0x29B's two doors stay where NiDL has them (4 cells apart: they're its own).
  102 doors became one-way (134 to 236). Tested in the emulator (USA, a new game started in 0x25F beside its door, not a playthrough):
  through the door into 0x29B, Kirby landed 3 cells from its door, the two Poppy Bros. Sr. fighting, the doors hidden. Logic checks
  pass on all three versions, with and without dropping through thin floors.

- **NiDL's grey blocks are KatAM's silver metal blocks** (item 9, the owner's picture: room 0x134's), collision `0xF5`, which only
  some abilities break; they were `0xF4`, which any ability's attack breaks (KatAM's helpers' block code, `code_0800ECAC_2.c`, tries
  only blocks of types 0 and 1; `0xF4` is type 1, `0xF5` type 2). All 39 in the NiDL rooms; Heavy Mole's dirt (the same NiDL kind,
  dug by any attack) stays star blocks. They keep NiDL's tiles (grey, bevelled, as KatAM's). Logic checks pass on all three versions
  (with metal blocks off: nothing they guard is needed). Not tested in the emulator: an ability given by RAM doesn't attack (the old
  `0xF4` blocks didn't break with it either), so which abilities break them in game wasn't seen.

- **Room 0x2FD's collision and cannons; bomb chains everywhere as NiDL's** (items 6 and 16): NiDL's invisible chain links (its `0x35`
  cells) are KatAM's own `0xF3` links, so a bomb block sets off every block NiDL links to it (0x2FD's ceiling and pits, 0x304's goal
  door chain from the bottom); bomb blocks are only where NiDL has them (the extra ones the port added are gone: every room's count now
  matches NiDL's, but 0x27C's and 0x294's, whose bomb block starts broken where a door lands Kirby; their chains are star blocks); 0x2FD's
  Shotzos (its cannons, on its breakable ceiling) are back. Checked: the rooms' collision against NiDL's map, the bomb counts in all
  rooms, and in the emulator (USA, Kirby started alone in 0x2FD with Bomb given by RAM, not a playthrough): one hit on the bomb block
  broke the ceiling (linked only through the link column), the chain and the pits; the Shotzos showed before. Logic checks pass on
  all three versions, with and without dropping through thin floors. 0x304 wasn't played in the emulator.

- **NiDL boss rematch: a Maxim Tomato, never the shard** (item 14): eating it gives the victory dance and takes Kirby where the shard
  would (the NiDL hub, onto the level's door). Tested in the emulator (USA, a new game started in Mr. Shine & Mr. Bright's room
  0x164 with shard 3 owned, the boss's HP zeroed in RAM, not a real fight): a tomato and no shard; Kirby walked into it, the Kirbys
  danced, and he arrived in the NiDL hub (room 0x001). All three versions build.

- **Grand Wheelie stands on the ground** (item 15): its terrain box ended 3 pixels under its drawn wheel. Tested in the emulator
  (USA, a new game started in room 0xEE, his fight): before, 3 pixels of sky under the wheel; after, the wheel's edge on the ground's
  first row. All three versions build.

- **NiDL's loose items are taken once per visit**, as KatAM's (item 17): each has its own gone bit (template unk2 3, `rooms.c`'s
  `NIDL_TAKEN`: KatAM's 256 item numbers are all used), cleared with KatAM's (a game's start, a game over...). Tested in the
  emulator (USA, a new game started in room 0xF7 on its 1UP, not a playthrough): eaten (lives 2 to 3), through the door to 0x100
  and back onto its spot: it stayed gone (lives 3); the ROM before the change gave it again (lives 4). Logic checks pass on all
  three versions.

- **NiDL chests, made by the ROM Expansion**: 28 consumable chests in the NiDL rooms (2 to 5 a level, none in the Fountain of
  Dreams), at reward spots (an item behind breakable blocks or in a dead end becomes a chest holding it; dead ends with none get a new
  one), made whether or not anything is randomized. They open once and stay open (treasure indices 84-111); the completion counts only
  KatAM's 84. **The randomizer reads them** with the NiDL rooms' loose items, so they're randomized, shuffled, emptied by No Chests and
  used by the chest shards like KatAM's. Also fixed: on an expanded ROM the randomizer's chest lists went to the table the game no
  longer reads, so a shuffled chest froze the game when opened. Checked: every chest (KatAM's and NiDL's) is in its room's list in the
  table the game reads, with chests Unchanged, Shuffle and No Chests; every NiDL chest is reachable (2 behind metal blocks, which need
  an ability); logic checks pass on all three versions, with and without dropping through thin floors. In the emulator (USA, a new
  game started beside the chest, not a playthrough): a NiDL chest in room 0x1C2 opened, its index was set, it showed open after and the
  game went on; a chest Shuffle moved into KatAM's room 0xB2 opened and the game went on, where the same ROM with the old table entry
  froze. Not tested in the emulator: the European and Japanese ROMs, and the completion percentage.

- **The ROM is made in the background**: the progress bar at the bottom follows each step (with its name beside it), the rest of the
  window is greyed out until it's done, and any warnings show once it ends. Compiled; not run in the real Windows program from here.

- **Mirror shard options**: field shards (Unchanged, Random or Custom, 1-2 copies of the 8, lying in rooms) and **Add shards in
  chests** (off by default, 1-2 copies of the 8, injected into existing chests that held a consumable, never new chests) work
  separately or both at once. Shard chests are left out of the pause screen's chest lists. Checked: with 1 field copy and 3 chest
  copies (before the cap went to 2), 8 field shards and 24 shard chests; a converted chest opened in the emulator and gave its shard.
- **Only boss shards take Kirby out of the level.** A shard from a chest or a room plays its cutscene and Kirby stays, with no
  victory dance. Checked in the emulator: a field shard and a chest's shard (Kirby stayed in the room and moved after), and Kracko's
  shard (danced, then Central Circle).
- **No Chests** is above every chest option: no chests at all (the protected ones aside), no shard chests, the option greyed out,
  and the pause screen's chest lists hold only what's left. **Shuffle** never puts two chests on one spot.
- **NiDL boss shards send Kirby back to the NiDL hub**, onto that level's door, the way KatAM's go to Central Circle. The Power Orb's
  alone goes on to Nightmare's room (the rest of NiDL's final fight). NiDL's empty walk-out rooms after a boss (4 of them) are left
  out of the port, since nothing leads there any more. Checked: the shard exit table, the post-shard state run in the emulator from
  Shine & Bright's room (Kirby arrived in the NiDL hub at Butter Building's door), and the logic checks (all 280 rooms reached, no
  traps) on all three versions.
- **Map viewer, unrandomized ROMs**: KatAM's map stays exactly the pause screen's (`MapVanilla`), and the NiDL levels go around it,
  never in its gaps. Checked: no NiDL level room inside KatAM's map (only the NiDL hub, beside Central Circle, as before).
- **Bosses Give Random Shards** is a toggle, off by default: KatAM's 8 bosses get the 8 shards shuffled, then the NiDL bosses
  spread the 8 evenly. Tested: Kracko's new shard dropped, was collected, and its rematch left a tomato (HP zeroed in RAM, not a
  real fight).
- **Boss rooms have no doors out.** The shard is the only way out. A boss room is the end of a branch, and any one-way door that was
  in one was moved elsewhere. The logic checks confirm it.
- **Hubs** have at most 2 regular doors; hub doors are counted apart. The world map and the interconnected hub links stay as
  approved.
- **Skip the Tutorial** (Miscellaneous > Game): a game starts as a continued save does, in Central Circle, with no intro, tutorial
  or mirror cutscene, and the tutorial chest's item given once, even with a save made by another ROM.
- **Debug click-to-warp** in the Map Viewer (Debug menu): clicking a room moves Kirby there. Debug only: remove it before release.
- **Whispy Woods' face** is drawn as NiDL draws it.
- **Warp stars** float where NiDL puts them; **cannons** sit on their fuse exactly as KatAM's do (slightly off-centre: the fuse
  enters the cannon's left half).
- **Settings window**: Load and Save Settings on their own rows. Miscellaneous > Game holds Skip the Tutorial, Reduce Flashing,
  Random Intro Parade and the NiDL title screen (greyed out until a NiDL ROM is loaded). Toggles and options use the base
  randomizer's Title Case ("Add Shards in Chests", "Include Mini-Bosses").
- **Map viewer, randomized ROMs**: whole clusters, no 2-4 room islands, levels packed tight.
