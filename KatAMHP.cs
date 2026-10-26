using KatAMInternal;

namespace KatAMRandomizer {
    /* KatAMHP; The HP options (the Enemies, Mini-Bosses and Bosses tabs): each one's HP stays, is shuffled among them or is picked
     * between Min HP and Max HP, then the Percentage HP Modifier (-50% to 50%) goes on top of that when it's on.
     * • KatAM's HP is in its object tables: the regular enemies' in gUnk_08351648 (their properties, which KatAMROMReader reads from
     *   the ROM and KatAMPropertiesManagement writes back); types 0x38-0x52's (mini-bosses and bosses) by the number of players in
     *   gUnk_08351530 (1-4), Dark Mind's first form's by its part too (gUnk_08351608, subtype2). The HP bar's fill takes a factor per
     *   type (0x4000 / its HP: gUnk_083513E8, gUnk_08351458 and gUnk_08351628), changed to match. The bosses' moves that change
     *   under half their HP read the same tables, so they keep doing so. (The tables at the Japanese release's addresses, as the
     *   base randomizer keeps its own: RomRegion.Parameters.)
     * • The ROM Expansion's ported enemies, mini-bosses and bosses have their own tables ("ROM Expansion/nidl.c", nidl_miniboss.c,
     *   nidl_boss.c: gNidlEnemyHp, gNidlMinibossHp, gNidlBossHp by the number of players), the HP bar working their fill out from them
     *   (NidlHpBarFill); with its code in the ROM they join KatAM's in the same pools.
     * • Mini-bosses' and bosses' HP is shuffled and picked for one player: with more, each one's is scaled as the game scales it (KatAM's
     *   Mr. Frosty: 30, 40, 48 and 54); NiDL's mini-bosses have one HP for any number of players; */
    internal static class KatAMHP {
        // KatAM's tables (the Japanese release's addresses);
        const long RegularFill = 0x335BBC,      // gUnk_083513E8: the HP bar's fill factor per type (u16);
                   BossFill = 0x335C2C,         // gUnk_08351458: types 0x38-0x52's fill factors, by the number of players (u16[4]);
                   BossHP = 0x335D04,           // gUnk_08351530: types 0x38-0x52's HP, by the number of players (s16[4]);
                   DarkMindHP = 0x335DDC,       // gUnk_08351608: Dark Mind's first form's HP, 4 parts by the number of players (s16[4][4]);
                   DarkMindFill = 0x335DFC;     // gUnk_08351628: its fill factors (u16[4][4]);
        const byte FirstBossType = 0x38, DarkMindForm1 = 0x4F;
        const int MaxHP = 9999;

        /* The enemies whose HP stays: those no hit hurts (Gordo, the Shotzos), what isn't one (0x2C; 0x30, Dark Mind's bombs; 0x31, the ROM
         * Expansion's enemies, their HP their own; 0xA2, the parasol) and 0x37 (a Waddle Dee the game treats as Shadow Kirby, with the
         * mini-bosses' 30); */
        static readonly HashSet<byte> fixedEnemies = new HashSet<byte>() { 0x06, 0x2C, 0x30, 0x31, 0x35, 0x37, 0x70, 0xA2 };

        /* The ROM Expansion's: its enemies (gNidlEnemyHp's first 15, NIDL_BROOM_HATTER to NIDL_GIP: the rest are Whispy Woods' apples,
         * stars, Paint Roller's paintings), its mini-bosses (gNidlMinibossHp's first 5: the Meta-Knights' is what their knights have left)
         * and its bosses (gNidlBossHp, s16[4] each); */
        const int NidlEnemies = 15, NidlMinibosses = 5, NidlBosses = 9;

        static short Read16(byte[] rom, long address) => (short)(rom[address] | (rom[address + 1] << 8));
        static void Write16(long address, int value) => Utils.WriteToROM(address, BitConverter.GetBytes((ushort)value));

        // Fill(); The HP bar's fill factor for an HP (0x4000 full);
        static int Fill(int hp) => 0x4000 / Math.Max(hp, 1);

