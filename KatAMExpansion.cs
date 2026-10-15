using KatAMInternal;
using System.Text;

namespace KatAMRandomizer {
    /* KatAMExpansion; ROM Expansion: enemies ported from Kirby: Nightmare in Dream Land into Kirby & the Amazing Mirror, any version
     * (USA, European, Japanese revision 1), with the NiDL ROM of the same region: USA with USA, European with European, Japanese with
     * Japanese. The European version shows the English names in every language; the Japanese one shows none (its names are images
     * of Japanese text).
     * The behaviors are re-implemented in C against the KatAM decompilation (see "ROM Expansion/" for the sources and how they're
     * built) and ship compiled in KatAMExpansionCode. The graphics are never shipped: they are read from the player's own NiDL ROM.
     * Every ported enemy is object type 0x31 (the only free regular enemy type); the template's behavior byte picks the enemy
     * (high nibble) and its variant (low nibble). See "ROM Expansion/nidl.h" for how they share it.
     * Layout: the ROM is expanded from 16 MB to 32 MB (the most the GBA maps, 0x08000000-0x09FFFFFF), and everything new goes in
     * the new half, so the expansion has room for every enemy and leaves the game's own free space alone:
     * • 0x1000000 (0x09000000): compiled code, then the converted graphics, animations, names and the extended sprite tables.
     * • gSpriteTables (USA 0x3B909C): anims, attrs and oamData are copied with more entries: the first free anim (USA 939) is the
     *   shared hurt frame, the next ones are the enemies' frames (one anim per enemy, one variant per frame).
     * • Object type 0x31: spawn function and parameters; its AnimInfo table holds every enemy's frames.
     * • HP bar names: the two copies of an enemy's name (USA 0x08034ED2, 0x08035B34) call NidlCopyName instead.
     * • The hit flash keeps the enemy's own palette (USA 0x080857EE), see AddEnemies.
     * Every address differs between versions: KatAMExpansionCode.Versions has each one's (and its own build of the code); */
    internal static partial class KatAMExpansion {
        public const byte NightmareID = 0x31;

        const int ExpandedSize = 0x2000000, CodeAddress = 0x1000000, ReservedSize = ExpandedSize - CodeAddress;

        /* NiDL: graphics descriptor per enemy (type - 8): tile count (+2), palette (+8), LZ77 tiles (+12). The frame tables (and every
         * NiDL address in the enemies' notes) are the USA ones; the Japanese and European ROMs have the same data, moved by FrameShift; */
        // Palette15: NiDL's shared object palette 15 (the level loads it; the Poppy Bros. Jr.'s ball and bomb are drawn with it);
        sealed record NightmareRegion(string GameCode, int Descriptors, int FrameShift, string KatAM, int Palette15);

        static readonly NightmareRegion[] NightmareRegions = {
            new NightmareRegion("A7KE", 0x73EEA0, 0, "USA", 0x0D07A8),
            new NightmareRegion("A7KJ", 0x742DAC, 0x3A8C, "Japan", 0x0CFF60),   // Hoshi no Kirby: Yume no Izumi Deluxe;
            new NightmareRegion("A7KP", 0x97B678, 0x23C7EC, "Europe", 0x0D3228),
        };

        static NightmareRegion RegionOf(byte[] rom) => rom == null || rom.Length < 0x800000 ? null
            : NightmareRegions.FirstOrDefault(region => Encoding.ASCII.GetString(rom, 0xAC, 4) == region.GameCode);

        /* HP bar name copies patched to call NidlCopyName (see ApplyNamePatch), from where they start (the version's NameSite1 and 2):
         * their length (up to the end of their bl CpuSet), where their literal for the name table is, and whether they have the
         * object's type pointer in ip (else the object in r2); */
        static readonly (int length, int literal, bool fromTypePointer)[] NameCopies = { (28, 0x52, true), (30, 0x48, false) };

        // VersionOf(); The version of a KatAM ROM the ROM Expansion has a build for, or null;
        static KatAMExpansionCode.Version VersionOf(byte[] rom) => rom == null || rom.Length < 0x1000000 ? null
            : KatAMExpansionCode.Versions.FirstOrDefault(v => Encoding.ASCII.GetString(rom, 0xAC, 4) == v.GameCode && rom[0xBC] == v.Revision);

        // Part; One NiDL frame drawn into a frame, with its colors remapped (NiDL colors of other palettes go to unused ones);
        // Part; A NiDL frame (from the enemy's frame table, or Table when the enemy has more than one), recolored;
        sealed record Part(int Frame, Dictionary<int, int> Recolor = null, int Table = 0, bool Palette15 = false);

        /* Enemy; A ported enemy: its index (NIDL_* in nidl.h), name, NiDL object type and frame table, its frames (in the order the
         * C code numbers them), its hurt frame, palette changes, the letters of its name and the behavior bytes the randomizer uses; */
        sealed record Enemy(int Index, string Name, int Type, int FrameTable, List<Part[]> Frames, Part[] Hurt,
                            Dictionary<int, ushort> PaletteChanges, (int type, int letter)[] Glyphs, byte[] Behaviors, byte[] FlyingBehaviors = null,
                            byte[] WaterBehaviors = null);

        public static readonly IReadOnlyList<(int index, string name)> Enemies;

        static readonly List<Enemy> Definitions = new List<Enemy> {
            BroomHatter(),
            Cappy(),
            Twizzy(),
            Bubbles(),
            Slippy(),
            Kabu(),
            Coconut(),
            Coner(),
            Starman(),
            Needlous(),
            BladeKnight(),
            Chilly(),
            CoolSpook(),
            PoppyBrosJr(),
            Gip(),
        };

        static KatAMExpansion() {
            Enemies = Definitions.Select(e => (e.Index, e.Name)).ToList();
        }

