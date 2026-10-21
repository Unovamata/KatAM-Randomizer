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
trails, its burst when beaten, the intro (the orb's path, spin, pulses and slash, Kirby's swings), the room's colors after the
door's fade, the cloud bands' drift, the Star Rod's sparkles' colors. Checked against NiDL's recording (USA, Kirby idle with his HP
held, NiDL's looks forced to repeat by a test ROM): the orb at the same spot and frame on every frame it shows (fight frames -254
to 200), its shots and their trails on every frame of 0-4000, the cameras on all frames from -319 to 4000, screenshots of the
intro matching but for Kirby's colors (KatAM's player palette) and a few background pixels a step of red apart (the room's
conversion); the finale played through by script in all three versions (the orb's HP set to 1 by a RAM write, Kirby steered onto
the warp star), alone and with helpers, Nightmare's fight still matching NiDL's on all 4399 frames. Left:
- **The rod's shine**: NiDL's runs on its frame counter since power-on, so when it shines differs from game to game; the port's
  runs on KatAM's.
- **Not tested**: the fight played by hand; the ending's cloud drift against NiDL's.

## 36. The finale: the owner's next list
The owner's list: the Power Orb fight's lag, the orb's flight into the moon, the cinematic from the Power Orb to Nightmare, the
Master Sword after Nightmare, the ability card stuck on the screen after him, the warp star out of place for a frame, a faster ride
down from the moon, the pause screen's glitch after Nightmare, the Fountain of Dreams hub's camera. Done, each checked by script in
the emulator (USA, the ending in all three versions; not played by hand):
- **Lag**: the helpers' Star Rod stars fly without their sparkle and with no KatAM hit box, and the attack poses are drawn in big
  OAM pieces: over frames 330-2240 of the scripted fight with three helpers, 379 lag frames down to 63.
- **The orb into the moon**: NiDL's own picture of it, its pieces placed as NiDL's: the same spot, pieces and size as NiDL's
  recording on all 216 frames of its shrink, its colors pulsing on through the freeze as NiDL's, its dot where NiDL's is.
- **Power Orb to Nightmare**: the flashing's lag frames gone (only the room load's, under the white, left), its flicker NiDL's (2
  frames on, 2 off) over all 155 frames, his room's background where NiDL's is through his intro and fight (it was 16 pixels off
  each way).
- **After Nightmare**: Normal, the HUD's card NORMAL with no picture (from a game whose Star Rod had Master saved, a RAM write).
- **The warp star**: found on 2 frames of the ending (stars drawn one frame at a clamped spot); fixed, checked frame by frame
  alone and with helpers, and the ride into the moon too.
- **The ride down**: twice as fast (about 55 seconds to 28).
- **The pause**: reproduced (START in the fade into the hub); START is ignored on the finale's flights now; the pause still opens
  in the hub.
- **The hub's camera**: held at (8, 8) (it rose 3-7 pixels as Kirby flew up).
- **Found on the way**: the game froze in the Power Orb's fight when a helper lost there came back in another room (the arena kept
  holding its camera, and KatAM's helper AI looked there for a spot forever): the scripted fight with three helpers froze at frame
  2246 (2659 before these changes); it runs on to 4800 now in all three versions.

Left:
- **The ride into the moon**: the star and the Kirbys are KatAM's warp star sprite (NiDL's star is four 16x16 pieces, with Kirby
  in his rod pose); the star's dot is 1-2 pixels off NiDL's.
- **The other NiDL level hubs**: their cameras rise up to 8 pixels as Kirby flies up, as the Fountain of Dreams hub's did. Not
  changed; ask the owner.
- **Nightmare's intro**: Kirby's pose (NiDL's holds the Star Rod up), KatAM's HUD, NiDL's random twinkling stars.

## 37. Mini-bosses and Mike: the owner's list
The owner's list: mini-boss cameras locking in the wrong places (check every NiDL mini-boss's against NiDL's, not mid room);
Poppy Bros. Jr.'s bombs not inhalable (a softlock); the Meta-Knights inhalable only once beaten; every mini-boss's projectiles
inhalable; the Meta-Knights invincible while invisible; Mike's last song stuttering, its white flashes going on after the ability's
lost and through rooms; Mike with no start-up time as NiDL's. Done, checked by script in the emulator (not played by hand):
- **Cameras**: each fight's screen is where NiDL holds its camera, measured in NiDL in every mini-boss room (USA) and matched in
  the port: 0x07C (320, 8), 0x09B (16, 8), 0x0EE (336, 8), 0x0F8 (216, 8), 0x1C3 (200, 8), 0x23E (24, 8), 0x242 and 0x244 (264,
  24), 0x29B (24, 8), the 16x11 rooms (8, 8), the Meta-Knights' rooms 0x0F5, 0x155, 0x1CA, 0x29C (24, following Kirby 8-24 up and
  down), 0x27C (following Kirby 30-225 across, at 181). They locked on the room's first camera lock record, which is only the
  trigger NiDL's camera reacts to (0x07C's 49 pixels right of NiDL's, Fire Lion's 48 and 56 off).
