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
- **Mike's start-up time** (the owner: as NiDL's): NiDL's Mike recorded frame by frame (its script `0x080455C8`, B pressed for each
  use) and matched: from the press, the start 25, 33 and 43 frames (24, 34, 42 before), the songs 32 frames (28, 28, 32), the end
  5 (Kirby back on the 62nd, 70th and 80th frame, as NiDL's), the third use's jump NiDL's heights frame by frame, the flashes on
  NiDL's frames (1st-3rd, 5th-7th, every other one to the 15th, the 19th and 23rd of each song; they went on and off every 4).
  Checked by script (Mike given by RAM writes): the USA, European and Japanese builds the same on all 480 frames (the counter,
  Kirby's height, the 36 flash frames, the ability gone after the third song).
- **Poppy Bros. Jr.'s bombs** (the owner: as NiDL's): recorded in NiDL (USA), the "bomb" is a Maxim Tomato (type 0x28; 0x27's is
  an apple). Inhaled, the rider lets the tomato off: it bounces twice away from Kirby, untouchable (Kirby goes through it, his
  inhale doesn't take it), and where it stops it becomes a Maxim Tomato item; the apple lies there as an enemy (it hurts, is beaten,
  is inhaled). Beaten, the rider goes as any enemy does and Poppy Bros. Jr. jumps off at once, untouchable for 30 frames or till he
  lands (the port waited 30 frames before jumping). The port blew the tomato up and let Kirby inhale it while it bounced; now as
  NiDL's: KatAM's Maxim Tomato where it stops, the bounces NiDL's frame by frame (from where it lies: 2.25 up when it comes off, 7.5
  at the top, down after 15 frames, 0.875 up the frame after, 3.5 at the second top, one more frame along the floor, then the
  tomato; the first bounce was lost before, the engine's start zeroing it). Checked by script: room 0x276 (its rider inhaled: the
  same path and tomato in the USA, European and Japanese builds; Kirby inhaled and ate the tomato), the same rider made an apple in
  a test ROM (it stopped, Kirby inhaled it; Kirby put on it by RAM writes while it bounced: nothing, and as it stopped it hurt him
  and was beaten), room 0x101 (a rider touched: beaten, Poppy Bros. Jr. jumped off at once, untouchable 30 frames).

Left:
- **Mini-boss cameras in rooms bigger than a screen** (the owner: cameras always exactly NiDL's): the fight's screen is NiDL's,
  but the way there isn't yet: NiDL's camera follows Kirby with no easing, slides to the fight's screen 2 pixels a frame once the
  room's trigger object (type 4) comes on, and in the tall rooms 0x09B and 0x27C goes on following him up and down (the port holds
  it); 0x244 has two holds. Being worked on.
- **Mike's colours**: his megaphone and hat colours, and the flash (NiDL's screen goes yellow, the port's brightens to white).
- **Poppy Bros. Jr.**: the tomato or apple comes off a frame after KatAM's inhale takes the rider (NiDL's the same frame), at 0.5
  a frame (NiDL's 0.496 to 0.498, from a small random angle); KatAM's Maxim Tomato, not NiDL's blinking one.
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
  ground). Orange Ocean's hub (0x25A) has the same layout, its ledges KatAM's one-way floors and its middle solid: not changed (the owner:
  leave it).
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
- **Not tested**: played by hand; the randomized map viewer layout (the rows are made the same way).

## 39. Meta Knight's fight 1:1 from NiDL
The owner: recreate Meta Knight's fight (Orange Ocean's boss) 1:1 from NiDL's, keeping the rule that he throws Kirby no sword (Kirby
fights with his own powers). Done: his intro, duel and defeat from NiDL's code (see the ROM Expansion README's Meta Knight row).
Checked by script in the emulator (not played by hand):
- **The duel**: against NiDL's recorded fights (USA), his state, frame, spot and facing on every frame: about 27,000 frames with
  Kirby standing still, in the air, high up and far left (every state but 16 and 23). With Kirby walking it matches but for the
  test's Kirby walking faster than NiDL's.
- **Hits**: a fight replayed from NiDL's recording (Kirby's spot, NiDL's random numbers and its hits' HP fed in): the same 5 hurts
  and 2 guards on the same frames. One hit's first jitter step (and its mosaic) shows a frame late: KatAM sees a hit a frame after
  NiDL does.
- **Beaten**: Sword Kirby slashing at him (his HP set to 10 by a RAM write): guards, hurts, KatAM's flash and burst, then NiDL's
  defeat, all of it from his landing to his flight off (221 frames) as NiDL's; the hop before it is as long as where he is makes
  it. The shard after.
