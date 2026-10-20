using KatAMInternal;
using System.Text;

namespace KatAMRandomizer {
    /* KatAMExpansion; ROM Expansion: enemies ported from Kirby: Nightmare in Dream Land into Kirby & the Amazing Mirror, any version
     * (USA, European, Japanese revision 1), with the NiDL ROM of the same region: USA with USA, European with European, Japanese with
     * Japanese. The European version shows the English names in every language; the Japanese one shows none (an empty bar: its
     * names, both of its table's languages, are images of Japanese text, no letters to make others of).
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

        const int ExpandedSize = 0x2000000, CodeAddress = 0x1000000, MapTableAddress = ExpandedSize - 0x100, ReservedSize = MapTableAddress - CodeAddress;

        /* NiDL: graphics descriptor per enemy (type - 8): tile count (+2), palette (+8), LZ77 tiles (+12). The frame tables (and every
         * NiDL address in the enemies' notes) are the USA ones; the Japanese and European ROMs have the same data, moved by FrameShift
         * (the European one's from Mr. Bright's table, 0x08753E3C, on, LateFrames, by LateShift: FrameAddress()); */
        // Palette15: NiDL's shared object palette 15 (the level loads it; the Poppy Bros. Jr.'s ball and bomb are drawn with it);
        /* Cards: the ability cards (HUD portraits) by NiDL ability: palette pointers (CardPalettes + ability * CardPaletteStride) and tiles
         * pointers (CardTiles + ability * CardTileStride; Europe's are per language, English first); RoomTable: the rooms (see
         * KatAMExpansionRooms.cs); */
        sealed record NightmareRegion(string GameCode, int Descriptors, int FrameShift, string KatAM, int Palette15,
                                      int CardPalettes, int CardPaletteStride, int CardTiles, int CardTileStride, int RoomTable, int LateShift);
        const int LateFrames = 0x753E3C;
        static int FrameAddress(NightmareRegion region, int usa) => usa + (usa >= LateFrames ? region.LateShift : region.FrameShift);

        static readonly NightmareRegion[] NightmareRegions = {
            new NightmareRegion("A7KE", 0x73EEA0, 0, "USA", 0x0D07A8, 0x731A90, 8, 0x731A94, 8, 0x7E1D58, 0),
            new NightmareRegion("A7KJ", 0x742DAC, 0x3A8C, "Japan", 0x0CFF60, 0x73599C, 8, 0x7359A0, 8, 0x7E60DC, 0x3A8C),     // Hoshi no Kirby: Yume no Izumi Deluxe;
            new NightmareRegion("A7KP", 0x97B678, 0x23C7EC, "Europe", 0x0D3228, 0x96D770, 4, 0x96D7E0, 20, 0xBFD8E8, 0x23CA5C),
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

        /* Part; A NiDL frame (from the enemy's frame table, or Table when the enemy has more than one), recolored, drawn with NiDL's
         * palette 15 or with another of the descriptor's palettes (Palette), Dx, Dy pixels from the others (NiDL draws some bosses as
         * several frames); Common: from NiDL's shared object tiles (CommonTiles, its effects: Whispy Woods' puffs) instead; OnlyPalette:
         * only its pieces NiDL draws with that palette (its OAM's; NiDL draws Kracko with several at once, a KatAM sprite with one);
         * OrbShots: from the Power Orb's shots' own tiles and palette (OrbShotGraphics(): raw tiles the orb loads for them); StarRod: the
         * shared tiles with the Star Rod's star over some (StarRodTiles()); Shared: its pieces with the same tiles drawn from the same
         * tiles of the frame (the Power Orb's slash: up to 17 pieces, 4 different ones); Tile: instead, one 8x8 tile of NiDL's (4 bits
         * a pixel, its palette 15's colors) around the origin (the dot NiDL draws the Power Orb as, far off in the moon); Cutscene: a
         * frame of NiDL's cutscene of Nightmare's arrival (CutsceneGraphics(): its own frame table, tiles and two palettes; 1 its first
         * tiles, the comet's, 2 its second, the blue star's); */
        sealed record Part(int Frame, Dictionary<int, int> Recolor = null, int Table = 0, bool Palette15 = false, int Palette = 0, int Descriptor = -1,
                           int Dx = 0, int Dy = 0, bool Common = false, int OnlyPalette = -1, bool OrbShots = false, bool StarRod = false,
                           bool Shared = false, byte[] Tile = null, int Cutscene = 0);

        /* Enemy; A ported enemy: its index (NIDL_* in nidl.h), name, NiDL object type and frame table, its frames (in the order the
         * C code numbers them), its hurt frame, palette changes, the letters of its name and the behavior bytes the randomizer uses; */
        sealed record Enemy(int Index, string Name, int Type, int FrameTable, List<Part[]> Frames, Part[] Hurt,
                            Dictionary<int, ushort> PaletteChanges, (int type, int letter)[] Glyphs, byte[] Behaviors, byte[] FlyingBehaviors = null,
                            byte[] WaterBehaviors = null, int Descriptor = -1, int Palette = 0, bool Hidden = false);

        /* (Enemy: Descriptor, a mini-boss descriptor index for the graphics instead of the type's, and Palette, which of its palettes; Hidden:
         * not a room's enemy, kept out of the randomizer's options: a boss's, as Whispy Woods' apples;) */
        public static readonly IReadOnlyList<(int index, string name)> Enemies;

        /* Miniboss; A ported mini-boss (object type 0x40, see "ROM Expansion/nidl_miniboss.c"): its index (NIDL_* mini-bosses in nidl.h),
         * name, NiDL mini-boss index (its type is 0x31 + index; graphics descriptor at Descriptors + MinibossDescriptors + index * 4),
         * frame table, frames, hurt frame (64x96, centered), the letters of its name, the behavior bytes the randomizer uses and its object
         * type (MinibossIDs). HurtShift
         * moves the hurt frame that many pixels down within it, for mini-bosses too tall for it, and every frame as much (the C code moves
         * its boxes as much down: DRAWN_LOWER in fire_lion.c, the same value). ExtraHurts follow the hurt frame (the C code picks one: the
         * object's hurt). Icon: the frames the map tracker shows for it (frame 0 if none; see WriteMapTable); IconOverRoom: its one icon
         * frame is drawn over its room there (Whispy Woods' trunk is the room's tiles, only its face is a sprite); IconCells (instead of Icon):
         * its icon's cells side by side, each its frames drawn on each other where the game draws them, from their origins (Kracko: its
         * cloud's top, its bottom and its eye, sprites at one spot; Mr. Bright: his face over his body); */
        sealed record Miniboss(int Index, string Name, int NightmareIndex, int FrameTable, List<Part[]> Frames, Part[] Hurt,
                               (int type, int letter)[] Glyphs, byte[] Behaviors, int HurtShift = 0, byte Type = 0x40,
                               List<Part[]> ExtraHurts = null, int[] Icon = null, bool IconOverRoom = false, int[][] IconCells = null, int EffectOnly = 0);

        /* Mini-boss object types: an object's frame index is a byte, so the mini-bosses are spread over types 0x40-0x42 (all mini-bosses
         * to KatAM), each with its own AnimInfo table; the behavior byte picks the mini-boss either way; */
        public static readonly byte[] MinibossIDs = { 0x40, 0x41, 0x42 };
        const int MinibossDescriptors = 0x244;

        /* MinibossHurt; A mini-boss type's hurt frame: its size, origin and KatAM OAM pieces (shape, size, x, y, first tile), drawn from
         * its last tiles of VRAM (the same as nidl.c's NidlHurtTiles). 64x96 (a 64x64 and a 64x32 piece), but type 0x42's (the
         * Meta-Knights, several at once) 32x64; */
        sealed record MinibossHurt(int Width, int Height, int OriginX, int OriginY, (int shape, int size, int x, int y, int tile)[] Pieces) {
            public int Tiles => Width * Height / 64;
        }
        static readonly MinibossHurt BigHurt = new MinibossHurt(64, 96, 32, 48, new[] { (0, 3, 0, 0, 0), (1, 3, 0, 64, 64) }),
                                     KnightHurt = new MinibossHurt(32, 64, 16, 28, new[] { (2, 3, 0, 0, 0) });
        static MinibossHurt HurtOf(byte type) => type == 0x42 ? KnightHurt : BigHurt;    // (and the bosses', 0x43);
        const int EnemyCount = 18;  // NIDL_ENEMY_COUNT: gNidlData's mini-bosses follow its enemies;
        const int MinibossCount = 6;    // NIDL_MINIBOSS_COUNT: gNidlData's bosses follow its mini-bosses;

        /* Bosses (object type 0x43, see "ROM Expansion/nidl_boss.c"): Miniboss records too (their index is NIDL_* bosses in nidl.h), built
         * the same way (BigHurt), kept out of the mini-boss options; */
        public const byte BossID = 0x43;
        /* Type 0x44 (OBJ_EMPTY_44, a boss to KatAM as 0x43 is) takes the bosses that don't fit in type 0x43's 256 frames (an object's
         * frame index is a byte); */
        public const byte BossID2 = 0x44;
        static readonly byte[] BossIDs = { BossID, BossID2 };
        public static bool IsBossType(byte type) => BossIDs.Contains(type);
        // BossTypeOf(); The object type of a boss (NIDL_* bosses);
        public static byte BossTypeOf(int boss) => BossDefinitions.FirstOrDefault(b => b.Index == boss)?.Type ?? BossID;
        static readonly List<Miniboss> BossDefinitions = new List<Miniboss> {
            WhispyWoods(),
            Kracko(),
            ShineAndBright(),
            MetaKnight(),
            PaintRoller(),
            HeavyMole(),
            KingDedede(),
            PowerOrb(),
            Nightmare(),
        };
        static List<Miniboss> AllBosses => MinibossDefinitions.Concat(BossDefinitions).ToList();
        static IEnumerable<byte> BossTypes => MinibossIDs.Concat(BossIDs);

        public static readonly IReadOnlyList<(int index, string name)> Minibosses;

        static readonly List<Miniboss> MinibossDefinitions = new List<Miniboss> {
            PoppyBrosSr(),
            MrTickTock(),
            GrandWheelie(),
            Bugzzy(),
            FireLion(),
            MetaKnights(),
        };

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
            Apple(),
            FallingStar(),
            SpitStar(),
        };

        /* ParadeLoops; The ported enemies the intro's parade shows (KatAMTitleScreen.AddNidlTypes): their kind there (KatAMTitleScreen's
         * Kinds: 0 small ground walkers, 2 fliers) and the loop their sprite plays (frame, frames shown: their walk or flight, as their
         * C code plays it; "enemies/*.c"), an extra variant of their anim; */
        static readonly Dictionary<string, (int kind, (int frame, int delay)[] loop)> ParadeLoops = new Dictionary<string, (int, (int, int)[])> {
            ["Blade Knight"] = (0, new[] { (1, 6), (5, 8), (3, 5), (4, 11), (3, 6), (5, 8), (1, 5), (0, 11) }),
            ["Needlous"] = (0, new[] { (0, 4), (1, 4), (2, 6), (1, 2) }),
            ["Starman"] = (0, new[] { (0, 4), (4, 6), (6, 10), (5, 6), (0, 4), (1, 6), (2, 10), (3, 6) }),
            ["Gip"] = (0, new[] { (2, 8), (3, 6), (0, 8), (1, 6) }),
            ["Twizzy"] = (2, new[] { (0, 4), (1, 4) }),
            ["Cool Spook"] = (2, new[] { (0, 12), (1, 12), (2, 12), (3, 12) }),
            ["Bubbles"] = (2, new[] { (0, 20), (10, 2), (14, 4), (10, 2) }),
        };

