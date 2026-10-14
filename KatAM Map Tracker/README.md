# KatAM Map Tracker

An optional companion window for the KatAM Randomizer. It draws the map of your randomized ROM as you explore it, in the style of the game's pause screen map, using the game's own graphics.

- The map is generated from the ROM's (randomized) mirrors, so rooms connected by a mirror are drawn next to each other, wherever they were in the original game.
- Rooms use the in-game map squares: **yellow** once every mirror in the room was used, **orange** while some weren't, and the room Kirby is in **pulses** with the game's own animation. Hubs are big squares, Boss and Goal game rooms big star squares.
- Lines use the in-game map lines: Two-Way mirrors are dotted lines, One-Way mirrors are chains of arrows pointing the way they go.
- Discovered rooms are tinted by area, like the colored regions of the in-game map.
- Hovering a room shows a preview of it with its in-game graphics and background, and where its mirrors are (**green** once used, **yellow** if not).
- Rooms appear once you enter them, and lines once you go through a mirror. Arriving through a Two-Way mirror also reveals the way back.
- Progress is saved after every discovery to `<your rom>.gba.tracker.json`, next to the ROM. Progress saved for another seed is ignored.

## Requirements

- Windows with the .NET 8 Desktop Runtime.
- [mGBA](https://mgba.io/) 0.10 or newer (it includes Lua scripting).
- The Japanese ROM (Hoshi no Kirby: Kagami no Daimeikyuu, `AGB-B8KJ`), randomized or not.

## How to use

1. Open **KatAM Map Tracker** and pick the ROM you're playing (`File > Open ROM...`). It's remembered for next time.
   You can also pass the ROM path as an argument: `"KatAM Map Tracker.exe" "C:\Games\KatAM.gba"`.
2. In mGBA, open the same ROM, then `Tools > Scripting...`, and in that window `File > Load script...` and pick `Lua/katam_tracker.lua`.
   The scripting window shows `[KatAM Tracker] Connected to the tracker on port 8765.` once linked.
3. Play. The tracker window can be moved to another screen.

The order doesn't matter: the script keeps retrying, so the tracker can be opened or restarted at any time.

## Controls

| Action | Control |
| --- | --- |
| Zoom | Mouse wheel, `+` / `-`, or the toolbar buttons |
| Move the map | Drag with the left mouse button (stops following Kirby) |
| Preview a room | Hover it |
| Center a room | Double click it |
| Follow Kirby on/off | `Space` or the **Follow Kirby** button |
| Fit the discovered map | `F` or **Fit Map** |
| Forget the progress | `File > Reset Progress...` |

## How it works

- `Lua/katam_tracker.lua` runs inside mGBA. Every frame it reads Kirby's current room from RAM (`0x02020F40`), and when it changes it sends `ROOM <hex id>` to `127.0.0.1:8765`.
- The tracker reads the rooms, doors and graphics straight from the ROM, lays the map out from Central Circle, and reveals it as room reports arrive:
  - Rooms, doors, tilesets, tilemaps and palettes: addresses from ReflexViewer (`Documentation/Reflex JP Tools`).
  - Room backgrounds: room parameter `+0x1E` selects entry `0x614 + index` of the graphics table at `0xD2E5F0` (LZ77 tiles, palette, raw tilemap).
  - Map graphics: the pause screen map tiles at `0x33F688`, palette at `0x33F588`, square pulse frames at `0x3458AE` and line colors at `0x34591E`.
- When Kirby moves from room A to room B, the mirror in A leading to B is marked as found. Rooms never hold two mirrors to the same room, so the mirror is always known. Arriving through a Two-Way mirror also reveals the mirror back.
- Room changes that aren't mirrors (deaths, warp stars, cannons, menus) only reveal the room.
- To use another port, change `PORT` in the Lua script and `TrackerServer.DefaultPort`.
