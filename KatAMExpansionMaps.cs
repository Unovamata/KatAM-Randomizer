using System;
using System.Collections.Generic;
using System.Linq;

namespace KatAMRandomizer {
    /* KatAMExpansionMaps; The expansion's areas' maps (the owner: "KatAM's map item, its sprite recolored to each NiDL and Squeak Squad
     * area's theme, one per area, so the map viewer shows the area once its map is found"): map 0-7 the NiDL levels', 8-15 the Squeak
     * Squad worlds', owned in gTreasures.bigChestField's bits 10-25 ("ROM Expansion/squeak.h"'s AREA_MAP_BIT; KatAM's 10 maps take
     * bits 0-9), each in a chest of its area (a NiDL level's best chest, PlanChests(); a Squeak Squad world's deepest chest of the ones
     * holding a consumable, a big one first, SqueakMapChests()), the chest's template's unk16 CHEST_MARK_MAP << 8 | the map's number
     * + 1, its item one of KatAM's maps 1-9 (chest.c's NidlChestOpen gives it back and makes the popup the area's). The Fountain of
     * Dreams is all boss rooms: its map's chest is in its hub, after Nightmare (PlanChests()'s FountainChestAt). Their look: KatAM's map, its body in the area's
     * colors (AreaMapThemes): rising out of the chest (KatAM's popup, drawn with the chest's palette, its colors 8-10 the area's 3
     * shades, gNidlMaps.colors: chest.c's NidlMapPopup), and in the Collection Room's grid (collection.c: the NiDL maps the third row after
     * KatAM's columns, the Squeak Squad ones the fourth; anim 0x388's map with the area's light and mid shades and a dark shared by
     * all, 4 maps to an OBJ palette, AreaMapPalettes), named "• AREA" over KatAM's map text ("Use this for navigation."); */
    internal static partial class KatAMExpansion {
        internal const int AreaMapCount = 16, AreaMapBit = 10, ChestMarkMap = 1, NidlAreaMaps = 8;
        // (the Fountain of Dreams' map, in its hub's chest: all its level's rooms are boss rooms);
        internal const int FountainMap = 7;
        static readonly string[] AreaMapNames = {
            "VEGETABLE VALLEY", "ICE CREAM ISLAND", "BUTTER BUILDING", "GRAPE GARDEN", "YOGURT YARD", "ORANGE OCEAN", "RAINBOW RESORT", "FOUNTAIN OF DREAMS",
            "PRISM PLAINS", "NATURE NOTCH", "CUSHY CLOUD", "JAM JUNGLE", "VOCANO VALLEY", "ICE ISLAND", "SECRET SEA", "GAMBLE GALAXY" };
        // (each map's light, mid and dark shades, 0xRRGGBB);
        static readonly (int light, int mid, int dark)[] AreaMapThemes = {
            (0xD0F068, 0x88C018, 0x407008), (0xF8E8C8, 0xE0B890, 0xA07050), (0xF8E078, 0xD0A030, 0x805810), (0xE8A8F8, 0xB048C8, 0x601878),
            (0xD8B088, 0x986840, 0x583818), (0xF8B090, 0xE86848, 0x982820), (0xB0F8F0, 0x40C8D8, 0x107890), (0xA8B0F8, 0x6060D0, 0x282078),
            (0xC0F8B8, 0x60D070, 0x208038), (0xC8D080, 0x809038, 0x405018), (0xF8F0F8, 0xF0C0E0, 0xA06890), (0xF090B0, 0xB82860, 0x680830),
            (0xE87040, 0x782820, 0x401008), (0xE8F8F8, 0xB0E8E8, 0x4098C8), (0x60B8E8, 0x1860A8, 0x101868), (0x9078C8, 0x382870, 0x180C40) };
        // (the shared dark of the Collection Room's icons, their black and the map's yellow dots, KatAM's);
        const int AreaMapInk = 0x282830;
        // (KatAM's map icon in the Collection Room: anim 0x388's 0x19, its colors 7 black, 8 light, 9 the dots, 11 mid, 15 dark);
        const int CollectionMapVariant = 0x19;
        /* (the OBJ palette slots the Collection Room's new columns take, two columns to a palette: the free ones (12-15) for the columns
         * that can show with KatAM's spray columns, KatAM's sprays' own (4-9) for the ones shown only once those are off the screen,
         * collection.c's 10 columns and a scroll's 11 apart; the maps' palettes; the cans' (KatAMExpansionSprays.cs, 2 a palette)); */
        static readonly byte[] AreaMapPaletteSlots = { 0xE, 0xF, 4, 7 };

