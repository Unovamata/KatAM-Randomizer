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

        // A room of a stage: its files (map, front and main tilesets, objects, far view);
        internal sealed record SqueakRoomFiles(int Area, int Stage, int Index, string Map, string Front, string Main, string Objects, string View);

        /* SqueakStageRooms(); A stage's rooms in play order, from its .mxi: 12 bytes (+8 its area, stage and room count), then chunks
         * (two letters, u16 length, value): MA starts a room (its map), FB and BB its tilesets, DE its objects, 3D its far view. Rooms
         * whose map isn't in the ROM are left out (Nature Notch's sixth stage lists two, 12 and 13, it never had); */
        static List<SqueakRoomFiles> SqueakStageRooms(byte[] rom, int area, int stage) {
            byte[] list = SqueakFile(rom, $"map/a{area}s{stage}.mxi");
            List<SqueakRoomFiles> rooms = new List<SqueakRoomFiles>();
            if (list == null) return rooms;
            Dictionary<string, string> room = null;
            void Finish() {
                if (room != null && room.ContainsKey("MA") && SqueakFileAnyCase(rom, room["MA"]) != null)
                    rooms.Add(new SqueakRoomFiles(area, stage, rooms.Count + 1, room["MA"], room.GetValueOrDefault("FB"), room.GetValueOrDefault("BB"),
                                                  room.GetValueOrDefault("DE"), room.GetValueOrDefault("3D")));
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
         * terrain's (table 2, the main tileset), 5 the third layer's (table 2, drawn over both), 6-8 their collision (u32 a cell),
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
        }

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
            // A tile's pixel (its 4-bit index, 0 transparent) and color;
            public int Pixel(int tile, int x, int y) => Tiles[tile * 32 + y * 4 + x / 2] >> (x % 2 * 4) & 15;
            public int Color(int tile, int index) => Colors[(tile < TilePalettes.Length ? TilePalettes[tile] & 15 : 0) * 16 + index] & 0x7FFF;
        }

        /* A converted foreground: KatAM's tiles (4 bits a pixel, tile 0 empty), its 6 palettes, its tilemap (a row of 8-pixel tiles at
         * a time), how far it was reduced (colors merged, tiles merged to fit), and the 4 tilemap entries (top left, top right, bottom
         * left, bottom right) of each block asked for drawn without its front layer (a star block's cell once it's broken: the block
         * is the front layer's); */
        internal sealed record SqueakForeground(byte[] Tiles, ushort[] Palettes, ushort[] Tilemap, int Columns, int Rows, int MergedColors, int MergedTiles,
                                                ushort[][] Bare = null);

        /* SqueakRoomTile(); A room's 8-pixel tile at (tx, ty) as one picture, the layers drawn in the DS's order: the main terrain,
         * the front layer over it (but `bare`), the third over both; 64 colors (BGR555), -1 where none is drawn; */
        static int[] SqueakRoomTile(SqueakMap map, SqueakTileset front, SqueakTileset main, int tx, int ty, bool bare = false) {
            int[] pixels = Enumerable.Repeat(-1, 64).ToArray();
            int quarter = (ty & 1) * 2 + (tx & 1);
            foreach ((int grid, int table, SqueakTileset tiles) in new[] { (4, 2, main), (3, 1, front), (5, 2, main) }) {
                if (tiles == null || bare && grid == 3) continue;
                int block = map.Block(grid, tx / 2, ty / 2);
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

        // SqueakColorDistance(); How far apart two BGR555 colors look (weighted RGB, as the eye sees green most, blue least);
        static int SqueakColorDistance(int a, int b) {
            int r = (a & 31) - (b & 31), g = (a >> 5 & 31) - (b >> 5 & 31), bl = (a >> 10 & 31) - (b >> 10 & 31);
            return 3 * r * r + 4 * g * g + 2 * bl * bl;
        }

        /* SqueakPackPalettes(); The color sets (each tile's colors) packed into at most `palettes` palettes of 15 colors: the biggest
         * sets first, each into the palette it adds fewest colors to; null if they don't fit; */
        static List<HashSet<int>> SqueakPackPalettes(IEnumerable<HashSet<int>> sets, int palettes) {
            List<HashSet<int>> packed = new List<HashSet<int>>();
            foreach (HashSet<int> set in sets.Where(s => s.Count > 0).OrderByDescending(s => s.Count)) {
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

        /* ConvertSqueakForeground(); A room's three layers as KatAM's one foreground: each 8-pixel tile drawn whole (SqueakRoomTile()),
         * the same pictures (flipped too) one tile. Its colors go into KatAM's 6 palettes of 15 (SqueakPackPalettes()); where they
         * don't fit, the two closest colors become one (the less used takes the other's) until they do. Past KatAM's 768 tiles (three
         * rooms, the busiest), the two most alike tiles of a palette become one (the less used takes the other's place) until they
         * fit (the owner allows compressing tilesets); */
        static SqueakForeground ConvertSqueakForeground(SqueakMap map, SqueakTileset front, SqueakTileset main, int maxTiles = ForegroundTiles,
                                                        IReadOnlyList<(int x, int y)> bare = null) {
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
            int[] cellPicture = new int[columns * rows];
            for (int ty = 0; ty < rows; ty++)
                for (int tx = 0; tx < columns; tx++) cellPicture[ty * columns + tx] = Picture(SqueakRoomTile(map, front, main, tx, ty));
            bare ??= new List<(int, int)>();
            int[] barePicture = bare.SelectMany(b => Enumerable.Range(0, 4).Select(q => Picture(SqueakRoomTile(map, front, main, b.x * 2 + (q & 1), b.y * 2 + q / 2, true)))).ToArray();
            int[] uses = new int[pictures.Count];
            foreach (int p in cellPicture.Concat(barePicture)) uses[p]++;

            // Colors: merged, the closest pair at a time, until every tile's set fits a palette and the sets fit KatAM's 6;
            Dictionary<int, int> colorUses = new Dictionary<int, int>();
            for (int p = 0; p < pictures.Count; p++)
                foreach (int c in pictures[p].Where(c => c >= 0)) colorUses[c] = colorUses.GetValueOrDefault(c) + uses[p];
            Dictionary<int, int> merged = colorUses.Keys.ToDictionary(c => c, c => c);
            int Final(int c) { while (merged[c] != c) c = merged[c]; return c; }
            List<HashSet<int>> Sets() => pictures.Select(p => p.Where(c => c >= 0).Select(Final).ToHashSet()).Distinct(HashSet<int>.CreateSetComparer()).ToList();
            List<HashSet<int>> palettes;
            int mergedColors = 0;
            while ((palettes = SqueakPackPalettes(Sets(), ForegroundPalettes)) == null) {
                List<int> live = colorUses.Keys.Where(c => merged[c] == c).ToList();
                (int a, int b, int d) closest = (-1, -1, int.MaxValue);
                for (int i = 0; i < live.Count; i++)
                    for (int j = i + 1; j < live.Count; j++) {
                        int d = SqueakColorDistance(live[i], live[j]);
                        if (d < closest.d) closest = (live[i], live[j], d);
                    }
                if (closest.a < 0) throw new InvalidOperationException("A Squeak Squad room's colors don't fit KatAM's palettes.");
                (int keep, int drop) = colorUses[closest.a] >= colorUses[closest.b] ? (closest.a, closest.b) : (closest.b, closest.a);
                merged[drop] = keep;
                colorUses[keep] += colorUses[drop];
                mergedColors++;
            }

            // Each picture in KatAM's colors: its palette (the first holding all its colors) and indexes (0 transparent);
            ushort[] paletteColors = new ushort[ForegroundPalettes * 16];
            List<List<int>> paletteLists = palettes.Select(p => p.OrderBy(c => c).ToList()).ToList();
            for (int p = 0; p < paletteLists.Count; p++)
                for (int c = 0; c < paletteLists[p].Count; c++) paletteColors[p * 16 + 1 + c] = (ushort)paletteLists[p][c];
            (int palette, byte[] indexes) Indexed(int[] pixels) {
                HashSet<int> colors = pixels.Where(c => c >= 0).Select(Final).ToHashSet();
                int palette = Math.Max(0, palettes.FindIndex(p => colors.IsSubsetOf(p)));
                byte[] indexes = pixels.Select(c => c < 0 ? (byte)0 : (byte)(paletteLists[palette].IndexOf(Final(c)) + 1)).ToArray();
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
                (int palette, byte[] indexes) = Indexed(pictures[p]);
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
            int mergedTiles = 0, liveTiles = tiles.Count;
            if (liveTiles > maxTiles) {
                int Difference(byte[] a, byte[] b, int palette) {
                    int sum = 0;
                    for (int i = 0; i < 64; i++) {
                        if (a[i] == b[i]) continue;
                        if (a[i] == 0 || b[i] == 0) { sum += 3000; continue; }
                        sum += SqueakColorDistance(paletteColors[palette * 16 + a[i]], paletteColors[palette * 16 + b[i]]);
                    }
                    return sum;
                }
                while (liveTiles > maxTiles) {
                    (int a, int b, int d) best = (-1, -1, int.MaxValue);
                    List<int> live = Enumerable.Range(1, tiles.Count - 1).Where(t => tileTarget[t] == t).ToList();
                    foreach (IGrouping<int, int> group in live.GroupBy(t => tiles[t].palette))
                        foreach (int i in group)
                            foreach (int j in group)
                                if (i < j) {
                                    int d = Difference(tiles[i].indexes, tiles[j].indexes, group.Key);
                                    if (d < best.d) best = (i, j, d);
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

            byte[] tileData = new byte[kept.Count * 32];
            for (int t = 0; t < kept.Count; t++)
                for (int i = 0; i < 64; i += 2) tileData[t * 32 + i / 2] = (byte)(kept[t][i] | kept[t][i + 1] << 4);
            ushort Entry(int picture) {
                (int tile, bool h, bool v) = pictureTile[picture];
                int target = Target(tile);
                return (ushort)(number[target] | (h ? 0x400 : 0) | (v ? 0x800 : 0) | tiles[target].palette << 12);
            }
            ushort[] tilemap = cellPicture.Select(Entry).ToArray();
            ushort[][] bareEntries = Enumerable.Range(0, bare.Count).Select(b => Enumerable.Range(0, 4).Select(q => Entry(barePicture[b * 4 + q])).ToArray()).ToArray();
            return new SqueakForeground(tileData, paletteColors, tilemap, columns, rows, mergedColors, mergedTiles, bareEntries);
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
         * colors averaged over the pixels each covers: a 512-pixel view at 3/4 is 48 tiles wide), at least `minHeight` tall (the last
         * row repeated, as the DS's smeared edge: KatAM's camera shows 160 rows); the tiles grouped by their colors into 8 palettes,
         * each palette the 15 colors that draw its tiles best (k-means), each tile then drawn with the palette that draws it best, a
         * few rounds; past 900 tiles, the two most alike made one (as BuildBackground()); */
        static SqueakBackground ConvertSqueakFarView(int[,] picture, double scale = 0.75, int minHeight = 160, int maxTiles = BackgroundTiles) {
            int sourceWidth = picture.GetLength(0), sourceHeight = picture.GetLength(1);
            int width = (int)Math.Round(sourceWidth * scale / 8) * 8, height = Math.Max((int)Math.Round(sourceHeight * scale / 8) * 8, (minHeight + 7) / 8 * 8);
            int shrunkHeight = Math.Min(height, (int)Math.Round(sourceHeight * scale));
            // (each pixel the average of the source pixels it covers, weighted by how much of each);
            double[,,] image = new double[width, height, 3];
            double step = 1 / scale;
            for (int y = 0; y < height; y++)
                for (int x = 0; x < width; x++) {
                    double y0 = Math.Min(y, shrunkHeight - 1) * step, x0 = x * step, r = 0, g = 0, b = 0, total = 0;
                    for (int sy = (int)y0; sy < Math.Min(sourceHeight, (int)Math.Ceiling(y0 + step)); sy++)
                        for (int sx = (int)x0; sx < Math.Min(sourceWidth, (int)Math.Ceiling(x0 + step)); sx++) {
                            double weight = (Math.Min(sx + 1, x0 + step) - Math.Max(sx, x0)) * (Math.Min(sy + 1, y0 + step) - Math.Max(sy, y0));
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
                for (int round = 0; round < 8; round++) {
                    double[][] sums = new double[count][];
                    int[] counts = new int[count];
                    for (int k = 0; k < count; k++) sums[k] = new double[3];
                    foreach (double[] p in points) {
                        int best = 0;
                        double bestDistance = double.MaxValue;
                        for (int k = 0; k < count; k++) {
                            double d = Distance(p[0], p[1], p[2], centers[k][0], centers[k][1], centers[k][2]);
                            if (d < bestDistance) { bestDistance = d; best = k; }
                        }
                        counts[best]++;
                        for (int c = 0; c < 3; c++) sums[best][c] += p[c];
                    }
                    for (int k = 0; k < count; k++)
                        if (counts[k] > 0) centers[k] = sums[k].Select(s => s / counts[k]).ToArray();
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
                for (int t = 0; t < tileCount; t++) group[t] = Enumerable.Range(0, Palettes).OrderBy(k => FeatureDistance(features[t], groupCenters[k])).First();
                for (int k = 0; k < Palettes; k++) {
                    double[][] members = Enumerable.Range(0, tileCount).Where(t => group[t] == k).Select(t => features[t]).ToArray();
                    if (members.Length > 0) groupCenters[k] = Enumerable.Range(0, 6).Select(c => members.Average(m => m[c])).ToArray();
                }
            }
            double[][][] palettes = new double[Palettes][][];
            double TileError(int t, double[][] palette) {
                double sum = 0;
                foreach (double[] p in tilePixels[t]) sum += palette.Min(c => Distance(p[0], p[1], p[2], c[0], c[1], c[2]));
                return sum;
            }
            for (int round = 0; round < 8; round++) {
                for (int k = 0; k < Palettes; k++) {
                    List<double[]> points = Enumerable.Range(0, tileCount).Where(t => group[t] == k).SelectMany(t => tilePixels[t]).ToList();
                    palettes[k] = Centers(points, Colors, round == 0 ? null : palettes[k]);
                }
                for (int t = 0; t < tileCount; t++)
                    group[t] = Enumerable.Range(0, Palettes).OrderBy(k => TileError(t, palettes[k])).First();
            }

            // KatAM's colors (BGR555), each tile's indexes, the same tiles (flipped too) one;
            ushort[] paletteColors = new ushort[Palettes * 16];
            for (int k = 0; k < Palettes; k++)
                for (int c = 0; c < Colors; c++) {
                    int r = Math.Clamp((int)Math.Round(palettes[k][c][0]), 0, 31), g = Math.Clamp((int)Math.Round(palettes[k][c][1]), 0, 31),
                        b = Math.Clamp((int)Math.Round(palettes[k][c][2]), 0, 31);
                    paletteColors[k * 16 + 1 + c] = (ushort)(r | g << 5 | b << 10);
                }
            byte[] Indexes(int t) => tilePixels[t].Select(p => (byte)(1 + Enumerable.Range(0, Colors).OrderBy(c =>
                Distance(p[0], p[1], p[2], paletteColors[group[t] * 16 + 1 + c] & 31, paletteColors[group[t] * 16 + 1 + c] >> 5 & 31, paletteColors[group[t] * 16 + 1 + c] >> 10 & 31)).First())).ToArray();
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

            // Past KatAM's 900: the two most alike of a palette made one, the less used taking the other's place;
            int[] target = Enumerable.Range(0, tiles.Count).ToArray();
            int Target(int t) { while (target[t] != t) t = target[t]; return t; }
            int live = tiles.Count, merged = 0;
            while (live > maxTiles) {
                (int a, int b, double d) best = (-1, -1, double.MaxValue);
                List<int> alive = Enumerable.Range(0, tiles.Count).Where(t => target[t] == t).ToList();
                foreach (IGrouping<int, int> sameGroup in alive.GroupBy(t => tiles[t].palette)) {
                    List<int> list = sameGroup.ToList();
                    for (int i = 0; i < list.Count; i++)
                        for (int j = i + 1; j < list.Count; j++) {
                            byte[] a = tiles[list[i]].indexes, b = tiles[list[j]].indexes;
                            double d = 0;
                            for (int p = 0; p < 64 && d < best.d; p++) {
                                int ca = paletteColors[sameGroup.Key * 16 + a[p]], cb = paletteColors[sameGroup.Key * 16 + b[p]];
                                d += Distance(ca & 31, ca >> 5 & 31, ca >> 10 & 31, cb & 31, cb >> 5 & 31, cb >> 10 & 31) * Math.Min(uses[list[i]], uses[list[j]]);
                            }
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
                   KatAMSpikes = 0x1C, KatAMWater = 0x48, KatAMStarBlock = 0xF0;

        /* SqueakCellWord(); A cell's collision as Squeak Squad reads it (0x0220FDEC): the main terrain's grid (section 7), else, where
         * that cell is empty, the front layer's (section 6); */
        static uint SqueakCellWord(SqueakMap map, int x, int y) {
            uint main = map.Collision(7, x, y);
            return main != 0 ? main : map.Collision(6, x, y);
        }

        /* SqueakCollision(); KatAM's collision for a Squeak Squad cell (see the notes' collision word): its kind (bits 14-15: air, solid,
         * thin, slope), its flags (bits 8-13: 0x100 a ladder, on a thin floor its top; 0x200 a thin floor dropped through, 0x300 one
         * that isn't), its block (bits 24-26: 4 a star block, 5 a big one: breakable, KatAMStarBlock, what they leave the main layer's
         * cell; 2 an electric zapper: spikes), the ids on air (0x40-0x4C, without the rope's 0x38 above them: spikes; doors' 0x80 and
         * up: air, the doors are objects) and its liquid (bits 30-31, on air: 1 water, 2 lava, which hurts as spikes do; with block 2,
         * fire jets: air). Ropes and chains (top byte 0x38, ids 0x40-0x46) are climbed as ladders. Currents (ids 0x10-0x3C) aren't
         * moved yet: still air or water; */
        static byte SqueakCollision(uint word) {
            int kind = (int)(word >> 14 & 3), flags = (int)(word >> 8 & 0x3F) << 8, block = (int)(word >> 24 & 7), id = (int)(word >> 16 & 0xFF);
            int liquid = (int)(word >> 30), shape = (int)(word & 0xFF);
            bool rope = (word >> 24) == 0x38 && id is >= 0x40 and <= 0x46;
            switch (kind) {
                case 1:
                    if (block is 4 or 5) return KatAMStarBlock;
                    if (block == 2) return KatAMSpikes;
                    return KatAMSolid;
                case 2:
                    if ((flags & 0x100) != 0 && (flags & 0x200) == 0) return KatAMLadderTop;
                    if (SqueakSlopes.TryGetValue(shape, out var thin)) return thin.thin;
                    return (flags & 0x300) == 0x300 ? KatAMFirmThin : KatAMThin;
                case 3:
                    return SqueakSlopes.TryGetValue(shape, out var slope) ? (liquid == 1 ? slope.water : slope.solid) : KatAMSolid;
            }
            if (rope || (flags & 0x100) != 0) return KatAMLadder;
            if (liquid == 2) return KatAMSpikes;
            if (liquid == 1 && block != 2) return KatAMWater;
            if (id is 0x40 or 0x44 or 0x48 or 0x4C && liquid == 0) return KatAMSpikes;
            return KatAMAir;
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
        // (rooms.c's gNidlRoomTops: the camera from the room's top, 1, down to 8 pixels from its bottom, ROOM_FULL_BOTTOM);
        const byte SqueakCameraTop = 0x81;
        static readonly HashSet<(int area, int stage)> SqueakTestStages = new HashSet<(int, int)> { (1, 8), (1, 9), (2, 9) };

        /* roomTables; The room tables' copies AddRooms() made (where, how many entries, and how many fit: SqueakTableRoom more), null
         * without NiDL's rooms (ForgetRooms()); */
        static Dictionary<string, (int at, int count, int capacity)> roomTables;

        // The Squeak Squad rooms of the last ROM made: their files, room number and size in blocks (for the map viewer and the tests);
        internal sealed record SqueakPortedRoom(SqueakRoomFiles Files, int Room, int Width, int Height);
        static List<SqueakPortedRoom> squeakRooms = new List<SqueakPortedRoom>();
        public static IReadOnlyList<SqueakPortedRoom> SqueakPortedRooms => squeakRooms;

        // SqueakWorldRooms(); The 8 worlds' rooms, stage by stage, in play order;
        static List<SqueakRoomFiles> SqueakWorldRooms(byte[] squeak) {
            List<SqueakRoomFiles> rooms = new List<SqueakRoomFiles>();
            for (int area = 1; area <= 8; area++)
                for (int stage = 0; stage <= 12; stage++)
                    if (!SqueakTestStages.Contains((area, stage))) rooms.AddRange(SqueakStageRooms(squeak, area, stage));
            return rooms;
        }

        /* AddSqueakRooms(); Every Squeak Squad room into KatAM (see above): its foreground (ConvertSqueakForeground(), its star blocks'
         * cells drawn bare for what they leave), its far view as its background (shared by the rooms showing it), its collision
         * (SqueakCollision(): a star block a special block record, leaving air, or water in water), its CPU helpers' zones; no objects
         * nor doors yet; */
        static string AddSqueakRooms(byte[] rom, byte[] squeak, KatAMExpansionCode.Version version, ref int cursor) {
            IReadOnlyDictionary<string, int> symbols = version.RomSymbols;
            int here = cursor;      // (the cursor, which the local functions can't take by reference);
            int roomProps = symbols["gRoomProps"], roomInfos = symbols["gUnk_08D6CD0C"], arrivals = symbols["gUnk_0893CB44"];
            squeakRooms = new List<SqueakPortedRoom>();
            List<SqueakRoomFiles> files = SqueakWorldRooms(squeak);
            if (files.Count == 0) return "The Squeak Squad ROM's stages weren't found.";

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
            int noTypes = Allocate(ref here, 4, 4);
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

            // The room tables, moved and grown;
            int rows = SqueakFirstRoom + files.Count;
            int newProps = Allocate(ref here, rows * 0x28, 4), newInfos = Allocate(ref here, rows * 4, 4), newArrivals = Allocate(ref here, rows * 2, 4);
            Array.Copy(rom, roomProps, rom, newProps, KatAMRoomCount * 0x28);
            Array.Fill(rom, (byte)0xFF, newProps + KatAMRoomCount * 0x28, (rows - KatAMRoomCount) * 0x28);
            Array.Copy(rom, roomInfos, rom, newInfos, KatAMRoomCount * 4);
            for (int room = KatAMRoomCount; room < rows; room++) WriteInt(rom, newInfos + room * 4, ReadInt(rom, roomInfos + 0x397 * 4));
            Array.Copy(rom, arrivals, rom, newArrivals, KatAMRoomCount * 2);
            Array.Fill(rom, (byte)0, newArrivals + KatAMRoomCount * 2, (rows - KatAMRoomCount) * 2);

            Dictionary<string, int> backgroundOf = new Dictionary<string, int>();
            Dictionary<string, (int width, int height)> backgroundSize = new Dictionary<string, (int, int)>();
            try {
                for (int i = 0; i < files.Count; i++) {
                    SqueakRoomFiles f = files[i];
                    int room = SqueakFirstRoom + i;
                    SqueakMap map = new SqueakMap(SqueakUnpack(SqueakFileAnyCase(squeak, f.Map)));
                    SqueakTileset front = f.Front == null || SqueakFileAnyCase(squeak, f.Front) is not byte[] frontFile ? null : new SqueakTileset(frontFile);
                    SqueakTileset main = f.Main == null || SqueakFileAnyCase(squeak, f.Main) is not byte[] mainFile ? null : new SqueakTileset(mainFile);
                    int width = map.Width, height = map.Height;
                    if (width * height > BigRoomBlocks) return $"Squeak Squad's room {f.Area}-{f.Stage}-{f.Index} is bigger than KatAM can hold.";

                    // Collision; the star blocks (and what they leave: air, water in water);
                    byte[] collision = new byte[width * height];
                    List<(int x, int y)> blocks = new List<(int, int)>();
                    List<byte> leaves = new List<byte>();
                    for (int y = 0; y < height; y++)
                        for (int x = 0; x < width; x++) {
                            uint word = SqueakCellWord(map, x, y);
                            byte value = collision[y * width + x] = SqueakCollision(word);
                            if (value != KatAMStarBlock) continue;
                            blocks.Add((x, y));
                            leaves.Add(word >> 30 == 1 ? KatAMWater : KatAMAir);
                        }
                    for (int b = 0; b < blocks.Count; b++) {
                        (int x, int y) = blocks[b];
                        bool Water(int cx, int cy) => cx >= 0 && cy >= 0 && cx < width && cy < height && collision[cy * width + cx] is >= 0x34 and <= 0x48;
                        if (Water(x - 1, y) || Water(x + 1, y) || Water(x, y - 1) || Water(x, y + 1)) leaves[b] = KatAMWater;
                    }

                    // Foreground: its tiles, palettes and tilemap;
                    SqueakForeground foreground = ConvertSqueakForeground(map, front, main, ForegroundTiles, blocks);
                    int tilesAt = AllocateCompressed(rom, CompressLZ77(foreground.Tiles), ref here);
                    int info = Allocate(ref here, 16, 4);
                    WriteInts(rom, info, tilesAt + 0x08000000, 0, 0);
                    WriteShorts(rom, info + 12, 0, room);
                    int colors = Allocate(ref here, 0xC0, 4), palette = Allocate(ref here, 12, 4);
                    for (int c = 0; c < 96; c++) WriteShort(rom, colors + c * 2, foreground.Palettes[c]);
                    WriteInts(rom, palette, colors + 0x08000000, paletteCycles);
                    WriteShorts(rom, palette + 8, 0, room);
                    byte[] tilemap = new byte[foreground.Tilemap.Length * 2];
                    for (int t = 0; t < foreground.Tilemap.Length; t++) WriteShort(tilemap, t * 2, foreground.Tilemap[t]);
                    int tilemapAt = AllocateCompressed(rom, CompressLZ77(tilemap), ref here), tilemapEntry = Allocate(ref here, 0x20, 4);
                    WriteShorts(rom, tilemapEntry, foreground.Columns, foreground.Rows);
                    WriteInts(rom, tilemapEntry + 0x18, tilemapAt + 0x08000000, room << 16);

                    // Background: the far view, shared;
                    string view = f.View ?? "";
                    if (!backgroundOf.TryGetValue(view, out int background)) {
                        byte[] viewFile = f.View == null ? null : SqueakFileAnyCase(squeak, f.View);
                        if (viewFile == null) background = Props(0x3C9, 15);
                        else {
                            SqueakBackground farView = ConvertSqueakFarView(SqueakFarViewPicture(viewFile));
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
                        }
                        backgroundOf[view] = background;
                    }
                    /* (it pans across once as the camera crosses the room, BackgroundRates(), the camera here from the room's top down to 8
                     * pixels from its bottom, 32 lower than KatAM's: its range the room's height - 168; KatAM draws the background's top at
                     * (its height - 160) - unk0C - (the room's height - 160 - the camera's y) * the rate, so unk0C is 8 rates up, keeping
                     * its top row at the screen's top with the camera at the room's top); */
                    (int rateX, int rateY) = backgroundSize.TryGetValue(view, out var bgSize) ? BackgroundRates(bgSize.width, bgSize.height, width * 16, height * 16 + 32, 0) : (0, 0);
                    int backgroundDown = -((8 * rateY + 0xFF) >> 8);

                    // Collision and its special blocks: a star block's, what it leaves (its cell drawn bare, foreground.Bare);
                    List<byte> specials = new List<byte>();
                    for (int b = 0; b < blocks.Count; b++) {
                        specials.AddRange(new byte[] { 1, KatAMStarBlock, (byte)blocks[b].x, (byte)blocks[b].y, 0x12, 0, 0xFF, 0xFF });
                        foreach (ushort e in foreground.Bare[b]) specials.AddRange(BitConverter.GetBytes(e));
                        specials.AddRange(new byte[] { leaves[b], 0xFF });
                    }
                    specials.AddRange(new byte[] { 0, 0, 0, 0, 0, 0, 0xFF, 0xFF });
                    int collisionAt = AllocateCompressed(rom, CompressRL(collision), ref here), specialsAt = AllocateCompressed(rom, specials.ToArray(), ref here);
                    int solidity = Allocate(ref here, 8, 4);
                    WriteInts(rom, solidity, collisionAt + 0x08000000, specialsAt + 0x08000000);

                    int objects = WriteObjectList(rom, new List<byte[]>(), new List<byte>(), noTypes, ref here);
                    int navigation = BuildNavigation(rom, width, height, false, collision, navigationRoutes, new List<(int, int, int, int)>(), ref here);
                    int doors = DoorsList(new List<byte>(), 0);

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
                    WriteShorts(rom, newProps + room * 0x28, LevelSongs[(f.Area - 1) % LevelSongs.Length], 0, 0, 0x100, 0x100, 0, backgroundDown, rateX, rateY, RoomFlags,
                                Add("gForegroundInfo", info), Add("gForegroundPalettes", palette), Add("gForegroundTilemaps", tilemapEntry),
                                Add("gSolidityMaps", solidity), 0xFFFF, background, Add("gLevelObjLists", objects), Add("gUnk_08D63C28", navigation),
                                doorsIndex, 0);
                    squeakRooms.Add(new SqueakPortedRoom(f, room, width, height));
                }
            } catch (Exception exception) when (exception is IndexOutOfRangeException or ArgumentException or InvalidOperationException) {
                return $"The Squeak Squad ROM's rooms couldn't be read ({exception.Message}).";
            }

            // The tables the props index: the new entries in NiDL's copies' room, or new copies of KatAM's;
            foreach (string table in tableNames) {
                List<int> entries = added[table];
                if (nidlRooms) {
                    (int at, int count, int capacity) = roomTables[table];
                    if (count + entries.Count > capacity) return $"Too many Squeak Squad rooms for the room tables ({table}).";
                    WriteInts(rom, at + count * 4, entries.ToArray());
                    roomTables[table] = (at, count + entries.Count, capacity);
                    continue;
                }
                int size = First(table), copy = Allocate(ref here, (size + entries.Count) * 4, 4);
                Array.Copy(rom, symbols[table], rom, copy, size * 4);
                WriteInts(rom, copy + size * 4, entries.ToArray());
                RepointEverywhere(rom, version, symbols[table], copy);
            }
            RepointEverywhere(rom, version, roomProps, newProps);
            RepointEverywhere(rom, version, roomInfos, newInfos);
            RepointEverywhere(rom, version, arrivals, newArrivals);
            // (the map viewer's rooms block names the room info table: its copy now);
            if (nidlRooms) WriteInt(rom, MapRoomsBlock + 4 + 6 * 4, newInfos + 0x08000000);
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
