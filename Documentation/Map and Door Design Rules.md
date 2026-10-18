# Map and door design rules

The rulebook for laying out mirrors, doors and the ways between rooms, for the ROM Expansion's Nightmare in Dream Land rooms
(`KatAMExpansionRooms.cs`, `PlanDoors()`) and for anything that adds or moves a way between rooms later. The goal is always the
same: the map should read and play like *Kirby & the Amazing Mirror*, not like a pile of teleporters. When a new idea breaks one of
these rules, change the rule here first, on purpose, then the code.

Each rule says where it is enforced, so it can be checked. The logic checks (every room reached, every big switch pressed, no
place you can't get out of, with and without dropping through thin floors) must pass on the USA, European and Japanese builds
after any change.

## 1. What counts as a way between rooms

| Way | KatAM object | One-way or two-way | Notes |
|---|---|---|---|
| Mirror (door) | `0x6F` mirror, a door cell | either | The normal way. One-way mirrors are drawn with KatAM's one-way graphic. |
| Hub door | `0x97` door object | always two-way | Only in hubs (rule 4). Hidden until its big switch is pressed. |
| Mirra | `0x34` | always one-way | A hidden mirror that runs away (rule 6). |
| Warp star | `0x84` | one-way | NiDL's own; stays a warp star (rule 7). |
| Cannon | `0x8A` + fuse `0x78` | one-way | NiDL's own; stays a cannon (rule 7). |
| Edge | none (the room's edge) | one-way | NiDL's falls out of a room's bottom and climbs out of its top. |
| Boss shard | `0x65` shard | one-way | The only way out of a boss room (rule 8). |

## 2. How many doors a room has

- **At most 3 exits a room, 2 in a small room** (2 screens or less). `MaxExits()`.
- **The camera never shows 3 doors at once**: no door goes within a screen (`ViewWidth` x `ViewHeight` cells) of two others.
  `ViewFits()`. NiDL's rooms lagged and read as a mess with many mirrors in view.
- **Mirrors stay inside what the camera shows**: never on a row the camera can't reach, never in mid-air, always on a floor
  3 cells wide with room above. `RoomSpot()`, `Shown()`.
- **Doors keep their distance** from other doors, from where other doors land Kirby, and from warp stars and cannons
  (`MirrorSpacing` x 2 cells across, 2 up or down; half that in small rooms). `RoomSpot()`.
- NiDL's own door halls (Butter Building's tower floors, Rainbow Resort's rooms of doors) keep all their doors, drawn as NiDL's
  own doorways without mirrors, past these limits.
- **Secret doors stay hidden until uncovered**: a door behind breakable blocks or in a hidden spot only shows once Kirby finds it.
- **Enemies keep away from doors**: no enemy stands on a door or right where a door lands Kirby (3 cells across, 2 up or
  down), counted where the enemy ends up (a walker dropped onto the floor below its spot), in the NiDL rooms and in KatAM's
  rooms the NiDL doors land in (KatAM's enemy there is left out). `BuildObjects()`, `KatAMSpot()`, `FinishRooms()`.

## 3. One-way and two-way

- **Two-way doors are used sparingly**; most added connections are one-way throws, as KatAM's.
- **A two-way door leads back where you came from**: it lands Kirby on (or right beside) the door that brings him back.
- **A one-way door never throws Kirby onto a door, nor in front of a two-way one** (4 cells across, 2 up or down): its landing
  moves to a floor nearby that the camera shows. `Crowded()`, `Clear()`. Where there's none, it lands off the doors only (twice as
  far at most) before anywhere in the room: anywhere left a mini-boss room's way out landing Kirby cut off from the rest (0x29A's).
- **NiDL's own two-way doors stay two-way**, and its one-way mirrors were made two-way, unless the way back lands somewhere
  unreachable.

## 4. Hubs

- **A hub has at most 2 regular doors** (mirrors). Hub doors (`0x97`, the doors a big switch reveals) are counted apart and are
  not limited by this rule.
- **The NiDL switch rooms have 2 hub doors**, in one of KatAM's two hub-door layouts:
  - **A** (`0x122`): doors at the top left and top right, the big switch between them;
  - **B** (`0x250`): one door at the top middle, one at the bottom middle.
- **Every NiDL hub has 2 doors to its level's exploration map** (the owner's rule, for all hubs): each switch room's 2 floor
  mirrors, both two-way, lead into 2 different branches of its level, so the level branches from its hub (rule 5). These are
  its 2 regular doors; its hub doors are counted apart.
