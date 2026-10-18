# Working rules for this repository

These are the owner's standing rules for any AI assistant working here. They hold in every session.

## How to work

- **No subagents, no parallel agents, no workflows.** Do all research, searching and testing yourself, one step at a time. They use
  up the owner's credits fast. This includes research helpers (Explore, general-purpose, forks) and multi-agent workflows.
- **Don't change what wasn't asked for.** If a fix seems to need another design change (the world map, hub links, the layout of
  KatAM's own map, a rule in the rulebook), ask first or leave it alone. Never "improve" an approved design without being asked.
- **Read the rulebook before touching map design**: `Documentation/Map and Door Design Rules.md`. Change a rule there on purpose,
  with the owner's go-ahead, before changing the code that follows it.
- **The open requests are in `Documentation/Open Requests.md`.** Check it before saying something is done. Update it when an item
  is finished (and how it was checked) or when the owner adds or changes one.
- **Test in the emulator before saying something works**, and say how it was tested: a forced state (RAM writes) is not a real
  playthrough. Say plainly what wasn't tested.
- **Build and check all three versions** (USA, European, Japanese rev 1) after ROM changes. Run the logic checks (every room reached,
  every big switch pressed, nowhere Kirby gets trapped, with and without dropping through thin floors).

## Commits

- Develop on the branch the session names. Commit and push when a piece of work is done. No pull request unless the owner asks.
- Commit messages: a title line, a blank line, then one `// Name(); description;` line per change, then the attribution lines.
- No model names or IDs in commits, code or docs.

## Map viewer

- **Unrandomized ROMs (expanded or not): KatAM's map stays exactly KatAM's**, laid out as the in-game pause screen's area maps
  (`MapVanilla`). The NiDL levels are built **around** it, never in its gaps or moving it.
- **Randomized ROMs:** clusters are enough (`MapLayout`), as tight as they go, with no rogue islands.

## Settings window

- **Toggles and options use the base randomizer's Title Case** ("Spoiler Log", "Add More HP UPs", "Include Minibosses"): every
  word capitalized except short joining words ("Skip the Tutorial", "Add Shards in Chests"). Group titles and tabs too.
- Options that change the game but no randomization pool (title screen, flashing, tutorial) go in **Miscellaneous**, not in
  General Options or the ROM Expansion tab.
