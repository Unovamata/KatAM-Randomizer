using System;
using System.Collections.Generic;
using System.Linq;

namespace KatAMRandomizer {
    /* KatAMExpansionSqueakLevels; Squeak Squad's 8 worlds as KatAM levels, laid out by the rulebook ("Documentation/Map and Door Design
     * Rules.md"), as PlanDoors() lays NiDL's out, on Squeak Squad's own stages (PlanSqueakLevels()). The DS's own doors (its doorways,
     * drawn in the rooms' tiles, between a stage's rooms: AddSqueakRooms()) stay as they are; what this adds are KatAM mirrors:
     * • a world's regular stages are a tree from its first (its way in): every other stage is entered through a branch door in a room
     *   of another (deeper ones likelier, a big stage branching more; in a room's middle or hidden away, half and half), two-way, to a
     *   mirror back in the first room of the stage it leads to that takes one (rules 3 and 5); where no stage in the tree has a room
     *   taking a door (Prism Plains 1's are small, 2 doors each), a stage's goal door leads on into the next instead, one-way;
     * • only the deepest stage's goal leads to the boss stage (SqueakBossStages); the other goals end their branch, one-way, in turn:
     *   into a goal game (GoalRooms: Central Circle after), a stage of the next world (the previous one for the last world), a goal
     *   game (the DS's goal doors, drawn in the tiles, lead there: no mirror);
     * • the boss stage is entered only through its goal door; nothing else goes in, no door goes out of a boss room (rule 8: its shard
     *   is the way out); a room of it Kirby reaches only by a cutscene on the DS (the final stage's, between its bosses) gets a mirror
     *   from the room before it, where he lands there, one-way;
     * • loops, few (rule 5): one-way shortcuts from the deepest stages back to a shallower one (a third as many as the world has
     *   stages), and a hidden two-way door between each world and the next, into its first stage (Squeak Squad's alone: never into
     *   NiDL's, the owner's);
     * • no new door in a mini-boss's room, nor a landing there; a room keeps 2 exits at most, 3 if it's big (counting the DS's), never 3
     *   doors in one view, doors kept apart, from landings and from warp stars and cannons (rule 2: SqueakSpot()).
     * Left for later (Open Requests): the switch rooms and their Mirras, the links with KatAM's areas, the bosses' shards; */
    internal static partial class KatAMExpansion {
        // The stage with each world's boss (5-7, a copy of 5-6's two rooms, is left out of the port: SqueakTestStages);
        static readonly Dictionary<int, int> SqueakBossStages = new Dictionary<int, int> { [1] = 7, [2] = 7, [3] = 7, [4] = 7, [5] = 6, [6] = 7, [7] = 7, [8] = 4 };
        const int SqueakLayoutSeed = 0x4B5353, SqueakGoalFlags = 0x23, SqueakWorlds = 8;

