using System;
using System.Collections.Generic;
using System.Linq;

namespace KatAMRandomizer {
    /* KatAMExpansionNightmare; Nightmare's fight's graphics ("ROM Expansion/bosses/nightmare.c"), an anim of their own (type 0x44's 256
     * frames are full), from the player's NiDL ROM. NiDL draws him (0x080A9EF4) as his body frame (0x0875456C's 4-75: streamed frames,
     * each with its own palettes) and, over it, his top half (89-98, by his body frame: 4-7 89, 24-59 90-98, four frames each), with four
     * palettes: P0 his collar and arms, P1 his tornado (drawn in a palette that alternates, C and D), P2 his cape (drawn in a blend of
     * A and B) and P3 his light (60-67, appearing and vanishing). A KatAM sprite has one palette, so bosses/nightmare.c draws him as
     * three sprites: his collar's pieces, his tornado's and his cape's (or light's). NiDL's pieces of different palettes overlap both
     * ways, so each piece is cut down to what NiDL shows of it (masked by the pieces of other palettes NiDL draws in front of it, its top
     * half's first): the three sprites then make up NiDL's picture in any order. Frames that come out the same share one.
     * The anim: the shots and effects (0x08754738's 0-12, his descriptor's tiles and second palette), the sparkle columns (0x087546F8's
     * 0-3, 0x08754708's 0-3, 0x08754718's 0-7, streamed, in his tornado's palette), then the three sprites' frames.
     * gNidlNightmareFight (WriteNightmareFight()): the anim ID, the shots' and sparkles' first frames, per NiDL frame (0-75) the three
     * sprites' frames (NightmareNone: none), NiDL's four palettes (FightColors) and every frame's tiles (the sprites' VRAM); */
    internal static partial class KatAMExpansion {
        const int NightmareFrames = 0x75456C, NightmareShotFrames = 0x754738;
        const int NightmareBodyFrames = 76, NightmareParts = 3, NightmareShots = 13, NightmareSparkles = 16;
        const byte NightmareNone = 0xFF;
        static readonly (int table, int count)[] NightmareSparkleTables = { (0x7546F8, 4), (0x754708, 4), (0x754718, 8) };

        // NightmareTopOf(); His top half's frame over a body frame (NiDL's table 0x087493A4), or -1;
        static int NightmareTopOf(int frame) => frame >= 4 && frame < 8 ? 89 : frame >= 24 && frame < 60 ? 90 + (frame - 24) / 4 : -1;

        /* NightmareFight(); The anim's palettes and frames (see above), and the table of the three sprites' frames per NiDL frame (76 x 3:
         * collar, tornado, cape or light); */
        static (List<byte[]> palettes, List<(byte[] tiles, ushort[] oams, short[] attributes, int palette)> frames, byte[] table)
                NightmareFight(byte[] nightmare, NightmareRegion region) {
            int descriptor = ReadInt(nightmare, region.Descriptors + MinibossDescriptors + 38 * 4) - 0x08000000;
            List<Part[]> effects = Enumerable.Range(0, NightmareShots).Select(f => new[] { new Part(f, Table: NightmareShotFrames, Palette: 1) })
                .Concat(NightmareSparkleTables.SelectMany(s => Enumerable.Range(0, s.count).Select(f => new[] { new Part(f, Table: s.table) }))).ToList();
            (List<byte[]> palettes, List<(byte[] tiles, ushort[] oams, short[] attributes, int palette)> frames, byte[] _) =
                BuildGraphics(nightmare, region, descriptor, NightmareFrames, effects, new List<Part[]>(), new Dictionary<int, ushort>(), 64, 96, 32, 48);

            // His four palettes (P0-P2 his body's, frame 8's; P3 his light's, frame 60's), by where they are in the anim's;
            byte[][] roles = StreamedPalettes(nightmare, region, 8).Append(StreamedPalettes(nightmare, region, 60)[0]).ToArray();
            int RoleOf(int palette) => Array.FindIndex(roles, r => r.SequenceEqual(palettes[palette]));

            byte[] table = Enumerable.Repeat(NightmareNone, NightmareBodyFrames * NightmareParts).ToArray();
            for (int f = 4; f < NightmareBodyFrames; f++) {
                // NiDL's pieces front to back: his top half's, then his body's;
                List<(Piece piece, byte[] tiles, int palette)> pieces = new List<(Piece, byte[], int)>();
                if (NightmareTopOf(f) >= 0) pieces.AddRange(PartPieces(nightmare, region, 0, new Part(NightmareTopOf(f), Table: NightmareFrames), null, palettes));
                pieces.AddRange(PartPieces(nightmare, region, 0, new Part(f, Table: NightmareFrames), null, palettes));
                int[] role = pieces.Select(p => RoleOf(p.palette)).ToArray();
                if (role.Any(r => r < 0)) throw new InvalidOperationException("Not Nightmare's palettes.");

                // Each pixel shows the first piece over it: the pieces of other palettes behind it lose it;
                Dictionary<(int, int), int> first = new Dictionary<(int, int), int>();
                for (int i = 0; i < pieces.Count; i++) {
                    (Piece piece, byte[] pieceTiles, int _) = pieces[i];
                    byte[] masked = (byte[])pieceTiles.Clone();
                    int columns = piece.Width / 8;

                    for (int y = 0; y < piece.Height; y++) {
                        for (int x = 0; x < piece.Width; x++) {
                            int at = (y / 8 * columns + x / 8) * 32 + y % 8 * 4 + x % 8 / 2, shift = (x & 1) * 4;
                            if ((masked[at] >> shift & 0xF) == 0) continue;
                            (int, int) pixel = (piece.X + (piece.HFlip != 0 ? piece.Width - 1 - x : x), piece.Y + (piece.VFlip != 0 ? piece.Height - 1 - y : y));
                            if (!first.TryGetValue(pixel, out int over)) first[pixel] = role[i];
                            else if (over != role[i]) masked[at] &= (byte)~(0xF << shift);
                        }
                    }
                    pieces[i] = (piece, masked, pieces[i].palette);
                }

                // The three sprites' frames: P0's pieces, P1's, P2's or P3's (none: no piece shows);
                for (int part = 0; part < NightmareParts; part++) {
                    List<(Piece piece, byte[] tiles, int palette)> shown = pieces.Where((p, i) => (part < 2 ? role[i] == part : role[i] >= 2) && p.tiles.Any(b => b != 0)).ToList();
                    if (shown.Count == 0) continue;
                    var frame = NightmareFrame(shown);
                    int index = frames.FindIndex(o => o.tiles.SequenceEqual(frame.tiles) && o.oams.SequenceEqual(frame.oams)
                                                     && o.attributes.SequenceEqual(frame.attributes) && o.palette == frame.palette);
                    if (index < 0) {
                        index = frames.Count;
                        frames.Add(frame);
                    }
                    table[f * NightmareParts + part] = (byte)index;
                }
            }
            if (frames.Count > 256) throw new InvalidOperationException("Nightmare's fight has more than 256 frames.");
            return (palettes, frames, table);
        }

