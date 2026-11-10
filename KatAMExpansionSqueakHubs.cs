using System;
using System.Collections.Generic;
using System.Linq;

namespace KatAMRandomizer {
    /* KatAMExpansionSqueakHubs; Squeak Squad's worlds joined to the game as NiDL's levels are (the rulebook's rules 4 and 5, "Documentation/
     * Map and Door Design Rules.md"):
     * • a switch room per world (after the Squeak Squad hub, in world order), as NiDL's (BuildSwitchRooms()): one screen in KatAM's two
     *   hub-door layouts (A for worlds 1, 3, 5, 7, B for 2, 4, 6, 8), walled on top, a pedestal in the middle with the world's big switch
     *   on it, a mirror on each side's floor and two ability statues on its top row; its look the world's own tiles, a provisional one
     *   (SqueakSwitchTilemap(): the owner's to pick, mock-ups to come);
     * • its way in a Mirra hidden in the world's second half, one-way; its floor mirrors both ways into the world: the right one to the
     *   start of the stage before the boss's, the left one to another branch's (PlanSqueakLevels());
     * • its big switch sets the world's slot (SqueakSlot(): 0x20 on, the expansion's save, rooms.c's BigGetStateSlot), which shows its 2
     *   hub doors (object 0x97) and the world's door in the Squeak Squad hub (object 0x8C, Central Circle's door at MainHubDoors), and
     *   opens the world's door on the world map ("ROM Expansion/worldmap.c"): its hub door to the Squeak Squad hub, its link door to the
     *   KatAM room the world's path leads to on the world map (SqueakLinks: the owner's pick of the mock-up), which gets a door back
     *   (SqueakLinkSpots()); */
    internal static partial class KatAMExpansion {
        const int SqueakSlotBase = 0x20;
        static int SqueakSlot(int world) => SqueakSlotBase + world - 1;
        // (the NiDL level whose layout a world's switch room takes: 1 B, 0 A);
        static int SqueakLayoutLevel(int world) => world % 2 == 0 ? 1 : 0;
        // SqueakLinks; Where each world's path leads on the world map: Moonlight Mansion's door, Rainbow Route South, East, West, North,
        // Peppermint Palace East, Olive Ocean, Candy Constellation (the KatAM rooms of those doors, sWorldMapDoorRoomIds);
        static readonly int[] SqueakLinks = { 0xBE, 0xC2, 0x316, 0xC0, 0x24E, 0x122, 0x37A, 0x1EA };
        /* SqueakStatues: the ability statues on each switch room's top row (as SwitchStatues: 0x92 Beam, Burning, Laser, Fire, Stone, Fighter,
         * Bomb, Tornado; 0x93 Wheel, Ice, Spark, Missile, Smash, Throw, Hammer; 0x94 Parasol; 0x95 Sword, Cutter, Cupid), provisional:
         * Prism Plains Cutter and Beam, Nature Notch Bomb and Stone, Cushy Cloud Spark and Tornado, Jam Jungle Parasol and Throw, Vocal
         * Volcano Fire and Burning, Ice Island Ice and Hammer, Secret Sea Laser and Wheel, Gamble Galaxy Smash and Sword; */
        static readonly (byte type, byte subtype)[][] SqueakStatues = {
            new[] { ((byte)0x95, (byte)1), ((byte)0x92, (byte)0) }, new[] { ((byte)0x92, (byte)6), ((byte)0x92, (byte)4) },
            new[] { ((byte)0x93, (byte)2), ((byte)0x92, (byte)7) }, new[] { ((byte)0x94, (byte)0), ((byte)0x93, (byte)5) },
            new[] { ((byte)0x92, (byte)3), ((byte)0x92, (byte)1) }, new[] { ((byte)0x93, (byte)1), ((byte)0x93, (byte)6) },
            new[] { ((byte)0x92, (byte)2), ((byte)0x93, (byte)0) }, new[] { ((byte)0x93, (byte)4), ((byte)0x95, (byte)0) },
        };