        static ushort Bgr555(int rgb) => (ushort)((rgb >> 19 & 31) | (rgb >> 11 & 31) << 5 | (rgb >> 3 & 31) << 10);

        /* (Squeak Squad's Secret Map: its 7 pieces, treasures 93-99, each in its own chest as Squeak Squad has them, the chest's template's
         * unk16 CHEST_MARK_PIECE << 8 | the piece's number + 1, its item one of KatAM's maps (chest.c's NidlChestOpen gives the piece, owned
         * in gTreasures.musicPlayerAndSheetsField's bits 16-22, "ROM Expansion/squeak.h"'s SECRET_MAP_BIT); their icons Squeak Squad's
         * (collection/map's cells 0-6; 7 the whole map), shrunk, in the Collection Room's second row after KatAM's columns, the maps'
         * palettes' colors 4-6 their parchment); */
        internal const int SecretMapPieces = 7, ChestMarkPiece = 2;
        const int SqueakSecretMapFirst = 93, SecretMapIconSize = 16, SecretMapIconFit = 14;
        static readonly string SecretMapName = "SECRET MAP PIECE";

        // AreaMapItem(); The KatAM map (1-9) a chest holding map m gives for a moment;
        internal static byte AreaMapItem(int map) => (byte)(0xA + 1 + map % 9);

