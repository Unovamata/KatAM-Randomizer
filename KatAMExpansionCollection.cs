using System;
using System.Collections.Generic;
using System.Linq;

namespace KatAMRandomizer {
    /* KatAMExpansionCollection; The Collection Room as one grid that scrolls sideways ("ROM Expansion/collection.c" has the design):
     * KatAM's 10 x 4 cells as KatAM has them, then columns of the expansion's treasures (CollectionItem). Here gCollectionGrid (the
     * items, their composed texts), the lattice on BG2 (the room's image's: its first column, its middle ones repeated, its last), the
     * image's grid area without it (the stripes behind it, which follow a pattern: StripeAt), and the room's hooks; */
    internal static partial class KatAMExpansion {
        /* A cell after KatAM's columns (collection.c's GridItem): its row, its column (0: the first after KatAM's), its id (category
         * << 8 | number), its icon (anim, variant, OBJ palette), its composed text (an index in the texts given, -1: none) and the
         * room's text put up under it (gUnk_08386664's index); */
        internal record CollectionItem(int Row, int Column, int Id, int Anim, int Variant, int Palette, int Text, int RoomText);

        // (collection.c's CollectionGrid: its fields, its items' table; GridRoom's entries, after one entry);
        const int GridKatamAt = 0, GridExtraAt = 4, GridTextCountAt = 5, GridTextsAt = 8, GridLatticeTilesAt = 0xC, GridLatticeMapAt = 0x10,
                  GridImageTilesAt = 0x14, GridImageMapAt = 0x2C, GridItemsAt = 0x44, GridItemSize = 8, GridExtraMax = 13;
        const int GridRows = 4, GridShown = 10, GridRoomEntries = 4;
        // (the grid's box on the screen and its cells' width; BG2's map: tile rows 3-13, 64 tiles; BG0's grid area: tile columns 1-28, its tiles from 0x180);
        const int GridLeft = 12, GridRight = 228, GridTop = 26, GridBottom = 107, CellWidth = 22, CellHeight = 21;
        const int GridMapTop = 3, GridMapRows = 11, LatticeWidth = 64, GridImageLeft = 1, GridImageColumns = 28, GridImageTile = 0x180, GridImageTilesMax = 0x80;
        const int CollectionImageWidth = 30, CollectionImageHeight = 20;
        // (the lattice's frames' color: a cell's left side);
        const byte LatticeFrame = 12;

        /* TiledBgPixels(); A TiledBg (gUnk_082D7850's) of a size as pixels [y, x], its tilemap and tile count; null if it isn't one
         * of that size, or (unless blank ones are wanted) has no pixels; */
        static byte[,] TiledBgPixels(byte[] rom, int bgTable, int index, int width, int height, bool blank, out ushort[] map, out int tileCount) {
            map = null;
            tileCount = 0;
            int bg = ReadInt(rom, bgTable + index * 4) - 0x08000000;
            if (bg < 0 || bg + 0x1C > rom.Length || ReadShort(rom, bg) != width || ReadShort(rom, bg + 2) != height) return null;
            int tileset = ReadInt(rom, bg + 8) - 0x08000000, tilemap = ReadInt(rom, bg + 0x18) - 0x08000000;
            if (tileset < 0 || tileset >= rom.Length || tilemap < 0 || tilemap + width * height * 2 > rom.Length) return null;
            byte[] tiles = DecompressLZ77(rom, tileset);
            tileCount = tiles.Length / 32;
            map = new ushort[width * height];
            byte[,] pixels = new byte[height * 8, width * 8];
            bool any = false;
            for (int ty = 0; ty < height; ty++)
                for (int tx = 0; tx < width; tx++) {
                    int entry = ReadShort(rom, tilemap + (ty * width + tx) * 2), tile = entry & 0x3FF;
                    map[ty * width + tx] = (ushort)entry;
                    bool hflip = (entry & 0x400) != 0, vflip = (entry & 0x800) != 0;
                    for (int y = 0; y < 8; y++)
                        for (int x = 0; x < 8; x++) {
                            int sx = hflip ? 7 - x : x, sy = vflip ? 7 - y : y, at = tile * 32 + sy * 4 + sx / 2;
                            byte value = at < tiles.Length ? (byte)(tiles[at] >> (sx % 2 * 4) & 15) : (byte)0;
                            pixels[ty * 8 + y, tx * 8 + x] = value;
                            any |= value != 0;
                        }
                }
            return any || blank ? pixels : null;
        }