        /* BuildSqueakHub(); The Squeak Squad hub (room `hub`, after the worlds' rooms), a copy of Central Circle (the owner: "The way in is
         * a copy of the main game's central circle"): Central Circle's own tiles, palettes and tilemap (KatAM's entries, found by their
         * room: NiDL's ROM draws its golden mirror into copies of them) and background, its collision without its doors (its hidden
         * ones, their blocks and records, NiDL's golden mirror: air); Rainbow Route's mirror (MainHubEntry, 19, 14, drawn in its tiles)
         * leads to Prism Plains 1's start, the way in, as the NiDL hub's golden one; a mirror (object 0x6F) on its floor at
         * SqueakHubDoor, two-way with one at the same spot added to Central Circle (the grey door: the owner, "We will later decide on
         * position and visuals"). Its world doors (one per world, as Central Circle's areas', shown by each world's big switch) wait
         * for the switch rooms. Returns an error, or null; */
        static readonly (int x, int y) SqueakHubDoor = (11, 18);
        const int CentralCircle = 0x321;
        static string BuildSqueakHub(byte[] rom, IReadOnlyDictionary<string, int> symbols, bool nidlRooms, int roomProps, int newProps, int newInfos,
                                     int roomInfos, int hub, (int room, int x, int y) wayIn, int navigationRoutes, int noTypes,
                                     Func<string, int, int> add, Func<List<byte>, int, int> doorsList, out byte[] hubCollision, out int hubWidth,
                                     ref int cursor) {
            hubCollision = null;
            hubWidth = 0;
            int Props(int field) => ReadShort(rom, roomProps + CentralCircle * 0x28 + field * 2);
            int Table(string table) => nidlRooms ? roomTables[table].at : symbols[table];
            int Current(string table, int index) => ReadInt(rom, Table(table) + index * 4) - 0x08000000;
            // (KatAM's own entry of a table for Central Circle: the one naming its room at `field`);
            int Vanilla(string table, int field, int fallback) {
                for (int i = 0; i < 0x400; i++) {
                    int entry = ReadInt(rom, symbols[table] + i * 4);
                    if (entry >> 24 != 0x08) break;
                    if (ReadShort(rom, entry - 0x08000000 + field) == CentralCircle) return i;
                }
                return fallback;
            }
            int width = hubWidth = ReadShort(rom, Current("gForegroundTilemaps", Props(12))) / 2;

            // Central Circle's collision and special records, as they are now;
            int solidity = Current("gSolidityMaps", Props(13));
            byte[] collision = DecompressRL(rom, ReadInt(rom, solidity) - 0x08000000);
            int height = collision.Length / width;
            List<byte[]> specials = new List<byte[]>();
            for (int at = ReadInt(rom, solidity + 4) - 0x08000000; rom[at + 4] != 0; at += rom[at + 4]) specials.Add(rom[at..(at + rom[at + 4])]);
            byte[] Ending() => new byte[] { 0, 0, 0, 0, 0, 0, 0xFF, 0xFF };
            byte Facing(int x, int roomWidth) => (byte)((x - 1) * 2 < roomWidth ? 1 : 2);
            if (collision[SqueakHubDoor.y * width + SqueakHubDoor.x] != KatAMAir || collision[(SqueakHubDoor.y + 1) * width + SqueakHubDoor.x] != KatAMSolid)
                return "Central Circle has no floor for the Squeak Squad door.";

            // The hub: Central Circle's cells, its doors air, and its two doors;
            hubCollision = collision.Select(v => v >= 0xFC ? KatAMAir : v).ToArray();
            Door wayInDoor = new Door(MainHubEntry.x, MainHubEntry.y, wayIn.room, wayIn.x, wayIn.y, 1);
            Door backDoor = new Door(SqueakHubDoor.x, SqueakHubDoor.y, CentralCircle, SqueakHubDoor.x + 1, SqueakHubDoor.y + 1, Facing(SqueakHubDoor.x + 1, width));
            List<byte> hubSpecials = new List<byte>();
            foreach (Door door in new[] { wayInDoor, backDoor }) {
                hubCollision[door.Y * width + door.X] = 0xFF;
                hubSpecials.AddRange(DoorRecord(door, 0xFF));
            }
            hubSpecials.AddRange(Ending());
            int collisionAt = AllocateCompressed(rom, CompressRL(hubCollision), ref cursor), specialsAt = AllocateCompressed(rom, hubSpecials.ToArray(), ref cursor);
            int hubSolidity = Allocate(ref cursor, 8, 4);
            WriteInts(rom, hubSolidity, collisionAt + 0x08000000, specialsAt + 0x08000000);
            List<byte[]> templates = new List<byte[]> { MirrorTemplate(SqueakHubDoor.x, SqueakHubDoor.y, 0) };
            int objects = WriteObjectList(rom, templates, new List<byte> { MirrorType }, noTypes, ref cursor);
            int navigation = BuildNavigation(rom, width, height, false, hubCollision, navigationRoutes, new List<(int, int, int, int)>(), ref cursor);
            List<byte> nineROM = new List<byte>();
            nineROM.AddRange(NineROMEntry(wayInDoor.Room, wayInDoor.DestX, wayInDoor.DestY));
            nineROM.AddRange(NineROMEntry(backDoor.Room, backDoor.DestX, backDoor.DestY));
            int doors = doorsList(nineROM, 0);

            // (its props: Central Circle's, KatAM's own graphics, the new collision, objects, routes and doors; room 0x397's info, no routes);
            Array.Copy(rom, newProps + CentralCircle * 0x28, rom, newProps + hub * 0x28, 0x28);
            WriteShort(rom, newProps + hub * 0x28 + 10 * 2, Vanilla("gForegroundInfo", 14, Props(10)));
            WriteShort(rom, newProps + hub * 0x28 + 11 * 2, Vanilla("gForegroundPalettes", 10, Props(11)));
            WriteShort(rom, newProps + hub * 0x28 + 12 * 2, Vanilla("gForegroundTilemaps", 0x1E, Props(12)));
            WriteShort(rom, newProps + hub * 0x28 + 13 * 2, add("gSolidityMaps", hubSolidity));
            WriteShort(rom, newProps + hub * 0x28 + 16 * 2, add("gLevelObjLists", objects));
            WriteShort(rom, newProps + hub * 0x28 + 17 * 2, add("gUnk_08D63C28", navigation));
            WriteShort(rom, newProps + hub * 0x28 + 18 * 2, add("gUnk_08D640A4", doors));
            int roomInfo = Allocate(ref cursor, 0x4C, 4);
            Array.Copy(rom, ReadInt(rom, roomInfos + 0x397 * 4) - 0x08000000, rom, roomInfo, 0x4C);
            WriteShort(rom, roomInfo + 0x44, hub);
            WriteInt(rom, newInfos + hub * 4, roomInfo + 0x08000000);

            /* Central Circle: the door to the hub (its cell, its record, a mirror on it, and its doors list's entry: the list's own
             * struct pointed at a grown copy, as KatAM's edited rooms' are); */
            Door toHub = new Door(SqueakHubDoor.x, SqueakHubDoor.y, hub, SqueakHubDoor.x + 1, SqueakHubDoor.y + 1, Facing(SqueakHubDoor.x + 1, width));
            collision[toHub.Y * width + toHub.X] = 0xFF;
            List<byte> ccSpecials = specials.SelectMany(b => b).ToList();
            ccSpecials.AddRange(DoorRecord(toHub, 0xFF));
            ccSpecials.AddRange(Ending());
            int ccCollision = AllocateCompressed(rom, CompressRL(collision), ref cursor), ccSpecialsAt = AllocateCompressed(rom, ccSpecials.ToArray(), ref cursor);
            int ccSolidity = Allocate(ref cursor, 8, 4);
            WriteInts(rom, ccSolidity, ccCollision + 0x08000000, ccSpecialsAt + 0x08000000);
            WriteShort(rom, newProps + CentralCircle * 0x28 + 13 * 2, add("gSolidityMaps", ccSolidity));
            int list = Current("gLevelObjLists", Props(16));
            List<byte[]> ccTemplates = new List<byte[]>();
            for (int at = ReadInt(rom, list) - 0x08000000; rom[at] != 0 || rom[at + 1] != 0; at += 36) ccTemplates.Add(rom[at..(at + 36)]);
            List<byte> ccTypes = new List<byte>();
            for (int at = ReadInt(rom, list + 4) - 0x08000000; rom[at] != 0xFF; at++) ccTypes.Add(rom[at]);
            if (!ccTypes.Contains(MirrorType)) ccTypes.Add(MirrorType);
            ccTemplates.Add(MirrorTemplate(toHub.X, toHub.Y, ccTemplates.Count));
            if (ccTemplates.Count > 31) return "Central Circle has no room for the Squeak Squad door's mirror.";
            WriteShort(rom, newProps + CentralCircle * 0x28 + 16 * 2, add("gLevelObjLists", WriteObjectList(rom, ccTemplates, ccTypes, ReadInt(rom, list + 8) - 0x08000000, ref cursor)));
            int doorsStruct = Current("gUnk_08D640A4", Props(18));
            List<byte> ccNine = new List<byte>();
            int entry = ReadInt(rom, doorsStruct) - 0x08000000;
            for (; rom[entry] != 0 || rom[entry + 1] != 0; entry += rom[entry + 1]) ccNine.AddRange(rom[entry..(entry + rom[entry + 1])]);
            ccNine.AddRange(NineROMEntry(toHub.Room, toHub.DestX, toHub.DestY));
            ccNine.AddRange(new byte[] { 0, 0, 0xFF, 0xFF });
            int ccNineAt = AllocateCompressed(rom, ccNine.ToArray(), ref cursor);
            WriteInt(rom, doorsStruct, ccNineAt + 0x08000000);
            return null;
        }