        /* AddAreaMapIcons(); The maps' anim in the Collection Room (before the anims tables are copied), returned: variant m map m,
         * KatAM's map, its tiles drawing 1 black, 2 the dots, 3 the dark, 7 + 2k and 8 + 2k the light and mid of the palette's k-th map
         * (0, 1 a NiDL map, 2, 3 a Squeak Squad one), 4 palettes of 4 maps (a column pair each); with Squeak Squad's rooms (squeak), the
         * Secret Map's pieces' too: variant 16 + p piece p, its tiles drawing 1 black and 4-6 its parchment (SecretMapPieceIcons()); then
         * the Ability Scrolls' (ScrollIconVariant + n, the DS's own, ScrollIcons(): each in one of ScrollIconSlots' palettes) and the
         * pieces' chest popups (PiecePopupVariant + p: the piece's icon in the popup's colors, 7 black, 8-10 the parchment, which
         * chest.c's NidlMapPopup puts in the chest's palette; no palette of its own: an effect's sprite loads none); */
        static int AddAreaMapIcons(byte[] rom, byte[] squeak, int anims, int attrTable, int oamTable, int paletteBase, int tilesBase, ref int cursor,
                                   List<int> animVariants, List<int> animAttributes, List<int> animOams, int animId) {
            int icon = AnimVariant(rom, anims, CollectionIconAnim, CollectionMapVariant);
            if (ReadInt(rom, icon) != -2 || ReadInt(rom, icon + 12) != -1 || ReadInt(rom, icon + 20) != 4 || ReadInt(rom, icon + 32) != -4)
                throw new InvalidOperationException("KatAM's map icons aren't KatAM's.");
            int iconTiles = ReadInt(rom, icon + 16), iconFrame = ReadInt(rom, icon + 28);

            // The Collection Room's: 4 tile sets (k), 4 palettes (a column pair each);
            int[] tileSets = new int[4];
            for (int k = 0; k < 4; k++) {
                byte[] remap = Enumerable.Range(0, 16).Select(i => (byte)(i switch { 7 => 1, 9 => 2, 15 => 3, 8 => 7 + 2 * k, 11 => 8 + 2 * k, _ => 0 })).ToArray();
                tileSets[k] = Allocate(ref cursor, 4 * 32, 32, tilesBase);
                for (int i = 0; i < 4 * 32; i++) {
                    byte pixels = rom[tilesBase + iconTiles * 32 + i];
                    rom[tileSets[k] + i] = (byte)(remap[pixels & 15] | remap[pixels >> 4] << 4);
                }
            }
            (byte[][] pieceTiles, ushort[] parchment) = squeak == null ? (null, null) : SecretMapPieceIcons(squeak);
            secretMapShades = parchment;
            int iconPalette = paletteBase + ReadInt(rom, icon + 4) * 32;
            int[] palettes = new int[4];
            for (int pair = 0; pair < 4; pair++) {
                palettes[pair] = Allocate(ref cursor, 32, 32, paletteBase);
                ushort[] colors = new ushort[16];
                colors[1] = (ushort)ReadShort(rom, iconPalette + 7 * 2);
                colors[2] = (ushort)ReadShort(rom, iconPalette + 9 * 2);
                colors[3] = Bgr555(AreaMapInk);
                if (parchment != null) Array.Copy(parchment, 0, colors, 4, 3);
                for (int k = 0; k < 4; k++) {
                    int map = (k / 2) * NidlAreaMaps + pair * 2 + k % 2;
                    colors[7 + 2 * k] = Bgr555(AreaMapThemes[map].light);
                    colors[8 + 2 * k] = Bgr555(AreaMapThemes[map].mid);
                }
                for (int i = 0; i < 16; i++) WriteShort(rom, palettes[pair] + i * 2, colors[i]);
            }
            (byte[][] scrollTiles, int[] scrollPalette, ushort[][] scrollColors) = squeak == null ? (null, null, null) : ScrollIcons(squeak);
            int iconVariants = Allocate(ref cursor, (pieceTiles == null ? AreaMapCount : PiecePopupVariant + SecretMapPieces) * 4, 4);
            for (int map = 0; map < AreaMapCount; map++) {
                int column = map % NidlAreaMaps, k = (map / NidlAreaMaps) * 2 + column % 2, commands = Allocate(ref cursor, 36, 4);
                WriteInts(rom, commands, -2, (palettes[column / 2] - paletteBase) / 32, 16, -1, (tileSets[k] - tilesBase) / 32, 4, 1, iconFrame, -4);
                WriteInt(rom, iconVariants + map * 4, commands + 0x08000000);
            }
            // (the pieces: piece p in column p, its pair's palette; KatAM's map's frame, a 16x16 sprite of 4 tiles);
            for (int piece = 0; pieceTiles != null && piece < SecretMapPieces; piece++) {
                int tiles = Allocate(ref cursor, 4 * 32, 32, tilesBase), commands = Allocate(ref cursor, 36, 4);
                pieceTiles[piece].CopyTo(rom, tiles);
                WriteInts(rom, commands, -2, (palettes[piece / 2] - paletteBase) / 32, 16, -1, (tiles - tilesBase) / 32, 4, 1, iconFrame, -4);
                WriteInt(rom, iconVariants + (AreaMapCount + piece) * 4, commands + 0x08000000);
                // (its chest popup: the same in colors 7-10);
                int popupTiles = Allocate(ref cursor, 4 * 32, 32, tilesBase), popup = Allocate(ref cursor, 24, 4);
                byte[] popupRemap = Enumerable.Range(0, 16).Select(i => (byte)(i switch { 1 => 7, 4 => 8, 5 => 9, 6 => 10, _ => 0 })).ToArray();
                for (int i = 0; i < 4 * 32; i++) rom[popupTiles + i] = (byte)(popupRemap[pieceTiles[piece][i] & 15] | popupRemap[pieceTiles[piece][i] >> 4] << 4);
                WriteInts(rom, popup, -1, (popupTiles - tilesBase) / 32, 4, 1, iconFrame, -4);
                WriteInt(rom, iconVariants + (PiecePopupVariant + piece) * 4, popup + 0x08000000);
            }
            // (the scrolls: scroll n its own tiles and its palette group's palette);
            if (pieceTiles != null) {
                int[] scrollPalettes = new int[scrollColors.Length];
                for (int group = 0; group < scrollColors.Length; group++) {
                    scrollPalettes[group] = Allocate(ref cursor, 32, 32, paletteBase);
                    for (int i = 0; i < 16; i++) WriteShort(rom, scrollPalettes[group] + i * 2, scrollColors[group][i]);
                }
                for (int n = 0; n < SqueakScrolls; n++) {
                    int tiles = Allocate(ref cursor, 4 * 32, 32, tilesBase), commands = Allocate(ref cursor, 36, 4);
                    scrollTiles[n].CopyTo(rom, tiles);
                    WriteInts(rom, commands, -2, (scrollPalettes[scrollPalette[n]] - paletteBase) / 32, 16, -1, (tiles - tilesBase) / 32, 4, 1, iconFrame, -4);
                    WriteInt(rom, iconVariants + (ScrollIconVariant + n) * 4, commands + 0x08000000);
                }
                scrollIconPalettes = scrollPalette;
            }
            animVariants.Add(iconVariants);
            animAttributes.Add(ReadInt(rom, attrTable + CollectionIconAnim * 4) - 0x08000000);
            animOams.Add(ReadInt(rom, oamTable + CollectionIconAnim * 4) - 0x08000000);
            return animId;
        }