        // Broom Hatter; Frames: NiDL's body frames 4-7 with the broom dust (none, 8, 9, 10) on top. The dust is white in NiDL's
        // palette 15: it's drawn with color 6 of the Broom Hatter's palette, which it doesn't use. All 3 NiDL variants: 0 and 1 sweep
        // around (1 with other timing), 2 sweeps in place;
        static Enemy BroomHatter() {
            Dictionary<int, int> dust = new Dictionary<int, int> { [14] = 6 };
            List<Part[]> frames = new List<Part[]>();

            for (int body = 4; body <= 7; body++)
                foreach (int d in new[] { -1, 8, 9, 10 })
                    frames.Add(d < 0 ? new[] { new Part(body) } : new[] { new Part(body), new Part(d, dust) });

            return new Enemy(0, "Broom Hatter", 0x0B, 0x7521D8, frames, new[] { new Part(3) }, new Dictionary<int, ushort> { [6] = 0x7FFF },
                new[] {
                    (0x01, 0), (0x01, 1), (0x01, 2), (0x01, 2), (0x28, 2), (-1, -1),   // "Bro" (Bronto Burt), "o", "m" (Bomber), space;
                    (0x14, 0), (0x14, 5), (0x14, 2), (0x14, 2), (0x14, 4), (0x01, 1),  // "H", "a", "t", "t", "e" (Hot Head), "r";
                },
                new byte[] { 0x00, 0x01, 0x02 });
        }

        // Cappy; Frames: NiDL's 4 (standing, it hops turning around), the capless Cappy's 4, 6 and 5 (frame table 0x0875237C), and
        // NiDL's 1 (the cap alone). Variant 1 is the capless one, variant 2 hops in place; inhaled, variant 0 loses its cap and a
        // capless one springs out;
        static Enemy Cappy() => new Enemy(1, "Cappy", 0x10, 0x75235C,
            new List<Part[]> {
                new[] { new Part(4) }, new[] { new Part(4, null, 0x75237C) }, new[] { new Part(6, null, 0x75237C) }, new[] { new Part(5, null, 0x75237C) },
                new[] { new Part(1) },
            }, new[] { new Part(3) },
            new Dictionary<int, ushort>(),
            new[] { (0x0C, 0), (0x19, 2), (0x19, 1), (0x19, 1), (0x19, 5) },   // "C" (Cupie), "a", "p", "p", "y" (Sparky);
            new byte[] { 0x00, 0x01, 0x02 });

        // Twizzy; Frames: NiDL's 4, 5 (flapping) and 6 (sitting). All 10 NiDL variants: 0-4 and 8 fly (swooping, swooping after
        // Kirby, ambushing from above, bouncing diagonally, chasing, bobbing), 5-7 and 9 start on the ground;
        static Enemy Twizzy() => new Enemy(2, "Twizzy", 0x18, 0x752560, new List<Part[]> { new[] { new Part(4) }, new[] { new Part(5) }, new[] { new Part(6) } },
            new[] { new Part(3) }, new Dictionary<int, ushort>(),
            new[] { (0x1C, 0), (0x1C, 1), (0x1C, 2), (0x35, 4), (0x35, 4), (0x05, 5) },   // "Twi" (Twister), "z", "z" (Shotzo), "y" (Scarfy);
            new byte[] { 0x05, 0x06, 0x07, 0x09 }, new byte[] { 0x00, 0x01, 0x02, 0x03, 0x04, 0x08 });

        // Bubbles; Frames: NiDL's 4 to 21 (spinning 4-12, squashed 13-21). Variant 0 bounces around, variant 1 in place;
        static Enemy Bubbles() => new Enemy(3, "Bubbles", 0x1E, 0x7526A8, Enumerable.Range(4, 18).Select(f => new[] { new Part(f) }).ToList(),
            new[] { new Part(0) }, new Dictionary<int, ushort>(),
            new[] { (0x28, 0), (0x0C, 1), (0x18, 5), (0x18, 6), (0x18, 7), (0x18, 8), (0x04, 4) },   // "B" (Bomber), "u" (Cupie), "bble" (Sir Kibble), "s" (Squishy);
            new byte[] { 0x00, 0x01 });

        // Slippy; Frames: NiDL's 4 (sitting), 5 (blinking, falling), 6 (top of a jump), 8 (jumping), 7 (swimming), 9 (paddling). It
        // jumps around on land and swims in water (variant 0, either place); variant 1 sits;
        static Enemy Slippy() => new Enemy(4, "Slippy", 0x22, 0x752828, new[] { 4, 5, 6, 8, 7, 9 }.Select(f => new[] { new Part(f) }).ToList(),
            new[] { new Part(3) }, new Dictionary<int, ushort>(),
            new[] { (0x04, 0), (0x02, 1), (0x02, 2), (0x02, 3), (0x02, 4), (0x05, 5) },   // "S" (Squishy), "lipp" (Blipper), "y" (Scarfy);
            new byte[] { 0x00, 0x01 }, WaterBehaviors: new byte[] { 0x00 });

        // Kabu; Frames: NiDL's 4 to 12 (spinning 4-11, sitting 12). It spins and jumps around;
        static Enemy Kabu() => new Enemy(5, "Kabu", 0x15, 0x752438, Enumerable.Range(4, 9).Select(f => new[] { new Part(f) }).ToList(),
            new[] { new Part(3) }, new Dictionary<int, ushort>(),
            new[] { (0x18, 3), (0x14, 5), (0x18, 5), (0x0C, 1) },   // "K" (Sir Kibble), "a" (Hot Head), "b" (Sir Kibble), "u" (Cupie);
            new byte[] { 0x00, 0x01, 0x02, 0x03 });

        // Coconut; Frames: NiDL's 4 to 15 (rocking 4-6, hanging 5, tumbling 7-15). Variants 0 and 1 hang until Kirby passes
        // under them, then fall and explode; variant 2 rocks on the ground;
        static Enemy Coconut() => new Enemy(6, "Coconut", 0x17, 0x7524A4, Enumerable.Range(4, 12).Select(f => new[] { new Part(f) }).ToList(),
            new[] { new Part(3) }, new Dictionary<int, ushort>(),
            new[] { (0x26, 0), (0x26, 1), (0x0D, 3), (0x0D, 2), (0x26, 5), (0x0C, 1), (0x14, 2) },   // "Co" (Cookin), "c", "o" (Blockin), "n" (Cookin), "u" (Cupie), "t" (Hot Head);
            new byte[] { 0x02 }, new byte[] { 0x00, 0x01 });

