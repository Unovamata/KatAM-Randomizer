# KatAM Randomizer
 
Still in development, thank you for your patience.
This randomizer supports the Japanese (`AGB-B8KJ`), American (`AGB-B8KE`) and European (`AGB-B8KP`) versions of Kirby and the Amazing Mirror. The region is detected when the ROM is loaded.

- **Save Settings / Load Settings** store every option and the seed in a `.json` file, to reuse or share a configuration.
- With **Spoiler Log** ticked, randomizing also writes `KatAM - Spoiler Log.txt` next to the randomized ROM: the settings, every mirror and where it leads now, where the Boss & Goal game rooms are reached from, and the enemies, mini-bosses, chests, items, ability pedestals and map elements that changed.
- **Pause screen**: every ability shows the base pause help screen ("About Copy Abilities"; English in every language of the European version), still with the current ability's picture. The other screens' graphics (about 480 KB) are wiped to free room for gameplay additions.
- **ROM Expansion** (USA only) adds enemies from *Kirby: Nightmare in Dream Land* to the enemy pool. Load your own Nightmare in Dream Land (USA, `AGB-A7KE`) ROM in the ROM Expansion tab: the graphics are taken from it, nothing from that game ships with the randomizer. The new enemies appear when enemies are randomized.
  - **Mini-bosses**: Poppy Bros. Sr., Mr. Tick-Tock, Grand Wheelie and Bugzzy, ported from Nightmare in Dream Land, joins the mini-boss pool with "Include mini-bosses" (it appears when mini-bosses are randomized), with the HP bar, music and camera of KatAM's own mini-bosses. See `ROM Expansion/README.md`.
  - **New copy abilities**: Needle, Freeze, Light, Ball, Hi-Jump, Mike and Backdrop, from the Nightmare in Dream Land enemies and mini-bosses that give them (Needlous, Chilly, Cool Spook, Bubbles, Starman, Mr. Tick-Tock, Bugzzy), with Nightmare in Dream Land's poses, hats and ability cards. See `ROM Expansion/README.md`.
  - **Broom Hatter**: its behavior was re-implemented from Nightmare in Dream Land's code (sweep timings, speeds, frames, broom dust, random choice between sweeping and sweeping back and forth, stopping at walls and ledges). It uses the unused object type `0x31` and has its own name on the HP bar. The source is in `ROM Expansion/`.