- **All three versions**: given the same random seed, the European and Japanese builds match the USA one on every frame (the duel,
  560 frames; a fight with real hits to the knockout; the defeat). Without it, the Japanese build's seed differs from the start, as
  the game's own random numbers do, so he picks other moves.

- **The camera** (the owner: cameras always exactly NiDL's): NiDL's camera never moves in a screen's room (8, 8); KatAM's room is
  taller (padded below) and its camera sank 16 pixels with Kirby (24). Now the fight's arena holds it at NiDL's (8, 8) from Kirby's
  first frame in the room, through the fight and after it, right on it (not easing there), in every 16x11 room with a fight: Meta
  Knight's (0x29D), Mr. Shine & Mr. Bright's (0x164), the 16x11 mini-boss rooms. Checked by script: 0x29D at (8, 8) on every frame
  of Kirby's entry, the fight, his knockout and defeat (760 frames; NiDL's screenshots the same), and on every logged frame of a run
  in all three builds; 0x164 at (8, 8) through the fight and Kirby's defeat (USA); 0x135 (Mr. Tick-Tock) at (8, 8), its fight ended
  by a RAM write: its door opened and took Kirby to 0x131, whose camera was its own.

Left:
- **The thrust's screen shake**: NiDL's thrust down calls a screen shake as he lands (`0x080261D4`, not looked at in NiDL); not
  ported yet. The owner: keep it, to be done later.
- **Not ported**: NiDL's hit kinds 4 and 6 (states 16 and 23: inhaled, a hit on his box 0), which none of KatAM's attacks are.
- **His knockout**: KatAM's (its flash and burst, 24 frames) before NiDL's defeat, where NiDL holds him 53 frames on a dark screen.
- **Not tested**: played by hand; helpers and other players in the fight.

## 40. Mr. Shine & Mr. Bright's fight 1:1 from NiDL
The owner: recreate Mr. Shine & Mr. Bright's fight (Butter Building's boss) 1:1 from NiDL's, as Meta Knight's was (NiDL's scripts
written out in C, checked frame by frame against recordings), with their graphics, palette cycles, the day, sunset and night
background fade and the beam's darkening. Done: the whole fight from NiDL's code (see the ROM Expansion README's row). Checked by
script in the emulator (not played by hand), NiDL's recorded fight (USA) against KatAM's with NiDL's random seed and Kirby's spot fed
in every frame:
- **The fight**: their states, frames, spots and facing and every shot's and effect's kind, frame and spot the same on all 3570
  frames of a fight with Kirby standing still (his HP held; moved out of the beam once): the intro, both in the sky and on the
  ground, the trades, the falling stars, crescents, fireballs, dashes and trails, the beam with its sparkles, sparks and darkening,
  the sky's fades. A second fight the same on all 1770 frames.
- **Hits**: NiDL's hits (Kirby's spit stars) given to KatAM's object on the matching frames: Mr. Bright beaten first (2370 frames
  the same but for one byte nothing shows); Mr. Shine then Mr. Bright, through the walk to the middle and the shiver. The defeat
  starts a frame apart, KatAM's explosion takes them about a frame before NiDL's burst does.
- **A real hit**: Kirby inhales a falling star and spits it at Mr. Bright (his HP set to 1 by a RAM write): beaten once (it caught a
  bug where one star beat both, now fixed), the room held 27 frames, then he goes up and Mr. Shine comes down to the ground.
- **Looks**: screenshots side by side at matching frames: their sprites, Mr. Shine's glow, the warning flashes (frame by frame, with
  KatAM's flash clock set to NiDL's phase), the sun, the beam, the sky and the darkening as NiDL's.
- **All three versions**: the European and Japanese builds match NiDL's recording on all 3570 frames as the USA one does. Logic
  checks on all three: every room reached, every big switch pressed, nowhere Kirby is trapped, with and without dropping through
  thin floors.

Left:
- **Their bursts**: when one is beaten, KatAM's hit stars stand in for NiDL's effects (`0x95`, `0xA2`); the last burst is KatAM's
  explosion, not NiDL's eight stars, so shots still in the air fall on instead of popping, and KatAM holds Kirby still during it.
- **Sounds**: KatAM's nearest ones.
- **Not modelled**: NiDL's 31 frames without hits after a hurt; Kirby's box for their shots while he floats (fitted standing and
  knocked back).
- **The flashes' phase**: NiDL's run on its frame counter since power-on, so when they flash differs from game to game; the port's
  count from the fight's start.
