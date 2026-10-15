# ROM Expansion

Enemies and copy abilities ported from *Kirby: Nightmare in Dream Land* into *Kirby & the Amazing Mirror*. `KatAMExpansion.cs` applies them. Every version of KatAM works (USA `B8KE`, European `B8KP`, Japanese revision 1 `B8KJ`), with the NiDL ROM of the same region: USA (`A7KE`), European (`A7KP`) or Japanese (`A7KJ`, *Hoshi no Kirby: Yume no Izumi Deluxe*). The NiDL ROMs hold the same data, the Japanese one's frame tables `0x3A8C` further and its descriptors at `0x08742DAC`, the European one's `0x23C7EC` further and at `0x0897B678`. NiDL addresses in these notes and the code are the USA ones.

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
- `abilities.h`, `abilities.c`, `abilities/*.c`: the new copy abilities (see Abilities below); `abilities/*.c` has one file per ability.
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
| Broom Hatter | `0x0B` | All 3 NiDL variants: 0 sweeps forward twice, back and forth, or away from Kirby, at random; 1 the same with its own sweep timing, moving once it has landed; 2 sweeps in place. Frames 4-7 with the broom dust (8-10, NiDL palette 15, drawn with the unused color 6) on top; hurt frame 3. Water defeats it, as in NiDL |
| Cappy | `0x10` | Hops forward (0.25), its sprite turning around on every landing, facing Kirby again every 40 hops; frame 4, hurt frame 3. Variant 1 is the capless Cappy (frames 4 and 6 of its own frame table, alternating on every landing); variant 2 hops in place. Inhaled, variant 0 loses its cap like in NiDL: Kirby swallows just the cap (NiDL frame 1) and a capless Cappy springs out backwards and hops on (watched from the object's pre-callback, `unk7C`, which the engine keeps calling while it's inhaled; the capless one carries its own copy of the template). Water defeats it, as in NiDL |
| Twizzy | `0x18` | All 10 NiDL variants. Flying (0-4, 8): 0 swoops forward towards Kirby's height, then climbs away; 1 swoops forever, keeping to Kirby's height; 2 ambushes from the top of the room and loops; 3 flies diagonally bouncing off walls, floors and ceilings; 4 chases Kirby for a while, then flies off; 8 bobs in place. On the ground (5-7, 9): 5 takes off when Kirby comes near and dashes at it; 6 hops up and flutters down; 7 hops around and takes off to chase Kirby when it's near; 9 just sits. Water doesn't affect it, as in NiDL. Frames 4-5 (flapping), 6 (sitting); hurt frame 3 |
| Bubbles | `0x1E` | Variant 0 bounces forward (1.0) at random heights, spinning, squashing on floors, walls (turning around) and ceilings; variant 1 bounces in place. Water defeats variant 0 but not variant 1, as in NiDL. Gives Ball. Frames 4-21; hurt frame 0 |
| Slippy | `0x22` | On land it sits facing Kirby, looks around, and jumps low or high, in place, forward or turning around, chosen at random like NiDL; jumping into water it sinks and swims. Put in water it swims: drifts, paddles and kicks towards Kirby (straight, the other way or diagonally, picked at random), and leaps out at the surface. Variant 1 just sits facing Kirby. Variant 0 joins the underwater pool too. Frames 4, 5, 6, 8, 7 (swimming), 9 (paddling); hurt frame 3 |
| Kabu | `0x15` | All 4 NiDL variants: 0 sits, spins 1-3 times, then jumps at Kirby (5 jumps picked by weight, never the same twice); 1 rocks until Kirby is near, then vanishes in a flicker, reappears next to Kirby, drops and slides at it; 2 glides back and forth to points in front of and behind Kirby, then hops; 3 spins in place. Frames 4-12; hurt frame 3 |
| Coconut | `0x17` | Variants 0 and 1 hang (facing right or left) until Kirby passes under them, then fall tumbling and explode on landing (KatAM's small explosion, like Foley's); variant 2 rocks on the ground. Frames 4-15; hurt frame 3 |
| Coner | `0x29` | Faces Kirby and scuttles in bursts (1.0, 0.5, 0.25, stopped); variant 1 scuttles in place. In water it glides down (0.25 forward and down) and scuttles along the bottom, and joins the underwater pool. Frames 4-6; hurt frame 3 |
| Starman | `0x1F` | All 4 NiDL variants: 0 walks up to Kirby, flies off the top of the screen steering towards it, follows Kirby hidden up there, dives onto it and bounces, and flies away after the second dive; 1 walks and jumps now and then; 2 flies forward bobbing, then rises away (flying pool); 3 walks in place. Frames 4-20; hurt frame 3. Water defeats it, as in NiDL. Gives Hi-Jump |
| Needlous | `0x2C` | Walks towards Kirby, and when Kirby stays level with it (or after a while) hops, curls into a spiky ball, bounces and rolls fast for a second, rolling back from walls; one time in 4 it starts walking, else curled. Variant 1 walks in place. Frames 4-16 (15 and 16 are NiDL's white spin blur); hurt frame 3. Water defeats it, as in NiDL. Gives Needle |
| Blade Knight | `0x25` | All 3 NiDL variants: 0 walks up to Kirby, raises or lowers its sword and slashes, lunging (sometimes slashing back once), then backs off; 1 walks in place; 2 stands guard facing left and slashes when Kirby comes near. Variant 0 also slashes at Kirby's attacks and thrown things that come near (anything whose parent is a Kirby, found walking the task list). The slash's hitbox and sound are KatAM's Sword Knight's. Gives Sword. Frames 4-16; hurt frame 3. Water defeats it, as in NiDL |
| Chilly | `0x0E` | Both NiDL variants: 0 wiggles turning around until it faces Kirby, lunges at it 4 times (3.0 slowing down), then crouches shaking and freezes: a rotating ring of KatAM's Pengy's ice bursts out of it for 240 frames (NiDL's icy ring uses shared effect graphics; Pengy's ice hurts and sounds the same way). Starts with any of these at random. 1 lunges in place. Gives Freeze. Water defeats variant 0, not 1, as in NiDL. Frames 4-11; hurt frame 3 |
| Cool Spook | `0x13` | Both NiDL variants (flying pool): 0 flies left at 0.5 bobbing, through walls and water (NiDL's skips every terrain check); 1 bobs in place facing Kirby. Gives Light. Frames 4-7; hurt frame 3 |
| Poppy Bros. Jr. | `0x26`-`0x28` | NiDL's three types as one enemy (they share graphics): 0 hops (0.5 forward, low hops; every other hop it turns around one time in 3, for one hop); 1 jumps in after 30 frames, then hops; 2 hops in place; 3 and 4 roll a ball or a bomb (0.5), spinning, with Poppy Bros. Jr. hopping on top of it (an effect object drawing the enemy's own frames); 5 and 6 the same in place. Inhaled, the ball or bomb comes off and bounces away from Kirby twice (the bomb then explodes, KatAM's small explosion) and Kirby swallows Poppy Bros. Jr.; defeated, Poppy Bros. Jr. jumps off (variant 1). Water defeats them, as in NiDL. Frames 4-7, the ball's 9-16 and the bomb's 9-16 (an apple and a bomb, drawn with NiDL's shared object palette 15, read from the NiDL ROM: a second palette slot, key anim | 0x2000, while the rider keeps Poppy Bros. Jr.'s); hurt frame 3. Not ported: walking instead of hopping in the rooms NiDL marks for it |
| Gip | `0x2F` | Both NiDL variants: 0 walks towards Kirby (turning its back to it when it's close) and climbs walls it walks into; on a wall it clings, then flaps up, glides down, drops off or shakes and jumps at Kirby (weighted by where Kirby is), steps onto the top or drops off the bottom when the wall ends, slides down the side of ledges it walks off, and every 120 frames of walking throws a spinning star (a KatAM attack object drawing the enemy's own frames). Starting next to a wall it clings to it. 1 walks in place. Water defeats it, as in NiDL. Frames 4-16 and the star's 4-7; hurt frame 3. Not ported: the throw's sound |

## Abilities

NiDL's Needle, Freeze, Light, Ball and Hi-Jump are KatAM abilities 27-31 (`NIDL_ABILITY_*` in `abilities.h`). KatAM keeps Kirby's ability in 5 bits, so these five are all the room there is. Needlous, Chilly, Cool Spook, Bubbles and Starman give them.

| Ability | B button |
|---|---|
| Needle | Kirby sticks his needles out (10 frames) and keeps them out while B is held, then pulls them in. In the air he falls with needles all around, and keeps them out on landing if B is still held. The needles hurt what they touch (a cutting hit) and Kirby is unhurt while they're out |
| Freeze | Kirby wraps himself in ice crystals for as long as B is held (16 frames at least). What touches the ice freezes, with KatAM's Ice's freezing hit, and Kirby is unhurt meanwhile |
| Light | Kirby hovers and holds up a light (28 frames), which shines for 65 frames; then the ability is used up. NiDL's Light only lights up dark rooms. KatAM has none, so the light flashes the screen white (when the room isn't already blending) and hurts every enemy on the screen, like a weak Crash |
| Ball | Kirby hops (up to 20 pixels) and curls into a ball, which bounces once and then rests. B again launches it (up to 37 pixels), and it ends on landing. It can be steered in the air and hurts what it touches, Kirby unhurt meanwhile |
| Hi-Jump | Kirby crouches for 4 frames, then rockets up 4.16 pixels a frame for 25 frames or until he hits a ceiling, steerable sideways, hurting what he touches and unhurt; then he falls |

The timings and speeds were measured in NiDL. In water, every ability's B button does what KatAM's other abilities do (the base ability's entry). CPU helpers fight like Normal Kirby.

How they fit in:

- **Ability tables.** Every table KatAM indexes by ability with 27 entries (B on land and in water, setup, body palette, hats, the CPU helpers' AI) is copied with 32 entries, and the game's code is pointed to the copies. The new entries come from `gNidlAbilityTables`, or from Normal's.
- **Swallowing.** `sub_080547C4` took abilities `0x1B` and `0x1C` to mean "random" and "none". They're now `0x3B` and `0x3C`, and the one object type whose ability was `0x1B` (`0xCE`) is changed to match.
- **HUD and pause portraits** are NiDL's ability cards. `sub_080356AC` and `sub_08088640` are detoured to `NidlAbilityPortrait` and `NidlAbilityHudTimer`, and the special portraits (`0x1C`-`0x1F`) move to `0x2C`-`0x2F`.
- **Hats** (`KatAMExpansionHats.cs`). KatAM draws Kirby as a body plus a hat sprite, animating in step. The new abilities use Normal's body table (`gUnk_0834C5D4`), and each one's hat anim has a variant per entry of it (185): the body's commands, with NiDL's hat for the same pose.
  - `BodyMatches` pairs each KatAM body frame with one of NiDL's plain Kirby poses, by silhouette, checked by eye. Mirrored body frames (attributes `| 0x4000` / `| 0x8000`) are matched unmirrored, and their hat is mirrored the same way.
  - `HatFrames` pairs each plain pose with the same pose in the ability's block. Its body pixels match exactly, or match outside the hat where the hat hides part of it.
  - Poses with no match (inhaling, a mouthful, Kirby facing the camera with something on his head) get no hat.
- **Attack poses** (`KatAMExpansionMotions.cs`) are NiDL's frames, played at animation indexes `0x4A`-`0x59` (copy abilities' own), which `NidlAbilitySetup` points to them.
  - Kirby's own pixels go to the body sprite, which has the player's palette and 16 tiles of VRAM.
  - The rest (ice, needles, the cape) goes to the hat sprite, which has 48 tiles of VRAM.
  - KatAM loads the hat sprite's palette from the ability's hat, so the poses use the colors that palette has free. Needle's hat leaves few, so its needles are a duller yellow than NiDL's.
- **Attacks** (`NidlAttackCreate`) are Burning's attack box: an object following Kirby while he's in the ability's state, with the hit's kind (`unk68`, as in `gUnk_082DDFC8`) and strength.
