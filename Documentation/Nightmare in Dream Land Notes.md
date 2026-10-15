# Kirby: Nightmare in Dream Land (USA, `AGB-A7KE`) — porting notes

Reverse-engineering notes gathered while porting enemies (see `ROM Expansion/`). All addresses are NiDL USA unless marked KatAM.

The Japanese (`A7KJ`) and European (`A7KP`) ROMs have the same enemy data: their frame tables are `0x3A8C` / `0x23C7EC` further than the USA ones, and their descriptor tables are at `0x08742DAC` / `0x0897B678`.

## Engine

- **Objects** live in IWRAM, an array of `0x90`-byte slots at `0x03002790` (slot `n` = `0x03002790 + n * 0x90`). `0x03002490` points at the object being updated, `0x0300248C` holds its slot.
- `0x03004CA0[slot]` (s16) is the object type of each slot (`-1` free).
- **CreateObject(type)** `0x0800579C`: takes a free slot, copies the category from the master table and starts its spawn function as the slot's coroutine.
- **Master object table** `0x0872FF30`: `{u32 category, u32 spawnFunction}` per type. Category 3 = enemies/bosses.
- **Coroutines**: every object runs a script with its own 256-byte stack (`0x0203BFE0 + slot * 0x100`); script function per slot at `0x030025F0[slot]`. `0x08006148(fn, slot)` starts a script, `0x080CFDCC(n)` (ARM) waits `n` frames, `0x08006138` ends it (waits forever).
- **Dispatch** `0x08002E98(index, count, table)`: calls `table[index]` if `index < count`. Objects keep the state to start at `+0x14` and the current state at `+0x15`; each enemy has an init table (scripts) and a per-frame table.

### Object fields (offsets from the slot)

| Offset | Meaning |
|---|---|
| `+0x04` | Per-frame main function |
| `+0x12` | Category |
| `+0x14` / `+0x15` | Next / current state |
| `+0x28`, `+0x2C`, `+0x34` | Script-specific (Broom Hatter: `+0x28` script finished, `+0x34` last action) |
| `+0x38` | Frame table pointer (per enemy) |
| `+0x3C` | Current frame index |
| `+0x3E` | Bit 15: horizontal flip |
| `+0x43` | Facing (s8): `1` right, `-1` left |
| `+0x48` / `+0x4A` | X / Y in pixels (s16) |
| `+0x00` | Motion function (`0x080656B4` adds the accelerations to the velocities, caps them and moves; 0 freezes the object) |
| `+0x4C` / `+0x50` | X / Y in 16.16 |
| `+0x54` / `+0x58` | X / Y velocity, 16.16 pixels per frame |
| `+0x5C` / `+0x60` | X / Y acceleration |
| `+0x64` / `+0x68` | X / Y speed limit (`0x80000000` none) |
| `+0x7A` | Set by the collisions when landing |
| `+0x73` | Variant (from the room object) |
| `+0x8C` | Pointer to a sub-structure (`+0x1A`: extra sprite frame, e.g. broom dust, `0xFFFF` none) |

### Helpers

| Address | What it does |
|---|---|
| `0x080061C0(vx, vy)` | Sets velocity, mirrored by facing; `0x5A5A5A5A` keeps a component |
| `0x080062C4` | Stops: zeroes velocities, accelerations |
| `0x08006304` | Applies the facing to the sprite flip |
| `0x08006338(f)` | Sets frame `f` and applies the flip |
| `0x0800617C(f)` | Sets frame `f` (or `f | 1` when facing left), flip unchanged |
| `0x0806395C(s)` | Sets the next state |
| `0x08063E14` | Faces the closest Kirby (`0x08063B38` finds it by X distance) |
| `0x08064038` | Turns around |
| `0x08002EE8(n)` | Random number below `n` (12-bit LCG at `0x03000FB4`: `seed = seed * 61 + 0x579`) |
| `0x080692FC` | Common per-frame checks (hit, inhaled...); non-zero skips the enemy's own state |
| `0x08068E04`, `0x08069B44` | Physics/collision and drawing |

## Graphics

- **Frames**: the frame table (`+0x38`) is a list of pointers to 8-byte OAM records: `y (s8), attr0 high (shape << 6, 0x10 = last record), x (s8), attr1 high (size << 6, flips), flipped x (s8), flipped attr1 high, tile (10 bits) + priority`. The unflipped frames face right.
- **Per-enemy graphics descriptor** (Broom Hatter `0x0824D150`): tile count, palette pointer, LZ77 tile pointer, frame pointers.
- Tiles are BIOS LZ77 (`0x10`) and copied into OBJ VRAM with **2D mapping** (32 tiles per row); the Broom Hatter's block is 16 tiles wide.
- Graphics are loaded per room, for the enemy types the room's object list contains. Changing a room record's enemy index loads that enemy's graphics; changing the spawn function only does not.

