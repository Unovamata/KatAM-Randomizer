# Open requests

Everything the owner has asked for that is still open, in detail, plus the finished items that set a rule to keep. When an item
is done, move it to "Done" and say how it was checked. The design rules for doors, mirrors and hubs are in
[Map and Door Design Rules](Map%20and%20Door%20Design%20Rules.md).

## Still open

### 2. Save door visits, and show them in game from the save
- Saving is done (see Done: "NiDL rooms' door visits in the save"). What's left: the pause screen has no area map for the NiDL
  levels (the pause menu treats their rooms as the tutorial's), so a loaded save's NiDL visits show nowhere in game. KatAM's areas'
  maps follow the save as they always did (`gVisitedDoors` is KatAM's own saved table). Showing the NiDL levels in the pause screen
  needs area maps for them (graphics and the pause code's tables): a new feature; needs the owner's go-ahead on how it should look.

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

### 23. Shard chests open only once opened
- The owner: "Shard chests cannot be reopened once consumed." A mirror shard chest gets a chest bit of its own: it shows open only
  once opened (and stays so); opened when its shard is already owned (from a boss or the field), it gives a Maxim Tomato instead,
  as an owned field shard does. Today it uses its shard's bit, so it shows open, never opened, once the shard is owned elsewhere.

### 30. Warp stars: NiDL's scaling in their flight
- NiDL's warp star flight scales the star (and Kirby) as it flies; KatAM's here doesn't.

### 32. NiDL hubs have only one two-way door
- The owner: "All NiDL's hubs only have 1 2 way door."

## Done (rules to keep)

- **Blocks that stay after their bomb chain** (item 31): NiDL's data gives each breakable block the whole cell it leaves when broken
  (metatile and collision), but the port took only the metatile and always left air. In 0x248, NiDL's chain leaves the frames of
  blocks around the pink squares as blocks (only some of their cells open): the port drew them still there with no collision.
  Broken blocks now leave NiDL's collision too: air (water by water) as before, or NiDL's wall, thin floor, ladder, spikes, water, or
  the block again (solid: no other bomb breaks it). About 180 blocks in all leave something other than air. No visible block is left
  drawn without its collision now; the only such records left are in rooms' bottom rows, which the camera doesn't show (0x23A, 0x2C4).
  Tested in the emulator (USA, expanded, a new game started in 0x248 by an added Sword statue, Kirby attacking the bomb block: a
  real chain): the old build left the frame cells air, the new one leaves them solid (0x0D) and opens the cells NiDL opens; both draw
  the same. Logic checks pass on all three versions and a randomized ROM, with and without dropping through thin floors; the door
  plan is unchanged.

- **Enemies at rooms' entrances** (item 29): an audit of every room a door lands Kirby in found 7 enemies within 3 cells across (2 up or
  down) of a landing. Five were walkers in NiDL rooms (0x277, 0x278 twice, 0x27A, 0x2CB): their NiDL spot passed the check, then
  they were dropped onto the floor below it, right where Kirby lands. The check now runs again where they end up. One was KatAM's own
  Bronto Burt in 0x142, by the landing from 0xF4's door (the only spot that door's side of the room has). KatAM link spots now
  prefer a spot in the chosen room clear of its enemies (same room, so the levels' doors don't change). Where there is none, the
  KatAM enemy at a landing is left out once the randomizer is done. (The 0x34 left in 0x2FF is a Mirra on its door, not an
  enemy.) The audit finds none now in USA, European, Japanese and a randomized USA ROM. Tested in the emulator (USA, expanded, new
  games started on the doors, Up pressed: a real door trip): 0x276's door into 0x277 had a Waddle Dee by the landing in the old build
  and none now; 0xF4's into 0x142 had a Bronto Burt and none now. Logic checks pass on all of them, with and without dropping through
  thin floors; the door plan is unchanged. Not changed: 38 enemies (KatAM's rooms included) stand exactly 4 cells from a landing, one past the rule's 3.

- **Meta-Knights and Mr. Tick-Tock on the ground** (items 26, 27): their floor boxes reached further down than their frames' feet,
  so they stood on the box and floated: the knights 2 to 5 pixels (their box 12 pixels below their middle, the frames' feet at about
  9), Mr. Tick-Tock 5 (24 against 19). Their boxes (and hit boxes) now end at the feet. Measured in the emulator (USA, expanded, new
  games started in the rooms; a sprite's lowest outline row against Kirby's on the same floor): the knights in 0xF5 went from 2-5 pixels
  above the floor to within 1-2 pixels of it, over 12 frames of walking and leaping. Mr. Tick-Tock in 0xF8 went from 5 pixels up to
  level with Kirby. The other three Mr. Tick-Tocks (0x135, 0x1D0, 0x2EF) stand level with Kirby too, and the Japanese build's in 0xF8
  stands at the same spot. Logic checks pass on all three versions. Not changed: Bugzzy uses the same 24-pixel box as Mr. Tick-Tock
  and may float the same way (not reported, not measured).

- **Mini-boss doors in long rooms** (item 24): in 0x244 the door by Fire Lion's fight stood 2 cells past its screen, in the same
  walled space, so nothing hid it. Doors in a fight's area were also only shut once the fight's arena object came on screen, so in
  a long room they showed as Kirby walked up. Now a mini-boss's doors (its fight screen and the space it's fought in: the cells
  Kirby walks to from it, up to 4 across and 2 up or down, not past a wall) are built shut, and the arena object opens them once
  no mini-boss is left; the map, the logic and the doors' records are unchanged. 58 doors in 33 rooms are built shut (they include the
  ones the arena already shut, as 0xF5's). Tested in the emulator (USA, European and Japanese, expanded; a new game started in 0x244,
  Kirby moved next to the door in RAM, Fire Lion's HP set to 0 in RAM: a forced state, not a real fight): before the fight the old
  build shows the door and the new one doesn't; once Fire Lion's HP is 0 the door appears and Up takes Kirby to 0x164. 0xF5 rechecked
  the same way (its exit works). Logic checks pass on all three, with and without dropping through thin floors, and on a randomized
  ROM. Not tried: a real fight, every one of the 33 rooms.

- **The ability room's door in the NiDL hub** (item 33): the NiDL hub now has Central Circle's door to the ability room (0x3CA) at
  the same spot (top left, cell 3, 4). It's Central Circle's own: its hub mirror object (world slot 0x12, KatAM's switch, so it
  opens in both hubs together), the tiles that draw it (its mirror's recolored into the hub's door colors, since the hub's level
  doors took that palette), and its door, landing where Central Circle's does; the way out of the ability room is KatAM's, to
  Central Circle. Found and fixed on the way: a door missing from a room's doors list (the list the pause map's visits use) froze
  the game the moment it opened, so the hub's list carries it. Tested in the emulator (USA, European and Japanese, expanded, a new
  game started in the NiDL hub, slot 0x12 set in RAM: a forced state, not the real switch): the door appears as in Central Circle,
  the game keeps running, and Up on it takes Kirby to 0x3CA; without the slot it's hidden; the level doors still draw right.
  Logic checks pass on all three, the map viewer routes it (no unrouted lines). Not tried: pressing KatAM's real switch for it.

