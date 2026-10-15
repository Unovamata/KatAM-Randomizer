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

- Variants `0x08743188`: 0 `0x0808D558` (states `0x08743194`: Sweep `0x0808D640`, Back & Forth `0x0808D790`, Stand `0x0808D964`; per frame `0x087431A0`); 1 `0x0808DA00` (states `0x087431AC`: Sweep `0x0808DAE8` with waits 7, 3, 4, 6, 6, 2, 4, 4, 2 over 9 steps, Back & Forth `0x0808DC94` and Stand `0x0808DEBC` as variant 0's; velocities only once `obj+0x30` is set by the landing handler `0x0808D304`); 2 `0x0808DF58` sweeps in place (`0x0808DFC4`, every step 8 frames). Dust: `0x0808D4D0` none, `0x0808D4A8` frame 8, `0x0808D494` 9, `0x0808D4BC` 10. Handlers `0x087434C4`: leaving the floor (`0x0808D2B8`) stands, landing restarts the scripts.
## Coconut (type `0x17`)

- Spawn `0x0808E8D4`; variants 0 and 1 (`0x0808E914`, facing right or left) hang on frame 5 until Kirby is within 7 pixels sideways (`0x0808E9D4`), then fall (`0x0808EA00`): gravity and max fall by the record flags byte (`0x08743214` 0x1500/0x3000, `0x0874321C` 3.0/4.0), tumbling through frames 7-15 every 2 frames while turning around 4 times. Landing (`0x0808EB10`) it explodes (`0x0806A344`). Variant 2 (`0x0808EB24`) rocks on frames 4, 5, 6, 5 (6 frames each).
- Frames `0x087524A4`: 0-3 hurt, 4-6 rocking, 7-15 tumbling.

## Coner (type `0x29`)

- Spawn `0x0808FA10`, variants `0x087432F4`. Variant 0 (`0x0808FA50`) faces Kirby and scuttles (`0x0808FAB4`): speed 1.0 (frame 5), 0.5 (frame 6), 0.25 (frame 5), 0 (frame 4), for 8, 8, 16 and 16 frames (`0x087432EC`, 4 bytes per record flag; the second set is 4, 4, 6, 6). Variant 1 (`0x0808FBAC`) does it in place (`0x0808FC00`). States 1 and 2 sink and swim in water: its handlers (`0x08743518`: landed, water, in water, ..., wall) run from `0x080692FC`; entering water (`0x0808F978`) picks 2 when submerged (`obj+0x7B == 3`) else 1. 1 (`0x0808FB50`): 0.25 forward, gravity 0x1000, frame 6; 2 (`0x0808FB80`): 0.25 forward and 0.25 down, frame 6; landing (`0x0808F9B8`) scuttles again. Variant 1's per frame skips `0x080692FC`: it just sinks.
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

## Chilly (type `0x0E`)

- Spawn `0x08084CD4`, variants `0x08741FB8`; variant 0 (`0x08084D14`) states `0x08741FC0`, per frame `0x08741FD4`, handlers `0x08742D0C`. Start: state 0 (rand(4) 0 or 1), 2 or 3.
- 0 (`0x08084DC0`): wiggle (4, 5, 6, 7, 6, 5, 4 for 1, 2, 2, 8, 2, 2, 1) and turn around, 4 times, then turn once more; 1 (`0x08084E9C`): turn and wiggle until it faces Kirby (`0x08063DF4`) with Kirby within 32 pixels vertically; 2 (`0x08084F78`): 4 lunges (frame 4 for 4, 5 for 2, then 3.0, 2.0, 1.0, 0.5 every 2 frames on frames 5, 6, 6, 6, 0.5 on frame 7 for 18, stop `0x08006214`, 7, 6, 5, 4 for 3, 6, 5, 4; record flag set: 3 lunges from 4.0), then 3 (`0x08085180`): frame 10 shaking (velocity -1.0, 1.0, -1.0 for 1, 2, 1 frames) 15 times, spawns the freeze (`0x08064B28` with (category 2, type 0x68)), frame 11 shaking 64 times, state 0. 4 (`0x0808529C`) falls (gravity 0x2500).
- Freeze, type 0x68 (`0x0808CB70`): frame table `0x087522B4` (shared effect tiles, no descriptor), frames 0, none, 1, none every frame for 240 frames, sound 0xB9 every 5, sparkles (type 0xD7) every 20; ends when Chilly stops.
- Variant 1 (`0x080852CC`): lunges in place (4, 5, 6, 7, 6, 5, 4 for 4, 4, 8, 21, 6, 5, 4). Frames `0x08752284`: 0-3 hurt, 4-7 wiggling/lunging, 10 crouched, 11 arms up.

## Cool Spook (type `0x13`)

- Spawn `0x0807EDB4`: faces left, frames `0x087412EC` (4, 5, 6, 7, 12 frames each), `0x08065848(0, 0)` (a shared helper object, type 0xAE), variants `0x08741300`. Variant 0 (`0x0807EE14`, state `0x0807EE84`): 0.5 left, bobbing (`0x0807EF7C`: Y speeds -1.0, -0.5, 0, 0.5, 1.0, 0.5, 0, -0.5, 8 frames each); variant 1 (`0x0807EEC4`, `0x0807EF48`): faces Kirby, bobs in place. Its per frame (`0x0807EE60`) doesn't call `0x080692FC`. Frames `0x087523EC`: 0-3 hurt, 4-7 flapping.

## Poppy Bros. Jr. (types `0x26`, `0x27`, `0x28`)

- One tile sheet and palette (descriptors all `0x08257488`). Frame tables: `0x087528D0` (0x26: 0-3 hurt, 4-7 Poppy Bros. Jr.), `0x087528F0` (0x27: its ball at 0 and 9-16), `0x08752934` (0x28: the bomb at 0 and 9-16), `0x08752978` and `0x087529A8` (the ball and bomb alone: 4-11 are the same as the others' 9-16).
- 0x26 (`0x080819B4`, variants `0x087415B8`: 0 and 1 `0x080819F4`, 2 `0x08081D24`): states `0x087415C4` (hop `0x08081AAC`, walk `0x08081BC4`, jump in `0x08081C78`), per frame `0x087415D0`. Hop: frame 7 for 4, 0.5 forward (`0x087415AC`), 0.8 up, gravity 0x1500 (max 3.0) after 6 frames (`obj+0x24`), frames 5, 4, 5, 6. Landing, `0x08081E64`: with `0x03005554` 1 or 3 facing right (2 or 4 facing left) it walks (state 1), else it counts hops (`obj+0x28`) and every other one turns around one time in 3 (`obj+0x2C`: back after one hop). Variant 1 is variant 0 starting with 30 frames without physics (`obj+0x30`) and the jump in (2.0 up). Handlers: walls `0x08081F38` turn (and reset the counts), water `0x08081F50` defeats.
- Palette: 0x27 and 0x28 set `obj+0x40` to OAM palette 15 (NiDL's shared object palette, which levels load: USA `0x080D07A8`, Europe `0x080D3228`, Japan `0x080CFF60`) and keep Poppy Bros. Jr.'s in `obj+0x2C` for the rider (restored when inhaled).
- 0x27 and 0x28 (`0x08081F60`, `0x08082008`; `obj+0x28` 0 for the ball, 1 for the bomb), variants `0x08741604`: 0 `0x080820B8` (0.5 forward, `0x08082270`), 1 `0x080822B0` (alone), 2 `0x0808248C` (in place). The rider (`0x08082554`, `0x080825EC`): the object shows the ball (frames 9-16, every `0x087415F0`[flag] = 8 frames), a second sprite (`[obj+0x8C]`: +0x18 y, +0x1A frame) Poppy Bros. Jr. 16 pixels up, hopping (`obj+0x34` offset, `obj+0x18` speed: 1.5 up, gravity 0x1800; frames 7 for 4, then 6, 5, 4, 5 for 2, 2, 16, 6, then 6). Per frame `0x08082108`: inhaled (`obj+0x7C` 3 or 4) spawns `0x08064FC4(0, 31 or 32, 1)` (the ball or bomb alone) and shows frame 1 of `0x087528D0`; defeated (1) spawns `0x08064FC4(0, 30, 1)` 14 pixels up. Alone (`0x08082338`): faces away from Kirby, 0.5 away (`0x0806421C`), bounces 2.0 then 1.0 up (`0x087415F4`, gravity `0x087415FC`), spinning every 4 (`0x08082458`), then stops; the bomb explodes (category 6, 2) and is gone.

## Gip (type `0x2F`)

- Spawn `0x0808BDB4`, variants `0x08742940` (0 `0x0808BDF4`, 1 `0x0808C6AC`: walks in place). Variant 0's 12 states `0x08742948` (0-2 walk `0x0808BEA0`, 3 cling `0x0808BFC4`, 4 flap up `0x0808C02C`, 5 glide down `0x0808C260`, 6 climb `0x0808C0FC`, 7 slide `0x0808C32C`, 8 top `0x0808C478`, 9 drop `0x0808C4E4`, 10 jump `0x0808C53C`, 11 throw `0x0808C610`), per frame by group (`obj+0x15`) `0x08742978`. Handlers `0x08742DEC`: landed walk (`0x0808C71C`), left the floor 7 (`0x0808C74C`), water defeats, walls (`0x0808C79C`) 6 from walking, 3 from 9 and 10, ceiling (`0x0808C7EC`) 10 from 4.
- Wall probes `0x0808CA00` (right) / `0x0808CAB8` (left): the wall's distance within 12 pixels (collision tables `0x087339F0`...), at the record's offset; the start (`0x0808C82C`) snaps onto a wall and clings. Picks (`0x0808C8BC`): `0x08742998`[`0x08064314(1)`] → weights (`0x0808A84C`: first entry ≥ rand(101)) `0x087429DD` {40, 80, 90, 100, 100} for octants 0 and 7, else `0x087429E2` {20, 60, 80, 90, 100}, for states 4, 5, 9, 10, 3.
- Jump (`0x0808C980`): 2.0 forward, Y speed from Kirby's distance (capped at 3.0 up), gravity 0x2000. Throw: frame 14 for 16, sound 0xE9, `0x0808C934` (`0x08064B5C`: type 0x89 at (6, -6)), frames 15, 14, 7 for 12, 8, 8, then 120 frames of walking (`obj+0x28`). The star (type 0x89, `0x0808D218`): frame table `0x08752C18` (Gip's tiles; 4-7 spinning, 4 frames each), 1.5 forward, 3.0 up, gravity 0x1500.
- Frames `0x08752BD4`: 0-3 hurt, 4-7 walking, 8-12 clinging and flapping, 13 and 16 gliding, 14-15 throwing.

## Cappy's cap

- Variant 0's per frame (`0x0807E5BC`): while inhaled (`obj+0x7C == 3`) it calls `0x08064FC4(0, 8, 1, 0, x, y, flags)`, which spawns category 0, index 8 (type `0x10`, Cappy), variant 1 (capless, frame table `0x0875237C`); the inhaled one is drawn with frame 1, the cap. The capless one starts in state 1 (`0x0807E814`): faces Kirby, 0.5 back, 2.0 up, gravity 0x2000, frames 4, 5, 6, 5 (3, 2, 3, 2 frames), and hops on when it lands (`0x0807E884`). Variant 2 keeps its cap.
- KatAM: inhaling sets `base.unkC & 0x400` (`sub_0809C380`, `sub_0809C6D0` in object.c) and replaces `unk78`, but `ObjectMain` still calls `unk7C` every frame.

## Water

`0x080692FC` calls the object's handlers (`[[obj+0x8C]+0x54]`, from the type's record: 0x2C-byte records with the handler table at `+0x28`): `+0` landed, `+4` left the floor, `+8` entered water, `+12` left water (`+16`, `+20` walls). Most enemies' `+8` calls `0x0806A0F0(-2)`, which defeats them, like KatAM's engine does to its own enemies:

| Enemy | Handlers | Entering water |
|---|---|---|
| Broom Hatter | `0x087434C4` | defeated (`0x0808D354`) |
| Cappy | `0x08741B9C` | defeated (`0x0807E9C8`) |
| Twizzy | `0x08742D44` | nothing |
| Bubbles | `0x08742D7C` | defeated, but not variant 1 (`0x0808976C`) |
| Slippy | `0x08742DB4` | swims (its own scripts, see below) |
| Kabu | `0x08741BB8` | defeated (`0x0807FCAC`) |
| Coconut | `0x087434FC` | defeated (`0x0808ECB4`) |
| Coner | `0x08743518` | glides down (`0x0808F9D8`, state 2) |
| Starman | `0x08741BF0` | defeated (`0x080819A4`) |
| Needlous | `0x08740EE0` | defeated (`0x0807BE9C`) |
| Blade Knight | `0x08740EC4` | defeated (`0x0807B048`) |

## Kirby and his abilities

- Kirby's object is at `0x03002790` (`[0x03002490]`); `+0x3C` (u16) is the frame he shows, an index into the frame table `0x0874CFEC` (5610 entries; set through `0x08006338`/`0x0800634C`, drawn by `0x0803D55C`).
- Frame entries: `{ OAM layout | layers, palette, tiles, [palette 2, tiles 2] }`. Bit 0 of the first word marks two layers: the body (palette slot, tiles from the sprite's base) and the ability's hat (the next palette slot, tiles from base + 0x800, i.e. tile 0x40 on). Palettes are a u16 byte count and the colors; tiles are chunks (u16 byte count, data), each one row of 32 tiles further (2D layout), until 0xFFFF; a 0 pointer loads nothing. The OAM layout is the enemies' 8-byte format (`y, attr0 high | 0x10 last, x, attr1 high, flipped x, flipped attr1, tile`).
- Frames are grouped per ability. Every ability's block starts with the normal Kirby's frames in the same order (`0x00`-`0x75`: stand, walk, slide, run, jump, float...) wearing its hat, followed by its own attack frames; the first block (`0x000`-`0x0F5`) is plain Kirby, `0x11E`-`0x19A` Kirby with a mouthful.
- Blocks of the abilities ported to KatAM, found by swallowing their enemies (the game shows the ability's card): Needle `0x8E8` (yellow spiky Kirby), Freeze `0xA10` (blue and white hood; the barrier is `0xA86`-`0xA8E`), Hi-Jump `0xA8F` (red and purple cape), Ball `0xC54` (blue cap; rolled up `0xCD0`-`0xCF1`), Light (no hat: plain Kirby; holding the light `0xDFD`-`0xE06`). Others: Fire `0x19A`, Sword `0x3E8`, Burning `0x4E0`, Hammer `0x708`, Parasol `0x7F0`, Ice `0x985`, Beam `0xBB8`, UFO `0xF88`; ability pick-up poses `0x10E0`-`0x11C8`, Meta Knight `0x11C9`.
- Attack frames (needles, ice, rolling...) are one layer: Kirby and the effect together, 32-color palettes. The OAM entry's palette bits (`attr2 >> 12`, byte 7's high nibble) pick colors 0-15 or 16-31. Freeze's Kirby is drawn with the frosty second half, and its crystals with colors 12-14 of the first.
- Measured attacks (frames, delays):
  - Freeze: `0xA8E` (2), `0xA86`-`0xA8D` (1 each) looping while B is held (16 frames at least), `0xA8E` (2).
  - Light: `0xDFD`/`0xDFE` alternating 4 times, `0xDFF`-`0xE02`, then `0xE03`-`0xE05` looping for about 65 frames. The orb and the rainbow ring are separate objects.
  - Ball: hop `0xCCA` (6), `0xCCB`-`0xCD1` (2 each), up to 20 pixels; fall `0xCDA`-`0xCE0`, then `0xCD3`-`0xCD9`. Lands 2 pixels lower than standing, bounces about 9 pixels, then rests squashing (`0xCE2`-`0xCE6` and back, 3 each). B again: `0xCE9`, `0xCE7`, `0xCD2`-`0xCD6`, `0xC7B`/`0xC7C`, up to 37 pixels, steerable at up to 1.87 pixels a frame; landing ends the ball.
  - Hi-Jump: crouch 4 frames, rise at 4.16 pixels a frame (steerable to 2.0) for 25 frames or to a ceiling; cape frames `0xB2D`-`0xB31`.
  - Mike (`0x66B`-`0x68A`, no hat block): 3 songs, then the ability is gone. 1: `0x66B` (2), `0x66C` (6), `0x66D`, `0x66E`, `0x66F`, `0x66E`, `0x670`-`0x673` (2 each), then `0x674`/`0x675` (2 each) for 28 frames, `0x670` (5). 2: `0x676`-`0x67C` (4 each), `0x67D` (6), `0x67E`/`0x67D` for 28, `0x679` (5). 3: `0x67F`-`0x682` (2), `0x683` (5), a 26-pixel jump (`0x682`, `0x681`, `0x684`-`0x687`, 4 each, `0x688` 5), `0x689`/`0x68A` for 32, `0x683` (5). The screen flashes yellow while he sings.
  - Backdrop (block `0xE09`, red headband drawn with Kirby, colors 12-14): B is a wind-up (`0xE79` 4, `0xE7A` 6), a dash (`0xE7C`/`0xE85`, 2 each, 4 pixels a frame for 16 frames), then, missing, a stumble to a stop (`0xE7D`-`0xE82`, about 32 frames). Laser's block is `0x5E2` (visor), Wheel's `0x68B` (red cap), Throw's about `0xF6E` (blue headband).
- An enemy's ability is byte 12 of its variant data (Starman's `0x08741918`), copied at spawn (`0x080638B8`) to `obj+0` of the object's extra data; inhaling copies it to the mouth's struct (`0x03002170`, ability at `+0x0B`, `0x0806BE14`), and swallowing gives it (`0x08048342` -> `0x08009FCC`, Kirby's ability at `0x02004B50`). Patching that byte is how to test any ability.
- Per-ability code: two 30-entry tables, `0x0873A7BC` and `0x0873A8A8`, ordered as the blocks are (entry 18 is Beam, which sets frames `0xBBD`...).
- To test an ability in an emulator: room 1-1's first object records (`0x0835DD68`, see Levels) hold a Waddle Doo at records 1 and 4 (index 9); changing their index to another enemy's (type - 8) puts that enemy there, and swallowing it gives its ability.

## Mini-bosses (category 1, type = index + `0x31`)

- Room records with category 1 (`0x0873F23C`): index 0 Bonkers (`0x31`), 1 Poppy Bros. Sr. (`0x32`), 2 Grand Wheelie (`0x33`), 3 Mr. Frosty (`0x34`), 4 Mr. Tick-Tock (`0x35`), 5 Bugzzy (`0x36`), 6 Fire Lion (`0x37`), 7 Phan Phan (`0x38`), 8 Meta-Knights (`0x39`, Meta Knight; his knights are type `0x3A`, graphics descriptors 9-12).
- Graphics descriptors at `0x0873F0E4 + index * 4` (the enemies' format: palettes, often 2, at `+8`, LZ77 tiles at `+12`); the tiles use the enemies' 2D mapping. Frame tables: the `0x0875xxxx` literal each spawn stores at `obj+0x38` (`0x31` `0x08752ED8`, `0x32` `0x087530F8`, `0x33` `0x08752F74`, `0x34` `0x08753090`, `0x35` `0x08753180`, `0x36` `0x087535FC`, `0x37` `0x08753718`, `0x38` `0x087537FC`, `0x3A` `0x08753204`). Mini-bosses use object slots 32 and up.
- Spawn functions (master table): `0x31` `0x0809000C`, `0x32` `0x080910C0`, `0x33` `0x080941AC`, `0x35` `0x0809A0A8`, `0x36` `0x08091F08`, `0x37` `0x080957BC`, `0x39` `0x0809BBD8`, `0x3A` `0x0809C404`.
- Poppy Bros. Sr. is three objects: its body (`0x32`), its head (`0xB3`, `0x08091DDC`: frame table `0x08753128`, the body's frames 12 on; it copies the body's position and flip and plays the head animation the body sets in `obj+0x30`, table `0x08743A00` of (frame, frames) pairs ending with -1) and, while it throws, its hands (`0xB4`, `0x08091B6C`: frames 20 on, offsets from the body moving per frame), which create the bomb (category 9 index `0x6F`, `0x08093BD4`: thrown at 1.5 ahead and 0.5 up, gravity 1/8, bouncing once at 2.0, exploding when it lands again). `obj+0x7A` is set when it lands. See `ROM Expansion/minibosses/poppy_bros_sr.c` for its script.
- Mr. Tick-Tock (`0x35`) draws its body with *streamed* frames: frame table entries 0-16 of `0x08753180` point to `{OAM records, palette, tile rows}` (`0x0827970C` on), its tiles uploaded with each frame (rows of 32 VRAM tiles: `u16` bytes and data, ending with `0xFFFF`; the palette `u16 0x20` and 16 colors). Its descriptor (`0x08279E40`) holds only its rings (17-22) and notes (23-28). 24 states (`0x08745750`, per frame `0x087457B0`); decisions `0x08099DB0`/`0x08099DEC` with the tables at `0x087456CC`-`0x08745704`; notes are type `0x77` (`0x08099EE4`), the alarm's rings type `0x76` (`0x08099E9C`). See `ROM Expansion/minibosses/mr_tick_tock.c`.
- Grand Wheelie (`0x33`): every frame streamed (0-55 of `0x08752F74`; 56-66 are its small Wheelies, 67-70 dust). It plays frames through animation tables (`0x080956C8` starts one: (frame, frames) pairs, -3 loops, -2 holds; `0x0874433C`-`0x0874441C`). 11 states at `0x08744444`; roll speeds per variant `0x087444A4`; `0x08094810` counts decisions (every 4th: two hops and a small Wheelie, object category 0 `0x72`). See `ROM Expansion/minibosses/grand_wheelie.c`.
- Bugzzy (`0x36`): frames 0-63 of `0x087535FC` streamed; 13 states at `0x08743AE0` (walk 1, call 2, charge 3, jump 6-9, suplex 10, get up 11); calling, it brings NiDL types `0x14` (Bronto Burt) and `0x11` (Waddle Doo). Traced suplexes: 36 (5), 37 (8), 45, 46, 49, 47 (4 each) rising 29 pixels and 1.5 back, then 48 for 44 frames until it lands; another one leaps higher (51, 52/61, 53, 54, 55-59). See `ROM Expansion/minibosses/bugzzy.c`.
- Fire Lion (`0x37`): frames 0-51 of `0x08753718` streamed, 52-56 its fire bits (descriptor), 57 on the next mini-boss's. It keeps 4 objects of type `0xD6` around (its flames). Traced moves: idle 4-7 (8 each, 90 frames); roar: 3 hops (13, 14, 15), 38 pixels up; fireball: 44, 45, then 46/49/50/51 at 3.0; pounce: 17-20, 36-38 (48 pixels up, 1.9 forward), 40-43 twice; breath: 8-11 at 1.5, a leap (13-15), 26-29 three times; blaze: 34, 30-33 six times; hop back after each: 23, 24, 25 (62 pixels back). See `ROM Expansion/minibosses/fire_lion.c`.
- Meta-Knights: Meta Knight (`0x39`, `0x0809BBD8`, frames `0x08753510`) runs a line-up from `0x08745CFC` (`0x08745D4C` in the Extra game; index variant * 4 + `0x03002350` - 1): `{script data, knight HP share, 4 list pointers}`, each list 8-byte entries `{kind, variant, x, y}` in screen coordinates (kind 0-3 a knight, 4 a wait: variants 0-4 wait 30/60/120/180/240 frames, 5 the warp effect, `0x0809F680`, Meta Knight's frames 38-58 drawn from NiDL's common tiles at tile base 0; 5 ends the list). `0x02007D00[4]` holds each list's state (1 busy, 0 free, -1 done); `0x0809BC1C` refills free ones; all four done ends the fight. Knights (`0x3A`, `0x0809C404`, kind in `obj+0x74`): Axe Knight (`0x08753204`, descriptor 9, states `0x08747AA4`), Javelin Knight (`0x08753290`, 10, `0x08747ADC`), Mace Knight (`0x08753378`, 11, `0x08747BCC`), Trident Knight (`0x08753414`, 12, `0x08747BF4`); their weapons: the axe `0x81` (`0x0809CF60`), the mace `0x82` (`0x0809E284`, twirled `0x0809E670`, thrown `0x0809E320`), the trident `0x83` (`0x0809F37C`, speeds `0x08747C6C`), the javelin `0x84` (`0x0809DA1C`). See `ROM Expansion/minibosses/meta_knights.c`.
- KatAM's holding of Kirby (Phan Phan `0x080CE94C`, Mr. Frosty, Box Boxer...): the grabber sets `kirby->unk110` to a `Kirby_110` table (x ahead, y down, sprite order, frames, Kirby's animation, flags), `kirby->base.unk6C` to itself, and `sub_0804E09C` puts Kirby in the held state (`sub_0804E60C`), which restarts the table whenever the holder's `unk83` changes; flags `0x400` keep it held, `0x10` throws it hurt (`sub_0804EA18`), `2` lets it go (`sub_0804F32C`).

## Spawn functions

The master table (`0x0872FF30`) has 8-byte entries, `(kind, spawn function)`, so type `t`'s spawn is at `0x0872FF34 + t * 8`: Chilly `0x08084CD4`, Cool Spook `0x0807EDB4`, Starman `0x08080DD0`, Sword/Blade Knight `0x0807AF58`/`0x0807AF98`, Poppy Bros. Jr. `0x080819B4`/`0x08081F60`/`0x08082008`, Coner `0x0808FA10`, Needlous `0x0807BD20`, Gip `0x0808BDB4`.

## KatAM side (USA)

- `gSpriteTables` `0x083B909C` (anims, attrs, oamData, palette, tiles4bpp...); 939 anims. Tile and palette indexes are relative to the table bases, so new data must be aligned to them.
- Per-type tables: spawn functions `gSpawnFuncTable1` `0x0835107C`, parameters `gUnk_08351648` (0x18 bytes: hurt anim indexes, score, HP, ability, palette key, VRAM tiles, start function, AnimInfo), `gUnk_08352AD0` (8 bytes).
- Free object types: `0x2C` is the randomizer's "Nothing"; `0x31` is free (used by the Broom Hatter). `0x40-0x44` count as bosses (their HP bar multipliers, `gUnk_08351458`, are a placeholder 8192).
- HP bar names: `gUnk_08D60EE4[language]` + type * 0x100, 64x8 images; language 1 is English.
- Collision attributes: bit 0 marks an *empty* tile (Boxin's ledge check).
- Gravity: `ObjectMain` pulls every object down (`0x0809A990`: 0x15 per frame, at most 3.0, NiDL's 0x1500 and 3.0) unless `flags & 0x40` (and not `0x20`, which it sets once falling). Flying enemies set `0x40` in their start; getting hit clears it.
- Explosions: `0x08073D2C` is the small one Foley and Scarfy explode with (40x40, hurts Kirby); `0x08074420` is Bomber's (128x128).
- Free space from `0xF40000` (the randomizer's 9ROM lists) to the end of the 16 MB ROM. The expansion pads the ROM to 32 MB (the most the GBA maps, `0x08000000`-`0x09FFFFFF`; the game saves to SRAM, so no EEPROM range gets in the way) and puts everything at `0x09000000`.
- The KatAM decompilation builds byte-identical to the USA ROM, so `katam.elf` gives every symbol to link new code against (`--just-symbols`); `ld` adds interworking veneers for the long calls.
