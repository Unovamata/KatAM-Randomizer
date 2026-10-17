namespace KatAMRandomizer {
    /* KatAMExpansionCalm; The "Reduce flashing" option ("ROM Expansion/calm.c"): KatAM's white screen flashes (BLDY, lighten) eased in
     * as fades, by detouring the VBlank's copies of the blend registers (UpdateScreenDma, UpdateScreenCpuSet) to calm.c's. It needs the
     * expansion's code, which it puts in itself when no NiDL ROM is loaded (ROM padded to 32 MB, as the expansion's); with the ROM
     * Expansion, it runs after AddEnemies(); */
    internal static partial class KatAMExpansion {
        // ReduceFlashing(); See above. Returns an error, or null (the ROM unchanged on an error);
        public static string ReduceFlashing(ref byte[] rom) {
            KatAMExpansionCode.Version version = VersionOf(rom);

            if (version == null) return "Reduce flashing supports the USA, European and Japanese (revision 1) versions of Kirby & the Amazing Mirror.";
            if (rom.Length > ExpandedSize) return "That ROM is bigger than the GBA can map (32 MB).";

            byte[] patched = rom.Length < ExpandedSize ? Expand(rom) : (byte[])rom.Clone();
            int codeBase = CodeAddress + 0x08000000;

            // The code, unless the ROM Expansion put it there already (then its free space isn't empty);
            bool empty = true;
            for (int i = CodeAddress; i < CodeAddress + version.Code.Length && empty; i++) empty = patched[i] == 0xFF;
            if (empty) Array.Copy(version.Code, 0, patched, CodeAddress, version.Code.Length);

            // The trampolines go in calm.c's room for them (gNidlCalmTrampolines), their addresses in gNidlCalm;
            int cursor = CodeAddress + version.CodeSymbols["gNidlCalmTrampolines"], calm = CodeAddress + version.CodeSymbols["gNidlCalm"];
            if (ReadInt(patched, calm) != 0) return null;  // (applied already);
            int dma = Detour(patched, version.RomSymbols["UpdateScreenDma"], codeBase + version.CodeSymbols["NidlCalmUpdateScreenDma"], ref cursor),
                cpu = dma < 0 ? -1 : Detour(patched, version.RomSymbols["UpdateScreenCpuSet"], codeBase + version.CodeSymbols["NidlCalmUpdateScreenCpuSet"], ref cursor);
            if (dma < 0 || cpu < 0) return $"The {version.Name} ROM's screen code doesn't match.";
            WriteInt(patched, calm, dma + 0x08000000 + 1);
            WriteInt(patched, calm + 4, cpu + 0x08000000 + 1);

            rom = patched;
            return null;
        }
    }
}
