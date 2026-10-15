# ROM Expansion

Enemies ported from *Kirby: Nightmare in Dream Land* into *Kirby & the Amazing Mirror*. `KatAMExpansion.cs` applies them. Every version of KatAM works (USA `B8KE`, European `B8KP`, Japanese revision 1 `B8KJ`), with the NiDL ROM of the same region: USA (`A7KE`), European (`A7KP`) or Japanese (`A7KJ`, *Hoshi no Kirby: Yume no Izumi Deluxe*). The NiDL ROMs hold the same data, the Japanese one's frame tables `0x3A8C` further and its descriptors at `0x08742DAC`, the European one's `0x23C7EC` further and at `0x0897B678`. NiDL addresses in these notes and the code are the USA ones.

European games show the enemies' English names in every language. Japanese games show none: their HP bar names are stored differently (not as the USA's per-type images), so the bar keeps type `0x31`'s own name.

## KatAM versions

The code is written against the KatAM decompilation, which builds the USA version. `build.sh` links it once per version: the USA build against `katam.elf`, the others against `versions/<version>.txt`, the same symbols' addresses in that version, found in its ROM by `find_symbols.py <katam decomp> <ROM> <symbols...>` (functions by their code with call targets and pointers masked, variables through the pointers in matched functions, and the rest by the shift of the nearest symbol found). Those tables also hold what `KatAMExpansion.cs` patches in each version (sprite tables, spawn and parameter tables, the hit flash branch, the HP bar name copies, the number of anims: 938 in the Japanese ROM, 939 in the others). When the code starts using a KatAM symbol the tables don't have, `build.sh` says so: rerun `find_symbols.py` with the full list for each version's ROM.

## How they fit in

KatAM treats object types `0x00`-`0x37` as enemies all over its code, and the only free one is `0x31`. So every ported enemy is type `0x31`, and the template's behavior byte picks which: high nibble = enemy (`NIDL_*` in `nidl.h`), low nibble = its variant. What the game reads per type is shared, so `nidl.c` handles it per enemy:

| Shared per type | How each enemy gets its own |
|---|---|
| Animations (`AnimInfo` table, indexed by the object's frame) | One table holding every enemy's frames; each enemy has its own range (`animBase`) and anim ID |
| Palette key | The type has none; each enemy loads its palette with its anim ID as the key |
| Hurt frames (indexes 0 and 1) | A generic 32x32 frame drawn from the last 16 tiles of the object's VRAM, where each enemy uploads its own hurt frame |
| HP bar name | The game's two name copies (`0x08034ED2`, `0x08035B34`) are patched to call `NidlCopyName` |
| Behavior start after a hit | `NidlStart` calls the enemy's own start |
| Gravity | The engine pulls every enemy down unless flag `0x40` is set (KatAM's flying enemies set it); `NidlStart` sets it, and each enemy moves up and down like in NiDL (`NidlFall` is the engine's gravity, for the ones that just stand or walk) |

The hit flash also looked the palette up again by type when it ended; its branch at `0x080857EE` is patched to keep the enemy's palette (which is what it found for the game's own enemies anyway).

## Files

