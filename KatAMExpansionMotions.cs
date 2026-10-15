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
        const int MotionSlot = 0x4A, MotionSlots = 16;  // Animation indexes 0x4A-0x59 (copy abilities' own; below AnimationIndexes);
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
            // Hi-Jump: the rise (cape flapping, held);
            new Motion(4, 0, Frames(new[] { (0xB2D, 2), (0xB2E, 2) }, Frames(0xB2F, 0xB31, 2)), 2),
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
         * summary; */
        static ((byte[], ushort[], short[]) body, int[,] hat) MotionFrame(byte[] nightmare, NightmareRegion region, int frame, ushort[] plain) {
            const int Size = 128, Center = 64;
            int entry = ReadInt(nightmare, KirbyFrameTable + region.FrameShift + frame * 4) - 0x08000000;
            int first = ReadInt(nightmare, entry), address = (first & ~1) - 0x08000000;
            bool twoLayers = (first & 1) != 0;
            ushort[] palette1 = FramePalette(nightmare, ReadInt(nightmare, entry + 4)), palette2 = twoLayers ? FramePalette(nightmare, ReadInt(nightmare, entry + 12)) : null;
            byte[] tiles1 = KirbyTiles(nightmare, ReadInt(nightmare, entry + 8)), tiles2 = twoLayers ? KirbyTiles(nightmare, ReadInt(nightmare, entry + 16)) : null;
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
                bool second = twoLayers && piece.tile >= 0x40;
                byte[] tiles = second ? tiles2 : tiles1;
                ushort[] palette = second ? palette2 : palette1;
                int bank = second ? 0 : piece.bank & 1;

                for (int py = 0; py < piece.h; py++) {
                    for (int px = 0; px < piece.w; px++) {
                        int tile = piece.tile - (second ? 0x40 : 0) + py / 8 * 32 + px / 8;
                        if (tile * 32 >= tiles.Length) continue;
                        int value = tiles[tile * 32 + py % 8 * 4 + px % 8 / 2] >> (px % 2 * 4) & 0xF;
                        if (value == 0) continue;

                        int x = Center + piece.x + MotionDx + (piece.hf != 0 ? piece.w - 1 - px : px);
                        int y = Center + piece.y + MotionDy + (piece.vf != 0 ? piece.h - 1 - py : py);
                        if (x < 0 || y < 0 || x >= Size || y >= Size) continue;
                        bool isKirby = !second && bank == 0 && plainBank0 && value <= 11;
                        body[x, y] = isKirby ? value : 0;
                        colors[x, y] = isKirby ? 0 : palette[bank * 16 + value] + 1;
                    }
                }
            }

            // The body, unless it doesn't fit: then everything is the hat's;
            var bodyFrame = CutFrame(body, BodyTiles);
            if (CountCells(body) > BodyTiles) {
                for (int y = 0; y < Size; y++)
                    for (int x = 0; x < Size; x++)
                        if (body[x, y] != 0) { colors[x, y] = plain[body[x, y]] + 1; body[x, y] = 0; }
                bodyFrame = CutFrame(body, BodyTiles);
            }

            return (bodyFrame, colors);
        }

        static int ColorDistance(int a, int b) {
            int distance = 0;
            for (int shift = 0; shift < 15; shift += 5) distance += Math.Abs((a >> shift & 31) - (b >> shift & 31));
            return distance;
        }

        static int CountCells(int[,] canvas) => CutFrame(canvas, int.MaxValue).oams.Length / 3;

        /* CutFrame(); A canvas (128x128, origin at its center; 0: transparent) as a KatAM frame: its 8x8 cells with pixels, on a grid from the
         * top left (at most maxTiles), the box widened to be centered on the origin so it flips about it; */
        static (byte[] tiles, ushort[] oams, short[] attributes) CutFrame(int[,] canvas, int maxTiles) {
            const int Size = 128, Center = 64;
            int left = Size, right = 0, top = Size, bottom = 0;

            for (int y = 0; y < Size; y++)
                for (int x = 0; x < Size; x++)
                    if (canvas[x, y] != 0) { left = Math.Min(left, x); right = Math.Max(right, x + 1); top = Math.Min(top, y); bottom = Math.Max(bottom, y + 1); }
            if (left >= right) return (new byte[0], new ushort[0], new short[] { 0, 0, 0, 0, 0, 0 });

            int halfWidth = Math.Max(Center - left, right - Center);
            List<byte> output = new List<byte>();
            List<ushort> oams = new List<ushort>();

            for (int cy = top; cy < bottom; cy += 8) {
                for (int cx = left; cx < right; cx += 8) {
                    byte[] cell = new byte[32];
                    bool any = false;
                    for (int y = 0; y < 8; y++)
                        for (int x = 0; x < 8; x++) {
                            int value = cx + x < Size && cy + y < Size ? canvas[cx + x, cy + y] : 0;
                            if (value == 0) continue;
                            cell[y * 4 + x / 2] |= (byte)(value << (x % 2 * 4));
                            any = true;
                        }
                    if (!any || oams.Count / 3 >= maxTiles) continue;
                    oams.Add((ushort)(cy - top));
                    oams.Add((ushort)(cx - Center + halfWidth));
                    oams.Add((ushort)(output.Count / 32));
                    output.AddRange(cell);
                }
            }

            return (output.ToArray(), oams.ToArray(), new short[] { 0, (short)(oams.Count / 3), (short)(halfWidth * 2), (short)(bottom - top), (short)halfWidth, (short)(Center - top) });
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

        /* BuildMotions(); The body and hat motion anims (a variant per motion, in Motions' order), each ability's hat table entries at its
         * motion slots, and the body's AnimInfos for gNidlAbilityData.motions (NIDL_ABILITY_COUNT x MotionSlots, 0: none); */
        static int[] BuildMotions(byte[] rom, byte[] nightmare, NightmareRegion region, int[] hatTables, int[] hatPalettes, bool[][] paletteUsed,
                                  int tilesBase, ref int cursor, int firstAnimId, List<int> animVariants, List<int> animAttributes, List<int> animOams) {
            int place = cursor;     // (Local functions can't use `cursor`);
            ushort[] plain = PlainKirby(nightmare, region);
            int bodyAnim = firstAnimId + animVariants.Count, hatAnim = bodyAnim + 1;
            Dictionary<int, int> frameIndexes = new Dictionary<int, int>();
            List<(byte[] tiles, ushort[] oams, short[] attributes)> bodies = new List<(byte[], ushort[], short[])>(), hats = new List<(byte[], ushort[], short[])>();
            List<int[,]> hatPixels = new List<int[,]>();
            List<int> frameAbility = new List<int>();

            foreach (Motion motion in Motions)
                foreach ((int frame, int _) in motion.Frames) {
                    if (frameIndexes.ContainsKey(frame)) continue;
                    var (body, hat) = MotionFrame(nightmare, region, frame, plain);
                    frameIndexes[frame] = bodies.Count;
                    bodies.Add(body);
                    hatPixels.Add(hat);
                    frameAbility.Add(motion.Ability);
                }

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
                        foreach (int pixel in hatPixels[i])
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
            for (int i = 0; i < hatPixels.Count; i++) {
                ushort[] palette = palettes[frameAbility[i]];
                int[,] pixels = hatPixels[i], indexes = new int[pixels.GetLength(0), pixels.GetLength(1)];
                for (int y = 0; y < pixels.GetLength(1); y++)
                    for (int x = 0; x < pixels.GetLength(0); x++)
                        if (pixels[x, y] != 0) indexes[x, y] = 1 + Enumerable.Range(1, 15).OrderBy(c => ColorDistance(palette[c], pixels[x, y] - 1)).First() - 1;
                hats.Add(CutFrame(indexes, MotionHatTiles));
            }

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
    }
}