        /* SqueakShardExits(); Where each world's boss's shard takes Kirby (the rulebook's rule 8: a shard only ever sends Kirby to a central
         * hub, onto the level's door there): gNidlShardExits' entries after the NiDL bosses' ("ROM Expansion/nidl_boss.c", NidlShardExit()
         * after the shard's cutscene, HookShardExit()), the Squeak Squad hub, at the world's door's spot (Central Circle's door of the area
         * of its number, MainHubDoors, as the NiDL hub's levels': the world doors themselves come with the switch rooms), each Kirby on his
         * own cell of its ground (HubGroundBeside()); */
        const int ShardExitCount = 24;
        static string SqueakShardExits(byte[] rom, KatAMExpansionCode.Version version, IReadOnlyDictionary<string, int> symbols, int hub,
                                       Dictionary<int, int> bossRooms, byte[] hubCollision, int hubWidth) {
            if (HookShardExit(rom, version) is string error) return error;
            int exits = CodeAddress + version.CodeSymbols["gNidlShardExits"], n = 0;
            while (n < ShardExitCount && ReadShort(rom, exits + n * 8) != 0) n++;
            foreach ((int room, int world) in bossRooms.OrderBy(b => b.Key)) {
                if (n >= ShardExitCount) return "Too many boss rooms for the shards' exits.";
                (int x, int y) spot = MainHubDoors[(world - 1) % MainHubDoors.Length];
                WriteShorts(rom, exits + n * 8, room, hub);
                rom[exits + n * 8 + 4] = (byte)(spot.x + 1);
                rom[exits + n * 8 + 5] = (byte)(spot.y + 1);
                (rom[exits + n * 8 + 6], rom[exits + n * 8 + 7]) = HubGroundBeside(rom, hubCollision, hubWidth, symbols["gCollisionAttributes"], spot.x, spot.y);
                n++;
            }
            // (the map tracker's count of them, "SX": AddRooms() writes it with the NiDL rooms);
            System.Text.Encoding.ASCII.GetBytes("SX").CopyTo(rom, MapShardExits);
            WriteShort(rom, MapShardExits + 2, n);
            WriteInt(rom, MapShardExits + 4, exits + 0x08000000);
            return null;
        }

