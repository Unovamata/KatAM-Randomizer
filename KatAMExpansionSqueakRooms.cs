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

        /* SqueakFileAnyCase(); A file by its path, its case ignored: a stage's .mxi names its files in lower case ("map/0d_f.bin"),
         * some of them stored in upper case ("map/0D_f.bin"); */
        static byte[] SqueakFileAnyCase(byte[] rom, string path) {
            byte[] file = SqueakFile(rom, path);
            if (file != null) return file;
            int slash = path.LastIndexOf('/');
            string folder = path.Substring(0, slash + 1), name = path.Substring(slash + 1);
            foreach (string variant in new[] { name.ToUpperInvariant(), CapitalizedPrefix(name) }) {
                file = SqueakFile(rom, folder + variant);
                if (file != null) return file;
            }
            return null;
        }
        // (a tileset's name: its two hex digits in upper case, "0d_f.bin" -> "0D_f.bin");
        static string CapitalizedPrefix(string name) => name.Length > 2 && name[2] == '_' ? name.Substring(0, 2).ToUpperInvariant() + name.Substring(2) : name;

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

        // A converted foreground: KatAM's tiles (4 bits a pixel, tile 0 empty), its 6 palettes, its tilemap (a row of 8-pixel tiles at
        // a time), and how far it was reduced (colors merged, tiles merged to fit);
        internal sealed record SqueakForeground(byte[] Tiles, ushort[] Palettes, ushort[] Tilemap, int Columns, int Rows, int MergedColors, int MergedTiles);

        /* SqueakRoomTile(); A room's 8-pixel tile at (tx, ty) as one picture, the layers drawn in the DS's order: the main terrain,
         * the front layer over it, the third over both; 64 colors (BGR555), -1 where none is drawn; */
        static int[] SqueakRoomTile(SqueakMap map, SqueakTileset front, SqueakTileset main, int tx, int ty) {
            int[] pixels = Enumerable.Repeat(-1, 64).ToArray();
            int quarter = (ty & 1) * 2 + (tx & 1);
            foreach ((int grid, int table, SqueakTileset tiles) in new[] { (4, 2, main), (3, 1, front), (5, 2, main) }) {
                if (tiles == null) continue;
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
        static SqueakForeground ConvertSqueakForeground(SqueakMap map, SqueakTileset front, SqueakTileset main, int maxTiles = ForegroundTiles) {
            int columns = map.Width * 2, rows = map.Height * 2;
            // The pictures, and each tile's picture (flips found later, once the colors are final);
            List<int[]> pictures = new List<int[]>();
            Dictionary<string, int> pictureOf = new Dictionary<string, int>();
            int[] cellPicture = new int[columns * rows];
            for (int ty = 0; ty < rows; ty++)
                for (int tx = 0; tx < columns; tx++) {
                    int[] pixels = SqueakRoomTile(map, front, main, tx, ty);
                    string key = string.Join(",", pixels);
                    if (!pictureOf.TryGetValue(key, out int index)) {
                        pictureOf[key] = index = pictures.Count;
                        pictures.Add(pixels);
                    }
                    cellPicture[ty * columns + tx] = index;
                }
            int[] uses = new int[pictures.Count];
            foreach (int p in cellPicture) uses[p]++;

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
            ushort[] tilemap = new ushort[columns * rows];
            for (int c = 0; c < tilemap.Length; c++) {
                (int tile, bool h, bool v) = pictureTile[cellPicture[c]];
                int target = Target(tile);
                tilemap[c] = (ushort)(number[target] | (h ? 0x400 : 0) | (v ? 0x800 : 0) | tiles[target].palette << 12);
            }
            return new SqueakForeground(tileData, paletteColors, tilemap, columns, rows, mergedColors, mergedTiles);
        }
    }
}