        // Coner; Frames: NiDL's 4 (standing), 5 and 6 (scuttling). Variant 0 scuttles along, variant 1 in place; in water it glides
        // down and scuttles along the bottom;
        static Enemy Coner() => new Enemy(7, "Coner", 0x29, 0x752A70, new[] { 4, 5, 6 }.Select(f => new[] { new Part(f) }).ToList(),
            new[] { new Part(3) }, new Dictionary<int, ushort>(),
            new[] { (0x26, 0), (0x26, 1), (0x26, 5), (0x14, 4), (0x01, 1) },   // "Co", "n" (Cookin), "e" (Hot Head), "r" (Bronto Burt);
            new byte[] { 0x00, 0x01 }, WaterBehaviors: new byte[] { 0x00 });

        // Starman; Frames: NiDL's 4 to 20 (walking 4-10, crouching 11-12, flying 13-18, diving 20). It walks up to Kirby, flies off the
        // top of the screen and dives onto it, twice, then flies away;
        static Enemy Starman() => new Enemy(8, "Starman", 0x1F, 0x752708, Enumerable.Range(4, 17).Select(f => new[] { new Part(f) }).ToList(),
            new[] { new Part(3) }, new Dictionary<int, ushort>(),
            new[] { (0x04, 0), (0x14, 2), (0x14, 5), (0x01, 1), (0x28, 2), (0x14, 5), (0x26, 5) },   // "S" (Squishy), "ta" (Hot Head), "r" (Bronto Burt), "m" (Bomber), "a" (Hot Head), "n" (Cookin);
            new byte[] { 0x00, 0x01, 0x03 }, new byte[] { 0x02 });

        // Needlous; Frames: NiDL's 4 to 16 (walking 4-6, hopping 7-8, spinning 11-16). It walks, curls up and rolls; variant 1 walks
        // in place;
        static Enemy Needlous() => new Enemy(9, "Needlous", 0x2C, 0x752B08, Enumerable.Range(4, 13).Select(f => new[] { new Part(f) }).ToList(),
            new[] { new Part(3) }, new Dictionary<int, ushort>(),
            new[] { (0x1E, 0), (0x14, 4), (0x14, 4), (0x1E, 2), (0x02, 1), (0x1E, 1), (0x0C, 1), (0x04, 4) },   // "N" (Noddy), "ee" (Hot Head), "d" (Noddy), "l" (Blipper), "o" (Noddy), "u" (Cupie), "s" (Squishy);
            new byte[] { 0x00, 0x01 });

        // Blade Knight; Frames: NiDL's 4 to 16 (walking 4-9, slashing 10-16). It walks up to Kirby and slashes (sometimes twice),
        // lunging, then backs off; gives Sword;
        static Enemy BladeKnight() => new Enemy(10, "Blade Knight", 0x25, 0x752A24, Enumerable.Range(4, 13).Select(f => new[] { new Part(f) }).ToList(),
            new[] { new Part(3) }, new Dictionary<int, ushort>(),
            new[] {
                (0x28, 0), (0x02, 1), (0x14, 5), (0x1E, 2), (0x14, 4), (-1, -1),   // "B" (Bomber), "l" (Blipper), "a" (Hot Head), "d" (Noddy), "e" (Hot Head), space;
                (0x18, 3), (0x26, 5), (0x26, 4), (0x16, 3), (0x04, 5), (0x14, 2),  // "K" (Sir Kibble), "ni" (Cookin), "g" (Pengy), "h" (Squishy), "t" (Hot Head);
            },
            new byte[] { 0x00, 0x01, 0x02 });

        // Chilly; Frames: NiDL's 4 to 11 (wiggling and lunging 4-7, crouched 10, arms up 11). It wiggles turning around until it
        // faces Kirby, lunges at it, then freezes (a ring of KatAM's Pengy's ice); variant 1 lunges in place; gives Ice;
        static Enemy Chilly() => new Enemy(11, "Chilly", 0x0E, 0x752284, Enumerable.Range(4, 8).Select(f => new[] { new Part(f) }).ToList(),
            new[] { new Part(3) }, new Dictionary<int, ushort>(),
            new[] { (0x0C, 0), (0x04, 5), (0x26, 4), (0x02, 1), (0x02, 1), (0x05, 5) },   // "C" (Cupie), "h" (Squishy), "i" (Cookin), "ll" (Blipper), "y" (Scarfy);
            new byte[] { 0x00, 0x01 });

        // Cool Spook; Frames: NiDL's 4 to 7 (flapping). It flies left bobbing, through walls; variant 1 bobs in place facing Kirby;
        static Enemy CoolSpook() => new Enemy(12, "Cool Spook", 0x13, 0x7523EC, Enumerable.Range(4, 4).Select(f => new[] { new Part(f) }).ToList(),
            new[] { new Part(3) }, new Dictionary<int, ushort>(),
            new[] {
                (0x0C, 0), (0x1E, 1), (0x1E, 1), (0x02, 1), (-1, -1),   // "C" (Cupie), "oo" (Noddy), "l" (Blipper), space;
                (0x04, 0), (0x02, 3), (0x1E, 1), (0x1E, 1), (0x17, 3),  // "S" (Squishy), "p" (Blipper), "oo" (Noddy), "k" (Rocky);
            },
            new byte[0], new byte[] { 0x00, 0x01 });

