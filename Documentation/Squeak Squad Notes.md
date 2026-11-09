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
| 0x37 | metaknight_cmn, metaknight_ext | Meta Knight (ported: `squeak/meta_knight_kss.c`) |
| 0x38 | dedede | King Dedede |
| 0x39 | kracko | Mecha-Kracko (ported: `squeak/mecha_kracko.c`) |
| 0x3A | mrsMoley_cmn | Mrs. Moley (ported: `squeak/mrs_moley.c`) |
| 0x3B | yadgaine | Yadgaine (ported: `squeak/yadgaine.c`) |
| 0x3C | bohboh | Bohboh (ported: `squeak/bohboh.c`) |
| 0x3D | dancho00, dancho01 | Daroach and Dark Daroach (his boss fight, parameter 0 and 1: ported, `squeak/daroach_boss.c`) |
| 0x3E, 0x3F, 0x46 | darkZERO_cmn, darkZERO_ext | Dark Nebula (0x46 his coming, 0x3F his fight: ported, `squeak/dark_nebula.c`); 0x3E the dark star Kirby chases through a8s4s3 and a8s4s4 (ported: `squeak/dark_star.c`); Dark Daroach is 0x3D's parameter 1 |

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
| 0x37-0x46 | the bosses | Meta Knight 0x02116C7C, Dedede 0x020E7E58, Mecha-Kracko 0x02132D7C, Mrs. Moley 0x0212CF40, Yadgaine 0x0211A4E8, Bohboh 0x02111370, 0x3D 0x02126F48, Dark Nebula / Dark Daroach 0x0211B680, 0x02122CD0, 0x021376EC | | |

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

