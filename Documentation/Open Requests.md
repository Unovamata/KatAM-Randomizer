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

### 6. Room 0x2FD: collision and cannons
- Its collision is broken and its cannons are missing.

### 7. Map viewer: goal rooms drawn by the room that leads to them
- Goal rooms get their own icon beside each room whose goal mirror leads to them, as if each were a room of its own, instead of
  one goal room every goal mirror's line crosses the map to. Fewer long lines over the map.

### 8. Warp star rides: NiDL's animation
- Room 0x2F9's warp star ride has no animation, and room 0x280 needs it too. Port NiDL's warp star animations, for every warp star
  room, with support for the helper Kirbys (and any NPC riding along).

### 9. Ability blocks: KatAM's grey ones, everywhere
- NiDL's ability-only blocks (as in room 0x271) break with any ability. KatAM has its own grey ability blocks: every such block in
  every NiDL room becomes KatAM's.

### 10. Screen shake as KatAM's
- The screen shakes far too much; something broke along the way. Match KatAM's screen shake.

### 11. Room 0x29B: mini-boss room doors
- It has 2 doors very close together. A room Kirby is thrown into for a mini-boss (or boss) fight: its way in is one-way and its
  way out is one-way too.

### 12. Room 0x298: give it a purpose
- It's completely empty. Add something meaningful: a chest or a branching path.

### 13. Map viewer: NiDL room spacing as KatAM's
- The NiDL levels' spacing is nothing like KatAM's room-to-room spacing: rooms bleed into each other. The spacing between rooms
  must be consistent (as KatAM's), as much as it can be. This is a rule (rulebook, section 11): enforce it harder.

### 14. NiDL boss rematch: a Maxim Tomato, not the shard
- A NiDL boss whose shard is already owned still drops the shard. The flow: the shard if Kirby doesn't have it; if he does, a Maxim
  Tomato, and grabbing it takes him out to the hub, as the shard would.

### 16. Bomb chains, for good
- Rooms 0x304 and 0x2FD's bomb block chains are broken. Fix every room with bomb chains, checked against NiDL's own maps.

### 18. Batteries in the NiDL rooms
- Add batteries at valuable, rewarding places in the NiDL rooms.

### 19. Rooms using 0x1B9's background: corrupted

### 20. Room 0x147: falling doesn't kill Kirby
- The owner asked whether that's expected. It isn't: the room has no fall-out exit (its only ways out are its two doors), so a fall
  should cost a life. To look into.

## Done (rules to keep)

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
