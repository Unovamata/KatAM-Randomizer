using System;
using System.Collections.Generic;
using System.Linq;

namespace KatAMRandomizer {
    /* KatAMExpansionSprays; Squeak Squad's spray paints that KatAM doesn't have: its 18 sprays are KatAM's 13 (YELLOW to CHALK, by
     * name; their chests hold items, KatAMExpansionSqueakRooms.cs) and SHADOW, IVORY, CITRUS, WHITE and LAVENDER: Kirby colors 16-20,
     * owned in the spray paints' field's bits 14-18, found in Squeak Squad's chests and shown in the Collection Room's grid, the first
     * row after KatAM's columns (KatAMExpansionCollection.cs; "ROM Expansion/collection.c"). Here: their palettes (Squeak Squad's collection/spray14-18.cla, KatAM's layout: 1
     * the outline, 2-8 the body, 9-11 the feet, 12-15 the whites) as anim 0x15's variants 16-20 (Kirby) and anim 0x1A0's (the UFO: the
     * body's colors, the UFO's own after them); the tables a color is read from lengthened to 21: the Collection Room's previews and the game's
     * (gUnk_08386AF0, gUnk_082EB4B4: anim 0x15 and the color), the continue screen's palettes (gUnk_0834BB20); the sprays' cells in
     * the Collection Room's grid: their cans and texts; */
    internal static partial class KatAMExpansion {
        // (the new sprays: how many, Squeak Squad's number of the first, its spray paints' treasure numbers' first: KatAM's 13 first);
        const int SqueakSprayCount = 5, SqueakSprayFile = 14, SqueakSprayColor = 16, KatAMColors = 16, KatAMSprayColors = 14;
        const int KirbyPaletteAnim = 0x15, UfoPaletteAnim = 0x1A0, CollectionIconAnim = 0x388, CollectionCells = 40;
        const int CollectionTextColumns = 30, CollectionTextRows = 7, CollectionLanguages = 6;
        /* (the OBJ palettes the cans take, two cans each: the Collection Room's free ones (after its icons' and previews' 0-11) for the
         * columns that show with KatAM's spray columns, KatAM's sprays' 6 for the fifth, shown only once those are off the screen:
         * KatAMExpansionMaps.cs's AreaMapPaletteSlots has the rest); */
        static readonly byte[] SqueakSprayIconSlots = { 0xC, 0xD, 6 };
        // (KatAM's sprays' names in English, by color: their texts, "SPRAY PAINT : NAME", give the letters of the new ones');
        static readonly string[] KatAMSprayNames = { "PINK", "YELLOW", "RED", "GREEN", "SNOW", "CARBON", "OCEAN", "SAPPHIRE", "GRAPE",
                                                     "EMERALD", "ORANGE", "CHOCOLATE", "CHERRY", "CHALK" };
        // (the new sprays' names, Squeak Squad's sprays 14-18, collection/spray2's cells 14-18; its 1-13 are KatAM's colors 1-13's);
        static readonly string[] SqueakSprayNames = { "SHADOW", "IVORY", "CITRUS", "WHITE", "LAVENDER" };
        const int VitalityId = 0x800;

        // SqueakSprayPalette(); The new spray n's (0-4) Kirby palette;
        static ushort[] SqueakSprayPalette(byte[] squeak, int n) {
            byte[] cla = SqueakFile(squeak, $"collection/spray{SqueakSprayFile + n:D2}.cla");
            if (cla == null || cla.Length < 12 + 32) throw new ArgumentException("A spray's palette is missing.");
            return Enumerable.Range(0, 16).Select(i => (ushort)ReadShort(cla, 12 + i * 2)).ToArray();
        }

        // AnimVariant(); A variant's commands' address (file offset) in an anims table;
        static int AnimVariant(byte[] rom, int anims, int animId, int variant) =>
            ReadInt(rom, ReadInt(rom, anims + animId * 4) - 0x08000000 + variant * 4) - 0x08000000;