        // Poppy Bros. Jr.; NiDL types 0x26-0x28 (one tile sheet). Frames: NiDL's 4 to 7, then the ball's 9 to 16 (type 0x27's table) and
        // the bomb's (type 0x28's). Variants: 0 hops, 1 jumps in, 2 hops in place, 3 and 4 ride a ball or a bomb, 5 and 6 in place;
        // 7 and 8 (the ball or bomb that comes off when Kirby inhales the rider) are only spawned;
        static Enemy PoppyBrosJr() => new Enemy(13, "Poppy Bros. Jr.", 0x26, 0x7528D0,
            Enumerable.Range(4, 4).Select(f => new[] { new Part(f) })
                .Concat(Enumerable.Range(9, 8).Select(f => new[] { new Part(f, null, 0x7528F0, true) }))
                .Concat(Enumerable.Range(9, 8).Select(f => new[] { new Part(f, null, 0x752934, true) })).ToList(),
            new[] { new Part(3) }, new Dictionary<int, ushort>(),
            new[] {
                (0x16, 0), (0x1E, 1), (0x02, 3), (0x02, 3), (0x05, 5), (-1, -1),   // "P" (Pengy), "o" (Noddy), "pp" (Blipper), "y" (Scarfy), space;
                (0x28, 0), (0x01, 1), (0x1E, 1), (0x04, 4), (-1, -1),               // "B" (Bomber), "r" (Bronto Burt), "o" (Noddy), "s" (Squishy), space;
                (0x10, 0), (0x01, 1),                                               // "J" (Jack), "r" (Bronto Burt);
            },
            new byte[] { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 });

        // Gip; Frames: NiDL's 4 to 16 (walking 4-7, clinging and flapping 8-12, gliding 13 and 16, throwing 14 and 15), then its
        // star's 4-7 (frame table 0x08752C18). It walks, clings to walls, flaps up and glides down them, jumps at Kirby off them and
        // throws stars; variant 1 walks in place;
        static Enemy Gip() => new Enemy(14, "Gip", 0x2F, 0x752BD4,
            Enumerable.Range(4, 13).Select(f => new[] { new Part(f) }).Concat(Enumerable.Range(4, 4).Select(f => new[] { new Part(f, null, 0x752C18) })).ToList(),
            new[] { new Part(3) }, new Dictionary<int, ushort>(),
            new[] { (0x03, 0), (0x26, 4), (0x02, 3) },   // "G" (Glunk), "i" (Cookin), "p" (Blipper);
            new byte[] { 0x00, 0x01 });

        // IsNightmareInDreamLand(); A Kirby: Nightmare in Dream Land ROM (USA, Japanese or European);
        public static bool IsNightmareInDreamLand(byte[] rom) => RegionOf(rom) != null;

        // BehaviorsOf(); The behavior bytes of type 0x31 the randomizer may use, for the enemies picked (only the flying ones, only the
        // swimming ones, or all);
        public static List<byte> BehaviorsOf(IEnumerable<int> enemies, bool flying = false, bool water = false) =>
            BehaviorGroupsOf(enemies, flying, water).SelectMany(group => group).ToList();

        /* BehaviorGroupsOf(); The same, one list per enemy (only the enemies that have some), so the randomizer can pick an enemy
         * first and then its variant, and give each enemy as much room in the pool as any of the game's own; */
        public static List<List<byte>> BehaviorGroupsOf(IEnumerable<int> enemies, bool flying = false, bool water = false) =>
            Definitions.Where(e => enemies.Contains(e.Index))
                       .Select(e => ((water ? e.WaterBehaviors : flying ? e.FlyingBehaviors : e.Behaviors) ?? Array.Empty<byte>())
                                    .Select(b => (byte)(e.Index << 4 | b)).ToList())
                       .Where(group => group.Count > 0).ToList();

        // NameOf(); The ported enemy a behavior byte of type 0x31 picks;
        public static string NameOf(byte behavior) => Definitions.FirstOrDefault(e => e.Index == behavior >> 4)?.Name ?? Definitions[0].Name;

        /* AddEnemies(); Adds every ported enemy as object type 0x31, expanding the ROM to 32 MB (rom is replaced by the bigger one).
         * Returns null when done, or why it couldn't be done (rom is left as it was); */
        public static string AddEnemies(ref byte[] rom, byte[] nightmare) {
            KatAMExpansionCode.Version version = VersionOf(rom);
            NightmareRegion region = RegionOf(nightmare);

            if (version == null) return "The ROM Expansion supports the USA, European and Japanese (revision 1) versions of Kirby & the Amazing Mirror.";
            if (region == null) return "That isn't a Kirby: Nightmare in Dream Land ROM.";
            if (region.KatAM != version.Name)
                return $"The Nightmare in Dream Land ROM must be from the same region as Kirby & the Amazing Mirror: {version.Name} with {version.Name} (this one is from {region.KatAM}).";
            if (rom.Length > ExpandedSize) return "That ROM is bigger than the GBA can map (32 MB).";

            byte[] original = rom;

            rom = Expand(rom);
            string error = AddEnemiesTo(rom, nightmare, region, version);
            if (error != null) rom = original;
            return error;
        }

        // Expand(); A copy of the ROM padded to 32 MB with 0xFF (what unused ROM reads as), the size emulators and flash carts expect;
        static byte[] Expand(byte[] rom) {
            byte[] expanded = new byte[ExpandedSize];

            Array.Copy(rom, expanded, rom.Length);
            Array.Fill(expanded, (byte)0xFF, rom.Length, ExpandedSize - rom.Length);
            return expanded;
        }