        // (the pieces' parchment, light, mid and dark (their icons' colors 4-6, their popup's 8-10), this ROM's: AddAreaMapIcons());
        static ushort[] secretMapShades;
        // (the maps' anim's variants after the pieces': the scrolls' icons, then the pieces' popups; each scroll's palette group);
        const int ScrollIconVariant = AreaMapCount + SecretMapPieces, PiecePopupVariant = ScrollIconVariant + SqueakScrolls;
        static int[] scrollIconPalettes;
        /* (the OBJ palettes the scrolls' cells take, a palette group each: KatAM's sprays' own (4-9) not the maps' and cans' (4, 6, 7):
         * the scrolls' columns, the extra columns 5-12, show only once KatAM's spray columns are off the screen); */
        static readonly byte[] ScrollIconSlots = { 5, 8, 9 };
        // (the scrolls' names, in their order: "ROM Expansion/scrolls.c"'s, "• NAME SCROLL" in the Collection Room);
        static readonly string[] ScrollNames = { "FIRE", "ICE", "SPARK", "BEAM", "TORNADO", "PARASOL", "HAMMER", "CUPID", "CUTTER", "LASER",
                                                 "SWORD", "WHEEL", "HI JUMP", "UFO", "SLEEP", "BOMB", "NINJA", "FIGHTER", "THROW", "MAGIC",
                                                 "ANIMAL", "BUBBLE", "METAL" };

        /* ScrollIcons(); The Ability Scrolls' icons (Squeak Squad's collection/skill cells 0-22, 16x16 each, as Squeak Squad draws them,
         * unmirrored), each as 4 tiles of a 16x16 sprite (KatAM's order), and their colors packed in palettes (the colors every icon has
         * first, then each icon's own band, as many to a palette as fit): each icon's palette group and the groups' 16 colors; */
        static (byte[][] tiles, int[] palette, ushort[][] colors) ScrollIcons(byte[] squeak) {
            SqueakCharacter skill = ReadSqueakCharacter(squeak, "skill", "collection");
            if (skill.Anims.Count < SqueakScrolls) throw new InvalidOperationException("Squeak Squad's scroll icons aren't Squeak Squad's.");
            Dictionary<(int palette, int color), int> colorMap = new Dictionary<(int, int), int>();
            for (int p = 0; p < 16; p++) for (int k = 1; k < 16; k++) colorMap[(p, k)] = p * 16 + k;
            ushort[][,] icons = new ushort[SqueakScrolls][,];
            for (int n = 0; n < SqueakScrolls; n++) {
                int[,] canvas = SqueakCanvas(skill, skill.Anims[n].Steps[0].Cell, colorMap);
                int size = canvas.GetLength(0), left = size, top = size;
                for (int x = 0; x < size; x++)
                    for (int y = 0; y < size; y++)
                        if (canvas[x, y] != 0) { left = Math.Min(left, x); top = Math.Min(top, y); }
                icons[n] = new ushort[SecretMapIconSize, SecretMapIconSize];
                for (int x = 0; x < SecretMapIconSize; x++)
                    for (int y = 0; y < SecretMapIconSize; y++) {
                        // (SqueakCanvas() mirrors: the icon as Squeak Squad draws it; 0x8000 a drawn pixel, the color in the rest);
                        int sx = left + SecretMapIconSize - 1 - x, sy = top + y, v = sx < size && sy < size ? canvas[sx, sy] : 0;
                        if (v != 0) icons[n][x, y] = (ushort)(0x8000 | skill.Palettes[Math.Min(v / 16, skill.Palettes.Count - 1)][v % 16] & 0x7FFF);
                    }
            }
            HashSet<ushort>[] used = icons.Select(icon => icon.Cast<ushort>().Where(v => v != 0).ToHashSet()).ToArray();
            List<ushort> common = used.Skip(1).Aggregate(new HashSet<ushort>(used[0]), (all, set) => { all.IntersectWith(set); return all; }).OrderBy(v => v).ToList();
            List<HashSet<ushort>> groups = new List<HashSet<ushort>>();
            int[] palette = new int[SqueakScrolls];
            foreach (int n in Enumerable.Range(0, SqueakScrolls).OrderByDescending(n => used[n].Count).ThenBy(n => n)) {
                HashSet<ushort> band = used[n].Where(v => !common.Contains(v)).ToHashSet();
                int g = groups.FindIndex(group => group.Union(band).Count() <= 15 - common.Count);
                if (g < 0) { groups.Add(new HashSet<ushort>()); g = groups.Count - 1; }
                groups[g].UnionWith(band);
                palette[n] = g;
            }
            if (groups.Count > ScrollIconSlots.Length) throw new InvalidOperationException("Squeak Squad's scroll icons take too many palettes.");
            ushort[][] colors = groups.Select(group => new ushort[] { 0 }.Concat(common).Concat(group.OrderBy(v => v)).Concat(Enumerable.Repeat((ushort)0, 16))
                                                         .Take(16).Select(v => (ushort)(v & 0x7FFF)).ToArray()).ToArray();
            byte[][] tiles = new byte[SqueakScrolls][];
            for (int n = 0; n < SqueakScrolls; n++) {
                tiles[n] = new byte[4 * 32];
                for (int x = 0; x < SecretMapIconSize; x++)
                    for (int y = 0; y < SecretMapIconSize; y++) {
                        if (icons[n][x, y] == 0) continue;
                        int index = Array.IndexOf(colors[palette[n]], (ushort)(icons[n][x, y] & 0x7FFF), 1);
                        int tile = (y / 8) * 2 + x / 8;
                        tiles[n][tile * 32 + y % 8 * 4 + x % 8 / 2] |= (byte)(index << (x % 2 * 4));
                    }
            }
            return (tiles, palette, colors);
        }