        // AnimVariantCount(); How many variants an anim has (its list's pointers, up to the first word that isn't one);
        static int AnimVariantCount(byte[] rom, int anims, int animId) {
            int list = ReadInt(rom, anims + animId * 4) - 0x08000000, count = 0;
            while ((uint)ReadInt(rom, list + count * 4) - 0x08000000u < 0x02000000u) count++;
            return count;
        }

        static int ColorDistance(ushort a, ushort b) {
            int r = (a & 31) - (b & 31), g = (a >> 5 & 31) - (b >> 5 & 31), bl = (a >> 10 & 31) - (b >> 10 & 31);
            return r * r + g * g + bl * bl;
        }

        /* SqueakSprayKatAMCan(); The KatAM spray (its color, 1-13) whose can a chest holding the new spray n shows rising (the chest's
         * item, chest.c's NidlChestOpen gives it back): the closest body (the can's color: the feet only band it); */
        public static int SqueakSprayKatAMCan(byte[] rom, int spriteTables, byte[] squeak, int n) {
            int anims = ReadInt(rom, spriteTables) - 0x08000000, paletteBase = ReadInt(rom, spriteTables + 12) - 0x08000000;
            ushort[] squeakColors = SqueakSprayPalette(squeak, n);
            return Enumerable.Range(1, KatAMSprayColors - 1).OrderBy(color => {
                int palette = paletteBase + ReadInt(rom, AnimVariant(rom, anims, KirbyPaletteAnim, color) + 4) * 32;
                return Enumerable.Range(2, 7).Sum(i => ColorDistance(squeakColors[i], (ushort)ReadShort(rom, palette + i * 2)));
            }).First();
        }

        /* AddSqueakSprayIcons(); The Collection Room's cans (before the anims tables are copied: a new anim, its frames anim 0x388's,
         * KatAM's Collection Room icons'): KatAM's spray can (0x388's variant 1, frame 1: its body colors 1-2, its band, the feet's,
         * 4-5), and a copy of its tiles drawing those with 3, 6, 9 and 10, two cans to a palette (KatAM's can palette, the sprays'
         * body colors 3 and 5, feet 9 and 10). Returns the anim's ID; */
        static int AddSqueakSprayIcons(byte[] rom, byte[] squeak, int anims, int attrTable, int oamTable, int paletteBase, int tilesBase,
                                       ref int cursor, List<int> animVariants, List<int> animAttributes, List<int> animOams, int animId) {
            int can = AnimVariant(rom, anims, CollectionIconAnim, 1);
            if (ReadInt(rom, can) != -2 || ReadInt(rom, can + 12) != -1 || ReadInt(rom, can + 20) != 4 || ReadInt(rom, can + 32) != -4)
                throw new InvalidOperationException("The Collection Room's spray can isn't KatAM's.");
            int canPalette = paletteBase + ReadInt(rom, can + 4) * 32, canTiles = ReadInt(rom, can + 16), frame = ReadInt(rom, can + 28);

            // The second can's tiles;
            byte[] remap = Enumerable.Range(0, 16).Select(i => (byte)(i switch { 1 => 3, 2 => 6, 4 => 9, 5 => 10, _ => i })).ToArray();
            int tilesB = Allocate(ref cursor, 4 * 32, 32, tilesBase);
            for (int i = 0; i < 4 * 32; i++) {
                byte pixels = rom[tilesBase + canTiles * 32 + i];
                rom[tilesB + i] = (byte)(remap[pixels & 15] | remap[pixels >> 4] << 4);
            }

            int variants = Allocate(ref cursor, SqueakSprayCount * 4, 4);
            for (int pair = 0; pair * 2 < SqueakSprayCount; pair++) {
                ushort[] a = SqueakSprayPalette(squeak, pair * 2), b = SqueakSprayPalette(squeak, Math.Min(pair * 2 + 1, SqueakSprayCount - 1));
                int palette = Allocate(ref cursor, 32, 32, paletteBase);
                Array.Copy(rom, canPalette, rom, palette, 32);
                foreach ((int index, ushort color) in new[] { (1, a[3]), (2, a[5]), (4, a[9]), (5, a[10]), (3, b[3]), (6, b[5]), (9, b[9]), (10, b[10]) })
                    WriteShort(rom, palette + index * 2, color);
                for (int which = 0; which < 2 && pair * 2 + which < SqueakSprayCount; which++) {
                    int commands = Allocate(ref cursor, 36, 4);
                    WriteInts(rom, commands, -2, (palette - paletteBase) / 32, 16, -1, which == 0 ? canTiles : (tilesB - tilesBase) / 32, 4, 1, frame, -4);
                    WriteInt(rom, variants + (pair * 2 + which) * 4, commands + 0x08000000);
                }
            }
            animVariants.Add(variants);
            animAttributes.Add(ReadInt(rom, attrTable + CollectionIconAnim * 4) - 0x08000000);
            animOams.Add(ReadInt(rom, oamTable + CollectionIconAnim * 4) - 0x08000000);
            return animId;
        }