        static string AddEnemiesTo(byte[] rom, byte[] nightmare, NightmareRegion region, KatAMExpansionCode.Version version) {
            int HurtAnimID = version.AnimCount,     // The first free anim ID (the vanilla table's size);
                SpriteTables = version.SpriteTables, SpawnTable = version.SpawnTable, ObjectParameters = version.ObjectParameters,
                ObjectParameters2 = version.ObjectParameters2, HitFlashBranch = version.HitFlashBranch;

            for (int i = CodeAddress; i < CodeAddress + ReservedSize; i++) {
                if (rom[i] != 0xFF) return "The free space used by the ROM Expansion is already in use.";
            }

            List<(Enemy enemy, byte[] palette, List<(byte[] tiles, ushort[] oams, short[] attributes, bool palette15)> frames, byte[] hurt)> built;

            try {
                built = Definitions.Select(e => Build(nightmare, region, e)).ToList();
            } catch (Exception exception) when (exception is IndexOutOfRangeException or ArgumentException or InvalidOperationException) {
                return $"The Nightmare in Dream Land ROM doesn't match the {region.KatAM} release.";
            }

            int anims = ReadInt(rom, SpriteTables) - 0x08000000, attrTable = ReadInt(rom, SpriteTables + 4) - 0x08000000,
                oamTable = ReadInt(rom, SpriteTables + 8) - 0x08000000, paletteBase = ReadInt(rom, SpriteTables + 12) - 0x08000000,
                tilesBase = ReadInt(rom, SpriteTables + 16) - 0x08000000;

            byte[] code = version.Code;
            Array.Copy(code, 0, rom, CodeAddress, code.Length);
            int cursor = CodeAddress + ((code.Length + 0xFF) & ~0xFF);

            // Every object of type 0x31 gets VRAM for the biggest frame, plus the hurt frame's tiles at the end;
            int frameTiles = built.SelectMany(b => b.frames).Max(f => f.tiles.Length / 32);
            int numTiles = frameTiles + HurtTiles;

            // Anims: the hurt frame (one 32x32 piece drawing the last HurtTiles tiles), then one per enemy;
            List<int> animVariants = new List<int>(), animAttributes = new List<int>(), animOams = new List<int>();

            int hurtCommands = Allocate(ref cursor, 12, 4);
            WriteInts(rom, hurtCommands, 60, 0, -4);
            int hurtVariants = Allocate(ref cursor, 4, 4);
            WriteInt(rom, hurtVariants, hurtCommands + 0x08000000);
            int hurtAttributes = Allocate(ref cursor, 12, 4);
            WriteShorts(rom, hurtAttributes, 0, 1, 32, 32, HurtOriginX, HurtOriginY);
            int hurtOam = Allocate(ref cursor, 6, 4);
            WriteShorts(rom, hurtOam, 0 | 0 << 14, 0 | 2 << 14, frameTiles);
            animVariants.Add(hurtVariants);
            animAttributes.Add(hurtAttributes);
            animOams.Add(hurtOam);

            // NiDL's palette 15, for the frames drawn with it (the C code loads it as a second palette, from one of them);
            int palette15At = Allocate(ref cursor, 32, 32, paletteBase);
            Array.Copy(nightmare, region.Palette15, rom, palette15At, 32);
            int palette15Index = (palette15At - paletteBase) / 32;

            List<(int animBase, int animId, int hurtTiles)> data = new List<(int, int, int)>();
            List<(int animId, int variant)> animInfos = new List<(int, int)> { (HurtAnimID, 0), (HurtAnimID, 0) };

            foreach ((Enemy enemy, byte[] palette, List<(byte[] tiles, ushort[] oams, short[] attributes, bool palette15)> frames, byte[] hurt) in built) {
                int animId = HurtAnimID + 1 + data.Count;

                // Tiles and palettes are indexes from their table's base, so they are aligned to it;
                int tilesAt = Allocate(ref cursor, frames.Sum(f => f.tiles.Length), 32, tilesBase);
                int paletteAt = Allocate(ref cursor, 32, 32, paletteBase);
                palette.CopyTo(rom, paletteAt);
                int hurtAt = Allocate(ref cursor, hurt.Length, 4);
                hurt.CopyTo(rom, hurtAt);

                int tileIndex = (tilesAt - tilesBase) / 32, paletteIndex = (paletteAt - paletteBase) / 32;
                int oamCount = frames.Sum(f => f.oams.Length);
                int attributesAt = Allocate(ref cursor, frames.Count * 12, 4), oamsAt = Allocate(ref cursor, oamCount * 2, 4);
                int variantsAt = Allocate(ref cursor, frames.Count * 4, 4);

                // Animation commands of every frame: GetPalette, GetTiles, ShowFrame and End (holds the frame);
                for (int i = 0, tile = 0, oam = 0; i < frames.Count; i++) {
                    (byte[] tiles, ushort[] oams, short[] attributes, bool palette15) = frames[i];
                    int commands = Allocate(ref cursor, 36, 4);

                    tiles.CopyTo(rom, tilesAt + tile * 32);
                    WriteInts(rom, commands, -2, palette15 ? palette15Index : paletteIndex, 16, -1, tileIndex + tile, tiles.Length / 32, 60, i, -4);
                    WriteInt(rom, variantsAt + i * 4, commands + 0x08000000);
                    WriteShorts(rom, attributesAt + i * 12, (short)(oam / 3), attributes[1], attributes[2], attributes[3], attributes[4], attributes[5]);
                    for (int o = 0; o < oams.Length; o++) WriteShort(rom, oamsAt + (oam + o) * 2, oams[o]);

                    tile += tiles.Length / 32;
                    oam += oams.Length;
                }

                data.Add((animInfos.Count, animId, hurtAt));
                for (int i = 0; i < frames.Count; i++) animInfos.Add((animId, i));

                animVariants.Add(variantsAt);
                animAttributes.Add(attributesAt);
                animOams.Add(oamsAt);
            }

            if (animInfos.Count > 256) throw new InvalidOperationException("The ported enemies have more than 256 frames.");

            // Extended anims, attrs and oamData tables (indexed by anim ID);
            int[] tables = { anims, attrTable, oamTable };
            List<int>[] entries = { animVariants, animAttributes, animOams };
            for (int t = 0; t < 3; t++) {
                int at = Allocate(ref cursor, (HurtAnimID + entries[t].Count) * 4, 4);

                Array.Copy(rom, tables[t], rom, at, HurtAnimID * 4);
                for (int i = 0; i < entries[t].Count; i++) WriteInt(rom, at + (HurtAnimID + i) * 4, entries[t][i] + 0x08000000);
                WriteInt(rom, SpriteTables + t * 4, at + 0x08000000);
            }

            int animInfosAt = Allocate(ref cursor, animInfos.Count * 4, 4);
            for (int i = 0; i < animInfos.Count; i++) {
                WriteShort(rom, animInfosAt + i * 4, animInfos[i].animId);
                rom[animInfosAt + i * 4 + 2] = (byte)animInfos[i].variant;
            }

            // HP bar names, one per enemy;
            int namesAt = Allocate(ref cursor, Definitions.Count * 0x100, 4);
            if (version.NameSite1 != 0)
                for (int i = 0; i < Definitions.Count; i++) ComposeName(rom, version, Definitions[i].Glyphs).CopyTo(rom, namesAt + i * 0x100);

            // gNidlData: names, then per enemy (animBase, animId, hurt tiles);
            int nidlData = CodeAddress + version.NidlData;
            WriteInt(rom, nidlData, namesAt + 0x08000000);
            for (int i = 0; i < data.Count; i++) {
                int entry = nidlData + 4 + Definitions[i].Index * 8;
                WriteShort(rom, entry, data[i].animBase);
                WriteShort(rom, entry + 2, data[i].animId);
                WriteInt(rom, entry + 4, data[i].hurtTiles + 0x08000000);
            }

            /* Object type 0x31: spawn function and parameters (hurt frames 0 & 1, Waddle Dee's score and HP, no ability, no palette
             * key (each enemy loads its own), VRAM tiles, behavior start, animations). Its second parameters are Waddle Dee's; */
            int codeBase = CodeAddress + 0x08000000;
            int parameters = ObjectParameters + NightmareID * 0x18;

            WriteInt(rom, SpawnTable + NightmareID * 4, codeBase + version.CreateNidlEnemy + 1);
            rom[parameters] = 0;
            rom[parameters + 1] = 1;
            Array.Copy(rom, ObjectParameters + 2, rom, parameters + 2, 4);
            WriteShort(rom, parameters + 6, 0);
            WriteShort(rom, parameters + 8, 0);
            WriteShort(rom, parameters + 10, 0);
            WriteInt(rom, parameters + 12, numTiles);
            WriteInt(rom, parameters + 16, codeBase + version.NidlStart + 1);
            WriteInt(rom, parameters + 20, animInfosAt + 0x08000000);
            Array.Copy(rom, ObjectParameters2, rom, ObjectParameters2 + NightmareID * 8, 8);

            /* The hit flash (sub_080857A0) keeps the palette to restore afterwards, but then looks it up again from the type's palette key
             * (none for type 0x31: palette 15, the flash's). Its "palette isn't 15" branch now skips that lookup (bne to the store); */
            if (rom[HitFlashBranch] != 0x00 || rom[HitFlashBranch + 1] != 0xD0) return $"The ROM's code doesn't match the {version.Name} release.";
            WriteShort(rom, HitFlashBranch, 0xD11A);

            // Japan has no names patch (version.NameSite1 is 0; its HP bar names are stored differently): the bar keeps type 0x31's;
            int[] sites = { version.NameSite1, version.NameSite2 };
            for (int i = 0; i < NameCopies.Length; i++) {
                if (sites[i] == 0) continue;
                (int length, int literal, bool fromTypePointer) = NameCopies[i];
                ApplyNamePatch(rom, sites[i], length, sites[i] + literal, fromTypePointer, codeBase + version.NidlCopyName);
            }

            return AddAbilities(rom, version, ref cursor, new Dictionary<string, int[]>());
        }