- **Inhaling**: Poppy Bros. Sr.'s bomb (0x07C), the knights alive (0x0F5: Meta Knight's bar goes down), Mr. Tick-Tock's note
  (0x135), Fire Lion's flames (0x2F4, 0x242), each pulled in, a mouthful and spat out (USA); Poppy Bros. Jr.'s bomb, pulled in
  with its rider and swallowed (room 0x101, all three versions). The thrown ball and bomb now bounce away as they came off (they
  stopped dead, the bomb going off by Kirby). Not inhaled: Mr. Tick-Tock's rings (around it as it rings) and the Mace Knight's mace
  (on its chain).
- **Knights while invisible**: untouchable while they blink in (18 frames). Checked with Kirby's inhale: a knight's fade stretched to
  200 frames by a RAM write, it blinked in front of the inhale for 90 frames and was never pulled; without the write it was
  pulled in at once.
- **Mike's flashes**: two flash tasks were made each song, one with garbage for its length (a C macro reading its argument twice):
  now one, 28, 28 and 32 frames, then off, nothing after the third song (Mike given by a RAM write, all three versions).
- **Found on the way**: the new fight areas moved the planned doors, and the layout left Grape Garden's first stage with no way in
  (the door planner took bomb chains as breakable from either side); fixed, every room reached again in all three versions.

Left:
- **Mike's start-up time**: the port's wind-ups are within a frame of NiDL's (24, 34, 42 frames against NiDL's 25, 33, 43; whole
  songs 56, 67, 78 against 61, 69, 79). Ask the owner what NiDL has that the port lacks.
- **Not tested**: the fights played by hand; a hit with an ability on a knight blinking in (only the inhale was tested: Kirby kept
  getting stuck in a Sword move in the scripted runs); a spat star hurting Poppy Bros. Sr. (a Bronto Burt got in the way).

## 38. Hubs, the finale and the map order: the owner's list
The owner's list: the NiDL hubs' cameras locked as KatAM's; room 0xC9's pass-through platforms not letting Kirby through and an
odd solid box in its middle; the Fountain of Dreams hub's Maxim Tomato to the right and the Master Sword's stand in its middle;
Nightmare's fight lagging as he calls up his particles; a faster cinematic after Nightmare; the Power Orb's room, Nightmare's and
the Fountain of Dreams hub before KatAM's last rooms in the map viewer's order. Done, checked by script in the emulator (USA unless
said; not played by hand):
- **Hub cameras**: the level hubs' cameras stay at (8, 8) as Kirby flies to their tops (0x064, 0x0C9, 0x1F9, 0x002; 0x064's and
  0x1F9's rose to 0 before). The NiDL hub, Central Circle's copy, scrolls with Kirby as Central Circle does (it's bigger than a screen).
- **Room 0xC9**: its floating platform is all a thin floor: Kirby jumps up through it anywhere, drops through its ledges (Down, or
  Down and B), and the big switch stands on its middle (KatAM's one-way floor there: on one he drops through, the switch fell to the
  ground). Orange Ocean's hub (0x25A) has the same layout, its ledges KatAM's one-way floors and its middle solid: not changed.
- **The Fountain of Dreams hub**: the Master Sword's stand on the middle platform (shown, and giving Master, once the game is beaten,
  as Central Circle's; checked with the beaten state forced by a RAM write: Kirby walked into it and got Master), the Maxim Tomato
  on the floor's right.
- **Nightmare's lag**: the helpers' aim walked the whole task list for every height they tried (7% of a frame with three helpers),
  and each Star Rod star walked it every frame for his box: now found once. With three helpers and every Kirby shooting, the
  scripted fight's lag frames went from 350 to 100 (USA, European), 367 to 135 (Japanese); alone, 5 before and after. Nightmare's
  fight still matches NiDL's on all 4399 frames (all three versions). What's left of the lag is KatAM's own sprite drawing and
  collisions with that many stars and sparkles out (his charge, his hand, his knockback).
- **After Nightmare**: the ride down from the moon four times as fast as NiDL's (it was twice): from his room to the hub, 2620
  frames down to 1790 (about 44 seconds to 30), in all three versions.
- **Map viewer**: the Fountain of Dreams' three rooms lead the bottom row (the game's end), the Power Orb's room, Nightmare's, the
  hub, then KatAM's last rooms (checked on the original layout's render).

Left:
- **Orange Ocean's hub (0x25A)**: same layout as 0xC9's, its little side platforms drawn but KatAM's one-way floors (Kirby can't drop
  through them) and its middle a solid block: change it as 0xC9's? Ask the owner.
- **Not tested**: played by hand; the randomized map viewer layout (the rows are made the same way).

## 10. Screen shake as KatAM's
The owner: the screen shakes far too much. Not reproduced: KatAM's shake runs the same in the NiDL rooms as in KatAM's, and
nothing found shakes more often than in KatAM. Needs the owner's details (which room, what was happening).