- **The hubs stay interconnected the way they were approved**: each level's switch room reveals a hub door leading to
  (`LevelLinks`, `KatAMLinkDoors`):

  | Level | Its hub door leads to |
  |---|---|
  | Vegetable Valley | Rainbow Route East `0x316` |
  | Ice Cream Island | the Moonlight Mansion door `0xBE` |
  | Butter Building | Rainbow Resort's switch room |
  | Grape Garden | Peppermint Palace East `0x122` |
  | Yogurt Yard | Cabbage Cavern West `0x250` |
  | Orange Ocean | Cabbage Cavern East `0xBF` |
  | Rainbow Resort | the NiDL hub |
  | Fountain of Dreams | Cabbage Cavern East `0xBF` |

  Each KatAM room in that list gets a hub door back.
- **The world map is fixed as approved** (`worldmap.c`'s `gNidlWorldMapDoors`, one path per level's hub door). Neither the world
  map nor the hub links change unless the user asks for it.
- **Hub doors are always two-way and never lead to a Mirra.**
- The NiDL hub (a Central Circle copy at night) has one door per level, as Central Circle has one per area. It is reached
  through a golden mirror in Central Circle (beside Central Circle's own), and a pink mirror in the NiDL hub leads back.
- **The NiDL hub has Central Circle's ability room door**, at the same spot (top left, cell 3, 4), drawn and opened as Central
  Circle's (KatAM's own switch, world slot 0x12), leading to the ability room (0x3CA) as Central Circle's does; the way out of the
  ability room is KatAM's, to Central Circle.

## 5. Levels, stages and the order of things

- **A level is a tree of stages** from its first stage. Every other stage is entered through a branch door (in a room's middle or
  hidden, half and half), and its start gets a mirror back.
- **Only the deepest stage's goal leads to the boss stage.** The other goals end their branch, one-way, as KatAM's goal mirrors.
- **Loops are few and each has a reason** (about one per 7 rooms): one-way shortcuts home from deep stages, a two-way door between
  sibling branches, a hidden two-way door between each level and the next, one one-way throw into another level, and links with the
  KatAM area paired with the level (rooms with few doors, far from them).
- **Goal rooms keep their goal door alone**: no branch, no way back.
- **A level's switch room is found by exploring**: a Mirra hidden in the second half of the level leads to it, one-way, landing
  off its floor mirrors (rule 3). Its right floor mirror goes both ways to the start of the stage before the boss; its left one
  both ways to the start of a stage on another branch: the deepest stage off the way from the level's first stage to the boss's,
  not the Mirra's own (else the nearest one on that way). Rainbow Resort's and the Fountain of Dreams' switch rooms, both in
  Rainbow Resort, take different stages. `PlanDoors()`.

## 6. Mirras

- **As few as KatAM's**, at most one per room, never chained.
- **Always one-way, always hidden, never in mid-air.**
- **Each leads to real progress** (a switch room, a shortcut to a later stage), never sideways.
- Never in a boss room or a switch room. `MirraSpots()`.

## 7. Warp stars and cannons

- **They are key routes to other areas and secrets**: they stay KatAM's own objects, where NiDL has them, to where NiDL's take
  Kirby. They are not turned into mirrors. `Transport`.
- **Warp stars float where NiDL puts them** (a cell's middle). **Cannons sit on their fuse** as KatAM's own do (rooms `0x7E`,
  `0x143`, `0x1F6`, `0x33B`): x the right edge of the fuse cell the rope enters, y that cell's top. `TransportSpot()`.
- **They always land Kirby safely**: on a floor, never in a pit or the room's hidden bottom rows. A cannon fires Kirby to the top of
  its room and he must fall onto a floor. `Land()`.
- **A warp star flies NiDL's own ride**, as NiDL's star flies it, frame by frame: it's off as soon as Kirby is on (a jump up, a
  bob down under him, no wait), along its own path out of the room, through the sky rooms NiDL's goes through (each entered where
  NiDL's comes in), and into the room it lands in, where Kirby tumbles off as NiDL's does (the orb, his tumble and bounce) to
  where NiDL's Kirby stands. The helpers on the star ride with him. `NidlRides`, `WarpFlights()`, `warpstar.c`.