- **Room 0xF5's two-way door** (item 25): 0xF5 is the Meta-Knights' room. Its way back to 0xF1 landed Kirby right beside 0xF1's door
  into 0xF5, so the pair worked as a two-way door. Now a door out of a mini-boss room (or into one) never lands Kirby within 4 cells
  across and 2 up or down of a door, as for two-way doors. Where every floor near the landing is walled in (0xF1's door into 0xF5 sits
  in a pocket behind a bomb block's chain), Kirby lands past the blocks: in 0xF1 he now lands beside the bomb block, so going back
  into 0xF5 means breaking it first. The same rule moved 0xF1's door into 0xF5 off 0xF5's way out. Tested in the emulator (USA,
  expanded, a new game started in 0xF5, the Meta-Knights' HP set to 0 in RAM to open the doors: a forced state, not a real fight):
  Kirby took 0xF5's exit and landed in 0xF1 at (38, 6), by the bomb block, the door into 0xF5 behind the chain. Logic checks pass on
  all three versions, with and without dropping through thin floors.

- **Room 0xED's entrance from 0xEC is one-way** (item 28): 0xEC's door throws Kirby into 0xED, and 0xED has no door back to 0xEC
  (the owner's one-way entrances, `OneWayEntrances` in KatAMExpansionRooms.cs). Tested in the emulator (USA, expanded, a new game
  started in 0xEC): Up on its door put Kirby in 0xED, with no door where he landed. Logic checks pass on all three versions.

- **Grey blocks** (item 22): the owner confirmed they're fine now (KatAM's metal blocks; which abilities break them is in the ROM
  Expansion README).

- **Chests: what was wrong** (the owner's report: empty chests never opened, chests outside the camera, the viewer showing a Battery
  in a chest that gave a Soda). Checked in the ROMs (USA unrandomized, randomized, and with every option on, chests shuffled): 114
  chests, 114 different treasure indices, so no chest is a copy sharing another's bit; the NiDL chests (indices 84-127) are new
  chests holding the game's own chest items. Found and fixed: (1) the Map Viewer read chest contents in the items' object order
  (Cherry, Pep Brew, Meat, Tomato, Battery, 1UP) instead of the game's (1UP, Invincible Candy, Maxim Tomato, Meat, Pep Brew,
  Cherry), so a Pep Brew chest showed as a Battery, and other chests showed the wrong item: no chest ever holds a Battery; (2) two
  chests stood in a room's last column (0x06F, 0x0E9), half outside the camera: chests and batteries now never go in a room's first
  or last column. Tested in the emulator (USA, a new game started in 0x06F, not a playthrough): its chest shows whole at the room's
  end, and opening it gives a 1UP and sets only its own bit. Why a chest may still look empty unopened: a save from an older build
  (the chests' indices were dealt differently then), and by design a mirror shard chest, which shows open once that shard is owned
  from anywhere (a boss, a field shard): see item 23.

- **NiDL rooms' door visits in the save** (item 2): the ported rooms' visited doors (and rooms seen) were kept only in RAM, so a
  reset lost them; they're now saved with each file, in SRAM after KatAM's own save (a copy for each of KatAM's 6, with a checksum),
  written and read whenever KatAM writes and reads its world props, and cleared with them. KatAM's save layout is unchanged, so saves
  load across ROMs with and without the expansion. NiDL chests (treasure indices 84-127) and every mirror shard were already in
  KatAM's saved treasures. Tested in the emulator (USA, expanded, with a save file kept between runs): a new game started in 0x292,
  Kirby went through its door into 0x298 and opened its chest (the game saves on a chest): both copies of file 1 held the bits (0x292's
  door, 0x298 seen and its door), files 2 and 3 none; a second run from that save file (a fresh boot, file 1 continued) had the same
  bits and the chest open; an empty save had none; the same save with the NiDL area wiped (as a ROM without the expansion leaves it)
  loaded with the chest open and no NiDL visits. European and Japanese: a new game writes all 6 copies. Skip the Tutorial still starts
  in Central Circle (its clear runs this one). Not tried: a long playthrough, a vanilla ROM loading a save made here (its area is
  past what KatAM reads).

- **NiDL enemies in the intro parade** (item 1): with the ROM Expansion and Random Intro Parade, 7 ported enemies join the parade's
  kinds: Blade Knight, Needlous, Starman and Gip as small walkers, Twizzy, Cool Spook and Bubbles as fliers, each animated with its
  NiDL walk or flight (a looping variant added to its anim), its own palette slot. The parade's type tables moved to the free space
  with the new types; the randomizer's 16-palette and VRAM check counts them as any other type, so the limit holds. Tested in the
  emulator (the intro from boot, screenshots every 5 frames): USA (seed 1234) showed Blade Knight and Gip walking and Twizzy flying,
  European and Japanese (seed 77) Bubbles, Cool Spook, Starman and others; each in its own colours, facing the way it goes, on the
  parade's line, nothing garbled. Logic checks pass on all three. Not tried: every seed (the check is the same as for KatAM's types).

- **NiDL Meta Knight fight with Kirby's own powers** (item 4): Meta Knight no longer sets down a Sword statue (it stood in for NiDL's
  "GET IT!"), and the duel no longer waits for Kirby to have a power: he waits 1.5 seconds in his cape once on the floor (or until
  he's hit), then leaps down and fights. Nothing in the fight gives or takes an ability. Tested in the emulator (all three versions,
  expanded, a new game started in room 0x29D with the helpers, not a playthrough): he drops to the floor in his cape, waits, casts
  it off and duels; no statue appears and Kirby stays Normal. Not tried: entering with an ability through the level's door (the
  fight's code never touches Kirby's ability).

