# Open requests

What the owner has asked for that is still to do. When an item is finished, take it out. The design rules for doors, mirrors and
hubs are in [Map and Door Design Rules](Map%20and%20Door%20Design%20Rules.md).

## 2. The NiDL levels on the pause screen
The ported rooms' visited doors are saved, but the pause screen has no area maps for the NiDL levels (it treats their rooms as the
tutorial's), so those visits show nowhere in game. Needs area maps for them (graphics and the pause code's tables). Get the
owner's go-ahead on how they should look first.

## 5. Bosses as faithful 1:1 recreations of NiDL
- **Heavy Mole**: rework the fight to match NiDL's. It needs boss rooms that scroll (the screen following the mole through the dirt).
- **The other bosses**: check each against NiDL and fix what differs.

## 34. The Fountain of Dreams finale: what's left
- **Smaller differences from NiDL**: in the ending the Kirbys ride in KatAM's pose (NiDL's Kirby holds the Star Rod up); KatAM's
  HUD stays on (in the Power Orb's room it covers the cloud band's bottom); the room's own music plays, not NiDL's ending music;
  the ending's background is NiDL's mirrored left to right; at Nightmare's defeat his flight off starts after KatAM's burst, not 52
  frames after the hit as in NiDL.

## 35. The Power Orb's fight frame by frame as NiDL's
The owner: no change of look when it's hit, allies can't be called, the foreground shakes oddly, there should be no screen shake;
check the whole fight frame by frame and make it 1:1. Done: its look when hit, calling the helpers, the foreground steady and no
screen shake, its moves, dash and escape, the fall on to the ground and the time limit, its palette pulse, its shots and their
trails, its burst when beaten. Checked against NiDL's recording (USA, Kirby idle with his HP held, NiDL's looks forced to repeat
by a test ROM): its shots and their trails at the same spot and frame on every frame of fight frames 0-4000, the cameras the same
on all 4006 frames, frames 100-866 differing only around Kirby; the finale played through by script in all three versions (the
orb's HP set to 1 by a RAM write) with Nightmare's fight still matching NiDL's on all 4399 frames. Left:
- **The intro**: the orb's path is about 1 pixel left and 2-3 up of NiDL's for 60 frames, and Kirby's swing and its slash come a
  frame early.
- **Lag**: 3 more lag frames over the fight than without the trails.
- **KatAM's own**: the door's fade leaves the background a step bright for the fight's first 89 frames; Kirby's art and pose, and
  the Star Rod tip's glow coming at another time.
- **Not tested**: the fight played by hand.

## 10. Screen shake as KatAM's
The owner: the screen shakes far too much. Not reproduced: KatAM's shake runs the same in the NiDL rooms as in KatAM's, and
nothing found shakes more often than in KatAM. Needs the owner's details (which room, what was happening).
