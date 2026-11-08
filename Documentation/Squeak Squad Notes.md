# Kirby: Squeak Squad notes

What has been found in Kirby: Squeak Squad (USA, `AKWE`, Nintendo DS) for porting its cast into KatAM. Addresses are the USA
release's, in the ARM9's RAM (the main program, uncompressed, loaded at 0x02000000) or in an overlay's.

## The ROM

- An NDS ROM: the main program (ARM9, 1.6 MB, not compressed), 13 overlays (0-10 at 0x02209560, 11-12 at 0x02248E20) and a file
  system. While playing a stage, overlay 4 (gameplay) and overlay 12 are loaded.
- Tools: `ndspy` (Python) reads the file system and overlays; `capstone` disassembles (the code is mostly Thumb); `py-desmume`
  runs the game headless (`SDL_VIDEODRIVER=dummy`), with breakpoints (`memory.register_exec`), memory reads and writes, input and
  save states.
- Movement is in 20.12 fixed point (`fx32`: 0x1000 a pixel) per frame.

## Graphics files (`enemy/`)

Each character has a set of files of HAL's own formats, all little-endian:

| File | Magic | What it is |
| --- | --- | --- |
| `.cga` | `BCGA` | Tiles, 4 bits a pixel: a 0x14-byte header (`+0x10`: the tiles' size), then the tiles. |
| `.cla` | `BCLA` | A palette: a 12-byte header (`+8`: the number of colors, 16), then the colors (BGR555). Some characters have more than one (`chun_blue`, `chun_green`, `chun_yellow`: the Squeakers). `_dmg.cla`: a damage palette. |
| `.cea` | `BCEA` | Cells and animations (below). |
| `.rca` | `BRCA` | Boxes per animation frame, loaded by `LoadBoxes` (`0x02014010`). Header words at `+4`: `0x30`, 1, then the offsets of the animations (`+0x28`: a `u16` count, one less than its entries, a `u16`, then each animation's offset; an animation: a `u16` count, a `u16`, then (frame set, frames) `u16` pairs, its time), the frame sets (`+0x10`: a `u32` count, a `u32`, the offsets; a set: a `u32` count, then (box list, tag set) `u16` pairs), the box lists (`+0x14`: count, then (boxes, first box) `u16` pairs), the boxes (`+0x18`: a `u16`, a `u16` count, then (left, top, right, bottom) `s16`, pixels from the feet, x ahead, Y up; Perara's first -5, 10, 5, 0), the tag sets (`+0x1C`: count, offsets; a set: `u32` count, `u16` record indexes) and the records (`+0x20`: count, offsets; a record: `u16` tag, `u16` size, its data). Tags: 0x101 an attack (the body's `+0xF0` list: data `u32` kind first, 0 hurting by touch, 6 grabbing, then the damage at `+8`: Uja's 4, 6, 9), 0x102 where it's hit (`+0xF4`), 0x201 and 0x202 the body (`+0xF8`, `+0xFC`). At run time the box is turned with the facing (x negated). |
| `.mca` | `BMCA` | Composite animations (below): the bigger characters, and Gussa, Cret, Bubble Head, Acchi. |

### `.cea`

- Header: `BCEA`, `0x11`, `0x0B`, the cells' offset (`0x14`), the animations' offset.
- Cells: `u16` count, `u16`, `u32` the OAM list's offset (from the cells' offset), then one 8-byte entry per cell: `u16` OAMs,
  `u16` the first OAM's index, `u16` the cell's first tile, `u16` its tile count. The OAM list: a `u32` count, then 8 bytes an
  OAM (`attr0`, `attr1`, `attr2`, padding: the DS's own OAM, with the tile number in `attr2` counted in 4-tile steps from the
  cell's first tile).
- Animations: `u16` count, `u16` 0x0C, `u32`, then one `u32` offset (from the animations' offset) per animation. An animation:
  `u8` frames, `u8` flags (1 loops), `u16`, `u32` total length, then 8 bytes a frame: `u16` cell, `u16` frames shown, `s16` x and
  `s16` y: where the cell is drawn, in pixels (y down), added to its position when the OAM is built (`0x01FF8720`, in ITCM).
- The cells face right (KatAM's enemies' frames face left).
- The header's byte at `+8`: with flag 8 (the characters' files, `0x0B`) a cell's entry is 8 bytes as above; without it (the
  shared effects' `efc_cmn0`/`efc_cmn1`, `0x03`) it is 4 bytes (OAMs, first OAM), and the OAMs count their tiles from the file's
  first.
- A frame's x offset turns with the character: facing left, it's drawn mirrored (Uja's animation 2, x 8, draws 8 pixels to the left
  of it then; checked in the DS emulator).

### `.mca`

Compositions of the character's own `.cea` animations, played together: the sprite object's `vtable +8` with `0x3000000 |
animation` (`0x1000000 |` plays a `.cea` one).

- Header: `BMCA`, a word, `+8` the compositions' offset, `+0xC` the animations' offset.
- Compositions: `u32` count, then per composition `u32` parts and `u32` their offset (from the compositions'); a part is 8 bytes:
  `u16` the `.cea` animation, `u16` its timing, `s16` x, `s16` y (pixels, y down, as the character faces right; mirrored when it
  faces left, `0x01FFA640`).
- Animations: `u16` count, `u16` 0x18, `u32`, then a `u32` offset (from the animations') per animation; one: `u8` steps, `u8` loops,
  `u16`, `u32` length, then 8 bytes a step: `u16` composition, `u16` frames, a word (always 0).
- A part's timing (read at `0x01FF9BC6`, `0x01FF9EBE`, `0x01FFA6D0`): 0, its animation plays at the composite animation's time
  within it (Gussa's body: 0 to 23, over again); 1, at the frames since the composite animation started (it goes on counting;
  the part's own animation loops at its own length). Checked with breakpoints in the DS emulator.
- The first part is drawn on top (Kracko's eye, part 0, over its cloud). The parts are drawn last to first.
- Some characters' parts loop at lengths whose combinations run long (Dedede's composite 0: 371,280 frames before it repeats), so
  the port plays them live, a sprite per part, rather than as frames drawn ahead of time.

### The stage's box tests

The stage's map object (`*0x02229F1C + 0x48`, from `0x02209560`) answers the enemies' own probes. Its vtable `+0x18(side, mode,
box, position)` (`0x0221022C`) places the box (normalized) at the position and tests a line just past one edge: 1 up (one unit
above the top), 2 down (the bottom edge; mode 1 also counts the floor under the position, `0x0220FCF8`), 4 left and 8 right (one
unit beside the box; when the position is solid and the box starts at the feet, the bottom end is raised a unit). Mode 1: either
end of the line solid (`0x0220FDEC`); mode 2: both. `+0x38` and `+0x3C` (`0x0220DCEC`, `0x0220DBB4`) give how far the box's right
or left edge is into a wall (negative) or 0x10000 (none): Crimp shifts its probes by it. Tiles are 16 pixels; a tile's word has
its kind in bits 14-15 (`0x4000` solid).

### Inhaled, bitten and thrown

An enemy's hit reaction (body vtable `+0x30`) gets the attack's kind first: 3-5 are Kirby's inhale pulling it, and its touch
reaction (Maiga's `0x020A1F90`) gets kind 6 when it reaches the inhale's mouth (logged on the DS: Kirby holding B,
Maiga takes kind 3 seven frames, then touch kind 6); kind 0 is Kirby's body, which hurts both (Kirby lost 4 of 36 HP, Maiga was
beaten). Maiga overrides both: pulled, it slides at the inhaler at 2.0 a frame (`0x020A116C`, its own speed, not the engine's
pull); at the mouth it holds Kirby (`0x020A0D84`): it sets his position each frame (his vtable `+0x10`, 8 pixels ahead of it,
bobbing up 2 pixels when its count is 0 in 8, down when 2), counts 1 a frame and 4 more each frame one of his pad's fields
(`+0xE8`'s `+0x14` across, `+0x18` up and down, `+0x1C`, `+0x20` the buttons) changed, and at 120 sets him 3 pixels higher with
VX -0x299A, VY 0, AY -0x2B8 and `+0xEC` (his untouchable frames) 30; his own thrown state (`0x0207FBE9`) hurts him when he lands
(6 of 36 HP), then the usual knock-back (VX -0x1B34) and 120 untouchable frames. KatAM has the same thrown state: a hold table
entry with flag 2 (`sub_0804F32C`: thrown, tumbling, hurt on landing, 1 HP).

Cret (`0x02072E3C`) is not pulled at all: it blows back (animation 3), and still inhaled once that has played, its cloud comes off
(animations 4 and 5, then 20 frames) and it pops (`0x02070C54`): Squeak Squad's random 0-99 under 30 spawns enemy 5 (Gordo,
factory record ID 5), else an item (`0x0203B210`: 6 food healing 9999, the Maxim Tomato; 7 heals 24 of 36; 8, 12; 9, 6; 1 the
1-UP; logged on the DS with the roll forced at `0x02070CA8`).

Uja (`0x02083F2C`) grabs on touch kind 6, which is its own grabbing boxes (`uja.rca`'s kind 6, the glove's fingers) touching
Kirby, inhaling or not (on the DS it took a standing Kirby): it moves to his spot, closes on him (anims 13-15, him 4 pixels up),
smothers him (anim 25) 120 frames (5 fewer each frame his pad's fields change), then sets his VX -0x499A, AY -0x2B8 and `+0xEC`
120: his hurt takes 6 of 36 at once and knocks him back at 0x1B34 (the way Uja came: he faces as it). Inhaled (kinds 3-5) it's
pulled in and swallowed as a Waddle Dee is (Kirby's states the same: `0x0201A195`, `0x0201A099`, then standing).

Bio Spark (`0x0207CDD4`) grabs on touch kind 6 too: `biospark.rca`'s kind 6 boxes are in its jump (animation 2, its hands; that
animation has no kind 0 box, so the jump doesn't hurt by touch). Its izuna (`0x0207AF94` on) sets its flag 0x200 each frame and its
`+0x18` bit 2 (its touch harmless; the kunai's stuck state sets 2, 4 and 8): Kirby 8 pixels ahead of it, 8 up (`vtable +0x10` of the
held, offset turned with its facing, Kirby facing it), 12 frames; up at 8.0 for 32 frames or till it's 16 pixels over the camera's
top (the stage object's `+0x6C`/`+0x70`, `0x0220A3E4`: the screen's left and top, y up), Kirby level with it; then still 33 frames
(`0x0207AA34` doesn't move it); then down at 8.0 till it's under 4 pixels over where it grabbed him (`0x02023158`: its `+0x1A0`,
`+0x1A4`), where its flag 0x200 goes and the ground, or the terrain at its feet (its middle and its body box's sides), lands it.
Landing, it hits Kirby with kind 0x8001 (`vtable +0x84`, `0x02021BC8`: damage 6): Kirby's hit reaction (`0x0201917C`) takes that
kind into his state `vtable +0x100` with the damage kept at `+0xC0`, and he loses it only when he lands (logged: 32 to 26 HP 37
frames on); Bio Spark sets his speeds (back 0x2000, up 0x2CCD, falling 0x2B8) and `+0xEC` 120, and shakes the screen
(`0x02035FC4(…, 4, 12, 2)`: logged as the camera's top jumping 3 pixels for 12 frames). Its first brain state for variant 2 is a
veneer (`0x02079A80` jumps to `0x02079A78`, which only lets the pad go). Its pad reader (`0x0207A594`) takes B at once but the
rest only once the pad's been non-neutral 4 frames running; its brain leaves the direction pressed while it waits on the ground,
so a one-frame A still counts (logged on the DS). `0x020B48E0(obj, type)` shows an effect where a thrown thing ends: its table at
`0x0215908C` (types 0-5 `efc_cmn0`'s animations 3, 4, 7, 7, 17, 2; 6 `efc_cmn1`'s 1). `biospark.rca`'s animations are the
`.cea`'s one on (its first is an extra), found by their frame totals and confirmed on the DS by the jump's grab boxes.

Starman's brain (`0x02084370` on, before its constructor's range) presses only B; its body reads A at `+0x38` and B at `+0x20`
(the setter writes both B's, `+0x20` and `+0x3C`), so its A moves never run. Variant 2's rocket holds up and B 34 frames but its
lift-off animation lasts 38: its flapping starts with B let go, and it skids (logged on the DS). Its flying caps its height at the
camera's top (the stage object's `+0x70`); variant 0 passes through the terrain (`vtable +0x40`, `0x02084C20`).

Bubble Head's brain releases pad up (its "Kirby 16 pixels above" shot) 3 frames after B, but its body blows the bubble 5 frames
after B (the shot's composite 5, 4 frames, then 6), so its upward bubble (kind 2: 1.5 forward, 0.55 up) never comes out (logged on
the DS with Kirby held 40 pixels above). Its stream and bubbles hit with kind 8 (damage 8): Kirby's ordinary hurt (logged: 36 to 28
HP, knocked back at 1.7). A bubble (`0x0209702C`) touched by an attack with flag 0x400, as Kirby's body's, pops (`0x02096E68`): it
goes on as it was till its animation's next loop, then a star burst (`0x020B48E0`'s type 3); it ignores inhales (kinds 3-5).
Created during its blower's update, its first update runs twice in the next frame (its sprite's time 2 after it, 3.2 moved).
The bubble's pop sets its `+0x14` bit 1 and raises it 8 pixels (`0x02096BC8`), which the floor's snap takes back on the ground.
A body's `+0x14` flag 8 is set every frame from its sprite's update (`0x02014590`: `vtable +0x1C`, `0x01FFA8B8`, which steps the
animation and answers whether it looped this frame, ended or isn't playing): a looping animation's flag pulses on its loop
frames (logged: Bubble Head's bubble every 8 frames, Bio Spark's slash brain waiting one 6-frame loop each wait).

Gaw Gaw's brain keeps its body's pointer at `+0x98` and its timers at `+0x9C` and `+0xA0` (its params at `+0xB0`). Variant 1's
look (`0x020AC4E0`) tests Kirby "within" its params' 72.0 and 18.0, floats (`0x42900000`, `0x41900000`) read as whole numbers, so
always: its random walk (`0x020AC37C`) is never reached. A state function's return value is r0 as it leaves, so a "void" entry can
run the next state at once or not: Gaw Gaw's walk entry (`0x020AF7F0`) leaves 0 (the walk starts the next frame: logged), its wall
jump's (`0x020AF5B0`) 1. Its hold (`0x020AE118`...) puts Kirby 8 pixels ahead each frame through his `vtable +0x10`; its throw
gives him speeds (2.3 back, 2.1 up, falling 0x2B8) and he loses 6 HP as he lands (logged: 36 to 30).

Acchi's brain keeps its timers at `+0x98` and `+0x9C` (its params at `+0xAC`) and never sets them when it's made: the memory's
leftovers (logged: `0x6C630039`), so variant 0's first rest (`0x02069B3C`, which waits for `+0x98` to reach 60) ends at once; after
a low shot the rest isn't set back to 0 (59 frames), after a lob it is (60). Its fire (`0x0206BCA0`, a projectile like Bubble
Head's bubbles: its first update runs twice in the next frame, logged 4 pixels across) has its terrain box (the `.rca`'s tag
0x202: 3 pixels round its middle flying) and flies through the terrain (flags 0x200 and 0x20000): it lands when that box's bottom
meets the floor (its distance down, `+0x84`, under 1, or `+0x14` bit 0x4000, the floor side met), its top a ceiling (0x2000), or
its middle is in the terrain (the stage's `vtable +0x1C`, a 1-pixel box); landed, it chains to its burn the same frame (logged:
the timer 120 set, 119 after it). Its touch reaction (`0x0206BB38`) puts it out under an attack with flag 0x400 when its own box
has it too, and ignores inhales (kinds 3-5); Kirby's body never reached it (no reaction logged as it burned him). The `.rca`'s
animations 1-8 are the `.cea`'s 8-15 (its 9-15 the `.mca`'s composites). Swallowed, it gives Fire (logged: the HUD's "FIRE").

Bun's brain (params at `+0xA8`, timer at `+0x98`) presses B (`0x02054EE8`: `+0x20` and `+0x3C`) and runs with its pad forward and
`+0x44` (`0x02054EF8`). Its grab reaction (vtable `+0x2C`, `0x0205695C`, kind 6) fires 19 frames after the reach starts though the
reach's grab box (`bun.rca`'s, on the `.cea` animation's frames 18-20) is there from its 18th: touch reactions run in the next
frame's collision pass, before the updates, and the boxes' time lags the animation's by a frame (logged: reach at frame 75, the
reaction at 94). It holds Kirby through his `vtable +0x10` (the holder, an offset turned with it, a held pose: 3 carried, 4
overhead, 8, 11, 17 and 4 in the swing, from `0x02158660`; the swing's offsets from `0x02186A44`), and throws him with VX 0x499A,
AY -0x2B8 (he moves from his next update, logged 16 then 20.6 pixels ahead); he loses 6 HP as he lands. Swallowed, it gives Throw.

Metalun's brain constructor (`0x0205155C`) puts it in mode 3 (`0x0201B688(…, 3, -1)`, the brain's `+8`), so its body's other mode
(A jumps, B stops the charge) never comes up. Its charge (`0x02051A4C`, `0x020517F0`, anim 5 twice) counts its frames at `+0x1FC`
up to 0x9C and starts over: it lurches 2.0 on counts 0, 25, 51, 77, 103 and 129 and 8.0 on 5, 31, 57, 83, 109 and 135 (logged: 2
and 8 pixels), stomping (sound 0x14B) with `0x02035FC4(…, 3, 4, 2)`, a 4-frame shake, on the ground but on count 0. Its walk
(`0x02052230`) counts at `+0x1F8`: 4.0 on 29, 37, 69 and 75, the count started over after 75, the walk itself over at its 80-frame
animation's end. The DS stops updating it well off the screen (logged: still 150 frames with Kirby 190 pixels away). Swallowed, it
gives Metal.

Squeak Squad's Waddle Dee (body `0x0202A9D0`, brain `0x02029760`, `prmNE01Pl0.kgb`: 24, 48 pixels, 120 frames; `prmNE01Ml0` walks
at 0.25, `Ml1`, when the spawn's second byte is 1, at 0.5) has six brains: 0 walks, turning at walls (`0x02029694`); 1 patrols 24
pixels either side of where it started (`0x020295EC`, `0x02029540`, `0x02029508`); 2 jumps once when Kirby is within 48 pixels
across (`0x020294E8`, `0x020294B8`, then `0x020294B0`: A let go, walking on); 3 jumps 120 frames on (`0x02029484`, `0x0202944C`,
`0x0202941C`: again once landed); 4 and 5 start under a parasol (the body's first states `0x02029F24` and `0x02029DD0`: its
parasol, kind 0 or 1, from `0x020299C4`), then walk (`0x020293C8`) or stand (`0x020293F0`, `0x0202934C`: no direction). Its body
turns a frame after the brain asks (logged: a step more the old way), and its A crouches 32 frames before the leap (logged; 2.5
up, 0.1 gravity, 49 frames in the air). The parasol (`0x020741A8`, made by `0x0207426C` for the Waddle Dee, Shotzo `0x0205D6E4`
and Waddle Doo `0x0205EFDC`; its params `prmCE08Ml0`/`MlG`) follows its holder (`0x02073C40`), lets go when the holder is hurt or,
kind 0, lands (`0x02073980`), and flies off (`0x02073B50`, `0x02073A88`: through the terrain, gone off the screen); KatAM's own
parasol (object `0xA2`, `parasol.c`) does the same for its Waddle Dee, Waddle Doo and Shotzo (`shotzo.s`' `sub_0811B44C`), so it
stays KatAM's.

Squeak Squad's UFO (body `0x02050E94`, brain `0x02050248`; its body's parameters, `+0x228`: 2.0, 3.0 as a float, 8.0, 0.3, 0.1, 4.0, 16.0; its brain's, `+0xAC`: 30, 30, 96, 48, 156, 4, 64.0, 2.0, 60) hovers (`0x02050AD8`, `0x02050368`: 24 frames at 0.3 forward and 0.1 up, 24 back and down, a frame still and it turns round), and its brain 0 (`0x02050054`) waits 48, 96 or 156 frames (30, 30, 40 in 100: `0x0204FFC0`, `0x0204FF8C`) and presses A: the body (`0x02050794`, `0x02050698`) darts at 3.0 to one of 16 spots round Kirby (`0x021868A4`: 16, -38; 36, -32; 50, -24; 56, -8; and their mirrors, Y up), its next spot 3 to 9 on round the 16 (its RNG 0-9: under 6, that plus 3; else plus 6), its velocity aimed once (atan2, cos, sin), stopping within 8 pixels of it. After 4 darts (`0x0204FF2C`) it flies at 2.0 (8 ways, `0x020509D8`) to Kirby's height 64 pixels to his side (`0x0204FE38`, within 2), faces him and presses B (`0x0204FE20`): its shooting animation (`0x02050524`), a shot (`0x02069718`) 18 frames in, then 60 frames from the B (`0x0204FDE8`) flying forward at 2.0 once the shot is done; then darting again. Logged (Kirby pinned 80 pixels away). KatAM's UFO is the same scheme with its own numbers.

Squeak Squad's Gordo 4 (body state `0x0204CB84`, then `0x0204C9E0`) flies a circle: from 180 degrees, 3 a frame (`+0x200`, 20.12 degrees; 360 wraps to 0 and clears its velocity after the move), its velocity forward the negated cosine and up the negated sine (its sine table `0x0215CE20`, 4096 sine and cosine pairs, 1.0 = 0x1000), through the terrain: 1.0, a 19.1-pixel radius, 120 frames a lap (logged: x 140.39-178.58, y 120-158.19 from a spawn at 160, 120). Its wall check flips `+0x1FC`, which only picks the velocity's sign when 0 (set to 1, never 0): it never turns. Squeak Squad's spawn Y counts from the stage's bottom (240 - Y) and its Y is up: a test spawn under 80 is in the ground.

Squeak Squad's Glunk (body `0x0205B478`, brain `0x0205A51C`, `prmNE04Pl0.kgb` and `Pl1.kgb`, the spawn's second byte picking `Pl1`) has one brain: it presses B every P0 + 2 frames (`0x0205A2EC`, `0x0205A2D4`: 240, or 120), counting from its start. Its body shoots on B standing (`0x0205B1DC` on land, `0x0205A9B8` in water) or in the air (`0x0205AD70`, `0x0205ABA0`): its 12-frame shooting animation 5 times, a shot (`0x0205BAFC`) at the end of each (`0x0205A828`, `0x0205AF10`, its shots counted at `+0x1F8` against `Pl0`'s `+0x30`, checked first), standing again the frame after the fifth. Logged (Kirby pinned away): volleys 241 and 121 frames after the spawn, then 242 and 122 apart, 61 frames long, shots 12 frames in and 12 apart. Standing in water with its spot no longer water it hops (`0x0205ADDC`: its shooting pose, 2.5 up), falls (`0x0205AC58`, its sway) and stands where it lands (land or water: its vtable `+0x80`).

Big Metalun (mid-boss 0x34: body `0x0210C4F8`, `EnemyBaseCtor(…, 0x34)`, its hurt anims both 0x100000A; brain `0x0210A940`;
`prmMB10Ml0` 0x667, 0x2B9, 0x399A, 60, 180, -0x1000, 0x2667, 0x52; `Pl0` 50, 0x30000, 0x30000, 32, 50, 25, 50; `Sl0` HP 300;
`enemy/bigmetalum` and `bigmetalum_dmg`, the hurt flash's palette, loaded by `0x0210A508`). Its brain (one state a frame, Squeak
Squad's random 0-99 from the LCG at `0x01FF8120`): `0x0210A868` faces Kirby; `0x0210A7F8` rests 60, 120 or 180 frames (30, 50, 20
in 100; `0x0210A7D0` counts at `+0x98`); `0x0210A754` walks 1, 2 or 4 steps (30, 50, 20: its `else if < 80` never picks 3) and
`0x0210A708` waits for the body's steps (`+0x208`) to reach them and its step to end (`+0x204` 0), a wall ahead turning it round;
`0x0210A6F0`, `0x0210A6D0` press B (the charge) for up to 180 frames (`0x0210A688`, a wall ending it); `0x0210A650` waits for the
body to settle (`+0x204` 0), turns and faces Kirby; `0x0210A5D0` then charges again after 60 frames (30 in 100: `0x0210A5A8`) or
starts over. Its body's roll (`0x0210B0B4`) stops at the stage's limits, the camera object's (`*(*0x021867AC + 4)`, `0x02035BA4(0)`)
`+0x5C` and `+0x64` (logged: 16 and 1584 in a 1600-pixel stage, `+0x60` and `+0x68` 8 and 216 the top and bottom, all four fixed as
the camera moves), at a wall (flag 0x800) or after 1000 frames (`+0x20C`), the bonk setting the daze's 180 (`Ml0`'s fifth value);
logged, the walk's 284 frames for two steps, the bonk at x 16, the daze 180 and the stand-up 7. KatAM's port stops it where KatAM
keeps its mini-bosses instead (`ObjXSomething`: its box 8 pixels inside the mini-boss's `unkA4`/`unkA8`, the camera lock's area or
else the room's limits).

Gao Gao (mid-boss 0x33: body `0x02100300`, vtable `0x0217FF88`, its hurt anims both 0x1000009; brain `0x020FE364`, vtable
`0x0217FF6C`, its states `0x020FDD74`-`0x020FE274`; `prmMB09Ml0` 0x1334 (the jump across), 0x800, 0x2800 (the jump up), 0x19A
(gravity), 0x1CCD and 0x800 (the hop back), 0x3CCD, 0x2800, 2, 0x399A (the fall's cap), 0x1000, 20, 0x1000, 0x2000, -0x1000 and
0x2667 (the defeat's hop), 82; `Pl0` 50, 0x30000, 0x30000, 32, 50, 25, 50; `Sl0` HP 140; `enemy/gaogao`, `gaogao_dmg` loaded by
`0x020FDCC4`). Its brain presses A (brain `+0x38`/`+0x1C`: `0x020FDD6C` sets, `0x020FDD64` clears) and B (`+0x3C`/`+0x20`:
`0x020FDD5C`, `0x020FDD54`); its body (`0x020FF940`) reads B first (with up the drill, `0x020FEDB8`; else the spin, `0x020FF160`), then
A (with down the hop back, `0x020FF534`; else the jump, `0x020FF88C`). Its counters `+0x9C`, `+0xA0`, `+0xA4` (jumps in place,
forward, back) are never set before use (logged: a fresh one attacked after its first jump). The drill (`0x020FEB14`) waits till its
feet reach the stage object's `+0x70` (`*0x02229F1C`, `0x0220A3E4`: the camera's top; `+0x6C` its left), then lands over Kirby or
the camera's left plus 54, 128 or 202. Its stars and puffs: `0x02021864(obj, big, stars)` puffs (`0x020219C4`'s type 2 if big, else 1)
and with `stars` makes two stars, `0x0212389C` kinds 1 and 2 at 32 pixels either side; `0x0202181C` the jump's dust (type 3 or 4).
`0x020219C4`'s types name animations through `0x02157DF8` (1: efc_cmn0's 23, 2: 22, 3: 19, 4: 20; 15: efc_cmn1's 5). The star
(`0x021236A0`, vtable `0x02183310`, `effect/efc_cmn0`): kinds 1 and 3 fly 3.0 forward and up, 2 and 4 back and up (`0x0212344C`,
`0x02123358`) through its animation 11, hang 100 frames in 12 (`0x02123268`, `0x02123218`), fade in 13 (`0x02123184`) and are gone;
other kinds fly at 4.0 (5) or do `0x0212312C` (6, the defeat's slide dust).

Tedhaun (mid-boss 0x35: body `0x02103B4C`, vtable `0x0218042C`; brain `0x02100FE8`, vtable `0x02180410`; `prmMB11Ml0` (63 values:
its flight, bob, circle, chase and carry speeds at `+0x20`-`+0x68` and `+0xEC`-`+0xF8`, its ghosts' at `+0x70`-`+0xE4`), `Sl0` HP 140).
Its brain presses A (`0x021009E8`) and B (`0x021009D8`); its body (`0x02103114`, `0x021013FC`) reads B (its call, `0x021029CC`), A
with up or down (circling, `0x02102E94`), A alone (chasing, `0x02102C0C`), its pad (flying, `0x02103070`, `0x02101490`). The circle
(`0x02101FB8`, `0x02101E20`, `0x02101CC8`, `0x02101B28`) steers two speeds (`+0x220`, `+0x224`) round; its float bobs
(`0x02101170`). Its three ghost spots (`+0x254`, `+0x258`, `+0x25C`, 12 bytes each) refill 20 frames after their ghost is gone (the
ghost's destructor frees its spot). Its ghosts (`0x02104F74`, `0x02104E34`, kinds 0 and 1, its own graphics' animation 9): see
`squeak/tedhaun.c`. Its brain's B (`0x02100BC4`, `0x02100B44`) waits for its ghosts to be ready before the call but not after: its body's
wind-up (28 frames) leaves them ready, so it picks again 3 frames after B.

Buboo (mid-boss 0x36: body `0x02108EAC`, vtable `0x02180A78`; brain `0x02105938`, start `0x02105830`, runner `0x0210586C`; `prmMB12Ml0`
(57 values: its flights at `+0x30`-`+0x3C`, the weave's `+0x48`-`+0x54`, the bob's `+0x5C`/`+0x64`, its spots `+0x40`-`+0x7C`, its
bubbles' `+0xAC`-`+0xD0`, the crab's `+0xD8`-`+0xE0`), `Pl0` (19), `Sl0` HP 140). Its brain presses B (`0x02105224`: `+0x3C` and
`+0x20`) and A (`0x02105234`: `+0x38` and `+0x1C`) and lets go of all while the crab is out (the body's `+0x208` 0: `0x0210523C`). Its
body: `+0x204` busy, `+0x208` the cluster, `+0x20C` its hits (5 burst it: `0x021088D4`), `+0x1F8` 36 frames safe, `+0x23C` and `+0x240`
the bob's base speed and frames, `+0x1A0`/`+0x1A4` its spot. The circle (`0x02107080`) moves along an angle (`+0x230`, 20.12 degrees)
turned `20480 - 2048 * k` a frame (`k`, `+0x22C`, one more each half turn: float maths on `0x0200A1E0`, `0x0200BCB0`, `0x0200C19C`,
`0x0200B970`, `0x0200BBE4`). Its bubbles (`0x02109E00`, vtable `0x02180CE4`; kinds 0 little, 1 the crab's, 2 big, 3 the defeat's) are
thrown things (`0x020B49D8`), not enemies; the big one holds an enemy made by `0x0202467C` with the template's type 0 (Waddle Dee) at
its `+0x134`. `0x0201B76C(list, kind, n)` counts the stage's objects of a kind. The sine table at `0x0215CE20` is 4096 (sine, cosine)
pairs in 4.12, indexed by the 16-bit angle (degrees in 20.12 times 65536/360, rounded, `>> 12`) `>> 4`; KatAM's `gSineTable` (1024 a
turn, 2.14) stands in.

### The stage's water splashes

The stage's map object (`*0x02229F1C + 0x48`, vtable `+0xB4`, `0x02215A10`) splashes: type 0 into the water, 1 out of it. Only at
the surface (`0x02215860`: the character's tile water and one beside, above or below it not, or the other way round); placed
6 pixels into the top water tile (`0x02215954`). The splash is the stage's material file's (`m_efc/clay`, `sand` or `snow`, named at
`*0x0222C104`) animation 4 (in) or 5 (out), the same in all three; at most one every 6 frames (`0x0221BCA0`).
- Waddle Dee: 10 cells of 12 tiles (120 tiles, the `.cga`'s 0xF00 bytes); animation 1 walks (cells 0-4-1, 11 or 8 frames each,
  looping), animation 2 the same 4 frames each.

### Parameters (`param/`)

`.kgb` files (`KGB`, `0x7B`, the number of values, `0`, then `u32` values), per character: `Ml0` movement (Waddle Dee's: walk speed
0x400, ...), `MlG` the gold version's (Gold Waddle Dee, walk speed 0x1334), `Ml1`/`Ml2` other levels, `Sl0` (Waddle Dee's: 10, its
HP?), `Pl0`. Names: `NE` regular enemies, `CE` copy-ability enemies, `MB` mid-bosses, `BB` bosses, `LB` the last boss,
`DB` Daroach's gang (Doc, Storo), `DE` the Squeakers. `PrmE_CMN_01.kgb`: shared enemy values.

## Enemy factory

`0x0202467C` creates an enemy from a spawn record: `r0` points at { context, enemy ID, three parameter bytes, x, y (`fx32`) }. It
switches on the ID (0x00-0x47): each case allocates the enemy's object (its own size and constructor), loads its graphics
(`0x020750F4`, "enemy/name") and creates its sprite object. The IDs:

| ID | Name in files | Enemy |
| --- | --- | --- |
| 0x00 | waddledee | Waddle Dee |
| 0x01 | brontoburt | Bronto Burt |
| 0x02 | blipper | Blipper |
| 0x03 | glunk | Glunk |
| 0x04 | scarfy | Scarfy |
| 0x05 | gordo | Gordo |
| 0x06 | builgue | Snooter |
| 0x07 | chip | Chip |
| 0x08 | batty | Batty |
| 0x09 | blockn | Blockin |
| 0x0A | bigwaddledee | Big Waddle Dee |
| 0x0B | gussa | Gussa |
| 0x0C | climb | Crimp |
| 0x0D | perara | Perara |
| 0x0E | gwaddledee | Gold Waddle Dee |
| 0x0F | maiga | Maiga |
| 0x10 | cret | Cret |
| 0x11 | uja | Uja |
| 0x13 | shutzo | Shotzo |
| 0x14 | waddledoo | Waddle Doo |
| 0x15 | hothead | Hot Head |
| 0x16 | flamer | Flamer |
| 0x17 | pengy | Pengy |
| 0x18 | sparky | Sparky |
| 0x19 | twister | Twister |
| 0x1A | swordknight | Sword Knight |
| 0x1C | annie | Cupie |
| 0x1D | sirkibble | Sir Kibble |
| 0x1E | laserball | Laser Ball |
| 0x1F | fally | Foley |
| 0x20 | willy | Wheelie |
| 0x21 | ufo | UFO |
| 0x22 | noddy | Noddy |
| 0x23 | dissy | Boxin |
| 0x24 | heavyknight | Heavy Knight |
| 0x25 | biospark | Bio Spark |
| 0x26 | starman | Starman |
| 0x27 | bubblehead | Bubble Head |
| 0x28 | gawgaw | Gaw Gaw |
| 0x2A | acchi | Acchi |
| 0x2B | bun | Bun |
| 0x2C | metalum | Metalun |
| 0x2D | frosty | Mr. Frosty (mid-boss; KatAM has it) |
| 0x2E | bonkers | Bonkers (mid-boss; KatAM has it) |
| 0x2F, 0x32 | | (the default case: a Waddle Dee) |
| 0x30 | vowfighter | Box Boxer (mid-boss; KatAM has it) |
| 0x31 | boxy | Boxy (mid-boss; KatAM has it) |
| 0x33 | gaogao | Gao Gao (mid-boss) |
| 0x34 | bigmetalum | Big Metalun (mid-boss) |
| 0x35 | tedhaun | Tedhaun (mid-boss) |
| 0x36 | buboo | Buboo (mid-boss) |
| 0x37 | metaknight_cmn | Meta Knight |
| 0x38 | dedede | King Dedede |
| 0x39 | | (Mecha-Kracko?) |
| 0x3A | mrsMoley_cmn | Mrs. Moley |
| 0x3B | yadgaine | Yadgaine |
| 0x3C | bohboh | Bohboh |
| 0x3D | dancho00, dancho01 | Daroach (his boss fight: `dan_dmg`, `prmBB07Ml0`/`Sl0`, its graphics loaded near `0x021270F0`) |
| 0x3E, 0x3F, 0x46 | darkZERO_cmn | Dark Nebula / Dark Daroach (to be matched) |

0x12, 0x1B, 0x29, 0x2F, 0x32, 0x40-0x45 and 0x47 fall to the default case. Parasol (the parasol some Waddle Dees, Waddle Doos and
Shotzos hold) and the Squeaks (spin, doc, stron, chun) are created elsewhere.

## The cast to port

Squeak Squad's enemies KatAM has (its own objects, with only the Squeak Squad behaviors it lacks to add): Waddle Dee, Bronto Burt,
Blipper, Glunk, Scarfy, Gordo, Snooter, Chip, Batty, Blockin, Big Waddle Dee, Shotzo, Waddle Doo, Hot Head, Flamer, Pengy, Sparky,
Twister, Sword Knight, Cupie, Sir Kibble, Laser Ball, Foley, Wheelie, UFO, Noddy, Boxin, Heavy Knight; mid-bosses Mr. Frosty,
Bonkers, Box Boxer, Boxy (Squeak Squad's 0x2D Mr. Frosty 0x020F4294, 0x2E Bonkers 0x020D4C10, 0x30 Box Boxer 0x020FD418, 0x31 Boxy
0x020F8B78; one brain each, their fight started by body `+0x218` (Mr. Frosty) or `+0x210` (the others): logged, their moves are KatAM's own mid-bosses' (Mr. Frosty's hops, slide and leap; Bonkers' back-step, swing, dash, leap and hops; Box Boxer's back-step, punch dash at 5.6 and hops; Boxy's leaps and slides at 2.5), so they stay KatAM's; each mid-boss's files: `param/prmMBnnMl0`, `Pl0`, `Sl0` (HP: Big Metalun's 300), `enemy/<name>` with a `_dmg.cla`
palette for its hurt flash). The others get a full port (body and brain constructors, from the factory; sizes in bytes):

| ID | Enemy | Body | Brain | Ported |
| --- | --- | --- | --- | --- |
| 0x0B | Gussa | 0x0206E364 (0x250) | 0x0206C3D0 (0xC0) | yes |
| 0x0C | Crimp | 0x020CE310 (0x248) | 0x020CC53C (0xBC) | yes |
| 0x0D | Perara | 0x0206FD54 (0x260) | 0x0206E85C (0xB8) | yes |
| 0x0E | Gold Waddle Dee | 0x020B6764 (0x250) | 0x020B4EF4 (0xC0) | yes |
| 0x0F | Maiga | 0x020A2108 (0x26C) | 0x020A0B58 (0xB4) | yes |
| 0x10 | Cret | 0x02073044 (0x284) | 0x020705AC (0xC8) | yes |
| 0x11 | Uja | 0x020840C8 (0x294) | 0x02080AD0 (0xC4) | yes |
| 0x25 | Bio Spark | 0x0207CF18 (0x264) | 0x0207A284 (0xB4) | yes |
| 0x26 | Starman | 0x020877B8 (0x24C) | 0x02084B10 (0xD0) | yes |
| 0x27 | Bubble Head | 0x020968D0 (0x244) | 0x02095980 (0xB8) | yes |
| 0x28 | Gaw Gaw | 0x020B0068 (0x260) | 0x020AC780 (0xBC) | yes |
| 0x2A | Acchi | 0x0206B1F0 (0x25C) | 0x02069D6C (0xB8) | yes |
| 0x2B | Bun | 0x02056AA4 (0x250) | 0x02055470 (0xB4) | yes |
| 0x2C | Metalun | 0x020526AC (0x248) | 0x0205155C (0xB8) | yes |
| 0x33 | Gao Gao (mid-boss) | 0x02100300 (0x254) | 0x020FE364 (0xD8) | yes |
| 0x34 | Big Metalun (mid-boss) | 0x0210C4F8 (0x24C) | 0x0210A940 (0xBC) | yes |
| 0x35 | Tedhaun (mid-boss) | 0x02103B4C (0x2AC) | 0x02100FE8 (0xCC) | yes |
| 0x36 | Buboo (mid-boss) | 0x02108EAC (0x274) | 0x02105938 (0xC8) | yes |
| 0x37-0x46 | the bosses | Meta Knight 0x02116C7C, Dedede 0x020E7E58, 0x39 0x02132D7C, Mrs. Moley 0x0212CF40, Yadgaine 0x0211A4E8, Bohboh 0x02111370, 0x3D 0x02126F48, Dark Nebula / Dark Daroach 0x0211B680, 0x02122CD0, 0x021376EC | | |

**Squeakers** (enemy 0x40, ported: `squeak/squeakers.c`): the stage's spawn list makes them (its body `0x020EC504`, base
`0x020A4F54(..., 0x40)`, vtable `0x0217DB64`; its spawn byte 8 the color, 0-3; brain `0x020E9444`, its states `0x020E8E7C`-`0x020E9378`;
body states `0x020E9528`-`0x020EC4BC`; params `prmDE01Pl0/Ml0/Sl0`). Its brain's target (`0x020E8F9C`): the stage's chests (the
category 0x2000 registry: `0x020CF348`, the best by `0x020CF320`'s score), with none or one in its hands its escape hole (the
stage's route manager `0x02209560`: `0x0220997C` the hole's node, `0x02209C7C` a spot's part of the stage, `0x02209BFC` the next node
on the way; its stage mode 2, `0x020A2BF8`: Kirby carries the stage's chest, all chase him). Its touch handler `0x020EC168` takes a
chest (touch kind 0x1001: `0x020A2BB0` holds it, `+0x16C`, `+0x1F4`, `+0x1EC` its carrying animations), its hits drop it
(`0x020A2A48`: the chest let go where it is, 40 frames before it's taken again) and maybe a treat (`0x020E9528`: 20%, items 9-14 of
`0x0203B0B8`, the foods, healing 4). The chest is carried at `vtable +0x78` (`0x020E9640`: an offset behind it plus a sway) through the
held thing's `+0x10` (`0x02012FB0`: holder's spot plus the offset turned with it). At its hole (`0x020EAF8C`, `0x020EAE0C`) it walks
there a pixel a frame, waits, and goes in (`0x020EAD78`, `0x020EACF0`: `0x020A2968` hands the chest to the registry). Its bombs:
`0x020ED308` makes them (`0x020ED208`, kind 0 small, 1 big; drawn with `enemy/chun`'s animations 33, 39-41), launched by
`0x020ECF88`/`0x020ECC60` (3.4 at 30 degrees, its random adjustments added up as the code does), the small one blasting where it
lands (`0x020ECE44`, `0x020ECE14`), the big one lit 192 frames then blasting 9 times round its spot (`0x020EC9F4`, `0x020EC7DC`).
Flag 0x200000 (`0x020EA2EC`-`0x020EAC30`): the route's tunnels, with their own animations (18-23).

**Spinni** (enemy 0x42, ported: `squeak/spinni.c`): its body `0x020AB6D0` (vtable `0x02174DF8`), brain `0x020A5BB0` (states
`0x020A5B2C`, `0x020A5AC4`, `0x020A5850` the think, `0x020A57B4`/`0x020A5780` the attack press; `0x020A54F4` the pad toward a target:
across beyond 8 pixels, or always on a floor or under a ceiling; a higher target: up, then 4 frames on A, a jump when it's higher
than it's away, else down and A, a long hop; a wall: up and A to a higher target, down to a lower, nothing to a level one; Kirby
within 64 by 32 without a chest: still). Params `prmDB01Ml0` (its words 3 run 1.4, 5 jump 4.0, 6 gravity, 7 fall cap, 8 and 9 the
take and hole pauses, 15 the pace period, 18/19/22/23 the paces, 25-33 its things), `Pl0` (the sight, 64 by 33), `Sl0` (HP 60). The
body switches sprites through `+0xE4` (0 `spin`, 1 `spin_at`). Its pace (`0x020A9E14`, `0x020A9EC0`) uses `0x02209D00`, the
camera's view (relative to the camera's middle, ±128 by ±96 on the DS, plus the margins of the table at `0x022275F4`: mode 0 48
pixels either side, 16 above, 48 below; mode 4 none), also its shuriken's end (mode 0). Its hit handler `0x020AAEA0`: kinds 3-5
(inhale) brace it (`0x020A65F4`, `0x020A6484`), a chest touch takes it (`0x020A9984`: all speeds zeroed, 48 frames still), else
damage, the phase moving at 90% HP (knockback `0x020A6C20`-`0x020A6910`, dizzy `0x020A6794`) and at 0. Its things: `0x020ABE08`,
made by `0x020ABF10(kind, body, offset)`: 0/1 shurikens (`0x020AB970`, flag 0x200), 2 the slash (`0x020ABA60`), 3 its claws
knocked off (`0x020ABCD0`, `0x020ABBD0`, `0x020ABB1C`, bursting at `0x020AB908`). Its rage (`0x020A9C68`, `0x020A9A74`) waits on the
stage's mode 2 (Kirby carrying the stage's chest), with `spin_col_ang`'s palette.

**Doc** (enemy 0x43, ported: `squeak/doc.c`): its body `0x020DD82C` (vtable `0x0217C874`; per frame `0x020DD674`), its routine
from `0x020D8F14` (`0x020D8934` chest or Kirby, `0x020D84F4` the attack's path, `0x020D8418` its pick: 5/40/35/20), its flight
`0x020DAC0C`/`0x020DA91C` (a quadratic Bezier, the control point beside the middle by 0.6 of the other axis' distance, its time the
distance over its pace's speed) and hover `0x020DA774`/`0x020DA590` (one circle 68 frames, two 116: the first quarter, 1.6 out
slowing by 0.05, drifts it about 21 pixels each way; the DS recording's (+20, +21.5) after a hover). Its spots: beside Kirby
`0x020D78F8` (odds by where Doc is from him) and the screen's corners `0x020D76EC` (odds by its last spot). Params `prmDB02Ml0`
(+0x2F0: its things' spots, gravity, the knockout's speeds, waits, paces), `Sl0` (HP 120). The body switches sprites through
`+0xE4` (0 `doc`, 1 `doc_at1`); `doc_at1.mca`'s compositions 0-3 are the claw (with the bomb), the two gun shots (ball and
rings) and the cannon's smoke. Its hit handler `0x020DD0AC`: dizzy (`+0x240`) a treat roll only; `+0x1C0` 2 (from the knockout
`0x020DC5F0` till the saucer's called, `0x020DB4F4`) ignores hits; a chest touch takes it; kinds 3-5 brace it (`0x020D7114`);
else damage, a knockout at 90% HP (`0x020DC5F0`-`0x020DB4C4`), else a flinch (`0x020DCDE8`). Its things: `0x020DF354`, made by
`0x020DF530(kind, doc, offset)` (vtable `0x0217CBCC`): 0/1 bolts (13.0, at 0 or 325 degrees), 2 the bomb (falls to Kirby's
height, bursts), 3 the missile, 4/5 the lid, 6 the empty saucer, 7 a burst. Their hit handler `0x020B4964` (the attack things'
base, Spinni's too) takes no grab and passes the rest to `0x02013050`: they go through Kirby. Its DS palettes: `doc` 0 and
`doc_at1` 0 (its body), `doc` 1 and `doc_at1` 1 (the saucer, the same colors), `doc_at1` 2 (the gun's rings, the same as the
common effects'), 3 (the arm, ball, bolts and smoke), 4 (the bomb): 57 colors, 5 KatAM palettes at least.

**Storo** (enemy 0x44, ported: `squeak/storo.c`): its body `0x020E43C0` (vtable `0x0217CF88`: +0x3C start `0x020E1DF0`, +0x30 hit
`0x020E3BA8`, +0x24 `0x020E3898`, +0x18 per frame `0x020E4220`), no pad brain: its states move it. Its start: anger pending
(`+0x254` 1) its palette fade; else by the stage's Squeak chase mode (`0x0220A350`): off, home; on, its chest in hands home, else
Kirby (within 64 pixels, or no chest) or the chest (`0x020E1D24`). A leg: `0x020E14F4` (20 frames standing), `0x020E0F00` (walking at
its pace, `0x020E253C`; turning once in 120 frames; more vertical than across 120 frames, the jump `0x020E0DAC`; Kirby within 64 by
64, 50% mallet `0x020E0380`, else body slam `0x020E0AB8`; home within 32, into its hole `0x020E1B30`). The mallet's star and the
slam's: `0x0212389C` (kinds 1 ahead, 2 behind) through `0x020A25A0`/`0x020A25C4`. Params `prmDB03Ml0`/`Ml1` (30 words: +8 the
knockout's HP percent 20/30, +0xC walk, +0x10..+0x18 the jump, +0x1C/+0x20 the attacks' waits, +0x24..+0x44 the slam and bounce,
+0x48/+0x4C the take and hole pauses, +0x54..+0x74 the pace). Its hit handler: dizzy or HP 0 a treat roll; `+0x1C0` 2 (from a flinch
or knockout till it lands) ignores hits; kinds 3-5 curl it (`0x020E004C`, `0x020DFE68`); a chest touch takes it unless its mallet's
out (`+0x23C` 7); damage with its element (fire, ice, spark poses and sounds), the knockout `0x020E2FF0`-`0x020E270C` at 20% HP,
else the flinch `0x020E35B4`-`0x020E32C8`. Its sprites: `stron`, `stron_atack` (its mallet), `stron_dmg` (hurt poses, `.mca` the
ice), palette 0 the same in all three. On the DS, Kirby held inside it doesn't lower its HP.

**King Dedede** (boss 0x38, ported: `squeak/king_dedede.c`): the factory's case makes him (`0x020258A8`: 0x22C bytes, `enemy/dedede`, a null brain `0x0203A9C4(0, 0)`), his body `0x020E7E58` (vtable `0x0217D6A4`: +0x3C start `0x020E7758`, +0x34 per frame `0x020E7F80`, +0x2C his catch `0x020E7C5C`, +0x30 his hit `0x020E79B0`), params `prmBB02Ml0` (59 words: +4 HP 200, +8 the intro's 90 frames, +0xC the 60 safe frames after a hit, +0x14 walk 0.7, +0x18 idle 60, +0x20..+0x3C the float, +0x50..+0x64 the catch and spit, +0x6C..+0x94 the hammer, +0xAC..+0xC4 the jump, +0xCC..+0xE8 the picks). His states `0x020E52DC`-`0x020E7758`: the pick `0x020E749C` (30 frames turned to Kirby, a roll of 0-99 rolled again till something is picked), walk `0x020E72D4`, jump `0x020E70E8`, fall `0x020E6D10`, hammer walk `0x020E6AF8` (its impact `0x020217F8`: `0x0212389C`'s star kind 1, 48 pixels ahead), hammer jump `0x020E680C`, float `0x020E6474` (its puff `0x0212EAA0`: Dedede's animation 20, no box), inhale `0x020E5D30` (the catch pulls the object to his mouth, `0x020E598C`; the spit `0x0212389C` kind 0: a star at 4.0 carrying it, `0x021234F0`, which lets it out through `0x020E56EC` past the area), summon `0x020E5EB0` (a Waddle Dee, spawn byte 8 4, at Kirby's x and the area's top, while `0x0201B76C(stage, 2, 2)` counts fewer than 5), hurt `0x020E5654` (20 frames), defeat `0x020E5530`-`0x020E52DC`. Landings: `0x02021864(obj, big, stars)`, big puffs and two stars 32 pixels either side. `dedede.mca`: composition 0 (his anim 22 and six sparkle streams, 84 frames), 1 (anim 32 and five bursts, 108 frames), the body listed last. His palettes 0 and 1 together have 27 colors (palette 1 a layer here). The star kinds (`0x021236A0`): 0 the spit (`0x021235BC`: efc_cmn0's animation 5, 4.0, flags 0x100 and 0x20000), 1 and 3 ahead, 2 and 4 behind, 6 another.

**Daroach in a stage** (enemy 0x45, ported: `squeak/daroach.c`): made by `0x020A51A8` (with `enemy/demo_dancho`, `0x020750F4`), his body `0x02134DA8` (`0x020A4F54(..., 0x45)`, vtable `0x02184B14`: +0x3C start `0x02134C74`, +0x34 per frame `0x02134ECC`, its hit handlers +0x2C/+0x30 `0x02134D38`/`0x02134D34` return 0; `0x0213482C` zeroes its speeds), its spawn byte 9 its facing (0: flag 4, left), flags 0x100 (the terrain not checked by `0x0201355C`), 0x20 and 0x20000 (not held in the stage). Params `prmDB04Ml0`: 60, 20, 60, -2.0, 2.0, 0, 0. A scene: `0x02134C74` takes Kirby's controls (`0x02012F94`: a handle on `0x01FF8180`'s list, released by `0x02012F68`; not in the stage's mode 3, `0x02209560`'s +0x75), hovering 60 frames (animation 1); `0x02134B94`/`0x02134ADC` its cape (2, through); `0x02134A30` held 20 frames (3); `0x021349E8` pointing (4, through); `0x021348F8` the controls back (mode 3: `0x0220A2A4(..., 0)`), flying off at (-2.0, 2.0) (5) and `0x0213488C` gone after 60 frames or out of the view (`0x02209D00`'s margins 1), its spawn record set to state 4 (`0x0220A2B4(..., +0x1C4, 2)`, `0x02218EF4`: not again till the area reloads). `demo_dancho.rca` has no boxes. Recorded on the DS: Kirby frozen 92 frames, then the flight. `0x02209D00`'s margins (`0x022275F4`, above, below, left, right in pixels): 0: 16, 48, 48, 48; 1: 24, 56, 56, 56; 2: 96, 104, 128, 128; 3: 192, 56, 56, 56; 4: none; 5: 24, 1024, 56, 56.

The Squeaks (Squeakers, Doc, Spinni, Storo, Daroach) and Parasol are created elsewhere. Their files: Squeakers `enemy/chun` (and `chun_blue`, `chun_green`, `chun_yellow` palettes; `prmDE01`, loaded at `0x020EC5F4`: its body near `0x020EC5xx`; its 3 palettes loaded by `0x020E8C80`; a thrown Squeaker, on the thrown-things base `0x020B49D8`, `0x020ED208`), Spinni `enemy/spin`, `spin_at` (`prmDB01`, `0x020AB764`, `0x020ABFC4`), Doc `enemy/doc`, `doc_at1` (`prmDB02`, `0x020DD8FC`, `0x020DF5E4`), Storo `enemy/stron`, `stron_atack` (`prmDB03`, `0x020E44A4`), Daroach `enemy/demo_dancho` in a stage (`prmDB04`, above) and `dancho00`, `dancho01` in his boss fight (`prmBB07`, enemy 0x3D). Copy abilities KatAM lacks (Ninja, Bubble,
Animal, Ghost, Metal): the owner decides; meanwhile those enemies give none.

Squeak Squad's brains for the enemies KatAM has (their body and brain constructors from the factory; each brain's first state per
variant, the spawn's first byte), against KatAM's own variants: a brain whose behavior KatAM's enemy has is left to KatAM's (as
NiDL's natives were); one it lacks is ported onto KatAM's own object (`ROM Expansion/squeak/natives.c`). Compared by logging both
(the DS with Kirby pinned away, `kss/emu/enelog.py`; KatAM in room 0x323, `sq/cmpnat.sh`):

| ID | Enemy | Body | Brain | Brains (first states) | Against KatAM's |
| --- | --- | --- | --- | --- | --- |
| 0x00 | Waddle Dee | 0x0202A9D0 | 0x02029760 | 0 0x02029694, 1 0x020295EC, 2 0x020294E8, 3 0x02029484, 4 0x020293C8, 5 0x020293F0 | 0, 4, 5 KatAM's 0, 3, 4; 1-3 ported (KatAM's 5-7) |
| 0x01 | Bronto Burt | 0x02029060 | 0x0202637C | 0 0x02026298 (a wave, 53 pixels up, 144 frames), 1 0x0202625C (a smaller wave), 2 0x0202622C (straight, to 1.0), 3 0x020261E4 (perched till Kirby is near), 4 0x02026110 (hidden till Kirby passes, then from the screen's top 48 pixels ahead of him: `0x02028BB4`) | all KatAM's: 0 and 1 its waves (0, 1), 2 its straight flight (4), 3 its perch (5), 4 its drop from the screen's top (2) |
| 0x02 | Blipper | 0x0208EBCC | 0x0208C2A8 | 0 and 5 0x0208C138 (steers at Kirby), 1 0x0208C10C (holds), 2 0x0208C074 (up and down, turning at walls), 3 0x0208BF94 and 4 0x0208BE78 (close in, then leap at Kirby) | KatAM's own five (`blipper.c`: its 0-4 split the same way); compared by their code, not logged (no DS save state in water) |
| 0x03 | Glunk | 0x0205B478 | 0x0205A51C | one (B every `Pl0`'s 240 + 2 frames, `Pl1`'s 120 + 2 when the spawn's second byte is 1) | KatAM's 0 and 1 are other beats (a pose every 3 sway loops; 5 shots every 190 frames); ported (KatAM's 5 and 6), with its hop out of water |
| 0x04 | Scarfy | 0x020490D4 | 0x02047974 | one (0x0204784C waits facing Kirby; inhaled, body `+0x204`, 0x02047740 holds B and steers at him, A past its time) | KatAM's own: inhaled it turns into its cyclops, backs off and dives at Kirby, bursting on him (logged on the DS: 1.0 back 30 frames, then 1.2 at him) |
| 0x05 | Gordo | 0x0204D2F4 | 0x0204C614 | 0 0x0204C42C (bob), 1 0x0204C2E8 (across, at Kirby), 2 0x0204C164 (up and down), 3 0x0204BFBC (up and down, 64-frame timer), 4 0x0204BF00 (circle) | 0-3 KatAM's own (0, 2, 1, 3); 4 ported (KatAM's 5) |
| 0x06 | Snooter | 0x02054C5C | 0x02052D98 | 0 (and any but 1) 0x02052C60 (looks round 2-3 times, 67 frames apart, walks 120 at 0.25), 1 0x02052ABC (faces Kirby, stands); a hit (body `+0x1F8`): 0x020529F8, 0x020529A4 (walks 300 frames, then faces Kirby) | 0 KatAM's own (look round, crawl, swallow, spit; its rush after a hit); 1 ported (KatAM's 5: stands) |
| 0x07 | Chip | 0x020333F0 | 0x02031CBC | 0 (and any but 1, 5) 0x02031BF0 (random hops in place or forward, small or big, facing Kirby), 1 and 5 0x02031A6C (the same, some backward) | KatAM's own: its hoppers face Kirby and hop at random (its 1 turning round at random too); logged on the DS: 0.5 across, 0.95 or 2.05 up, about 117 frames apart |
| 0x08 | Batty | 0x0204BB60 | 0x0204AF2C | 1 0x0204A984, others 0x0204AD34: wait (a flutter) till Kirby is near, chase him (`+0xB0`'s frames), fly back home and wait (1: 30 frames at home first) | KatAM's own: it wakes when Kirby is near, chases him and flies home (logged: the flutter, the dive at Kirby) |
| 0x09 | Blockin | 0x0204A414 | 0x020495F4 | one (0x0204951C, 0x02049420: a block till woken, body `+0x1FC`, then steering at Kirby; past its time, A and B) | KatAM's own: a block that wakes (stood on or inhaled) and flies at Kirby (logged: still with Kirby 30 pixels away) |
| 0x0A | Big Waddle Dee | 0x020358F4 | 0x02035048 | one (0x02034F7C: walks, 0.25) | KatAM's own walk (logged) |
| 0x13 | Shotzo | 0x0205E680 | 0x0205D15C | 0 and 6 0x0205CC10 (turns its barrel to Kirby, five directions), 1-5 0x0205CA90 (fixed barrel, a shot on a timer; the parasol ones) | KatAM's own: fixed and aiming Shotzos; its parasol is KatAM's |
| 0x14 | Waddle Doo | 0x020600CC | 0x0205EDF8 | 0, 5, 6 0x0205ED20 and 3 0x0205EB20 (walks 120 frames, then a beam or a jump at random, odds apart), 1 0x0205EC04 (stands), 2 0x0205EBD0 (stands, beams every 252 frames), 4 0x0205EA04 (a parasol, then 3's) | KatAM's own: 0 walks and beams or jumps at random, 1 a parasol, 2 stands, 3 stands and beams on a timer (logged: the walk, beam and jump frames) |
| 0x15 | Hot Head | 0x0203D8E8 | 0x0203CA3C | 0 and 3 0x0203C970 (walks 0.25, fireballs or its flame at Kirby), 1 0x0203C5DC (faces Kirby, stands), 2 0x0203C5B4 (stands, a fireball every 164 frames) | KatAM's own: 0 walks and attacks, 1 stands and attacks, 2 stands (logged) |
| 0x16 | Flamer | 0x02092790 | 0x0208FC34 | 0, 2, 4 (crawl round the terrain, then a fireball dash) and 1, 3, 5 (crawl at Kirby), each its own brain (0x0208FA98, 0x0208F6C4, 0x0208F620, 0x0208F250, 0x0208F1AC, 0x0208EEAC) | KatAM's own crawl and fireball dash (logged on flat ground only: the pairs alike) |
| 0x17 | Pengy | 0x020625B0 | 0x0206147C | 0, 3, 4, 6 0x020613B0 (slides 1.0, stops, breathes ice), 1 and 5 0x020611D4 (stands), 2 0x020611AC (stands, watching) | KatAM's own: walks and breathes ice at random, or stands (logged) |
| 0x18 | Sparky | 0x02034C34 | 0x02033B80 | 0 and 3-6 0x02033AB4 (hops 2.7 up, sparks), 1 0x02033820 and 2 0x020337F8 (stand) | KatAM's own: hops and sparks at Kirby near, or stands (logged) |
| 0x19 | Twister | 0x02045AFC | 0x02044B20 | 1 0x020441F0 (stands), others 0x020446EC (spins at Kirby near, 2.0 back and forth) | KatAM's own (logged) |
| 0x1A | Sword Knight | 0x0203FC54 | 0x0203E7E8 | 0 and 3-6 0x0203E628 (dashes 2.0 and slashes), 1 0x0203E0C8 (stands and slashes), 2 0x0203DBE8 (stands) | KatAM's own walking, standing slashers and idle one (logged) |
| 0x1C | Cupie | 0x020668C4 | 0x02063604 | each its own: 0 0x02063528 (a 40-pixel loop), 1 0x02063330 (a bigger loop), 2 0x02063138 (a rising loop), 3 0x02062F4C and 4 0x02062D38 (drop and hover, shooting), 5 0x02062A78 (a diamond of straight dives) | KatAM's own: its Cupies fly loops too (0 a big oval, 1 up and down, 2 a flat oval; 3 and 4 drop and shoot), the shapes apart (logged on both) |
| 0x1D | Sir Kibble | 0x02068FD0 | 0x020682B8 | 0 and 4-6 0x020681E8 (stands, a cutter every 165 frames), 1 0x02067FE0 and 2 0x02067D70 (walk 0.5, throw), 3 0x02067AD8 (stands) | KatAM's own standing and walking throwers (logged) |
| 0x1E | Laser Ball | 0x02057CEC | 0x020572E0 | 1 0x02056D70 (hovers), others 0x02056F3C (closes in, fires, flies off) | KatAM's own (logged) |
| 0x1F | Foley | 0x0205A030 | 0x02059320 | 0, 3, 4 (hang and bob), 1 (swings down at Kirby), 2 (dives at Kirby), each its own brain (0x02059138, 0x02058C30, 0x02058AEC; 0x02058FA0; 0x02058E04) | KatAM's own hanging and falling leaves (logged) |
| 0x20 | Wheelie | 0x0204745C | 0x0204626C | 1 0x02045D98 (stands), others 0x02045F7C (rolls at Kirby, 3.4) | KatAM's own (logged) |
| 0x21 | UFO | 0x02050E94 | 0x02050248 | 1 0x0204FCAC (faces Kirby, hovers), others 0x02050054 (darts round Kirby, then shoots beside him) | KatAM's own: its UFO (`sub_080C4ACC`, `sub_080C4B70`) darts to spots of a 16-spot table round Kirby, its next spot a few on at random, then flies beside him (56 pixels, after 3 darts; Squeak Squad's 64, after 4) and fires |
| 0x22 | Noddy | 0x02043F00 | 0x020430CC | 1 0x02042EC0 (sleeps, yawns), others 0x02042F64 (walks 0.25) | KatAM's own (logged) |
| 0x23 | Boxin | 0x0204F868 | 0x0204DE08 | 1 0x0204DA08, others 0x0204DD3C (alike near Kirby: hops back and punches) | KatAM's own (logged) |
| 0x24 | Heavy Knight | 0x02042B80 | 0x0204101C | 1 0x02040658 (stands and slashes), others 0x02040E08 (charges 2.0 and slashes) | KatAM's own (logged) |

## Stage objects

The stage's objects are 16-byte records: `*(*0x02229F1C + 0x90)` holds a `u16` count and, at `+4`, a pointer to them. A record:
`+0` its index, `+1` its kind (0, 5 and 6 are spawned through the factory), `+2` `u16` enemy ID, `+4` `u16` x in pixels, `+6` `u16`
y in pixels from the room's bottom (`*0x02229F1C + 0x60` is the room's height, `fx32`), `+8`, `+9`, `+0xA` its parameter bytes.
`0x0202EC50` spawns record `index` when the camera reaches it (the factory call is at `0x0202ECAC`).

The first room of Prism Plains' first stage holds a Waddle Dee (x 636), a Waddle Doo (x 1128) and a Sparky (x 1512). To watch any
enemy, the test harness loads a save state after Kirby lands there and rewrites the first record (ID, x, y, parameters): the enemy
appears next to him.

## How an enemy works

Squeak Squad is C++. An enemy is two objects, a **body** (its moves; Waddle Dee's 0x24C bytes, Perara's 0x260) and a **brain** (0xB8:
a virtual pad that the body reads). Their code was read with Ghidra's decompiler (headless, the ARM9 loaded at 0x02000000 as
ARMv5T, literal pools made read-only so constants fold).

- **States**: a member-function pointer at `+0x114` (`+0x118`: its adjustment, odd for a virtual one: `{0x3C, 1}` is the vtable's
  `+0x3C`, the "start" state) and a counter at `+0x128` (set to -1 on every change, raised before each run). The dispatcher
  (`vtable +0x34`, Perara's `0x0206E5A4`) runs the state again while it returns 1: entry functions set things up, set the state that
  runs every frame and return 1, so both run in the same frame (checked with breakpoints in the DS emulator).
- **Physics** (all 20.12, a frame; **Y up**: the stage's y grows upwards): `+0x74` x, `+0x78` y, `+0x8C` speed x (forward:
  `+0x14` bit 2 means facing left, and it is negated then), `+0x90` speed y, `+0x94`/`+0x98` speed caps, `+0x9C`/`+0xA0`
  acceleration, `+0xA4`/`+0xA8` friction, `+0x84` the distance down to the floor (under 1: on the ground). `0x02013D18` moves:
  acceleration, caps (`0x02013460`), friction (`0x02013370`), terrain (`0x0201355C`); the states check the ground right after.
  `0x02023468` stops everything, `0x02023410(v)` caps the fall at `v`, `0x020233EC(a, v)` sinks at `a` down to `v`.
  `0x020132B4` turns around: the facing bit, the speed and acceleration across, the walls ahead and behind swap, and the brain's
  direction is negated (its vtable `+0x10`, `0x02013368`: a "turn round", -1, becomes 1). The turn's frame still moves the old way.
- **Flags** (`+0x14`): bit 2 facing left, bit 3 the animation got through once, bit 6 in the air, `0x100` no terrain at all, `0x200`
  through the terrain (`0x0201355C` meets it, sets the flags below, but neither adds the push out of it to the move nor snaps to
  the floor; the stage's edges still hold the spot unless `0x20000`: Cret's and Uja's states set it, vtable `+0x40`), `0x400`
  touching a wall, `0x800` a wall ahead, `0x1000` a wall behind, `0x200000` in water.
- **Base class** (vtable): `+0x34` the dispatcher, `+0x3C` the start state, `+0x80` checked first by each state (the shared one,
  `0x02021408`: in water, the shared state `+0x6C`; Gold Waddle Dee's own, `0x020B5078`, swims), `+0x88` whether the brain runs.
  Bumping into Kirby runs the base class's `+0x48` and `+0x50` states.
- **Animations**: the sprite object's `vtable +8` with `0x1000000 | animation` plays one of the character's own (`.cea`).
  `0x02021A88` keeps the hurt animations (Perara: 7 and 8).
- **Parameters**: each `Params` object (`0x020213B8`, then `0x020212CC` loads its file) points at its values at `+0x0C`. The body
  keeps the movement ones twice: `+0x224` (`Ml0`) when the brain's `+8` is 3 (normal), else `+0x254` (`MlG`).
- **Brain**: `+0x8` 3 for the enemies' brains (the body's "normal mode": the other reads `MlG` and lets A jump in the air), `+0x10`
  its body, `+0x14` direction (1 forward, -1 turn around, 0 none; it holds until changed), `+0x18` up/down, `+0x38`/`+0x1C` button
  A, `+0x3C`/`+0x20` button B, `+0x74` its own state (one run a frame; states are member-function pointers in data tables). Helpers:
  `0x0206FF08` faces Kirby (each enemy has its own copy: unchanged when he's ahead), `0x0206FF64` Kirby within some distance
  across, `0x0206FF94` within a box. The brain runs before the body each frame. The spawn record's first parameter byte picks the
  brain's first state (Gold Waddle Dee: 0x020B4EF4's switch).
- **Random numbers**: a 64-bit LCG at `0x01FF8120` (`0x0200C7EC` multiplies); `random(0, 100)` picks weighted choices.
- **Effects and sounds**: `0x02021900` spawns an effect (offset, facing, type): types 0-5 are `effect/efc_cmn0`'s animations,
  6-16 `efc_cmn1`'s, by the table at `0x02157DF8` (type 3, the dust puff: `efc_cmn0` animation 19). Its last parameter makes the
  effect follow the character (`0x020218C0`: each frame, the character's position plus the offset). `0x02013EE8` plays a sound.

## Test harness (DS emulator)

`py-desmume` with a save state after Kirby lands in Prism Plains' first room. The first stage record is rewritten (ID, x, y,
parameters) and the enemy spawns next to Kirby; breakpoints on its state functions and its fields above give its moves frame by
frame, to compare with the port in KatAM (mGBA, same fields in KatAM units).
