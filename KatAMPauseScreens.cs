using KatAMInternal;

namespace KatAMRandomizer {
    /* KatAMPauseScreens; Every ability's pause help screen is replaced by the base one ("About Copy Abilities"), and the graphics of
     * the others (every ability, in every language) are wiped, about 480 KB in every version, to free room for gameplay additions.
     * The pause screen still shows the current ability's portrait and name (the HUD's, which stay), and keeps both frames (the solo
     * one and the multiplayer one, sHelpMenuUnkTiledBGsIndices[language][0 and 1]) and the SELECT "call" button after the tutorial
     * (a sprite). The European version keeps the English base screen for every language; the Japanese one keeps the Japanese one.
     * • sHelpMenuUnkTiledBGsIndices[6 languages][0x20] (USA 0x08358C14): [2 + ability] is each ability's screen (an index of
     *   gUnk_082D7850); every one of them becomes the kept screen.
     * • gUnk_082D7850 (USA 0x082D7850): pointers to each background ({ u16 width, height, unk4; u8 unk6, unk7; tileset (LZ77),
     *   tilesetCompressedSize, palette, u16 paletteOffset, paletteSize, tilemap }, 0x1C bytes), each stored as palette (when it
     *   has one), tileset, tilemap and the record itself, in one block. The dropped ones point to the kept record, and their blocks
     *   are filled with 0xFF. The unused entries (abilities 27-29, sharing one record) are left alone.
     * • CreateHelpMenu (USA 0x08124430) loads the screen of gKirbys[gLocalPlayerId].ability and then the portrait of
     *   (screen - row[2]); it now loads row[2] and the portrait of the ability itself, so abilities past the table (new ones) work
     *   too. Patched at AbilityLoad (the ldrb of the ability, which becomes movs r0, #0) and the 0x14 bytes from AbilityLoad + 0x18
     *   (the portrait's argument), which read the ability again with CreateHelpMenu's own literals (gKirbys at AbilityLoad - 0x12,
     *   the ability's offset 0x103 at AbilityLoad - 4);
     *
     * The area maps go too (DropAreaMaps(), with the help screens): SELECT on the pause screen goes from the help screen to the world
     * map, the big map of all areas, never to an area map (the Map Viewer shows each area's rooms), and the data only the area maps
     * use is wiped (filled with 0xFF): 62,190 bytes in the USA version, 62,088 in the Japanese one, 78,213 in the European one (five
     * languages of area titles):
     * • PauseMenuMain (src/pause_help.c) picks MENU_AREAMAP (4) from the help screen when Kirby is in an area's room; AreaMapChoice
     *   is its movs r2, #4, which becomes movs r2, #2 (MENU_WORLDMAP). Nothing else ever picks the area map (gPauseMenus' menuId
     *   starts as the help screen), so CreateAreaMap never runs.
     * • The backgrounds behind them (gUnk_082D7850's records 0xB7-0xC0, one per area from the tutorial's: palette, tileset, tilemap
     *   and the record, in one block each; the world map's is 0xB6 and stays): wiped, their entries pointing to the world map's.
     * • Their room layouts (gAreaMapRoomsTilemapOffsets and the run-length tilemaps after it, up to sAreaMapPalettePulseOffsets),
     *   the area map's frame (sAreaMapUITilemap, LZ77), its area titles (gAreaMapUIAreaTitleTilesets' LZ77 tilesets; the European
     *   version's table has them for 5 languages, with the world map frame's tileset of each language between them, which stays)
     *   and its shard and map room tiles (gAreaMapShardTileset and gAreaMapMapRoomTileset, 0x800 bytes).
     * • Kept: the room squares, lines and arrows (gAreaMapRoomsTileset), their palette (gAreaMapRoomsPalette) and colors
     *   (sAreaMapPalettePulseStates), which the Map Viewer draws its map with (MapGraphics), the world map's frame (sMapUITileset,
     *   sMapUIPalette, sWorldMapUITilemap, gMapUITilemapIndices), the area maps' room list (sAreaMapRoomInfos: GetKirbyRoomFlagIndex
     *   tells the world map where Kirby is with it) and every sprite; */
    internal static class KatAMPauseScreens {
        sealed record Version(string GameCode, int AbilityLoad, int Indices, int Backgrounds, ushort Kept, int AreaMapChoice, int RoomTilemaps,
                              int PulseOffsets, int AreaMapFrame, int AreaTitles, int AreaTitleCount, int MapRoomTiles);

