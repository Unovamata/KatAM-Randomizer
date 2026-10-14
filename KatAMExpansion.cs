using KatAMInternal;
using System.Text;

namespace KatAMRandomizer {
    /* KatAMExpansion; ROM Expansion: enemies ported from Kirby: Nightmare in Dream Land (USA, Japanese or European). Kirby & the Amazing Mirror USA ROMs only.
     * The behaviors are re-implemented in C against the KatAM decompilation (see "ROM Expansion/" for the sources and how they're
     * built) and ship compiled in KatAMExpansionCode. The graphics are never shipped: they are read from the player's own NiDL ROM.
     * Every ported enemy is object type 0x31 (the only free regular enemy type); the template's behavior byte picks the enemy
     * (high nibble) and its variant (low nibble). See "ROM Expansion/nidl.h" for how they share it.
     * Layout (free space after the randomizer's 9ROM lists):
     * • 0xF60000: compiled code, then the converted graphics, animations, names and the extended sprite tables.
     * • gSpriteTables (0x3B909C): anims, attrs and oamData are copied with more entries: anim 939 is the shared hurt frame,
     *   940 on are the enemies' frames (one anim per enemy, one variant per frame).
     * • Object type 0x31: spawn function and parameters; its AnimInfo table holds every enemy's frames.
     * • HP bar names: the two copies of an enemy's name (0x08034ED2, 0x08035B34) call NidlCopyName instead.
     * • The hit flash keeps the enemy's own palette (0x080857EE), see AddEnemies; */
    internal static class KatAMExpansion {
        public const byte NightmareID = 0x31;

        const int HurtAnimID = 939,                  // First free anim ID (the vanilla table has 939);
                  CodeAddress = 0xF60000, ReservedSize = 0xA0000,
                  SpriteTables = 0x3B909C, SpawnTable = 0x35107C, ObjectParameters = 0x351648, ObjectParameters2 = 0x352AD0;

        // The hit flash's "beq" after comparing the sprite's palette with 15 (0x080857EE), see AddEnemies;
        const int HitFlashBranch = 0x857EE;

        /* NiDL: graphics descriptor per enemy (type - 8): tile count (+2), palette (+8), LZ77 tiles (+12). The frame tables (and every
         * NiDL address in the enemies' notes) are the USA ones; the Japanese and European ROMs have the same data, moved by FrameShift; */
        sealed record NightmareRegion(string GameCode, int Descriptors, int FrameShift);

        static readonly NightmareRegion[] NightmareRegions = {
            new NightmareRegion("A7KE", 0x73EEA0, 0),           // USA;
            new NightmareRegion("A7KJ", 0x742DAC, 0x3A8C),      // Japan (Hoshi no Kirby: Yume no Izumi Deluxe);
            new NightmareRegion("A7KP", 0x97B678, 0x23C7EC),    // Europe;
        };

        static NightmareRegion RegionOf(byte[] rom) => rom == null || rom.Length < 0x800000 ? null
            : NightmareRegions.FirstOrDefault(region => Encoding.ASCII.GetString(rom, 0xAC, 4) == region.GameCode);

        /* HP bar name copies patched to call NidlCopyName (see ApplyNamePatch): where the copy starts, its length (up to the end of
         * its bl CpuSet), the literal it loads the name table from, and whether it has the object's type pointer in ip (else the object in r2); */
        static readonly (int at, int length, int literal, bool fromTypePointer)[] NameCopies = {
            (0x34ED2, 28, 0x34F24, true), (0x35B34, 30, 0x35B7C, false),
        };

        // Part; One NiDL frame drawn into a frame, with its colors remapped (NiDL colors of other palettes go to unused ones);
        sealed record Part(int Frame, Dictionary<int, int> Recolor = null);

