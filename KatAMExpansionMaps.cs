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
     * Dreams is all boss rooms (no chest there): its map waits for the owner's call. Their look: KatAM's map, its body in the area's
     * colors (AreaMapThemes): rising out of the chest (KatAM's popup, drawn with the chest's palette, its colors 8-10 the area's 3
     * shades, gNidlMaps.colors: chest.c's NidlMapPopup), and in the Collection Room's grid (collection.c: the NiDL maps the third row after
     * KatAM's columns, the Squeak Squad ones the fourth; anim 0x388's map with the area's light and mid shades and a dark shared by
     * all, 4 maps to an OBJ palette, AreaMapPalettes), named "• AREA" over KatAM's map text ("Use this for navigation."); */
    internal static partial class KatAMExpansion {
        internal const int AreaMapCount = 16, AreaMapBit = 10, ChestMarkMap = 1, NidlAreaMaps = 8;
        // (the map whose chest isn't placed: the Fountain of Dreams', all boss rooms);
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

        // AreaMapItem(); The KatAM map (1-9) a chest holding map m gives for a moment;
        internal static byte AreaMapItem(int map) => (byte)(0xA + 1 + map % 9);

        /* AddAreaMapIcons(); The maps' anim in the Collection Room (before the anims tables are copied), returned: variant m map m,
         * KatAM's map, its tiles drawing 1 black, 2 the dots, 3 the dark, 7 + 2k and 8 + 2k the light and mid of the palette's k-th map
         * (0, 1 a NiDL map, 2, 3 a Squeak Squad one), 4 palettes of 4 maps (a column pair each); 4-6 left for the Secret Map's pieces; */
        static int AddAreaMapIcons(byte[] rom, int anims, int attrTable, int oamTable, int paletteBase, int tilesBase, ref int cursor,
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
            int iconPalette = paletteBase + ReadInt(rom, icon + 4) * 32;
            int[] palettes = new int[4];
            for (int pair = 0; pair < 4; pair++) {
                palettes[pair] = Allocate(ref cursor, 32, 32, paletteBase);
                ushort[] colors = new ushort[16];
                colors[1] = (ushort)ReadShort(rom, iconPalette + 7 * 2);
                colors[2] = (ushort)ReadShort(rom, iconPalette + 9 * 2);
                colors[3] = Bgr555(AreaMapInk);
                for (int k = 0; k < 4; k++) {
                    int map = (k / 2) * NidlAreaMaps + pair * 2 + k % 2;
                    colors[7 + 2 * k] = Bgr555(AreaMapThemes[map].light);
                    colors[8 + 2 * k] = Bgr555(AreaMapThemes[map].mid);
                }
                for (int i = 0; i < 16; i++) WriteShort(rom, palettes[pair] + i * 2, colors[i]);
            }
            int iconVariants = Allocate(ref cursor, AreaMapCount * 4, 4);
            for (int map = 0; map < AreaMapCount; map++) {
                int column = map % NidlAreaMaps, k = (map / NidlAreaMaps) * 2 + column % 2, commands = Allocate(ref cursor, 36, 4);
                WriteInts(rom, commands, -2, (palettes[column / 2] - paletteBase) / 32, 16, -1, (tileSets[k] - tilesBase) / 32, 4, 1, iconFrame, -4);
                WriteInt(rom, iconVariants + map * 4, commands + 0x08000000);
            }
            animVariants.Add(iconVariants);
            animAttributes.Add(ReadInt(rom, attrTable + CollectionIconAnim * 4) - 0x08000000);
            animOams.Add(ReadInt(rom, oamTable + CollectionIconAnim * 4) - 0x08000000);
            return animId;
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

        /* AddAreaMaps(); The rest (after the anims tables are copied): gNidlMaps (the maps' shades), and the maps placed (present: the
         * NiDL levels', the Squeak Squad worlds') as the Collection Room grid's cells (their icons, their texts). Returns an error, or null; */
        static string AddAreaMaps(byte[] rom, byte[] squeak, KatAMExpansionCode.Version version, int iconAnim, bool[] present,
                                  List<CollectionItem> items, List<(int tileset, int tilemap)[]> texts, ref int cursor) {
            int colors = Allocate(ref cursor, AreaMapCount * 6, 4);
            for (int map = 0; map < AreaMapCount; map++) {
                WriteShort(rom, colors + map * 6, Bgr555(AreaMapThemes[map].light));
                WriteShort(rom, colors + map * 6 + 2, Bgr555(AreaMapThemes[map].mid));
                WriteShort(rom, colors + map * 6 + 4, Bgr555(AreaMapThemes[map].dark));
            }
            WriteInt(rom, CodeAddress + version.CodeSymbols["gNidlMaps"], colors + 0x08000000);
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
            return null;
        }
    }
}
