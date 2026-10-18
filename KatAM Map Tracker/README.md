# KatAM Map Tracker

An optional companion window for the KatAM Randomizer. It draws the map of your randomized ROM as you explore it, in the style of the game's pause screen map, using the game's own graphics.

- The map is generated from the ROM's (randomized) mirrors, so rooms connected by a mirror are drawn next to each other, wherever they were in the original game. Rooms are grouped in clusters of densely connected rooms, and connections are drawn with straight, 45° and 90° lines like a metro map. Lines are routed around the rooms, so they never touch or hide behind a room square (rooms and clusters are spread apart if a line can't get through). The tutorial rooms sit under the map, below Central Circle and above the rooms lined up at the bottom: the game's end. The arena's rooms (no part of exploring) and the empty debug room 0x3C9 are left out. The map comes full circle. Lines that go around a crowded map stay close to it. Outer clusters are pulled into the free space next to the clusters they connect to, so the map stays compact.
- Rooms use the in-game map squares: **yellow** once every mirror in the room was used, **orange** while some weren't, and the room Kirby is in **pulses** with the game's own animation. Hubs are big squares, Boss and Goal game rooms big star squares.
- Lines use the in-game map lines: Two-Way mirrors are dotted lines, One-Way mirrors, Warp Stars and Fuse Cannons are chains of arrows pointing the way they go. Lines come in a few colors (the game's purple, re-hued) so connections next to each other don't blend.
- Each cluster gets a colored region behind it, with a crisp outline, like the colored areas of the in-game map. Everything is drawn as pixel art (nearest neighbor), never blurred.
- Following Kirby pans smoothly to the room he enters, and every room change brings the camera back to him. Zooming eases too.
- `View > Reveal Entire Map (Spoilers)` shows every room and mirror of the seed, with what you haven't found yet faded, for when you want to go straight to the bosses. It's remembered with the seed's progress.
- `File > Export Map Image` saves the discovered map, or the entire map, as a PNG, drawn like the map zoomed all the way out (bigger squares, lines and icons, with Dynamic Scaling on), lines included.
- **Original layout**: when the ROM's mirrors are the original ones (an unmodified game, or a randomization that kept the mirrors), rooms are placed where the pause screen's area maps draw them, each area arranged around Rainbow Route like the world map, and the title bar says "(original layout)". Rooms the area maps don't show (tutorial, Dark Mind's rooms...) go next to the rooms they connect to, or in rows under the map (the tutorial, then the game's end).
- **Three kinds of map**: an unmodified game's is the original layout above; a randomized game's is laid out from its mirrors; a ROM Expansion game's is its KatAM's (the original layout when KatAM's mirrors are the original ones, the mirrors to the Nightmare in Dream Land rooms aside, else laid out from them) with the Nightmare in Dream Land levels packed tight around it: each level in the free spot (its rooms clear of every other cluster's) that keeps the whole map the most square, then the closest to its middle, so the levels fill the gaps between KatAM's clusters and none is left out on its own (the NiDL hub beside Central Circle), and KatAM's rows of rooms no mirror links to its map (the tutorial, the game's end) after the levels, so the map still ends with them; each level like an area map (`Core/MapExpansion.cs`): from its way in outwards, each room where the door to it is in the room before it (a door on a room's right edge puts the next room to its right; one in its ceiling, above), as far apart as KatAM's linked rooms sit on its own maps (8 tiles, more past a long room), kept clear of rooms it isn't linked to, so a level grows the branches, loops and corridors its doors make; a room linked to none of its level's (a door swapped with another level's, a switch room) sits by its stage's rooms, else by the level's way in, so no stage's room strays from its level. Lines between two other places go around a level, not through it. KatAM's original layout is recognized by its mirrors' destinations (the ROM Expansion moves a few hub doors within their rooms, for symmetry). The randomizer doesn't shuffle NiDL's own doors, so they look the same randomized or not.
- **Drawing**: only what's in the window (and a margin around it) is drawn: rooms, line segments and object icons outside it are skipped (culling), and each line's segments are worked out once per map, so big maps (the ROM Expansion's 560 rooms) pan and zoom smoothly; exported images draw everything.
- **Show on map** (`View > Show on Map`, off by default but Fuse Cannons and Warp Stars): the in-game sprites of each room's objects, each kind on its own: bosses, mini-bosses, Big Switches, Mirror Shards, chests, items (food, 1UPs...), Fuse Cannons and Warp Stars. They're drawn inside the room as pixel art, all the same size (bosses, mini-bosses and Big Switches bigger), up to four in a 2x2 grid. Hovering the room lists what's in it. Since it reads the ROM, it shows where the randomizer put things, the ROM Expansion's Nightmare in Dream Land mini-bosses included. Fuse Cannons and Warp Stars stay faded until the room they send Kirby to is visited.
- **Chest contents** (`View > Show Chest Contents (Spoilers)`, off by default): chests show what they hold instead of the chest: the item, or the Collection Room sprite of the spray, music sheet, map, Vitality or Sound Player.
- **Area maps** (`View > Reveal Areas of Found Maps`, off by default): the rooms of every area whose map the save loaded in the emulator holds are shown, faded until visited, like the pause screen does. It follows the loaded save, so switching saves never reveals areas found in another one, and it's never saved with the progress.
- **Minimalist mode** (`M`, or `View > Minimalist Mode`) hides the menus, toolbar and status bar, leaving only the map next to your game. `M` or `Esc` brings them back, and right clicking the map opens every option in any mode. It's remembered for next time.
- **Dynamic scaling** (`View > Dynamic Scaling`, on by default): room squares are drawn bigger than their map size. Zoomed in, squares are half again as big while lines keep their in-game thickness, so the paths stay easy to follow. Zoomed out, squares and lines grow until rooms are at least 20 pixels on screen, so the whole map can be previewed. Big rooms stay bigger than small ones and neighbors never touch.
- **Debug: click a room to warp** (`Debug > Click a Room to Warp Kirby There`, off by default; a debugging aid, to be removed for the release): with it on and the emulator script (version 4) connected, clicking a room moves Kirby into it, onto where a mirror into that room lands him (else the room's own default spawn), the way the game's own warps do: the tracker sends the script `WARP <room> <x> <y>` over the same connection, and the script writes Kirby's destination and starts his room transition. It waits up to 10 seconds while Kirby is busy (a cutscene, an attack, no HP). Rooms not yet shown can be clicked with `View > Reveal Entire Map` on.
- Hovering a room shows a preview of it with its in-game graphics and background, and where its mirrors are (**green** once used, **yellow** if not).
- Rooms appear once you enter them, and lines once you go through a mirror. Arriving through a Two-Way mirror also reveals the way back.
- Progress is saved after every discovery to `<your rom>.gba.tracker.json`, next to the ROM. Progress saved for another seed is ignored.

## Building the executable

Run `Build Map Tracker.bat` (it needs the [.NET 8 SDK](https://dotnet.microsoft.com/download/dotnet/8.0)). It creates `publish\KatAM Map Tracker.exe`, a single self-contained executable, with `publish\Lua\katam_tracker.lua` next to it. Players don't need .NET installed to run it.

## Requirements

- Windows.
- [mGBA](https://mgba.io/) 0.10 or newer (it includes Lua scripting).
- A Kirby & the Amazing Mirror ROM, randomized or not: Japanese (`AGB-B8KJ`), American (`AGB-B8KE`) or European (`AGB-B8KP`).

## How to use

1. Open **KatAM Map Tracker** and pick the ROM you're playing (`File > Open ROM...`, or drag the ROM file onto the window). It's remembered for next time, and `File > Open Recent` lists the last ROMs you opened.
   You can also pass the ROM path as an argument: `"KatAM Map Tracker.exe" "C:\Games\KatAM.gba"`.
2. In mGBA, open the same ROM, then `Tools > Scripting...`, and in that window `File > Load script...` and pick `Lua/katam_tracker.lua`.
   The scripting window shows `[KatAM Tracker] Connected to the tracker on port 8765.` once linked.
3. Play. The tracker window can be moved to another screen.

The order doesn't matter: the script keeps retrying, so the tracker can be opened or restarted at any time.

## Controls

| Action | Control |
| --- | --- |
| Zoom | Mouse wheel, `+` / `-`, or the toolbar buttons |
| Move the map | Drag with the left mouse button (stops following Kirby until he changes rooms) |
| Preview a room | Hover it |
| Center a room | Double click it |
| Follow Kirby on/off | `Space` or the **Follow Kirby** button |
| Fit the discovered map | `F` or **Fit Map** |
| Reveal the whole map (spoilers) | `View > Reveal Entire Map (Spoilers)` |
| Save the map as an image | `File > Export Map Image` |
| Open a recent ROM | `File > Open Recent` |
| Open a ROM by dragging it | Drop the `.gba` file onto the window |
| Minimalist mode (only the map) | `M` (`Esc` leaves it) |
| Show bosses, items, chests... | `View > Show on Map` |
| All options, in any mode | Right click the map |
| Forget the progress | `File > Reset Progress...` |

## How it works

- `Lua/katam_tracker.lua` runs inside mGBA. Every frame it reads Kirby's current room from RAM (`0x02020F40`), and when it changes it sends `ROOM <hex id>` to `127.0.0.1:8765`.
- The tracker reads the rooms, doors and graphics straight from the ROM, lays the map out from Central Circle, and reveals it as room reports arrive:
  - Rooms, doors, tilesets, tilemaps and palettes: addresses from ReflexViewer (`Documentation/Reflex JP Tools`).
  - Room backgrounds: room parameter `+0x1E` selects entry `0x614 + index` of the graphics table at `0xD2E5F0` (LZ77 tiles, palette, raw tilemap).
  - Map graphics: the pause screen map tiles at `0x33F688`, palette at `0x33F588`, square pulse frames at `0x3458AE` and line colors at `0x34591E`.
  - Room objects: pointer table at `0xD2EBC4` + short room index × 4, to a 16 byte header and 36 byte objects (`+12` object ID, `+14` behavior, which is what a chest holds).
  - Item and boss sprites: located by planting each object next to Kirby in an emulator and reading the hardware sprites back. Each sprite is a few pieces, each a run of consecutive tiles in the ROM with a palette; `Core/MapEntities.cs` lists them. King Golem's tower is part of his arena's tiles, so his icon stitches the arena's tiles and his face sprite.
  - Treasure sprites come from the Collection Room (16x16 sprites, tiles from `0x7BAEB8`, palettes from `0x4A2818`).
  - ROM Expansion: a ROM the randomizer expanded (its ROM Expansion tab) is 32 MB and ends with a table the randomizer writes (the last `0x100` bytes: `NIDLMAP`, then each ported mini-boss's object type, `0x40`-`0x42`, the behavior nibble that picks it, its name and a ready-made icon), which `Core/RomMap.cs` reads (`IsExpanded`, `ExpansionMinibosses`); the title bar says "(ROM Expansion)". The game code stays the vanilla one, so the region is read as usual.
  - Nightmare in Dream Land rooms: with "Include Rooms", the table also holds a rooms block at `+0xE0` (`NR`, the NiDL hub's room, then pointers to the room tables the game uses: the randomizer copied them with the new rooms' entries). `RomMap` then reads every room's own table entries (long room parameters `+0x18` tilemap, `+0x1A` collision and doors, `+0x20` objects; each NiDL room has its own doors list, `0x11F` on), so the map shows the NiDL rooms: the NiDL hub (a hub square, `RoomInfo.IsNightmareHub`) behind Central Circle's golden twin mirror, each level's switch room and stages (`RoomInfo.NightmareLevel`, from their area, 11-18; `NightmareStage` and `NightmareIndex`, the NiDL stage + 1 and room number in their area entry's padding, `+0x4A`, `+0x4B`, which the game doesn't read; older expanded ROMs had them at `+0x48`, `+0x49`), laid out by `Core/MapExpansion.cs`.
  - Area maps: the Lua script reads `gTreasures.bigChestField` (`0x0203897C`, one bit per area, the same in every version) and sends `MAPS <hex>`. The rooms of each area come from the game's pause map table (`sAreaMapRoomInfos` in the KatAM decompilation); see `Core/AreaMaps.cs`.
  - These are the Japanese addresses. The American and European ROMs move each block of data as a whole, so the addresses are shifted per block (see `RomVersion` in `Core/RomMap.cs`): level data `+0x2FE74` / `+0x3A2D8`, pointer tables `+0x34BE4` / `+0x3F1C0`, map screen graphics `+0x1B844` / `+0x20324`. The RAM room address is the same in every version.
- Connections are routed with A* on the 8 pixel grid in the 8 directions: rooms other than the two being connected block the cells around them, turns cost a little (sharp turns aren't allowed) and cells used by other lines cost a bit more, so lines bend around rooms with 45° turns and spread out.
- When Kirby moves from room A to room B, the mirror in A leading to B is marked as found. Rooms never hold two mirrors to the same room, so the mirror is always known. Arriving through a Two-Way mirror also reveals the mirror back.
- Warp Stars and Fuse Cannons are One-Way connections too, read from their objects (the room they send Kirby to); using one reveals its line. Other room changes (deaths, menus) only reveal the room.
- To use another port, change `PORT` in the Lua script and `TrackerServer.DefaultPort`.
