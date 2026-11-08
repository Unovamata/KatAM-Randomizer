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
| 0x2D-0x36 | | The mid-bosses (graphics loaded elsewhere; to be matched) |
| 0x37 | metaknight_cmn | Meta Knight |
| 0x38 | dedede | King Dedede |
| 0x39 | | (Mecha-Kracko?) |
| 0x3A | mrsMoley_cmn | Mrs. Moley |
| 0x3B | yadgaine | Yadgaine |
| 0x3C | bohboh | Bohboh |
| 0x3D | | (Daroach?) |
| 0x3E, 0x3F, 0x46 | darkZERO_cmn | Dark Nebula / Dark Daroach (to be matched) |

0x12, 0x1B, 0x29, 0x2F, 0x32, 0x40-0x45 and 0x47 fall to the default case. Parasol (the parasol some Waddle Dees, Waddle Doos and
Shotzos hold) and the Squeaks (spin, doc, stron, chun) are created elsewhere.

## The cast to port

Squeak Squad's enemies KatAM has (its own objects, with only the Squeak Squad behaviors it lacks to add): Waddle Dee, Bronto Burt,
Blipper, Glunk, Scarfy, Gordo, Snooter, Chip, Batty, Blockin, Big Waddle Dee, Shotzo, Waddle Doo, Hot Head, Flamer, Pengy, Sparky,
Twister, Sword Knight, Cupie, Sir Kibble, Laser Ball, Foley, Wheelie, UFO, Noddy, Boxin, Heavy Knight; mid-bosses Mr. Frosty,
Bonkers, Box Boxer, Boxy. The others get a full port (body and brain constructors, from the factory; sizes in bytes):

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
| 0x2D-0x36 | the mid-bosses | 0x020F4294, 0x020D4C10, 0x020FD418, 0x020F8B78, 0x02100300, 0x0210C4F8, 0x02103B4C, 0x02108EAC | | |
| 0x37-0x46 | the bosses | Meta Knight 0x02116C7C, Dedede 0x020E7E58, 0x39 0x02132D7C, Mrs. Moley 0x0212CF40, Yadgaine 0x0211A4E8, Bohboh 0x02111370, 0x3D 0x02126F48, Dark Nebula / Dark Daroach 0x0211B680, 0x02122CD0, 0x021376EC | | |

The Squeaks (Squeakers, Doc, Spinni, Storo, Daroach) and Parasol are created elsewhere. Copy abilities KatAM lacks (Ninja, Bubble,
Animal, Ghost, Metal): the owner decides; meanwhile those enemies give none.

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
