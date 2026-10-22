# Open requests

What the owner has asked for that is still to do. When an item is finished, take it out. The design rules for doors, mirrors and
hubs are in [Map and Door Design Rules](Map%20and%20Door%20Design%20Rules.md).

## 5. Bosses as faithful 1:1 recreations of NiDL
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

## 43. Kracko's fight 1:1 from NiDL, and its stage's lag
The owner: recreate Kracko's fight (Grape Garden's boss) 1:1 from NiDL's; and its stage lags a lot, can that be solved? Done: the
whole fight from NiDL's code, both its parts, the eye chasing Kirby up the tower and the cloud at the top (see the ROM Expansion
README's row). Checked by script in the emulator (not played by hand):
- **The fight**: NiDL's recorded run (USA, Kirby floating up the tower) against KatAM's test build with NiDL's random seed, Kirby's
  spot, whether he's floating or landed and NiDL's camera fed in every frame: its state, spot, speed, frame and work fields and its
  orbs', cloud's and bolts' frames and spots the same on every frame for 3000 frames: the eye's drift, turns and lunges, its rise,
  the camera's lock at the top, the cloud forming, the lightning, the swoop, the bob, a Waddle Doo dropped.
- **A hit**: a new NiDL recording, Kirby inhaling the Doo and spitting the star at the cloud; the same hit given to KatAM's test
  build a frame after NiDL's: its freeze (13 frames, jittering, its eye spinning, its cloud puffing fast), then its lightning going
  on, the same but for the hit's own frame (KatAM sees hits a frame late), the eye's spin phase (NiDL's from its power-on clock) and
  one frame's bolts (NiDL's landing dust took a random number first). Found and fixed: NiDL makes no hit stars for Kracko (the port
  had them); NiDL's hurt step runs after every object's turn; its cloud sees the new mode in the hit's own frame.
- **Real play** (the normal build, nothing fed): Kirby flying up the tower: the eye chases him, rises, the camera locks at the top,
  the cloud forms, then lightning, swoop, bob, Doo, sweep. Kirby inhaling the Doo and spitting the star: a real hit (60 to 50 HP,
  NiDL's 10); then, its HP set to 10 by a RAM write, a real killing blow: KatAM's freeze on NiDL's defeat frame, the explosion, the
  shard, Kirby taking it, the cutscene, his dance, and the NiDL hub's door. Found and fixed: KatAM's HUD showed Kracko's HP bar
  whenever the eye touched Kirby (NiDL shows none till the cloud forms: the eye's touch is now an attack box of its own, still 1 HP
  a touch); the camera let go at the defeat (now held while a Kirby's on the top screen, as NiDL's).
- **Helpers**: the three helpers in the tower froze the game as the camera locked at the top with them below (KatAM's helper AI
  looks forever for a zone it can reach inside the camera, and the tower's zones were in separate groups). Every room's zones are
  now joined (22 of the 279 NiDL rooms were split; KatAM's own rooms unchanged): the helpers come in at the top and fight.
- **All three versions**: the European build plays the same run frame for frame; the Japanese one runs at its own timing (the
  inputs retimed); both through the real hit, the kill, the shard and the hub. Logic checks on all three: every room reached, every
  big switch pressed, nowhere Kirby is trapped, with and without dropping through thin floors.
- **Lag**: not reproduced. The new fight, Kirby flying up and through the cloud's attacks (3400 frames): 1 lag frame (the room
  loading); with the three helpers fighting it: 3. KatAM's own Kracko measured 5 in 2400 before.

Left:
- **The lag the owner sees**: ask when it lags (the climb, the cloud, the lightning), on which emulator or console, with helpers out
  or not, and on which seed.
- **The eye's spin and the flashes' phase**: from the fight's start (NiDL's from power-on, so they differ from game to game there
  too). **Its defeat's start**: KatAM's explosion stands in for NiDL's. **Sounds**: KatAM's nearest.
- **Not tested**: played by hand; a real saved game (the script starts a new game in the tower; there the shard took Kirby to the
  NiDL hub after his dance); attacks other than the spit star; two players.

## 44. Heavy Mole's fight 1:1 from NiDL
The owner: recreate Heavy Mole's fight (Yogurt Yard's boss) 1:1 from NiDL's. Done: the whole fight from NiDL's code, the screen
scrolling with it through the dirt and looping every 512 pixels, NiDL's pocket where Kirby comes in, the dirt dug as NiDL's (see the
ROM Expansion README's row). Checked by script in the emulator (not played by hand):
- **The fight**: NiDL's recorded fight (USA) against KatAM's test build with NiDL's random seed and frame count fed in and Kirby kept
  40 pixels behind it: its state, frame, spot and work fields, every child's kind, frame and spot (its arms, drill, hatch, eye,
  clods, missiles, their bursts) and the camera the same on every frame for 1777 frames; the loop at 768 as NiDL's (the screen
  goes on with no seam); 7000 frames without a crash.