**Mecha-Kracko** (boss 0x39, ported: `squeak/mecha_kracko.c`): its body `0x02132D7C` (`enemy/kracko`; vtable `0x02184908`: +0x34 per
frame `0x02132FFC`, +0x30 its hit `0x02132990`, +0x2C its touch `0x02132C80`, +0x20 its hurt flash `0x021327E4`, between two copies
of its palette: `enemy/kracko_dmg` isn't in the game, no flash shows), params `PrmBB03Ml0` (35 words: +0/+4 the side spots 80 from
the camera's middle, 64 under its top; +8 the low line 152 under it; +0xC/+0x10 its waits 30 and 60 more; +0x18-+0x20 its wave, 30
frames an arc, 5 arcs, 68.0 high; +0x24 its intro 120 frames; +0x28/+0x2C its swoop, 120 frames, 4.0; +0x34 4.0 its shakes; +0x38 30
frames shaking before an enemy; +0x3C/+0x40 its dive 60 over Kirby at 2.0; +0x44/+0x48 15 frames still, 90 in its field; +0x4C 2.0
back; +0x50 its crossing's 60-frame steps; +0x58/+0x5C its low sweep 120 frames out, 150 across; +0x64 60 frames' shudder;
+0x68-+0x88 its defeat: 120 frames, the core falling from 192 right of the camera at 0.2, Kirby put 64 right of it), `PrmBB03Sl0`
HP 320, then 400. Its states `0x02130510`-`0x02132744` (their table `0x02184730`); its spots `0x02132840` (kinds 0 its side's, 1
the other, 2/3 left and right, 4/5 out of view on its side or the other at the low line, 7 the low line's middle; from the camera:
its middle, its top); its picks `0x021322A4` (first form: rand(9) under 5, or 10 with more than 3 of the stage's enemies, a move,
`0x02132220`: rand(9) under 4 its wave `0x02132020`, else its swoop `0x02131EE4`; else an enemy `0x02131B30`; second form,
`+0x438`: rand(100), or rand(70) with more than 3, under 25 the dive `0x0213180C`, 45 the crossing `0x0213153C`, 65 the low sweep
`0x02130FF0`, 70 the field swoop `0x02131E4C`, else an enemy); its enemy `0x021319C8` (`EnemyFactory`: rand(9) under 2 a Starman
0x26, under 5 a Waddle Dee 0x00, else a Waddle Doo 0x14, variant 0, facing the middle). Its hit sends its first form's last into the
base's pause (`0x020216E8`, `0x02021648`: 60 frames, the stage's time object slowed) and `0x02130C64` (`.mca`'s 4, the cloud breaking),
`0x02130BB0` (`.mca`'s 0, its core), `0x02130ABC` (60 frames' shudder, `.mca`'s 1), `0x02130A2C` (HP max and HP 400); its last
into the defeat `0x02130924`. Its eye `0x0213025C` (a sprite of its own, registered with `0x02039B28`): shown while its `.mca` is
2, 3, 6 or 7, `kracko.cea`'s 8-15 by the way to Kirby (dx/dy against tan 67.5° and 22.5°, cached at `0x022016F0`). Its low sweep and
defeat call the stage map's +0x88 (`0x02216FF4`, `0x02216CB8`) at the low line: a cloud-floor tile (kind 0x60) where one is
missing, the sweep at the columns of `0x0215BFE0`'s first row (0, 1, 4, 5, 10, 11, 14, 15). Its stage `map/a3s7s2` (`a3s7.mxi`'s
part 2: `12_f`/`12_b`, `pa_12`, `d_viw_0A`; its spawn in `a3s7s2.doe`: 0x39 at 192, 104), 18 by 21 tiles, its camera fixed (left
16, top 240 up), the floor row 15 (Kirby at 96 up). Daroach brings it in (the stage's scene, `demo_dancho`).

**Yadgaine** (boss 0x3B, ported: `squeak/yadgaine.c`): its body `0x0211A4E8` (`enemy/yadgaine`; vtable `0x021824F0`: +0x34 per frame
`0x0211A8C0`, +0x30 its hit `0x0211A13C`, +0x2C its touch `0x0211A3DC`, +0x18 `0x0211A098`: its hurt flash from `yadogaine_dmg`'s
palettes, `0x0222BFDC` = its x (its wall) while it shows, and its x held at most the camera's left + 110 as logged), params
`prmBB05Ml0` (+0x0C 20 safe frames, +0x14/+0x18 the intro's 150 frames at 1.0, +0x1C 1.0 more for its rush, +0x24 the rounds' 0.5,
+0x28/+0x2C the pauses' 20 and 50, +0x30 the rounds' 90 frames, +0x34/+0x38 the rocks' 80 or 120 across the right, +0x3C every 15,
+0x44..+0x58 its dash (1.0, 0.3 more, till 110) and back (1.0, 0.2 less, till 64), +0x60..+0x68 its swipe, +0x6C..+0x78 its legs'
blocks' spots, +0x7C..+0x90 the blocks' speeds, +0x98..+0xA4 its lunge, +0xAC..+0xC4 its shot), `Sl0` (400 HP, its legs' 220 and
290, 360 of damage before its second form: `+0x20C`), `Pl0` (its picks: +0x04..+0x50; the rocks' kinds +0x58..+0x60; the legs'
blocks +0x68..+0x70 of +0x74). Its states `0x02117048`-`0x0211A038` (the chase `0x0211983C`: rocks, timer, the picks). Its legs
`0x021185FC` (the back one, 0x21186D0's class) and `0x02117C44` (the front one), both of `0x021188DC`'s class (vtable +0x38 its mode,
`0x0211851C`/`0x02117B64`: 0 walking `.mca`'s 2/3, 1 the same with its step sounds, 2 raised 8 then 9 / 12 then 13, 3 struck 10
then 11 / 14 then 15, 4 still; broken `.mca`'s 4/5; +0x30 their hit, +0x28 their foot touching the ground, called by the stage's
terrain, `0x022110A8` walking (a camera jolt) and `0x0221116C` resting (a jolt and the block chance)); a third, `0x02117280`
(`.cea`'s 37, `prmBB06Ml0` loaded over its own), holds its shell's box (`.rca`'s 35: 197 behind, 208 up) and places the stage's
background layer 3 (`0x02211540`, `0x02211534`: its shell, drawn by `0x0220AF90`/`0x0220B0B4` from the stage's second layer, 16 by 13
tiles; `0x022114F4` turns BG3 on). The shell's picture: `map/a4s7s2.mxp` (RLE, type 0x30), its sections' offsets at +0xC: the
map's width (the first's first byte), its 16x16 blocks (the third's, after 2 bytes: 4 tile entries each, tile, flips, palette - 6)
and the shell's 16 by 13 blocks (the fifth's top left); tiles and palettes `map/30_b.bin` (+0x210, +0x10), one 16-color palette
(14 colors used). Placed on the screen with its column 128, row 171 at the body's feet plus its body's step's offset down
(`.mca`'s 0, part 0: `.cea`'s 3, its steps 0-3 down); in a fight only columns 14 on and rows 27-171 ever show. What it throws and knocks down (`0x0212E248`/`0x0212E198`, the class `0x02184338`, kinds: 0 its
shot's puff `.cea`'s 35, 1 its capsule 36, 2-4 blocks 34 (landed, a solid rectangle in the stage's list `0x0222C018`, broken 32
ahead of the leg that knocked it), 5 a small rock 33, 6 a big one 32, 7 a gordo (`enemy/gordo`'s 3: bouncing once, 2.5 up, then
through everything), 8 Daroach (`demo_dancho`'s 1-5, off at -2.0, 2.0)). `.rca`'s animations are the `.cea`'s two behind. Its
stage `map/a4s7s2` (`a4s7.mxi`'s part 2: `30_f`/`30_b`, `pa_30`, `d_viw_14`; its spawn in `a4s7s2.doe`: 0x3B at 64, 160), 64 by 15
tiles, its width set to 512000 pixels: the camera's lock (`0x02035C04`) moves on with the scroll for good.

**Bohboh** (boss 0x3C, ported: `squeak/bohboh.c`): its body `0x02111370` (`EnemyBaseCtor(0x3C)`, `enemy/bohboh`, its hurt palettes
`bo_bo_dmg`; vtable `0x02181770`: +0x34 per frame `0x02111568`, +0x30 its hit `0x02110F98` (none while +0x1F8, its 32 safe frames,
or while it holds Kirby, +0x164), +0x2C its touch `0x021111CC` (kind 6 on a player: it grabs him, `0x0210EB40`), +0x18 its hurt flash
`0x02110D98`, +0x84 `0x02110D50` (lets what it holds go with damage), +0x3C `0x0210E1FC`), flags 0x200 and 0x20000 (through the terrain,
not held in the stage), params `prmBB06Ml0` (+0x08 0.1 gravity, +0x0C 32 safe frames, +0x14/+0x18 the intro's 60 frames and 2.0,
+0x1C..+0x24 coming in from 80 out and 80 over a point in 60 frames, +0x30/+0x34 its waits 30 and 60, +0x38/+0x3C the points right and
left of the middle (+1), +0x44..+0x50 its leap (3.0 across, 3.5 down, 0.11 up a frame; 8 frames to settle), +0x58 its charge 2.0,
+0x5C a fire every 30 frames, +0x64 its pass 2.0, +0x70..+0x7C its swoop (60 frames backing 0.4 and rising 0.2, then 4.0 at Kirby's
feet), +0x80 60 frames out of view, +0x84..+0x90 its dive (0.2 back, 2.0 up, 0.2 gravity), +0x94 120 frames in the lava, +0x98..+0xA0
its rise (0.5 up, 60 frames, its flare 20), +0xA4..+0xB4 its grab (60 frames' carry, thrown at 5.0 down to point 2 (+0xAC 3), 6 damage,
Kirby's 120 safe frames)), `Pl0` (+0x00 its short wait 50 in 100; +0x08..+0x10 and +0x18..+0x24 its picks with two of the stage's
enemies out or more, and fewer; +0x2C its swoop after a charge or pass 30; +0x34/+0x38 a drop every 40 + rand(20) frames in the lava;
+0x3C an enemy 30 in 100), `Sl0` (350 HP). Its states `0x0210E16C`-`0x02110C7C`: intro `0x02110C7C`-`0x021109F4` (its animation 4,
small, far off), wait `0x021108EC`/`0x02110898`, picks `0x02110828` (`0x0220A224`: the stage's records out), `0x0211072C`/`0x02110654`;
leap `0x021104D8`-`0x02110290`, charge `0x02110150`-`0x0210FEF8`, pass `0x0210FE4C`-`0x0210FB48`, after them `0x0210FA6C`/`0x0210F9C0`,
swoop `0x0210F874`-`0x0210F380`, dive `0x0210F288`/`0x0210F15C` (into the lava: `0x022098F8` 5 under it), the lava `0x0210EEE0` (drops
from the camera's top-left + rand(256) across: `0x0220A230(stage, 1 + rand(4))` the stage's records 1-4, a Waddle Dee, Pengy, Sir
Kibble and Hot Head, one of each out at most; else `0x02127824(1, ...)`), rise `0x0210EE18`-`0x0210EC98`, grab `0x0210EB40`-
`0x0210E788`, defeat `0x0210E730`-`0x0210E294`. What it brings in (`0x02127750`, vtable `0x02183980`, its own `prmBB06Ml0`): kind 0
its fire (`0x021278CC`, at its spot: `.cea`'s 12, 0.1 gravity, bursting on the ground through the stage's effect 6), 1 a fire from
the top (11, alike, `0x020B48E0`'s 6), 2 the lava's hurt (`0x0212737C`, made by its intro at the camera's middle and the floor:
`.cea`'s 18, no cell; `.rca`'s box kind 2, 128 across and 10 high, hurting Kirby, and Bohboh as it dives in: 20). `.mca`: 0 flying,
1 carrying (its claws, `.cea`'s 9), 2 rising out of the lava, 3 its flare. `.rca`'s animations are the `.cea`'s one on; its own hurt
boxes (0x102) in its animations 0, 4 (far off), 7 (its dive) and 8 (in the lava), those of 4 and 7 flagged 0x8000 (untouchable: see
"Where it's hit" below; elsewhere animation 0's box is hit, as recorded); its swoop (6) has two grabbing boxes (0x101 kind 6); its far
flight (4) and throw (10) no touch. Its stage `map/a5s7s2` (`a5s7.mxi`'s part 2: `09_f`/`09_b`, no palette, `d_viw_0F`; its spawn in
`a5s7s2.doe`: 0x3C at 36, 236; records 1-4 the enemies it drops), 34 by 26 tiles, the camera held at 272, 136 (its middle), the floor
and the lava's top at 80 up: its points 0 (356, 176), 1 (200, 176), 2 (272, 44, in the lava), linked 0 and 1 (the table after the
points); its zones across: 1 left of 208, 2 to 336 (the lava), 0 right of it. `a5s7s1.mxp` and `a5s7s2.mxp` are stored uncompressed
(the other stages' are RLE): loading them by repointing the stage table needs a test ROM with them compressed; Kirby goes in through
`a5s7s1`'s door at (134, 545).

**Mrs. Moley** (boss 0x3A, ported: `squeak/mrs_moley.c`): the factory's case makes her (`0x02025962`, `enemy/mrsMoley_cmn`, a null brain), her body `0x0212CF40` (vtable `0x02183F60`: +0x34 per frame `0x0212D198`, +0x30 her hit `0x0212C73C`, +0x2C her touch `0x0212CD2C`, +0x18 her hurt flash `0x0212A89C`: `morly_dmg`'s palettes every 3 frames for 20), params `PrmBB04Ml0` (89 words: +0/+4 her waits under a hole, 40 and 30 after going down by choice; +8 70, her attack chance; +0xC 20 frames before she picks; +0x10..+0x24 the hole she comes up in by Kirby's side; +0x28 20 safe frames; +0x2C..+0x38 her dive: 10 frames, 5.0 up, 140 frames over Kirby, 0.2 gravity; +0x44..+0x5C her crash: 60 frames' shake, 20 HP lost, 20 HP her last stand, 160 frames, her hop 1.2 up, 0.06 gravity; +0x64..+0x134 her things' speeds; +0x13C/+0x140 her extra throws, rand(3); +0x144..+0x15C her things' weights), `PrmBB04Sl0` HP 300. She's drawn from three sprite files, `0x02022F00(this, key, slot)`'s third argument the file: 0 `mrsMoley_cmn` (her small back self's animations 3-7, her big front self's 8-12, stuck 13, defeat 14-15), 1 `_ext00` (her dive: sparkles, jumping out, rising, upside down; landing; crawling, hopping, in a hole, sinking; and with `0x3000000|n` its `.mca`'s composites), 2 `_ext01` (her throw; its `.mca`'s composites the thing in her hand). Her states `0x0212A9C8`-`0x0212C68C`, her modes (`+0x1FC`: 0 the middle back hole, 1 a back hole, 2 a front hole: only then can she be hit) and phases (`+0x1F8`: 1 after choosing to go down, 2 her last stand: always attacking, from the middle). What she throws (`0x02129958(kind, her, offset, way, frames)`, `0x021292EC`, `_ext01`): kinds 0 a rock, 1 a stone, 2 a pebble, 3 a bolt (380 frames), 4 a ring (Wheel), 5 a bomb (Bomb; flashing its last 60), 6 a can (Fire), 7 a saw (Cutter), 8 and 9 grit (harmless, through the floor after a bounce); their flight `0x02127A50` (bouncing: up again at the speed thrown, less each bounce by its own, forward less by its own; bursting after its frames, on Kirby, `0x02129260`, or hit, `0x021291C8`); the abilities through `0x0203A9C4(…, ability)`. Her dirt (`0x0212A624(kind, her, spot, way)`, `0x0212A404`, `_ext00`): 0 the petals' burst, 1 and 2 petals (way 0 falling slowly; 1-6 knocked off, facing right 1-3, left 4-6), 3 her shadow (on Kirby 120 frames, then under her till she lands). Her stage is `map/a2s7s2.mxp` (her spawn in `a2s7s2.doe`: 0x3A at 244, 408), 18 by 30 tiles: its points (`0x02209CC0`) 0 (40, 408) and 1 (248, 408) the front holes, 2 (152, 392) the middle one, 3 (232, 396) and 4 (72, 396) the back ones; its zones (`0x02209C7C`) 0 the left 80 pixels, 1 the right 80, 5 between, 4 and 3 the back holes' tiles, 6 everything over 224 (184 pixels over the front holes: her leap ends 32 pixels over it); Kirby stands at 416, 8 pixels under the holes.
- **Stage rooms, for porting them** (found while porting the rooms; USA):
  - A stage's `map/aNsM.mxi` lists its rooms in play order, each a run of chunks (tag, `u16` length, value): `MA` the map
    (`.mxp`), `FB` and `BB` its two tilesets (file names in lower case: look them up without case), `DE` its objects (`.doe`),
    `PL` a palette file (only a few boss rooms: `pa_NN.pal`), `3D` its far view (`d_viw_NN.nsbtx`, drawn by the 3D engine on BG0),
    `TM`. Its header's bytes 8-11: area, stage (both from 0), room count. The 8 worlds are areas 1-8; area 9 is the Arena's copies
    of the boss rooms; Prism Plains' stages 8 and 9 and Nature Notch's 9 are test rooms (flat placeholder tiles). Some `.mxp` files
    are stored plain (`MaD\0`), not RLE.
  - A room's sections (offsets at +0x0C): 0 width, height in 16-pixel blocks (bytes); 1 and 2 two block tables (`u16` count, then
    8 bytes a block: four tilemap entries, top left, top right, bottom left, bottom right: tile and flips, palette bits 0);
    3, 4, 5 the block grids (`u16` a cell): 3 the front layer (table 1, `FB` tiles, the DS's BG2, priority 1), 4 the main terrain
    (table 2, `BB` tiles, BG3, priority 2), 5 a third (mostly empty); 6, 7, 8 their collision (`u32` a cell; the terrain tests read
    section 7's grid, `+0x38` of the map object, then section 6's, `+0x3C`, when 7's cell is empty).
  - A tileset (`map/NN_f.bin`, `NN_b.bin`): `u32` offsets and sizes (palettes at +0x10, 0x200 bytes: 16 palettes; the tiles, 4bpp,
    0x8000 or 0x9000 bytes; then one byte a tile, its palette: the tilemap entries' palette bits are all 0, the loader takes each
    tile's own). Both of a room's tilesets carry the same 16 palettes; a few are flat placeholders a stage never draws with.
  - A collision word (`0x0220FDEC`, `0x0220FF0C`, `0x02209880` and the tests at `0x02216540`-`0x02217050`, and the tiles each
    marks): bits 14-15 its kind (0 air, 1 solid `0x4000`, 2 a thin floor `0x8000`, 3 a slope `0xC000`); a slope's low byte its
    shape (read off every room's cells, what lies above, below and beside each code; the low nibble how deep the slope's edge is
    at the block's middle, from its top): floors 0x14 and 0x1C the upper and lower halves of a slope falling right, 0x28 a 45° one
    falling, 0xE8 rising, 0xF4 and 0xFC the upper and lower halves rising; ceilings 0x68 and 0xA8 (45°, solid on the left, on the
    right), 0x74 and 0x7C, 0x94 and 0x9C (halves); thin floors the same floor codes with 0x200; bits 8-13 flags: `0x100` a ladder
    (`0x8100` its top), `0x200` a thin floor Kirby jumps through, `0x300` another thin floor (Down drops Kirby through both: 2-2-3's
    `0x300` ones, checked in the DS emulator), `0x400` (front layer) a cover over a
    secret way, cleared when Kirby is under it, `0x800` burnt by fire (a candle on the cells, the grid's solid one flagged so:
    4-2-3's log bridges), `0x1000` set off by a bomb block (the grid's solid cell flagged so, top byte 0x38) or a stake beside
    them (top byte 0x39), or by a switch (a switch block group, gimmick 0x10), `0x2000` not yet known;
    bits 16-23 an id: `0x80` on doors (the room's door numbers, main layer), cannon fuses (`0x83`-`0x8C`, front layer), a big star
    block's quarters (`0x00`, `0x10`, `0x20`, `0x30`), ropes (`0x40` middle, `0x41` bottom, `0x42` top) and chains (`0x44`-`0x46`)
    with top byte `0x38`, spikes (`0x40`, `0x44`, `0x48`, `0x4C`: up, sideways, down, sideways), currents (water's `0x10`-`0x3C`,
    and the same alone in air); bits 24-26 a block (4 a star block, 5 a big one; with `0x38`: 1 a stake, 2 an electric zapper);
    bits 27-29 the surface (1 ground, 2 sand and metal, 3 grass, 4 clouds and snow, 5 ice and lava floors, 7 special); bits 30-31
    a liquid, read where the low half is air (1 water, `0x40000000`, 2 lava, `0x80000000`; `0x42000000` fire jets).
  - The objects (`.doe`): `De`, then `u16` offsets: +2 (12) the enemy list, +6 the item list. The enemy list: `u16` count at +12, then
    16-byte records from +14 (the stage-object records above; the loaded file is the list). The item list: `u16` count, then 16-byte
    records: `+2` `u16` its kind, `+4` x, `+6` y from the top, `+0xB` its flags (as the enemies'), `+0xD` a chest's treasure number.
    Kinds (570 items in the 8 worlds; seen in the DS emulator, Kirby moved beside each): 3 a 1UP, 5 a Maxim Tomato, 4 (the
    invincible candy, a red and white swirl), 6 (a drumstick), 7 (an energy drink) and 8 (a cherry: Prism Plains 1's second room,
    x 840) food, 1 (a bubble holding a small Kirby, the 1UP's look) and 0x15 (a bubble with a question mark, not yet confirmed what
    it gives); 0x11 and 0x14 treasure chests (0x14 the big red one; their
    treasure numbers at `+0xD`); 0x17-0x30 abilities, the ability the kind less 0x16, on a stand or in a floating bubble (0x17
    Fire, 0x18 Ice, 0x19 Spark, 0x1C Parasol, 0x1D Cutter, 0x26 Sword, 0x29 Ninja: seen). The Squeaks (enemy IDs 0x40-0x44: Squeakers, which `+8` colors, Spinni, Doc, Storo; 0x45 Daroach in a stage) are in the
    enemy list.
  - The spawn flags (`+0xB`, enemies and items), from the stage-object code: a record spawns while its state (`+1`) is 0, 5 or 6
    (`0x0220A368`); with bit 4 it spawns only once the stage's treasure event has run (`0x02218D00`: the stage's `+0x14` set to 1;
    in stage mode 2, Kirby carrying the stage's chest, everything spawns), without it only before: all 856 Squeakers, most of
    Spinni's, Doc's and Storo's records and the dark star's are these chase spawns (`0x02219028` brings out the Squeaks among them);
    bit 1 keeps a defeated one gone till the area reloads (state 4, else 2: `0x02218EF4`); bit 8 is on the mid-bosses and bosses.
  - The item maker (`0x0203B210`, item IDs 0-14: 0 nothing, 1-3 the same object with kinds 3, 1 and 2 (`0x0203A398`: 3 the 1UP),
    4 and 5 others, 6-14 food: 6 the Maxim Tomato, 7 heals 24 of 36, 8 12, 9 6). The floating bubbles' parameters (section 10's
    type 0x11 table: 0x10 or 0x11 and an item ID, 0x00 or 0x01 and an ability) name these IDs: 1 (40 of them), 2, 3, 6 (21), 7...
  - Section 9 after the zone grid: a `u16` count and 8-byte points (`u16` index, x, y from the top, size), then a table; the paths
    and spots of the Squeakers and of the bosses (Mrs. Moley's holes); most rooms carry one dummy point.
  - Section 10, the gimmicks (`yakumono`, made by `0x0221D5B8`'s switch when the camera nears them; checked by screenshots in the
    DS emulator, Kirby moved beside each): `u16` size, then `u16` the entries' offset, the tables' offset, 0, `u8` entries, tables;
    an entry is 8 bytes (`u8` type, index, `u16` x, y from the top, a byte); a table (offsets listed from the tables' offset) is
    `u8` type, count, `u16` size, then each entry's parameters by its index. Types: 0 a small star switch, 1 a hanging rope,
    2 shutters (gates), 3 a floor hanging from a rope, 4 (unseen: a path), 5 thunder clouds, 6 a rolling boulder, 7 fire columns,
    8 falling rocks, 9 icicles, 0xA candles (dark rooms), 0xB a rotating cannon, 0xC a cannon, 0xD a warp star's flight (its path in
    the table), 0xE grass tufts, 0xF barrels (holding food: `0x10` and the food), 0x10 the blocks a switch toggles, 0x11 floating
    bubbles (`0x10` and a food, or `0` and an ability), 0x12, 0x13, 0x14 (rectangles: camera or scroll areas), 0x15, 0x16 (the
    Squeakers' treasure rooms' timings), 0x17, 0x18, 0x19 (171 in two lava rooms), 0x1A: unseen helpers, not yet identified.
  - The gimmicks in the 8 worlds (a room's entries, type: count in rooms): switches 0: 79 in 48, ropes 1: 25 in 9, shutters 2:
    119 in 46, rope floors 3: 16, paths 4: 38, thunder clouds 5: 19, rolling boulders 6: 15, fire columns 7: 43, falling rocks 8:
    26, icicles 9: 17, candles 0xA: 8, rotating cannons 0xB: 44, cannons 0xC: 3, warp star flights 0xD: 19, grass 0xE: 47,
    barrels 0xF: 46 in 24, 0x10 ("floor": wooden planks, Prism Plains 3's third room): 48 in 31, bubbles 0x11: 227 in 76, and
    0x12-0x1A (0x14: 58, 0x15: 178 in 102 rooms, 0x16: 95 in 80, 0x18: 33, 0x19: 171 in 24, along Vocal Volcano's lava; not yet
    identified). `0x0221D5B8` makes each from its class's constructor (0x0F's `0x0222277C`, vtable `0x02229594`; 0's `0x022254AC`,
    `0x02229B84`; 2's `0x02224EFC`, `0x02229B24`; 0x10's `0x02223900`, `0x02229838`). A gimmick is a state machine: its state at +4,
    vtable slots 2-12 the states' functions (each returns the next state), slot 16 whether it takes a message (+4 its kind: 7, 8,
    10, 11...) in its state, slot 17 what the message does. Their graphics are `yakumono/*` (ball_rock, barrel, bubble, candle,
    cannon, fire_long, floor, grass_*, ice_flash, icicle, rock, rot_cannon, shutter, shutter_needle, snow, spark, stream,
    switch_small, thunder_cloud, warp_star). On the DS: Prism Plains 4's third room's switch is a small red button and its shutter
    a stone pillar from the ceiling; Vocal Volcano 5's second room's fire columns spout from the lava. Not in KatAM yet: the port
    leaves the shutters' ways open (their cells are air: the shutter is an object), nothing blocks.
  - The gimmicks talk to each other: a gimmick sends a message to others by their index in the room's list (`0x0221CE2C`: the
    targets' indices, how many, the message's kind); each takes it or not by its state (vtable slot 16) and acts on it (slot 17).
    A message to a gimmick not made yet is dropped (the manager's table of made gimmicks, `+0x1C`, is NULL there). Each class's
    constructor, by type, from the factory's jump table (`0x0221D5B8`): 0 `0x022254AC`, 1 `0x022249C8`, 2 `0x02224EFC`, 3
    `0x022219C4`, 4 `0x02223D78`, 5 `0x02220C00`, 6 `0x0221EE04`, 7 `0x02220094`, 8 `0x02221740`, 9 `0x022233B0`, 0xA
    `0x022237A4`, 0xB `0x02224730`, 0xC `0x02220710`, 0xD `0x02225BC0`, 0xE `0x02222078`, 0xF `0x0222277C`, 0x10 `0x02223900`, 0x11
    `0x02221BA4`, 0x12 `0x022202E4`, 0x13 `0x022240E8`, 0x14 `0x02224B88`, 0x15 `0x0221E5A4`, 0x16 `0x02222A50`, 0x17
    `0x02224384`, 0x18 `0x02226384`, 0x19 `0x02225E9C`, 0x1A `0x0222684C`. A state n runs vtable slot n + 3 (slot 2 returns the
    state as it is), returning the next state.
    - The switch (0) sends kind 0 (`0x022252F0`: its first byte's high nibble how many targets, then their indices); its targets
      in the 8 worlds' files (the unused rooms too): shutters (65 switches), switch block groups (16), relays (9), bubbles (4),
      a path (1).
    - A trigger area (0x15, `0x0221E5A4`, vtable `0x02228F48`): its parameter's u16: bits 0-6 its width, 7-13 its height (blocks of
      16 pixels: the box x 0 to w, y -h to 0 in the DS's world, whose y goes up: on the screen from its spot right and down; 4-3-6's
      are under the holes its shutters close, which shut over Kirby once he's dropped through), 14-15 how many targets (1-3), its bytes 2-4 their
      indices. A body moving into it (message kind 5) fires it once (state 1, `0x0221E4EC`: sends kind 2 to its targets); it fires
      again only after the body has left (state 0, `0x0221E520`, clears its flag when no body touched it that frame). 4-3-6's two
      close the shutters over the holes above them once Kirby's dropped through; 4-3-2's start a relay; 8-1-9's close five pairs of
      floor shutters one by one up its shaft.
    - A delay relay (0x16, `0x02222A50`, vtable `0x02229608`): its first byte's low 7 bits how many targets (up to 5), bit 7 that
      it takes kind 4 only (the other relays kinds 0-2); bytes 1-5 the targets' indices, then a u16 each, the frames before its
      message goes. A message while it's idle (`0x022229B0`: the kind kept, 4 sent on as 2) starts it (`0x02222930`): each target's
      wait set, a target with none (or not made) sent its message at once; then each frame (`0x022228C0`) each wait counts down
      and the message goes as it runs out. 3-6-6's switch starts two that lower its spikes at once; 4-3-2's, started by a trigger
      area, send its boulders down (gimmick 6, placed over the room's top) 140 and 360 frames later. The Squeakers' treasure rooms'
      0x16 (bit 7 set) wait for kind 4 (the Squeak Lair's events, not ported).
    - The shutters (2, `0x02224EFC`, vtable `0x02229B24`): its parameter's u16: bits 0-2 its look (`0x02227EA4`, 0x14 bytes each: 0-2
      `yakumono/shutter`, a stone pillar, at 0, 90 and 270 degrees; 3-6 `yakumono/shutter_needle`, spikes, at 0, 180, 270 and 90;
      the rooms use 0-4), bit 3 that it starts open, bits 4-15 the frames it stays changed (0: for good). Its box (the constructor's
      cases, around its spot): the pillar 1 by 2 blocks (turned 2 by 1), the spikes 2 by 1 and a pixel (`0xFFFEF000` to `0x10000`).
      Its animations (`0x02227E7C`, by bit 3: start 0 / 2, change 1 / 3, hold 2 / 0, change back 3 / 1): 0 shut (the pillar; the
      spikes up), 1 opening (the pillar crumbles up into its top; the spikes sink), 2 open, 3 shutting. Its states: 0 waiting; a
      message (kinds 0 or 2, taken in states 0 and 4 only: `0x02224EA4`) starts 1 (its change's animation, sound 0x14B and a quake
      for the pillar, 0x2D for the spikes); 2 when it's over, the other animation held, the wait set: 3 for good (no wait) or 4
      counting down (a message then sets it again, `0x02224E70`); at 0 the change back and 5, then 0. Checked in the DS emulator:
      1-5-1's shut pillar stops Kirby at its side (x 394, its box from 400); 3-6-6's raised spikes knock him back.
  - The fire columns (7, `0x02220094`, vtable `0x022290EC`; `yakumono/fire_long`: animations 0 the pilot flame, 1 flaring up, 2
    the column, 3 dying down): its parameter's first u16: bits 0-3 its look's turn (`0x02227B30`: 0 up, 1 at 180 degrees, 2 at
    270, 3 at 90), bits 4-15 the frames it burns (0: for good); its second: bits 0-3 its mode (0 out, waiting for a message; 1
    lit, waiting; 2 burning by itself: `0x02227AF8`, `0x02227AFC` its start), bits 4-15 the frames it stays out (0: for good).
    Its states (slots 3-7): 0 waiting (a message, kinds 0 or 2, lights one out, puts out one lit: `0x0221FF14`); 1 out
    (`0x0221FE88`: its wait, then flaring up); 2 flaring (`0x0221FE24`: then the column, sound 0x69); 3 lit (`0x0221FDC4`: its
    time, then dying down); 4 dying (`0x0221FD68`: then its pilot flame and, by its mode, `0x02227B08`, 0 waiting or 1 out). Its
    box: 1 by 4 blocks around its spot (turned, 4 by 1). The rooms' (2-5-2, 5-2, 5-5, 7-5-1, 8-1): modes 0 and 2, all four turns.
  - The barrel (0x0F; `yakumono/barrel`, its animations: 0 open with its lid and a down arrow, 1 the lid closing, 2 closed, 3 the
    lid opening, 4 shaking, 5 bursting, its hoops and staves flying, 6 burning, 7 its pieces): one holding something (its table's
    second byte not 0xFF: the food or ability as the bubbles', its first byte's high nibble 1 a food, 0 an ability) starts
    closed; an empty one open (Kirby can get in: message 7 while empty, then 8 out; 10 shakes it). Message 11 (an attack, not
    with its 0x1000 flag) bursts it (sound 0x51), leaving what it holds (`0x0220A200` a food, `0x0220A2A4` an ability).
  - The grass tufts (0xE; ctor `0x02222078`, vtable `0x02229500`; `yakumono/grass_A_S` and `grass_A_L`, `0x02227C28`'s 0x14-byte
    entries by its table's first byte's low nibble: 0 small, 1 large): its box (-1.0, 0) to (1.0, 3.0) small, (-2.0, 0) to (2.0,
    3.5) large; it takes messages 7 and 8 (`+0x48`, `+0x4C`). Its messages (`0x02221F90`): 5, a body moving through it (the
    box saved at +0x5C..+0x68: it rustles, `0x02221DFC`, its timer from `0x02227C1C` by size, sound 0x58, then back, 0x56/0x57 by
    size), 0xB an attack (its flags' bit 0: cut, state 5 `0x02221C64`), 0xC another. Cut (`0x02221C64`), it lets out what it
    holds (its table's second byte, not 0xFF: by the first byte's high nibble, 0 an ability `0x0220A2A4`, 1 a food
    `0x0220A200`, as the barrel's and the bubbles'), plays its cut animation (sound 0x59) and stays cut; one that held something
    grows back after 600 frames (`0x02221F58`: +0x6C counts to 0x258). Not ported yet.
  - Section 11, the doors: `u16` count, then 10 bytes a door: `u16` its number (the collision's `0x80 + n` marks it), `u8` flags,
    `u8` kind, `u16` x, y from the top (the middle of the cell Kirby stands in: a floor under it, but for a door high in Gao Gao's
    room in Nature Notch 3), `u8` the stage's room it leads to (0-based, the `.mxi`'s order) and `u8` that room's door (its number;
    `0xFF`: none). Kind 6 is where Kirby starts (most doors lead to one), 4 a door, 0x42 a dark doorway, 0x40, 0x41, 0x80, 0x82,
    0xC1 and 5 other entrances: all entered with Up, none walked into (checked in the DS emulator, Kirby put on one of each kind
    and walked through it both ways, then on it pressing Up; 0xC0 and kind 0 are only in the test stages 1-8, 1-9, 2-9). Flags:
    0x10 and 0x90 ways on, 0x23 with no room the stage's goal, 4 a hidden way (46 of them: the DS draws no door there and Up does
    nothing, no switch in its room; they lead to the stages' gold-pile treasure rooms, whose doorway leads back to the stage's
    start), 2 others (0xC1's). 562 doors lead to another room of their stage; 4 to rooms Nature Notch 6 lists but never had.
  - The far views (`map/d_viw_NN.nsbtx`, 39 of them): a BTX0 file, its TEX0 block (+0x10) holding one 256-color texture (format 4)
    and its palette: 512 by 256 pixels (33), 256 by 256 (5) or 256 by 512 (d_viw_0C, a climb's); a painted panorama, its top 192
    rows what the DS shows (the rest its last row smeared down). In KatAM: shrunk to 3/4 (48 tiles wide, 20 tall with the smear),
    fitted to 8 palettes of 15 colors in at most 900 tiles (`ConvertSqueakFarView()`: all 39 fit, at most 10 tiles shared).
  - Section 12, the foreground's tile animations: 12 bytes (the count a `u16` at +10), then 12 bytes each: `u16` a block, `u8` its
    frames (1-8), `u8` its table (0 the front layer's, 1 the main terrain's, its grid and the third layer's), 8 bytes the frames'
    durations (frames). The block shows blocks block + 1 to block + frames in turn (it looks as its first): a main tileset holds
    1152 tiles where its blocks' entries name 1024 (10 bits), the frames' tiles after. 45 of the 8 worlds' 493 rooms animate: grass
    swaying (Prism Plains 1's first room), water, flames along lava (Vocal Volcano 4's fourth), the glow behind King Dedede
    (Prism Plains 7's second: 51 animated blocks, two timings); at most 3 different timings in a room.
  - **In KatAM** (`KatAMExpansionSqueakRooms.cs`, `AddSqueakRooms()`; the e2e harness's `KSSROOMS=1`, not yet an option): the 493
    rooms of the 8 worlds (the test stages left out), each whole, as rooms 0x400 on, past KatAM's 0x3D7 (its goal games' 0x3D4-0x3DD
    among them). The three tables with a row per room (`gRoomProps`, `gUnk_08D6CD0C`, `gUnk_0893CB44`) move, grown to the last
    room, every reference repointed (KatAM's code and the expansion's name each by its base alone: checked as the ROM is made); the
    tables the props index get the rooms' entries after NiDL's, in room their copies leave (`SqueakTableRoom`). A room's foreground
    is its three layers as one picture (`ConvertSqueakForeground()`), a star block's cell drawn without the front layer for what it
    leaves (the block is the front layer's); its background the far view; its collision `SqueakCollision()` (a star block KatAM's
    0xF0, its special block record leaving air, or water in water). "ROM Expansion/rooms.c" holds rooms of up to 4032 blocks
    (Cushy Cloud 1's first room, 96 x 42; `BIG_BLOCKS`): slot 0 in KatAM's arrays, the other slots' collision and zones in KatAM's
    tilemap array after slot 0's tilemap and their tilemaps at the heap's end (the heap 0x7C80 bytes: the game used under 6 KB of it
    in every room and fight measured, the title and pause screens too); `gNidlRoomTops`' `ROOM_FULL_BOTTOM` lets the camera go down
    to 8 pixels from the room's bottom (KatAM's 40 hide its last rows: the DS shows them, and these rooms get no padding). Doors
    lists past KatAM's keep their visited doors in rooms.c's area (1206 lists), the items' taken bits too (kinds 5-7 for Squeak
    Squad's, 256 each). Checked in the GBA emulator (forced starts, not a playthrough): Prism Plains 1's first two rooms (the star
    blocks inhaled away), Cushy Cloud 1's first (the biggest; the helpers' room loaded in another slot at once), Secret Sea 1's third (water),
    a ladder climbed in Cushy Cloud 6's second.
  - **Their objects in KatAM** (`SqueakObjects()`): a spawn's spot is its feet (y from the room's top), KatAM's templates 8 pixels
    over them (its own enemies' and items' stand a cell's half over a floor: measured on every room of the USA ROM). Enemies KatAM
    has (`SqueakNatives`) get KatAM's type, the spawn's first byte picking the behavior (the brains table above: KatAM's own where
    it has that brain, `squeak/natives.c`'s where not; Glunk's by the second byte); the ported characters (`SqueakCharacters`)
    type 0x2C, the first byte their variant (Daroach's scene: his second byte his facing); mid-bosses and bosses
    (`SqueakBosses`, and KatAM's own Mr. Frosty, Bonkers, Box Boxer, Boxy) their mini-boss type, after an arena object
    (`enemies/arena.c`, NiDL's type) holding the camera on their screen (`SqueakArena()`: across a room two screens wide at
    most, else round the spot; 32 pixels of floor under it; from Kirby's first frame in a room a screen big, 288 by 240).
    0x3D with its second byte 1 is Dark Daroach; Dark Nebula's coming (0x46) is left out (his fight's port, 0x3F, does both).
    Items: 1UPs (kinds 1, 3), Invincible Candy (4), Maximum Tomato (5), Meat (6), Pep Brew (7), Cherry (8, and the "?" 0x15),
    taken once per visit (rooms.c's kinds 5-7); an ability (kinds 0x17 on, or a floating bubble's 0/1 and ability) is the
    enemy giving it (`SqueakAbilityEnemies`), standing on the floor under it (10 cells down at most), or a Cherry when none
    does (12, 18, 26). The floating bubbles' foods by the item maker's IDs: 1 the 1UP, 6 Maximum Tomato, 7 Pep Brew, 8 Meat, others
    a Cherry (IDs 2-5 and 9-18 not yet seen in the game: the rooms they're in, the stages' treasure rooms, fill them only after an
    event). Not yet: the chase spawns (flag 4). KatAM draws every placed item with its glow (effect 0x2BB, `sub_0809513C`, one
    frame in four).
  - **The treasures** (the chests' `+0xD`, 0-119): which is which goes by its range, not a table: the stage result's info screen
    (overlay 9, `0x0220AC70`, the 14 `result/infoNN.scr`) is picked by it: 0-4 Star Seals, 5 the Sound Player, 6-13 Vitality
    ("collect two to extend your life"), 14-20 Secret Door Keys, 21-27 Ghost Medals, 28-50 Ability Scrolls, 51-61 music, 62-69
    Boss Battle Badges (the boss rooms' chests), 70-87 Spray Paints, 88-92 Copy Palettes, 93-99 the Secret Map's pieces, 100-118
    Graphic Pieces, 119 the Shortcake; 13 the Bubble Items (not a treasure). On the DS a chest is swallowed into Kirby's stomach
    and opened at the stage's end; its treasure shown then. Treasure 0 is in 5 chests (Prism Plains 1's fifth room and the secret
    stage 2-8), 66 in two (Bohboh's two rooms).
  - **The platinum chests** (the owner: they "can use the same open/closed graphics in Squeak Squad"): a Squeak Squad room's big
    chest is KatAM's big chest drawn with `item/treasure`'s animations 6 (closed) and 7 (open): `SQUEAK_PLATINUM_CHEST`, a
    graphics-only character (16 pixels lower, as its feet are its origin and KatAM's chest's its middle; its frames in the big
    chest's 16 tiles, checked as the ROM is made), shown by `SqueakAnimFor()` for a big chest of `SQUEAK_CHEST` (frames 0, 2
    closed; 1, 3 open), its palette loaded by `SqueakCreateChest()`. The small chests keep KatAM's look (`item/treasure`'s 1, 2:
    the same red chest). Each chest stands on the floor under its spot (`OnFloor()` to the room's bottom; one buried in the rock,
    as Jam Jungle 4's fifth room's on the DS, on the floor of the first open space under it): KatAM's `CreateChest()` puts a
    chest's bottom on the cell edge at or under its template's y + half its height, so a big one's template is 16 over the floor,
    a small one's 8 (a big one at the floor's top - 8, as before, sank a cell into the floor and fell out of the room: Prism
    Plains 3's fifth room's was never there).
  - **Collision, the blocks grid first**: Squeak Squad reads a cell's collision (`0x0220FDEC`, its low half: kind, flags, shape)
    from the room's grid at +0x38 and, where that one's kind is air, from the one at +0x3C: section 6 is +0x38 and section 7 +0x3C
    (checked in the DS emulator against 1-3-1's loaded room: each grid matched to its section). Section 6 is the blocks grid: the
    star blocks, the zappers, the metal blocks and bricks, Cushy Cloud's and Secret Sea's clouds and much plain terrain; its kind
    byte (bits 24-31) sorts them into families (bits 24-26 the block: 1 metal, 2 zapper, 4 star, 5 big star; bits 27-29 a family:
    0x00 bricks, 0x08, 0x10, 0x18, 0x20 the clouds, 0x28, 0x38 ropes). `SqueakCellWord()` reads it so (an empty main cell takes
    the blocks grid's, with its water). So a door with a star block in the blocks grid over it is behind the block, as on the DS
    (1-6-3's at 79, 5, behind a 10 by 4 wall of them; 3-3-3's, 7-6-8's, 8-2-3's): in KatAM, broken with the Sword, the door shows
    and leads where the DS's does (0x423 to 0x424, 0x48B to 0x48D, 0x5A9 to 0x5A2: 0x5A7 to 0x5A0 since 5-7 was left out). What a broken block leaves is the main grid's
    cell (a star block over the main grid's solid leaves solid, 1-3-11's floor ones). The first rule tried (the main grid first)
    walled the doors behind star blocks into the open: the door sweep showed it.
  - **Bomb chains, stakes and switch blocks**: the blocks grid's cells flagged 0x1000, touching each other, go together: set off by
    the bomb block among them (1-1-4's grey wall: a slide into the bomb sets it off, one into the wall away from it doesn't,
    checked in the DS emulator; 3-2-2's bomb faces only the pocket its chain walls in, and an attack from under it, through the
    block below, sets it off), by a stake beside them (all 13 of the 0x39 stakes are), or by a switch: gimmick 0 (the small
    star switch, `0x022254AC`; a body moving through it or an attack presses it, 0x02225458) sends its targets their message
    (`0x022252F0`: its parameter's first byte's high nibble the count, then each target's index in the room's gimmick list; its
    look `0x02227F40`, look 0 `yakumono/switch_small`, the states' animations 0 up, 1 going down, 2 down, 3 coming up); a target
    that's gimmick 0x10 (`0x02223900`) clears its group (3-4-8's switch by the tower names index 5, the group in the beam over
    it); the switch rises again once its targets are done, and pressed again toggles them back. Groups with none of these (the
    2-cell ones in the stages' last rooms, 2-4-1's floor over its hidden way) are a Bomb's blast's. In KatAM: chains (the bomb
    block and a stake 0xF1, the blocks 0xF2, the air cells 0xF3), each broken cell drawn with only the third layer (the DS clears
    both grids; 1-1-4's grass stays), and the switches chest.c's SqueakCreateSwitch (KatAMExpansionSqueakRooms.cs).
  - **Bricks and metal blocks**: the blocks grid's metal blocks (block 1, grey) and bricks (family 0x00, brown, riveted: 1-6-5's
    wall round a chest, 2-6-2's, 4-3-6's in the cannons' way) are broken by a cannon's shot (4-3-6's chain goes through them on the
    DS) and strong attacks: KatAM's metal blocks (0xF5, attribute category 2), which its cannon shots break too (checked: 4-3-6's
    chain in KatAM breaks its way through, the same cannons in turn as the DS's).
  - **Their chests in KatAM**: KatAM's small and big chests (objects 0x80, 0x81), their treasure index `0x9F` (`chest.c`'s
    `SQUEAK_CHEST`: `gTreasures.shardField`'s bit 31, which nothing reads), their own number (0-127, in the rooms' order) in the
    template's `+0x12`, whether opened in `rooms.c`'s saved area (`SqueakChestOpened`, 16 bytes before the taken bits); their spawn
    `SqueakCreateChest` (it shows one opened open), their opening `NidlChestOpen` (it sets its bit). Each is in its room's 9ROM
    list too ({1, 8, 0xFF, 0xFF, x, y}, first, counted in the list's entry): opening a chest marks it there (`sub_08002B30`),
    which never ends without it (the game hangs, its VBlank counter still running); `rooms.c`'s versions of it and of the doors'
    lookups now stop at the list's end. Each holds a consumable for now (KatAM's chest items in turn); the chests of the
    treasures the owner listed (Ability Scrolls, Spray Paints, the Secret Map, Vitality) are to hold those. The chase spawns'
    22 chests (flag 4) aren't placed yet.
  - **Warp stars** (gimmick 0xD, ctor `0x02225BC0`, vtable `0x02229C20`): its parameters' words: the stage's room it flies to
    (0-based, the low byte; 254, 255: one Kirby comes in on), a code, then its path in segments of 6 words: a kind (33 and 49: a
    quadratic curve; 32 and 0 end it), frames, where it ends, its control point. The DS flies the curve frame by frame from where
    the star is (1-4-1's ride every 20 frames). A ride (checked in the DS emulator with the stage's room table rebuilt in free RAM,
    `kr/ds/starland2.py`: the table at `0x22c8f50`, its count at `0x22c8f4c`): out of the star's room along its path, a white fade,
    then in the room it names: the star Kirby comes in on (254/255) flies its own path and he drops off at its end onto the floor
    under it (1-4-2: path end 102, 70, he lands at 98, 144); a room with no such star but one to get on, that one carries him on
    (3-4-4 → 3-4-5 → 3-4-6: lands at 104, 160 under 106, 120); a room with neither, he comes in where it starts (its kind-6 door)
    and drops (8-4-4 → 8-4-5: at 56, 136, landing at 56, 160). In KatAM (`SqueakRides()`): each star to get on is KatAM's warp star
    flying that ride with the NiDL rides' machinery (`RideFlight()`, its legs the DS's paths frame by frame, Kirby's drop as a hop
    and a fall), landing where the DS lands him. A star coming in under a room's last rows starts its path where it's first inside
    (KatAM's room change puts Kirby back inside: 8-4-4's flew 64 pixels high). All 12 rides checked in KatAM (each lands where the
    DS lands Kirby, give or take 4 pixels).
  - **Cannons**: gimmick 0xB (`0x02224730`) is a cannon Kirby gets in, its parameter's bits 2-4 its way (0 up, clockwise by 45
    degrees: KatAM's eight-direction cannon's order), bits 0-1 its kind (0 fixed: fires on its own about 35 frames after he's in;
    1 turning: A fires it). Gimmick 0xC (`0x02220710`) is a fuse cannon: first byte the stage's room it shoots Kirby to, second
    that room's door; its fuse in the blocks grid (ids 0x81-0x8F, low bits its ways: 1 right, 2 left, 4 down, 8 up; one end where
    it's lit). In KatAM: KatAM's eight-direction cannon (0x8B; chest.c's `SqueakCreateCannon` holds a fixed one's way, fires it
    after the DS's wait, and carries every shot on to the next cannon or wall as the DS's do, KatAM's dropping it after 45
    frames), and KatAM's fuse cannon (0x8A) with KatAM's fuse cells and fuse object (cannon_fuse.c, which burns down out of the
    object's cell: a fuse lit from the side gets its object a cell up). Checked in KatAM: 4-3-6's chain of six fixed cannons, the
    same route as the DS's; 3-4-2's fuse, lit with Fire, burns up to the cannon, which shoots Kirby into 3-4-3 at its door 0. The
    gimmick's spot is the cannon's foot, its shot leaving 8 pixels up: 8-1-5's turning cannon, in the middle of eight doors each
    behind metal blocks, fires through the middles of all eight from there (KatAM's cannon is placed 8 up; checked in KatAM: each
    way's shot breaks its blocks, and its door then takes Kirby to its room). A fuse lit from the side (5-1-1's: its end on the
    floor, the rope going right into the wall) keeps KatAM's fuse object a cell up, the lit cell a corner down into the rest: KatAM's
    burner (cannon_fuse.c) starts with a step down out of the object's cell, which clears the bit its end needs to set the cannon's
    state slot (with the object on a rope cell instead, the burn ran to the cannon and then fell off the room, the cannon never
    firing); Fire's breath from the floor beside it reaches the object. KatAM's cannon lands Kirby in the next room falling, then
    bounces him forward twice (`sub_08051874`: 2, then 0.8 pixels a frame); 5-1-1's lands him in 5-1-6, a small room between two
    pits, on a ledge over steps down, and the bounces carried him down them into the right pit: in the Squeak Squad rooms he lands
    without the move forward (chest.c's `SqueakCannonArrival`, the state `sub_0805C024` gives him), on the ledge at the door, as
    the DS puts him.
  - **Their doors in KatAM** (`SqueakMap.Doors()`, `AddSqueakRooms()`): each door leading to another of its stage's rooms is a
    KatAM door (its cell 0xFF, 0xFD in water; in a star block what the block leaves, the door's record before the block's), with
    its 8ROM record (kind 3: the room, the landing cell + 1, facing the room's middle) and a 9ROM entry for each place its room's
    doors lead to, after its chests: Kirby lands on the door it names, standing (the cell's floor under him). The hidden ways are
    doors drawn as on the DS (none); the goals and the starts aren't doors yet (the levels' structure). Checked in the GBA emulator
    (forced: Kirby held on the door's cell, Up pressed, his room and cell read after): all 562 doors in the USA ROM, each landing in
    its room on the door it names, none hanging (six needed a second try: an enemy's hit, the helpers pushing Kirby off a cell-wide
    ledge, a door in the air Kirby floats up to); every tenth in the European and Japanese ROMs (three needed a second try, enemy
    timing). The rooms' visited doors are the doors lists' (16 bits: the most places a room's doors lead to is 11).
  - **Their tile animations in KatAM** (`SqueakMap.Animations()`, `ConvertSqueakForeground()`): each 8-pixel tile of a cell showing
    an animated block is drawn at every frame (its other animated blocks of the same timing moving with it); a tile whose frames
    all look the same is an ordinary tile, the others get tiles of their own (no flips; all their frames on one palette, their
    colors packed as one set), a timing's in a row after the room's other tiles. KatAM's foreground animations (ForegroundInfo +8,
    +0xC, as NiDL's rooms' `TileAnimations()`) copy each timing's tiles in turn, a step per frame for its duration: 3 at most (KatAM
    runs 16). They take no tile room from the others: the same three busiest rooms merge tiles as before (none animated), the
    colors of 8 rooms merge a few shades more or less. Checked in the GBA emulator (forced starts, not a playthrough): shots a few
    frames apart in Prism Plains 1's first room (the grass), 2's first, 7's second (the glow) and Vocal Volcano 4's fourth (the
    flames) in the USA ROM, and those three in the European and Japanese ones; the 42 animated rooms the room sweep starts in ran 720 frames each (live, Kirby stays).
  - **Tutorial signs**: the three in Prism Plains 2 (its first, third and fifth rooms: "Tap to pop!", "Find treasure and move on.",
    "Mix bubbles!") are front-layer blocks without collision, 16 by 11 cells; left out (`SqueakSigns()`: no other room has such a
    block), the owner: KatAM skips them.
- **Stage files** (`map/*.mxp`, NDS RLE, type 0x30): `MaD\0`, 8 bytes, then 13 section offsets (`u32` from +0x0C): section 0 the width and height in tiles (bytes); 3-5 and 6-8 the tile grids (16 and 32 bits a tile); 9 the zone grid (a byte a tile), then a `u16` count and 8 bytes a point (`u8` kind, a byte, `u16` x, y and a size, pixels from the top), then the points' table; `.doe` the objects (an enemy list and an item list: see "Stage rooms, for porting them"). A stage's `.mxi` (`map/aNsM.mxi`) lists its parts' files (`MA` the map, `FB`/`BB` the graphics, `DE` the objects, `PL` a palette, `3D` the background); loaded, its table sits at the stage info (`*(*0x02229F1C + 0x40)` + 0x10, 0x1C bytes a part). To load a stage in the DS emulator, repoint that table's strings to the stage's files and let Kirby lose a life (the stage restarts from its first part): Mrs. Moley's stage was recorded this way from a save in 1-1.
- **Ability numbers** (`0x0203A9C4`'s, from the factory's cases): 1 Fire (Hot Head, Acchi, Flamer), 2 Ice (Pengy), 3 Spark (Sparky), 4 Beam (Waddle Doo), 5 Tornado (Twister), 7 Cutter (Sir Kibble), 8 Laser (Laser Ball), 9 Bomb (Foley), 10 Wheel (Wheelie), 11 UFO, 13 (Cupie), 14 Sleep (Noddy), 15 (Starman), 16 Sword (Sword Knight, Heavy Knight), 17 (Bun), 19 (Bio Spark), 20 (Boxin), 21 (Gaw Gaw), 22 Bubble (Bubble Head), 23 Metal (Metalun).

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

The stage's objects are 16-byte records: `*(*0x02229F1C + 0x90)` holds a `u16` count and, at `+4`, a pointer to them (the room's
`.doe` itself, loaded: its enemy list, below). A record: `+0` its index and `+1` its state, both written as the room loads (`0x02218F40`
on: 0 waiting, 1 alive, 2 gone, 4 not again till the area reloads; its `+0xB` bits 1, 4 and 8 pick the first), `+2` `u16` enemy ID,
`+4` `u16` x in pixels, `+6` `u16` y in pixels from the room's top (`0x0220A368` turns it into the room's y-up: `*0x02229F1C + 0x60`,
the room's height, `fx32`, less it), `+8`, `+9`, `+0xA` its parameter bytes, `+0xB` its flags. `0x0202EC50` spawns record `index`
when the camera reaches it: IDs below 0x40 through the factory (`0x0202ECAC`), 0x40-0x45 (the Squeaks) through `0x020A5048`.

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

## Where it's hit (0x0201322C)

The collision pass (`0x0201B79C`) takes an object's attackers only while `0x0201322C` says it can be hit: not with its +0x18 bit 2
(value 4: set by the grabbers' and holders' touch, Bio Spark's, Bun's, Gaw Gaw's, King Dedede's catch, cleared after), not while its
+0xEC safe timer runs, and only with a 0x102 box whose first entry's flag word (its record's `u16` at +2) lacks 0x8000. Its box: the
frame's own (`+0xF4` list, +0x128 on, count +0x24C), else its default (+8 on, count +0x248), which `0x02014218` copies from the frame its
sprite shows when loaded into slot 0 (animation 0's first: `LoadBoxes` twice, then the 0x102 and 0x202 lists copied). So a frame with
no 0x102 of its own is still hit, by animation 0's box, and a composite (`.mca`) whose parts' animations have none (Mecha-Kracko's) is
hit by it in every pose. The flag word's other bits: byte 0 the inhale it takes (0 any, 1 not the weak one, 2 only the strongest,
3 none: `0x02013050`), 1 and 2 grabs and kind 7 refused (`0x02126B8C`). Recorded per state on the DS (`hitline.py`: the test's
inputs each frame): Daroach hittable everywhere but his warp (flag), Bohboh everywhere but far off and in its dive (flag; its own
handler refuses while it holds Kirby and 32 frames after a hit), Mecha-Kracko in all its first form's states (by its default box),
Mrs. Moley as the port has her (untouchable in the back holes, down a hole, in her dives). Flagged animations of the ported characters:
Bio Spark's kunai (15) and hidden pose from its second frame (16), Buboo's bubbles' pops (3, 6), Squeakers' grab of the chest (chun's
48) and their bomb's blast (41), Tedhaun's grab and hold (2, 3), King Dedede's chew, spit and hurt (29, 30, 38-41), Cret's ball on
the ground (11: the port's already intangible), Mrs. Moley's back and hole poses, Daroach's warp (2, 3) and his bomb's blast (21).

**Daroach** (boss 0x3D, ported: `squeak/daroach_boss.c`): his body `0x02126F48` (vtable `0x02183680`: +0x3C his idle `0x02126658`,
+0x30 his hit `0x02126B8C`, +0x2C his touch `0x02126E00`; params `prmBB07Ml0`/`Sl0`, 400 HP), +0x1F8 his parameter (0 in a6s7s2, 1 in
a8s4s2: Dark Daroach, his palette object filled from `dancho01.cla` for both sprites, his moves' other branches; 0 again in the
arena's a9s7s2, 1 in a9s9s2), +0x200 his point (4 at first), +0xB8 his warps left, +0xB4 his wait. His sprites: slot 0 `dancho00`,
slot 1 `dancho01` (his wand: its raise 1, aim 3, lowering 16, `.mca`'s 0/1 its charge, 2/3 the beam: 16 parts out to 248 pixels).
States `0x02124DCC`-`0x021267EC` (in the port's comments); his zones `0x0212688C` (the stage's grid at Kirby; out: his point on
Kirby's side, Kirby's side's low point or 4, the low point across, 2 from the top points else 1). The beam's box is on its part 48
ahead (`dancho01.rca`'s 12: 31 to 332 ahead, 5 over to 4 under; Dark Daroach's 14: to 320, 15 over to 14 under); his own touch box is
missing in his charge's 7 frames of 8 and his beam's 2 of 6 (the composites' body's `.rca` frames shorter than its `.cea`'s). His
stars (`0x02124894`, vtable `0x02183640`: touch `0x02124844` sets +0x12C, bursting it; hit only by inhales) and bomb
(`0x02124290`, vtable `0x021835C0`; `0x0203A9C4`'s 9, Bomb; touch sets +0x12C, unused). `0x020B48E0`'s effect types: 0 `efc_cmn0`'s 3,
1 its 4, 2 and 3 its 7, 4 its 17, 5 its 2, 6 `efc_cmn1`'s 1. Recorded on the DS in a6s7s2 (`load.py`: the stage table repointed).

**Meta Knight** (boss 0x37, ported: `squeak/meta_knight_kss.c`): his body `0x02116C7C` (vtable `0x02181E80`: +0x3C his idle
`0x02116300`, +0x34 per frame `0x02116ED4`, +0x30 his hit `0x021166BC`, +0x2C his touch `0x02116ACC`, +0x24 a spat star on him
`0x021164F4`, +0x18 his flash and intro `0x02113018`; params `PrmBB01Ml0` (moves), `PrmBB01Sl0` (400 HP)). +0x228 his mode (1 a walk
at Kirby, 2 back, 3 a run, 4-6 following, 7-9 picking), +0x22C a follow's frames, +0x230 his safe frames (20 after a hit, 13 after
a dodge; they count down each hit too), his hit streak. His sprites: slot 0 `metaknight_cmn` (his body: 0x02022F00's slot 0), slot
1 `metaknight_ext` (+0xE4: his sword's big moves, its `.mca`'s composites 0-5: the thrust's landing, the beam's ready, the wave's
raise, the tornado's call, the tornado itself and its end). Both in `metaknight_cmn`'s colors; his cells mix his body's palette
(0) and his sword's (1, and 2-3 its glows) piece by piece, so the port keeps them whole in his sword's layer (`WholeCells`). States
`0x02113A48`-`0x02116468`; picks `0x02113190` (the port's `Pick`); his arena zones `0x02209C7C` (the stage's grid from the view's
middle less 144; values in the port's header). His things (`0x021122F8`, kind 0 beam `0x02111F90`, 1 wave `0x02111E44`, 2 tornado
`0x02111CFC`; their hit handler `0x021120B8` returns 0, nothing hurts them): the tornado's box (0x101, kind 6, 191 up) catches Kirby
(`0x02111A84`), holds and lifts him (`0x021119B0`: 2 pixels a frame, 140 over the floor at most, sparkles every 12 frames), and lets
him go hurt after 34 frames (`0x0211190C`: 10 HP). His stars: `0x0212389C`'s kinds 3 and 4 (16 pixels ahead or behind; brain
`0x0203A9C4`'s 16, Sword). His sword's reach: the kind 0 boxes of `metaknight_cmn.rca` and `metaknight_ext.rca` on his slashes'
frames. Recorded on the DS in his stage (`a7s7s2`, `load.py`: the stage table repointed; `rec_mk.py`, hooking the things'
constructors `0x021121D8`, `0x02112CD0` and the star's `0x021236A0`): his intro, walks, runs, jumps, slashes, his drop through the
thin floor, beams, wave, tornado (Kirby caught and lifted), thrust.

**Dark Nebula** (bosses 0x46 and 0x3F, ported: `squeak/dark_nebula.c`): 0x46 (`0x021376EC`, in a8s4s5) his coming: the dark
star (`darkZERO_cmn`'s 1) from 360 pixels left at 3.0 for 120 frames (`0x0213756C`), its spin three times (`0x02137430`), forming
(ext's 22, `0x02137338`), his eye opening (ext's `.mca` 1, `0x02137264`), looking (`.mca` 2, three loops, `0x02137188`) and glinting
(`.mca` 3, `0x021370B4`), 24 frames, then the stage switches to a8s4s6 (`0x0220A3EC`). 0x3F (`0x02122CD0`, vtable `0x02182FB8`:
+0x34 per frame `0x0212301C`, +0x30 his hit `0x021228C4`, +0x18 each frame `0x02122804`; params `prmLB02Ml0` (150 values),
`prmLB02Pl0` (30), `prmLB02Sl0` (300 HP)) his fight; his states `0x0211CD00`-`0x021226E8`. +0x1F8 his eye (`0x0211C4E0`, vtable
`0x02182EEC`; its +0x38 mode: 0 still, 1 shooting (`0x0211BEB4`: each frame 1 in 100 a star at 1.5 at Kirby's 24 pixels ahead,
else 4 in 100 at Kirby at 2.0), 2 half shut, 3 twinkle (its end: the form flash `0x021220D0`), 4 blink, 5-6 beaten), +0x1FC his
flames (`0x0211B98C`, following him), +0x200 his blink after a hit (60 frames), +0x208 a timer, +0x20C shots left, +0x210 his
side, +0x224 his form (1 fire, 2 spark, 3 ice; 0 none), +0x228 the next, +0x22C its delay (240), +0x230 moves before a change
(3-5). Spots are from the camera view (its left +0x1C, top +0x20 of Camera(0); y up): a8s4s6 is 288 by 240, the floor 80 up, the
view's top 144 over it. Start `0x021226E8` (the black star, cmn's 2, a form at random, 50 frames) → `0x021225B4` (shake, eye
twinkle) → `0x021220D0` (the screen whitens, `0x0203C174`, 60 frames) → `0x02121E4C` (his form: cmn's 7, 8, 6; clearing 60) →
idle `0x02121CB8` (40) → pick `0x02121A64` by form (PL[3]-PL[11]): fire his dashes `0x02121994` / shared `0x021204DC` / his
fireball `0x0211EBE4`; spark his wave `0x02121480` / shared / his bolts `0x0211E850`; ice his orbit `0x02120C54` / shared / his
pillar `0x0211E290`. Shared (rand >> 30): a row `0x02120220`, a rain `0x0211FC28`, four `0x0211F628`, a drop `0x0211F0EC`. Each
move ends in `0x021224F0` (the count; past it `0x021222E4` picks the next form); `0x02122804` then swoops him to (130, 33) in 20
frames (`0x0212222C`, `0x021221C8`), 30 frames (`0x02122164`), the twinkle and flash. His things (`0x0212FDC4`, 0x160 bytes, kind
at +0x130; their hit `0x0212FC08`): 0 star (cmn's 9), 1 the fireball (`0x0212F89C`: 50 HP, 5 frames safe after a hit; broken, 5
sparks), 2 its burst (`0x0212F448`: cmn's 12, its fire wall ext's `.mca` 0 50-69 frames, cmn's 19), 3 flames along the floor (cmn's
11 at 2.6; `0x0212F194`: the stage's vtable +0x90, `0x02216BD8`, sets the 16-pixel floor tile it's over burning: the stage's fire
tile, `map/25_f.bin`'s, hurting; recorded 42-52 frames each, 47 on average), 4 the fire's heat (`0x0212F138`: a grab box only,
harmless), 5 bolts (ext's 1-4 by direction), 6 the cage (ext's `.mca` 0), 7 the ice pillar (cmn's 20), 8 sparks (cmn's 11,
blinking, 20 frames). Recorded on the DS in his stage (`load_dn.py`: the stage table repointed to a8s4s6; `rec_dn.py`; `force2.py`:
a state written at frame 200, his spot and his things' each frame, screenshots), each move compared with KatAM's forced the same
way (`cmp.py`). The DS drops frames now and then (lag frames in its recordings, found by hooking his per-frame function); KatAM
holds every object still while Kirby is hurt (its hit pause), so its logs skip those frames.

**The dark star** (enemy 0x3E, ported: `squeak/dark_star.c`): its body `0x0211B680` (`EnemyBaseCtor(0x3E)`, vtable `0x021828F8`,
`darkZERO_cmn`'s animation 1; params `PrmLB01Ml0` 14336, 615, 16384, 615 (3.5 and 0.15: stored as 20.12, 615 not 614),
`PrmLB01Pl0` 120, 90, 64 px, 16 px, 4137, 10, 90, 60, 30, `PrmLB01Sl0` 10 HP). Its .rca has only a body box (0x202) on animation 1:
no attack, no hurt. It faces away from Kirby when made (`0x0201AE58`'s offsets are in the object's facing) and its speeds are in
its facing's frame (negative: backward). States: `0x0211B4C8` start (1.2 backward, 0.03 gravity), `0x0211B418` drift (120 frames;
on a floor 3.5 up, 0.15 gravity), `0x0211AED8` hop (3.5 up, 0.15; across `-(|Kirby's vx| + 0.88)`, 0 when more than PL[2] ahead;
in the route's end zone within 128 of its end point: vtable +0x3C), `0x0211AE38` hopping (a new hop on `vy <= -3.5`, within 12
of the route's line, `0x0211A97C`, or below Kirby), `0x0211B37C`/`0x0211B2EC` the end (still till `+0x264`, set by `0x0211AA50`
from the camera's +0x84 bit 8: the camera locked on a stage zone; then from the zone's point + 1), `0x0211ACBC` a leg (P0 where
it is, P1 and P2 the next two points; PL[6], PL[7], then PL[8] frames; sound 0x1C9 for the first 4), `0x0211AAE0` along it
(quadratic Bezier, t = frames / its frames; at its end, P2 and the next leg the frame after), past the last point
`0x0211AA74` (speed `(P2 - P1) / (2 frames)` per axis, through everything, till out of view: margins 1), then +0x3C again. The
camera is `0x02035BA4(0)` (`*(0x021867AC) + 4`). **Stage routes**: a stage file's section 9 is a zone byte per tile (the
`0x02209C7C` lookup: the zone's point), then a `u16` count and 8-byte points (`0x02209CC0`: `u8` index, a byte, `u16` x, y from
the top, size; 255 marks the end), then a point-by-point table. a8s4s3 (1664 by 240): points 0-7 at y 168 from x 272 to 1416, 8
the end (1596, 148), 9-18 the end's loop; its spawn (8, 68) at the left end; the camera locks on 1392-1648 when Kirby reaches
1388. a8s4s4 (288 by 1216): 0-7 zigzag up from (256, 944) to (140, 224), 8 the end (192, 64), 9-16 a loop of four legs; its
spawn (24, 964). Recorded in both (`kss/ds/load_st.py`, `rec_ds.py`: Kirby driven by a script).

**Composite parts' first frame**: a part's sprite is made on the cell it shows (`SqueakCreatePart`'s `cell`). KatAM's sprite init
(`EffectObjectInitSprite` → `sub_08155128`) queues its first frame's tiles at once; made on the character's cell 0, a part with a
small VRAM block got cell 0's tiles (Meta Knight's tornado: its 103-tile column) written over the blocks after its own, and a
sprite there that didn't change frames kept the garbage (his sword's layer, from his tornado's call to its end). Found by dumping
the upload queue (`gUnk_03002EC0`, 64 entries: source, VRAM, size) on the frame the layer's tiles changed.

## Test harness (DS emulator)

`py-desmume` with a save state after Kirby lands in Prism Plains' first room. The first stage record is rewritten (ID, x, y,
parameters) and the enemy spawns next to Kirby; breakpoints on its state functions and its fields above give its moves frame by
frame, to compare with the port in KatAM (mGBA, same fields in KatAM units).