        //////////////////////////////////////////////////////////////////////////////////////////////

        // The hurt frame: 32x32 pixels (HurtTiles tiles), NiDL's origin at its center (KatAM's one pixel higher, see ReadFrame);
        const int HurtTiles = 16, HurtOriginX = 16, HurtOriginY = 15;

        /* Build(); The enemy's palette, frames (tiles in 1D order, OAM entries, attributes: oam index, pieces, box width, height,
         * origin x, y) and hurt frame (HurtTiles tiles of 32x32 pixels); */
        static (Enemy, byte[], List<(byte[], ushort[], short[], bool)>, byte[]) Build(byte[] nightmare, NightmareRegion region, Enemy enemy) {
            int descriptor = ReadInt(nightmare, region.Descriptors + (enemy.Type - 8) * 4) - 0x08000000, frameTable = enemy.FrameTable + region.FrameShift;
            int paletteAddress = ReadInt(nightmare, descriptor + 8) - 0x08000000, tilesAddress = ReadInt(nightmare, descriptor + 12) - 0x08000000;

            if (nightmare[tilesAddress] != 0x10) throw new InvalidOperationException("Not LZ77 tiles.");

            byte[] tiles = DecompressLZ77(nightmare, tilesAddress);
            byte[] palette = new byte[32];
            Array.Copy(nightmare, paletteAddress, palette, 0, 32);
            foreach ((int color, ushort value) in enemy.PaletteChanges) WriteShort(palette, color * 2, value);

            List<(byte[], ushort[], short[], bool)> frames = new List<(byte[], ushort[], short[], bool)>();

            foreach (Part[] parts in enemy.Frames) {
                List<(Piece piece, Dictionary<int, int> recolor)> pieces = parts.SelectMany(p => ReadFrame(nightmare, p.Table != 0 ? p.Table + region.FrameShift : frameTable, p.Frame).Select(piece => (piece, p.Recolor))).ToList();

                int halfWidth = pieces.Max(p => Math.Max(-p.piece.X, p.piece.X + p.piece.Width));
                int top = pieces.Min(p => p.piece.Y), bottom = pieces.Max(p => p.piece.Y + p.piece.Height);

                List<byte> frameTiles = new List<byte>();
                List<ushort> oams = new List<ushort>();

                foreach ((Piece piece, Dictionary<int, int> recolor) in pieces) {
                    oams.Add((ushort)((piece.Y - top) | piece.Shape << 14));
                    oams.Add((ushort)((piece.X + halfWidth) | piece.Size << 14 | piece.HFlip << 12 | piece.VFlip << 13));
                    oams.Add((ushort)(frameTiles.Count / 32));
                    frameTiles.AddRange(PieceTiles(tiles, piece, recolor));
                }

                frames.Add((frameTiles.ToArray(), oams.ToArray(), new short[] { 0, (short)pieces.Count, (short)(halfWidth * 2), (short)(bottom - top), (short)halfWidth, (short)-top }, parts.Any(p => p.Palette15)));
            }

            // Hurt frame: drawn pixel by pixel into 32x32 and cut into tiles;
            int[,] canvas = new int[32, 32];
            foreach (Part part in enemy.Hurt) {
                foreach (Piece piece in ReadFrame(nightmare, frameTable, part.Frame)) {
                    byte[] pieceTiles = PieceTiles(tiles, piece, part.Recolor);
                    int columns = piece.Width / 8;

                    for (int y = 0; y < piece.Height; y++) {
                        for (int x = 0; x < piece.Width; x++) {
                            int t = y / 8 * columns + x / 8, b = pieceTiles[t * 32 + y % 8 * 4 + x % 8 / 2];
                            int color = (x & 1) != 0 ? b >> 4 : b & 0xF;
                            int cx = HurtOriginX + piece.X + (piece.HFlip != 0 ? piece.Width - 1 - x : x);
                            int cy = HurtOriginY + piece.Y + (piece.VFlip != 0 ? piece.Height - 1 - y : y);

                            if (color != 0 && cx >= 0 && cx < 32 && cy >= 0 && cy < 32) canvas[cy, cx] = color;
                        }
                    }
                }
            }

            byte[] hurt = new byte[HurtTiles * 32];
            for (int y = 0; y < 32; y++)
                for (int x = 0; x < 32; x++)
                    hurt[(y / 8 * 4 + x / 8) * 32 + y % 8 * 4 + x % 8 / 2] |= (byte)((x & 1) != 0 ? canvas[y, x] << 4 : canvas[y, x]);

            return (enemy, palette, frames, hurt);
        }