        /* SecretMapPieceIcons(); The Secret Map's pieces' icons (Squeak Squad's collection/map cells 0-6, as Squeak Squad draws them,
         * unmirrored), each as 4 tiles of a 16x16 sprite (KatAM's order: the top row's two, then the bottom's): the piece shrunk to fit
         * 14x14 (a pixel drawn where half its area is), its shades in 3 (4 light, 5 mid, 6 dark: the brightness of all the pieces' pixels
         * in 3 groups, each group's color its pixels' average), an outline of 1 (black) around it; and the 3 colors; */
        static (byte[][] tiles, ushort[] shades) SecretMapPieceIcons(byte[] squeak) {
            SqueakCharacter map = ReadSqueakCharacter(squeak, "map", "collection");
            if (map.Anims.Count < SecretMapPieces) throw new InvalidOperationException("Squeak Squad's Secret Map isn't Squeak Squad's.");
            Dictionary<(int palette, int color), int> colorMap = new Dictionary<(int, int), int>();
            List<ushort> rgb = new List<ushort>();
            for (int p = 0; p < map.Palettes.Count; p++)
                for (int c = 1; c < 16; c++) { colorMap[(p, c)] = rgb.Count + 1; rgb.Add(map.Palettes[p][c]); }
            static double Brightness(ushort c) => 0.299 * (c & 31) + 0.587 * (c >> 5 & 31) + 0.114 * (c >> 10 & 31);

            // Each piece shrunk: its pixels' (brightness, r, g, b), or none where it's see-through;
            (double light, double r, double g, double b)?[][,] shrunk = new (double, double, double, double)?[SecretMapPieces][,];
            for (int piece = 0; piece < SecretMapPieces; piece++) {
                int[,] canvas = SqueakCanvas(map, map.Anims[piece].Steps[0].Cell, colorMap);
                int size = canvas.GetLength(0), left = size, right = -1, top = size, bottom = -1;
                for (int x = 0; x < size; x++)
                    for (int y = 0; y < size; y++)
                        if (canvas[x, y] != 0) { left = Math.Min(left, x); right = Math.Max(right, x); top = Math.Min(top, y); bottom = Math.Max(bottom, y); }
                if (right < 0) throw new InvalidOperationException("Squeak Squad's Secret Map isn't Squeak Squad's.");
                int width = right - left + 1, height = bottom - top + 1;
                double scale = Math.Max(1.0, Math.Max(width, height) / (double)SecretMapIconFit);
                int w = Math.Max(1, (int)Math.Round(width / scale)), h = Math.Max(1, (int)Math.Round(height / scale));
                shrunk[piece] = new (double, double, double, double)?[w, h];
                for (int tx = 0; tx < w; tx++)
                    for (int ty = 0; ty < h; ty++) {
                        int x0 = left + (int)(tx * scale), x1 = Math.Max(x0 + 1, left + (int)((tx + 1) * scale));
                        int y0 = top + (int)(ty * scale), y1 = Math.Max(y0 + 1, top + (int)((ty + 1) * scale));
                        int all = 0, drawn = 0; double light = 0, r = 0, g = 0, b = 0;
                        for (int x = x0; x < x1 && x <= right; x++)
                            for (int y = y0; y < y1 && y <= bottom; y++) {
                                all++;
                                // (SqueakCanvas() mirrors: the piece as Squeak Squad draws it);
                                int value = canvas[right - (x - left), y];
                                if (value == 0) continue;
                                ushort c = rgb[value - 1];
                                drawn++; light += Brightness(c); r += c & 31; g += c >> 5 & 31; b += c >> 10 & 31;
                            }
                        if (drawn * 2 >= all && drawn > 0) shrunk[piece][tx, ty] = (light / drawn, r / drawn, g / drawn, b / drawn);
                    }
            }

            // The 3 shades: the pixels' brightness in 3 groups (k-means, from the darkest, the middle and the brightest);
            var pixels = shrunk.SelectMany(s => s.Cast<(double light, double r, double g, double b)?>()).Where(p => p != null).Select(p => p.Value).ToList();
            List<double> sorted = pixels.Select(p => p.light).OrderBy(l => l).ToList();
            double[] centers = { sorted[0], sorted[sorted.Count / 2], sorted[^1] };
            int Group(double light) => Enumerable.Range(0, 3).OrderBy(k => Math.Abs(light - centers[k])).First();
            for (int round = 0; round < 16; round++)
                for (int k = 0; k < 3; k++) {
                    List<double> members = pixels.Where(p => Group(p.light) == k).Select(p => p.light).ToList();
                    if (members.Count > 0) centers[k] = members.Average();
                }
            // (4 the brightest, 6 the darkest);
            int[] order = Enumerable.Range(0, 3).OrderByDescending(k => centers[k]).ToArray();
            ushort[] shades = new ushort[3];
            for (int i = 0; i < 3; i++) {
                var members = pixels.Where(p => Group(p.light) == order[i]).ToList();
                if (members.Count == 0) throw new InvalidOperationException("Squeak Squad's Secret Map isn't Squeak Squad's.");
                shades[i] = (ushort)((int)Math.Round(members.Average(p => p.r)) | (int)Math.Round(members.Average(p => p.g)) << 5 | (int)Math.Round(members.Average(p => p.b)) << 10);
            }

            byte[][] tiles = new byte[SecretMapPieces][];
            for (int piece = 0; piece < SecretMapPieces; piece++) {
                int w = shrunk[piece].GetLength(0), h = shrunk[piece].GetLength(1), ox = (SecretMapIconSize - w) / 2, oy = (SecretMapIconSize - h) / 2;
                byte[,] icon = new byte[SecretMapIconSize, SecretMapIconSize];
                for (int x = 0; x < w; x++)
                    for (int y = 0; y < h; y++)
                        if (shrunk[piece][x, y] is var (light, _, _, _)) icon[ox + x, oy + y] = (byte)(4 + Array.IndexOf(order, Group(light)));
                // (the outline: each see-through pixel next to the piece);
                byte[,] outlined = (byte[,])icon.Clone();
                for (int x = 0; x < SecretMapIconSize; x++)
                    for (int y = 0; y < SecretMapIconSize; y++)
                        if (icon[x, y] == 0 && new[] { (1, 0), (-1, 0), (0, 1), (0, -1) }.Any(d => x + d.Item1 >= 0 && y + d.Item2 >= 0 &&
                                x + d.Item1 < SecretMapIconSize && y + d.Item2 < SecretMapIconSize && icon[x + d.Item1, y + d.Item2] != 0))
                            outlined[x, y] = 1;
                tiles[piece] = new byte[4 * 32];
                for (int x = 0; x < SecretMapIconSize; x++)
                    for (int y = 0; y < SecretMapIconSize; y++) {
                        int tile = y / 8 * 2 + x / 8;
                        tiles[piece][tile * 32 + y % 8 * 4 + x % 8 / 2] |= (byte)(outlined[x, y] << (x % 2 * 4));
                    }
            }
            return (tiles, shades);
        }

