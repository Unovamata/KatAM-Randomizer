using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace KatAMMapTracker {
    // DoorInfo; A door in a room. Wide doors made of several objects pointing to the same room are merged in one;
    public sealed class DoorInfo {
        public int Room { get; init; }
        public int Destination { get; init; }

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

    // RoomInfo; A room and its doors as read from the ROM;
    public sealed class RoomInfo {
        public int Id { get; init; }
        public int ShortIndex { get; init; }

        // Room size in 8x8 tiles;
        public int Width { get; init; }
        public int Height { get; init; }

        public List<DoorInfo> Doors { get; } = new List<DoorInfo>();
    }

    /* RomMap; Reads the rooms, doors and room graphics of a Kirby & the Amazing Mirror (JP) ROM.
     * Since it reads the ROM data directly, it reflects the randomized mirrors of any generated seed;
     *
     * Addresses come from ReflexViewer (Documentation/Reflex JP Tools):
     * • Long room parameters: 0x903338 + room * 0x28. +0x14 tileset, +0x16 palette, +0x24 short room index.
     * • Cover data (objects): pointer table at 0xD2E74C + short * 4. Doors are objects of type 3:
     *   +2 X, +3 Y (16px units), +4 object size, +8 destination room.
     * • Room appearance: pointer table at 0xD2F93C + short * 4. +0 width, +2 height (8px tiles), +0x18 tilemap.
     * • Palettes: 0xD2E6A4 + palette * 4. Tilesets: 0xD2FDB8 + tileset * 4. Both LZ77 compressed but palettes; */
    public sealed class RomMap {
        const int LongRoomParameters = 0x903338,
                  LongRoomParameterSize = 0x28,
                  MaxLongRoom = 0x3D7,
                  MaxShortRoom = 0x11F,
                  CoverDataTable = 0xD2E74C,
                  RoomAppearanceTable = 0xD2F93C,
                  PaletteTable = 0xD2E6A4,
                  TilesetTable = 0xD2FDB8,
                  DoorObjectType = 3;

        // Central Circle, the main hub. The map is laid out from it;
        public const int CentralCircle = 0x321;

        public byte[] Rom { get; }
        public uint Crc32 { get; }
        public string GameCode { get; }
        public Dictionary<int, RoomInfo> Rooms { get; } = new Dictionary<int, RoomInfo>();

        readonly Dictionary<int, byte[]> tilesetCache = new Dictionary<int, byte[]>();

        public RomMap(byte[] rom) {
            Rom = rom;
            Crc32 = ComputeCrc32(rom);
            GameCode = System.Text.Encoding.ASCII.GetString(rom, 0xAC, 4);

            ReadRooms();
        }

        public static RomMap Load(string path) => new RomMap(File.ReadAllBytes(path));

        public bool IsSupportedGame => GameCode == "B8KJ";

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

                if (shortIndex >= MaxShortRoom) continue;

                int appearance = ReadPointer(RoomAppearanceTable + shortIndex * 4);

                if (appearance < 0) continue;

                int width = Read16(appearance), height = Read16(appearance + 2);

                if (width == 0 || height == 0) continue;

                RoomInfo info = new RoomInfo { Id = room, ShortIndex = shortIndex, Width = width, Height = height };

                ReadDoors(info);

                // Several long rooms may share a short room. The first one is the one the game uses;
                if (Rooms.Values.Any(r => r.ShortIndex == shortIndex)) continue;

                Rooms[room] = info;
            }

            LinkTwoWayDoors();
        }

        /* LinkTwoWayDoors(); A door is Two-Way when it lands Kirby next to a door leading back (doors land Kirby up to 2-3 tiles away).
         * Arriving through it reveals that door back as well; */
        void LinkTwoWayDoors() {
            foreach (RoomInfo room in Rooms.Values) {
                foreach (DoorInfo door in room.Doors) {
                    if (!Rooms.TryGetValue(door.Destination, out RoomInfo destination)) continue;

                    DoorInfo back = destination.Doors.FirstOrDefault(d => d.Destination == room.Id);

                    if (back == null) continue;

                    int landX = (door.LandX - 8) / 16, landY = (door.LandY - 8) / 16;
                    bool landsOnDoor = back.Tiles.Any(t => Math.Abs(t.x - landX) <= 3 && Math.Abs(t.y - landY) <= 3);

                    if (landsOnDoor) door.DoorBack = back;
                }
            }
        }

        void ReadDoors(RoomInfo room) {
            int header = ReadPointer(CoverDataTable + room.ShortIndex * 4);

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

        /* RenderRoom(); Renders a room with its in-game graphics as 0xAARRGGBB pixels;
         * pixelsPerTile: 8 for full size; 4, 2 or 1 average the tiles down for thumbnails; */
        public int[] RenderRoom(int roomId, int pixelsPerTile, out int width, out int height) {
            RoomInfo room = Rooms[roomId];
            width = room.Width * pixelsPerTile;
            height = room.Height * pixelsPerTile;

            int[] pixels = new int[width * height];
            int parameters = LongRoomParameters + roomId * LongRoomParameterSize;
            int tilesetIndex = Read16(parameters + 0x14), paletteIndex = Read16(parameters + 0x16);

            int paletteHeader = ReadPointer(PaletteTable + paletteIndex * 4),
                palette = paletteHeader < 0 ? -1 : ReadPointer(paletteHeader),
                appearance = ReadPointer(RoomAppearanceTable + room.ShortIndex * 4),
                tilemapAddress = appearance < 0 ? -1 : ReadPointer(appearance + 0x18);

            byte[] tileset = GetTileset(tilesetIndex);
            byte[] tilemap = tilemapAddress < 0 ? null : DecompressLZ77(tilemapAddress);

            if (palette < 0 || tileset == null || tilemap == null) return pixels;

            int[] colors = new int[256];

            for (int i = 0; i < 256; i++) {
                int color = Read16(palette + i * 2);
                int r = (color & 0x1F) * 255 / 31, g = ((color >> 5) & 0x1F) * 255 / 31, b = ((color >> 10) & 0x1F) * 255 / 31;

                colors[i] = unchecked((int)0xFF000000) | (r << 16) | (g << 8) | b;
            }

            int step = 8 / pixelsPerTile;
            int[] tile = new int[64];

            for (int i = 0; i < room.Width * room.Height && i * 2 + 1 < tilemap.Length; i++) {
                int entry = tilemap[i * 2] | (tilemap[i * 2 + 1] << 8);
                int character = entry & 0x3FF, paletteRow = entry >> 12;
                bool flipX = (entry & 0x400) != 0, flipY = (entry & 0x800) != 0;

                // Decoding the 4bpp tile;
                for (int y = 0; y < 8; y++) {
                    for (int x = 0; x < 8; x++) {
                        int offset = character * 32 + y * 4 + x / 2;
                        int value = offset < tileset.Length ? tileset[offset] : 0;
                        int index = (x & 1) != 0 ? value >> 4 : value & 0xF;

                        tile[(flipY ? 7 - y : y) * 8 + (flipX ? 7 - x : x)] = colors[paletteRow * 16 + index];
                    }
                }

                int tileX = (i % room.Width) * pixelsPerTile, tileY = (i / room.Width) * pixelsPerTile;

                // Averaging the tile down to the requested size;
                for (int y = 0; y < pixelsPerTile; y++) {
                    for (int x = 0; x < pixelsPerTile; x++) {
                        int r = 0, g = 0, b = 0;

                        for (int sy = 0; sy < step; sy++) {
                            for (int sx = 0; sx < step; sx++) {
                                int color = tile[(y * step + sy) * 8 + x * step + sx];
                                r += (color >> 16) & 0xFF;
                                g += (color >> 8) & 0xFF;
                                b += color & 0xFF;
                            }
                        }

                        int samples = step * step;
                        pixels[(tileY + y) * width + tileX + x] = unchecked((int)0xFF000000) | ((r / samples) << 16) | ((g / samples) << 8) | (b / samples);
                    }
                }
            }

            return pixels;
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