        // PieceTiles(); The 4bpp tiles of a piece in 1D order (NiDL's VRAM is 2D, 32 tiles per row; its data, 16 tiles per row);
        static byte[] PieceTiles(byte[] tiles, Piece piece, Dictionary<int, int> recolor) {
            List<byte> output = new List<byte>();

            for (int y = 0; y < piece.Height / 8; y++) {
                for (int x = 0; x < piece.Width / 8; x++) {
                    int source = piece.Tile + y * 32 + x;
                    source = source / 32 * 16 + source % 32;

                    for (int b = 0; b < 32; b++) {
                        byte pixels = tiles[source * 32 + b];
                        if (recolor != null) pixels = (byte)(recolor.GetValueOrDefault(pixels & 0xF, pixels & 0xF) | recolor.GetValueOrDefault(pixels >> 4, pixels >> 4) << 4);
                        output.Add(pixels);
                    }
                }
            }

            return output.ToArray();
        }

        /* ApplyNamePatch(); Replaces a name copy with: r0 = the object, then a call to NidlCopyName (its address goes in the literal the
         * copy loaded the name table from), returning to the nops that fill the rest of the copy; */
        static void ApplyNamePatch(byte[] rom, int at, int length, int literal, bool fromTypePointer, int function) {
            const ushort Nop = 0x46C0;
            List<ushort> code = new List<ushort>();

            if (fromTypePointer) {
                code.Add(0x4660);                                           // mov r0, ip (the object's type);
                code.Add(0x3882);                                           // subs r0, #0x82;
            } else {
                code.Add(0x1C10);                                           // adds r0, r2, #0 (the object);
            }

            int ldr = at + code.Count * 2;
            code.Add((ushort)(0x4B00 | (literal - ((ldr + 4) & ~3)) / 4));  // ldr r3, [literal];
            int adr = at + code.Count * 2, back = (adr + 8 + 3) & ~3;
            code.Add((ushort)(0xA100 | (back - ((adr + 4) & ~3)) / 4));     // add r1, pc, #: where to come back (after bx r3);
            code.Add(0x3101);                                               // adds r1, #1 (Thumb);
            code.Add(0x468E);                                               // mov lr, r1;
            code.Add(0x4718);                                               // bx r3;
            while (code.Count * 2 < length) code.Add(Nop);

            if (back >= at + length) throw new InvalidOperationException("The name patch doesn't fit.");
            for (int i = 0; i < code.Count; i++) WriteShort(rom, at + i * 2, code[i]);
            WriteInt(rom, literal, function + 1);
        }

        //////////////////////////////////////////////////////////////////////////////////////////////

        // Piece; One OAM piece of a NiDL frame, mirrored: KatAM sprites face left, NiDL's face right. Y is 1 pixel lower to stand like KatAM enemies;
        record Piece(int X, int Y, int Width, int Height, int Shape, int Size, int HFlip, int VFlip, int Tile);

        static readonly int[,,] Shapes = {
            { { 8, 8 }, { 16, 16 }, { 32, 32 }, { 64, 64 } },
            { { 16, 8 }, { 32, 8 }, { 32, 16 }, { 64, 32 } },
            { { 8, 16 }, { 8, 32 }, { 16, 32 }, { 32, 64 } },
        };

        /* ReadFrame(); NiDL frame records are 8 bytes: y, attr0 (shape, 0x10 = last), x, attr1 (size, flips), flipped x,
         * flipped attr1, tile (10 bits); */
        static List<Piece> ReadFrame(byte[] nightmare, int frameTable, int frame) {
            List<Piece> pieces = new List<Piece>();
            int address = ReadInt(nightmare, frameTable + frame * 4) - 0x08000000;

            for (int i = 0; i < 16; i++, address += 8) {
                int shape = nightmare[address + 1] >> 6, size = nightmare[address + 3] >> 6;

                pieces.Add(new Piece((sbyte)nightmare[address + 4], (sbyte)nightmare[address] + 1,
                    Shapes[shape, size, 0], Shapes[shape, size, 1], shape, size,
                    1 - ((nightmare[address + 3] >> 4) & 1), (nightmare[address + 3] >> 5) & 1,
                    nightmare[address + 6] | (nightmare[address + 7] & 3) << 8));

                if ((nightmare[address + 1] & 0x10) != 0) break;
            }

            return pieces;
        }