- No mirror is placed next to them (rule 2).

## 8. Bosses and mini-bosses

- **A boss room has no door out, and is the end of its branch.** Taking the shard its boss leaves is the only way out. A one-way door
  that was in a boss room goes elsewhere. A boss whose shard is owned leaves a Maxim Tomato
  instead, never the shard, and grabbing it takes Kirby out as the shard would. `bossExitsOf`,
  `gNidlShardExits`, `NidlShardExit()`.
- **A shard only ever sends Kirby to a central hub**: KatAM's to Central Circle, NiDL's to the NiDL hub, onto that level's door
  there. Never into another level or room. The one exception is the Power Orb, whose fight goes on in Nightmare's room, as in NiDL.
  NiDL's empty walk-out rooms after a boss are left out of the port: nothing leads there any more.
- **Which shard a boss leaves can be shuffled** (Bosses Give Random Shards): KatAM's bosses always hold all 8 between them, so the
  game can be beaten from KatAM alone; NiDL's spread the 8 evenly. An owned shard is always a Maxim Tomato, never a cutscene in the way.
- **Doors into boss rooms are one-way.** Nothing else (branches, links, Mirras) goes in a boss stage.
- **A mini-boss's fight area has no door until it is beaten**: doors there show once the fight ends, whenever Kirby comes in (a
  long room's too, before the fight is on screen). The fight area for doors is its screen and the space it's fought in: the cells
  Kirby walks to from it, up to 4 cells across and 2 up or down (not past a wall). Those doors are built shut and open when the
  mini-boss is beaten. No two-way door in a mini-boss's room; a door into one throws Kirby in, one-way, and its way out is one-way
  too, never right beside the way in. `Arenas()`, `InArena()`, `ArenaCells()`, `arena.c`.

## 9. Every place gets you back out

- **Every room is reached** from the NiDL hub's starter mirror: a group of rooms nothing leads into gets a one-way throw into its
  first room. `Reached()`.
