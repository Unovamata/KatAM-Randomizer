namespace KatAMRandomizer {
    /* Rooms: every room of Kirby: Nightmare in Dream Land, converted from the player's NiDL ROM into KatAM's free room slots (the
     * rooms whose gRoomProps entry is all 0xFFFF), with their foreground, background, collision, enemies, mini-bosses and items.
     * Doors, big switches, warp stars and bosses aren't ported yet: the rooms aren't reachable from KatAM's map.
     * NiDL rooms (USA addresses; the European and Japanese ROMs hold the same rooms, their table elsewhere: region.RoomTable):
     * • RoomTable: per level (0-7) 8 stage lists (RoomTable + level * 0x20 + stage * 4), then the levels' hub lists (+0x100 + level *
     *   4); each list holds room records, null where a room is missing, until a record whose index (+2) isn't its place.
     * • Room record: +8 cells (LZ77, width * height u32: metatile, attribute; the attribute's high byte is the collision), +0xC
     *   breakable block numbers (LZ77, u16 per cell), +0x10 object data, +0x14 u16 width, height (16x16 metatiles), +0x18 the
     *   foreground (palette: u16 byte count and colors, loaded at palette 2; LZ77 tiles at 0x06004000; LZ77 metatiles, 4 tilemap
     *   entries each), +0x28 the background (palette: u16 byte count and colors, at palette 8; LZ77 tiles; map: u16 byte count, u16
     *   width, height, then 0xFFFF and the LZ77 map in 64x32 screenblock order, or the map itself, row by row), +0x44 exits (12 bytes:
     *   room, 0, u16 tile x, y, landing x, y, 0), +0x48 objects (8 bytes: category + 1, index, variant, flags, u16 x, y) up to the
     *   record itself, +0x50 u16 x, y: where Kirby starts.
     * • Collision: high nibble = kind (0, 5, 7 solid; 1 one-way (0x10 a door); 2 breakable; 3 ladders (0x30, 0x31) and walls; 4 air
     *   currents; 6 spikes; 8, C water; 9 water with one-way floors; E water with spikes), low nibble = shape in NiDL's height maps
     *   (0x087328F0: empty, full, 45° floors, gentle floors, ceilings). Collision maps it to KatAM's values (gCollisionAttributes).
     * KatAM (per version, KatAMExpansionCode's RomSymbols): gRoomProps (0x28 bytes per room: song, ..., indexes into the tables below,
     * doorsIdx), gForegroundInfo (tiles at 0x06008000), gForegroundPalettes (6 palettes at 0), gForegroundTilemaps (RoomTiledBG: width,
     * height in tiles, LZ77 tilemap), gRoomTiledBGs (the background: tiles at 0x06000000, at most 900 of them as KatAM's HUD tiles
     * follow, palettes 6-13, its map row by row), gSolidityMaps (RL collision per 16x16 block, special blocks list), gLevelObjLists
     * (object templates), gUnk_08D63C28 (CPU helpers' zones), gUnk_08D6CD0C (per room: area, routes; written in place) and
     * gUnk_08D640A4 (doors per doorsIdx). The tables are copied with the new entries and every reference to them repointed. A room
     * can have at most 1950 blocks (KatAM's buffers for the collision, its zones and tilemap), so the 7 bigger NiDL rooms are split
     * in parts. Every new room uses doorsIdx 287, the one gVisitedDoors entry (0x120) no vanilla room uses; */
    internal static partial class KatAMExpansion {
        const int MaxRoomBlocks = 1950, BackgroundTiles = 900, SharedDoors = 287, RoomFlags = 0x79;

        // NiDL collision -> KatAM collision (see the notes above);
        static readonly byte[] RoomCollision = {
            0x00, 0x0D, 0x0B, 0x0C, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x01, 0x02, 0x03, 0x04, 0x00, 0x00,
            0x00, 0x0F, 0x1A, 0x1B, 0x14, 0x15, 0x16, 0x17, 0x00, 0x0E, 0x18, 0x19, 0x10, 0x11, 0x12, 0x13,
            0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D,
            0x20, 0x21, 0x21, 0x0D, 0x0D, 0x00, 0x00, 0x00, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x0C, 0x0B, 0x0C, 0x09, 0x0A, 0x09, 0x0A,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x0C, 0x05, 0x06, 0x07, 0x08, 0x00,
            0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x0D, 0x0B, 0x0C, 0x05, 0x06, 0x07, 0x08, 0x0D,
            0x0D, 0x0B, 0x0C, 0x05, 0x06, 0x07, 0x08, 0x0D, 0x0B, 0x0C, 0x05, 0x06, 0x07, 0x08, 0x0D, 0x0D,
            0x48, 0x0D, 0x38, 0x3A, 0x40, 0x41, 0x42, 0x43, 0x34, 0x36, 0x3C, 0x3D, 0x3E, 0x3F, 0x48, 0x48,
            0x48, 0x0F, 0x1A, 0x1B, 0x14, 0x15, 0x16, 0x17, 0x48, 0x0F, 0x1A, 0x1B, 0x14, 0x15, 0x16, 0x17,
            0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48,
            0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48,
            0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x38, 0x3A, 0x38, 0x3A, 0x34, 0x36, 0x34, 0x36,
            0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48,
            0x6D, 0x6D, 0x6D, 0x6D, 0x6D, 0x6D, 0x6D, 0x6D, 0x6D, 0x6D, 0x6D, 0x6D, 0x6D, 0x6D, 0x6D, 0x6D,
            0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48,
        };

        /* NiDL enemies (category 0, index = NiDL type - 8) -> KatAM object types: the ones in both games, and the ported ones (type 0x31,
         * the behavior's high nibble, NIDL_* in nidl.h; NiDL's variant in the low nibble). NiDL's 0x09 (a Waddle Dee with a headband) is
         * a Waddle Dee, 0x19 and 0x2A are both Shotzos; */
        static readonly Dictionary<int, byte> NativeEnemies = new Dictionary<int, byte> {
            [0] = 0x00, [1] = 0x00, [2] = 0x1E, [4] = 0x16, [5] = 0x15, [7] = 0x18, [9] = 0x12, [10] = 0x06, [12] = 0x01, [14] = 0x28,
            [17] = 0x35, [18] = 0x19, [19] = 0x1C, [20] = 0x04, [21] = 0x05, [24] = 0x14, [25] = 0x03, [27] = 0x02, [28] = 0x1A,
            [34] = 0x35, [35] = 0x13, [37] = 0x1B,
        };
        static readonly Dictionary<int, int> PortedEnemies = new Dictionary<int, int> {
            [3] = 0, [8] = 1, [16] = 2, [22] = 3, [26] = 4, [13] = 5, [15] = 6, [33] = 7, [23] = 8, [36] = 9, [29] = 10, [6] = 11,
            [11] = 12, [30] = 13, [31] = 13, [32] = 13, [39] = 14,
        };
        // NiDL mini-bosses (category 1) -> KatAM type and behavior (0: Bonkers, 3: Mr. Frosty, 7: Phan Phan; the rest ported);
        static readonly Dictionary<int, (byte type, byte behavior)> RoomMinibosses = new Dictionary<int, (byte, byte)> {
            [0] = (0x39, 0), [1] = (0x40, 0x00), [2] = (0x40, 0x20), [3] = (0x38, 0), [4] = (0x40, 0x10), [5] = (0x41, 0x30),
            [6] = (0x41, 0x40), [7] = (0x3A, 0), [8] = (0x42, 0x50), [9] = (0x42, 0x50),
        };
        // NiDL items (category 3): 1-Up, Maxim Tomato, Invincibility Candy, Energy Drink -> KatAM's 1UP, Maximum Tomato, Lollipop, Pep Brew;
        static readonly Dictionary<int, byte> RoomItems = new Dictionary<int, byte> { [1] = 0x63, [2] = 0x61, [3] = 0x64, [4] = 0x5F };
        // KatAM song per NiDL level (Vegetable Valley ... Rainbow Resort, then the Fountain of Dreams);
        static readonly int[] LevelSongs = { 5, 8, 9, 3, 6, 8, 10, 3 };

        /* NightmareRoom; A NiDL room: level, stage (-1 for a level's hub list), index, record (file offset); its part when split
         * (X, Y, Width, Height in metatiles), and the KatAM room it becomes; */
        public sealed record NightmareRoom(int Level, int Stage, int Index, int Record, int X, int Y, int Width, int Height) {
            public int Room { get; set; }
        }

        public static IReadOnlyList<NightmareRoom> PortedRooms { get; private set; } = new List<NightmareRoom>();

        // NightmareRooms(); NiDL's rooms, split where KatAM can't hold them whole; the 2x2 placeholder hubs left out;
        static List<NightmareRoom> NightmareRooms(byte[] nightmare, int table) {
            List<(int level, int stage, int index, int record)> records = new List<(int, int, int, int)>();

            for (int level = 0; level < 8; level++) {
                for (int stage = -1; stage < 8; stage++) {
                    int list = ReadInt(nightmare, stage < 0 ? table + 0x100 + level * 4 : table + level * 0x20 + stage * 4);
                    if (stage >= 0 && list == 0) continue;
                    list -= 0x08000000;

                    for (int index = 0; index < 80; index++) {
                        int record = ReadInt(nightmare, list + index * 4);
                        if (record == 0 && stage >= 0) continue;
                        if ((record >> 24) != 8 || ReadShort(nightmare, (record & 0x1FFFFFF) + 2) != index) break;
                        records.Add((level, stage, index, record & 0x1FFFFFF));
                    }
                }
            }

            List<NightmareRoom> rooms = new List<NightmareRoom>();
            foreach ((int level, int stage, int index, int record) in records) {
                int width = ReadShort(nightmare, record + 0x14), height = ReadShort(nightmare, record + 0x16);
                if (width <= 2 && height <= 2) continue;

                // Split along the longer side into the fewest parts that fit;
                int parts = (width * height + MaxRoomBlocks - 1) / MaxRoomBlocks;
                bool across = width >= height;
                int length = across ? width : height;
                for (int part = 0; part < parts; part++) {
                    int start = length * part / parts, end = length * (part + 1) / parts;
                    rooms.Add(across ? new NightmareRoom(level, stage, index, record, start, 0, end - start, height)
                                     : new NightmareRoom(level, stage, index, record, 0, start, width, end - start));
                }
            }
            return rooms;
        }

        /* AddRooms(); Every NiDL room into KatAM's free room slots (see the notes above); the room records' foregrounds, palettes and
         * backgrounds are shared as in NiDL; */
        static string AddRooms(byte[] rom, byte[] nightmare, NightmareRegion region, KatAMExpansionCode.Version version, ref int cursor) {
            IReadOnlyDictionary<string, int> symbols = version.RomSymbols;
            int roomProps = symbols["gRoomProps"], roomInfos = symbols["gUnk_08D6CD0C"];
            List<NightmareRoom> rooms;

            try {
                rooms = NightmareRooms(nightmare, region.RoomTable);
            } catch (ArgumentException) {
                return $"The Nightmare in Dream Land ROM doesn't match the {region.KatAM} release.";
            }

            // The vanilla rooms: their tables' sizes, the free slots, and what new rooms borrow (all from rooms every version has);
            const int RoomCount = 0x3D7;
            List<int> free = new List<int>();
            int[] tableSizes = new int[20];
            for (int room = 1; room < RoomCount; room++) {
                int props = roomProps + room * 0x28;
                if (Enumerable.Range(0, 19).All(i => ReadShort(rom, props + i * 2) == 0xFFFF)) { free.Add(room); continue; }
                for (int field = 10; field < 19; field++) {
                    int index = ReadShort(rom, props + field * 2);
                    if (index != 0xFFFF) tableSizes[field] = Math.Max(tableSizes[field], index + 1);
                }
            }
            if (free.Count < rooms.Count) return "KatAM has too few free rooms for Nightmare in Dream Land's.";

            int Props(int room, int field) => ReadShort(rom, roomProps + room * 0x28 + field * 2);
            int Entry(string table, int index) => ReadInt(rom, symbols[table] + index * 4) - 0x08000000;
            int paletteCycles = ReadInt(rom, Entry("gForegroundPalettes", Props(0x323, 11)) + 4),        // Unused (no cycles), but read;
                navigationRoutes = Entry("gUnk_08D63C28", Props(0x3C9, 17)),                                  // Its 4 generic route lists;
                noRoutes = ReadInt(rom, ReadInt(rom, roomInfos + 0x397 * 4) - 0x08000000);                    // A list with no routes;

            List<int> infos = new List<int>(), palettes = new List<int>(), tilemaps = new List<int>(), backgrounds = new List<int>(),
                      solidities = new List<int>(), objects = new List<int>(), navigations = new List<int>();
            Dictionary<int, int> infoOf = new Dictionary<int, int>(), paletteOf = new Dictionary<int, int>(), backgroundOf = new Dictionary<int, int>();
            int noSpecials = Allocate(ref cursor, 8, 4);
            WriteInts(rom, noSpecials, 0, unchecked((int)0xFFFF0000));
            int noTypes = Allocate(ref cursor, 4, 4);
            WriteInt(rom, noTypes, 0xFF);

            try {
                for (int i = 0; i < rooms.Count; i++) {
                    NightmareRoom r = rooms[i];
                    r.Room = free[i];
                    int record = r.Record, roomWidth = ReadShort(nightmare, record + 0x14);
                    int Address(int offset) => ReadInt(nightmare, record + offset) - 0x08000000;
                    byte[] cells = DecompressLZ77(nightmare, Address(8));

                    // Foreground tiles (NiDL's own LZ77) and palette (NiDL's palettes 2-7, KatAM's 0-5), shared;
                    int tilesAddress = Address(0x1C), paletteAddress = Address(0x18);
                    if (!infoOf.TryGetValue(tilesAddress, out int info)) {
                        int tiles = CopyLZ77(rom, nightmare, tilesAddress, ref cursor);
                        int at = Allocate(ref cursor, 16, 4);
                        WriteInts(rom, at, tiles, tiles, tiles);
                        WriteShorts(rom, at + 12, 0, r.Room);
                        infoOf[tilesAddress] = info = tableSizes[10] + infos.Count;
                        infos.Add(at + 0x08000000);
                    }
                    if (!paletteOf.TryGetValue(paletteAddress, out int palette)) {
                        int colors = Allocate(ref cursor, 0xC0, 4);
                        Array.Copy(nightmare, paletteAddress + 2, rom, colors, 0xC0);
                        int at = Allocate(ref cursor, 12, 4);
                        WriteInts(rom, at, colors + 0x08000000, paletteCycles);
                        WriteShorts(rom, at + 8, 0, r.Room);
                        paletteOf[paletteAddress] = palette = tableSizes[11] + palettes.Count;
                        palettes.Add(at + 0x08000000);
                    }

                    // Foreground tilemap: each metatile's 4 entries, palettes down by 2;
                    byte[] metatiles = DecompressLZ77(nightmare, Address(0x20));
                    int width = r.Width * 2, height = r.Height * 2;
                    byte[] tilemap = new byte[width * height * 2];
                    for (int y = 0; y < height; y++) {
                        for (int x = 0; x < width; x++) {
                            int cell = (r.Y + y / 2) * roomWidth + r.X + x / 2;
                            int entry = ReadShort(metatiles, ReadShort(cells, cell * 4) * 8 + ((y & 1) * 2 + (x & 1)) * 2);
                            WriteShort(tilemap, (y * width + x) * 2, ShiftPalette(entry));
                        }
                    }
                    byte[] compressedTilemap = CompressLZ77(tilemap);
                    int tilemapAt = Allocate(ref cursor, compressedTilemap.Length, 4);
                    compressedTilemap.CopyTo(rom, tilemapAt);
                    int tilemapEntry = Allocate(ref cursor, 0x20, 4);
                    WriteShorts(rom, tilemapEntry, width, height);
                    WriteInts(rom, tilemapEntry + 0x18, tilemapAt + 0x08000000, r.Room << 16);
                    tilemaps.Add(tilemapEntry + 0x08000000);

                    // Background, shared;
                    int backgroundMap = Address(0x30);
                    if (!backgroundOf.TryGetValue(backgroundMap, out int background)) {
                        int at = BuildBackground(rom, nightmare, Address(0x2C), Address(0x28), backgroundMap, r.Room, ref cursor);
                        backgroundOf[backgroundMap] = background = tableSizes[15] + backgrounds.Count;
                        backgrounds.Add(at + 0x08000000);
                    }

                    // Collision, and a special block record per breakable block (what's left after: NiDL's metatile for the block's number);
                    byte[] collision = new byte[r.Width * r.Height];
                    for (int y = 0; y < r.Height; y++)
                        for (int x = 0; x < r.Width; x++) collision[y * r.Width + x] = RoomCollision[cells[((r.Y + y) * roomWidth + r.X + x) * 4 + 3]];
                    int solidity = BuildSolidity(rom, nightmare, r, cells, collision, metatiles, noSpecials, ref cursor);
                    solidities.Add(solidity + 0x08000000);

                    objects.Add(BuildObjects(rom, nightmare, r, noTypes, ref cursor) + 0x08000000);
                    navigations.Add(BuildNavigation(rom, r, navigationRoutes, ref cursor) + 0x08000000);

                    // Area and routes (a room of its own, like room 0x397), written into the vanilla table's free entry;
                    int roomInfo = Allocate(ref cursor, 0x4C, 4);
                    WriteInt(rom, roomInfo, noRoutes);
                    WriteShort(rom, roomInfo + 0x44, r.Room);
                    rom[roomInfo + 0x46] = 0x0A;
                    WriteInt(rom, roomInfos + r.Room * 4, roomInfo + 0x08000000);

                    // The background scrolls as in NiDL: its rates (+0x34, x and y in 16ths) as KatAM's (8.8), bottom-aligned;
                    WriteShorts(rom, roomProps + r.Room * 0x28, LevelSongs[r.Level], 0, 0, 0x100, 0x100, 0, 0,
                                ReadShort(nightmare, record + 0x34) * 16, ReadShort(nightmare, record + 0x36) * 16, RoomFlags,
                                info, palette, tableSizes[12] + i, tableSizes[13] + i, 0xFFFF, background, tableSizes[16] + i,
                                tableSizes[17] + i, SharedDoors, 0);
                }
            } catch (Exception exception) when (exception is IndexOutOfRangeException or ArgumentException) {
                return $"The Nightmare in Dream Land ROM doesn't match the {region.KatAM} release.";
            }

            // The tables, copied with the new entries; doors: entry 287 (no doors);
            int doorless = Allocate(ref cursor, 8, 4);
            WriteInts(rom, doorless, noRoutes, 0);
            (string table, int size, List<int> entries)[] tables = {
                ("gForegroundInfo", tableSizes[10], infos), ("gForegroundPalettes", tableSizes[11], palettes),
                ("gForegroundTilemaps", tableSizes[12], tilemaps), ("gSolidityMaps", tableSizes[13], solidities),
                ("gRoomTiledBGs", tableSizes[15], backgrounds), ("gLevelObjLists", tableSizes[16], objects),
                ("gUnk_08D63C28", tableSizes[17], navigations), ("gUnk_08D640A4", SharedDoors, new List<int> { doorless + 0x08000000 }),
            };
            foreach ((string table, int size, List<int> entries) in tables) {
                int at = Allocate(ref cursor, (size + entries.Count) * 4, 4);
                Array.Copy(rom, symbols[table], rom, at, size * 4);
                WriteInts(rom, at + size * 4, entries.ToArray());
                Repoint(rom, symbols[table] + 0x08000000, at + 0x08000000);
            }

            PortedRooms = rooms;
            return null;
        }

        /* Breakable blocks (NiDL collision 0x2X) -> KatAM's: star blocks 0xF0, metal 0xF4 (0x2D), bomb blocks 0xF1 (0x2E) and the blocks
         * a bomb block breaks 0xF2 (0x2F); */
        static byte BreakableBlock(int collision) => collision switch { 0x2D => 0xF4, 0x2E => 0xF1, 0x2F => 0xF2, _ => 0xF0 };

        /* BuildSolidity(); The room's (part's) SolidityMap: its RL collision and its special blocks: per breakable block, its KatAM
         * value, where it is, and what it leaves when broken (the 4 tilemap entries of the metatile NiDL's object data lists for the
         * block's number, nothing or water); returns its address; */
        static int BuildSolidity(byte[] rom, byte[] nightmare, NightmareRoom r, byte[] cells, byte[] collision, byte[] metatiles,
                                 int noSpecials, ref int cursor) {
            int roomWidth = ReadShort(nightmare, r.Record + 0x14), data = ReadInt(nightmare, r.Record + 0x10) - 0x08000000;
            byte[] numbers = DecompressLZ77(nightmare, ReadInt(nightmare, r.Record + 0xC) - 0x08000000);
            List<byte> specials = new List<byte>();
            bool Water(int x, int y) => x >= 0 && y >= 0 && x < r.Width && y < r.Height && collision[y * r.Width + x] is >= 0x34 and <= 0x48;

            for (int y = 0; y < r.Height && y < 256; y++) {
                for (int x = 0; x < r.Width && x < 256; x++) {
                    int cell = (r.Y + y) * roomWidth + r.X + x, kind = cells[cell * 4 + 3];
                    if (kind >> 4 != 2) continue;

                    int number = ReadShort(numbers, cell * 2), metatile = 0;
                    // The numbers' metatiles: u32 each from the object data, up to 0xFFFFFFFF;
                    for (int n = 0; data >= 0 && n <= number; n++) {
                        int value = ReadInt(nightmare, data + n * 4);
                        if (value == -1) { metatile = 0; break; }
                        if (n == number) metatile = value & 0xFFFF;
                    }
                    byte left = (byte)(Water(x - 1, y) || Water(x + 1, y) || Water(x, y - 1) || Water(x, y + 1) ? 0x48 : 0);
                    byte block = BreakableBlock(kind);
                    collision[y * r.Width + x] = block;
                    specials.AddRange(new byte[] { 1, block, (byte)x, (byte)y, 0x12, 0, 0xFF, 0xFF });
                    for (int q = 0; q < 4; q++) specials.AddRange(BitConverter.GetBytes((ushort)ShiftPalette(ReadShort(metatiles, metatile * 8 + q * 2))));
                    specials.AddRange(new byte[] { left, 0xFF });
                }
            }

            byte[] compressed = CompressRL(collision);
            int collisionAt = Allocate(ref cursor, compressed.Length, 4);
            compressed.CopyTo(rom, collisionAt);
            int specialsAt = noSpecials;
            if (specials.Count > 0) {
                specials.AddRange(new byte[] { 0, 0, 0, 0, 0, 0, 0xFF, 0xFF });
                specialsAt = Allocate(ref cursor, specials.Count, 4);
                specials.ToArray().CopyTo(rom, specialsAt);
            }
            int at = Allocate(ref cursor, 8, 4);
            WriteInts(rom, at, collisionAt + 0x08000000, specialsAt + 0x08000000);
            return at;
        }

        // ShiftPalette(); A NiDL tilemap entry with its palette moved down by 2 (NiDL's level palettes start at 2, KatAM's at 0);
        static int ShiftPalette(int entry) => (entry & 0xFFF) | (Math.Max((entry >> 12) - 2, 0) << 12);

        // CopyLZ77(); NiDL's LZ77 data as it is (it's VRAM safe: NiDL decompresses it to VRAM too);
        static int CopyLZ77(byte[] rom, byte[] nightmare, int address, ref int cursor) {
            int length = CompressedLengthLZ77(nightmare, address);
            int at = Allocate(ref cursor, length, 4);
            Array.Copy(nightmare, address, rom, at, length);
            return at + 0x08000000;
        }

        static int CompressedLengthLZ77(byte[] source, int address) {
            int size = ReadInt(source, address) >> 8, position = address + 4;

            for (int written = 0; written < size;) {
                byte flags = source[position++];
                for (int bit = 0; bit < 8 && written < size; bit++) {
                    if ((flags & (0x80 >> bit)) == 0) { position++; written++; continue; }
                    written += (source[position] >> 4) + 3;
                    position += 2;
                }
            }
            return position - address;
        }

        /* BuildBackground(); A KatAM RoomTiledBG for a NiDL background: only the tiles its map uses (renumbered, at most
         * BackgroundTiles), palettes 8-15 as KatAM's 6-13, the map row by row; returns its address; */
        static int BuildBackground(byte[] rom, byte[] nightmare, int tilesAddress, int paletteAddress, int map, int room, ref int cursor) {
            byte[] tiles = DecompressLZ77(nightmare, tilesAddress);
            int width = ReadShort(nightmare, map + 2), height = ReadShort(nightmare, map + 4);
            bool compressed = ReadShort(nightmare, map + 6) == 0xFFFF;
            byte[] source = compressed ? DecompressLZ77(nightmare, map + 8) : null;

            Dictionary<int, int> renumbered = new Dictionary<int, int>();
            List<byte> usedTiles = new List<byte>();
            byte[] entries = new byte[width * height * 2];
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    int entry = compressed ? ReadShort(source, ((x >> 5) * 1024 + y * 32 + (x & 31)) * 2) : ReadShort(nightmare, map + 6 + (y * width + x) * 2);
                    int tile = entry & 0x3FF;
                    if (!renumbered.TryGetValue(tile, out int number)) {
                        // Past KatAM's limit, tiles share the last one (none of NiDL's backgrounds but one need it);
                        number = renumbered.Count < BackgroundTiles ? renumbered.Count : BackgroundTiles - 1;
                        if (renumbered.Count < BackgroundTiles) usedTiles.AddRange(tile * 32 + 32 <= tiles.Length ? tiles.Skip(tile * 32).Take(32) : new byte[32]);
                        renumbered[tile] = number;
                    }
                    WriteShort(entries, (y * width + x) * 2, ShiftPalette((entry & 0xFC00) | number));
                }
            }

            byte[] compressedTiles = CompressLZ77(usedTiles.ToArray());
            int tilesAt = Allocate(ref cursor, compressedTiles.Length, 4);
            compressedTiles.CopyTo(rom, tilesAt);
            int colors = Allocate(ref cursor, 0x100, 4);
            Array.Copy(nightmare, paletteAddress + 2, rom, colors, Math.Min(0x100, ReadShort(nightmare, paletteAddress)));
            int mapAt = Allocate(ref cursor, entries.Length, 4);
            entries.CopyTo(rom, mapAt);

            int at = Allocate(ref cursor, 0x20, 4);
            WriteShorts(rom, at, width, height);
            WriteInts(rom, at + 8, tilesAt + 0x08000000, usedTiles.Count / 32, colors + 0x08000000);
            WriteShorts(rom, at + 0x14, 0x60, 0x80);
            WriteInts(rom, at + 0x18, mapAt + 0x08000000, room << 16);
            return at;
        }

        /* BuildObjects(); The room's (part's) enemies, mini-bosses and items as KatAM object templates (LevelInfo_1E4: templates, the
         * types in the room, an empty list, how many types); returns its address; */
        static int BuildObjects(byte[] rom, byte[] nightmare, NightmareRoom r, int noTypes, ref int cursor) {
            List<byte[]> templates = new List<byte[]>();
            List<byte> types = new List<byte>();
            int list = ReadInt(nightmare, r.Record + 0x48);

            for (int at = list == 0 ? r.Record : list - 0x08000000; at + 8 <= r.Record; at += 8) {
                int category = nightmare[at] - 1, index = nightmare[at + 1], variant = nightmare[at + 2];
                int x = ReadShort(nightmare, at + 4) - r.X * 16, y = ReadShort(nightmare, at + 6) - r.Y * 16;
                if (x < 0 || y < 0 || x >= r.Width * 16 || y >= r.Height * 16) continue;

                byte type, behavior = 0;
                bool miniboss = false;
                if (category == 0 && NativeEnemies.TryGetValue(index, out type)) { }
                else if (category == 0 && PortedEnemies.TryGetValue(index, out int ported)) {
                    type = NightmareID;
                    // Poppy Bros. Jr. on an apple or with a bomb: their variants in poppy_bros_jr.c;
                    if (index == 31) variant = variant == 0 ? 3 : variant == 1 ? 7 : 5;
                    else if (index == 32) variant = variant == 0 ? 4 : variant == 1 ? 8 : 6;
                    behavior = (byte)(ported << 4 | (variant & 0xF));
                } else if (category == 1 && RoomMinibosses.TryGetValue(index, out (byte type, byte behavior) boss)) {
                    (type, behavior) = boss;
                    miniboss = true;
                } else if (category == 3 && RoomItems.TryGetValue(index, out type)) { }
                else continue;

                byte[] template = new byte[36];
                template[0] = 0x01; template[1] = 0x24; template[3] = (byte)templates.Count;
                WriteShorts(template, 6, x, y);
                template[10] = template[11] = template[13] = 0xFF;
                template[12] = type; template[14] = behavior;
                // A mini-boss fights in the whole room (NiDL's are the size of their arena);
                if (miniboss) WriteShorts(template, 0x1A, -x, -y, r.Width * 16, r.Height * 16);
                templates.Add(template);
                if (!types.Contains(type)) types.Add(type);
            }

            int templatesAt = Allocate(ref cursor, templates.Count * 36 + 36, 4);
            for (int i = 0; i < templates.Count; i++) templates[i].CopyTo(rom, templatesAt + i * 36);
            Array.Fill(rom, (byte)0xFF, templatesAt + templates.Count * 36, 36);
            rom[templatesAt + templates.Count * 36] = rom[templatesAt + templates.Count * 36 + 1] = 0;
            int typesAt = Allocate(ref cursor, types.Count + 1, 4);
            types.Append((byte)0xFF).ToArray().CopyTo(rom, typesAt);

            int objectList = Allocate(ref cursor, 16, 4);
            WriteInts(rom, objectList, templatesAt + 0x08000000, typesAt + 0x08000000, noTypes + 0x08000000);
            rom[objectList + 12] = (byte)types.Count;
            return objectList;
        }

        /* BuildNavigation(); The CPU helpers' zones (LevelInfo_1E8): the whole room is zone 0, its point the room's center; the route
         * lists are room 0x3C9's (generic); returns its address; */
        static int BuildNavigation(byte[] rom, NightmareRoom r, int routes, ref int cursor) {
            byte[] zones = CompressRL(new byte[r.Width * r.Height]);
            int zonesAt = Allocate(ref cursor, zones.Length, 4);
            zones.CopyTo(rom, zonesAt);

            int zone = Allocate(ref cursor, 0x34, 4);
            Array.Fill(rom, (byte)0xFF, zone, 0x34);
            rom[zone] = 0;
            WriteShorts(rom, zone + 0x20, r.Width * 8, r.Height * 8, 0x20);
            new byte[] { 0, 0, 0, 1, 1 }.CopyTo(rom, zone + 0x26);
            WriteShort(rom, zone + 0x32, 0);

            int at = Allocate(ref cursor, 0x1C, 4);
            Array.Copy(rom, routes, rom, at, 16);
            WriteInts(rom, at + 16, zonesAt + 0x08000000, zone + 0x08000000, 1);
            return at;
        }

        // CompressLZ77(); GBA BIOS LZ77 (type 0x10), VRAM safe (never copies from 1 byte back);
        static byte[] CompressLZ77(byte[] data) {
            List<byte> output = new List<byte> { 0x10, (byte)data.Length, (byte)(data.Length >> 8), (byte)(data.Length >> 16) };
            Dictionary<int, List<int>> heads = new Dictionary<int, List<int>>();
            int Key(int i) => i + 2 < data.Length ? data[i] | data[i + 1] << 8 | data[i + 2] << 16 : -1;
            void Add(int i) {
                int key = Key(i);
                if (key < 0) return;
                if (!heads.TryGetValue(key, out List<int> list)) heads[key] = list = new List<int>();
                list.Add(i);
            }

            for (int i = 0; i < data.Length;) {
                int flagsAt = output.Count;
                output.Add(0);
                for (int bit = 0; bit < 8 && i < data.Length; bit++) {
                    int bestLength = 0, bestDistance = 0;
                    if (heads.TryGetValue(Key(i), out List<int> candidates)) {
                        for (int c = candidates.Count - 1, tried = 0; c >= 0 && tried < 64; c--, tried++) {
                            int j = candidates[c], distance = i - j;
                            if (distance > 0x1000) break;
                            if (distance < 2) continue;
                            int length = 0;
                            while (length < 18 && i + length < data.Length && data[j + length] == data[i + length]) length++;
                            if (length > bestLength) { bestLength = length; bestDistance = distance; }
                            if (length == 18) break;
                        }
                    }
                    if (bestLength >= 3) {
                        output[flagsAt] |= (byte)(0x80 >> bit);
                        int code = (bestLength - 3) << 12 | (bestDistance - 1);
                        output.Add((byte)(code >> 8));
                        output.Add((byte)code);
                        for (int k = 0; k < bestLength; k++) Add(i + k);
                        i += bestLength;
                    } else {
                        output.Add(data[i]);
                        Add(i);
                        i++;
                    }
                }
            }
            while (output.Count % 4 != 0) output.Add(0);
            return output.ToArray();
        }

        // CompressRL(); GBA BIOS run-length (type 0x30);
        static byte[] CompressRL(byte[] data) {
            List<byte> output = new List<byte> { 0x30, (byte)data.Length, (byte)(data.Length >> 8), (byte)(data.Length >> 16) };
            List<byte> literal = new List<byte>();
            void Flush() {
                for (int i = 0; i < literal.Count; i += 128) {
                    int count = Math.Min(128, literal.Count - i);
                    output.Add((byte)(count - 1));
                    output.AddRange(literal.Skip(i).Take(count));
                }
                literal.Clear();
            }

            for (int i = 0; i < data.Length;) {
                int run = 1;
                while (i + run < data.Length && data[i + run] == data[i] && run < 130) run++;
                if (run >= 3) {
                    Flush();
                    output.Add((byte)(0x80 | (run - 3)));
                    output.Add(data[i]);
                    i += run;
                } else {
                    literal.Add(data[i++]);
                }
            }
            Flush();
            while (output.Count % 4 != 0) output.Add(0);
            return output.ToArray();
        }
    }
}
