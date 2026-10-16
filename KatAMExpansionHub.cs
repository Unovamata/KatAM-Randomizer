namespace KatAMRandomizer {
    /* KatAMExpansion's NiDL hub (see MainHubDoors): Central Circle's props, collision, tiles and CPU helpers' zones, at night:
     * • Background: NiDL's final boss background (the Fountain of Dreams' first room's: stars, the pink and cyan nebula, the violet
     *   sky, the sunset hills), cut down to 32x36 tiles and quantized to 3 palettes (6-8), one per band of rows (HubBackground);
     * • Palettes: Central Circle's scenery (palettes 4 and 5) tinted to the night (NightTint);
     * • Doors: Central Circle's doors are special blocks (kind 1) its hub mirror objects (0x8C) break once their world slot is set,
     *   drawing the door's tiles over the 3x3 cells above its door cell. Each level gets the door at its MainHubDoors cell, its
     *   tiles copied (into the tiles Central Circle's tilemap doesn't use) with their colors in a palette of their own, recolored
     *   for the level (DoorThemes): palettes 1 and 2 (Vegetable Valley's, Ice Cream Island's: Central Circle's doors used them) and
     *   9-13 (the others': the background's palette block carries them). The Fountain of Dreams' starry door keeps its palettes:
     *   its own colors are palette 3's 4-7, which only it uses; */
    internal static partial class KatAMExpansion {
        // The door palettes, per level (the Fountain of Dreams': none, see above);
        static readonly int[] DoorPalettes = { 1, 2, 9, 10, 11, 12, 13, -1 };
        const int FountainLevel = 7, StarryPalette = 3;

        /* DoorThemes; How a level's door is recolored (its glass, white and black, GlassColors, stay; its bits of clouds get the night
         * tint): "rotate" turns the hue of the colors whose hue is between From and To by Degrees, "colorize" gives every color hue
         * Degrees; both scale the saturation and value. Vegetable Valley: carrot trunks; Ice Cream Island: tropical sea; Butter Building:
         * butter; Grape Garden: grape; Yogurt Yard: strawberry yogurt; Orange Ocean: orange; Rainbow Resort: pastel ice; the Fountain
         * of Dreams: violet night; */
        static readonly (bool colorize, double degrees, double saturation, double value, double from, double to)[] DoorThemes = {
            (false, -12, 1.5, 1.05, 15, 50), (false, -25, 1.1, 1.05, 0, 360), (true, 48, 0.45, 1.1, 0, 360), (false, 10, 1.15, 1.0, 0, 360),
            (true, 335, 0.28, 1.12, 0, 360), (false, 180, 1.1, 1.05, 0, 360), (false, 60, 1.0, 1.05, 0, 360), (false, 45, 1.1, 0.9, 180, 260),
        };
        static readonly HashSet<int> GlassColors = new HashSet<int> { 0x7D20, 0x3C00, 0x7E20, 0x7ECA, 0x7F75, 0x7FFF, 0x0000, 0x7CC0 };

        // The final boss background's rows the hub's is made of, and its 3 bands (rows of 8 pixels in it, one palette each);
        static readonly (int from, int to)[] HubBackgroundRows = { (296, 376), (376, 520), (776, 840) };
        static readonly int[] HubBackgroundBands = { 0, 14, 28 };
        const int HubBackgroundPalette = 6;

        static void BuildMainHub(byte[] rom, byte[] nightmare, IReadOnlyDictionary<string, int> symbols, int hub, List<Door> hubDoors, int backgroundRecord,
                                 int[] tableSizes, List<int> infos, List<int> palettes, List<int> tilemaps, List<int> solidities, List<int> backgrounds,
                                 List<int> objects, int noTypes, ref int cursor) {
            int roomProps = symbols["gRoomProps"], roomInfos = symbols["gUnk_08D6CD0C"];
            int Props(int field) => ReadShort(rom, roomProps + EntryRoom * 0x28 + field * 2);
            int Entry(string table, int index) => ReadInt(rom, symbols[table] + index * 4) - 0x08000000;

            int info = Entry("gForegroundInfo", Props(10)), tilesPointer = ReadInt(rom, info);
            byte[] tiles = DecompressLZ77(rom, tilesPointer - 0x08000000), sourceTiles = (byte[])tiles.Clone();
            int tilemapEntry = Entry("gForegroundTilemaps", Props(12)), columns = ReadShort(rom, tilemapEntry), width = columns / 2;
            byte[] map = DecompressLZ77(rom, ReadInt(rom, tilemapEntry + 0x18) - 0x08000000);
            int paletteEntry = Entry("gForegroundPalettes", Props(11)), sourceColors = ReadInt(rom, paletteEntry) - 0x08000000;
            int Color(int palette, int index) => ReadShort(rom, sourceColors + (palette * 16 + index) * 2) & 0x7FFF;
            int solidity = Entry("gSolidityMaps", Props(13));
            byte[] collision = DecompressRL(rom, ReadInt(rom, solidity) - 0x08000000);

            // Central Circle's doors' special blocks (kind 1: their 4 tilemap entries);
            Dictionary<(int x, int y), int[]> blocks = new Dictionary<(int, int), int[]>();
            for (int at = ReadInt(rom, solidity + 4) - 0x08000000; rom[at + 4] != 0; at += rom[at + 4])
                if (rom[at] == 1) blocks[(rom[at + 2], rom[at + 3])] = Enumerable.Range(0, 4).Select(q => ReadShort(rom, at + 8 + q * 2)).ToArray();
            int MapEntry(int x, int y, int q) => ReadShort(map, ((y * 2 + q / 2) * columns + x * 2 + q % 2) * 2);

            // Palettes: Central Circle's, its scenery at night, palettes 1 and 2 for doors, the starry door's colors;
            int[] colors = new int[96];
            for (int i = 0; i < 96; i++) colors[i] = Color(i / 16, i % 16);
            for (int i = 4 * 16 + 1; i < 96; i++) if (i % 16 != 0) colors[i] = NightTint(colors[i]);
            for (int i = 4; i <= 7; i++) colors[StarryPalette * 16 + i] = ThemeColor(FountainLevel, Color(StarryPalette, i));
            int[][] doorColors = new int[8][];

            // The doors' tiles: the tiles of their special blocks that aren't the tilemap's, recolored into their palette;
            // (the doors that keep their tiles keep them too);
            HashSet<int> usedTiles = Enumerable.Range(0, map.Length / 2).Select(i => ReadShort(map, i * 2) & 0x3FF).ToHashSet();
            for (int level = 0; level < MainHubDoors.Length; level++) {
                if (DoorPalettes[level] >= 0) continue;
                (int x, int y) door = MainHubDoors[level];
                for (int y = door.y - 2; y <= door.y; y++)
                    for (int x = door.x - 1; x <= door.x + 1; x++) usedTiles.UnionWith(blocks[(x, y)].Select(e => e & 0x3FF));
            }
            Queue<int> freeTiles = new Queue<int>(Enumerable.Range(1, tiles.Length / 32 - 1).Where(t => !usedTiles.Contains(t)));
            Dictionary<(int x, int y), int[]> drawn = new Dictionary<(int, int), int[]>();
            for (int level = 0; level < MainHubDoors.Length; level++) {
                (int x, int y) door = MainHubDoors[level];
                List<int> sources = new List<int>();
                for (int y = door.y - 2; y <= door.y; y++)
                    for (int x = door.x - 1; x <= door.x + 1; x++)
                        for (int q = 0; q < 4; q++)
                            if (blocks[(x, y)][q] != MapEntry(x, y, q) && !sources.Contains(blocks[(x, y)][q] & 0xF3FF)) sources.Add(blocks[(x, y)][q] & 0xF3FF);

                Dictionary<int, int> newEntries = new Dictionary<int, int>();
                if (DoorPalettes[level] >= 0 && freeTiles.Count >= sources.Count) {
                    // The door's colors, recolored (the most common 15 if more), and its tiles remapped to them;
                    int Recolored(int palette, int index) {
                        int color = Color(palette, index);
                        return palette >= 4 ? NightTint(color) : GlassColors.Contains(color) ? color : ThemeColor(level, color);
                    }
                    Dictionary<int, int> counts = new Dictionary<int, int>();
                    foreach (int source in sources)
                        for (int b = 0; b < 64; b++) {
                            int index = sourceTiles[(source & 0x3FF) * 32 + b / 2] >> (b % 2 * 4) & 0xF;
                            if (index != 0) counts[Recolored(source >> 12, index)] = counts.GetValueOrDefault(Recolored(source >> 12, index)) + 1;
                        }
                    int[] palette = counts.OrderByDescending(c => c.Value).ThenBy(c => c.Key).Take(15).Select(c => c.Key).ToArray();
                    doorColors[level] = palette;
                    foreach (int source in sources) {
                        int tile = freeTiles.Dequeue();
                        for (int b = 0; b < 32; b++) {
                            int pixels = sourceTiles[(source & 0x3FF) * 32 + b], value = 0;
                            for (int half = 0; half < 2; half++) {
                                int index = pixels >> (half * 4) & 0xF;
                                if (index != 0) value |= (1 + NearestColor(palette, Recolored(source >> 12, index))) << (half * 4);
                            }
                            tiles[tile * 32 + b] = (byte)value;
                        }
                        newEntries[source] = DoorPalettes[level] << 12 | tile;
                    }
                }
                for (int y = door.y - 2; y <= door.y; y++)
                    for (int x = door.x - 1; x <= door.x + 1; x++)
                        drawn[(x, y)] = Enumerable.Range(0, 4).Select(q => {
                            int entry = blocks[(x, y)][q];
                            if (entry == MapEntry(x, y, q)) return entry;
                            return newEntries.TryGetValue(entry & 0xF3FF, out int recolored) ? recolored | (entry & 0x0C00) : entry;
                        }).ToArray();
            }
            for (int level = 0; level < 2; level++)
                for (int i = 0; i < 15; i++) colors[DoorPalettes[level] * 16 + 1 + i] = doorColors[level] != null && i < doorColors[level].Length ? doorColors[level][i] : 0;

            int infoAt = Allocate(ref cursor, 16, 4), tilesAt = AllocateCompressed(rom, CompressLZ77(tiles), ref cursor);
            Array.Copy(rom, info, rom, infoAt, 16);
            for (int i = 0; i < 3; i++) if (ReadInt(rom, infoAt + i * 4) == tilesPointer) WriteInt(rom, infoAt + i * 4, tilesAt + 0x08000000);
            WriteShort(rom, infoAt + 14, hub);
            int colorsAt = Allocate(ref cursor, 0xC0, 4), paletteAt = Allocate(ref cursor, 12, 4);
            for (int i = 0; i < 96; i++) WriteShort(rom, colorsAt + i * 2, colors[i]);
            Array.Copy(rom, paletteEntry, rom, paletteAt, 12);
            WriteInt(rom, paletteAt, colorsAt + 0x08000000);
            WriteShort(rom, paletteAt + 10, hub);

            // Tilemap, with the door that's open from the start; collision and special blocks: the doors, and the hidden ones' blocks;
            byte[] hubMap = (byte[])map.Clone();
            void Draw(byte[] target, (int x, int y) door) {
                for (int y = door.y - 2; y <= door.y; y++)
                    for (int x = door.x - 1; x <= door.x + 1; x++)
                        for (int q = 0; q < 4; q++) WriteShort(target, ((y * 2 + q / 2) * columns + x * 2 + q % 2) * 2, drawn[(x, y)][q]);
            }
            List<byte> specials = new List<byte>();
            List<byte[]> templates = new List<byte[]>();
            foreach (Door door in hubDoors) {
                if (door.Slot == 0) collision[door.Y * width + door.X] = 0xFF;
                if (door.Slot == 0 && Array.IndexOf(MainHubDoors, (door.X, door.Y)) >= 0) Draw(hubMap, (door.X, door.Y));
                specials.AddRange(DoorRecord(door, 0xFF));
                if (door.Slot == 0) continue;

                for (int y = door.Y - 2; y <= door.Y; y++) {
                    for (int x = door.X - 1; x <= door.X + 1; x++) {
                        byte here = collision[y * width + x];
                        specials.AddRange(new byte[] { 1, here, (byte)x, (byte)y, 0x12, 0, 0xFF, 0xFF });
                        foreach (int entry in drawn[(x, y)]) specials.AddRange(BitConverter.GetBytes((ushort)entry));
                        specials.AddRange(new byte[] { x == door.X && y == door.Y ? (byte)0xFF : here, 0xFF });
                    }
                }
                templates.Add(HubMirrorTemplate(door.X, door.Y - 1, templates.Count, door.Slot));
            }
            specials.AddRange(new byte[] { 0, 0, 0, 0, 0, 0, 0xFF, 0xFF });
            int tilemapAt = Allocate(ref cursor, 0x20, 4), mapAt = AllocateCompressed(rom, CompressLZ77(hubMap), ref cursor);
            Array.Copy(rom, tilemapEntry, rom, tilemapAt, 0x20);
            WriteInt(rom, tilemapAt + 0x18, mapAt + 0x08000000);
            WriteShort(rom, tilemapAt + 0x1E, hub);
            int collisionAt = AllocateCompressed(rom, CompressRL(collision), ref cursor), specialsAt = AllocateCompressed(rom, specials.ToArray(), ref cursor);
            int solidityAt = Allocate(ref cursor, 8, 4);
            WriteInts(rom, solidityAt, collisionAt + 0x08000000, specialsAt + 0x08000000);
            int objectList = WriteObjectList(rom, templates, templates.Count > 0 ? new List<byte> { HubMirrorType } : new List<byte>(), noTypes, ref cursor);

            int background = HubBackground(rom, nightmare, backgroundRecord, hub, doorColors, ref cursor);

            /* Props (Central Circle's, with the new entries), its area info (Central Circle's). The background's scroll rates (8.8: unk0E,
             * unk10): KatAM aligns its bottom with the room's (minus the 40 pixels it hides) and scrolls it by the rate (level.c,
             * sub_080019F8), so its top shows when the camera is at the room's top with this rate; */
            int props = roomProps + hub * 0x28, roomHeight = collision.Length / width * 16, backgroundHeight = ReadShort(rom, background + 2) * 8;
            Array.Copy(rom, roomProps + EntryRoom * 0x28, rom, props, 0x28);
            WriteShorts(rom, props + 7 * 2, 0x80, Math.Max(0, (backgroundHeight - 160) * 0x100 / (roomHeight - 160)));
            WriteShorts(rom, props + 10 * 2, tableSizes[10] + infos.Count, tableSizes[11] + palettes.Count, tableSizes[12] + tilemaps.Count, tableSizes[13] + solidities.Count);
            WriteShort(rom, props + 15 * 2, tableSizes[15] + backgrounds.Count);
            WriteShort(rom, props + 16 * 2, tableSizes[16] + objects.Count);
            WriteShort(rom, props + 18 * 2, SharedDoors);
            infos.Add(infoAt + 0x08000000);
            palettes.Add(paletteAt + 0x08000000);
            tilemaps.Add(tilemapAt + 0x08000000);
            solidities.Add(solidityAt + 0x08000000);
            backgrounds.Add(background + 0x08000000);
            objects.Add(objectList + 0x08000000);
            int roomInfo = Allocate(ref cursor, 0x4C, 4);
            Array.Copy(rom, ReadInt(rom, roomInfos + EntryRoom * 4) - 0x08000000, rom, roomInfo, 0x4C);
            WriteShort(rom, roomInfo + 0x44, hub);
            WriteInt(rom, roomInfos + hub * 4, roomInfo + 0x08000000);
        }

        /* HubBackground(); The hub's RoomTiledBG (see above): NiDL's background drawn (its palettes 8-15, colors from its palette data;
         * a pixel 0 is the palette's first color), the rows of HubBackgroundRows stacked, each band's colors reduced to 15 (Quantize),
         * its tiles deduplicated; its palettes 6-8, then the doors' 9-13; returns its address; */
        static int HubBackground(byte[] rom, byte[] nightmare, int record, int room, int[][] doorColors, ref int cursor) {
            int Address(int offset) => ReadInt(nightmare, record + offset) - 0x08000000;
            int paletteData = Address(0x28), map = Address(0x30);
            byte[] sourceTiles = DecompressLZ77(nightmare, Address(0x2C));
            int sourceWidth = ReadShort(nightmare, map + 2), sourceHeight = ReadShort(nightmare, map + 4);
            bool compressed = ReadShort(nightmare, map + 6) == 0xFFFF;
            byte[] entries = compressed ? DecompressLZ77(nightmare, map + 8) : null;
            int colorCount = ReadShort(nightmare, paletteData) / 2;
            int SourcePixel(int x, int y) {
                int tx = x / 8, ty = y / 8;
                int entry = compressed ? ReadShort(entries, ((tx >> 5) * 1024 + ty * 32 + (tx & 31)) * 2) : ReadShort(nightmare, map + 6 + (ty * sourceWidth + tx) * 2);
                int px = (entry & 0x400) != 0 ? 7 - x % 8 : x % 8, py = (entry & 0x800) != 0 ? 7 - y % 8 : y % 8;
                int offset = (entry & 0x3FF) * 32 + py * 4 + px / 2;
                int index = offset < sourceTiles.Length ? sourceTiles[offset] >> (px % 2 * 4) & 0xF : 0;
                int color = index == 0 ? 0 : ((entry >> 12) - 8) * 16 + index;
                return color >= 0 && color < colorCount ? ReadShort(nightmare, paletteData + 2 + color * 2) & 0x7FFF : 0;
            }

            const int width = 32;
            List<int> rows = HubBackgroundRows.SelectMany(r => Enumerable.Range(r.from, r.to - r.from)).Where(y => y < sourceHeight * 8).ToList();
            int height = rows.Count / 8;
            int[,] pixels = new int[width * 8, height * 8];
            for (int y = 0; y < height * 8; y++)
                for (int x = 0; x < width * 8; x++) pixels[x, y] = SourcePixel(x % (sourceWidth * 8), rows[y]);

            int Band(int tileRow) => HubBackgroundBands.Count(b => b <= tileRow) - 1;
            int[][] bandColors = new int[HubBackgroundBands.Length][];
            for (int band = 0; band < bandColors.Length; band++) {
                int top = HubBackgroundBands[band] * 8, bottom = band + 1 < HubBackgroundBands.Length ? HubBackgroundBands[band + 1] * 8 : height * 8;
                List<int> bandPixels = new List<int>();
                for (int y = top; y < bottom; y++) for (int x = 0; x < width * 8; x++) bandPixels.Add(pixels[x, y]);
                bandColors[band] = Quantize(bandPixels, 15);
            }

            List<byte> tiles = new List<byte>();
            Dictionary<string, int> tileNumbers = new Dictionary<string, int>();
            byte[] mapEntries = new byte[width * height * 2];
            for (int ty = 0; ty < height; ty++) {
                int band = Band(ty);
                for (int tx = 0; tx < width; tx++) {
                    byte[] tile = new byte[32];
                    for (int y = 0; y < 8; y++)
                        for (int x = 0; x < 8; x++) tile[y * 4 + x / 2] |= (byte)((1 + NearestColor(bandColors[band], pixels[tx * 8 + x, ty * 8 + y])) << (x % 2 * 4));
                    string key = Convert.ToHexString(tile);
                    if (!tileNumbers.TryGetValue(key, out int number)) {
                        number = tileNumbers.Count < BackgroundTiles ? tileNumbers.Count : BackgroundTiles - 1;
                        if (tileNumbers.Count < BackgroundTiles) tiles.AddRange(tile);
                        tileNumbers[key] = number;
                    }
                    WriteShort(mapEntries, (ty * width + tx) * 2, (HubBackgroundPalette + band) << 12 | number);
                }
            }

            int[] palette = new int[0x80];
            for (int band = 0; band < bandColors.Length; band++)
                for (int i = 0; i < bandColors[band].Length; i++) palette[band * 16 + 1 + i] = bandColors[band][i];
            for (int level = 0; level < DoorPalettes.Length; level++) {
                if (DoorPalettes[level] < HubBackgroundPalette || doorColors[level] == null) continue;
                for (int i = 0; i < doorColors[level].Length; i++) palette[(DoorPalettes[level] - HubBackgroundPalette) * 16 + 1 + i] = doorColors[level][i];
            }

            int tilesAt = AllocateCompressed(rom, CompressLZ77(tiles.ToArray()), ref cursor), colorsAt = Allocate(ref cursor, 0x100, 4);
            for (int i = 0; i < 0x80; i++) WriteShort(rom, colorsAt + i * 2, palette[i]);
            int mapAt = AllocateCompressed(rom, mapEntries, ref cursor);
            int at = Allocate(ref cursor, 0x20, 4);
            WriteShorts(rom, at, width, height);
            WriteInts(rom, at + 8, tilesAt + 0x08000000, tiles.Count / 32, colorsAt + 0x08000000);
            WriteShorts(rom, at + 0x14, 0x60, 0x80);
            WriteInts(rom, at + 0x18, mapAt + 0x08000000, room << 16);
            return at;
        }

        /* Quantize(); A palette of at most `count` colors for the pixels (RGB555): the most common color, then each time the color
         * farthest from those picked (weighted by how common it is), refined by k-means; */
        static int[] Quantize(List<int> pixels, int count) {
            Dictionary<int, int> histogram = pixels.GroupBy(p => p).ToDictionary(g => g.Key, g => g.Count());
            List<int> distinct = histogram.Keys.OrderBy(c => c).ToList();
            if (distinct.Count <= count) return distinct.ToArray();
            List<(double r, double g, double b)> centers = new List<(double, double, double)> { Rgb(histogram.MaxBy(h => h.Value).Key) };
            while (centers.Count < count) {
                int farthest = distinct.MaxBy(c => centers.Min(k => Distance(Rgb(c), k)) * Math.Sqrt(histogram[c]));
                centers.Add(Rgb(farthest));
            }
            for (int iteration = 0; iteration < 10; iteration++) {
                (double r, double g, double b, double n)[] sums = new (double, double, double, double)[count];
                foreach (int c in distinct) {
                    (double r, double g, double b) rgb = Rgb(c);
                    int k = Enumerable.Range(0, count).MinBy(i => Distance(rgb, centers[i]));
                    double n = histogram[c];
                    sums[k] = (sums[k].r + rgb.r * n, sums[k].g + rgb.g * n, sums[k].b + rgb.b * n, sums[k].n + n);
                }
                for (int k = 0; k < count; k++) if (sums[k].n > 0) centers[k] = (sums[k].r / sums[k].n, sums[k].g / sums[k].n, sums[k].b / sums[k].n);
            }
            return centers.Select(k => (int)Math.Round(k.r) | (int)Math.Round(k.g) << 5 | (int)Math.Round(k.b) << 10).Distinct().ToArray();
        }

        static (double r, double g, double b) Rgb(int color) => (color & 31, color >> 5 & 31, color >> 10 & 31);
        static double Distance((double r, double g, double b) a, (double r, double g, double b) b) =>
            (a.r - b.r) * (a.r - b.r) * 0.3 + (a.g - b.g) * (a.g - b.g) * 0.59 + (a.b - b.b) * (a.b - b.b) * 0.11;
        static int NearestColor(int[] palette, int color) => Enumerable.Range(0, palette.Length).MinBy(i => Distance(Rgb(palette[i]), Rgb(color)));

        // NightTint(); A scenery color at night: its hue halfway to violet, more saturated, darker;
        static int NightTint(int color) {
            (double h, double s, double v) = ToHsv(color);
            double toward = 270, delta = ((toward - h + 540) % 360) - 180;
            return FromHsv((h + delta * 0.55 + 360) % 360, Math.Min(1, s * 0.8 + 0.25), v * 0.75 + 0.05);
        }

        // ThemeColor(); A door color recolored for the level (see DoorThemes);
        static int ThemeColor(int level, int color) {
            if (GlassColors.Contains(color)) return color;
            var theme = DoorThemes[level];
            (double h, double s, double v) = ToHsv(color);
            if (theme.colorize) { h = theme.degrees; s = s > 0.08 ? Math.Max(s, theme.saturation) : theme.saturation * 0.7; }
            else if (theme.from <= theme.to ? h >= theme.from && h <= theme.to : h >= theme.from || h <= theme.to) {
                h = (h + theme.degrees + 360) % 360;
                s = Math.Min(1, s * theme.saturation);
            }
            return FromHsv(h, s, Math.Min(1, v * theme.value));
        }

        static (double h, double s, double v) ToHsv(int color) {
            double r = (color & 31) / 31.0, g = (color >> 5 & 31) / 31.0, b = (color >> 10 & 31) / 31.0;
            double max = Math.Max(r, Math.Max(g, b)), min = Math.Min(r, Math.Min(g, b)), d = max - min, h = 0;
            if (d > 0) h = max == r ? 60 * (((g - b) / d + 6) % 6) : max == g ? 60 * ((b - r) / d + 2) : 60 * ((r - g) / d + 4);
            return (h, max == 0 ? 0 : d / max, max);
        }

        static int FromHsv(double h, double s, double v) {
            double c = v * s, x = c * (1 - Math.Abs(h / 60 % 2 - 1)), m = v - c;
            (double r, double g, double b) = h < 60 ? (c, x, 0.0) : h < 120 ? (x, c, 0.0) : h < 180 ? (0.0, c, x) : h < 240 ? (0.0, x, c) : h < 300 ? (x, 0.0, c) : (c, 0.0, x);
            int Channel(double value) => Math.Clamp((int)Math.Round((value + m) * 31), 0, 31);
            return Channel(r) | Channel(g) << 5 | Channel(b) << 10;
        }
    }
}
