# ROM Expansion

Enemies ported from *Kirby: Nightmare in Dream Land* (USA) into *Kirby & the Amazing Mirror* (USA). `KatAMExpansion.cs` applies them.

## Broom Hatter

- `broom_hatter.c`: the behavior, written against the [KatAM decompilation](https://github.com/jiangzhengwenjz/katam) headers. NiDL runs it as coroutine scripts; each script is copied as a table of steps (speed, body frame, broom dust, frames to wait).
- `build.sh /path/to/katam`: compiles it with the decompilation's `agbcc`, links it at `0x08FC0000` against `katam.elf` and prints the symbol offsets. The resulting `broom_hatter.bin` is what `KatAMExpansion.cs` embeds (`BroomHatterCode`).

What `KatAMExpansion.AddBroomHatter()` writes into the KatAM ROM:

| Where | What |
|---|---|
| `0xFC0000` | Code, then the converted graphics, animations and the extended sprite tables |
| `gSpriteTables` (`0x3B909C`) | `anims`, `attrs` and `oamData` copied with one more entry, anim `939` |
| `gSpawnFuncTable1[0x31]` | `CreateBroomHatter` |
| Object parameters (`0x351648 + 0x31 * 0x18`) | Hurt frames, Waddle Dee's score and HP, no ability, palette key `939`, 10 tiles, start function, animations |
| HP bar names (`gUnk_08D60EE4`) | "Broom Hatter", put together from the letters of other enemy names |

Graphics come from the player's NiDL ROM: frames `0x087521D8` (8-byte OAM records), palette `0x0824CCF8`, LZ77 tiles `0x0824CD18`. The broom dust (NiDL palette 15, white) is drawn with color 6 of the Broom Hatter's palette, which it doesn't use, so body and dust are a single sprite.