        static readonly Version[] Versions = {
            new Version("B8KE", 0x1246C2, 0x358C14, 0x2D7850, 0x20, 0x124D64, 0x35EECC, 0x3610E8, 0x361E14, 0xD61248, 9, 0xD612E4),
            new Version("B8KP", 0x12470A, 0x35D6F4, 0x2D96F0, 0x20, 0x124DF8, 0x3639AC, 0x365BC8, 0x3668F4, 0xD6B7C8, 45, 0xD6B8F4),
            new Version("B8KJ", 0x1245CA, 0x33D3D0, 0x2D8758, 0x02, 0x124C60, 0x343688, 0x3458A4, 0x3465D0, 0xD2C6C0, 9, 0xD2C75C),
        };

        const int Languages = 6, RowLength = 0x20, FirstScreen = 2, LastScreen = 0xB5, RecordSize = 0x1C;

        // The area maps': the world map's background record and the areas' after it, the 11 room layout offsets, the shard and map room tiles;
        const int WorldMapBackground = 0xB6, AreaBackgrounds = 10, RoomTilemapCount = 11, MapRoomTilesSize = 0x800;
        const ushort PickAreaMap = 0x2204, PickWorldMap = 0x2202;  // movs r2, #4 (MENU_AREAMAP); movs r2, #2 (MENU_WORLDMAP);

        // Original CreateHelpMenu code at AbilityLoad (ldrb r0, [r0]; adds r0, #2; lsls r0, #1; adds r0, r7; adds r0, r4; ldrh r1, [r0]);
        static readonly byte[] Original = { 0x00, 0x78, 0x02, 0x30, 0x40, 0x00, 0xC0, 0x19, 0x00, 0x19, 0x01, 0x88 };

        // Apply(); Points every pause help screen to the base one and wipes the rest. Does nothing on an unknown or already patched ROM;
        public static void Apply(byte[] rom) {
            Version version = Versions.FirstOrDefault(v => v.GameCode == System.Text.Encoding.ASCII.GetString(rom, 0xAC, 4));

            if (version == null || !rom.AsSpan(version.AbilityLoad, Original.Length).SequenceEqual(Original)) return;

            PatchCode(rom, version.AbilityLoad);

            for (int language = 0; language < Languages; language++)
                for (int screen = FirstScreen; screen < RowLength; screen++)
                    WriteU16(rom, version.Indices + (language * RowLength + screen) * 2, version.Kept);

            int kept = ReadPointer(rom, version.Backgrounds + version.Kept * 4),
                unused = ReadPointer(rom, version.Backgrounds + 0x1D * 4);

            for (int screen = FirstScreen; screen <= LastScreen; screen++) {
                int entry = version.Backgrounds + screen * 4, record = ReadPointer(rom, entry);

                if (record == kept || record == unused) continue;

                int tileset = ReadPointer(rom, record + 8), palette = ReadPointer(rom, record + 16);
                ushort paletteSize = ReadU16(rom, record + 22);
                int start = paletteSize > 0 ? Math.Min(tileset, palette) : tileset;

                WriteU32(rom, entry, (uint)(kept + 0x8000000));
                rom.AsSpan(start, record + RecordSize - start).Fill(0xFF);
            }
        }

        static void PatchCode(byte[] rom, int at) {
            int kirbys = Literal(rom, at - 0x12), offset = Literal(rom, at - 4), block = at + 0x18;

            WriteU16(rom, at, 0x2000);                                      // movs r0, #0 (screen: row[2]);

            ushort[] code = {
                LoadLiteral(2, block, kirbys),                              // ldr r2, =gKirbys;
                0x7829,                                                     // ldrb r1, [r5] (gLocalPlayerId);
                0x23D4, 0x005B,                                             // movs r3, #0xD4; lsls r3, r3, #1 (sizeof(struct Kirby));
                0x434B,                                                     // muls r3, r1;
                0x18D2,                                                     // adds r2, r2, r3;
                LoadLiteral(3, block + 12, offset),                         // ldr r3, =0x103;
                0x5CD2,                                                     // ldrb r2, [r2, r3] (the ability);
                0x2101,                                                     // movs r1, #1;
                0x46C0,                                                     // nop;
            };

            for (int i = 0; i < code.Length; i++) WriteU16(rom, block + i * 2, code[i]);
        }