        /* Enemy; A ported enemy: its index (NIDL_* in nidl.h), name, NiDL object type and frame table, its frames (in the order the
         * C code numbers them), its hurt frame, palette changes, the letters of its name and the behavior bytes the randomizer uses; */
        sealed record Enemy(int Index, string Name, int Type, int FrameTable, List<Part[]> Frames, Part[] Hurt,
                            Dictionary<int, ushort> PaletteChanges, (int type, int letter)[] Glyphs, byte[] Behaviors, byte[] FlyingBehaviors = null);

        public static readonly IReadOnlyList<(int index, string name)> Enemies;

        static readonly List<Enemy> Definitions = new List<Enemy> {
            BroomHatter(),
            Cappy(),
            Twizzy(),
            Bubbles(),
            Slippy(),
            Kabu(),
        };

        static KatAMExpansion() {
            Enemies = Definitions.Select(e => (e.Index, e.Name)).ToList();
        }

        // Broom Hatter; Frames: NiDL's body frames 4-7 with the broom dust (none, 8, 9, 10) on top. The dust is white in NiDL's
        // palette 15: it's drawn with color 6 of the Broom Hatter's palette, which it doesn't use;
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
                new byte[] { 0x00 });
        }

        // Cappy; Frames: NiDL's 4 (standing, it hops turning around). Variant 2 hops in place;
        static Enemy Cappy() => new Enemy(1, "Cappy", 0x10, 0x75235C, new List<Part[]> { new[] { new Part(4) } }, new[] { new Part(3) },
            new Dictionary<int, ushort>(),
            new[] { (0x0C, 0), (0x19, 2), (0x19, 1), (0x19, 1), (0x19, 5) },   // "C" (Cupie), "a", "p", "p", "y" (Sparky);
            new byte[] { 0x00 });

        // Twizzy; Frames: NiDL's 4, 5 (flapping) and 6 (sitting). Variant 0 swoops, variant 5 sits until Kirby comes near;
        static Enemy Twizzy() => new Enemy(2, "Twizzy", 0x18, 0x752560, new List<Part[]> { new[] { new Part(4) }, new[] { new Part(5) }, new[] { new Part(6) } },
            new[] { new Part(3) }, new Dictionary<int, ushort>(),
            new[] { (0x1C, 0), (0x1C, 1), (0x1C, 2), (0x35, 4), (0x35, 4), (0x05, 5) },   // "Twi" (Twister), "z", "z" (Shotzo), "y" (Scarfy);
            new byte[] { 0x00, 0x05 }, new byte[] { 0x00 });

        // Bubbles; Frames: NiDL's 4 to 21 (spinning 4-12, squashed 13-21). Variant 0 bounces around, variant 1 in place;
        static Enemy Bubbles() => new Enemy(3, "Bubbles", 0x1E, 0x7526A8, Enumerable.Range(4, 18).Select(f => new[] { new Part(f) }).ToList(),
            new[] { new Part(0) }, new Dictionary<int, ushort>(),
            new[] { (0x28, 0), (0x0C, 1), (0x18, 5), (0x18, 6), (0x18, 7), (0x18, 8), (0x04, 4) },   // "B" (Bomber), "u" (Cupie), "bble" (Sir Kibble), "s" (Squishy);
            new byte[] { 0x00, 0x01 });

        // Slippy; Frames: NiDL's 4 (sitting), 5 (blinking, falling), 6 (top of a jump), 8 (jumping). It jumps around on land;
        static Enemy Slippy() => new Enemy(4, "Slippy", 0x22, 0x752828, new[] { 4, 5, 6, 8 }.Select(f => new[] { new Part(f) }).ToList(),
            new[] { new Part(3) }, new Dictionary<int, ushort>(),
            new[] { (0x04, 0), (0x02, 1), (0x02, 2), (0x02, 3), (0x02, 4), (0x05, 5) },   // "S" (Squishy), "lipp" (Blipper), "y" (Scarfy);
            new byte[] { 0x00 });

        // IsNightmareInDreamLand(); A Kirby: Nightmare in Dream Land ROM (USA, Japanese or European);
        // Kabu; Frames: NiDL's 4 to 12 (spinning 4-11, sitting 12). It spins and jumps around;
        static Enemy Kabu() => new Enemy(5, "Kabu", 0x15, 0x752438, Enumerable.Range(4, 9).Select(f => new[] { new Part(f) }).ToList(),
            new[] { new Part(3) }, new Dictionary<int, ushort>(),
            new[] { (0x18, 3), (0x14, 5), (0x18, 5), (0x0C, 1) },   // "K" (Sir Kibble), "a" (Hot Head), "b" (Sir Kibble), "u" (Cupie);
            new byte[] { 0x00 });

        public static bool IsNightmareInDreamLand(byte[] rom) => RegionOf(rom) != null;

        // BehaviorsOf(); The behavior bytes of type 0x31 the randomizer may use, for the enemies picked (only the flying ones, or all);
        public static List<byte> BehaviorsOf(IEnumerable<int> enemies, bool flying = false) =>
            Definitions.Where(e => enemies.Contains(e.Index))
                       .SelectMany(e => (flying ? e.FlyingBehaviors ?? Array.Empty<byte>() : e.Behaviors).Select(b => (byte)(e.Index << 4 | b))).ToList();

        // NameOf(); The ported enemy a behavior byte of type 0x31 picks;
        public static string NameOf(byte behavior) => Definitions.FirstOrDefault(e => e.Index == behavior >> 4)?.Name ?? Definitions[0].Name;

        /* AddEnemies(); Adds every ported enemy as object type 0x31. Returns null when done, or why it couldn't be done; */
        public static string AddEnemies(byte[] rom, byte[] nightmare) {
            if (RomRegion.Detect(rom) != RomRegion.USA) return "The ROM Expansion only supports the USA version of Kirby & the Amazing Mirror.";
            if (!IsNightmareInDreamLand(nightmare)) return "That isn't a Kirby: Nightmare in Dream Land ROM.";

            for (int i = CodeAddress; i < CodeAddress + ReservedSize; i++) {
                if (rom[i] != 0xFF) return "The free space used by the ROM Expansion is already in use.";
            }

            List<(Enemy enemy, byte[] palette, List<(byte[] tiles, ushort[] oams, short[] attributes)> frames, byte[] hurt)> built;

            try {
                NightmareRegion region = RegionOf(nightmare);
                built = Definitions.Select(e => Build(nightmare, region, e)).ToList();
            } catch (Exception exception) when (exception is IndexOutOfRangeException or ArgumentException or InvalidOperationException) {
                return "The Nightmare in Dream Land ROM doesn't match the USA release.";
            }

            int anims = ReadInt(rom, SpriteTables) - 0x08000000, attrTable = ReadInt(rom, SpriteTables + 4) - 0x08000000,
                oamTable = ReadInt(rom, SpriteTables + 8) - 0x08000000, paletteBase = ReadInt(rom, SpriteTables + 12) - 0x08000000,
                tilesBase = ReadInt(rom, SpriteTables + 16) - 0x08000000;

            byte[] code = KatAMExpansionCode.Code;
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

            List<(int animBase, int animId, int hurtTiles)> data = new List<(int, int, int)>();
            List<(int animId, int variant)> animInfos = new List<(int, int)> { (HurtAnimID, 0), (HurtAnimID, 0) };

            foreach ((Enemy enemy, byte[] palette, List<(byte[] tiles, ushort[] oams, short[] attributes)> frames, byte[] hurt) in built) {
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
                    (byte[] tiles, ushort[] oams, short[] attributes) = frames[i];
                    int commands = Allocate(ref cursor, 36, 4);

                    tiles.CopyTo(rom, tilesAt + tile * 32);
                    WriteInts(rom, commands, -2, paletteIndex, 16, -1, tileIndex + tile, tiles.Length / 32, 60, i, -4);
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
            for (int i = 0; i < Definitions.Count; i++) ComposeName(rom, Definitions[i].Glyphs).CopyTo(rom, namesAt + i * 0x100);

            // gNidlData: names, then per enemy (animBase, animId, hurt tiles);
            int nidlData = CodeAddress + KatAMExpansionCode.GNidlData;
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

            WriteInt(rom, SpawnTable + NightmareID * 4, codeBase + KatAMExpansionCode.CreateNidlEnemy + 1);
            rom[parameters] = 0;
            rom[parameters + 1] = 1;
            Array.Copy(rom, ObjectParameters + 2, rom, parameters + 2, 4);
            WriteShort(rom, parameters + 6, 0);
            WriteShort(rom, parameters + 8, 0);
            WriteShort(rom, parameters + 10, 0);
            WriteInt(rom, parameters + 12, numTiles);
            WriteInt(rom, parameters + 16, codeBase + KatAMExpansionCode.NidlStart + 1);
            WriteInt(rom, parameters + 20, animInfosAt + 0x08000000);
            Array.Copy(rom, ObjectParameters2, rom, ObjectParameters2 + NightmareID * 8, 8);

            /* The hit flash (sub_080857A0) keeps the palette to restore afterwards, but then looks it up again from the type's palette key
             * (none for type 0x31: palette 15, the flash's). Its "palette isn't 15" branch now skips that lookup (bne to the store); */
            if (rom[HitFlashBranch] != 0x00 || rom[HitFlashBranch + 1] != 0xD0) return "The ROM's code doesn't match the USA release.";
            WriteShort(rom, HitFlashBranch, 0xD11A);

            foreach ((int at, int length, int literal, bool fromTypePointer) in NameCopies)
                ApplyNamePatch(rom, at, length, literal, fromTypePointer, codeBase + KatAMExpansionCode.NidlCopyName);

            return null;
        }

        //////////////////////////////////////////////////////////////////////////////////////////////

        // The hurt frame: 32x32 pixels (HurtTiles tiles), NiDL's origin at its center (KatAM's one pixel higher, see ReadFrame);
        const int HurtTiles = 16, HurtOriginX = 16, HurtOriginY = 15;

        /* Build(); The enemy's palette, frames (tiles in 1D order, OAM entries, attributes: oam index, pieces, box width, height,
         * origin x, y) and hurt frame (HurtTiles tiles of 32x32 pixels); */
        static (Enemy, byte[], List<(byte[], ushort[], short[])>, byte[]) Build(byte[] nightmare, NightmareRegion region, Enemy enemy) {
            int descriptor = ReadInt(nightmare, region.Descriptors + (enemy.Type - 8) * 4) - 0x08000000, frameTable = enemy.FrameTable + region.FrameShift;
            int paletteAddress = ReadInt(nightmare, descriptor + 8) - 0x08000000, tilesAddress = ReadInt(nightmare, descriptor + 12) - 0x08000000;

            if (nightmare[tilesAddress] != 0x10) throw new InvalidOperationException("Not LZ77 tiles.");

            byte[] tiles = DecompressLZ77(nightmare, tilesAddress);
            byte[] palette = new byte[32];
            Array.Copy(nightmare, paletteAddress, palette, 0, 32);
            foreach ((int color, ushort value) in enemy.PaletteChanges) WriteShort(palette, color * 2, value);

            List<(byte[], ushort[], short[])> frames = new List<(byte[], ushort[], short[])>();

            foreach (Part[] parts in enemy.Frames) {
                List<(Piece piece, Dictionary<int, int> recolor)> pieces = parts.SelectMany(p => ReadFrame(nightmare, frameTable, p.Frame).Select(piece => (piece, p.Recolor))).ToList();

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

                frames.Add((frameTiles.ToArray(), oams.ToArray(), new short[] { 0, (short)pieces.Count, (short)(halfWidth * 2), (short)(bottom - top), (short)halfWidth, (short)-top }));
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
        const int NameTables = 0xD60EE4, EnglishNames = 1, NameEnd = 62;

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
        static byte[] ComposeName(byte[] rom, (int type, int letter)[] glyphs) {
            int english = ReadInt(rom, NameTables + EnglishNames * 4) - 0x08000000;
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