        //////////////////////////////////////////////////////////////////////////////////////////////

        /* Enemy names; The HP bar shows a 64x8 image per object type (8 tiles, gUnk_08D60EE4[language] + type * 0x100), right-aligned.
         * New names are put together from the letters of other names: a letter is a run of columns with fill pixels (8 or 15),
         * letters are 1 column apart (3 for a space), and the outline (7) is drawn around the fill afterwards, as the game's names are;
         * Letters are (object type, letter index) in the English names; */
        const int EnglishNames = 1, NameEnd = 62;

        static int[,] ReadName(byte[] rom, int table, int type) {
            int[,] pixels = new int[8, 64];
            int address = table + type * 0x100;

            for (int tile = 0; tile < 8; tile++)
                for (int y = 0; y < 8; y++)
                    for (int x = 0; x < 8; x++) {
                        byte b = rom[address + tile * 32 + y * 4 + x / 2];
                        pixels[y, tile * 8 + x] = (x & 1) != 0 ? b >> 4 : b & 0xF;
                    }

            return pixels;
        }

        static bool IsFill(int color) => color == 8 || color == 15;

        // Letter(); The fill pixels of a letter of a name;
        static int[,] Letter(byte[] rom, int table, int type, int index) {
            int[,] name = ReadName(rom, table, type);
            int letter = -1;

            for (int x = 0; x < 64; x++) {
                bool isFilled = Enumerable.Range(0, 8).Any(y => IsFill(name[y, x]));
                if (!isFilled) continue;

                int end = x;
                while (end + 1 < 64 && Enumerable.Range(0, 8).Any(y => IsFill(name[y, end + 1]))) end++;

                if (++letter == index) {
                    int[,] pixels = new int[8, end - x + 1];
                    for (int y = 0; y < 8; y++)
                        for (int c = x; c <= end; c++) pixels[y, c - x] = IsFill(name[y, c]) ? name[y, c] : 0;
                    return pixels;
                }

                x = end;
            }

            throw new InvalidOperationException($"Letter {index} of object 0x{type:X2}'s name wasn't found.");
        }

        // ComposeName(); A name image (8 tiles) made of letters of the game's English names;
        static byte[] ComposeName(byte[] rom, KatAMExpansionCode.Version version, (int type, int letter)[] glyphs) {
            int english = ReadInt(rom, version.Names + EnglishNames * 4) - 0x08000000;
            List<int[,]> letters = glyphs.Select(g => g.type < 0 ? null : Letter(rom, english, g.type, g.letter)).ToList();

            int width = 0;
            for (int i = 0; i < letters.Count; i++) {
                if (letters[i] == null) width += 3;
                else width += letters[i].GetLength(1) + (i > 0 && letters[i - 1] != null ? 1 : 0);
            }

            int[,] fill = new int[8, 64];
            for (int i = 0, x = NameEnd - width + 1; i < letters.Count; i++) {
                if (letters[i] == null) { x += 3; continue; }
                if (i > 0 && letters[i - 1] != null) x++;

                for (int y = 0; y < 8; y++)
                    for (int c = 0; c < letters[i].GetLength(1); c++)
                        if (letters[i][y, c] != 0) fill[y, x + c] = letters[i][y, c];

                x += letters[i].GetLength(1);
            }

            byte[] tiles = new byte[0x100];
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 64; x++) {
                    int color = fill[y, x];
                    if (color == 0 && Enumerable.Range(-1, 3).Any(dy => Enumerable.Range(-1, 3).Any(dx =>
                            y + dy >= 0 && y + dy < 8 && x + dx >= 0 && x + dx < 64 && fill[y + dy, x + dx] != 0))) color = 7;

                    tiles[x / 8 * 32 + y * 4 + x % 8 / 2] |= (byte)((x & 1) != 0 ? color << 4 : color);
                }
            }

            return tiles;
        }

        // DecompressLZ77(); GBA BIOS LZ77 (type 0x10);
        static byte[] DecompressLZ77(byte[] source, int address) {
            int size = ReadInt(source, address) >> 8, position = address + 4;
            byte[] output = new byte[size];

            for (int written = 0; written < size;) {
                byte flags = source[position++];

                for (int bit = 0; bit < 8 && written < size; bit++) {
                    if ((flags & (0x80 >> bit)) == 0) {
                        output[written++] = source[position++];
                        continue;
                    }

                    int length = (source[position] >> 4) + 3, distance = ((source[position] & 0xF) << 8 | source[position + 1]) + 1;
                    position += 2;

                    for (int i = 0; i < length && written < size; i++, written++) output[written] = output[written - distance];
                }
            }

            return output;
        }

        // Allocate(); Next free address, aligned (to a table's base, for tiles and palettes);
        static int Allocate(ref int cursor, int size, int alignment, int alignedTo = 0) {
            while ((cursor - alignedTo) % alignment != 0) cursor++;

            int address = cursor;
            cursor += size;

            if (cursor > CodeAddress + ReservedSize) throw new InvalidOperationException("The ROM Expansion doesn't fit in its free space.");

            return address;
        }

        static int ReadInt(byte[] data, int address) => BitConverter.ToInt32(data, address);

        static void WriteInt(byte[] data, int address, int value) => BitConverter.GetBytes(value).CopyTo(data, address);

        static void WriteInts(byte[] data, int address, params int[] values) {
            for (int i = 0; i < values.Length; i++) WriteInt(data, address + i * 4, values[i]);
        }

        static void WriteShort(byte[] data, int address, int value) => BitConverter.GetBytes((short)value).CopyTo(data, address);

        static void WriteShorts(byte[] data, int address, params int[] values) {
            for (int i = 0; i < values.Length; i++) WriteShort(data, address + i * 2, values[i]);
        }
    }
}
