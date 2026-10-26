namespace KatAMRandomizer {
    /* KatAMExpansionCalm; The "Reduce Flashing" option ("ROM Expansion/calm.c"): KatAM's white screen flashes (BLDY, lighten) eased in
     * as fades, by detouring the VBlank's copies of the blend registers (UpdateScreenDma, UpdateScreenCpuSet) to calm.c's. It needs the
     * expansion's code, which it puts in itself when no NiDL ROM is loaded (PatchWithCode); with the ROM Expansion, it runs after
     * AddEnemies(); */
    internal static partial class KatAMExpansion {
        // ReduceFlashing(); See above. Returns an error, or null (the ROM unchanged on an error);
        public static string ReduceFlashing(ref byte[] rom) => PatchWithCode(ref rom, "Reduce Flashing", (patched, version) => {
            // The trampolines go in calm.c's room for them (gNidlCalmTrampolines), their addresses in gNidlCalm;
            int codeBase = CodeAddress + 0x08000000, cursor = CodeAddress + version.CodeSymbols["gNidlCalmTrampolines"],
                calm = CodeAddress + version.CodeSymbols["gNidlCalm"];
            if (ReadInt(patched, calm) != 0) return null;  // (applied already);
            int dma = Detour(patched, version.RomSymbols["UpdateScreenDma"], codeBase + version.CodeSymbols["NidlCalmUpdateScreenDma"], ref cursor),
                cpu = dma < 0 ? -1 : Detour(patched, version.RomSymbols["UpdateScreenCpuSet"], codeBase + version.CodeSymbols["NidlCalmUpdateScreenCpuSet"], ref cursor);
            if (dma < 0 || cpu < 0) return $"The {version.Name} ROM's screen code doesn't match.";
            WriteInt(patched, calm, dma + 0x08000000 + 1);
            WriteInt(patched, calm + 4, cpu + 0x08000000 + 1);
            return null;
        });

        /* SkipTutorial(); The "Skip the Tutorial" option ("ROM Expansion/skip.c"): the save buffers' world props clear (sbClearFunctions[1],
         * what a new save runs) becomes NidlSkipClear, given the original and the item and treasure index of the tutorial's big chest
         * (TutorialChestRoom's, as the randomizer left it: what skipping it would lose); and the single-player start (sub_08123FD4, what
         * a new and a continued game run) is detoured to NidlSkipStart (its trampoline in gNidlSkipTrampolines, its address in
         * gNidlSkip +8), for the files no clear touched. Returns an error, or null; */
        const int TutorialChestRoom = 0x325;
        public static string SkipTutorial(ref byte[] rom) => PatchWithCode(ref rom, "Skip the Tutorial", (patched, version) => {
            int table = version.RomSymbols["sbClearFunctions"], skip = CodeAddress + version.CodeSymbols["gNidlSkip"];
            int clear = CodeAddress + 0x08000000 + version.CodeSymbols["NidlSkipClear"] + 1;
            if (ReadInt(patched, table + 4) == clear) return null;  // (applied already);
            // (KatAM's clear, or the ROM Expansion's, which runs it: rooms.c's NidlClearSave);
            if (ReadInt(patched, table + 4) >> 24 is not (0x08 or 0x09)) return $"The {version.Name} ROM's save code doesn't match.";

            // (the room's object list, through the original tables: the ROM Expansion's copies point to the same lists);
            int objects = ReadShort(patched, version.RomSymbols["gRoomProps"] + TutorialChestRoom * 0x28 + 16 * 2);
            int list = ReadInt(patched, version.RomSymbols["gLevelObjLists"] + objects * 4) - 0x08000000;
            byte item = 0, index = 0;
            for (int at = ReadInt(patched, list) - 0x08000000; patched[at] != 0 || patched[at + 1] != 0; at += 36)
                if (patched[at + 12] is 0x80 or 0x81) { item = patched[at + 14]; index = patched[at + 17]; break; }
            WriteInt(patched, skip, ReadInt(patched, table + 4));
            patched[skip + 4] = item;
            patched[skip + 5] = index;
            int cursor = CodeAddress + version.CodeSymbols["gNidlSkipTrampolines"];
            int start = Detour(patched, version.RomSymbols["sub_08123FD4"], CodeAddress + 0x08000000 + version.CodeSymbols["NidlSkipStart"], ref cursor);
            if (start < 0) return $"The {version.Name} ROM's game start code doesn't match.";
            WriteInt(patched, skip + 8, start + 0x08000000 + 1);
            WriteInt(patched, table + 4, clear);
            return null;
        });

        /* UnlockMasterSword(); The "Unlock the Master Sword" option: the Master Sword's stand in Central Circle (OBJ_MASTER_SWORD_STAND)
         * only works and shows once the game is beaten (gAIKirbyState >= AI_KIRBY_STATE_UNK1); both checks (as it runs, sub_080AA618,
         * and as it's drawn, sub_080AAA64) are dropped, so it's there from the start. Returns an error, or null; */
        const int SwordRunCheck = 0x14, SwordDrawCheck = 0x14;
        public static string UnlockMasterSword(ref byte[] rom) => PatchWithCode(ref rom, "Unlock the Master Sword", (patched, version) => {
            int run = version.RomSymbols["sub_080AA618"] + SwordRunCheck, draw = version.RomSymbols["sub_080AAA64"] + SwordDrawCheck;
            if (ReadShort(patched, run) == 0x46C0 && ReadShort(patched, draw) == 0xE004) return null;  // (applied already);
            // (bls past the stand's work, bhi past its hiding: a nop, and an always taken b);
            if (ReadShort(patched, run) != 0xD950 || ReadShort(patched, draw) != 0xD804) return $"The {version.Name} ROM's Master Sword code doesn't match.";
            WriteShort(patched, run, 0x46C0);
            WriteShort(patched, draw, 0xE004);
            return null;
        });

        /* UnlockAbilityRoom(); The "Unlock the Ability Room" option ("ROM Expansion/unlock.c"): Central Circle's door to the ability room
         * (and the NiDL hub's copy) is a hub mirror object (0x8C) waiting for world slot 0x12, which KatAM sets once all 15 big switches
         * are pressed; the hub mirror's init (SpecialHubMirrorInit, its address in CreateSpecialHubMirror's literal pool, as the object's
         * unk78) becomes NidlHubMirrorInit, which sets that slot first, the original kept in gNidlUnlock. Returns an error, or null; */
        public static string UnlockAbilityRoom(ref byte[] rom) => PatchWithCode(ref rom, "Unlock the Ability Room", (patched, version) => {
            int create = version.RomSymbols["CreateSpecialHubMirror"], init = version.RomSymbols["SpecialHubMirrorInit"] + 0x08000000 + 1;
            int hook = CodeAddress + 0x08000000 + version.CodeSymbols["NidlHubMirrorInit"] + 1, unlock = CodeAddress + version.CodeSymbols["gNidlUnlock"];
            // (its literal pool: words after its code, within 0x100 bytes of its start; USA 0x0801E37C);
            for (int at = (create + 3) & ~3; at < create + 0x100; at += 4) {
                if (ReadInt(patched, at) == hook) return null;  // (applied already);
                if (ReadInt(patched, at) != init) continue;
                WriteInt(patched, unlock, init);
                WriteInt(patched, at, hook);
                return null;
            }
            return $"The {version.Name} ROM's hub mirror code doesn't match.";
        });

        /* StartingKirby(); The "Starting Lives" and "Random Starting HP" options, in KatAM's own code (no ROM Expansion needed): a new
         * game's Kirby gets `kirby->lives = 2` and `maxHp = NumVitalitiesCollected() + 6` (sub_0803EA90), a game over gives the 2 lives
         * back (sub_0804FBFC twice, sub_080506A8) and an HP Up's chest works the HP out again (sub_0800B97C, with helpers and without):
         * each `movs r0, #2` takes `lives` (the owner's: 1-5), each `adds r0, #6` takes `hp` (the owner's: 3-6; the HUD's HP bar fits
         * about 34, HP Ups on top). Returns an error, or null; */
        static readonly (string function, int offset, int next)[] LivesSites = {
            ("sub_0803EA90", 0x96, 0x7010), ("sub_0804FBFC", 0x3E0, 0x7008), ("sub_0804FBFC", 0x5A8, 0x7008), ("sub_080506A8", 0x14A, 0x7018) };
        static readonly (string function, int offset, int next)[] HPSites = {
            ("sub_0803EA90", 0x82, 0x4A34), ("sub_0800B97C", 0x176, 0x4A22), ("sub_0800B97C", 0x21E, 0x4A24) };
        public static string StartingKirby(byte[] rom, int lives, int hp) {
            KatAMExpansionCode.Version version = VersionOf(rom);

            if (version == null) return "Starting Lives and Random Starting HP support the USA, European and Japanese (revision 1) versions of Kirby & the Amazing Mirror.";
            // (movs r0, #n and adds r0, #n: their immediate in the low byte);
            foreach (var (sites, opcode) in new[] { (LivesSites, 0x2000), (HPSites, 0x3000) })
                foreach (var (function, offset, next) in sites) {
                    int at = version.RomSymbols[function] + offset;
                    if ((ReadShort(rom, at) & 0xFF00) != opcode || ReadShort(rom, at + 2) != next) return $"The {version.Name} ROM's Kirby code doesn't match.";
                }
            foreach (var (function, offset, _) in LivesSites) rom[version.RomSymbols[function] + offset] = (byte)lives;
            foreach (var (function, offset, _) in HPSites) rom[version.RomSymbols[function] + offset] = (byte)hp;
            return null;
        }

        /* PatchWithCode(); Runs `patch` on a copy of the ROM holding the expansion's code, which it puts in when the ROM Expansion
         * didn't (its free space still empty; the ROM padded to 32 MB, as the expansion's). `rom` becomes the copy unless there's an
         * error (returned, or null); `option` names what needs it in errors; */
        static string PatchWithCode(ref byte[] rom, string option, Func<byte[], KatAMExpansionCode.Version, string> patch) {
            KatAMExpansionCode.Version version = VersionOf(rom);

            if (version == null) return $"{option} supports the USA, European and Japanese (revision 1) versions of Kirby & the Amazing Mirror.";
            if (rom.Length > ExpandedSize) return "That ROM is bigger than the GBA can map (32 MB).";

            byte[] patched = rom.Length < ExpandedSize ? Expand(rom) : (byte[])rom.Clone();

            bool empty = true;
            for (int i = CodeAddress; i < CodeAddress + version.Code.Length && empty; i++) empty = patched[i] == 0xFF;
            if (empty) Array.Copy(version.Code, 0, patched, CodeAddress, version.Code.Length);

            string error = patch(patched, version);
            if (error == null) rom = patched;
            return error;
        }

        // CodeSymbolOf(); Where one of the ROM Expansion code's symbols is in the ROM, or -1 when its code isn't in it (KatAMHP.cs: its HP tables);
        public static int CodeSymbolOf(byte[] rom, string name) {
            KatAMExpansionCode.Version version = VersionOf(rom);

            if (version == null || rom.Length < ExpandedSize || !version.CodeSymbols.TryGetValue(name, out int offset)) return -1;
            for (int i = CodeAddress; i < CodeAddress + version.Code.Length; i++)
                if (rom[i] != 0xFF) return CodeAddress + offset;
            return -1;
        }
    }
}
