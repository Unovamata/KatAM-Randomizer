using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace KatAMMapTracker {
    // DoorKind; What takes Kirby to another room: a mirror (door), or a Warp Star or Fuse Cannon, which work like One-Way mirrors;
    public enum DoorKind { Mirror, WarpStar, FuseCannon }

    // DoorInfo; A door in a room. Wide doors made of several objects pointing to the same room are merged in one;
    public sealed class DoorInfo {
        public int Room { get; init; }
        public int Destination { get; init; }
        public DoorKind Kind { get; init; } = DoorKind.Mirror;

        // Door position in the room, in pixels;
        public int X { get; init; }
        public int Y { get; init; }

        // Where Kirby lands in the destination room, in pixels;
        public int LandX { get; init; }
        public int LandY { get; init; }

        // Door at the other side if this is a Two-Way mirror (it lands Kirby on a door leading back), else null;
        public DoorInfo DoorBack { get; internal set; }

        // Door objects of this door, in 16px tiles;
        internal List<(int x, int y)> Tiles { get; init; }

        public string Key => DoorKey(Room, Destination);

        public static string DoorKey(int room, int destination) => $"{room:X}>{destination:X}";
    }

    // RoomObject; An object placed in a room (enemy, item, chest, switch...), as read from the room's object list;
    // Destination: the room a Warp Star (template +0x1E) or a Fuse Cannon (template +0x12) sends Kirby to, else -1; LandX, LandY: where, in tiles;
    public sealed record RoomObject(int Id, int Behavior, int X, int Y, int Destination = -1, int LandX = 0, int LandY = 0);

    // RoomInfo; A room and its doors as read from the ROM;
    public sealed class RoomInfo {
        public int Id { get; init; }
        public int ShortIndex { get; init; }

        // Its entries in the room tables (long room parameters +0x18 tilemap, +0x1A collision and doors, +0x20 objects): in the game
        // they're all ShortIndex's, but the ROM Expansion's rooms have their own (they share one doors list, ShortIndex 0x11F);
        public int TilemapIndex { get; init; }
        public int CoverIndex { get; init; }
        public int ObjectIndex { get; init; }

        // The Nightmare in Dream Land level (0-7) of a ROM Expansion room, or -1 (see RomMap.ReadExpansionRooms); its NiDL hub;
        public int NightmareLevel { get; init; } = -1;
        public bool IsNightmareHub { get; init; }

        // Room parameter +0x00; Area/theme of the room. 4 are the hubs, 0x25 the boss & goal game rooms;
        public int Area { get; init; }

        public bool IsHub => Area == 4 || IsNightmareHub;
        public bool IsPathEnder => Area == 0x25;

        // Room size in 8x8 tiles;
        public int Width { get; init; }
        public int Height { get; init; }

        public List<DoorInfo> Doors { get; } = new List<DoorInfo>();

        // Objects of the room, in the order of the ROM;
        public List<RoomObject> Objects { get; } = new List<RoomObject>();
    }

    /* ExpansionMiniboss; A mini-boss the randomizer's ROM Expansion ported from Nightmare in Dream Land: its object type (0x40-0x42), its
     * index (the behavior's high nibble picks it), its name and its sprite; */
    public sealed record ExpansionMiniboss(int Type, int Index, string Name, Sprite Icon);

    /* RomVersion; The Japanese, American and European versions of the game hold the same data at different addresses.
     * Every block of data is moved as a whole, so the Japanese addresses below are translated with the shift of their block:
     * • Level data (long room parameters): 0x84xxxx - 0x93xxxx.
     * • Pointer tables (cover data, room appearance, palettes, tilesets, graphics): 0xD2xxxx.
     * • Pause screen map graphics: 0x33F5xx - 0x3459xx; */
    public sealed record RomVersion(string GameCode, string Name, int LevelDataShift, int PointerTableShift, int MapScreenShift) {
        public static readonly RomVersion Japan = new RomVersion("B8KJ", "Japan", 0, 0, 0);
        public static readonly RomVersion USA = new RomVersion("B8KE", "USA", 0x2FE74, 0x34BE4, 0x1B844);
        public static readonly RomVersion Europe = new RomVersion("B8KP", "Europe", 0x3A2D8, 0x3F1C0, 0x20324);

        public static readonly RomVersion[] All = { Japan, USA, Europe };

        public static RomVersion FromGameCode(string gameCode) => Array.Find(All, version => version.GameCode == gameCode);
    }

    /* RomMap; Reads the rooms, doors and room graphics of a Kirby & the Amazing Mirror ROM (any region).
     * Since it reads the ROM data directly, it reflects the randomized mirrors of any generated seed;
     *
     * Addresses come from ReflexViewer (Documentation/Reflex JP Tools). They're the Japanese ones (see RomVersion):
     * • Long room parameters: 0x903338 + room * 0x28. +0x14 tileset, +0x16 palette, +0x24 short room index.
     * • Cover data (objects): pointer table at 0xD2E74C + short * 4. Doors are objects of type 3:
     *   +2 X, +3 Y (16px units), +4 object size, +8 destination room.
     * • Room appearance: pointer table at 0xD2F93C + short * 4. +0 width, +2 height (8px tiles), +0x18 tilemap.
     * • Palettes: 0xD2E6A4 + palette * 4. Tilesets: 0xD2FDB8 + tileset * 4. Both LZ77 compressed but palettes.
     * • Long room parameter +0x00: area/theme (4 = hubs, 0x25 = boss & goal game rooms). +0x1E: background.
     * • Room objects: pointer table at 0xD2EBC4 + short * 4, to a 16 byte header followed by 36 byte objects ("01 24" header):
     *   +6 X, +8 Y (pixels), +12 object ID, +14 behavior (e.g. what a chest holds); */
    public sealed class RomMap {
        const int LongRoomParameterSize = 0x28,
                  MaxLongRoom = 0x3D7,
                  MaxShortRoom = 0x11F,
                  FirstBackground = 0x614,
                  CoverObjectType = 1,
                  FillerTile = 0xF200,
                  DoorObjectType = 3,
                  WarpStarObject = 0x84,
                  FuseCannonObject = 0x8A;

        // Central Circle, the main hub. The map is laid out from it;
        public const int CentralCircle = 0x321;

        // Rooms left out of the map: 0x3C9 is an empty debug room nothing leads to (and the arena's, see IsArenaRoom);
        static readonly HashSet<int> HiddenRooms = new HashSet<int> { 0x3C9 };

        // IsArenaRoom(); The arena's rooms (0x3B6 entrance, 0x3B7 rest room, 0x3BB-0x3BD fights), left out: they're no part of exploring;
        static bool IsArenaRoom(int room) => room >= 0x3B6 && room <= 0x3BD;

        // Addresses of the Japanese version, translated to the ROM's region (an expanded ROM's copies, see ReadExpansionRooms);
        readonly int LongRoomParameters;
        int CoverDataTable,
            RoomAppearanceTable,
            PaletteTable,
            TilesetTable,
            BackgroundTable,
            ObjectListTable,
            RoomAreaTable = -1,
            NightmareHub = -1;

        public byte[] Rom { get; }
        public uint Crc32 { get; }
        public string GameCode { get; }
        public RomVersion Version { get; }
        public Dictionary<int, RoomInfo> Rooms { get; } = new Dictionary<int, RoomInfo>();

        // The ROM Expansion's mini-bosses (empty for unexpanded ROMs, see ReadExpansion);
        public List<ExpansionMiniboss> ExpansionMinibosses { get; } = new List<ExpansionMiniboss>();
        public bool IsExpanded { get; private set; }

        readonly Dictionary<int, byte[]> tilesetCache = new Dictionary<int, byte[]>();

        public RomMap(byte[] rom) {
            Rom = rom;
            Crc32 = ComputeCrc32(rom);
            GameCode = System.Text.Encoding.ASCII.GetString(rom, 0xAC, 4);
            Version = RomVersion.FromGameCode(GameCode);

            // Other games aren't read;
            if (Version == null) return;

            LongRoomParameters = 0x903338 + Version.LevelDataShift;
            CoverDataTable = 0xD2E74C + Version.PointerTableShift;
            RoomAppearanceTable = 0xD2F93C + Version.PointerTableShift;
            PaletteTable = 0xD2E6A4 + Version.PointerTableShift;
            TilesetTable = 0xD2FDB8 + Version.PointerTableShift;
            BackgroundTable = 0xD2E5F0 + FirstBackground * 4 + Version.PointerTableShift;
            ObjectListTable = 0xD2EBC8 + Version.PointerTableShift;

            ReadExpansionRooms();
            ReadRooms();
            ReadExpansion();
        }

        /* ReadExpansion(); A ROM the randomizer expanded (ROM Expansion tab) is 32 MB and ends with its map table (the last 0x100
         * bytes; written by KatAMExpansion.WriteMapTable): "NIDLMAP" and version 1, u16 entries, u16 entry size, then per mini-boss
         * its object type, index, u16 0, its icon (file offset: u16 width, u16 height, a BGR555 color per pixel, | 0x8000 where it's
         * drawn) and its name (24 bytes, ASCII); */
        const int ExpandedSize = 0x2000000, ExpansionTable = ExpandedSize - 0x100;

        void ReadExpansion() {
            if (Rom.Length != ExpandedSize || System.Text.Encoding.ASCII.GetString(Rom, ExpansionTable, 7) != "NIDLMAP" || Rom[ExpansionTable + 7] != 1) return;

            int count = Read16(ExpansionTable + 8), entrySize = Read16(ExpansionTable + 10);

            IsExpanded = true;
            for (int i = 0; i < count && entrySize >= 32 && ExpansionTable + 12 + (i + 1) * entrySize <= Rom.Length; i++) {
                int entry = ExpansionTable + 12 + i * entrySize, icon = Read32(entry + 4);
                string name = System.Text.Encoding.ASCII.GetString(Rom, entry + 8, 24).TrimEnd('\0');
                Sprite sprite = null;

                if (icon >= 0 && icon + 4 <= Rom.Length) {
                    int width = Read16(icon), height = Read16(icon + 2);

                    if (width > 0 && height > 0 && icon + 4 + width * height * 2 <= Rom.Length) {
                        int[] pixels = new int[width * height];
                        for (int p = 0; p < pixels.Length; p++) {
                            int color = Read16(icon + 4 + p * 2);
                            if ((color & 0x8000) != 0) pixels[p] = ToArgb(color & 0x7FFF);
                        }
                        sprite = new Sprite(width, height, pixels);
                    }
                }
                ExpansionMinibosses.Add(new ExpansionMiniboss(Rom[entry], Rom[entry + 1], name, sprite));
            }
        }

        /* ReadExpansionRooms(); An expanded ROM with the Nightmare in Dream Land rooms (KatAMExpansionRooms.cs) has its rooms block
         * in the map table, at +0xE0: "NR", u16 the NiDL hub's room, then pointers to the room tables the game uses (it copied them
         * with the new rooms' entries): collision and doors, tilemaps, palettes, tilesets, backgrounds, objects, and the rooms' areas
         * (+0x46: 11 + the NiDL level for the NiDL rooms); */
        void ReadExpansionRooms() {
            int block = ExpansionTable + 0xE0;
            if (Rom.Length != ExpandedSize || Rom[block] != 'N' || Rom[block + 1] != 'R') return;

            int[] tables = Enumerable.Range(0, 7).Select(i => ReadPointer(block + 4 + i * 4)).ToArray();
            if (tables.Any(t => t < 0)) return;

            NightmareHub = Read16(block + 2);
            (CoverDataTable, RoomAppearanceTable, PaletteTable, TilesetTable, BackgroundTable, ObjectListTable, RoomAreaTable) =
                (tables[0], tables[1], tables[2], tables[3], tables[4], tables[5], tables[6]);
        }

        // NightmareLevelOf(); A room's NiDL level (its area - 11), or -1;
        int NightmareLevelOf(int room) {
            int info = RoomAreaTable < 0 ? -1 : ReadPointer(RoomAreaTable + room * 4);
            int area = info < 0 || info + 0x47 > Rom.Length ? -1 : Rom[info + 0x46];
            return area >= 11 && area < 19 ? area - 11 : -1;
        }

        public static RomMap Load(string path) => new RomMap(File.ReadAllBytes(path));

        public bool IsSupportedGame => Version != null;

        int Read16(int address) => Rom[address] | (Rom[address + 1] << 8);

        int Read32(int address) => Rom[address] | (Rom[address + 1] << 8) | (Rom[address + 2] << 16) | (Rom[address + 3] << 24);

        // ReadPointer(); Reads a GBA ROM pointer (0x08xxxxxx) and returns its file offset, or -1;
        int ReadPointer(int address) {
            if (address < 0 || address + 4 > Rom.Length) return -1;

            uint pointer = (uint)Read32(address);

            if (pointer < 0x08000000 || pointer >= 0x0A000000) return -1;

            int offset = (int)(pointer & 0x01FFFFFF);

            return offset < Rom.Length ? offset : -1;
        }

        void ReadRooms() {
            for (int room = 0; room < MaxLongRoom; room++) {
                int parameters = LongRoomParameters + room * LongRoomParameterSize;
                int shortIndex = Read16(parameters + 0x24);
                // The ROM Expansion's rooms share the doors list 0x11F, past the game's;
                bool expansionRoom = RoomAreaTable >= 0 && shortIndex == MaxShortRoom;

                if (shortIndex >= MaxShortRoom && !expansionRoom) continue;

                // (in an expanded ROM every room's own entries: Central Circle's tilemap and collision have the mirror to NiDL);
                bool ownEntries = RoomAreaTable >= 0;
                int tilemapIndex = ownEntries ? Read16(parameters + 0x18) : shortIndex,
                    coverIndex = ownEntries ? Read16(parameters + 0x1A) : shortIndex,
                    objectIndex = ownEntries ? Read16(parameters + 0x20) : shortIndex;
                int appearance = ReadPointer(RoomAppearanceTable + tilemapIndex * 4);

                if (appearance < 0) continue;

                int width = Read16(appearance), height = Read16(appearance + 2);

                if (width == 0 || height == 0) continue;

                RoomInfo info = new RoomInfo {
                    Id = room, ShortIndex = shortIndex, TilemapIndex = tilemapIndex, CoverIndex = coverIndex, ObjectIndex = objectIndex,
                    Area = Read16(parameters), Width = width, Height = height, NightmareLevel = NightmareLevelOf(room), IsNightmareHub = room == NightmareHub,
                };

                ReadDoors(info);
                ReadObjects(info);

                // Several long rooms may share a short room. The first one is the one the game uses;
                if (!expansionRoom && Rooms.Values.Any(r => r.ShortIndex == shortIndex) || HiddenRooms.Contains(room) || IsArenaRoom(room)) continue;

                Rooms[room] = info;
            }

            AddLaunchers();
            LinkTwoWayDoors();
        }

        /* AddLaunchers(); Warp Stars and Fuse Cannons, as One-Way doors from where they are to the room they send Kirby to
         * (unless a mirror already leads there). Landing: Warp Star template +0x1A/+0x1C, Fuse Cannon +0x18/+0x19, in tiles; */
        void AddLaunchers() {
            foreach (RoomInfo room in Rooms.Values) {
                foreach (RoomObject obj in room.Objects.Where(o => o.Destination >= 0)) {
                    if (obj.Destination == room.Id || !Rooms.ContainsKey(obj.Destination) || room.Doors.Any(d => d.Destination == obj.Destination)) continue;

                    room.Doors.Add(new DoorInfo {
                        Room = room.Id,
                        Destination = obj.Destination,
                        Kind = obj.Id == WarpStarObject ? DoorKind.WarpStar : DoorKind.FuseCannon,
                        X = obj.X,
                        Y = obj.Y,
                        LandX = Math.Max(0, obj.LandX) * 16 + 8,
                        LandY = Math.Max(0, obj.LandY) * 16 + 8,
                        Tiles = new List<(int x, int y)> { (obj.X / 16, obj.Y / 16) },
                    });
                }
            }
        }

        /* LinkTwoWayDoors(); A door is Two-Way when it lands Kirby next to a door leading back (doors land Kirby up to 2-3 tiles away).
         * Arriving through it reveals that door back as well; */
        void LinkTwoWayDoors() {
            foreach (RoomInfo room in Rooms.Values) {
                foreach (DoorInfo door in room.Doors) {
                    if (door.Kind != DoorKind.Mirror || !Rooms.TryGetValue(door.Destination, out RoomInfo destination)) continue;

                    DoorInfo back = destination.Doors.FirstOrDefault(d => d.Destination == room.Id && d.Kind == DoorKind.Mirror);

                    if (back == null) continue;

                    int landX = (door.LandX - 8) / 16, landY = (door.LandY - 8) / 16;
                    bool landsOnDoor = back.Tiles.Any(t => Math.Abs(t.x - landX) <= 3 && Math.Abs(t.y - landY) <= 3);

                    if (landsOnDoor) door.DoorBack = back;
                }
            }
        }

        // ReadObjects(); Reads the objects of a room, until the list ends (no more "01 24" object headers);
        void ReadObjects(RoomInfo room) {
            int header = ReadPointer(ObjectListTable + room.ObjectIndex * 4), templates = header < 0 ? -1 : ReadPointer(header);

            if (templates < 0) return;

            for (int address = templates; address + 36 <= Rom.Length && room.Objects.Count < 256; address += 36) {
                if (Rom[address] != 0x01 || Rom[address + 1] != 0x24) break;

                int id = Rom[address + 12], x = Read16(address + 6), y = Read16(address + 8);

                room.Objects.Add(id == WarpStarObject ? new RoomObject(id, Rom[address + 14], x, y, Read16(address + 0x1E), (short)Read16(address + 0x1A), (short)Read16(address + 0x1C))
                               : id == FuseCannonObject ? new RoomObject(id, Rom[address + 14], x, y, Read16(address + 0x12), Rom[address + 0x18], Rom[address + 0x19])
                               : new RoomObject(id, Rom[address + 14], x, y));
            }
        }

        void ReadDoors(RoomInfo room) {
            int header = ReadPointer(CoverDataTable + room.CoverIndex * 4);

            if (header < 0) return;

            int address = ReadPointer(header + 4);

            if (address < 0) return;

            Dictionary<int, List<(int x, int y)>> doorTiles = new Dictionary<int, List<(int x, int y)>>();
            Dictionary<int, (int x, int y)> landing = new Dictionary<int, (int x, int y)>();

            for (int i = 0; i < 1024 && address + 12 <= Rom.Length; i++) {
                int size = Read16(address + 4);

                if (size == 0) break;

                if (Rom[address] == DoorObjectType) {
                    int destination = Read16(address + 8);

                    if (!doorTiles.ContainsKey(destination)) doorTiles[destination] = new List<(int x, int y)>();
                    if (!landing.ContainsKey(destination)) landing[destination] = (Rom[address + 10], Rom[address + 11]);
                    doorTiles[destination].Add((Rom[address + 2], Rom[address + 3]));
                }

                address += size;
            }

            foreach (KeyValuePair<int, List<(int x, int y)>> kvp in doorTiles) {
                // Door objects are 16x16, so the door is drawn at the center of all its objects;
                room.Doors.Add(new DoorInfo {
                    Room = room.Id,
                    Destination = kvp.Key,
                    X = (int)(kvp.Value.Average(t => t.x) * 16) + 8,
                    Y = (int)(kvp.Value.Average(t => t.y) * 16) + 8,
                    LandX = landing[kvp.Key].x * 16 + 8,
                    LandY = landing[kvp.Key].y * 16 + 8,
                    Tiles = kvp.Value,
                });
            }
        }

        // RoomPalette(); The room's 256 colors (as 0xAARRGGBB), or null if it has none;
        public int[] RoomPalette(int roomId) {
            if (!Rooms.ContainsKey(roomId)) return null;

            int paletteIndex = Read16(LongRoomParameters + roomId * LongRoomParameterSize + 0x16);
            int paletteHeader = ReadPointer(PaletteTable + paletteIndex * 4), palette = paletteHeader < 0 ? -1 : ReadPointer(paletteHeader);

            if (palette < 0 || palette + 512 > Rom.Length) return null;

            int[] colors = new int[256];
            for (int i = 0; i < 256; i++) colors[i] = ToArgb(Read16(palette + i * 2));

            return colors;
        }

        /* RenderRoom(); Renders a room with its in-game graphics, background included, as 0xAARRGGBB pixels;
         * pixelsPerTile: 8 for full size; 4, 2 or 1 average the tiles down for thumbnails.
         * includeBackground: false leaves the background out, transparent, keeping only the room's own tiles; */
        public int[] RenderRoom(int roomId, int pixelsPerTile, out int width, out int height, bool includeBackground = true) {
            RoomInfo room = Rooms[roomId];
            int fullWidth = room.Width * 8, fullHeight = room.Height * 8;
            int[] canvas = new int[fullWidth * fullHeight];

            width = room.Width * pixelsPerTile;
            height = room.Height * pixelsPerTile;

            int parameters = LongRoomParameters + roomId * LongRoomParameterSize;
            int tilesetIndex = Read16(parameters + 0x14), paletteIndex = Read16(parameters + 0x16),
                backgroundIndex = Read16(parameters + 0x1E);

            int paletteHeader = ReadPointer(PaletteTable + paletteIndex * 4),
                palette = paletteHeader < 0 ? -1 : ReadPointer(paletteHeader),
                appearance = ReadPointer(RoomAppearanceTable + room.TilemapIndex * 4),
                tilemapAddress = appearance < 0 ? -1 : ReadPointer(appearance + 0x18);

            byte[] tileset = GetTileset(tilesetIndex);
            byte[] tilemap = tilemapAddress < 0 ? null : DecompressLZ77(tilemapAddress);

            if (palette < 0 || tileset == null || tilemap == null) return Downsample(canvas, fullWidth, fullHeight, pixelsPerTile);

            int[] colors = new int[256];

            for (int i = 0; i < 256; i++) colors[i] = ToArgb(Read16(palette + i * 2));

            if (includeBackground) {
                Array.Fill(canvas, colors[0]);
                DrawBackground(canvas, fullWidth, fullHeight, backgroundIndex, colors);
            }

            DrawLayer(canvas, fullWidth, fullHeight, tileset, tilemap, room.Width, room.Height, colors, true, false);
            DrawCoverBlocks(canvas, fullWidth, fullHeight, room, tileset, colors);

            return Downsample(canvas, fullWidth, fullHeight, pixelsPerTile);
        }

        /* DrawBackground(); Draws the room's background layer (BG0 in-game), repeated to fill the room;
         * Room parameter +0x1E selects the background: entry 0x614 + index of the graphics table at 0xD2E5F0.
         * Background header: +0 width, +2 height (tiles), +8 tileset (LZ77), +0x10 palette, +0x14 first color,
         * +0x16 color count, +0x18 tilemap (uncompressed); */
        void DrawBackground(int[] canvas, int width, int height, int backgroundIndex, int[] roomColors) {
            if (backgroundIndex == 0xFFFF) return;

            int header = ReadPointer(BackgroundTable + backgroundIndex * 4);

            if (header < 0) return;

            int mapWidth = Read16(header), mapHeight = Read16(header + 2);
            int tilesAddress = ReadPointer(header + 8), paletteAddress = ReadPointer(header + 0x10),
                tilemapAddress = ReadPointer(header + 0x18);
            int firstColor = Read16(header + 0x14), colorCount = Read16(header + 0x16);

            if (mapWidth == 0 || mapHeight == 0 || tilesAddress < 0 || paletteAddress < 0 || tilemapAddress < 0) return;
            if (firstColor + colorCount > 256 || tilemapAddress + mapWidth * mapHeight * 2 > Rom.Length) return;

            byte[] tiles = DecompressLZ77(tilesAddress);

            if (tiles == null) return;

            int[] colors = (int[])roomColors.Clone();

            for (int i = 0; i < colorCount; i++) colors[firstColor + i] = ToArgb(Read16(paletteAddress + i * 2));

            byte[] tilemap = new byte[mapWidth * mapHeight * 2];
            Array.Copy(Rom, tilemapAddress, tilemap, 0, tilemap.Length);

            DrawLayer(canvas, width, height, tiles, tilemap, mapWidth, mapHeight, colors, false, true);
        }

        /* DrawCoverBlocks(); Draws the blocks placed over the tilemap (breakable blocks, switches, etc.);
         * The tilemap holds placeholder tiles under them, which look garbled if they aren't covered.
         * Cover objects are type 1 in the room's object list: +2 X, +3 Y (16px units), +8 four tilemap entries (2x2); */
        void DrawCoverBlocks(int[] canvas, int width, int height, RoomInfo room, byte[] tileset, int[] colors) {
            int header = ReadPointer(CoverDataTable + room.CoverIndex * 4);
            int address = header < 0 ? -1 : ReadPointer(header + 4);

            if (address < 0) return;

            for (int i = 0; i < 1024 && address + 16 <= Rom.Length; i++) {
                int size = Read16(address + 4);

                if (size == 0) break;

                if (Rom[address] == CoverObjectType) {
                    byte[] block = new byte[8];
                    Array.Copy(Rom, address + 8, block, 0, 8);

                    int blockX = Rom[address + 2] * 16, blockY = Rom[address + 3] * 16;

                    for (int quarter = 0; quarter < 4; quarter++) {
                        int x = blockX + quarter % 2 * 8, y = blockY + quarter / 2 * 8;

                        if (x + 8 > width || y + 8 > height) continue;

                        DrawTile(canvas, width, x, y, tileset, block[quarter * 2] | (block[quarter * 2 + 1] << 8), colors, true);
                    }
                }

                address += size;
            }
        }

        /* DrawLayer(); Draws a 4bpp tile layer;
         * transparent: color 0 of every palette row is skipped, like the GBA does for upper layers;
         * repeat: the tilemap repeats to fill the canvas (backgrounds); */
        static void DrawLayer(int[] canvas, int width, int height, byte[] tiles, byte[] tilemap, int mapWidth, int mapHeight,
                              int[] colors, bool transparent, bool repeat) {
            for (int tileY = 0; tileY < height / 8; tileY++) {
                for (int tileX = 0; tileX < width / 8; tileX++) {
                    int mapX = repeat ? tileX % mapWidth : tileX, mapY = repeat ? tileY % mapHeight : tileY;

                    if (mapX >= mapWidth || mapY >= mapHeight) continue;

                    int i = mapY * mapWidth + mapX;

                    if (i * 2 + 1 >= tilemap.Length) continue;

                    int entry = tilemap[i * 2] | (tilemap[i * 2 + 1] << 8);

                    // Filler tile (tile 0x200, palette row 15) used out of the camera's reach; It's left out so it doesn't look garbled;
                    if (transparent && (entry & 0xF3FF) == FillerTile) continue;

                    DrawTile(canvas, width, tileX * 8, tileY * 8, tiles, entry, colors, transparent);
                }
            }
        }

        // DrawTile(); Draws one 4bpp tile from a tilemap entry (tile index, flips and palette row);
        static void DrawTile(int[] canvas, int width, int left, int top, byte[] tiles, int entry, int[] colors, bool transparent) {
            int character = entry & 0x3FF, paletteRow = entry >> 12;
            bool flipX = (entry & 0x400) != 0, flipY = (entry & 0x800) != 0;

            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    int sourceX = flipX ? 7 - x : x, sourceY = flipY ? 7 - y : y;
                    int offset = character * 32 + sourceY * 4 + sourceX / 2;
                    int value = offset < tiles.Length ? tiles[offset] : 0;
                    int index = (sourceX & 1) != 0 ? value >> 4 : value & 0xF;

                    if (transparent && index == 0) continue;

                    canvas[(top + y) * width + left + x] = colors[paletteRow * 16 + index];
                }
            }
        }

        // Downsample(); Averages the full size image down to pixelsPerTile pixels per 8x8 tile;
        static int[] Downsample(int[] canvas, int width, int height, int pixelsPerTile) {
            if (pixelsPerTile >= 8) return canvas;

            int step = 8 / pixelsPerTile, outWidth = width / step, outHeight = height / step, samples = step * step;
            int[] output = new int[outWidth * outHeight];

            for (int y = 0; y < outHeight; y++) {
                for (int x = 0; x < outWidth; x++) {
                    int r = 0, g = 0, b = 0;

                    for (int sy = 0; sy < step; sy++) {
                        for (int sx = 0; sx < step; sx++) {
                            int color = canvas[(y * step + sy) * width + x * step + sx];
                            r += (color >> 16) & 0xFF;
                            g += (color >> 8) & 0xFF;
                            b += color & 0xFF;
                        }
                    }

                    output[y * outWidth + x] = unchecked((int)0xFF000000) | ((r / samples) << 16) | ((g / samples) << 8) | (b / samples);
                }
            }

            return output;
        }

        // ToArgb(); Converts a GBA BGR555 color to 0xAARRGGBB;
        public static int ToArgb(int color) {
            int r = (color & 0x1F) * 255 / 31, g = ((color >> 5) & 0x1F) * 255 / 31, b = ((color >> 10) & 0x1F) * 255 / 31;

            return unchecked((int)0xFF000000) | (r << 16) | (g << 8) | b;
        }

        byte[] GetTileset(int tilesetIndex) {
            if (tilesetCache.TryGetValue(tilesetIndex, out byte[] cached)) return cached;

            int header = ReadPointer(TilesetTable + tilesetIndex * 4),
                address = header < 0 ? -1 : ReadPointer(header);

            byte[] tileset = address < 0 ? null : DecompressLZ77(address);
            tilesetCache[tilesetIndex] = tileset;

            return tileset;
        }

        // DecompressLZ77(); GBA BIOS LZ77 (YumeDXDecode in ReflexViewer). Returns null on invalid data;
        byte[] DecompressLZ77(int address) {
            if (address + 4 > Rom.Length || Rom[address] != 0x10) return null;

            int size = Rom[address + 1] | (Rom[address + 2] << 8) | (Rom[address + 3] << 16);
            byte[] output = new byte[size];
            int read = address + 4, written = 0;

            while (written < size) {
                if (read >= Rom.Length) return null;

                int flags = Rom[read++];

                for (int bit = 0; bit < 8 && written < size; bit++, flags <<= 1) {
                    if ((flags & 0x80) == 0) {
                        if (read >= Rom.Length) return null;
                        output[written++] = Rom[read++];
                        continue;
                    }

                    if (read + 1 >= Rom.Length) return null;

                    int token = (Rom[read] << 8) | Rom[read + 1];
                    read += 2;

                    int length = (token >> 12) + 3, distance = (token & 0xFFF) + 1;

                    if (written - distance < 0) return null;

                    for (int i = 0; i < length && written < size; i++, written++) {
                        output[written] = output[written - distance];
                    }
                }
            }

            return output;
        }

        static uint ComputeCrc32(byte[] data) {
            uint[] table = new uint[256];

            for (uint i = 0; i < 256; i++) {
                uint c = i;
                for (int k = 0; k < 8; k++) c = (c & 1) != 0 ? 0xEDB88320 ^ (c >> 1) : c >> 1;
                table[i] = c;
            }

            uint crc = 0xFFFFFFFF;
            foreach (byte b in data) crc = table[(crc ^ b) & 0xFF] ^ (crc >> 8);

            return crc ^ 0xFFFFFFFF;
        }
    }
}