        // SqueakPlanRoom; What the plan knows of a Squeak Squad room: its place, size, KatAM collision, the DS's doors, its fights, its warp
        // stars and cannons (cells);
        internal sealed class SqueakPlanRoom {
            public int Index, Room, Area, Stage, Number, Width, Height;
            public byte[] Collision;
            public List<SqueakDoor> Doors;
            public bool Midboss, Boss;
            public List<(int x, int y)> Transports = new List<(int, int)>();
            // (the stage's rooms its warp stars and fuse cannons take Kirby to, numbered from 1);
            public List<int> Rides = new List<int>();
            public byte At(int x, int y) => x < 0 || y < 0 || x >= Width || y >= Height ? KatAMSolid : Collision[y * Width + x];
        }

        // (a cell Kirby goes through: air, water, ladders, doors; star blocks too, once broken, if blocks);
        static bool SqueakOpen(byte v, bool blocks = false) => v is KatAMAir or KatAMWater or KatAMLadder or KatAMLadderTop or >= 0xFC || blocks && v == KatAMStarBlock;
        // (a cell Kirby stands on: solid, slopes, thin floors, a ladder's top, blocks);
        static bool SqueakFloor(byte v) => v is (>= 0x05 and <= 0x1B) or KatAMLadderTop or (>= KatAMStarBlock and <= KatAMMetalBlock);
        // SqueakGoal(); Whether a DS door is its stage's goal; SqueakArrival(), where its doors land Kirby (no room, not a goal);
        /* (a goal: flags 0x23, with bit 7 too (4-4-9's: as the doors' 0x90), whatever its room (2-8-7's names room 13 of a stage of 7, the
         * only one of the 48 that names one: the DS's goal all the same); a room's start: no room, not a goal); */
        static bool SqueakGoal(SqueakDoor d) => (d.Flags & 0x7F) == SqueakGoalFlags;
        static bool SqueakArrival(SqueakDoor d) => d.Room == SqueakNoRoom && !SqueakGoal(d);

