# Kirby: Nightmare in Dream Land (USA, `AGB-A7KE`) — porting notes

Reverse-engineering notes gathered while porting the Broom Hatter (see `ROM Expansion/`). All addresses are NiDL USA unless marked KatAM.

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
| `+0x4C` / `+0x50` | X / Y in 16.16 |
| `+0x54` / `+0x5C` | X / Y velocity, 16.16 pixels per frame |
| `+0x60`, `+0x68` | Acceleration / max speed |
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

| Type | Enemy | In KatAM |
|---|---|---|
| `0x08` | Waddle Dee | yes |
| `0x0B` | **Broom Hatter** | no (ported) |
| `0x0C` | Pengy | yes |
| `0x0E` | Chilly | no |
| `0x10` | Cappy | no |
| `0x11` | Hot Head | yes |
| `0x12` | Togezo | no |
| `0x14` | Bronto Burt | yes |
| `0x15` | Kabu | no |
| `0x18` | Twizzy | no |
| `0x19` | Shotzo | yes |
| `0x1A` | Sparky (the green hopper of stage 1-1) | yes |
| `0x1D` | Scarfy | yes |
| `0x26`, `0x27`, `0x28` | Poppy Bros. Jr. (alone, on an apple, with a bomb) | no |

The others weren't identified with certainty; `sw2/` style contact sheets (put each index in Sparky's 1-1 record at `0x0835DD99`) show them all.

## Broom Hatter (type `0x0B`)

- Spawn `0x0808D4E8`, main `0x0808D58C`; variant spawns `0x08743188`; init scripts by state `0x08743194`, per-frame by state `0x087431A0`.
- Scripts: `0x0808D640` sweep (2 loops), `0x0808D790` back and forth (3 loops), `0x0808D964` stand; next action `0x0808D3E4` (table `0x0874313C` = `0,1,2,0,1,2,0,1`, never the last one); ledge/wall stop `0x0808D364`.
- Velocity tables `0x08743144` (1.0, 0.5, 0.25, 0.25, 2.0) and `0x08743158` (1.0, 0.5, 0.5, -1.0, -0.5, -0.5).
- Frames `0x087521D8`: 0-2 dizzy, 3 knocked away, 4-7 sweeping, 8-10 broom dust. Palette `0x0824CCF8` (the dust uses palette 15, white), tiles LZ77 `0x0824CD18`.

## KatAM side (USA)

- `gSpriteTables` `0x083B909C` (anims, attrs, oamData, palette, tiles4bpp...); 939 anims. Tile and palette indexes are relative to the table bases, so new data must be aligned to them.
- Per-type tables: spawn functions `gSpawnFuncTable1` `0x0835107C`, parameters `gUnk_08351648` (0x18 bytes: hurt anim indexes, score, HP, ability, palette key, VRAM tiles, start function, AnimInfo), `gUnk_08352AD0` (8 bytes).
- Free object types: `0x2C` is the randomizer's "Nothing"; `0x31` is free (used by the Broom Hatter). `0x40-0x44` count as bosses.
- HP bar names: `gUnk_08D60EE4[language]` + type * 0x100, 64x8 images; language 1 is English.
- Collision attributes: bit 0 marks an *empty* tile (Boxin's ledge check).
- Free space from `0xF40000` (the randomizer's 9ROM lists) to the end; the expansion uses `0xFC0000`.
- The KatAM decompilation builds byte-identical to the USA ROM, so `katam.elf` gives every symbol to link new code against (`--just-symbols`); `ld` adds interworking veneers for the long calls.
