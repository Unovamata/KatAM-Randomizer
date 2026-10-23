namespace KatAMRandomizer {
    /* KatAMExpansion's ability motions: the new abilities' attack poses (Needle's spikes, Freeze's ice, Ball's rolling...), which KatAM's
     * Kirby doesn't have, are NiDL's frames (see "ROM Expansion/abilities.h").
     * • Each motion is a sequence of NiDL Kirby frames, played at an animation index of the ability (MotionSlot + its number): the C code's
     *   setup (NidlAbilitySetup) points the player's body animation there (gUnk_02021590) to a variant of the body motion anim, and the
     *   ability's hat table points to the same variant of the hat motion anim.
     * • NiDL's attack frames are one layer, with Kirby and the effect together. Kirby's pixels (palette colors 1-11, when they are the
     *   plain Kirby's) go to the body, which takes the player's palette and has VRAM for 16 tiles; the rest (the effect, or the whole
     *   frame when Kirby isn't plain or doesn't fit) to the hat sprite, which has VRAM for 48, in the colors its
     *   hat palette has free; */
    internal static partial class KatAMExpansion {
        const int MotionSlot = 0x67, MotionSlots = 16;  // Animation indexes 0x67-0x76 (KatAM's copy abilities' attacks; not its tumble, 0x4A-0x59);
        const int BodyTiles = 16, MotionHatTiles = 48;
        const int MotionDx = 0, MotionDy = -3;          // NiDL's Kirby frames -> KatAM's (BodyMatches' usual offset);

        record Motion(int Ability, int Number, (int frame, int delay)[] Frames, int Loop);   // Loop: frame to jump back to (-1: hold the last);

        static (int, int)[] Frames(int first, int last, int delay) => Enumerable.Range(first, last - first + 1).Select(f => (f, delay)).ToArray();
        static (int, int)[] Frames(params (int, int)[][] parts) => parts.SelectMany(p => p).ToArray();

        // The motions, numbered per ability as in "ROM Expansion/abilities.h" (NIDL_MOTION_*); delays as measured in NiDL;
        static readonly Motion[] Motions = {
            // Needle: spikes out (on the ground, then held), spikes in, in the air (out, then held);
            new Motion(0, 0, Frames(Frames(0x96E, 0x972, 2), Frames(0x973, 0x977, 2)), 5),
            new Motion(0, 1, new[] { (0x972, 2), (0x971, 2), (0x970, 2), (0x96F, 2), (0x96E, 2) }, -1),
            new Motion(0, 2, Frames(Frames(0x979, 0x97D, 2), Frames(0x97E, 0x982, 2)), 5),
            // Freeze: the start and end, the breath (held);
            new Motion(1, 0, new[] { (0xA8E, 2) }, -1),
            new Motion(1, 1, Frames(0xA86, 0xA8D, 1), 0),
            // Light: holding the light up, then shining (held);
            new Motion(2, 0, Frames(new[] { (0xDFD, 2), (0xDFE, 2), (0xDFD, 2), (0xDFE, 2), (0xDFD, 2), (0xDFE, 2), (0xDFD, 2), (0xDFE, 2),
                                            (0xDFF, 2), (0xE00, 2), (0xE01, 4), (0xE02, 4) }, Frames(0xE03, 0xE05, 3)), 12),
            // Ball: the hop into a ball, rolling (falling), the bounce's squash, resting, the launch, rolling (rising);
            new Motion(3, 0, Frames(new[] { (0xCCA, 6) }, Frames(0xCCB, 0xCD1, 2)), -1),
            new Motion(3, 1, Frames(Frames(0xCDA, 0xCE0, 1), Frames(0xCD3, 0xCD9, 1)), 7),
            new Motion(3, 2, new[] { (0xCE9, 2), (0xCE7, 2) }, -1),
            new Motion(3, 3, new[] { (0xCE2, 3), (0xCE3, 3), (0xCE4, 3), (0xCE5, 3), (0xCE6, 3), (0xCE5, 3), (0xCE4, 3), (0xCE3, 3) }, 0),
            new Motion(3, 4, Frames(new[] { (0xCE9, 2), (0xCE7, 2) }, Frames(0xCD2, 0xCD6, 2), new[] { (0xC7B, 2), (0xC7C, 2) }), 7),
            new Motion(3, 5, Frames(0xCD2, 0xCD9, 1), 0),
            // Mike: the three songs (megaphone, microphone, microphone jumping), the start then the song held; their ends;
            new Motion(5, 0, Frames(new[] { (0x66B, 2), (0x66C, 6), (0x66D, 2), (0x66E, 2), (0x66F, 2), (0x66E, 2) }, Frames(0x670, 0x673, 2),
                                    new[] { (0x674, 2), (0x675, 2) }), 10),
            new Motion(5, 1, Frames(Frames(0x676, 0x67C, 4), new[] { (0x67D, 6), (0x67E, 2), (0x67D, 2) }), 8),
            new Motion(5, 2, Frames(Frames(0x67F, 0x682, 2), new[] { (0x683, 5), (0x682, 4), (0x681, 4) }, Frames(0x684, 0x687, 4),
                                    new[] { (0x688, 5), (0x689, 2), (0x68A, 2) }), 12),
            new Motion(5, 3, new[] { (0x670, 5) }, -1),
            new Motion(5, 4, new[] { (0x679, 5) }, -1),
            new Motion(5, 5, new[] { (0x683, 5) }, -1),
            /* Hi-Jump: the crouch (held), then the rise, its cape flapping, in NiDL's five sets by his speed sideways (0x0804A3FA: up to
             * 0.25, 1.0, 1.25, 1.75 and more, leaning further), each a frame every 3 (NiDL's beat; "ROM Expansion/abilities/hi_jump.c"
             * picks the set and starts it on the beat); */
            new Motion(4, 0, new[] { (0xB2D, 1) }, -1),
            new Motion(4, 1, Frames(0xB2E, 0xB31, 3), 0),
            new Motion(4, 2, Frames(0xB32, 0xB35, 3), 0),
            new Motion(4, 3, Frames(0xB36, 0xB39, 3), 0),
            new Motion(4, 4, Frames(0xB3A, 0xB3D, 3), 0),
            new Motion(4, 5, Frames(0xB3E, 0xB41, 3), 0),
            /* Star Rod (the Power Orb's fight): held up; the swing that shoots a star; flying right, left, up, down, each then held; their
             * ends, back to holding it up; hurt (as recorded in NiDL's fight, frame by frame); the fight's intro, NiDL's whole flight up
             * to the orb (flying up, then down, the swing at it, the rod held out, a swing, held up; "ROM Expansion/abilities/star_rod.c");
             * the swing on the ground or in the air, in Nightmare's fight (its star on the 5th frame); in his intro, knocked off the warp
             * star: tumbling, the rod turning with him (96 and 111 down to 97, a frame each, from the one before NiDL's first, 109: KatAM
             * steps it as it starts), then fallen, his back to us, the rod held out ("ROM Expansion/bosses/nightmare.c"); */
            new Motion(7, 0, new[] { (0x103F, 1) }, -1),
            new Motion(7, 1, Frames(new[] { (0x1040, 4) }, Frames(0x1041, 0x104F, 1), new[] { (0x1050, 2), (0x103F, 1) }), -1),
            new Motion(7, 2, new[] { (0x1057, 1), (0x1058, 2), (0x1059, 1) }, -1),
            new Motion(7, 3, new[] { (0x105A, 1), (0x105B, 2), (0x105C, 1) }, -1),
            new Motion(7, 4, new[] { (0x1054, 1), (0x1055, 2), (0x1056, 1) }, -1),
            new Motion(7, 5, new[] { (0x1051, 1), (0x1052, 2), (0x1053, 1) }, -1),
            new Motion(7, 6, new[] { (0x1058, 2), (0x1057, 3), (0x103F, 1) }, -1),
            new Motion(7, 7, new[] { (0x105B, 2), (0x105A, 3), (0x103F, 1) }, -1),
            new Motion(7, 8, new[] { (0x1055, 2), (0x1054, 3), (0x103F, 1) }, -1),
            new Motion(7, 9, new[] { (0x1052, 2), (0x1051, 3), (0x103F, 1) }, -1),
            new Motion(7, 10, new[] { (0x105D, 6), (0x105E, 2), (0x105F, 1) }, -1),
            new Motion(7, 11, new[] { (0x1056, 78), (0x1055, 8), (0x1054, 12), (0x1051, 4), (0x1052, 4), (0x1053, 28), (0x1040, 1), (0x1042, 1),
                                      (0x1045, 1), (0x1048, 1), (0x104B, 1), (0x104E, 1), (0x105D, 64), (0x105E, 2), (0x105F, 2), (0x1041, 1),
                                      (0x1044, 1), (0x1045, 1), (0x1048, 3), (0x1049, 1), (0x104C, 1), (0x104D, 1), (0x104F, 1), (0x103F, 1) }, -1),
            new Motion(7, 12, Frames(new[] { (0x1026, 1), (0x1027, 1) }, Frames(0x1029, 0x1034, 1), new[] { (0x1035, 2) }), -1),
            new Motion(7, 13, Enumerable.Range(0, 16).Select(i => (0x1060 + ((14 - i) & 15), 1)).ToArray(), 0),
            new Motion(7, 14, new[] { (0x1009, 1) }, -1),
        };

        // NiDL's plain Kirby palette colors 1-11, the same as KatAM's (the player's palette);
        static ushort[] PlainKirby(byte[] nightmare, NightmareRegion region) {
            int entry = ReadInt(nightmare, KirbyFrameTable + region.FrameShift) - 0x08000000;
            return FramePalette(nightmare, ReadInt(nightmare, entry + 4));
        }

        // FramePalette(); A Kirby frame's first palette, 32 colors (attack frames use the second 16 too);
        static ushort[] FramePalette(byte[] nightmare, int pointer) {
            ushort[] colors = new ushort[32];
            if (pointer == 0) return colors;
            int address = pointer - 0x08000000;
            for (int i = 0; i < Math.Min(32, ReadShort(nightmare, address) / 2); i++) colors[i] = (ushort)ReadShort(nightmare, address + 2 + i * 2);
            return colors;
        }

        /* MotionFrame(); A NiDL frame cut into the body's frame (player palette indexes) and the hat's pixels (RGB555 + 1; 0: none), see the
         * summary, and whether the body went to the hat (`allHat`: it does, whether it fits or not); */
        static ((byte[], ushort[], short[]) body, int[,] hat, bool allHat) MotionFrame(byte[] nightmare, NightmareRegion region, int frame, ushort[] plain,
                                                                                    bool allHat = false) {
            const int Size = 128, Center = 64;
            int entry = ReadInt(nightmare, KirbyFrameTable + region.FrameShift + frame * 4) - 0x08000000;
            int first = ReadInt(nightmare, entry), address = (first & ~1) - 0x08000000;
            bool twoLayers = (first & 1) != 0;
            ushort[] palette1 = FramePalette(nightmare, ReadInt(nightmare, entry + 4)), palette2 = twoLayers ? FramePalette(nightmare, ReadInt(nightmare, entry + 12)) : null;
            byte[] tiles1 = KirbyTiles(nightmare, ReadInt(nightmare, entry + 8)), tiles2 = twoLayers ? KirbyTiles(nightmare, ReadInt(nightmare, entry + 16)) : null;
            // (NiDL's shared object palette 15: the Star Rod's sparkles, drawn with Kirby);
            ushort[] palette15 = Enumerable.Range(0, 16).Select(c => (ushort)ReadShort(nightmare, region.Palette15 + c * 2)).ToArray();
            bool plainBank0 = Enumerable.Range(2, 10).All(i => palette1[i] == plain[i]);

            int[,] body = new int[Size, Size], colors = new int[Size, Size];   // colors: RGB555 + 1 (0: none);
            List<(int x, int y, int w, int h, int hf, int vf, int tile, int bank)> pieces = new List<(int, int, int, int, int, int, int, int)>();
            for (int i = 0; i < 64; i++, address += 8) {
                int shape = nightmare[address + 1] >> 6, size = nightmare[address + 3] >> 6;
                pieces.Add(((sbyte)nightmare[address + 2], (sbyte)nightmare[address], Shapes[shape, size, 0], Shapes[shape, size, 1], (nightmare[address + 3] >> 4) & 1,
                            (nightmare[address + 3] >> 5) & 1, nightmare[address + 6] | (nightmare[address + 7] & 3) << 8, nightmare[address + 7] >> 4));
                if ((nightmare[address + 1] & 0x10) != 0) break;
            }

            // Drawn back to front (the first OAM entry is on top);
            for (int p = pieces.Count - 1; p >= 0; p--) {
                var piece = pieces[p];
                bool second = twoLayers && piece.tile >= 0x40, shared = !second && piece.bank == 15;
                byte[] tiles = second ? tiles2 : tiles1;
                ushort[] palette = second ? palette2 : shared ? palette15 : palette1;
                int bank = second || shared ? 0 : piece.bank & 1;

                for (int py = 0; py < piece.h; py++) {
                    for (int px = 0; px < piece.w; px++) {
                        int tile = piece.tile - (second ? 0x40 : 0) + py / 8 * 32 + px / 8;
                        if (tile * 32 >= tiles.Length) continue;
                        int value = tiles[tile * 32 + py % 8 * 4 + px % 8 / 2] >> (px % 2 * 4) & 0xF;
                        if (value == 0) continue;

                        int x = Center + piece.x + MotionDx + (piece.hf != 0 ? piece.w - 1 - px : px);
                        int y = Center + piece.y + MotionDy + (piece.vf != 0 ? piece.h - 1 - py : py);
                        if (x < 0 || y < 0 || x >= Size || y >= Size) continue;
                        // Kirby's pixels: the first layer's plain colors 1-11, and the second layer's in plain Kirby's colors (NiDL's Kirby is
                        // always pink, so Needle's spiked ball, drawn on the second layer, has his body's colors: the player's in KatAM);
                        int plainIndex = second ? Array.IndexOf(plain, palette[value], 2, 10) : -1;
                        bool isKirby = !second && !shared && bank == 0 && plainBank0 && value <= 11 || plainIndex >= 2;
                        body[x, y] = !isKirby ? 0 : plainIndex >= 2 ? plainIndex : value;
                        colors[x, y] = isKirby ? 0 : palette[bank * 16 + value] + 1;
                    }
                }
            }

            // The body, unless it doesn't fit: then everything is the hat's;
            var bodyFrame = CutFrame(body, BodyTiles);
            if (allHat || CountCells(body) > BodyTiles) {
                allHat = true;
                for (int y = 0; y < Size; y++)
                    for (int x = 0; x < Size; x++)
                        if (body[x, y] != 0) { colors[x, y] = plain[body[x, y]] + 1; body[x, y] = 0; }
                bodyFrame = CutFrame(body, BodyTiles);
            }

            return (bodyFrame, colors, allHat);
        }

        static int ColorDistance(int a, int b) {
            int distance = 0;
            for (int shift = 0; shift < 15; shift += 5) distance += Math.Abs((a >> shift & 31) - (b >> shift & 31));
            return distance;
        }

        static int CountCells(int[,] canvas) => CutFrame(canvas, int.MaxValue).tiles.Length / 32;

        // OAM's shapes in 8x8 cells (wide, high; its shape and size bits), the biggest first;
        static readonly (int w, int h, int shape, int size)[] PieceShapes = {
            (8, 8, 0, 3), (8, 4, 1, 3), (4, 8, 2, 3), (4, 4, 0, 2), (4, 2, 1, 2), (2, 4, 2, 2), (4, 1, 1, 1), (1, 4, 2, 1), (2, 2, 0, 1), (2, 1, 1, 0),
            (1, 2, 2, 0), (1, 1, 0, 0),
        };

        /* CutFrame(); A canvas (128x128, origin at its center; 0: transparent) as a KatAM frame: its 8x8 cells with pixels (at most
         * maxTiles), on the grid (one of the 64 offsets from the top left) needing the fewest (Needle's spiked ball in the air needs 50 on
         * the top left's, 48 on its best: the hat's VRAM), drawn in big OAM pieces where they can be, the box widened to be centered on the
         * origin so it flips about it; */
        static (byte[] tiles, ushort[] oams, short[] attributes) CutFrame(int[,] canvas, int maxTiles) {
            const int Size = 128, Center = 64;
            int left = Size, right = 0, top = Size, bottom = 0;

            for (int y = 0; y < Size; y++)
                for (int x = 0; x < Size; x++)
                    if (canvas[x, y] != 0) { left = Math.Min(left, x); right = Math.Max(right, x + 1); top = Math.Min(top, y); bottom = Math.Max(bottom, y + 1); }
            if (left >= right) return (new byte[0], new ushort[0], new short[] { 0, 0, 0, 0, 0, 0 });

            bool Any(int cx, int cy) {
                for (int y = Math.Max(cy, 0); y < cy + 8 && y < Size; y++)
                    for (int x = Math.Max(cx, 0); x < cx + 8 && x < Size; x++)
                        if (canvas[x, y] != 0) return true;
                return false;
            }
            int Cells(int gridLeft, int gridTop) {
                int count = 0;
                for (int cy = gridTop; cy < bottom; cy += 8)
                    for (int cx = gridLeft; cx < right; cx += 8)
                        if (Any(cx, cy)) count++;
                return count;
            }
            (int ox, int oy) best = (0, 0);
            int fewest = Cells(left, top);
            for (int oy = 0; oy < 8; oy++)
                for (int ox = 0; ox < 8; ox++) {
                    int count = Cells(left - ox, top - oy);
                    if (count < fewest) { fewest = count; best = (ox, oy); }
                }
            int gridLeft = left - best.ox, gridTop = top - best.oy, gridRight = gridLeft + (right - gridLeft + 7) / 8 * 8;

            int halfWidth = Math.Max(Center - gridLeft, gridRight - Center);
            int columns = (right - gridLeft + 7) / 8, rows = (bottom - gridTop + 7) / 8, kept = 0;
            byte[,][] cells = new byte[columns, rows][];

            for (int row = 0; row < rows; row++) {
                for (int column = 0; column < columns; column++) {
                    int cx = gridLeft + column * 8, cy = gridTop + row * 8;
                    byte[] cell = new byte[32];
                    bool any = false;
                    for (int y = 0; y < 8; y++)
                        for (int x = 0; x < 8; x++) {
                            int value = cx + x >= 0 && cy + y >= 0 && cx + x < Size && cy + y < Size ? canvas[cx + x, cy + y] : 0;
                            if (value == 0) continue;
                            cell[y * 4 + x / 2] |= (byte)(value << (x % 2 * 4));
                            any = true;
                        }
                    if (!any || kept >= maxTiles) continue;
                    cells[column, row] = cell;
                    kept++;
                }
            }

            /* The cells as OAM pieces, the biggest shapes first where every cell they cover has pixels (the same tiles, in fewer pieces:
             * KatAM's DisplaySprite takes its time per piece, and NiDL's frames on four Kirbys made dozens each); */
            List<byte> output = new List<byte>();
            List<ushort> oams = new List<ushort>();
            bool[,] used = new bool[columns, rows];
            foreach ((int w, int h, int shape, int size) in PieceShapes)
                for (int row = 0; row + h <= rows; row++)
                    for (int column = 0; column + w <= columns; column++) {
                        bool fits = true;
                        for (int dy = 0; dy < h && fits; dy++)
                            for (int dx = 0; dx < w && fits; dx++)
                                fits = cells[column + dx, row + dy] != null && !used[column + dx, row + dy];
                        if (!fits) continue;
                        oams.Add((ushort)(row * 8 | shape << 14));
                        oams.Add((ushort)((gridLeft + column * 8 - Center + halfWidth) | size << 14));
                        oams.Add((ushort)(output.Count / 32));
                        for (int dy = 0; dy < h; dy++)
                            for (int dx = 0; dx < w; dx++) {
                                output.AddRange(cells[column + dx, row + dy]);
                                used[column + dx, row + dy] = true;
                            }
                    }

            return (output.ToArray(), oams.ToArray(), new short[] { 0, (short)(oams.Count / 3), (short)(halfWidth * 2), (short)(bottom - gridTop), (short)halfWidth, (short)(Center - gridTop) });
        }

        /* BuildBackdropPalette(); Backdrop plays like KatAM's Throw, whose body draws a headband in colors 12-14 of the body palette anim
         * (gUnk_08350A3C[Throw]): an anim like it whose palette has NiDL's red Backdrop headband there (frame 0xE20's). Returns its
         * AnimInfo for gUnk_08350A3C[Backdrop]; */
        static int BuildBackdropPalette(byte[] rom, byte[] nightmare, NightmareRegion region, int bodyPalettes, int anims, int attrTable, int oamTable,
                                        int paletteBase, ref int cursor, int firstAnimId, List<int> animVariants, List<int> animAttributes, List<int> animOams) {
            const int Throw = 10, HeadbandFrame = 0xE20;
            int throwAnim = ReadShort(rom, bodyPalettes + Throw * 4), throwVariant = rom[bodyPalettes + Throw * 4 + 2];
            int commands = ReadInt(rom, ReadInt(rom, anims + throwAnim * 4) - 0x08000000 + throwVariant * 4) - 0x08000000;
            if (ReadInt(rom, commands) != -2 || ReadInt(rom, commands + 12) != -1) throw new ArgumentException("Throw's palette anim");

            int entry = ReadInt(nightmare, KirbyFrameTable + region.FrameShift + HeadbandFrame * 4) - 0x08000000;
            ushort[] headband = FramePalette(nightmare, ReadInt(nightmare, entry + 4));
            int paletteAt = Allocate(ref cursor, 32, 32, paletteBase);
            Array.Copy(rom, paletteBase + ReadInt(rom, commands + 4) * 32, rom, paletteAt, 32);
            for (int c = 12; c <= 14; c++) WriteShort(rom, paletteAt + c * 2, headband[c]);

            // Its commands: GetPalette (the new palette), Throw's first GetTiles and ShowFrame, End;
            int at = Allocate(ref cursor, 9 * 4, 4), variantsAt = Allocate(ref cursor, 4, 4);
            WriteInts(rom, at, -2, (paletteAt - paletteBase) / 32, 16);
            Array.Copy(rom, commands + 12, rom, at + 12, 5 * 4);
            WriteInt(rom, at + 32, -4);
            WriteInt(rom, variantsAt, at + 0x08000000);

            animVariants.Add(variantsAt);
            animAttributes.Add(ReadInt(rom, attrTable + throwAnim * 4) - 0x08000000);
            animOams.Add(ReadInt(rom, oamTable + throwAnim * 4) - 0x08000000);
            return firstAnimId + animVariants.Count - 1;
        }

        /* StarRodPalettes(); NiDL's Star Rod palette cycle (its Kirby's second palette, the rod's: 0x0803DB74): its own (frame 0xFED's
         * second), then bright (0x08226254: kept right after its own, 1A46 for its unused color 0), then halfway (each channel's average,
         * as its blend, 0x08003014, at 128 of 256). Writes the Star Rod hat's palette (`hat`, which has NiDL's colors) bright, halfway and
         * as it is, and returns where; */
        static int StarRodPalettes(byte[] rom, byte[] nightmare, NightmareRegion region, ushort[] hat, ref int cursor) {
            int entry = ReadInt(nightmare, KirbyFrameTable + region.FrameShift + 0xFED * 4) - 0x08000000;
            ushort[] own = FramePalette(nightmare, ReadInt(nightmare, entry + 12));
            byte[] colors = own.Skip(1).Take(15).SelectMany(c => BitConverter.GetBytes(c)).ToArray();
            int at = -1;
            for (int i = 0, found; at < 0 && (found = nightmare.AsSpan(i).IndexOf(colors)) >= 0; i += found + 1) {
                int bright = i + found - 2 + 32;
                if (ReadShort(nightmare, bright + 8 * 2) == own[8] && ReadShort(nightmare, bright + 11 * 2) == own[11]
                    && Enumerable.Range(1, 15).Any(c => ReadShort(nightmare, bright + c * 2) != own[c])) at = bright;
            }
            if (at < 0) throw new ArgumentException("The Star Rod's bright palette");

            int written = Allocate(ref cursor, 3 * 32, 4);
            for (int c = 0; c < 16; c++) {
                int nidl = own[c] == hat[c] ? c : Array.IndexOf(own, hat[c], 1);
                ushort bright = c == 0 || nidl < 1 ? hat[c] : (ushort)ReadShort(nightmare, at + nidl * 2);
                int half = 0;
                for (int shift = 0; shift < 15; shift += 5) half |= ((bright >> shift & 31) + (hat[c] >> shift & 31)) / 2 << shift;
                WriteShort(rom, written + c * 2, bright);
                WriteShort(rom, written + 32 + c * 2, half);
                WriteShort(rom, written + 64 + c * 2, hat[c]);
            }
            return written;
        }

        /* HiJumpFlash(); The colors Kirby's Hi-Jump frames glow in as he rises (NiDL's 32 at the region's HiJumpFlash, 0x0804A29A: his frames'
         * own, 0x081F1F48's, bright yellow, for both their 16-color halves): the body's (its colors 1-11, the plain Kirby's, which the body
         * keeps as NiDL's frames number them), then the Hi-Jump hat's (`hat`: each its color in NiDL's frames, the closest when it isn't
         * one of them, bright). Writes those 32 colors (0 for the body's unused) and returns where; */
        const int HiJump = 4;
        static int HiJumpFlash(byte[] rom, byte[] nightmare, NightmareRegion region, ushort[] hat, ref int cursor) {
            int entry = ReadInt(nightmare, KirbyFrameTable + region.FrameShift + 0xB2E * 4) - 0x08000000;
            ushort[] own = FramePalette(nightmare, ReadInt(nightmare, entry + 4));
            ushort[] flash = Enumerable.Range(0, 32).Select(c => (ushort)ReadShort(nightmare, region.HiJumpFlash + c * 2)).ToArray();
            if (Enumerable.Range(1, 11).Count(c => flash[c] != own[c]) < 6) throw new ArgumentException("The Hi-Jump's glow isn't in the Nightmare in Dream Land ROM");

            int written = Allocate(ref cursor, 64, 4);
            for (int c = 1; c < 12; c++) WriteShort(rom, written + c * 2, flash[c]);
            for (int c = 1; c < 16; c++) {
                int nidl = Array.IndexOf(own, hat[c], 1);
                if (nidl < 0) nidl = Enumerable.Range(1, 31).OrderBy(i => ColorDistance(own[i], hat[c])).First();
                WriteShort(rom, written + 32 + c * 2, flash[nidl]);
            }
            return written;
        }

        /* BuildMotions(); The body and hat motion anims (a variant per motion, in Motions' order), each ability's hat table entries at its
         * motion slots, and the body's AnimInfos for gNidlAbilityData.motions (NIDL_ABILITY_COUNT x MotionSlots, 0: none). The Star Rod's
         * hat frames (its motions are the last) have glowing versions, as its hat's (AddGlows(): the hat anim, first frame and count in
         * `glows`' 3-5; the glow on the hat, over the frame's own body), and its hat's palette its cycle (StarRodPalettes(): `glows`' 9); */
        static int[] BuildMotions(byte[] rom, byte[] nightmare, NightmareRegion region, int[] hatTables, int[] hatPalettes, bool[][] paletteUsed,
                                  int tilesBase, ref int cursor, int firstAnimId, List<int> animVariants, List<int> animAttributes, List<int> animOams,
                                  int[] glows) {
            int place = cursor;     // (Local functions can't use `cursor`);
            ushort[] plain = PlainKirby(nightmare, region);
            int bodyAnim = firstAnimId + animVariants.Count, hatAnim = bodyAnim + 1;
            Dictionary<int, int> frameIndexes = new Dictionary<int, int>();
            List<(byte[] tiles, ushort[] oams, short[] attributes)> bodies = new List<(byte[], ushort[], short[])>(), hats = new List<(byte[], ushort[], short[])>();
            List<int[,]> hatPixels = new List<int[,]>(), glowPixels = new List<int[,]>();
            List<int> frameAbility = new List<int>();

            foreach (Motion motion in Motions)
                foreach ((int frame, int _) in motion.Frames) {
                    if (frameIndexes.ContainsKey(frame)) continue;
                    var (body, hat, allHat) = MotionFrame(nightmare, region, frame, plain);
                    frameIndexes[frame] = bodies.Count;
                    bodies.Add(body);
                    hatPixels.Add(hat);
                    glowPixels.Add(motion.Ability == StarRod ? MotionFrame(nightmare, region, frame + StarRodGlow, plain, allHat).hat : hat);
                    frameAbility.Add(motion.Ability);
                }
            int glowFirst = frameAbility.IndexOf(StarRod);
            if (glowFirst < 0 || frameAbility.Skip(glowFirst).Any(a => a != StarRod)) throw new ArgumentException("The Star Rod's motions aren't the last");

            /* The hat's palette: KatAM loads the ability's hat palette for Kirby's hat sprite (its anims' GetPalette don't count), so the motions
             * get the colors the hat doesn't use, for their most used colors (the ones the hat has already are shared); the rest take the
             * closest color; */
            ushort[][] palettes = new ushort[NewAbilities][];
            for (int k = 0; k < NewAbilities; k++) {
                palettes[k] = new ushort[16];
                for (int c = 0; c < 16; c++) palettes[k][c] = (ushort)ReadShort(rom, hatPalettes[k] + c * 2);
                Dictionary<int, int> uses = new Dictionary<int, int>();
                for (int i = 0; i < hatPixels.Count; i++)
                    if (frameAbility[i] == k)
                        foreach (int pixel in hatPixels[i].Cast<int>().Concat(glowPixels[i] != hatPixels[i] ? glowPixels[i].Cast<int>() : Enumerable.Empty<int>()))
                            if (pixel != 0) uses[pixel - 1] = uses.GetValueOrDefault(pixel - 1) + 1;
                Queue<int> wanted = new Queue<int>(uses.OrderByDescending(u => u.Value).ThenBy(u => u.Key).Select(u => u.Key)
                                                       .Where(color => !Enumerable.Range(1, 15).Any(c => paletteUsed[k][c] && palettes[k][c] == color)));
                for (int c = 1; c < 16 && wanted.Count > 0; c++) {
                    if (paletteUsed[k][c]) continue;
                    palettes[k][c] = (ushort)wanted.Dequeue();
                    paletteUsed[k][c] = true;
                }
                for (int c = 0; c < 16; c++) WriteShort(rom, hatPalettes[k] + c * 2, palettes[k][c]);
            }
            glows[9] = StarRodPalettes(rom, nightmare, region, palettes[StarRod], ref place);
            glows[10] = HiJumpFlash(rom, nightmare, region, palettes[HiJump], ref place);
            (byte[], ushort[], short[]) HatOf(int[,] pixels, ushort[] palette) {
                int[,] indexes = new int[pixels.GetLength(0), pixels.GetLength(1)];
                for (int y = 0; y < pixels.GetLength(1); y++)
                    for (int x = 0; x < pixels.GetLength(0); x++)
                        if (pixels[x, y] != 0) indexes[x, y] = 1 + Enumerable.Range(1, 15).OrderBy(c => ColorDistance(palette[c], pixels[x, y] - 1)).First() - 1;
                return CutFrame(indexes, MotionHatTiles);
            }
            List<(byte[] tiles, ushort[] oams, short[] attributes)> glowing = new List<(byte[], ushort[], short[])>();
            for (int i = 0; i < hatPixels.Count; i++) {
                hats.Add(HatOf(hatPixels[i], palettes[frameAbility[i]]));
                glowing.Add(glowPixels[i] != hatPixels[i] ? HatOf(glowPixels[i], palettes[frameAbility[i]]) : hats[i]);
            }
            (glows[4], glows[5]) = AddGlows(hats, glowing, glowFirst);

            // Data: per anim, tiles (aligned to the tiles base), attributes and OAM;
            (int tileIndex, int[] frameTile) WriteFrames(List<(byte[] tiles, ushort[] oams, short[] attributes)> frames) {
                int tilesAt = Allocate(ref place, Math.Max(32, frames.Sum(f => f.tiles.Length)), 32, tilesBase);
                int attributesAt = Allocate(ref place, frames.Count * 12, 4), oamsAt = Allocate(ref place, Math.Max(2, frames.Sum(f => f.oams.Length) * 2), 4);
                int[] frameTile = new int[frames.Count];

                for (int i = 0, tile = 0, oam = 0; i < frames.Count; i++) {
                    (byte[] tiles, ushort[] oams, short[] attributes) = frames[i];
                    tiles.CopyTo(rom, tilesAt + tile * 32);
                    WriteShorts(rom, attributesAt + i * 12, (short)(oam / 3), attributes[1], attributes[2], attributes[3], attributes[4], attributes[5]);
                    for (int o = 0; o < oams.Length; o++) WriteShort(rom, oamsAt + (oam + o) * 2, oams[o]);
                    frameTile[i] = tile;
                    tile += tiles.Length / 32;
                    oam += oams.Length;
                }
                animAttributes.Add(attributesAt);
                animOams.Add(oamsAt);
                return ((tilesAt - tilesBase) / 32, frameTile);
            }

            var bodyData = WriteFrames(bodies);
            var hatData = WriteFrames(hats);

            // Commands per motion: (the body's GetPalette, like KatAM's,) GetTiles when the frame has any, ShowFrame; then a JumpBack, or End;
            int WriteVariants(List<(byte[] tiles, ushort[] oams, short[] attributes)> frames, (int tileIndex, int[] frameTile) data, bool hat) {
                int variantsAt = Allocate(ref place, Motions.Length * 4, 4);
                for (int m = 0; m < Motions.Length; m++) {
                    List<int> words = new List<int>(), starts = new List<int>();
                    if (!hat) words.AddRange(new[] { -2, 0, 16 });
                    foreach ((int frame, int delay) in Motions[m].Frames) {
                        int index = frameIndexes[frame];
                        starts.Add(words.Count);
                        if (frames[index].tiles.Length > 0) words.AddRange(new[] { -1, data.tileIndex + data.frameTile[index], frames[index].tiles.Length / 32 });
                        words.AddRange(new[] { delay, index });
                    }
                    if (Motions[m].Loop >= 0) words.AddRange(new[] { -3, words.Count - starts[Motions[m].Loop] });
                    else words.Add(-4);

                    int at = Allocate(ref place, words.Count * 4, 4);
                    for (int w = 0; w < words.Count; w++) WriteInt(rom, at + w * 4, words[w]);
                    WriteInt(rom, variantsAt + m * 4, at + 0x08000000);
                }
                return variantsAt;
            }

            animVariants.Add(WriteVariants(bodies, bodyData, false));
            animVariants.Add(WriteVariants(hats, hatData, true));
            glows[3] = hatAnim;

            int[] bodyInfos = new int[NewAbilities * MotionSlots];
            for (int m = 0; m < Motions.Length; m++) {
                (int ability, int number) = (Motions[m].Ability, Motions[m].Number);
                bodyInfos[ability * MotionSlots + number] = bodyAnim | m << 16;
                if (hatTables[ability] == 0) continue;
                WriteShort(rom, hatTables[ability] + (MotionSlot + number) * 4, hatAnim);
                rom[hatTables[ability] + (MotionSlot + number) * 4 + 2] = (byte)m;
            }
            cursor = place;
            return bodyInfos;
        }

        /* BuildRodStand(); The Star Rod on its own, upright, for its stand in the NiDL hub ("ROM Expansion/abilities/star_rod_stand.c"):
         * NiDL's rod held straight up (RodStandFrame's second layer, the rod alone), its bottom's middle at the frame's origin (the stand's
         * spot, on the floor), in the Star Rod hat's colors (`hatPalette`, the closest of them), and its glowing version (AddGlows(): `glows`'
         * 6-8): RodStandFrame's own (+ StarRodGlow) is the same frame, so it's the held rod's glow (RodHeldFrame's, the hat's when Kirby
         * stands), the glowing frame's pixels round the star that aren't the plain one's, laid round the upright rod's star. One anim, its
         * variant the hat's palette (as KatAM's objects load theirs, by their anim: ObjectInitSprite) and the frame; returns its anim ID
         * and the VRAM tiles it needs; */
        const int RodStandFrame = 0x1023, RodHeldFrame = 0xFED, RodGlowReach = 10;
        static (int animId, int tiles) BuildRodStand(byte[] rom, byte[] nightmare, NightmareRegion region, int hatPalette, int paletteBase, int tilesBase,
                                                     ref int cursor, int firstAnimId, List<int> animVariants, List<int> animAttributes, List<int> animOams,
                                                     int[] glows) {
            const int Size = 128, Center = 64;
            int[,] rod = RodPixels(nightmare, region, RodStandFrame), held = RodPixels(nightmare, region, RodHeldFrame),
                   heldGlow = RodPixels(nightmare, region, RodHeldFrame + StarRodGlow);
            // (a star's tip: its top row's middle);
            (int x, int y) Tip(int[,] pixels) {
                for (int y = 0; y < Size; y++) {
                    List<int> xs = Enumerable.Range(0, Size).Where(x => pixels[x, y] != 0).ToList();
                    if (xs.Count > 0) return ((xs.Min() + xs.Max()) / 2, y);
                }
                throw new ArgumentException("The Star Rod's frames");
            }
            (int rodX, int rodY) = Tip(rod);
            (int heldX, int heldY) = Tip(held);
            int[,] glow = (int[,])rod.Clone();
            for (int y = 0; y < Size; y++)
                for (int x = 0; x < Size; x++) {
                    int toX = x + rodX - heldX, toY = y + rodY - heldY;
                    if (heldGlow[x, y] == 0 || heldGlow[x, y] == held[x, y] || Math.Abs(x - heldX) > RodGlowReach || y - heldY > 2 * RodGlowReach
                        || toX < 0 || toY < 0 || toX >= Size || toY >= Size) continue;
                    glow[toX, toY] = heldGlow[x, y];
                }

            // (moved: the rod's bottom row's middle to just over the origin; in the hat's colors);
            int bottom = Enumerable.Range(0, Size).Last(y => Enumerable.Range(0, Size).Any(x => rod[x, y] != 0));
            List<int> bottomXs = Enumerable.Range(0, Size).Where(x => rod[x, bottom] != 0).ToList();
            int dx = Center - (bottomXs.Min() + bottomXs.Max() + 1) / 2, dy = Center - 1 - bottom;
            ushort[] palette = Enumerable.Range(0, 16).Select(c => (ushort)ReadShort(rom, hatPalette + c * 2)).ToArray();
            (byte[], ushort[], short[]) Frame(int[,] pixels) {
                int[,] indexes = new int[Size, Size];
                for (int y = 0; y < Size; y++)
                    for (int x = 0; x < Size; x++)
                        if (pixels[x, y] != 0 && x + dx >= 0 && y + dy >= 0 && x + dx < Size && y + dy < Size)
                            indexes[x + dx, y + dy] = Enumerable.Range(1, 15).OrderBy(c => ColorDistance(palette[c], pixels[x, y] - 1)).First();
                return CutFrame(indexes, MotionHatTiles);
            }
            List<(byte[] tiles, ushort[] oams, short[] attributes)> frames = new List<(byte[], ushort[], short[])> { Frame(rod) },
                glowing = new List<(byte[], ushort[], short[])> { Frame(glow) };
            (glows[7], glows[8]) = AddGlows(frames, glowing, 0);
            glows[6] = firstAnimId + animVariants.Count;

            // Data: tiles (aligned to the tiles base), attributes, OAM; the variant: GetPalette, GetTiles (both frames'), ShowFrame, End;
            int tilesAt = Allocate(ref cursor, frames[0].tiles.Length, 32, tilesBase);
            int attributesAt = Allocate(ref cursor, frames.Count * 12, 4), oamsAt = Allocate(ref cursor, frames.Sum(f => f.oams.Length) * 2, 4);
            frames[0].tiles.CopyTo(rom, tilesAt);
            for (int i = 0, oam = 0; i < frames.Count; oam += frames[i].oams.Length, i++) {
                WriteShorts(rom, attributesAt + i * 12, (short)(oam / 3), frames[i].attributes[1], frames[i].attributes[2], frames[i].attributes[3],
                            frames[i].attributes[4], frames[i].attributes[5]);
                for (int o = 0; o < frames[i].oams.Length; o++) WriteShort(rom, oamsAt + (oam + o) * 2, frames[i].oams[o]);
            }
            int[] words = { -2, (hatPalette - paletteBase) / 32, 16, -1, (tilesAt - tilesBase) / 32, frames[0].tiles.Length / 32, 1, 0, -4 };
            int variantAt = Allocate(ref cursor, words.Length * 4, 4), variantsAt = Allocate(ref cursor, 4, 4);
            for (int w = 0; w < words.Length; w++) WriteInt(rom, variantAt + w * 4, words[w]);
            WriteInt(rom, variantsAt, variantAt + 0x08000000);
            animVariants.Add(variantsAt);
            animAttributes.Add(attributesAt);
            animOams.Add(oamsAt);
            return (glows[6], frames[0].tiles.Length / 32);
        }

        // RodPixels(); A NiDL Kirby frame's second layer (the Star Rod's: the rod alone) as RGB555 + 1 (0: none), its origin at the middle;
        static int[,] RodPixels(byte[] nightmare, NightmareRegion region, int frame) {
            const int Size = 128, Center = 64;
            KirbyFrame source = ReadKirbyFrame(nightmare, region, frame);
            int[,] pixels = new int[Size, Size];
            if (source.Tiles2 == null) throw new ArgumentException("The Star Rod's frames");

            // (back to front: NiDL's first piece is on top);
            foreach (Piece piece in Enumerable.Reverse(source.Pieces)) {
                if (piece.Tile < 0x40) continue;
                for (int py = 0; py < piece.Height; py++)
                    for (int px = 0; px < piece.Width; px++) {
                        int tile = piece.Tile - 0x40 + py / 8 * 32 + px / 8;
                        int value = source.Tiles2[tile * 32 + py % 8 * 4 + px % 8 / 2] >> (px % 2 * 4) & 0xF;
                        int x = Center + piece.X + (piece.HFlip != 0 ? piece.Width - 1 - px : px);
                        int y = Center + piece.Y + (piece.VFlip != 0 ? piece.Height - 1 - py : py);
                        if (value == 0 || x < 0 || y < 0 || x >= Size || y >= Size) continue;
                        pixels[x, y] = ReadShort(source.Palette2, value * 2) + 1;
                    }
            }
            return pixels;
        }
    }
}