- **Unlock the Master Sword** (item 3): a Miscellaneous toggle. The Master Sword's stand in Central Circle (`OBJ_MASTER_SWORD_STAND`)
  is KatAM's own, which works and shows only once the game is beaten (`gAIKirbyState` 9999); the option drops both checks (as it
  runs, `sub_080AA618`, and as it's drawn, `sub_080AAA64`), so it's there from a new game. Everything else is KatAM's: one Kirby
  holds it at a time, and it's back on its stand when he loses it. Tested in the emulator in all three versions (randomized and
  expanded, with Skip the Tutorial): a new game, walking from the spawn cloud to the stand, Kirby became Master (ability 0x1A, the
  sword in his hand and "MASTER" on the HUD); without the option the stand isn't there and Kirby stays Normal. Applying it twice
  changes nothing. Not tried: losing the sword and taking it back, multiplayer.

- **NiDL bosses on the map viewer** (item 21): the nine ported bosses (types `0x43`/`0x44`) have map table entries of their own
  ("BS" at `+0xCC`: the map table had no room left), read by the viewer as the mini-bosses' and shown with "Show on Map". Whispy
  Woods' trunk is its room's tiles, so its icon is the room cropped around it with the face drawn where the game draws it (the
  entry's spare bytes hold the face's origin); icons made of several sprites at one spot are stacked (Kracko's cloud and eye, Mr.
  Bright's face on his body, Paint Roller's two palettes), and Mr. Shine and Mr. Bright are both shown. Checked with the viewer's
  code outside the app (USA expanded ROM): every boss room's boss is found (rooms 0x003, 0x004, 0x09F, 0x104, 0x164, 0x1D1, 0x24C,
  0x29D, 0x313), with its icon; Whispy's icon compared with an emulator screenshot of the fight. Not tried in the app.