        /* SqueakLinkSpots(); Where a world's door back stands in its KatAM room, by NiDL's own rule for these rooms (KatAMLinkDoors): a
         * room with one door on its top row takes 0x122's layout (its door moved from the middle to cell 12, its statues between, the new
         * door at cell 3: 0x316 and 0xBE did that for NiDL's doors, so with NiDL's rooms there the world's stands in the middle, drawn at
         * x 128, as 0x122's NiDL door); one with two on top the middle (0xC0; 0x122 without NiDL's rooms: with them, where they took the
         * middle, the floor's right, under Central Circle's door); one with a door on its lower floor (0xC2, 0x24E, 0x1EA) takes 0xBF's
         * layout (that door moved from under the big switch to cell 11, the new one at cell 5, as 0x250's for NiDL); Kirby lands on the
         * door. Moves: (from, to) cells; statue moves: (from x, to x); landings: the rooms whose doors land on a moved door; */
        static ((int x, int y) door, int drawX, ((int x, int y) from, (int x, int y) to)[] moves, (int from, int to)[] statues, int[] landings)
            SqueakLinkSpot(int world, bool nidlRooms) {
            var topMove = new[] { ((8, 3), (12, 3)) };
            var statueMove = new[] { (64, 96), (200, 160) };
            var floorMove = new[] { ((8, 8), (11, 8)) };
            var none = new ((int, int), (int, int))[0];
            var noStatues = new (int, int)[0];
            return world switch {
                1 or 3 => nidlRooms ? ((8, 3), 128, none, noStatues, new int[0]) : ((3, 3), -1, topMove, statueMove, new[] { CentralCircle }),
                2 => ((5, 8), -1, floorMove, noStatues, new[] { 0x24F }),
                4 => ((8, 3), 128, none, noStatues, new int[0]),
                5 or 8 => ((5, 8), -1, floorMove, noStatues, new[] { CentralCircle }),
                6 => nidlRooms ? ((12, 8), -1, none, noStatues, new int[0]) : ((8, 3), 128, none, noStatues, new int[0]),
                _ => ((3, 3), -1, topMove, statueMove, new[] { 0xBF }),
            };
        }

        /* KatAMRoomEdit; A KatAM room (Central Circle, the rooms Squeak Squad's worlds link to, the rooms landing on a door they move) as it is
         * once the randomizer is done and NiDL's rooms have added their doors (FinishRooms(): squeakFinish), edited, then written back into
         * its own table entries (SaveKatAMRooms(): none of these rooms shares one): its collision, special records, objects, doors list (9ROM
         * entries) and foreground tilemap (read only: a new door's cell look); */
        sealed class KatAMRoomEdit {
            public int Room, Width, Height, TypesRest, DoorsStruct;
            public byte[] Collision;
            public List<byte[]> Specials = new List<byte[]>(), Templates = new List<byte[]>(), Nine = new List<byte[]>();
            public List<byte> Types = new List<byte>();
            public ushort[] Tilemap;
            public int Columns;
            public bool Changed;
            public ushort[] Look(int x, int y) => new[] { Tilemap[2 * y * Columns + 2 * x], Tilemap[2 * y * Columns + 2 * x + 1],
                                                          Tilemap[(2 * y + 1) * Columns + 2 * x], Tilemap[(2 * y + 1) * Columns + 2 * x + 1] };
        }

        // LoadKatAMRoom(); A KatAM room's edit, from its props (newProps) and the tables they index (`table`: a table's base);
        static KatAMRoomEdit LoadKatAMRoom(byte[] rom, int newProps, Func<string, int> table, int room) {
            int Props(int field) => ReadShort(rom, newProps + room * 0x28 + field * 2);
            int Current(string name, int index) => ReadInt(rom, table(name) + index * 4) - 0x08000000;
            KatAMRoomEdit edit = new KatAMRoomEdit { Room = room };
            int tilemapEntry = Current("gForegroundTilemaps", Props(12));
            edit.Columns = ReadShort(rom, tilemapEntry);
            edit.Width = edit.Columns / 2;
            byte[] tilemap = DecompressLZ77(rom, ReadInt(rom, tilemapEntry + 0x18) - 0x08000000);
            edit.Tilemap = Enumerable.Range(0, tilemap.Length / 2).Select(i => (ushort)ReadShort(tilemap, i * 2)).ToArray();
            int solidity = Current("gSolidityMaps", Props(13));
            edit.Collision = DecompressRL(rom, ReadInt(rom, solidity) - 0x08000000);
            edit.Height = edit.Collision.Length / edit.Width;
            for (int at = ReadInt(rom, solidity + 4) - 0x08000000; rom[at + 4] != 0; at += rom[at + 4]) edit.Specials.Add(rom[at..(at + rom[at + 4])]);
            int list = Current("gLevelObjLists", Props(16));
            for (int at = ReadInt(rom, list) - 0x08000000; rom[at] != 0 || rom[at + 1] != 0; at += 36) edit.Templates.Add(rom[at..(at + 36)]);
            for (int at = ReadInt(rom, list + 4) - 0x08000000; rom[at] != 0xFF; at++) edit.Types.Add(rom[at]);
            edit.TypesRest = ReadInt(rom, list + 8) - 0x08000000;
            edit.DoorsStruct = Current("gUnk_08D640A4", Props(18));
            for (int at = ReadInt(rom, edit.DoorsStruct) - 0x08000000; rom[at] != 0 || rom[at + 1] != 0; at += rom[at + 1]) edit.Nine.Add(rom[at..(at + rom[at + 1])]);
            return edit;
        }

