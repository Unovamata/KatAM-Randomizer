# KatAM Map Tracker

An optional companion window for the KatAM Randomizer. It draws the map of your randomized ROM as you explore it, using the rooms' in-game graphics.

- The map is generated from the ROM's (randomized) mirrors, so rooms connected by a mirror are drawn next to each other, wherever they were in the original game.
- Rooms appear once you enter them. Their mirrors appear as markers: **yellow** if you haven't gone through them yet, **green** once you have.
- Lines join the mirrors you used: **blue** for Two-Way mirrors, **orange arrows** for One-Way ones.
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
| Center a room | Double click it |
| Follow Kirby on/off | `Space` or the **Follow Kirby** button |
| Fit the discovered map | `F` or **Fit Map** |
| Forget the progress | `File > Reset Progress...` |

## How it works

- `Lua/katam_tracker.lua` runs inside mGBA. Every frame it reads Kirby's current room from RAM (`0x02020F40`), and when it changes it sends `ROOM <hex id>` to `127.0.0.1:8765`.
- The tracker reads the rooms, doors and graphics straight from the ROM (addresses from ReflexViewer, in `Documentation/Reflex JP Tools`), lays the map out from Central Circle, and reveals it as room reports arrive.
- When Kirby moves from room A to room B, the mirror in A leading to B is marked as found. Rooms never hold two mirrors to the same room, so the mirror is always known. Arriving through a Two-Way mirror also reveals the mirror back.
- Room changes that aren't mirrors (deaths, warp stars, cannons, menus) only reveal the room.
- To use another port, change `PORT` in the Lua script and `TrackerServer.DefaultPort`.