        static KatAMExpansion() {
            Enemies = Definitions.Where(e => !e.Hidden).Select(e => (e.Index, e.Name)).ToList();
            Minibosses = MinibossDefinitions.Select(m => (m.Index, m.Name)).ToList();
        }

        /* Meta-Knights; NiDL mini-boss 8 (Meta Knight, type 0x39, who calls in his knights: type 0x3A, one graphics descriptor each,
         * mini-boss descriptors 9-12). Frames: Axe Knight's 0-34 (0x08753204), Javelin Knight's 0-57 (0x08753290), Mace Knight's 0-38
         * (0x08753378) and Trident Knight's 0-62 (0x08753414), each with its own palette (palettes 0-3). Hurt: each knight's frame 3
         * (the object's hurt picks it). Variants 0-4: NiDL's five line-ups (0x08745CFC); */
        static Miniboss MetaKnights() {
            List<Part[]> frames = new List<Part[]>();

            frames.AddRange(Enumerable.Range(0, 35).Select(f => new[] { new Part(f) }));
            frames.AddRange(Enumerable.Range(0, 58).Select(f => new[] { new Part(f, Table: 0x753290, Descriptor: 10) }));
            frames.AddRange(Enumerable.Range(0, 39).Select(f => new[] { new Part(f, Table: 0x753378, Descriptor: 11) }));
            frames.AddRange(Enumerable.Range(0, 63).Select(f => new[] { new Part(f, Table: 0x753414, Descriptor: 12) }));

            return new Miniboss(5, "Meta-Knights", 9, 0x753204, frames, new[] { new Part(3) },
                new[] {
                    (0x38, 0), (0x14, 4), (0x14, 2), (0x14, 5), (0x0B, 4),                         // "M" (Mr.Frosty), "eta" (Hot Head), "-" (Roly-Poly);
                    (0x1A, 5), (0x1A, 6), (0x1A, 7), (0x1A, 8), (0x1A, 9), (0x1A, 10), (0x04, 4),   // "Knight" (Sword Knight), "s" (Squishy);
                },
                new byte[] { 0x00, 0x01, 0x02, 0x03, 0x04 }, Type: 0x42,
                ExtraHurts: new List<Part[]> {
                    new[] { new Part(3, Table: 0x753290, Descriptor: 10) },
                    new[] { new Part(3, Table: 0x753378, Descriptor: 11) },
                    new[] { new Part(3, Table: 0x753414, Descriptor: 12) },
                },
                Icon: new[] { 4, 35 + 4, 93 + 9, 132 + 4 });
        }

        /* Fire Lion; NiDL mini-boss 6 (type 0x37). Frames: NiDL's 0-56 (streamed frames 0-51, its fire bits 52-56). Hurt: NiDL's 0,
         * drawn 12 pixels lower (hurtShift 12 in nidl_miniboss.c); */
        static Miniboss FireLion() => new Miniboss(4, "Fire Lion", 6, 0x753718, Enumerable.Range(0, 57).Select(f => new[] { new Part(f) }).ToList(),
            new[] { new Part(0) },
            new[] {
                (0x13, 0), (0x26, 4), (0x01, 1), (0x14, 4), (-1, -1),   // "F" (Flamer), "i" (Cookin), "r" (Bronto Burt), "e" (Hot Head), space;
                (0x0F, 0), (0x26, 4), (0x1E, 1), (0x26, 5),             // "L" (Leap), "i" (Cookin), "o" (Noddy), "n" (Cookin);
            },
            new byte[] { 0x00 }, HurtShift: 12, Type: 0x41, Icon: new[] { 4 });

        /* Bugzzy; NiDL mini-boss 5 (type 0x36). Frames: NiDL's 0-63 (streamed frames). Hurt: NiDL's 3; */
        static Miniboss Bugzzy() => new Miniboss(3, "Bugzzy", 5, 0x7535FC, Enumerable.Range(0, 64).Select(f => new[] { new Part(f) }).ToList(),
            new[] { new Part(3) },
            new[] { (0x28, 0), (0x0C, 1), (0x16, 3), (0x35, 4), (0x35, 4), (0x05, 5) },   // "B" (Bomber), "u" (Cupie), "g" (Pengy), "zz" (Shotzo), "y" (Scarfy);
            new byte[] { 0x00 }, Type: 0x41, Icon: new[] { 4 });

        /* Grand Wheelie; NiDL mini-boss 2 (type 0x33). Frames: NiDL's 0-55 (streamed frames). Hurt: NiDL's 35. Variants 0-3 roll at
         * 1.5, 2.625, 0.9 and 1.57 (0x087444A4); */
        static Miniboss GrandWheelie() => new Miniboss(2, "Grand Wheelie", 2, 0x752F74, Enumerable.Range(0, 56).Select(f => new[] { new Part(f) }).ToList(),
            new[] { new Part(35) },
            new[] {
                (0x03, 0), (0x01, 1), (0x14, 5), (0x26, 5), (0x1E, 2), (-1, -1),   // "G" (Glunk), "r" (Bronto Burt), "a" (Hot Head), "n" (Cookin), "d" (Noddy), space;
                (0x1D, 0), (0x1D, 1), (0x1D, 2), (0x1D, 3), (0x1D, 4), (0x1D, 5), (0x1D, 6),   // "Wheelie";
            },
            new byte[] { 0x00, 0x01, 0x02, 0x03 }, Icon: new[] { 4 });

        /* Mr. Tick-Tock; NiDL mini-boss 4 (type 0x35). Frames: NiDL's 0-28 (its body's are streamed frames, 0-16; its rings 17-22 and
         * notes 23-28 are drawn with its descriptor's palette). Hurt: NiDL's 13. Variant 1 is faster; */
        static Miniboss MrTickTock() => new Miniboss(1, "Mr. Tick-Tock", 4, 0x753180, Enumerable.Range(0, 29).Select(f => new[] { new Part(f) }).ToList(),
            new[] { new Part(13) },
            new[] {
                (0x38, 0), (0x38, 1), (0x38, 2),                                    // "Mr." (Mr.Frosty, written like it);
                (0x1C, 0), (0x1C, 2), (0x0D, 3), (0x0D, 4), (0x0B, 4),              // "T", "i" (Twister), "ck" (Blockin), "-" (Roly-Poly);
                (0x1C, 0), (0x0D, 2), (0x0D, 3), (0x0D, 4),                         // "T" (Twister), "ock" (Blockin);
            },
            new byte[] { 0x00, 0x01 }, Icon: new[] { 4 });

        /* Whispy Woods; NiDL boss 5 (type 0x40, mini-boss descriptor 35). NiDL draws its face as three frames of one table (0x087542C0)
         * at the same spot: the eyes (i), the nose (38 + i) and the mouth (56 + i), each frame placed lower on its own (0x080B2890 adds
         * 26 and 42 to the spot only to ask whether the nose and mouth are on screen, and takes them back before drawing them: added
         * here too, the nose and mouth hung twice as low, the mouth in the roots); its frames 0-17 are the three together. Then its puff (object 0x7E's frames, 0x0874C568, 0-7: NiDL's shared object tiles, palette 15). Hurt: crying (12).
         * Its apples are an enemy (Apple); */
        static Miniboss WhispyWoods() {
            Part[] Face(int i) => new[] { new Part(i), new Part(38 + i), new Part(56 + i) };
            List<Part[]> frames = Enumerable.Range(0, 18).Select(Face).ToList();
            frames.AddRange(Enumerable.Range(0, 8).Select(f => new[] { new Part(f, Table: 0x74C568, Palette15: true, Common: true) }));

            return new Miniboss(0, "Whispy Woods", 35, 0x7542C0, frames, Face(12),
                new[] {
                    (0x00, 0), (0x23, 1), (0x26, 4), (0x04, 4), (0x02, 3), (0x05, 5), (-1, -1),   // "W" (Waddle Dee), "h" (Shooty), "i" (Cookin), "s" (Squishy), "p" (Blipper), "y" (Scarfy), space;
                    (0x00, 0), (0x1E, 1), (0x1E, 1), (0x1E, 2), (0x04, 4),                        // "W" (Waddle Dee), "o", "o", "d" (Noddy), "s" (Squishy);
                },
                new byte[0], Type: BossID, Icon: new[] { 4 }, IconOverRoom: true);
        }

        /* Kracko; NiDL boss 6 (type 0x41, Grape Garden's), its cloud fight. NiDL draws it with three palettes at once (the cloud's top 0,
         * its bottom 1, the eye and lightning 2), a KatAM sprite has one: its object draws the cloud's top, two more sprites follow it
         * (bosses/kracko.c), so the cloud's frames are cut by palette. Frames: the cloud's top (NiDL's 12, then 13 and 14, its spikes out),
         * 0-2; its bottom, 3-5; its eye (4-11: looking right, then each eighth of a turn clockwise; NiDL's own object, at the cloud's
         * middle), 6-13; its lightning bolts (14-29), one or more of the lightning object's segments (0x0C5, its table 0x08754448: 4-11
         * run down) stacked 16 pixels apart, 1-8 of them, two of each (jagged differently). Hurt: the cloud's top; */
        static Miniboss Kracko() {
            int[] jitter = { 0, 2, -2, 1, -1, 3, -3, 1 };
            List<Part[]> frames = new List<Part[]>();

            for (int palette = 0; palette < 2; palette++)
                frames.AddRange(new[] { 12, 13, 14 }.Select(cloud => new[] { new Part(cloud, Palette: palette, OnlyPalette: palette) }));
            frames.AddRange(Enumerable.Range(4, 8).Select(eye => new[] { new Part(eye, Palette: 2) }));
            for (int length = 1; length <= 8; length++)
                for (int jag = 0; jag < 2; jag++)
                    frames.Add(Enumerable.Range(0, length).Select(k => new Part(4 + (k * 5 + jag * 3) % 8, Table: 0x754448, Palette: 2,
                                                                                    Dx: jitter[(k + jag * 3) % 8], Dy: 8 + 16 * k)).ToArray());

            return new Miniboss(1, "Kracko", 36, 0x7543E8, frames, new[] { new Part(12, OnlyPalette: 0) },
                new[] { (0x45, 0), (0x45, 1), (0x45, 2), (0x45, 3), (0x45, 4), (0x45, 5) },    // "Kracko" (KatAM's own Kracko);
                new byte[0], Type: BossID, IconCells: new[] { new[] { 0, 3, 6 } });
        }