        // Changes(); Whether the options change any HP;
        static bool Changes(GenerationOptions type, bool percentage, int percent) =>
            type == GenerationOptions.Shuffle || type == GenerationOptions.Random || (percentage && percent != 0);

        // NewHP(); The HP of each, as the options say;
        static List<int> NewHP(List<int> hp, GenerationOptions type, int min, int max, bool percentage, int percent) {
            List<int> result = type switch {
                GenerationOptions.Shuffle => Utils.Shuffle(hp),
                GenerationOptions.Random => hp.Select(_ => Utils.GetRandomRange(min, max + 1)).ToList(),
                _ => new List<int>(hp),
            };

            if (percentage) result = result.Select(value => (int)Math.Round(value * (100 + percent) / 100.0, MidpointRounding.AwayFromZero)).ToList();
            return result.Select(value => Math.Clamp(value, 1, MaxHP)).ToList();
        }

        // Scaled(); A row of HP by the number of players with its one player's HP made `hp`, the others scaled alike;
        static int[] Scaled(short[] row, int hp) =>
            row.Select((value, players) => players == 0 ? hp : Math.Clamp((int)Math.Round((double)value * hp / Math.Max(row[0], (short)1), MidpointRounding.AwayFromZero), 1, MaxHP)).ToArray();

        static short[] Row(byte[] rom, long address) => Enumerable.Range(0, 4).Select(players => Read16(rom, address + players * 2)).ToArray();

        /* Enemies(); The enemies' HP: KatAM's in their properties (written with the rest of them: `modified`), each type's HP bar fill with
         * it; the ROM Expansion's in its table; */
        public static void Enemies(Dictionary<byte, Properties> properties, List<Properties> modified) {
            Settings settings = Settings.GetInstance();
            if (!Changes(settings.EnemiesHPType, settings.isEnemyHPPercentageModified, settings.EnemiesHPPercentage)) return;

            byte[] rom = Processing.ROMData;
            List<byte> types = Processing.enemiesDictionary.Keys.Where(type => !fixedEnemies.Contains(type) && properties.ContainsKey(type)).ToList();
            int nidl = KatAMExpansion.CodeSymbolOf(rom, "gNidlEnemyHp");
            List<int> hp = types.Select(type => (int)properties[type].HP).ToList();

            if (nidl >= 0) hp.AddRange(Enumerable.Range(0, NidlEnemies).Select(i => (int)Read16(rom, nidl + i * 2)));
            List<int> newHP = NewHP(hp, settings.EnemiesHPType, settings.EnemiesHPMin, settings.EnemiesHPMax, settings.isEnemyHPPercentageModified, settings.EnemiesHPPercentage);

            for (int i = 0; i < types.Count; i++) {
                Properties property = properties[types[i]];

                // (a byte in the properties, as the game's HP is);
                property.HP = (byte)Math.Min(newHP[i], byte.MaxValue);
                if (!modified.Contains(property)) modified.Add(property);
                Write16(RomRegion.Current.Parameters(RegularFill) + types[i] * 2, Fill(property.HP));
            }
            if (nidl >= 0)
                for (int i = 0; i < NidlEnemies; i++) Write16(nidl + i * 2, newHP[types.Count + i]);
        }