        /* StripeAt(); The room's background behind the lattice (BG0's colors 4-6, which the room cycles): rows of 8 pixels of each
         * color, a period of 64 rows: in the first 32, 4, 6, 5 sliding left a pixel a row for 8 rows, still for 16, sliding back for
         * 8; in the other 32, 6, 4, 5 sliding right and back the same way (as the image has it all round the lattice); */
        static byte StripeAt(int x, int y) {
            int row = y % 64, k = row % 32, slide = k <= 8 ? k : k <= 23 ? 8 : 31 - k;
            int at = ((row < 32 ? x + slide : x - slide) % 24 + 24) % 24;
            return (row < 32 ? new byte[] { 4, 6, 5 } : new byte[] { 6, 4, 5 })[at / 8];
        }
        static bool IsStripe(byte value) => value is 4 or 5 or 6;
        static bool InGridBox(int x, int y) => x >= GridLeft && x < GridRight && y >= GridTop && y < GridBottom;

        /* GridTiles(); 4bpp pixels [y, x] as tiles (LZ77), the blank one first unless the tiles start elsewhere, each once, and the
         * tilemap of a tile rectangle (palette banks from another map, if given); (tileset, tilemap) file offsets and the tile count; */
        static (int tileset, int tilemap, int count) GridTiles(byte[] rom, byte[,] pixels, int left, int top, int columns, int rows, int firstTile,
                                                               Func<int, int, int> bank, ref int cursor) {
            List<byte[]> tiles = new List<byte[]>();
            Dictionary<string, int> known = new Dictionary<string, int>();
            if (firstTile == 0) { tiles.Add(new byte[32]); known[Convert.ToBase64String(tiles[0])] = 0; }
            ushort[] map = new ushort[columns * rows];
            for (int ty = 0; ty < rows; ty++)
                for (int tx = 0; tx < columns; tx++) {
                    byte[] tile = new byte[32];
                    for (int y = 0; y < 8; y++)
                        for (int x = 0; x < 8; x++) tile[y * 4 + x / 2] |= (byte)(pixels[(top + ty) * 8 + y, (left + tx) * 8 + x] << (x % 2 * 4));
                    string key = Convert.ToBase64String(tile);
                    if (!known.TryGetValue(key, out int index)) { known[key] = index = tiles.Count; tiles.Add(tile); }
                    map[ty * columns + tx] = (ushort)(firstTile + index | (bank == null ? 0 : bank(left + tx, top + ty) << 12));
                }
            byte[] compressed = CompressLZ77(tiles.SelectMany(t => t).ToArray());
            int tileset = Allocate(ref cursor, compressed.Length, 4);
            compressed.CopyTo(rom, tileset);
            int tilemap = Allocate(ref cursor, map.Length * 2, 4);
            for (int i = 0; i < map.Length; i++) WriteShort(rom, tilemap + i * 2, map[i]);
            return (tileset, tilemap, tiles.Count);
        }

        // (KatAM's English collection texts whose names are known, by their cell's id: CollectionLetters() reads their letters);
        static IEnumerable<(int id, string name)> KatAMNamedCells() {
            for (int color = 0; color < KatAMSprayColors; color++) yield return (0x101 + color, KatAMSprayNames[color]);
            yield return (VitalityId, "VITALITY");
            yield return (0x100, "DEFAULT");
            string[] maps = { "WORLD MAP", "RAINBOW ROUTE", "MOONLIGHT MANSION", "CABBAGE CAVERN", "MUSTARD MOUNTAIN", "CARROT CASTLE",
                              "OLIVE OCEAN", "PEPPERMINT PALACE", "RADISH RUINS", "CANDY CONSTELLATION" };
            for (int area = 0; area < maps.Length; area++) yield return (0x200 + area, maps[area]);
        }
        // (letters KatAM's texts don't have, drawn as its own: 2-pixel strokes, 10 rows from the name line's row 4; a space's blank columns);
        static readonly Dictionary<char, string[]> DrawnLetters = new Dictionary<char, string[]> {
            ['J'] = new[] { "....77", "....77", "....77", "....77", "....77", "....77", "....77", "77..77", "77..77", ".7777." },
            ['X'] = new[] { "77..77", "77..77", "77..77", ".7777.", "..77..", "..77..", ".7777.", "77..77", "77..77", "77..77" },
        };
        const int DrawnLetterTop = 4, SpaceWidth = 3, English = 1;

