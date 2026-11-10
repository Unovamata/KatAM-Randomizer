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
                  GridImageTilesAt = 0x14, GridImageMapAt = 0x2C, GridItemsAt = 0x44, GridItemSize = 8, GridExtraMax = 12;
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

            // The lattice (BG2): column 0 the image's first, the last its last, the ones between its 1-8 over and over; the stripes clear;
            byte[,] lattice = new byte[(GridMapTop + GridMapRows) * 8, LatticeWidth * 8];
            for (int y = GridTop; y < GridBottom; y++)
                for (int x = GridLeft; x < GridLeft + columns * CellWidth; x++) {
                    int k = (x - GridLeft) / CellWidth, r = (x - GridLeft) % CellWidth;
                    int s = k == 0 ? 0 : k == columns - 1 ? GridShown - 1 : 1 + (k - 1) % (GridShown - 2);
                    byte value = images[source][y, Math.Min(GridLeft + s * CellWidth + r, CollectionImageWidth * 8 - 1)];
                    lattice[y, x] = IsStripe(value) ? (byte)0 : value;
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