        /* PlanSqueakLevels(); The mirrors to add to each room (by its index), from SqueakLayoutSeed; each world's way in (its first
         * stage's start: the room, cell + 1) and the boss rooms (their room ids, by world); */
        static Dictionary<int, List<Door>> PlanSqueakLevels(List<SqueakPlanRoom> rooms, out (int room, int x, int y)[] worldStarts,
                                                           out Dictionary<int, int> bossRooms) {
            Random random = new Random(SqueakLayoutSeed);
            Dictionary<int, List<Door>> doors = rooms.ToDictionary(r => r.Index, r => new List<Door>());
            var byStage = rooms.GroupBy(r => (r.Area, r.Stage)).ToDictionary(g => g.Key, g => g.OrderBy(r => r.Number).ToList());
            Dictionary<int, SqueakPlanRoom> byRoom = rooms.ToDictionary(r => r.Room);
            bool IsBossStage((int area, int stage) s) => SqueakBossStages.TryGetValue(s.area, out int boss) && boss == s.stage;
            HashSet<int> goalRooms = rooms.Where(r => r.Doors.Any(SqueakGoal)).Select(r => r.Room).ToHashSet();
            List<(int room, int x, int y)> landings = new List<(int, int, int)>();
            byte Facing(SqueakPlanRoom to, int x) => (byte)((x - 1) * 2 < to.Width ? 1 : 2);

            // Where Kirby stands at a cell: down to the floor;
            (int x, int y) Drop(SqueakPlanRoom r, int x, int y) {
                while (y + 1 < r.Height && SqueakOpen(r.At(x, y + 1))) y++;
                return (x, y);
            }
            // A stage's start: its first room's first arrival (the DS's kind 6, or another entrance), on the floor;
            (int room, int x, int y) StageStart((int area, int stage) s) {
                SqueakPlanRoom first = byStage[s][0];
                SqueakDoor arrival = first.Doors.FirstOrDefault(SqueakArrival);
                (int x, int y) at = arrival != null ? Drop(first, arrival.X, arrival.Y) : SqueakSpot(first, false, false) ?? (first.Width / 2, first.Height / 2);
                return (first.Room, at.x + 1, at.y + 1);
            }

            // The doors a room has (the DS's ways on, goals, and the new mirrors), and its cells taken (doors, arrivals, landings, rides);
            int Exits(SqueakPlanRoom r) => r.Doors.Count(d => d.Room != SqueakNoRoom || SqueakGoal(d)) + doors[r.Index].Count;
            int MaxExits(SqueakPlanRoom r) => r.Width * r.Height <= 2 * ViewWidth * (ViewHeight + 1) ? 2 : 3;
            IEnumerable<(int x, int y)> Drawn(SqueakPlanRoom r) => r.Doors.Where(d => d.Room != SqueakNoRoom || SqueakGoal(d)).Select(d => (d.X, d.Y))
                                                                           .Concat(doors[r.Index].Select(d => (d.X, d.Y)));
            IEnumerable<(int x, int y)> Taken(SqueakPlanRoom r) => Drawn(r).Concat(r.Doors.Where(SqueakArrival).Select(d => (d.X, d.Y)))
                                                                          .Concat(landings.Where(l => l.room == r.Room).Select(l => (l.x - 1, l.y - 1)));
            bool ViewFits(SqueakPlanRoom r, int x, int y) {
                List<(int x, int y)> near = Drawn(r).Where(d => Math.Abs(d.x - x) < ViewWidth && Math.Abs(d.y - y) < ViewHeight).ToList();
                for (int i = 0; i < near.Count; i++)
                    for (int j = i + 1; j < near.Count; j++) {
                        int[] xs = { x, near[i].x, near[j].x }, ys = { y, near[i].y, near[j].y };
                        if (xs.Max() - xs.Min() < ViewWidth && ys.Max() - ys.Min() < ViewHeight) return false;
                    }
                return true;
            }
            /* The rooms Kirby reaches in each stage from its start, through the DS's doors (they lead on, one-way: a room's arrival is no
             * door back) and the ones added so far; a room nothing leads into on the DS (Prism Plains 1's fifth) isn't one, until a door
             * throws him in (see the end); */
            HashSet<int> reachable = new HashSet<int>();
            void Reach(int room) {
                Queue<int> queue = new Queue<int>();
                if (reachable.Add(room)) queue.Enqueue(room);
                while (queue.Count > 0) {
                    SqueakPlanRoom r = byRoom[queue.Dequeue()];
                    IEnumerable<int> next = r.Doors.Where(d => d.Room != SqueakNoRoom).Select(d => byStage[(r.Area, r.Stage)].FirstOrDefault(t => t.Number == d.Room + 1)?.Room ?? -1)
                                                   .Concat(r.Rides.Select(n => byStage[(r.Area, r.Stage)].FirstOrDefault(t => t.Number == n)?.Room ?? -1))
                                                   .Concat(doors[r.Index].Select(d => d.Room)).Where(byRoom.ContainsKey);
                    foreach (int n in next) if (reachable.Add(n)) queue.Enqueue(n);
                }
            }
            // Hosts(); Whether a room takes a new door: one Kirby reaches, under its exits, not a goal room, a boss stage's, nor a mini-boss's;
            bool Hosts(SqueakPlanRoom r) => reachable.Contains(r.Room) && Exits(r) < MaxExits(r) && !goalRooms.Contains(r.Room)
                                            && !IsBossStage((r.Area, r.Stage)) && !r.Midboss && !r.Boss;

            /* SqueakSpot(); A floor cell for a mirror (door) or a landing: a floor 3 cells wide under it, open beside and above it, not
             * water, where the camera shows it (rows MirrorTop to the height - 3), one Kirby reaches from where he comes in (through star
             * blocks too), away from the room's doors, arrivals, landings and rides (2 x MirrorSpacing across, 2 up or down), the view
             * keeping 2 doors at most; in the room's middle, or hidden (the farthest from where he comes in, behind blocks); */
            (int x, int y)? SqueakSpot(SqueakPlanRoom r, bool hidden, bool door) {
                int width = r.Width, height = r.Height;
                int[] Distances(bool blocks) {
                    int[] distance = Enumerable.Repeat(-1, width * height).ToArray();
                    Queue<(int x, int y)> open = new Queue<(int, int)>();
                    foreach (SqueakDoor d in r.Doors)
                        if (d.X >= 0 && d.Y >= 0 && d.X < width && d.Y < height && distance[d.Y * width + d.X] < 0) { distance[d.Y * width + d.X] = 0; open.Enqueue((d.X, d.Y)); }
                    while (open.Count > 0) {
                        (int x, int y) = open.Dequeue();
                        foreach ((int nx, int ny) in new[] { (x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1) })
                            if (nx >= 0 && ny >= 0 && nx < width && ny < height && distance[ny * width + nx] < 0 && SqueakOpen(r.At(nx, ny), blocks)) {
                                distance[ny * width + nx] = distance[y * width + x] + 1;
                                open.Enqueue((nx, ny));
                            }
                    }
                    return distance;
                }
                int[] plain = Distances(false), broken = Distances(true);
                List<(int x, int y)> taken = Taken(r).ToList();
                (int x, int y)? best = null;
                double bestScore = double.MinValue;
                for (int y = MirrorTop; y <= height - 3; y++) {
                    for (int x = 1; x <= width - 2; x++) {
                        int distance = broken[y * width + x];
                        if (distance < 0 || r.At(x, y) != KatAMAir || !SqueakOpen(r.At(x, y - 1)) || !SqueakOpen(r.At(x - 1, y)) || !SqueakOpen(r.At(x + 1, y))) continue;
                        if (Enumerable.Range(x - 1, 3).Any(fx => !SqueakFloor(r.At(fx, y + 1)))) continue;
                        if (taken.Any(c => Math.Abs(c.x - x) < MirrorSpacing * 2 && Math.Abs(c.y - y) <= 2)) continue;
                        if (r.Transports.Any(t => Math.Abs(t.x - x) < MirrorSpacing * 2 && Math.Abs(t.y - y) <= 2)) continue;
                        if (door && !ViewFits(r, x, y)) continue;
                        double score = hidden ? distance + (plain[y * width + x] < 0 ? 60 : 0) : -Math.Abs(x - width / 2.0) + random.NextDouble() * 6;
                        if (score > bestScore) { bestScore = score; best = (x, y); }
                    }
                }
                return best;
            }
            // HostSpot(); A spot for a new door in a room of a stage that takes one (not its first, if it has others), bigger rooms likelier;
            (SqueakPlanRoom room, int x, int y)? HostSpot((int area, int stage) s, bool hidden) {
                List<SqueakPlanRoom> list = byStage[s];
                var order = list.Skip(list.Count > 1 ? 1 : 0).OrderByDescending(r => random.NextDouble() * (1 + r.Width * r.Height / 330.0)).Concat(list.Take(list.Count > 1 ? 1 : 0));
                foreach (SqueakPlanRoom r in order)
                    if (Hosts(r) && SqueakSpot(r, hidden, true) is (int x, int y)) return (r, x, y);
                return null;
            }
            /* EntrySpot(); Where a door into a stage stands, two-way (a branch's way back): in its first room, in its middle, if it takes
             * one (the DS's doors only lead on: entered past its first room, the rooms before would be cut off); else none, and the
             * branch throws Kirby onto its start, one-way; */
            (SqueakPlanRoom room, int x, int y)? EntrySpot((int area, int stage) s) {
                SqueakPlanRoom first = byStage[s][0];
                bool hosts = Exits(first) < MaxExits(first) && !goalRooms.Contains(first.Room) && !first.Midboss && !first.Boss;
                return hosts && SqueakSpot(first, false, true) is (int x, int y) ? (first, x, y) : null;
            }
            // LandingIn(); Where a one-way door throws Kirby into a stage: a floor in one of its rooms away from its doors; its start, if none;
            (int room, int x, int y) LandingIn((int area, int stage) s) {
                foreach (SqueakPlanRoom r in byStage[s].Where(r => !goalRooms.Contains(r.Room) && !r.Midboss && !r.Boss).OrderBy(_ => random.Next()))
                    if (SqueakSpot(r, false, false) is (int x, int y)) {
                        landings.Add((r.Room, x + 1, y + 1));
                        return (r.Room, x + 1, y + 1);
                    }
                return StageStart(s);
            }
            Door Add(SqueakPlanRoom at, int x, int y, (int room, int x, int y) to, DoorKind kind, bool oneWay, bool hidden = false) {
                Door door = new Door(x, y, to.room, to.x, to.y, byRoom.TryGetValue(to.room, out SqueakPlanRoom target) ? Facing(target, to.x) : (byte)1,
                                     OneWay: oneWay, Hidden: hidden, Kind: kind);
                doors[at.Index].Add(door);
                if (reachable.Contains(at.Room) && byRoom.ContainsKey(to.room)) Reach(to.room);
                return door;
            }
            // A two-way pair: a door at one spot to the other, and one back; Kirby lands on the door back, as KatAM's;
            void Pair((SqueakPlanRoom room, int x, int y) a, (SqueakPlanRoom room, int x, int y) b, DoorKind kind, bool hidden) {
                Add(a.room, a.x, a.y, (b.room.Room, b.x + 1, b.y + 1), kind, false, hidden);
                Add(b.room, b.x, b.y, (a.room.Room, a.x + 1, a.y + 1), kind == DoorKind.Branch ? DoorKind.BranchBack : kind, false);
            }

            worldStarts = new (int, int, int)[SqueakWorlds + 1];
            bossRooms = new Dictionary<int, int>();
            List<(int area, int stage)> stages = byStage.Keys.OrderBy(s => s.Item1).ThenBy(s => s.Item2).ToList();
            Dictionary<(int, int), int> depthOf = new Dictionary<(int, int), int>();
            Dictionary<(int, int), (int, int)> goalBranch = new Dictionary<(int, int), (int, int)>();
            for (int world = 1; world <= SqueakWorlds; world++) {
                List<(int area, int stage)> regular = stages.Where(s => s.area == world && !IsBossStage(s)).ToList();
                if (regular.Count == 0) continue;
                worldStarts[world] = StageStart(regular[0]);
                Reach(worldStarts[world].room);

                // The tree: each stage after the first hangs off one already in it (deeper ones likelier, up to 3 branches a big stage);
                Dictionary<(int, int), int> depth = new Dictionary<(int, int), int> { [regular[0]] = 0 }, children = new Dictionary<(int, int), int> { [regular[0]] = 0 };
                int Branches((int area, int stage) p) => Math.Clamp(1 + byStage[p].Count / 4, 1, 3);
                bool hiddenNext = random.Next(2) == 0;
                // (a stage hangs off the first in the tree with a room that takes a door, those under their branches first, deeper ones
                // likelier; Prism Plains 1's rooms are small, 2 doors each: its branches hang off the stages after it; the stages left
                // out wait for more to be in, round after round);
                List<(int area, int stage)> waiting = regular.Skip(1).OrderBy(_ => random.Next()).ToList();
                for (bool progress = true; progress && waiting.Count > 0;) {
                    progress = false;
                    foreach (var s in waiting.ToList()) {
                        var froms = depth.Keys.OrderBy(p => children[p] < Branches(p) ? 0 : 1).ThenByDescending(p => random.NextDouble() * (1 + depth[p])).ToList();
                        foreach (var from in froms) {
                            // (the branch door in a room of `from`, two-way to a mirror in `s`'s first room that takes one; else one-way, onto
                            // a floor there);
                            if (HostSpot(from, hiddenNext) is not (SqueakPlanRoom, int, int) at) continue;
                            if (EntrySpot(s) is (SqueakPlanRoom, int, int) entry) Pair(at, entry, DoorKind.Branch, hiddenNext);
                            else Add(at.room, at.x, at.y, StageStart(s), DoorKind.Branch, true, hiddenNext);
                            hiddenNext = !hiddenNext;
                            children[from]++;
                            depth[s] = depth[from] + 1;
                            children[s] = 0;
                            waiting.Remove(s);
                            progress = true;
                            break;
                        }
                        // (none takes a door: a stage's goal door, unused, leads on into it instead, one-way, as the DS's goals lead on);
                        if (!waiting.Contains(s)) continue;
                        foreach (var from in froms) {
                            if (goalBranch.ContainsKey(from) || !byStage[from].Any(r => r.Doors.Any(SqueakGoal))) continue;
                            goalBranch[from] = s;
                            Reach(StageStart(s).room);
                            children[from]++;
                            depth[s] = depth[from] + 1;
                            children[s] = 0;
                            waiting.Remove(s);
                            progress = true;
                            break;
                        }
                    }
                }
                foreach (var pair in depth) depthOf[pair.Key] = pair.Value;
            }

            /* The goals: the deepest stage's to the boss stage; the others in turn into a goal game, the next world's stage, a goal game
             * (the DS's goal doors, its own doorways: Plain); */
            Dictionary<(int, int), (int room, int x, int y)> goals = new Dictionary<(int, int), (int, int, int)>();
            for (int world = 1; world <= SqueakWorlds; world++) {
                List<(int area, int stage)> reached = stages.Where(s => s.area == world && depthOf.ContainsKey(s)).ToList();
                if (reached.Count == 0) continue;
                var gate = reached.OrderByDescending(s => depthOf[s]).ThenByDescending(s => s.stage).First();
                var boss = stages.FirstOrDefault(s => s.area == world && IsBossStage(s));
                int ends = 0;
                foreach (var s in reached) {
                    if (goalBranch.TryGetValue(s, out var child)) { goals[s] = StageStart(child); continue; }
                    if (s == gate && boss != default) { goals[s] = StageStart(boss); continue; }
                    int other = world == SqueakWorlds ? world - 1 : world + 1;
                    var theirs = stages.Where(t => t.area == other && depthOf.ContainsKey(t)).ToList();
                    goals[s] = ends++ % 3 == 1 && theirs.Count > 0 ? LandingIn(theirs[random.Next(theirs.Count)])
                             : (GoalRooms[(world + ends) % GoalRooms.Length], GoalLanding.x, GoalLanding.y);
                }
            }
            foreach (SqueakPlanRoom r in rooms)
                foreach (SqueakDoor goal in r.Doors.Where(SqueakGoal))
                    if (goals.TryGetValue((r.Area, r.Stage), out var to) && goal.X < r.Width && goal.Y < r.Height)
                        doors[r.Index].Add(new Door(goal.X, goal.Y, to.room, to.x, to.y, byRoom.TryGetValue(to.room, out SqueakPlanRoom target) ? Facing(target, to.x) : (byte)1,
                                                    OneWay: true, Kind: DoorKind.Goal, Plain: true));

            /* The boss stages: the DS's doors on (into the boss room: one-way, there's none back); a room the DS reaches only by a
             * cutscene (no door or warp star of the stage leads to it: the final stage's, between its fights, room 5 to 7, 6 to 9...)
             * gets a mirror, one-way, from the first room before it with no way on yet (its fight's end), where Kirby lands there; the
             * last room with a boss is the world's boss room. A warp star is a way on (8-4's rides, 3 to 4 to 5: the star lands Kirby
             * where the DS does; its arrival door in 8-4-4 is under the room's floor, in its last rows); */
            foreach (var s in stages.Where(IsBossStage)) {
                List<SqueakPlanRoom> list = byStage[s];
                HashSet<int> led = list.SelectMany(r => r.Doors.Where(d => d.Room != SqueakNoRoom).Select(d => d.Room + 1).Concat(r.Rides)).ToHashSet();
                List<SqueakPlanRoom> ends = new List<SqueakPlanRoom>();
                foreach (SqueakPlanRoom next in list) {
                    if (next != list[0] && !led.Contains(next.Number) && ends.Count > 0) {
                        SqueakPlanRoom before = ends[0];
                        ends.RemoveAt(0);
                        (int x, int y) arrival = before.Doors.FirstOrDefault(SqueakArrival) is SqueakDoor d ? Drop(before, d.X, d.Y) : (before.Width / 2, before.Height / 2);
                        (int x, int y) landing = next.Doors.FirstOrDefault(SqueakArrival) is SqueakDoor n ? Drop(next, n.X, n.Y) : (next.Width / 2, next.Height / 2);
                        Add(before, arrival.x, arrival.y, (next.Room, landing.x + 1, landing.y + 1), DoorKind.Boss, true);
                    }
                    if (!next.Doors.Any(d => d.Room != SqueakNoRoom) && next.Rides.Count == 0) ends.Add(next);
                }
                if (list.LastOrDefault(r => r.Boss) is SqueakPlanRoom bossRoom) bossRooms[bossRoom.Room] = s.area;
            }

            /* Loops, few and each for a reason: one-way shortcuts from the deepest stages back to one at least two branches shallower
             * (else any shallower), a third as many as a world has stages; a hidden two-way door between each world and the next; */
            for (int world = 1; world <= SqueakWorlds; world++) {
                List<(int area, int stage)> reached = stages.Where(s => s.area == world && depthOf.ContainsKey(s)).ToList();
                foreach (var a in reached.OrderByDescending(s => depthOf[s]).Take(Math.Max(1, reached.Count / 3))) {
                    var home = reached.Where(s => depthOf[s] <= depthOf[a] - 2).ToList();
                    if (home.Count == 0) home = reached.Where(s => depthOf[s] < depthOf[a]).ToList();
                    if (home.Count == 0) continue;
                    if (HostSpot(a, true) is (SqueakPlanRoom, int, int) at) Add(at.room, at.x, at.y, LandingIn(home[random.Next(home.Count)]), DoorKind.Loop, true, true);
                }
                if (world == SqueakWorlds) continue;
                List<(int area, int stage)> next = stages.Where(s => s.area == world + 1 && depthOf.ContainsKey(s)).ToList();
                if (reached.Count == 0 || next.Count == 0) continue;
                // (into the next world's first stage, its way in: its tree is reached from there; from any of ours that takes a door);
                var theirs = next[0];
                foreach (var ours in reached.OrderBy(_ => random.Next()))
                    if (HostSpot(ours, true) is (SqueakPlanRoom, int, int) here) {
                        if (EntrySpot(theirs) is (SqueakPlanRoom, int, int) there) Pair(here, there, DoorKind.Link, true);
                        else Add(here.room, here.x, here.y, StageStart(theirs), DoorKind.Link, true, true);
                        break;
                    }
            }
            /* Every room reached (rule 9): one nothing leads into gets a one-way door into it, where the DS lands Kirby there (its
             * arrival), from a room of its stage that takes one (else of its world), hidden; but for the rooms a stage reaches only
             * through its Squeaks on the DS (after its goal: the gold-pile treasure rooms and the hideouts where Spinni, Doc or Storo
             * fight for the chest they took), which the Squeak Lair doors lead to (Open Requests: not made yet), so SqueakLair(); */
            bool SqueakLair(SqueakPlanRoom r) {
                List<SqueakPlanRoom> list = byStage[(r.Area, r.Stage)];
                int goal = list.FirstOrDefault(t => t.Doors.Any(SqueakGoal))?.Number ?? int.MaxValue;
                return r.Number > goal;
            }
            for (bool progress = true; progress;) {
                progress = false;
                foreach (SqueakPlanRoom r in rooms.Where(r => !reachable.Contains(r.Room) && !IsBossStage((r.Area, r.Stage)) && !SqueakLair(r))) {
                    if (r.Doors.FirstOrDefault(SqueakArrival) is not SqueakDoor arrival) continue;
                    (int x, int y) landing = Drop(r, arrival.X, arrival.Y);
                    var hosts = byStage[(r.Area, r.Stage)].Concat(rooms.Where(t => t.Area == r.Area && t.Stage != r.Stage)).Where(Hosts);
                    foreach (SqueakPlanRoom host in hosts)
                        if (SqueakSpot(host, true, true) is (int x, int y)) {
                            Add(host, x, y, (r.Room, landing.x + 1, landing.y + 1), DoorKind.Loop, true, true);
                            progress = true;
                            break;
                        }
                }
            }
            return doors;
        }
    }
}