        // SaveKatAMRooms(); The edited rooms written into their table entries (`table`: a table's base; the props, `props`, name them): new
        // collision and records, objects, and doors lists (each list's struct a copy, with its count, pointing at the grown list);
        static void SaveKatAMRooms(byte[] rom, int props, Func<string, int> table, IEnumerable<KatAMRoomEdit> edits, ref int cursor) {
            int Slot(string name, int room, int field) => table(name) + ReadShort(rom, props + room * 0x28 + field * 2) * 4;
            foreach (KatAMRoomEdit edit in edits.Where(e => e.Changed)) {
                if (edit.Templates.Count > 31) throw new InvalidOperationException($"Room 0x{edit.Room:X} has {edit.Templates.Count} objects (KatAM's rooms hold 31).");
                List<byte> specials = edit.Specials.SelectMany(b => b).ToList();
                specials.AddRange(new byte[] { 0, 0, 0, 0, 0, 0, 0xFF, 0xFF });
                int collisionAt = AllocateCompressed(rom, CompressRL(edit.Collision), ref cursor), specialsAt = AllocateCompressed(rom, specials.ToArray(), ref cursor);
                int solidity = Allocate(ref cursor, 8, 4);
                WriteInts(rom, solidity, collisionAt + 0x08000000, specialsAt + 0x08000000);
                WriteInt(rom, Slot("gSolidityMaps", edit.Room, 13), solidity + 0x08000000);
                WriteInt(rom, Slot("gLevelObjLists", edit.Room, 16), WriteObjectList(rom, edit.Templates, edit.Types, edit.TypesRest, ref cursor) + 0x08000000);
                List<byte> nine = edit.Nine.SelectMany(b => b).ToList();
                nine.AddRange(new byte[] { 0, 0, 0xFF, 0xFF });
                int nineAt = AllocateCompressed(rom, nine.ToArray(), ref cursor), doorsAt = Allocate(ref cursor, 8, 4);
                Array.Copy(rom, edit.DoorsStruct, rom, doorsAt, 8);
                WriteInt(rom, doorsAt, nineAt + 0x08000000);
                WriteInt(rom, Slot("gUnk_08D640A4", edit.Room, 18), doorsAt + 0x08000000);
            }
        }

        /* squeakFinish; The KatAM rooms' edits for Squeak Squad (its hub's grey door in Central Circle, the worlds' doors back in the rooms
         * their paths lead to, and the doors they move), made once the randomizer is done and after NiDL's (FinishRooms() runs it): the
         * props and tables where AddSqueakRooms() left them (squeakTables), each room as it is then; returns the cursor after; */
        static Func<byte[], int, int> squeakFinish;
        static int squeakProps;
        static Dictionary<string, int> squeakTables = new Dictionary<string, int>();

        // AddKatAMDoor(); A door in an edited room: its cell made a door (a plain one), or one an object shows (Slot: a 0x97 door, its block
        // record making the cell the door once the world slot is set, as KatAM's hub doors), its record, and its 9ROM entry;
        static void AddKatAMDoor(KatAMRoomEdit edit, Door door, int drawX = -1) {
            byte here = edit.Collision[door.Y * edit.Width + door.X];
            if (door.Slot == 0) edit.Collision[door.Y * edit.Width + door.X] = 0xFF;
            else {
                List<byte> block = new List<byte> { 1, here, (byte)door.X, (byte)door.Y, 0x12, 0, 0xFF, 0xFF };
                foreach (ushort entry in edit.Look(door.X, door.Y)) block.AddRange(BitConverter.GetBytes(entry));
                block.AddRange(new byte[] { 0xFF, 0xFF });
                edit.Specials.Add(block.ToArray());
            }
            edit.Specials.Add(DoorRecord(door, 0xFF));
            byte[] template = door.Slot != 0 ? DoorToHubTemplate(door.X, door.Y, edit.Templates.Count, door.Slot, door.Design) : MirrorTemplate(door.X, door.Y, edit.Templates.Count);
            if (drawX >= 0) WriteShort(template, 6, drawX);
            edit.Templates.Add(template);
            byte type = door.Slot != 0 ? DoorToHubType : MirrorType;
            if (!edit.Types.Contains(type)) edit.Types.Add(type);
            edit.Nine.Add(NineROMEntry(door.Room, door.DestX, door.DestY));
            edit.Changed = true;
        }

