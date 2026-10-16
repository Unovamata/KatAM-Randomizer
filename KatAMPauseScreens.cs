using KatAMInternal;

namespace KatAMRandomizer {
    /* KatAMPauseScreens; Every ability's pause help screen is replaced by the base one ("About Copy Abilities"), and the graphics of
     * the others (every ability, in every language) are wiped, about 480 KB in every version, to free room for gameplay additions.
     * The pause screen still shows the current ability's portrait and name (the HUD's, which stay), and keeps both frames (the solo
     * one and the multiplayer one, sHelpMenuUnkTiledBGsIndices[language][0 and 1]) and the SELECT "call" button after the tutorial
     * (a sprite). The European version keeps the English base screen for every language; the Japanese one keeps the Japanese one.
     * • sHelpMenuUnkTiledBGsIndices[6 languages][0x20] (USA 0x08358C14): [2 + ability] is each ability's screen (an index of
     *   gUnk_082D7850); every one of them becomes the kept screen.
     * • gUnk_082D7850 (USA 0x082D7850): pointers to each background ({ u16 width, height, unk4; u8 unk6, unk7; tileset (LZ77),
     *   tilesetCompressedSize, palette, u16 paletteOffset, paletteSize, tilemap }, 0x1C bytes), each stored as palette (when it
     *   has one), tileset, tilemap and the record itself, in one block. The dropped ones point to the kept record, and their blocks
     *   are filled with 0xFF. The unused entries (abilities 27-29, sharing one record) are left alone.
     * • CreateHelpMenu (USA 0x08124430) loads the screen of gKirbys[gLocalPlayerId].ability and then the portrait of
     *   (screen - row[2]); it now loads row[2] and the portrait of the ability itself, so abilities past the table (new ones) work
     *   too. Patched at AbilityLoad (the ldrb of the ability, which becomes movs r0, #0) and the 0x14 bytes from AbilityLoad + 0x18
     *   (the portrait's argument), which read the ability again with CreateHelpMenu's own literals (gKirbys at AbilityLoad - 0x12,
     *   the ability's offset 0x103 at AbilityLoad - 4); */
    internal static class KatAMPauseScreens {
        sealed record Version(string GameCode, int AbilityLoad, int Indices, int Backgrounds, ushort Kept);

        static readonly Version[] Versions = {
            new Version("B8KE", 0x1246C2, 0x358C14, 0x2D7850, 0x20),
            new Version("B8KP", 0x12470A, 0x35D6F4, 0x2D96F0, 0x20),
            new Version("B8KJ", 0x1245CA, 0x33D3D0, 0x2D8758, 0x02),
        };

        const int Languages = 6, RowLength = 0x20, FirstScreen = 2, LastScreen = 0xB5, RecordSize = 0x1C;

        // Original CreateHelpMenu code at AbilityLoad (ldrb r0, [r0]; adds r0, #2; lsls r0, #1; adds r0, r7; adds r0, r4; ldrh r1, [r0]);
        static readonly byte[] Original = { 0x00, 0x78, 0x02, 0x30, 0x40, 0x00, 0xC0, 0x19, 0x00, 0x19, 0x01, 0x88 };

        // Apply(); Points every pause help screen to the base one and wipes the rest. Does nothing on an unknown or already patched ROM;
        public static void Apply(byte[] rom) {
            Version version = Versions.FirstOrDefault(v => v.GameCode == System.Text.Encoding.ASCII.GetString(rom, 0xAC, 4));

            if (version == null || !rom.AsSpan(version.AbilityLoad, Original.Length).SequenceEqual(Original)) return;

            PatchCode(rom, version.AbilityLoad);

            for (int language = 0; language < Languages; language++)
                for (int screen = FirstScreen; screen < RowLength; screen++)
                    WriteU16(rom, version.Indices + (language * RowLength + screen) * 2, version.Kept);

            int kept = ReadPointer(rom, version.Backgrounds + version.Kept * 4),
                unused = ReadPointer(rom, version.Backgrounds + 0x1D * 4);

            for (int screen = FirstScreen; screen <= LastScreen; screen++) {
                int entry = version.Backgrounds + screen * 4, record = ReadPointer(rom, entry);

                if (record == kept || record == unused) continue;

                int tileset = ReadPointer(rom, record + 8), palette = ReadPointer(rom, record + 16);
                ushort paletteSize = ReadU16(rom, record + 22);
                int start = paletteSize > 0 ? Math.Min(tileset, palette) : tileset;

                WriteU32(rom, entry, (uint)(kept + 0x8000000));
                rom.AsSpan(start, record + RecordSize - start).Fill(0xFF);
            }
        }

        static void PatchCode(byte[] rom, int at) {
            int kirbys = Literal(rom, at - 0x12), offset = Literal(rom, at - 4), block = at + 0x18;

            WriteU16(rom, at, 0x2000);                                      // movs r0, #0 (screen: row[2]);

            ushort[] code = {
                LoadLiteral(2, block, kirbys),                              // ldr r2, =gKirbys;
                0x7829,                                                     // ldrb r1, [r5] (gLocalPlayerId);
                0x23D4, 0x005B,                                             // movs r3, #0xD4; lsls r3, r3, #1 (sizeof(struct Kirby));
                0x434B,                                                     // muls r3, r1;
                0x18D2,                                                     // adds r2, r2, r3;
                LoadLiteral(3, block + 12, offset),                         // ldr r3, =0x103;
                0x5CD2,                                                     // ldrb r2, [r2, r3] (the ability);
                0x2101,                                                     // movs r1, #1;
                0x46C0,                                                     // nop;
            };

            for (int i = 0; i < code.Length; i++) WriteU16(rom, block + i * 2, code[i]);
        }

        // Literal(); Address of the literal a Thumb "ldr rd, [pc, #imm]" at the given address loads;
        static int Literal(byte[] rom, int at) => ((at + 4) & ~3) + (rom[at] * 4);

        static ushort LoadLiteral(int register, int at, int literal) => (ushort)(0x4800 | (register << 8) | ((literal - ((at + 4) & ~3)) / 4));

        static ushort ReadU16(byte[] rom, int at) => BitConverter.ToUInt16(rom, at);
        static int ReadPointer(byte[] rom, int at) => (int)(BitConverter.ToUInt32(rom, at) - 0x8000000);
        static void WriteU16(byte[] rom, int at, ushort value) => BitConverter.GetBytes(value).CopyTo(rom, at);
        static void WriteU32(byte[] rom, int at, uint value) => BitConverter.GetBytes(value).CopyTo(rom, at);
    }
}