        /* Mr. Shine & Mr. Bright; NiDL boss 4 (type 0x3F, Butter Building's). Two frame tables of one descriptor: Mr. Shine's (0x08754210:
         * his body 10-18, the moon 4-9, the crescent 20-27, the fireball 28-29, the sun's beam 34-35, with the descriptor's palettes 0, 1
         * and 2; the falling star is an enemy, FallingStar) and Mr. Bright's (0x0875412C: his flaming body, streamed frames with their own
         * palette, facing 21-32 and rolling 33-44; his face and hands 4-15, drawn over it; the sun 16-19). Frames as bosses/shine_bright.c numbers
         * them: 0-8 Mr. Shine, 9-14 the moon, 15-38 Mr. Bright's body, 39-50 his face, 51-54 the sun, 55-62 the crescent, 63-64 the
         * fireball, 65-66 the beam (two of its frames, stacked). Hurt: Mr. Shine cracked (0); */
        static Miniboss ShineAndBright() {
            const int Shine = 0x754210, Bright = 0x75412C;
            List<Part[]> frames = new List<Part[]>();

            frames.AddRange(Enumerable.Range(10, 9).Select(f => new[] { new Part(f) }));
            frames.AddRange(Enumerable.Range(4, 6).Select(f => new[] { new Part(f) }));
            frames.AddRange(Enumerable.Range(21, 24).Select(f => new[] { new Part(f, Table: Bright) }));
            frames.AddRange(Enumerable.Range(4, 12).Select(f => new[] { new Part(f, Table: Bright) }));
            frames.AddRange(Enumerable.Range(16, 4).Select(f => new[] { new Part(f, Table: Bright) }));
            frames.AddRange(Enumerable.Range(20, 8).Select(f => new[] { new Part(f) }));
            frames.AddRange(Enumerable.Range(28, 2).Select(f => new[] { new Part(f, Palette: 1) }));
            frames.AddRange(new[] { 34, 35 }.Select(f => new[] { new Part(f, Palette: 2, Dy: 48), new Part(f, Palette: 2, Dy: 64) }));

            return new Miniboss(2, "Mr. Shine & Mr. Bright", 34, Shine, frames, new[] { new Part(0) },
                new[] {
                    (0x04, 0), (0x23, 1), (0x26, 4), (0x26, 5), (0x02, 5), (-1, -1),               // "S" (Squishy), "h" (Shooty), "in" (Cookin), "e" (Blipper), space;
                    (0x28, 0), (0x01, 1), (0x26, 4), (0x11, 2), (0x23, 1), (0x01, 4),              // "B" (Bomber), "r" (Bronto Burt), "i", "g" (Big Waddle Dee), "h", "t" (Bronto Burt);
                },
                new byte[0], Type: BossID, IconCells: new[] { new[] { 0 }, new[] { 15, 39 } });
        }

        /* Meta Knight; NiDL boss 2 (type 0x3D, Orange Ocean's). His body is streamed frames (0x08753BB4: his cape 0-9, his poses 21-52,
         * each with its own palette), Galaxia a frame of the sword table (0x08753DA0: 16-23, eight ways, the descriptor's second palette,
         * gold). Frames as bosses/meta_knight.c numbers them: 0-9 the cape, 10-41 the poses, 42-49 the sword. Hurt: standing (21); */
        static Miniboss MetaKnight() {
            List<Part[]> frames = new List<Part[]>();

            frames.AddRange(Enumerable.Range(0, 10).Select(f => new[] { new Part(f) }));
            frames.AddRange(Enumerable.Range(21, 32).Select(f => new[] { new Part(f) }));
            frames.AddRange(Enumerable.Range(16, 8).Select(f => new[] { new Part(f, Table: 0x753DA0, Palette: 1) }));

            return new Miniboss(3, "Meta Knight", 32, 0x753BB4, frames, new[] { new Part(21) },
                new[] {
                    (0x38, 0), (0x14, 4), (0x14, 2), (0x14, 5), (-1, -1),                          // "M" (Mr.Frosty), "eta" (Hot Head), space;
                    (0x1A, 5), (0x1A, 6), (0x1A, 7), (0x1A, 8), (0x1A, 9), (0x1A, 10),             // "Knight" (Sword Knight);
                },
                new byte[0], Type: BossID, IconCells: new[] { new[] { 0, 10 } });
        }

        /* Paint Roller; NiDL boss 1 (type 0x3C, Ice Cream Island's). His frames are streamed frames with two palettes each (0x08753994),
         * cut by palette (OnlyPalette): 0-40 the first palette's pieces, 41-81 the second's (bosses/paint_roller.c draws them as two
         * sprites). Type 0x44 (BossID2: type 0x43's frames are full). Hurt: standing (0), its first palette's pieces; */
        static Miniboss PaintRoller() {
            List<Part[]> frames = new List<Part[]>();

            for (int palette = 0; palette < 2; palette++)
                frames.AddRange(Enumerable.Range(0, 41).Select(f => new[] { new Part(f, OnlyPalette: palette) }));

            return new Miniboss(4, "Paint Roller", 31, 0x753994, frames, new[] { new Part(0, OnlyPalette: 0) },
                new[] {
                    (0x16, 0), (0x2D, 1), (0x26, 4), (0x26, 5), (0x23, 4), (-1, -1),               // "P" (Pengy), "a" (Batty), "in" (Cookin), "t" (Shooty), space;
                    (0x17, 0), (0x1E, 1), (0x02, 1), (0x02, 1), (0x02, 5), (0x01, 1),              // "R" (Rocky), "o" (Noddy), "ll" (Blipper), "e", "r" (Bronto Burt);
                },
                new byte[0], Type: BossID2, IconCells: new[] { new[] { 0, 41 } });
        }

        /* Heavy Mole; NiDL boss 3 (type 0x3E, Yogurt Yard's; mini-boss descriptor 33, five palettes). Frames as bosses/heavy_mole.c numbers
         * them: 0-7 the body (0x08753E3C's 0-7, the first palette), 8-43 its arms (26-61: six ways of six, the second palette), 44-67 the
         * fired balls (64-87: six ways of four, the second palette). Type 0x44. Hurt: its body (0); */
        static Miniboss HeavyMole() {
            List<Part[]> frames = new List<Part[]>();

            frames.AddRange(Enumerable.Range(0, 8).Select(f => new[] { new Part(f) }));
            frames.AddRange(Enumerable.Range(26, 36).Select(f => new[] { new Part(f, Palette: 1) }));
            for (int way = 0; way < 6; way++)
                frames.AddRange(Enumerable.Range(64 + way * 4, 4).Select(f => new[] { new Part(f, Palette: 1) }));

            return new Miniboss(5, "Heavy Mole", 33, 0x753E3C, frames, new[] { new Part(0) },
                new[] {
                    (0x29, 0), (0x29, 1), (0x29, 2), (0x29, 3), (0x29, 4), (-1, -1),               // "Heavy" (Heavy Knight), space;
                    (0x38, 0), (0x1E, 1), (0x02, 1), (0x02, 5),                                    // "M" (Mr.Frosty), "o" (Noddy), "le" (Blipper);
                },
                new byte[0], Type: BossID2);
        }

        /* King Dedede; NiDL boss 0 (type 0x3B, Rainbow Resort's; mini-boss descriptor 30 holds only his effects). His frames are streamed
         * frames (0x087538E0), with the same two palettes in every one, cut by palette as Paint Roller's: the ones he uses (used,
         * bosses/dedede.c's sFrames), the first palette's pieces, then the same, the second's (drawn as two sprites). Type 0x44. Hurt:
         * standing (4), its first palette's pieces; */
        static Miniboss KingDedede() {
            int[] used = { 4, 5, 6, 7, 13, 14, 15, 16, 17, 18, 19, 24, 25, 26, 27, 28, 29, 34, 35, 36, 39, 41, 42, 43 };
            List<Part[]> frames = new List<Part[]>();

            for (int palette = 0; palette < 2; palette++)
                frames.AddRange(used.Select(f => new[] { new Part(f, OnlyPalette: palette) }));

            return new Miniboss(6, "King Dedede", 30, 0x7538E0, frames, new[] { new Part(4, OnlyPalette: 0) },
                new[] {
                    (0x46, 0), (0x46, 1), (0x46, 2), (0x46, 3), (-1, -1),                          // "King" (King Golem), space;
                    (0x32, 0), (0x02, 5), (0x00, 2), (0x02, 5), (0x00, 2), (0x02, 5),              // "D" (Droppy), "e" (Blipper), "d" (Waddle Dee);
                },
                new byte[0], Type: BossID2);
        }

        // OrbDot; The Power Orb's end's dot: 3 black pixels across (NiDL's palette 15's color 7), as NiDL loads it with the orb's end (its
        // VRAM's tile 28); (a property: the bosses' list is made before the static fields after it);
        static byte[] OrbDot => Convert.FromHexString("0000000000000000000000000000770000007707000077000000000000000000");
        /* Nightmare's Power Orb; NiDL boss 7 (type 0x42, the Fountain of Dreams' first; mini-boss descriptor 37): only the frames it uses
         * (type 0x44's 256 are shared): 0x087544B4's 4-7, spinning, 8-11 shrinking (11, the smallest, blinks as it charges), then its
         * shots' (its 20-29 and 39-42, OrbShots: the spinning star 20-23, squashed as it dashes 24-27, the crescents 28-29, the sparkles
         * a dashing star trails 39-42), then the slash Kirby's swing makes in its intro (its 14-19, a line of pieces growing across the
         * screen; Shared, 16 tiles; its second palette), then its end's dot (OrbDot: NiDL draws the orb and the star with its Kirbys so
         * far off in the moon; EffectOnly, only its end's effect draws it, as type 0x44's 256 frames are full), numbered from 0
         * (bosses/power_orb.c). Type 0x44. Hurt: its 0; */
        static Miniboss PowerOrb() => new Miniboss(7, "Power Orb", 37, 0x7544B4,
            Enumerable.Range(4, 8).Select(f => new[] { new Part(f) })
                .Concat(Enumerable.Range(20, 10).Concat(Enumerable.Range(39, 4)).Select(f => new[] { new Part(f, OrbShots: true) }))
                .Concat(Enumerable.Range(14, 6).Select(f => new[] { new Part(f, Shared: true) }))
                .Concat(new[] { new[] { new Part(0, Tile: OrbDot) } }).ToList(),
            new[] { new Part(0) },
            new[] {
                (0x16, 0), (0x1E, 1), (0x1C, 1), (0x02, 5), (0x01, 1), (-1, -1),               // "P" (Pengy), "o" (Noddy), "w" (Twister), "e" (Blipper), "r" (Bronto Burt), space;
                (0x1B, 2), (0x01, 1), (0x28, 3),                                                // "O" (UFO), "r", "b" (Bomber);
            },
            new byte[0], Type: BossID2, EffectOnly: 1);

        /* Nightmare; NiDL boss 8 (type 0x43, the Fountain of Dreams' last; mini-boss descriptor 38). His frames are streamed frames with
         * three palettes (0x0875456C's 8-15, cut by palette: 0-7 the first's pieces, 8-15 the second's, 16-23 the third's; bosses/
         * nightmare.c draws them as three sprites), then his beams (61-63, their own palette), then his intro's (EffectOnly: its effects
         * draw them): the comet he arrives as and the blue star he throws (Cutscene, the cutscene's 0-5 and 6), the burst a blue star
         * makes (NiDL's shared effects' 0x0874CA78, its 0-22, with the shared tiles and palette 15). Type 0x44. Hurt: his collar and
         * arms (8); */
        static Miniboss Nightmare() {
            List<Part[]> frames = new List<Part[]>();

            for (int palette = 0; palette < 3; palette++)
                frames.AddRange(Enumerable.Range(8, 8).Select(f => new[] { new Part(f, OnlyPalette: palette) }));
            frames.AddRange(Enumerable.Range(61, 3).Select(f => new[] { new Part(f) }));
            frames.AddRange(Enumerable.Range(0, 7).Select(f => new[] { new Part(f, Cutscene: f < 6 ? 1 : 2) }));
            frames.AddRange(Enumerable.Range(0, 23).Select(f => new[] { new Part(f, Table: 0x74CA78, Palette15: true, Common: true) }));

            return new Miniboss(8, "Nightmare", 38, 0x75456C, frames, new[] { new Part(8, OnlyPalette: 0) },
                new[] {
                    (0x1E, 0), (0x26, 4), (0x29, 8), (0x29, 9), (0x29, 10),                        // "N" (Noddy), "i" (Cookin), "ght" (Heavy Knight);
                    (0x28, 2), (0x2D, 1), (0x01, 1), (0x02, 5),                                    // "m" (Bomber), "a" (Batty), "r" (Bronto Burt), "e" (Blipper);
                },
                new byte[0], Type: BossID2, EffectOnly: 30);
        }