## Levels

- Room object records are 8 bytes: `category+1, index, variant, flags, x (u16), y (u16)`. Stage 1-1 (first room) records start at `0x0835DD68`.
- `0x08064FC0(category, index, ...)` maps a record to an object type with per-category tables: enemies `0x0873F198` (index `i` -> type `8 + i`), mini-bosses `0x0873F23C`, bosses `0x0873F264`...
- Entering stage 1-1 from boot: see the emulator scripts (Start, A x4 through the menus, Left + Up at the first door).

## Enemy types (category 0, type = index + 8)

Spawn functions are in the master table (`0x0872FF30`), frame tables are the `0x0875xxxx` literal each spawn function loads, and graphics descriptors are at `0x0873EEA0 + index * 4` (`+2` tile count, `+8` palette, `+12` LZ77 tiles; null for enemies drawn with shared graphics). Names from WiKirby's NiDL enemy list, matched by sprite.

| Type | Enemy | Frames | In KatAM |
|---|---|---|---|
| `0x08` | Waddle Dee | `0x08752118` | yes |
| `0x09` | (headband, not a regular enemy) | `0x08752194` | - |
| `0x0A` | Noddy | `0x08752150` | yes |
| `0x0B` | **Broom Hatter** | `0x087521D8` | no (ported) |
| `0x0C` | Pengy | `0x08752204` | yes |
| `0x0D` | Laser Ball | `0x08752248` | yes |
| `0x0E` | **Chilly** | `0x08752284` | no |
| `0x0F` | Sir Kibble | `0x0875230C` | yes |
| `0x10` | **Cappy** | `0x0875235C` | no |
| `0x11` | Waddle Doo | `0x087523A0` | yes |
| `0x12` | Gordo (no descriptor) | `0x08752108` | yes |
| `0x13` | **Cool Spook** | `0x087523EC` | no |
| `0x14` | Bronto Burt | `0x0875240C` | yes |
| `0x15` | **Kabu** | `0x08752438` | no |
| `0x16` | Bomber | `0x0875246C` | yes |
| `0x17` | **Coconut** | `0x087524A4` | no |
| `0x18` | **Twizzy** | `0x08752560` | no |
| `0x19` | Shotzo | `0x087524E4` | yes |
| `0x1A` | Sparky | `0x08752584` | yes |
| `0x1B` | Twister | `0x087525E4` | yes |
| `0x1D` | Scarfy | `0x0875265C` | yes |
| `0x1E` | **Bubbles** | `0x087526A8` | no |
| `0x1F` | **Starman** | `0x08752708` | no |
| `0x20` | Hot Head | `0x0875275C` | yes |
| `0x21` | Glunk | `0x087527DC` | yes |
| `0x22` | **Slippy** | `0x08752828` | no |
| `0x24`, `0x25` | Sword Knight, **Blade Knight** (same code) | `0x087529D8`, `0x08752A24` | Sword Knight only |
| `0x26`, `0x27`, `0x28` | **Poppy Bros. Jr.** (alone, on an apple, with a bomb) | `0x087528D0`... | no |
| `0x29` | **Coner** | `0x08752A70` | no |
| `0x2C` | **Needlous** | `0x08752B08` | no |
| `0x2D` | UFO | `0x08752B4C` | yes |
| `0x2F` | **Gip** | `0x08752BD4` | no |

Most enemies' first frames (0-3) are their hurt frames. Frames are drawn around the object's position, about 24x24.

## Broom Hatter (type `0x0B`)

- Spawn `0x0808D4E8`, main `0x0808D58C`; variant spawns `0x08743188`; init scripts by state `0x08743194`, per-frame by state `0x087431A0`.
- Scripts: `0x0808D640` sweep (2 loops), `0x0808D790` back and forth (3 loops), `0x0808D964` stand; next action `0x0808D3E4` (table `0x0874313C` = `0,1,2,0,1,2,0,1`, never the last one); ledge/wall stop `0x0808D364`.
- Velocity tables `0x08743144` (1.0, 0.5, 0.25, 0.25, 2.0) and `0x08743158` (1.0, 0.5, 0.5, -1.0, -0.5, -0.5).
- Frames `0x087521D8`: 0-2 dizzy, 3 knocked away, 4-7 sweeping, 8-10 broom dust. Palette `0x0824CCF8` (the dust uses palette 15, white), tiles LZ77 `0x0824CD18`.

## Coconut (type `0x17`)

