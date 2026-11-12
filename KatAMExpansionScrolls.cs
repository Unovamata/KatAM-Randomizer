using System;

namespace KatAMRandomizer {
    /* KatAMExpansionScrolls; What Squeak Squad's Ability Scrolls do (the owner: "For KatAM existing abilities, the scroll will upgrade
     * them with the DS moveset 1:1"): "ROM Expansion/scrolls.c" has the upgrades, each run from the KatAM code it hooks here. Only with
     * Squeak Squad's rooms, where the scrolls' chests are (KatAMExpansionMaps.cs); */
    internal static partial class KatAMExpansion {
        /* AddScrollUpgrades(); Hooks KatAM's ability code into scrolls.c's upgrades:
         * • the fire and ice breath's state (sub_0805D268), set once as the breath starts (sub_0805D164's literal): ScrollBreath, which
         *   runs it and aims the breath as the DS's Fire and Ice scrolls do;
         * • a Kirby's input, read each frame (sub_0803F324, detoured): ScrollKirbyInput, which reads it and then works Spark's charge;
         * • Spark's attack as it starts (sub_08074AB0, KatAM's spark, detoured): ScrollSparkAttack, its level's attack;
         * the detoured functions' trampolines (running them as they were) in gScrollInputOriginal and gScrollSparkOriginal. Returns an
         * error, or null; */
        static string AddScrollUpgrades(byte[] rom, KatAMExpansionCode.Version version, ref int cursor) {
            int codeBase = CodeAddress + 0x08000000;
            if (Repoint(rom, version.RomSymbols["sub_0805D268"] + 0x08000000 + 1, codeBase + version.CodeSymbols["ScrollBreath"] + 1) != 1)
                return $"The {version.Name} ROM's fire breath code isn't where it should be.";
            int input = Detour(rom, version.RomSymbols["sub_0803F324"], codeBase + version.CodeSymbols["ScrollKirbyInput"], ref cursor),
                spark = Detour(rom, version.RomSymbols["sub_08074AB0"], codeBase + version.CodeSymbols["ScrollSparkAttack"], ref cursor);
            if (input < 0 || spark < 0) return $"The {version.Name} ROM's Kirby input or spark code doesn't match.";
            WriteInt(rom, CodeAddress + version.CodeSymbols["gScrollInputOriginal"], input + 0x08000000 + 1);
            WriteInt(rom, CodeAddress + version.CodeSymbols["gScrollSparkOriginal"], spark + 0x08000000 + 1);
            return null;
        }
    }
}