        /* SqueakMapChests(); Each Squeak Squad world's map's chest (its room's file and the chest's record in its .doe's items): of the
         * chests SqueakObjects() makes (no chase spawn, none a gimmick holds), out of a boss room, holding a consumable (a treasure the
         * owner's list doesn't bring: SqueakTreasures' item ones keep theirs), a big one first, the deepest (its stage, its room); */
        static Dictionary<(SqueakRoomFiles room, int record), int> SqueakMapChests(byte[] squeak, List<SqueakRoomFiles> files) {
            Dictionary<int, (bool big, int stage, int index, SqueakRoomFiles room, int record)> best = new Dictionary<int, (bool, int, int, SqueakRoomFiles, int)>();
            foreach (SqueakRoomFiles f in files) {
                if (f.Area < 1 || f.Area > AreaMapCount - NidlAreaMaps || f.Objects == null) continue;
                byte[] objects = SqueakFileAnyCase(squeak, f.Objects);
                if (objects == null || objects.Length < 8 || SqueakFights(squeak, f).boss) continue;
                (HashSet<int> heldItems, _) = SqueakHeldRecords(new SqueakMap(SqueakUnpack(SqueakFileAnyCase(squeak, f.Map))));
                int items = ReadShort(objects, 6);
                for (int i = 0, count = items + 2 <= objects.Length ? ReadShort(objects, items) : 0; i < count; i++) {
                    int at = items + 2 + i * 16, kind = ReadShort(objects, at + 2), treasure = objects[at + 0xD];
                    if ((objects[at + 0xB] & SqueakChaseSpawn) != 0 || heldItems.Contains(i) || kind is not (SqueakChestSmall or SqueakChestBig)) continue;
                    if (SqueakTreasures.Any(t => t.item && treasure >= t.first && treasure <= t.last)) continue;
                    var candidate = (big: kind == SqueakChestBig, stage: f.Stage, index: f.Index, room: f, record: i);
                    if (!best.TryGetValue(f.Area, out var held) || (candidate.big, candidate.stage, candidate.index).CompareTo((held.big, held.stage, held.index)) > 0)
                        best[f.Area] = candidate;
                }
            }
            return best.ToDictionary(e => (e.Value.room, e.Value.record), e => NidlAreaMaps + e.Key - 1);
        }
        // (the Squeak Squad worlds' maps whose chests SqueakObjects() made, this ROM's, and their rooms' stage files: AddSqueakRooms() starts it afresh);
        static readonly Dictionary<int, SqueakRoomFiles> squeakMapsPlaced = new Dictionary<int, SqueakRoomFiles>();
        internal static IReadOnlyDictionary<int, SqueakRoomFiles> SqueakMapRooms => squeakMapsPlaced;
        // (the Secret Map's pieces whose chests SqueakObjects() made, the same);
        static readonly Dictionary<int, SqueakRoomFiles> squeakPiecesPlaced = new Dictionary<int, SqueakRoomFiles>();
        internal static IReadOnlyDictionary<int, SqueakRoomFiles> SecretMapPieceRooms => squeakPiecesPlaced;
        /* (Squeak Squad's Ability Scrolls, treasures 28-50 (the owner: "Implement all Ability Scrolls in their respective chests to add
         * this data to the game already"): each in its own chest as Squeak Squad has it, the chest's template's unk16
         * CHEST_MARK_SCROLL << 8 | the scroll's number + 1, its item one of KatAM's maps for its popup (chest.c's NidlChestOpen gives the
         * scroll: owned in the save's scroll bits, "ROM Expansion/rooms.c"'s SqueakScrollOwned(), not KatAM's map; its popup the DS's
         * icon of that scroll, chest.c's NidlScrollPopup); what a scroll does: "ROM Expansion/scrolls.c"; its cell in the Collection
         * Room: AddAreaMaps(); the ones SqueakObjects() placed, the same as the pieces); */
        internal const int SqueakScrolls = 23, ChestMarkScroll = 3;
        // (a scroll's cell's id in the Collection Room: category 0x40, collection.c's GridOwned);
        const int ScrollItemId = 0x4000;
        const int SqueakScrollFirst = 28;
        static readonly Dictionary<int, SqueakRoomFiles> squeakScrollsPlaced = new Dictionary<int, SqueakRoomFiles>();
        internal static IReadOnlyDictionary<int, SqueakRoomFiles> AbilityScrollRooms => squeakScrollsPlaced;