- Spawn `0x0808E8D4`; variants 0 and 1 (`0x0808E914`, facing right or left) hang on frame 5 until Kirby is within 7 pixels sideways (`0x0808E9D4`), then fall (`0x0808EA00`): gravity and max fall by the record flags byte (`0x08743214` 0x1500/0x3000, `0x0874321C` 3.0/4.0), tumbling through frames 7-15 every 2 frames while turning around 4 times. Landing (`0x0808EB10`) it explodes (`0x0806A344`). Variant 2 (`0x0808EB24`) rocks on frames 4, 5, 6, 5 (6 frames each).
- Frames `0x087524A4`: 0-3 hurt, 4-6 rocking, 7-15 tumbling.

## Coner (type `0x29`)

- Spawn `0x0808FA10`, variants `0x087432F4`. Variant 0 (`0x0808FA50`) faces Kirby and scuttles (`0x0808FAB4`): speed 1.0 (frame 5), 0.5 (frame 6), 0.25 (frame 5), 0 (frame 4), for 8, 8, 16 and 16 frames (`0x087432EC`, 4 bytes per record flag; the second set is 4, 4, 6, 6). Variant 1 (`0x0808FBAC`) does it in place (`0x0808FC00`). States 1 and 2 (`0x0808FB50`, `0x0808FB80`) sink and swim in water (`0x080692FC` picks them).
- Frames `0x08752A70`: 0-3 hurt, 4 standing, 5-6 scuttling.

## Starman (type `0x1F`)