        /* CollectionLetters(); KatAM's letters (the name line's pixels, as wide as each letter), from its English collection texts
         * (KatAMNamedCells(): the last runs of the name line are the name's letters), and the ones drawn; none in a ROM whose English
         * texts are empty (the Japanese one); */
        static Dictionary<char, byte[,]> CollectionLetters(byte[] rom, KatAMExpansionCode.Version version) {
            int bgTable = version.RomSymbols["gUnk_082D7850"], textTable = version.RomSymbols["gUnk_08386664"], cells = version.RomSymbols["gUnk_08386A50"];
            Dictionary<char, byte[,]> letters = new Dictionary<char, byte[,]>();
            foreach ((int id, string name) in KatAMNamedCells()) {
                int cell = Enumerable.Range(0, CollectionCells).FirstOrDefault(c => ReadShort(rom, cells + c * 4) == id, -1);
                if (cell < 0) continue;
                byte[,] pixels = CollectionTextPixels(rom, bgTable, ReadShort(rom, textTable + (English * 0x27 + rom[cells + cell * 4 + 3]) * 2));
                if (pixels == null) continue;
                string bare = name.Replace(" ", "");
                List<(int start, int end)> runs = TextRuns(pixels);
                if (runs.Count <= bare.Length) continue;
                for (int i = 0; i < bare.Length; i++) {
                    if (letters.ContainsKey(bare[i])) continue;
                    (int start, int end) = runs[runs.Count - bare.Length + i];
                    byte[,] glyph = new byte[TextLineBottom - TextLineTop, end - start];
                    for (int y = TextLineTop; y < TextLineBottom; y++)
                        for (int x = start; x < end; x++) glyph[y - TextLineTop, x - start] = pixels[y, x];
                    letters[bare[i]] = glyph;
                }
            }
            if (letters.Count > 0)
                foreach ((char letter, string[] rows) in DrawnLetters) {
                    if (letters.ContainsKey(letter)) continue;
                    byte[,] glyph = new byte[TextLineBottom - TextLineTop, rows[0].Length];
                    for (int y = 0; y < rows.Length; y++)
                        for (int x = 0; x < rows[y].Length; x++) glyph[DrawnLetterTop + y, x] = rows[y][x] == '.' ? (byte)0 : TextWhite;
                    letters[letter] = glyph;
                }
            return letters;
        }