- **Room 0x147's camera one tile higher** (item 20; the owner: "camera can move 1 tile up"): the camera's top was stored as a wrong
  code (pixels / 8 + 1 for rooms.c's 0, 1, 2), so the 41 rooms meant to start a row down (NiDL's repeated top row) started two rows
  down (0x147 at 32 pixels: flying to its ceiling Kirby went half off the screen), and split rooms' lower parts 16 pixels down
  instead of KatAM's 8. Tested in the emulator (USA, a new game started in 0x147, not a playthrough): the camera's top is now 16
  (`viewportPosition.y` 0x1000, was 0x2000), the snowy wall top shows and Kirby stays in view at the ceiling. The other 40 rooms
  weren't looked at one by one. Logic and chest checks pass on all three versions, with and without dropping through thin floors.

- **Randomized maps: every cluster alike** (the owner: the outer maps were far more spread out than the clusters in the middle):
  on a randomized ROM each NiDL level is laid out as KatAM's clusters are (`MapLayout.LayoutCluster()`: its 48-pixel grid, pulled
  compact) and packed with their margin (`MapExpansion.ClusterLevel()`); beside KatAM's original map the levels keep the area map
  layout. Checked with the viewer's layout code outside the app (mirror-randomized expanded USA ROM): the levels are compact
  clusters like the middle ones, the map went from 3080x3416 to 2144x2480, every line routed, none touching; the unrandomized map's
  layout is unchanged. Not tried in the app.

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
