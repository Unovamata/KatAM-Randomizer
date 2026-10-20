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
- **King Dedede**: done (see Done: "King Dedede swallows and spits out the Kirbys").
- **Nightmare's Power Orb**: done (see Done: "The Power Orb's fight is NiDL's Star Rod flight").
- **Every boss** must play like NiDL's. Whispy Woods' face is done (see below); check the others against NiDL. Nightmare's own
  fight is on the ground with the Star Rod in NiDL, as here (item 34), but in his caped look, not the arms-out one the port uses.
- **King Dedede's camera**: done (see Done: "King Dedede's camera is NiDL's").
- **The Power Orb's camera and cinematics** (the owner: "power orb camera movements and transition cinematics have to be ported
  as well."): NiDL's camera movements in the fight and its transition cinematics. Done: the intro (the camera rising from the
  room's bottom with every Kirby flying up to the orb, see item 34), the fight's sinking screen, and its end (the orb beaten, the
  warp star's ride into the moon, the screen rising to it: item 34), Nightmare's intro and his defeat (item 34). Still to do:
  the ending down from the moon (item 34).
- **The helpers in the Star Rod fight** (the owner: "AI Kirbys should move up and down, left and right towards Power Orb to
  simulate that they are engaged in the fight and maybe open the door to avoiding its moves by chance, so disabling the flag so
  they get still ain't quite cutting it."): they keep moving towards the orb in every direction, and dodge its attacks now and
  then; never just standing still. Done: each helper flies to a new spot before the orb every 20 to 51 frames (40 to 134 pixels
  left of it, 32 up or down from its row, never nearer than 40), shooting when in line with it, and when one of its shots comes at
  him (72 pixels ahead, in his row), half the time he dodges it, 16 frames up or down away from it. Tested in the emulator (USA,
  the 3 helpers through the orb's intro and 740 frames of the fight): before, the helpers moved 47-53% of the frames, staying at
  the screen's left (x 36-109); now 70-78%, all over the screen's height (y 18-133) and up to the orb (x 56-150, the orb at
  174-210), shooting it (screenshots). The European and Japanese builds: built and checked by the logic checks only.

### 34. The Fountain of Dreams finale
The owner: "Once Power Orb has been beaten the cinematic plays, all NPC Kirbys go to the warp star, cinematic plays, Nightmare
boss room, nightmare cinematic, fight starts. Nightmare and Power Orb do not drop any shards. The amazing mirror teleports to the
Nightmare fight. After beating nightmare, the player should be teleported to the Fountain of Wishes hub where there would be a
maximum tomato and an amazing mirror warp to 0x38D." Asked what each part meant, the owner said:
- Central Circle's Dimension Mirror and the NiDL hub's both take Kirby to the Power Orb's room, then Nightmare; they work as
  KatAM's does, once KatAM has the 8 shards.
- After the Power Orb: its cinematic, every helper Kirby goes to the warp star, the ride's cinematic, Nightmare's room, his
  cinematic, then the fight. Neither drops a shard.
- After Nightmare: NiDL's cinematic of Kirby going down from the moon, then the player is in the Fountain of Dreams hub (its
  switch room).
- The Fountain of Dreams hub loses all its doors (and its big switch, and every door elsewhere into it) for an Amazing Mirror to
  0x38D, the start of KatAM's final boss sequence, unlocked as the other mirrors are, once KatAM has the 8 shards; a Maximum
  Tomato is there.
- The Power Orb's and Nightmare's rooms can't be randomized.

**Done so far** (the finale's ways in and out; `finale.c`, `KatAMExpansionRooms.cs`' `FinaleMirrors()`): Central Circle's and the
NiDL hub's Dimension Mirrors lead into the Power Orb's room, where its fight starts, once the 8 shards are found (KatAM's own
mirror, its destination changed); the door that led there from Rainbow Resort (0xF7) is gone. The Power Orb and Nightmare leave no
shard (nor do they take part in Bosses Give Random Shards: NiDL's 7 others share the shards). The Power Orb beaten, a warp star
(KatAM's) comes where it was, the helpers fly to it by themselves (they wait just left of it until the player is on, then board),
it leaves as soon as every Kirby in the room is on it (KatAM's 5-second wait at most), and every Kirby rides it to Nightmare's room, his own ability
back as it leaves (no shard dance gives it back any more). Nightmare beaten, the Kirbys dance and land in the Fountain of Dreams
hub, on its floor's left. That room has lost its Mirra in from Rainbow Resort (0x30F), its two floor mirrors (to 0x2FE and 0x30A),
its two hub doors (to 0xBF and the NiDL hub; their doors back are gone too) and its big switch: an Amazing Mirror (KatAM's
Dimension Mirror, to 0x38D, whole once the 8 shards are found) stands in the room's middle, and a Maximum Tomato under it on the
raised platform (see the owner's next request below). The Power Orb's and Nightmare's rooms were never randomized and still aren't (the randomizer reads none of the expansion's
boss or switch rooms' objects; the Fountain hub's tomato stays a tomato). Tested in the emulator (USA, with KatAM's 8 shards
written into RAM where noted): Central Circle's mirror, after KatAM's opening scene, into the Power Orb's room at its start; the
NiDL hub's mirror likewise; the orb's HP set to 1, beaten by a Star Rod star, its warp star where it died, Kirby flown onto it, the
ride to Nightmare's room (his ability NORMAL again on landing); Nightmare's HP set to 1 and his guard lowered in RAM (test only:
the port's Nightmare takes only strong hits), beaten by a slide, the dance, then the Fountain hub; its Amazing Mirror (8 shards)
to 0x38D, Dark Meta Knight's room. The orb's warp star and Nightmare's end tested on the European and Japanese builds too. The logic
checks (with and without dropping through thin floors, all three versions) take the Dimension Mirror as the way in: every room
reached, all 7 big switches pressed, nowhere trapped (the Fountain hub leads out through its Amazing Mirror).

**The owner, later**: "Camera is too far up in the Nightmare room." and "Likewise, when entering the Amazing Mirror, you have to
copy the cinematic before the Power Orb, the one where Kirby flies to the Power Orb and then the battle can start." (the
Dimension Mirrors into the Power Orb's room: NiDL's flight up to the orb plays, then the fight). The camera is done: Nightmare's
room's screen is NiDL's, held at (8, 216) over the floor Kirby lands on (it was centred on Nightmare, too high for Kirby to be in
it, so it never locked); checked in the emulator in all three versions, Kirby on the floor at the screen's bottom left and
Nightmare at its top, as NiDL's screen. The intro is done too: Kirby comes in at NiDL's start, at the room's bottom, and NiDL's
intro plays, recorded from NiDL frame by frame (its room's natural start): the screen rises 761 pixels to the fight's (4 pixels a
frame, slowing to 1), every Kirby flies up into it from under its left on NiDL's path with NiDL's frames (the helpers in formation
behind the player), the orb comes in from above its right and down, Kirby swings at it (its slash across the screen), it pulses 5
times and spins fast round to its spot, and the fight starts there, 319 frames in, as NiDL's (`power_orb.c`, `star_rod.c`'s
`StarRodIntro()`, `arena.c`'s `ArenaRise()`; the rulebook's rule on where the orb's room is entered changed with it). Tested in the
emulator: USA, through the NiDL hub's mirror and through Central Circle's (KatAM's 8 shards written into RAM), screenshots compared
with NiDL's at the same frames (the camera, Kirby's and the orb's places and frames, the slash, all as NiDL's; the camera a frame
behind the sprites, KatAM's camera running first); with the 3 helpers (their own starting spots written so they're in the hub);
then the fight, the orb's HP set to 1 in RAM after the intro, beaten by a Star Rod star, its warp star boarded, the ride to
Nightmare's room (the camera staying on the fight's screen after the orb's defeat, as before). European and Japanese: the intro
through the NiDL hub's mirror, the same positions every frame as the USA one, and the fight after it. The background now scrolls
as NiDL's up and down (see the cloud band below); NiDL's cloud bands also drift sideways, each at its own speed, the port's don't.

**The owner, after that** (the finale's list again, with): "The Fountain of dreams Mirror should be at the center of the room. The
tomato should be at the bottom center of the room in top of the elevated platform.", "Kirby should keep the Star Rod in the
Nightmare fight." and "Fountain of dreams BG is off centered in the hub."
- The hub: done. The Amazing Mirror hangs in the middle of the room as the camera shows it (then 128, 92: its sprite's middle at
  the screen's; since moved up, see the owner's next request below), in front of the Fountain; the tomato is under it, on the raised platform at the room's bottom middle (128, 121); the
  background's Fountain was 15 pixels right of the middle (measured in the emulator: x 143, the room's middle 128), now a column
  left of where it was, at 127. Kirby jumps up into the mirror (it hangs above the platform; KatAM's mirror takes him when he's
  inside its box pressing Up, in the air too). Tested in the emulator (USA, 8 shards written into RAM): the room as Kirby arrives
  (screenshot, the Fountain's and the mirror's middles measured), a jump from the platform and Up into the mirror, to 0x38D.
- Kirby keeping the Star Rod in Nightmare's fight: done. NiDL's Nightmare is fought with the Star Rod on the ground (recorded in
  NiDL: Kirby walks, runs, jumps holding the rod up and B swings it, shooting a star; frames `0xFED`-`0x1035`). Every Kirby rides the
  orb's warp star into Nightmare's room keeping it, and keeps it there: the rod shows in every pose NiDL has one for (its hat,
  matched to KatAM's poses), B swings it as NiDL's (16 frames, the star on the 5th, 20 ahead, 4 pixels a frame, either way), a hit
  doesn't take it (KatAM's hit handler detoured) and anything else that does (death, Select) gives it back; after Nightmare, the
  ability from before the orb comes back. Tested in the emulator (USA): from the NiDL hub's mirror through the orb's intro, fight
  and warp star into Nightmare's room, the Star Rod still held on landing; in Nightmare's room, walking (the rod on his head),
  swings both ways and in the air (screenshots), three hits taken with the rod kept, two stars taking Nightmare from 60 to 58 HP;
  his HP set to 1 in RAM and a star finishing him, the dance, the Fountain hub; with the 3 helpers in the room (every Kirby holding
  it, stars flying; nothing breaks). European and Japanese: in Nightmare's room, the rod kept through hits, stars hurting him. Not
  tested: Kirby coming out with an ability other than Normal (none was held before the orb in these runs).

**Nightmare's defeat** (done; `bosses/nightmare.c`'s `NightmareEnd()`, `finale.c`'s `NidlEndingFlight()`): NiDL's, from the recording
below: KatAM's freeze and burst, then 24 frames after it (as the Power Orb's end) he flies off right from where he was, slowing (4, 2,
1, 1/2, 1/4 pixels a frame, 16 frames each), turns to light, comes back as light at (136, 282), caped with his top half over him (NiDL
draws his frame 96 with his body frames), spreads out and breaks up into light, blinking, with the light over him, all on NiDL's frames
for NiDL's times and mirrored as NiDL draws him; bursts go off along the floor; then every Kirby still in the room is flown out where
he stands (a flight: KatAM's fade to white, then the Fountain of Dreams hub) with his ability from before the Power Orb back. The
Kirbys stay free through it (NiDL's walks and jumps holding the Star Rod); the screen stays put. No dance any more. KatAM loads only 7
sprite palettes a room and his fight fills them (his second and third palettes and his beams' even shared the last slot), so the
defeat puts his palettes into the intro's leftover slots. Tested in the emulator (a new game in the NiDL hub, its mirror into the
Power Orb's room, the orb's HP set to 1 in RAM and beaten, the ride to Nightmare's room and his intro, then his HP held at 1 in RAM
and Star Rod stars by set inputs; USA solo and with the 3 helpers, European and Japanese rev 1 solo): screenshots beside NiDL's at the same moments (his caped shape, the spread, the breakup, the light over him, the bursts, the
fade, all as NiDL's; his spot when he comes back the same to the pixel), every Kirby landing in the Fountain hub. The logic checks
pass on all three. Not tested: a Kirby knocked out or in another room when the flight starts (it flies only those in the room and
up). Not matching NiDL: NiDL's flight off starts from where he was hit 52 frames after it; KatAM's own burst comes first here.

**The owner, on the ending**: "Work on the The ending down from the moon: the crescent moon, the warp star ride down through the
Power Orb's room, without the narration, King Dedede flying by, and cutting to the fountain of dreams before kirby throws the wand
as a transition from the Power Orb/Nightmare fight into the Fountain of Dreams hub." Taken as: the moon turning to a crescent, the
warp star with the Kirbys flying out of it and down through the Power Orb's room (its sky, as NiDL's moon room is that room), King
Dedede flying by, no narration, and at NiDL's cut to the Fountain of Dreams (before Kirby throws the Star Rod onto it) the Fountain
of Dreams hub instead.

**Still to do**: the ending down from the moon (after the defeat, before the hub). Recorded in NiDL (USA; Nightmare's HP held at 1 and Kirby kept beside him
until a star hits, frame by frame, screenshots every 60 to 150 frames): his defeat (the hit at its frame 2246: the freeze and the
burst, then he flies off right and dissolves into sparkles, his frames 76-88, about 900 frames), then a scene of its own (from 3164,
about 4200 frames): the full moon, a burst in it leaving a crescent, Kirby on the warp star flying out of it and down through the
night sky under NiDL's narration ("Our hero Kirby has fought bravely to recover the Star Rod..."), King Dedede flying by, the sky
turning to Dream Land's dusk, then the Fountain of Dreams with the Star Rod back on it, a flash to white and the credits. Porting it
needs NiDL's cutscene backgrounds and its text (or a room of its own for the ride down). The owner's next list keeps "the ending
down from the moon" (no word on the narration or King Dedede: taken as NiDL has it). Recorded since (NiDL, frame by frame, its
objects, OAM, palettes, the room and the layers):
- His defeat (NiDL's frames from the hit; his object, type 0x43, runs it all): 52 frames frozen in his hurt frame (the hit's flash
  and bursting stars), then he flies right from where he was on his frames 68-71 (3 frames each), 16 frames each at 4, 2, 1, 1/2
  and 1/4 pixels a frame (124 pixels), turns to light there (60 up to 67, 2 frames each), and comes back as light at (136, 282) in
  his room (66 down to 60), caped (50 for 5 frames, 51 for 10), then spreads out (76/77, 2 frames each, 40 frames), blinking:
  78/79 (60 frames), 80/81 (60), 82/85 (120), 83/84 (64), 86-88, gone 535 frames after the hit; from 280 to 430 a light over him
  (frames 13-17 of NiDL's table 0x08754738, the descriptor's own tiles and first palette). Frames 68-71 use his three palettes,
  76-80 the first two, 50-51 the third, 81 the first, 82-88 his light's (the ones the port loads already). Kirby stays free (he
  walks and jumps holding the Star Rod). From 714 to 910 bursts along the floor (NiDL's shared burst, y 352), then the fade to
  white and the moon.
- The moon scene is a room of its own, the finale stage's third (16x68 cells, no objects): the Power Orb's room again (the same
  foreground, the moon at its top, and the same background picture), the camera from its top (8) down 3/16 of a pixel a frame (to
  666 in 4400 frames), NiDL's cutscene objects (type 100) on it (Kirby on the warp star out of the moon, King Dedede), the
  narration on its top layer (BG0, off once the text is done), then the Fountain of Dreams scene (objects of type 101) and the
  credits. The port can play the ride down in its Power Orb's room. Nightmare's
fight itself isn't NiDL's yet (item 5): NiDL fights him in his caped look (his body, 48-55, under his collar and cape, 96), the port
in his arms-out one (8-15), so his look changes as the fight starts after his entrance. Not tested yet: the whole finale with
helpers in one go. Also open:
- His fight's palettes: KatAM loads 7 sprite palettes a room and loads past them into the last slot; by his fight his intro's and
  his own fill them, and his second and third palettes and his beams' take that last slot in turn, so one of them can show another's
  colours during the fight (found while porting his defeat, which works round it; the fight itself not fixed yet).
- His entrance: the helpers start shooting once they're up, before his light has grown into his shape (NiDL has no helpers).

**The owner, on the finale's list** (asked whether the ending should have the narration and King Dedede and where it ends): "the
cinematic after the orb is beaten, the helpers flying to the warp star by themselves, Nightmare's intro and chase, the ending down
from the moon, and helpers that move toward the orb and dodge during the fight.", "The amazing mirror in the fountain of dreams hub
should be further up and drop the pedestals." and "The shown sprite strip is cutoff in the power orb room compared to NiDL." (a
screenshot of NiDL's Power Orb fight, the pink and purple cloud band along the screen's bottom circled).
- The finale's list: every part of it stays wanted, the ending down from the moon included (still to do, above), and played
  together with the helpers.
- The hub's Amazing Mirror further up, and its two ability statues ("pedestals", on the top row) gone: done. The mirror hangs 24
  pixels higher (128, 68), under the ceiling where the statues stood, above the Fountain's bowl, which now shows under it; the
  statues (Smash and Burning) aren't placed in the Fountain of Dreams hub (the other switch rooms keep theirs). Kirby floats up
  into it (a jump from the platform no longer reaches it). Tested in the emulator (USA, European and Japanese rev 1; a new game
  started in the hub, KatAM's 8 shards written into RAM): the room as Kirby arrives (screenshots), then by set inputs a jump
  onto the platform, a float up under the mirror and Up: into it, to 0x38D, Dark Meta Knight's room.
- The Power Orb's room's cloud band: done. NiDL draws this room's background at the camera's spot (measured in NiDL by matching
  its screenshots to the background: through the intro and the fight, the background's row at the screen's top is the camera's
  y), its top at the room's top; KatAM bottom-aligns backgrounds, so the port's sat 176 pixels higher, and the room's starry sky
  (its foreground, opaque, with the moon, ending in a straight edge) hid the band's dark purple top: the band started pink at a
  hard line. Now the room's background is drawn as NiDL's (`KatAMExpansionRooms.cs`, the room's props), and the band comes up
  under the sky with its dark top fading in, where NiDL shows it. With it, the background's tiles past KatAM's 900 (this one has
  956) are merged by how little the picture changes (in their own colours, times how often they're drawn): before, the 56 least
  used went, whole rows drawn once, the foot of the trees at the room's bottom, which the intro now shows (they came out as
  blocks); the same rule tidies the two other backgrounds over 900 (the Fountain of Dreams' floor's last rows, Nightmare's
  room's bottom row; a few dozen star pixels elsewhere change). Tested in the emulator (USA, European and Japanese rev 1; a new
  game in the NiDL hub, its mirror into the room, the fight left running with Kirby's HP topped up in RAM): screenshots beside
  NiDL's at the same camera heights (254 and 276-278: the band in the same place, its dark top showing; the intro at 697: the
  trees in the same place), the background's row at the screen's top matched to the camera's (278 at 278); the orb's end after
  it (USA: beaten by a star with its HP set to 1, the ride to the moon and into Nightmare's room). Not matching NiDL yet: its
  cloud bands drift sideways, each at its own speed; the port's stand still. KatAM's HUD, taller than NiDL's, covers the band's
  bottom.

**Nightmare's intro, the hit and his entrance** (done; `bosses/nightmare.c`'s `Knock()`, `Puff()`, `Light()`): as NiDL's, recorded
frame by frame: the last blue star bursts on the warp star, which is gone the next frame (the Kirbys drawn without it), a puff where
it was (NiDL's shared effect), and the Kirbys are knocked off: NiDL's tumble up and down to the floor, its bounce and its slide back
(KatAM's tumbling animations, as the rides' tumbles, then Kirby's back to us, KatAM's door pose: NiDL's has the Star Rod held out to
the right, the port's rod is held up, the Star Rod's pose for that body), the helpers spreading out to the player's right as they
fly; 260 frames after the hit they're up and theirs again (the flight ends standing, facing Nightmare, no hop), as his light grows
into his white shape where NiDL's appears (113, 280: his own frames 67 down to 60, two frames each); the fight starts there.
Tested in the emulator (USA, European and Japanese rev 1; as the Power Orb's end, with the 3 helpers, and USA alone): screenshots
side by side with NiDL's (the hit, the tumble, the slide, the light, his spot) and every frame's spots in the log (the tumble's path
as NiDL's to the pixel, the slide back to x 101, every Kirby standing on the floor once up); the fight after it (the helpers
shooting, his beams). The logic checks pass on all three.

**Nightmare's intro, the comet and the blue stars** (done; `bosses/nightmare.c`'s intro effects): as NiDL's cutscene, recorded
frame by frame (its objects' spots, speeds, frames and sizes, the OAM's affine matrices, the palette RAM): his comet far off in the
sky (NiDL's own cutscene graphics, found in the NiDL ROM by `KatAMExpansion.cs`'s `CutsceneGraphics()`), the dark orb drifting left
and waking up, then off to the right; the screen flashing as it wakes (the room's palettes brighter, the sprites' darker, NiDL's
exact colors; with "Reduce Flashing" on, a steady fade up and down without the flicker); the 12 blue stars it throws (each its
moves, growing and spinning, blinking out; their tables replayed against the recording, every frame the same) and the 6 bursts on
them. Tested in the emulator (USA, European and Japanese rev 1; as the Power Orb's end, with the 3 helpers): screenshots side by
side with NiDL's at 14 moments (the comet, the flash, the stars and bursts in the same spots within a frame); the screen's
brightness every 2 frames through the flash (132 frames, NiDL's flicker and steps; and with "Reduce Flashing": the steady steps);
the intro ends on the same frame as before (no slowdown). The logic checks pass on all three.

**Nightmare's intro, the star's arrival** (done; `bosses/nightmare.c`'s `NightmareIntroRider()`): as NiDL's, recorded frame by
frame: the screen from his room's top down to the fight's screen (a pixel a frame, then a pixel every 4 frames), the star flying
down into it with the Kirbys along NiDL's path, then about the screen until they hop off (KatAM's hop, for now) 910 frames in, when
the fight starts; Nightmare waits hidden until then (his fight started the moment the Kirbys came in, as they land in his screen
for a frame first: he now waits while a Kirby in his room rides a warp star). Tested in the emulator (USA, European and Japanese
rev 1; as the Power Orb's end, with the 3 helpers): the whole arrival, the hop off, then the fight with all four Kirbys holding the
Star Rod (screenshots); the logic checks pass on all three.

**The Power Orb's end is NiDL's** (done; `bosses/power_orb.c`'s `PowerOrbEnd()`): recorded in NiDL frame by frame (the orb's, the
warp star's and Kirby's spots, the camera, the sprites' sizes in OAM, twice: the orb beaten at the fight's start and 1200 frames
in). KatAM's freeze, flashes and burst stay (NiDL's are the same); 24 frames after the burst the orb is back, wobbling, and flies
off up to the right, and the warp star comes 11 pixels under where it was. With every Kirby on, the star flies NiDL's launch up off
the screen; 21 frames in, the screen rises to the room's top (NiDL's easing: 1.25 pixels a frame, a 16th more each frame, slowing
in the last stretch), the star hidden just over it; then the orb, a quarter of its size and smaller every 20 frames, flies into the
moon, the star with the Kirbys 60 frames behind it the same way, both then black dots (NiDL's 8x8 dot, rebuilt from its VRAM), and
410 frames after the orb the fade to white and Nightmare's room. The screen is held where the fight left it until the ride: before,
beaten after the screen had sunk (any fight longer than a few seconds), the camera went back up to the fight's first screen and took
the Kirbys flying with the Star Rod up with it, leaving the warp star below, out of reach (a softlock; reproduced in the emulator,
the orb beaten 1000 frames into the fight). Tested in the emulator (USA, European and Japanese rev 1; a new game started in the NiDL
hub, its mirror into the Power Orb's room, the orb's HP set to 1 in RAM, Kirby killing it and flying onto the star by set inputs):
beaten at the fight's start with the 3 helpers (all four board, ride, shrink into the moon and land in Nightmare's room; KatAM's
arrows pointing at off-screen helpers don't show while the star waits above the screen), and solo; beaten 1000 frames in, solo (the
screen stays at 211, Kirby reaches the star, the screen rises 203 pixels to the top before the moon). Screenshots compared with
NiDL's frame by frame (the burst, the flight off, the launch, the moon, the dots, the fade). Not tested: the end with helpers after
a long fight; the helpers' own reaching of a star placed far from them.

**The helpers fly to the orb's warp star** (done): tested in the emulator (USA, European and Japanese rev 1; a new game started in
the NiDL hub with the 3 helpers, its mirror into the Power Orb's room, the orb's HP set to 1 in RAM, Kirby killing it and flying
onto the star by set inputs): the helpers came to the star by themselves, waited just left of it in a row, and boarded the moment
Kirby did; the star left about 30 frames later, once all four were on (before: KatAM's full 300 frames), and all four landed in
Nightmare's room. Solo, the star left as soon as Kirby was on.

### 10. Screen shake as KatAM's
- The screen shakes far too much; something broke along the way. Match KatAM's screen shake.
- Not reproduced yet; needs the owner's details (which room, what was happening). Checked so far: KatAM's shake itself runs the
  same in NiDL rooms as in KatAM's (a strength-2 shake written into its task in RAM: the same 16 frames of 2 pixels in 0x0F0 and in
  Central Circle); the expansion's code asks for a shake only when Grand Wheelie hits a wall and when Bugzzy slams Kirby, once each
  (strength 3, as KatAM's mini-bosses); NiDL's Shotzos are KatAM's Shotzo (0x35), which doesn't shake, not its 8-way cannon (which
  shakes on every shot); 600 frames of the 0x164 boss fight shook 4 times, each when Kirby was hit (62 frames). KatAM shakes on every
  hit on an enemy (1), every enemy defeated and every hit on a boss (3), so randomized mini-bosses in enemy spots ("Include
  Minibosses") shake the screen as often as they do in KatAM.

## Done (rules to keep)

- **King Dedede's camera is NiDL's** (item 5; the owner: "I noticed that DeDeDe's camera doesn't lock the same as NiDL."): NiDL's
  fight was recorded in an emulator: Kirby drops into the room from its top, the camera following him down 2.5 pixels a frame (its
  top 80 pixels above him) from the room's top to the ring's screen, then held there, (24, 168) to (264, 328), the whole fight.
  Before, Kirby landed on the ring's floor with the screen already on it, and the screen was the ring's middle, not NiDL's. Now he
  comes in at the room's top as NiDL's (`DededeBoss`, KatAMExpansionRooms.cs), the arena waits at the top for him (a drop-in arena:
  `arena.c`) and its screen rises with him as he falls (85 pixels below its top, NiDL's 80: the look is the same) until it's NiDL's
  screen, held from then on; King Dedede comes in from the ring's right as before. Tested in the emulator: new games started at the
  top of his room 0x313 with no input, the camera and Kirby read every frame and screenshots compared with NiDL's: USA, European and
  Japanese rev 1 builds all start at the top (8), follow the drop at 2.5 pixels a frame and hold at 168 (the fight's screen); the
  logic checks (with and without dropping through thin floors) are unchanged: every room reached, all 8 big switches pressed,
  nowhere trapped. Not tested: a whole fight to his defeat after the drop, or with helpers.
- **The Power Orb's fight is NiDL's Star Rod flight, for Kirby and every helper** (item 5; the owner: "The Power Orb's Star Rod
  flying sequence, for the player and all helpers."): NiDL's fight was recorded frame by frame in an emulator (Kirby's flight and
  stars, the orb's states, shots and timings, its true camera). Now Kirby comes in at the top of the fall, in the air where NiDL's
  flies at its start, and every Kirby in the room takes the Star Rod (a new ability, 34: NiDL's frames of Kirby with the rod, its
  star and HUD card) and flies over the screen shooting stars, as NiDL's (speeds, swing and shot timings, knockback); the screen
  sinks 0.2 pixels a frame as they fight, as NiDL's camera does; the orb hovers, roams, charges and zips with NiDL's shots
  (streams, fans, pairs, the charge's column of stars and its dash, the zip's crescents), 60 HP, 1 a star. The helpers fly by an
  AI of their own (KatAM's helper AI held still meanwhile). Kirby's ability before the fight comes back after the shard's dance.
  Before, the orb was fought on the floor at the bottom of the room. Fixed along the way: the star never damaged the orb (KatAM
  only takes a hit from an attack with a kind bit; it has KatAM's star kind now); the game froze with helpers in the room (KatAM's
  helper AI looks forever for a navigation zone inside the camera: the sinking fight has a zone all the way down now, and riding
  helpers' AI is held); the camera stayed held after the orb was beaten mid-sink, so after the shard the Kirbys hung in the air
  and never danced (a softlock); the ability card read NORMAL when the rod was given; the Japanese build's addresses of two HUD
  functions were wrong (every NiDL ability's HUD update called into the middle of other code there). Tested in the emulator (new
  games started in the Power Orb's room 0x003, the player held B or followed set inputs, the helpers KatAM's CPU Kirbys): USA solo,
  the orb's HP read every 10 frames while Kirby shot (60 to 34 in 600 frames, 1 a hit); USA with 3 helpers, 2100 frames, no
  freeze, the helpers flying, shooting, hurt, dying and coming back on the Star Rod, the player dying and coming back flying; USA,
  EU and Japanese rev 1 with 3 helpers, the whole fight to its end: the orb beaten (frame 600), the player flies into the shard,
  the cutscene, the Kirbys fall to the room's floor and dance, then Nightmare's room (2131-2133 frames), screenshots of the flight,
  the stars, the HUD card (STAR ROD, スター・ロッド) and the dance; the ability coming back, with the saved ability set to Fire by a
  RAM write mid-fight (a forced state). Logic checks on all three: 280 rooms reached, 8 big switches, no traps, with and without
  drops. Not tested: a real playthrough into the room through its door, link play, a rematch with the shard owned (the Maxim
  Tomato), winning solo, a helper called by phone after the orb is beaten.

- **King Dedede swallows and spits out the Kirbys** (item 5; the owner: "King Dedede inhales, swallows and spits out Kirby and
  the helpers."): NiDL's fight was recorded frame by frame in an emulator (his state, frame and position, Kirby's, every object's).
  His inhale is now NiDL's: the mouth opens (28 frames), he sucks up to 90 frames with sparkles flying into his mouth, every Kirby in
  NiDL's catch box in front of him (the player's and the helpers) is caught with KatAM's own grab and drawn into his mouth, hidden;
  he shakes puffed up for 30 frames, turns to the room's middle and spits them out in a star (NiDL's object 0x70, ported as
  `enemies/spit_star.c`) that flies along the floor with them inside until the wall or the screen's side, where it bursts and they
  pop out hurt (1 HP). Before, his inhale only drew Kirby in and touching him hurt. Tested in the emulator (new games started in his
  room 0x313, Kirby played by the game's own inputs, the helpers by KatAM's CPU): USA solo, 4 swallows in 3000 frames, each 1 HP,
  57-78 frames from catch to release by how far the star flies (NiDL's recorded one: 75), screenshots of the spit compared with
  NiDL's (the star comes out on the frame he opens his mouth); USA with 3 CPU helpers (Dedede's HP raised to 600 in a test copy so
  they don't beat him first), all 4 Kirbys caught at once and spat out together in one star (4000 frames: 4 swallows, 2-4 Kirbys each, every one 1 HP). European: the same 2 swallows as the
  USA run; Japanese rev 1: a swallow and its release, 1 HP. Logic checks on all three: 280 rooms reached, 8 big switches, no traps,
  with and without drops. Not tested: a real playthrough to the boss.
- **Warp stars fly NiDL's own rides** (item 30; the owner: "The NiDL warp stars do this animation with scale and some places is
  off. Some routines are a bit off so you should reference all the properties of scale, rotation, positioning, and states these
  animations go off from since some are slightly off."): NiDL's warp star (object type 0x4A) was recorded frame by frame in an
  emulator, for the 6 of the ported stars Kirby gets on in NiDL. Each runs a ride of its own: jumps 16 pixels up as Kirby gets on,
  bobs 32 down under him for 24 frames, flies its own path out of the room trailing stars, crosses the sky rooms on its way (each
  entered where NiDL's comes in), dives into the room it lands in, the orb bursts and Kirby tumbles off and bounces once to where he
  stands. The star's growing as Kirby gets on is its frames (a 24-pixel star, then a 32x32 one with Kirby drawn on it), and the
  turning is Kirby's tumbling frames. Two rides also shrink the star and Kirby as they fly off into the distance (the owner asked
  again: "I vividly remember NiDL's stars have some scaling of Kirby and the star going on, did you copy that?"; the first pass
  had checked only Vegetable Valley 2's OAM): Ice Cream Island 2's (from 1/1 to 2/3 over its climb, then at once 1/4, to 1/10 by
  its room change) and Orange Ocean 3's sky (from 1/1 to 1/20). NiDL scales them with affine matrices (pure scale: no rotation,
  no double size), all their pieces (Kirby's, the star's 4) around the star's middle, each moved in toward it. OAM read every
  frame of all 6 rides, to their end: the other 4 never scale.
  The ported stars now fly those rides (KatAM's star and rider, NiDL's paths): `NidlRides` holds each ride's moves, `WarpFlights()`
  makes them KatAM flights, and `warpstar.c` skips KatAM's 300-frame wait before take-off, takes the flight through more than one
  room, ends it where Kirby stands, and draws Kirby and his star at NiDL's size frame by frame where it shrinks them (each step
  carries NiDL's matrix value; Kirby's drawing detoured, the riding star's task swapped). Before each room change the star flies
  its moves in NiDL's 18-frame fade, with KatAM's own fade to white (the one KatAM's flights start before theirs): without it,
  KatAM's room change loaded the next room on screen (3 frames of garbled tiles). Before, KatAM's star waited 300 frames, sank to
  the floor, and its sky rides were an arc
  that took Kirby off the top of Vegetable Valley 2's sky. Tested in the emulator (USA, expanded; new games started in the stars'
  rooms, Kirby put on the star in RAM: the ride itself is the game's): Vegetable Valley 2 (0x74 → sky 0x7B → 0x7C), Ice Cream
  Island 2 (0xED → 0xEE), Butter Building 4 (0x150 → 0x151 → 0x152), Yogurt Yard 5 (0x240 → 0x241), Orange Ocean 3 (0x27E → sky
  0x280 → 0x27F) and Rainbow Resort 2 (0x2EB → sky 0x2F9 → 0x2EC): each goes through NiDL's rooms, comes into each where NiDL's star
  does (VV's sky at 164, 496 and its last room at 13, 16), and Kirby ends where NiDL's does (VV, ICI and YY at x 119, BB 90, OO 79,
  RR 122; KatAM's Kirby stands up to 4 pixels lower on the same floor). European and Japanese: VV and OO, and VV and RR, the same.
  The shrink and fade, tested the same way, OAM read every frame: ICI 2 and OO 3 draw Kirby and the star at NiDL's size frame by
  frame (KatAM's ride 5 frames behind NiDL's in ICI as Kirby gets on, the sizes on the same spots: OO at 152, 208 both at
  NiDL's 1024), compared with NiDL's frames in screenshots, on USA, European and Japanese; the screen goes white over the 18 fade
  frames, the room changes under it, and the landing room fades in, no garbled tiles; all 6 rides still land where they did (the
  4 others not scaled, as NiDL's). Logic checks pass on all three and a randomized USA ROM. Not the same as NiDL: the camera is
  KatAM's, which follows Kirby (NiDL's scrolls the sky rooms at its own pace: its shrunken star sits higher on screen); the star
  and rider are KatAM's sprites. Not tested: Grape Garden 3's star (stage 3-2, room 2: Kirby couldn't get on it in NiDL, so it
  keeps the flight it had, without the wait); KatAM's own warp stars (the code leaves them as they were: they have subtypes under
  6 and their flights none of the marks); riding a shrinking star with a copy ability (its hat is drawn by the same function, so
  it's scaled with him, but no ability could be had in the rooms tested).

- **NiDL hubs have 2 doors into their level** (item 32, the owner's answer: "NiDL's doors inside the hub rooms, should have 2
  doors as this allows for branching paths inside the hub doors for NiDL, this is a rule that applies for all hubs. I'm talking
  about the doors that go to the exploration map, not the ones that go to the hub."): each level's switch room now has 2 floor
  mirrors into its level, both two-way: the right one to the start of the stage before the boss (as before), the left one to the
  start of a stage on another branch (the deepest off the way to the boss stage, not the Mirra's stage). The hidden Mirra's way in
  now lands off them: on the pedestal's left end beside the big switch (layout A), or 2 cells right of the left mirror (layout B,
  where every floor cell is within 4 cells of a door: rule 3's fallback). Checked in the USA build's data: all 8 switch rooms
  have both mirrors, each leading to a different stage and back onto the mirror. Logic checks pass on USA, European, Japanese and a
  randomized USA ROM (280 rooms reached, 8 switches, nothing trapped, with and without drops). Tested in the emulator (USA,
  expanded): new games started in 0x064 (layout A) and 0x0C9 (layout B), Kirby walked to the left mirror and pressed Up, arriving
  in 0x097 and 0x0FD on the mirror back; Up there took him back onto the left mirror. The Mirra's doors in 0x09D and 0x0F6 were
  taken with their cell set to the door in RAM, standing in for the Mirra breaking (a forced state; the trip itself was the game's):
  Kirby landed in 0x064 on the pedestal at cell (6, 7), the switch not pressed, and in 0x0C9 at (3, 6). Not tested: breaking a
  Mirra in a script. The start-up jumps set it off before Kirby could inhale it, and its door didn't open in those runs, on this
  morning's build too, so not from this change. Worth a check by hand.

- **Shard chests open only once opened** (item 23): a mirror shard chest used its shard's bit as its own, so it showed open, never
  opened, once the shard was owned from anywhere. Each shard chest now has a treasure index of its own (0x88 + k, the 16 chests at
  most: KatAM's `shardField` bits 8-23, which nothing else uses or counts). Opening one marks it, so it shows open once opened and
  stays so. Opened when its shard is already owned (a boss's, the field's, the other copy's), it lets out a Maxim Tomato instead of
  the shard, as an owned field shard is one. Tested in the emulator (USA, randomized with 2 copies of shards in chests; new games
  started by the chests, Up to open: real openings; shard owned set in RAM for one case): 0x095's chest (shard 2) gave the shard and
  set shard 2's bit and its own (index 137); with shard 2 already owned it gave a Maxim Tomato (Kirby's HP 1 to 6) and set its own bit.
  With a save file kept between runs, rebooting showed 0x095's chest open. The other shard 2 chest (0x159, index 145) showed closed
  with shard 2 owned, then gave a tomato and set only its own bit. European and Japanese builds give the 16 chests the same indices.
  Logic checks pass on all three. Not tried: a vanilla ROM with only the shard chest option (the code is the same file).

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

- **Warp star rides through NiDL's skies** (item 8, rooms 0x2F9 and 0x280; replaced since by NiDL's own rides, item 30 above):
  Kirby arrived on the sky room's bottom row and its edge
  exit sent him straight on, a white flash. A warp star into a sky room got a flight of its own (`SkyFlight()`, gone): KatAM's flight
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