        /* CollectionNames(); Names as pixels on a text's name line (its 16 rows, as wide as they are): KatAM's letters 1 pixel apart, a
         * space 3 more; in a ROM without them (the Japanese one), Squeak Squad's font (subgame/font_asc, ASCII from 0x20: SqueakCanvas()
         * draws a cell facing the other way, as KatAM's frames face; its bottom on the line's KatAM letters' bottom row); without that
         * either, empty; */
        static byte[][,] CollectionNames(byte[] rom, byte[] squeak, KatAMExpansionCode.Version version, string[] names) {
            Dictionary<char, byte[,]> letters = CollectionLetters(rom, version);
            SqueakCharacter font = null;
            if (!names.All(n => n.All(c => c == ' ' || letters.ContainsKey(c)))) {
                letters.Clear();
                try { font = squeak == null ? null : ReadSqueakCharacter(squeak, "font_asc", "subgame"); }
                catch (Exception exception) when (exception is IndexOutOfRangeException or ArgumentException or InvalidOperationException) { font = null; }
                if (font != null) {
                    Dictionary<(int palette, int color), int> ink = Enumerable.Range(0, 16).SelectMany(p => Enumerable.Range(1, 15).Select(c => (p, c))).ToDictionary(k => k, _ => 1);
                    foreach (char c in names.SelectMany(n => n).Distinct().Where(c => c != ' ')) {
                        int[,] canvas = SqueakCanvas(font, c - 0x20, ink);
                        int size = canvas.GetLength(0), left = size, right = -1, top = size, bottom = -1;
                        for (int x = 0; x < size; x++)
                            for (int y = 0; y < size; y++)
                                if (canvas[x, y] != 0) { left = Math.Min(left, x); right = Math.Max(right, x); top = Math.Min(top, y); bottom = Math.Max(bottom, y); }
                        if (right < 0 || bottom - top + 1 > TextLineBottom - TextLineTop) continue;
                        byte[,] glyph = new byte[TextLineBottom - TextLineTop, right - left + 1];
                        int last = Math.Max(bottom - top, DrawnLetterTop + 9);
                        for (int x = left; x <= right; x++)
                            for (int y = top; y <= bottom; y++)
                                if (canvas[x, y] != 0) glyph[last - (bottom - y), right - x] = TextWhite;
                        letters[c] = glyph;
                    }
                }
            }
            byte[][,] result = new byte[names.Length][,];
            for (int n = 0; n < names.Length; n++) {
                int width = 0;
                foreach (char c in names[n]) width += (c == ' ' ? SpaceWidth : letters.TryGetValue(c, out byte[,] g) ? g.GetLength(1) : 0) + 1;
                result[n] = new byte[TextLineBottom - TextLineTop, Math.Max(0, width - 1)];
                int at = 0;
                foreach (char c in names[n]) {
                    if (c == ' ') { at += SpaceWidth + 1; continue; }
                    if (!letters.TryGetValue(c, out byte[,] glyph)) continue;
                    for (int y = 0; y < glyph.GetLength(0); y++)
                        for (int x = 0; x < glyph.GetLength(1); x++) result[n][y, at + x] = glyph[y, x];
                    at += glyph.GetLength(1) + 1;
                }
            }
            return result;
        }

