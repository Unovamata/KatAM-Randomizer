namespace KatAMRandomizer {
    /* KatAMExpansionChests; Mirror shards in chests ("ROM Expansion/chest.c"; KatAMItems puts the shards it adds in small chests
     * holding KatAMItems.ShardChest + n): a chest's opening (sub_0800AFC8) detoured to chest.c's, which lets shard n out of those
     * and runs the original for the others. It needs the expansion's code, which it puts in itself when no NiDL ROM is loaded
     * (PatchWithCode); */
    internal static partial class KatAMExpansion {
        // AddShardChests(); See above. Returns an error, or null (the ROM unchanged on an error);
        public static string AddShardChests(ref byte[] rom) => PatchWithCode(ref rom, "Mirror shards in chests", (patched, version) => {
            // The trampoline goes in chest.c's room for it (gNidlChestTrampolines), its address in gNidlChest;
            int cursor = CodeAddress + version.CodeSymbols["gNidlChestTrampolines"], chest = CodeAddress + version.CodeSymbols["gNidlChest"];
            if (ReadInt(patched, chest) != 0) return null;  // (applied already);
            int open = Detour(patched, version.RomSymbols["sub_0800AFC8"], CodeAddress + 0x08000000 + version.CodeSymbols["NidlChestOpen"], ref cursor);
            if (open < 0) return $"The {version.Name} ROM's chest code doesn't match.";
            WriteInt(patched, chest, open + 0x08000000 + 1);
            return null;
        });
    }
}