        /* MoveKatAMDoor(); A door of an edited room moved (its cell's collision and records, its 0x97 door object), and the doors of the
         * `landing` rooms landing on it (their records and 9ROM entries) landing on it there; */
        static void MoveKatAMDoor(KatAMRoomEdit edit, (int x, int y) from, (int x, int y) to, IEnumerable<KatAMRoomEdit> landing) {
            int w = edit.Width;
            (edit.Collision[to.y * w + to.x], edit.Collision[from.y * w + from.x]) = (edit.Collision[from.y * w + from.x], edit.Collision[to.y * w + to.x]);
            foreach (byte[] record in edit.Specials)
                if (record[0] is 1 or 3 && record[2] == from.x && record[3] == from.y) { record[2] = (byte)to.x; record[3] = (byte)to.y; }
            foreach (byte[] template in edit.Templates)
                if (template[12] == DoorToHubType && ReadShort(template, 6) / 16 == from.x && (ReadShort(template, 8) - 1) / 16 == from.y)
                    WriteShorts(template, 6, to.x * 16 + 8, (to.y + 1) * 16);
            var moved = (from.x, from.y, to.x, to.y);
            foreach (KatAMRoomEdit other in landing) {
                foreach (byte[] record in other.Specials)
                    if (record[0] == 3 && record.Length >= 12 && ReadShort(record, 8) == edit.Room && LandsOn(record[10], record[11], moved)) {
                        record[10] = (byte)(record[10] + to.x - from.x);
                        record[11] = (byte)(record[11] + to.y - from.y);
                        other.Changed = true;
                    }
                foreach (byte[] entry in other.Nine)
                    if (entry[0] == 2 && entry.Length >= 8 && ReadShort(entry, 4) == edit.Room && LandsOn(entry[6], entry[7], moved)) {
                        entry[6] = (byte)(entry[6] + to.x - from.x);
                        entry[7] = (byte)(entry[7] + to.y - from.y);
                        other.Changed = true;
                    }
            }
            edit.Changed = true;
        }

        /* SqueakSwitchTilemap(); A switch room's tilemap (SwitchWidth x SwitchHeight cells, `solid`, `thin`: its collision's kinds) in a
         * room's tiles (its tilemap, `columns` wide, and collision): each cell the look the room gives most often to a cell of the same kind
         * (solid, air, thin floor) with the same kinds around it, the 4 beside weighing 3, the corners 1 (outside a room: solid); returns
         * the tilemap and how well it matched (the sum of each cell's best score); */
        static (ushort[] tilemap, int score) SqueakSwitchTilemap(ushort[] tilemap, int columns, byte[] collision, int width, int height,
                                                                 Func<int, int, bool> solid, Func<int, int, bool> thin) {
            static int Kind(byte v) => v == KatAMAir ? 0 : v == KatAMSolid ? 1 : v is KatAMThin or KatAMFirmThin ? 2 : -1;
            (int dx, int dy, int weight)[] around = { (-1, 0, 3), (1, 0, 3), (0, -1, 3), (0, 1, 3), (-1, -1, 1), (1, -1, 1), (-1, 1, 1), (1, 1, 1) };
            int SourceKind(int x, int y) => x < 0 || y < 0 || x >= width || y >= height ? 1 : Kind(collision[y * width + x]);
            int TargetKind(int x, int y) => x < 0 || y < 0 || x >= SwitchWidth || y >= SwitchHeight || solid(x, y) ? 1 : thin(x, y) ? 2 : 0;
            ulong Key(ushort[] look) => (ulong)look[0] | (ulong)look[1] << 16 | (ulong)look[2] << 32 | (ulong)look[3] << 48;
            // (the room's cells by kind and the kinds around them: each look's count);
            var groups = new Dictionary<(int kind, int around), Dictionary<ulong, int>>();
            for (int y = 0; y < height; y++)
                for (int x = 0; x < width; x++) {
                    int kind = SourceKind(x, y);
                    if (kind < 0) continue;
                    int signature = 0;
                    foreach (var (dx, dy, _) in around) signature = signature * 4 + (SourceKind(x + dx, y + dy) + 1);
                    ushort[] look = { tilemap[2 * y * columns + 2 * x], tilemap[2 * y * columns + 2 * x + 1], tilemap[(2 * y + 1) * columns + 2 * x], tilemap[(2 * y + 1) * columns + 2 * x + 1] };
                    if (!groups.TryGetValue((kind, signature), out var looks)) groups[(kind, signature)] = looks = new Dictionary<ulong, int>();
                    looks[Key(look)] = looks.GetValueOrDefault(Key(look)) + 1;
                }
            ushort[] result = new ushort[SwitchWidth * 2 * SwitchHeight * 2];
            int total = 0;
            for (int y = 0; y < SwitchHeight; y++)
                for (int x = 0; x < SwitchWidth; x++) {
                    int kind = TargetKind(x, y);
                    int[] want = around.Select(a => TargetKind(x + a.dx, y + a.dy)).ToArray();
                    int bestScore = -1;
                    Dictionary<ulong, int> best = new Dictionary<ulong, int>();
                    foreach (var group in groups.Where(g => g.Key.kind == kind)) {
                        int score = 0;
                        for (int k = 0, s = group.Key.around; k < around.Length; k++) {
                            int have = (s >> (2 * (around.Length - 1 - k)) & 3) - 1;
                            if (have == want[k]) score += around[k].weight;
                        }
                        if (score < bestScore) continue;
                        if (score > bestScore) { bestScore = score; best = new Dictionary<ulong, int>(); }
                        foreach (var look in group.Value) best[look.Key] = best.GetValueOrDefault(look.Key) + look.Value;
                    }
                    total += Math.Max(bestScore, 0);
                    ulong chosen = best.Count == 0 ? 0 : best.OrderByDescending(l => l.Value).ThenBy(l => l.Key).First().Key;
                    for (int q = 0; q < 4; q++) result[(2 * y + q / 2) * SwitchWidth * 2 + 2 * x + q % 2] = (ushort)(chosen >> (16 * q));
                }
            return (result, total);
        }