        /* ComposeCollectionTexts(); New texts made from a family of KatAM's collection texts (gUnk_08386664's indices: the sprays', the
         * maps') in each language: the template one with its name cleared (the runs of the name line that differ among the family's
         * texts, after the ones that are the same in all, and before the ones that are, which stay after the new name as far after it)
         * and each new name written there; the lines under it the template's ("Use this for navigation."). LZ77 tiles and a tilemap
         * (TextTiles()); per name, its (tileset, tilemap) in each language, a language whose texts don't fit this with the first
         * one's that did; */
        static List<(int tileset, int tilemap)[]> ComposeCollectionTexts(byte[] rom, KatAMExpansionCode.Version version, int[] family, int template,
                                                                         byte[][,] names, ref int cursor, bool keepBelow = true) {
            int bgTable = version.RomSymbols["gUnk_082D7850"], textTable = version.RomSymbols["gUnk_08386664"];
            byte[,] Text(int language, int text) => CollectionTextPixels(rom, bgTable, ReadShort(rom, textTable + (language * 0x27 + text) * 2));
            (int tileset, int tilemap)[,] composed = new (int, int)[CollectionLanguages, names.Length];
            List<int> pending = new List<int>();
            for (int language = 0; language < CollectionLanguages; language++) {
                byte[][,] texts = family.Select(t => Text(language, t)).ToArray();
                if (texts.Any(t => t == null)) { pending.Add(language); continue; }
                List<(int start, int end)>[] runs = texts.Select(TextRuns).ToArray();
                // (the runs the same in all: before the name, and after it, from the end);
                int Same(Func<List<(int start, int end)>, int, (int start, int end)?> run) {
                    int count = 0;
                    for (; ; count++) {
                        (int start, int end)? first = run(runs[0], count);
                        if (first == null) return count;
                        for (int t = 1; t < texts.Length; t++) {
                            (int start, int end)? other = run(runs[t], count);
                            if (other == null || other.Value.end - other.Value.start != first.Value.end - first.Value.start ||
                                !SameColumns(texts[0], first.Value.start, texts[t], other.Value.start, first.Value.end - first.Value.start)) return count;
                        }
                    }
                }
                int before = Same((r, i) => i < r.Count ? r[i] : null), after = Same((r, i) => i < r.Count ? r[r.Count - 1 - i] : null);
                List<(int start, int end)> baseRuns = runs[template];
                if (before + after >= baseRuns.Count || before == 0) { pending.Add(language); continue; }
                byte[,] basePixels = texts[template];
                int nameStart = baseRuns[before].start, nameEnd = baseRuns[baseRuns.Count - 1 - after].end;
                int suffixStart = after > 0 ? baseRuns[baseRuns.Count - after].start : 0, width = basePixels.GetLength(1);

                for (int n = 0; n < names.Length; n++) {
                    byte[,] pixels = (byte[,])basePixels.Clone();
                    for (int y = TextLineTop; y < TextLineBottom; y++)
                        for (int x = nameStart; x < width; x++) pixels[y, x] = 0;
                    // (keepBelow false: the lines under the name line cleared, the template's being another item's);
                    for (int y = TextLineBottom; y < pixels.GetLength(0) && !keepBelow; y++)
                        for (int x = 0; x < width; x++) pixels[y, x] = 0;
                    for (int y = 0; y < names[n].GetLength(0); y++)
                        for (int x = 0; x < names[n].GetLength(1) && nameStart + x < width; x++) pixels[TextLineTop + y, nameStart + x] = names[n][y, x];
                    int at = nameStart + names[n].GetLength(1) + 1;
                    // (what came after the name, as far after it);
                    if (after > 0) {
                        int shift = at - 1 + (suffixStart - nameEnd);
                        for (int y = TextLineTop; y < TextLineBottom; y++)
                            for (int x = suffixStart; x < width && shift + x - suffixStart < width; x++)
                                pixels[y, shift + x - suffixStart] = basePixels[y, x];
                    }
                    composed[language, n] = TextTiles(rom, pixels, ref cursor);
                }
            }
            // (a language whose texts don't fit this, or are empty: the first one's that did);
            int fits = Enumerable.Range(0, CollectionLanguages).FirstOrDefault(l => !pending.Contains(l), -1);
            if (fits < 0) throw new InvalidOperationException("The Collection Room's new texts couldn't be composed.");
            foreach (int language in pending) for (int n = 0; n < names.Length; n++) composed[language, n] = composed[fits, n];

            List<(int tileset, int tilemap)[]> result = new List<(int, int)[]>();
            for (int n = 0; n < names.Length; n++)
                result.Add(Enumerable.Range(0, CollectionLanguages).Select(language => composed[language, n]).ToArray());
            return result;
        }