- **Every place Kirby can reach has a way back out** (to the hub or KatAM's rooms), **without needing Down + B** through a thin floor.
  A place with none gets a one-way throw out, into the first stage of its level that has a way out. The way-out pass in `PlanDoors()`.
- **Thin floors only become KatAM's undroppable kind** when, from every place Kirby arrives, dropping through leads nowhere he
  doesn't reach anyway. `ThinFloors()`.
- A one-way throw's landing never traps Kirby (rule 3), and breakable blocks a door lands Kirby inside start broken.

## 10. Chests, items and shards

- **Chests go where KatAM would put one**, as a reward for exploring (dead ends, secret rooms, behind blocks, the end of a branch),
  for a challenge, or as a surprise. Never in a boss room, a switch room or a mini-boss's fight area.
- **NiDL chests**: some of NiDL's loose consumables turn into consumable chests, sparingly (a few per level, as KatAM has). The
  ROM Expansion makes them when it ports the rooms, following KatAM's patterns, whether or not the ROM is randomized; the randomizer
  only reads them afterwards, as it reads KatAM's. A chest per 8 rooms of a level (2 to 5), one a room at most, at the best reward
  spots: a loose item behind breakable blocks, a dead end, far from where Kirby comes in; an item there becomes a chest holding it, a
  dead end with none gets a new chest as far into the room as Kirby goes. A dead end with nothing in it at all (no enemy, item or
  boss: 0x298) always gets one, over the level's count: no room is there for nothing. Always on a floor the camera shows, whole: never in a room's first or last column, which the camera shows only half of.
  `PlanChests()`.
- **A chest is listed in its room's doors list**, or opening it hangs the game: the ported rooms' lists are rebuilt after the
  randomizer (`FinishRooms()`), and KatAM's chest lists go into the table the game reads (the expansion moved it).
- **To the randomizer a chest is a consumable spot**: shuffled, a chest can land wherever a consumable was in the original game.
- **Protected, never randomized**: big switches, levers and story progression chests (and the shard chests the randomizer makes).
- **The chest options, top to bottom** (Chests & Items):
  - **No Chests is above every other chest option**: no chests at all in the game (the protected ones aside), so no shards in
    chests either (that option is greyed out). The pause screen's chest lists show only what's left.
  - **Shuffle moves chests around, never two onto one spot** (one would be lost). Unchanged keeps them where they are.
- **Mirror shards the randomizer adds**, two options, used separately or both at once:
  - **Field shards**: Unchanged (none added), Random (1 or 2) or Custom (1 or 2). Each step adds one more copy of all 8 shards, each
    copy as 8 shards lying in rooms, each one replacing a random loose item (never a chest).
  - **Add Shards in Chests**: 1 or 2. Each step adds one more copy of all 8 shards, put into existing chests that held a consumable
    (KatAM's, and NiDL's once they have them). Never a new chest, never a treasure chest, and never so many that the consumable
    chests run out (2 copies take 16 of KatAM's 41).
- **Only a boss's shard takes Kirby out of the level** (to Central Circle, or the NiDL hub). A shard from a chest or lying in a room
  plays its cutscene, then the game goes back to the same room where Kirby was: no victory dance, no warp. `NidlShardDone()`.
- **An owned shard never blocks the way**: a field shard already owned is a Maxim Tomato; a chest whose shard is owned shows open.
- **NiDL's loose items are taken once per visit**, as KatAM's: once eaten they don't come back until the game forgets them.
- **Batteries** go in the NiDL rooms too, at valuable, rewarding places: the farthest floor behind breakable blocks, else the
  farthest floor of a dead end; a battery per 8 rooms of a level (2 to 5), one a room at most, never in a chest's room, a boss room,
  a switch room or a mini-boss's fight area. Taken once per visit, as the items. `PlanChests()`.

## 11. The map viewer

- **Unrandomized ROMs (expanded or not): KatAM's map stays exactly KatAM's**, as the pause screen's area maps draw it
  (`MapVanilla`), and the NiDL levels go **around** it: never in its gaps, never moving it. Each level hugs KatAM's map, the nearest
  free spot first (then the one nearest the rooms it links to, then the squarest map), so the levels surround it on every side; the
  NiDL hub sits just outside it, on the side nearest Central Circle. `MapExpansion.Place(original)`.
- **Clusters come before routes** (the owner's rule): a NiDL level is drawn as one cluster as dense as KatAM's areas on the same map
  (the same map space per room, on average), not spread out so its lines run straight; lines may wind and cross to keep it so.
  Not tighter than KatAM's either. `MapExpansion.Tighten()`.
- **A central hub's lines share one trunk**: the fewer lines out of a hub, the better. Central Circle's and the NiDL hub's
  connections leave it along one line that branches off near each room it leads to. Only those two: other rooms linked to them
  (switch rooms, level hubs, any room) keep a line of their own each. `MapPlan.RouteAll()`.
- **Randomized ROMs**: clusters are enough, and **every cluster looks the same**: each NiDL level is laid out as compact as KatAM's
  clusters there (`MapLayout.LayoutCluster()`, its grid) and packed as close to the others as they are to each other.
  `MapExpansion.ClusterLevel()`.
- **Each level is one cluster**; a stage's rooms stay together, and a room linked only to another level sits by its own stage's
  rooms.
- **The whole map is tight and as square as it fits, with no rogue rooms**: each level goes in the free spot that keeps the map the
  most square, then the closest to its middle (on a randomized map, filling the gaps between clusters); no level is left out in a
  corner on its own.
  The tutorial and the game's end come last, under it all. `MapExpansion.Place()`.
- **A randomized map has no islands of 2-4 rooms**: its clusters are whole communities of rooms (Louvain with its aggregation
  phase), and any cluster under 6 rooms joins the one it shares the most mirrors with. `MapLayout.FindClusters()`.
- Rooms are placed where their doors lead (a door on a room's right edge puts the next room to its right), as KatAM's area maps.
- **The spacing between rooms is consistent, as KatAM's** (on an unrandomized map, the levels as its area maps; on a randomized one, as its clusters), room to room, as much as it can be: no room bleeds into another. This
  holds for the NiDL levels as for KatAM's.
- **Goal rooms are drawn beside each room whose goal mirror leads to them**, as a room of their own each time, not as one room every
  goal mirror's line crosses the map to.