        /* AddAreaMaps(); The rest (after the anims tables are copied): gNidlMaps (the maps' shades, then the Secret Map's pieces'), and
         * the maps placed (present: the NiDL levels', the Squeak Squad worlds') and the Secret Map's pieces placed (piecesPresent) as the
         * Collection Room grid's cells (their icons, their texts). Returns an error, or null; */
        static string AddAreaMaps(byte[] rom, byte[] squeak, KatAMExpansionCode.Version version, int iconAnim, bool[] present, bool[] piecesPresent,
                                  bool[] scrollsPresent, List<CollectionItem> items, List<(int tileset, int tilemap)[]> texts, ref int cursor) {
            int colors = Allocate(ref cursor, (AreaMapCount + 1) * 6, 4);
            for (int map = 0; map < AreaMapCount; map++) {
                WriteShort(rom, colors + map * 6, Bgr555(AreaMapThemes[map].light));
                WriteShort(rom, colors + map * 6 + 2, Bgr555(AreaMapThemes[map].mid));
                WriteShort(rom, colors + map * 6 + 4, Bgr555(AreaMapThemes[map].dark));
            }
            for (int i = 0; i < 3 && secretMapShades != null; i++) WriteShort(rom, colors + AreaMapCount * 6 + i * 2, secretMapShades[i]);
            WriteInt(rom, CodeAddress + version.CodeSymbols["gNidlMaps"], colors + 0x08000000);
            // (the pieces' chest popups: the maps' anim and the first piece's popup variant, chest.c's NidlPiecePopupStart());
            if (secretMapShades != null) WriteShorts(rom, CodeAddress + version.CodeSymbols["gNidlMaps"] + 4, iconAnim, PiecePopupVariant);
            int cells = version.RomSymbols["gUnk_08386A50"];
            int[] mapTexts = new int[10];
            for (int cell = 0; cell < CollectionCells; cell++) {
                int id = ReadShort(rom, cells + cell * 4);
                if (id >> 8 == 2 && (id & 0xFF) < 10) mapTexts[id & 0xFF] = rom[cells + cell * 4 + 3];
            }
            if (mapTexts.Distinct().Count() != 10) return $"The {version.Name} ROM's Collection Room isn't KatAM's.";
            int[] maps = Enumerable.Range(0, AreaMapCount).Where(m => present[m]).ToArray();
            int firstText = texts.Count;
            texts.AddRange(ComposeCollectionTexts(rom, version, mapTexts, 1, CollectionNames(rom, squeak, version, maps.Select(m => AreaMapNames[m]).ToArray()), ref cursor));
            for (int i = 0; i < maps.Length; i++) {
                int map = maps[i], column = map % NidlAreaMaps;
                items.Add(new CollectionItem(2 + map / NidlAreaMaps, column, 0x200 + AreaMapBit + map, iconAnim, map, AreaMapPaletteSlots[column / 2],
                                             firstText + i, mapTexts[1]));
            }
            // (the Secret Map's pieces: the second row, piece p in column p, one text for all, "• SECRET MAP PIECE");
            if (secretMapShades != null && piecesPresent.Any(placed => placed)) {
                int pieceText = texts.Count;
                texts.AddRange(ComposeCollectionTexts(rom, version, mapTexts, 1, CollectionNames(rom, squeak, version, new[] { SecretMapName }), ref cursor));
                for (int piece = 0; piece < SecretMapPieces; piece++)
                    if (piecesPresent[piece])
                        items.Add(new CollectionItem(1, piece, 0x2000 + piece, iconAnim, AreaMapCount + piece, AreaMapPaletteSlots[piece / 2], pieceText, mapTexts[1]));
            }
            /* (the Ability Scrolls placed: after the others, the first row's columns 5-12 then the other rows' 8-12, scroll by scroll, each
             * "• NAME SCROLL" with nothing under it (the maps' "use it to..." line isn't theirs); collection.c's GridOwned asks
             * SqueakScrollOwned(): owned in the expansion's save); */
            if (scrollIconPalettes != null && scrollsPresent.Any(placed => placed)) {
                (int row, int column)[] scrollCells = Enumerable.Range(5, 8).Select(c => (0, c))
                    .Concat(Enumerable.Range(1, 3).SelectMany(r => Enumerable.Range(8, 5).Select(c => (r, c)))).ToArray();
                int scrollText = texts.Count;
                texts.AddRange(ComposeCollectionTexts(rom, version, mapTexts, 1, CollectionNames(rom, squeak, version, ScrollNames.Select(n => n + " SCROLL").ToArray()),
                                                      ref cursor, keepBelow: false));
                for (int n = 0; n < SqueakScrolls; n++)
                    if (scrollsPresent[n])
                        items.Add(new CollectionItem(scrollCells[n].row, scrollCells[n].column, ScrollItemId + n, iconAnim, ScrollIconVariant + n,
                                                     ScrollIconSlots[scrollIconPalettes[n]], scrollText + n, mapTexts[1]));
            }
            return null;
        }
    }
}