- **The room's background**: at their fight's camera its two back layers sit about 4 pixels further right than NiDL's (the room
  port's background scrolling, not changed here); the room's foreground colours show a step brighter than NiDL's (the room's data
  is NiDL's, but KatAM's fade in from white, as the tests entered the room, stops a step short of it). Ask the owner before
  changing either.
- **Not tested**: played by hand; Kirby hit by their attacks against NiDL frame by frame (KatAM's own hit reaction; the recordings
  kept him out of the beam); helpers and other players in the fight.

## 41. King Dedede's fight 1:1 from NiDL
The owner: recreate King Dedede's fight (Rainbow Resort's boss) 1:1 from NiDL's, as Meta Knight's and Mr. Shine & Mr. Bright's
were (NiDL's code written out in C, checked frame by frame against recordings), with his graphics, effects, the inhale, the hammer,
the stars and the defeat. Done: the whole fight from NiDL's code (see the ROM Expansion README's row). Checked by script in the
emulator (not played by hand), NiDL's recorded fights (USA) against KatAM's with NiDL's random seed and Kirby's spot fed in every
frame:
- **The fight**: his state, frame, spot, facing and mirroring and every child's kind, frame and spot (his stars, dust, breath, hit
  stars, his hammer's and inhale's boxes) the same on every frame from the intro's end, in four fights: Kirby still by the left wall
  (2746 frames: walks, belly slides, big jumps and their stars, hammer swings, the inhale catching nothing, puffing up, floating and
  bouncing, the breath), Kirby on the right (1246) and Kirby high up while he floats (1446).
- **A hit and a catch**: the fourth fight (746 frames): NiDL's hit (a spit star) given to KatAM's object on the matching frame: his
  hurt (15 frames on frame 8, jittering, his hit stars about him), then the walk, the inhale catching Kirby on NiDL's frame, the
  swallow and the spit, all the same; but for a star NiDL's Kirby inhales (54 frames), which stays in KatAM's run (its Kirby is only
  placed). This found that NiDL's hurt freezes only his own script (the port had held the whole room), that his hit stars follow his
  spot from before his own turn, that he takes no hits for 31 frames after one, that on the ground he's put back on a floor within a
  tile under his feet, and the swallow's shake (-2 then +2).
- **The recorded Kirby**: NiDL's harness Kirby is inert (never hurt, no box of its own), so for these runs KatAM's Kirby was made a
  point and Dedede's touch harmless (RAM writes); the inhale's reach was traced with it (19 pixels from the box's middle caught, 20
  not). In the game the box meets Kirby's own KatAM box.
- **A real hit**: Kirby inhaled the star Dedede's hammer knocked out and spat it at him: 10 of his 60 HP gone, his hurt and hit stars.
- **The defeat**: Kirby with Sword slashing beside him (his HP set to 1 by a RAM write): KatAM's explosion, then his defeat as NiDL's
  (17 frames to frame 10, the slide 2, 1 and 0.5 pixels a frame, 28 in all, the daze's frames on NiDL's frames), on the ground and
  beaten in the air.
- **Looks**: screenshots side by side at matching frames: his sprites, his stars' frames (their blue flash too), his hurt flash
  (frame by frame, with KatAM's flash clock set to NiDL's phase).
- **All three versions**: the European and Japanese builds match NiDL's four recordings as the USA one does (the Japanese reaches the
  fight a frame sooner). Logic checks on all three: every room reached, every big switch pressed, nowhere Kirby is trapped, with and
  without dropping through thin floors.

Left:
- **His defeat's start**: KatAM's explosion stands in for NiDL's grey flashing screen and burst of eight stars (`0x9B`), as for the
  other bosses.
- **Sounds**: KatAM's nearest.
- **The flashes' phase**: NiDL's run on its frame counter since power-on, so when they flash differs from game to game; the port's
  count from the fight's start.
- **Not traced**: NiDL's Kirby's own box (the recordings' Kirby has none; KatAM's is used for the catch and his touch); whether NiDL
  takes a hit on the 31st frame after one (a frame either way); a Kirby blinking after a hit isn't caught (assumed).
- **The camera**: KatAM's screen in his room sits 8 pixels left of and 16 above NiDL's (24, 168) (screenshots matched), so the left
  star of a pair shows at its edge where NiDL's is off the screen. The fight cameras (see 39) don't cover this room yet: ask the owner.