        /* Repoint(); The words in [from, to) (file offsets) holding an address, set to another; how many; */
        static int Repoint(byte[] rom, int from, int to, int address, int newAddress) {
            int count = 0;
            for (int at = from & ~3; at + 4 <= to; at += 4)
                if (ReadInt(rom, at) == address) { WriteInt(rom, at, newAddress); count++; }
            return count;
        }

        // A collection text (gUnk_082D7850's TiledBg, 30 x 7 tiles) as pixels, [y, x]; null if it isn't one;
        static byte[,] CollectionTextPixels(byte[] rom, int bgTable, int index) =>
            TiledBgPixels(rom, bgTable, index, CollectionTextColumns, CollectionTextRows, true, out _, out _);

        // The name line's rows (the texts' first line, tile rows 1-2), and its glyphs: the runs of columns with pixels in them;
        const int TextLineTop = 8, TextLineBottom = 24;
        // (the letters' last row in the line, KatAM's text color; Squeak Squad's names' white, collection/spray2.cla's color 10);
        const int TextBaseline = 12, SqueakNameWhite = 10;
        const byte TextWhite = 7;
        static List<(int start, int end)> TextRuns(byte[,] pixels) {
            List<(int, int)> runs = new List<(int, int)>();
            int width = pixels.GetLength(1), start = -1;
            for (int x = 0; x <= width; x++) {
                bool any = false;
                for (int y = TextLineTop; x < width && y < TextLineBottom && !any; y++) any = pixels[y, x] != 0;
                if (any && start < 0) start = x;
                if (!any && start >= 0) { runs.Add((start, x)); start = -1; }
            }
            return runs;
        }

        static bool SameColumns(byte[,] a, int aStart, byte[,] b, int bStart, int width) {
            for (int x = 0; x < width; x++)
                for (int y = TextLineTop; y < TextLineBottom; y++)
                    if (a[y, aStart + x] != b[y, bStart + x]) return false;
            return true;
        }

        /* SqueakSprayTexts(); The new sprays' texts in each language (the Collection Room's, over the one it puts up: collection.c): the
         * language's text of KatAM's spray CHALK with its name in KatAM's letters (ComposeCollectionTexts()), or, in a ROM without KatAM's
         * English texts (the Japanese one's are empty), Squeak Squad's own names (collection/spray2's cells, its sprays' numbers: their
         * white letters, which are KatAM's strokes but 7 pixels tall, standing on the line as KatAM's 10; SqueakCanvas() draws a cell
         * facing the other way, as KatAM's frames face); */
        static List<(int tileset, int tilemap)[]> SqueakSprayTexts(byte[] rom, byte[] squeak, KatAMExpansionCode.Version version, int[] sprayTexts, ref int cursor) {
            Dictionary<char, byte[,]> letters = CollectionLetters(rom, version);
            string[] newNames = SqueakSprayNames;
            byte[][,] names;
            if (newNames.All(n => n.All(letters.ContainsKey))) names = CollectionNames(rom, null, version, newNames);
            else {
                names = new byte[newNames.Length][,];
                SqueakCharacter spray2 = ReadSqueakCharacter(squeak, "spray2", "collection");
                Dictionary<(int palette, int color), int> white = Enumerable.Range(0, 16).ToDictionary(p => (p, SqueakNameWhite), _ => 1);
                for (int n = 0; n < newNames.Length; n++) {
                    int[,] canvas = SqueakCanvas(spray2, SqueakSprayFile + n, white);
                    int size = canvas.GetLength(0), left = size, right = -1, top = size, bottom = -1;
                    for (int x = 0; x < size; x++)
                        for (int y = 0; y < size; y++)
                            if (canvas[x, y] != 0) { left = Math.Min(left, x); right = Math.Max(right, x); top = Math.Min(top, y); bottom = Math.Max(bottom, y); }
                    if (right < 0 || bottom - top + 1 > TextBaseline) throw new InvalidOperationException("Squeak Squad's spray names couldn't be read.");
                    names[n] = new byte[TextLineBottom - TextLineTop, right - left + 1];
                    for (int x = left; x <= right; x++)
                        for (int y = top; y <= bottom; y++)
                            if (canvas[x, y] != 0) names[n][TextBaseline - (bottom - y), right - x] = TextWhite;
                }
            }
            return ComposeCollectionTexts(rom, version, sprayTexts, KatAMSprayColors - 1, names, ref cursor);
        }