- **A hit**: a NiDL recording with a hit; the same hit given to KatAM's test build on NiDL's frame: its hurt (23 frames: flashing,
  jittering, hit stars, its steps, hatch and eye held) the same on every frame for 1777 frames (its missiles left out: Kirby's spot
  differs).
- **Real play** (the test build with nothing fed: the normal game but for its frame dump): a new game started in room 0x249, Kirby
  going through its mirror: he lands in the pocket at (40, 297) (NiDL's 293.5: its Kirby's spot is 4 higher) and the fight starts;
  he jumps into its first missile, inhales it, jumps and spits it: a real hit (60 to 50 HP). Then, the missile held in his mouth
  and its HP set to 10 (the test build's fed hit: a forced state), the spit star kills it: KatAM's freeze and explosion, the shard,
  Kirby flying to it, the cutscene, back in the room, the NiDL hub. Kirby left behind can stand under the screen's bottom (17
  pixels, as the arena lets him) and falls off once the screen leaves him: he loses a life and the fight starts again.
- **Lag**: its special blocks are a grid (`gNidlGridRoom`, looked up at once: KatAM's walk through its 2112 records took most of a
  frame a block, the big lag); the dirt's crust drawn from rows of bits, the screen's scan four cells at a time, the turns' order
  sorted once, one break sound a dig, debris sprites kept for the next ones (each new one a task, VRAM and tiles). Its recorded
  fight (1954 frames, no helpers): 7 frames dropped (768 before; one is KatAM's room load), each where several cells break at once.
  Screenshots with and without the kept sprites are the same.
- **All three versions**: built; the European and Japanese ones run the fight 7000 frames (digging, missiles, arms, Kirby's deaths
  and the fight starting again) without a crash. Logic checks on all three: every room reached, every big switch pressed, nowhere
  Kirby is trapped, with and without dropping through thin floors (the same as before but for the door into this room, which lands
  in NiDL's pocket now).

Found, not changed (for the owner): after the shard's cutscene, KatAM puts Kirby back in the room for his victory dance at its own
table's spot for that shard (`gUnk_08352DBE`: row 9 here), which in this room is inside the dirt: he falls through it to the
pocket's floor (about 90 frames) before dancing. A fix would have Kirby dance where he stands after any NiDL boss (as
`NidlDanceOut()` does); it changes every NiDL boss's shard, so it waits for the owner's word.

Left:
- **KatAM's own**: KatAM sees a hit a frame after NiDL; the inhale's reach and pull are KatAM's, and Kirby's knock-back when hurt.
- **Sounds**: KatAM's with NiDL's notes (its hatch's ends differently).
- **Not tested**: played by hand; a real saved game; attacks other than the spit star (an ability digging the dirt); helpers; two
  players; the loop at 768 on the European and Japanese builds (Kirby has to be kept alive that far: only the USA test build does
  that); the lag with helpers.

## 45. The Star Rod in the NiDL hub, as the Master Sword in Central Circle
The owner: the Star Rod should be there for good in the NiDL hub, as the Master Sword is in Central Circle. Done: the rod stands in
the golden pedestal under the hub's Dimension Mirror (where Central Circle has the Master Sword's stand), from the start, with no
option; it works as the Master Sword's stand: a player's Kirby with no ability who touches it takes it (KatAM's get-ability pose,
the HUD card with its name and picture), the stand is empty while a player has it or it's out as an ability star (Select), and it
comes back with KatAM's puff once neither is. Its picture is NiDL's rod held straight up (frame `0x1023`), glowing and cycling its
colors as on Kirby. Taken there, it's Kirby's everywhere (see the ROM Expansion README's Star Rod row). Checked in the emulator
(USA, by script, not played by hand):
- **The stand** (a new game started in the hub): the rod in the pedestal; Kirby walks into it and gets it (ability 34, the card);
  B swings it and shoots a star; Select drops it as a star, the stand stays empty until that star is gone (about 12 seconds), then
  the rod is back.
- **In a level**: through the hub's golden mirror into Vegetable Valley with it: Kirby walks, jumps and swings; its stars took a
  Sparky's HP down and beat it; Sparky's hit didn't take the rod.
- **A boss** (forced: Kirby moved from the hub into Whispy Woods' room with the tracker's debug warp, by RAM writes, after taking
  the rod for real): its stars beat Whispy Woods; Kirby took the shard, danced, and came back to the hub still holding the rod.
- **The Power Orb's fight** (forced the same way, into its room): Kirby flies with the rod as before, the card without its picture.
- **All three versions**: built; the stand, the pick-up, a swing, the drop and the rod's return checked in each. Logic checks on
  all three: every room reached, every big switch pressed, nowhere Kirby is trapped, with and without dropping through thin floors
  (the same as before).

Left:
- **Seen, not changed (for the owner)**: the rod isn't drawn in the poses its hat has no NiDL frame for (swimming, climbing a
  ladder and a few others: `KatAMExpansionHats.cs`' `HatFrames`); its stars fly through walls (as in the finale's fights);
  outside the fights the HUD card shows the rod's picture, as for any ability (in the fights its name alone, as before).
- **Not tested**: played by hand; losing it to a death; two players (one rod, as one Master Sword); helpers (they never take it);
  a saved game with the rod; Nightmare's fight with a rod taken at the stand (after it Kirby is Normal, as before: the stand gives
  it again).