- `nidl.h`, `nidl.c`: the shared part (creation, palette, hurt frame, names, NiDL's frame sequences, helpers like facing Kirby and stopping at ledges).
- `nidl_data.c`: `gNidlData`, left blank and filled in by `KatAMExpansion.cs` (names, and each enemy's `animBase`, anim ID and hurt frame).
- `enemies/*.c`: one file per enemy, written against the [KatAM decompilation](https://github.com/jiangzhengwenjz/katam) headers. NiDL runs enemies as coroutine scripts; each script is copied as tables of steps.
- `build.sh /path/to/katam`: compiles everything with the decompilation's `agbcc`, links it at `0x09000000` for each KatAM version and writes `../KatAMExpansionCode.cs` (each version's code, the symbols `KatAMExpansion.cs` uses and what it patches).
- `find_symbols.py`, `versions/*.txt`: the KatAM symbols' addresses in each version (see above).

## What `KatAMExpansion.AddEnemies()` writes into the KatAM ROM

The ROM is first padded from 16 MB to 32 MB with `0xFF` (the most the GBA maps: `0x08000000`-`0x09FFFFFF`, which emulators and flash carts load like any other ROM; the game saves to SRAM, so nothing else depends on its size). Everything new goes in that second half, so the expansion has 16 MB for its enemies and leaves the game's own free space to the randomizer.


| Where | What |
|---|---|
| `0x1000000` | Code, then the converted graphics, animations, names and the extended sprite tables |
| `gSpriteTables` (`0x3B909C`) | `anims`, `attrs` and `oamData` copied with more entries: anim `939` is the hurt frame, `940` on are the enemies (one variant per frame) |
| `gSpawnFuncTable1[0x31]` | `CreateNidlEnemy` |
| Object parameters (`0x351648 + 0x31 * 0x18`) | Hurt frames 0 and 1, Waddle Dee's score and HP, no ability, no palette key, VRAM for the biggest frame plus the hurt frame, `NidlStart`, the shared animations |
| `0x08034ED2`, `0x08035B34`, `0x080857EE` | The patches above |

Graphics come from the player's NiDL ROM: each enemy's descriptor (`0x0873EEA0 + (type - 8) * 4`: palette, LZ77 tiles) and frame table (8-byte OAM records). Nothing from NiDL is shipped.

## Enemies

| Enemy | NiDL type | Notes |
|---|---|---|
| Broom Hatter | `0x0B` | Frames 4-7 with the broom dust (8-10, NiDL palette 15, drawn with the unused color 6) on top; hurt frame 3. Variant 1 sweeps in place |
| Cappy | `0x10` | Hops forward (0.25), its sprite turning around on every landing, facing Kirby again every 40 hops; frame 4, hurt frame 3. Variant 1 is the capless Cappy (frames 4 and 6 of its own frame table, alternating on every landing); variant 2 hops in place. Not ported: NiDL's cap coming off when inhaled |
| Twizzy | `0x18` | Variant 0 flies forward (0.75) swooping towards Kirby's height every 40 frames, then climbs away after 4 swoops; variant 5 sits until Kirby comes near, takes off, hovers and dashes at it. Frames 4-5 (flapping), 6 (sitting); hurt frame 3. NiDL's other 8 variants aren't ported |
| Bubbles | `0x1E` | Variant 0 bounces forward (1.0) at random heights, spinning, squashing on floors, walls (turning around) and ceilings; variant 1 bounces in place. Frames 4-21; hurt frame 0 |
| Slippy | `0x22` | Sits facing Kirby, looks around, and jumps low or high, in place, forward or turning around, chosen at random like NiDL. Frames 4, 5, 6, 8; hurt frame 3. NiDL's swimming states aren't ported |
| Kabu | `0x15` | Sits, spins in place 1-3 times, then faces Kirby and jumps (5 jumps picked by weight, never the same twice), spinning. Frames 4-12; hurt frame 3. NiDL's variants 1-3 aren't ported |
| Coconut | `0x17` | Variants 0 and 1 hang (facing right or left) until Kirby passes under them, then fall tumbling and explode on landing (KatAM's small explosion, like Foley's); variant 2 rocks on the ground. Frames 4-15; hurt frame 3 |
| Coner | `0x29` | Faces Kirby and scuttles in bursts (1.0, 0.5, 0.25, stopped); variant 1 scuttles in place. Frames 4-6; hurt frame 3. NiDL's water states aren't ported |
| Starman | `0x1F` | Walks up to Kirby, crouches and flies off the top of the screen steering towards it, follows Kirby hidden up there, then dives onto it and bounces; the second time it flies away. Frames 4-20; hurt frame 3. NiDL's variants 1-3 and its water state aren't ported |
| Needlous | `0x2C` | Walks towards Kirby, and when Kirby stays level with it (or after a while) hops, curls into a spiky ball, bounces and rolls fast for a second, rolling back from walls; one time in 4 it starts walking, else curled. Variant 1 walks in place. Frames 4-16 (15 and 16 are NiDL's white spin blur); hurt frame 3. Its water state isn't ported |
| Blade Knight | `0x25` | Walks up to Kirby, raises or lowers its sword and slashes, lunging (sometimes slashing back once), then backs off. The slash's hitbox and sound are KatAM's Sword Knight's. Gives Sword. Frames 4-16; hurt frame 3. NiDL's swing at things thrown at it, its variants 1-2 and its water state aren't ported |
