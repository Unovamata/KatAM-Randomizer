namespace KatAMRandomizer {
    /* KatAMExpansion's NiDL hub (see MainHubDoors): Central Circle's props, collision, tiles and CPU helpers' zones, at night:
     * • Background: NiDL's final boss background (the Fountain of Dreams' first room's: stars, the pink and cyan nebula, the violet
     *   sky and its galaxies), cut down to 32x36 tiles and quantized to 3 palettes (6-8), one per band of rows (HubBackground);
     * • Palettes: Central Circle's scenery (palettes 4 and 5) tinted to the night (NightTint);
     * • Mirrors: its golden one (MainHubEntry) to Vegetable Valley's first room, the way in, a pink one (MainHubReturn, MirrorTwin()) back to Central Circle, and
     *   Central Circle's Dimension Mirror (DimensionMirrorTemplate());
     * • Doors: as Central Circle's, special blocks (kind 1) its hub mirror objects (0x8C) break once their world slot is set, drawing
     *   the door over the 3x3 cells above its door cell (MainHubDoors: Central Circle's own doors' cells). Each level's door is its
     *   design (NidlDesigns: one of KatAM's door frames, anim 0x2C6, over its mirror, anim 0x2C5, as object 0x97 draws them; none of
     *   Central Circle's), drawn over the room's tiles there into new tiles (the ones Central Circle's tilemap doesn't use) with
     *   their colors in a palette of their own: 0-2 (Central Circle's tilemap only uses palette 0's transparent color, its doors
     *   palettes 1 and 2) and 9-13 (the background's palette block carries them); */
    internal static partial class KatAMExpansion {
        // The door palettes, per level;
        static readonly int[] DoorPalettes = { 1, 2, 0, 9, 10, 11, 12, 13 };
        const int DoorFrameAnim = 0x2C6, DoorMirrorAnim = 0x2C5;

        // The final boss background's rows the hub's is made of, and its 3 bands (rows of 8 pixels in it, one palette each);
        static readonly (int from, int to)[] HubBackgroundRows = { (296, 376), (376, 520), (520, 584) };
        static readonly int[] HubBackgroundBands = { 0, 14, 28 };
        const int HubBackgroundPalette = 6;

        static void BuildMainHub(byte[] rom, byte[] nightmare, IReadOnlyDictionary<string, int> symbols, int spriteTables, int hub, List<Door> hubDoors,
                                 int backgroundRecord, int[] tableSizes, List<int> infos, List<int> palettes, List<int> tilemaps, List<int> solidities,
                                 List<int> backgrounds, List<int> objects, int noTypes, ref int cursor) {
            int roomProps = symbols["gRoomProps"], roomInfos = symbols["gUnk_08D6CD0C"];
            int Props(int field) => ReadShort(rom, roomProps + EntryRoom * 0x28 + field * 2);
            int Entry(string table, int index) => ReadInt(rom, symbols[table] + index * 4) - 0x08000000;

            int info = Entry("gForegroundInfo", Props(10)), tilesPointer = ReadInt(rom, info);
            byte[] tiles = DecompressLZ77(rom, tilesPointer - 0x08000000);
            int tilemapEntry = Entry("gForegroundTilemaps", Props(12)), columns = ReadShort(rom, tilemapEntry), width = columns / 2;
            byte[] map = DecompressLZ77(rom, ReadInt(rom, tilemapEntry + 0x18) - 0x08000000);
            int paletteEntry = Entry("gForegroundPalettes", Props(11)), sourceColors = ReadInt(rom, paletteEntry) - 0x08000000;
            int solidity = Entry("gSolidityMaps", Props(13));
            byte[] collision = DecompressRL(rom, ReadInt(rom, solidity) - 0x08000000);

            // Palettes: Central Circle's, its scenery at night;
            int[] colors = new int[96];
            for (int i = 0; i < 96; i++) colors[i] = ReadShort(rom, sourceColors + i * 2) & 0x7FFF;
            for (int i = 4 * 16 + 1; i < 96; i++) if (i % 16 != 0) colors[i] = NightTint(colors[i]);
            // Its pink mirror back to Central Circle (MainHubReturn), as Central Circle's to here (not in the doors' palettes 0-2);
            MirrorTwin(map, columns, tiles, colors, Enumerable.Empty<int>(), CycledColors(rom, paletteEntry), 3);
            int[][] doorColors = new int[8][];
            // A pixel of the room's tilemap (RGB555, or -1: transparent);
            int RoomPixel(int x, int y) {
                int entry = ReadShort(map, ((y / 8) * columns + x / 8) * 2);
                int px = (entry & 0x400) != 0 ? 7 - x % 8 : x % 8, py = (entry & 0x800) != 0 ? 7 - y % 8 : y % 8;
                int index = tiles[(entry & 0x3FF) * 32 + py * 4 + px / 2] >> (px % 2 * 4) & 0xF;
                return index == 0 ? -1 : colors[(entry >> 12) * 16 + index];
            }

            // The doors: their designs drawn over the room's pixels (bottom center on the door cell's), the tiles they touch made anew;
            HashSet<int> usedTiles = Enumerable.Range(0, map.Length / 2).Select(i => ReadShort(map, i * 2) & 0x3FF).ToHashSet();
            Queue<int> freeTiles = new Queue<int>(Enumerable.Range(1, tiles.Length / 32 - 1).Where(t => !usedTiles.Contains(t)));
            byte[] sourceTiles = (byte[])tiles.Clone();
            Dictionary<(int x, int y), int[]> drawn = new Dictionary<(int, int), int[]>();
            (int width, int height, int originX, int originY, int[] pixels) mirror = DecodeSprite(rom, spriteTables, DoorMirrorAnim, 0);
            for (int level = 0; level < MainHubDoors.Length; level++) {
                (int x, int y) door = MainHubDoors[level];
                int left = (door.x - 1) * 16, top = (door.y - 2) * 16, bottomX = door.x * 16 + 8, bottomY = (door.y + 1) * 16;
                int[] area = new int[48 * 48];
                bool[] ofDoor = new bool[48 * 48];
                for (int y = 0; y < 48; y++) for (int x = 0; x < 48; x++) area[y * 48 + x] = RoomPixel(left + x, top + y);
                var frame = DecodeSprite(rom, spriteTables, DoorFrameAnim, NidlDesigns[level]);
                foreach (var sprite in new[] { mirror, frame }) {
                    for (int y = 0; y < sprite.height; y++)
                        for (int x = 0; x < sprite.width; x++) {
                            int color = sprite.pixels[y * sprite.width + x], ax = bottomX - sprite.originX + x - left, ay = bottomY - sprite.originY + y - top;
                            if (color < 0 || ax < 0 || ay < 0 || ax >= 48 || ay >= 48) continue;
                            area[ay * 48 + ax] = color;
                            ofDoor[ay * 48 + ax] = true;
                        }
                }

                // The tiles the door touches: their colors (15 at most), its palette;
                bool Touched(int tx, int ty) => Enumerable.Range(0, 64).Any(i => ofDoor[(ty * 8 + i / 8) * 48 + tx * 8 + i % 8]);
                List<int> doorPixels = new List<int>();
                for (int ty = 0; ty < 6; ty++)
                    for (int tx = 0; tx < 6; tx++)
                        if (Touched(tx, ty)) for (int i = 0; i < 64; i++) if (area[(ty * 8 + i / 8) * 48 + tx * 8 + i % 8] >= 0) doorPixels.Add(area[(ty * 8 + i / 8) * 48 + tx * 8 + i % 8]);
                int[] palette = doorColors[level] = Quantize(doorPixels, 15);
                int[][] entries = new int[9][];
                for (int cell = 0; cell < 9; cell++) entries[cell] = new int[4];
                for (int ty = 0; ty < 6; ty++) {
                    for (int tx = 0; tx < 6; tx++) {
                        int mapX = door.x * 2 - 2 + tx, mapY = door.y * 2 - 4 + ty, entry = ReadShort(map, (mapY * columns + mapX) * 2);
                        if (Touched(tx, ty) && freeTiles.Count > 0) {
                            int tile = freeTiles.Dequeue();
                            for (int i = 0; i < 64; i++) {
                                int color = area[(ty * 8 + i / 8) * 48 + tx * 8 + i % 8];
                                int value = color < 0 ? 0 : 1 + NearestColor(palette, color);
                                if (i % 2 == 0) tiles[tile * 32 + i / 2] = (byte)value; else tiles[tile * 32 + i / 2] |= (byte)(value << 4);
                            }
                            entry = DoorPalettes[level] << 12 | tile;
                        }
                        entries[ty / 2 * 3 + tx / 2][ty % 2 * 2 + tx % 2] = entry;
                    }
                }
                for (int cell = 0; cell < 9; cell++) drawn[(door.x - 1 + cell % 3, door.y - 2 + cell / 3)] = entries[cell];
            }
            for (int level = 0; level < DoorPalettes.Length; level++) {
                if (DoorPalettes[level] >= HubBackgroundPalette) continue;
                for (int i = 0; i < 15; i++) colors[DoorPalettes[level] * 16 + 1 + i] = i < doorColors[level].Length ? doorColors[level][i] : 0;
            }

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
            List<byte> types = templates.Count > 0 ? new List<byte> { HubMirrorType } : new List<byte>();
            byte[] dimensionMirror = DimensionMirrorTemplate(rom, symbols);
            if (dimensionMirror != null) {
                dimensionMirror[3] = (byte)templates.Count;
                templates.Add(dimensionMirror);
                types.Add(ShardType);
            }
            specials.AddRange(new byte[] { 0, 0, 0, 0, 0, 0, 0xFF, 0xFF });
            int tilemapAt = Allocate(ref cursor, 0x20, 4), mapAt = AllocateCompressed(rom, CompressLZ77(hubMap), ref cursor);
            Array.Copy(rom, tilemapEntry, rom, tilemapAt, 0x20);
            WriteInt(rom, tilemapAt + 0x18, mapAt + 0x08000000);
            WriteShort(rom, tilemapAt + 0x1E, hub);
            int collisionAt = AllocateCompressed(rom, CompressRL(collision), ref cursor), specialsAt = AllocateCompressed(rom, specials.ToArray(), ref cursor);
            int solidityAt = Allocate(ref cursor, 8, 4);
            WriteInts(rom, solidityAt, collisionAt + 0x08000000, specialsAt + 0x08000000);
            int objectList = WriteObjectList(rom, templates, types, noTypes, ref cursor);

            int background = HubBackground(rom, nightmare, backgroundRecord, hub, doorColors, ref cursor);

            // Props (Central Circle's, with the new entries), its area info (Central Circle's); the background spans the room once (BackgroundRates());
            int props = roomProps + hub * 0x28, roomHeight = collision.Length / width * 16, backgroundHeight = ReadShort(rom, background + 2) * 8;
            Array.Copy(rom, roomProps + EntryRoom * 0x28, rom, props, 0x28);
            (int rateX, int rateY) = BackgroundRates(ReadShort(rom, background) * 8, backgroundHeight, width * 16, roomHeight);
            WriteShorts(rom, props + 7 * 2, rateX, rateY);
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

        /* DimensionMirrorTemplate(); Central Circle's Dimension Mirror: its mirror shard object (0x65) of subtype 8 (shard.c, CreateShard:
         * the big mirror, whole once the 8 shards are found, and the way to the Dimension Mirror then), copied from its object list, or
         * null if it's not there; */
        const byte ShardType = 0x65;
        static byte[] DimensionMirrorTemplate(byte[] rom, IReadOnlyDictionary<string, int> symbols) {
            int index = ReadShort(rom, symbols["gRoomProps"] + EntryRoom * 0x28 + 16 * 2);
            int list = ReadInt(rom, symbols["gLevelObjLists"] + index * 4) - 0x08000000;
            for (int at = ReadInt(rom, list) - 0x08000000; rom[at] != 0 || rom[at + 1] != 0; at += 36)
                if (rom[at + 12] == ShardType && rom[at + 14] == 8) return rom.Skip(at).Take(36).ToArray();
            return null;
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

        /* DecodeSprite(); A frame of a KatAM anim (gSpriteTables: anims, attrs, oamData, palettes, tiles): its variant's commands load
         * its palette (-2, index) and tiles (-1, index, count) and show a frame (a duration, the frame); the frame's attrs (its first OAM
         * piece, pieces, width, height, origin) and pieces (3 halfwords: y | shape << 14, x | size << 14 | flips, tile). Its pixels: RGB555,
         * -1 where transparent; */
        static (int width, int height, int originX, int originY, int[] pixels) DecodeSprite(byte[] rom, int spriteTables, int anim, int variant) {
            int Table(int i) => ReadInt(rom, spriteTables + i * 4) - 0x08000000;
            int commands = ReadInt(rom, ReadInt(rom, Table(0) + anim * 4) - 0x08000000 + variant * 4) - 0x08000000;
            int palette = 0, tileIndex = 0, frame = 0;
            for (int i = 0; i < 40;) {
                int command = ReadInt(rom, commands + i * 4);
                if (command == -2) { palette = ReadInt(rom, commands + i * 4 + 4); i += 3; }
                else if (command == -1) { tileIndex = ReadInt(rom, commands + i * 4 + 4); i += 3; }
                else if (command >= 0) { frame = ReadInt(rom, commands + i * 4 + 4); break; }
                else i++;
            }
            int attributes = ReadInt(rom, Table(1) + anim * 4) - 0x08000000 + frame * 12, pieces = ReadInt(rom, Table(2) + anim * 4) - 0x08000000;
            int first = (short)ReadShort(rom, attributes), count = (short)ReadShort(rom, attributes + 2), width = ReadShort(rom, attributes + 4),
                height = ReadShort(rom, attributes + 6), originX = (short)ReadShort(rom, attributes + 8), originY = (short)ReadShort(rom, attributes + 10);
            int colors = Table(3) + palette * 32, tileData = Table(4);
            int[] pixels = Enumerable.Repeat(-1, width * height).ToArray();
            for (int p = count - 1; p >= 0; p--) {
                int a0 = ReadShort(rom, pieces + (first + p) * 6), a1 = ReadShort(rom, pieces + (first + p) * 6 + 2), tile = ReadShort(rom, pieces + (first + p) * 6 + 4);
                int y = a0 & 0x3FFF, x = a1 & 0xFFF, shape = a0 >> 14, size = a1 >> 14;
                if (y >= 0x2000) y -= 0x4000;
                if (x >= 0x800) x -= 0x1000;
                int pw = shape == 0 ? 8 << size : shape == 1 ? new[] { 16, 32, 32, 64 }[size] : new[] { 8, 8, 16, 32 }[size];
                int ph = shape == 0 ? 8 << size : shape == 1 ? new[] { 8, 8, 16, 32 }[size] : new[] { 16, 32, 32, 64 }[size];
                for (int py = 0; py < ph; py++)
                    for (int px = 0; px < pw; px++) {
                        int t = tileIndex + tile + py / 8 * (pw / 8) + px / 8;
                        int index = rom[tileData + t * 32 + py % 8 * 4 + px % 8 / 2] >> (px % 2 * 4) & 0xF;
                        int dx = x + ((a1 & 0x1000) != 0 ? pw - 1 - px : px), dy = y + ((a1 & 0x2000) != 0 ? ph - 1 - py : py);
                        if (index != 0 && dx >= 0 && dy >= 0 && dx < width && dy < height) pixels[dy * width + dx] = ReadShort(rom, colors + index * 2) & 0x7FFF;
                    }
            }
            return (width, height, originX, originY, pixels);
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