        /* Apple; Whispy Woods' apples (NiDL object 0x7D, its own frame table 0x08754308: Whispy Woods' frames 18-37, its second palette).
         * Hurt: its 4; */
        static Enemy Apple() => new Enemy(15, "Apple", 0, 0x754308, Enumerable.Range(0, 20).Select(f => new[] { new Part(f, Palette: 1) }).ToList(),
            new[] { new Part(4, Palette: 1) }, new Dictionary<int, ushort>(),
            new[] { (0x2D, 1), (0x02, 3), (0x02, 3), (0x02, 1), (0x02, 5) },   // "a" (Batty), "pp", "l", "e" (Blipper);
            new byte[0], Descriptor: 35, Palette: 1, Hidden: true);

        /* Falling star; Mr. Shine's from the sky (Mr. Shine & Mr. Bright's frame table 0x08754210, frames 38-43, its second palette), an
         * enemy so Kirby can inhale it and spit it back. Hurt: its 38; */
        static Enemy FallingStar() => new Enemy(16, "Falling Star", 0, 0x754210, Enumerable.Range(38, 6).Select(f => new[] { new Part(f, Palette: 1) }).ToList(),
            new[] { new Part(38, Palette: 1) }, new Dictionary<int, ushort>(),
            new[] { (0x04, 0), (0x23, 4), (0x2D, 1), (0x01, 1) },   // "S" (Squishy), "t" (Shooty), "a" (Batty), "r" (Bronto Burt);
            new byte[0], Descriptor: 34, Palette: 1, Hidden: true);

        /* Spit star; the star King Dedede spits the Kirbys he swallowed out in (NiDL object 0x70: frames of NiDL's shared effects,
         * 0x0874C44C, with its shared tiles and palette 15): 4 the star, 35-37 its burst, then the sparkles of his inhale (45); and,
         * from 5, the Star Rod's star (abilities/star_rod.c: NiDL's object 6, its 38-41 spinning, 42-43 its trail's sparkle, then its
         * burst, 0x0874C650's 0-10). Not hit; hurt: its star; */
        static Enemy SpitStar() => new Enemy(17, "Spit Star", 0, 0x74C44C,
            new[] { 4, 35, 36, 37, 45 }.Select(f => new[] { new Part(f, Palette15: true, Common: true) })
                .Concat(new[] { 38, 39, 40, 41, 42, 43 }.Select(f => new[] { new Part(f, Palette15: true, StarRod: true) }))
                .Concat(Enumerable.Range(0, 11).Select(f => new[] { new Part(f, Table: 0x74C650, Palette15: true, Common: true) })).ToList(),
            new[] { new Part(4, Palette15: true, Common: true) }, new Dictionary<int, ushort>(),
            new[] { (0x04, 0), (0x23, 4), (0x2D, 1), (0x01, 1) },   // "S" (Squishy), "t" (Shooty), "a" (Batty), "r" (Bronto Burt);
            new byte[0], Descriptor: 30, Hidden: true);

