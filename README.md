# KatAM Randomizer
 
Still in development, thank you for your patience.
This randomizer supports the Japanese (`AGB-B8KJ`), American (`AGB-B8KE`) and European (`AGB-B8KP`) versions of Kirby and the Amazing Mirror. The region is detected when the ROM is loaded.

- **Save Settings / Load Settings** store every option and the seed in a `.json` file, to reuse or share a configuration.
- With **Spoiler Log** ticked, randomizing also writes `KatAM - Spoiler Log.txt` next to the randomized ROM: the settings, every mirror and where it leads now, where the Boss & Goal game rooms are reached from, and the enemies, mini-bosses, chests, items, ability pedestals and map elements that changed.
- **ROM Expansion** (USA only) adds enemies from *Kirby: Nightmare in Dream Land* to the enemy pool. Load your own Nightmare in Dream Land (USA, `AGB-A7KE`) ROM in the ROM Expansion tab: the graphics are taken from it, nothing from that game ships with the randomizer. The new enemies appear when enemies are randomized.
  - **Broom Hatter**: its behavior was re-implemented from Nightmare in Dream Land's code (sweep timings, speeds, frames, broom dust, random choice between sweeping and sweeping back and forth, stopping at walls and ledges). It uses the unused object type `0x31` and has its own name on the HP bar. The source is in `ROM Expansion/`.
