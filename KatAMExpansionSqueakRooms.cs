namespace KatAMRandomizer {
    /* Squeak Squad's rooms, read from the player's own Squeak Squad ROM (see "Documentation/Squeak Squad Notes.md", "Stage rooms, for
     * porting them") and converted to KatAM's: a stage's map/aNsM.mxi lists its rooms in play order (their map, two tilesets, objects
     * and far view); a room's map (map/aNsMsK.mxp, RLE or plain) holds its size, two block tables, three block grids (the front layer,
     * the main terrain and a third, drawn over both), their collision, its zones and points, its gimmicks, its doors and its tile
     * animations. KatAM has one foreground layer of 768 tiles (BG3, its tiles at 0x06008000 up to its screen blocks at 0x0600E000) and
     * 6 palettes (0-5): the three layers are drawn into it as one picture (ConvertSqueakForeground()); */
    internal static partial class KatAMExpansion {
        // KatAM's foreground: its tiles (char block 2 up to its screen blocks) and palettes;
        const int ForegroundTiles = 768, ForegroundPalettes = 6;

        // SqueakFileAnyCase(); A file by its path, its case ignored (a stage's .mxi names "map/0d_f.bin", stored as "map/0D_f.bin");
        static byte[] SqueakFileAnyCase(byte[] rom, string path) => SqueakFile(rom, path) ?? SqueakFile(rom, path, true);

        /* SqueakUnpack(); A map file's data: NDS RLE (0x30: a flag byte, bit 7 a run of the next byte, its low bits + 3 times, else its
         * low bits + 1 bytes as they are), or stored plain ("MaD"); */
        static byte[] SqueakUnpack(byte[] packed) {
            if (packed[0] != 0x30) return packed;
            byte[] data = new byte[ReadInt(packed, 0) >> 8];
            for (int at = 4, length = 0; length < data.Length;) {
                int flag = packed[at++];
                if ((flag & 0x80) != 0) {
                    for (int n = (flag & 0x7F) + 3; n > 0 && length < data.Length; n--) data[length++] = packed[at];
                    at++;
                } else {
                    for (int n = (flag & 0x7F) + 1; n > 0 && length < data.Length; n--) data[length++] = packed[at++];
                }
            }
            return data;
        }

        // A room of a stage: its files (map, front and main tilesets, objects, far view, palette animation);
        internal sealed record SqueakRoomFiles(int Area, int Stage, int Index, string Map, string Front, string Main, string Objects, string View,
                                               string Palette = null);

        /* SqueakStageRooms(); A stage's rooms in play order, from its .mxi: 12 bytes (+8 its area, stage and room count), then chunks
         * (two letters, u16 length, value): MA starts a room (its map), FB and BB its tilesets, DE its objects, 3D its far view, PL its
         * palette animation (SqueakPaletteCycles(): the crystal caves' map/p_0f.pal and p_2b.pal). Rooms
         * whose map isn't in the ROM are left out (Nature Notch's sixth stage lists two, 12 and 13, it never had); */
        static List<SqueakRoomFiles> SqueakStageRooms(byte[] rom, int area, int stage) {
            byte[] list = SqueakFile(rom, $"map/a{area}s{stage}.mxi");
            List<SqueakRoomFiles> rooms = new List<SqueakRoomFiles>();
            if (list == null) return rooms;
            Dictionary<string, string> room = null;
            void Finish() {
                if (room != null && room.ContainsKey("MA") && SqueakFileAnyCase(rom, room["MA"]) != null)
                    rooms.Add(new SqueakRoomFiles(area, stage, rooms.Count + 1, room["MA"], room.GetValueOrDefault("FB"), room.GetValueOrDefault("BB"),
                                                  room.GetValueOrDefault("DE"), room.GetValueOrDefault("3D"), room.GetValueOrDefault("PL")));
            }
            for (int at = 12; at + 4 <= list.Length;) {
                string tag = System.Text.Encoding.ASCII.GetString(list, at, 2);
                int length = ReadShort(list, at + 2);
                string value = System.Text.Encoding.ASCII.GetString(list, at + 4, Math.Min(length, list.Length - at - 4)).Split('\0')[0];
                at += 4 + length;
                if (tag == "MA") {
                    Finish();
                    room = new Dictionary<string, string>();
                }
                if (room != null) room[tag] = value;
            }
            Finish();
            return rooms;
        }

        /* SqueakMap; A room's map: its 13 sections (offsets at +0x0C): 0 its width and height in 16-pixel blocks, 1 and 2 the block
         * tables (u16 count, then 4 tilemap entries a block), 3 the front layer's grid (table 1, the front tileset), 4 the main
         * terrain's (table 2, the main tileset), 5 the third layer's (table 2: the look a cell takes once cleared, never drawn
         * before: SqueakRoomTile()), 6-8 their collision (u32 a cell),
         * 9 zones and points, 10 gimmicks, 11 doors, 12 tile animations; */
        internal sealed class SqueakMap {
            public readonly byte[] Data;
            public readonly int Width, Height;
            readonly int[] sections = new int[13];

            public SqueakMap(byte[] data) {
                Data = data;
                for (int i = 0; i < 13; i++) sections[i] = ReadInt(data, 0x0C + i * 4);
                Width = data[sections[0]];
                Height = data[sections[0] + 1];
            }
            public int Section(int index) => sections[index];
            // The block at a cell of a grid (3, 4, 5), and a block's tilemap entry (quarter: top left, top right, bottom left, bottom right);
            public int Block(int grid, int x, int y) => ReadShort(Data, sections[grid] + (y * Width + x) * 2);
            public int TableCount(int table) => ReadShort(Data, sections[table]);
            public int Entry(int table, int block, int quarter) => ReadShort(Data, sections[table] + 2 + block * 8 + quarter * 2);
            // A cell's collision word in a grid (6, 7, 8);
            public uint Collision(int grid, int x, int y) => (uint)ReadInt(Data, sections[grid] + (y * Width + x) * 4);

            /* Doors(); Its doors (section 11: u16 count, 10 bytes a door): its number (the collision's 0x80 + n marks its cell), flags
             * (0x10 a way on, 0x90 back, 0x23 the stage's goal, 4 a hidden way: the DS draws no door there), kind (6 where Kirby
             * starts, 4 a door, 0x42 a dark doorway, 0x40, 0x41, 0x80, 0x82, 0xC0, 0xC1, 5 others: all of them entered with Up, as
             * checked in the DS emulator), its cell's middle (x, y from the top: the cell Kirby stands in), the stage's room it leads
             * to (0-based, 0xFF none: a start, a goal) and that room's door (its number); */
            public List<SqueakDoor> Doors() {
                List<SqueakDoor> doors = new List<SqueakDoor>();
                int at = sections[11];
                for (int i = 0, count = ReadShort(Data, at); i < count; i++) {
                    int door = at + 2 + i * 10;
                    doors.Add(new SqueakDoor(ReadShort(Data, door), Data[door + 2], Data[door + 3], ReadShort(Data, door + 4) / 16,
                                             ReadShort(Data, door + 6) / 16, Data[door + 8], Data[door + 9]));
                }
                return doors;
            }

            // Gimmicks(); Its gimmicks (see SqueakGimmick);
            public List<SqueakGimmick> Gimmicks() {
                List<SqueakGimmick> found = new List<SqueakGimmick>();
                int at = sections[10] + 2;
                if (sections[10] <= 0 || at + 8 > Data.Length) return found;
                int entries = ReadShort(Data, at), tables = ReadShort(Data, at + 2), entryCount = Data[at + 6], tableCount = Data[at + 7];
                for (int e = 0; e < entryCount; e++) {
                    int entry = at + entries + e * 8, type = Data[entry], index = Data[entry + 1];
                    byte[] parameters = null;
                    for (int j = 0; j < tableCount && parameters == null; j++) {
                        int table = at + ReadShort(Data, at + tables + j * 2), size = ReadShort(Data, table + 2);
                        if (Data[table] == type && index < Data[table + 1]) parameters = Data[(table + 4 + index * size)..(table + 4 + (index + 1) * size)];
                    }
                    found.Add(new SqueakGimmick(type, ReadShort(Data, entry + 2), ReadShort(Data, entry + 4), parameters));
                }
                return found;
            }

            /* Animations(); Its animated blocks (section 12: 12 bytes, the count at +10, then 12 bytes each: u16 the block, u8 its
             * frames, u8 its table, 8 bytes the frames' durations), checked against the DS's tilesets: a main tileset holds 1152
             * tiles, its blocks' entries name 1024 (10 bits), and the animated blocks' frames are the blocks after them; */
            public List<SqueakAnimation> Animations() {
                List<SqueakAnimation> animations = new List<SqueakAnimation>();
                int at = sections[12];
                if (at + 12 > Data.Length) return animations;
                for (int i = 0, count = ReadShort(Data, at + 10); i < count && at + 24 + i * 12 <= Data.Length; i++) {
                    int entry = at + 12 + i * 12, frames = Math.Min((int)Data[entry + 2], 8);
                    if (frames == 0) continue;
                    animations.Add(new SqueakAnimation(ReadShort(Data, entry), frames, Data[entry + 3],
                                                       Enumerable.Range(0, frames).Select(f => Math.Max(1, (int)Data[entry + 4 + f])).ToArray()));
                }
                return animations;
            }
        }

        /* SqueakMap.Gimmicks() (section 10, after a u16: u16 the entries' offset, u16 the tables', +6 the entries' count, +7 the
         * tables'; an entry 8 bytes: type, its index in its type's table, x, y (pixels, y from the top), a byte; a table: type, count,
         * u16 an entry's size, then its entries): a gimmick, its parameters its table's entry (null: none); */
        internal sealed record SqueakGimmick(int Type, int X, int Y, byte[] Parameters) {
            public int Word(int k) => Parameters != null && k * 2 + 2 <= Parameters.Length ? ReadShort(Parameters, k * 2) : 0;
        }

        /* A Squeak Squad tile animation (SqueakMap.Animations()): its block (of the front layer's table, layer 0, or the main
         * terrain's, 1: the main and third layers' grids) shows blocks Block + 1 to Block + Frames in turn, each for its duration
         * (frames); Block itself looks as its first; */
        internal sealed record SqueakAnimation(int Block, int Frames, int Layer, int[] Durations);

        // A Squeak Squad door (SqueakMap.Doors()): its cell, and where it leads (the stage's room, 0-based, and its door's number);
        internal sealed record SqueakDoor(int Number, int Flags, int Kind, int X, int Y, int Room, int Door);
        const int SqueakStartKind = 6, SqueakNoRoom = 0xFF;

        /* SqueakTileset; A tileset (map/NN_f.bin, NN_b.bin): u32 offsets and sizes (+0 the palettes' offset, 0x10; +4 their size, 16
         * palettes; +8 the tiles' size), the tiles (4 bits a pixel) after the palettes, then a byte a tile: its palette (the blocks'
         * tilemap entries carry palette 0, the loader takes each tile's own); */
        internal sealed class SqueakTileset {
            public readonly ushort[] Colors;
            public readonly byte[] Tiles, TilePalettes;
            public int Count => Tiles.Length / 32;

            public SqueakTileset(byte[] file) {
                int palettes = ReadInt(file, 0), tiles = palettes + ReadInt(file, 4), size = ReadInt(file, 8);
                Colors = new ushort[256];
                for (int i = 0; i < 256; i++) Colors[i] = (ushort)ReadShort(file, palettes + i * 2);
                Tiles = file[tiles..(tiles + size)];
                TilePalettes = file[(tiles + size)..Math.Min(file.Length, tiles + size + size / 32)];
            }
            // A tile's pixel (its 4-bit index, 0 transparent) and color (one the room's palette animation changes: SqueakCycled());
            public int Pixel(int tile, int x, int y) => Tiles[tile * 32 + y * 4 + x / 2] >> (x % 2 * 4) & 15;
            public int Color(int tile, int index) {
                int slot = (tile < TilePalettes.Length ? TilePalettes[tile] & 15 : 0) * 16 + index;
                return cycled != null && cycled[slot] != 0 ? cycled[slot] : Colors[slot] & 0x7FFF;
            }
            // The room's palette animation (SqueakPaletteCycles()): the colors it changes, each one of its own (SqueakCycled());
            int[] cycled;
            public List<SqueakPaletteCycle> Cycles { get; private set; }
            public void Cycle(List<SqueakPaletteCycle> cycles) {
                Cycles = cycles;
                cycled = new int[256];
                foreach (SqueakPaletteCycle cycle in cycles ?? new List<SqueakPaletteCycle>())
                    for (int index = 1; index < 16; index++)
                        if (cycle.Frames.Any(f => f[index] != cycle.Frames[0][index])) cycled[cycle.Row * 16 + index] = SqueakCycled(cycle.Row * 16 + index, cycle.Frames[0][index]);
            }
        }

        /* SqueakRoomTileset(); A room's tileset (its front or main, by name): one with no colors of its own (each palette's color 0 alone:
         * 4-3-7's, 4-3-8's and 8-1-6's, 31_*, 32_*, 33_*) takes the room's PL palette (map/pa_NN.pal, kind 2: u16 2, then 256 colors),
         * which the DS loads as the BG palette there (read in the DS emulator, 4-3-7 loaded with its own stage entry: the BG palette is
         * pa_19.pal's colors); the boss rooms' PL palettes of that kind (the light fades' targets) leave their tilesets' colors as they are; */
        static SqueakTileset SqueakRoomTileset(byte[] squeak, string name, SqueakRoomFiles f) {
            if (name == null || SqueakFileAnyCase(squeak, name) is not byte[] file) return null;
            SqueakTileset tileset = new SqueakTileset(file);
            if (Enumerable.Range(0, 256).All(i => i % 16 == 0 || (tileset.Colors[i] & 0x7FFF) == 0) && f.Palette is string palette
                && SqueakFileAnyCase(squeak, palette) is byte[] colors && colors.Length >= 2 + 512 && ReadShort(colors, 0) == 2)
                for (int i = 0; i < 256; i++) tileset.Colors[i] = (ushort)ReadShort(colors, 2 + i * 2);
            return tileset;
        }

        /* A palette animation's row (SqueakPaletteCycles()): the main tileset's palette it changes, how long each frame stays (the DS's
         * frames, as KatAM's) and each frame's 16 colors; */
        internal sealed record SqueakPaletteCycle(int Row, int[] Durations, ushort[][] Frames);

        /* SqueakPaletteCycles(); A room's palette animation (its .mxi's PL, map/p_NN.pal; the boss rooms' pa_NN.pal light fades, another
         * kind, aren't these): u16 its kind (1), u16 how many rows, then each row: u8 the main tileset's palette it changes, u8 its frame
         * count, a byte a frame (how long it stays), to an even length, then each frame's 16 colors. The crystal caves (0F_b.bin's
         * palettes 6-8, 2B_b.bin's 7-8) glow: their purple rims pulse and their sparkles turn; null for any other kind; */
        static List<SqueakPaletteCycle> SqueakPaletteCycles(byte[] file) {
            if (file == null || file.Length < 4 || ReadShort(file, 0) != 1) return null;
            List<SqueakPaletteCycle> cycles = new List<SqueakPaletteCycle>();
            int at = 4;
            for (int row = 0; row < ReadShort(file, 2); row++) {
                int palette = file[at], frames = file[at + 1];
                int[] durations = Enumerable.Range(0, frames).Select(f => (int)file[at + 2 + f]).ToArray();
                at += 2 + frames;
                at += at & 1;
                ushort[][] colors = Enumerable.Range(0, frames).Select(f => Enumerable.Range(0, 16).Select(i => (ushort)(ReadShort(file, at + (f * 16 + i) * 2) & 0x7FFF)).ToArray()).ToArray();
                at += frames * 32;
                cycles.Add(new SqueakPaletteCycle(palette, durations, colors));
            }
            return cycles;
        }

        /* SqueakCycled(); A color the room's palette animation changes, as a color of the converted pictures: its own (never merged with
         * another), its main tileset palette's slot above the color bits (so a KatAM palette lists a row's cycled colors together, after
         * its others), its first frame's color in the color bits (what SqueakColorDistance() and the palettes read); */
        const int SqueakCycledBit = 1 << 24;
        static int SqueakCycled(int slot, int color) => SqueakCycledBit | slot << 15 | color & 0x7FFF;

        /* A converted foreground: KatAM's tiles (4 bits a pixel, tile 0 empty), its 6 palettes, its tilemap (a row of 8-pixel tiles at
         * a time), how far it was reduced (colors merged, tiles merged to fit), and the 4 tilemap entries (top left, top right, bottom
         * left, bottom right) of each block asked for drawn without its front layer (a star block's cell once it's broken: the block
         * is the front layer's; a bomb chain's without its main terrain too); */
        internal sealed record SqueakForeground(byte[] Tiles, ushort[] Palettes, ushort[] Tilemap, int Columns, int Rows, int MergedColors, int MergedTiles,
                                                ushort[][] Bare = null, List<SqueakTileAnimation> Animations = null, List<SqueakColorCycle> Cycles = null);

        // A converted palette animation (KatAM's ForegroundPalette cycles): Count colors from FirstColor on, each frame's and how long it stays;
        internal sealed record SqueakColorCycle(int FirstColor, int Count, int[] Durations, ushort[][] Frames);
        // (KatAM's rooms keep at most 8: LevelInfo's unk63C[8]);
        const int SqueakMaxColorCycles = 8;

        // A converted animation: its tiles (FirstTile on, Tiles of them, frame 0's in the foreground's tiles), each frame's tiles (4 bits
        // a pixel) and duration;
        internal sealed record SqueakTileAnimation(int FirstTile, int Tiles, byte[][] Frames, int[] Durations);

        /* SqueakRoomTile(); A room's 8-pixel tile at (tx, ty) as one picture, the layers drawn in the DS's order: the main terrain
         * (but `cleared`: a bomb chain's broken cell), the front layer over it (but `bare`); the third layer only where `cleared`,
         * the look the DS swaps in for both once the cell's cleared. The DS draws grids 3 and 4 alone (BG2 and BG3; BG1 is off in its
         * stages, checked in the DS emulator in 4-6-3, 8-3-2 and 2-2-1: every cell in view where grids 4 and 5 differ shows grid 4,
         * an empty grid 4 shows nothing; 4-6-3's column a trigger area cleared shows grid 5). Drawn over both (as before), it hid
         * 4-6-3's bomb blocks and the grey floor they break behind its bricks, and the planks nailed over its sealed door; */
        static int[] SqueakRoomTile(SqueakMap map, SqueakTileset front, SqueakTileset main, int tx, int ty, bool bare = false,
                                    IReadOnlyDictionary<(int layer, int block), int> shown = null, bool cleared = false) {
            int[] pixels = Enumerable.Repeat(-1, 64).ToArray();
            int quarter = (ty & 1) * 2 + (tx & 1);
            foreach ((int grid, int table, SqueakTileset tiles) in new[] { (4, 2, main), (3, 1, front), (5, 2, main) }) {
                if (tiles == null || bare && grid == 3 || cleared && grid == 4 || !cleared && grid == 5) continue;
                int block = map.Block(grid, tx / 2, ty / 2);
                // (an animated block drawn as one of its frames: `shown`);
                if (shown != null && shown.TryGetValue((grid == 3 ? 0 : 1, block), out int frame)) block = frame;
                if (block >= map.TableCount(table)) continue;
                int entry = map.Entry(table, block, quarter), tile = entry & 0x3FF;
                if (tile >= tiles.Count) continue;
                bool hflip = (entry & 0x400) != 0, vflip = (entry & 0x800) != 0;
                for (int y = 0; y < 8; y++)
                    for (int x = 0; x < 8; x++) {
                        int index = tiles.Pixel(tile, hflip ? 7 - x : x, vflip ? 7 - y : y);
                        if (index != 0) pixels[y * 8 + x] = tiles.Color(tile, index);
                    }
            }
            return pixels;
        }

        /* SqueakSigns(); The cells of a room's tutorial signs ("Tap to pop!", "Find treasure and move on.", "Mix bubbles!": Prism Plains 2's
         * first, third and fifth rooms), left out (the owner: KatAM skips them): front-layer blocks without collision in a block at least
         * 10 by 6 cells, 4 in 5 of them filled (only those three in the 8 worlds, 16 by 11 each); */
        /* SqueakGoalDoorways(); The DS's goal doors' wide black doorways taken out of a room's tiles (the owner: "I think we can drop the
         * wide black doors and replace them with regular mirrors since these look very odd in-game"): a doorway is 2 by 2 blocks, two
         * pairs (b, b + 1 over c, c + 1), on the door's cell and the one left of it and the row above; 43 of the DS's are in the front
         * grid (grid 3: 738, 739 over 770, 771 in every front block table), 4-4-9's in the third (grid 5), both cleared there (the
         * main grid's tiles show); 1-2-5's is in the main grid (grid 4: a cave in its cliff), whose blocks take the ones right of
         * it (the cliff's). The door's mirror stands on its cell; */
        static SqueakMap SqueakGoalDoorways(SqueakMap map) {
            byte[] data = null;
            bool Doorway(int grid, int x, int y) {
                if (x < 1 || y < 1 || x >= map.Width || y >= map.Height) return false;
                int b = map.Block(grid, x - 1, y - 1), c = map.Block(grid, x - 1, y);
                return b != 0 && c != 0 && map.Block(grid, x, y - 1) == b + 1 && map.Block(grid, x, y) == c + 1;
            }
            void Set(int grid, int x, int y, int block) {
                data ??= (byte[])map.Data.Clone();
                WriteShort(data, map.Section(grid) + (y * map.Width + x) * 2, block);
            }
            foreach (SqueakDoor door in map.Doors().Where(SqueakGoal)) {
                int x = door.X, y = door.Y;
                int grid = Doorway(3, x, y) ? 3 : Doorway(5, x, y) ? 5 : Doorway(4, x, y) && x + 1 < map.Width ? 4 : -1;
                if (grid < 0) continue;
                for (int row = y - 1; row <= y; row++)
                    for (int column = x - 1; column <= x; column++)
                        Set(grid, column, row, grid == 4 ? map.Block(4, x + 1, row) : 0);
            }
            return data == null ? map : new SqueakMap(data);
        }

        static HashSet<(int x, int y)> SqueakSigns(SqueakMap map) {
            HashSet<(int x, int y)> cells = new HashSet<(int, int)>(), signs = new HashSet<(int, int)>(), seen = new HashSet<(int, int)>();
            for (int y = 0; y < map.Height; y++)
                for (int x = 0; x < map.Width; x++)
                    if (map.Block(3, x, y) != 0 && map.Collision(6, x, y) == 0) cells.Add((x, y));
            foreach ((int x, int y) start in cells) {
                if (!seen.Add(start)) continue;
                List<(int x, int y)> part = new List<(int, int)>();
                Stack<(int x, int y)> stack = new Stack<(int, int)>(new[] { start });
                while (stack.Count > 0) {
                    (int x, int y) = stack.Pop();
                    part.Add((x, y));
                    foreach ((int, int) next in new[] { (x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1) })
                        if (cells.Contains(next) && seen.Add(next)) stack.Push(next);
                }
                int width = part.Max(c => c.x) - part.Min(c => c.x) + 1, height = part.Max(c => c.y) - part.Min(c => c.y) + 1;
                if (width >= 10 && height >= 6 && part.Count * 5 > width * height * 4) signs.UnionWith(part);
            }
            return signs;
        }

        // SqueakColorDistance(); How far apart two BGR555 colors look (weighted RGB, as the eye sees green most, blue least);
        static int SqueakColorDistance(int a, int b) {
            int r = (a & 31) - (b & 31), g = (a >> 5 & 31) - (b >> 5 & 31), bl = (a >> 10 & 31) - (b >> 10 & 31);
            return 3 * r * r + 4 * g * g + 2 * bl * bl;
        }

        /* SqueakPackPalettes(); The color sets (each tile's colors) packed into at most `palettes` palettes of 15 colors: the biggest
         * sets first, each into the palette it adds fewest colors to; null if they don't fit. That order failing, others are tried
         * (the sets of one size shuffled, SqueakPackTries of them, fixed seeds: the same ROM every time) before the converter merges
         * colors (ConvertSqueakForeground()): 7-1-3's 52 colors took 19 merges with the first order alone; */
        const int SqueakPackTries = 24;
        static List<HashSet<int>> SqueakPackPalettes(IEnumerable<HashSet<int>> sets, int palettes) {
            List<HashSet<int>> all = sets.Where(s => s.Count > 0).ToList();
            if (all.Any(s => s.Count > 15)) return null;
            List<HashSet<int>> ordered = all.OrderByDescending(s => s.Count).ToList();
            if (SqueakPackOrder(ordered, palettes) is List<HashSet<int>> first) return first;
            for (int seed = 1; seed <= SqueakPackTries; seed++) {
                Random random = new Random(seed);
                List<HashSet<int>> shuffled = all.Select(s => (s, k: random.Next())).OrderByDescending(e => e.s.Count).ThenBy(e => e.k).Select(e => e.s).ToList();
                if (SqueakPackOrder(shuffled, palettes) is List<HashSet<int>> packed) return packed;
            }
            return null;
        }

        static List<HashSet<int>> SqueakPackOrder(List<HashSet<int>> sets, int palettes) {
            List<HashSet<int>> packed = new List<HashSet<int>>();
            foreach (HashSet<int> set in sets) {
                if (set.Count > 15) return null;
                HashSet<int> best = null;
                int added = int.MaxValue;
                foreach (HashSet<int> palette in packed) {
                    int extra = set.Count(c => !palette.Contains(c));
                    if (palette.Count + extra <= 15 && extra < added) { best = palette; added = extra; }
                }
                if (best == null) {
                    if (packed.Count == palettes) return null;
                    packed.Add(best = new HashSet<int>());
                }
                best.UnionWith(set);
            }
            return packed;
        }

        /* SqueakFitPalettes(); A room whose colors only fit KatAM's palettes merged (ConvertSqueakForeground(): a merged color changes
         * every tile it's in) fitted again tile by tile: each set of colors (a tile's, an animated slot's; its colors' pixels counted)
         * drawn with the palette that shows it closest (each of its colors the closest that palette has: SqueakColorDistance(), times
         * its pixels), each palette then the colors of its sets (past 15, the closest two made one, the less used dropped), round after
         * round from the merged palettes (`start`) until the sets stay put, the closest round kept. A palette no set takes starts again
         * from the set shown worst. A color the palette animation changes (SqueakCycled()) is never shown as another, nor another as
         * it, and no more DS rows' cycles than KatAM keeps (SqueakMaxColorCycles). Measured over the 89 rooms that need merges (each
         * pixel's color against the DS's, SqueakColorDistance()): shown far off (past 200), 37,563 pixels merged, 163 fitted; past a
         * near shade (60), 517,416 merged, 79,716 fitted (SqueakFitFar 8: fewer far ones than at 32 or 128). Returns the palettes and
         * each set's; */
        const int SqueakFitRounds = 40, SqueakFitFar = 8, SqueakNearShade = 60;
        static (List<HashSet<int>> palettes, int[] setPalette) SqueakFitPalettes(List<Dictionary<int, long>> sets, List<HashSet<int>> start) {
            static bool Cycled(int c) => c >= SqueakCycledBit;
            // (a color shown as another: its pixels times how far, a far one more than its distance, so a few pixels of an accent
            // aren't given up for many of a near shade);
            static long Far(int d) => d + (long)d * d / SqueakFitFar;
            long Cost(Dictionary<int, long> set, HashSet<int> palette) {
                long sum = 0;
                foreach ((int c, long pixels) in set) {
                    if (palette.Contains(c)) continue;
                    if (Cycled(c)) return long.MaxValue;
                    int closest = int.MaxValue;
                    foreach (int x in palette) if (!Cycled(x)) closest = Math.Min(closest, SqueakColorDistance(c, x));
                    if (closest == int.MaxValue) return long.MaxValue;
                    sum += pixels * Far(Math.Max(1, closest));
                }
                return sum;
            }
            // (a palette's colors, past 15 the closest two made one: the less used dropped; null if its animated colors alone don't fit);
            static HashSet<int> Reduce(Dictionary<int, long> colors) {
                Dictionary<int, long> left = new Dictionary<int, long>(colors);
                while (left.Count > 15) {
                    List<int> plain = left.Keys.Where(c => !Cycled(c)).ToList();
                    (int a, int b, long d) closest = (-1, -1, long.MaxValue);
                    for (int i = 0; i < plain.Count; i++)
                        for (int j = i + 1; j < plain.Count; j++) {
                            long d = Math.Min(left[plain[i]], left[plain[j]]) * Far(Math.Max(1, SqueakColorDistance(plain[i], plain[j])));
                            if (d < closest.d) closest = (plain[i], plain[j], d);
                        }
                    if (closest.a < 0) return null;
                    (int keep, int drop) = left[closest.a] >= left[closest.b] ? (closest.a, closest.b) : (closest.b, closest.a);
                    left[keep] += left[drop];
                    left.Remove(drop);
                }
                return left.Keys.ToHashSet();
            }
            static int Cycles(List<HashSet<int>> palettes) => palettes.Sum(p => p.Where(Cycled).Select(c => (c >> 15 & 0xFF) >> 4).Distinct().Count());

            List<HashSet<int>> palettes = start.Select(p => p.ToHashSet()).ToList();
            while (palettes.Count < ForegroundPalettes) palettes.Add(new HashSet<int>());
            (List<HashSet<int>> palettes, int[] setPalette, long cost) best = (null, null, long.MaxValue);
            int[] last = null;
            for (int round = 0; round < SqueakFitRounds; round++) {
                // Each set to the palette that shows it closest (the first of those alike);
                int[] now = new int[sets.Count];
                long[] costs = new long[sets.Count];
                long total = 0;
                for (int s = 0; s < sets.Count; s++) {
                    costs[s] = long.MaxValue;
                    for (int p = 0; p < palettes.Count; p++) {
                        long cost = Cost(sets[s], palettes[p]);
                        if (cost < costs[s]) (costs[s], now[s]) = (cost, p);
                    }
                    if (costs[s] == long.MaxValue) { total = long.MaxValue; break; }
                    total += costs[s];
                }
                if (total == long.MaxValue) break;
                if (total < best.cost && Cycles(palettes) <= SqueakMaxColorCycles) best = (palettes.Select(p => p.ToHashSet()).ToList(), now, total);
                if (total == 0 || (last != null && now.SequenceEqual(last))) break;
                last = now;
                // Each palette its sets' colors;
                List<HashSet<int>> next = new List<HashSet<int>>();
                HashSet<int> seeded = new HashSet<int>();
                for (int p = 0; p < palettes.Count; p++) {
                    Dictionary<int, long> colors = new Dictionary<int, long>();
                    for (int s = 0; s < sets.Count; s++)
                        if (now[s] == p)
                            foreach ((int c, long pixels) in sets[s]) colors[c] = colors.GetValueOrDefault(c) + pixels;
                    if (colors.Count == 0) {
                        int worst = Enumerable.Range(0, sets.Count).Where(s => !seeded.Contains(s) && sets[s].Count is > 0 and <= 15)
                                              .OrderByDescending(s => costs[s]).DefaultIfEmpty(-1).First();
                        if (worst >= 0 && costs[worst] > 0) {
                            seeded.Add(worst);
                            foreach ((int c, long pixels) in sets[worst]) colors[c] = pixels;
                        }
                    }
                    next.Add(Reduce(colors) ?? palettes[p]);
                }
                palettes = next;
            }
            return best.palettes == null ? (null, null) : (best.palettes, best.setPalette);
        }

        /* ConvertSqueakForeground(); A room's three layers as KatAM's one foreground: each 8-pixel tile drawn whole (SqueakRoomTile()),
         * the same pictures (flipped too) one tile; the animated ones a tile of their own, with every frame (Animations). Its colors go into KatAM's 6 palettes of 15 (SqueakPackPalettes()); where they
         * don't fit, the two closest colors become one (the less used takes the other's) until they do, then the colors those merges
         * moved far are fitted again tile by tile (SqueakFitPalettes()). Past KatAM's 768 tiles (three
         * rooms, the busiest), the two most alike tiles of a palette become one (the less used takes the other's place) until they
         * fit (the owner allows compressing tilesets); */
        static SqueakForeground ConvertSqueakForeground(SqueakMap map, SqueakTileset front, SqueakTileset main, int maxTiles = ForegroundTiles,
                                                        IReadOnlyList<(int x, int y)> bare = null, IReadOnlySet<(int x, int y)> signs = null,
                                                        IReadOnlySet<(int x, int y)> cleared = null) {
            int columns = map.Width * 2, rows = map.Height * 2;
            // The pictures, and each tile's picture (flips found later, once the colors are final), the bare blocks' after the room's;
            List<int[]> pictures = new List<int[]>();
            Dictionary<string, int> pictureOf = new Dictionary<string, int>();
            int Picture(int[] pixels) {
                string key = string.Join(",", pixels);
                if (!pictureOf.TryGetValue(key, out int index)) {
                    pictureOf[key] = index = pictures.Count;
                    pictures.Add(pixels);
                }
                return index;
            }
            /* The animated cells (map.Animations()): a cell showing an animated block moves with that block's timing (its durations;
             * its other animated blocks of the same timing move with it, any other stays as it is). Each of its tiles drawn at each
             * frame: one whose frames all look the same is a picture as any other; the others are slots of their timing (the same
             * frames one slot), each a tile of its own (no flips), all its frames drawn with one palette; */
            List<SqueakAnimation> animations = map.Animations();
            Dictionary<(int layer, int block), int> animationOf = new Dictionary<(int, int), int>();
            for (int k = 0; k < animations.Count; k++) animationOf.TryAdd((animations[k].Layer, animations[k].Block), k);
            List<int[]> timings = new List<int[]>();
            List<List<int[][]>> slots = new List<List<int[][]>>();     // (each timing's slots: their frames' pictures);
            Dictionary<string, (int timing, int slot)> slotOf = new Dictionary<string, (int, int)>();
            List<int> slotUses = new List<int>();
            (int timing, int slot)[] cellSlot = new (int, int)[columns * rows];

            int[] cellPicture = new int[columns * rows];
            for (int ty = 0; ty < rows; ty++)
                for (int tx = 0; tx < columns; tx++) {
                    bool signed = signs != null && signs.Contains((tx / 2, ty / 2));
                    cellSlot[ty * columns + tx] = (-1, -1);
                    List<int> moving = new List<int>();
                    if (animationOf.Count > 0)
                        foreach ((int grid, int layer) in new[] { (4, 1), (3, 0) })
                            if (!(grid == 3 && signed) && animationOf.TryGetValue((layer, map.Block(grid, tx / 2, ty / 2)), out int k) && !moving.Contains(k)) moving.Add(k);
                    if (moving.Count > 0) {
                        int[] durations = animations[moving[0]].Durations;
                        moving.RemoveAll(k => !animations[k].Durations.SequenceEqual(durations));
                        int[][] frames = Enumerable.Range(0, durations.Length).Select(f => SqueakRoomTile(map, front, main, tx, ty, signed,
                            moving.ToDictionary(k => (animations[k].Layer, animations[k].Block), k => animations[k].Block + 1 + f))).ToArray();
                        if (frames.Any(f => !f.SequenceEqual(frames[0]))) {
                            int timing = timings.FindIndex(t => t.SequenceEqual(durations));
                            if (timing < 0) {
                                timing = timings.Count;
                                timings.Add(durations);
                                slots.Add(new List<int[][]>());
                            }
                            string key = timing + ":" + string.Join("/", frames.Select(f => string.Join(",", f)));
                            if (!slotOf.TryGetValue(key, out var slot)) {
                                slotOf[key] = slot = (timing, slots[timing].Count);
                                slots[timing].Add(frames);
                                slotUses.Add(0);
                            }
                            cellSlot[ty * columns + tx] = slot;
                            cellPicture[ty * columns + tx] = -1;
                            continue;
                        }
                    }
                    cellPicture[ty * columns + tx] = Picture(SqueakRoomTile(map, front, main, tx, ty, signed));
                }
            List<(int timing, int slot)> allSlots = slots.SelectMany((list, t) => list.Select((_, i) => (t, i))).ToList();
            int slotTiles = allSlots.Count;
            if (slotTiles >= maxTiles / 2) throw new InvalidOperationException("A Squeak Squad room's animated tiles don't fit KatAM's foreground.");
            Dictionary<(int, int), int> slotIndex = allSlots.Select((s, i) => (s, i)).ToDictionary(p => p.s, p => p.i);
            int[] slotCount = new int[slotTiles];
            foreach ((int timing, int slot) c in cellSlot) if (c.timing >= 0) slotCount[slotIndex[c]]++;
            bare ??= new List<(int, int)>();
            int[] barePicture = bare.SelectMany(b => Enumerable.Range(0, 4).Select(q => Picture(SqueakRoomTile(map, front, main, b.x * 2 + (q & 1), b.y * 2 + q / 2, true, null, cleared?.Contains(b) == true)))).ToArray();
            int[] uses = new int[pictures.Count];
            foreach (int p in cellPicture.Concat(barePicture)) if (p >= 0) uses[p]++;

            // Colors: merged, the closest pair at a time, until every tile's set fits a palette and the sets fit KatAM's 6;
            Dictionary<int, int> colorUses = new Dictionary<int, int>();
            for (int p = 0; p < pictures.Count; p++)
                foreach (int c in pictures[p].Where(c => c >= 0)) colorUses[c] = colorUses.GetValueOrDefault(c) + uses[p];
            for (int i = 0; i < slotTiles; i++)
                foreach (int c in slots[allSlots[i].timing][allSlots[i].slot].SelectMany(f => f).Where(c => c >= 0)) colorUses[c] = colorUses.GetValueOrDefault(c) + slotCount[i];
            Dictionary<int, int> merged = colorUses.Keys.ToDictionary(c => c, c => c);
            int Final(int c) { while (merged[c] != c) c = merged[c]; return c; }
            // (a color merged only into near shades: the last of them, SqueakFitPalettes() leaving those merges as they are);
            Dictionary<int, int> mergedHow = new Dictionary<int, int>();
            int Near(int c) { while (merged[c] != c && mergedHow[c] <= SqueakNearShade) c = merged[c]; return c; }
            // (a slot's colors: all its frames', as one palette draws them all);
            List<HashSet<int>> Sets() => pictures.Select(p => p.Where(c => c >= 0).Select(Final).ToHashSet())
                .Concat(allSlots.Select(s => slots[s.timing][s.slot].SelectMany(f => f).Where(c => c >= 0).Select(Final).ToHashSet()))
                .Distinct(HashSet<int>.CreateSetComparer()).ToList();
            List<HashSet<int>> palettes;
            int mergedColors = 0;
            while ((palettes = SqueakPackPalettes(Sets(), ForegroundPalettes)) == null) {
                // (a color the palette animation changes is never merged: SqueakCycled());
                List<int> live = colorUses.Keys.Where(c => merged[c] == c && c < SqueakCycledBit).ToList();
                (int a, int b, int d) closest = (-1, -1, int.MaxValue);
                for (int i = 0; i < live.Count; i++)
                    for (int j = i + 1; j < live.Count; j++) {
                        int d = SqueakColorDistance(live[i], live[j]);
                        if (d < closest.d) closest = (live[i], live[j], d);
                    }
                if (closest.a < 0) throw new InvalidOperationException("A Squeak Squad room's colors don't fit KatAM's palettes.");
                (int keep, int drop) = colorUses[closest.a] >= colorUses[closest.b] ? (closest.a, closest.b) : (closest.b, closest.a);
                merged[drop] = keep;
                mergedHow[drop] = closest.d;
                colorUses[keep] += colorUses[drop];
                mergedColors++;
            }
            /* Each set of colors (a picture's; an animated slot's, all its frames'), its colors' pixels counted; merged colors fitted
             * again tile by tile (SqueakFitPalettes()): each set its palette, each color the closest its palette has (Shown()); */
            List<Dictionary<int, long>> setColors = new List<Dictionary<int, long>>();
            Dictionary<string, int> setOf = new Dictionary<string, int>();
            int SetOf(IEnumerable<int> colors) {
                string key = string.Join(",", colors.Where(c => c >= 0).Distinct().OrderBy(c => c));
                if (!setOf.TryGetValue(key, out int s)) {
                    setOf[key] = s = setColors.Count;
                    setColors.Add(new Dictionary<int, long>());
                }
                return s;
            }
            void Count(int s, IEnumerable<int> pixels, long times) {
                foreach (int c in pixels.Where(c => c >= 0)) setColors[s][c] = setColors[s].GetValueOrDefault(c) + times;
            }
            int[] pictureSet = new int[pictures.Count];
            int[] NearPixels(int[] pixels) => pixels.Select(c => c < 0 ? c : Near(c)).ToArray();
            for (int p = 0; p < pictures.Count; p++) Count(pictureSet[p] = SetOf(NearPixels(pictures[p])), NearPixels(pictures[p]), uses[p]);
            int[] slotSet = new int[slotTiles];
            for (int i = 0; i < slotTiles; i++) {
                int[][] frames = slots[allSlots[i].timing][allSlots[i].slot].Select(NearPixels).ToArray();
                slotSet[i] = SetOf(frames.SelectMany(f => f));
                foreach (int[] frame in frames) Count(slotSet[i], frame, slotCount[i]);
            }
            int[] setPalette = null;
            /* (only where a merge shows a pixel more than a near shade off, SqueakNearShade; and the merges within near shades kept,
             * Near(): the fit keeps tiles apart that a merge made one, ROM room the European version is short of: fitting every
             * merged color took 43 KB more, past the 42 KB it had left; this way 9 KB, and a near shade's merge shows no difference); */
            long FarMerged() {
                long far = 0;
                for (int p = 0; p < pictures.Count; p++)
                    foreach (int c in pictures[p]) if (c >= 0 && SqueakColorDistance(c, Final(c)) > SqueakNearShade) far += uses[p];
                return far;
            }
            if (mergedColors > 0 && FarMerged() > 0) {
                (List<HashSet<int>> fitted, int[] fittedSets) = SqueakFitPalettes(setColors, palettes);
                if (fitted != null) (palettes, setPalette) = (fitted, fittedSets);
            }
            Dictionary<(int, int), int> shown = new Dictionary<(int, int), int>();
            int Shown(int palette, int c) {
                if (setPalette == null) return Final(c);
                c = Near(c);
                if (palettes[palette].Contains(c)) return c;
                if (!shown.TryGetValue((palette, c), out int x))
                    shown[(palette, c)] = x = palettes[palette].Where(k => k < SqueakCycledBit).OrderBy(k => SqueakColorDistance(c, k)).ThenBy(k => k).First();
                return x;
            }
            int PaletteOf(int set) => setPalette?[set] ?? Math.Max(0, palettes.FindIndex(p => setColors[set].Keys.Select(Final).ToHashSet().IsSubsetOf(p)));

            // Each picture in KatAM's colors: its palette (the first holding all its colors) and indexes (0 transparent);
            ushort[] paletteColors = new ushort[ForegroundPalettes * 16];
            List<List<int>> paletteLists = palettes.Select(p => p.OrderBy(c => c).ToList()).ToList();
            for (int p = 0; p < paletteLists.Count; p++)
                for (int c = 0; c < paletteLists[p].Count; c++) paletteColors[p * 16 + 1 + c] = (ushort)(paletteLists[p][c] & 0x7FFF);
            /* The palette animation (main.Cycles): in each KatAM palette, each DS row's cycled colors (together, the last: SqueakCycled())
             * one KatAM cycle, its frames the DS row's colors at those slots; */
            List<SqueakColorCycle> colorCycles = new List<SqueakColorCycle>();
            for (int p = 0; p < paletteLists.Count && main?.Cycles != null; p++)
                foreach (IGrouping<int, int> row in paletteLists[p].Select((c, i) => (c, i)).Where(e => e.c >= SqueakCycledBit)
                                                                   .GroupBy(e => (e.c >> 15 & 0xFF) >> 4, e => e.i)) {
                    SqueakPaletteCycle cycle = main.Cycles.First(k => k.Row == row.Key);
                    int[] at = row.ToArray();
                    int[] indexes = at.Select(i => paletteLists[p][i] >> 15 & 15).ToArray();
                    if (at.Last() - at[0] != at.Length - 1) throw new InvalidOperationException("A Squeak Squad room's cycled colors aren't together in a palette.");
                    colorCycles.Add(new SqueakColorCycle(p * 16 + 1 + at[0], at.Length, cycle.Durations,
                                                         cycle.Frames.Select(f => indexes.Select(i => f[i]).ToArray()).ToArray()));
                }
            if (colorCycles.Count > SqueakMaxColorCycles) throw new InvalidOperationException("A Squeak Squad room's palette animation needs more cycles than KatAM keeps.");
            (int palette, byte[] indexes) Indexed(int[] pixels, int set) {
                int palette = PaletteOf(set);
                byte[] indexes = pixels.Select(c => c < 0 ? (byte)0 : (byte)(paletteLists[palette].IndexOf(Shown(palette, c)) + 1)).ToArray();
                return (palette, indexes);
            }

            // KatAM's tiles: the pictures, flips of one another one tile, the empty one tile 0;
            static byte[] Flip(byte[] t, bool h, bool v) {
                byte[] f = new byte[64];
                for (int y = 0; y < 8; y++)
                    for (int x = 0; x < 8; x++) f[y * 8 + x] = t[(v ? 7 - y : y) * 8 + (h ? 7 - x : x)];
                return f;
            }
            List<(int palette, byte[] indexes)> tiles = new List<(int, byte[])> { (0, new byte[64]) };
            Dictionary<string, (int tile, bool h, bool v)> tileOf = new Dictionary<string, (int, bool, bool)> { [string.Join(",", new byte[64]) + "|0"] = (0, false, false) };
            (int tile, bool h, bool v)[] pictureTile = new (int, bool, bool)[pictures.Count];
            List<int> tileUses = new List<int> { 0 };
            for (int p = 0; p < pictures.Count; p++) {
                (int palette, byte[] indexes) = Indexed(pictures[p], pictureSet[p]);
                if (indexes.All(i => i == 0)) palette = 0;
                (int tile, bool h, bool v) found = (-1, false, false);
                foreach ((bool h, bool v) in new[] { (false, false), (true, false), (false, true), (true, true) })
                    if (tileOf.TryGetValue(string.Join(",", Flip(indexes, h, v)) + "|" + palette, out var at)) { found = (at.tile, h ^ at.h, v ^ at.v); break; }
                if (found.tile < 0) {
                    found = (tiles.Count, false, false);
                    tileOf[string.Join(",", indexes) + "|" + palette] = found;
                    tiles.Add((palette, indexes));
                    tileUses.Add(0);
                }
                pictureTile[p] = found;
                tileUses[found.tile] += uses[p];
            }

            // Past KatAM's room for tiles: the two most alike of a palette made one, the less used taking the other's place;
            int[] tileTarget = Enumerable.Range(0, tiles.Count).ToArray();
            int Target(int t) { while (tileTarget[t] != t) t = tileTarget[t]; return t; }
            // (the animations' slots take their room after the others);
            int mergedTiles = 0, liveTiles = tiles.Count, staticTiles = maxTiles - slotTiles;
            if (liveTiles > staticTiles) {
                int Difference(byte[] a, byte[] b, int palette) {
                    int sum = 0;
                    for (int i = 0; i < 64; i++) {
                        if (a[i] == b[i]) continue;
                        if (a[i] == 0 || b[i] == 0) { sum += 3000; continue; }
                        sum += SqueakColorDistance(paletteColors[palette * 16 + a[i]], paletteColors[palette * 16 + b[i]]);
                    }
                    return sum;
                }
                /* (two tiles' difference never changes: worked out once, `apart`, by their places among their palette's tiles, then looked
                 * up each round; the same two found as when each round worked them all out again, which took seconds in the busiest
                 * rooms); */
                int[] place = new int[tiles.Count];
                Dictionary<int, (int count, int[] pairs)> apart = new Dictionary<int, (int, int[])>();
                foreach (IGrouping<int, int> group in Enumerable.Range(1, tiles.Count - 1).GroupBy(t => tiles[t].palette)) {
                    List<int> list = group.ToList();
                    int[] pairs = new int[list.Count * list.Count];
                    for (int i = 0; i < list.Count; i++) place[list[i]] = i;
                    for (int i = 0; i < list.Count; i++)
                        for (int j = i + 1; j < list.Count; j++) pairs[i * list.Count + j] = Difference(tiles[list[i]].indexes, tiles[list[j]].indexes, group.Key);
                    apart[group.Key] = (list.Count, pairs);
                }
                while (liveTiles > staticTiles) {
                    (int a, int b, int d) best = (-1, -1, int.MaxValue);
                    List<int> live = Enumerable.Range(1, tiles.Count - 1).Where(t => tileTarget[t] == t).ToList();
                    foreach (IGrouping<int, int> group in live.GroupBy(t => tiles[t].palette)) {
                        (int count, int[] pairs) = apart[group.Key];
                        foreach (int i in group)
                            foreach (int j in group)
                                if (i < j) {
                                    int d = pairs[place[i] * count + place[j]];
                                    if (d < best.d) best = (i, j, d);
                                }
                    }
                    if (best.a < 0) throw new InvalidOperationException("A Squeak Squad room's tiles don't fit KatAM's foreground.");
                    (int keep, int drop) = tileUses[best.a] >= tileUses[best.b] ? (best.a, best.b) : (best.b, best.a);
                    tileTarget[drop] = keep;
                    tileUses[keep] += tileUses[drop];
                    liveTiles--;
                    mergedTiles++;
                }
            }
            // (the kept tiles numbered again, in order);
            int[] number = new int[tiles.Count];
            List<byte[]> kept = new List<byte[]>();
            for (int t = 0; t < tiles.Count; t++)
                if (Target(t) == t) { number[t] = kept.Count; kept.Add(tiles[t].indexes); }

            /* The animations' tiles after them: a timing's slots in a row, its first frame's in the tiles, every frame's for KatAM's
             * foreground animations to copy in turn (one per timing); */
            static byte[] Packed(byte[] indexes) {
                byte[] packed = new byte[32];
                for (int i = 0; i < 64; i += 2) packed[i / 2] = (byte)(indexes[i] | indexes[i + 1] << 4);
                return packed;
            }
            List<SqueakTileAnimation> tileAnimations = new List<SqueakTileAnimation>();
            int[] slotPalette = new int[slotTiles];
            List<byte[]> slotFirst = new List<byte[]>();
            for (int t = 0, first = kept.Count; t < timings.Count; first += slots[t].Count, t++) {
                byte[][] frames = Enumerable.Range(0, timings[t].Length).Select(_ => new byte[slots[t].Count * 32]).ToArray();
                for (int i = 0; i < slots[t].Count; i++) {
                    int[][] slotFrames = slots[t][i];
                    for (int f = 0; f < frames.Length; f++) {
                        (int palette, byte[] indexes) = Indexed(slotFrames[f], slotSet[slotIndex[(t, i)]]);
                        slotPalette[slotIndex[(t, i)]] = palette;
                        Packed(indexes).CopyTo(frames[f], i * 32);
                        if (f == 0) slotFirst.Add(Packed(indexes));
                    }
                }
                tileAnimations.Add(new SqueakTileAnimation(first, slots[t].Count, frames, timings[t]));
            }
            byte[] tileData = new byte[(kept.Count + slotTiles) * 32];
            for (int t = 0; t < kept.Count; t++) Packed(kept[t]).CopyTo(tileData, t * 32);
            for (int i = 0; i < slotTiles; i++) slotFirst[i].CopyTo(tileData, (kept.Count + i) * 32);
            ushort Entry(int picture) {
                (int tile, bool h, bool v) = pictureTile[picture];
                int target = Target(tile);
                return (ushort)(number[target] | (h ? 0x400 : 0) | (v ? 0x800 : 0) | tiles[target].palette << 12);
            }
            ushort[] tilemap = cellPicture.Select((p, c) => p >= 0 ? Entry(p) : (ushort)(kept.Count + slotIndex[cellSlot[c]] | slotPalette[slotIndex[cellSlot[c]]] << 12)).ToArray();
            ushort[][] bareEntries = Enumerable.Range(0, bare.Count).Select(b => Enumerable.Range(0, 4).Select(q => Entry(barePicture[b * 4 + q])).ToArray()).ToArray();
            return new SqueakForeground(tileData, paletteColors, tilemap, columns, rows, mergedColors, mergedTiles, bareEntries, tileAnimations, colorCycles);
        }

        /* SqueakFarViewPicture(); A far view (map/d_viw_NN.nsbtx: a TEX0 block, one 256-color texture, 512 or 256 pixels wide, 256 tall,
         * or 256 by 512) as the DS shows it: its top 192 rows (the rest repeats its last row, the plane's smeared edge; the tall one, a
         * climb's, all of it); colors BGR555 (a TEX0's dictionaries: a header, 4 bytes a name's hash, the entries, 16 bytes a name); */
        static int[,] SqueakFarViewPicture(byte[] file) {
            int tex0 = ReadInt(file, 0x10);
            int textureInfo = tex0 + ReadShort(file, tex0 + 0x0E), textureData = tex0 + ReadInt(file, tex0 + 0x14);
            int paletteInfo = tex0 + ReadInt(file, tex0 + 0x34), paletteData = tex0 + ReadInt(file, tex0 + 0x38);
            int Entry(int dictionary) => dictionary + 4 + 8 + 4 * file[dictionary + 1] + 4;
            int parameters = ReadInt(file, Entry(textureInfo));
            int width = 8 << (parameters >> 20 & 7), height = 8 << (parameters >> 23 & 7), format = parameters >> 26 & 7;
            if (format != 4) throw new InvalidOperationException("A Squeak Squad far view isn't the 256-color texture expected.");
            int pixels = textureData + (parameters & 0xFFFF) * 8, colors = paletteData + (ReadShort(file, Entry(paletteInfo)) & 0x1FFF) * 8;
            int shown = height > width ? height : Math.Min(height, 192);
            int[,] picture = new int[width, shown];
            for (int y = 0; y < shown; y++)
                for (int x = 0; x < width; x++) picture[x, y] = ReadShort(file, colors + file[pixels + y * width + x] * 2) & 0x7FFF;
            return picture;
        }

        // A converted far view: KatAM's background tiles (4 bits a pixel), 8 palettes (KatAM's 6-13, color 0 unused) and map (width x
        // height tiles, palettes 6-13);
        internal sealed record SqueakBackground(byte[] Tiles, ushort[] Palettes, ushort[] Map, int Width, int Height, int MergedTiles);

        /* ConvertSqueakFarView(); A far view as KatAM's background, its 900 tiles and 8 palettes of 15 colors: shrunk to `scale` (its
         * colors averaged over the pixels each covers: a 512-pixel view at 3/4 is 48 tiles wide), at least `minHeight` tall (KatAM's
         * camera shows 160 rows: one shorter is shrunk less; the last row repeated only past its picture); the tiles grouped by their colors into 8 palettes,
         * each palette the 15 colors that draw its tiles best (k-means), each tile then drawn with the palette that draws it best, a
         * few rounds; past 900 tiles, the two most alike made one (as BuildBackground()); */
        static SqueakBackground ConvertSqueakFarView(int[,] picture, double scale = 0.75, int minHeight = 160, int maxTiles = BackgroundTiles) {
            int sourceWidth = picture.GetLength(0), sourceHeight = picture.GetLength(1);
            /* (never narrower than KatAM's screen: the 256-pixel views at 3/4 were 192 wide, the screen's last 48 pixels black (Dark
             * Nebula's room, 8-4-6): those shrink to 240 wide only, their height as much if that fits the tiles, else 3/4); */
            double scaleX = Math.Max(scale, 240.0 / sourceWidth), scaleY = scale;
            if (scaleX != scale && Math.Round(sourceWidth * scaleX / 8) * Math.Max(Math.Round(sourceHeight * scaleX / 8), (minHeight + 7) / 8) <= maxTiles) scaleY = scaleX;
            /* (never shorter than `minHeight` either: the DS's 192 rows at 3/4 were 144 for KatAM's 160, the last row repeated under
             * them in streaks (the owner: "some Squeak Squad backgrounds have some stretching artifacts at the bottom"): shrunk less
             * instead, both ways, so its rows fill the screen as the DS's do; the tiles past 900 merged as below); */
            if (sourceHeight * scaleY < minHeight) {
                scaleY = Math.Min(1.0, (double)minHeight / sourceHeight);
                scaleX = Math.Max(scaleX, scaleY);
            }
            int width = (int)Math.Round(sourceWidth * scaleX / 8) * 8, height = Math.Max((int)Math.Round(sourceHeight * scaleY / 8) * 8, (minHeight + 7) / 8 * 8);
            int shrunkHeight = Math.Min(height, (int)Math.Round(sourceHeight * scaleY));
            // (each pixel the average of the source pixels it covers, weighted by how much of each);
            double[,,] image = new double[width, height, 3];
            double stepX = 1 / scaleX, stepY = 1 / scaleY;
            for (int y = 0; y < height; y++)
                for (int x = 0; x < width; x++) {
                    double y0 = Math.Min(y, shrunkHeight - 1) * stepY, x0 = x * stepX, r = 0, g = 0, b = 0, total = 0;
                    for (int sy = (int)y0; sy < Math.Min(sourceHeight, (int)Math.Ceiling(y0 + stepY)); sy++)
                        for (int sx = (int)x0; sx < Math.Min(sourceWidth, (int)Math.Ceiling(x0 + stepX)); sx++) {
                            double weight = (Math.Min(sx + 1, x0 + stepX) - Math.Max(sx, x0)) * (Math.Min(sy + 1, y0 + stepY) - Math.Max(sy, y0));
                            if (weight <= 0) continue;
                            int c = picture[sx, sy];
                            r += (c & 31) * weight; g += (c >> 5 & 31) * weight; b += (c >> 10 & 31) * weight; total += weight;
                        }
                    image[x, y, 0] = r / total; image[x, y, 1] = g / total; image[x, y, 2] = b / total;
                }

            int columns = width / 8, rows = height / 8, tileCount = columns * rows;
            const int Palettes = 8, Colors = 15;
            static double Distance(double r1, double g1, double b1, double r2, double g2, double b2) =>
                3 * (r1 - r2) * (r1 - r2) + 4 * (g1 - g2) * (g1 - g2) + 2 * (b1 - b2) * (b1 - b2);
            // Nearest(); Which of 0 to count-1 has the least `cost` (the first of equals, as OrderBy().First(): without sorting them);
            static int Nearest(int count, Func<int, double> cost) {
                int best = 0;
                double least = cost(0);
                for (int k = 1; k < count; k++) {
                    double c = cost(k);
                    if (c.CompareTo(least) < 0) { least = c; best = k; }
                }
                return best;
            }
            double[][] TilePixels(int t) {
                double[][] p = new double[64][];
                for (int i = 0; i < 64; i++) {
                    int x = t % columns * 8 + i % 8, y = t / columns * 8 + i / 8;
                    p[i] = new[] { image[x, y, 0], image[x, y, 1], image[x, y, 2] };
                }
                return p;
            }
            double[][][] tilePixels = Enumerable.Range(0, tileCount).Select(TilePixels).ToArray();

            // k-means of colors: `count` centers for the pixels given (started from the most spread);
            static double[][] Centers(List<double[]> points, int count, double[][] start = null) {
                if (points.Count == 0) return Enumerable.Range(0, count).Select(_ => new double[3]).ToArray();
                double[][] centers = start?.Select(c => (double[])c.Clone()).ToArray() ?? new double[count][];
                if (start == null) {
                    centers[0] = (double[])points[0].Clone();
                    double[] nearest = points.Select(p => Distance(p[0], p[1], p[2], centers[0][0], centers[0][1], centers[0][2])).ToArray();
                    for (int k = 1; k < count; k++) {
                        int far = Array.IndexOf(nearest, nearest.Max());
                        centers[k] = (double[])points[far].Clone();
                        for (int i = 0; i < points.Count; i++)
                            nearest[i] = Math.Min(nearest[i], Distance(points[i][0], points[i][1], points[i][2], centers[k][0], centers[k][1], centers[k][2]));
                    }
                }
                // (the points and the centers in flat arrays: the same sums in the same order, faster);
                double[] flat = new double[points.Count * 3], at = new double[count * 3];
                for (int i = 0; i < points.Count; i++) (flat[i * 3], flat[i * 3 + 1], flat[i * 3 + 2]) = (points[i][0], points[i][1], points[i][2]);
                for (int round = 0; round < 8; round++) {
                    for (int k = 0; k < count; k++) (at[k * 3], at[k * 3 + 1], at[k * 3 + 2]) = (centers[k][0], centers[k][1], centers[k][2]);
                    double[] sums = new double[count * 3];
                    int[] counts = new int[count];
                    for (int i = 0; i < flat.Length; i += 3) {
                        double r = flat[i], g = flat[i + 1], b = flat[i + 2];
                        int best = 0;
                        double bestDistance = double.MaxValue;
                        for (int k = 0; k < count; k++) {
                            double d = Distance(r, g, b, at[k * 3], at[k * 3 + 1], at[k * 3 + 2]);
                            if (d < bestDistance) { bestDistance = d; best = k; }
                        }
                        counts[best]++;
                        sums[best * 3] += r; sums[best * 3 + 1] += g; sums[best * 3 + 2] += b;
                    }
                    for (int k = 0; k < count; k++)
                        if (counts[k] > 0) centers[k] = new[] { sums[k * 3] / counts[k], sums[k * 3 + 1] / counts[k], sums[k * 3 + 2] / counts[k] };
                }
                return centers;
            }

            /* The tiles grouped by their colors (k-means of their dark half's and light half's averages: a tile of greens and greys
             * isn't a tile of its average's), then palettes and groups refined in turn; */
            double[] Feature(double[][] p) {
                double[][] sorted = p.OrderBy(q => q[0] * 3 + q[1] * 4 + q[2] * 2).ToArray();
                return Enumerable.Range(0, 3).Select(c => sorted.Take(32).Average(q => q[c])).Concat(Enumerable.Range(0, 3).Select(c => sorted.Skip(32).Average(q => q[c]))).ToArray();
            }
            double[][] features = tilePixels.Select(Feature).ToArray();
            static double FeatureDistance(double[] a, double[] b) => Distance(a[0], a[1], a[2], b[0], b[1], b[2]) + Distance(a[3], a[4], a[5], b[3], b[4], b[5]);
            double[][] groupCenters = new double[Palettes][];
            groupCenters[0] = features[0];
            double[] nearestCenter = features.Select(f => FeatureDistance(f, groupCenters[0])).ToArray();
            for (int k = 1; k < Palettes; k++) {
                groupCenters[k] = features[Array.IndexOf(nearestCenter, nearestCenter.Max())];
                for (int t = 0; t < tileCount; t++) nearestCenter[t] = Math.Min(nearestCenter[t], FeatureDistance(features[t], groupCenters[k]));
            }
            int[] group = new int[tileCount];
            for (int round = 0; round < 10; round++) {
                for (int t = 0; t < tileCount; t++) group[t] = Nearest(Palettes, k => FeatureDistance(features[t], groupCenters[k]));
                for (int k = 0; k < Palettes; k++) {
                    double[][] members = Enumerable.Range(0, tileCount).Where(t => group[t] == k).Select(t => features[t]).ToArray();
                    if (members.Length > 0) groupCenters[k] = Enumerable.Range(0, 6).Select(c => members.Average(m => m[c])).ToArray();
                }
            }
            double[][][] palettes = new double[Palettes][][];
            // (TileError(); how far a tile's pixels are from their nearest colors in a palette, both flat arrays: r, g, b, ...);
            double[][] tileFlat = tilePixels.Select(pixels => pixels.SelectMany(q => q).ToArray()).ToArray();
            double TileError(int t, double[] palette) {
                double[] p = tileFlat[t];
                double sum = 0;
                for (int i = 0; i < p.Length; i += 3) {
                    // (the nearest color's distance, as Enumerable.Min(): a NaN the answer);
                    double nearest = Distance(p[i], p[i + 1], p[i + 2], palette[0], palette[1], palette[2]);
                    for (int c = 3; c < palette.Length && !double.IsNaN(nearest); c += 3) {
                        double d = Distance(p[i], p[i + 1], p[i + 2], palette[c], palette[c + 1], palette[c + 2]);
                        if (d < nearest || double.IsNaN(d)) nearest = d;
                    }
                    sum += nearest;
                }
                return sum;
            }
            for (int round = 0; round < 8; round++) {
                for (int k = 0; k < Palettes; k++) {
                    List<double[]> points = Enumerable.Range(0, tileCount).Where(t => group[t] == k).SelectMany(t => tilePixels[t]).ToList();
                    palettes[k] = Centers(points, Colors, round == 0 ? null : palettes[k]);
                }
                double[][] flatPalettes = palettes.Select(palette => palette.SelectMany(c => c).ToArray()).ToArray();
                for (int t = 0; t < tileCount; t++)
                    group[t] = Nearest(Palettes, k => TileError(t, flatPalettes[k]));
            }

            // KatAM's colors (BGR555), each tile's indexes, the same tiles (flipped too) one;
            ushort[] paletteColors = new ushort[Palettes * 16];
            for (int k = 0; k < Palettes; k++)
                for (int c = 0; c < Colors; c++) {
                    int r = Math.Clamp((int)Math.Round(palettes[k][c][0]), 0, 31), g = Math.Clamp((int)Math.Round(palettes[k][c][1]), 0, 31),
                        b = Math.Clamp((int)Math.Round(palettes[k][c][2]), 0, 31);
                    paletteColors[k * 16 + 1 + c] = (ushort)(r | g << 5 | b << 10);
                }
            byte[] Indexes(int t) => tilePixels[t].Select(p => (byte)(1 + Nearest(Colors, c =>
                Distance(p[0], p[1], p[2], paletteColors[group[t] * 16 + 1 + c] & 31, paletteColors[group[t] * 16 + 1 + c] >> 5 & 31, paletteColors[group[t] * 16 + 1 + c] >> 10 & 31)))).ToArray();
            static byte[] Flip(byte[] t, bool h, bool v) {
                byte[] f = new byte[64];
                for (int y = 0; y < 8; y++)
                    for (int x = 0; x < 8; x++) f[y * 8 + x] = t[(v ? 7 - y : y) * 8 + (h ? 7 - x : x)];
                return f;
            }
            List<(int palette, byte[] indexes)> tiles = new List<(int, byte[])>();
            List<int> uses = new List<int>();
            Dictionary<string, (int tile, bool h, bool v)> tileOf = new Dictionary<string, (int, bool, bool)>();
            (int tile, bool h, bool v)[] mapTiles = new (int, bool, bool)[tileCount];
            for (int t = 0; t < tileCount; t++) {
                byte[] indexes = Indexes(t);
                (int tile, bool h, bool v) found = (-1, false, false);
                foreach ((bool h, bool v) in new[] { (false, false), (true, false), (false, true), (true, true) })
                    if (tileOf.TryGetValue(Convert.ToBase64String(Flip(indexes, h, v)) + group[t], out var at)) { found = (at.tile, h ^ at.h, v ^ at.v); break; }
                if (found.tile < 0) {
                    found = (tiles.Count, false, false);
                    tileOf[Convert.ToBase64String(indexes) + group[t]] = found;
                    tiles.Add((group[t], indexes));
                    uses.Add(0);
                }
                mapTiles[t] = found;
                uses[found.tile]++;
            }

            /* Past KatAM's 900: the two most alike of a palette made one, the less used taking the other's place. Two tiles' difference
             * (their pixels' colors' distances, whole numbers) is worked out once (`apart`, by their places in their palette's tiles), then
             * each round weighed by the less used one's uses: the sum the pixels' distances times it made, exactly (whole numbers well
             * within a double's), so the same two picked as when each round summed them again (the owner: "the program has taken over 2
             * minutes to create the ROM": the far views took a minute); */
            int[] target = Enumerable.Range(0, tiles.Count).ToArray();
            int Target(int t) { while (target[t] != t) t = target[t]; return t; }
            int live = tiles.Count, merged = 0;
            int[] place = new int[tiles.Count];
            Dictionary<int, (int count, long[] pairs)> apart = new Dictionary<int, (int, long[])>();
            if (live > maxTiles)
                foreach (IGrouping<int, int> sameGroup in Enumerable.Range(0, tiles.Count).GroupBy(t => tiles[t].palette)) {
                    List<int> list = sameGroup.ToList();
                    long[] pairs = new long[list.Count * list.Count];
                    apart[sameGroup.Key] = (list.Count, pairs);
                    for (int i = 0; i < list.Count; i++) place[list[i]] = i;
                    for (int i = 0; i < list.Count; i++)
                        for (int j = i + 1; j < list.Count; j++) {
                            byte[] a = tiles[list[i]].indexes, b = tiles[list[j]].indexes;
                            long d = 0;
                            for (int p = 0; p < 64; p++) {
                                int ca = paletteColors[sameGroup.Key * 16 + a[p]], cb = paletteColors[sameGroup.Key * 16 + b[p]];
                                d += (long)Distance(ca & 31, ca >> 5 & 31, ca >> 10 & 31, cb & 31, cb >> 5 & 31, cb >> 10 & 31);
                            }
                            pairs[i * list.Count + j] = d;
                        }
                }
            while (live > maxTiles) {
                (int a, int b, double d) best = (-1, -1, double.MaxValue);
                List<int> alive = Enumerable.Range(0, tiles.Count).Where(t => target[t] == t).ToList();
                foreach (IGrouping<int, int> sameGroup in alive.GroupBy(t => tiles[t].palette)) {
                    List<int> list = sameGroup.ToList();
                    (int stride, long[] pairs) = apart[sameGroup.Key];
                    for (int i = 0; i < list.Count; i++)
                        for (int j = i + 1; j < list.Count; j++) {
                            double d = (double)(pairs[place[list[i]] * stride + place[list[j]]] * Math.Min(uses[list[i]], uses[list[j]]));
                            if (d < best.d) best = (list[i], list[j], d);
                        }
                }
                if (best.a < 0) break;
                (int keep, int drop) = uses[best.a] >= uses[best.b] ? (best.a, best.b) : (best.b, best.a);
                target[drop] = keep;
                uses[keep] += uses[drop];
                live--;
                merged++;
            }
            int[] number = new int[tiles.Count];
            List<byte[]> kept = new List<byte[]>();
            for (int t = 0; t < tiles.Count; t++)
                if (Target(t) == t) { number[t] = kept.Count; kept.Add(tiles[t].indexes); }
            byte[] tileData = new byte[kept.Count * 32];
            for (int t = 0; t < kept.Count; t++)
                for (int i = 0; i < 64; i += 2) tileData[t * 32 + i / 2] = (byte)(kept[t][i] | kept[t][i + 1] << 4);
            ushort[] map = new ushort[tileCount];
            for (int t = 0; t < tileCount; t++) {
                (int tile, bool h, bool v) = mapTiles[t];
                int into = Target(tile);
                map[t] = (ushort)(number[into] | (h ? 0x400 : 0) | (v ? 0x800 : 0) | (6 + tiles[into].palette) << 12);
            }
            return new SqueakBackground(tileData, paletteColors, map, columns, rows, merged);
        }
        /* KatAM's slope codes for Squeak Squad's (a slope's low byte: bits 4-5 its shape, 1 a half slope, 2 a 45° one; the low nibble
         * how deep its edge is at the block's middle, from its top; the top bits which way, read off every room: what lies above,
         * below and beside each code's cells): floors 0x14 and 0x1C the upper and lower halves falling right, 0x28 falling, 0xE8
         * rising, 0xF4 and 0xFC the upper and lower halves rising; ceilings 0x68 and 0xA8 (solid on the left, on the right), 0x74
         * and 0x7C, 0x94 and 0x9C their halves. KatAM's (gCollisionAttributes' top nibble, its height maps 0x08350C38: from a
         * block's bottom, x its column) 1 x/2, 2 x/2 + 7, 3 x, 4 15 - x, 5 (15 - x)/2, 6 (15 - x)/2 + 7; ceilings 7-12 the same
         * upside down: codes 0x05, 0x06, 0x0B, 0x0C, 0x08, 0x07; 0x04, 0x03, 0x0A, 0x09, 0x01, 0x02; in water 0x34-0x43; thin
         * (dropped through) 0x10-0x13, 0x18, 0x19; */
        static readonly Dictionary<int, (byte solid, byte water, byte thin)> SqueakSlopes = new Dictionary<int, (byte, byte, byte)> {
            [0x14] = (0x07, 0x42, 0x12), [0x1C] = (0x08, 0x43, 0x13), [0x28] = (0x0C, 0x3A, 0x19), [0xE8] = (0x0B, 0x38, 0x18),
            [0xF4] = (0x06, 0x41, 0x11), [0xFC] = (0x05, 0x40, 0x10), [0x68] = (0x0A, 0x36, 0x0A), [0xA8] = (0x09, 0x34, 0x09),
            [0x74] = (0x04, 0x3F, 0x04), [0x7C] = (0x03, 0x3E, 0x03), [0x94] = (0x01, 0x3C, 0x01), [0x9C] = (0x02, 0x3D, 0x02),
        };
        const byte KatAMAir = 0x00, KatAMSolid = 0x0D, KatAMThin = 0x0E, KatAMFirmThin = 0x0F, KatAMLadder = 0x21, KatAMLadderTop = 0x20,
                   KatAMSpikes = 0x1C, KatAMWater = 0x48, KatAMStarBlock = 0xF0, KatAMMetalBlock = 0xF5, SqueakBigStarBlock = 0xF1;

        /* SqueakRopeCells(); The rope cells a cut clears (squeak/gimmicks.c's SqueakRopeMain(), as the DS's stage, 0x022164C0, clears them):
         * for each rope the converter makes (gimmick 1 with its two bytes, among the room's first SqueakMailGimmicks), its column's cells
         * from its spot down its length, and on up and down while the blocks grid's cell is a rope's (top byte 0x38, its id's 0xC0 bits
         * 0x40: the DS's own test). Each gets a block record (AddSqueakRooms()): its look without the front layer (where the DS draws the
         * rope, blocks 0x260-0x262), leaving air; */
        static List<(int x, int y)> SqueakRopeCells(SqueakMap map) {
            List<(int x, int y)> cells = new List<(int, int)>();
            List<SqueakGimmick> gimmicks = map.Gimmicks();
            bool Rope(int x, int y) => x >= 0 && y >= 0 && x < map.Width && y < map.Height && map.Collision(6, x, y) >> 24 == 0x38
                                       && (map.Collision(6, x, y) >> 16 & 0xC0) == 0x40;
            for (int gi = 0; gi < gimmicks.Count && gi < SqueakMailGimmicks; gi++) {
                SqueakGimmick g = gimmicks[gi];
                if (g.Type != SqueakRopeGimmick || g.Parameters == null || g.Parameters.Length < 2) continue;
                int x = g.X / 16, top = g.Y / 16, bottom = (g.Y + Math.Max((int)g.Parameters[1], 16) - 1) / 16;
                while (Rope(x, top - 1)) top--;
                while (Rope(x, bottom + 1)) bottom++;
                for (int y = top; y <= bottom; y++)
                    if (Rope(x, y) && !cells.Contains((x, y))) cells.Add((x, y));
            }
            return cells;
        }

        /* SqueakRoomCollision(); A room's KatAM collision, its breakable blocks (star blocks, and the bricks and metal blocks: KatAM's metal
         * blocks, see SqueakTurnCannon), each one's value and what it leaves (the main grid's cell, the DS's once the block's is cleared,
         * unless that's a block too: then air, water in water), and the cells of its bomb chains (`chains`, drawn cleared once broken).
         * A bomb chain is the blocks grid's cells flagged 0x1000 (set off by a bomb block), touching each other, with a bomb block among
         * them (the grid's solid cell flagged so, top byte 0x38: 1-1-4's, under its wall of grey blocks) or a stake beside them (the
         * grid's 0x39 solids, all 13 of them beside one: 2-6-2's walls its door in): KatAM's bomb block 0xF1 there (a stake's too),
         * the blocks it sets off 0xF2 (the cells whose main terrain isn't air: the DS draws them on the main layer, its 0x262 in 1-1-4)
         * and KatAM's invisible links 0xF3 (its air cells: the chain goes on through them, as NiDL's); all leave air, water in water, as
         * the DS clears both grids (the third layer's grass stays). Flagged cells with neither stay as they are (a Bomb's blast's
         * secrets: 2-4-1's floor over its hidden way); */
        static (byte[] collision, List<(int x, int y)> blocks, List<byte> leaves, List<byte> values, HashSet<(int x, int y)> chains, HashSet<(int x, int y)> burnt) SqueakRoomCollision(SqueakMap map) {
            int width = map.Width, height = map.Height;
            byte[] collision = new byte[width * height];
            List<(int x, int y)> blocks = new List<(int, int)>();
            List<byte> leaves = new List<byte>(), blockValues = new List<byte>();
            HashSet<(int x, int y)> chains = new HashSet<(int, int)>(), burnt = new HashSet<(int, int)>(), loose = new HashSet<(int, int)>();
            List<List<(int x, int y)>> chainList = new List<List<(int x, int y)>>();
            /* (the cells fire burns, flagged 0x800, with their candles, aren't bomb chains: 4-2-3's burnable log bridges between its towers'
             * sealed parts and the wall walling its start in, Fire's on the DS; as bomb blocks they opened the towers to the levels' planner,
             * whose route then ran through parts Kirby couldn't reach: they're star blocks, below); */
            bool Flagged(int cx, int cy) => cx >= 0 && cy >= 0 && cx < width && cy < height && (map.Collision(6, cx, cy) & 0x1000) != 0;
            bool Stake(int cx, int cy) => cx >= 0 && cy >= 0 && cx < width && cy < height && map.Collision(6, cx, cy) >> 24 == 0x39
                                          && (map.Collision(6, cx, cy) & 0xC000) == 0x4000;
            // (the block groups a switch, a trigger area or a relay sends a message: chains too, broken by the room's gimmick task,
            // squeak/gimmicks.c, see SqueakSwitchTargets());
            List<SqueakGimmick> gimmicks = map.Gimmicks();
            HashSet<(int x, int y)> switched = gimmicks.SelectMany(g => SqueakSwitchTargets(gimmicks, g)).ToHashSet();
            for (int y = 0; y < height; y++)
                for (int x = 0; x < width; x++) {
                    if (!Flagged(x, y) || chains.Contains((x, y))) continue;
                    List<(int x, int y)> chain = new List<(int, int)> { (x, y) };
                    HashSet<(int, int)> seen = new HashSet<(int, int)> { (x, y) };
                    for (int c = 0; c < chain.Count; c++) {
                        (int cx, int cy) = chain[c];
                        foreach ((int nx, int ny) in new[] { (cx + 1, cy), (cx - 1, cy), (cx, cy + 1), (cx, cy - 1) })
                            if ((Flagged(nx, ny) || Stake(nx, ny)) && seen.Add((nx, ny))) chain.Add((nx, ny));
                    }
                    /* (a group nothing ported sets off, with no bomb block, switch or stake: what the DS's rolling boulders and gimmick 0x13
                     * break, 4-3-2's wall a boulder rolls into, 4-6-7's floors over its shaft, 8-1-10's, 8-2-4's: star blocks below, Kirby
                     * breaks them himself (the owner: every room reached); the treasure rooms' boards of 2 over their doors, the
                     * Squeaks', stay); */
                    if (!chain.Any(c => (map.Collision(6, c.x, c.y) & 0xC000) != 0 || switched.Contains(c))) {
                        if (chain.Count > 2) loose.UnionWith(chain);
                        continue;
                    }
                    chains.UnionWith(chain);
                    chainList.Add(chain);
                }
            for (int y = 0; y < height; y++)
                for (int x = 0; x < width; x++) {
                    uint word = SqueakCellWord(map, x, y);
                    byte value = collision[y * width + x] = SqueakCollision(word);
                    if (chains.Contains((x, y))) {
                        uint front = map.Collision(6, x, y), terrain = map.Collision(7, x, y);
                        value = collision[y * width + x] = (front & 0xC000) != 0 ? (byte)0xF1 : (terrain & 0xC000) != 0 ? (byte)0xF2 : (byte)0xF3;
                        blocks.Add((x, y));
                        blockValues.Add(value);
                        leaves.Add((terrain & 0xC000) == 0 && terrain >> 30 == 1 ? KatAMWater : KatAMAir);
                        continue;
                    }
                    /* (what Fire burns or melts on the DS, KatAM's star blocks: a solid cell its blocks grid flags 0x800 under top byte 0x38,
                     * the walls and log bridges 4-2-3 and 6-2-5 are walled in by, and the frozen blocks over a way, the blocks grid's ice
                     * (top byte 0x28) drawn with its tile 0x220 or 0x221 on a main cell that isn't solid, 6-1-4's over its door. KatAM has
                     * no block only Fire breaks, and can't promise Fire there: any attack breaks them (the owner: every room reached); a
                     * burnt cell leaves air and its main layer's look goes too, a melted one what the main grid has there); */
                    /* (a loose group's thin floors stay thin floors, the DS's main grid's (8-4-4's ledges in the dark star's shaft, room 0x5DE:
                     * the owner, "platforms are not pass through. They are inhalable. Boulders pass through the platforms": as star blocks
                     * they were solid and inhalable, and the boulders went through them), each with a block record a boulder applies,
                     * as the DS's break them as they fall on them (squeak/gimmicks.c's BoulderBreakUnder()): its look gone, air left); */
                    uint frontWord = map.Collision(6, x, y), mainWord = map.Collision(7, x, y);
                    // (and the open cell over one, of its group, where the main grid draws the ledge's top: it goes with it);
                    bool ledge = loose.Contains((x, y)) && (mainWord & 0xC000) == 0x8000 && SqueakThinFloor(value);
                    bool ledgeTop = loose.Contains((x, y)) && value == KatAMAir && map.Block(4, x, y) != 0 && loose.Contains((x, y + 1))
                                    && (map.Collision(7, x, y + 1) & 0xC000) == 0x8000;
                    if (ledge || ledgeTop) {
                        blocks.Add((x, y));
                        blockValues.Add(value);
                        leaves.Add(KatAMAir);
                        burnt.Add((x, y));
                        continue;
                    }
                    bool burns = ((frontWord & 0x800) != 0 && frontWord >> 24 == 0x38 || loose.Contains((x, y))) && value != KatAMAir;
                    bool melts = (frontWord & 0xC000) == 0x4000 && frontWord >> 24 == 0x28 && map.Block(3, x, y) is 0x220 or 0x221
                                 && (mainWord & 0xC000) != 0x4000;
                    /* (the blocks grid's block 3 over a solid main cell: the walk-once blocks, SqueakCrumbles(); star blocks, which
                     * squeak/gimmicks.c's Crumbles() breaks once a Kirby stands on one, as any attack does; and the solid cells under
                     * one that the blocks grid flags 0x400, each under the one before: they go after it, 3-6-2's second row, 4-4-4's,
                     * 3-2-3's and 3-1-3's shafts, 14 deep. Broken, each is drawn with the third layer alone, as the DS swaps it in
                     * (`burnt`: 3-6-2's show the forest behind, 3-2-3's the water), and leaves air, or water where its column stands
                     * in water, above and below it (3-2-3's nine, under water: they kept Kirby dry once broken); */
                    bool hidden = value == KatAMSolid && SqueakCrumbleChain(map, x, y);
                    melts |= hidden;
                    if (burns || melts) {
                        collision[y * width + x] = KatAMStarBlock;
                        blocks.Add((x, y));
                        blockValues.Add(KatAMStarBlock);
                        byte under = melts && !hidden ? SqueakCollision(mainWord) : hidden && SqueakCrumbleInWater(map, x, y) ? KatAMWater : KatAMAir;
                        leaves.Add(under is KatAMStarBlock or KatAMMetalBlock or KatAMSolid ? KatAMAir : under);
                        if (burns || hidden) burnt.Add((x, y));
                        continue;
                    }
                    // (the blocks grid's bricks, its solid cells of family 0x00: KatAM's metal blocks, see SqueakTurnCannon);
                    if ((map.Collision(6, x, y) & 0xC000) == 0x4000 && map.Collision(6, x, y) >> 24 == 0 && value == KatAMSolid)
                        value = collision[y * width + x] = KatAMMetalBlock;
                    if (value is not (KatAMStarBlock or KatAMMetalBlock)) continue;
                    blocks.Add((x, y));
                    blockValues.Add(value);
                    byte left = (map.Collision(6, x, y) & 0xC000) != 0 ? SqueakCollision(map.Collision(7, x, y)) : KatAMStarBlock;
                    leaves.Add(left is not (KatAMStarBlock or KatAMMetalBlock) ? left : word >> 30 == 1 ? KatAMWater : KatAMAir);
                }
            /* (the big star blocks, the blocks grid's block 5: 2 by 2 cells, its front blocks 0x2E0, 0x2E1 over 0x300, 0x301, side by side in
             * rows of up to 5 (2-6-4's, 8-1-11's); each KatAM's star block, a hit on one cell broke that cell alone, its 3 others' look
             * left half drawn (the owner: "Big star blocks break very oddly"): their cells KatAM's bomb blocks (0xF1), a hit on any
             * one setting off the rest, KatAM's chain, as a big star breaks whole; each cell drawn broken as a star block's, without
             * its front layer, not as a chain's: not in `chains`); */
            for (int b = 0; b < blocks.Count; b++) {
                (int x, int y) = blocks[b];
                uint front = map.Collision(6, x, y);
                if (blockValues[b] == KatAMStarBlock && (front & 0xC000) == 0x4000 && (front >> 24 & 7) == 5)
                    collision[y * width + x] = blockValues[b] = SqueakBigStarBlock;
            }
            /* (a chain whose bomb blocks Kirby can't reach from outside it, the open cells beside them a closed pocket of at most 200
             * cells or none: 3-6-2's bomb in the middle of its 5 by 3 wall, 3-2-2's facing only the pocket of the door it walls in:
             * on the DS an attack from outside sets them off, from under 3-2-2's bomb through the block below it (checked in the DS
             * emulator; a slide into 1-1-4's wall away from its bomb doesn't); in KatAM, where only the bomb block itself does, the
             * chain's blocks facing the outside are bomb blocks too); */
            bool Open(int cx, int cy) => cx >= 0 && cy >= 0 && cx < width && cy < height && !chains.Contains((cx, cy))
                && (collision[cy * width + cx] is KatAMAir or KatAMWater or KatAMLadder or KatAMLadderTop or KatAMSpikes || SqueakThinFloor(collision[cy * width + cx]));
            (int x, int y)[] Around((int x, int y) c) => new[] { (c.x + 1, c.y), (c.x - 1, c.y), (c.x, c.y + 1), (c.x, c.y - 1) };
            foreach (List<(int x, int y)> chain in chainList) {
                if (!chain.Any(c => collision[c.y * width + c.x] == 0xF1)) continue;   // (a switch's: its switch sets it off);
                HashSet<(int x, int y)> pocket = new HashSet<(int x, int y)>();
                Queue<(int x, int y)> open = new Queue<(int x, int y)>();
                foreach ((int x, int y) trigger in chain.Where(c => collision[c.y * width + c.x] == 0xF1))
                    foreach ((int x, int y) n in Around(trigger))
                        if (Open(n.x, n.y) && pocket.Add(n)) open.Enqueue(n);
                while (open.Count > 0 && pocket.Count <= 200)
                    foreach ((int x, int y) n in Around(open.Dequeue()))
                        if (Open(n.x, n.y) && pocket.Add(n)) open.Enqueue(n);
                if (pocket.Count > 200) continue;
                foreach ((int x, int y) c in chain)
                    if (collision[c.y * width + c.x] == 0xF2 && Around(c).Any(n => Open(n.x, n.y) && !pocket.Contains(n)))
                        collision[c.y * width + c.x] = blockValues[blocks.IndexOf(c)] = 0xF1;
            }
            for (int b = 0; b < blocks.Count; b++) {
                (int x, int y) = blocks[b];
                bool Water(int cx, int cy) => cx >= 0 && cy >= 0 && cx < width && cy < height && collision[cy * width + cx] is >= 0x34 and <= 0x48;
                if (leaves[b] == KatAMAir && (Water(x - 1, y) || Water(x + 1, y) || Water(x, y - 1) || Water(x, y + 1))) leaves[b] = KatAMWater;
            }
            return (collision, blocks, leaves, blockValues, chains, burnt);
        }

        /* SqueakCellWord(); A cell's collision as Squeak Squad reads it (0x0220FDEC, its low half: kind, flags, shape): the blocks' grid
         * (section 6, the room's +0x38: its star blocks, zappers, Secret Sea's clouds and much of the terrain) where its kind isn't air,
         * else the main terrain's (section 7, +0x3C), and an empty main cell the blocks grid's (its water); a door behind a star block
         * (the main grid's id 0x81 on air) opens when the block is broken, as on the DS; */
        static uint SqueakCellWord(SqueakMap map, int x, int y) {
            uint main = map.Collision(7, x, y), front = map.Collision(6, x, y);
            return (front & 0xC000) != 0 || main == 0 ? front : main;
        }

        /* SqueakDigCells(); The cells Animal Kirby digs through (the DS's 0x02216528, its stage's +0x9C: a blocks grid's cell, section 6, of
         * kind solid whose bits 27-29 are 1-3; 1-3-5, 5-3-1, 5-4-1, 7-3-3 and 8-3-3 hold 1s, Nature Notch's 2-3-5, 2-3-6, 2-8-5 and 2-8-6
         * 2s, Ice Island's 6-1-2 and 6-1-3 3s), those KatAM's solid cells here: each dug as the DS clears it, drawn bare with the third
         * layer (`cleared`), its own block record apart from the room's (gSqueakDigRooms: "ROM Expansion/abilities/squeak_moves.c"); */
        static List<(int x, int y)> SqueakDigCells(SqueakMap map, byte[] collision) {
            List<(int x, int y)> cells = new List<(int, int)>();
            for (int y = 0; y < map.Height; y++)
                for (int x = 0; x < map.Width; x++) {
                    uint word = map.Collision(6, x, y);
                    if ((word & 0xC000) == 0x4000 && (word >> 27 & 7) is >= 1 and <= 3 && collision[y * map.Width + x] == KatAMSolid) cells.Add((x, y));
                }
            return cells;
        }

        /* SqueakCurrents(); A room's currents (the collision word's ids 0x10-0x3C on air or water, see the notes): a byte a cell, 0 none,
         * else 0x80 | its way << 2 | its strength (the id less 0x10, over 4: its low 2 bits the way, 0 up, 1 right, 2 down, 3 left, the
         * rest the strength, 0-2: 0.5, 1 and 2 pixels a frame, measured in the DS emulator, 1-1-1's cells rewritten); squeak/gimmicks.c's
         * Currents() moves the Kirbys in them (the owner: "Currents in water don't have any effect on the player."); null where none; */
        static byte[] SqueakCurrents(SqueakMap map) {
            byte[] grid = new byte[map.Width * map.Height];
            bool any = false;
            for (int y = 0; y < map.Height; y++)
                for (int x = 0; x < map.Width; x++) {
                    uint word = SqueakCellWord(map, x, y);
                    int kind = (int)(word >> 14 & 3), id = (int)(word >> 16 & 0xFF), block = (int)(word >> 24 & 7);
                    if (kind != 0 || block != 0 || id < 0x10 || id > 0x3C || (id & 3) != 0) continue;
                    int index = (id - 0x10) / 4;
                    grid[y * map.Width + x] = (byte)(0x80 | (index & 3) << 2 | index >> 2);
                    any = true;
                }
            return any ? grid : null;
        }

        /* SqueakCrumble(); A walk-once block (the DS's blocks grid's block 3 on air, over a solid main cell: rows of cracked blocks, 1-6-6's
         * castle walkway, 2-6-7's leafy bridge, 28 rooms); SqueakCrumbleChain(), one or a hidden cell under it (flagged 0x400 on air in the
         * blocks grid, over solid ground) that goes after it: checked in the DS emulator, 3-6-2's: Kirby standing on it, it breaks, then
         * the cell under it; */
        static bool SqueakCrumble(SqueakMap map, int x, int y) => y >= 0 && (map.Collision(6, x, y) & 0xC000) == 0 && (map.Collision(6, x, y) >> 24 & 7) == 3
                                                                  && (map.Collision(7, x, y) & 0xC000) == 0x4000;
        static bool SqueakCrumbleHidden(SqueakMap map, int x, int y) => y >= 0 && (map.Collision(6, x, y) & 0x400) != 0 && (map.Collision(6, x, y) & 0xC000) == 0
                                                                        && (map.Collision(6, x, y) >> 24 & 7) != 3 && (map.Collision(7, x, y) & 0xC000) == 0x4000;
        static bool SqueakCrumbleChain(SqueakMap map, int x, int y) {
            if (SqueakCrumble(map, x, y)) return true;
            for (int up = y; SqueakCrumbleHidden(map, x, up); up--)
                if (SqueakCrumble(map, x, up - 1)) return true;
            return false;
        }

        // SqueakCrumbleInWater(); A walk-once block's column (it and its hidden cells) with water over its top and under its bottom;
        static bool SqueakCrumbleInWater(SqueakMap map, int x, int y) {
            bool Water(int cy) => cy >= 0 && cy < map.Height && (map.Collision(7, x, cy) & 0xC000) == 0 && map.Collision(7, x, cy) >> 30 == 1;
            int top = y, bottom = y;
            while (top > 0 && !SqueakCrumble(map, x, top)) top--;
            while (SqueakCrumbleHidden(map, x, bottom + 1)) bottom++;
            return Water(top - 1) && Water(bottom + 1);
        }

        /* SqueakCrumbles(); The room's walk-once blocks (SqueakCrumble()) for squeak/gimmicks.c's Crumbles(): u16 a count, then 4 bytes
         * each: its cell (x, y) and how many hidden cells under it go after it; null where none; */
        static byte[] SqueakCrumbles(SqueakMap map) {
            List<byte> list = new List<byte> { 0, 0 };
            int count = 0;
            for (int y = 0; y < map.Height; y++)
                for (int x = 0; x < map.Width; x++) {
                    if (!SqueakCrumble(map, x, y)) continue;
                    if (x > 0xFF || y > 0xFF) throw new InvalidOperationException("A Squeak Squad room's walk-once block is past cell 255.");
                    int chain = 0;
                    while (SqueakCrumbleHidden(map, x, y + 1 + chain)) chain++;
                    list.AddRange(new byte[] { (byte)x, (byte)y, (byte)chain, 0 });
                    count++;
                }
            if (count == 0) return null;
            list[0] = (byte)count;
            list[1] = (byte)(count >> 8);
            return list.ToArray();
        }

        // SqueakFirm(); KatAM's thin floor Kirby doesn't drop through for one he does (KatAMExpansionRooms.cs' ThinFloors()' Kept());
        static byte SqueakFirm(byte thin) => thin switch { KatAMThin => KatAMFirmThin, >= 0x10 and <= 0x13 => (byte)(thin + 4), 0x18 => 0x1A, 0x19 => 0x1B, _ => thin };
        // SqueakThinFloor(); Any of KatAM's thin floors, dropped through or not;
        static bool SqueakThinFloor(byte v) => v is KatAMThin or KatAMFirmThin or (>= 0x10 and <= 0x17) or (>= 0x18 and <= 0x1B);

        /* SqueakCollision(); KatAM's collision for a Squeak Squad cell (see the notes' collision word): its kind (bits 14-15: air, solid,
         * thin, slope), its flags (bits 8-13: 0x100 a ladder, on a thin floor its top; 0x200 and 0x300 thin floors, both dropped
         * through), its block (bits 24-26: 4 a star block, 5 a big one: breakable, KatAMStarBlock, what they leave the main layer's
         * cell; 2 an electric zapper: spikes), the ids on air (0x40-0x4C, without the rope's 0x38 above them: spikes; doors' 0x80 and
         * up: air, the doors are objects) and its liquid (bits 30-31, on air: 1 water, 2 lava, which hurts as spikes do; with block 2,
         * fire jets: air). Ropes and chains (top byte 0x38, ids 0x40-0x46) are climbed as ladders. Currents (ids 0x10-0x3C) stay air or
         * water here: SqueakCurrents() keeps their ways and strengths; */
        static byte SqueakCollision(uint word) {
            int kind = (int)(word >> 14 & 3), flags = (int)(word >> 8 & 0x3F) << 8, block = (int)(word >> 24 & 7), id = (int)(word >> 16 & 0xFF);
            int liquid = (int)(word >> 30), shape = (int)(word & 0xFF);
            bool rope = (word >> 24) == 0x38 && id is >= 0x40 and <= 0x46;
            switch (kind) {
                case 1:
                    if (block is 4 or 5) return KatAMStarBlock;
                    // (block 1, the grey metal blocks a cannon's shot breaks: KatAM's metal blocks, see SqueakTurnCannon);
                    if (block == 1) return KatAMMetalBlock;
                    if (block == 2) return KatAMSpikes;
                    return KatAMSolid;
                case 2:
                    if ((flags & 0x100) != 0 && (flags & 0x200) == 0) return KatAMLadderTop;
                    /* (0x300's Kirby drops through: Down drops him through 2-2-3's, checked in the DS emulator, KatAM's thin floors (Down and
                     * B); 0x200's he only jumps up through, never drops through (the owner, of 5-5-2's planks, room 0x517: "the platform can
                     * be passed from below but not dropped down from above. It's part of the puzzle, there are many instances of this bug
                     * happening in game"): KatAM's firm ones, SqueakFirm()); */
                    byte thinFloor = SqueakSlopes.TryGetValue(shape, out var thin) ? thin.thin : KatAMThin;
                    return (flags & 0x300) == 0x300 ? thinFloor : SqueakFirm(thinFloor);
                case 3:
                    return SqueakSlopes.TryGetValue(shape, out var slope) ? (liquid == 1 ? slope.water : slope.solid) : KatAMSolid;
            }
            if (rope || (flags & 0x100) != 0) return KatAMLadder;
            if (liquid == 2) return KatAMSpikes;
            if (liquid == 1 && block != 2) return KatAMWater;
            if (id is 0x40 or 0x44 or 0x48 or 0x4C && liquid == 0) return KatAMSpikes;
            return KatAMAir;
        }

        /* Squeak Squad's enemies KatAM has (the notes' table of their brains against KatAM's own): its object type and, by the spawn's
         * first parameter byte (Squeak Squad's brain), the behavior (KatAM's own where it has that brain, else squeak/natives.c's;
         * past the list, the first). Glunk's is by the second byte (Squeak Squad's Pl1, 5 or 6), Waddle Dee's 4 holds a parasol (KatAM's
         * 3); the last four are KatAM's own mid-bosses (Mr. Frosty, Bonkers, Box Boxer, Boxy); */
        static readonly Dictionary<int, (byte type, byte[] behaviors)> SqueakNatives = new Dictionary<int, (byte, byte[])> {
            [0x00] = (0x00, new byte[] { 0, 5, 6, 7, 3, 4 }),       // Waddle Dee;
            [0x01] = (0x01, new byte[] { 0, 1, 4, 5, 2 }),          // Bronto Burt;
            [0x02] = (0x02, new byte[] { 0, 1, 2, 3, 4 }),          // Blipper;
            [0x03] = (0x03, new byte[] { 5 }),                      // Glunk;
            [0x04] = (0x05, new byte[] { 0 }),                      // Scarfy;
            [0x05] = (0x06, new byte[] { 0, 2, 1, 3, 5 }),          // Gordo;
            [0x06] = (0x07, new byte[] { 0, 5 }),                   // Snooter;
            [0x07] = (0x08, new byte[] { 0 }),                      // Chip;
            [0x08] = (0x2D, new byte[] { 0 }),                      // Batty;
            [0x09] = (0x0D, new byte[] { 0 }),                      // Blockin;
            [0x0A] = (0x11, new byte[] { 0 }),                      // Big Waddle Dee;
            [0x13] = (0x35, new byte[] { 0, 2, 2, 2, 2, 2 }),       // Shotzo (aiming, fixed);
            [0x14] = (0x12, new byte[] { 0, 2, 3, 0, 1 }),          // Waddle Doo;
            [0x15] = (0x14, new byte[] { 0, 2, 1 }),                // Hot Head;
            [0x16] = (0x13, new byte[] { 0 }),                      // Flamer;
            [0x17] = (0x16, new byte[] { 0, 1, 1, 0, 0, 1 }),       // Pengy;
            [0x18] = (0x19, new byte[] { 0, 1, 1 }),                // Sparky;
            [0x19] = (0x1C, new byte[] { 0, 1 }),                   // Twister;
            [0x1A] = (0x1A, new byte[] { 0, 1, 2 }),                // Sword Knight;
            [0x1C] = (0x0C, new byte[] { 0, 1, 2, 3, 4 }),          // Cupie;
            [0x1D] = (0x18, new byte[] { 0, 1, 1, 2 }),             // Sir Kibble;
            [0x1E] = (0x15, new byte[] { 0, 1 }),                   // Laser Ball;
            [0x1F] = (0x22, new byte[] { 0, 1, 2 }),                // Foley;
            [0x20] = (0x1D, new byte[] { 0, 1 }),                   // Wheelie;
            [0x21] = (0x1B, new byte[] { 0, 1 }),                   // UFO;
            [0x22] = (0x1E, new byte[] { 0, 1 }),                   // Noddy;
            [0x23] = (0x25, new byte[] { 0 }),                      // Boxin;
            [0x24] = (0x29, new byte[] { 0, 1 }),                   // Heavy Knight;
            [0x2D] = (0x38, new byte[] { 0 }),                      // Mr. Frosty;
            [0x2E] = (0x39, new byte[] { 0 }),                      // Bonkers;
            [0x30] = (0x3C, new byte[] { 0 }),                      // Box Boxer;
            [0x31] = (0x3D, new byte[] { 0 }),                      // Boxy;
        };
        static readonly HashSet<byte> KatAMMidbossTypes = new HashSet<byte> { 0x38, 0x39, 0x3C, 0x3D };
        /* Squeak Squad's own characters (type 0x2C, the behavior byte the character, squeak.h's SQUEAK_*, the spawn's first byte its
         * variant: each picks its brain as Squeak Squad's does), and its mid-bosses and bosses (on their mini-boss type, SQUEAK_MIDBOSS |
         * the character): 0x3D is Dark Daroach when its second byte is 1; Dark Nebula's coming (0x46, a8s4s5) is his fight's start here
         * (0x3F, a8s4s6: squeak/dark_nebula.c does both); */
        static readonly Dictionary<int, int> SqueakCharacters = new Dictionary<int, int> {
            [0x0B] = 2, [0x0C] = 3, [0x0D] = 0, [0x0E] = 1, [0x0F] = 5, [0x10] = 6, [0x11] = 8, [0x25] = 9, [0x26] = 12, [0x27] = 13,
            [0x28] = 15, [0x2A] = 16, [0x2B] = 18, [0x2C] = 19, [0x3E] = 68, [0x40] = 27, [0x42] = 29, [0x43] = 31, [0x44] = 33, [0x45] = 34,
        };
        static readonly Dictionary<int, int> SqueakBosses = new Dictionary<int, int> {
            [0x33] = 21, [0x34] = 20, [0x35] = 23, [0x36] = 25, [0x37] = 53, [0x38] = 35, [0x39] = 40, [0x3A] = 37, [0x3B] = 42, [0x3C] = 47,
            [0x3D] = 49, [0x3F] = 58,
        };
        const int DarkDaroachIndex = 51;
        /* A Squeak Lair's fight (PlanSqueakLevels()): its mini-boss's template unk22 SQUEAK_LAIR_FIGHT (squeak.h: SqueakBossFloor() puts a
         * boss the randomizer picks there on its area's floor), and its Squeakers, LairSqueakers at most; */
        const byte SqueakLairFight = 0x10;
        /* A mini-boss's template unk22 bit 2: KatAM's mini-boss defeat (sub_0809AF38, sub_0809EF88) doesn't put the room's music back
         * (sub_08086C48); Dark Nebula's ("ROM Expansion/squeak/dark_nebula.c") fades it out itself, as the DS's; */
        const byte KeepFightMusic = 4;
        /* (two: with four, and the Squeaks the fight calls in, a hideout ran at 59% speed in the GBA emulator (0x481: 177 frames of the
         * game's in 300), with two at full speed; the owner: "an unbearable amount of lag" in the Squeak hideouts); */
        const int LairSqueakers = 2;
        // The spawn flags (+0xB): 4, a chase spawn (only once the stage's treasure is carried: not made yet);
        const int SqueakChaseSpawn = 4;

        /* The items (.doe item kinds, the item maker's IDs in the floating bubbles): KatAM's 1UP, Invincible Candy, Maximum Tomato, Meat,
         * Pep Brew (the energy drink) and Cherry; a bubble holding the 1UP's look (kind 1) the 1UP, the bubbles' other foods a Cherry.
         * An ability (kinds 0x17-0x30, the ability the kind less 0x16; a bubble's 0 or 1 and the ability) is the enemy that gives it
         * (SqueakAbilityEnemies: KatAM's, or Squeak Squad's own); an ability none gives, food. The "?" bubble (kind 0x15, and a bubble
         * holding ability 0: the DS numbers its abilities from 1) gives a random ability: KatAM's random ability pedestal (the owner:
         * "Random pedestal bubbles turn into consumables. Why? Literally there's a random pedestal in game"); one a barrel or a grass
         * tuft holds comes out a Cherry (SqueakHeld()); */
        const byte KatAMCherry = 0x5E, KatAMPepBrew = 0x5F, KatAMMeat = 0x60, KatAMTomato = 0x61, KatAMOneUp = 0x63, KatAMCandy = 0x64;
        const byte KatAMRandomPedestal = 0x96;
        const int SqueakRandomItem = 0x15;
        static readonly Dictionary<int, byte> SqueakItemKinds = new Dictionary<int, byte> {
            [1] = KatAMOneUp, [3] = KatAMOneUp, [4] = KatAMCandy, [5] = KatAMTomato, [6] = KatAMMeat, [7] = KatAMPepBrew, [8] = KatAMCherry,
        };
        static readonly Dictionary<int, byte> SqueakBubbleItems = new Dictionary<int, byte> {
            [1] = KatAMOneUp, [6] = KatAMTomato, [7] = KatAMPepBrew, [8] = KatAMMeat,
        };
        // (its object type, behavior, variant);
        static readonly Dictionary<int, (byte type, byte behavior, byte variant)> SqueakAbilityEnemies = new Dictionary<int, (byte, byte, byte)> {
            [1] = (0x14, 0, 0), [2] = (0x16, 0, 0), [3] = (0x19, 0, 0), [4] = (0x12, 0, 0), [5] = (0x1C, 0, 0), [6] = (0x00, 3, 0),
            [7] = (0x18, 0, 0), [8] = (0x15, 0, 0), [9] = (0x22, 0, 0), [10] = (0x1D, 0, 0), [11] = (0x1B, 0, 0), [13] = (0x0C, 0, 0),
            [14] = (0x1E, 0, 0), [15] = (SqueakID, 12, 1), [16] = (0x1A, 0, 0), [17] = (SqueakID, 18, 0), [19] = (SqueakID, 9, 0),
            [20] = (0x25, 0, 0), [21] = (SqueakID, 15, 0), [22] = (SqueakID, 13, 0), [23] = (SqueakID, 19, 0),
        };
        const int SqueakAbilityKinds = 0x16, SqueakChestSmall = 0x11, SqueakChestBig = 0x14, SqueakBubbleGimmick = 0x11;
        /* Squeak Squad's chests as KatAM's, small and big (0x80, 0x81), their treasure index chest.c's SQUEAK_CHEST, their own number
         * (0-127, in the rooms' order) in the template's +0x12 (rooms.c keeps which are open, saved). Their treasures (the treasure
         * number at the item's +0xD; the stage result's info screens, overlay 9's 0x0220AC70: by its range) aren't in KatAM yet: each
         * holds a consumable (KatAM's chest items, dealt in turn as the NiDL rooms' new chests); the ranges of the ones to carry Squeak
         * Squad's items (SqueakTreasureItems: the owner's list) are kept for them; */
        const byte SqueakChestIndex = 0x9F, KatAMBigChestType = 0x81;
        const int SqueakChestsMax = 128;
        static readonly (int first, int last, string name, bool item)[] SqueakTreasures = {
            (0, 4, "Star Seal", false), (5, 5, "Sound Player", false), (6, 13, "Vitality", true), (14, 20, "Secret Door Key", false),
            (21, 27, "Ghost Medal", false), (28, 50, "Ability Scroll", true), (51, 61, "Music", false), (62, 69, "Boss Battle Badge", false),
            (70, 87, "Spray Paint", true), (88, 92, "Copy Palette", false), (93, 99, "Secret Map", true), (100, 118, "Graphic Piece", false),
            (119, 119, "Shortcake", false),
        };
        // The Squeak Squad items' gone bits (rooms.c's kinds 5-7: SQUEAK_TAKEN_KIND on, 256 numbers each);
        const byte SqueakTakenKind = 5;

        /* SqueakFights(); Whether a room holds a mini-boss (Squeak Squad's 0x33-0x36, or one of KatAM's own: KatAMMidbossTypes) and a
         * boss (0x37-0x3F), as SqueakObjects() places them (the chase spawns left out); */
        static (bool midboss, bool boss) SqueakFights(byte[] squeak, SqueakRoomFiles f) {
            byte[] objects = f.Objects == null ? null : SqueakFileAnyCase(squeak, f.Objects);
            bool midboss = false, boss = false;
            for (int i = 0, count = objects == null ? 0 : ReadShort(objects, 12); i < count; i++) {
                int at = 14 + i * 16, id = ReadShort(objects, at + 2);
                if ((objects[at + 0xB] & SqueakChaseSpawn) != 0) continue;
                if (SqueakBosses.ContainsKey(id)) { if (id >= 0x37) boss = true; else midboss = true; }
                else if (SqueakNatives.TryGetValue(id, out var native) && KatAMMidbossTypes.Contains(native.type)) midboss = true;
            }
            return (midboss, boss);
        }

        /* SqueakObjects(); A room's objects as KatAM templates: its .doe's enemies, mid-bosses, bosses and items, and its floating
         * bubbles (section 10's gimmick 0x11). Spots: Squeak Squad's are the feet (y from the room's top), KatAM's 8 pixels over them
         * (its own enemies' and items' on a floor). A mid-boss or boss fights on a screen: its template's area (SqueakArena()), KatAM's
         * camera held there by an arena object (Boss()). Chests: KatAM's (see SqueakChestIndex). No enemy stands on a door or where
         * one lands Kirby (`doorCells`: SqueakDoorClear cells across, 2 up or down, at its spot or the floor under it), as the NiDL
         * rooms' (the rulebook's rule 2, BuildObjects()): it's left out. A world's boss in its boss room (`bossShard`: the shard it leaves
         * + 1, its template's unk11, "ROM Expansion/squeak.h"'s SqueakBossShard) leaves it as it goes, the way out to the hub (the
         * rulebook's rule 8). Not yet: the chase spawns, the chests' Squeak Squad treasures; */
        const int SqueakDoorClear = 3;
        const byte SqueakChestAfterBoss = 1;     // squeak.h's SQUEAK_CHEST_AFTER_BOSS (a chest's template unk14);
        /* (a Gold Waddle Dee's chest, squeak.h's SQUEAK_CHEST_AFTER_ENEMY: the DS's Gold Waddle Dees (enemy 0x0E) carry flag 8, as its
         * bosses do, and their room's one chest, a piece of the Secret Map, comes once one's beaten (the owner: "Golden Waddle Dee's
         * when defeated or inhaled, they drop the chest they possess in the position they died"): the chest's template's unk18 the Gold
         * Waddle Dee's template's unk2 and unk3, chest.c's SqueakEnemyChestWait); */
        const byte SqueakChestAfterEnemy = 2;
        const int SqueakGoldWaddleDee = 0x0E, SqueakDropsReward = 8;
        /* The Vitality halves (treasures 6-13): KatAM's Vitality (chest item 6) in a chest flagged a half (its template's unk14's high
         * byte, squeak.h's SQUEAK_CHEST_HALF); their chests' numbers in chest.c's gSqueakVitalityChests, two of them opened one Vitality; */
        const int SqueakVitalityFirst = 6, SqueakVitalityLast = 13, SqueakVitalityChestsMax = 8;
        const byte KatAMVitality = 6, SqueakChestHalf = 1;
        // (the spray paints KatAM doesn't have: treasures 83-87, Squeak Squad's sprays 14-18; KatAM's chest items 0x14-0x21 its sprays, by color);
        const int SqueakSprayFirst = 83, KatAMSprayItem = 0x14;
        static (List<byte[]> templates, List<byte> types) SqueakObjects(byte[] squeak, SqueakRoomFiles f, SqueakMap map, byte[] collision, bool arenas,
                                                                         List<(int x, int y)> doorCells, byte bossShard, ref int taken, ref int chests,
                                                                         List<int> vitalityChests = null, int[] sprayCans = null,
                                                                         Dictionary<(SqueakRoomFiles room, int record), int> mapChests = null,
                                                                         Dictionary<int, byte[]> heldChests = null, SqueakLairPlan lair = null,
                                                                         List<byte[]> chaseChests = null) {
            List<byte[]> templates = new List<byte[]>();
            List<byte> types = new List<byte>();
            int width = map.Width, height = map.Height;
            bool Open(int cx, int cy) => cx >= 0 && cy >= 0 && cx < width && cy < height &&
                                         collision[cy * width + cx] is KatAMAir or KatAMLadder or (>= 0x34 and <= KatAMWater);
            // (an enemy a bubble held stands on the floor under it, up to 10 cells down; over a pit it stays);
            int OnFloor(int x, int y, int reach = 10) {
                for (int cy = y / 16; cy < height && cy <= y / 16 + reach; cy++)
                    if (!Open(x / 16, cy)) return cy == y / 16 ? y : cy * 16;
                return y;
            }
            void Add(byte[] template) {
                templates.Add(template);
                if (!types.Contains(template[12])) types.Add(template[12]);
            }
            bool NearDoor(int cx, int cy) => doorCells.Any(c => Math.Abs(c.x - cx) <= SqueakDoorClear && Math.Abs(c.y - cy) <= 2);
            // (the dark star isn't an enemy: it waits by the stage's first door to lead Kirby on, where Squeak Squad puts it);
            void Enemy(int x, int y, byte type, byte behavior, byte variant = 0, bool byDoor = false) {
                if (!byDoor && (NearDoor(x / 16, (y - 1) / 16) || NearDoor(x / 16, (OnFloor(x, y) - 1) / 16))) return;
                byte[] template = ObjectTemplate(x, y - 8, templates.Count, type);
                template[14] = behavior;
                template[0x12] = variant;
                Add(template);
            }
            /* A mid-boss or boss and, first, its arena (enemies/arena.c: it holds the camera on the fight's screen, from Kirby's first
             * frame in a room a screen big, as Squeak Squad's arenas: subtype2 ONE_SCREEN). The arena is NiDL's type's (0x31): without
             * the NiDL ROM, none yet (the camera follows Kirby); */
            void Boss(int x, int y, byte type, byte behavior, byte shard = 0, int floorRow = -1) {
                /* (a Squeak Lair's fight, `floorRow` set, whose spot is in the ground (the Squeakers' hideouts: its first Squeaker's,
                 * 12 pixels into the treasure pile's floor) stands on that floor's top: in it, it was pushed out sideways); */
                if (floorRow >= 0 && !Open(x / 16, (y - 1) / 16)) {
                    int cy = (y - 1) / 16;
                    while (cy > 0 && !Open(x / 16, cy - 1)) cy--;
                    y = cy * 16;
                }
                byte[] template = ObjectTemplate(x, y - 8, templates.Count + 1, type);
                template[14] = behavior;
                template[0x11] = shard;
                SqueakArena(template, map, collision, floorRow);
                if (!arenas) {
                    template[3] = (byte)templates.Count;
                    Add(template);
                    return;
                }
                byte[] arena = (byte[])template.Clone();
                arena[3] = (byte)templates.Count;
                arena[12] = NightmareID; arena[14] = ArenaBehavior;
                arena[0x10] = (byte)(floorRow < 0 && width * 16 <= SqueakScreenRoom.width && height * 16 <= SqueakScreenRoom.height ? 2 : 0);
                Add(arena);
                Add(template);
            }
            int takenAt = taken, chestAt = chests;
            void Item(int x, int y, byte type) {
                byte[] template = ObjectTemplate(x, y - 8, templates.Count, type);
                template[2] = (byte)(SqueakTakenKind + takenAt / 256);
                template[3] = (byte)(takenAt++ % 256);
                Add(template);
            }
            void Ability(int x, int y, int ability) {
                if (ability == 0) RandomPedestal(x, y);
                else if (SqueakAbilityEnemies.TryGetValue(ability, out var enemy)) Enemy(x, OnFloor(x, y), enemy.type, enemy.behavior, enemy.variant);
                else Item(x, y, KatAMCherry);
            }
            // (KatAM's random ability pedestal on the floor under the spot, its cell's middle, as KatAM's own: 16 pixels over the floor);
            void RandomPedestal(int x, int y) {
                int cx = x / 16 * 16 + 8;
                byte[] template = ObjectTemplate(cx, OnFloor(cx, y) - 16, templates.Count, KatAMRandomPedestal);
                template[14] = 0;
                template[15] = 0;
                Add(template);
            }

            byte[] objects = f.Objects == null ? null : SqueakFileAnyCase(squeak, f.Objects);
            // (the records a grass tuft or a barrel hides come out of it: SqueakHeldRecords());
            (HashSet<int> heldItems, HashSet<int> heldEnemies) = SqueakHeldRecords(map);
            // (a lair whose Squeak isn't among its records, the Squeakers' hideout picked by Randomize Squeak Lairs: its fight where its first
            // Squeaker stands, the lair's Squeak leading the others);
            bool lairSqueakHere = lair != null && objects != null && Enumerable.Range(0, ReadShort(objects, 12)).Any(i => (objects[14 + i * 16 + 0xB] & SqueakChaseSpawn) != 0
                && SqueakCharacters.TryGetValue(ReadShort(objects, 14 + i * 16 + 2), out int c) && c == lair.Character);
            // (the room's Gold Waddle Dee holding its chest, SqueakChestAfterEnemy);
            byte[] goldDee = null;
            if (objects != null) {
                for (int i = 0, count = ReadShort(objects, 12); i < count; i++) {
                    int at = 14 + i * 16, id = ReadShort(objects, at + 2), x = ReadShort(objects, at + 4), y = ReadShort(objects, at + 6);
                    int v = objects[at + 8], w = objects[at + 9], flags = objects[at + 0xB];
                    /* (a Squeak Lair's fight, PlanSqueakLevels(): its Squeak at its spot a mini-boss on 0x40 as the randomizer's
                     * (SQUEAK_MIDBOSS, squeak.h; its template's unk22 SQUEAK_LAIR_FIGHT), its fight area the screen over the floor Kirby
                     * lands on; its Squeakers (LairSqueakers at most, one a spot) enemies around it, as the DS's hideouts'); */
                    if (lair != null && (flags & SqueakChaseSpawn) != 0 && !heldEnemies.Contains(i) && SqueakChaseIds.Contains(id) && SqueakCharacters.TryGetValue(id, out int squeakOne)) {
                        if ((squeakOne == lair.Character || !lairSqueakHere && squeakOne == SqueakerCharacter) && !templates.Any(t => (t[0x22] & SqueakLairFight) != 0)) {
                            Boss(x, y, SqueakAsMidboss, (byte)(SqueakMidbossFlag | lair.Character), floorRow: lair.Landing.y - 1);
                            templates[^1][0x22] |= SqueakLairFight;
                        } else if (squeakOne == SqueakerCharacter && templates.Count(t => t[12] == SqueakID && t[14] == SqueakerCharacter) < LairSqueakers
                                   && !templates.Any(t => t[12] == SqueakID && ReadShort(t, 6) == x && ReadShort(t, 8) == y - 8)) {
                            Enemy(x, y, SqueakID, (byte)squeakOne, (byte)v);
                        }
                        continue;
                    }
                    if ((flags & SqueakChaseSpawn) != 0 || heldEnemies.Contains(i)) continue;
                    if (SqueakNatives.TryGetValue(id, out var native)) {
                        byte behavior = v < native.behaviors.Length ? native.behaviors[v] : native.behaviors[0];
                        if (id == 0x03 && w == 1) behavior = 6;
                        if (KatAMMidbossTypes.Contains(native.type)) Boss(x, y, native.type, behavior);
                        else Enemy(x, y, native.type, behavior);
                    } else if (SqueakCharacters.TryGetValue(id, out int character)) {
                        // (Daroach's scene: his second byte his facing, 0 left; squeak.h's variant bit 0 right);
                        byte variant = (byte)(id == 0x45 ? (w != 0 ? 1 : 0) : v);
                        // (the dark star at its spot itself, not stood on a floor: an enemy's template is 8 over its spot);
                        bool darkStar = character == SqueakDarkStarCharacter;
                        int before = templates.Count;
                        Enemy(x, darkStar ? y + 8 : y, SqueakID, (byte)character, variant, darkStar);
                        if (id == SqueakGoldWaddleDee && (flags & SqueakDropsReward) != 0 && templates.Count > before) goldDee = templates[^1];
                    } else if (SqueakBosses.TryGetValue(id, out int boss)) {
                        if (id == 0x3D && w == 1) boss = DarkDaroachIndex;
                        Boss(x, y, SqueakDefinitionOf(boss).MidbossType, (byte)(SqueakMidbossFlag | boss), id >= 0x37 ? bossShard : (byte)0);
                        // (Dark Nebula's own defeat fades his music out as the DS's: KatAM's mini-boss defeat leaves it be, unk22 bit 2);
                        if (id == 0x3F) templates[^1][0x22] |= KeepFightMusic;
                    }
                }
                int items = ReadShort(objects, 6);
                for (int i = 0, count = items + 2 <= objects.Length ? ReadShort(objects, items) : 0; i < count; i++) {
                    int at = items + 2 + i * 16, kind = ReadShort(objects, at + 2), x = ReadShort(objects, at + 4), y = ReadShort(objects, at + 6);
                    /* (a chest a barrel or a grass tuft holds (2-1-3's, 4-1-6's and 7-4-5's Ability Scrolls, 1-5-1's, 4-3-8's and 5-5-4's
                     * Copy Palettes, 2-5-5's Graphic Piece): made as the others, its template kept for the holder, heldChests, which
                     * lets it out, squeak/gimmicks.c's LetOutChest(), not put in the room); */
                    bool held = heldItems.Contains(i), heldChest = held && heldChests != null && kind is SqueakChestSmall or SqueakChestBig;
                    /* (a chest the DS brings out only in the chase once the stage's treasure is carried (its flag 4: 6-1-7's, 6-5-6's,
                     * 7-1-6's, 7-3-4's and 8-1-15's Ability Scrolls, five Vitality halves, ...; the owner: the chase-only chests "only in
                     * the chase"): made as the others, its template kept for the room's chase, chaseChests, which brings it out once the
                     * chase is on (squeak/gimmicks.c's ChaseChest()), not put in the room; in a stage with no chase, there from the start;
                     * the chase's other things stay out); */
                    bool chaseChest = kind is SqueakChestSmall or SqueakChestBig && (objects[at + 0xB] & SqueakChaseSpawn) != 0;
                    if ((objects[at + 0xB] & SqueakChaseSpawn) != 0 && !chaseChest || held && !heldChest) continue;
                    if (kind is SqueakChestSmall or SqueakChestBig) {
                        /* (on the floor under its spot: Squeak Squad's is sometimes the floor's top, sometimes a cell over it, its chests
                         * falling onto it; KatAM's CreateChest() puts a chest's bottom on the cell edge at or under its template's y + half
                         * its height, a big one's 16, a small one's 8: a big one's at the floor's top - 8 sank a cell into the floor and fell
                         * out of the room; Jam Jungle 4's fifth room's is in the rock on the DS, drawn over by it; the bosses' fall from
                         * high up); */
                        /* (a chase's chest over a pit: on the dry floor nearest beside its spot, the DS's chase room being another
                         * (7-3-4's scroll, between its pillars over the water: the ledge right of them); */
                        bool DryFloor(int cx) {
                            if (cx < 0 || cx >= width) return false;
                            for (int cy = y / 16; cy < height && cy <= y / 16 + 6; cy++) {
                                byte c = collision[cy * width + cx];
                                if (c != KatAMAir) return cy > y / 16 && !Open(cx, cy);
                            }
                            return false;
                        }
                        if (chaseChest && OnFloor(x, y, height) == y && Open(x / 16, y / 16)) {
                            for (int d = 1; d <= 8; d++) {
                                if (DryFloor(x / 16 + d)) { x = (x / 16 + d) * 16 + 8; break; }
                                if (DryFloor(x / 16 - d)) { x = (x / 16 - d) * 16 + 8; break; }
                            }
                        }
                        int floor = OnFloor(x, y, height);
                        /* (one on a moving platform or a thunder cloud (gimmicks 4, 5: their top 8 pixels over their spot), 1-6-7's, 6-4-4's,
                         * 7-5-1's: on its top, where the platform carries it, squeak/gimmicks.c's PlatformCarryObjects(); on the floor under
                         * it, the platform went off without it: the owner, "Some chests on top of movable platforms are not moving with the
                         * platforms"); */
                        SqueakGimmick under = map.Gimmicks().FirstOrDefault(g => g.Type is SqueakPlatformGimmick or SqueakCloudGimmick
                                                                                  && Math.Abs(g.X - x) <= 24 && y >= g.Y - 24 && y <= g.Y);
                        if (under != null) floor = under.Y - 8;
                        /* (one buried in the rock, under its top: on the floor of the first open space under it, where it falls to; a
                         * door over it is no rock: 2-6-3's big chest on its ledge, a mirror's door cell over it, fell to the floor below,
                         * on the Cherry there); */
                        bool OpenOrDoor(int cx, int cy) => Open(cx, cy) || cx >= 0 && cy >= 0 && cx < width && cy < height && collision[cy * width + cx] >= 0xFC;
                        if (under == null && !Open(x / 16, y / 16) && !OpenOrDoor(x / 16, (y - 1) / 16)) {
                            int cy = y / 16;
                            while (cy < height && !Open(x / 16, cy)) cy++;
                            if (cy < height) floor = OnFloor(x, cy * 16, height);
                        }
                        /* (a big one, Squeak Squad's platinum chest, is KatAM's small chest too (the owner: "We should drop the big
                         * platinum chests from the ROM and use KatAM's small chests instead. Ensure these have physics."): KatAM's
                         * chests fall, ObjectMain's gravity, as nothing sets their flag 0x40); */
                        byte[] chest = ObjectTemplate(x, floor - 8, templates.Count, SmallChestType);
                        chest[14] = KatAMChestItems[chestAt * 17 % KatAMChestItems.Length];
                        if (goldDee != null && !held && !chaseChest) {
                            chest[0x14] = SqueakChestAfterEnemy;
                            chest[0x18] = goldDee[2];
                            chest[0x19] = goldDee[3];
                        }
                        // (a Vitality half, treasures 6-13: KatAM's Vitality, flagged a half; two make one: chest.c's NidlChestOpen);
                        int treasure = objects[at + 0xD];
                        if (treasure >= SqueakVitalityFirst && treasure <= SqueakVitalityLast) {
                            chest[14] = KatAMVitality;
                            chest[0x15] = SqueakChestHalf;
                            vitalityChests?.Add(chestAt);
                        }
                        /* (a spray paint KatAM doesn't have, treasures 83-87 (70-82 KatAM's, by name: they hold items): KatAMExpansionSprays.cs;
                         * the chest's item KatAM's spray of the closest color, whose can rises, its template's unk16 the spray's number + 1:
                         * chest.c's NidlChestOpen gives the spray, not KatAM's); */
                        if (sprayCans != null && treasure >= SqueakSprayFirst && treasure < SqueakSprayFirst + sprayCans.Length) {
                            chest[14] = (byte)(KatAMSprayItem + sprayCans[treasure - SqueakSprayFirst]);
                            chest[0x16] = (byte)(treasure - SqueakSprayFirst + 1);
                        }
                        /* (a world's map, KatAMExpansionMaps.cs: the chest's item one of KatAM's maps, its template's unk16 the mark and the
                         * map's number + 1: chest.c's NidlChestOpen gives the area's map, not KatAM's); */
                        if (mapChests != null && mapChests.TryGetValue((f, i), out int areaMap)) {
                            chest[14] = AreaMapItem(areaMap);
                            chest[0x16] = (byte)(areaMap + 1);
                            chest[0x17] = ChestMarkMap;
                            squeakMapsPlaced[areaMap] = f;
                        }
                        /* (a piece of the Secret Map, treasures 93-99, KatAMExpansionMaps.cs: the chest's item one of KatAM's maps, its template's
                         * unk16 the piece's mark and number + 1: chest.c's NidlChestOpen gives the piece, not KatAM's map); */
                        if (treasure >= SqueakSecretMapFirst && treasure < SqueakSecretMapFirst + SecretMapPieces) {
                            int piece = treasure - SqueakSecretMapFirst;
                            chest[14] = AreaMapItem(piece);
                            chest[0x16] = (byte)(piece + 1);
                            chest[0x17] = ChestMarkPiece;
                            squeakPiecesPlaced[piece] = f;
                        }
                        /* (an Ability Scroll, treasures 28-50, KatAMExpansionMaps.cs: as a piece, its mark CHEST_MARK_SCROLL: chest.c's
                         * NidlChestOpen gives the scroll, owned in its own bits, not KatAM's map); */
                        if (treasure >= SqueakScrollFirst && treasure < SqueakScrollFirst + SqueakScrolls) {
                            int scroll = treasure - SqueakScrollFirst;
                            chest[14] = AreaMapItem(scroll % 9);
                            chest[0x16] = (byte)(scroll + 1);
                            chest[0x17] = ChestMarkScroll;
                            squeakScrollsPlaced[scroll] = f;
                        }
                        chest[0x11] = SqueakChestIndex;
                        chest[0x12] = (byte)chestAt++;
                        if (heldChest) heldChests[i] = chest;
                        else if (chaseChest && chaseChests != null) chaseChests.Add(chest);
                        else Add(chest);
                    }
                    else if (kind == SqueakRandomItem) RandomPedestal(x, y);
                    else if (SqueakItemKinds.TryGetValue(kind, out byte item)) Item(x, y, item);
                    else if (kind > SqueakAbilityKinds) Ability(x, y, kind - SqueakAbilityKinds);
                }
            }

            /* The floating bubbles (the gimmicks' entries: type, index, x, y; their table: a byte, its high half 1 a food, 0 an ability,
             * its low half 1 an item of the room's list (the second byte its index: 4-6-8's chest, 8-1-11's big chest, 6-2-5's
             * Cherry), 0 the food's or ability's ID (the second byte)). One holding an item of the list adds nothing: the item stands
             * there itself (the owner: "There are instances where there's a chest in a bubble and [...] a cherry shows on top of the
             * chest, it's either one or the other but the chest always wins"); */
            int gimmicks = map.Section(10) + 2;
            if (map.Section(10) > 0 && gimmicks + 8 <= map.Data.Length) {
                int entries = ReadShort(map.Data, gimmicks), tables = ReadShort(map.Data, gimmicks + 2);
                int entryCount = map.Data[gimmicks + 6], tableCount = map.Data[gimmicks + 7];
                byte[] Parameters(int type, int index) {
                    for (int j = 0; j < tableCount; j++) {
                        int table = gimmicks + ReadShort(map.Data, gimmicks + tables + j * 2), size = ReadShort(map.Data, table + 2);
                        if (map.Data[table] == type && index < map.Data[table + 1]) return map.Data[(table + 4 + index * size)..(table + 4 + index * size + size)];
                    }
                    return null;
                }
                for (int e = 0; e < entryCount; e++) {
                    int at = gimmicks + entries + e * 8;
                    if (map.Data[at] != SqueakBubbleGimmick || Parameters(map.Data[at], map.Data[at + 1]) is not byte[] held || held.Length < 2) continue;
                    int x = ReadShort(map.Data, at + 2), y = ReadShort(map.Data, at + 4);
                    if ((held[0] & 0x0F) == 1) continue;
                    if ((held[0] & 0x10) != 0) Item(x, y, SqueakBubbleItems.GetValueOrDefault(held[1], KatAMCherry));
                    else Ability(x, y, held[1]);
                }
            }
            taken = takenAt;
            chests = chestAt;
            /* (a world's boss room: the boss's shard comes over its chest's spot, centred on the room's art, as the DS's reward
             * (SqueakBossShard); the chest itself, the DS's reward dropped after the fight, is moved into the room before it
             * (AddSqueakRooms(), SqueakAntechamber: no treasure in a boss room); left there, it would show only once the boss is gone,
             * squeak.h's SQUEAK_CHEST_AFTER_BOSS); */
            if (bossShard != 0 && templates.FirstOrDefault(t => t[12] is >= 0x40 and <= 0x42 && t[14] >= SqueakMidbossFlag && t[0x11] == bossShard) is byte[] bossTemplate) {
                foreach (byte[] chest in templates.Where(t => t[12] == SmallChestType && t[0x11] == SqueakChestIndex)) {
                    chest[0x14] = SqueakChestAfterBoss;
                    WriteShort(bossTemplate, 0x14, ReadShort(chest, 6));
                }
            }
            return (templates, types);
        }

        /* SqueakArena(); A mid-boss's or boss's fight area (its template's +0x1A: left and top from its spot, width, height): a screen
         * (KatAM's 240 by 160, the room's if smaller, 8 pixels in from its sides) across the room's middle when the room is two screens
         * wide at most, else round its spot; its bottom 32 pixels under the floor it stands on (the HUD over half of that). Squeak
         * Squad's arenas are rooms a screen big (SqueakScreenRoom: 288 by 240 pixels, the DS's 256 by 192 and a margin); */
        static readonly (int width, int height) SqueakScreenRoom = (288, 240);
        // (`from`: the row to look for the floor from, under the spot if none: a Squeak Lair's, the row Kirby lands on);
        /* (a Squeak Lair's, `from` its landing's floor: the whole room, the camera scrolling over it all as the DS's, its hideouts (18 by
         * 15 cells, 18 by 32) a screen and more on the GBA; one screen's cut its top off, the owner: "Some Squeak Lair rooms get cutoff
         * and the camera does not scroll all the way up"); */
        static void SqueakArena(byte[] template, SqueakMap map, byte[] collision, int from = -1) {
            const int ScreenWidth = 240, ScreenHeight = 160, Edge = 8, UnderFloor = 32, ArenaRoom = 36;
            int width = map.Width * 16, height = map.Height * 16, x = ReadShort(template, 6), y = ReadShort(template, 8);
            if (from >= 0) {
                WriteShorts(template, 0x1A, Edge - x, Edge - y, width - 2 * Edge, height - 2 * Edge);
                return;
            }
            int arenaWidth = Math.Min(ScreenWidth, width - 2 * Edge), arenaHeight = Math.Min(ScreenHeight, height - 2 * Edge);
            int left = map.Width <= ArenaRoom ? (width - arenaWidth) / 2 : x - arenaWidth / 2;
            int floor = from >= 0 ? from : y / 16;
            while (floor < map.Height && collision[floor * map.Width + Math.Clamp(x / 16, 0, map.Width - 1)] is KatAMAir or KatAMLadder or (>= 0x34 and <= KatAMWater)) floor++;
            int top = floor * 16 + UnderFloor - arenaHeight;
            left = Math.Clamp(left, Edge, width - Edge - arenaWidth);
            top = Math.Clamp(top, Edge, height - Edge - arenaHeight);
            WriteShorts(template, 0x1A, left - x, top - y, arenaWidth, arenaHeight);
        }

        /* Squeak Squad's rooms in the ROM (AddSqueakRooms()): the 8 worlds' stages (SqueakWorldRooms(), the test stages left out), each
         * room whole, numbered from SqueakFirstRoom up (past KatAM's 0x3D7 rooms, its goal games' 0x3D4-0x3DD among them). The three
         * tables with a row per room, gRoomProps, gUnk_08D6CD0C (room info: area, CPU helpers' routes) and gUnk_0893CB44 (where a warp
         * star's flight lands), move, grown to the last room (the rows between KatAM's and Squeak Squad's free), every reference to
         * them repointed: KatAM's code names each by its base alone, as the expansion's does (checked here). The tables the props index
         * (foreground info, palettes, tilemaps, collision, backgrounds, objects, CPU helpers' zones, doors lists) get the rooms'
         * entries after NiDL's, in the room AddRooms() leaves in its copies (roomTables; without NiDL's rooms, copies of KatAM's).
         * The randomizer and the map viewer read KatAM's own rows, which stay as they were; */
        const int SqueakFirstRoom = 0x400, SqueakTableRoom = 1024;
        // (rooms.c's visited doors: (NIDL_VISITED_SIZE - NIDL_TAKEN_SIZE - SQUEAK_CHESTS_SIZE - SQUEAK_WORLD_SIZE) / 2 - 2 lists past KatAM's);
        const int VisitedLists = (0xA00 - 0x80 - 0x10 - 0x20 - 4) / 2 - 2;
        // (rooms.c's gNidlRoomTops: the camera from the room's top, 1, down to 32 pixels over its bottom, ROOM_DS_BOTTOM: the DS's);
        const byte SqueakCameraTop = 0x81;
        /* (and 5-7, a copy of Vocal Volcano's boss stage, 5-6's two rooms with Bohboh: left out as the Arena's copies of the boss rooms;
         * and 2-8, another test stage: its rooms 2 to 6 have no tiles at all, only collision, its first a jumble of the generic tileset's
         * blocks (00_f, 00_b, as 1-8's and 1-9's), its chests treasure 0: the owner, "Room 0x46C is completely garbled"); */
        static readonly HashSet<(int area, int stage)> SqueakTestStages = new HashSet<(int, int)> { (1, 8), (1, 9), (2, 8), (2, 9), (5, 7) };
        /* (and 8-4's last two rooms: 8-4-8, a copy of 8-4-6, Dark Nebula's fight (the same map and objects), and 8-4-9, of 8-4-7, the room
         * before it (the same, its door to 8-4-8): his fight once, in 8-4-6, his shard there; nothing in the stage leads to the copies); */
        static readonly HashSet<(int area, int stage, int room)> SqueakCopiedRooms = new HashSet<(int, int, int)> { (8, 4, 8), (8, 4, 9) };

        /* roomTables; The room tables' copies AddRooms() made (where, how many entries, and how many fit: SqueakTableRoom more), null
         * without NiDL's rooms (ForgetRooms()); */
        static Dictionary<string, (int at, int count, int capacity)> roomTables;

        // The Squeak Squad rooms of the last ROM made: their files, room number and size in blocks (for the map viewer and the tests);
        internal sealed record SqueakPortedRoom(SqueakRoomFiles Files, int Room, int Width, int Height);
        /* SqueakLairMinibosses; The Squeak Lairs' mini-bosses (their room, their template's offset in the ROM): the randomizer's mini-boss
         * randomization takes them with KatAM's (KatAMMinibosses; the owner: "Any boss can appear in these lairs and these can also be
         * randomized as well"); */
        static readonly List<(int room, int address)> squeakLairMinibosses = new List<(int, int)>();
        public static IReadOnlyList<(int room, int address)> SqueakLairMinibosses => squeakLairMinibosses;
        /* SqueakLairEscorts; The Squeakers around a lair's Squeak (their room, their template's offset in the ROM): the mini-boss
         * randomization takes them off the stage when the lair's fight isn't its Squeak any more (KatAMMinibosses; the owner: "Squeak
         * hideouts have the base Squeak enemy and a miniboss [...] It's either one or the other"); */
        static readonly List<(int room, int address)> squeakLairEscorts = new List<(int, int)>();
        public static IReadOnlyList<(int room, int address)> SqueakLairEscorts => squeakLairEscorts;
        /* SqueakAntechamber; The room before a world's boss room as it was written (AddSqueakRooms()): its templates and types, its doors'
         * places, its lists' indexes (to be written again with the boss room's chest), its collision, its doors' cells and the one(s) to
         * the boss room; Spot(): where the chest goes, a floor cell (open, its cell over it open too, the one under it floor: solid or a
         * thin floor, not spikes or water) 3 cells across or 2 up or down from every door, the nearest to the door to the boss room; */
        sealed class SqueakAntechamber {
            public List<byte[]> Templates; public List<byte> Types; public List<(int room, int x, int y)> DoorPlaces;
            public int ObjectsIndex, DoorsIndex;
            readonly byte[] collision; readonly int width, height; readonly List<(int x, int y)> doors, toBoss;
            public readonly List<(int x, int y)> Taken = new List<(int x, int y)>();
            public SqueakAntechamber(List<byte[]> templates, List<byte> types, List<(int, int, int)> doorPlaces, int objectsIndex, int doorsIndex,
                                     byte[] collision, int width, int height, List<(int x, int y)> doors, List<(int x, int y)> toBoss) {
                (Templates, Types, DoorPlaces, ObjectsIndex, DoorsIndex) = (templates, types, doorPlaces, objectsIndex, doorsIndex);
                (this.collision, this.width, this.height, this.doors, this.toBoss) = (collision, width, height, doors, toBoss);
            }
            public (int, int)? Spot() {
                byte At(int x, int y) => x < 0 || y < 0 || x >= width || y >= height ? KatAMSolid : collision[y * width + x];
                (int x, int y) target = toBoss.Count > 0 ? toBoss[0] : (width / 2, height / 2);
                (int, int)? best = null;
                int bestDistance = int.MaxValue;
                for (int y = 1; y < height - 1; y++)
                    for (int x = 1; x < width - 1; x++) {
                        byte under = At(x, y + 1);
                        if (At(x, y) != KatAMAir || At(x, y - 1) != KatAMAir || !(under == KatAMSolid || under == KatAMThin || under == KatAMFirmThin)) continue;
                        if (doors.Any(d => Math.Abs(d.x - x) < 3 && Math.Abs(d.y - y) <= 2) || Taken.Contains((x, y))) continue;
                        int distance = (x - target.x) * (x - target.x) + (y - target.y) * (y - target.y);
                        if (distance < bestDistance) (best, bestDistance) = ((x, y), distance);
                    }
                return best;
            }
        }

        /* OffStageX; Where a template stands that KatAM never spawns: KatAM's spawner (CreateLevelObjects) makes only the ones within a
         * screen of the camera, and no room is near this wide (the Map Viewer skips them too); */
        public const int OffStageX = 0x7FF0;
        static List<SqueakPortedRoom> squeakRooms = new List<SqueakPortedRoom>();
        public static IReadOnlyList<SqueakPortedRoom> SqueakPortedRooms => squeakRooms;
        // (and the doors the levels' plan gave them, PlanSqueakLevels(), by room);
        static Dictionary<int, List<Door>> squeakPlanned = new Dictionary<int, List<Door>>();
        // (the DS doors left without a mirror, their rooms holding KatAM's 31 objects already: AddSqueakRooms());
        static int squeakDoorMirrorsLeft;
        public static int SqueakDoorMirrorsLeft => squeakDoorMirrorsLeft;
        public static IReadOnlyDictionary<int, List<Door>> SqueakPlannedDoors => squeakPlanned;

        // SqueakWorldRooms(); The 8 worlds' rooms, stage by stage, in play order;
        static List<SqueakRoomFiles> SqueakWorldRooms(byte[] squeak) {
            List<SqueakRoomFiles> rooms = new List<SqueakRoomFiles>();
            for (int area = 1; area <= 8; area++)
                for (int stage = 0; stage <= 12; stage++)
                    if (!SqueakTestStages.Contains((area, stage)))
                        rooms.AddRange(SqueakStageRooms(squeak, area, stage).Where(f => !SqueakCopiedRooms.Contains((f.Area, f.Stage, f.Index))));
            return rooms;
        }

        /* Squeak Squad's cannons. Gimmick 0xB (0x02224730): a cannon Kirby gets in, its parameter's bits 2-4 its way (0 up, then a
         * turn of 45 degrees clockwise each: KatAM's eight ways, gUnk_0834C160's order), bits 0-1 its kind: 0 fixed (it fires him its
         * way about 35 frames after he gets in, checked in the DS emulator: 4-3-6's chain of them), 1 turning (it turns, A fires); its
         * spot is the cannon's foot, the shot leaving 8 pixels up (8-1-5's turning cannon: its eight ways go through the middles of the
         * eight doors round it, each behind metal blocks the shot breaks). In KatAM: its eight-direction cannon (EightDirCannonType,
         * 8 pixels up, +0x14 the way; +0x22 bit 0 firing on its own; a fixed one also
         * SqueakFixedCannon in +0x11, a turning one SqueakTurningCannon: chest.c's SqueakCreateCannon holds a fixed one's way and fires
         * after Squeak Squad's wait, where KatAM's turns once first, and carries the shot on as far as Squeak Squad's go, where KatAM's
         * drops him after 45 frames; Squeak Squad's grey metal blocks (the blocks grid's block 1) and brown bricks (its solids of
         * family 0x00, its kind byte 0: 4-3-6's shots go through them on the DS, 1-6-5's wall them round a chest) are KatAM's metal
         * blocks, which its shots and strong abilities break; the grid's other families (0x08, 0x10, 0x18, 0x20 Cushy Cloud's
         * clouds, 0x28, 0x38) stay solid). Gimmick 0xC (0x02220710): a fuse cannon, its parameter's first byte the stage's room it shoots Kirby
         * to (0-based), the second that room's door he comes out at (its number); its fuse in the blocks' grid (section 6: ids 0x81
         * to 0x8F, the low bits its ways: 1 right, 2 left, 4 down, 8 up), from where it's lit (one way only) through the walls to the
         * cell under the cannon. In KatAM (SqueakFuse()): its cannon (CannonType, as the NiDL rooms' Transport cannons: on the fuse's
         * last cell, +0x12 the room, +0x18 the landing cell + 1, the fuse's state slot 1) and fuse: KatAM's fuse cells (0x22-0x27,
         * cannon_fuse.c, solid), its fuse object (FuseType) where it's lit, KatAM's fuse burning down out of the object's cell into
         * the first (lit from the side, 5-1-1's: the object a cell up, the lit cell a corner down into the rest: the burner's first
         * step, down out of the object's cell, clears what its end needs to fire the cannon); each fuse cell's
         * two special block records, the wall without the rope (the nearest wall's look) and its own look, as the NiDL rooms'; */
        const int SqueakTurnCannon = 0xB, SqueakFuseCannon = 0xC;
        const byte EightDirCannonType = 0x8B, SqueakFixedCannon = 0x9E, SqueakTurningCannon = 0x9D;
        // (a switch: gimmick 0, KatAM's object 0x8D with its template's unk11 squeak.h's SQUEAK_SWITCH_OBJECT, chest.c's SqueakCreateSwitch);
        const byte HammerPegType = 0x8D, SqueakSwitchObject = 0x9C;
        const int SqueakSwitchGimmick = 0, SqueakSwitchBlocks = 0x10;
        // SqueakChestCells(); The cells of a room's chests (its .doe items of kinds 0x11, 0x14): no door goes on one (NearGimmick());
        static IEnumerable<(int x, int y)> SqueakChestCells(byte[] squeak, SqueakRoomFiles f) {
            byte[] objects = f.Objects == null ? null : SqueakFileAnyCase(squeak, f.Objects);
            if (objects == null || objects.Length < 8) yield break;
            int items = ReadShort(objects, 6);
            for (int i = 0, count = items + 2 <= objects.Length ? ReadShort(objects, items) : 0; i < count && items + 2 + i * 16 + 8 <= objects.Length; i++) {
                int at = items + 2 + i * 16;
                if (ReadShort(objects, at + 2) is SqueakChestSmall or SqueakChestBig) yield return (ReadShort(objects, at + 4) / 16, ReadShort(objects, at + 6) / 16);
            }
        }
        /* SqueakRoomsInvincible(); KatAM's invincibility (the Invincible Candy's: sub_0808324C starts it, its task sub_08083494 runs it,
         * sub_081232AC the candy's own) is off in the rooms over 0x383, KatAM's special rooms, and so in every Squeak Squad room (0x400 on):
         * the candy, from a chest or not, did nothing there (the owner: "Invincibility Lollipops are not activating from chests?"). Each
         * test, "ldr r0, =0x383; cmp room, r0; bhi off; subs r0, #98; cmp room, r0; beq off" (the second for room 0x321), becomes "ldr r0,
         * =0x384; subs r0, room, r0; cmp r0, #0x7B; bls off; adds r0, #0x63; beq off": off from 0x384 to 0x3FF and in 0x321 as before, on
         * from 0x400. The three found by their code, as each version has them; returns how many there are (done before counted); */
        static int SqueakRoomsInvincible(byte[] rom) {
            int found = 0;
            for (int a = 0; a + 12 <= 0x400000; a += 2) {
                int ldr = ReadShort(rom, a), cmp = ReadShort(rom, a + 2), bhi = ReadShort(rom, a + 4), sub = ReadShort(rom, a + 6), cmp2 = ReadShort(rom, a + 8),
                    beq = ReadShort(rom, a + 10);
                if ((ldr & 0xFF00) != 0x4800 || (beq & 0xFF00) != 0xD000) continue;
                int literal = ((a + 4) & ~3) + (ldr & 0xFF) * 4, value = ReadInt(rom, literal);
                // (done already: "subs r0, room, r0; cmp r0, #0x7B; bls; adds r0, #0x63");
                if (value == 0x384 && (cmp == 0x1A08 || cmp == 0x1A18) && bhi == 0x287B && (sub & 0xFF00) == 0xD900 && cmp2 == 0x3063) {
                    found++;
                    continue;
                }
                if (value != 0x383 || cmp is not (0x4281 or 0x4283) || (bhi & 0xFF00) != 0xD800 || sub != 0x3862 || cmp2 != cmp) continue;
                int room = cmp & 7;
                WriteInt(rom, literal, 0x384);
                WriteShort(rom, a + 2, 0x1A00 | room << 3);                     // subs r0, room, r0;
                WriteShort(rom, a + 4, 0x287B);                                  // cmp r0, #0x7B;
                WriteShort(rom, a + 6, 0xD900 | (((sbyte)(bhi & 0xFF) - 1) & 0xFF));  // bls off (one instruction further on);
                WriteShort(rom, a + 8, 0x3063);                                  // adds r0, #0x63;
                found++;
            }
            return found;
        }
        /* SqueakShutCells(); The cells a room's stone shutters close while shut (gimmick 2 among the ones that talk: a stone pillar starting
         * shut, its look 0 upright, 1 and 2 on their side: squeak/gimmicks.c's boxes, as the logic checks read them); */
        static HashSet<(int x, int y)> SqueakShutCells(SqueakMap map) {
            HashSet<(int x, int y)> cells = new HashSet<(int, int)>();
            List<SqueakGimmick> gimmicks = map.Gimmicks();
            for (int i = 0; i < gimmicks.Count && i < SqueakMailGimmicks; i++) {
                SqueakGimmick g = gimmicks[i];
                if (g.Type != SqueakShutterGimmick || g.Parameters == null || g.Parameters.Length < 2) continue;
                int word = g.Word(0);
                if ((word & 7) > 2 || (word & 8) != 0) continue;
                (int x0, int y0, int x1, int y1) = (word & 7) == 0 ? (-8, -16, 8, 16) : (-16, -8, 16, 8);
                for (int cx = (g.X + x0 + 1) / 16; cx <= (g.X + x1 - 2) / 16; cx++)
                    for (int cy = (g.Y + y0 + 1) / 16; cy <= (g.Y + y1 - 2) / 16; cy++) cells.Add((cx, cy));
            }
            return cells;
        }
        // (the gimmicks that talk to others: squeak/gimmicks.c; its shutters on object 0x8D with their template's unk11 SqueakShutterObject);
        const int SqueakShutterGimmick = 2, SqueakFireGimmick = 7, SqueakTriggerArea = 0x15, SqueakRelay = 0x16, SqueakRandomRelay = 0x17,
                  SqueakQuake = 0x13, SqueakCameraArea = 0x14, SqueakEmitter = 0x19, SqueakStreaks = 0x18, SqueakLight = 0x12, SqueakSoundGimmick = 0x1A, SqueakMailGimmicks = 60;
        const byte SqueakShutterObject = 0x9B, SqueakFireObject = 0x9A;
        // (per look its box, pixels around its spot: squeak/gimmicks.c's sShutterBoxes);
        static readonly int[][] SqueakShutterBoxes = { new[] { -8, -16, 8, 16 }, new[] { -16, -8, 16, 8 }, new[] { -16, -8, 16, 8 }, new[] { -17, -8, 16, 8 },
                                                       new[] { -16, -8, 17, 8 }, new[] { -8, -16, 8, 17 }, new[] { -8, -17, 8, 16 } };
        /* The grass tufts (0xE) and barrels (0xF), their objects squeak/gimmicks.c's: its template's unk12 the grass's look, unk14 what
         * comes out (the KatAM object's type, its behavior byte over it), unk16 its variant (a Squeak Squad character's: SqueakHeld()); */
        const int SqueakGrassGimmick = 0xE, SqueakBarrelGimmick = 0xF;
        const byte SqueakGrassObject = 0x99, SqueakBarrelObject = 0x98, KatAMAbilityStar = 0xA3;
        // (the icicles, gimmick 9: squeak/gimmicks.c's SqueakCreateIcicle);
        const int SqueakIcicleGimmick = 9, SqueakRockGimmick = 8;
        const byte SqueakIcicleObject = 0x96, SqueakRockObject = 0x95;
        // (the moving platforms: squeak/gimmicks.c's SqueakCreatePlatform, its first word and its legs, 3 at most, in the template);
        const int SqueakPlatformGimmick = 4, SqueakPlatformLegs = 3, SqueakCloudGimmick = 5, SqueakRopeGimmick = 1, SqueakRopeFloor = 3;
        const byte SqueakPlatformObject = 0x94, SqueakCloudObject = 0x93, SqueakRopeObject = 0x92;
        // (the rolling boulders: squeak/gimmicks.c's SqueakCreateBoulder, its first word in the template, its index for the messages that start it);
        const int SqueakBoulderGimmick = 6, SqueakCandleGimmick = 0xA;
        const byte SqueakBoulderObject = 0x91, SqueakCandleObject = 0x90;
        // (Squeak Squad's abilities, by its numbers, as KatAM's: those KatAM has; the others, Ninja, Animal, Ghost, Bubble, Metal, none);
        static readonly Dictionary<int, byte> SqueakKatAMAbilities = new Dictionary<int, byte> {
            [1] = 1, [2] = 2, [3] = 15, [4] = 7, [5] = 16, [6] = 5, [7] = 6, [8] = 13, [9] = 9, [10] = 4, [11] = 14, [13] = 19, [14] = 11,
            [15] = KatAMHiJump, [16] = 18, [20] = 20,
        };
        const byte KatAMHiJump = 31;   // abilities.h's NIDL_ABILITY_HI_JUMP (KIRBY_ABILITY_COUNT + 4);
        /* What a grass tuft or a barrel hides: its table's first byte's high nibble 1 a record of the room's item list, 0 one of its
         * enemy list, its second byte the record's index (0xFF none): on the DS, cut or burst, it lets the record out (0x0220A200: the
         * item maker; 0x0220A2A4: the enemy spawner, 0x02218E20), which isn't out till then (1-2-2's grass hides the Cherry its item
         * list puts there, 1-5-1's barrels its eight items, 1-9-23's grass an enemy). SqueakHeldRecords(): the records the room's
         * holders hide, which SqueakObjects() leaves for them; SqueakHeld(): a holder's, as the KatAM object it lets out (its type,
         * behavior and variant as SqueakObjects() would place it; an ability item KatAM's ability star with the ability, or a Cherry
         * where KatAM lacks it); */
        static (HashSet<int> items, HashSet<int> enemies) SqueakHeldRecords(SqueakMap map) {
            HashSet<int> items = new HashSet<int>(), enemies = new HashSet<int>();
            foreach (SqueakGimmick g in map.Gimmicks())
                if (g.Type is SqueakGrassGimmick or SqueakBarrelGimmick && g.Parameters?.Length >= 2 && g.Parameters[1] != 0xFF)
                    (g.Parameters[0] >> 4 == 1 ? items : enemies).Add(g.Parameters[1]);
            return (items, enemies);
        }
        static (byte type, byte behavior, byte variant) SqueakHeld(byte[] objects, byte[] par) {
            if (objects == null || par == null || par.Length < 2 || par[1] == 0xFF) return (0, 0, 0);
            if (par[0] >> 4 == 1) {
                int items = ReadShort(objects, 6), at = items + 2 + par[1] * 16;
                if (items + 2 > objects.Length || par[1] >= ReadShort(objects, items) || at + 16 > objects.Length) return (0, 0, 0);
                int kind = ReadShort(objects, at + 2);
                if (SqueakItemKinds.TryGetValue(kind, out byte item)) return (item, 0, 0);
                if (kind == SqueakRandomItem) return (KatAMCherry, 0, 0);
                if (kind > SqueakAbilityKinds && SqueakKatAMAbilities.TryGetValue(kind - SqueakAbilityKinds, out byte ability)) return (KatAMAbilityStar, ability, 0);
                return (KatAMCherry, 0, 0);
            } else {
                int at = 14 + par[1] * 16;
                if (par[1] >= ReadShort(objects, 12) || at + 16 > objects.Length) return (0, 0, 0);
                int id = ReadShort(objects, at + 2), v = objects[at + 8], w = objects[at + 9];
                if (SqueakNatives.TryGetValue(id, out var native) && !KatAMMidbossTypes.Contains(native.type)) {
                    byte behavior = v < native.behaviors.Length ? native.behaviors[v] : native.behaviors[0];
                    return (native.type, id == 0x03 && w == 1 ? (byte)6 : behavior, 0);
                }
                if (SqueakCharacters.TryGetValue(id, out int character)) return (SqueakID, (byte)character, (byte)(id == 0x45 ? (w != 0 ? 1 : 0) : v));
                return (0, 0, 0);
            }
        }

        /* SqueakSwitchTargets(); The block groups a switch sets off (0x022252F0: its parameter's first byte's high nibble the count of
         * its targets, then each one's index in the room's gimmick list), the cells of those that are switch block groups (gimmick
         * 0x10: its spot in the group; 3-4-8's switch by the tower, index 2, names 5, the group in the beam over it); the others
         * (shutters, gimmick 2) are left out, KatAM's shutters being open; */
        static IEnumerable<(int x, int y)> SqueakSwitchTargets(List<SqueakGimmick> gimmicks, SqueakGimmick sender) {
            foreach (int index in SqueakMessageTargets(sender))
                if (index < gimmicks.Count && gimmicks[index].Type == SqueakSwitchBlocks) yield return (gimmicks[index].X / 16, gimmicks[index].Y / 16);
        }

        /* SqueakMessageTargets(); The gimmicks (their indices in the room's list) a gimmick sends messages to (squeak/gimmicks.c): a
         * switch's (its first byte's high nibble how many, then their indices), a trigger area's (its parameter's bits 14-15 how
         * many, its bytes 2-4 their indices), a relay's (its first byte's low 7 bits how many, its bytes 1-5 their indices), a quake
         * trigger's (its second u16's bits 14-15 how many, its bytes 4 on their indices); */
        static IEnumerable<int> SqueakMessageTargets(SqueakGimmick sender) {
            byte[] p = sender.Parameters;
            if (p == null || p.Length == 0) yield break;
            (int count, int first, int most) = sender.Type switch {
                SqueakSwitchGimmick => (p[0] >> 4, 1, 3),
                SqueakTriggerArea => (p.Length > 1 ? p[1] >> 6 : 0, 2, 3),
                SqueakRelay => (p[0] & 0x7F, 1, 5),
                SqueakRandomRelay => (p.Skip(4).TakeWhile(b => b != 0xFF).Count(), 4, 8),
                SqueakQuake => (p.Length > 3 ? p[3] >> 6 : 0, 4, 3),
                SqueakRopeGimmick => (p[0] >> 4, 2, 3),
                _ => (0, 0, 0) };
            for (int k = 0; k < count && k < most && first + k < p.Length; k++) yield return p[first + k];
        }

        /* SqueakDarkStarRoute(); A room's route for the dark star (squeak/dark_star.c's SqueakRoute): the map's section 9, a zone a cell
         * (its point's index), then a u16 count and 8-byte points (u8 index, a byte, u16 x, y from the top, its order: 0 the first,
         * 255 the end; written 8 bytes each: x, y, order, 0, agbcc's size for the struct), and the boxes of its camera areas that hold the camera (0x14, its parameter's +0xD bit 0; its box from its spot,
         * the u16 at +8's bits 0-6 blocks right, 7-13 down), whose hold starts the star's loop: its address, or 0 (no route); */
        const int SqueakDarkStarCharacter = 68;
        static int SqueakDarkStarRoute(byte[] rom, SqueakMap map, ref int cursor) {
            int at = map.Section(9), cells = map.Width * map.Height;
            if (at <= 0 || at + cells + 2 > map.Data.Length) return 0;
            int count = ReadShort(map.Data, at + cells), points = at + cells + 2;
            if (count == 0 || count > 255 || points + count * 8 > map.Data.Length) return 0;
            var locks = map.Gimmicks().Where(g => g.Type == SqueakCameraArea && g.Parameters?.Length >= 14 && (g.Parameters[13] & 1) != 0)
                .Select(g => { int box = ReadShort(g.Parameters, 8); return (x0: g.X, y0: g.Y, x1: g.X + (box & 0x7F) * 16, y1: g.Y + (box >> 7 & 0x7F) * 16); })
                .ToList();
            int zones = Allocate(ref cursor, cells, 4), pointsAt = Allocate(ref cursor, count * 8, 4), locksAt = Allocate(ref cursor, Math.Max(1, locks.Count) * 8, 4);
            Array.Copy(map.Data, at, rom, zones, cells);
            for (int i = 0; i < count; i++)
                WriteShorts(rom, pointsAt + i * 8, ReadShort(map.Data, points + i * 8 + 2), ReadShort(map.Data, points + i * 8 + 4), ReadShort(map.Data, points + i * 8 + 6), 0);
            for (int i = 0; i < locks.Count; i++) WriteShorts(rom, locksAt + i * 8, locks[i].x0, locks[i].y0, locks[i].x1, locks[i].y1);
            int route = Allocate(ref cursor, 16, 4);
            rom[route] = (byte)map.Width;
            rom[route + 1] = (byte)map.Height;
            rom[route + 2] = (byte)count;
            rom[route + 3] = (byte)locks.Count;
            WriteInts(rom, route + 4, zones + 0x08000000, pointsAt + 0x08000000, locksAt + 0x08000000);
            return route;
        }

        /* SqueakGimmickList(); A room's gimmicks its task runs (squeak/gimmicks.c's gSqueakGimmickRooms: trigger areas, relays, quake
         * triggers, the effect emitters, the camera areas (not in a room whose fight's arena holds the camera: `arena`), and the block
         * groups made chains,
         * `chains`): a count, then 24 bytes each (type, index, x, y, the parameters' first 18 bytes); null none. A camera area's bounds
         * (its first 4 u16s: x0, y0, x1, y1) have their y in the DS's world, up from the room's bottom: here from its top (`height`, the
         * room's in pixels, less them, swapped), as the entries' spots are; */
        static byte[] SqueakGimmickList(List<SqueakGimmick> gimmicks, IReadOnlySet<(int x, int y)> chains, bool arena, int height, SqueakChase chase = null,
                                        IReadOnlyList<(int x, int y)> lairExits = null, int currents = 0, int crumbles = 0,
                                        IEnumerable<SqueakPhaseWarp> phaseWarps = null) {
            List<byte> list = new List<byte> { 0, 0, 0, 0 };
            for (int i = 0; i < gimmicks.Count && i < SqueakMailGimmicks; i++) {
                SqueakGimmick g = gimmicks[i];
                if (!(g.Type is SqueakTriggerArea or SqueakRelay or SqueakRandomRelay or SqueakQuake or SqueakEmitter or SqueakStreaks or SqueakLight or SqueakSoundGimmick || g.Type == SqueakCameraArea && !arena
                      || g.Type == SqueakSwitchBlocks && chains.Contains((g.X / 16, g.Y / 16)))) continue;
                byte[] entry = new byte[24];
                entry[0] = (byte)g.Type;
                entry[1] = (byte)i;
                WriteShort(entry, 2, g.X);
                WriteShort(entry, 4, g.Y);
                g.Parameters?.Take(18).ToArray().CopyTo(entry, 6);
                if (g.Type == SqueakCameraArea && g.Parameters?.Length >= 8) {
                    int top = height - ReadShort(g.Parameters, 6), bottom = height - ReadShort(g.Parameters, 2);
                    WriteShort(entry, 6 + 2, Math.Max(0, top));
                    WriteShort(entry, 6 + 6, Math.Max(0, bottom));
                }
                list.AddRange(entry);
                list[0]++;
            }
            if (chase != null) AddChaseEntries(list, chase, gimmicks.Count);
            // (a Squeak Lair's way out, shut till its mini-boss is beaten: SqueakLairExitEntry, x and y its cell's pixels, its index past
            // the others');
            int next = gimmicks.Count;
            for (int at = 4; at < list.Count; at += 24) next = Math.Max(next, list[at + 1] + 1);
            foreach ((int x, int y) in lairExits ?? new List<(int, int)>()) {
                if (next >= SqueakMailGimmicks || list[0] == 0xFF) break;
                byte[] entry = new byte[24];
                entry[0] = SqueakLairExitEntry;
                entry[1] = (byte)next++;
                WriteShort(entry, 2, x * 16);
                WriteShort(entry, 4, y * 16);
                list.AddRange(entry);
                list[0]++;
            }
            // (the room's currents, SqueakCurrentsEntry: its first 4 bytes the address of their grid, SqueakCurrents());
            if (currents != 0 && next < SqueakMailGimmicks && list[0] != 0xFF) {
                byte[] entry = new byte[24];
                entry[0] = SqueakCurrentsEntry;
                entry[1] = (byte)next++;
                WriteInt(entry, 6, currents);
                list.AddRange(entry);
                list[0]++;
            }
            // (its walk-once blocks, SqueakCrumblesEntry: its first 4 bytes the address of their list, SqueakCrumbles());
            if (crumbles != 0 && next < SqueakMailGimmicks && list[0] != 0xFF) {
                byte[] entry = new byte[24];
                entry[0] = SqueakCrumblesEntry;
                entry[1] = (byte)next++;
                WriteInt(entry, 6, crumbles);
                list.AddRange(entry);
                list[0]++;
            }
            // (the finale's ways on by themselves, SqueakPhaseWarpEntry: the room, the cell + 1, after its fight or not: SqueakPhaseWarps);
            foreach (SqueakPhaseWarp warp in phaseWarps ?? Enumerable.Empty<SqueakPhaseWarp>()) {
                if (next >= SqueakMailGimmicks || list[0] == 0xFF) break;
                byte[] entry = new byte[24];
                entry[0] = SqueakPhaseWarpEntry;
                entry[1] = (byte)next++;
                WriteShort(entry, 2, warp.X * 16 + 8);
                WriteShort(entry, 4, warp.Y * 16 + 8);
                WriteShort(entry, 6, warp.ToRoom);
                entry[8] = (byte)warp.ToX;
                entry[9] = (byte)warp.ToY;
                entry[10] = (byte)(warp.AfterFight ? 1 : 0);
                list.AddRange(entry);
                list[0]++;
            }
            // (its candles' light, SqueakCandleLightEntry: the far view's light the room's candles turn up and down, squeak/gimmicks.c's
            // CandleLight());
            if (gimmicks.Any(g => g.Type == SqueakCandleGimmick) && next < SqueakMailGimmicks && list[0] != 0xFF) {
                byte[] entry = new byte[24];
                entry[0] = SqueakCandleLightEntry;
                entry[1] = (byte)next++;
                list.AddRange(entry);
                list[0]++;
            }
            return list[0] == 0 && list[1] == 0 ? null : list.ToArray();
        }

        /* The chases (squeak.h's "The chases", squeak/gimmicks.c; the owner: "an invisible trigger wall on the position of the chest, the
         * height of the wall will be of the entire room ... whenever the player passes that threshold, the chase will activate"): a room's
         * list carries its chase's key (its header's second byte: a Squeak Squad stage's, its rooms' all the same, so the chase goes on
         * through them; a KatAM room's own), a wall at each of its big chests (SqueakChaseWall: x the chest's), and the Squeaks that come
         * out once one's passed (SqueakChaseSpawn: KatAM's template spot, the character and its variant): in Squeak Squad's stages its
         * own chase spawns (its objects' flag 4: the Squeaks among them, Spinni, Doc, Storo, the Squeakers), which come out on the DS
         * once Kirby carries off the stage's chest; their indexes after the room's gimmicks'; */
        const byte SqueakChaseWall = 0xF0, SqueakChaseSpawnEntry = 0xF1;
        // (a Squeak Lair's way out: squeak/gimmicks.c's GIMMICK_LAIR_EXIT, its door opened once no mini-boss is left in the room);
        const byte SqueakLairExitEntry = 0xF2;
        // (a room's currents: squeak/gimmicks.c's GIMMICK_CURRENTS, its grid's address in its first 4 bytes, SqueakCurrents());
        const byte SqueakCurrentsEntry = 0xF3;
        // (a room's walk-once blocks: squeak/gimmicks.c's GIMMICK_CRUMBLES, their list's address in its first 4 bytes, SqueakCrumbles());
        const byte SqueakCrumblesEntry = 0xF4;
        // (the finale's way on by itself: squeak/gimmicks.c's GIMMICK_PHASE_WARP, SqueakPhaseWarps);
        const byte SqueakPhaseWarpEntry = 0xF5;
        // (a chest the chase brings: at its template's spot, its item, number, mark (unk16), half flag and type: squeak/gimmicks.c's GIMMICK_CHASE_CHEST);
        const byte SqueakChaseChestEntry = 0xF6;
        // (a room's candles' light: squeak/gimmicks.c's GIMMICK_CANDLE_LIGHT);
        const byte SqueakCandleLightEntry = 0xF7;
        internal sealed record SqueakChase(byte Key, List<(int x, int y)> Walls, List<(int x, int y, byte character, byte variant)> Spawns,
                                           List<byte[]> Chests = null);
        static void AddChaseEntries(List<byte> list, SqueakChase chase, int firstIndex) {
            int index = firstIndex;
            list[1] = chase.Key;
            void Add(byte type, int x, int y, params byte[] par) {
                if (index >= SqueakMailGimmicks || list[0] == 0xFF) return;
                byte[] entry = new byte[24];
                entry[0] = type;
                entry[1] = (byte)index++;
                WriteShort(entry, 2, x);
                WriteShort(entry, 4, y);
                par.CopyTo(entry, 6);
                list.AddRange(entry);
                list[0]++;
            }
            foreach ((int x, int y) in chase.Walls) Add(SqueakChaseWall, x, y);
            foreach ((int x, int y, byte character, byte variant) in chase.Spawns) Add(SqueakChaseSpawnEntry, x, y, character, variant);
            foreach (byte[] chest in chase.Chests ?? new List<byte[]>())
                Add(SqueakChaseChestEntry, ReadShort(chest, 6), ReadShort(chest, 8), chest[14], chest[0x12], chest[0x16], chest[0x17], chest[0x15], chest[0xC]);
        }
        // (the Squeaks the chases bring: the Squeakers, Spinni, Doc, Storo; at most this many in a room, KatAM's 32 spawned templates);
        static readonly HashSet<int> SqueakChaseIds = new HashSet<int> { 0x40, 0x42, 0x43, 0x44 };
        const int SqueakChaseSpawnsMax = 6;

        /* SqueakChaseOf(); A room's chase entries: its chase spawns (the Squeaks among the records with flag 4, not by a door: the
         * rulebook's rule 2, as SqueakObjects()' enemies) and its big chests' walls; */
        static (List<(int x, int y)> walls, List<(int x, int y, byte character, byte variant)> spawns) SqueakChaseOf(byte[] squeak, SqueakRoomFiles f,
                                                                                                                       List<(int x, int y)> doorCells) {
            List<(int x, int y)> walls = new List<(int, int)>();
            List<(int x, int y, byte character, byte variant)> spawns = new List<(int, int, byte, byte)>();
            byte[] objects = f.Objects == null ? null : SqueakFileAnyCase(squeak, f.Objects);
            if (objects == null || objects.Length < 16) return (walls, spawns);
            for (int i = 0, count = ReadShort(objects, 12); i < count; i++) {
                int at = 14 + i * 16, id = ReadShort(objects, at + 2), x = ReadShort(objects, at + 4), y = ReadShort(objects, at + 6);
                if ((objects[at + 0xB] & SqueakChaseSpawn) == 0 || !SqueakChaseIds.Contains(id) || !SqueakCharacters.TryGetValue(id, out int character)) continue;
                if (doorCells != null && doorCells.Any(c => Math.Abs(c.x - x / 16) <= SqueakDoorClear && Math.Abs(c.y - (y - 1) / 16) <= 2)) continue;
                if (spawns.Count < SqueakChaseSpawnsMax) spawns.Add((x, y - 8, (byte)character, objects[at + 8]));
            }
            int items = ReadShort(objects, 6);
            for (int i = 0, count = items + 2 <= objects.Length ? ReadShort(objects, items) : 0; i < count; i++) {
                int at = items + 2 + i * 16;
                if (ReadShort(objects, at + 2) == SqueakChestBig) walls.Add((ReadShort(objects, at + 4), ReadShort(objects, at + 6)));
            }
            return (walls, spawns);
        }
        static readonly Dictionary<int, byte> SqueakFuseCells = new Dictionary<int, byte> {
            [0xC] = 0x22, [0x5] = 0x23, [0x6] = 0x24, [0x3] = 0x25, [0x9] = 0x26, [0xA] = 0x27 };
        internal sealed record SqueakFuseLine(List<(int x, int y, byte value)> Cells, (int x, int y) Light, (int x, int y) Last);
        // SqueakFuse(); A room's fuse (null: none, or none KatAM's fuse can burn, lit going up), traced from where it's lit;
        static SqueakFuseLine SqueakFuse(SqueakMap map) {
            Dictionary<(int x, int y), int> ways = new Dictionary<(int, int), int>();
            for (int y = 0; y < map.Height; y++)
                for (int x = 0; x < map.Width; x++)
                    if ((map.Collision(6, x, y) >> 16 & 0xFF) is uint id and >= 0x81 and <= 0x8F) ways[(x, y)] = (int)(id & 0xF);
            if (!ways.Any(w => w.Value is 1 or 2 or 4)) return null;
            (int x, int y) light = ways.First(w => w.Value is 1 or 2 or 4).Key;
            (int dx, int dy)[] steps = { (1, 0), (-1, 0), (0, 1), (0, -1) };
            List<(int x, int y, byte value)> cells = new List<(int, int, byte)>();
            (int x, int y) at = light, from = (-1, -1), last = light;
            while (true) {
                (int x, int y) next = (-1, -1);
                for (int k = 0; k < 4; k++)
                    if ((ways[at] & 1 << k) != 0 && (at.x + steps[k].dx, at.y + steps[k].dy) is var n && n != from && ways.ContainsKey(n)) next = n;
                if (at != light) cells.Add((at.x, at.y, SqueakFuseCells.GetValueOrDefault(ways[at], (byte)0x22)));
                last = at;
                if (next.x < 0 || cells.Count > ways.Count) break;
                (from, at) = (at, next);
            }
            // (lit from the side: the object a cell up, the lit cell a corner from it into the rest: KatAM's burner starts with a step
            // down out of the object's cell, which its end needs);
            if (ways[light] != 4) {
                cells.Insert(0, (light.x, light.y, ways[light] == 1 ? (byte)0x26 : (byte)0x27));
                light = (light.x, light.y - 1);
            }
            return new SqueakFuseLine(cells, light, last);
        }

        /* Squeak Squad's warp stars (gimmick 0xD): its parameters' words: the stage's room it flies to (0-based, the low byte; 254 or
         * 255: one Kirby comes in on, no star to get on), a code, then its path, segments of 6 words: a kind (33 and 49 a curve; any
         * other ends it), its frames, where it ends and its control point (a quadratic curve from where the star is, frame by frame, as
         * the DS flies it: 1-4-1's ride, checked every 20 frames). A ride (the DS emulator, the stage's rooms in its table): the star's
         * path out of its room, then the room it names, where the star Kirby comes in on flies its path, and he drops off at its end
         * onto the floor under it (1-4-2's ends at 102, 70; he lands at 98, 144); a room with no such star but one to get on (3-4-5,
         * 7-2-2, 8-3-6, 8-3-7): he comes in on that one, and it flies its own path on to the room it names (3-4-4's ride goes through
         * 3-4-5 into 3-4-6, landing at 104, 160 under the path's 106, 120); a room with neither (8-4-5, 8-4-4's star's): he comes in
         * where the room starts (its door of kind 6, 3, 8: at 56, 136) and drops onto the floor. In KatAM (SqueakRides()): a warp star (WarpStarType) where
         * the DS's stands, its flight the ride (a Transport's RideLeg's, flown by RideFlight(): KatAM's fade to white after each leg but
         * the last, as the DS's white after its star is off the screen), landing where Kirby drops onto the floor; */
        const int SqueakWarpStar = 0xD, SqueakStarArrival = 254, SqueakRideFade = 18, SqueakRideLegs = 6, SqueakEntryRows = 5;
        // SqueakStarPath(); A star's path, where it is frame by frame;
        static List<(int x, int y)> SqueakStarPath(SqueakGimmick star) {
            List<(int x, int y)> path = new List<(int, int)>();
            double x = star.X, y = star.Y;
            for (int w = 2; (w + 6) * 2 <= (star.Parameters?.Length ?? 0); w += 6) {
                int kind = star.Word(w), frames = star.Word(w + 1);
                if (kind is not (33 or 49) || frames <= 0) break;
                double endX = (short)star.Word(w + 2), endY = (short)star.Word(w + 3), controlX = (short)star.Word(w + 4), controlY = (short)star.Word(w + 5);
                for (int k = 1; k <= frames; k++) {
                    double t = (double)k / frames;
                    path.Add(((int)Math.Round((1 - t) * (1 - t) * x + 2 * t * (1 - t) * controlX + t * t * endX),
                              (int)Math.Round((1 - t) * (1 - t) * y + 2 * t * (1 - t) * controlY + t * t * endY)));
                }
                (x, y) = (endX, endY);
            }
            return path;
        }
        // SqueakMoves(); A path as a ride's moves (pixels a frame, for so many frames) from (x, y);
        static (int dx, int dy, int frames)[] SqueakMoves(int x, int y, List<(int x, int y)> path) {
            List<(int dx, int dy, int frames)> moves = new List<(int, int, int)>();
            foreach ((int px, int py) in path) {
                (int dx, int dy) = (px - x, py - y);
                (x, y) = (px, py);
                if (moves.Count > 0 && moves[^1].dx == dx && moves[^1].dy == dy) moves[^1] = (dx, dy, moves[^1].frames + 1);
                else moves.Add((dx, dy, 1));
            }
            return moves.ToArray();
        }
        /* SqueakRides(); The warp stars to get on (by the room's index in `files`), each a Transport: its spot, its ride (the rooms
         * SqueakFirstRoom on), where it lands Kirby (the room, the cell + 1) and its flight (from `flight` on). Kirby's drop: a hop up,
         * then a fall, faster and faster, to where he stands on the floor under the path's end (8 pixels over it); */
        static Dictionary<int, List<Transport>> SqueakRides(byte[] squeak, List<SqueakRoomFiles> files, Dictionary<(int, int, int), int> roomOf,
                                                            List<SqueakGimmick>[] starsOf, List<SqueakDoor>[] doorsOf, int[] widthOf, int[] heightOf, int flight) {
            Dictionary<int, List<Transport>> rides = new Dictionary<int, List<Transport>>();
            static int Target(SqueakGimmick star) => star.Word(0) & 0xFF;
            /* (a leg: the star's path from where it is, or, coming into a room, from where it's first in its area: KatAM's room change
             * puts Kirby inside the room, 4 rows over its bottom at most (8-4-4's star, under its last row, flew 64 pixels high), and
             * the moves go on from there); */
            RideLeg Leg(int room, SqueakGimmick star, bool fade) {
                List<(int x, int y)> path = SqueakStarPath(star);
                (int x, int y) from = (star.X, star.Y);
                bool Inside((int x, int y) p) => p.x >= 16 && p.y >= 16 && p.x < widthOf[room] * 16 - 16 && p.y < (heightOf[room] - SqueakEntryRows) * 16;
                if (!Inside(from) && path.FindIndex(p => Inside(p)) is int first and >= 0) {
                    from = path[first];
                    path = path.Skip(first + 1).ToList();
                }
                return new RideLeg(SqueakFirstRoom + room, from.x, from.y, SqueakMoves(from.x, from.y, path),
                                   Fade: fade ? new[] { (0, 0, SqueakRideFade) } : Array.Empty<(int, int, int)>());
            }
            for (int i = 0; i < files.Count; i++)
                foreach (SqueakGimmick star in starsOf[i].Where(g => Target(g) < SqueakStarArrival)) {
                    List<RideLeg> legs = new List<RideLeg> { Leg(i, star, true) };
                    int room = i;
                    SqueakGimmick flying = star, arrival = null;
                    while (legs.Count < SqueakRideLegs && roomOf.TryGetValue((files[room].Area, files[room].Stage, Target(flying) + 1), out int to)) {
                        room = to;
                        if ((arrival = starsOf[to].FirstOrDefault(g => Target(g) >= SqueakStarArrival)) != null) {
                            legs.Add(Leg(to, arrival, false));
                            break;
                        }
                        if ((flying = starsOf[to].FirstOrDefault(g => Target(g) < SqueakStarArrival)) != null) {
                            legs.Add(Leg(to, flying, true));
                            continue;
                        }
                        // (no star in that room: he comes in where the room starts, its door of kind 6, and drops: 8-4-4's into 8-4-5);
                        if (doorsOf[to].FirstOrDefault(d => d.Kind == SqueakStartKind) is SqueakDoor start) {
                            arrival = star;
                            legs.Add(new RideLeg(SqueakFirstRoom + to, start.X * 16 + 8, start.Y * 16 + 8, Array.Empty<(int, int, int)>()));
                        }
                        break;
                    }
                    if (arrival == null) continue;
                    // (where he drops: the floor under the path's end, as KatAM reads the room, SqueakCollision());
                    SqueakMap map = new SqueakMap(SqueakUnpack(SqueakFileAnyCase(squeak, files[room].Map)));
                    (int endX, int endY) = legs[^1].End;
                    int column = Math.Clamp(endX / 16, 0, map.Width - 1), floor = Math.Max(endY, 0) / 16;
                    while (floor < map.Height && SqueakCollision(SqueakCellWord(map, column, floor)) is KatAMAir or KatAMLadder or (>= 0x34 and <= KatAMWater)) floor++;
                    int stand = floor * 16 - 8;
                    List<(int dx, int dy, int frames)> hop = new List<(int, int, int)>();
                    if (stand > endY) {
                        hop.Add((0, -2, 2));
                        hop.Add((0, -1, 2));
                        for (int y = endY - 6, speed = 2; y < stand; speed++) {
                            int dy = Math.Min(Math.Min(speed / 2, 6), stand - y);
                            hop.Add((0, dy, 1));
                            y += dy;
                        }
                    } else hop.Add((0, 0, 1));
                    legs[^1] = legs[^1] with { Hop = hop.ToArray(), Bounce = new[] { (0, 0, 1) } };
                    (int x, int y) landing = (endX / 16 + 1, Math.Max(stand, endY) / 16 + 1);
                    if (!rides.ContainsKey(i)) rides[i] = new List<Transport>();
                    // (the star in its room: 3-4-5's is under its last row, where Kirby starts in it);
                    (int x, int y) spot = (Math.Clamp(star.X, 8, widthOf[i] * 16 - 8), Math.Clamp(star.Y, 8, heightOf[i] * 16 - 8));
                    rides[i].Add(new Transport(spot.x, spot.y, false, SqueakFirstRoom + room, landing.x, landing.y, flight++, legs.ToArray()));
                }
            return rides;
        }

        /* WriteSqueakForeground(); A converted foreground into the ROM (its tiles, its animations, KatAM's foreground animations as
         * TileAnimations()' (ForegroundInfo +8, +0xC): a timing's tiles (VRAM from 0x06008000), copied whole, a step per frame, its source
         * the frame's tiles and its duration; its palettes, its tilemap), for room `room`: its info, palette and tilemap entries; the
         * tiles and tilemap shared with a room whose are the same (AllocateShared()); */
        static (int info, int palette, int tilemapEntry) WriteSqueakForeground(byte[] rom, SqueakForeground foreground, int room, int paletteCycles,
                                                                                ref int cursor) {
            int tilesAt = AllocateShared(rom, CompressLZ77(foreground.Tiles), ref cursor);
            /* (its animations, KatAM's foreground animations as TileAnimations()' (ForegroundInfo +8, +0xC): a timing's tiles
             * (VRAM from 0x06008000), copied whole, a step per frame, its source the frame's tiles and its duration); */
            int animationsAt = 0;
            if (foreground.Animations.Count > 0) {
                animationsAt = Allocate(ref cursor, foreground.Animations.Count * 0x14, 4);
                for (int k = 0; k < foreground.Animations.Count; k++) {
                    SqueakTileAnimation animation = foreground.Animations[k];
                    int bytes = animation.Tiles * 32, steps = Allocate(ref cursor, animation.Frames.Length * 8, 4);
                    for (int step = 0; step < animation.Frames.Length; step++) {
                        int frameAt = Allocate(ref cursor, bytes, 4);
                        animation.Frames[step].CopyTo(rom, frameAt);
                        WriteInt(rom, steps + step * 8, frameAt + 0x08000000);
                        WriteShorts(rom, steps + step * 8 + 4, animation.Durations[step], 0);
                    }
                    WriteInts(rom, animationsAt + k * 0x14, animation.FirstTile * 32, bytes, bytes, steps + 0x08000000);
                    WriteShorts(rom, animationsAt + k * 0x14 + 0x10, animation.Frames.Length, 1);
                }
                animationsAt += 0x08000000;
            }
            int info = Allocate(ref cursor, 16, 4);
            WriteInts(rom, info, tilesAt + 0x08000000, 0, animationsAt);
            WriteShorts(rom, info + 12, foreground.Animations.Count, room);
            int colors = Allocate(ref cursor, 0xC0, 4), palette = Allocate(ref cursor, 12, 4);
            for (int c = 0; c < 96; c++) WriteShort(rom, colors + c * 2, foreground.Palettes[c]);
            /* (its palette animation, KatAM's cycles (sub_08003108()): a pointer a cycle, each {its frames, u16 frame count, first color,
             * color count}, each frame {its colors, u16 how long it stays}); */
            List<SqueakColorCycle> cycles = foreground.Cycles ?? new List<SqueakColorCycle>();
            int cyclesAt = paletteCycles;
            if (cycles.Count > 0) {
                cyclesAt = Allocate(ref cursor, cycles.Count * 4, 4) + 0x08000000;
                for (int k = 0; k < cycles.Count; k++) {
                    SqueakColorCycle cycle = cycles[k];
                    int frameList = Allocate(ref cursor, cycle.Frames.Length * 4, 4), entry = Allocate(ref cursor, 12, 4);
                    for (int f = 0; f < cycle.Frames.Length; f++) {
                        int frameColors = Allocate(ref cursor, cycle.Count * 2, 4), frame = Allocate(ref cursor, 8, 4);
                        for (int c = 0; c < cycle.Count; c++) WriteShort(rom, frameColors + c * 2, cycle.Frames[f][c]);
                        WriteInt(rom, frame, frameColors + 0x08000000);
                        WriteShorts(rom, frame + 4, cycle.Durations[f], 0);
                        WriteInt(rom, frameList + f * 4, frame + 0x08000000);
                    }
                    WriteInt(rom, entry, frameList + 0x08000000);
                    WriteShorts(rom, entry + 4, cycle.Frames.Length, cycle.FirstColor, cycle.Count, 0);
                    WriteInt(rom, cyclesAt - 0x08000000 + k * 4, entry + 0x08000000);
                }
            }
            WriteInts(rom, palette, colors + 0x08000000, cyclesAt);
            WriteShorts(rom, palette + 8, cycles.Count, room);
            byte[] tilemap = new byte[foreground.Tilemap.Length * 2];
            for (int t = 0; t < foreground.Tilemap.Length; t++) WriteShort(tilemap, t * 2, foreground.Tilemap[t]);
            int tilemapAt = AllocateShared(rom, CompressLZ77(tilemap), ref cursor), tilemapEntry = Allocate(ref cursor, 0x20, 4);
            WriteShorts(rom, tilemapEntry, foreground.Columns, foreground.Rows);
            WriteInts(rom, tilemapEntry + 0x18, tilemapAt + 0x08000000, room << 16);
            return (info, palette, tilemapEntry);
        }

        /* AddSqueakRooms(); Every Squeak Squad room into KatAM (see above): its foreground (ConvertSqueakForeground(), its star blocks'
         * cells drawn bare for what they leave), its far view as its background (shared by the rooms showing it), its collision
         * (SqueakCollision(): a star block a special block record, leaving air, or water in water), its CPU helpers' zones, its
         * objects (SqueakObjects()), its doors to the stage's other rooms (the goals not yet); */
        // (the chases made in the Squeak Squad rooms: rooms of a chase's stage, walls, Squeaks; for the checks);
        public static (int rooms, int walls, int spawns, int chests) SqueakChaseCounts => (squeakChaseRooms, squeakChaseWalls, squeakChaseSpawns, squeakChaseChests);
        static int squeakChaseRooms, squeakChaseWalls, squeakChaseSpawns, squeakChaseChests;
        // (the rooms with currents written, SqueakCurrents(): AddSqueakRooms() counts them afresh);
        public static int SqueakCurrentRooms => squeakCurrentRooms;
        static int squeakCurrentRooms;
        public static int SqueakCrumbleRooms => squeakCrumbleRooms;
        static int squeakCrumbleRooms;
        static string AddSqueakRooms(byte[] rom, byte[] squeak, KatAMExpansionCode.Version version, ref int cursor) {
            IReadOnlyDictionary<string, int> symbols = version.RomSymbols;
            int here = cursor;      // (the cursor, which the local functions can't take by reference);
            squeakChaseRooms = squeakChaseWalls = squeakChaseSpawns = squeakChaseChests = squeakDoorMirrorsLeft = squeakCurrentRooms = squeakCrumbleRooms = 0;
            int roomProps = symbols["gRoomProps"], roomInfos = symbols["gUnk_08D6CD0C"], arrivals = symbols["gUnk_0893CB44"];
            squeakRooms = new List<SqueakPortedRoom>();
            squeakLairMinibosses.Clear();
            squeakLairEscorts.Clear();
            List<SqueakRoomFiles> files = SqueakWorldRooms(squeak);
            if (files.Count == 0) return "The Squeak Squad ROM's stages weren't found.";
            // (invincibility in the Squeak Squad rooms, SqueakRoomsInvincible());
            if (SqueakRoomsInvincible(rom) != 3) return $"The {version.Name} ROM's invincibility code doesn't match.";
            // (the spray paints' chests: the KatAM can each shows, KatAMExpansionSprays.cs);
            int[] sprayCans = Enumerable.Range(0, SqueakSprayCount).Select(n => SqueakSprayKatAMCan(rom, version.SpriteTables, squeak, n)).ToArray();
            // (the worlds' maps' chests, KatAMExpansionMaps.cs);
            Dictionary<(SqueakRoomFiles room, int record), int> mapChests = SqueakMapChests(squeak, files);
            squeakMapsPlaced.Clear();
            squeakPiecesPlaced.Clear();
            squeakScrollsPlaced.Clear();

            // (the expansion's code names the room tables by their bases only, as KatAM's does: a pointer inside one couldn't move with it);
            int codeEnd = CodeAddress + version.Code.Length;
            foreach ((int table, int size) in new[] { (roomProps, KatAMRoomCount * 0x28), (roomInfos, KatAMRoomCount * 4), (arrivals, KatAMRoomCount * 2) })
                for (int i = CodeAddress; i < codeEnd; i += 4) {
                    int word = ReadInt(rom, i) - 0x08000000;
                    if (word > table && word < table + size) return $"The ROM Expansion's code points inside KatAM's room tables ({word + 0x08000000:X8}).";
                }

            // (without NiDL's rooms, rooms.c's big rooms are set up here: AddRooms() does it otherwise);
            bool nidlRooms = roomTables != null;
            if (!nidlRooms && BigRooms(rom, version) is string bigError) return bigError;

            int[] vanillaSizes = RoomTableSizes(rom, roomProps, out _);
            int Props(int room, int field) => ReadShort(rom, roomProps + room * 0x28 + field * 2);
            int Entry(string table, int index) => ReadInt(rom, symbols[table] + index * 4) - 0x08000000;
            int paletteCycles = ReadInt(rom, Entry("gForegroundPalettes", Props(0x323, 11)) + 4),        // Unused (no cycles), but read;
                navigationRoutes = Entry("gUnk_08D63C28", Props(0x3C9, 17));                                  // Its 4 generic route lists;
            int noTypes = Allocate(ref here, 4, 4), squeakTaken = 0, squeakChests = 0, squeakCannons = 0, squeakSwitches = 0;
            List<int> vitalityChests = new List<int>();
            // (the chases' chests, by room: AddChaseEntries() makes them the chase's; the map viewer's block lists them, "CC" below);
            List<(int room, byte[] chest)> chaseChestsPlaced = new List<(int, byte[])>();
            // (per room its gimmicks' list for their task, squeak/gimmicks.c's gSqueakGimmickRooms: 0 none);
            int[] gimmickLists = new int[files.Count];
            // (per room its dig cells' records, abilities/squeak_moves.c's gSqueakDigRooms: 0 none);
            int[] digLists = new int[files.Count];
            // (the stages with a chase, their key from 1 in order, and every room of them: SqueakChaseOf());
            Dictionary<(int area, int stage), byte> chaseKeys = new Dictionary<(int, int), byte>();
            foreach (SqueakRoomFiles cf in files)
                if (!chaseKeys.ContainsKey((cf.Area, cf.Stage)) && SqueakChaseOf(squeak, cf, null).spawns.Count > 0 && chaseKeys.Count < 0x7F)
                    chaseKeys[(cf.Area, cf.Stage)] = (byte)(chaseKeys.Count + 1);
            // (per room the dark star's route, squeak/dark_star.c's gSqueakDarkStarRoutes: 0 none, SqueakDarkStarRoute());
            int[] darkStarRoutes = new int[files.Count];
            // (the arenas are NiDL's type's objects: only with its enemies, CreateNidlEnemy its spawn);
            bool arenas = ReadInt(rom, version.SpawnTable + NightmareID * 4) == CodeAddress + 0x08000000 + version.CreateNidlEnemy + 1;
            WriteInt(rom, noTypes, 0xFF);

            // Each table's new entries, after NiDL's or KatAM's (the doors lists after KatAM's shared one: from FirstDoorsList, so their
            // visited doors are rooms.c's);
            string[] tableNames = { "gForegroundInfo", "gForegroundPalettes", "gForegroundTilemaps", "gSolidityMaps", "gRoomTiledBGs", "gLevelObjLists",
                                    "gUnk_08D63C28", "gUnk_08D640A4" };
            int[] tableFields = { 10, 11, 12, 13, 15, 16, 17, 18 };
            Dictionary<string, List<int>> added = tableNames.ToDictionary(t => t, t => new List<int>());
            int First(string table) => nidlRooms ? roomTables[table].count : table == "gUnk_08D640A4" ? SharedDoors : vanillaSizes[tableFields[Array.IndexOf(tableNames, table)]];
            int Add(string table, int address) {
                added[table].Add(address + 0x08000000);
                return First(table) + added[table].Count - 1;
            }
            int DoorsList(List<byte> nineROM, int chests) {
                nineROM.AddRange(new byte[] { 0, 0, 0xFF, 0xFF });
                int nineROMAt = Allocate(ref here, nineROM.Count, 4), entry = Allocate(ref here, 8, 4);
                nineROM.ToArray().CopyTo(rom, nineROMAt);
                WriteInts(rom, entry, nineROMAt + 0x08000000, chests);
                return entry;
            }
            if (!nidlRooms) Add("gUnk_08D640A4", DoorsList(new List<byte>(), 0));

            // The room tables, moved and grown (with the Squeak Squad hub after the worlds' rooms, then the worlds' switch rooms, then the
            // Squeak Squad ability room, then the Dark Nebula hub where Squeak Squad's final stage is: BuildSqueakFinaleHub());
            bool finaleStage = files.Any(f => (f.Area, f.Stage) == SqueakFinalStage);
            int hubRoom = SqueakFirstRoom + files.Count, firstSwitchRoom = hubRoom + 1, abilityRoom = firstSwitchRoom + SqueakWorlds;
            int finaleHubRoom = finaleStage ? abilityRoom + 1 : -1, rows = abilityRoom + (finaleStage ? 2 : 1);
            int newProps = Allocate(ref here, rows * 0x28, 4), newInfos = Allocate(ref here, rows * 4, 4), newArrivals = Allocate(ref here, rows * 2, 4);
            Array.Copy(rom, roomProps, rom, newProps, KatAMRoomCount * 0x28);
            Array.Fill(rom, (byte)0xFF, newProps + KatAMRoomCount * 0x28, (rows - KatAMRoomCount) * 0x28);
            Array.Copy(rom, roomInfos, rom, newInfos, KatAMRoomCount * 4);
            for (int room = KatAMRoomCount; room < rows; room++) WriteInt(rom, newInfos + room * 4, ReadInt(rom, roomInfos + 0x397 * 4));
            Array.Copy(rom, arrivals, rom, newArrivals, KatAMRoomCount * 2);
            Array.Fill(rom, (byte)0, newArrivals + KatAMRoomCount * 2, (rows - KatAMRoomCount) * 2);

            Dictionary<int, List<Transport>> ridesFrom = new Dictionary<int, List<Transport>>();
            Dictionary<string, int> backgroundOf = new Dictionary<string, int>();
            Dictionary<string, (int width, int height)> backgroundSize = new Dictionary<string, (int, int)>();
            Dictionary<string, SqueakBackground> convertedViews = new Dictionary<string, SqueakBackground>();   // (converted ahead, below);
            // ViewBackground(); A far view's background (map/d_viw_NN.nsbtx, `room` the first room showing it), converted once and shared:
            // its table entry, or -1 if the view isn't there (its size in backgroundSize);
            int ViewBackground(string view, int room) {
                if (backgroundOf.TryGetValue(view, out int background)) return background;
                byte[] viewFile = SqueakFileAnyCase(squeak, view);
                if (viewFile == null) return -1;
                SqueakBackground farView = convertedViews.TryGetValue(view, out SqueakBackground ahead) ? ahead : ConvertSqueakFarView(SqueakFarViewPicture(viewFile));
                int bgTiles = AllocateCompressed(rom, CompressLZ77(farView.Tiles), ref here), bgColors = Allocate(ref here, 0x100, 4);
                for (int c = 0; c < 128; c++) WriteShort(rom, bgColors + c * 2, farView.Palettes[c]);
                byte[] bgMap = new byte[farView.Map.Length * 2];
                for (int t = 0; t < farView.Map.Length; t++) WriteShort(bgMap, t * 2, farView.Map[t]);
                int bgMapAt = AllocateCompressed(rom, bgMap, ref here), at = Allocate(ref here, 0x20, 4);
                WriteShorts(rom, at, farView.Width, farView.Height);
                WriteInts(rom, at + 8, bgTiles + 0x08000000, farView.Tiles.Length / 32, bgColors + 0x08000000);
                WriteShorts(rom, at + 0x14, 0x60, 0x80);
                WriteInts(rom, at + 0x18, bgMapAt + 0x08000000, room << 16);
                background = Add("gRoomTiledBGs", at);
                backgroundSize[view] = (farView.Width * 8, farView.Height * 8);
                return backgroundOf[view] = background;
            }
            try {
                // (each room's doors and width, and the rooms by their stage and place in it: the doors' targets);
                Dictionary<(int area, int stage, int index), int> roomOf = new Dictionary<(int, int, int), int>();
                List<SqueakDoor>[] doorsOf = new List<SqueakDoor>[files.Count];
                List<SqueakGimmick>[] starsOf = new List<SqueakGimmick>[files.Count];
                int[] widthOf = new int[files.Count], heightOf = new int[files.Count];
                List<SqueakPlanRoom> planRooms = new List<SqueakPlanRoom>();
                Dictionary<string, SqueakTileset> frontOf = new Dictionary<string, SqueakTileset>();
                for (int i = 0; i < files.Count; i++) {
                    ReportEach(0.00, 0.045, i, files.Count, "Reading Squeak Squad's rooms");
                    SqueakMap map = new SqueakMap(SqueakUnpack(SqueakFileAnyCase(squeak, files[i].Map)));
                    roomOf[(files[i].Area, files[i].Stage, files[i].Index)] = i;
                    doorsOf[i] = map.Doors();
                    starsOf[i] = map.Gimmicks().Where(g => g.Type == SqueakWarpStar).ToList();
                    widthOf[i] = map.Width;
                    heightOf[i] = map.Height;
                    (bool midboss, bool boss) = SqueakFights(squeak, files[i]);
                    planRooms.Add(new SqueakPlanRoom {
                        Index = i, Room = SqueakFirstRoom + i, Area = files[i].Area, Stage = files[i].Stage, Number = files[i].Index,
                        Width = map.Width, Height = map.Height, Collision = SqueakRoomCollision(map).collision, Doors = doorsOf[i],
                        Midboss = midboss, Boss = boss, Shut = SqueakShutCells(map),
                        Transports = map.Gimmicks().Where(g => g.Type is SqueakWarpStar or SqueakTurnCannon or SqueakFuseCannon).Select(g => (g.X / 16, g.Y / 16)).ToList(),
                        Switches = map.Gimmicks().Where(g => g.Type == SqueakSwitchGimmick).Select(g => (g.X / 16, g.Y / 16)).ToList(),
                        Gimmicks = map.Gimmicks().Where(g => g.Type is SqueakRopeGimmick or SqueakShutterGimmick or SqueakPlatformGimmick or SqueakCloudGimmick
                                                             or SqueakBoulderGimmick or SqueakFireGimmick or SqueakRockGimmick or SqueakIcicleGimmick or SqueakCandleGimmick)
                                                 .Select(g => (g.X / 16, g.Y / 16)).Concat(SqueakChestCells(squeak, files[i])).ToList(),
                        Rides = map.Gimmicks().Where(g => g.Type == SqueakFuseCannon && (g.Word(0) & 0xFF) < SqueakStarArrival).Select(g => (g.Word(0) & 0xFF) + 1).ToList() });
                    // (whether it draws the DS's nest: Randomize Squeak Lairs, SqueakNestKey(), SqueakNestDrawn());
                    if (files[i].Front != null && !frontOf.ContainsKey(files[i].Front))
                        frontOf[files[i].Front] = SqueakRoomTileset(squeak, files[i].Front, files[i]);
                    planRooms[^1].NestKey = SqueakNestKey(map);
                    planRooms[^1].NestDrawn = files[i].Front != null && SqueakNestDrawn(map, frontOf[files[i].Front]);
                    // (its chase records' Squeaks, for the Squeak Lairs: SqueakPlanRoom.Squeaks);
                    byte[] records = files[i].Objects == null ? null : SqueakFileAnyCase(squeak, files[i].Objects);
                    for (int r = 0, count = records == null || records.Length < 16 ? 0 : ReadShort(records, 12); r < count; r++) {
                        int at = 14 + r * 16, id = ReadShort(records, at + 2);
                        if ((records[at + 0xB] & SqueakChaseSpawn) != 0 && SqueakChaseIds.Contains(id) && SqueakCharacters.TryGetValue(id, out int character))
                            planRooms[^1].Squeaks.Add((character, ReadShort(records, at + 4), ReadShort(records, at + 6), records[at + 0xB]));
                    }
                }
                // (where the warp stars and fuse cannons land Kirby, SqueakRides() and the fuse cannons' doors: for the levels' way-out pass);
                Report(0.045, "Finding where Squeak Squad's warp stars and cannons land");
                foreach ((int from, List<Transport> list) in SqueakRides(squeak, files, roomOf, starsOf, doorsOf, widthOf, heightOf, 0))
                    foreach (Transport t in list) {
                        planRooms[from].RideTo.Add((t.X / 16, t.Y / 16, t.Room, t.DestX - 1, t.DestY - 1));
                        /* (and its room, the levels' way on (SqueakPlanRoom.Rides): where the ride lands Kirby, past the sky rooms it flies
                         * across (3-4-5, 8-3-6, 8-3-7), never stood in: counted as reached, 3-4-5 took world 3's way to world 4, which
                         * nothing then led to); */
                        int landed = t.Room - SqueakFirstRoom;
                        if (landed >= 0 && landed < files.Count && files[landed].Area == files[from].Area && files[landed].Stage == files[from].Stage)
                            planRooms[from].Rides.Add(files[landed].Index);
                        // (the rooms it flies across on the way, SqueakPlanRoom.Crossed);
                        foreach (RideLeg leg in t.Ride?.Skip(1).SkipLast(1) ?? Enumerable.Empty<RideLeg>())
                            if (leg.Room - SqueakFirstRoom is int crossed && crossed >= 0 && crossed < planRooms.Count && crossed != landed) planRooms[crossed].Crossed = true;
                    }
                foreach (SqueakPlanRoom plan in planRooms) {
                    SqueakMap map = new SqueakMap(SqueakUnpack(SqueakFileAnyCase(squeak, files[plan.Index].Map)));
                    foreach (SqueakGimmick g in map.Gimmicks().Where(g => g.Type == SqueakFuseCannon))
                        if (SqueakFuse(map) is SqueakFuseLine fuse && roomOf.TryGetValue((files[plan.Index].Area, files[plan.Index].Stage, (sbyte)(g.Word(0) & 0xFF) + 1), out int to)
                            && doorsOf[to].FirstOrDefault(d => d.Number == g.Word(0) >> 8) is SqueakDoor landing)
                            plan.RideTo.Add((fuse.Last.x + 1, fuse.Last.y, SqueakFirstRoom + to, landing.X, landing.Y));
                }
                // (the levels, KatAMExpansionSqueakLevels.cs: the mirrors each room gets);
                Report(0.05, "Planning Squeak Squad's levels, doors and mirrors");
                Dictionary<int, List<Door>> planned = PlanSqueakLevels(planRooms, firstSwitchRoom, out var worldStarts, out var bossRooms, out var switchDoors);
                List<SqueakLook> looks = new List<SqueakLook>();
                squeakPlanned = planned.ToDictionary(p => SqueakFirstRoom + p.Key, p => p.Value);
                /* (the warp stars, SqueakRides(): their flights after the ones in the flights' table WarpFlights() moved, which it
                 * moves here without NiDL's rooms, warpstar.c's detours with it, WarpRideDetours()); */
                if (!nidlRooms) {
                    WarpFlights(rom, symbols, ref here);
                    if (WarpRideDetours(rom, version) is string rideError) return rideError;
                }
                if (flightTable >= 0) ridesFrom = SqueakRides(squeak, files, roomOf, starsOf, doorsOf, widthOf, heightOf, flightCount);

                /* (the rooms' foregrounds and the far views first, as many at once as the computer has cores: each made from its own files
                 * alone (and the lairs planned above: their nests), the same whatever the order, then written in the rooms' order below
                 * (ViewBackground() and the foregrounds' WriteSqueakForeground()), so the ROM is the same byte for byte. One at a time they
                 * took a minute and a half, with nothing shown (the owner: "Ensure the program does not get stuck while also making ROM
                 * progress obvious"). The jobs handed out one at a time, the longest first (the far views, then the rooms by their size: a
                 * far view takes as long as about 30,000 blocks of rooms), so none is left alone at the end; the bar moves by that time; */
                SqueakForeground[] foregrounds = new SqueakForeground[files.Count];
                List<(string view, byte[] file)> viewFiles = files.Select(f => f.View).Where(v => v != null).Distinct()
                    .Select(v => (v, SqueakFileAnyCase(squeak, v))).Where(v => v.Item2 != null).ToList();
                const double ViewCost = 30000;
                double Cost(int job) => job < viewFiles.Count ? ViewCost : widthOf[job - viewFiles.Count] * heightOf[job - viewFiles.Count];
                List<int> jobOrder = Enumerable.Range(0, viewFiles.Count + files.Count).OrderByDescending(Cost).ToList();
                double allCost = jobOrder.Sum(Cost), costDone = 0;
                int jobsDone = 0;
                Report(0.065, "Converting Squeak Squad's room tiles and far views");
                try {
                    System.Threading.Tasks.Parallel.ForEach(System.Collections.Concurrent.Partitioner.Create(jobOrder, System.Collections.Concurrent.EnumerablePartitionerOptions.NoBuffering), job => {
                        if (job < viewFiles.Count) {
                            SqueakBackground converted = ConvertSqueakFarView(SqueakFarViewPicture(viewFiles[job].file));
                            lock (convertedViews) convertedViews[viewFiles[job].view] = converted;
                        } else {
                            int i = job - viewFiles.Count, room = SqueakFirstRoom + i;
                            SqueakRoomFiles f = files[i];
                            SqueakMap map = new SqueakMap(SqueakUnpack(SqueakFileAnyCase(squeak, f.Map)));
                            // (one too big is turned down below);
                            if (map.Width * map.Height <= BigRoomBlocks) {
                                SqueakTileset front = SqueakRoomTileset(squeak, f.Front, f), main = SqueakRoomTileset(squeak, f.Main, f);
                                main?.Cycle(f.Palette == null ? null : SqueakPaletteCycles(SqueakFileAnyCase(squeak, f.Palette)));
                                var cells = SqueakRoomCollision(map);
                                List<(int x, int y)> digs = SqueakDigCells(map, cells.collision);
                                SqueakMap drawn = SqueakGoalDoorways(SqueakNests(map, squeakLairs.Where(l => l.FromRoom == room).Select(l => (l.FromX, l.FromY)), squeakLairs.Any(l => l.Room == room)));
                                foregrounds[i] = ConvertSqueakForeground(drawn, front, main, ForegroundTiles, cells.blocks.Concat(SqueakRopeCells(map)).Concat(digs).ToList(),
                                                                         SqueakSigns(map), cells.chains.Union(cells.burnt).Union(digs).ToHashSet());
                            }
                        }
                        lock (foregrounds) {
                            costDone += Cost(job);
                            Report(0.065 + (0.78 - 0.065) * costDone / allCost, $"Converting Squeak Squad's room tiles and far views ({++jobsDone}/{jobOrder.Count})");
                        }
                    });
                } catch (AggregateException exception) {
                    // (as if it had been one at a time: the first one's error, for the error message below);
                    System.Runtime.ExceptionServices.ExceptionDispatchInfo.Capture(exception.InnerExceptions[0]).Throw();
                }

                /* (a world's boss room's chest moved into the room before it, its stage's first, by its door to the boss: the owner, "Treasures
                 * in Boss rooms like the one in Dedede's or Mecha Kracko's are completely forbidden, please relocate these chests to somewhere
                 * else sensible in Squeak Squad's map. The boss rooms can only have the shard/tomato spawn and that's it." That room is written
                 * first: what it was written with is kept (SqueakAntechamber) to write it again with the chest; the boss room's own chest
                 * template stays, off the stage (OffStageX: never spawned), so its other templates' numbers stay as they are); */
                Dictionary<int, SqueakAntechamber> antechambers = new Dictionary<int, SqueakAntechamber>();
                // (each room's music: SqueakRoomSong(), its stage's music the one a room with none keeps);
                int lastMusic = 0;
                (int area, int stage) musicStage = (-1, -1);
                for (int i = 0; i < files.Count; i++) {
                    ReportEach(0.78, 0.85, i, files.Count, "Writing Squeak Squad's rooms");
                    SqueakRoomFiles f = files[i];
                    int room = SqueakFirstRoom + i;
                    SqueakMap map = new SqueakMap(SqueakUnpack(SqueakFileAnyCase(squeak, f.Map)));
                    if ((f.Area, f.Stage) != musicStage) {
                        musicStage = (f.Area, f.Stage);
                        lastMusic = SqueakStageMusic(squeak, files, f.Area, f.Stage);
                    }
                    int song = SqueakRoomSong(map.Data, f.Area, ref lastMusic);
                    SqueakTileset front = SqueakRoomTileset(squeak, f.Front, f), main = SqueakRoomTileset(squeak, f.Main, f);
                    main?.Cycle(f.Palette == null ? null : SqueakPaletteCycles(SqueakFileAnyCase(squeak, f.Palette)));
                    int width = map.Width, height = map.Height;
                    if (width * height > BigRoomBlocks) return $"Squeak Squad's room {f.Area}-{f.Stage}-{f.Index} is bigger than KatAM can hold.";

                    // Collision; the star blocks (and what they leave: air, water in water);
                    (byte[] collision, List<(int x, int y)> blocks, List<byte> leaves, List<byte> blockValues, HashSet<(int x, int y)> chains, HashSet<(int x, int y)> burnt) = SqueakRoomCollision(map);
                    // (Animal's dig cells, SqueakDigCells(): drawn bare after the blocks and the ropes, their records apart);
                    List<(int x, int y)> digCells = SqueakDigCells(map, collision);

                    /* Doors (SqueakMap.Doors()): each way to another of the stage's rooms, entered with Up, Kirby landing on the door it
                     * names there (the cell he stands in), facing the room's middle: its cell KatAM's door (0xFF, 0xFD in water; in a
                     * star block what the block leaves, its record before the block's, as BuildSolidity()'s secret doors), its 8ROM record
                     * and a 9ROM entry for each place they lead to (KatAM looks a door's up as Kirby goes through it, never ending
                     * without it). The goals (no room) and the starts aren't doors; a hidden way (flag 4), a door the DS draws none
                     * for, gets a mirror as the others; */
                    List<byte> doorRecords = new List<byte>(), blockDoorRecords = new List<byte>();
                    List<(int room, int x, int y)> doorPlaces = new List<(int, int, int)>();
                    // (a Squeak Lair's DS doors aren't doors, its mirror its way out; a DS door into a lair leads there: PlanSqueakLevels());
                    SqueakLairPlan lairHere = squeakLairs.FirstOrDefault(l => l.Room == room);
                    // (the mirrors the DS's doors get, below: their cells, one-way where they have no way back, hidden till a block on them breaks);
                    List<(int x, int y, bool oneWay, bool secret)> dsMirrors = new List<(int, int, bool, bool)>();
                    for (int doorIndex = 0; doorIndex < doorsOf[i].Count && lairHere == null; doorIndex++) {
                        SqueakDoor door = doorsOf[i][doorIndex];
                        if (door.Kind == SqueakStartKind || door.Room == SqueakNoRoom || door.X >= width || door.Y >= height) continue;
                        if (!roomOf.TryGetValue((f.Area, f.Stage, door.Room + 1), out int to)) continue;
                        SqueakLairPlan into = squeakLairs.FirstOrDefault(l => l.FromRoom == room && l.FromDoor == doorIndex);
                        // (the door it lands on by its place in the room's list: 7 rooms number theirs out of order, 3-1-5's 0, 0, 2, and
                        // 3-1-4's way names its second);
                        SqueakDoor landing = door.Door < doorsOf[to].Count ? doorsOf[to][door.Door] : doorsOf[to].FirstOrDefault(d => d.Number == door.Door);
                        if (landing == null) continue;
                        int cell = door.Y * width + door.X, block = blocks.IndexOf((door.X, door.Y));
                        bool water = block >= 0 ? leaves[block] == KatAMWater : collision[cell] is >= 0x34 and <= 0x48;
                        byte value = water ? (byte)0xFD : (byte)0xFF;
                        (int room, int x, int y) place = into != null ? into.Landing : (SqueakFirstRoom + to, landing.X + 1, landing.Y + 1);
                        byte facing = into != null ? (byte)((into.Landing.x - 1) * 2 < widthOf[into.Room - SqueakFirstRoom] ? 1 : 2) : (byte)(landing.X * 2 < widthOf[to] ? 1 : 2);
                        byte[] record = { 3, value, (byte)door.X, (byte)door.Y, 0x0E, 0, 0xFF, 0xFF, (byte)place.room, (byte)(place.room >> 8),
                                          (byte)place.x, (byte)place.y, facing, 0xFF };
                        if (block >= 0) {
                            leaves[block] = value;
                            blockDoorRecords.AddRange(record);
                        } else {
                            collision[cell] = value;
                            doorRecords.AddRange(record);
                        }
                        if (!doorPlaces.Contains(place)) doorPlaces.Add(place);
                        /* (a mirror on it, as on every door but a wide one leading on (the owner: "There's some doors that are black doors,
                         * not mirrors. All doors have to be mirrors unless the door is wide and warps to another place that is not a goal
                         * room"): the DS's doorways are narrow; but a nest's (a Squeak Lair's door: the nest is its look); two-way where
                         * PlanSqueakLevels() gave it a way back. A hidden way (flag 4: the DS draws none) gets one too (the owner: "I think
                         * hidden doors should be replaced with visible mirror doors"), but not one with no floor under it (4-6-3's, which
                         * Up can't use); */
                        bool floored = door.Y + 1 < height && SqueakFloor(collision[cell + width]);
                        if (into == null && ((door.Flags & SqueakHiddenDoor) == 0 || floored))
                            dsMirrors.Add((door.X, door.Y, !planned[to].Any(b => b.Kind == DoorKind.Back && b.Room == room && b.DestX == door.X + 1 && b.DestY == door.Y + 1), block >= 0));
                    }
                    // (the levels' doors, PlanSqueakLevels(): KatAM's, as the DS's; a mirror on each but the goals, the DS's own doorways; a
                    // Mirra's, to the switch room, once its tiles are known, below);
                    foreach (Door door in planned[i].Where(d => d.Kind != DoorKind.SwitchEntry)) {
                        int cell = door.Y * width + door.X, block = blocks.IndexOf((door.X, door.Y));
                        bool water = block >= 0 ? leaves[block] == KatAMWater : collision[cell] is >= 0x34 and <= 0x48;
                        byte value = water ? (byte)0xFD : (byte)0xFF;
                        if (block >= 0) {
                            leaves[block] = value;
                            blockDoorRecords.AddRange(DoorRecord(door, value));
                        } else {
                            /* (a Squeak Lair's way out is built shut, its record alone, until its mini-boss is beaten: squeak/gimmicks.c's
                             * LairExit() (and with the NiDL ROM's arenas, enemies/arena.c's OpenDoors()) makes it a door then, its mirror
                             * showing (rule 8)); */
                            if (!door.Shut) collision[cell] = value;
                            doorRecords.AddRange(DoorRecord(door, value));
                        }
                        if (!doorPlaces.Contains((door.Room, door.DestX, door.DestY))) doorPlaces.Add((door.Room, door.DestX, door.DestY));
                    }

                    // Foreground: its tiles, palettes and tilemap (a nest, the DS's own, on each DS door into a Squeak Lair, and no black
                    // doorway in a lair: SqueakNests(); no goal door's wide doorway: SqueakGoalDoorways());
                    SqueakMap drawn = SqueakGoalDoorways(SqueakNests(map, squeakLairs.Where(l => l.FromRoom == room).Select(l => (l.FromX, l.FromY)), lairHere != null));
                    // (the cut ropes' cells drawn bare too, after the blocks: SqueakRopeCells());
                    List<(int x, int y)> ropeCells = SqueakRopeCells(map);
                    SqueakForeground foreground = foregrounds[i] ?? ConvertSqueakForeground(drawn, front, main, ForegroundTiles, blocks.Concat(ropeCells).Concat(digCells).ToList(),
                                                                                           SqueakSigns(map), chains.Union(burnt).Union(digCells).ToHashSet());
                    foregrounds[i] = null;      // (written: its memory let go);
                    (int info, int palette, int tilemapEntry) = WriteSqueakForeground(rom, foreground, room, paletteCycles, ref here);

                    // Background: the far view, shared;
                    string view = f.View ?? "";
                    if (!backgroundOf.TryGetValue(view, out int background)) {
                        background = f.View == null ? -1 : ViewBackground(f.View, room);
                        if (background < 0) background = backgroundOf[view] = Props(0x3C9, 15);
                    }
                    /* (it pans across once as the camera crosses the room, BackgroundRates(), the camera here from the room's top down to 8
                     * pixels from its bottom, 32 lower than KatAM's: its range the room's height - 168; KatAM draws the background's top at
                     * (its height - 160) - unk0C - (the room's height - 160 - the camera's y) * the rate, so unk0C is 8 rates up, keeping
                     * its top row at the screen's top with the camera at the room's top); */
                    (int rateX, int rateY) = backgroundSize.TryGetValue(view, out var bgSize) ? BackgroundRates(bgSize.width, bgSize.height, width * 16, height * 16 + 32, 0) : (0, 0);
                    int backgroundDown = -((8 * rateY + 0xFF) >> 8);

                    // Collision and its special blocks: a star block's, what it leaves (its cell drawn bare, foreground.Bare);
                    List<byte> specials = new List<byte>();
                    // (a fuse cannon's fuse, SqueakFuse(): its cells, each one's records the wall without the rope, then its own look);
                    List<SqueakGimmick> gimmicks = map.Gimmicks();
                    SqueakFuseLine fuse = gimmicks.Any(g => g.Type == SqueakFuseCannon) ? SqueakFuse(map) : null;
                    if (fuse != null) {
                        HashSet<(int, int)> fuseCells = fuse.Cells.Select(c => (c.x, c.y)).ToHashSet();
                        ushort[] Look(int x, int y) => new[] { foreground.Tilemap[2 * y * foreground.Columns + 2 * x], foreground.Tilemap[2 * y * foreground.Columns + 2 * x + 1],
                                                               foreground.Tilemap[(2 * y + 1) * foreground.Columns + 2 * x], foreground.Tilemap[(2 * y + 1) * foreground.Columns + 2 * x + 1] };
                        foreach ((int x, int y, byte value) in fuse.Cells) {
                            (int x, int y) wall = (x, y);
                            for (int d = 1, best = int.MaxValue; d <= 6 && best == int.MaxValue; d++)
                                for (int wy = y - d; wy <= y + d; wy++)
                                    for (int wx = x - d; wx <= x + d; wx++)
                                        if (wx >= 0 && wy >= 0 && wx < width && wy < height && collision[wy * width + wx] == KatAMSolid && !fuseCells.Contains((wx, wy))
                                            && Math.Abs(wx - x) + Math.Abs(wy - y) < best) {
                                            best = Math.Abs(wx - x) + Math.Abs(wy - y);
                                            wall = (wx, wy);
                                        }
                            foreach ((int lx, int ly) in new[] { wall, (x, y) }) {
                                specials.AddRange(new byte[] { 1, value, (byte)x, (byte)y, 0x12, 0, 0xFF, 0xFF });
                                foreach (ushort e in Look(lx, ly)) specials.AddRange(BitConverter.GetBytes(e));
                                specials.AddRange(new byte[] { value, 0xFF });
                            }
                        }
                        foreach ((int x, int y, byte value) in fuse.Cells) collision[y * width + x] = value;
                    }
                    for (int b = 0; b < blocks.Count; b++) {
                        specials.AddRange(new byte[] { 1, blockValues[b], (byte)blocks[b].x, (byte)blocks[b].y, 0x12, 0, 0xFF, 0xFF });
                        foreach (ushort e in foreground.Bare[b]) specials.AddRange(BitConverter.GetBytes(e));
                        specials.AddRange(new byte[] { leaves[b], 0xFF });
                    }
                    // (a rope's cells, cleared once it's cut: its look gone, air, water in water; a ladder's, the rope's KatAM look);
                    for (int r = 0; r < ropeCells.Count; r++) {
                        (int x, int y) = ropeCells[r];
                        byte value = collision[y * width + x];
                        if (value is not (KatAMLadder or KatAMLadderTop)) continue;
                        specials.AddRange(new byte[] { 1, value, (byte)x, (byte)y, 0x12, 0, 0xFF, 0xFF });
                        foreach (ushort e in foreground.Bare[blocks.Count + r]) specials.AddRange(BitConverter.GetBytes(e));
                        specials.AddRange(new byte[] { map.Collision(7, x, y) >> 30 == 1 ? KatAMWater : KatAMAir, 0xFF });
                    }
                    /* (a Mirra's door, to the switch room: its cell as it is until the Mirra breaks, which applies its block record, making
                     * it the door, its look its own; then its record, as KatAM's Mirras': the Mirra stands on a floor that can't break, so
                     * it opens its door where it is, KatAM's as NiDL's, mirra.c); */
                    foreach (Door door in planned[i].Where(d => d.Kind == DoorKind.SwitchEntry)) {
                        byte cellValue = collision[door.Y * width + door.X], value = cellValue is >= 0x34 and <= 0x48 ? (byte)0xFD : (byte)0xFF;
                        doorRecords.AddRange(DoorRecord(door, value));
                        doorRecords.AddRange(new byte[] { 1, cellValue, (byte)door.X, (byte)door.Y, 0x12, 0, 0xFF, 0xFF });
                        for (int q = 0; q < 4; q++) doorRecords.AddRange(BitConverter.GetBytes(foreground.Tilemap[(2 * door.Y + q / 2) * foreground.Columns + 2 * door.X + q % 2]));
                        doorRecords.AddRange(new byte[] { value, 0xFF });
                        if (!doorPlaces.Contains((door.Room, door.DestX, door.DestY))) doorPlaces.Add((door.Room, door.DestX, door.DestY));
                    }
                    // (the dig cells' records: u16 how many, then KatAM's block records, each its cell bare, leaving air or water);
                    if (digCells.Count > 0) {
                        List<byte> digs = new List<byte>(BitConverter.GetBytes((ushort)digCells.Count));
                        for (int d = 0; d < digCells.Count; d++) {
                            (int x, int y) = digCells[d];
                            digs.AddRange(new byte[] { 1, KatAMSolid, (byte)x, (byte)y, 0x12, 0, 0xFF, 0xFF });
                            foreach (ushort e in foreground.Bare[blocks.Count + ropeCells.Count + d]) digs.AddRange(BitConverter.GetBytes(e));
                            digs.AddRange(new byte[] { map.Collision(7, x, y) >> 30 == 1 ? KatAMWater : KatAMAir, 0xFF });
                        }
                        digLists[i] = Allocate(ref here, digs.Count, 4);
                        digs.ToArray().CopyTo(rom, digLists[i]);
                    }
                    specials.InsertRange(0, blockDoorRecords);
                    specials.AddRange(doorRecords);
                    specials.AddRange(new byte[] { 0, 0, 0, 0, 0, 0, 0xFF, 0xFF });
                    int collisionAt = AllocateCompressed(rom, CompressRL(collision), ref here), specialsAt = AllocateCompressed(rom, specials.ToArray(), ref here);
                    int solidity = Allocate(ref here, 8, 4);
                    WriteInts(rom, solidity, collisionAt + 0x08000000, specialsAt + 0x08000000);

                    // (its doors, the DS's and the levels', and where the levels' doors land Kirby in it: no enemy there);
                    List<(int x, int y)> doorCells = doorsOf[i].Select(d => (d.X, d.Y)).Concat(planned[i].Select(d => (d.X, d.Y)))
                        .Concat(planned.Values.SelectMany(list => list).Where(d => d.Room == SqueakFirstRoom + i).Select(d => (d.DestX - 1, d.DestY - 1))).ToList();
                    // (its world's shard, + 1, if it's the world's boss room: KatAM's 8, one a world, the randomizer's option shuffling them);
                    byte bossShard = bossRooms.TryGetValue(SqueakFirstRoom + i, out int bossWorld) ? (byte)bossWorld : (byte)0;
                    Dictionary<int, byte[]> heldChests = new Dictionary<int, byte[]>();
                    // (its chase's chests, if its stage has a chase: SqueakObjects() keeps them for it, AddChaseEntries());
                    List<byte[]> chaseChests = chaseKeys.ContainsKey((f.Area, f.Stage)) ? new List<byte[]>() : null;
                    (List<byte[]> templates, List<byte> types) = SqueakObjects(squeak, f, map, collision, arenas, doorCells, bossShard, ref squeakTaken, ref squeakChests,
                                                                                    vitalityChests, sprayCans, mapChests, heldChests, lairHere, chaseChests);
                    // (the dark star's route, where it is: the room's own, as the DS's, squeak/dark_star.c);
                    if (templates.Any(t => t[12] == SqueakID && t[14] == SqueakDarkStarCharacter)) darkStarRoutes[i] = SqueakDarkStarRoute(rom, map, ref here);
                    // (its cannons: an eight-direction cannon for each of gimmick 0xB, and its fuse cannon with its fuse; its switches);
                    // (the gimmicks that talk to each other: their task's list, its index in the room's list each one's);
                    (bool fightMidboss, bool fightBoss) = SqueakFights(squeak, f);
                    SqueakChase chase = null;
                    if (chaseKeys.TryGetValue((f.Area, f.Stage), out byte chaseKey)) {
                        (List<(int x, int y)> walls, List<(int x, int y, byte character, byte variant)> spawns) = SqueakChaseOf(squeak, f, doorCells);
                        /* (a Squeak Lair: none, its Squeak records its fight's and its Squeakers' (SqueakObjects()): the chase brought its
                         * Squeak out again beside its mini-boss, two of them (the owner: "Squeak lairs have double minibosses, that is a
                         * huge no-no")); */
                        if (lairHere != null) spawns.Clear();
                        chase = new SqueakChase(chaseKey, walls, spawns, chaseChests);
                        squeakChaseChests += chaseChests?.Count ?? 0;
                        foreach (byte[] chaseChest in chaseChests ?? new List<byte[]>()) chaseChestsPlaced.Add((SqueakFirstRoom + i, chaseChest));
                        squeakChaseRooms++;
                        squeakChaseWalls += walls.Count;
                        squeakChaseSpawns += spawns.Count;
                    }
                    // (its currents' grid, for the room's task: SqueakCurrents());
                    int currentsAt = 0;
                    if (SqueakCurrents(map) is byte[] currentGrid) {
                        currentsAt = Allocate(ref here, currentGrid.Length, 4);
                        currentGrid.CopyTo(rom, currentsAt);
                        squeakCurrentRooms++;
                    }
                    // (and its walk-once blocks' list: SqueakCrumbles());
                    int crumblesAt = 0;
                    if (SqueakCrumbles(map) is byte[] crumbleList) {
                        crumblesAt = Allocate(ref here, crumbleList.Length, 4);
                        crumbleList.CopyTo(rom, crumblesAt);
                        squeakCrumbleRooms++;
                    }
                    if (SqueakGimmickList(gimmicks, chains, arenas && (fightMidboss || fightBoss), map.Height * 16, chase,
                                          planned[i].Where(d => d.Shut).Select(d => (d.X, d.Y)).ToList(),
                                          currentsAt != 0 ? currentsAt + 0x08000000 : 0, crumblesAt != 0 ? crumblesAt + 0x08000000 : 0,
                                          SqueakPhaseWarps.Where(w => w.Room == room)) is byte[] gimmickList) {
                        gimmickLists[i] = Allocate(ref here, gimmickList.Length, 4);
                        gimmickList.CopyTo(rom, gimmickLists[i]);
                    }
                    for (int gi = 0; gi < gimmicks.Count; gi++) {
                        SqueakGimmick g = gimmicks[gi];
                        if (g.Type == SqueakSwitchGimmick) {
                            /* (its targets, by their index in the room's list: squeak/gimmicks.c, its unk14 and unk16 its parameter's 4
                             * bytes; a switch none of whose targets is ported isn't made: a bubble, a path); */
                            if (gi >= SqueakMailGimmicks || g.Parameters == null || g.Parameters.Length < 4
                                || !SqueakMessageTargets(g).Any(t => t < gimmicks.Count && t < SqueakMailGimmicks
                                                                     && (gimmicks[t].Type is SqueakShutterGimmick or SqueakFireGimmick or SqueakTriggerArea or SqueakRelay or SqueakRockGimmick or SqueakPlatformGimmick or SqueakQuake
                                                                         or SqueakBoulderGimmick
                                                                         || gimmicks[t].Type == SqueakSwitchBlocks && chains.Contains((gimmicks[t].X / 16, gimmicks[t].Y / 16)))))
                                continue;
                            byte[] button = ObjectTemplate(g.X, g.Y, templates.Count, HammerPegType);
                            button[0x11] = SqueakSwitchObject;
                            button[0x12] = (byte)gi;
                            WriteShort(button, 0x14, g.Parameters[0] | g.Parameters[1] << 8);
                            WriteShort(button, 0x16, g.Parameters[2] | g.Parameters[3] << 8);
                            templates.Add(button);
                            if (!types.Contains(HammerPegType)) types.Add(HammerPegType);
                            squeakSwitches++;
                        } else if (g.Type == SqueakShutterGimmick && gi < SqueakMailGimmicks) {
                            /* (a shutter: its index and its parameter, squeak/gimmicks.c's SqueakCreateShutter; and +0x16 on, its cells' values
                             * open, as the room has them, in ShutterSetBox()'s order: what it puts back when it opens. Read off the room's
                             * collision when it shut, one made again over its own shut cells (it was gone from the screen and came back) put
                             * its wall back for good: the owner's "even if you press the button, the collision never moves away"); */
                            byte[] shutter = ObjectTemplate(g.X, g.Y, templates.Count, HammerPegType);
                            shutter[0x11] = SqueakShutterObject;
                            shutter[0x12] = (byte)gi;
                            WriteShort(shutter, 0x14, g.Word(0));
                            int[] box = SqueakShutterBoxes[Math.Min(g.Word(0) & 7, SqueakShutterBoxes.Length - 1)];
                            for (int cy = (g.Y + box[1] + 1) >> 4, n = 0; cy <= (g.Y + box[3] - 2) >> 4; cy++)
                                for (int cx = (g.X + box[0] + 1) >> 4; cx <= (g.X + box[2] - 2) >> 4 && n < 4; cx++, n++)
                                    shutter[0x16 + n] = cx >= 0 && cy >= 0 && cx < width && cy < height ? collision[cy * width + cx] : KatAMAir;
                            templates.Add(shutter);
                            if (!types.Contains(HammerPegType)) types.Add(HammerPegType);
                            squeakSwitches++;
                        } else if (g.Type == SqueakFireGimmick && gi < SqueakMailGimmicks && g.Parameters?.Length >= 4) {
                            // (a fire column: its index and its two words, squeak/gimmicks.c's SqueakCreateFire);
                            byte[] fire = ObjectTemplate(g.X, g.Y, templates.Count, HammerPegType);
                            fire[0x11] = SqueakFireObject;
                            fire[0x12] = (byte)gi;
                            WriteShort(fire, 0x14, g.Word(0));
                            WriteShort(fire, 0x16, g.Word(1));
                            templates.Add(fire);
                            if (!types.Contains(HammerPegType)) types.Add(HammerPegType);
                            squeakSwitches++;
                        } else if (g.Type is SqueakGrassGimmick or SqueakBarrelGimmick) {
                            // (a grass tuft or a barrel: what it holds, squeak/gimmicks.c's SqueakCreateGrass, SqueakCreateBarrel);
                            (byte type, byte behavior, byte variant) = SqueakHeld(f.Objects == null ? null : SqueakFileAnyCase(squeak, f.Objects), g.Parameters);
                            byte[] holder = ObjectTemplate(g.X, g.Y, templates.Count, HammerPegType);
                            holder[0x11] = g.Type == SqueakGrassGimmick ? SqueakGrassObject : SqueakBarrelObject;
                            holder[0x12] = (byte)(g.Parameters?.Length > 0 ? g.Parameters[0] & 15 : 0);
                            holder[0x14] = type;
                            holder[0x15] = behavior;
                            holder[0x16] = variant;
                            /* (a chest it holds, SqueakObjects()'s heldChests: its type and item, its mark and number (unk16), its own
                             * number and its half flag (unk18): squeak/gimmicks.c's LetOutChest()); */
                            if (g.Parameters?.Length >= 2 && g.Parameters[0] >> 4 == 1 && heldChests.TryGetValue(g.Parameters[1], out byte[] heldChest)) {
                                holder[0x14] = heldChest[12];
                                holder[0x15] = heldChest[14];
                                holder[0x16] = heldChest[0x16];
                                holder[0x17] = heldChest[0x17];
                                holder[0x18] = heldChest[0x12];
                                holder[0x19] = heldChest[0x15];
                            }
                            templates.Add(holder);
                            if (!types.Contains(HammerPegType)) types.Add(HammerPegType);
                            squeakSwitches++;
                        } else if (g.Type == SqueakRockGimmick && gi < SqueakMailGimmicks) {
                            // (a falling rock: its index, its messages its start, squeak/gimmicks.c's SqueakCreateRock);
                            byte[] rock = ObjectTemplate(g.X, g.Y, templates.Count, HammerPegType);
                            rock[0x11] = SqueakRockObject;
                            rock[0x12] = (byte)gi;
                            WriteShort(rock, 0x14, g.Word(0));
                            templates.Add(rock);
                            if (!types.Contains(HammerPegType)) types.Add(HammerPegType);
                            squeakSwitches++;
                        } else if (g.Type == SqueakPlatformGimmick && g.Parameters?.Length >= 2) {
                            // (a moving platform: its index for its messages, its first word, its legs' two bytes each);
                            byte[] platform = ObjectTemplate(g.X, g.Y, templates.Count, HammerPegType);
                            platform[0x11] = SqueakPlatformObject;
                            platform[0x12] = (byte)Math.Min(gi, 0xFF);
                            WriteShort(platform, 0x14, g.Word(0));
                            for (int leg = 0; leg < SqueakPlatformLegs && 3 + 2 * leg < g.Parameters.Length; leg++) {
                                platform[0x16 + 2 * leg] = g.Parameters[2 + 2 * leg];
                                platform[0x17 + 2 * leg] = g.Parameters[3 + 2 * leg];
                            }
                            templates.Add(platform);
                            if (!types.Contains(HammerPegType)) types.Add(HammerPegType);
                            squeakSwitches++;
                        } else if (g.Type == SqueakRopeFloor && g.Parameters?.Length >= 1) {
                            /* (a rope floor: a moving platform with no legs that breaks, the DS's 0x022219C4: its look its first byte's low
                             * nibble; it falls when a Kirby stands on it if bits 4-6 are 1 (6-2-1's), else on its rope's message); */
                            byte[] floor = ObjectTemplate(g.X, g.Y, templates.Count, HammerPegType);
                            floor[0x11] = SqueakPlatformObject;
                            floor[0x12] = (byte)Math.Min(gi, 0xFF);
                            WriteShort(floor, 0x14, (g.Parameters[0] & 0xF) | ((g.Parameters[0] >> 4 & 7) == 1 ? 2 : 3) << 4 | 1 << 7);
                            templates.Add(floor);
                            if (!types.Contains(HammerPegType)) types.Add(HammerPegType);
                            squeakSwitches++;
                        } else if (g.Type == SqueakRopeGimmick && g.Parameters?.Length >= 2 && gi < SqueakMailGimmicks) {
                            // (a rope: its 6 bytes in unk14, unk16, unk18, its targets messaged when an attack cuts it);
                            byte[] rope = ObjectTemplate(g.X, g.Y, templates.Count, HammerPegType);
                            rope[0x11] = SqueakRopeObject;
                            rope[0x12] = (byte)gi;
                            for (int b = 0; b < 6; b++) rope[0x14 + b] = b < g.Parameters.Length ? g.Parameters[b] : (byte)0xFF;
                            templates.Add(rope);
                            if (!types.Contains(HammerPegType)) types.Add(HammerPegType);
                            squeakSwitches++;
                        } else if (g.Type == SqueakCloudGimmick && g.Parameters?.Length >= 6) {
                            /* (a thunder cloud: a moving platform's first word and legs, the legs from its 6th byte, its two u16s before
                             * them the frames it rests and zaps, unk1C and unk1E); */
                            byte[] cloud = ObjectTemplate(g.X, g.Y, templates.Count, HammerPegType);
                            cloud[0x11] = SqueakCloudObject;
                            cloud[0x12] = (byte)Math.Min(gi, 0xFF);
                            WriteShort(cloud, 0x14, g.Word(0));
                            for (int leg = 0; leg < SqueakPlatformLegs && 7 + 2 * leg < g.Parameters.Length; leg++) {
                                cloud[0x16 + 2 * leg] = g.Parameters[6 + 2 * leg];
                                cloud[0x17 + 2 * leg] = g.Parameters[7 + 2 * leg];
                            }
                            WriteShort(cloud, 0x1C, ReadShort(g.Parameters, 2));
                            WriteShort(cloud, 0x1E, ReadShort(g.Parameters, 4));
                            templates.Add(cloud);
                            if (!types.Contains(HammerPegType)) types.Add(HammerPegType);
                            squeakSwitches++;
                        } else if (g.Type == SqueakCandleGimmick) {
                            // (a candle: its word, its lit frames in bits 4-15, squeak/gimmicks.c's SqueakCreateCandle);
                            byte[] candle = ObjectTemplate(g.X, g.Y, templates.Count, HammerPegType);
                            candle[0x11] = SqueakCandleObject;
                            WriteShort(candle, 0x14, g.Word(0));
                            templates.Add(candle);
                            if (!types.Contains(HammerPegType)) types.Add(HammerPegType);
                            squeakSwitches++;
                        } else if (g.Type == SqueakBoulderGimmick) {
                            // (a rolling boulder: its index for the message that starts a waiting one, its first word);
                            byte[] boulder = ObjectTemplate(g.X, g.Y, templates.Count, HammerPegType);
                            boulder[0x11] = SqueakBoulderObject;
                            boulder[0x12] = (byte)Math.Min(gi, 0xFF);
                            WriteShort(boulder, 0x14, g.Word(0));
                            templates.Add(boulder);
                            if (!types.Contains(HammerPegType)) types.Add(HammerPegType);
                            squeakSwitches++;
                        } else if (g.Type == SqueakIcicleGimmick) {
                            byte[] icicle = ObjectTemplate(g.X, g.Y, templates.Count, HammerPegType);
                            icicle[0x11] = SqueakIcicleObject;
                            templates.Add(icicle);
                            if (!types.Contains(HammerPegType)) types.Add(HammerPegType);
                            squeakSwitches++;
                        } else if (g.Type == SqueakTurnCannon) {
                            bool fixedWay = (g.Word(0) & 3) == 0;
                            // (its spot is the cannon's foot: the shot's line is 8 pixels up, 8-1-5's eight doors on its lines);
                            byte[] cannon = ObjectTemplate(g.X, g.Y - 8, templates.Count, EightDirCannonType);
                            cannon[0x14] = (byte)(g.Word(0) >> 2 & 7);
                            cannon[0x22] = (byte)(fixedWay ? 1 : 0);
                            cannon[0x11] = fixedWay ? SqueakFixedCannon : SqueakTurningCannon;
                            templates.Add(cannon);
                            if (!types.Contains(EightDirCannonType)) types.Add(EightDirCannonType);
                            squeakCannons++;
                        } else if (g.Type == SqueakFuseCannon && fuse != null
                                   && roomOf.TryGetValue((f.Area, f.Stage, (sbyte)(g.Word(0) & 0xFF) + 1), out int to)
                                   && doorsOf[to].FirstOrDefault(d => d.Number == g.Word(0) >> 8) is SqueakDoor landing) {
                            byte[] lit = ObjectTemplate(fuse.Light.x * 16 + 8, fuse.Light.y * 16 + 8, templates.Count, FuseType);
                            lit[4] = lit[5] = 1;
                            templates.Add(lit);
                            if (!types.Contains(FuseType)) types.Add(FuseType);
                            byte[] cannon = ObjectTemplate((fuse.Last.x + 1) * 16, fuse.Last.y * 16, templates.Count, CannonType);
                            cannon[4] = cannon[5] = 1;
                            WriteShort(cannon, 0x12, SqueakFirstRoom + to);
                            cannon[0x14] = 2;
                            WriteShort(cannon, 0x18, (landing.X + 1) << 8 | (landing.Y + 1));
                            cannon[0x22] = 2;
                            templates.Add(cannon);
                            if (!types.Contains(CannonType)) types.Add(CannonType);
                            squeakCannons++;
                        }
                    }
                    // (its warp stars, KatAM's (see Transport): +0xE the flight, +0x1A, +0x1C the landing cell + 1, +0x1E the room);
                    foreach (Transport t in ridesFrom.GetValueOrDefault(i, new List<Transport>())) {
                        byte[] star = ObjectTemplate(t.X, t.Y, templates.Count, WarpStarType);
                        star[14] = (byte)t.Flight;
                        WriteShorts(star, 0x1A, t.DestX, t.DestY, t.Room);
                        templates.Add(star);
                        if (!types.Contains(WarpStarType)) types.Add(WarpStarType);
                    }
                    // (a mirror on each of the levels' doors but the goals, as the NiDL rooms': a one-way one drawn as one, mirror_door.c; a
                    // Mirra on its door's cell, x the cell's middle, y its top, as KatAM's, and the door's mirror hidden till it's a door);
                    foreach (Door door in planned[i].Where(d => !d.Plain)) {
                        byte[] mirror = MirrorTemplate(door.X, door.Y, templates.Count);
                        if (door.OneWay) mirror[0x23] |= 1;
                        if (door.Kind == DoorKind.SwitchEntry || door.Shut) mirror[14] = 1;
                        templates.Add(mirror);
                        if (!types.Contains(MirrorType)) types.Add(MirrorType);
                        if (door.Kind != DoorKind.SwitchEntry) continue;
                        templates.Add(ObjectTemplate(door.X * 16 + 8, door.Y * 16, templates.Count, MirraType));
                        if (!types.Contains(MirraType)) types.Add(MirraType);
                    }
                    // (the DS's doors' mirrors, above; a secret one's hidden till its cell is a door, as NiDL's; while KatAM's 31 objects
                    // last: past them the doorway stays as the DS drew it, SqueakDoorMirrorsLeft counting those);
                    foreach ((int x, int y, bool oneWay, bool secret) in dsMirrors) {
                        if (templates.Count >= 31) { squeakDoorMirrorsLeft++; continue; }
                        byte[] mirror = MirrorTemplate(x, y, templates.Count);
                        if (oneWay) mirror[0x23] |= 1;
                        if (secret) mirror[14] = 1;
                        templates.Add(mirror);
                        if (!types.Contains(MirrorType)) types.Add(MirrorType);
                    }
                    // (KatAM keeps which of a room's objects are gone in 32 bits, 31 none);
                    if (templates.Count > 31) return $"Squeak Squad's room {f.Area}-{f.Stage}-{f.Index} has {templates.Count} objects (KatAM's rooms hold 31).";
                    if (squeakTaken > 3 * 256) return "Too many Squeak Squad items for their gone bits.";
                    if (squeakChests > SqueakChestsMax) return "Too many Squeak Squad chests for their bits.";
                    if (bossShard != 0 && antechambers.TryGetValue(room - 1, out SqueakAntechamber before)) {
                        foreach (byte[] chest in templates.Where(t => t[12] == SmallChestType && t[0x11] == SqueakChestIndex && t[0x14] == SqueakChestAfterBoss)) {
                            if (before.Spot() is not (int, int) spot) return $"Squeak Squad's room 0x{room - 1:X} has no floor for its boss room's chest.";
                            byte[] moved = (byte[])chest.Clone();
                            WriteShorts(moved, 6, spot.Item1 * 16 + 8, spot.Item2 * 16 + 8);
                            moved[3] = (byte)before.Templates.Count;
                            moved[0x14] = 0;
                            before.Templates.Add(moved);
                            before.Taken.Add(spot);
                            WriteShort(chest, 6, OffStageX);
                        }
                        if (before.Templates.Count > 31) return $"Squeak Squad's room 0x{room - 1:X} has too many objects with its boss room's chest.";
                        if (!before.Types.Contains(SmallChestType)) before.Types.Add(SmallChestType);
                        added["gLevelObjLists"][before.ObjectsIndex - First("gLevelObjLists")] = WriteObjectList(rom, before.Templates, before.Types, noTypes, ref here) + 0x08000000;
                        List<byte[]> movedChests = before.Templates.Where(t => t[12] == SmallChestType && t[0x11] == SqueakChestIndex).ToList();
                        List<byte> movedNine = movedChests.SelectMany(t => ChestEntry(ReadShort(t, 6), ReadShort(t, 8))).ToList();
                        foreach ((int room, int x, int y) place in before.DoorPlaces) movedNine.AddRange(NineROMEntry(place.room, place.x, place.y));
                        added["gUnk_08D640A4"][before.DoorsIndex - First("gUnk_08D640A4")] = DoorsList(movedNine, movedChests.Count) + 0x08000000;
                    }
                    int objects = WriteObjectList(rom, templates, types, noTypes, ref here);
                    // (a Squeak Lair's mini-boss's template, for the randomizer's mini-boss pool: SqueakLairMinibosses);
                    int lairFight = templates.FindIndex(t => t[12] != NightmareID && (t[0x22] & SqueakLairFight) != 0);
                    if (lairFight >= 0) squeakLairMinibosses.Add((room, ReadInt(rom, objects) - 0x08000000 + lairFight * 36));
                    // (and its Squeakers, SqueakLairEscorts);
                    for (int t = 0; lairFight >= 0 && t < templates.Count; t++)
                        if (templates[t][12] == SqueakID && templates[t][14] == SqueakerCharacter)
                            squeakLairEscorts.Add((room, ReadInt(rom, objects) - 0x08000000 + t * 36));
                    int navigation = BuildNavigation(rom, width, height, false, collision, navigationRoutes, new List<(int, int, int, int)>(), ref here);
                    /* (its chests in its 9ROM list, first, as KatAM's: {1, 8, 0xFF, 0xFF, the template's x, y}, and counted: opening one
                     * marks it there, sub_08002B30 looking it up by its spot, never ending without it; then its doors' places); */
                    List<byte[]> chestTemplates = templates.Where(t => t[12] == SmallChestType && t[0x11] == SqueakChestIndex).ToList();
                    List<byte> nineROM = chestTemplates.SelectMany(t => ChestEntry(ReadShort(t, 6), ReadShort(t, 8))).ToList();
                    foreach ((int room, int x, int y) place in doorPlaces) nineROM.AddRange(NineROMEntry(place.room, place.x, place.y));
                    int doors = DoorsList(nineROM, chestTemplates.Count);

                    // Room info: room 0x397's (no routes, the helpers' flags), its room; the map viewer's +0x4A, +0x4B: stage, room;
                    int roomInfo = Allocate(ref here, 0x4C, 4);
                    Array.Copy(rom, ReadInt(rom, roomInfos + 0x397 * 4) - 0x08000000, rom, roomInfo, 0x4C);
                    WriteShort(rom, roomInfo + 0x44, room);
                    rom[roomInfo + 0x4A] = (byte)f.Stage;
                    rom[roomInfo + 0x4B] = (byte)f.Index;
                    WriteInt(rom, newInfos + room * 4, roomInfo + 0x08000000);

                    rom[CodeAddress + version.CodeSymbols["gNidlRoomTops"] + room] = SqueakCameraTop;
                    int doorsIndex = Add("gUnk_08D640A4", doors);
                    if (doorsIndex < FirstDoorsList) return "Squeak Squad's doors lists start too early.";
                    if (doorsIndex >= FirstDoorsList + VisitedLists) return "Too many doors lists for rooms.c's visited doors.";
                    int infoIndex = Add("gForegroundInfo", info), paletteIndex = Add("gForegroundPalettes", palette);
                    int tilemapIndex = Add("gForegroundTilemaps", tilemapEntry), solidityIndex = Add("gSolidityMaps", solidity);
                    int objectsIndex = Add("gLevelObjLists", objects);
                    WriteShorts(rom, newProps + room * 0x28, song, 0, 0, 0x100, 0x100, 0, backgroundDown, rateX, rateY, RoomFlags,
                                infoIndex, paletteIndex, tilemapIndex, solidityIndex, 0xFFFF, background, objectsIndex, Add("gUnk_08D63C28", navigation),
                                doorsIndex, 0);
                    // (the room before a world's boss room, its stage's: kept, to take that room's chest, SqueakAntechamber);
                    if (bossRooms.ContainsKey(room + 1) && i + 1 < files.Count && (files[i + 1].Area, files[i + 1].Stage) == (f.Area, f.Stage)) {
                        List<(int x, int y)> toBoss = new List<(int, int)>();
                        for (int at = 0; at + 12 <= specials.Count && specials[at + 4] != 0; at += specials[at + 4] | specials[at + 5] << 8)
                            if (specials[at] == 3 && (specials[at + 8] | specials[at + 9] << 8) == room + 1) toBoss.Add((specials[at + 2], specials[at + 3]));
                        antechambers[room] = new SqueakAntechamber(templates, types, doorPlaces, objectsIndex, doorsIndex, collision, width, height,
                                                                   doorCells, toBoss);
                    }
                    // (its look, for the switch rooms drawn in a world's tiles);
                    looks.Add(new SqueakLook { World = f.Area, Stage = f.Stage, Number = f.Index, Room = room, Width = width, Height = height, Columns = foreground.Columns,
                                               Tilemap = foreground.Tilemap, Tiles = foreground.Tiles, View = f.View,
                                               Collision = collision, Info = infoIndex, Palette = paletteIndex, Background = background,
                                               BackgroundWidth = bgSize.width, BackgroundHeight = bgSize.height, Fight = planRooms[i].Midboss || planRooms[i].Boss });
                    squeakRooms.Add(new SqueakPortedRoom(f, room, width, height));
                }
                Report(0.85, "Building the Squeak Squad hub, ability room and switch rooms");
                // (where the hub's door to the ability room lands Kirby: its doorway, SqueakAbilityRoomDoor(), facing the room's middle);
                if (SqueakAbilityRoomFiles(squeak) is not SqueakRoomFiles abilityFiles
                    || new SqueakMap(SqueakUnpack(SqueakFileAnyCase(squeak, abilityFiles.Map))) is not SqueakMap abilityMap
                    || SqueakAbilityRoomDoor(abilityMap) is not SqueakDoor abilityDoorway)
                    return "Squeak Squad's ability room wasn't found.";
                (int x, int y, byte facing) abilityLanding = (abilityDoorway.X + 1, abilityDoorway.Y + 1, (byte)(abilityDoorway.X * 2 < abilityMap.Width ? 1 : 2));
                if (BuildSqueakHub(rom, symbols, nidlRooms, roomProps, newProps, newInfos, roomInfos, hubRoom, firstSwitchRoom, abilityRoom, abilityLanding, worldStarts[1],
                                   navigationRoutes, noTypes, Add, DoorsList,
                                   view => ViewBackground(view, hubRoom) is int at && at >= 0 ? (at, backgroundSize[view].width, backgroundSize[view].height) : null,
                                   out KatAMRoomEdit centralCircle, out byte[] hubCollision, out int hubWidth, ref here) is string hubError)
                    return hubError;
                if (BuildSqueakAbilityRoom(rom, squeak, version, t => nidlRooms ? roomTables[t].at : symbols[t], newProps, newInfos, abilityRoom, hubRoom,
                                           navigationRoutes, noTypes, paletteCycles, Add, DoorsList,
                                           view => ViewBackground(view, abilityRoom) is int at && at >= 0 ? (at, backgroundSize[view].width, backgroundSize[view].height) : null,
                                           ref here) is string abilityError)
                    return abilityError;
                // (the finale: into Squeak Squad's final stage through Central Circle's Dimension Mirror (and the NiDL hub's), whose
                // template the Dark Nebula hub's takes first; Dark Nebula's way out to the Dark Nebula hub, its mirror on into NiDL's finale);
                byte[] finaleMirrorTemplate = DimensionMirrorTemplate(rom, symbols);
                if (SqueakFinale(rom, version, symbols, t => nidlRooms ? roomTables[t].at : symbols[t], out (int room, int x, int y) finaleExit, out int fountainHub) is string finaleError)
                    return finaleError;
                (int room, int x, int y) finaleHubLanding = (finaleHubRoom, SqueakFinaleHubLanding.x, SqueakFinaleHubLanding.y);
                if (SqueakShardExits(rom, version, symbols, hubRoom, bossRooms, hubCollision, hubWidth, finaleHubLanding) is string exitError) return exitError;

                /* The switch rooms' hub doors (KatAMExpansionSqueakHubs.cs): to the Squeak Squad hub, onto the world's door there; to the
                 * KatAM room the world's path leads to (SqueakLinks), onto its door back there (SqueakLinkSpot(): those rooms as they'll be,
                 * with NiDL's doors or without, moved where the rule says, the rooms landing on a moved door landing on it there), all
                 * shown by the world's big switch (its slot); the designs: the hub's KatAM's hub door's, a KatAM room's its own (KatAMDesigns),
                 * the world's that of Central Circle's door it took in the hub (the area's whose door it is). The KatAM rooms' edits wait
                 * for the randomizer (squeakFinish, FinishRooms()), as NiDL's do; */
                Door greyDoor = new Door(SqueakHubDoor.x, SqueakHubDoor.y, hubRoom, SqueakHubDoor.x + 1, SqueakHubDoor.y + 1, (byte)(SqueakHubDoor.x * 2 < centralCircle.Width ? 1 : 2),
                                         Design: SqueakGreyDoorDesign);
                var linkEdits = new List<(int room, ((int x, int y) door, int drawX, ((int x, int y) from, (int x, int y) to)[] moves, (int from, int to)[] statues, int[] landings) plan, Door back)>();
                foreach (int world in switchDoors.Keys.OrderBy(w => w)) {
                    int switchRoom = firstSwitchRoom + world - 1, level = SqueakLayoutLevel(world), link = SqueakLinks[world - 1];
                    (int x, int y) hubDoor = SwitchHubDoor(level), linkDoor = SwitchLinkDoor(level), spot = MainHubDoors[world - 1];
                    byte[] ccDoor = centralCircle.Specials.FirstOrDefault(r => r[0] == 3 && r[2] == spot.x && r[3] == spot.y);
                    int design = ccDoor != null ? KatAMDesigns.GetValueOrDefault(ReadShort(ccDoor, 8), HubDesign) : HubDesign;
                    var plan = SqueakLinkSpot(world, nidlRooms);
                    Door back = new Door(plan.door.x, plan.door.y, switchRoom, linkDoor.x + 1, linkDoor.y + 1, 1, Slot: SqueakSlot(world), Design: design, Kind: DoorKind.KatAMLink);
                    linkEdits.Add((link, plan, back));
                    switchDoors[world].Add(new Door(linkDoor.x, linkDoor.y, link, plan.door.x + 1, plan.door.y + 1, (byte)(plan.door.x * 2 < SwitchWidth ? 1 : 2),
                                                    Slot: SqueakSlot(world), Design: KatAMDesigns.GetValueOrDefault(link, HubDesign), Kind: DoorKind.KatAMLink));
                    switchDoors[world].Add(new Door(hubDoor.x, hubDoor.y, hubRoom, spot.x + 1, spot.y + 1, (byte)(spot.x * 2 < hubWidth ? 1 : 2),
                                                    Slot: SqueakSlot(world), Design: HubDesign, Kind: DoorKind.Hub));
                    squeakPlanned[link] = squeakPlanned.GetValueOrDefault(link, new List<Door>()).Append(back).ToList();
                }
                squeakFinish = (target, finishAt) => {
                    Dictionary<int, KatAMRoomEdit> edits = new Dictionary<int, KatAMRoomEdit>();
                    KatAMRoomEdit Edit(int katamRoom) => edits.TryGetValue(katamRoom, out KatAMRoomEdit e) ? e
                        : edits[katamRoom] = LoadKatAMRoom(target, squeakProps, t => squeakTables[t], katamRoom);
                    AddKatAMDoor(Edit(CentralCircle), greyDoor);
                    foreach (var (link, plan, back) in linkEdits) {
                        KatAMRoomEdit katam = Edit(link);
                        List<KatAMRoomEdit> landing = plan.landings.Select(Edit).ToList();
                        foreach (var move in plan.moves) MoveKatAMDoor(katam, move.from, move.to, landing);
                        foreach (var (fromX, toX) in plan.statues)
                            foreach (byte[] template in katam.Templates.Where(t => t[12] is >= 0x92 and <= 0x95 && ReadShort(t, 6) == fromX)) WriteShort(template, 6, toX);
                        AddKatAMDoor(katam, back, plan.drawX);
                    }
                    if (Environment.GetEnvironmentVariable("SQUEAK_EDIT_DEBUG") != null)
                        foreach (KatAMRoomEdit e in edits.Values)
                            Console.Error.WriteLine($"edit {e.Room:X} {e.Width}x{e.Height}: " + string.Join(" ", e.Templates.Select(t => $"{t[12]:X2}@{ReadShort(t, 6)},{ReadShort(t, 8)}/{t[4]:X}"))
                                                    + " | doors " + string.Join(" ", e.Specials.Where(r => r[0] == 3).Select(r => $"({r[2]},{r[3]})->{ReadShort(r, 8):X}")));
                    SaveKatAMRooms(target, squeakProps, t => squeakTables[t], edits.Values, ref finishAt);
                    return finishAt;
                };
                if (BuildSqueakSwitchRooms(rom, firstSwitchRoom, looks, switchDoors, newProps, newInfos, roomInfos, navigationRoutes, noTypes, Add, DoorsList,
                                           view => SqueakFileAnyCase(squeak, view) is byte[] file ? SqueakFarViewPicture(file) : null, ref here) is string switchError)
                    return switchError;
                foreach (var pair in switchDoors) squeakPlanned[firstSwitchRoom + pair.Key - 1] = pair.Value;
                if (finaleStage) {
                    Report(0.985, "Drawing the Dark Nebula hub");
                    int finaleBoss = bossRooms.FirstOrDefault(b => b.Value == SqueakFinaleShard).Key;
                    if (BuildSqueakFinaleHub(rom, version, finaleHubRoom, fountainHub, finaleMirrorTemplate, finaleExit,
                                             looks.Where(l => (l.World, l.Stage) == SqueakFinalStage && !l.Fight), looks.FirstOrDefault(l => l.Room == finaleBoss)?.View,
                                             ReadShort(rom, newProps + hubRoom * 0x28), newProps, newInfos, roomInfos, navigationRoutes, noTypes, Add, DoorsList,
                                             view => SqueakFileAnyCase(squeak, view) is byte[] file ? SqueakFarViewPicture(file) : null, ref squeakTaken, ref here) is string finaleHubError)
                        return finaleHubError;
                    if (squeakTaken > 3 * 256) return "Too many Squeak Squad items for their gone bits.";
                }
                squeakPlanned[hubRoom] = squeakHubDoors.Append(squeakAbilityDoor).ToList();
                squeakPlanned[CentralCircle] = squeakPlanned.GetValueOrDefault(CentralCircle, new List<Door>())
                    .Append(new Door(SqueakHubDoor.x, SqueakHubDoor.y, hubRoom, SqueakHubDoor.x + 1, SqueakHubDoor.y + 1, 1)).ToList();

                /* The world map (KatAMExpansionWorldMap.cs: installed here without NiDL's rooms) and where Kirby is on it in Squeak Squad's
                 * rooms ("ROM Expansion/worldmap.c"'s gSqueakWorldRooms: each world's first room, the hub, the first switch room); */
                if (AddWorldMap(rom, version, ref here) is string mapError) return mapError;
                int worldRooms = CodeAddress + version.CodeSymbols["gSqueakWorldRooms"];
                for (int world = 1; world <= SqueakWorlds; world++) {
                    int firstOf = files.FindIndex(f => f.Area == world);
                    if (firstOf < 0) return $"Squeak Squad's world {world} has no rooms.";
                    WriteShort(rom, worldRooms + (world - 1) * 2, SqueakFirstRoom + firstOf);
                }
                WriteShorts(rom, worldRooms + SqueakWorlds * 2, hubRoom, firstSwitchRoom);
            } catch (Exception exception) when (exception is IndexOutOfRangeException or ArgumentException or InvalidOperationException) {
                return $"The Squeak Squad ROM's rooms couldn't be read ({exception.Message}).";
            }

            Report(0.995, "Writing Squeak Squad's room tables");
            /* The warp stars' rides: the flights' table grown by them (KatAM's code's pointer to it moved), each one's script
             * (RideFlight()), its landing in the arrivals' copy; */
            List<Transport> rides = ridesFrom.Values.SelectMany(list => list).OrderBy(t => t.Flight).ToList();
            if (rides.Count > 0) {
                if (flightCount + rides.Count > 0x100) return "Too many warp star flights.";
                int grown = Allocate(ref here, (flightCount + rides.Count) * 4, 4);
                Array.Copy(rom, flightTable, rom, grown, flightCount * 4);
                Repoint(rom, flightTable + 0x08000000, grown + 0x08000000);
                flightTable = grown;
                foreach (Transport t in rides) RideFlight(rom, t, grown, newArrivals, ref here);
                flightCount += rides.Count;
            }

            // The tables the props index: the new entries in NiDL's copies' room, or new copies of KatAM's (where squeakFinish finds them);
            squeakProps = newProps;
            squeakTables = new Dictionary<string, int>();
            foreach (string table in tableNames) {
                List<int> entries = added[table];
                if (nidlRooms) {
                    (int at, int count, int capacity) = roomTables[table];
                    if (count + entries.Count > capacity) return $"Too many Squeak Squad rooms for the room tables ({table}).";
                    WriteInts(rom, at + count * 4, entries.ToArray());
                    roomTables[table] = (at, count + entries.Count, capacity);
                    squeakTables[table] = at;
                    continue;
                }
                int size = First(table), copy = Allocate(ref here, (size + entries.Count) * 4, 4);
                Array.Copy(rom, symbols[table], rom, copy, size * 4);
                WriteInts(rom, copy + size * 4, entries.ToArray());
                RepointEverywhere(rom, version, symbols[table], copy);
                squeakTables[table] = copy;
            }
            RepointEverywhere(rom, version, roomProps, newProps);
            RepointEverywhere(rom, version, roomInfos, newInfos);
            RepointEverywhere(rom, version, arrivals, newArrivals);
            // (the map viewer's rooms block names the room info table: its copy now);
            if (nidlRooms) WriteInt(rom, MapRoomsBlock + 4 + 6 * 4, newInfos + 0x08000000);
            /* The map viewer's Squeak Squad rooms block (KatAM Map Tracker's RomMap.ReadSqueakRooms(); the map table's +0xD4, free
             * till now, points at it): "SQ", u16 the first room, u16 the rooms (the worlds', the hub, the switch rooms, the ability
             * room, the Dark Nebula hub), u16 the hub, u16 the first switch room, u16 the Squeak Squad ability room, then the tables the
             * game uses (gRoomProps, gUnk_08D6CD0C, gSolidityMaps, gForegroundTilemaps, gForegroundPalettes, gForegroundInfo, gRoomTiledBGs, gLevelObjLists), then a byte a room: its world
             * (1-8; the hub 0; a switch room its world's; the Dark Nebula hub the final stage's, 8), and one more: its stage (0 the hub and
             * the switch rooms; the Dark Nebula hub 4), then the finale's ways on by themselves (SqueakPhaseWarps): "PW", u16 how many,
             * 8 bytes each: u16 the room, u16 the room it goes on to, u8 x, u8 y (Kirby's cell in the room), u8 x, u8 y (his landing cell
             * there, + 1), then the chases' chests (SqueakChase's Chests: in no object list, the chase brings them out): "CC", u16 how
             * many, 12 bytes each: u16 the room, u16 x, u16 y (the chest's template's), u8 its type (0x80, 0x81), u8 its item, u8 its
             * number or mark's number (template +0x16), u8 its mark (+0x17), u8 its half flag (+0x15), u8 0; */
            int mapBlock = Allocate(ref here, 12 + 8 * 4 + (rows - SqueakFirstRoom) * 2 + 4 + SqueakPhaseWarps.Count * 8 + 4 + chaseChestsPlaced.Count * 12, 4);
            System.Text.Encoding.ASCII.GetBytes("SQ").CopyTo(rom, mapBlock);
            WriteShort(rom, mapBlock + 2, SqueakFirstRoom);
            WriteShort(rom, mapBlock + 4, rows - SqueakFirstRoom);
            WriteShort(rom, mapBlock + 6, hubRoom);
            WriteShort(rom, mapBlock + 8, firstSwitchRoom);
            WriteShort(rom, mapBlock + 10, abilityRoom);
            WriteInts(rom, mapBlock + 12, new[] { newProps, newInfos }.Concat(new[] { "gSolidityMaps", "gForegroundTilemaps", "gForegroundPalettes",
                "gForegroundInfo", "gRoomTiledBGs", "gLevelObjLists" }.Select(t => squeakTables[t])).Select(a => a + 0x08000000).ToArray());
            int worlds = mapBlock + 12 + 8 * 4, stages = worlds + (rows - SqueakFirstRoom);
            // (free space is 0xFF: the hub's and the switch rooms' bytes are 0 unless set below);
            Array.Clear(rom, worlds, (rows - SqueakFirstRoom) * 2);
            for (int i = 0; i < files.Count; i++) {
                rom[worlds + i] = (byte)files[i].Area;
                rom[stages + i] = (byte)files[i].Stage;
            }
            for (int w = 0; w < SqueakWorlds; w++) rom[worlds + firstSwitchRoom - SqueakFirstRoom + w] = (byte)(w + 1);
            // (the Dark Nebula hub: with the final stage's rooms, after Dark Nebula's, where his way out leads);
            if (finaleStage) (rom[worlds + finaleHubRoom - SqueakFirstRoom], rom[stages + finaleHubRoom - SqueakFirstRoom]) = ((byte)SqueakFinalStage.area, (byte)SqueakFinalStage.stage);
            int phaseWarps = stages + (rows - SqueakFirstRoom);
            System.Text.Encoding.ASCII.GetBytes("PW").CopyTo(rom, phaseWarps);
            WriteShort(rom, phaseWarps + 2, SqueakPhaseWarps.Count);
            for (int w = 0; w < SqueakPhaseWarps.Count; w++) {
                SqueakPhaseWarp warp = SqueakPhaseWarps[w];
                WriteShort(rom, phaseWarps + 4 + w * 8, warp.Room);
                WriteShort(rom, phaseWarps + 6 + w * 8, warp.ToRoom);
                (rom[phaseWarps + 8 + w * 8], rom[phaseWarps + 9 + w * 8]) = ((byte)warp.X, (byte)warp.Y);
                (rom[phaseWarps + 10 + w * 8], rom[phaseWarps + 11 + w * 8]) = ((byte)warp.ToX, (byte)warp.ToY);
            }
            int chaseChestList = phaseWarps + 4 + SqueakPhaseWarps.Count * 8;
            System.Text.Encoding.ASCII.GetBytes("CC").CopyTo(rom, chaseChestList);
            WriteShort(rom, chaseChestList + 2, chaseChestsPlaced.Count);
            for (int c = 0; c < chaseChestsPlaced.Count; c++) {
                (int chestRoom, byte[] chest) = chaseChestsPlaced[c];
                int at = chaseChestList + 4 + c * 12;
                WriteShort(rom, at, chestRoom);
                WriteShort(rom, at + 2, ReadShort(chest, 6));
                WriteShort(rom, at + 4, ReadShort(chest, 8));
                (rom[at + 6], rom[at + 7], rom[at + 8], rom[at + 9], rom[at + 10], rom[at + 11]) = (chest[0xC], chest[14], chest[0x16], chest[0x17], chest[0x15], 0);
            }
            WriteInt(rom, MapSqueakBlock, mapBlock + 0x08000000);
            // (the gimmicks' lists, squeak/gimmicks.c's gSqueakGimmickRooms: a pointer a room from SqueakFirstRoom, and how many);
            int gimmickTable = Allocate(ref here, files.Count * 4, 4);
            for (int k = 0; k < files.Count; k++) WriteInt(rom, gimmickTable + k * 4, gimmickLists[k] == 0 ? 0 : gimmickLists[k] + 0x08000000);
            WriteInts(rom, CodeAddress + version.CodeSymbols["gSqueakGimmickRooms"], gimmickTable + 0x08000000, files.Count);
            // (the dig cells' lists, abilities/squeak_moves.c's gSqueakDigRooms: the same way);
            int digTable = Allocate(ref here, files.Count * 4, 4);
            for (int k = 0; k < files.Count; k++) WriteInt(rom, digTable + k * 4, digLists[k] == 0 ? 0 : digLists[k] + 0x08000000);
            WriteInts(rom, CodeAddress + version.CodeSymbols["gSqueakDigRooms"], digTable + 0x08000000, files.Count);
            // (the dark star's routes, squeak/dark_star.c's gSqueakDarkStarRoutes: the same way);
            int routeTable = Allocate(ref here, files.Count * 4, 4);
            for (int k = 0; k < files.Count; k++) WriteInt(rom, routeTable + k * 4, darkStarRoutes[k] == 0 ? 0 : darkStarRoutes[k] + 0x08000000);
            WriteInts(rom, CodeAddress + version.CodeSymbols["gSqueakDarkStarRoutes"], routeTable + 0x08000000, files.Count);
            // (the Vitality halves' chests, chest.c's gSqueakVitalityChests: 0xFF none);
            for (int v = 0; v < SqueakVitalityChestsMax; v++)
                rom[CodeAddress + version.CodeSymbols["gSqueakVitalityChests"] + v] = (byte)(v < vitalityChests.Count ? vitalityChests[v] : 0xFF);
            // (the chests' spawn and opening, chest.c's);
            if (squeakChests > 0 && HookSqueakChests(rom, version) is string chestError) return chestError;
            // (the fixed cannons' wait, chest.c's);
            if (squeakCannons > 0 && HookSqueakCannons(rom, version) is string cannonError) return cannonError;
            if (squeakSwitches > 0 && HookSqueakSwitches(rom, version) is string buttonError) return buttonError;
            cursor = finishCursor = here;
            return null;
        }

        // RepointEverywhere(); A table moved: KatAM's code's literals (Repoint()) and the expansion's code's;
        static void RepointEverywhere(byte[] rom, KatAMExpansionCode.Version version, int from, int to) {
            Repoint(rom, from + 0x08000000, to + 0x08000000);
            for (int i = CodeAddress; i < CodeAddress + version.Code.Length; i += 4)
                if (ReadInt(rom, i) == from + 0x08000000) WriteInt(rom, i, to + 0x08000000);
        }
    }
}