- **Not tested**: played by hand; his hammer and breath hitting Kirby against NiDL frame by frame (KatAM's own hit reaction); helpers
  in the fight; the real hit and the defeat in the European and Japanese builds (only the recordings).

## 42. Paint Roller's fight 1:1 from NiDL
The owner: recreate Paint Roller's fight (Ice Cream Island's boss) 1:1 from NiDL's, as Meta Knight's, Mr. Shine & Mr. Bright's and
King Dedede's were (NiDL's code written out in C, checked frame by frame against recordings), with his graphics, paintings, effects,
hurt and defeat. Done: the whole fight from NiDL's code (see the ROM Expansion README's row). Checked by script in the emulator (not
played by hand), NiDL's recorded fights (USA) against KatAM's with NiDL's random seed and Kirby's spot fed in every frame:
- **The fight**: his state, frame, spot and facing and every child's kind, frame and spot (his paintings blinking, on the easel and
  come to life: the car, the Waddle Dee, the microphone, the baseball, the bomb and its blast, the cloud and its lightning, the
  parasol and its puff; his skate dust and its puffs) the same on every frame for 6000 frames from the room's start (Kirby standing
  by the left wall), but for the touches under Left. This found NiDL's object order (by group, then slot; those created during a
  frame run after the others: his dust takes his spot from before his move, but on the frame it's made) and the parasol's end (its
  frame kept a frame, its hurt frame the next, then the puff).
- **A hit**: NiDL's spit star given to KatAM's object a frame after NiDL's hit (KatAM sees its hits a frame late): his hurt (13
  frames, jittering, flashing, his hit stars about him), then his skating on, the same but for the hit's own frame (NiDL's first
  jitter shows a frame sooner; KatAM's two steps the next frame catch up). Found: NiDL makes his move in the frame he's hit, then
  the hurt's first step.
- **The defeat**: Kirby with Sword slashing as he skates by (his HP set to 1 by a RAM write): his NiDL defeat frame through KatAM's
  explosion, then the shard falls.
- **Inhaling**: Kirby inhaling (B held, Down to swallow; no NiDL data fed): the parasol gave Parasol, the baseball Ball, the car
  Wheel, the microphone Mike, the bomb Crash (his HP held under half by a RAM write, where the bomb and the Kirby painting come
  up), the Kirby painting and the Waddle Dee nothing.
- **Looks**: screenshots side by side at matching frames (his dust, a painting on the easel, the parasol, the cloud and its
  lightning, the bomb's fuse and blast): the same, the room's view too. Found: the paintings drew behind his dust (KatAM's enemy
  depth); now NiDL's depths (the paintings in front of his effects, the blast and the puffs in front of Kirby). The map viewer's
  icon (rendered from the built ROM): him standing.
- **All three versions**: the European and Japanese builds match the 6000-frame recording as the USA one does (the Japanese reaches
  the fight 2 frames sooner in the script). Logic checks on all three: every room reached, every big switch pressed, nowhere Kirby
  is trapped, with and without dropping through thin floors.

Left:
- **A painting touching Kirby**: NiDL's paintings get knocked back (20 frames) and burst in white (`0x98`); here they're KatAM
  enemies and die KatAM's way at once, as the other ported enemies (4 touches in the recording). Ask the owner whether NiDL's
  should be ported.
- **Kirby's box**: KatAM's Kirby's own box reaches 2 pixels lower than NiDL's, so a painting passing just under him can touch him
  where NiDL's doesn't (the parasol at frame 672 of the recording; for the comparison Kirby's box was set to NiDL's there by a RAM
  write). The other ported fights work the same way (NiDL's boxes against KatAM's Kirby).
- **His defeat's start**: KatAM's explosion stands in for NiDL's red flash and burst of stars, as for the other bosses.
- **Sounds**: KatAM's nearest. **The flashes' phase**: as King Dedede's (counted from the fight's start).
- **KatAM's skipped frames**: KatAM skips a whole frame now and then (the frame after `BigFillLevelInfo`; frame 194 and 4582 of the
  recording); KatAM's own, not the fight's (the script feeds NiDL's data past them).
- **Not tested**: played by hand; the cloud inhaled (it flies out of reach of a Kirby on the floor; Spark by the same table); hit
  stars from real hits against NiDL (only the fed hit); helpers in the fight; the defeat and inhaling in the European and Japanese
  builds (only the recordings).

## 10. Screen shake as KatAM's
The owner: the screen shakes far too much. Not reproduced: KatAM's shake runs the same in the NiDL rooms as in KatAM's, and
nothing found shakes more often than in KatAM. Needs the owner's details (which room, what was happening).
