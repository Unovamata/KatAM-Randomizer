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
| `.rca` | `BRCA` | Not decoded yet (likely the hit boxes). |
| `.mca` | | Not decoded yet (the bigger characters). |

### `.cea`

- Header: `BCEA`, `0x11`, `0x0B`, the cells' offset (`0x14`), the animations' offset.
- Cells: `u16` count, `u16`, `u32` the OAM list's offset (from the cells' offset), then one 8-byte entry per cell: `u16` OAMs,
  `u16` the first OAM's index, `u16` the cell's first tile, `u16` its tile count. The OAM list: a `u32` count, then 8 bytes an
  OAM (`attr0`, `attr1`, `attr2`, padding: the DS's own OAM, with the tile number in `attr2` counted in 4-tile steps from the
  cell's first tile).
- Animations: `u16` count, `u16` 0x0C, `u32`, then one `u32` offset (from the animations' offset) per animation. An animation:
  `u16` frames, `u16` flags (1 loops), `u32` total length, then 8 bytes a frame: `u16` cell, `u16` frames shown, `u32`.
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

## Stage objects

The stage's objects are 16-byte records: `*(*0x02229F1C + 0x90)` holds a `u16` count and, at `+4`, a pointer to them. A record:
`+0` its index, `+1` its kind (0, 5 and 6 are spawned through the factory), `+2` `u16` enemy ID, `+4` `u16` x in pixels, `+6` `u16`
y in pixels from the room's bottom (`*0x02229F1C + 0x60` is the room's height, `fx32`), `+8`, `+9`, `+0xA` its parameter bytes.
`0x0202EC50` spawns record `index` when the camera reaches it (the factory call is at `0x0202ECAC`).

The first room of Prism Plains' first stage holds a Waddle Dee (x 636), a Waddle Doo (x 1128) and a Sparky (x 1512). To watch any
enemy, the test harness loads a save state after Kirby lands there and rewrites the first record (ID, x, y, parameters): the enemy
appears next to him.
