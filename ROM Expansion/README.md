# ROM Expansion

Enemies ported from *Kirby: Nightmare in Dream Land* (USA) into *Kirby & the Amazing Mirror* (USA). `KatAMExpansion.cs` applies them.

## How they fit in

KatAM treats object types `0x00`-`0x37` as enemies all over its code, and the only free one is `0x31`. So every ported enemy is type `0x31`, and the template's behavior byte picks which: high nibble = enemy (`NIDL_*` in `nidl.h`), low nibble = its variant. What the game reads per type is shared, so `nidl.c` handles it per enemy:

| Shared per type | How each enemy gets its own |
|---|---|
| Animations (`AnimInfo` table, indexed by the object's frame) | One table holding every enemy's frames; each enemy has its own range (`animBase`) and anim ID |
| Palette key | The type has none; each enemy loads its palette with its anim ID as the key |
| Hurt frames (indexes 0 and 1) | A generic 32x32 frame drawn from the last 16 tiles of the object's VRAM, where each enemy uploads its own hurt frame |
| HP bar name | The game's two name copies (`0x08034ED2`, `0x08035B34`) are patched to call `NidlCopyName` |
| Behavior start after a hit | `NidlStart` calls the enemy's own start |

The hit flash also looked the palette up again by type when it ended; its branch at `0x080857EE` is patched to keep the enemy's palette (which is what it found for the game's own enemies anyway).

## Files

- `nidl.h`, `nidl.c`: the shared part (creation, palette, hurt frame, names, NiDL's frame sequences, helpers like facing Kirby and stopping at ledges).
- `nidl_data.c`: `gNidlData`, left blank and filled in by `KatAMExpansion.cs` (names, and each enemy's `animBase`, anim ID and hurt frame).
- `enemies/*.c`: one file per enemy, written against the [KatAM decompilation](https://github.com/jiangzhengwenjz/katam) headers. NiDL runs enemies as coroutine scripts; each script is copied as tables of steps.
- `build.sh /path/to/katam`: compiles everything with the decompilation's `agbcc`, links it at `0x08F60000` against `katam.elf` and writes `../KatAMExpansionCode.cs` (the code and the symbols `KatAMExpansion.cs` uses).

## What `KatAMExpansion.AddEnemies()` writes into the KatAM ROM

| Where | What |
|---|---|
| `0xF60000` | Code, then the converted graphics, animations, names and the extended sprite tables |
| `gSpriteTables` (`0x3B909C`) | `anims`, `attrs` and `oamData` copied with more entries: anim `939` is the hurt frame, `940` on are the enemies (one variant per frame) |
| `gSpawnFuncTable1[0x31]` | `CreateNidlEnemy` |
| Object parameters (`0x351648 + 0x31 * 0x18`) | Hurt frames 0 and 1, Waddle Dee's score and HP, no ability, no palette key, VRAM for the biggest frame plus the hurt frame, `NidlStart`, the shared animations |
| `0x08034ED2`, `0x08035B34`, `0x080857EE` | The patches above |

Graphics come from the player's NiDL ROM: each enemy's descriptor (`0x0873EEA0 + (type - 8) * 4`: palette, LZ77 tiles) and frame table (8-byte OAM records). Nothing from NiDL is shipped.

## Enemies

| Enemy | NiDL type | Notes |
|---|---|---|
| Broom Hatter | `0x0B` | Frames 4-7 with the broom dust (8-10, NiDL palette 15, drawn with the unused color 6) on top; hurt frame 3. Variant 1 sweeps in place |
| Cappy | `0x10` | Hops forward (0.25), its sprite turning around on every landing, facing Kirby again every 40 hops; frame 4, hurt frame 3. Variant 2 hops in place. Not ported: NiDL's cap coming off when inhaled (a separate capless Cappy) |
| Twizzy | `0x18` | Variant 0 flies forward (0.75) swooping towards Kirby's height every 40 frames, then climbs away after 4 swoops; variant 5 sits until Kirby comes near, takes off, hovers and dashes at it. Frames 4-5 (flapping), 6 (sitting); hurt frame 3. NiDL's other 8 variants aren't ported |
| Bubbles | `0x1E` | Variant 0 bounces forward (1.0) at random heights, spinning, squashing on floors, walls (turning around) and ceilings; variant 1 bounces in place. Frames 4-21; hurt frame 0 |
| Slippy | `0x22` | Sits facing Kirby, looks around, and jumps low or high, in place, forward or turning around, chosen at random like NiDL. Frames 4, 5, 6, 8; hurt frame 3. NiDL's swimming states aren't ported |