        // Minibosses(); The mini-bosses' HP: KatAM's (types 0x38-0x3F) by the number of players, with their HP bar's fill; the ROM Expansion's;
        public static void Minibosses() {
            Settings settings = Settings.GetInstance();
            if (!Changes(settings.MinibossesHPType, settings.isMinibossHPPercentageModified, settings.MinibossesHPPercentage)) return;

            byte[] rom = Processing.ROMData;
            long table = RomRegion.Current.Parameters(BossHP);
            List<byte> types = Processing.minibossesDictionary.Keys.ToList();
            List<short[]> rows = types.Select(type => Row(rom, table + (type - FirstBossType) * 8)).ToList();
            int nidl = KatAMExpansion.CodeSymbolOf(rom, "gNidlMinibossHp");
            List<int> hp = rows.Select(row => (int)row[0]).ToList();

            if (nidl >= 0) hp.AddRange(Enumerable.Range(0, NidlMinibosses).Select(i => (int)Read16(rom, nidl + i * 2)));
            List<int> newHP = NewHP(hp, settings.MinibossesHPType, settings.MinibossesHPMin, settings.MinibossesHPMax, settings.isMinibossHPPercentageModified, settings.MinibossesHPPercentage);

            for (int i = 0; i < types.Count; i++) WriteRow(types[i], Scaled(rows[i], newHP[i]));
            if (nidl >= 0)
                for (int i = 0; i < NidlMinibosses; i++) Write16(nidl + i * 2, newHP[types.Count + i]);
        }

        /* Bosses(); The bosses' HP: KatAM's (types 0x45-0x52; Dark Mind's first form, all its parts scaled alike) by the number of players,
         * with their HP bar's fill; the ROM Expansion's by the number of players; */
        public static void Bosses() {
            Settings settings = Settings.GetInstance();
            if (!Changes(settings.BossesHPType, settings.isBossHPPercentageModified, settings.BossesHPPercentage)) return;

            byte[] rom = Processing.ROMData;
            long table = RomRegion.Current.Parameters(BossHP), darkMind = RomRegion.Current.Parameters(DarkMindHP);
            List<byte> types = Processing.bossesDictionary.Keys.Where(type => type != DarkMindForm1).ToList();
            List<short[]> rows = types.Select(type => Row(rom, table + (type - FirstBossType) * 8)).ToList();
            List<short[]> darkMindRows = Enumerable.Range(0, 4).Select(part => Row(rom, darkMind + part * 8)).ToList();
            int nidl = KatAMExpansion.CodeSymbolOf(rom, "gNidlBossHp");
            List<short[]> nidlRows = nidl < 0 ? new List<short[]>() : Enumerable.Range(0, NidlBosses).Select(i => Row(rom, nidl + i * 8)).ToList();
            List<int> hp = rows.Concat(darkMindRows.Take(1)).Concat(nidlRows).Select(row => (int)row[0]).ToList();
            List<int> newHP = NewHP(hp, settings.BossesHPType, settings.BossesHPMin, settings.BossesHPMax, settings.isBossHPPercentageModified, settings.BossesHPPercentage);

            for (int i = 0; i < types.Count; i++) WriteRow(types[i], Scaled(rows[i], newHP[i]));

            // (Dark Mind's first form: its parts' HP as its first part's for one player, scaled alike);
            int first = newHP[types.Count];
            for (int part = 0; part < 4; part++) {
                int[] row = Scaled(darkMindRows[part], (int)Math.Round((double)darkMindRows[part][0] * first / Math.Max(darkMindRows[0][0], (short)1), MidpointRounding.AwayFromZero));
                for (int players = 0; players < 4; players++) {
                    int value = Math.Clamp(row[players], 1, MaxHP);
                    Write16(darkMind + part * 8 + players * 2, value);
                    Write16(RomRegion.Current.Parameters(DarkMindFill) + part * 8 + players * 2, Fill(value));
                }
            }

            for (int i = 0; i < nidlRows.Count; i++) {
                int[] row = Scaled(nidlRows[i], newHP[types.Count + 1 + i]);
                for (int players = 0; players < 4; players++) Write16(nidl + i * 8 + players * 2, row[players]);
            }
        }

        // WriteRow(); A mini-boss's or boss's HP by the number of players (gUnk_08351530), and its HP bar's fill (gUnk_08351458);
        static void WriteRow(byte type, int[] row) {
            for (int players = 0; players < 4; players++) {
                Write16(RomRegion.Current.Parameters(BossHP) + (type - FirstBossType) * 8 + players * 2, row[players]);
                Write16(RomRegion.Current.Parameters(BossFill) + (type - FirstBossType) * 8 + players * 2, Fill(row[players]));
            }
        }
    }
}