        // SqueakLook; A Squeak Squad room's converted look, for the switch rooms drawn in its tiles: its foreground tilemap and collision,
        // and the table entries of its tiles, palettes and background;
        sealed class SqueakLook {
            public int World, Stage, Number, Room, Width, Height, Columns, Info, Palette, Background, BackgroundWidth, BackgroundHeight;
            public bool Fight;
            public ushort[] Tilemap;
            public byte[] Collision, Tiles;
            public string View;
        }

        /* SqueakSwitchDesigns; The switch rooms by eye (the owner's picks of the mock-ups, Open Requests), per world: the far view behind
         * it (the DS's map/d_viw_NN, any world's: null, its source room's), the room its tiles are drawn after (stage, room: null, the
         * world's room that gives its layout best), and its dressing: blocks of the source room's cells "x y sx sy w h" (the switch
         * room's cell, the source's top left cell, the block's size; ~ before sx: flipped left to right, the block's columns too; !
         * first: over the terrain too, its collision kept), put after the terrain; the open air cells a back wall fills (three
         * quarters of their pixels drawn or more) or touching no terrain are left clear, so the far view shows; */
        sealed record SqueakSwitchDesign(string View, (int stage, int number)? Source, string[] Dressing);
        static readonly Dictionary<int, SqueakSwitchDesign> SqueakSwitchDesigns = new Dictionary<int, SqueakSwitchDesign> {
            [1] = new SqueakSwitchDesign("00", null, new string[0]),
            [2] = new SqueakSwitchDesign("06", (6, 1), new string[0]),
            [3] = new SqueakSwitchDesign(null, null, new string[0]),
            [4] = new SqueakSwitchDesign("22", null, new string[0]),
            [5] = new SqueakSwitchDesign("25", null, new string[0]),
            [6] = new SqueakSwitchDesign("16", null, new string[0]),
            [7] = new SqueakSwitchDesign("18", null, new string[0]),
            [8] = new SqueakSwitchDesign("26", (3, 1), new string[0]),
        };

