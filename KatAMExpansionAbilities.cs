namespace KatAMRandomizer {
    /* KatAMExpansion's abilities: NiDL's Needle, Freeze, Light, Ball, Hi-Jump, Mike and Backdrop, as KatAM abilities 27-33, and its Star
     * Rod, 34, which only the Power Orb's fight gives (see "ROM Expansion/abilities.h").
     * • Every table KatAM indexes by ability is copied to the expansion with 34 entries (AbilityTables), and the game's code (literal pools
     *   in 0x08000000-0x08160000) points to the copy. The new entries are gNidlAbilityTables', or the base ability's.
     * • The ability field is widened to 6 bits (WidenAbilityField); swallowing (sub_080547C4) is detoured to NidlSwallow;
     * • sub_080356AC (HUD portraits) and sub_08088640 (special portraits) are detoured to the C code (see Detour), and sub_0804A728 (Kirby
     *   hit), so the Star Rod isn't lost to a hit, DisplaySprite (every sprite's drawing), so the Star Rod's hat glows as NiDL's
     *   (NidlDisplaySprite, AddGlows()), and sub_08038B34 (Kirbys standing on one another), so the Star Rod's flying Kirbys fly through
     *   one another (NidlKirbyStack); */
    internal static partial class KatAMExpansion {
        const int FirstAbility = 27, NewAbilities = 8, AbilitySlots = FirstAbility + NewAbilities, PortraitTiles = 30;

        // NiDL's IDs of the new abilities (Needle, Freeze, Light, Ball, Hi-Jump, Mike, Backdrop, Star Rod), for their cards;
        static readonly int[] NightmareAbilities = { 12, 14, 21, 18, 15, 7, 22, 25 };

        // gNidlAbilityTables' fields: base (a byte each, padded), then the B on land, B in water and setup functions;
        const int LandBField = (NewAbilities + 3) & ~3, WaterBField = LandBField + NewAbilities * 4, SetupField = WaterBField + NewAbilities * 4;

        /* The relocated tables: KatAM symbol, entry size, entries, gNidlAbilityTables field (offset of its function pointers; -1: none), and
         * the ability whose entry the new ones copy (-1: their base ability's); */
        static readonly (string symbol, int entrySize, int entries, int field, int template)[] AbilityTables = {
            ("gUnk_0834C2AC", 4, 27, LandBField, -1),   // B on land;
            ("gUnk_0834C318", 4, 27, WaterBField, -1),  // B in water;
            ("gUnk_083509D0", 4, 27, SetupField, -1),   // Setup (body animations);
            ("gUnk_08350A3C", 4, 28, -1, -1),           // Body palette anim;
            ("gUnk_08D60FDC", 4, 27, -1, -1),           // Hats (AnimInfo per animation index);
            ("gUnk_082DDE84", 4, 27, -1, -1),           // CPU helpers' AI;
            ("gUnk_082DDEF0", 4, 27, -1, -1),
            ("gUnk_082DDF5C", 4, 27, -1, -1),
            ("gUnk_0834C4B0", 4, 33, -1, 1),            // State after getting an ability (Normal's is its own);
            ("gUnk_08350AAC", 4, 33, -1, 0),            // Pause world map: body;
            ("gUnk_08350B30", 4, 32, -1, 0),            // Pause world map: hat (Normal's: none);
        };

        const int CodeEnd = 0x160000;

        /* AddAbilities(); Relocates the ability tables, detours the HUD portraits and fills gNidlAbilityData. `newEntries[table]`, when
         * given, replaces the base ability's entries of AbilityTables[table] (hats, Backdrop's palette; -1: the base's); `motions`, BuildMotions' body AnimInfos;
         * `glows`, the Star Rod's glowing frames (anim, first frame, count: its hat's, its motions', its stand's; AddGlows()), then its
         * palettes (StarRodPalettes()), then the Hi-Jump's glow (HiJumpFlash()) and its effects' anim (HiJumpEffects); */
        static string AddAbilities(byte[] rom, byte[] nightmare, NightmareRegion region, KatAMExpansionCode.Version version, ref int cursor,
                                   int[] motions, int[] glows, Dictionary<string, int[]> newEntries) {
            int codeBase = CodeAddress + 0x08000000;
            int tables = CodeAddress + version.CodeSymbols["gNidlAbilityTables"], data = CodeAddress + version.CodeSymbols["gNidlAbilityData"];

            foreach ((string symbol, int entrySize, int entries, int field, int template) in AbilityTables) {
                int old = version.RomSymbols[symbol];
                int copy = Allocate(ref cursor, AbilitySlots * entrySize, 4);

                Array.Copy(rom, old, rom, copy, Math.Min(entries, FirstAbility) * entrySize);
                for (int k = 0; k < NewAbilities; k++) {
                    int value = ReadInt(rom, old + (template >= 0 ? template : rom[tables + k]) * entrySize);
                    if (newEntries.TryGetValue(symbol, out int[] given) && given[k] != -1) value = given[k];
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

            /* Swallowing (sub_080547C4) takes 0x1B as "a random ability" and 0x1C as "no ability" (special objects, sub_0809C48C): they're
             * now NIDL_SWALLOW_RANDOM (0x3B) and NIDL_SWALLOW_NONE (0x3C), out of the new abilities' way, for NidlSwallow. Object type 0xCE
             * is the only one whose ability is 0x1B; */
            int special = version.RomSymbols["SwallowSpecialSite"], randomType = version.RomSymbols["gUnk_08351648"] + 0xCE * 0x18 + 6;
            if (ReadShort(rom, special) != 0x211C || ReadShort(rom, randomType) != 0x1B) return $"The {version.Name} ROM's swallowing code doesn't match.";
            WriteShort(rom, special, 0x213C);
            WriteShort(rom, randomType, 0x3B);
            if (Detour(rom, version.RomSymbols["sub_080547C4"], codeBase + version.CodeSymbols["NidlSwallow"], ref cursor) < 0
                || !WidenAbilityField(rom)) return $"The {version.Name} ROM's swallowing code doesn't match.";

            // Kirby hit (sub_0804A728): the Star Rod isn't lost to a hit (NidlKirbyHit, "ROM Expansion/abilities.c");
            int hit = Detour(rom, version.RomSymbols["sub_0804A728"], codeBase + version.CodeSymbols["NidlKirbyHit"], ref cursor);
            if (hit < 0) return $"The {version.Name} ROM's hit code doesn't match.";
            WriteInt(rom, CodeAddress + version.CodeSymbols["gNidlKirbyHitOriginal"], hit + 0x08000000 + 1);

            // Kirbys standing on one another (sub_08038B34): not the Star Rod's flying Kirbys (NidlKirbyStack, "ROM Expansion/abilities/star_rod.c");
            int stack = Detour(rom, version.RomSymbols["sub_08038B34"], codeBase + version.CodeSymbols["NidlKirbyStack"], ref cursor);
            if (stack < 0) return $"The {version.Name} ROM's Kirby collision code doesn't match.";
            WriteInt(rom, CodeAddress + version.CodeSymbols["gNidlKirbyStackOriginal"], stack + 0x08000000 + 1);

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

            // The motions' body animations (gNidlAbilityData.motions, after the portraits);
            for (int i = 0; i < motions.Length; i++) WriteInt(rom, data + 8 + NewAbilities * 8 + i * 4, motions[i]);

            /* The Star Rod's glow: DisplaySprite's trampoline, the glowing frames, the palettes (gNidlAbilityData.displaySpriteOriginal, glows and
             * rodPalettes, after the motions); then the Hi-Jump's glow and effects' anim (hiJumpFlash, hiJumpFx); */
            int display = Detour(rom, version.RomSymbols["DisplaySprite"], codeBase + version.CodeSymbols["NidlDisplaySprite"], ref cursor), after = data + 8 + NewAbilities * 8 + motions.Length * 4;
            if (display < 0) return $"The {version.Name} ROM's sprite code doesn't match.";
            WriteInt(rom, after, display + 0x08000000 + 1);
            for (int i = 0; i < 9; i++) WriteShort(rom, after + 4 + i / 3 * 8 + i % 3 * 2, glows[i]);
            WriteShort(rom, after + 4 + 8 + 6, glows[12]);    // (the motions' `plain`: the jump's spin's first frame);
            WriteInt(rom, after + 28, glows[9] + 0x08000000);
            WriteInt(rom, after + 32, glows[10] + 0x08000000);
            WriteShort(rom, after + 36, glows[11]);
            return null;
        }

        /* WidenAbilityField(); transitioningAbility's ability goes from 5 bits to 6 (see "ROM Expansion/abilities.h"), the same in every
         * version (found by their code):
         * • every "transitioningAbility & 0x1F" (adds/movs rY, #0xDD, then movs rZ, #0x1F within 5 instructions): 18 of them -> 0x3F;
         * • the random flag (0x20 -> 0x40): sub_0805C11C's test, sub_08068CA4's two sets ("orrs r0, rX" with 0x20 -> adds r0, #0x40);
         * • the spat star's "several" flag (sub_080725E0): read from unkF1 (+0xF1) instead of transitioningAbility (+0xDD); */
        static bool WidenAbilityField(byte[] rom) {
            List<int> masks = new List<int>();
            for (int i = 0; i < CodeEnd; i += 2) {
                int op = ReadShort(rom, i);
                if ((op & 0xF8FF) != 0x30DD && (op & 0xF8FF) != 0x20DD) continue;
                for (int k = 1; k < 6; k++) {
                    if ((ReadShort(rom, i + k * 2) & 0xF8FF) != 0x201F) continue;
                    masks.Add(i + k * 2);
                    break;
                }
            }
            int random = FindCode(rom, 0x7812, 0x2020, 0x4010, 0x2800, -1, 0x201F, 0x4010, 0x0200),
                set1 = FindCode(rom, 0x211F, 0x4008, 0x4330, 0x1C29, 0x31DD), set2 = FindCode(rom, 0x4008, 0x4649, 0x4308, 0x4661, 0x7008),
                star = FindCode(rom, 0x1C32, 0x32DD, 0x7811, 0x2040, 0x4008, 0x1C14);
            if (masks.Count != 18 || random < 0 || set1 < 0 || set2 < 0 || star < 0) return false;

            foreach (int at in masks) WriteShort(rom, at, (ReadShort(rom, at) & 0xFF00) | 0x3F);
            WriteShort(rom, random + 2, 0x2040);
            WriteShort(rom, set1 + 4, 0x3040);
            WriteShort(rom, set2 + 4, 0x3040);
            WriteShort(rom, star + 2, 0x32F1);
            return true;
        }

        // FindCode(); The one place in the game's code with these halfwords (-1: any), or -1 (none, or more than one);
        static int FindCode(byte[] rom, params int[] halfwords) {
            int found = -1;
            for (int i = 0; i + halfwords.Length * 2 <= CodeEnd; i += 2) {
                bool match = true;
                for (int k = 0; k < halfwords.Length && match; k++)
                    match = halfwords[k] < 0 || ReadShort(rom, i + k * 2) == halfwords[k];
                if (!match) continue;
                if (found >= 0) return -1;
                found = i;
            }
            return found;
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
         * (checked for code that depends on where it is; literal loads get a copy of their literal), then a jump to the rest. Returns
         * the trampoline's offset, or -1; */
        static int Detour(byte[] rom, int function, int target, ref int cursor) {
            if ((function & 3) != 0) return -1;
            List<int> literals = new List<int>();
            for (int i = 0; i < 8; i += 2) {
                int op = BitConverter.ToUInt16(rom, function + i);
                if ((op & 0xF800) == 0x4800) { literals.Add(i); continue; }
                if ((op & 0xF000) == 0xD000 || (op & 0xF800) == 0xE000 || (op & 0xE000) == 0xE000
                    || (op & 0xF800) == 0xA000 || (op & 0xFF00) == 0x4700) return -1;
            }

            int trampoline = Allocate(ref cursor, 16 + literals.Count * 4, 4);
            Array.Copy(rom, function, rom, trampoline, 8);
            WriteShorts(rom, trampoline + 8, 0x4B00, 0x4718);    // ldr r3, [pc, #0]; bx r3;
            WriteInt(rom, trampoline + 12, function + 8 + 0x08000000 + 1);
            for (int j = 0; j < literals.Count; j++) {
                int i = literals[j], op = BitConverter.ToUInt16(rom, function + i), copy = trampoline + 16 + j * 4;
                WriteInt(rom, copy, ReadInt(rom, ((function + i + 4) & ~3) + (op & 0xFF) * 4));
                WriteShort(rom, trampoline + i, (op & 0xFF00) | ((copy - ((trampoline + i + 4) & ~3)) / 4));
            }

            WriteShorts(rom, function, 0x4B00, 0x4718);
            WriteInt(rom, function + 4, target + 1);
            return trampoline;
        }
    }
}