        /* DropAreaMaps(); SELECT goes from the help screen to the world map, and the area maps' own data is wiped. Does nothing on an
         * unknown or already patched ROM, or one whose area map data isn't where it should be; returns the bytes freed; */
        public static int DropAreaMaps(byte[] rom) {
            Version version = Versions.FirstOrDefault(v => v.GameCode == System.Text.Encoding.ASCII.GetString(rom, 0xAC, 4));

            if (version == null || ReadU16(rom, version.AreaMapChoice) != PickAreaMap || BitConverter.ToUInt32(rom, version.RoomTilemaps) != 0
                || BitConverter.ToUInt32(rom, version.RoomTilemaps + (RoomTilemapCount - 1) * 4) >= version.PulseOffsets - version.RoomTilemaps
                || rom[version.AreaMapFrame] != 0x10) return 0;

            List<(int start, int end)> wiped = new List<(int start, int end)>();
            int world = ReadPointer(rom, version.Backgrounds + WorldMapBackground * 4);

            for (int area = 0; area < AreaBackgrounds; area++) {
                int entry = version.Backgrounds + (WorldMapBackground + 1 + area) * 4, record = ReadPointer(rom, entry),
                    start = Math.Min(ReadPointer(rom, record + 8), Math.Min(ReadPointer(rom, record + 16), ReadPointer(rom, record + 24)));

                if (start < 0 || record - start > 0x4000) return 0;
                wiped.Add((start, record + RecordSize));
            }
            wiped.Add((version.RoomTilemaps, version.PulseOffsets));
            wiped.Add((version.AreaMapFrame, version.AreaMapFrame + Lz77Length(rom, version.AreaMapFrame)));
            foreach (int title in Enumerable.Range(0, version.AreaTitleCount).Select(i => ReadPointer(rom, version.AreaTitles + i * 4)).Distinct()) {
                if (rom[title] != 0x10) return 0;
                wiped.Add((title, title + Lz77Length(rom, title)));
            }
            wiped.Add((version.MapRoomTiles, version.MapRoomTiles + MapRoomTilesSize));

            WriteU16(rom, version.AreaMapChoice, PickWorldMap);
            for (int area = 0; area < AreaBackgrounds; area++) WriteU32(rom, version.Backgrounds + (WorldMapBackground + 1 + area) * 4, (uint)(world + 0x8000000));
            foreach ((int start, int end) in wiped) rom.AsSpan(start, end - start).Fill(0xFF);

            return wiped.Sum(w => w.end - w.start);
        }

        // Lz77Length(); The size of the LZ77 data (the GBA BIOS's, type 0x10) at the given address;
        static int Lz77Length(byte[] rom, int at) {
            int size = rom[at + 1] | (rom[at + 2] << 8) | (rom[at + 3] << 16), p = at + 4, written = 0;

            while (written < size) {
                int flags = rom[p++];

                for (int bit = 0; bit < 8 && written < size; bit++) {
                    if ((flags & (0x80 >> bit)) != 0) {
                        written += (rom[p] >> 4) + 3;
                        p += 2;
                    } else {
                        written++;
                        p++;
                    }
                }
            }
            return p - at;
        }

        // Literal(); Address of the literal a Thumb "ldr rd, [pc, #imm]" at the given address loads;
        static int Literal(byte[] rom, int at) => ((at + 4) & ~3) + (rom[at] * 4);

        static ushort LoadLiteral(int register, int at, int literal) => (ushort)(0x4800 | (register << 8) | ((literal - ((at + 4) & ~3)) / 4));

        static ushort ReadU16(byte[] rom, int at) => BitConverter.ToUInt16(rom, at);
        static int ReadPointer(byte[] rom, int at) => (int)(BitConverter.ToUInt32(rom, at) - 0x8000000);
        static void WriteU16(byte[] rom, int at, ushort value) => BitConverter.GetBytes(value).CopyTo(rom, at);
        static void WriteU32(byte[] rom, int at, uint value) => BitConverter.GetBytes(value).CopyTo(rom, at);
    }
}
