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
| 0x3E, 0x3F, 0x46 | darkZERO_cmn, darkZERO_ext | Dark Nebula (0x46 his coming, 0x3F his fight and his defeat, `0x0211DF2C`-`0x0211CDCC` with `ML[128]`-`ML[149]` and its sparks `0x0211CBAA`/`0x0211C9A8`: ported, `squeak/dark_nebula.c`); 0x3E the dark star Kirby chases through a8s4s3 and a8s4s4 (ported: `squeak/dark_star.c`); Dark Daroach is 0x3D's parameter 1 |

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
else damage, a knockout at 90% HP (`0x020DC5F0`-`0x020DB4C4`), else a flinch (`0x020DCDE8`). Its brace (an inhale, `0x020D7114`)
plays `doc`'s anim 21, so `doc_at1`'s arm and its part go: the port ends the part it holds (the claw's bomb, the cannon's smoke)
there and when it grabs a chest mid-attack (the owner: "One of the minibosses in the Squeak hideouts activated an attack and I
canceled, however the attack got locked in there"; seen in the GBA emulator, lair 0x547, Doc forced into its cannon, Kirby
inhaling mid-smoke: before, the smoke stayed beside it as it flew off and fired its gun; now it goes as it braces). Its things: `0x020DF354`, made by
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
`.cea`'s 18, no cell; `.rca`'s box kind 2, 128 across and 10 high, hurting Kirby, and Bohboh as it dives in: 20; ported as it
is: KatAM's rooms have no lava, so on a KatAM floor it hurts unseen across the floor's middle, the owner's pick: "Keep as it is"). `.mca`: 0 flying,
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
  - The ability items' looks (`item/copy`): the floating bubble (`0x0203A748`, its animation `0x021581E8`'s by the ability) and
    the pedestal (`0x0203A8A0`, vtable entry at `0x02166C24`; its animation `0x021582D8`'s by the ability: 1-9 0x21-0x29, 10-16
    0x2C-0x32, 17-26 0x36-0x3F, 27-29 0x33-0x35 (Sword in Fire, Ice and Spark), 30-31 0x2A-0x2B (Bomb in Fire and Spark), 0 0x41),
    animation n its cell n from 33 on: the ability's icon over a gold stand, the stand's pieces in the .cla's palette 0, the
    icon's in 1-3 (Parasol's, Wheel's and Fighter's in 0), more colors in all than one palette holds (KatAM draws the icon by a
    sprite of its own: `squeak/gimmicks.c`'s `SqueakCreatePedestal`). The icons: 1 Fire, 2 Ice, 3 Spark, 4 Beam, 5 Tornado, 6
    Parasol, 7 Cutter, 8 Laser, 9 Bomb, 10 Wheel, 11 UFO, 12 an orange lantern, 13 Cupid, 14 Sleep, 15 Starman's (a red wing), 16
    Sword, 17 Bun's (a blue note), 18 a black top hat, 19 Ninja, 20 Fighter's headband (Boxin's), 21 Animal, 22 Bubble, 23 Metal,
    24 Ghost, 25 a sign with a star, 26 Triple Star (12, 18 and 25 given by no enemy in the factory's cases).
  - **The ability room** (stage 9-1, `map/a9s1s1`, its .mxi's header: area 8, stage 0, from 0, one room; 9-10 the same room): a hall 92 x 16
    cells, two thin shelves (rows 5 and 8) over the floor (row 11), a bell tower over a building at the right end; its start
    (door 0, kind 6) at its left, its doorway (door 1, kind 4, flags 0x23: the goal) at (77, 7); its far view `d_viw_0B`, tiles
    `11_f`/`11_b`. Its 25 items are abilities 1-24 and 26 (no 25), each the pedestal: the top shelf's at y 80 (one at 76), the
    floor's at 176, Triple Star's on the roof at (1384, 112). Its map sets no music (+0x0B 0). Swapped into stage 1-1 (its .mxi,
    or its room's files) in the DS emulator, the room loads but its items don't come out: the DS shows them only in its Arena
    mode (not traced). Ported as the Squeak Squad ability room (`KatAMExpansionSqueakLevels.cs`'s `BuildSqueakAbilityRoom()`).
  - The spawn flags (`+0xB`, enemies and items), from the stage-object code: a record spawns while its state (`+1`) is 0, 5 or 6
    (`0x0220A368`); with bit 4 it spawns only once the stage's treasure event has run (`0x02218D00`: the stage's `+0x14` set to 1;
    in stage mode 2, Kirby carrying the stage's chest, everything spawns), without it only before: all 856 Squeakers, most of
    Spinni's, Doc's and Storo's records and the dark star's are these chase spawns (`0x02219028` brings out the Squeaks among them);
    bit 1 keeps a defeated one gone till the area reloads (state 4, else 2: `0x02218EF4`); bit 8 is on the mid-bosses and bosses.
    Ported as the chases (`squeak/gimmicks.c`, `squeak.h`'s "The chases"; the owner's design, not the DS's trigger): KatAM has no
    carried chest, so the stage's treasure event is a wall at the big chest's x (gimmick `0xF0`, the room's whole height; a Kirby
    crossing it either way turns the stage's chase on), and the Squeaks among the bit-4 records (`SqueakChaseOf()`: IDs 0x40,
    0x42-0x44; at most 6 a room, none by a door) are gimmicks `0xF1`, made as the camera nears them while the chase is on; the
    stage's mode 2 (all chase Kirby) is `SqueakObject.chase`: no chests taken, the angry pace, short waits. Every room of a chase
    stage carries its key (the gimmick list's header byte 1); a room of another key ends it. 37 stages have one; 6-5 has bit-4
    Squeaks but no big chest (its chase starts some other way on the DS, not traced), so it has no wall. The bit-4 chests (the
    chase's: 22, 5 of them Ability Scrolls, 5 Vitality halves, the rest food and 1UPs) are gimmicks `0xF6` (`ChaseChest()`,
    `AddChaseEntries()`): in no object list, the entry holds the chest's template's fields (item, number, mark, half flag, type),
    and the chest is made at its spot once the chase is on, once while the room's loaded (an opened one shows open, as
    `SqueakCreateChest` makes them). The map viewer's block lists them after the finale's ways on ("CC").
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
    Squeakers' treasure rooms' timings), 0x17, 0x18 (snow and wind streaks), 0x19 (lava sparks), 0x1A (a sound): the last three on-screen ambience (below).
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
      for the pillar, `0x02035FC4` with kind 2 for 22 frames, on the change back too: `0x02224E3C`, `0x02224D0C`; 0x2D for the
      spikes); 2 when it's over, the other animation held, the wait set: 3 for good (no wait) or 4
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
    with its 0x1000 flag) bursts it (sound 0x51), leaving what it holds (`0x0220A200` a food, `0x0220A2A4` an ability). Its states
    (vtable `0x02229594`, the state in `+4`, `0x022225E0` the switch; each returns the next): 1 the lid closing (animation 1) then
    2 closed once it ends (an effect, `0x0221F844`, with `0x01000012`), 3 the lid opening then 4: open again once it ends, a
    0x1E-frame timer (`+0x10`) and 5 till it runs out (none gets in meanwhile); 6 shaking then 7 closed again; 8 bursting. Only
    the empty ones open: the rooms' are 8-1-4's two (`00 FF`); the others hold something (one of them a chest: below).
  - The grass tufts (0xE; ctor `0x02222078`, vtable `0x02229500`; `yakumono/grass_A_S` and `grass_A_L`, `0x02227C28`'s 0x14-byte
    entries by its table's first byte's low nibble: 0 small, 1 large): its box (-1.0, 0) to (1.0, 3.0) small, (-2.0, 0) to (2.0,
    3.5) large; it takes messages 7 and 8 (`+0x48`, `+0x4C`). Its messages (`0x02221F90`): 5, a body moving through it (the
    box saved at +0x5C..+0x68: it rustles, `0x02221DFC`, its timer from `0x02227C1C` by size, sound 0x58, then back, 0x56/0x57 by
    size), 0xB an attack (its flags' bit 0: cut, state 5 `0x02221C64`), 0xC another. Cut (`0x02221C64`), it lets out what it
    holds (its table's second byte, not 0xFF: by the first byte's high nibble, 0 an ability `0x0220A2A4`, 1 a food
    `0x0220A200`, as the barrel's and the bubbles'), plays its cut animation (sound 0x59) and stays cut; one that held something
    grows back after 600 frames (`0x02221F58`: +0x6C counts to 0x258). Its looks (`0x02227C28`, by the low nibble): 0
    `grass_A_S`, 1 `grass_A_L`, 2 `grass_B_S`, 3 `grass_B_L`, 4 `grass_C_S`, 5 `grass_C_L` (the rooms use all six); animations
    0 still, 1 rustling, 5 its blades flying, 6 its stub shaking and going.
  - **What a grass tuft or a barrel holds is a record of its room's lists**: its second byte is an index, not an item ID:
    `0x0220A200(stage, i)` lets out record i of the room's item list (the item maker, `0x020972FC`/`0x020973AC`), `0x0220A2A4(stage,
    i)` record i of its enemy list (`0x02218E20(stage + 0x90's table, i, 0)`, the spawner). Such a record isn't out till then:
    1-2-2's grass at 604, 144 hides the Cherry its item list puts there, 1-5-1's eight barrels its eight items, 1-9-23's grass an
    enemy. The barrel's spot is its top (its box -1.0, -2.0 to 1.0, 0 in the DS's y-up world: 32 pixels down to the floor, 1-9-22's
    on a floor two cells under them), the grass's its feet.
  - **In KatAM** (`squeak/gimmicks.c`'s SqueakCreateGrass, SqueakCreateBarrel; the converter's `SqueakHeld()`): the records they
    hide are left out of the room's objects, each holder's template naming the KatAM object it lets out (a food KatAM's, an enemy
    as `SqueakObjects()` would place it, an ability item KatAM's ability star, object 0xA3, inhaled for the ability). An attack is
    told by KatAM's collision pass, which marks an object an attack reaches (flags 0x40000, the attacker in unk6C) before asking
    whether it hurts it (never, with flags 0x8000); a Kirby's plain touch is marked so too (his body's unk68 0x80, how a touch
    beats a weak enemy): a holder takes hits of KatAM's enemies' kind but masks his body out (unk5C 0x80, as a beaten enemy's), and
    counts a Kirby's own hit only with an attack bit over his body's (a slide, a dash). Walked through, a tuft rustles; hit, it's
    cut (KatAM's cutter sound, its blades an object of their own) and grows back 600 frames on when it held something; a barrel
    bursts (KatAM's block-break sound) and is gone. A chest a holder hides (7 in the rooms: 2-1-3, 4-1-6 (grass), 7-4-5's
    Ability Scrolls, 1-5-1, 4-3-8, 5-5-4's Copy Palettes, 2-5-5's Graphic Piece) is the converter's chest template handed to the
    holder (its type and item, mark, number and half flag) and let out as the chest itself, standing where the holder stood
    (`LetOutChest()`); a tuft that held one doesn't grow back. Hiding in an empty barrel: Kirby on the floor in front of it,
    pressing Down (read from his held keys by the barrel: his crouch takes his new-press bit first), is taken as KatAM's cannon
    takes him (`sub_08051C40`: hidden, his collisions off), held at the barrel's bottom; a direction shakes it (animation 4); A lets
    him out into KatAM's fall (`sub_08044EA8`) with 3 pixels a frame up; then 30 frames before anyone gets in again.
  - The icicles (9, `0x022233B0`, vtable `0x02229740`; `yakumono/icicle`: 0 hanging, 1 shaking, 2 falling, 3 shattering): state 0
    (`0x02223260`) waits for a body under it (a box from the body up 12 blocks); state 1 (`0x0222324C`) lets a falling icicle go,
    an object of its own (0x138 bytes, vtable `0x02229788`), made again once the last is gone. Its spot is its tip (the DS draws
    it above). In KatAM (`squeak/gimmicks.c`'s SqueakCreateIcicle): one object hanging over its spot, shaking 30 frames once a
    Kirby is under it (24 pixels across, 192 down), falling with gravity and hurting as it goes (a plain hit, KatAM's enemy-touch
    kind 0x80, strength 3), shattering on the floor (KatAM's block-break sound) and hanging again 180 frames on.
  - The falling rocks (8, `0x02221740`, vtable `0x02229348`; `yakumono/rock`: 0 appearing, 1 falling and spinning, 2 breaking): state
    0 (`0x0221E664`) does nothing; a message (kinds 0-2, `0x022216F4`) starts state 1 (`0x02221688`), which lets a rock fall (an
    object of its own, 0x138 bytes, vtable `0x02229390`, its look by the parameter's bits 0-2, `0x02227BD0`). The parameter's bits
    4-15 are frames before it comes: 8-1-8's five at 10, 30, 60, 100, 120 (a relay, started by a rope, messages all five at once).
    What messages them: 8-1-8's rope (gimmick 1) through a relay; 2-5-5's and 5-3-4's a random relay (0x17).
  - The random relay (0x17, `0x02224384`, vtable `0x02229948`): its parameter's u16s the least and most frames it waits, then up
    to 8 targets' indices (0xFF the end). It waits a random number of frames between the two (state 1, `0x022241A4`), sends its
    targets its message and waits again; a message (kinds 0-2, 4 as 2: `0x022242CC`) sets the kind it sends.
  - The camera areas (0x14, `0x02224B88`, vtable `0x02229A7C`): a body moving into its box (its u16 at +8: bits 0-6 its width,
    7-13 its height in blocks, 14-15 how many targets, +0xA their indices) sends its targets kind 2 (`0x02224AA0`) and, with +0xD's
    bit 0, sets the camera's bounds to its four u16s at +0 (x0, y0, x1, y1 in pixels). Not ported (the camera areas' bullet below).
  - Affine animations (rock's, ball_rock's): an animation whose head's third byte is set has its steps in pairs, the step drawn and
    then its angle (1/4096 turns), x and y scales (0x1000 for 1.0). In KatAM (`ReadSqueakCharacter()`, the gimmicks' graphics only)
    each step's cell at its angle is a cell of its own, drawn turned (`SqueakCanvas()`).
  - In KatAM: the falling rocks (`squeak/gimmicks.c`'s SqueakCreateRock: hidden till a message, then its delay, appearing 16
    frames, falling with gravity and hurting, a plain hit, breaking on the floor) and the random relay (the room task's
    RandomRelay). Checked in the GBA emulator (USA, 5-3-4, Kirby put in the room): its five rocks appear, fall and break on the floor
    in turn as the relay sends.
  - **The moving platforms** (gimmick 4, ctor `0x02223D78`, vtable `0x02229898`, 0x6C bytes; graphics `yakumono/floor`, a 48-pixel
    plank: look 0 its animation 0, look 1 its 3, still; 2 and 5 a dip). Its first word: bits 0-3 its look; 4-6 how it starts (the
    constructor: 0 and 1 its state 1, moving at once; 2 and 3 its state 0, waiting: the message handler `0x02223C6C` sends 2 and 3
    to state 1 and 1 to state 4; the message check `0x02223CA0` takes, in states 0-3, kind 6 for 2 when waiting and for 1 when
    moving, kinds 0-2 for 3 when waiting: 2 waits for a body standing on it, 3 for a switch's or an area's message); 7-9 what it
    does after its last leg (state 2, `0x02223AA0`: 0 stops, state 3; 1 starts its breaking, states 5-7: a shake `0x02227024`
    with the table `0x0222C5A0`, +2, -4, +3, -2, +1 pixels over 13 frames, then a fall `0x02227304` at 0.2 a frame, then a puff and
    it's gone; 2 and 3 go back to its first leg: the constructor adds, for 2, one leg back to its start, for 3, its legs backwards,
    so 2 goes round, 3 back and forth); 10-11 its speed (`0x02227DB0`: 0x1000, 0x2000, 0x4000, 1, 2, 4 pixels a frame); 12-14 its
    legs, each two signed bytes after the word (cells across and up), a straight run (`0x022270A4`) at its speed. Its box
    (-1.5, -0.25)-(1.5, 0.5) cells: 48 pixels wide, its top 8 pixels over its spot. Its states: state k runs vtable slot k + 3 (a
    gimmick's base: slot 3 state 0, which stays; each later slot returns its own state). The 38 in the 8 worlds start 0, 2 or 3,
    end 0, 1, 2 or 3, 1-3 legs (7-5-1's two of 80 and 75 cells across, at 4 pixels a frame).
  - In KatAM (`squeak/gimmicks.c`'s SqueakCreatePlatform, on the hammer peg's type as the other gimmicks; template unk14 the word,
    unk16-unk1A the legs): it moves as the DS's, leg by leg at its speed, and **carries** the Kirbys on it (KatAM has no moving floor:
    its solid objects are terrain cells). Its task runs before the Kirbys' (priority 0x1000 to their 0x3000): it moves, then each
    Kirby standing on it (on it last frame and still over it, or whose feet came down through its top since his last update) moves
    with it, his feet (his y + his box's bottom, unk3F) put on its top and he's marked on the floor as KatAM's own landing does
    (`Macro_080063B0`: unk62's 4, his fall speed and his movement override's y 0, flags 0x1060 cleared), so his update walks, ducks
    and jumps from it as from the ground. Checked in the GBA emulator (USA): 8-1-1's waiting platform (Kirby lands on it, his feet on
    its top; it starts the next frame and carries him up 18 cells at exactly 2.00 pixels a frame for 144 frames, then stops with him
    on it); 6-1-4's back-and-forth one (exactly 1.00 a frame each way, his height fixed); walking on it as it moves (his own walk
    and its move added), jumping off (he leaves it at once). Not checked: helpers riding, a breaking platform (7-5-1's, 1-9-17's),
    one started by a switch (6-2-1's, 8-1-2's), the camera on a long ride, Europe and Japan in the emulator.
  - **The thunder clouds** (gimmick 5, ctor `0x02220C00`, vtable `0x02229230`; `yakumono/thunder_cloud`: 0 resting, 6 flashing white,
    4 a green burst, the rest sparks and puffs): a moving platform's first word (the same bits: look, start, end, speed, legs) and
    legs, its legs from its 6th byte (`ldrsb` +6, +7 on), the same straight runs (`0x022270A4`) at its speed table's (`0x02227BA0`)
    speeds; its box (-1.5, -0.5)-(1.5, 0.5) cells. Its zaps: its +0x5C phases 0-6, moved on as its animations end (slot 2,
    `0x02220A24`, a jump table: 1 makes it harmful, harm kind 0x0B and strength 8 at +0x48 and +0x4C, 6 back to rest), a sound
    (0x11F) on 1, 3 and 5; its two u16s (+2 and +4, 30 and 30 in every room) the frames it rests and zaps. In KatAM
    (`squeak/gimmicks.c`'s SqueakCreateCloud, object 0x93): a moving platform with the cloud's look (it carries Kirby), resting
    unk1C frames, then zapping unk1E frames flashing (animation 6) and hurting a Kirby touching it, its riders too (a plain hit).
    **Corrected** (read again from its code): its look's animations (0x02227BAC) are 0x3000000-3, its .mca's composites, not its
    plain animations: 0 resting (147 frames: the cloud bobbing, its anim 0, with three sparkle anims 1-3 at offsets), 1 the zap
    (12 frames: anim 4, the big green burst, ±48 pixels, under anim 5, sparks), 2 the shake before breaking (17 frames: anim 6, the
    cloud flashing white, with sparks 7-9), 3 breaking up (47 frames: anim 13 flickering out, puffs 10-12). Slot 2 (0x02220A24)
    runs as an animation's loop ends: phase 1 starts the zap (composite 1, harm kind 0x0B, strength 8), phases 2-5 count its loops
    (sound 0x11F on 3 and 5), 6 back to composite 0 and phase 0, +0x60 reset. Phase 1 is set only by its message handler
    (0x02220ADC, kind 0xB, an attack: 0x02220B0C sets 1 while resting, 2 while zapping): it has no timer; its two u16s (30, 30)
    aren't read by it. Its harm comes from yakumono/thunder_cloud.rca's boxes: the zap's ±40 and ±42. States 4-6 (0x02220854,
    0x0222080C, 0x022207EC) are its break: composite 2 for 30 frames, then 3 (sound 0x29), then gone. In KatAM
    (`KatAMExpansionSqueak.cs`'s SqueakFlattenComposites: the 4 composites as anims 14-17, frame by frame, each part's cell's pieces
    merged into one cell): resting until struck (Struck(), as the grass), the zap after its resting loop, 5 of its loops, its box
    ±40; its break the shake, then the puffs. Checked in the GBA emulator (USA, Europe, Japan, 3-5-1): untouched it never zaps;
    struck by the Sword, the clouds zap as above, hurting Kirby on one.
  - **The ropes** (gimmick 1, ctor `0x022249C8`, vtable `0x02229A2C`): no sprite (the rope is drawn in the terrain: KatAM ladders
    here), an area: width from `0x02229A20` by the first byte's low nibble (pixels 6-9 of its cell), the second byte its length
    in pixels, down from its spot; the first byte's high nibble how many targets, from the third byte (0xFF none). A cutting
    attack on it (message 0xB with the attack's flag 8, `0x02224960`) cuts it (state 1, `0x022248A0`: sound 0x48 or 0x49 by its
    look) and messages its targets: the floor it holds (3-3-6, 3-3-7), 8-1-8's rock relay. 25 in the ported rooms.
    **The rope floors** (gimmick 3, ctor `0x022219C4`, vtable `0x022293F8`, looks at `0x02227BF4`): a 48-pixel plank (the
    platform's box) hanging from a rope; its first byte's low nibble its look, bits 4-6 1 when it falls once a body stands on it
    (6-2-1's), else on its rope's message; it shakes, falls and breaks. 16 in the ported rooms.
    Cut, the DS's rope goes too: the stage's `0x022164C0` (the stage object's vtable `+0xA0`) walks the cells up from the cut, then
    down, while the blocks grid's id is a rope's (its `0xC0` bits `0x40`), each a short fade (`0x022124F8`: tiles 0x10-0x14, two
    frames each) and then cleared (`0x02215830`), one further out each frame. The rope is drawn on the front layer (blocks
    0x260-0x262 over the blocks grid's 0x38 0x40-0x42 cells: 3-3-6, 3-3-7, 4-4-1's chains 0x280-0x282).
    In KatAM (`squeak/gimmicks.c`): a rope is an object hidden (object 0x92, SqueakCreateRope; template unk14-unk19 its 6 bytes),
    its box over the rope (2-14 pixels across its cell, its length down), taking attacks as the grass tufts do (Struck()) but only
    a cutting one, as the DS's flag 8: a Kirby's with Cutter, Sword or Master, or Needle (the owner: "Needle counts as a cutting
    ability."; checked in the GBA emulator: Needle Kirby cut room 0x488's rope and its platform fell) (not his inhale or a plain
    hit, which dropped the plank before; the owner: "if I hit the string with a cutting ability, it stays there"); cut once, it
    plays the Cutter's sound, messages its targets (kind 1), and its cells go from the cut's row out, a cell each way every 2 frames (each one's block
    record, `SqueakRopeCells()`: the cell drawn without its front layer, air), so the rope's look goes and it can't be climbed. Not
    inhaled (unk5C 0x20). A rope floor is a moving platform with no legs whose end is
    the break (its word: the look, start 2 for a rider's weight or 3 for a message, end 1), so it carries Kirby, then shakes,
    falls and breaks with the block-break sound on the first solid cell under it (or 160 frames on). The platforms and clouds
    hold every body, as the DS's floors do (the owner: "Entities go through the movable platforms, which doesn't happen in Squeak
    Squad"): KatAM's own objects ride them as Kirby does (PlatformCarryObjects(): those on the terrain, coming down onto the top
    or on it last frame), and Squeak Squad's bodies, which move themselves, land on them in their own move (squeak.c's SqueakMove(),
    SqueakPlatformLanding()); they aren't inhaled (unk5C 0x20: the owner swallowed one). Checked in the GBA emulator
    (USA, Europe and Japan, 3-3-7, Kirby with Sword put over its two planks): his Sword cuts the left rope, its plank shakes, falls
    and breaks on the floor under it, the right plank stays with him on it; (USA, 6-2-1) standing on its floor drops it onto the
    spikes, where it breaks. Not checked: 8-1-8's rope starting its rock relay, 3-3-6's.
  - **The rolling boulders** (gimmick 6, ctor `0x0221EE04`, vtable `0x02228FE8`; `yakumono/ball_rock`: animation 0 the rock, a
    64-pixel ball, 1 and 2 its pieces spinning, affine): its look by the first byte's bits 0-3 (table `0x02227AA0`, one), its spot its
    middle, its box ±2 cells; bits 4-5 0 rolls at once (the constructor's state 1), else it waits for a message (state 0: 2-4-1's
    quake triggers, 4-3-2's relays, 4-6-7's trigger areas). State 1 (`0x0221ED00`) makes a speed ramp (`0x02226CE4`, its update
    `0x02226D0C`): bits 6-7 pick from `0x02227AB4` (0x2F frames to 2 pixels a frame, 0x40 to 3, 0x61 to 4). State 2 (`0x0221EAE0`)
    is a body of the DS's physics rolling on the terrain (the DS turns its one frame as it rolls): on landing a quake (`0x02035FC4`,
    4, 16, 2), hitting a wall (its body's flags 0x400, 0x800) a bigger one (7), sound 0x4F and it breaks (`0x0221E7D8`: its pieces),
    the dust round it while it rolls (flag 2, not 0x1000); it hurts with harm kind 0x0E, strength 8 (`0x0221ED6C`). The ones that
    roll at once (2-4-2's two, 4-3-2's, 8-2-4's) sit in closed pockets 4 cells wide, a floor under them: a body at rest stays put.
    18 in the ported rooms: 2-4-1, 2-4-2, 4-3-2, 4-6-7, 8-2-4, and the boss stages' 7-7-4 and 8-4-4.
    In KatAM (`squeak/gimmicks.c`'s SqueakCreateBoulder, object 0x91; template unk12 its index, unk14 its word): KatAM's own terrain
    collision (ObjectMain's `sub_0809D8C8`, a box of ±28 pixels), gravity, rolling down the slope under it (the ground looked at 16
    pixels either side, inside its round, lower on one side), on along flat ground the way it went (a boulder that hasn't gone yet
    stays, as the DS's), its speed ramped up as the DS's; turned as it rolls (animation 3, `SqueakRolled()`: animation 0's cell
    turned a sixteenth a step, the step picked by how far it went, anticlockwise rolling left); on a wall it breaks (KatAM's
    block-break sound, a puff, a piece flying off spinning: animation 1); it hurts as the other gimmicks do (a plain hit). The
    quakes are left out (the screen shake, item 10 of the open requests). Checked in the GBA emulator (USA): 2-4-1, Kirby walked
    down to its quake trigger: the boulder waiting on the slope rolls down at the DS's ramp, turning anticlockwise (measured
    between frames 2 apart), hits Kirby (OUCH, a bar of health) and bursts on the wall at the bottom; 4-6-7, Kirby walked into its
    trigger area: its boulder starts and rolls down at exactly 2.00 pixels a frame; 2-4-2: its boulder stays in its pocket (above
    the camera's top). First it rolled toward Kirby on flat ground, out of its pocket, and burst on the pocket's wall; then it
    took a wall it touched for higher ground and turned back: both fixed. Europe: 2-4-1's the same (it rolls down and hits Kirby).
    Not checked: 4-3-2's and 8-2-4's, Japan's boulders in the emulator.
  - **The candles** (gimmick 0xA, ctor `0x022237A4`, vtable `0x022297E8`; `yakumono/candle`: 0 lit, its flame flickering in a glow, 1
    out): 4-3-7's, 4-3-8's and 8-1-6's, 8 in dark rooms. Its box ±0.5 cells across, 1.5 up from its spot. Its message handler
    (`0x022236A0`): a burning attack (the message's flags 0x901) lights it (state 1, `0x02223688`) for the parameter's bits 4-15
    frames (400-540 in the rooms; again while lit: as long again); a blowing one (flag 2) puts it out with a puff (`0x0221F844`, kind
    5). Lit, it counts down (`0x02223664`) and goes out (`0x02223514`). The first one lit while none of the room's is (`0x02223588`,
    the room's others counted by `0x0222361C`) calls the room's far-view fade (`0x02209F94`'s +0xAC, its vtable +0x10, `0x0220A12C`)
    with 30 frames and fades the stage's light (+0xA8) to full (0x1000) over 30; the last one going out calls the fade with -30 and
    fades the light to 0x404 (a quarter). Read in the DS emulator with 4-3-7 loaded on its own stage entry (stage 1-1's `.mxi` replaced
    by 4-3's with room 7 first): the light starts full (the light record's 255, 255, 255), the fade object is the plain one (vtable
    `0x02228CAC`, a no-op), and the BG palette is the room's `PL` palette, `map/pa_19.pal` (255 of its 256 colors): the candle rooms'
    tilesets (31_*, 32_*, 33_*) hold no colors (each palette's color 0 alone), their `PL` palettes (pa_19, pa_1C, pa_22, kind 2) are
    their colors, and these rooms aren't dark on the DS (4-3-7: warm orange bricks). The converter takes them as the tilesets' colors
    there (`KatAMExpansionSqueakRooms.cs`'s `SqueakRoomTileset()`; the rooms were black before). In KatAM (`squeak/gimmicks.c`'s
    SqueakCreateCandle, object 0x90; template unk14 its word): lit by an attack of a Kirby with Fire, Burning or NiDL's Light (his own,
    or one he let go: its parent; Light's flash with the NiDL ROM), with the Fire attack's sound, for its frames, then out; lit, it
    isn't culled off screen. The far view's light is the room's gimmick `0xF7` (`CandleLight()`, added in every room with candles):
    the far view's colors (BG palettes 6-13) times the light, which fades as the DS's. Checked in the GBA emulator (USA, 4-3-7, Kirby
    put by its first candle, his ability set by a RAM write): Fire's breath lights it, the far view stays full, and 400 frames on it
    goes out and the far view dims to a quarter over 30 frames; Light's flash lights it the same; Europe and Japan: Fire the same.
    Not checked: Burning, 4-3-8's and 8-1-6's candles, a second candle lit while one burns, the DS's blowing attack putting one out
    (not ported).
  - The camera areas (0x14) hold the view and message food bubbles (2-3-5's, 2-6-2's, 3-2-4's, 4-4-6's targets are bubbles,
    which the port places as food already). Their bounds' y is the DS's world's, up from the room's bottom (2-3-8, 512 pixels
    tall: the area over its top screen holds y 304-496, its top screen from the top). Ported (squeak/gimmicks.c's CameraArea: the
    Kirby's view's bounds' targets, unk_S32Vec2_7C and 84, KatAM easing his view to them; let go out of the bounds), not in rooms
    whose fight's arena holds the camera.
  - Gimmicks 0x18, 0x19 and 0x1A, identified: on-screen ambience only. 0x18 (ctor `0x02226384`, vtable `0x02229DD4`; 33, in Ice
    Island's rooms and 3-4-1) draws `yakumono/snow` and `yakumono/stream` streaks while its box is on the screen (the camera's,
    `0x02035BA4`); 0x19 (`0x02225E9C`, vtable `0x02229D30`; 171, along Vocal Volcano's lava and 2-5-2/3, 3-2-2/3) the lava's sparks
    (`yakumono/spark`), no box to touch; 0x1A (`0x0222684C`; one, 4-3-2) plays a sound every so often (its low nibble the sound,
    table `0x02227FB4`, its second byte how many times). None changes the way through. 0x19 is an effect emitter (its state
    0x02225D14): its first byte's low nibble its look (0x02229CBC: 0 yakumono/bubble, 1 yakumono/spark), animation 0 (0x02227F90);
    when its wait (+0x5C) runs out it plays that once at its spot if the spot is in the camera's view (0x0221FA50), then waits
    (rand's top 2 bits + 1) x 100 frames. Ported (squeak/gimmicks.c's Emitter, squeak.c's SqueakEffectInRoom). 0x18 is a
    streak emitter (0x02225F80): every 10 frames (+0x60) near the view, at a random spot along one side of its box (its word's bits
    4-6: 0 the top, its byte 2 cells across; 1 the bottom; 2 the left, byte 3 cells down; 3 the right), its look (bits 0-3:
    0x02229D78, 0 yakumono/stream, 1 yakumono/snow) and one of 3 animations (0x02227F9C, at random), moving at a speed its
    constructor sets (0x02226384: a table at 0x0222C75C by look, side and speed (its bits 7-9), 8 bytes an entry: the stream's
    (0, -4/-6/-8) from the top, (0, 4/6/8) from the bottom, (4/6/8, 0) from the left, (-4/-6/-8, 0) from the right, fx32 a frame,
    y up; the snow's all (0, -1): it falls 1 pixel a frame whatever its side). The spot: the top or bottom, a random cell's middle
    across (byte 2 cells), the left or right, a random cell's middle down (byte 3 cells); the animations (0x02227F9C): the stream's
    0, 1, 2, the snow's 0, 1, 0, picked at random; the near-view check (0x0221FA50) a box from the camera's spot 320 wide and 256
    down. The streak goes to the shared effect queue (0x0221F89C kind 6, 0x02039B28): both looks' animations loop (stream 14-16
    frames, snow 28 and 48), so the effect system ends them; not read. Ported (squeak/gimmicks.c's Streaks, squeak.c's
    SqueakDriftInRoom: ended once seen and off the screen, or after 255 frames).
  - **The quake triggers** (gimmick 0x13, ctor `0x022240E8`, vtable `0x022298F8`): its second u16 its box (bits 0-6 blocks across,
    7-13 down, as a trigger area's), bits 14-15 how many targets, its bytes 4 on their indices. A body moving into it (message 5) or
    a switch's message (kinds 0 and 1: `0x022240A8`) sets it off once (`0x02224034`): the screen quakes (its first u16's low 4
    bits the kind, `0x02227DFC`, the rest the frames), sound 0x68, and its targets get kind 2. On the DS a rolling boulder sets
    4-3-2's off, rolling down its shaft (the relay it messages sets off the chain to the door). In KatAM (the room task's Quake()):
    a Kirby within 64 pixels of its box, or a message, sets it off; the quake is left out.
  - Gimmick 0x12 (ctor `0x022202E4`, vtable `0x0222913C`): its first two bytes a box (blocks across, up); a body moving in or a
    switch's message fades the room's colors (its u16 at +2 the frames, bytes 5-7 a color /255: `0x02209F94`, `0x0220A12C`).
    7-3-1's whitens its top. `0x0220A12C` calls the stage's +0xAC object's vtable +0x10 with the frames: +0xAC is the far view (the
    room's `3D` file, `0x0221AD54`: by its resource's count a plain one (vtable `0x02228CAC`, +0x10 a no-op), type 1 (`0x02228C90`,
    a no-op too) or type 2 (`0x02228C74`, `0x0221A84C`): a list of 17 palettes, entry k (1-16) each channel (A x (16 - k) + B x k)
    / 16 of its palettes at +4 and +0x18; +0x10 is `0x0221A75C` (frames > 0: count up, else down), its update `0x0221A774` showing
    entry counter x 16 / frames). The resource is the room's `PL` file (the `.mxi` entry's +0x14; +0x18 is `3D`): type 2 is a
    `map/pa_NN.pal` (u16 2, then 256 colors: the boss rooms'), its start (+4) the colors in use (`0x02218210()` + 0x18), its own
    (+0x18) the file's; no `PL` (2-3-1, 2-8-1, 7-3-1), a plain one: no fade. The gimmick's trigger (`0x02220224`) also fades the
    far view's light (the stage's +0xA8, `0x0221992C`: +0x24-0x2C its color, 4.12, +0x30-0x38 from, +0x3C-0x44 to, +0x20 frames)
    to its bytes 5-7 / 255; that light starts at the room's light record's bytes 3-5 / 255 (the map object's +0x64; read in the DS
    emulator: 255, 255, 255 in 2-3-1 and 7-3-1). The gimmick's color is 255, 255, 255 in all three rooms: full to full, so nothing
    dims on the DS. The candles (gimmick 0xA) use the same light: the last one out, to 0x404 (a quarter) over 30 frames; the first
    one lit, back to full (ported: gimmick `0xF7`, `CandleLight()`).
    Ported (squeak/gimmicks.c's Light: the far view's colors start at the DS's start, full, and step up the same way).
  - **What only Fire or an unported gimmick opens** (the owner: every room reached), KatAM's star blocks, any attack breaking them
    (`SqueakRoomCollision()`): the cells Fire burns (the blocks grid's flag 0x800 under top byte 0x38: 4-2-3's start walled in by
    logs, 6-2-5's); the frozen blocks (the blocks grid's ice, top byte 0x28, drawn with its tile 0x220 or 0x221 over a main cell
    that isn't solid: 6-1-4's over its door; melted, what the main grid has there); the blocks grid's block 3 over a solid main cell
    and the covered (0x400) solid cell under one (4-4-4's two shafts down to its doors, 35 rooms' small floors: not identified on the
    DS, a hidden breakable block by where they stand); and the flagged groups (0x1000) nothing ported sets off, with no bomb block,
    switch or stake (what the boulders and gimmick 0x13 break: 4-3-2's wall, 4-6-7's floors over its shaft, 8-1-10's, 8-2-4's; the
    treasure rooms' boards of 2 cells over their doors stay). A burnt cell or a group's is drawn without its main layer once broken.
    Checked in the GBA emulator (USA, 4-2-3, plain Kirby): inhaling the burnable cell by the start takes it, the star he spits
    breaks the log column, and he walks out of the box.
  - A DS door's room-and-door names the door it lands on by its place in that room's list: 7 rooms number theirs out of order
    (3-1-5's 0, 0, 2), and 3-1-4's way into 3-1-5 names its second (looked up by number, it was lost and 3-1-4 had no way out).
  - Section 11, the doors: `u16` count, then 10 bytes a door: `u16` its number (the collision's `0x80 + n` marks it), `u8` flags,
    `u8` kind, `u16` x, y from the top (the middle of the cell Kirby stands in: a floor under it, but for a door high in Gao Gao's
    room in Nature Notch 3), `u8` the stage's room it leads to (0-based, the `.mxi`'s order) and `u8` that room's door (its number;
    `0xFF`: none). Kind 6 is where Kirby starts (most doors lead to one), 4 a door, 0x42 a dark doorway, 0x40, 0x41, 0x80, 0x82,
    0xC1 and 5 other entrances: all entered with Up, none walked into (checked in the DS emulator, Kirby put on one of each kind
    and walked through it both ways, then on it pressing Up; 0xC0 and kind 0 are only in the test stages 1-8, 1-9, 2-9). Flags:
    0x10 and 0x90 ways on, 0x23 with no room the stage's goal, 4 a hidden way (46 of them: the DS draws no door there and Up does
    nothing, no switch in its room; they lead to the stages' gold-pile treasure rooms, whose doorway leads back to the stage's
    start), 2 others (0xC1's). 562 doors lead to another room of their stage; 4 to rooms Nature Notch 6 lists but never had.
  - The Squeaks' hideouts: after a stage's goal, beside its gold-pile room (the hidden way's), the same four 18 by 15 rooms in
    every stage that has them, each with an arrival (no room) and a doorway (flags 0x90, room 0 door 0: back where Kirby came
    from): the Squeakers' (6-9 Squeakers, records with flag 4 or 5), Spinni's, Storo's and Doc's (the Squeak with a few
    Squeakers). The stage's thief is the Squeak whose record has flags 0xC (4, a chase record, and 8): its hideout is the one the
    DS's lair leads to. The lair doors' look (the nests) is in the rooms' own front layer: every room's front block table (map
    table 1) holds a nest, 5 blocks across and 3 down, blocks 0x110-0x114 (the flag and the top), 0x130-0x134 and 0x150-0x154
    (its feet; a second copy from 0x115, the same), drawn with the room's front tileset (`map/NN_f.bin`, its tiles about 672-735
    in 04_f): each front tileset has its world's (a stump in 04_f/28_f, a grassy mound in 00_f/01_f/08_f, rock piles, castles,
    igloos, a submarine in 0B_f/0C_f, factories in 1A_f/21_f/22_f/24_f, UFOs in 25_f-27_f, a silhouette in the dark stages'
    31_f-33_f; none in 20_f, 29_f, 2C_f, 2D_f, 3B_f, 3E_f, 3F_f). No map places them: the game writes them in when a hidden lair
    door shows (not traced in its code). The hideouts' black doorway is their third layer (grid 5: block 604 over 796, the doorway
    cell and the one above, the only blocks there). The port: KatAMExpansionSqueakLevels.cs' Squeak Lairs,
    KatAMExpansionSqueakNests.cs. 3-4-5, 8-3-6 and 8-3-7 are no hideouts: sky rooms the warp stars' rides pass through (an
    arrival only).
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
    in every room and fight measured, the title and pause screens too); `gNidlRoomTops`' `ROOM_DS_BOTTOM` stops the camera 32
    pixels over the room's bottom, as the DS's (KatAM's 40 are for its rooms' padding, which these rooms don't get). Measured in the DS
    emulator (Prism Plains 1's first stage, 1600 x 240): the DS's screen shows y 16-208 and holds there with Kirby at 216 (its camera's
    centre, `*(*0x021867AC + 4)` `+0x14`/`+0x18`, kept 104 under the screen's top and at most the stage's height less 120), so its
    last two rows (filler in many stages) and its top 16 pixels are never shown, 16 at the sides too. Doors
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
  - **Dirt Animal Kirby digs** (the stage's `+0x9C`, `0x02216528`): the blocks grid's solid cells (low half `0x4000`) of families 1-3 (bits 27-29: 0x08, 0x10, 0x18); dug, the cell is cleared to the main grid's. In KatAM (`SqueakDigCells()`) they stay KatAM's solid (0x0D) and each room keeps a list of block records that redraw them bare (`gSqueakDigRooms`); Animal's digs clear them (`abilities/squeak_moves.c`). Rooms with dirt: 0x40E (130 cells), 0x448 (563), 0x449 (652), 0x500 (33), 0x50A (32), 0x524 (16), 0x525 (439), 0x585 (72), 0x5CE (670); none of them had to merge tiles for the bare looks.
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
    both grids; 1-1-4's grass stays), and the switches chest.c's SqueakCreateSwitch (KatAMExpansionSqueakRooms.cs): pressed by a
    Kirby touching it or by an attack on it, a thrown one too (the owner: "If I throw a knife at them, they do not activate which
    this doesn't happen in KatAM"; squeak/gimmicks.c's SqueakCreateButton(): the item holders' kind 1 | 0x80, harmless, unk68 0,
    and the holders' flags, not the hammer peg's 0x18B00, with which KatAM's collision pass never paired an attack with it; not
    inhaled, unk5C 0x20; checked in the GBA emulator, room 0x419's switch: a Cutter thrown from 48 pixels presses it, Kirby
    inhaling at it presses it and leaves it there).
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
    treasures the owner listed (Ability Scrolls, Spray Paints, the Secret Map) are to hold those (the Spray Paints and the Secret
    Map's pieces do: below). The chase spawns' 22 chests (flag 4) come out with the chase (gimmick `0xF6`, above).
  - **The Ability Scrolls' icons** (treasures 28-50): `collection/skill`, 23 cells and animations, one a scroll, each a 16 by 16
    sprite in its own palette row (5 rows, 80 colors); taken in the treasures' order (cell n is treasure 28 + n: not checked against
    the DS's own lookup, which isn't found). A scroll chest's popup shows it (`chest.c`'s `NidlScrollPopup`): KatAM's popup task as it
    is (`sub_0800B7A4`), its sprite the scroll's frame, the palette it reads each frame (the chest's, for a map) the scroll's slot for
    that call, flipped to face right as the DS's (the frame builder mirrors every Squeak Squad cell). The result screen's
    `result/icon` has scroll icons too (cells 21-29, nine), not one a scroll. Checked in the GBA emulator: room 0x455's chest (scroll
    3) opened with Up, its popup the DS's icon of scroll 3.
  - **What a scroll does** (the DS): owned scrolls are a mask in the player's info (`+0x2C`, built by its constructor `0x02020CF0`
    from the save's treasure bits: save at `*(0x02185BA0)`, treasure k bit k % 16 of the u16 at `+0x100 + k / 16 * 2`; scroll n is
    treasure 28 + n), asked by `HasScroll(info, n)` (`0x02020AD8`), always with a constant scroll number, from 40 places. By number:
    Fire 0 (`0x0202FDD0`), Ice 1 (`0x02030F6C`), Spark 2 (`0x02088444`), Beam 3 (`0x02088B44`), Tornado 4 (`0x020BA8F8`,
    `0x020BAD4C`, `0x020BB03C`), Parasol 5 (`0x020B8F18`, `0x020B90A8`), Hammer 6 (`0x020944B8`, `0x02094A88`), Cupid 7
    (`0x020BE864`), Cutter 8 (`0x020BA1C8`), Laser 9 (`0x02099E94`), Sword 10 (`0x0201FC50`, ability mixing), Wheel 11 (`0x02092B24`,
    `0x0209326C`), Hi-Jump 12 (`0x020BF8E8`), UFO 13 (`0x020BD560`), Sleep 14 (`0x020BF294`, overlay 1 `0x0220E612`), Bomb 15
    (`0x0201FC50`, ability mixing; `0x020B2D24`), Ninja 16 (`0x020C8618`, `0x020C8ED4`, `0x020C91E0`), Fighter 17 (`0x020C5828`),
    Throw 18 (`0x020C0E00`), Magic 19 (`0x020C2CC0`), Animal 20 (`0x0208B7F0`), Bubble 21 (`0x02089700`, `0x02089B90`,
    `0x02089C64`), Metal 22 (`0x020CAF44`). All 23 are in the stages' chests (one each). Each ability's parameters are its
    `param/PrmKirby<Ability>.kgb` (the words after a 4-word header, at the player's `+0x394`).
    - Fire, Ice: each puff of breath the ability shoots (its own object, `operator_new(0x130)`) takes its speed from the
      parameters; with the scroll, Up held adds words 4 (-6144, up 1.5 pixels a frame) to its height speed, Down word 3 (6144),
      forward word 5 (0). The DS's breath doesn't bend without it. Ported (`ROM Expansion/scrolls.c`'s `ScrollBreath`).
    - Spark (`0x02088444`, every frame; `PrmKirbySpark.kgb`'s words 8, 8, 16, 16, 360, 0, 16384, 24576): each new D-pad press
      (input `+0x30`, any way; not B) adds word 3 (16) to the charge (`+0x3A0`) and sounds `0xB0` or `0xB1` at random (half and half);
      it falls 1 a frame. Past word 0 times word 3 (128): level 1 (`+0x3A4`), the charge back to 16, and the aura object
      (`0x02087EC4`, `0x02087AA4`: `spark_ext`'s anim 30, sound `0xB0`, no hit box); with the scroll, past word 1 times word 3 (128):
      level 2 (its `.mca` composite 3, sound `0xB1`, hit box 5 every 3 frames: `.rca` 43), back to level 1 with the charge at 128
      when it runs out (level 1 doesn't fall). Hurt, the water or no control (its flags) end the level. B (`0x020882F8`) takes the
      level (0 for level 2 with forward held); after the start-up its attack (`0x02088130`, `0x02087FC0`): level 0 composite 0
      (sound `0xAF`, Kirby's base attack), 1 composite 1 (`0x51`, hit box 25 every frame, two shapes in turn: `.rca` 41), 2 composite
      2 (`0xB2`, 40: `.rca` 42), which after 30 frames goes on as composite 0 while B is held; the attack ends charge and level each
      frame. At the attack's start the aura goes, or at level 2 with forward held flies off as the ball (composite 4, 4 pixels up,
      6 pixels a frame through everything; hit box 20 then 5 every 3 frames, `.rca` 44; sound `0xB2`, `0xAF` 30 frames on). Every
      61 frames a glint flickers at random in Kirby's body box (`spark_cmn`'s anim 58). The composites (`spark_ext.mca`): 0 = 37
      (8 up) + 25, 1 = 26 (8 up) + 25, 2 = 28 + 27 (8 up) + 38, 3 = 23 + 22 + 31, 4 = 23 + 22 + 32; 25 and 38 show Kirby's own
      cells (5, 8, 11, 14) every other frame. Ported (`ROM Expansion/scrolls.c`'s Spark).
    - Beam (`0x02088B44`, after the start-up `0x02088C9C`: `beam_cmn`'s 21): the whip, `beam_ext.mca` composite 0 (anim 23 + Kirby's 22,
      34 frames, sound `0xD3`, hit boxes of 15 on 15 of its frames: `.rca` 27), or with the scroll composite 1 (anim 25, 4 pixels
      forward, + Kirby's 24, 44 frames, sound `0xD4`, hit boxes of 20 (one of 9) on 19 frames, sweeping from higher above him to
      lower in front: `.rca` 28), then the common end state. The DS's Beam has no other move. Ported (`scrolls.c`'s Beam).
    - Tornado (start `0x020BB03C`: composite 0, sound `0x90`, or `0x92` with the scroll; start-up `0x020BAD4C`; the spin `0x020BA8F8`,
      composite 1 at its end, sound `0x91` at frame 24): with the scroll, while the tornado has no element yet, each frame
      `0x02012E84` reads what Kirby touches (his `+0x100`, the kind of what met him: 1, 7, 8, `0x16` fire; 2, 10, 11, `0x17`
      electric; 9, or his flag `0x400000`, water) and the tornado takes it (sound `0x93`): `tornado_ext.mca` composites 8 (fire),
      9 (water), 10 (electric): anims 28, 32, 36 (the sparks) at (-10, -30) with 27, 31, 35 (the tornado's body, in the element's
      colors: orange, blue, green) at his feet, in place of the plain tornado; its box then (-22, 46, 25, 0) of 25 for the plain
      (-11, 30, 12, 0) of 20. Ported (`scrolls.c`'s Tornado): fire and electric, from what KatAM's collision lists hold that
      frame with those hit kinds (0x100, 0x400; fire first, as the DS checks it first); not water, KatAM's tornado stopping in
      water. Tested in the emulator (room 0x65's Sparky, all three versions; Tornado and the scroll given by RAM writes): the
      attack's kind goes from `0x30000043` to `0x30000443`, strength 2 to 3, the green tornado drawn and KatAM's hidden till the
      end animation; fire only with the Sparky's hit kind forced to 0x100 (orange tornado, `0x30000143`); without the scroll
      KatAM's tornado unchanged.
    - Parasol (its Kirby's attacks are `parasol_ext`'s animations, as each ability's `_ext` file's; its swing `0x020B97BC`: 21,
      then 22, air 17, 18, sound `0xA2`; B held after it, `0x020B92F0`: 26 then, `0x020B90A8`, 27, the parasol held out forward,
      spinning, as KatAM's twirl, air 22, 23, a box of 8 (`.rca` 27, 14 to 34 in front), till B is let go,
      `0x020B8F18`, then 28 or 24 to the end): with the scroll the twirl is `parasol_ext.mca`'s composite 2 (93 frames, a loop: Kirby,
      part 15, thrusting his parasol forward, anim 30, and parts 0-14, anims 31-37, single 8x8 stars and sparkles in palette 6
      bursting from its tip), sound `0xA3` at its start and every 37 frames (`0x0200C80C`'s count), its boxes (`.rca` 40, the
      composite's: `.cea`'s 38 animations, then the composites) one frame in three: 13 to 33-63 pixels in front, 12 under his feet
      to 30 over them, of 10 the first and 8 the rest, as the plain twirl's 8. Ported (`scrolls.c`'s Parasol): KatAM's own twirl
      (already forward, its box 10 to 38 in front, strength 1) keeps its look; the DS's stars (parts 0-14 flattened into one
      animation) are drawn about Kirby and the DS's boxes and sound replace its box. Not the water parasol (`parasol_wat`). Tested
      in the emulator (room 0x65's Sparky, all three versions; Parasol and the scroll given by RAM writes): the stars drawn, KatAM's
      twirl box off, the Sparky (under the ledge, out of KatAM's twirl's reach) losing 1 HP every 3 frames; without the scroll KatAM's
      twirl unchanged and the Sparky untouched.
    - Sleep (each frame of the sleep `0x020BF294`): with the scroll `prmKirbySleep.kgb`'s word 1 (10) is added a frame and a hit
      point given each time it makes word 0 (180): one each 18 frames; A or B pressed (the DS's own, scroll or not) cuts the sleep
      by 2 frames, sound `0xA9`. Ported (`scrolls.c`'s Sleep; the cut comes with the scroll, KatAM's sleep having none). Tested in
      the emulator (room 0x65, all three versions; Sleep given and HP set to 2 by RAM writes): his 239-frame sleep heals 2 to 6 by
      18-frame steps; A pressed every 4 frames wakes him 76 frames sooner; without the scroll no healing, the sleep as long.
    - Cutter (as Kirby throws it, `0x020BA1C8`): with the scroll the thrown cutter is `cutter_ext.mca`'s composite 0 (`0x020B9F18`:
      anim 25, a big blue blade spinning, with 27, sparkles about it) for the plain 23 (a small yellow one), its box 39 x 27 (`.rca`:
      -20 to 19 across, -13 to 14 up) for 27 x 20, the same damage (20) and flight. Ported (`scrolls.c`'s Cutter): KatAM's own cutter
      (its task `sub_08070580`, strength 4, box -12 to 12 across, -7 to 8) flies as it does, drawn as the DS's (KatAM's not drawn),
      its box the DS's. Tested in the emulator (room 0x65, all three versions; Cutter and the scroll given by RAM writes): the thrown
      cutter's box -20, -14 to 19, 13, KatAM's look off and the DS's blade drawn following it; without the scroll KatAM's cutter
      unchanged (box -12, -7 to 12, 7, its yellow look).
    - Laser (its attack `0x0209A0C0`: `laser_ext`'s 25, then `0x02099FB0`: the shot, object `0x020373F8` with `laser_ext`'s
      21-23 and `prmLaserEffectLaser.kgb`, its `.rca` boxes of 20, sound `0x95`, and composite 0, 15 frames, `0x02099E94`): with
      the scroll, B still held as composite 0 ends, the charge (`0x02099D00`: composite 1, `laser_ext`'s 33-36, sparkles closing in
      on the gun at (6, -11), with the DS's Kirby flat, 30 and 31; sound `0x138` every 5 frames), charged past 29 frames
      (composite 2: and 32, an orb flashing blue and gold at (0, -8), its last part, under Kirby). B let go: not charged, the end;
      charged, composite 3 (`0x02099BF0`: 38 and 39, sparkles at (5, -18); 6 frames), then the charged shot (the same object, its
      type 1: `laser_ext`'s 42-44, a thicker bolt, its boxes of 30, as big as the plain shot's; sound `0x95`; on through enemies:
      its hit, `0x02037280`, ends only the plain shot, type 0, and its boxes' body record has the flag 0x20) and composite 4
      (`0x02099B78`: 41, a flash, 28, a ring bursting; 20 frames), then the end. Ported (`scrolls.c`'s Laser): KatAM's laser
      (`sub_0806A520`: `sub_08063E50`, its shot `sub_0807BCE0`, then `sub_08063F74`, ending past its counter's 8) is held while
      B is held after it, KatAM's Kirby in his laser pose with the DS's sparkles, sound and orb (behind him); let go charged, the
      sparkles, then KatAM's own shot (flight, turns on slopes, box, sound) at strength 3 (KatAM's 2 for the DS's 20), on through
      enemies (its hit callback, `unk7C`: KatAM's burst and end only on terrain, as KatAM's own `sub_08084C34` lets a shot through),
      drawn as the DS's bolt (across, up or down as it goes), the flash and ring. Tested in the emulator (room 0x65, all three versions; Laser
      and the scroll given by RAM writes, an enemy's HP set to 10 and held in place by RAM writes): B held, the charge from 9
      frames after the shot, the orb 30 frames on, let go, the charged shot 6 frames later, Kirby standing 20 frames after it;
      the plain shot takes 2 HP and ends there, the charged one 3 and goes on past the enemy (KatAM's own shot ends on it); let go
      early, the laser ends that frame; without the scroll KatAM's laser
      unchanged (ends 9 frames after the shot, B held or not).
    - UFO (each frame, `0x020BD560`): with the scroll, while UFO Kirby has none, a star (`0x020BB85C`, its state `0x020BB488`:
      `ufo_ext`'s 36, a sparkling star; its `.rca`'s box, -6 to 5 across, -5 to 5 up, of 10, every other frame; the box's
      first word, 2, is no element: Spark's, Fire's and Ice's boxes all have it)
      circles him: two angles from 0, `prmKirbyUfo.kgb`'s words 21 (4 degrees) and 22 (3) on each frame; at the first, word 20 (22
      pixels) from his feet, its height times word 23 (1), drawn in front of him while that sine is at most 0, behind him else;
      turned about his feet by the second (so round at 7 degrees a frame). It goes (`0x020BB7A8`: `0x020B48E0`'s star burst,
      `efc_cmn0`'s 7) once he's gone or out of play, a new one the next frame. Ported (`scrolls.c`'s UFO): a piece made from his
      input while he's UFO Kirby and has none, gone with the star burst when he isn't, leaves the room or is out of play (flags
      `0x2000000`); its box every other frame, strength 2 (KatAM's UFO laser's 4, `sub_0807BF2C`, for the DS's 20), the UFO
      laser's hit kind (`0x20000103`: a KatAM attack needs a kind bit to hit).
      Tested in the emulator (room 0x65, all three versions; UFO and the scroll given by RAM writes, an enemy's HP set to 10 and held
      in its path by RAM writes): the star circles his feet at 22 pixels, in front of him and behind him by turns; the enemy loses 2
      HP every other frame while the star is on it; the ability byte cleared, the star bursts and goes; without the scroll, no star.
    - Hi-Jump (`highjump_ext`, an ability the DS keeps but doesn't hand out; the rise `0x020BF8E8`, each frame: `highjump_ext.mca`'s
      anims 12-16, composites 9-13, by how fast he goes sideways; their boxes, `.rca` 26-30, of 20, their body record's flags 0x10):
      with the scroll anims 17-21, composites 16-20: the same, and part 37 under him (its last part) at (0, -9), an orange glow 48
      pixels across about him, flickering every other frame; their boxes, `.rca` 31-35, the same of 20, their flags 0x30: on through
      what he hits (the 0x20 the Laser scroll's charged shot has). Ported (`scrolls.c`'s Hi-Jump): KatAM's Hi-Jump (NiDL's,
      `abilities/hi_jump.c`) goes on through what it hits the whole rise already; the glow is drawn about him, behind the Kirbys,
      while he rises (`HiJumpRiseNow`). Tested in the emulator (room 0x65, USA, EU and JP; Hi-Jump and the scroll given by RAM writes):
      the glow from the rise's first frames to its end, the big and small glow on alternate frames behind Kirby, gone as he falls.
    - Wheel (the roll, `0x02092B24` and `0x0209326C`, each frame: `wheel_ext.mca`'s composite 1, its box of 20, 8 behind to 10
      in front and 21 up): with the scroll, while the wheel has no element, it takes the one of what meets him (`0x02012E84`, as
      the Tornado scroll's: fire, water, electricity) and rolls on to its end as that element's wheel (`0x02093028`: composites
      5, 9 and 13, loop sound 0xB9, 0xBE in water; fire the wheel in its colors, 31, its glow, 37, flickering, its flames, 38;
      electric 33, 41, 42), its box the same, of the same 20, with the element. Ported (`scrolls.c`'s Wheel): KatAM's wheel
      (its attack `sub_0807A3E4`, its task `sub_0807A4F4`, while he rolls: anims 0x34, 0x68) takes the element of a fire or
      electric attack or enemy that meets his wheel's box (`ElementTouch`, the Tornado scroll's search), its kind bit added to
      KatAM's attack (strength as it was, 2: the DS's damage is the plain wheel's), the DS's element wheel drawn in place of
      KatAM's (Kirby's sprite hidden, and KatAM's disc behind the wheel, `sub_08090F68`'s effect, its flag 0x400) to the end of
      the roll. Not water: KatAM's wheel stops rolling in water (`sub_080630B0`: he swims). Tested in the emulator (room 0x65,
      USA, EU and JP; Wheel and the scroll given by RAM writes, a Sparky held in the wheel's path by RAM writes, and for fire its
      kind changed to fire by RAM writes): the wheel takes electricity (attack kind `0x30000443`) or fire (`0x30000143`) as it
      meets the Sparky, the DS's electric or fire wheel drawn from that frame with Kirby and KatAM's disc hidden, Kirby shown
      again when the roll ends at the wall; before the Sparky, KatAM's own wheel.
    - Cupid (an ability the DS keeps but doesn't hand out; `PrmKirbyAngel.kgb`): the shot (`0x020BE864`) is one arrow, aimed and
      charged while B is held (`0x020BE5A4`: types 0, 4, 8, straight, up, down; speed between words 18 and 20 by the charge, fall
      word 16, sound 0xED). With the scroll and an input no button sets (`+0x28`; the owner chose it always on) the shot's start
      ends in a volley (`0x020BE724`): three arrows at once (`0x020BE28C`, types 12-14 of the table `0x021592B0`), from words 61-62,
      65-66, 69-70 (20 forward and 16 up from his feet, 14 and 10, 8 and 4), straight on (word 57, angle 0) at word 63's 6 pixels
      a frame with no fall (word 59, 0), sound 0xEC; not charged, not aimed. Ported (`scrolls.c`'s Cupid): KatAM's shot
      (`sub_080680C0`, detoured) makes three of KatAM's arrows (`sub_08081864`) from the DS's places, each kept straight on at the
      DS's speed against KatAM's slowing and fall (`CupidArrowMain`, till it sticks or ends), at KatAM's strength (3; charged past
      0x30, 5) with KatAM's pose, sound and stuck arrows; one volley a shot (KatAM's own charged triple's two more arrows not made).
      Tested in the emulator (room 0x65, USA, EU and JP; Cupid and the scroll given by RAM writes, a Sparky held in their path with
      its HP set to 20 by RAM writes): tapped, three arrows 6 pixels apart flying 6 pixels a frame level to the wall, the Sparky
      losing 4 HP to each (12); held 70 frames, three at strength 5 (6 HP each; the third lands in its hurt frames); without the
      scroll, KatAM's one arrow, slowing and falling (4 HP).
    - Bomb: the DS's scroll (HasScroll 15) adds nothing to Bomb Kirby's own moves; it opens his mixes (`0x0201FC50`: Bomb with
      Fire, Ice or Spark, abilities 0x1B-0x1D, the files `kirby/bomb_ice`, `kirby/bomb_spark`; the fire bomb keeps the plain
      bomb's): the thrown bomb (anim 30 of `bomb_ext`, `bomb_ice_ext`, `bomb_spark_ext`: purple, blue, green), the blast (anim 31,
      the same in all) of that element. KatAM has no mixing: the owner chose the element from what touches the bomb, kept till
      the ability is lost. Ported (`scrolls.c`'s Bomb): KatAM's bomb, thrown or set (`sub_08082380`, detoured), that meets a fire,
      ice or electric attack or enemy (`ElementTouchAt`, the bomb's box) gives his bombs that element (`BOMB_ELEMENT`, his ability
      byte 0x1A6: 0xB0 + 1 fire, 2 ice, 3 electricity; cleared when he isn't Bomb Kirby), the latest met; an ice or spark bomb
      drawn as the DS's (KatAM's hidden with the flag 0x400); its blast (`sub_08074240`, detoured) of that kind: ice 0x200 and
      electricity 0x400 in place of KatAM's 0x100, fire KatAM's own 0x100 (KatAM's blast burns already) and KatAM's bomb look
      (the DS's fire bomb is the plain one's files). The bomb in Kirby's hand before the throw stays KatAM's (part of his own
      sprite). The element pieces (Tornado, Wheel, Bomb) run just before KatAM's collision pass, and an object's box meets
      theirs edge to edge included (a bomb thrown at a Sparky's small box goes off on its edge). Tested in the emulator (room
      0x65, USA, EU and JP; Bomb and the scroll given by RAM writes, a Sparky held in the bomb's arc by RAM writes, its kind
      changed to ice or fire by RAM writes): the first bomb meets it and takes electricity (or ice, or fire), its own blast
      already that element's (`0x20000403`, `0x20000203`; fire `0x20000103`); the next bomb drawn green (or blue; fire, KatAM's)
      from the throw, its blast the same; the ability cleared, the element gone; without the scroll, KatAM's bombs unchanged.
    - Sword: the DS's scroll (HasScroll 10) adds nothing to Sword Kirby's own moves either; it opens his Ice and Spark mixes
      (`0x0201FC50`: Sword with Ice or Spark, abilities 0x1E, 0x1F; the Fire Sword, Sword with Fire, needs no scroll), whose sword
      is that element's (`kirby/sword_ice`, `sword_spark`, `sword_fire`: Kirby's every pose with the element's sword, the same
      frames as the plain sword's in the element's colors, its palette 1: the blade 2-6, the cap 7-9 the plain sword's, 10-14) and
      whose cuts hit with that element. KatAM has no mixing: the owner chose the element from what touches the sword, as for the
      bomb, kept till the ability is lost (fire too, with the scroll: no scroll, KatAM's sword unchanged). Ported (`scrolls.c`'s
      Sword): a cut of Sword Kirby's (his sword, `sub_080753CC`, the one object that lives as long as he is Sword Kirby and takes
      each swing's box from its frame; the other cut tasks `sub_08080130`, `sub_08080FBC`, `sub_08083B04` and the slide,
      `sub_080714A0`) that meets a fire, ice or electric attack or enemy (`ElementTouchObj`: the cut's hit box, `unk3C`-`unk3F`,
      ObjectSetBounds', mirrored facing left as KatAM's attack test `sub_08038010` reads it) gives his sword that element
      (`SWORD_ELEMENT`, his ability byte 0x1A6: 0xC0 + 1 fire, 2 ice, 3 electricity; cleared when he isn't Sword Kirby), the latest
      met; then every cut of his carries its kind bit (0x100, 0x200, 0x400, added to KatAM's cut 0x2000) and his sword is drawn in
      the DS's element sword's colors: KatAM's sword palette (his second OBJ palette, 4 + his number) has the DS's layout, its
      colors 2-14 written each frame (`gSwordColors`, from `sword_fire_ext`, `sword_ice_ext`, `sword_spark_ext`'s palette 1; KatAM
      draws the blade and its swing from it, so the swing takes the element's colors too), and into `gUnk_02022320` as well: the
      palette KatAM's palette effects (a flash, a fade) put back each frame before their blend (`sub_0803A1F4`, priority 0xFFFE,
      after the piece); written only while his second sprite uses that palette (in water KatAM gives it 0xE, every Kirby's: not tested
      in water). Tested in the emulator (room 0x65, USA, EU and
      JP; Sword and the scroll given by RAM writes, a Sparky held in front of him by RAM writes, its HP set to 20, its kind
      changed to fire or ice by RAM writes, or its own spark attack): the slash that meets it takes fire, ice or electricity (0xC1,
      0xC2, 0xC3) on the frame KatAM's slash hits it, the sword's attack then `0x32002103`, `0x32002203` or `0x32002403`, palette
      4's colors 2-14 the DS's (the fire sword orange, the ice sword pale blue, the spark sword green, blade and swing); facing
      left, the Sparky in front and behind, taken on the frames KatAM's own slash hits it; the slide (Down and A) takes fire and
      carries it (`0x10402143`); the ability cleared, the element gone, and Sword taken again drawn in KatAM's own colors; the
      Sparky's element cleared each frame, KatAM's sword unchanged; without the scroll, KatAM's sword unchanged (no piece). On JP
      a palette effect ran through the test (frames 2283-2330) and had put KatAM's colors back each frame before `gUnk_02022320`
      was written too; after, the element colors stay through it.
    - Hammer: Hammer Kirby's B (0x02094DB0) picks by his flags (`+0x14`: 0x40 in the air, 0x200000 in water, set by the shared
      move's water check, 0x02013AD2) and his input: on the ground out of water, with Up held (input `+0x18`) the flaming flip
      (0x0209417C: hammer_ext's 21, 25 frames held up then hits of 60), dashing (`+0x44`) 0x02093FC8 (hammer_ext's 22), else the
      charge (0x02094D08: hammer_ext's 30, the hammer held behind him; then each frame 0x02094A88); in water the same charge in its
      own frames (0x02094738: hammer_ext's 31, 0x020944B8, its giant swing hammer_ext.mca's composite 2); in the air 0x02093CA4, in
      water 0x02093B90 (the swing, hammer_ext's 20). The charge (`PrmKirbyHammer.kgb`'s words 16384, 410, 3, 8, 60; its count `+0x128`
      from -1): let go of B (input `+0x20`) once it has gone on for word 3 (8) frames, the swing (hammer_ext's 25, `+0xB4` 0;
      hammer_ext.rca's 5, its boxes in frames 1-4, 20 damage), and on its end, on the ground, hammer_ext.mca's composite 0 (the
      hammer down, the dust; 0x02035FC4's quake (4, 5, 2), sound 0xB3). With the scroll (HasScroll 6, `0x02094A88`), at word 4 (60):
      efc_cmn0's 29 (0x02039B28, the template 0x021586C0: `effect/efc_cmn0`, animation 0x1D) 32 pixels behind him and 16 up, sound
      0x148, hammer_ext's 42 (the giant hammer held behind him, glowing); let go then, `+0xB4` 1 and the giant swing (hammer_ext's 32,
      7 frames; hammer_ext.rca's 12, its boxes in frames 4-6, 50 damage), on the ground composite 1 (the giant hammer down and back
      to its size, the dust 36 pixels ahead), the same quake and sound (in the air hammer_ext's 34). hammer_ext.rca's animation n is
      hammer_ext's n + 20 for these (5: 25, 12: 32, 15: 35; their totals). Each cell draws Kirby with the file's palette 0 and the
      hammer with its palette 1; KatAM's Kirby palette has the DS's palette 0's colors 1-11 (12-14, his eyes' shine, its own).
      Ported (`scrolls.c`'s Hammer): with the scroll, Hammer Kirby's B (`sub_080618C0`, detoured) on the ground, out of water, not
      dashing nor with Up held (KatAM's own flip, unchanged) starts the DS's charge, its base move the scroll's upgrade is built on
      (`HammerCharge`, a state of Kirby's: KatAM's gravity and slowing, NidlAbilityGravity); let go after 8 frames and before 60,
      KatAM's own swing (sub_080618C0's last branch: animation 53, `sub_080622A0`); at 60 the DS's second level (the glint, sound
      0x148); let go then, the DS's giant swing (`HammerSwing`, 7 frames) and, on the ground, its landing (`HammerLand`, 20 frames;
      KatAM's shortest shake, RequestScreenShake 1, 14 frames, for the DS's 5; sound 0xB3). Through them Kirby is drawn as the DS's
      frames (SqueakEffect's OnlyPalettes: his body, palette 0, drawn with his own palette, so a sprayed Kirby or a helper keeps his
      colors, and the hammer, palette 1, in its own: SQUEAK_FX_HAMMER_*), KatAM's sprite hidden (TORNADO_HIDE) and his hammer too
      (his animation 102, HAMMER_HIDDEN: none in the hammer's table, gUnk_083502C0, and nothing in KatAM checks it). The giant
      swing hits as KatAM's hammer (its kind 0x34005003, its knock back 0x1E0), with the DS's three boxes in its frames 4-6, once a
      swing (0x40000 kept, as KatAM's hammer), its strength KatAM's swing's 5 times the DS's 50 / 20: 12. Tested in the emulator
      (room 0x65, USA, EU and JP; Hammer and the scroll given by RAM writes, a Sparky held 41 pixels ahead of him by RAM writes, its
      HP set to 99 and its element cleared each frame): B held 75 frames, the charge (75 frames), the giant swing taking 12 HP (99
      to 87, once), the landing, standing again 20 frames later, the frames drawn (the hammer behind him, the giant one glowing, the
      swing over his head, the hammer down with the dust); let go at 20 frames, KatAM's swing (5 HP); a tap, KatAM's swing after
      the 8 frames; without the scroll, KatAM's swing at once; Up and B, KatAM's flip (25 HP), unchanged. Not ported: the DS's
      underwater charge (KatAM's water swing kept), the air landing's own frames (he goes back to KatAM's fall).
    - Fighter: the DS's Force Blast (0x020C5828, each frame of the charge; `PrmKirbyFighter.kgb`'s words 11-13: 10, 60, 120,
      frames from its start) goes up a level at 10 frames and at 60, and with the scroll (HasScroll 17) at 120 to a third, its
      biggest blast (fighter_ext.mca's composite 2); with the scroll and his HP under 11, the first level is that third. KatAM's
      (`sub_08065900`, the charge; `sub_08065A0C`, the blast on its 5th frame) has its small wave, its big one past 30 frames, and
      at 1 HP its biggest (the 1 HP attack, `sub_0807F128`), its charge aura (`sub_08094540`, made by `sub_0809447C` as the charge
      starts) drawn and sounded by level (its third at 1 HP; CHARGE_1-3 every 6 frames). Ported (`scrolls.c`'s Fighter): with the
      scroll, the charge held to the DS's 120 frames reaches that biggest blast (a piece, FIGHTER_CHARGE, made from his input while
      he charges, counts it): the aura then in its 1 HP look and sound and, let go, the 1 HP attack, as at 1 HP (`sub_08094540`
      and `sub_08065A0C` detoured, run with his HP read as 1 and his own put back; KatAM's own low-HP blast is the DS's low-HP
      shortcut, kept). The aura's detour first showed that a detour's trampoline jumping back with `ldr r3; bx r3` broke a function
      whose first instructions load `r3` (`sub_08094540`'s `ldr r3, [r2]`, `gCurTask`): the aura's task read the wrong object
      and ended after a frame, scroll or not; the trampolines now jump back keeping every register (`push {r0, r1}`; `ldr r0`;
      `str r0, [sp, #4]`; `pop {r0, pc}`), and no other detour had such code (checked, all three versions). Tested in the emulator
      (room 0x65, USA, EU and JP; Fighter and the scroll given by RAM writes, a Sparky held 61 pixels ahead of him, its HP set to
      99): B held 145 frames, the aura through the charge (as vanilla KatAM's, checked against the unmodified USA ROM), its second
      look at 30 frames, its third at 120, the blast taking 6 HP (KatAM's 1 HP attack), Kirby's HP kept (5); without the scroll,
      the big wave (4 HP), the aura's third never shown. The Hammer scroll's giant swing and the save hooks (a new game saved,
      then continued from the file select) were run again after the trampoline change. The sounds weren't listened to.
    - Throw: the DS's grab (0x020C0E00, each frame of the lunge; once Kirby holds something, his `+0x14`'s 8) makes a 0x134-byte
      object (0x020C0938, 0x020C0890: the held one kept at its `+0x130` and carried at its spot, thrown with it: its flight
      0x020C0054 and its bounce 0x020BFFFC, gone 12 frames after, sound 0x135, as KatAM's thrown enemy) whose look is a 10-frame
      flash at the grab (0x020C0568: throw_ext's 43, a blue ball bursting into stars). With the scroll (HasScroll 18) its type is
      4-7 by the held one's element (its `+0x10`, which the enemy factory 0x0202467C sets after making it: Hot Head, Flamer, Acchi 1,
      fire; Pengy 2, water; Sparky, Laser Ball, UFO 3, electric; any other 0; 0x02012E84's numbering), its look throw_ext.mca's
      composite 1-4 (42, a ring of stars 8 pixels up, with 32-35: the ball in the element's colors, none purple, fire orange, water
      cyan, electric green, palette 2 or 1, the stars 6). Nothing else reads the type (only 0x020C0568 and the constructor touch
      `+0x12C`): the throw, its flight and its hit (.rca's 22 and 23: 30, kind 0) are the same with the scroll or without. KatAM's
      grab (`sub_0807A7E8`, the thrown object made from what Kirby holds, `sub_0809C48C`) shows no flash, so the DS's base one isn't
      kept. Ported (`scrolls.c`'s Throw): with the scroll, the grab (detoured) makes a piece (`THROW_FLASH`) showing the upgrade's
      element flash (SQUEAK_FX_THROW_*: each composite as its ball and its stars) on what he holds, centered on it and following
      it, for its 10 frames; the element by the same enemies (KatAM's types `OBJ_HOT_HEAD`, `OBJ_FLAMER`, `OBJ_PENGY`,
      `OBJ_SPARKY`, `OBJ_LASER_BALL`, `OBJ_UFO`; Squeak Squad's Acchi), not by the ability they give (the DS's Chilly, an Ice
      enemy, has none). Tested in the emulator (room 0x65, USA, EU and JP; Throw and the scroll given by RAM writes, the Sparky put
      28 pixels ahead of Kirby by RAM writes, and for the others its type byte rewritten to Hot Head, Pengy or Waddle Doo as it
      was inhaled): B held, the Sparky pulled in and grabbed at the same frame as without the scroll, the green flash on it for 10
      frames (the ball, then the burst and the stars), the orange, cyan and purple ones for the others; the throw after it
      unchanged; without the scroll, KatAM's grab and throw frame for frame the same, no flash. Not tested: the Squeak Squad
      Acchi's, and grabbing an effect object (`sub_0807A7E8`'s other caller: none, the purple flash).
    - Magic: KatAM's B is a roulette of cards round Kirby; the DS's (kirby/magic_ext, PrmKirbyMagic.kgb, B at the vtable's +0x88,
      0x020C34F0) throws: Up held, a jack-in-the-box out of his hat (0x020C2C20, 89 frames, the clown's hit 20); dashing, three cards
      (0x020C28C8: each spins 15 frames rising, holds, flies off 8 pixels a frame; 10 each); else, in the air too, three doves
      (0x020C2A78, flying off forward and up, each its own speed and gain; 10 each); doves and cards gone on a hit or off the screen.
      With the scroll (HasScroll 19, 0x020C2CC0), B held through the whole move starts the DS's roulette as it ends (0x020C33D0): the
      world held still, the background darkened by half, Kirby thinking, the thought's icon going round five outcomes one each 4
      frames, B (or 300 frames) stopping it; 60 frames on, the outcome (0x020C2F88): cherry (every enemy a cherry), 1UP (every enemy a
      1UP), "?" (two random abilities mixed), candy (600 frames of invincibility) or sun (five star bursts at fixed spots, 50 each,
      and 50 to every enemy); then the hat in confetti and, but for the "?", the ability used up. The owner's pick: with the scroll,
      Magic is the DS's upgraded Magic, its three throws and its roulette; without it, KatAM's Magic as it is. Ported
      (`scrolls.c`'s Magic): B (`sub_0806898C`, detoured) with the scroll does the DS's moves (Up, KatAM's dash flag 0x10, else),
      Kirby drawn as the DS's frames (`MAGIC_BODY`, as the Hammer scroll's), the doves, cards, star bursts and the outcome's sweep
      pieces of their own, the world held by KatAM's own freeze (`sub_08033540`, Kirby's flag 0x2000 keeping him going; the doves and
      cards in flight held and drawn as they are), the darkening KatAM's blend (BLDY 8 of 16). The outcomes are KatAM's Magic's own:
      cherry and 1UP its hit turning what's on the screen into food or 1UPs (`sub_08083040`'s, without its shake and flash: KatAM's
      foods, picked at random, for the DS's cherry; they stay, as KatAM's items, where the DS's go after 360 frames; KatAM's enemies
      wobble 30 frames before turning), "?" its random ability (`sub_08068CA4`'s, KatAM's roulette on the HUD: KatAM has no
      mixing), candy its invincibility (`sub_0808324C`, 600 frames, KatAM's candy music), sun the DS's star bursts (fitted to the
      240 by 160 screen) and a hit on every enemy on the screen. The DS's damage in KatAM's strengths as the Beam scroll's (one for
      each 5): 10 is 2, 20 is 4, 50 is 10. In water, and without the scroll, KatAM's own Magic. The darkening goes off at once at
      the end (the DS's fades 4 frames), and the DS's last 2-frame pose (19) is left out. Checked against the DS in an emulator
      (py-desmume, the USA ROM, Magic and the scroll given by RAM writes): the three throws by their inputs, the roulette from B held,
      its stop by B, its icons, timings and outcomes. Tested in KatAM's emulator (room 0x65, USA; Magic and the scroll given by RAM
      writes, a Sparky ahead of him): B, three doves; B held to the move's end, the roulette (the screen darkened, the Sparky held
      still, the icons going round), B stopping it; the outcomes forced by RAM writes as it stopped: cherry (the Sparky became a
      food), 1UP (a 1UP), candy (KatAM's invincibility sparkle), sun (the star bursts, the Sparky killed), "?" (KatAM's random
      ability roulette, ending on an ability); after the others the used-up ring and Normal, the darkening gone. Room 3: a double
      tap dash and B, three cards spinning, holding and flying; Up and B, the jack-in-the-box. EU and JP: the doves, the roulette
      darkened, an outcome. Without the scroll, KatAM's own card roulette. The palette slots were checked (5 of KatAM's 7 used, the
      Sparky's rows the same before and during the roulette). Not tested: Magic in water, a helper's Magic (KatAM's freeze is per
      player, and the roulette stops by itself at 300 frames), and the sounds weren't listened to.
  - **The Secret Map's pieces** (treasures 93-99, taken as pieces 1-7 in order: not checked against the DS): each in its own chest
    (2-6-6, 3-3-7, 4-4-5, 5-4-5, 6-6-5, 7-6-9, 8-2-6), marked `chest[0x16]` the piece's number + 1 and `chest[0x17]` 2 (the area
    maps' mark is 1), the item one of KatAM's maps. Their art is `collection/map`: anims 0-6 the pieces (parchment, about 60 x 50
    pixels each, sepia), 7 the whole map. `KatAMExpansionMaps.cs` and `ROM Expansion/chest.c`, `collection.c` have the rest.
  - **Vitality halves** (the owner: "Two halves make one Vitality"): a chest of treasures 6-13 holds KatAM's Vitality
    (treasure 6) with `SQUEAK_CHEST_HALF` in its template's high byte of `+0x14`; the converter lists those chests' numbers in
    `chest.c`'s `gSqueakVitalityChests` (8 at most, 0xFF unused). Opening one goes through KatAM's own Vitality (its
    animation, `gTreasures.vitalityField` + 1, max HP + 1), then `NidlChestOpen` counts the listed chests opened: an odd count
    (this is a pair's first half) takes the Vitality back, so max HP grows on every second half. Three of the eight are in
    their rooms from the start (1-6-7, 2-6-7 and 3-6-2's); the other five (2-6, 4-3, 6-3, 7-6 and 8-3's) are chase chests
    (flag 4), out once their stage's chase is on (gimmick `0xF6`).
    Tested in the emulator (1-6-7's chest, all three versions, Kirby walking onto it and pressing Up): opened first, max HP stays
    6 and the Vitality count 0; with another half's chest marked opened beforehand (a RAM write to its saved bit), the Vitality
    count goes to 1 and max HP to 7. Not played through from two real chests in a row. The chest randomizer leaves these chests
    alone (it shuffles only KatAM's own list).
  - **Spray paints** (treasures 70-87, taken as Squeak Squad's sprays 1-18 in order: not checked against the DS's chests; `collection/spray01-18.cla` their Kirby palettes,
    `collection/spray2`'s cells 1-18 their names; `spray00` is PINK, Kirby's own): its 1-13 are KatAM's 13 sprays by name (YELLOW,
    RED, GREEN, SNOW, CARBON, OCEAN, SAPPHIRE, GRAPE, EMERALD, ORANGE, CHOCOLATE, CHERRY, CHALK; YELLOW, ORANGE, CHERRY and CHALK
    the same colors, the others other shades), so only 14-18 are ported (the owner: "remove any duplicate spray color"): SHADOW,
    IVORY, CITRUS, WHITE, LAVENDER, in treasures 83-87's chests (`chest[0x16]` the spray's number + 1, the item KatAM's spray of
    the closest body color, whose can rises: SHADOW CARBON's, IVORY CHALK's, CITRUS GREEN's, WHITE SNOW's, LAVENDER GRAPE's).
    The palettes are in KatAM's Kirby layout (1 the outline, 2-8 the body, 9-11 the feet, 12-15 the whites; Squeak Squad's outline
    is sometimes colored), so they are KatAM's anim 0x15's variants 16-20 as they are. `KatAMExpansionSprays.cs` and `ROM Expansion/collection.c` (the Collection Room's grid) have the rest.
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
  - **Palette animations** (a room's `.mxi` `PL`; the owner: "Fix animated tiles in general"): `map/p_0f.pal` (2-1-4, 2-1-5,
    2-5-4, 3-1-4, 3-1-5, 3-1-6, 4-4-5) and `map/p_2b.pal` (2-2-1), the crystal caves: `u16` its kind (1), `u16` how many rows,
    then each row: `u8` the main tileset's palette it changes (0F_b.bin's 6, 7, 8; 2B_b.bin's 7, 8: the purple rims and the big
    crystals pulse, the green crystals' sparkles turn), `u8` its frame count (15, 14, 12), a byte a frame (5 each), to an even
    length, then 16 colors a frame. The boss rooms' `pa_NN.pal` are another kind (2, the light fades). Seven more (`p_1a`, `p_21`,
    `p_22`, `p_23`, `p_24`, `p_26`, `p_2d`) no room lists and the DS's code never names (no `map/p_` string in it; the ROM names
    only `p_0f` and `p_2b`): left out. In KatAM (`SqueakPaletteCycles()`, `ConvertSqueakForeground()`): each color a row changes is
    a color of its own (`SqueakCycled()`: never merged, sorted after its palette's others), and each KatAM palette's run of one
    row's colors one of KatAM's own palette cycles (ForegroundPalette +4, +8: `sub_08003108()` copies a frame's colors in for its
    duration; at most 8 a room, these rooms need 3). Checked in the GBA emulator (a forced start in 2-1-4, room 0x432, USA): the BG
    palette read every 10 frames, palettes 0, 2 and 4 change as the DS's rows' frames, and shots show the rims and crystals glow.
  - **Colors** (the owner: "There's a lot of color palette problems with enemies and stages."; swept with every room: the converter's
    numbers for all 482 rooms, the DS against KatAM side by side in the rooms that changed most, and a fly through every room for
    the enemies'):
    - *The stages.* KatAM's foreground has 6 palettes of 15 colors for the DS's 16 rows of the three layers; 89 rooms' tiles don't
      pack into 6 as they are. The converter merged the two closest colors everywhere until they did, every tile with either color
      changed (7-1-3's walls: 43% of their pixels more than a near shade off, the teal rock gone to green). Now the merges within a
      near shade (`SqueakColorDistance()` 60) stay, and the colors merges moved further are fitted again tile by tile: each set of
      colors (a tile's, an animated slot's) drawn with the palette that shows it closest, each color the closest that palette has,
      the palettes rebuilt from their sets' own colors, round after round (`SqueakFitPalettes()`, a far shift weighing more than a
      near one). The pixels shown far off (past 200) in all those rooms: 37,563 merged, 163 fitted; past a near shade: 517,416
      merged, 79,716 fitted. Rooms whose colors fit, or whose merges stay within near shades, come out as before. ROM room: fitting
      every merged color took 43 KB in the European ROM, which had 42 KB left (its Collection Room's five languages); keeping the
      near merges, 9 KB (33 KB left there). Checked in the GBA emulator (forced starts) against DS shots
      of the same rooms: 7-1-3 (0x56E), 2-1-4 (0x432), 2-5-6 (0x457), 3-1-5 (0x470).
    - *A DS-side note.* The DS loads a room's BG palette from its `BB` tileset (`0x022184A2`-`0x0221855A`: the part's `+8` file,
      its 0x200 bytes at `+0x10` copied to the map's `+0x18`). A stage loaded in the DS emulator through stage 1-1's `.mxi` starts
      (from this save) in the stage's fifth part, not its first: the room put first showed another (7-1-3 looked orange: that was
      7-1-5's tileset 39). The `.mxi` is now made with every part the room. 2-1-4's big crystal pink on the DS and lilac in KatAM
      is its palette animation (`p_0f.pal`) caught at two moments, both colors its frames.
    - *The enemies.* KatAM keeps 7 enemy palettes (OBJ slots 8-14) and loads an 8th into slot 14 over the 7th's, its key kept out
      of the table: each loads again as it spawns and the other shows in its colors. A fly through every Squeak Squad room (a test
      script in the GBA emulator, as `debug/fly.lua` flies: warped in, Kirby flown in rows 96 pixels apart at 12 pixels a frame,
      his HP kept, `gUnk_02022E60[7]` watched) found it in 55 rooms (7 kinds of KatAM enemies, items and chests, and a Squeak Squad
      character on top: 0x48F 57 times in one pass). `rooms.c`'s `BigLoadPalette` (KatAM's loader redirected) gives the 8th the first slot no live object draws with,
      KatAM's way only with all 7 in use. Measured with the same fly (both ROMs flown alike: 487 rooms whole, 2 left mid-way by a
      warp; 0x493, 0x578, 0x5D1 and 0x5D2 have no door to land Kirby on), a palette loaded over another: 571 times in 55 rooms before, 96 in 22 after, those at moments with 8 or
      more palettes on the screen at once (0x4A4, 0x534 and 0x4FA the most), more than KatAM's 7 slots hold. In 0x461 the grass
      tuft drew in fire colors, its barrel in a chest's and Acchi in another's; now in their own (same frame of the same flight
      in the USA, European and Japanese ROMs). The characters' own colors are the DS's `.cla` palettes as they are (only
      Squeakers draws one near-white as white).
  - **Currents** (the owner: "Currents in water don't have any effect on the player."): a cell's id `0x10`-`0x3C` (a step of 4),
    in water or in air, pushes Kirby: (id - 0x10) / 4 its index, the index's low 2 bits the way (0 up, 1 right, 2 down, 3 left),
    the rest the strength (0.5, 1 or 2 pixels a frame, added to his position; measured in the DS emulator, 1-1-1's cells rewritten
    in the live grid). 14 rooms have them. In KatAM: a byte a cell (`SqueakCurrents()`: 0x80 | way << 2 | strength), and the
    room's gimmick list entry 0xF3 (`squeak/gimmicks.c`'s `Currents()`) moves each Kirby in the room by it, never into a wall,
    an up current taking away his fall. Checked in the GBA emulator (forced starts): 7-2-3's air current carries Kirby 2 pixels a
    frame, 7-1-3's water current 2 pixels a frame and he can't swim against it. The logic check's `curcheck.py`: every door and
    landing in the 14 rooms reached (a move against a strength-2 current not counted), none trapped.
  - **Walk-once blocks** (the owner: "Blocks that you can only walk once and they get destroyed after do not work"): the blocks grid's
    block 3 on air (kind 0) over a solid main cell, rows of cracked blocks (1-6-6's castle walkway, 2-6-7's leafy bridge; 348 in 28
    rooms), drawn in the main layer, what's behind them in the third. Under one, the blocks grid's 0x400 cells on air over solid
    ground go with it, each under the one before (3-6-2's second row, 3-2-3's and 3-1-3's shafts, up to 14 deep). Checked in the DS
    emulator (3-6-2, Kirby put on its first one at x 968): it breaks 12 frames after he lands (the star blocks' break), he falls,
    the cell under it 7 frames later; put at x 961, over the edge of two, only the one under his middle breaks and he stays on the
    other. A broken one shows the third layer (3-6-2's forest behind; 3-2-3's water). In KatAM (`SqueakCrumbles()`,
    `SqueakCrumbleChain()`): each is a star block (any attack breaks it too), drawn broken with the third layer alone (`burnt`),
    leaving air, or water where its column stands in water (3-2-3's nine); `squeak/gimmicks.c`'s `Crumbles()` (the room's
    gimmick list entry 0xF4: a cell list, each with how many hidden cells under it) breaks the one under a standing Kirby's
    middle 11 frames after KatAM sees him on it (12 in all), then its hidden cells 7 frames apart (`sub_08001408()`, the star
    blocks' own break). Checked in the GBA emulator (forced starts, USA): in 3-6-2 (0x4A0) Kirby lands at frame 14, falls from
    frame 27 through both rows, the cells beside stay; in 3-2-3 (0x479) the column under him opens down its 7 hidden cells, shows
    water, and he sinks through it swimming. Not played through a whole stage.
  - **Tutorial signs**: the three in Prism Plains 2 (its first, third and fifth rooms: "Tap to pop!", "Find treasure and move on.",
    "Mix bubbles!") are front-layer blocks without collision, 16 by 11 cells; left out (`SqueakSigns()`: no other room has such a
    block), the owner: KatAM skips them.
- **Stage files** (`map/*.mxp`, NDS RLE, type 0x30): `MaD\0`, 8 bytes, then 13 section offsets (`u32` from +0x0C): section 0 the width and height in tiles (bytes); 3-5 and 6-8 the tile grids (16 and 32 bits a tile); 9 the zone grid (a byte a tile), then a `u16` count and 8 bytes a point (`u8` kind, a byte, `u16` x, y and a size, pixels from the top), then the points' table; `.doe` the objects (an enemy list and an item list: see "Stage rooms, for porting them"). A stage's `.mxi` (`map/aNsM.mxi`) lists its parts' files (`MA` the map, `FB`/`BB` the graphics, `DE` the objects, `PL` a palette, `3D` the background); loaded, its table sits at the stage info (`*(*0x02229F1C + 0x40)` + 0x10, 0x1C bytes a part). To load a stage in the DS emulator, repoint that table's strings to the stage's files and let Kirby lose a life (the stage restarts from its first part): Mrs. Moley's stage was recorded this way from a save in 1-1.
- **Ability numbers** (`0x0203A9C4`'s, from the factory's cases): 1 Fire (Hot Head, Acchi, Flamer), 2 Ice (Pengy), 3 Spark (Sparky), 4 Beam (Waddle Doo), 5 Tornado (Twister), 7 Cutter (Sir Kibble), 8 Laser (Laser Ball), 9 Bomb (Foley), 10 Wheel (Wheelie), 11 UFO, 13 (Cupie), 14 Sleep (Noddy), 15 (Starman), 16 Sword (Sword Knight, Heavy Knight), 17 (Bun), 19 (Bio Spark), 20 (Boxin), 21 (Gaw Gaw), 22 Bubble (Bubble Head), 23 Metal (Metalun).

**Daroach in a stage** (enemy 0x45, ported: `squeak/daroach.c`): made by `0x020A51A8` (with `enemy/demo_dancho`, `0x020750F4`), his body `0x02134DA8` (`0x020A4F54(..., 0x45)`, vtable `0x02184B14`: +0x3C start `0x02134C74`, +0x34 per frame `0x02134ECC`, its hit handlers +0x2C/+0x30 `0x02134D38`/`0x02134D34` return 0; `0x0213482C` zeroes its speeds), its spawn byte 9 its facing (0: flag 4, left), flags 0x100 (the terrain not checked by `0x0201355C`), 0x20 and 0x20000 (not held in the stage). Params `prmDB04Ml0`: 60, 20, 60, -2.0, 2.0, 0, 0. A scene: `0x02134C74` takes Kirby's controls (`0x02012F94`: a handle on `0x01FF8180`'s list, released by `0x02012F68`; not in the stage's mode 3, `0x02209560`'s +0x75), hovering 60 frames (animation 1); `0x02134B94`/`0x02134ADC` its cape (2, through); `0x02134A30` held 20 frames (3); `0x021349E8` pointing (4, through); `0x021348F8` the controls back (mode 3: `0x0220A2A4(..., 0)`), flying off at (-2.0, 2.0) (5) and `0x0213488C` gone after 60 frames or out of the view (`0x02209D00`'s margins 1), its spawn record set to state 4 (`0x0220A2B4(..., +0x1C4, 2)`, `0x02218EF4`: not again till the area reloads). `demo_dancho.rca` has no boxes. Recorded on the DS: Kirby frozen 92 frames, then the flight. `0x02209D00`'s margins (`0x022275F4`, above, below, left, right in pixels): 0: 16, 48, 48, 48; 1: 24, 56, 56, 56; 2: 96, 104, 128, 128; 3: 192, 56, 56, 56; 4: none; 5: 24, 1024, 56, 56.

The Squeaks (Squeakers, Doc, Spinni, Storo, Daroach) and Parasol are created elsewhere. Their files: Squeakers `enemy/chun` (and `chun_blue`, `chun_green`, `chun_yellow` palettes; `prmDE01`, loaded at `0x020EC5F4`: its body near `0x020EC5xx`; its 3 palettes loaded by `0x020E8C80`; a thrown Squeaker, on the thrown-things base `0x020B49D8`, `0x020ED208`), Spinni `enemy/spin`, `spin_at` (`prmDB01`, `0x020AB764`, `0x020ABFC4`), Doc `enemy/doc`, `doc_at1` (`prmDB02`, `0x020DD8FC`, `0x020DF5E4`), Storo `enemy/stron`, `stron_atack` (`prmDB03`, `0x020E44A4`), Daroach `enemy/demo_dancho` in a stage (`prmDB04`, above) and `dancho00`, `dancho01` in his boss fight (`prmBB07`, enemy 0x3D). Copy abilities KatAM lacks (Ninja, Bubble,
Animal, Ghost, Metal): ported as KatAM abilities 35-38 one by one ("ROM Expansion/README.md", Abilities); Bio Spark gives Ninja, Gaw Gaw Animal, Bubble Head Bubble and Metalun Metal, as the DS's (Ghost isn't one of the scrolls' and isn't ported).

**Metal Kirby** (its constructor `0x020CB160`: `kirby/metal_cmn`, `metal_ext`, `metal_wat`, `normal_dmg`, `PrmKirbyMetal.kgb` and
`PrmKirbyMetalKirbyBase.kgb`, the latter in Kirby's own movement words' place at the player's `+0x154`; flags `|= 0x800000`, read
only by `0x0201355C`: the currents don't move him). Its movement words that differ from `prmKirbyBase.kgb` (20.12): 1 ground
slowing 0.1 to 0.15, 2-3 walk 0.16 to 0.03 a frame up to 1.42 to 0.5, 5 dash top 1.93 to 0.93 (unused: logged, his dash is his
walk), 6 jump 4.78 to 2.68, 7 its cut on letting go 1.0 to 2.5, 9 fall 2.5 to 4.5, 10-11 air 0.15 to 0.05 a frame up to 1.93 to
1.23, 13-14 float sideways 0.13 to 0.03 up to 1.1 to 0.35, 16-17 flap 0.47 to 0.27 up to 1.3 to 0.3, 19 float fall 1.03 to 2.03,
20, 21, 23, 25 (the slide's), 40-51 (water: he sinks). Logged in the DS's emulator (`metal_wd.dst`, 1-1): walk +0.03 to 0.50, let
go 0.50, 0.35, 0.20, 0.05, 0; the jump 2.51 the first frame, then 0.17 less a frame, about 20 pixels, the same with A held or
tapped; in the air +0.05 a frame to 1.23; floating (A tapped in the air) up 0.30 a frame, let go down 0.09 more a frame to 2.03.
His hurt boxes (`metal_cmn.rca`'s 0x102 records) shut out the attack kinds `0x034B` (0 touch, 1, 3, 6 grab, 8, 9; some frames
`0x036B`, 5 too), Kirby's own `0x000A`: kind 2 (the stage's hazards, Bohboh's lava) still hurts. B (`0x020CAF44`): in the air with
Down, the stomp (`0x020CADD0`, `0x020CAC54`, `0x020CAA64`); else with the scroll (22) the roll (`0x020CA9B8`, `0x020CA838`,
`0x020CA4E0`, `0x020CA3F8`); else nothing (`0x02017DE4`: no inhale). Its steps (`0x020CAFFC`, the vtable's +0x14): on the walk's
animation frames 16 and 46 of each 60 (`metal_cmn`'s 12: two steps a cycle, the 3-frame cells), the quake `0x02035FC4(…, 3, 5, 2)`
and sound `0xAC`. `metal_ext.rca`'s indexes are its `.cea`'s plus one: the touch boxes of 20 at .rca 25, 28 and 36 are the roll
(24), the stomp's fall (27) and the roll in the air (35). Its effects (`0x02018C24`'s types t: `efc_cmn0`'s animation 0x10 + t):
type 2, the roll's start, `efc_cmn0`'s 18; type 6, the stomp's landing, 22. Ported in `abilities/squeak_moves.c` (see its notes).

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

### Found while fixing the owner's playtest list
- **The third tile layer (grid 5) is never drawn** before its cell is cleared: BG1 is off in the rooms checked (4-6-3, 8-3-2,
  2-2-1), BG3 shows grid 4 where grids 4 and 5 differ, and a trigger-cleared column of 4-6-3 shows grid 5 after. Grid 5 is the
  look a cell takes once cleared (`SqueakRoomTile()`). Drawn over the room, it hid bomb blocks, floors, the planks over sealed
  doors and dirt walls (the owner's "invisible walls").
- **Switches** (vtable `0x02229B84`): they stay down while any stone door they opened is still open; once all are back in state 0
  (`0x02225298`, `0x0222524C`) they rise, and a touch or an attack presses them again. A new touch while down sends again.
- **Stone doors' cells**: the template keeps the four cells under the pillar's box (+0x16: air, solid, water, a chain link...), which
  it puts back when it opens, not what was there when the room loaded (a door shut on its own cells kept them solid).
- **Boulders** break the star blocks (0xF0) they roll into, on their side, a cell beyond their radius; waiting boulders (bits 4-5)
  have no physics until their message, as the DS's state 0.
- **Big star blocks** (front-layer block 5, 2x2): their four cells are one chain of bomb blocks (0xF1), broken together.
- **Lifts** (gimmicks 4, 5) carry a Kirby who attacks on them (KatAM's flags 0x800 and 0x2000000 mid-swing), and the chests
  standing on them (KatAM's chests fall as any object: one held from its first frame stays).
- **Stage 2-8 is a test stage**: rooms 2-6 have no tiles at all (only collision), room 1 a jumble of the generic `00_f`/`00_b`
  blocks, the background `d_viw_00`, its chests treasure 0, as 1-8 and 1-9. Left out of the port (`SqueakTestStages`).
- **Sky rooms** (3-4-5, 8-3-6, 8-3-7): a warp star flies across them and lands in the room after; the level planner counts the room
  the ride lands in (`SqueakRides()`), never the one it crosses.
- **Searching for chests**: the Squeaks look for the nearest chest and the one they touched every frame. Walking all 128 task slots
  for it was 17% of a lair's frame; they now follow the live task list (`NextObject()` in `squeakers.c`), as the game's own loop.
- **Lairs**: the fight's area is the whole room (`SqueakArena()`), not one screen over the landing floor. A lair whose fight the
  randomizer changed has its Squeakers' x set to 0x7FF0 (`OffStageX`): the game spawns only templates near the camera, so they
  never appear, and the Map Viewer skips templates there.
- **Big Metalun's rock droppers** (gimmick 8, fed by the random relay 0x17) rain only while a mini-boss is alive in the room with a
  Kirby inside its template's fight area (`MinibossFighting()` in `gimmicks.c`).
- **Rooms a ride crosses**: a warp star's ride flies from its room through others into the one it lands in; the DS puts a star at
  each crossed room's edge to carry the flight on (7-2-2: 7-2-1's star flies through it into 7-2-3). A crossed room's arrival is
  that edge, not a place to stand: the planner marks them (`SqueakPlanRoom.Crossed`) and throws Kirby into none.
- **Invincibility above room 0x383**: KatAM's invincibility (`sub_0808324C`, its task `sub_08083494`, the candy's `sub_081232AC`)
  is off in its special rooms, every room over 0x383; `SqueakRoomsInvincible()` makes the three tests skip 0x384-0x3FF only.
- **Floating bubbles' first byte** (gimmick 0x11's parameter): its high half 1 a food, 0 an ability; its low half 1 an item of the
  room's .doe list (the second byte its number: 4-6-8's chest, 8-1-11's big chest, the item's own flag 1), 0 by ID (the second byte
  the food's or the ability's; ability 0 the random one, as the "?" item, kind 0x15, which KatAM's random pedestal 0x96 stands for).
- **Hidden doors (flag 4)** are invisible on the DS (the Squeaks' temporary doors appear there); in the port they now carry a mirror.
- **Metalun on slopes**: its walk moves in lurches; one down a slope leaves it a few pixels above the ground. With floor under its
  feet it walks on (and its charge only drifts off a ledge), so it keeps its walk's pace downhill.
- **Gold Waddle Dees hold their room's chest**: each of the 7 (2-6-6, 3-3-7, 4-4-5, 5-4-5, 6-6-5, 7-6-9, 8-2-6) has enemy flag 8 (byte
  +0xB of its .doe record), as the bosses and some mid-bosses whose rooms' rewards come after the fight, and its room has one chest,
  a big one (kind 0x14) holding a piece of the Secret Map (treasures 93-99), placed high in the room. An item's flag bit 0 isn't the
  link: many chests and foods have it, with no enemy flagged. The port hides the chest until the Gold Waddle Dee is gone (beaten,
  swallowed, or fallen out of the room) and puts it where it went, or at its own spot over a pit (`chest.c`'s
  `SqueakEnemyChestWait`, `SQUEAK_CHEST_AFTER_ENEMY`). 2-6-6's walks off its logs into the pit within seconds unless Kirby gets it.
- **Big chests**: Squeak Squad's big (platinum) chests are KatAM's small chests in the port (the owner's call), so the platinum
  chest's graphics are gone from the ROM; KatAM's chests fall (`ObjectMain`'s gravity: nothing sets their flag 0x40).

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
  Bumping into Kirby runs the base class's `+0x48` and `+0x50` states. All 117 enemy vtables share them: `+0x48` `0x02022CB4`
  (hurt: a hit-stop counter at +0x13E up to the parameter block's +0x30 or +0x34, the sprite shaken by `0x02185E04`'s table,
  then defeated if out of HP, else the knock-back), `+0x4C` `0x02022B4C` (its start: speeds from the block's +0x3C/+0x44, or
  +0x50/+0x58, frames from +0x4C or +0x60 into +0x190), `+0x50` `0x020229C0` (the flight, to the ground and the timer's end);
  the block is the body's +0x1B4 (not found: not the `Sl0` files). The water state: `+0x6C` `0x02021D5C` (+0x190 = 42, the
  splash, `0x02215A10`'s, one of the two animations at +0x1D8/+0x1E0 at random, stopped, +0x90 -0x666: sinking 0.4) then `+0x70`
  `0x02021D30` each frame (moves, and once +0x190 is spent, `0x020235C0(1)`: defeated); `0x02021EF8` (+0x68) defeats one out of
  the camera's view. Ported: `squeak.c`'s SqueakDrown.
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
Dark Daroach's defeat (ported): `0x02124EFC` (his HP out; with flag 0x40 set, still in a hop, it waits) stops him (+0x8C-+0xA8
all 0), shakes the screen (`0x02035FC4` 3, 5, 2), clears +0xE4 (his wand's sprite), plays `dancho00`'s 27 (his beaten pose) and
sets his state to `0x02124DCC`: at +0x128 = params +0x7C (120) `0x02039B28` shows `efc_cmn0`'s 10 at him, `0x02124D18` makes
the 0x12C object of vtable `0x02183600` (`dancho00`'s 28, the dark star leaving him: thrown 1.0 ahead, 4.0 up, -0.1 gravity,
`0x02124B9C`; bouncing at 4.0 off the floor and turned round by walls, sound 0x7D, `0x02124A7C`; at rest `0x021249E8`), a bigger
shake (4, 15, 2) and `0x02075FF4(+0x204, "enemy/dancho00")` (his own colors back); at params +0x90 (600) `0x020B3CE4` (the stage
moves on). Daroach's (parameter 0) instead: at 60 `0x020CF07C`, at 120 the HUD's and sound's (the boss's shard follows). Params are
`PrmBB07Ml0`'s from its +0x10 (the port's `IDLE_SHORT` is +0x00). His hurt flash (ported: `squeak.c`'s `SqueakDamageColors`, the
character's `DamagePalette`): his constructor loads `enemy/dan_dmg` into +0x33C and his own colors (`dancho00`, or `dancho01` for
Dark Daroach) into +0x23C, 0x80 colors each (his 8 palettes); the enemies' base update `0x020144F4` counts his safe timer (+0xEC,
10 after a hit) down, then calls his vtable +0x20 (`0x02126B30`) with "still safe": it loads +0x23C + ((+0xEC / 3) & 1) * 0x100
into his sprite's palettes, so `dan_dmg`'s colors (his body red) show at counts 9, 5, 4 and 3, his own otherwise; Dark Daroach
flashes in `dan_dmg` too. His beam (`dancho01.mca`'s 2 and 3): its first part (animations
11 and 13, 48 ahead) reaches 104 pixels on from its spot, the 16-pixel pieces of 12 and 14 go from 152 to 248; a 128-pixel frame
cut the first part at 64 (the port's frames are 256 across).

His fight's lag (the owner: "Massive amounts of lag whenever the Squeak Squad boss invokes stars."; measured in the emulator on Dark
Daroach's room 0x5DC, 1800 frames from the same start, a frame "overloaded" when the CPU had under 8000 cycles left: 258 before, 29
after, 5 of them the room loading; Daroach's 0x56B: 11). What it was and what changed:
- **His beam's 16 sprite parts** (each a KatAM effect object, its VRAM and tile copy): parts that play in step are drawn by one sprite
  (`KatAMExpansionSqueak.cs`'s `SqueakMergeParts`, the character's `MergeParts`), cut in runs at most 128 pixels across so each finds
  a hole in the VRAM heap (the whole beam's 183 tiles found none for half a second): the beam is 2 sprites and the 7 sparkles. A
  merged cell counts its pieces' tiles from its first cell's (a piece's 10 bits reach 4096 tiles; his cells are past that in his two
  files' tiles, which first drew the beam as scattered bits of other frames).
- **VRAM.** KatAM's stage heap is 512 tiles. His two beam halves (drawn by nothing) each took his biggest frame's 103 tiles: now none
  (`squeak.c`'s `SqueakNoSprite`: unkC 0x200, as the NiDL bosses' own objects, and flag 8). His stars, bomb and wand (`FitVram`) take
  the frame they show (a star 16, the wand 13, the fire column's base 103), grown when a bigger one shows (`SqueakFitVram`, the tiles
  per frame in `SqueakData.cellTiles`); before, three stars took 309 tiles and the beam none was left.
- **Failed VRAM.** KatAM's `VramMalloc` gives 0 (not `ewram_end`'s address) when the heap is full, and its object update
  (`sub_0809A630`) asks again for the type's 123 tiles every frame a sprite has 0; each failed search tries every free segment against
  the ones after it, a fifth of a frame. A Squeak part or character that waits for VRAM now asks only when `SqueakVramFits` (a walk of
  the heap's blocks) finds a hole that big, and reloads its frame's tiles once it has it (KatAM copies them only as a frame starts).
- **His beam's sound** (0x1AF, archive 14's 8): 42 one-tick notes a tick apart, each ringing about 30 frames on its release (0xEB), so
  KatAM mixed up to ten voices (about a fifth of a frame). The converter caps that sound's release at 0x60 (`SqueakSoundReleaseCaps`):
  the same notes, pans and bends, each ringing about 4 frames. The DS's own players set no channel limit for it (their masks are 0).

Dark Nebula's fight (the owner: "massive lag when summoning stars. You can use the game's existing stars", "fire attack looks very
off and it's very laggy"; measured the same way in his room 0x5E0, his RNG seeded so one run goes through his spark, ice and fire
forms, 3000 frames):
- **His stars** are KatAM's own: Dark Mind's shots (objects 0xCB-0xCE, fire, ice, spark, mixed, a kind at random), moved by KatAM,
  inhaled as a star to spit back (no ability), a puff on a Kirby (`dark_nebula.c`'s `Shoot()`, `StarMain()`).
- **Palettes.** KatAM keeps 7 enemy palettes a room (OBJ slots 8-14, `gUnk_02022E60`) and loads past them into the last, over the
  palette there (`sub_0803DFAC`). His fight wanted 8 (his black star's, his three forms', his fire and ice attacks' (cmn's palette
  4), his sparks', the burning floor's, the Dark Mind stars'): his fire wall drew in the burning floor's colors, an olive block with a
  blue edge. His forms now share one slot (`PlayForm()`, `squeak.c`'s `SqueakTakePalette`): one form shows at a time.
- **His fire wall** (cmn's `.mca` 0) has six parts: three 128 pixels over the floor and three 128 more over those. The top row is
  over KatAM's view (the DS's is 16 pixels taller; his room's camera doesn't move) and took 148 tiles of VRAM, so the bottom right part
  found none: it's left out (`KatAMExpansionSqueak.cs`'s `HighParts`), and the burst takes the frame it shows (`FitVram`).
- **The burning floor** was a thing a 16-pixel tile (up to 16 with his fire wall up: a lag every other frame for about 100 frames).
  Each flame now has one trail (`Ignite()`, `Burn()`): hidden, drawing each burning tile with the trail's sprite (KatAM's
  `DisplaySprite` for each, as `sub_0809A7A4` draws an object), one box across its tiles; the same 47 frames a tile.
- **His spark cage** (ext's `.mca` 0: 14 parts in step, a lag every third frame for about 60 frames) is drawn by a few merged sprites
  (`MergeParts`, as Daroach's beam).
- **Spawning.** KatAM's `ObjectInitSprite` asks for the Squeak type's 123 tiles for every Squeak object made, which the character's
  own size replaces at once (`SqueakCreate`); with the heap without 123 in one piece that failed search took a sixth of a frame a
  spawn. A Squeak object's sprite is now made as for one in another room (no VRAM asked for), then given its own.
- Measured: 3000 frames, 119 overloaded with the palette and fire wall fixes in, 36 after the rest (the room's loading 1, the cage
  9 where it was about 40, the fire attack 8 where it was about 50, a few as Kirby is hit).

His ice floor and his look (the owner: "When using its ice attack, it doesn't add the ice platforms at the bottom.", "Dark Nebula's
sprite looks off."):
- **The ice floor.** His pillar (`0x0212EBC4`) calls the stage's vtable +0x8C (`0x022146EC`) for each 16-pixel tile of the floor's
  top row it passes: only a plain floor tile (collision 0x4000, not burning, not ice already) turns to ice (collision 0x2900), its
  block from a8s4s6's front table by its neighbors (0xC0 alone, 0x120 with ice on its right, 0x180 with ice on its left, 0x1E0 both)
  plus the tile's x % 3 (`0x0200C80C(x, 3)`'s remainder), the tile over it the block - 0x20; its neighbors' blocks moved on as well
  (+0x60 the left one, +0xC0 the right one). It stays through his other forms till his fire's flames burn over it (`0x02214560`).
  Here: the iced tiles are bits 16-31 of his burning-floor word (`BURNING`, a 16-pixel tile each from the DS view's left), set by
  the pillar, cleared by `Ignite()`; his aura (alive all his fight) draws them (`IceFloor()`): 13 looks
  (`KatAMExpansionSqueak.cs`'s `SqueakIceFloorLook`, `ReadSqueakBlocks`: the 4 kinds by x % 3, and the three between blocks side by
  side, one sprite for three of a long row's), copied once into a VRAM block of their own, each drawn with `DisplaySprite` on the
  room's own tiles, its bottom on the bottom of the floor's top row. Not slippery: KatAM has no slippery floor.
- **The blocks come out mirrored** unless turned before: `SqueakCanvas()` mirrors every Squeak Squad cell (the DS's face right,
  KatAM's frames left); a stage's blocks are drawn as they are, so `ReadSqueakBlocks` gives each cell mirrored (each piece on the
  other side, its flip turned). Before that the left and right ends were swapped (an outline between the end and the next block).
- **His aura** (cmn's animations of the purple outline with its flames and its white rim) faced left (`Thing()` sets every thing
  facing left) while he faces right: it was drawn mirrored about his origin, off his star's outline (thick on one side, the white
  rim lost). It now faces as he does.
- Checked in his room 0x5E0, the RNG seeded, his states forced (his pillar, then his fireball): the ice in all three versions, the
  fireball's flames melting it tile by tile, his fire wall and the burning floor drawn with it. Lag: the ice's drawing a third of
  the sprites it was; with his burning floor up the overloaded frames are the burning floor's own (the same every-third-frame
  pattern with and without the ice). Not seen in a run that wasn't forced: Kirby idle dies as his first pillar starts.

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
His stage's tint: the stage's struct at `0x02209F84` (+0x1C frames gone, +0x20 length, +0x24-0x2C the colors' levels now, +0x30-0x38
from, +0x3C-0x44 to, 20.12, red/green/blue), set at his fight's start (`0x021225B4`, after ML[6]'s 50 frames) and at each form pick
(`0x021222E4`), ML[7] = 120 frames: his form's own channel 0x1000, the other two 0x808 (fire red, spark green, ice blue); none, all
0x1000. His HP bar: `0x020B308C` at the same start (the HUD's boss bar filling over 60 frames, sound 0x177). His music: never set by
him (no `Macro_08100F18` call): the stage's map music, 37. The stage's +0x9C bit 2 is set as the screen whitens at a form change
(`0x021220D0`) and cleared as it clears (`0x02121D14`); only Kirby's states `0x02017B60`, `0x02037DC4` and `0x020BBD9C` read it. In
KatAM the tint goes on the far view (BG palettes 6-13) and must be written into KatAM's copy of the loaded colors too
(`gUnk_02022120`): KatAM's palette effects (a copy ability's flash: `sub_0805C11C`, `sub_0803CBC4`) end by copying it back over the
BG palettes (`sub_0803A1F4`).

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
spawn (24, 964). Recorded in both (`kss/ds/load_st.py`, `rec_ds.py`: Kirby driven by a script). In KatAM the converted rooms
carry their route (`gSqueakDarkStarRoutes`: the zone per tile and the points, written by `SqueakDarkStarRoute()`; agbcc pads the
point struct of three `s16` to 8 bytes, so the points are 8 bytes each), which the star follows as above (`RouteHop()`,
`RouteLine()`); its spot is the record's own, not stood on a floor, and the converter's door rule doesn't drop it. Replayed against
the recordings (Kirby forced to the DS's positions and speeds each frame): the corridor within a few pixels early and about 25
later, end and loop on the same frames; the shaft to the same end, off the DS's path for a while half-way.

**Composite parts' first frame**: a part's sprite is made on the cell it shows (`SqueakCreatePart`'s `cell`). KatAM's sprite init
(`EffectObjectInitSprite` → `sub_08155128`) queues its first frame's tiles at once; made on the character's cell 0, a part with a
small VRAM block got cell 0's tiles (Meta Knight's tornado: its 103-tile column) written over the blocks after its own, and a
sprite there that didn't change frames kept the garbage (his sword's layer, from his tornado's call to its end). Found by dumping
the upload queue (`gUnk_03002EC0`, 64 entries: source, VRAM, size) on the frame the layer's tiles changed.

## Sounds

All of Squeak Squad's sounds are in `sound/sound_data.sdat` (42 music sequences, each with its own bank; 22 sound-effect archives;
83 banks; 2 wave archives, every wave IMA-ADPCM; no name table, no streamed music). Note-wait (a note's length also a wait before
the next command) is on when a track starts; every music track turns it off (`C7 0`), the sound effects never do; no call is nested.
A sound effect that loops (the dark star's) relies on its notes' waits: converted without them it looped with no wait in one frame
and froze KatAM (MP2K runs a track till it waits). The conversion follows each track's mode, and a loop back with no wait in it
gets one (`KatAMExpansionSqueakSound.cs`'s `Block()`; every converted song scanned: none left).

- **Music.** The DS's 43 music ids (USA arm9 `0x02159770`, 4 bytes an id: its sequence the second u16). A room's music id is its
  map's byte `0x0B` (the decompressed `.mxp`; RLE `0x30`): 0 keeps the one playing, 24 is the side rooms' (the treasure rooms'). The
  stage manager (`[0x0222C040]`) keeps it at `+0x74`. The boss arenas (each world's last stage, its second room) set the boss
  themes: 21 (worlds 1 and 7), 33 (2 and 5), 34 (3, 4, 6, 8); the rooms before them 29. The mid-bosses' own fight music wasn't
  found.
- **Sound effects.** The DS's sound ids (USA arm9 `0x02159DDC`, 12 bytes an id: word 0 its index in its archive << 16 | its
  archive). Played by `0x02013EE8` (object, id: once), `0x02013E6C` (object, id: kept in the object's list at `+0x108`, a loop or a
  long sound) and `0x02013DDC` (object, id, fade: stops it), `0x02013D88` (stops them all), or straight by `0x020B0604` (the sound
  system's handle `[0x02185BA4]`, id); `0x02013EE8` itself is `0x020B0604` on the object's handle.
- In KatAM (`KatAMExpansionSqueakSound.cs`): the 43 music ids, then the 760 sound ids, follow NiDL's songs in KatAM's grown song
  table (`gExpansionSongs[1]` the first); a sound id that names no sequence is an empty song.

## Test harness (DS emulator)

`py-desmume` with a save state after Kirby lands in Prism Plains' first room. The first stage record is rewritten (ID, x, y,
parameters) and the enemy spawns next to Kirby; breakpoints on its state functions and its fields above give its moves frame by
frame, to compare with the port in KatAM (mGBA, same fields in KatAM units).
