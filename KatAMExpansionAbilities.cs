namespace KatAMRandomizer {
    /* KatAMExpansion's abilities: NiDL's Needle, Freeze, Light, Ball and Hi-Jump, as KatAM abilities 27-31 (see "ROM Expansion/abilities.h").
     * • Every table KatAM indexes by ability with 27 entries is copied to the expansion with 32 (AbilityTables), and the game's code
     *   (literal pools in 0x08000000-0x08160000) points to the copy. The new entries are gNidlAbilityTables', or the base ability's.
     * • sub_080356AC (HUD portraits) and sub_08088640 (special portraits) are detoured to the C code (see Detour);
     * • World map icons (gUnk_08350AAC body, gUnk_08350B30 hat) already have entries 27-31: normal body, no hat; */
    internal static partial class KatAMExpansion {
        const int FirstAbility = 27, NewAbilities = 5, AbilitySlots = 32, PortraitTiles = 30;

        // NiDL's IDs of the new abilities (Needle, Freeze, Light, Ball, Hi-Jump), for their cards;
        static readonly int[] NightmareAbilities = { 12, 14, 21, 18, 15 };

        // The relocated tables: KatAM symbol, entry size, entries, gNidlAbilityTables field (offset of its 5 pointers; -1: none);
        static readonly (string symbol, int entrySize, int entries, int field)[] AbilityTables = {
            ("gUnk_0834C2AC", 4, 27, 8),     // B on land;
            ("gUnk_0834C318", 4, 27, 28),    // B in water;
            ("gUnk_083509D0", 4, 27, 48),    // Setup (body animations);
            ("gUnk_08350A3C", 4, 28, -1),    // Hat palette anim;
            ("gUnk_08D60FDC", 4, 27, -1),    // Hats (AnimInfo per animation index);
            ("gUnk_082DDE84", 4, 27, -1),    // CPU helpers' AI;
            ("gUnk_082DDEF0", 4, 27, -1),
            ("gUnk_082DDF5C", 4, 27, -1),
        };

        const int CodeEnd = 0x160000;

        /* AddAbilities(); Relocates the ability tables, detours the HUD portraits and fills gNidlAbilityData. `newEntries[table]`, when
         * given, replaces the base ability's entries of AbilityTables[table] (hats, palettes); */
        static string AddAbilities(byte[] rom, byte[] nightmare, NightmareRegion region, KatAMExpansionCode.Version version, ref int cursor,
                                   Dictionary<string, int[]> newEntries) {
            int codeBase = CodeAddress + 0x08000000;
            int tables = CodeAddress + version.CodeSymbols["gNidlAbilityTables"], data = CodeAddress + version.CodeSymbols["gNidlAbilityData"];

            foreach ((string symbol, int entrySize, int entries, int field) in AbilityTables) {
                int old = version.RomSymbols[symbol];
                int copy = Allocate(ref cursor, AbilitySlots * entrySize, 4);

                Array.Copy(rom, old, rom, copy, Math.Min(entries, FirstAbility) * entrySize);
                for (int k = 0; k < NewAbilities; k++) {
                    int value = ReadInt(rom, old + rom[tables + k] * entrySize);
                    if (newEntries.TryGetValue(symbol, out int[] given)) value = given[k];
                    else if (field >= 0 && ReadInt(rom, tables + field + k * 4) != 0) value = ReadInt(rom, tables + field + k * 4);
                    WriteInt(rom, copy + (FirstAbility + k) * entrySize, value);
                }
                if (Repoint(rom, old + 0x08000000, copy + 0x08000000) == 0) return $"The {version.Name} ROM's ability table {symbol} isn't where it should be.";
            }

            // HUD portraits;
            int portrait = Detour(rom, version.RomSymbols["sub_080356AC"], codeBase + version.CodeSymbols["NidlAbilityPortrait"], ref cursor),
                hudTimer = Detour(rom, version.RomSymbols["sub_08088640"], codeBase + version.CodeSymbols["NidlAbilityHudTimer"], ref cursor);
            if (portrait < 0 || hudTimer < 0) return $"The {version.Name} ROM's HUD code doesn't match.";
            WriteInt(rom, data, portrait + 0x08000000 + 1);
            WriteInt(rom, data + 4, hudTimer + 0x08000000 + 1);

            /* Swallowing (sub_080547C4) takes 0x1B as "a random ability" and 0x1C as "no ability" (special objects, sub_0809C48C):
             * they're now 0x3B (0x1B | KIRBY_ABILITY_CHANGE_RANDOM, which it ORs in anyway) and 0x3C, out of Needle's and Freeze's way.
             * Object type 0xCE is the only one whose ability is 0x1B; */
            int swallow = version.RomSymbols["sub_080547C4"], special = version.RomSymbols["SwallowSpecialSite"];
            int randomType = version.RomSymbols["gUnk_08351648"] + 0xCE * 0x18 + 6;
            if (ReadShort(rom, swallow + 0x74) != 0x2E1C || ReadShort(rom, swallow + 0x11C) != 0x2E1B || ReadShort(rom, special) != 0x211C
                || ReadShort(rom, randomType) != 0x1B) return $"The {version.Name} ROM's swallowing code doesn't match.";
            WriteShort(rom, swallow + 0x74, 0x2E3C);
            WriteShort(rom, swallow + 0x11C, 0x2E3B);
            WriteShort(rom, special, 0x213C);
            WriteShort(rom, randomType, 0x3B);

            /* Portraits: NiDL's ability cards, the same 5x6 tiles as KatAM's (the name, then the picture), stored as two groups of 16 tiles
             * (rows 0, 2, 4, then 1, 3, 5); KatAM's are row by row; */
            for (int k = 0; k < NewAbilities; k++) {
                int palette = ReadInt(nightmare, region.CardPalettes + NightmareAbilities[k] * region.CardPaletteStride) - 0x08000000;
                int tiles = ReadInt(nightmare, region.CardTiles + NightmareAbilities[k] * region.CardTileStride) - 0x08000000;
                int paletteAt = Allocate(ref cursor, 32, 4), tilesAt = Allocate(ref cursor, PortraitTiles * 32, 4);

                Array.Copy(nightmare, palette, rom, paletteAt, 32);
                for (int row = 0; row < 6; row++)
                    for (int column = 0; column < 5; column++)
                        Array.Copy(nightmare, tiles + ((row / 2) * 5 + column + (row % 2) * 16) * 32, rom, tilesAt + (row * 5 + column) * 32, 32);
                WriteInt(rom, data + 8 + k * 8, paletteAt + 0x08000000);
                WriteInt(rom, data + 12 + k * 8, tilesAt + 0x08000000);
            }
            return null;
        }

        // Repoint(); Points the code's literals of `from` (in 0x08000000-0x08160000) to `to`; how many there were;
        static int Repoint(byte[] rom, int from, int to) {
            int count = 0;

            for (int i = 0; i < CodeEnd; i += 4) {
                if (ReadInt(rom, i) != from) continue;
                WriteInt(rom, i, to);
                count++;
            }
            return count;
        }

        /* Detour(); Makes the Thumb function at `function` (4-aligned) jump to `target` (ldr r3, =target; bx r3: r3 is free at the
         * entry of the functions detoured, which take 3 arguments) and builds a trampoline running the original: its first 8 bytes
         * (checked for code that depends on where it is), then a jump to the rest. Returns the trampoline's offset, or -1; */
        static int Detour(byte[] rom, int function, int target, ref int cursor) {
            if ((function & 3) != 0) return -1;
            for (int i = 0; i < 8; i += 2) {
                int op = BitConverter.ToUInt16(rom, function + i);
                if ((op & 0xF800) == 0x4800 || (op & 0xF000) == 0xD000 || (op & 0xF800) == 0xE000 || (op & 0xE000) == 0xE000
                    || (op & 0xF800) == 0xA000 || (op & 0xFF00) == 0x4700) return -1;
            }

            int trampoline = Allocate(ref cursor, 16, 4);
            Array.Copy(rom, function, rom, trampoline, 8);
            WriteShorts(rom, trampoline + 8, 0x4B00, 0x4718);    // ldr r3, [pc, #0]; bx r3;
            WriteInt(rom, trampoline + 12, function + 8 + 0x08000000 + 1);

            WriteShorts(rom, function, 0x4B00, 0x4718);
            WriteInt(rom, function + 4, target + 1);
            return trampoline;
        }
    }
}