- Spawn `0x08080DD0`, variants `0x08741544` (4); variant 0 (`0x08080E10`) has 6 states (`0x08741554`, per frame `0x0874156C`): walk (`0x08080EA4`, 1.0 from `0x087414F8`, frames `0x08081814`) until Kirby is within 47 pixels; crouch and take off (`0x08080F34`/`0x08080FA4`: 2.0 up, steering 0x1900 a frame towards Kirby up to 2.0) until 9 pixels above the screen (`0x03002158` + 4 is the camera's y); hide (`0x08081084`/`0x080810C4`) for 78 frames (`0x08741500`), following Kirby's x but for the last 30 (`0x08741502`); dive (`0x08081140`: 2.0 down, gravity 0x1500, at most 3.0), bounce 2.0, then stand 60 frames and walk again, or the second time fly away (`0x08081274`: 0.4 forward, rising 0x1900 faster a frame). State 4 (`0x080812F0`) swims.
- Frames `0x08752708`: 0-3 hurt, 4-10 walking, 11-12 crouching, 13-18 flying, 20 diving.

## Needlous (type `0x2C`)

- Spawn `0x0807BD20`, variants `0x08740A78`; variant 0 (`0x0807C080`) states `0x08740AC8`, per frame `0x08740AE0`, landing handler `0x0807BE08`, wall handlers `0x0807BEAC`/`0x0807BED4` (turn around; while rolling, stop 16 frames). Start (`0x0807BD7C`) walks one time in 4, else curls.
- Walk (`0x0807BF0C`/`0x0807C138`): 0.5, frames `0x08740A98`; at most 240 frames (`0x08740A80`) or 60 with Kirby within 9 pixels vertically (`0x08740A88`, `0x08063D2C` is dy to Kirby); 150 frames after rolling before it can curl again.
- Curl (`0x0807BF74`): 3.0 up, gravity 0x2500, frames 7, 8, then spin `0x08740AAC` (14, 15, 11, 13, 16, 12); bounces 2.0 (`0x0807BFD0`) and 1.0 (`0x0807C000`); roll (`0x0807C030`/`0x0807C2E0`) 3.0 (`0x08740A90`) for 60 frames.
- Frames `0x08752B08`: 0-3 hurt, 4-6 walking, 7-8 hopping, 11-16 spinning (15 and 16 mostly color 15, white: the spin's blur).

## Blade Knight (type `0x25`, Sword Knight's scripts)

- Spawn `0x0807AF98` (Sword Knight `0x0807AF58`), variants `0x08740990`; variant 0 (`0x0807B300`) states `0x087409FC`, per frame `0x08740A1C`.
- Walk (`0x0807B3F8`/`0x0807B430`): 0.5, frames `0x0874099C`, stops at ledges (`0x0807B088`); within 63 pixels (`0x0807B294`) one time in 4 raises the sword (frame 15, 16 frames, `0x0807B49C`) else lowers it (frame 16, `0x0807B584`).
- Slashes: down (`0x0807B4F0`: 14, 12, 11, 10, 16) and up (`0x0807B5D8`: 10, 11, 12, 14, 15), 2, 2, 1, 2, 8 frames; the hitbox comes on the second frame (`0x0807B1E4`); they lunge (`0x0807B0B4`: 4.0, 3.0, 2.0, 1.0, 0, every 3 frames, `0x087409E4`). After one (`0x0807B16C`, `0x0807B1A8`): the other once, one time in 4, else back off (`0x0807B66C`: -0.5 facing Kirby, 90 frames).
- `0x0807B200` swings (`0x0807B6E8`) at something within 30x48 pixels while Kirby is further than 63.
- Frames `0x08752A24`: 0-3 hurt, 4-9 walking, 10-16 slashing.

## Slippy (type `0x22`)

- Spawn `0x08089C98`: put in water (tile attribute 128 or more, `0x080224B0` sets `obj+0x7B` to 3) it starts swimming (state 6, frame 7, heading 0 facing right else 256).
- Swimming: drift (`0x0808A298`, 6) heads at 0.5 (`0x0806421C(angle, speed)`: polar velocity, 512 a circle, 0 right, 128 down; cosines `0x0872FB30`) for 26 frames, 3 times in 4 30 more, paddles (4, 5, 6, 5, 9: 3, 3, 4, 2, 3 frames), then kicks, one of 3: `0x0808A3C4` (7) at 3.0, 2.0, 1.0; `0x0808A4D0` (8) waits 10 frames (frame 9), then 2.25, 2.0, 1.0, 0.5, again one time in 3; `0x0808A610` (9) waits 10, then 3.2, 2.83, 1.41, 0.71, then 8 one time in 3. Every step lasts 8 frames, frame 7.
- Kick headings `0x08742862`, 8 per kick, indexed by Kirby's octant (`0x08064314`: 0 below right, more below; 1 below right, more right; 2, 3 below left; 4, 5 above left; 6, 7 above right): straight `128 0 256 128 384 256 0 384`, the other way `0 128 128 256 256 384 384 0`, diagonal `64 64 192 192 320 320 448 448`.
- Out of the water (`0x08021C14(x, y)`: water at a point; 8 pixels above, `0x0808A36C`) it leaps (`0x0808A110`, 4): bobs 2.0, 1.0 down, 1.0, 2.0 up (4 frames each), then jumps 5.0 up and 1.0 towards Kirby, gravity 0.25 (frames 8, 6, 5). Into the water from a jump (`0x0808A204`, 5): 0.5 forward, heading diagonally down, 40 frames (frame 5). State 10 sinks (gravity 0x1500).
- Variant 1 (`0x0808A7A8`): faces Kirby, frame 4, nothing else.
- KatAM: swimmers set `unkC & 1` (the engine kills anything else in water, `sub_0809D710`); the engine has no terrain outside the level's bounds, so they keep themselves inside (`NidlKeepInLevel`); its swimmers stop in the surface's 16 pixel tile, so the port looks 16 pixels up instead of 8.

## Spawn functions

The master table (`0x0872FF30`) has 8-byte entries, `(kind, spawn function)`, so type `t`'s spawn is at `0x0872FF34 + t * 8`: Chilly `0x08084CD4`, Cool Spook `0x0807EDB4`, Starman `0x08080DD0`, Sword/Blade Knight `0x0807AF58`/`0x0807AF98`, Poppy Bros. Jr. `0x080819B4`/`0x08081F60`/`0x08082008`, Coner `0x0808FA10`, Needlous `0x0807BD20`, Gip `0x0808BDB4`.

## KatAM side (USA)

- `gSpriteTables` `0x083B909C` (anims, attrs, oamData, palette, tiles4bpp...); 939 anims. Tile and palette indexes are relative to the table bases, so new data must be aligned to them.
- Per-type tables: spawn functions `gSpawnFuncTable1` `0x0835107C`, parameters `gUnk_08351648` (0x18 bytes: hurt anim indexes, score, HP, ability, palette key, VRAM tiles, start function, AnimInfo), `gUnk_08352AD0` (8 bytes).
- Free object types: `0x2C` is the randomizer's "Nothing"; `0x31` is free (used by the Broom Hatter). `0x40-0x44` count as bosses.
- HP bar names: `gUnk_08D60EE4[language]` + type * 0x100, 64x8 images; language 1 is English.
- Collision attributes: bit 0 marks an *empty* tile (Boxin's ledge check).
- Gravity: `ObjectMain` pulls every object down (`0x0809A990`: 0x15 per frame, at most 3.0, NiDL's 0x1500 and 3.0) unless `flags & 0x40` (and not `0x20`, which it sets once falling). Flying enemies set `0x40` in their start; getting hit clears it.
- Explosions: `0x08073D2C` is the small one Foley and Scarfy explode with (40x40, hurts Kirby); `0x08074420` is Bomber's (128x128).
- Free space from `0xF40000` (the randomizer's 9ROM lists) to the end of the 16 MB ROM. The expansion pads the ROM to 32 MB (the most the GBA maps, `0x08000000`-`0x09FFFFFF`; the game saves to SRAM, so no EEPROM range gets in the way) and puts everything at `0x09000000`.
- The KatAM decompilation builds byte-identical to the USA ROM, so `katam.elf` gives every symbol to link new code against (`--just-symbols`); `ld` adds interworking veneers for the long calls.
