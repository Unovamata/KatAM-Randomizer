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

        /* Areas: each NiDL level is an area of its own, as KatAM's (gUnk_08D6CD0C +0x46: 0-8 KatAM's, 9 the tutorial, 10 the Dimension
         * Mirror), AreaBase + level (the NiDL hub with the Fountain of Dreams), and its rooms (hub first) take the free room numbers of
         * one hundred (LevelHundreds; KatAM numbers its rooms by hundreds too, mostly by area): the NiDL hub and the Fountain of Dreams
         * 1-99, Vegetable Valley 100-199 ... Rainbow Resort 700-799. The HUD shows the area's plate (its name, AreaPlates) in its corner,
         * and the CPU helpers' routes are per area (gUnk_08D6DCAC); both only had areas 0-10: see ExtendAreas(); */
        const int AreaBase = 11, MapRoomsBlock = MapTableAddress + 0xE0;
        static readonly int[] LevelHundreds = { 1, 2, 3, 4, 5, 6, 7, 0 };
        static readonly (string first, string second)[] LevelNames = {
            ("VEGETABLE", "VALLEY"), ("ICE CREAM", "ISLAND"), ("BUTTER", "BUILDING"), ("GRAPE", "GARDEN"), ("YOGURT", "YARD"),
            ("ORANGE", "OCEAN"), ("RAINBOW", "RESORT"), ("FOUNTAIN OF", "DREAMS"),
        };

        /* Big switches: one per level, in its switch room (see SwitchRooms; NiDL's own aren't ported), as KatAM's areas: KatAM's
         * object 0x86, setting the level's world state slot (template +4, saved: gUnk_02028C10, 0x20 of them; KatAM's big switches use
         * slots 1-15, Central Circle's last mirror 0x12, Dark Meta Knight 0x13), which lights the level's mirror in the NiDL hub and the
         * one its switch room shows above the switch (object 0x8C), and its door on the world map (KatAMExpansionWorldMap.cs);
         * LevelLinks: where that mirror leads, as its path on the world map (worldmap.c's gNidlWorldMapDoors): a KatAM switch room (onto
         * its floor, left of the pedestal: SwitchLanding, so Kirby doesn't land on its big switch), another level's switch room (Nidl(level),
         * onto its SwitchArrival) or the NiDL hub (Hub, onto the level's door there). Chains of at most 2 links, a few to KatAM rooms:
         * Vegetable Valley to Rainbow Route East (0x316), Ice Cream Island to the Moonlight Mansion door (0xBE), Butter Building to Rainbow
         * Resort to the hub, Grape Garden to Peppermint Palace East (0x122), Yogurt Yard to Cabbage Cavern West (0x250), Orange Ocean and
         * the Fountain of Dreams to Cabbage Cavern East (0xBF) (the KatAM rooms of KatAM's world map doors: sWorldMapDoorRoomIds);
         * Door designs (object 0x97's subtype, its frame: CreateDoorToHub; KatAM shows the design of where a door leads, KatAMDesigns):
         * each level has one KatAM's Central Circle doesn't (NidlDesigns), for its door in the NiDL hub and the doors leading to it; */
        static int LevelSlot(int level) => 0x14 + level;
        const int Hub = -1;
        static int Nidl(int level) => -2 - level;
        static readonly int[] LevelLinks = { 0x316, 0xBE, Nidl(6), 0x122, 0x250, 0xBF, Hub, 0xBF };
        static readonly (int x, int y) SwitchLanding = (5, 9);
        static readonly int[] NidlDesigns = { 11, 3, 13, 7, 5, 2, 6, 1 };
        static readonly Dictionary<int, int> KatAMDesigns = new Dictionary<int, int> {
            [0x316] = 14, [0x2B2] = 5, [0x24F] = 7, [0xC1] = 13, [0x123] = 6, [0x37A] = 11, [0x186] = 2, [0x122] = 15, [0xC0] = 8, [0xC2] = 10,
            [0x250] = 4, [0xBF] = 12, [0x24E] = 16, [0x1EA] = 0, [0xBE] = 9,
        };
        const int HubDesign = 17;
        static int LinkDesign(int link) => link == Hub ? HubDesign : link < 0 ? NidlDesigns[-2 - link] : KatAMDesigns.GetValueOrDefault(link, HubDesign);
        const byte BigSwitchType = 0x86, HubMirrorType = 0x8C;

        /* Switch rooms: a level's entry, as KatAM's switch rooms (0x24F, 0x316, 0xC1...: one screen walled on top, a pedestal in the
         * middle with the big switch on it, a mirror on each side's floor) in the level's tiles (see BuildSwitchRooms()): its 2 mirrors
         * (SwitchSides) lead to the level's first stages (the others follow them), Kirby arrives at SwitchArrival,
         * and the mirror above the switch (SwitchTop) shows once it's pressed (LevelLinks); */
        const int SwitchStage = 8, SwitchWidth = 16, SwitchHeight = 10, SwitchX = 136, SwitchY = 128;
        static readonly (int x, int y)[] SwitchSides = { (1, 8), (14, 8) };
        static readonly (int x, int y) SwitchArrival = (4, 8), SwitchTop = (8, 3);

        /* The NiDL hub (see KatAMExpansionHub.cs): Central Circle again, at night; its golden mirror (MainHubReturn) back to Central
         * Circle's twin; each level has one of Central Circle's own doors (MainHubDoors: its door cells), recolored for the level's
         * theme, and hidden until the level's big switch is pressed (but Vegetable Valley's, the way in): Vegetable Valley the trees',
         * Ice Cream Island the waves', Butter Building the stone pillars', Grape Garden the purple one, Yogurt Yard the rocks', Orange
         * Ocean the blue blocks', Rainbow Resort the ice one, the Fountain of Dreams the starry one; */
        static readonly (int x, int y)[] MainHubDoors = { (6, 12), (25, 17), (15, 4), (28, 11), (15, 18), (5, 17), (2, 11), (27, 4) };
        static readonly (int x, int y) MainHubReturn = (19, 14);

        /* NightmareRoom; A NiDL room: level, stage, index, record (file offset); its part when split
         * (X, Y, Width, Height in metatiles), and the KatAM room it becomes; */
        public sealed record NightmareRoom(int Level, int Stage, int Index, int Record, int X, int Y, int Width, int Height) {
            public int Room { get; set; }
            public bool IsSwitchRoom => Stage == SwitchStage;
            public int Area => AreaBase + Level;
            // The room as KatAM holds it (see Padded());
            public int FullWidth => Padded(Width, Height).width;
            public int FullHeight => Padded(Width, Height).height;
        }

        /* Padded(); A room's size in KatAM: KatAM's camera never shows a room's bottom 40 pixels nor 8 on its other sides (its rooms
         * hide that much past their edges), so NiDL's rooms get their last row repeated HiddenRows times below, and their last column
         * and row repeated up to a screen (256 x 208: smaller rooms push the camera off them); */
        static (int width, int height) Padded(int width, int height) => (Math.Max(width, MinWidth), Math.Max(height + HiddenRows, MinHeight));
        const int HiddenRows = 3, MinWidth = 16, MinHeight = 13;

        public static IReadOnlyList<NightmareRoom> PortedRooms { get; private set; } = new List<NightmareRoom>();
        public static IReadOnlyDictionary<NightmareRoom, List<Door>> PortedDoors { get; private set; } = new Dictionary<NightmareRoom, List<Door>>();
        // The NiDL hub's room and doors (see BuildMainHub());
        public static (int Room, IReadOnlyList<Door> Doors) PortedMainHub { get; private set; } = (0, new List<Door>());

        /* NightmareRooms(); NiDL's level rooms, split where KatAM can't hold them whole. Left out: the levels' hub lists (hub, museum,
         * arena, boss tower, boss room: no gameplay of their own), level 7's stage 1 (the boss rush's copies of the boss rooms) and
         * stage 2 (unused test rooms), and rooms with no exits nor objects outside the boss stages (empty floors no door leads to); */
        static List<NightmareRoom> NightmareRooms(byte[] nightmare, int table) {
            List<(int level, int stage, int index, int record)> records = new List<(int, int, int, int)>();

            for (int level = 0; level < 8; level++) {
                for (int stage = 0; stage < (level == 7 ? 1 : 8); stage++) {
                    int list = ReadInt(nightmare, table + level * 0x20 + stage * 4);
                    if (list == 0) continue;
                    list -= 0x08000000;

                    for (int index = 0; index < 80; index++) {
                        int record = ReadInt(nightmare, list + index * 4);
                        if (record == 0) continue;
                        if ((record >> 24) != 8 || ReadShort(nightmare, (record & 0x1FFFFFF) + 2) != index) break;
                        records.Add((level, stage, index, record & 0x1FFFFFF));
                    }
                }
            }

            List<NightmareRoom> rooms = new List<NightmareRoom>();
            foreach ((int level, int stage, int index, int record) in records) {
                int width = ReadShort(nightmare, record + 0x14), height = ReadShort(nightmare, record + 0x16);
                if (width <= 2 && height <= 2) continue;
                bool bossStage = level == 7 || stage == records.Where(r => r.level == level).Max(r => r.stage);
                if (!bossStage && ReadInt(nightmare, record + 0x44) == 0 && ReadInt(nightmare, record + 0x48) == 0) continue;

                // Split along the longer side into the fewest parts that fit (padded, see Padded());
                bool across = width >= height;
                int length = across ? width : height, parts = 1;
                int Blocks(int part) {
                    (int w, int h) = across ? Padded((length + part - 1) / part, height) : Padded(width, (length + part - 1) / part);
                    return w * h;
                }
                while (Blocks(parts) > MaxRoomBlocks) parts++;
                for (int part = 0; part < parts; part++) {
                    int start = length * part / parts, end = length * (part + 1) / parts;
                    rooms.Add(across ? new NightmareRoom(level, stage, index, record, start, 0, end - start, height)
                                     : new NightmareRoom(level, stage, index, record, 0, start, width, end - start));
                }
            }
            return rooms;
        }

        /* BuildSwitchRooms(); The switch rooms, as NiDL rooms appended to a copy of the NiDL ROM (records whose cells are made here, the
         * rest the level's first room's: foreground tiles, palette, metatiles, background), so they're converted as the others: per
         * level, its most common metatiles for air, a floor's top, the ground below it and one-way platforms (a floor's top where it
         * has none), laid out as KatAM's switch rooms (rows 0-9; the padding repeats the floor below):
         *   ################      0
         *   ##............##      1-3
         *   ##............##      4-5
         *   #..............#      6
         *   ................      7
         *   .M....#####...M.      8    floor mirrors (1, 8), (14, 8); the pedestal, the big switch on it
         *   ################      9 */
        static List<NightmareRoom> BuildSwitchRooms(ref byte[] nightmare, List<NightmareRoom> rooms) {
            List<NightmareRoom> switchRooms = new List<NightmareRoom>();
            int start = (nightmare.Length + 3) & ~3;
            List<byte> added = new List<byte>();
            int Put(byte[] data) {
                while (added.Count % 4 != 0) added.Add(0);
                int at = start + added.Count;
                added.AddRange(data);
                return at;
            }

            for (int level = 0; level < 8; level++) {
                NightmareRoom source = rooms.Where(r => r.Level == level).OrderBy(r => r.Stage).ThenBy(r => r.Index).First();
                byte[] sourceCells = DecompressLZ77(nightmare, ReadInt(nightmare, source.Record + 8) - 0x08000000);
                int sourceWidth = ReadShort(nightmare, source.Record + 0x14), sourceHeight = ReadShort(nightmare, source.Record + 0x16);
                byte Kind(int x, int y) => RoomCollision[sourceCells[(y * sourceWidth + x) * 4 + 3]];
                // The most common metatile of the cells (with a cell above) that match;
                int Common(Func<byte, byte, bool> match, int fallback) {
                    Dictionary<int, int> counts = new Dictionary<int, int>();
                    for (int y = 1; y < sourceHeight; y++) {
                        for (int x = 0; x < sourceWidth; x++) {
                            if (!match(Kind(x, y), Kind(x, y - 1))) continue;
                            int metatile = ReadShort(sourceCells, (y * sourceWidth + x) * 4);
                            counts[metatile] = counts.GetValueOrDefault(metatile) + 1;
                        }
                    }
                    return counts.Count == 0 ? fallback : counts.MaxBy(c => c.Value).Key;
                }
                int air = Common((kind, above) => kind == 0x00, 0);
                int top = Common((kind, above) => kind == 0x0D && above == 0x00, -1);
                int ground = Common((kind, above) => kind == 0x0D && above == 0x0D, top);
                if (top < 0) top = ground < 0 ? air : ground;
                if (ground < 0) ground = top;
                int platform = Common((kind, above) => kind == 0x0F && above == 0x00, -1);

                byte[] cells = new byte[SwitchWidth * SwitchHeight * 4];
                void Set(int x, int y, int metatile, byte collision) {
                    WriteShort(cells, (y * SwitchWidth + x) * 4, metatile);
                    cells[(y * SwitchWidth + x) * 4 + 3] = collision;
                }
                // NiDL collision: 0 nothing, 0x01 solid, 0x11 a one-way floor (KatAM's 0x0F); a solid cell with air above is a floor's top;
                bool Solid(int x, int y) => y == 0 || y == SwitchHeight - 1 || (y <= 5 && (x <= 1 || x >= SwitchWidth - 2))
                                            || (y == 6 && (x == 0 || x == SwitchWidth - 1)) || (y == 8 && x >= 6 && x <= 10);
                for (int y = 0; y < SwitchHeight; y++) {
                    for (int x = 0; x < SwitchWidth; x++) {
                        if (!Solid(x, y)) Set(x, y, air, 0x00);
                        else Set(x, y, y > 0 && !Solid(x, y - 1) ? top : ground, 0x01);
                    }
                }

                byte[] record = nightmare.Skip(source.Record).Take(0x54).ToArray();
                WriteShort(record, 2, 0);
                WriteInts(record, 8, Put(CompressLZ77(cells)) + 0x08000000, Put(CompressLZ77(new byte[SwitchWidth * SwitchHeight * 2])) + 0x08000000, 0);
                WriteShorts(record, 0x14, SwitchWidth, SwitchHeight);
                WriteInts(record, 0x44, 0, 0);
                WriteShorts(record, 0x50, SwitchArrival.x * 16 + 8, SwitchArrival.y * 16 + 8);
                int at = Put(record);
                switchRooms.Add(new NightmareRoom(level, SwitchStage, 0, at, 0, 0, SwitchWidth, SwitchHeight));
            }

            byte[] extended = new byte[start + added.Count];
            nightmare.CopyTo(extended, 0);
            added.CopyTo(extended, start);
            nightmare = extended;
            return switchRooms;
        }

        /* AddRooms(); Every NiDL room into KatAM's free room slots (see the notes above); the room records' foregrounds, palettes and
         * backgrounds are shared as in NiDL; */
        static string AddRooms(byte[] rom, byte[] nightmare, NightmareRegion region, KatAMExpansionCode.Version version, ref int cursor) {
            IReadOnlyDictionary<string, int> symbols = version.RomSymbols;
            int roomProps = symbols["gRoomProps"], roomInfos = symbols["gUnk_08D6CD0C"];
            List<NightmareRoom> rooms;

            try {
                rooms = NightmareRooms(nightmare, region.RoomTable);
                rooms.AddRange(BuildSwitchRooms(ref nightmare, rooms));
            } catch (Exception exception) when (exception is IndexOutOfRangeException or ArgumentException or InvalidOperationException) {
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

            // Room numbers: the NiDL hub's first, then each level's (its switch room first) from its hundred, then from any left;
            if (free.Count < rooms.Count + 1) return "KatAM has too few free rooms for Nightmare in Dream Land's.";
            List<int> left = new List<int>(free);
            int mainHub = left.Where(n => n / 100 == LevelHundreds[7]).DefaultIfEmpty(left[0]).First();
            left.Remove(mainHub);
            for (int level = 0; level < 8; level++) {
                foreach (NightmareRoom r in rooms.Where(r => r.Level == level).OrderBy(r => r.IsSwitchRoom ? 0 : 1)) {
                    int room = left.Where(n => n / 100 == LevelHundreds[level]).DefaultIfEmpty(left[0]).First();
                    r.Room = room;
                    left.Remove(room);
                }
            }
            Dictionary<NightmareRoom, List<Door>> doors;
            List<Door> mainHubDoors;
            (int room, int x, int y) firstStart;
            byte firstFacing;
            try {
                doors = PlanDoors(nightmare, rooms, mainHub, out mainHubDoors, out firstStart, out firstFacing);
            } catch (Exception exception) when (exception is IndexOutOfRangeException or ArgumentException or InvalidOperationException) {
                return $"The Nightmare in Dream Land ROM doesn't match the {region.KatAM} release.";
            }

            try {
                for (int i = 0; i < rooms.Count; i++) {
                    NightmareRoom r = rooms[i];
                    int record = r.Record, roomWidth = ReadShort(nightmare, record + 0x14);
                    int Address(int offset) => ReadInt(nightmare, record + offset) - 0x08000000;
                    byte[] cells = DecompressLZ77(nightmare, Address(8));
                    // The NiDL cell for a cell of the padded room (see Padded()): the edge's own where past it;
                    int Cell(int x, int y) => (r.Y + Math.Min(y, r.Height - 1)) * roomWidth + r.X + Math.Min(x, r.Width - 1);

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
                    int width = r.FullWidth * 2, height = r.FullHeight * 2;
                    byte[] tilemap = new byte[width * height * 2];
                    for (int y = 0; y < height; y++) {
                        for (int x = 0; x < width; x++) {
                            int cell = Cell(x / 2, y / 2);
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
                    // (the padding's breakable blocks are solid: KatAM would look for their special block records);
                    byte[] collision = new byte[r.FullWidth * r.FullHeight];
                    for (int y = 0; y < r.FullHeight; y++) {
                        for (int x = 0; x < r.FullWidth; x++) {
                            int kind = cells[Cell(x, y) * 4 + 3];
                            bool padding = x >= r.Width || y >= r.Height;
                            collision[y * r.FullWidth + x] = padding && kind >> 4 == 2 ? (byte)0x0D : RoomCollision[kind];
                        }
                    }
                    int solidity = BuildSolidity(rom, nightmare, r, cells, collision, metatiles, doors[r], noSpecials, ref cursor);
                    solidities.Add(solidity + 0x08000000);

                    objects.Add(BuildObjects(rom, nightmare, r, doors[r], noTypes, ref cursor) + 0x08000000);
                    navigations.Add(BuildNavigation(rom, r, navigationRoutes, ref cursor) + 0x08000000);

                    // Area and routes (a room of its own, like room 0x397), written into the vanilla table's free entry;
                    int roomInfo = Allocate(ref cursor, 0x4C, 4);
                    WriteInt(rom, roomInfo, noRoutes);
                    WriteShort(rom, roomInfo + 0x44, r.Room);
                    rom[roomInfo + 0x46] = (byte)r.Area;
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

            BuildMainHub(rom, nightmare, symbols, version.SpriteTables, mainHub, mainHubDoors, rooms.First(r => r.Level == 7 && r.Stage == 0 && r.Index == 0).Record, tableSizes,
                         infos, palettes, tilemaps, solidities, backgrounds, objects, noTypes, ref cursor);
            WriteShort(rom, CodeAddress + version.CodeSymbols["gNidlHubRoom"], mainHub);

            /* The entry room's collision gets an entry of its own (still its vanilla one: the randomizer writes into it), which
             * FinishRooms() replaces with a copy that has the door to Nightmare in Dream Land, once the randomizer is done; */
            int entrySolidity = tableSizes[13] + solidities.Count, entryTilemap = Entry("gForegroundTilemaps", Props(EntryRoom, 12)),
                entryWidth = ReadShort(rom, entryTilemap) / 2;
            solidities.Add(Entry("gSolidityMaps", Props(EntryRoom, 13)) + 0x08000000);
            WriteShort(rom, roomProps + EntryRoom * 0x28 + 13 * 2, entrySolidity);

            /* Its tilemap: the Rainbow Route mirror on the right end of its platform (tiles 37-40, rows 24-29: door 19, 14) mirrored onto
             * the left end (door EntryX, EntryY), flipped; a mirror in the room's own tiles, as golden as KatAM's; */
            {
                int columns = ReadShort(rom, entryTilemap);
                byte[] map = DecompressLZ77(rom, ReadInt(rom, entryTilemap + 0x18) - 0x08000000);
                for (int y = 24; y <= 29; y++) {
                    for (int x = 37; x <= 40; x++) {
                        int tile = ReadShort(map, (y * columns + x) * 2);
                        if (tile != 0) WriteShort(map, (y * columns + columns - 1 - x) * 2, tile ^ 0x400);
                    }
                }
                byte[] compressedMap = CompressLZ77(map);
                int mapAt = Allocate(ref cursor, compressedMap.Length, 4), tilemapEntry = Allocate(ref cursor, 0x20, 4);
                compressedMap.CopyTo(rom, mapAt);
                Array.Copy(rom, entryTilemap, rom, tilemapEntry, 0x20);
                WriteInt(rom, tilemapEntry + 0x18, mapAt + 0x08000000);
                WriteShort(rom, roomProps + EntryRoom * 0x28 + 12 * 2, tableSizes[12] + tilemaps.Count);
                tilemaps.Add(tilemapEntry + 0x08000000);
            }
            ExtendAreas(rom, symbols, ref cursor);

            /* 9ROM: the doors list every ported room shares (doorsIdx 287): one entry (kind 2, size 8, room, landing + 1) per place a
             * door leads to. KatAM looks a door up there when Kirby goes through it (and a mirror, to show whether it was used), until it
             * finds it: it must be there. Its door count is 0, so the map counts none of them; */
            List<byte> nineROM = new List<byte>();
            foreach (Door door in doors.Values.SelectMany(list => list).Concat(mainHubDoors).DistinctBy(d => (d.Room, d.DestX, d.DestY)))
                nineROM.AddRange(NineROMEntry(door.Room, door.DestX, door.DestY));
            nineROM.AddRange(new byte[] { 0, 0, 0xFF, 0xFF });
            int nineROMAt = Allocate(ref cursor, nineROM.Count, 4), doorless = Allocate(ref cursor, 8, 4);
            nineROM.ToArray().CopyTo(rom, nineROMAt);
            WriteInts(rom, doorless, nineROMAt + 0x08000000, 0);

            // The tables, copied with the new entries;
            (string table, int size, List<int> entries)[] tables = {
                ("gForegroundInfo", tableSizes[10], infos), ("gForegroundPalettes", tableSizes[11], palettes),
                ("gForegroundTilemaps", tableSizes[12], tilemaps), ("gSolidityMaps", tableSizes[13], solidities),
                ("gRoomTiledBGs", tableSizes[15], backgrounds), ("gLevelObjLists", tableSizes[16], objects),
                ("gUnk_08D63C28", tableSizes[17], navigations), ("gUnk_08D640A4", SharedDoors, new List<int> { doorless + 0x08000000 }),
            };
            Dictionary<string, int> moved = new Dictionary<string, int>();
            foreach ((string table, int size, List<int> entries) in tables) {
                int at = moved[table] = Allocate(ref cursor, (size + entries.Count) * 4, 4);
                Array.Copy(rom, symbols[table], rom, at, size * 4);
                WriteInts(rom, at + size * 4, entries.ToArray());
                Repoint(rom, symbols[table] + 0x08000000, at + 0x08000000);
            }

            PortedRooms = rooms;
            PortedDoors = doors;
            PortedMainHub = (mainHub, mainHubDoors);

            /* The map tracker's rooms block, in the map table (see WriteMapTable(): its mini-bosses end before it): "NR", the NiDL hub's
             * room, then where the room tables are (gSolidityMaps, gForegroundTilemaps, gForegroundPalettes, gForegroundInfo,
             * gRoomTiledBGs, gLevelObjLists: the copies; gUnk_08D6CD0C: the areas); */
            System.Text.Encoding.ASCII.GetBytes("NR").CopyTo(rom, MapRoomsBlock);
            WriteShort(rom, MapRoomsBlock + 2, mainHub);
            WriteInts(rom, MapRoomsBlock + 4, new[] { "gSolidityMaps", "gForegroundTilemaps", "gForegroundPalettes", "gForegroundInfo", "gRoomTiledBGs", "gLevelObjLists" }
                .Select(t => moved[t] + 0x08000000).Append(roomInfos + 0x08000000).ToArray());
            entry = (moved["gSolidityMaps"] + entrySolidity * 4, cursor, firstStart, firstFacing, entryWidth, moved["gUnk_08D640A4"] + Props(EntryRoom, 18) * 4);
            return null;
        }

        /* ExtendAreas(); Areas AreaBase... for the NiDL levels (see Areas):
         * • gUnk_08D6DCAC (the CPU helpers' routes, per area: 0-10) is copied with the Dimension Mirror's (no routes, as KatAM's rooms
         *   of area 10) for each new area, and repointed;
         * • AreaPlates (gUnk_08D60EE4: per language, a block of 0x6900 bytes ending with the area plates, 0x200 each from +0x5300, 4bpp,
         *   8x2 tiles: the area's number and name on two lines; the HUD copies area * 0x200 from there, so area 11's would be the next
         *   block's start): each block is copied with a plate per new area, composed of the letters of the game's English plates
         *   (ComposePlate()), or the Dimension Mirror's blank plate where they can't be read; */
        const int PlateBlockSize = 0x6900, PlatesAt = 0x5300, PlateSize = 0x200, VanillaAreas = 11;
        static void ExtendAreas(byte[] rom, IReadOnlyDictionary<string, int> symbols, ref int cursor) {
            int routes = symbols["gUnk_08D6DCAC"], newRoutes = Allocate(ref cursor, (AreaBase + 8) * 4, 4);
            Array.Copy(rom, routes, rom, newRoutes, VanillaAreas * 4);
            for (int area = VanillaAreas; area < AreaBase + 8; area++) WriteInt(rom, newRoutes + area * 4, ReadInt(rom, routes + 10 * 4));
            Repoint(rom, routes + 0x08000000, newRoutes + 0x08000000);

            int table = symbols["AreaPlates"];
            List<int> blocks = Enumerable.Range(0, 6).Select(i => ReadInt(rom, table + i * 4)).Where(b => (b >> 24) == 8).Distinct().ToList();
            Dictionary<char, bool[,]> letters = blocks.Select(b => PlateLetters(rom, b - 0x08000000)).FirstOrDefault(l => l != null);
            Dictionary<int, int> copies = new Dictionary<int, int>();
            foreach (int block in blocks) {
                int from = block - 0x08000000, at = Allocate(ref cursor, PlateBlockSize + 8 * PlateSize, 4);
                Array.Copy(rom, from, rom, at, PlateBlockSize);
                for (int level = 0; level < 8; level++) {
                    byte[] plate = letters == null ? null : ComposePlate(letters, LevelNames[level].first, LevelNames[level].second);
                    if (plate == null) Array.Copy(rom, from + PlatesAt + 10 * PlateSize, rom, at + PlatesAt + (AreaBase + level) * PlateSize, PlateSize);
                    else plate.CopyTo(rom, at + PlatesAt + (AreaBase + level) * PlateSize);
                }
                copies[block] = at + 0x08000000;
            }
            for (int i = 0; i < 6; i++) {
                int block = ReadInt(rom, table + i * 4);
                if (copies.TryGetValue(block, out int copy)) WriteInt(rom, table + i * 4, copy);
            }
        }

        /* A plate's pixels (64 x 16): letters (colors 8, the first of a word, and 9) outlined (13), the number ("(1)": its digit in color
         * 8, the brackets 12) before the first line, and on the second, stripes (12) on rows 2, 4, 5 around the name; the English plates'
         * words, to read their letters from: */
        static readonly (string first, string second)[] PlateWords = {
            ("RAINBOW", "ROUTE"), ("MOONLIGHT", "MANSION"), ("CABBAGE", "CAVERN"), ("MUSTARD", "MOUNTAIN"), ("CARROT", "CASTLE"),
            ("OLIVE", "OCEAN"), ("PEPPERMINT", "PALACE"), ("RADISH", "RUINS"), ("CANDY", "CONSTELLATION"),
        };
        static int[,] PlatePixels(byte[] rom, int plate) {
            int[,] pixels = new int[16, 64];
            for (int y = 0; y < 16; y++)
                for (int x = 0; x < 64; x++) {
                    int b = rom[plate + ((y / 8) * 8 + x / 8) * 32 + y % 8 * 4 + x % 8 / 2];
                    pixels[y, x] = (x & 1) != 0 ? b >> 4 : b & 15;
                }
            return pixels;
        }

        /* PlateLetters(); The letters of a block's English plates (their letter pixels: 6 rows), or null if they aren't English ones;
         * a run of letters touching each other (CANDY's D and Y) is split by the widths the other plates give its first letters; */
        static Dictionary<char, bool[,]> PlateLetters(byte[] rom, int block) {
            Dictionary<char, bool[,]> letters = new Dictionary<char, bool[,]>();
            List<(int[,] pixels, int top, string word, List<(int from, int to)> runs)> lines = new List<(int[,], int, string, List<(int, int)>)>();
            for (int area = 0; area < PlateWords.Length; area++) {
                int[,] pixels = PlatePixels(rom, block + PlatesAt + area * PlateSize);
                foreach ((int top, string word) in new[] { (1, PlateWords[area].first), (9, PlateWords[area].second) }) {
                    // The columns with letter pixels, in runs: one run per letter (the first line's first: the number's digit);
                    List<(int from, int to)> runs = new List<(int, int)>();
                    for (int x = 0; x < 64; x++) {
                        if (!Enumerable.Range(top, 6).Any(y => pixels[y, x] is 8 or 9)) continue;
                        if (runs.Count > 0 && runs[^1].to == x - 1) runs[^1] = (runs[^1].from, x);
                        else runs.Add((x, x));
                    }
                    if (top == 1 && runs.Count > 0) runs.RemoveAt(0);
                    if (runs.Count > word.Length) return null;
                    lines.Add((pixels, top, word, runs));
                }
            }
            void Take(int[,] pixels, int top, char letter, int from, int to) {
                bool[,] glyph = new bool[6, to - from + 1];
                for (int y = 0; y < 6; y++)
                    for (int x = from; x <= to; x++) glyph[y, x - from] = pixels[top + y, x] is 8 or 9;
                letters.TryAdd(letter, glyph);
            }
            foreach (var line in lines.Where(l => l.runs.Count == l.word.Length))
                for (int i = 0; i < line.word.Length; i++) Take(line.pixels, line.top, line.word[i], line.runs[i].from, line.runs[i].to);
            foreach (var line in lines.Where(l => l.runs.Count < l.word.Length)) {
                List<(int from, int to)> runs = new List<(int, int)>(line.runs);
                for (int i = 0, j = 0; i < line.word.Length && j < runs.Count; i++, j++) {
                    int width = runs[j].to - runs[j].from + 1;
                    if (runs.Count - j < line.word.Length - i && letters.TryGetValue(line.word[i], out bool[,] known) && known.GetLength(1) < width) {
                        Take(line.pixels, line.top, line.word[i], runs[j].from, runs[j].from + known.GetLength(1) - 1);
                        runs.Insert(j + 1, (runs[j].from + known.GetLength(1), runs[j].to));
                    } else Take(line.pixels, line.top, line.word[i], runs[j].from, runs[j].to);
                }
            }
            // F: E without its bottom bar;
            if (letters.TryGetValue('E', out bool[,] e) && !letters.ContainsKey('F')) {
                bool[,] f = (bool[,])e.Clone();
                for (int x = 1; x < f.GetLength(1); x++) f[5, x] = false;
                letters['F'] = f;
            }
            return letters;
        }

        /* ComposePlate(); A plate: the first line left-aligned from x 3, the second right-aligned to x 58 between stripes, as KatAM's
         * (without a number); null if a letter is missing or a line doesn't fit; */
        static byte[] ComposePlate(Dictionary<char, bool[,]> letters, string first, string second) {
            int[,] pixels = new int[16, 64];
            bool Line(string text, int top, bool right) {
                if (text.Any(c => c != ' ' && !letters.ContainsKey(c))) return false;
                int width = 0;
                for (int i = 0; i < text.Length; i++)
                    width += text[i] == ' ' ? 3 : letters[text[i]].GetLength(1) + (i + 1 < text.Length && text[i + 1] != ' ' ? 1 : 0);
                int x = right ? 59 - width : 3;
                if (x < 1 || x + width > 61) return false;
                for (int i = 0; i < text.Length; i++) {
                    if (text[i] == ' ') { x += 3; continue; }
                    bool[,] glyph = letters[text[i]];
                    int color = i == 0 || text[i - 1] == ' ' ? 8 : 9;
                    for (int y = 0; y < 6; y++)
                        for (int c = 0; c < glyph.GetLength(1); c++)
                            if (glyph[y, c]) pixels[top + y, x + c] = color;
                    x += glyph.GetLength(1) + 1;
                }
                // The outline: every empty pixel of the line next to a letter's;
                for (int y = top - 1; y <= top + 6; y++)
                    for (int c = 0; c < 64; c++) {
                        if (pixels[y, c] != 0) continue;
                        bool near = false;
                        for (int dy = -1; dy <= 1; dy++)
                            for (int dx = -1; dx <= 1; dx++) {
                                int yy = y + dy, xx = c + dx;
                                if (yy >= top && yy < top + 6 && xx >= 0 && xx < 64 && pixels[yy, xx] is 8 or 9) near = true;
                            }
                        if (near) pixels[y, c] = 13;
                    }
                return true;
            }
            if (!Line(first, 1, false) || !Line(second, 9, true)) return null;
            foreach (int y in new[] { 10, 12, 13 })
                for (int x = 0; x < 63; x++) if (pixels[y, x] == 0) pixels[y, x] = 12;

            byte[] plate = new byte[PlateSize];
            for (int y = 0; y < 16; y++)
                for (int x = 0; x < 64; x++)
                    plate[((y / 8) * 8 + x / 8) * 32 + y % 8 * 4 + x % 8 / 2] |= (byte)((x & 1) != 0 ? pixels[y, x] << 4 : pixels[y, x]);
            return plate;
        }

        /* Door; A door of a ported room (KatAM's 8ROM: a special block record, kind 3, at its cell, collision 0xFF, or 0xFD in water):
         * where it leads (the KatAM room, and the cell Kirby lands in plus one, as the records hold it), and whether a mirror (object
         * 0x6F) stands on it, and which way Kirby faces there (1 right, 2 left: towards the room's middle); an edge (collision 0xFE, 0xFC in
         * water) takes Kirby through as he touches it, without a mirror; */
        public sealed record Door(int X, int Y, int Room, int DestX, int DestY, byte Facing, bool Edge = false, int Slot = 0, int Design = HubDesign);

        // The KatAM room with the mirror to Nightmare in Dream Land: Central Circle, on its platform's left end (FinishRooms());
        const int EntryRoom = 0x321, EntryX = 11, EntryY = 14;
        static (int solidityEntry, int cursor, (int room, int x, int y) firstStart, byte facing, int width, int doorsEntry) entry;

        /* PlanDoors(); Every ported room's doors, as KatAM mirrors:
         * • NiDL's doors (exits to a room of the same stage) where they were, to where they led;
         * • warp stars and cannons (their exits, kind 30) become mirrors where they stood, to where Kirby starts in the room they lead to
         *   (their landing is where their flight starts: under the tower they fly Kirby up into, in 3-2);
         * • a stage's goal door (kind 34) leads to the next stage of its level; the levels' boss stages (their last) and the Fountain of
         *   Dreams have no doors of their own, so each of their rooms gets a mirror where Kirby lands, to the next room, then to the next
         *   level's switch room (the Fountain of Dreams: the NiDL hub, onto its mirror);
         * • a room left past its edge (kind 26: its x the room, 0xFFFF none; Kirby falls from the sky out of its bottom, climbs a tower
         *   out of its top) gets edges on its top and bottom rows' open cells;
         * • the hubs: Central Circle's golden mirror (EntryRoom) leads to the NiDL hub (mainHub, its doors in mainHubDoors), whose
         *   mirrors lead to the levels' switch rooms (hidden until their big switch is pressed, Slot, but Vegetable Valley's) and back;
         *   a switch room's side mirrors lead to its level's first 4 stages (the others follow them; the Fountain of Dreams': its only
         *   one), and the mirror its big switch reveals to where LevelLinks says;
         * • a split room's parts lead into each other through their shared edge.
         * The doors to the hubs' museum and arena (kind 21) aren't ported; */
        static Dictionary<NightmareRoom, List<Door>> PlanDoors(byte[] nightmare, List<NightmareRoom> rooms, int mainHub, out List<Door> mainHubDoors,
                                                               out (int room, int x, int y) firstStart, out byte firstFacing) {
            Dictionary<NightmareRoom, List<Door>> doors = rooms.ToDictionary(r => r, r => new List<Door>());
            var byRoom = rooms.GroupBy(r => (r.Level, r.Stage, r.Index)).ToDictionary(g => g.Key, g => g.ToList());
            List<(int level, int stage)> stages = rooms.Where(r => !r.IsSwitchRoom).Select(r => (r.Level, r.Stage)).Distinct().ToList();
            bool IsBossStage((int level, int stage) s) => s.level == 7 || s.stage == stages.Where(t => t.level == s.level).Max(t => t.stage);

            // The part holding a NiDL room's cell, and that cell in it;
            (NightmareRoom part, int x, int y) PartAt((int, int, int) key, int x, int y) {
                foreach (NightmareRoom part in byRoom[key]) {
                    if (x >= part.X && x < part.X + part.Width && y >= part.Y && y < part.Y + part.Height) return (part, x - part.X, y - part.Y);
                }
                NightmareRoom last = byRoom[key].Last();
                return (last, Math.Clamp(x - last.X, 0, last.Width - 1), Math.Clamp(y - last.Y, 0, last.Height - 1));
            }
            byte[] Cells(NightmareRoom r) => DecompressLZ77(nightmare, ReadInt(nightmare, r.Record + 8) - 0x08000000);
            // Whether Kirby can go through a NiDL cell (nothing, water, ladders; breakable blocks too, once broken, if blocks);
            bool OpenCell(byte[] cells, int width, int x, int y, bool blocks = false) {
                int kind = cells[(y * width + x) * 4 + 3];
                return RoomCollision[kind] is 0x00 or 0x48 or 0x20 or 0x21 || blocks && kind >> 4 == 2;
            }
            // Where Kirby starts (+0x50, pixels), down to the floor;
            (NightmareRoom part, int x, int y) Start((int, int, int) key) {
                NightmareRoom any = byRoom[key][0];
                int width = ReadShort(nightmare, any.Record + 0x14), height = ReadShort(nightmare, any.Record + 0x16);
                int x = Math.Clamp(ReadShort(nightmare, any.Record + 0x50) / 16, 0, width - 1), y = Math.Clamp(ReadShort(nightmare, any.Record + 0x52) / 16, 0, height - 1);
                byte[] cells = Cells(any);
                bool Floor(int cy) => RoomCollision[cells[(cy * width + x) * 4 + 3]] is >= 0x01 and <= 0x1B || (cells[(cy * width + x) * 4 + 3] >> 4) == 2;
                while (y + 1 < height && !Floor(y + 1)) y++;
                return PartAt(key, x, y);
            }
            (int room, int x, int y) Target((NightmareRoom part, int x, int y) at) => (at.part.Room, at.x + 1, at.y + 1);
            (int, int, int) StageStart((int level, int stage) s) => Target(Start((s.level, s.stage, byRoom.Keys.Where(k => k.Item1 == s.level && k.Item2 == s.stage).Min(k => k.Item3))));
            (int room, int x, int y) back = (EntryRoom, EntryX + 1, EntryY + 1);
            NightmareRoom SwitchRoomOf(int level) => rooms.First(r => r.IsSwitchRoom && r.Level == level);
            (int, int, int) SwitchRoomArrival(int level) => Target((SwitchRoomOf(level), SwitchArrival.x, SwitchArrival.y));
            (int, int, int) MainHubAt((int x, int y) cell) => (mainHub, cell.x + 1, cell.y + 1);
            (int, int, int) After((int level, int stage) s) {
                if (IsBossStage(s)) return s.level == 7 ? MainHubAt(MainHubDoors[7]) : SwitchRoomArrival(s.level + 1);
                List<(int level, int stage)> level = stages.Where(t => t.level == s.level).ToList();
                return StageStart(level[level.IndexOf(s) + 1]);
            }
            Dictionary<int, int> widths = rooms.ToDictionary(r => r.Room, r => r.Width);
            byte Facing((int room, int x, int y) to) => (byte)(!widths.TryGetValue(to.room, out int w) || (to.x - 1) * 2 < w ? 1 : 2);
            void Add((NightmareRoom part, int x, int y) at, (int room, int x, int y) to, bool edge = false, int slot = 0, int design = HubDesign) {
                if (doors[at.part].Any(d => d.X == at.x && d.Y == at.y)) return;
                doors[at.part].Add(new Door(at.x, at.y, to.room, to.x, to.y, Facing(to), edge, slot, design));
            }

            List<((NightmareRoom, int, int) at, (int, int, int) to)> edges = new List<((NightmareRoom, int, int), (int, int, int))>();
            foreach (var key in byRoom.Keys) {
                NightmareRoom r = byRoom[key][0];
                if (r.IsSwitchRoom) continue;
                (int level, int stage) s = (key.Item1, key.Item2);
                int width = ReadShort(nightmare, r.Record + 0x14), height = ReadShort(nightmare, r.Record + 0x16);
                int exits = ReadInt(nightmare, r.Record + 0x44), objects = ReadInt(nightmare, r.Record + 0x48);
                int exitsEnd = objects != 0 ? objects - 0x08000000 : r.Record;
                List<(int x, int y)> transports = new List<(int, int)>();
                for (int at = objects == 0 ? r.Record : objects - 0x08000000; objects != 0 && at + 8 <= r.Record; at += 8) {
                    // Warp stars (gimmick 4) and cannons (6);
                    if (nightmare[at] == 5 && (nightmare[at + 1] == 4 || nightmare[at + 1] == 6))
                        transports.Add((ReadShort(nightmare, at + 4) / 16, ReadShort(nightmare, at + 6) / 16));
                }

                // NiDL's exits; a door with two exits (one only after a big switch) keeps its plain one, listed later;
                List<(int x, int y, (int, int, int) to)> found = new List<(int, int, (int, int, int))>();
                for (int at = exits - 0x08000000; exits != 0 && at + 12 <= exitsEnd; at += 12) {
                    int destination = nightmare[at], kind = nightmare[at + 1], x = ReadShort(nightmare, at + 2), y = ReadShort(nightmare, at + 4);
                    int landX = ReadShort(nightmare, at + 6) / 16, landY = ReadShort(nightmare, at + 8) / 16;
                    if (kind == 0 && x < width && y < height && byRoom.ContainsKey((s.level, s.stage, destination)))
                        found.Insert(0, (x, y, Target(PartAt((s.level, s.stage, destination), landX, landY))));
                    else if (kind == 34 && x < width && y < height)
                        found.Add((x, y, After(s)));
                    else if (kind == 30 && transports.Count > 0 && byRoom.ContainsKey((s.level, s.stage, x)))
                        found.Add((transports[0].x, transports[0].y, Target(Start((s.level, s.stage, x)))));
                    else if (kind == 26 && byRoom.ContainsKey((s.level, s.stage, x))) {
                        byte[] cells = Cells(r);
                        var to = Target(PartAt((s.level, s.stage, x), landX, landY));
                        for (int cx = 0; cx < width; cx++) {
                            if (OpenCell(cells, width, cx, 0)) edges.Add((PartAt(key, cx, 0), to));
                            if (OpenCell(cells, width, cx, height - 1)) edges.Add((PartAt(key, cx, height - 1), to));
                        }
                    }
                }
                foreach ((int x, int y, (int, int, int) to) in found) Add(PartAt(key, x, y), to);

                if (IsBossStage(s)) {
                    int next = byRoom.Keys.Where(k => k.Item1 == s.level && k.Item2 == s.stage && k.Item3 > key.Item3).Select(k => k.Item3).DefaultIfEmpty(-1).Min();
                    Add(Start(key), next >= 0 ? Target(Start((s.level, s.stage, next))) : After(s));
                }
            }

            foreach (var (at, to) in edges) Add(at, to, true);

            // The switch rooms; the NiDL hub's mirrors (Central Circle's golden mirror leads to the NiDL hub's, FinishRooms());
            mainHubDoors = new List<Door>();
            for (int level = 0; level < 8; level++) {
                NightmareRoom switchRoom = SwitchRoomOf(level);
                List<(int level, int stage)> levelStages = stages.Where(t => t.level == level && !IsBossStage(t)).DefaultIfEmpty(stages.First(t => t.level == level)).ToList();
                for (int i = 0; i < SwitchSides.Length && i < levelStages.Count; i++)
                    Add((switchRoom, SwitchSides[i].x, SwitchSides[i].y), StageStart(levelStages[i]));
                int linked = LevelLinks[level];
                (int, int, int) link = linked == Hub ? MainHubAt(MainHubDoors[level]) : linked < 0 ? SwitchRoomArrival(-2 - linked) : (linked, SwitchLanding.x, SwitchLanding.y);
                Add((switchRoom, SwitchTop.x, SwitchTop.y), link, slot: LevelSlot(level), design: LinkDesign(linked));

                (int room, int x, int y) arrival = SwitchRoomArrival(level);
                mainHubDoors.Add(new Door(MainHubDoors[level].x, MainHubDoors[level].y, arrival.room, arrival.x, arrival.y, Facing(arrival), Slot: level == 0 ? 0 : LevelSlot(level)));
            }
            mainHubDoors.Add(new Door(MainHubReturn.x, MainHubReturn.y, back.room, back.x, back.y, 1));
            firstStart = MainHubAt(MainHubReturn);
            firstFacing = 2;

            // A split room's parts: the cells on their shared edge (not solid) take Kirby across;
            foreach (var parts in byRoom.Values.Where(p => p.Count > 1)) {
                byte[] cells = Cells(parts[0]);
                int width = ReadShort(nightmare, parts[0].Record + 0x14);
                // (through breakable blocks too: Heavy Mole's arena is all blocks);
                bool Open(int x, int y) => OpenCell(cells, width, x, y, true);
                for (int i = 0; i + 1 < parts.Count; i++) {
                    NightmareRoom a = parts[i], b = parts[i + 1];
                    if (b.X > a.X) {
                        for (int y = 0; y < a.Height; y++) {
                            if (Open(b.X - 1, y)) Add((a, a.Width - 1, y), (b.Room, 2, y + 1), true);
                            if (Open(b.X, y)) Add((b, 0, y), (a.Room, a.Width - 1, y + 1), true);
                        }
                    } else {
                        for (int x = 0; x < a.Width; x++) {
                            if (Open(x, b.Y - 1)) Add((a, x, a.Height - 1), (b.Room, x + 1, 2), true);
                            if (Open(x, b.Y)) Add((b, x, 0), (a.Room, x + 1, a.Height - 1), true);
                        }
                    }
                }
            }
            return doors;
        }

        /* FinishRooms(); After the randomizer: Central Circle (EntryRoom) gets its door to Nightmare in Dream Land (the golden mirror its
         * tilemap got in AddRooms()), in copies of its collision, special blocks (8ROM) and doors list (9ROM) as the randomizer left them; the 9ROM list keeps its door count,
         * so the map's doors don't change; */
        public static void FinishRooms(byte[] rom) {
            if (entry.solidityEntry == 0) return;
            int cursor = entry.cursor;
            // Collision and special blocks;
            int solidity = ReadInt(rom, entry.solidityEntry) - 0x08000000;
            byte[] collision = DecompressRL(rom, ReadInt(rom, solidity) - 0x08000000);
            int width = entry.width;
            collision[EntryY * width + EntryX] = 0xFF;
            List<byte> specials = new List<byte>();
            for (int at = ReadInt(rom, solidity + 4) - 0x08000000; rom[at + 4] != 0; at += rom[at + 4])
                specials.AddRange(rom.Skip(at).Take(rom[at + 4]));
            specials.AddRange(DoorRecord(new Door(EntryX, EntryY, entry.firstStart.room, entry.firstStart.x, entry.firstStart.y, entry.facing), 0xFF));
            specials.AddRange(new byte[] { 0, 0, 0, 0, 0, 0, 0xFF, 0xFF });
            byte[] compressed = CompressRL(collision);
            int collisionAt = Allocate(ref cursor, compressed.Length, 4), specialsAt = Allocate(ref cursor, specials.Count, 4), newSolidity = Allocate(ref cursor, 8, 4);
            compressed.CopyTo(rom, collisionAt);
            specials.ToArray().CopyTo(rom, specialsAt);
            WriteInts(rom, newSolidity, collisionAt + 0x08000000, specialsAt + 0x08000000);
            WriteInt(rom, entry.solidityEntry, newSolidity + 0x08000000);

            // 9ROM: its doors list with the new door, before the list's end (its door count stays: the map doesn't count it);
            int doorsInfo = ReadInt(rom, entry.doorsEntry) - 0x08000000;
            List<byte> nineROM = new List<byte>();
            for (int at = ReadInt(rom, doorsInfo) - 0x08000000; rom[at + 1] != 0; at += rom[at + 1]) nineROM.AddRange(rom.Skip(at).Take(rom[at + 1]));
            nineROM.AddRange(NineROMEntry(entry.firstStart.room, entry.firstStart.x, entry.firstStart.y));
            nineROM.AddRange(new byte[] { 0, 0, 0xFF, 0xFF });
            int nineROMAt = Allocate(ref cursor, nineROM.Count, 4), newDoorsInfo = Allocate(ref cursor, 8, 4);
            nineROM.ToArray().CopyTo(rom, nineROMAt);
            Array.Copy(rom, doorsInfo, rom, newDoorsInfo, 8);
            WriteInt(rom, newDoorsInfo, nineROMAt + 0x08000000);
            WriteInt(rom, entry.doorsEntry, newDoorsInfo + 0x08000000);
            entry = default;
        }

        const byte MirrorType = 0x6F;

        // MirrorTemplate(); A mirror (object 0x6F) standing on a door cell, as KatAM's (its x, y: the cell's center);
        static byte[] MirrorTemplate(int x, int y, int number) => ObjectTemplate(x * 16 + 8, y * 16 + 8, number, MirrorType);

        /* DoorToHubTemplate(); A door object (0x97, CreateDoorToHub) under a door cell (its x, y: the cell's bottom center), as KatAM's
         * switch rooms': once the world slot is set, it breaks the cell's special block (making the door) and shows its mirror, whose
         * frame is the design (anim 0x2C6's variant); */
        const byte DoorToHubType = 0x97;
        static byte[] DoorToHubTemplate(int x, int y, int number, int slot, int design) {
            byte[] template = ObjectTemplate(x * 16 + 8, (y + 1) * 16, number, DoorToHubType);
            template[4] = (byte)slot;
            template[14] = template[15] = (byte)design;
            return template;
        }

        // HubMirrorTemplate(); A hub mirror object (0x8C) at a cell's center, revealing the door below it once the world slot is set;
        static byte[] HubMirrorTemplate(int x, int y, int number, int slot) {
            byte[] template = ObjectTemplate(x * 16 + 8, y * 16 + 8, number, HubMirrorType);
            template[4] = (byte)slot; template[5] = 1;
            return template;
        }

        /* BigSwitchTemplate(); A big switch (object 0x86, as KatAM's: +0x22 6) setting the world slot; its +0x14 picks what pressing
         * it runs: the level's world map door (see KatAMExpansionWorldMap.cs); */
        static byte[] BigSwitchTemplate(int x, int y, int number, int slot, int level) {
            byte[] template = ObjectTemplate(x, y, number, BigSwitchType);
            template[4] = (byte)slot; template[5] = 1;
            WriteShort(template, 0x14, NidlBigSwitchId(level));
            template[0x22] = 6;
            return template;
        }

        static byte[] ObjectTemplate(int x, int y, int number, byte type) {
            byte[] template = new byte[36];
            template[0] = 0x01; template[1] = 0x24; template[3] = (byte)number;
            WriteShorts(template, 6, x, y);
            template[10] = template[11] = template[13] = 0xFF;
            template[12] = type;
            return template;
        }

        // NineROMEntry(); A 9ROM entry: kind 2 (a door), size 8, the room and landing cell + 1 it leads to;
        static byte[] NineROMEntry(int room, int x, int y) => new byte[] { 2, 8, 0xFF, 0xFF, (byte)room, (byte)(room >> 8), (byte)x, (byte)y };

        // DoorRecord(); An 8ROM door record: kind 3, collision, cell, size 14, room, landing cell + 1, facing;
        static byte[] DoorRecord(Door door, byte collision) {
            byte[] record = { 3, collision, (byte)door.X, (byte)door.Y, 0x0E, 0, 0xFF, 0xFF, 0, 0, (byte)door.DestX, (byte)door.DestY, door.Facing, 0xFF };
            WriteShort(record, 8, door.Room);
            return record;
        }

        /* Breakable blocks (NiDL collision 0x2X) -> KatAM's: star blocks 0xF0, metal 0xF4 (0x2D), bomb blocks 0xF1 (0x2E) and the blocks
         * a bomb block breaks 0xF2 (0x2F); */
        static byte BreakableBlock(int collision) => collision switch { 0x2D => 0xF4, 0x2E => 0xF1, 0x2F => 0xF2, _ => 0xF0 };

        /* BuildSolidity(); The room's (part's) SolidityMap: its RL collision and its special blocks: per breakable block, its KatAM
         * value, where it is, and what it leaves when broken (the 4 tilemap entries of the metatile NiDL's object data lists for the
         * block's number, nothing or water); then its doors (see Door); returns its address; */
        static int BuildSolidity(byte[] rom, byte[] nightmare, NightmareRoom r, byte[] cells, byte[] collision, byte[] metatiles,
                                 List<Door> doors, int noSpecials, ref int cursor) {
            int roomWidth = ReadShort(nightmare, r.Record + 0x14), data = ReadInt(nightmare, r.Record + 0x10) - 0x08000000;
            byte[] numbers = DecompressLZ77(nightmare, ReadInt(nightmare, r.Record + 0xC) - 0x08000000);
            List<byte> specials = new List<byte>();
            bool Water(int x, int y) => x >= 0 && y >= 0 && x < r.FullWidth && y < r.FullHeight && collision[y * r.FullWidth + x] is >= 0x34 and <= 0x48;

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
                    collision[y * r.FullWidth + x] = block;
                    specials.AddRange(new byte[] { 1, block, (byte)x, (byte)y, 0x12, 0, 0xFF, 0xFF });
                    for (int q = 0; q < 4; q++) specials.AddRange(BitConverter.GetBytes((ushort)ShiftPalette(ReadShort(metatiles, metatile * 8 + q * 2))));
                    specials.AddRange(new byte[] { left, 0xFF });
                }
            }

            // Doors: their cells and 8ROM records;
            foreach (Door door in doors) {
                bool water = Water(door.X, door.Y) && collision[door.Y * r.FullWidth + door.X] != 0;
                byte value = door.Edge ? (byte)(water ? 0xFC : 0xFE) : (byte)(water ? 0xFD : 0xFF);
                if (door.Slot == 0) collision[door.Y * r.FullWidth + door.X] = value;
                specials.AddRange(DoorRecord(door, value));
                if (door.Slot == 0) continue;

                /* A hidden door (a switch room's, lit by its big switch): its door object (0x97, as KatAM's switch rooms') breaks the
                 * special block of its cell once the switch's slot is set: the cell's own tiles, and the door's collision; */
                int metatile = ReadShort(cells, ((r.Y + door.Y) * roomWidth + r.X + door.X) * 4);
                specials.AddRange(new byte[] { 1, collision[door.Y * r.FullWidth + door.X], (byte)door.X, (byte)door.Y, 0x12, 0, 0xFF, 0xFF });
                for (int q = 0; q < 4; q++) specials.AddRange(BitConverter.GetBytes((ushort)ShiftPalette(ReadShort(metatiles, metatile * 8 + q * 2))));
                specials.AddRange(new byte[] { value, 0xFF });
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
         * types in the room, an empty list, how many types), and its doors' mirrors; returns its address; */
        static int BuildObjects(byte[] rom, byte[] nightmare, NightmareRoom r, List<Door> doors, int noTypes, ref int cursor) {
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
                if (miniboss) WriteShorts(template, 0x1A, -x, -y, r.FullWidth * 16, r.FullHeight * 16);
                templates.Add(template);
                if (!types.Contains(type)) types.Add(type);
            }

            // A switch room's big switch (NiDL's own aren't ported: a level has one, see LevelSlot());
            if (r.IsSwitchRoom) {
                templates.Add(BigSwitchTemplate(SwitchX, SwitchY, templates.Count, LevelSlot(r.Level), r.Level));
                types.Add(BigSwitchType);
            }

            // A mirror on each door (see Door); a hidden one is a door object (0x97, its own mirror), showing once the slot is set;
            foreach (Door door in doors.Where(d => !d.Edge)) {
                byte type = door.Slot != 0 ? DoorToHubType : MirrorType;
                templates.Add(door.Slot != 0 ? DoorToHubTemplate(door.X, door.Y, templates.Count, door.Slot, door.Design) : MirrorTemplate(door.X, door.Y, templates.Count));
                if (!types.Contains(type)) types.Add(type);
            }

            return WriteObjectList(rom, templates, types, noTypes, ref cursor);
        }

        // WriteObjectList(); A room's LevelInfo_1E4: its templates (then an end), the types in it, an empty list, how many types;
        static int WriteObjectList(byte[] rom, List<byte[]> templates, List<byte> types, int noTypes, ref int cursor) {
            int templatesAt = Allocate(ref cursor, templates.Count * 36 + 36, 4);
            for (int i = 0; i < templates.Count; i++) templates[i].CopyTo(rom, templatesAt + i * 36);
            Array.Fill(rom, (byte)0xFF, templatesAt + templates.Count * 36, 36);
            rom[templatesAt + templates.Count * 36] = rom[templatesAt + templates.Count * 36 + 1] = 0;
            int typesAt = Allocate(ref cursor, types.Count + 1, 4);
            types.Append((byte)0xFF).ToArray().CopyTo(rom, typesAt);

            int objectList = Allocate(ref cursor, 16, 4);
            WriteInts(rom, objectList, templatesAt + 0x08000000, typesAt + 0x08000000, noTypes + 0x08000000);
            rom[objectList + 12] = (byte)types.Count;
            rom[objectList + 13] = rom[objectList + 14] = rom[objectList + 15] = 0;
            return objectList;
        }

        // AllocateCompressed(); Data copied into the expansion, 4-aligned; its offset;
        static int AllocateCompressed(byte[] rom, byte[] data, ref int cursor) {
            int at = Allocate(ref cursor, data.Length, 4);
            data.CopyTo(rom, at);
            return at;
        }

        // DecompressRL(); KatAM's RL data (collision): u32 size << 8, then runs (flag bit 7: (flag & 0x7F) + 3 copies of a byte; else (flag + 1) bytes);
        static byte[] DecompressRL(byte[] rom, int address) {
            int size = ReadInt(rom, address) >> 8, at = address + 4, written = 0;
            byte[] output = new byte[size];
            while (written < size) {
                byte flag = rom[at++];
                if ((flag & 0x80) != 0) { int n = (flag & 0x7F) + 3; for (int k = 0; k < n && written < size; k++) output[written++] = rom[at]; at++; }
                else { int n = (flag & 0x7F) + 1; for (int k = 0; k < n && written < size; k++) output[written++] = rom[at++]; }
            }
            return output;
        }

        /* BuildNavigation(); The CPU helpers' zones (LevelInfo_1E8): the whole room is zone 0, its point the room's center; the route
         * lists are room 0x3C9's (generic); returns its address; */
        static int BuildNavigation(byte[] rom, NightmareRoom r, int routes, ref int cursor) {
            byte[] zones = CompressRL(new byte[r.FullWidth * r.FullHeight]);
            int zonesAt = Allocate(ref cursor, zones.Length, 4);
            zones.CopyTo(rom, zonesAt);

            int zone = Allocate(ref cursor, 0x34, 4);
            Array.Fill(rom, (byte)0xFF, zone, 0x34);
            rom[zone] = 0;
            WriteShorts(rom, zone + 0x20, r.FullWidth * 8, r.FullHeight * 8, 0x20);
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