        /* TextTiles(); Pixels as a text's tiles (LZ77) and its tilemap (palette 0, as KatAM's texts): the blank tile first, the tiles
         * once each; (tileset, tilemap) file offsets; */
        static (int tileset, int tilemap) TextTiles(byte[] rom, byte[,] pixels, ref int cursor) {
            List<byte[]> tiles = new List<byte[]> { new byte[32] };
            Dictionary<string, int> known = new Dictionary<string, int> { [Convert.ToBase64String(new byte[32])] = 0 };
            ushort[] map = new ushort[CollectionTextColumns * CollectionTextRows];
            for (int ty = 0; ty < CollectionTextRows; ty++)
                for (int tx = 0; tx < CollectionTextColumns; tx++) {
                    byte[] tile = new byte[32];
                    for (int y = 0; y < 8; y++)
                        for (int x = 0; x < 8; x++) tile[y * 4 + x / 2] |= (byte)(pixels[ty * 8 + y, tx * 8 + x] << (x % 2 * 4));
                    string key = Convert.ToBase64String(tile);
                    if (!known.TryGetValue(key, out int index)) { known[key] = index = tiles.Count; tiles.Add(tile); }
                    map[ty * CollectionTextColumns + tx] = (ushort)index;
                }
            byte[] compressed = CompressLZ77(tiles.SelectMany(t => t).ToArray());
            int tileset = Allocate(ref cursor, compressed.Length, 4);
            compressed.CopyTo(rom, tileset);
            int tilemap = Allocate(ref cursor, map.Length * 2, 4);
            for (int i = 0; i < map.Length; i++) WriteShort(rom, tilemap + i * 2, map[i]);
            return (tileset, tilemap);
        }

