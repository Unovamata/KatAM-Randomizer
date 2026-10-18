namespace KatAMRandomizer {
    /* KatAMExpansionTitle; Nightmare in Dream Land's title screen animation, the Kirby swarm ("ROM Expansion/title.c" runs it): its
     * data copied from the player's NiDL ROM into gNidlTitle, and KatAM's intro (sub_08145B64, LogoEnd's only call) detoured to
     * CreateNidlTitle. Everything is found through NiDL's own code, which is the same in every region (the data moves, and the
     * European swarm's entries differ a little):
     * • the swarm's setup (USA 0x080CAA3C) ends its loop with cmp r5, #count - 1; ble (0x2D72 0xDDD6): its literals hold the entries
     *   (+0x68: 4 halfwords each, script, delay, x, y) and the scripts' pointer table (+0x70, the scripts' commands right after it),
     *   the update (+0x288 from the setup) the frame table's;
     * • the frames: NiDL's frame table, 8 bytes each (attribute 0, attribute 1, attribute 1 flipped, attribute 2), the leading run
     *   of 32x32 ones (63) being the Kirby poses: their tiles at attribute 2 in NiDL's 2D sprite sheet, attribute 1's flips;
     * • the sprite sheet: the title's graphics load script (LZ77 into 0x02020000, then copied to 0x06010000: 0x00400003, a 16-tile
     *   wide blob for the sheet's left half, 0x002E8004, another for its right half), found by its 0x02020000, 0x00400003,
     *   0x02020000 words, the blobs' pointers 4 bytes before and 0x14 after;
     * • the palette (NiDL's Kirby one, palette 0 of the swarm's sprites), the literal 8 bytes before the code 0xB530 0x1C05 0x205C
     *   0x223E of the function loading it;
     * Each pose is copied as 16 tiles in 1D order (KatAM maps object tiles in 1D), once even when several frames draw it; */
    internal static partial class KatAMExpansion {
        const int TitleMaxPoses = 64, TitleEntrySize = 8, TitleCommandSize = 14, TitleFrameSize = 8, TitlePoseTiles = 16, TitleSheetWidth = 32, TitleBlobWidth = 16;

        /* AddTitle(); Copies NiDL's swarm into the expanded ROM (gNidlTitle) and detours KatAM's intro to it. Returns an error, or
         * null; */
        static string AddTitle(byte[] rom, byte[] nightmare, KatAMExpansionCode.Version version, ref int cursor) {
            int setup = -1, codeBase = CodeAddress + 0x08000000;

            for (int at = FindNightmareCode(nightmare, 0, 0x2D72, 0xDDD6); at >= 0 && setup < 0; at = FindNightmareCode(nightmare, at + 2, 0x2D72, 0xDDD6))
                if (at >= 0x5C && BitConverter.ToUInt16(nightmare, at - 0x5C) == 0xB5F0) setup = at - 0x5C;
            if (setup < 0) return "Nightmare in Dream Land's title screen wasn't found in its ROM.";

            int count = nightmare[setup + 0x5C] + 1, entries = ReadInt(nightmare, setup + 0x68) - 0x08000000,
                scripts = ReadInt(nightmare, setup + 0x70) - 0x08000000, frames = ReadInt(nightmare, setup + 0x288) - 0x08000000;

            // The scripts' pointers, up to their commands (which follow the table);
            List<int> scriptAt = new List<int>();
            for (int at = scripts; ; at += 4) {
                int pointer = ReadInt(nightmare, at) - 0x08000000;
                if (pointer < scripts || pointer >= scripts + 0x10000 || (scriptAt.Count > 0 && at >= scriptAt.Min())) break;
                scriptAt.Add(pointer);
            }
            int commandsStart = scriptAt.Min(), commandsEnd = scriptAt.Max() + TitleCommandSize * 4;

            // The Kirby poses: the leading 32x32 frames, one sprite each, 16 pixels up and left of the Kirby;
            List<int> poseBases = new List<int>();
            List<byte> framePoses = new List<byte>();
            List<ushort> frameFlips = new List<ushort>();
            for (int frame = ReadInt(nightmare, frames) - 0x08000000; ; frame = ReadInt(nightmare, frames + framePoses.Count * 4) - 0x08000000) {
                ushort attribute0 = BitConverter.ToUInt16(nightmare, frame), attribute1 = BitConverter.ToUInt16(nightmare, frame + 2),
                       attribute2 = BitConverter.ToUInt16(nightmare, frame + 6);
                if ((attribute0 & 0xC0FF) != 0xF0 || (attribute1 & 0xC1FF) != 0x81F0) break;
                int tile = attribute2 & 0x3FF;
                if (!poseBases.Contains(tile)) poseBases.Add(tile);
                framePoses.Add((byte)poseBases.IndexOf(tile));
                frameFlips.Add((ushort)(attribute1 & 0x3000));
            }

            int load = FindNightmareWords(nightmare, 0x2020000, 0x400003, 0x2020000), kirbyPalette = FindNightmareCode(nightmare, 0, 0xB530, 0x1C05, 0x205C, 0x223E);
            if (framePoses.Count == 0 || poseBases.Count > TitleMaxPoses || load < 0 || kirbyPalette < 0) return "Nightmare in Dream Land's title screen wasn't found in its ROM.";

            byte[] left = DecompressLZ77(nightmare, ReadInt(nightmare, load - 4) - 0x08000000),
                   right = DecompressLZ77(nightmare, ReadInt(nightmare, load + 0x14) - 0x08000000);
            byte[] poseTiles = new byte[poseBases.Count * TitlePoseTiles * 32];
            for (int pose = 0; pose < poseBases.Count; pose++) {
                for (int tile = 0; tile < TitlePoseTiles; tile++) {
                    int row = poseBases[pose] / TitleSheetWidth + tile / 4, column = poseBases[pose] % TitleSheetWidth + tile % 4;
                    byte[] half = column < TitleBlobWidth ? left : right;
                    int from = (row * TitleBlobWidth + column % TitleBlobWidth) * 32;
                    if (from + 32 <= half.Length) Array.Copy(half, from, poseTiles, (pose * TitlePoseTiles + tile) * 32, 32);
                }
            }

            int entriesAt = Allocate(ref cursor, count * TitleEntrySize, 4), tableAt = Allocate(ref cursor, scriptAt.Count * 4, 4),
                commandsAt = Allocate(ref cursor, commandsEnd - commandsStart, 4), posesAt = Allocate(ref cursor, framePoses.Count, 4),
                flipsAt = Allocate(ref cursor, frameFlips.Count * 2, 4), tilesAt = Allocate(ref cursor, poseTiles.Length, 4),
                paletteAt = Allocate(ref cursor, 32, 4);
            Array.Copy(nightmare, entries, rom, entriesAt, count * TitleEntrySize);
            Array.Copy(nightmare, commandsStart, rom, commandsAt, commandsEnd - commandsStart);
            for (int i = 0; i < scriptAt.Count; i++) WriteInt(rom, tableAt + i * 4, commandsAt + scriptAt[i] - commandsStart + 0x08000000);
            framePoses.ToArray().CopyTo(rom, posesAt);
            for (int i = 0; i < frameFlips.Count; i++) WriteShort(rom, flipsAt + i * 2, frameFlips[i]);
            poseTiles.CopyTo(rom, tilesAt);
            Array.Copy(nightmare, ReadInt(nightmare, kirbyPalette - 8) - 0x08000000, rom, paletteAt, 32);

            int data = CodeAddress + version.CodeSymbols["gNidlTitle"];
            WriteShorts(rom, data, count, poseBases.Count);
            WriteInts(rom, data + 4, entriesAt + 0x08000000, tableAt + 0x08000000, posesAt + 0x08000000, flipsAt + 0x08000000,
                      tilesAt + 0x08000000, paletteAt + 0x08000000);

            return Detour(rom, version.RomSymbols["sub_08145B64"], codeBase + version.CodeSymbols["CreateNidlTitle"], ref cursor) < 0
                ? $"The ROM's code doesn't match the {version.Name} release." : null;
        }

        // FindNightmareCode(); The first halfword-aligned place in the NiDL ROM from `start` holding these halfwords, or -1;
        static int FindNightmareCode(byte[] nightmare, int start, params int[] halfwords) {
            for (int at = start; at + halfwords.Length * 2 <= nightmare.Length; at += 2) {
                int i = 0;
                while (i < halfwords.Length && BitConverter.ToUInt16(nightmare, at + i * 2) == halfwords[i]) i++;
                if (i == halfwords.Length) return at;
            }
            return -1;
        }

        // FindNightmareWords(); The first word-aligned place in the NiDL ROM holding these words, or -1;
        static int FindNightmareWords(byte[] nightmare, params int[] words) {
            for (int at = 0; at + words.Length * 4 <= nightmare.Length; at += 4) {
                int i = 0;
                while (i < words.Length && ReadInt(nightmare, at + i * 4) == words[i]) i++;
                if (i == words.Length) return at;
            }
            return -1;
        }
    }
}