        // StreamedPalettes(); The palettes of one of Nightmare's streamed frames;
        static byte[][] StreamedPalettes(byte[] nightmare, NightmareRegion region, int frame) {
            int entry = ReadInt(nightmare, FrameAddress(region, NightmareFrames) + frame * 4) - 0x08000000;
            if (!IsStreamed(nightmare, entry)) throw new InvalidOperationException("Not a streamed frame.");
            int at = ReadInt(nightmare, entry + 4) - 0x08000000;
            return Enumerable.Range(0, ReadShort(nightmare, at) / 32).Select(k => nightmare[(at + 2 + k * 32)..(at + 2 + k * 32 + 32)]).ToArray();
        }

        // NightmareFrame(); A frame of pieces (as BuildGraphics() makes them), in the first one's palette;
        static (byte[] tiles, ushort[] oams, short[] attributes, int palette) NightmareFrame(List<(Piece piece, byte[] tiles, int palette)> pieces) {
            int halfWidth = pieces.Max(p => Math.Max(-p.piece.X, p.piece.X + p.piece.Width));
            int top = pieces.Min(p => p.piece.Y), bottom = pieces.Max(p => p.piece.Y + p.piece.Height);
            List<byte> tiles = new List<byte>();
            List<ushort> oams = new List<ushort>();

            foreach ((Piece piece, byte[] pieceTiles, int _) in pieces) {
                oams.Add((ushort)((piece.Y - top) | piece.Shape << 14));
                oams.Add((ushort)((piece.X + halfWidth) | piece.Size << 14 | piece.HFlip << 12 | piece.VFlip << 13));
                oams.Add((ushort)(tiles.Count / 32));
                tiles.AddRange(pieceTiles);
            }
            return (tiles.ToArray(), oams.ToArray(), new short[] { 0, (short)pieces.Count, (short)(halfWidth * 2), (short)(bottom - top), (short)halfWidth, (short)-top },
                    pieces[0].palette);
        }

        /* WriteNightmareFight(); gNidlNightmareFight: u16 the anim ID, u8 the shots' first frame, u8 the sparkles'; u8 [76][3] the three
         * sprites' frames per NiDL frame; u16 [4][16] NiDL's palettes A-D; u8 [256] each frame's tiles; */
        static void WriteNightmareFight(byte[] rom, byte[] nightmare, NightmareRegion region, int at, int animId,
                                        List<(byte[] tiles, ushort[] oams, short[] attributes, int palette)> frames, byte[] table) {
            WriteShort(rom, at, animId);
            rom[at + 2] = 0;
            rom[at + 3] = NightmareShots;
            table.CopyTo(rom, at + 4);
            Array.Copy(nightmare, region.FightColors, rom, at + 4 + table.Length, 4 * 32);
            for (int i = 0; i < frames.Count; i++) rom[at + 4 + table.Length + 4 * 32 + i] = (byte)(frames[i].tiles.Length / 32);
        }
    }
}