        /* AddSqueakSprays(); The rest (after the anims tables are copied: anims is the new one): the colors' variants, the tables,
         * the sprays' cells in the Collection Room's grid (added to items, their texts to texts). Returns an error, or null; */
        static string AddSqueakSprays(byte[] rom, byte[] squeak, KatAMExpansionCode.Version version, int iconAnim, int paletteBase,
                                      List<CollectionItem> items, List<(int tileset, int tilemap)[]> texts, ref int cursor) {
            int anims = ReadInt(rom, version.SpriteTables) - 0x08000000;

            // Kirby's and the UFO's palettes: their variants copied, the sprays' after them (a palette, then End);
            foreach (int animId in new[] { KirbyPaletteAnim, UfoPaletteAnim }) {
                if (AnimVariantCount(rom, anims, animId) != KatAMColors) return $"The {version.Name} ROM's Kirby colors aren't KatAM's.";
                int list = Allocate(ref cursor, (KatAMColors + SqueakSprayCount) * 4, 4), old = ReadInt(rom, anims + animId * 4) - 0x08000000;
                Array.Copy(rom, old, rom, list, KatAMColors * 4);
                int ufo = paletteBase + ReadInt(rom, AnimVariant(rom, anims, UfoPaletteAnim, 0) + 4) * 32;
                for (int n = 0; n < SqueakSprayCount; n++) {
                    ushort[] colors = SqueakSprayPalette(squeak, n);
                    int palette = Allocate(ref cursor, 32, 32, paletteBase), commands = Allocate(ref cursor, 24, 4);
                    for (int i = 0; i < 16; i++) WriteShort(rom, palette + i * 2, animId == UfoPaletteAnim && i > 8 ? ReadShort(rom, ufo + i * 2) : colors[i]);
                    WriteInts(rom, commands, -2, (palette - paletteBase) / 32, 16, 1, -1, -4);
                    WriteInt(rom, list + (KatAMColors + n) * 4, commands + 0x08000000);
                }
                WriteInt(rom, anims + animId * 4, list + 0x08000000);
            }

            int colorCount = KatAMColors + SqueakSprayCount, code = version.RomSymbols["sub_08145B64"] + 0x20000;
            int roomFrom = version.RomSymbols["sub_081434F8"], roomTo = version.RomSymbols["sub_08145B64"];
            // The colors' anim and variant (gUnk_082EB4B4, gUnk_08386AF0: anim 0x15, the color), one table for both;
            int previews = Allocate(ref cursor, colorCount * 4, 4);
            for (int color = 0; color < colorCount; color++) WriteInts(rom, previews + color * 4, KirbyPaletteAnim | color << 16);
            int game = version.RomSymbols["gUnk_082EB4B4"] + 0x08000000, room = version.RomSymbols["gUnk_08386AF0"] + 0x08000000;
            if (Repoint(rom, 0, code, game, previews + 0x08000000) != 2 || Repoint(rom, 0, code, game + 2, previews + 0x08000002) != 2 ||
                Repoint(rom, roomFrom, roomTo, room, previews + 0x08000000) != 6)
                return $"The {version.Name} ROM's color tables aren't KatAM's.";

            /* The continue screen's palettes (gUnk_0834BB20, 32 bytes a color): KatAM's sprays' take colors 3, 4, 5, 10, 6, 7 of Kirby's
             * as their 1-6, the rest the same in all (CHERRY's: colors 16 and 17 have none in KatAM); */
            int continueOld = version.RomSymbols["gUnk_0834BB20"], continues = Allocate(ref cursor, colorCount * 32, 4);
            Array.Copy(rom, continueOld, rom, continues, KatAMColors * 32);
            for (int n = 0; n < SqueakSprayCount; n++) {
                ushort[] colors = SqueakSprayPalette(squeak, n);
                int at = continues + (KatAMColors + n) * 32;
                Array.Copy(rom, continueOld + 12 * 32, rom, at, 32);
                int[] from = { 3, 4, 5, 10, 6, 7 };
                for (int i = 0; i < from.Length; i++) WriteShort(rom, at + (i + 1) * 2, colors[from[i]]);
            }
            if (Repoint(rom, 0, code, continueOld + 0x08000000, continues + 0x08000000) != 4) return $"The {version.Name} ROM's continue palettes aren't KatAM's.";

            // The Collection Room's grid (KatAMExpansionCollection.cs): the sprays in its first row after KatAM's, their cans, their texts;
            int cells = version.RomSymbols["gUnk_08386A50"];
            int[] sprayTexts = new int[KatAMSprayColors];
            for (int cell = 0; cell < CollectionCells; cell++) {
                int id = ReadShort(rom, cells + cell * 4);
                if (id > 0x100 && id <= 0x100 + KatAMSprayColors) sprayTexts[id - 0x101] = rom[cells + cell * 4 + 3];
            }
            if (sprayTexts.Distinct().Count() != KatAMSprayColors) return $"The {version.Name} ROM's Collection Room isn't KatAM's.";
            int firstText = texts.Count;
            texts.AddRange(SqueakSprayTexts(rom, squeak, version, sprayTexts, ref cursor));
            // (the text the room puts up, under the spray's own: CHALK's, whose "SPRAY PAINT :" it keeps);
            for (int n = 0; n < SqueakSprayCount; n++)
                items.Add(new CollectionItem(0, n, 0x100 + SqueakSprayColor + 1 + n, iconAnim, n, SqueakSprayIconSlots[n / 2], firstText + n, sprayTexts[KatAMSprayColors - 1]));
            return null;
        }
    }
}