## 46. Helpers against the ported mini-bosses
The owner: the helpers (CPU Kirbys) don't know what to do with the ported mini-bosses. Found: with no ability, KatAM's helper AI
hurts a mini-boss only by inhaling what it throws and spitting it back, and it only looks at objects; the ported mini-bosses'
projectiles are attack objects, so the Normal helpers never went for them (helpers with an ability did fight). Done: a helper with
no ability and nothing in his mouth who goes for a ported mini-boss now goes for its nearest projectile he can inhale (Poppy Bros.
Sr.'s bombs, Mr. Tick-Tock's notes, Fire Lion's flames, the knights' axes, javelins and tridents), inhales it and spits it back at
the mini-boss. Checked in the emulator (USA, by script: a new game started in each arena with three Normal helpers, Kirby standing
still, the mini-boss's HP read every 3 seconds for 30 seconds), before, then after:
- **Poppy Bros. Sr.** (0x2EE): 30 HP the whole time; now beaten in 24 seconds (screenshots: the helpers inhale the bombs and spit
  stars at him).
- **Mr. Tick-Tock** (0x2EF): 30 the whole time; now beaten in 9 seconds, the room's doors open after.
- **Fire Lion** (0x2F4, 0x2F8): 30 the whole time; now one hit (30 to 20). KatAM's own Mr. Frosty gets the same from Normal
  helpers in this test: one hit in 30 seconds.
- **Bugzzy** (0x2F3, 0x2F7), **Grand Wheelie** (0x306), **the Meta-Knights** (0x0F5, 0x155): exactly the same before and after
  (the helpers already inhale the enemies Bugzzy and Grand Wheelie call, and the knights themselves).
- **All three versions**: built; in the European and Japanese ones Poppy Bros. Sr. and Mr. Tick-Tock were beaten the same way.
  Logic checks on all three: every room reached, every big switch pressed, nowhere Kirby is trapped, with and without dropping
  through thin floors (the same as before).

Not tested: played by hand; with Kirby fighting too; helpers with an ability (their code isn't changed); a ported mini-boss the
randomizer puts in an enemy's spot.

## 47. The owner's list: pause maps, Star Rod, Mike, Dedede, Nightmare, Map Viewer
- **Pause screen maps** (done; this also settles the old item 2, the NiDL levels' area maps: there are none): with the ROM
  Expansion, SELECT on the pause screen switches between the help screen and the world map only; the area maps' data is wiped
  (`KatAMPauseScreens.DropAreaMaps()`): their backgrounds, room layouts, frame, area titles and shard and map room tiles, 62,190
  bytes in the USA version, 62,088 in the Japanese one, 78,213 in the European one, ready for something else (the ranges are in
  the code's comment). Kept: the room squares, lines, arrows and colors the Map Viewer draws with, the world map's own graphics
  and every sprite. Checked in the emulator (USA by script, in Rainbow Route's Central Circle): before, Help, Rainbow Route's
  area map, world map, Help; now Help, world map, Help, world map, back to the game, the world map drawn as before. The same in
  the European (English) and Japanese builds. A build with and without it differs only in the one changed instruction, the 10
  background table entries and the wiped ranges; the kept graphics are byte for byte the original ROM's, and so are the other
  languages' world map frames between the European area titles. Not tested: the European version in its other languages
  (their data is untouched), pausing in a NiDL room (unchanged: as before, its rooms count as the tutorial's), the Map Viewer
  app itself (it reads only the kept graphics).
- **Star Rod damage** (done): its stars hit with the Master Sword slash's strength, 8 (`sub_08075DB0`'s; it was 1), still as stars
  (KatAM adds the target's star modifier; the sword adds its fire and cut modifiers instead, so the two differ only where those
  differ: Mr. Frosty, weak to fire, takes 11 from the sword's slash and 8 from a star). Nightmare is kept NiDL's: his star
  modifier goes from 9 to 2, so a star still takes 10 of his 60 HP, 6 stars as before (his HP bar's fill is tuned to 60). The
  Power Orb counts its hits itself (one per star), so its fight doesn't change. Checked in the emulator (USA, by script): Mr.
  Frosty took 1 per Star Rod star before, 8 now (30, 22, 14, 6, beaten; the rod given by RAM write); the Master Sword, taken from
  Central Circle's stand for real (the Unlock the Master Sword option) and brought into his room by RAM warp, took 11 with its
  slash (attack strength 8 read from RAM); Nightmare's fight played by the finale script, Kirby firing: 60, 50, 40, 30, 20, 10.
  Not tested: played by hand; the rod against other bosses; Nightmare's last hit (the run ended at 10 HP).
- **Mike**: wrong palette when it's activated.
- **Dedede's fight**: the Fountain of Dreams background is corrupted on the fight's first frames.
- **Nightmare's defeat**: may show a wrong palette.
- **The cinematic on the way to the Fountain of Dreams**: Dedede flies too fast; keep his normal flying speed.
- **Map Viewer**: show Nightmare's entire sprite.

## 10. Screen shake as KatAM's
The owner: the screen shakes far too much. Not reproduced: KatAM's shake runs the same in the NiDL rooms as in KatAM's, and
nothing found shakes more often than in KatAM. Needs the owner's details (which room, what was happening).