        /* BuildSqueakSwitchRooms(); The 8 switch rooms (from `first`, world by world; see the notes above): each drawn in the tiles of the
         * world's room whose cells best give the layout's (SqueakSwitchTilemap(); not a fight's room), its palettes and far view; its
         * doors (`doors`, by world: the floor mirrors PlanSqueakLevels() made, then the hub doors) and objects (the big switch on its
         * pedestal, the statues, a mirror on each floor door, a door object on each hub door, layout B's middle ones drawn at x 128);
         * the camera KatAM's (rooms.c's gNidlRoomTops 0: the room a screen, it stays put, as the NiDL switch rooms'); */
        static string BuildSqueakSwitchRooms(byte[] rom, int first, List<SqueakLook> looks, Dictionary<int, List<Door>> doors, int newProps,
                                             int newInfos, int roomInfos, int navigationRoutes, int noTypes, Func<string, int, int> add,
                                             Func<List<byte>, int, int> doorsList, Func<string, int[,]> farView, ref int cursor) {
            for (int world = 1; world <= SqueakWorlds; world++) {
                int room = first + world - 1, level = SqueakLayoutLevel(world);
                bool b = LayoutB(level);
                bool Pedestal(int x, int y) => SwitchPedestal(level, x, y);
                bool Solid(int x, int y) => y == 0 || y >= SwitchFloor || Pedestal(x, y) || (b
                    ? (y <= 3 && (x <= 1 || x >= SwitchWidth - 2)) || (y >= 4 && (x == 0 || x == SwitchWidth - 1)) || (y >= 7 && (x <= 3 || x >= SwitchWidth - 4))
                    : y <= 5 && (x <= 1 || x >= SwitchWidth - 2));
                // (its look: the design's room, else the world's room that gives it best);
                SqueakSwitchDesign design = SqueakSwitchDesigns.GetValueOrDefault(world, new SqueakSwitchDesign(null, null, new string[0]));
                SqueakLook source = null;
                ushort[] tilemap = null;
                int bestScore = -1;
                foreach (SqueakLook look in looks.Where(l => l.World == world && !l.Fight && (design.Source == null || (l.Stage, l.Number) == design.Source.Value))) {
                    (ushort[] map, int score) = SqueakSwitchTilemap(look.Tilemap, look.Columns, look.Collision, look.Width, look.Height, Solid, (x, y) => false);
                    if (score > bestScore) { bestScore = score; source = look; tilemap = map; }
                }
                if (source == null) return $"Squeak Squad's world {world} has no room to draw its switch room in.";
                // (the open air a back wall fills, clear; then the dressing);
                // (a back wall: three quarters of the cell's pixels drawn or more; a tuft of grass, a leaf, a lip, much less);
                int Drawn(ushort entry) {
                    int tile = entry & 0x3FF, count = 0;
                    if (tile * 32 + 32 > source.Tiles.Length) return 0;
                    for (int b = 0; b < 32; b++) count += ((source.Tiles[tile * 32 + b] & 0x0F) != 0 ? 1 : 0) + ((source.Tiles[tile * 32 + b] & 0xF0) != 0 ? 1 : 0);
                    return count;
                }
                int At(int x, int y, int q) => (2 * y + q / 2) * SwitchWidth * 2 + 2 * x + q % 2;
                // (and one touching no terrain: a lip or a tuft left floating, as Vocal Volcano's lines over its floor mirrors);
                bool Touches(int x, int y) => new[] { (x - 1, y), (x + 1, y), (x, y - 1), (x, y + 1) }
                    .Any(c => c.Item1 < 0 || c.Item2 < 0 || c.Item1 >= SwitchWidth || c.Item2 >= SwitchHeight || Solid(c.Item1, c.Item2));
                for (int y = 0; y < SwitchHeight; y++)
                    for (int x = 0; x < SwitchWidth; x++)
                        if (!Solid(x, y) && (Enumerable.Range(0, 4).Sum(q => Drawn(tilemap[At(x, y, q)])) >= 192 || !Touches(x, y)))
                            for (int q = 0; q < 4; q++) tilemap[At(x, y, q)] = 0;
                foreach (string block in design.Dressing) {
                    bool over = block.StartsWith("!");
                    string[] f = block.TrimStart('!').Split(' ');
                    bool flip = f[2].StartsWith("~");
                    int dx = int.Parse(f[0]), dy = int.Parse(f[1]), sx = int.Parse(f[2].TrimStart('~')), sy = int.Parse(f[3]), w = int.Parse(f[4]), h = int.Parse(f[5]);
                    for (int by = 0; by < h; by++)
                        for (int bx = 0; bx < w; bx++) {
                            int x = dx + bx, y = dy + by, cx = sx + (flip ? w - 1 - bx : bx), cy = sy + by;
                            if (x < 0 || y < 0 || x >= SwitchWidth || y >= SwitchHeight || cx >= source.Width || cy >= source.Height || Solid(x, y) && !over) continue;
                            for (int q = 0; q < 4; q++) {
                                int sq = flip ? q ^ 1 : q;
                                ushort entry = source.Tilemap[(2 * cy + sq / 2) * source.Columns + 2 * cx + sq % 2];
                                tilemap[At(x, y, q)] = flip ? (ushort)(entry ^ 0x400) : entry;
                            }
                        }
                }
                /* (its far view: the design's, as the mock-ups showed it: its middle 256 columns, its full height, 192 rows, at its own size,
                 * covering the room, so the camera shows the part the mock-up did; else the source room's, as converted for it); */
                int background = source.Background, backgroundWidth = source.BackgroundWidth, backgroundHeight = source.BackgroundHeight;
                if (design.View != null && farView($"map/d_viw_{design.View}.nsbtx") is int[,] picture) {
                    int pictureWidth = picture.GetLength(0), pictureHeight = Math.Min(picture.GetLength(1), SwitchHeight * 16);
                    double scale = (double)SwitchHeight * 16 / pictureHeight;
                    int cropWidth = Math.Min(pictureWidth, (int)Math.Round(SwitchWidth * 16 / scale)), left = (pictureWidth - cropWidth) / 2;
                    int[,] crop = new int[cropWidth, pictureHeight];
                    for (int y = 0; y < pictureHeight; y++)
                        for (int x = 0; x < cropWidth; x++) crop[x, y] = picture[left + x, y];
                    SqueakBackground view = ConvertSqueakFarView(crop, scale, SwitchHeight * 16);
                    int bgTiles = AllocateCompressed(rom, CompressLZ77(view.Tiles), ref cursor), bgColors = Allocate(ref cursor, 0x100, 4);
                    for (int c = 0; c < 128; c++) WriteShort(rom, bgColors + c * 2, view.Palettes[c]);
                    byte[] bgMap = new byte[view.Map.Length * 2];
                    for (int t = 0; t < view.Map.Length; t++) WriteShort(bgMap, t * 2, view.Map[t]);
                    int bgMapAt = AllocateCompressed(rom, bgMap, ref cursor), entry = Allocate(ref cursor, 0x20, 4);
                    WriteShorts(rom, entry, view.Width, view.Height);
                    WriteInts(rom, entry + 8, bgTiles + 0x08000000, view.Tiles.Length / 32, bgColors + 0x08000000);
                    WriteShorts(rom, entry + 0x14, 0x60, 0x80);
                    WriteInts(rom, entry + 0x18, bgMapAt + 0x08000000, room << 16);
                    background = add("gRoomTiledBGs", entry);
                    (backgroundWidth, backgroundHeight) = (view.Width * 8, view.Height * 8);
                }

                /* (padded as the NiDL switch rooms, Padded(): 13 rows, the last its ground's again, so KatAM's camera, hiding 8 pixels at the
                 * top and 40 at the bottom, shows the room as one screen and stays put: at 12 rows it had 144 pixels for its 160 and
                 * showed past the room's edge, the owner: "It's even showing out of bounds areas of the room"); */
                int rows = Padded(SwitchWidth, SwitchHeight).height;
                ushort[] padded = new ushort[SwitchWidth * 2 * rows * 2];
                for (int ty = 0; ty < rows * 2; ty++)
                    Array.Copy(tilemap, Math.Min(ty, SwitchHeight * 2 - 2 + ty % 2) * SwitchWidth * 2, padded, ty * SwitchWidth * 2, SwitchWidth * 2);
                byte[] collision = new byte[SwitchWidth * rows];
                for (int x = 0; x < SwitchWidth * (rows - SwitchHeight); x++) collision[SwitchHeight * SwitchWidth + x] = KatAMSolid;
                for (int y = 0; y < SwitchHeight; y++)
                    for (int x = 0; x < SwitchWidth; x++) collision[y * SwitchWidth + x] = Solid(x, y) ? KatAMSolid : KatAMAir;
                ushort[] Look(int x, int y) => new[] { tilemap[2 * y * SwitchWidth * 2 + 2 * x], tilemap[2 * y * SwitchWidth * 2 + 2 * x + 1],
                                                       tilemap[(2 * y + 1) * SwitchWidth * 2 + 2 * x], tilemap[(2 * y + 1) * SwitchWidth * 2 + 2 * x + 1] };

                // Its doors: the floor mirrors' cells doors, the hub doors' a block record the door object breaks once the slot is set;
                List<Door> list = doors.GetValueOrDefault(world, new List<Door>());
                List<byte> specials = new List<byte>(), nineROM = new List<byte>();
                foreach (Door door in list) {
                    if (door.Slot == 0) collision[door.Y * SwitchWidth + door.X] = 0xFF;
                    else {
                        specials.AddRange(new byte[] { 1, collision[door.Y * SwitchWidth + door.X], (byte)door.X, (byte)door.Y, 0x12, 0, 0xFF, 0xFF });
                        foreach (ushort entry in Look(door.X, door.Y)) specials.AddRange(BitConverter.GetBytes(entry));
                        specials.AddRange(new byte[] { 0xFF, 0xFF });
                    }
                    specials.AddRange(DoorRecord(door, 0xFF));
                    nineROM.AddRange(NineROMEntry(door.Room, door.DestX, door.DestY));
                }
                specials.AddRange(new byte[] { 0, 0, 0, 0, 0, 0, 0xFF, 0xFF });
                int collisionAt = AllocateCompressed(rom, CompressRL(collision), ref cursor), specialsAt = AllocateCompressed(rom, specials.ToArray(), ref cursor);
                int solidity = Allocate(ref cursor, 8, 4);
                WriteInts(rom, solidity, collisionAt + 0x08000000, specialsAt + 0x08000000);

                // Its objects: the big switch, the statues, the doors' mirrors and door objects;
                List<byte[]> templates = new List<byte[]> { BigSwitchTemplate(SwitchAt(level).x, SwitchAt(level).y, 0, SqueakSlot(world), NidlWorldMapDoors / 2 + world - 1) };
                List<byte> types = new List<byte> { BigSwitchType };
                for (int i = 0; i < 2; i++) {
                    (byte type, byte subtype) = SqueakStatues[world - 1][i];
                    byte[] statue = ObjectTemplate(SwitchStatueXs(level)[i], 48, templates.Count, type);
                    statue[14] = subtype; statue[15] = 0;
                    templates.Add(statue);
                    if (!types.Contains(type)) types.Add(type);
                }
                foreach (Door door in list) {
                    byte type = door.Slot != 0 ? DoorToHubType : MirrorType;
                    byte[] mirror = door.Slot != 0 ? DoorToHubTemplate(door.X, door.Y, templates.Count, door.Slot, door.Design) : MirrorTemplate(door.X, door.Y, templates.Count);
                    if (b && door.Slot != 0 && door.X == 8) WriteShort(mirror, 6, 128);
                    if (door.OneWay && door.Slot == 0) mirror[0x23] |= 1;
                    templates.Add(mirror);
                    if (!types.Contains(type)) types.Add(type);
                }
                int objects = WriteObjectList(rom, templates, types, noTypes, ref cursor);
                int navigation = BuildNavigation(rom, SwitchWidth, rows, false, collision, navigationRoutes, new List<(int, int, int, int)>(), ref cursor);
                int doorsAt = doorsList(nineROM, 0);

                // Its tilemap, and its props: the source room's tiles, palettes and far view, its own tilemap, collision, objects, doors;
                byte[] tilemapBytes = new byte[padded.Length * 2];
                for (int t = 0; t < padded.Length; t++) WriteShort(tilemapBytes, t * 2, padded[t]);
                int tilemapAt = AllocateCompressed(rom, CompressLZ77(tilemapBytes), ref cursor), tilemapEntry = Allocate(ref cursor, 0x20, 4);
                WriteShorts(rom, tilemapEntry, SwitchWidth * 2, rows * 2);
                WriteInts(rom, tilemapEntry + 0x18, tilemapAt + 0x08000000, room << 16);
                (int rateX, int rateY) = backgroundWidth > 0 ? BackgroundRates(backgroundWidth, backgroundHeight, SwitchWidth * 16, rows * 16) : (0, 0);
                int doorsIndex = add("gUnk_08D640A4", doorsAt);
                if (doorsIndex >= FirstDoorsList + VisitedLists) return "Too many doors lists for rooms.c's visited doors.";
                WriteShorts(rom, newProps + room * 0x28, LevelSongs[(world - 1) % LevelSongs.Length], 0, 0, 0x100, 0x100, 0, 0, rateX, rateY, RoomFlags,
                            source.Info, source.Palette, add("gForegroundTilemaps", tilemapEntry), add("gSolidityMaps", solidity), 0xFFFF,
                            background, add("gLevelObjLists", objects), add("gUnk_08D63C28", navigation), doorsIndex, 0);
                int roomInfo = Allocate(ref cursor, 0x4C, 4);
                Array.Copy(rom, ReadInt(rom, roomInfos + 0x397 * 4) - 0x08000000, rom, roomInfo, 0x4C);
                WriteShort(rom, roomInfo + 0x44, room);
                WriteInt(rom, newInfos + room * 4, roomInfo + 0x08000000);
                squeakSwitchLooks[world] = source.Room;
            }
            return null;
        }
        // (the room each world's switch room is drawn in: for the notes, the mock-ups);
        static Dictionary<int, int> squeakSwitchLooks = new Dictionary<int, int>();
        public static IReadOnlyDictionary<int, int> SqueakSwitchLooks => squeakSwitchLooks;
    }
}