        /* AddCollectionGrid(); The Collection Room's grid (see above): its items (none: the room stays KatAM's) and their texts (each
         * a (tileset, tilemap) per language), its backgrounds, its hooks. Returns an error, or null; */
        static string AddCollectionGrid(byte[] rom, KatAMExpansionCode.Version version, List<CollectionItem> items, List<(int tileset, int tilemap)[]> texts, ref int cursor) {
            if (items.Count == 0) return null;
            string notKatAMs = $"The {version.Name} ROM's Collection Room isn't KatAM's.";
            int extra = items.Max(i => i.Column) + 1, columns = GridShown + extra;
            if (extra > GridExtraMax || items.Any(i => i.Row < 0 || i.Row >= GridRows || i.Column < 0)) throw new InvalidOperationException("The Collection Room's grid is too big.");
            int grid = CodeAddress + version.CodeSymbols["gCollectionGrid"];
            int bgTable = version.RomSymbols["gUnk_082D7850"], textTable = version.RomSymbols["gUnk_08386664"];

            // The room's image in each language (its first text: gUnk_08386664[language][0]), the ones with KatAM's lattice and stripes;
            byte[][,] images = new byte[CollectionLanguages][,];
            ushort[][] maps = new ushort[CollectionLanguages][];
            for (int language = 0; language < CollectionLanguages; language++) {
                byte[,] pixels = TiledBgPixels(rom, bgTable, ReadShort(rom, textTable + language * 0x27 * 2), CollectionImageWidth, CollectionImageHeight, false,
                                               out ushort[] map, out int tileCount);
                if (pixels == null || tileCount > GridImageTile) continue;
                bool katams = true;
                for (int row = 0; row < GridRows && katams; row++)
                    for (int column = 0; column < GridShown && katams; column++)
                        katams = pixels[GridTop + 3 + row * CellHeight, GridLeft + column * CellWidth] == LatticeFrame;
                for (int y = GridTop - 3; y < GridBottom + 2 && katams; y++)
                    for (int x = 0; x < CollectionImageWidth * 8 && katams; x++)
                        if (!InGridBox(x, y) && IsStripe(pixels[y, x])) katams = pixels[y, x] == StripeAt(x, y);
                        else if (InGridBox(x, y) && !IsStripe(pixels[y, x])) katams = pixels[y, x] != 0;
                if (katams) { images[language] = pixels; maps[language] = map; }
            }
            int source = images[1] != null ? 1 : Array.FindIndex(images, i => i != null);
            if (source < 0) return notKatAMs;
            // (a language whose lattice isn't the source's keeps its image as it is: the cells' icons scroll, its lattice doesn't);
            for (int language = 0; language < CollectionLanguages; language++)
                if (images[language] != null)
                    for (int y = GridTop; y < GridBottom && images[language] != null; y++)
                        for (int x = GridLeft; x < GridRight; x++)
                            if (!IsStripe(images[source][y, x]) && images[language][y, x] != images[source][y, x]) { images[language] = null; break; }

            /* The lattice (BG2): column 0 the image's first, the last its last, the ones between its 1-8 over and over; the stripes clear;
             * a grid wider than BG2 (512 pixels: 23 columns, 518) wraps round to its left, as BG2 does, where window 0 (the grid's box)
             * hides the map's first 12 pixels while the grid's start shows; */
            byte[,] lattice = new byte[(GridMapTop + GridMapRows) * 8, LatticeWidth * 8];
            if (GridLeft + columns * CellWidth - LatticeWidth * 8 > GridLeft) throw new InvalidOperationException("The Collection Room's grid is too big.");
            for (int y = GridTop; y < GridBottom; y++)
                for (int x = GridLeft; x < GridLeft + columns * CellWidth; x++) {
                    int k = (x - GridLeft) / CellWidth, r = (x - GridLeft) % CellWidth;
                    int s = k == 0 ? 0 : k == columns - 1 ? GridShown - 1 : 1 + (k - 1) % (GridShown - 2);
                    byte value = images[source][y, Math.Min(GridLeft + s * CellWidth + r, CollectionImageWidth * 8 - 1)];
                    lattice[y, x % (LatticeWidth * 8)] = IsStripe(value) ? (byte)0 : value;
                }
            (int latticeTiles, int latticeMap, _) = GridTiles(rom, lattice, 0, GridMapTop, LatticeWidth, GridMapRows, 0, null, ref cursor);
            WriteInts(rom, grid + GridLatticeTilesAt, latticeTiles + 0x08000000, latticeMap + 0x08000000);

            // Each language's grid area (BG0) with the stripes all over the grid's box, at tile 0x180 (one copy per different one);
            List<(byte[] data, int tileset, int tilemap)> done = new List<(byte[], int, int)>();
            for (int language = 0; language < CollectionLanguages; language++) {
                if (images[language] == null) continue;
                byte[,] pixels = (byte[,])images[language].Clone();
                for (int y = GridTop; y < GridBottom; y++)
                    for (int x = GridLeft; x < GridRight; x++) pixels[y, x] = StripeAt(x, y);
                ushort[] map = maps[language];
                byte[] data = new byte[GridMapRows * 8 * GridImageColumns * 8 + GridMapRows * GridImageColumns];
                int d = 0;
                for (int y = GridMapTop * 8; y < (GridMapTop + GridMapRows) * 8; y++)
                    for (int x = GridImageLeft * 8; x < (GridImageLeft + GridImageColumns) * 8; x++) data[d++] = pixels[y, x];
                for (int ty = GridMapTop; ty < GridMapTop + GridMapRows; ty++)
                    for (int tx = GridImageLeft; tx < GridImageLeft + GridImageColumns; tx++) data[d++] = (byte)(map[ty * CollectionImageWidth + tx] >> 12);
                int same = done.FindIndex(e => e.data.SequenceEqual(data));
                (int tileset, int tilemap) at;
                if (same >= 0) at = (done[same].tileset, done[same].tilemap);
                else {
                    (int tileset, int tilemap, int count) = GridTiles(rom, pixels, GridImageLeft, GridMapTop, GridImageColumns, GridMapRows, GridImageTile,
                                                                      (tx, ty) => map[ty * CollectionImageWidth + tx] >> 12, ref cursor);
                    if (count > GridImageTilesMax) return notKatAMs;
                    at = (tileset, tilemap);
                    done.Add((data, tileset, tilemap));
                }
                WriteInt(rom, grid + GridImageTilesAt + language * 4, at.tileset + 0x08000000);
                WriteInt(rom, grid + GridImageMapAt + language * 4, at.tilemap + 0x08000000);
            }

            // The items and their texts;
            int cells = version.RomSymbols["gUnk_08386A50"];
            WriteInt(rom, grid + GridKatamAt, cells + 0x08000000);
            rom[grid + GridExtraAt] = (byte)extra;
            rom[grid + GridTextCountAt] = (byte)texts.Count;
            if (texts.Count > 0) {
                int table = Allocate(ref cursor, CollectionLanguages * texts.Count * 8, 4);
                for (int language = 0; language < CollectionLanguages; language++)
                    for (int i = 0; i < texts.Count; i++)
                        WriteInts(rom, table + (language * texts.Count + i) * 8, texts[i][language].tileset + 0x08000000, texts[i][language].tilemap + 0x08000000);
                WriteInt(rom, grid + GridTextsAt, table + 0x08000000);
            }
            foreach (CollectionItem item in items) {
                int at = grid + GridItemsAt + (item.Row * GridExtraMax + item.Column) * GridItemSize;
                WriteShort(rom, at, item.Id);
                WriteShort(rom, at + 2, item.Anim);
                rom[at + 4] = (byte)item.Variant;
                rom[at + 5] = (byte)item.Palette;
                rom[at + 6] = (byte)(item.Text < 0 ? 0xFF : item.Text);
                rom[at + 7] = (byte)item.RoomText;
            }

            // The hooks: the cells read from EWRAM (collection.c's gNidlBigArea), the room's opening, the step after its image, its frame, its end;
            int roomFrom = version.RomSymbols["sub_081434F8"], roomTo = version.RomSymbols["sub_08145B64"];
            int ram = ReadInt(rom, CodeAddress + version.CodeSymbols["gNidlBigArea"]) + GridRoomEntries;
            int Code(string name) => CodeAddress + 0x08000001 + version.CodeSymbols[name];
            int Rom(string name) => version.RomSymbols[name] + 0x08000001;
            if (Repoint(rom, roomFrom, roomTo, cells + 0x08000000, ram) != 20 ||
                Repoint(rom, roomFrom, roomTo, Rom("sub_0814405C"), Code("GridRoomFrame")) != 1 ||
                Repoint(rom, 0, roomFrom, Rom("sub_081428A0"), Code("GridRoomStart")) != 1 ||
                Repoint(rom, version.RomSymbols["sub_08143E6C"], version.RomSymbols["sub_08143EF4"], Rom("sub_08143EF4"), Code("GridRoomBg")) != 1 ||
                Repoint(rom, roomFrom, version.RomSymbols["sub_08143664"], Rom("sub_08143664"), Code("GridRoomEnd")) != 1)
                return notKatAMs;
            return null;
        }
    }
}