        /* Poppy Bros. Sr.; NiDL mini-boss 1 (type 0x32). Frames: every body frame (NiDL's 4-9, hopping) with every head frame over it (12-19,
         * drawn by its head object in NiDL), then its hands (20-25) and its bomb (26-30, NiDL's second palette). Hurt: NiDL's 10;
         * Variants 0 and 1 hop a different number of times before each bomb (0x087438DC); */
        static Miniboss PoppyBrosSr() {
            List<Part[]> frames = new List<Part[]>();

            for (int body = 4; body <= 9; body++)
                for (int head = 12; head <= 19; head++) frames.Add(new[] { new Part(head), new Part(body) });
            frames.AddRange(Enumerable.Range(20, 6).Select(f => new[] { new Part(f) }));
            frames.AddRange(Enumerable.Range(26, 5).Select(f => new[] { new Part(f, Palette: 1) }));

            return new Miniboss(0, "Poppy Bros. Sr.", 1, 0x7530F8, frames, new[] { new Part(10) },
                new[] {
                    (0x16, 0), (0x1E, 1), (0x02, 3), (0x02, 3), (0x05, 5), (-1, -1),   // "P" (Pengy), "o" (Noddy), "pp" (Blipper), "y" (Scarfy), space;
                    (0x28, 0), (0x01, 1), (0x1E, 1), (0x04, 4), (-1, -1),               // "B" (Bomber), "r" (Bronto Burt), "o" (Noddy), "s" (Squishy), space;
                    (0x04, 0), (0x01, 1),                                               // "S" (Squishy), "r" (Bronto Burt);
                },
                new byte[] { 0x00, 0x01 });
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

        // MinibossBehaviorGroupsOf(); The behavior bytes of a mini-boss type (MinibossIDs) the randomizer may use, one list per mini-boss picked;
        public static List<List<byte>> MinibossBehaviorGroupsOf(IEnumerable<int> minibosses, byte type) =>
            MinibossDefinitions.Where(m => m.Type == type && minibosses.Contains(m.Index)).Select(m => m.Behaviors.Select(b => (byte)(m.Index << 4 | b)).ToList())
                               .Where(group => group.Count > 0).ToList();

        // MinibossNameOf(); The ported mini-boss a behavior byte of a mini-boss type picks;
        public static string MinibossNameOf(byte behavior) => MinibossDefinitions.FirstOrDefault(m => m.Index == behavior >> 4)?.Name ?? MinibossDefinitions[0].Name;

        // NameOf(); The ported enemy a behavior byte of type 0x31 picks;
        public static string NameOf(byte behavior) => Definitions.FirstOrDefault(e => e.Index == behavior >> 4)?.Name ?? Definitions[0].Name;

        /* AddEnemies(); Adds every ported enemy as object type 0x31, expanding the ROM to 32 MB (rom is replaced by the bigger one).
         * Returns null when done, or why it couldn't be done (rom is left as it was); */
        public static string AddEnemies(ref byte[] rom, byte[] nightmare, bool rooms = true, bool title = false) {
            KatAMExpansionCode.Version version = VersionOf(rom);
            NightmareRegion region = RegionOf(nightmare);

            if (version == null) return "The ROM Expansion supports the USA, European and Japanese (revision 1) versions of Kirby & the Amazing Mirror.";
            if (region == null) return "That isn't a Kirby: Nightmare in Dream Land ROM.";
            if (region.KatAM != version.Name)
                return $"The Nightmare in Dream Land ROM must be from the same region as Kirby & the Amazing Mirror: {version.Name} with {version.Name} (this one is from {region.KatAM}).";
            if (rom.Length > ExpandedSize) return "That ROM is bigger than the GBA can map (32 MB).";

            byte[] original = rom;

            rom = Expand(rom);
            string error = AddEnemiesTo(rom, nightmare, region, version, rooms, title);
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

        static string AddEnemiesTo(byte[] rom, byte[] nightmare, NightmareRegion region, KatAMExpansionCode.Version version, bool rooms, bool title) {
            int HurtAnimID = version.AnimCount,     // The first free anim ID (the vanilla table's size);
                SpriteTables = version.SpriteTables, SpawnTable = version.SpawnTable, ObjectParameters = version.ObjectParameters,
                ObjectParameters2 = version.ObjectParameters2, HitFlashBranch = version.HitFlashBranch;

            for (int i = CodeAddress; i < CodeAddress + ReservedSize; i++) {
                if (rom[i] != 0xFF) return "The free space used by the ROM Expansion is already in use.";
            }

            List<(Enemy enemy, byte[] palette, List<(byte[] tiles, ushort[] oams, short[] attributes, int palette)> frames, byte[] hurt)> built;
            List<(List<byte[]> palettes, List<(byte[] tiles, ushort[] oams, short[] attributes, int palette)> frames, byte[] hurt)> builtMinibosses;

            try {
                built = Definitions.Select(e => Build(nightmare, region, e)).ToList();
                builtMinibosses = AllBosses.Select(m => BuildGraphics(nightmare, region,
                    ReadInt(nightmare, region.Descriptors + MinibossDescriptors + m.NightmareIndex * 4) - 0x08000000, m.FrameTable, m.Frames,
                    new[] { m.Hurt }.Concat(m.ExtraHurts ?? new List<Part[]>()).ToList(), new Dictionary<int, ushort>(), HurtOf(m.Type).Width,
                    HurtOf(m.Type).Height, HurtOf(m.Type).OriginX, HurtOf(m.Type).OriginY, m.HurtShift)).ToList();
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
            List<Miniboss> allBosses = AllBosses;  // The mini-bosses, then the bosses (their graphics are built the same way);
            (int animBase, int animId, int hurtTiles)[] minibossData = new (int, int, int)[allBosses.Count];
            List<(int animId, int variant)> animInfos = new List<(int, int)> { (HurtAnimID, 0), (HurtAnimID, 0) };

            List<KatAMTitleScreen.NidlType> paradeTypes = new List<KatAMTitleScreen.NidlType>();

            /* AddAnim(); One anim (its ID: the next free one) for an enemy's or mini-boss's palettes, frames and hurt frame; its frames go
             * to the type's AnimInfo table infos (but the last effectOnly: only its effects draw them, by the anim's variants). Returns its
             * first AnimInfo, anim ID and where its hurt frame went. An enemy in the parade (ParadeLoops) gets a variant more, after its
             * frames: its loop, and a parade type; */
            (int, int, int) AddAnim(List<byte[]> palettes, List<(byte[] tiles, ushort[] oams, short[] attributes, int palette)> frames, byte[] hurt,
                                    List<(int animId, int variant)> infos, string parade = null, int effectOnly = 0) {
                int animId = HurtAnimID + animVariants.Count;

                // Tiles and palettes are indexes from their table's base, so they are aligned to it;
                int tilesAt = Allocate(ref cursor, frames.Sum(f => f.tiles.Length), 32, tilesBase);
                int[] paletteIndexes = palettes.Select(palette => {
                    int paletteAt = Allocate(ref cursor, 32, 32, paletteBase);
                    palette.CopyTo(rom, paletteAt);
                    return (paletteAt - paletteBase) / 32;
                }).ToArray();
                int hurtAt = Allocate(ref cursor, hurt.Length, 4);
                hurt.CopyTo(rom, hurtAt);

                int tileIndex = (tilesAt - tilesBase) / 32;
                int oamCount = frames.Sum(f => f.oams.Length);
                int attributesAt = Allocate(ref cursor, frames.Count * 12, 4), oamsAt = Allocate(ref cursor, oamCount * 2, 4);
                bool inParade = parade != null && ParadeLoops.ContainsKey(parade);
                int variantsAt = Allocate(ref cursor, (frames.Count + (inParade ? 1 : 0)) * 4, 4);
                int[] frameTiles = new int[frames.Count];

                // Animation commands of every frame: GetPalette, GetTiles, ShowFrame and End (holds the frame);
                for (int i = 0, tile = 0, oam = 0; i < frames.Count; i++) {
                    frameTiles[i] = tile;
                    (byte[] tiles, ushort[] oams, short[] attributes, int palette) = frames[i];
                    int commands = Allocate(ref cursor, 36, 4);

                    tiles.CopyTo(rom, tilesAt + tile * 32);
                    WriteInts(rom, commands, -2, palette < 0 ? palette15Index : paletteIndexes[palette], 16, -1, tileIndex + tile, tiles.Length / 32, 60, i, -4);
                    WriteInt(rom, variantsAt + i * 4, commands + 0x08000000);
                    WriteShorts(rom, attributesAt + i * 12, (short)(oam / 3), attributes[1], attributes[2], attributes[3], attributes[4], attributes[5]);
                    for (int o = 0; o < oams.Length; o++) WriteShort(rom, oamsAt + (oam + o) * 2, oams[o]);

                    tile += tiles.Length / 32;
                    oam += oams.Length;
                }

                /* The parade's loop: each step's GetPalette, GetTiles and ShowFrame (8 words), then JumpBack to the first; its type stands
                 * on the line the parade walks on (its frames' lowest point: height - origin's y), as wide as its widest frame; */
                if (inParade) {
                    (int kind, (int frame, int delay)[] loop) = ParadeLoops[parade];
                    int commands = Allocate(ref cursor, loop.Length * 32 + 8, 4);

                    for (int s = 0; s < loop.Length; s++) {
                        (int frame, int delay) = loop[s];
                        int palette = frames[frame].palette;
                        WriteInts(rom, commands + s * 32, -2, palette < 0 ? palette15Index : paletteIndexes[palette], 16, -1,
                                  tileIndex + frameTiles[frame], frames[frame].tiles.Length / 32, delay, frame);
                    }
                    WriteInts(rom, commands + loop.Length * 32, -3, loop.Length * 8);
                    WriteInt(rom, variantsAt + frames.Count * 4, commands + 0x08000000);
                    var shown = loop.Select(s => frames[s.frame]).ToList();
                    paradeTypes.Add(new KatAMTitleScreen.NidlType(kind, animId, frames.Count, shown.Max(f => f.tiles.Length / 32),
                                                                  (shown.Max(f => (int)f.attributes[2]) + 7) & ~7, shown.Max(f => f.attributes[3] - f.attributes[5])));
                }

                int animBase = infos.Count;
                for (int i = 0; i < frames.Count - effectOnly; i++) infos.Add((animId, i));

                animVariants.Add(variantsAt);
                animAttributes.Add(attributesAt);
                animOams.Add(oamsAt);
                return (animBase, animId, hurtAt);
            }

            foreach ((Enemy enemy, byte[] palette, List<(byte[] tiles, ushort[] oams, short[] attributes, int palette)> frames, byte[] hurt) in built)
                data.Add(AddAnim(new List<byte[]> { palette }, frames, hurt, animInfos, enemy.Name));

            /* Per mini-boss type: its hurt anim (HurtOf: drawing the last tiles of the object's VRAM, after the type's biggest frame), then
             * its mini-bosses' anims; */
            Dictionary<byte, int> minibossFrameTiles = new Dictionary<byte, int>();
            Dictionary<byte, List<(int animId, int variant)>> minibossAnimInfos = new Dictionary<byte, List<(int, int)>>();
            foreach (byte type in BossTypes) {
                List<int> ofType = Enumerable.Range(0, allBosses.Count).Where(i => allBosses[i].Type == type).ToList();
                if (ofType.Count == 0) continue;

                MinibossHurt typeHurt = HurtOf(type);
                int typeFrameTiles = minibossFrameTiles[type] = ofType.SelectMany(i => builtMinibosses[i].frames).Max(f => f.tiles.Length / 32);
                int typeHurtCommands = Allocate(ref cursor, 12, 4);
                WriteInts(rom, typeHurtCommands, 60, 0, -4);
                int typeHurtVariants = Allocate(ref cursor, 4, 4);
                WriteInt(rom, typeHurtVariants, typeHurtCommands + 0x08000000);
                int typeHurtAttributes = Allocate(ref cursor, 12, 4);
                WriteShorts(rom, typeHurtAttributes, 0, (short)typeHurt.Pieces.Length, (short)typeHurt.Width, (short)typeHurt.Height, (short)typeHurt.OriginX,
                            (short)typeHurt.OriginY);
                int typeHurtOam = Allocate(ref cursor, typeHurt.Pieces.Length * 6, 4);
                for (int p = 0; p < typeHurt.Pieces.Length; p++) {
                    (int shape, int size, int x, int y, int tile) = typeHurt.Pieces[p];
                    WriteShorts(rom, typeHurtOam + p * 6, (short)(y | shape << 14), (short)(x | size << 14), (short)(typeFrameTiles + tile));
                }
                int typeHurtAnimID = HurtAnimID + animVariants.Count;
                animVariants.Add(typeHurtVariants);
                animAttributes.Add(typeHurtAttributes);
                animOams.Add(typeHurtOam);

                // Hurt frames 0 and 1, and 2: a defeated mini-boss shows the one after its second (sub_0809D1E0);
                List<(int animId, int variant)> infos = minibossAnimInfos[type] = Enumerable.Repeat((typeHurtAnimID, 0), 3).ToList();
                foreach (int i in ofType) {
                    (List<byte[]> palettes, List<(byte[] tiles, ushort[] oams, short[] attributes, int palette)> frames, byte[] hurt) = builtMinibosses[i];
                    minibossData[i] = AddAnim(palettes, frames, hurt, infos, effectOnly: allBosses[i].EffectOnly);
                }
                if (infos.Count > 256) throw new InvalidOperationException($"The ported mini-bosses of type 0x{type:X2} have more than 256 frames.");
            }

            if (animInfos.Count > 256) throw new InvalidOperationException("The ported enemies have more than 256 frames.");

            // The new abilities' hats and motions (anims after the enemies');
            int[] hatTables, motions, hatPalettes = new int[NewAbilities];
            int backdropPalette;
            bool[][] paletteUsed = new bool[NewAbilities][];
            try {
                hatTables = BuildHats(rom, nightmare, region, version.RomSymbols["gUnk_0834C5D4"], anims, attrTable, oamTable, tilesBase, paletteBase, ref cursor, HurtAnimID,
                                      animVariants, animAttributes, animOams, hatPalettes, paletteUsed);
                motions = BuildMotions(rom, nightmare, region, hatTables, hatPalettes, paletteUsed, tilesBase, ref cursor, HurtAnimID, animVariants,
                                       animAttributes, animOams);
                backdropPalette = BuildBackdropPalette(rom, nightmare, region, version.RomSymbols["gUnk_08350A3C"], anims, attrTable, oamTable, paletteBase,
                                                       ref cursor, HurtAnimID, animVariants, animAttributes, animOams);
            } catch (Exception exception) when (exception is IndexOutOfRangeException or ArgumentException) {
                return $"The Nightmare in Dream Land ROM doesn't match the {region.KatAM} release.";
            }

            // Extended anims, attrs and oamData tables (indexed by anim ID);
            int[] tables = { anims, attrTable, oamTable };
            List<int>[] entries = { animVariants, animAttributes, animOams };
            for (int t = 0; t < 3; t++) {
                int at = Allocate(ref cursor, (HurtAnimID + entries[t].Count) * 4, 4);

                Array.Copy(rom, tables[t], rom, at, HurtAnimID * 4);
                for (int i = 0; i < entries[t].Count; i++) WriteInt(rom, at + (HurtAnimID + i) * 4, entries[t][i] + 0x08000000);
                WriteInt(rom, SpriteTables + t * 4, at + 0x08000000);
            }

            // The intro's parade: the ported enemies' types (their anims are in place now);
            int paradeAt = Allocate(ref cursor, KatAMTitleScreen.NidlTypesSize(paradeTypes.Count), 4);
            KatAMTitleScreen.AddNidlTypes(rom, paradeAt, paradeTypes);

            int animInfosAt = Allocate(ref cursor, animInfos.Count * 4, 4);
            for (int i = 0; i < animInfos.Count; i++) {
                WriteShort(rom, animInfosAt + i * 4, animInfos[i].animId);
                rom[animInfosAt + i * 4 + 2] = (byte)animInfos[i].variant;
            }

            Dictionary<byte, int> minibossAnimInfosAt = new Dictionary<byte, int>();
            foreach ((byte type, List<(int animId, int variant)> infos) in minibossAnimInfos) {
                int at = minibossAnimInfosAt[type] = Allocate(ref cursor, infos.Count * 4, 4);
                for (int i = 0; i < infos.Count; i++) {
                    WriteShort(rom, at + i * 4, infos[i].animId);
                    rom[at + i * 4 + 2] = (byte)infos[i].variant;
                }
            }

            // HP bar names, one per enemy, then one per mini-boss, then one per boss;
            int namesAt = Allocate(ref cursor, Definitions.Count * 0x100, 4), minibossNamesAt = Allocate(ref cursor, MinibossDefinitions.Count * 0x100, 4),
                bossNamesAt = Allocate(ref cursor, BossDefinitions.Count * 0x100, 4);
            for (int i = 0; i < Definitions.Count; i++) ComposeName(rom, version, Definitions[i].Glyphs).CopyTo(rom, namesAt + i * 0x100);
            for (int i = 0; i < MinibossDefinitions.Count; i++) ComposeName(rom, version, MinibossDefinitions[i].Glyphs).CopyTo(rom, minibossNamesAt + i * 0x100);
            for (int i = 0; i < BossDefinitions.Count; i++) ComposeName(rom, version, BossDefinitions[i].Glyphs).CopyTo(rom, bossNamesAt + i * 0x100);

            // gNidlData: names, then per enemy (animBase, animId, hurt tiles);
            int nidlData = CodeAddress + version.NidlData;
            WriteInt(rom, nidlData, namesAt + 0x08000000);
            for (int i = 0; i < data.Count; i++) {
                int entry = nidlData + 4 + Definitions[i].Index * 8;
                WriteShort(rom, entry, data[i].animBase);
                WriteShort(rom, entry + 2, data[i].animId);
                WriteInt(rom, entry + 4, data[i].hurtTiles + 0x08000000);
            }
            // Then the mini-bosses': names, then per mini-boss (animBase, animId, hurt tiles);
            int minibossNidlData = nidlData + 4 + EnemyCount * 8;
            WriteInt(rom, minibossNidlData, minibossNamesAt + 0x08000000);
            for (int i = 0; i < MinibossDefinitions.Count; i++) {
                int entry = minibossNidlData + 4 + MinibossDefinitions[i].Index * 8;
                WriteShort(rom, entry, minibossData[i].animBase);
                WriteShort(rom, entry + 2, minibossData[i].animId);
                WriteInt(rom, entry + 4, minibossData[i].hurtTiles + 0x08000000);
            }
            // Then the bosses' the same way;
            int bossNidlData = minibossNidlData + 4 + MinibossCount * 8;
            WriteInt(rom, bossNidlData, bossNamesAt + 0x08000000);
            for (int i = 0; i < BossDefinitions.Count; i++) {
                int entry = bossNidlData + 4 + BossDefinitions[i].Index * 8;
                (int animBase, int animId, int hurtTiles) = minibossData[MinibossDefinitions.Count + i];
                WriteShort(rom, entry, animBase);
                WriteShort(rom, entry + 2, animId);
                WriteInt(rom, entry + 4, hurtTiles + 0x08000000);
            }

            WriteMapTable(rom, nightmare, region, builtMinibosses, ref cursor);

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

            /* Object types 0x40-0x42: spawn function and parameters, as Mr. Frosty's (type 0x38: score, HP) but for its hurt frames (0 &
             * 1), no ability (the code sets each mini-boss's), no palette key, VRAM tiles (the type's biggest frame, then the hurt frame),
             * behavior start and animations. Their second parameters are Mr. Frosty's; */
            /* Type 0x43 (the bosses) the same, as Kracko's (0x45: score, HP; the code sets each boss's HP) with the bosses' functions; */
            const int FrostyID = 0x38, KrackoID = 0x45;
            foreach (byte type in minibossAnimInfosAt.Keys) {
                int minibossParameters = ObjectParameters + type * 0x18, like = IsBossType(type) ? KrackoID : FrostyID;
                WriteInt(rom, SpawnTable + type * 4, codeBase + version.CodeSymbols[IsBossType(type) ? "CreateNidlBoss" : "CreateNidlMiniboss"] + 1);
                Array.Copy(rom, ObjectParameters + like * 0x18, rom, minibossParameters, 0x18);
                rom[minibossParameters] = 0;
                rom[minibossParameters + 1] = 1;
                WriteShort(rom, minibossParameters + 6, 0);
                WriteShort(rom, minibossParameters + 8, 0);
                WriteInt(rom, minibossParameters + 12, minibossFrameTiles[type] + HurtOf(type).Tiles);
                WriteInt(rom, minibossParameters + 16, codeBase + version.CodeSymbols[IsBossType(type) ? "NidlBossStart" : "NidlMinibossStart"] + 1);
                WriteInt(rom, minibossParameters + 20, minibossAnimInfosAt[type] + 0x08000000);
                Array.Copy(rom, ObjectParameters2 + like * 8, rom, ObjectParameters2 + type * 8, 8);
            }

            /* The HP bar's fill (0x08034F38): HP times gUnk_08351458[type - 0x38][players - 1], >> 8 (64: full). The table sits right before
             * gUnk_08351648 in every version, and holds a placeholder (8192) for types 0x40-0x44: the mini-bosses (30 HP) get Mr. Frosty's
             * 546 (30 HP as well, for one player) for any number of players; */
            int hpBar = version.RomSymbols["gUnk_08351648"] - 0x1F0;
            if (ReadShort(rom, hpBar + 8 * 8) != 8192) return $"The ROM's data doesn't match the {version.Name} release.";
            // (and the bosses, 60 HP as KatAM's, the 273 of Kracko's for one player);
            foreach (byte type in minibossAnimInfosAt.Keys)
                for (int players = 0; players < 4; players++) WriteShort(rom, hpBar + (type - FrostyID) * 8 + players * 2, IsBossType(type) ? 273 : 546);

            /* A boss's defeat runs gUnk_08350E58[type - 0x43] as it explodes (KatAM's bosses leave their shard there): type 0x43's (none)
             * and 0x44's (none) are the bosses' NidlBossDefeat; */
            int bossHandlers = version.RomSymbols["gUnk_08350E58"];
            foreach (byte type in BossIDs) {
                if (ReadInt(rom, bossHandlers + (type - BossID) * 4) != 0) return $"The ROM's data doesn't match the {version.Name} release.";
                WriteInt(rom, bossHandlers + (type - BossID) * 4, codeBase + version.CodeSymbols["NidlBossDefeat"] + 1);
            }

            /* A defeated mini-boss blinks with the animation gUnk_08D610B4[type - 0x38] (sub_0809D060), a table of 8 (0x38-0x3F): type 0x40
             * read past its end. It's copied with 5 more entries (0x40-0x44, none: no blinking), and its one use points at the copy; */
            int blinks = version.RomSymbols["gUnk_08D610B4"], blinksAt = Allocate(ref cursor, 13 * 4, 4);
            byte[] blinksPointer = BitConverter.GetBytes(blinks + 0x08000000);
            List<int> blinksUses = Enumerable.Range(0, (CodeAddress - 4) / 4).Select(i => i * 4)
                                             .Where(at => rom.AsSpan(at, 4).SequenceEqual(blinksPointer)).ToList();
            if (blinksUses.Count != 1) return $"The ROM's code doesn't match the {version.Name} release.";
            Array.Copy(rom, blinks, rom, blinksAt, 8 * 4);
            WriteInts(rom, blinksAt + 8 * 4, 0, 0, 0, 0, 0);
            WriteInt(rom, blinksUses[0], blinksAt + 0x08000000);

            /* The hit flash (sub_080857A0) keeps the palette to restore afterwards, but then looks it up again from the type's palette key
             * (none for type 0x31: palette 15, the flash's). Its "palette isn't 15" branch now skips that lookup (bne to the store); */
            if (rom[HitFlashBranch] != 0x00 || rom[HitFlashBranch + 1] != 0xD0) return $"The ROM's code doesn't match the {version.Name} release.";
            WriteShort(rom, HitFlashBranch, 0xD11A);

            int[] sites = { version.NameSite1, version.NameSite2 };
            for (int i = 0; i < NameCopies.Length; i++) {
                (int length, int literal, bool fromTypePointer) = NameCopies[i];
                ApplyNamePatch(rom, sites[i], length, sites[i] + literal, fromTypePointer, codeBase + version.NidlCopyName);
            }

            string error = AddAbilities(rom, nightmare, region, version, ref cursor, motions, new Dictionary<string, int[]> {
                ["gUnk_08D60FDC"] = hatTables.Select(table => table + 0x08000000).ToArray(),
                ["gUnk_08350A3C"] = new[] { -1, -1, -1, -1, -1, -1, backdropPalette, -1 },
            });
            // NiDL's title screen swarm (KatAMExpansionTitle.cs), before the rooms: FinishRooms() allocates after them;
            error ??= title ? AddTitle(rom, nightmare, version, ref cursor) : null;
            return error ?? (rooms ? AddWorldMap(rom, version, ref cursor) ?? AddRooms(rom, nightmare, region, version, ref cursor) : null);
        }

        //////////////////////////////////////////////////////////////////////////////////////////////

        // The hurt frame: 32x32 pixels (HurtTiles tiles), NiDL's origin at its center (KatAM's one pixel higher, see ReadFrame);
        const int HurtTiles = 16, HurtOriginX = 16, HurtOriginY = 15;

        /* Build(); The enemy's palette, frames (tiles in 1D order, OAM entries, attributes: oam index, pieces, box width, height,
         * origin x, y; palette: 0, or -1 for NiDL's palette 15) and hurt frame (HurtTiles tiles of 32x32 pixels); */
        static (Enemy, byte[], List<(byte[], ushort[], short[], int)>, byte[]) Build(byte[] nightmare, NightmareRegion region, Enemy enemy) {
            int descriptor = ReadInt(nightmare, enemy.Descriptor >= 0 ? region.Descriptors + MinibossDescriptors + enemy.Descriptor * 4
                                                                      : region.Descriptors + (enemy.Type - 8) * 4) - 0x08000000;
            (List<byte[]> palettes, List<(byte[], ushort[], short[], int)> frames, byte[] hurt) =
                BuildGraphics(nightmare, region, descriptor, enemy.FrameTable, enemy.Frames, new List<Part[]> { enemy.Hurt }, enemy.PaletteChanges, 32, 32,
                              HurtOriginX, HurtOriginY);
            // (One palette per enemy: frames drawn with another one are drawn with this one);
            return (enemy, palettes[enemy.Palette], frames.Select(f => (f.Item1, f.Item2, f.Item3, f.Item4 < 0 ? -1 : 0)).ToList(), hurt);
        }

        /* BuildGraphics(); Palettes (the descriptor's, recolored by paletteChanges), frames (as Build's; palette: the descriptor's palette
         * index, or -1 for NiDL's palette 15) and hurt frames (hurtWidth x hurtHeight pixels each, the NiDL origin at hurtOriginX,
         * hurtOriginY, one after the other) of a NiDL graphics descriptor; shift moves every frame and the hurt frames that many pixels
         * down. Parts with a Descriptor (a NiDL mini-boss descriptor index) are drawn from that one's tiles, with its palettes (added after
         * the others); */
        static (List<byte[]>, List<(byte[], ushort[], short[], int)>, byte[]) BuildGraphics(byte[] nightmare, NightmareRegion region, int descriptor,
                int frameTable, List<Part[]> frameParts, List<Part[]> hurts, Dictionary<int, ushort> paletteChanges, int hurtWidth, int hurtHeight,
                int hurtOriginX, int hurtOriginY, int shift = 0) {
            int paletteAddress = ReadInt(nightmare, descriptor + 8) - 0x08000000, tilesAddress = ReadInt(nightmare, descriptor + 12) - 0x08000000;
            int paletteCount = ReadShort(nightmare, descriptor);

            frameTable = FrameAddress(region, frameTable);
            if (nightmare[tilesAddress] != 0x10) throw new InvalidOperationException("Not LZ77 tiles.");
            if (paletteCount < 1 || paletteCount > 8) throw new InvalidOperationException("Not a graphics descriptor.");

            byte[] tiles = DecompressLZ77(nightmare, tilesAddress);
            List<byte[]> palettes = new List<byte[]>();
            for (int p = 0; p < paletteCount; p++) {
                byte[] palette = new byte[32];
                Array.Copy(nightmare, paletteAddress + p * 32, palette, 0, 32);
                if (p == 0) foreach ((int color, ushort value) in paletteChanges) WriteShort(palette, color * 2, value);
                palettes.Add(palette);
            }

            List<(byte[], ushort[], short[], int)> frames = new List<(byte[], ushort[], short[], int)>();

            // Other descriptors' tiles and where their palettes went;
            Dictionary<int, (byte[] tiles, int palette)> others = new Dictionary<int, (byte[], int)>();
            byte[] common = null, starRod = null;
            (int table, byte[][] tiles, byte[][] palettes)? cutscene = null;
            List<(Piece, byte[], int)> Pieces(Part part) => PartOffset(part, UnmovedPieces(part).Where(p => part.OnlyPalette < 0 || p.Item1.Palette == part.OnlyPalette).ToList());
            List<(Piece, byte[], int)> PartOffset(Part part, List<(Piece, byte[], int)> pieces) =>
                pieces.Select(p => (p.Item1 with { X = p.Item1.X + part.Dx, Y = p.Item1.Y + part.Dy }, p.Item2, p.Item3)).ToList();
            List<(Piece, byte[], int)> UnmovedPieces(Part part) {
                if (part.Tile != null) return new List<(Piece, byte[], int)> { (new Piece(-4, -4, 8, 8, 0, 0, 0, 0, 0), part.Tile, -1) };
                if (part.StarRod) return PartPieces(nightmare, region, frameTable, part, starRod ??= StarRodTiles(nightmare, region), palettes);
                if (part.Common) return PartPieces(nightmare, region, frameTable, part, common ??= CommonTiles(nightmare, region), palettes);
                if (part.OrbShots) {
                    if (!others.TryGetValue(OrbShotsKey, out (byte[] tiles, int palette) shots)) {
                        (byte[] shotTiles, byte[] shotPalette) = OrbShotGraphics(nightmare);
                        shots = others[OrbShotsKey] = (shotTiles, palettes.Count);
                        palettes.Add(shotPalette);
                    }
                    return PartPieces(nightmare, region, frameTable, part, shots.tiles, palettes).Select(p => (p.Item1, p.Item2, shots.palette)).ToList();
                }
                if (part.Cutscene != 0) {
                    (int table, byte[][] tiles, byte[][] palettes) cut = cutscene ??= CutsceneGraphics(nightmare);
                    if (!others.TryGetValue(CutsceneKey, out (byte[] tiles, int palette) loaded)) {
                        loaded = others[CutsceneKey] = (null, palettes.Count);
                        palettes.AddRange(cut.palettes);
                    }
                    // (its pieces' palettes: the first or the second, as their OAM's picks them);
                    return PartPieces(nightmare, region, cut.table, part, cut.tiles[part.Cutscene - 1], palettes)
                        .Select(p => (p.Item1, p.Item2, loaded.palette + (p.Item1.Palette & 1))).ToList();
                }
                if (part.Descriptor < 0) return PartPieces(nightmare, region, frameTable, part, tiles, palettes);
                if (!others.TryGetValue(part.Descriptor, out (byte[] tiles, int palette) other)) {
                    int at = ReadInt(nightmare, region.Descriptors + MinibossDescriptors + part.Descriptor * 4) - 0x08000000;
                    int otherTiles = ReadInt(nightmare, at + 12) - 0x08000000, otherPalettes = ReadInt(nightmare, at + 8) - 0x08000000;

                    if (nightmare[otherTiles] != 0x10 || ReadShort(nightmare, at) < 1 || ReadShort(nightmare, at) > 4)
                        throw new InvalidOperationException("Not a graphics descriptor.");
                    other = others[part.Descriptor] = (DecompressLZ77(nightmare, otherTiles), palettes.Count);
                    for (int p = 0; p < ReadShort(nightmare, at); p++) palettes.Add(nightmare[(otherPalettes + p * 32)..(otherPalettes + p * 32 + 32)]);
                }
                return PartPieces(nightmare, region, frameTable, part, other.tiles, palettes)
                    .Select(p => (p.Item1, p.Item2, p.Item3 < 0 ? p.Item3 : p.Item3 + other.palette)).ToList();
            }

            foreach (Part[] parts in frameParts) {
                List<(Piece piece, byte[] tiles, int palette)> pieces = parts.SelectMany(Pieces).ToList();

                int halfWidth = pieces.Max(p => Math.Max(-p.piece.X, p.piece.X + p.piece.Width));
                int top = pieces.Min(p => p.piece.Y), bottom = pieces.Max(p => p.piece.Y + p.piece.Height);

                List<byte> frameTiles = new List<byte>();
                List<ushort> oams = new List<ushort>();

                bool shared = parts.Any(p => p.Shared);
                foreach ((Piece piece, byte[] pieceTiles, int palette) in pieces) {
                    int at = frameTiles.Count;
                    for (int t = 0; shared && t + pieceTiles.Length <= frameTiles.Count; t += 32)
                        if (frameTiles.Skip(t).Take(pieceTiles.Length).SequenceEqual(pieceTiles)) { at = t; break; }
                    oams.Add((ushort)((piece.Y - top) | piece.Shape << 14));
                    oams.Add((ushort)((piece.X + halfWidth) | piece.Size << 14 | piece.HFlip << 12 | piece.VFlip << 13));
                    oams.Add((ushort)(at / 32));
                    if (at == frameTiles.Count) frameTiles.AddRange(pieceTiles);
                }

                if (pieces.Any(p => p.palette >= palettes.Count)) throw new InvalidOperationException("No such palette.");
                frames.Add((frameTiles.ToArray(), oams.ToArray(), new short[] { 0, (short)pieces.Count, (short)(halfWidth * 2), (short)(bottom - top), (short)halfWidth, (short)(-top - shift) },
                            pieces.Any(p => p.palette < 0) ? -1 : pieces[0].palette));
            }

            // Hurt frames: each drawn pixel by pixel into hurtWidth x hurtHeight and cut into tiles (rows of hurtWidth / 8);
            List<byte> hurtTiles = new List<byte>();
            foreach (Part[] hurtParts in hurts) {
                int[,] canvas = new int[hurtHeight, hurtWidth];
                foreach (Part part in hurtParts) {
                    foreach ((Piece piece, byte[] pieceTiles, int _) in Pieces(part)) {
                        int columns = piece.Width / 8;

                        for (int y = 0; y < piece.Height; y++) {
                            for (int x = 0; x < piece.Width; x++) {
                                int t = y / 8 * columns + x / 8, b = pieceTiles[t * 32 + y % 8 * 4 + x % 8 / 2];
                                int color = (x & 1) != 0 ? b >> 4 : b & 0xF;
                                int cx = hurtOriginX + piece.X + (piece.HFlip != 0 ? piece.Width - 1 - x : x);
                                int cy = hurtOriginY + shift + piece.Y + (piece.VFlip != 0 ? piece.Height - 1 - y : y);

                                if (color != 0 && cx >= 0 && cx < hurtWidth && cy >= 0 && cy < hurtHeight) canvas[cy, cx] = color;
                            }
                        }
                    }
                }

                byte[] hurt = new byte[hurtWidth * hurtHeight / 2];
                for (int y = 0; y < hurtHeight; y++)
                    for (int x = 0; x < hurtWidth; x++)
                        hurt[(y / 8 * (hurtWidth / 8) + x / 8) * 32 + y % 8 * 4 + x % 8 / 2] |= (byte)((x & 1) != 0 ? canvas[y, x] << 4 : canvas[y, x]);
                hurtTiles.AddRange(hurt);
            }

            return (palettes, frames, hurtTiles.ToArray());
        }

        /* PartPieces(); A part's pieces, with their tiles (1D, recolored) and palette (-1: NiDL's palette 15). The frame table entry is
         * either a frame's OAM records, drawn from the descriptor's tiles, or a streamed frame (Mr. Tick-Tock's): 12 bytes, pointers to
         * its OAM records, its palette (u16 0x20, then the colors; added to palettes) and its tiles, uploaded with the frame (rows of
         * VRAM, 32 tiles each: u16 bytes and the tiles, until 0xFFFF); */
        /* StarRodTiles(); The shared tiles (CommonTiles()) as NiDL has them in the Star Rod's fights: the star it shoots (its shared frames
         * 38-41) over rows 28-31, columns 7-13 (as its frames number them), raw tiles 7 a row the level loads there, 0x2328 after
         * Palette15 in every release (USA 0x080D2AD0; checked by a tile of it); */
        static readonly byte[] StarRodTile = Convert.FromHexString("00000000000000000060060000f44f0000effe0040eeee04f0eeee0fe4eeee4e");
        static byte[] StarRodTiles(byte[] nightmare, NightmareRegion region) {
            byte[] tiles = (byte[])CommonTiles(nightmare, region).Clone();
            int at = region.Palette15 + 0x2328;
            if (!nightmare[(at + 8 * 32)..(at + 9 * 32)].SequenceEqual(StarRodTile)) throw new InvalidOperationException("The Star Rod's star isn't in the Nightmare in Dream Land ROM.");
            for (int row = 0; row < 4; row++)
                Array.Copy(nightmare, at + row * 7 * 32, tiles, ((28 + row) * 16 + 7) * 32, 7 * 32);
            return tiles;
        }

        /* CutsceneGraphics(); The frame table, tiles (the comet's, the blue star's: LZ77, 16 a row as the descriptors') and two palettes
         * of NiDL's cutscene of Nightmare's arrival, from where its code loads them (USA 0x08075264: the table, the palettes, 0x030015B0,
         * the first tiles, 0x02020000, 0x06013000, the second tiles, 0x06014000, its literals), found by those values; */
        const int CutsceneKey = -3;
        static (int table, byte[][] tiles, byte[][] palettes) CutsceneGraphics(byte[] nightmare) {
            for (int at = 20; at + 12 <= nightmare.Length; at += 4) {
                if (ReadInt(nightmare, at) != 0x06013000 || ReadInt(nightmare, at + 8) != 0x06014000 || ReadInt(nightmare, at - 12) != 0x030015B0
                    || ReadInt(nightmare, at - 4) != 0x02020000) continue;
                int table = ReadInt(nightmare, at - 20) - 0x08000000, palette = ReadInt(nightmare, at - 16) - 0x08000000;
                int[] blocks = { ReadInt(nightmare, at - 8) - 0x08000000, ReadInt(nightmare, at + 4) - 0x08000000 };
                if (blocks.Any(b => b < 0 || b >= nightmare.Length || nightmare[b] != 0x10)) throw new InvalidOperationException("Not LZ77 tiles.");
                // (as many tiles as 8 rows of 16 hold: the frames' pieces reach row 7);
                byte[][] tiles = blocks.Select(b => {
                    byte[] block = DecompressLZ77(nightmare, b), padded = new byte[Math.Max(block.Length, 8 * 16 * 32)];
                    block.CopyTo(padded, 0);
                    return padded;
                }).ToArray();
                return (table, tiles, new[] { nightmare[palette..(palette + 32)], nightmare[(palette + 32)..(palette + 64)] });
            }
            throw new InvalidOperationException("Nightmare's arrival isn't in the Nightmare in Dream Land ROM.");
        }

        /* OrbShotGraphics(); The Power Orb's shots' tiles (raw, 16 per row as the descriptors') and palette: the orb loads them from a
         * descriptor of its own (USA 0x082FEFF4: palettes 1, tiles 0x37, 0x17, palette, tiles right after it), found by those values; */
        const int OrbShotsKey = -2;
        static (byte[] tiles, byte[] palette) OrbShotGraphics(byte[] nightmare) {
            for (int at = 0; at + 16 <= nightmare.Length; at += 4) {
                if (ReadInt(nightmare, at) != 0x00370001 || ReadInt(nightmare, at + 4) != 0x17) continue;
                int palette = ReadInt(nightmare, at + 8) - 0x08000000, tiles = ReadInt(nightmare, at + 12) - 0x08000000;
                if (tiles != palette + 0x20 || palette < 0 || tiles + 0x37 * 32 > nightmare.Length) continue;
                byte[] block = new byte[256 * 32];
                Array.Copy(nightmare, tiles, block, 0, 0x37 * 32);
                return (block, nightmare[palette..(palette + 32)]);
            }
            throw new InvalidOperationException("The Power Orb's shots aren't in the Nightmare in Dream Land ROM.");
        }

        /* CommonTiles(); NiDL's shared object tiles as its frames number them (16 per row, as the descriptors'): the LZ77 tiles after
         * palette 15 (USA 0x080D07C8, 256 tiles) are its tiles 256-511 (puff 0x0874C568's frames draw tiles 654-846: 2D VRAM, 32 a row);
         * the others, empty here, are the level's; */
        static byte[] CommonTiles(byte[] nightmare, NightmareRegion region) {
            int at = region.Palette15 + 0x20;
            if (nightmare[at] != 0x10) throw new InvalidOperationException("Not LZ77 tiles.");
            byte[] tiles = new byte[512 * 32], block = DecompressLZ77(nightmare, at);
            Array.Copy(block, 0, tiles, 256 * 32, Math.Min(block.Length, 256 * 32));
            return tiles;
        }

        static List<(Piece, byte[], int)> PartPieces(byte[] nightmare, NightmareRegion region, int frameTable, Part part, byte[] tiles, List<byte[]> palettes) {
            int table = part.Table != 0 ? FrameAddress(region, part.Table) : frameTable;
            int entry = ReadInt(nightmare, table + part.Frame * 4) - 0x08000000, records = entry;
            int palette = part.Palette15 ? -1 : part.Palette, width = 16;
            byte[] source = tiles;

            int[] streamedPalettes = null;
            if (IsStreamed(nightmare, entry)) {
                int paletteAt = ReadInt(nightmare, entry + 4) - 0x08000000 + 2, rows = ReadInt(nightmare, entry + 8) - 0x08000000;
                int count = ReadShort(nightmare, paletteAt - 2) / 32;

                records = ReadInt(nightmare, entry) - 0x08000000;
                // Its palettes (Paint Roller's have two: its pieces pick theirs, Piece.Palette);
                streamedPalettes = Enumerable.Range(0, count).Select(k => {
                    byte[] colors = nightmare[(paletteAt + k * 32)..(paletteAt + k * 32 + 32)];
                    int index = palettes.FindIndex(p => p.SequenceEqual(colors));
                    if (index < 0) {
                        index = palettes.Count;
                        palettes.Add(colors);
                    }
                    return index;
                }).ToArray();
                palette = streamedPalettes[0];
                List<byte> rowTiles = new List<byte>();
                for (int bytes; (bytes = ReadShort(nightmare, rows)) != 0xFFFF; rows += 2 + bytes) {
                    if (bytes > 32 * 32) throw new InvalidOperationException("Not a streamed frame.");
                    byte[] row = new byte[32 * 32];
                    Array.Copy(nightmare, rows + 2, row, 0, bytes);
                    rowTiles.AddRange(row);
                }
                source = rowTiles.ToArray();
                width = 32;
            }

            return ReadFrameAt(nightmare, records).Select(piece => (piece, PieceTiles(source, piece, part.Recolor, width),
                                                                     streamedPalettes == null ? palette : streamedPalettes[piece.Palette % streamedPalettes.Length])).ToList();
        }

        // IsStreamed(); A frame table entry that's a streamed frame (three ROM pointers, the second to 16-color palettes: u16 bytes, 1-4 of them);
        static bool IsStreamed(byte[] nightmare, int entry) {
            bool IsPointer(int value) => value >= 0x08000000 && value < 0x08000000 + nightmare.Length;
            int oam = ReadInt(nightmare, entry), palette = ReadInt(nightmare, entry + 4), rows = ReadInt(nightmare, entry + 8);

            int header = IsPointer(palette) ? ReadShort(nightmare, palette - 0x08000000) : 0;

            return IsPointer(oam) && IsPointer(palette) && IsPointer(rows) && header >= 0x20 && header <= 0x80 && header % 0x20 == 0;
        }

        /* PieceTiles(); The 4bpp tiles of a piece in 1D order (NiDL's VRAM is 2D, 32 tiles per row; its data, 16 tiles per row; a streamed
         * frame's, 32 per row: width); */
        static byte[] PieceTiles(byte[] tiles, Piece piece, Dictionary<int, int> recolor, int width = 16) {
            List<byte> output = new List<byte>();

            for (int y = 0; y < piece.Height / 8; y++) {
                for (int x = 0; x < piece.Width / 8; x++) {
                    int source = piece.Tile + y * 32 + x;
                    if (width == 16) source = source / 32 * 16 + source % 32;

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
        record Piece(int X, int Y, int Width, int Height, int Shape, int Size, int HFlip, int VFlip, int Tile, int Palette = 0);   // (Palette: NiDL's, its OAM's)

        static readonly int[,,] Shapes = {
            { { 8, 8 }, { 16, 16 }, { 32, 32 }, { 64, 64 } },
            { { 16, 8 }, { 32, 8 }, { 32, 16 }, { 64, 32 } },
            { { 8, 16 }, { 8, 32 }, { 16, 32 }, { 32, 64 } },
        };

        /* ReadFrame(); NiDL frame records are 8 bytes: y, attr0 (shape, 0x10 = last), x, attr1 (size, flips), flipped x,
         * flipped attr1, tile (10 bits); */
        static List<Piece> ReadFrame(byte[] nightmare, int frameTable, int frame) => ReadFrameAt(nightmare, ReadInt(nightmare, frameTable + frame * 4) - 0x08000000);

        static List<Piece> ReadFrameAt(byte[] nightmare, int address) {
            List<Piece> pieces = new List<Piece>();

            // Until the record marked last (Kracko's cloud has 28; 64 at most);
            for (int i = 0; i < 64; i++, address += 8) {
                int shape = nightmare[address + 1] >> 6, size = nightmare[address + 3] >> 6;

                pieces.Add(new Piece((sbyte)nightmare[address + 4], (sbyte)nightmare[address] + 1,
                    Shapes[shape, size, 0], Shapes[shape, size, 1], shape, size,
                    1 - ((nightmare[address + 3] >> 4) & 1), (nightmare[address + 3] >> 5) & 1,
                    nightmare[address + 6] | (nightmare[address + 7] & 3) << 8, nightmare[address + 7] >> 4));

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

        // ComposeName(); A name image (8 tiles) made of letters of the game's English names (none in the Japanese game: an empty one);
        static byte[] ComposeName(byte[] rom, KatAMExpansionCode.Version version, (int type, int letter)[] glyphs) {
            if (version.GameCode == "B8KJ") return new byte[0x100];
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

        /* WriteMapTable(); What the map tracker ("KatAM Map Tracker", Core/RomMap.cs) reads to tell an expanded ROM and draw its ported
         * mini-bosses, in the last 0x100 bytes of the ROM (MapTableAddress): "NIDLMAP" and a version byte (1), u16 entries, u16 entry
         * size (32), then per mini-boss: object type, index (the behavior's high nibble), u16 0 (or for an icon drawn over its room,
         * IconOverRoom, its frame's origin x and y + 128), the icon's address (file offset) and
         * its name (24 bytes, ASCII, 0-padded). An icon is u16 width, u16 height, then a BGR555 color per pixel (| 0x8000 where it's
         * drawn): its Icon frames in a grid as square as can be (the Meta-Knights' 2x2), each bottom-centered in its cell. The bosses'
         * entries are the same, in a table of their own (the map table's last free bytes, MapBosses, +0xCC: "BS", u16 count, its
         * address); */
        static void WriteMapTable(byte[] rom, byte[] nightmare, NightmareRegion region,
                                  List<(List<byte[]> palettes, List<(byte[] tiles, ushort[] oams, short[] attributes, int palette)> frames, byte[] hurt)> built,
                                  ref int cursor) {
            const int EntrySize = 32, Gap = 2;

            System.Text.Encoding.ASCII.GetBytes("NIDLMAP\x01").CopyTo(rom, MapTableAddress);
            WriteShort(rom, MapTableAddress + 8, MinibossDefinitions.Count);
            WriteShort(rom, MapTableAddress + 10, EntrySize);
            // (the bosses' entries, built as the mini-bosses' (AllBosses, after them), in a table of their own: the map table has no
            // room for them; "BS", u16 count, its address at MapBosses);
            int bosses = Allocate(ref cursor, BossDefinitions.Count * EntrySize, 4);
            System.Text.Encoding.ASCII.GetBytes("BS").CopyTo(rom, MapBosses);
            WriteShort(rom, MapBosses + 2, BossDefinitions.Count);
            WriteInt(rom, MapBosses + 4, bosses + 0x08000000);
            for (int i = 0; i < AllBosses.Count; i++) {
                Miniboss m = AllBosses[i];
                List<(byte[] tiles, ushort[] oams, short[] attributes, int palette)> frames = (m.Icon ?? new[] { 0 }).Select(f => built[i].frames[f]).ToList();
                // (its cells side by side, each its frames drawn on each other from their origins: a lone frame is itself);
                List<List<(byte[] tiles, ushort[] oams, short[] attributes, int palette)>> cells = m.IconCells != null
                    ? m.IconCells.Select(cell => cell.Select(f => built[i].frames[f]).ToList()).ToList() : frames.Select(f => new[] { f }.ToList()).ToList();
                int columns = (int)Math.Ceiling(Math.Sqrt(cells.Count)), rows = (cells.Count + columns - 1) / columns;
                (int minX, int minY, int width, int height) Box(List<(byte[] tiles, ushort[] oams, short[] attributes, int palette)> cell) {
                    int minX = cell.Min(f => -f.attributes[4]), minY = cell.Min(f => -f.attributes[5]);
                    return (minX, minY, cell.Max(f => f.attributes[2] - f.attributes[4]) - minX, cell.Max(f => f.attributes[3] - f.attributes[5]) - minY);
                }
                int cellWidth = cells.Max(c => Box(c).width), cellHeight = cells.Max(c => Box(c).height);
                int width = columns * cellWidth + Gap * (columns - 1), height = rows * cellHeight + Gap * (rows - 1);
                ushort[] pixels = new ushort[width * height];

                for (int c = 0; c < cells.Count; c++) {
                    (int minX, int minY, int boxWidth, int boxHeight) = Box(cells[c]);
                    foreach ((byte[] tiles, ushort[] oams, short[] attributes, int palette) in cells[c]) {
                        byte[] colors = palette < 0 ? nightmare[region.Palette15..(region.Palette15 + 32)] : built[i].palettes[palette];
                        int left = c % columns * (cellWidth + Gap) + (cellWidth - boxWidth) / 2 - attributes[4] - minX;
                        int top = c / columns * (cellHeight + Gap) + cellHeight - boxHeight - attributes[5] - minY;

                        // The first piece is on top, so they're drawn last to first;
                        for (int p = oams.Length / 3 - 1; p >= 0; p--) {
                            int shape = oams[p * 3] >> 14, size = oams[p * 3 + 1] >> 14, tile = oams[p * 3 + 2];
                            int pieceY = oams[p * 3] & 0x3FFF, pieceX = oams[p * 3 + 1] & 0xFFF;
                            bool flipH = (oams[p * 3 + 1] & 0x1000) != 0, flipV = (oams[p * 3 + 1] & 0x2000) != 0;
                            int w = shape == 0 ? 8 << size : shape == 1 ? new[] { 16, 32, 32, 64 }[size] : new[] { 8, 8, 16, 32 }[size];
                            int h = shape == 0 ? 8 << size : shape == 1 ? new[] { 8, 8, 16, 32 }[size] : new[] { 16, 32, 32, 64 }[size];

                            for (int y = 0; y < h; y++) {
                                for (int x = 0; x < w; x++) {
                                    int t = tile + y / 8 * (w / 8) + x / 8, b = tiles[t * 32 + y % 8 * 4 + x % 8 / 2];
                                    int color = (x & 1) != 0 ? b >> 4 : b & 0xF;
                                    int px = left + pieceX + (flipH ? w - 1 - x : x), py = top + pieceY + (flipV ? h - 1 - y : y);

                                    if (color != 0 && px >= 0 && py >= 0 && px < width && py < height) pixels[py * width + px] = (ushort)(ReadShort(colors, color * 2) | 0x8000);
                                }
                            }
                        }
                    }
                }

                int icon = Allocate(ref cursor, 4 + pixels.Length * 2, 4),
                    entry = i < MinibossDefinitions.Count ? MapTableAddress + 12 + i * EntrySize : bosses + (i - MinibossDefinitions.Count) * EntrySize;
                WriteShort(rom, icon, width);
                WriteShort(rom, icon + 2, height);
                for (int p = 0; p < pixels.Length; p++) WriteShort(rom, icon + 4 + p * 2, pixels[p]);
                rom[entry] = m.Type;
                rom[entry + 1] = (byte)m.Index;
                WriteShort(rom, entry + 2, 0);
                // (an icon drawn over its room: its frame's origin + 128, x then y, where the tracker draws it from the boss's spot);
                if (m.IconOverRoom && frames.Count == 1) {
                    rom[entry + 2] = (byte)(frames[0].attributes[4] + 128);
                    rom[entry + 3] = (byte)(frames[0].attributes[5] + 128);
                }
                WriteInt(rom, entry + 4, icon);
                Array.Clear(rom, entry + 8, 24);
                System.Text.Encoding.ASCII.GetBytes(m.Name).CopyTo(rom, entry + 8);
            }
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

        static int ReadShort(byte[] data, int address) => BitConverter.ToUInt16(data, address);

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
