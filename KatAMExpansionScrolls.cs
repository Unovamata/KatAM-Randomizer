using System;

namespace KatAMRandomizer {
    /* KatAMExpansionScrolls; What Squeak Squad's Ability Scrolls do (the owner: "For KatAM existing abilities, the scroll will upgrade
     * them with the DS moveset 1:1"): "ROM Expansion/scrolls.c" has the upgrades, each run from the KatAM code it hooks here. Only with
     * Squeak Squad's rooms, where the scrolls' chests are (KatAMExpansionMaps.cs); */
    internal static partial class KatAMExpansion {
        /* AddScrollUpgrades(); Hooks KatAM's ability code into scrolls.c's upgrades:
         * • the fire and ice breath's state (sub_0805D268), set once as the breath starts (sub_0805D164's literal): ScrollBreath, which
         *   runs it and aims the breath as the DS's Fire and Ice scrolls do;
         * Returns an error, or null; */
        static string AddScrollUpgrades(byte[] rom, KatAMExpansionCode.Version version) {
            int codeBase = CodeAddress + 0x08000000;
            if (Repoint(rom, version.RomSymbols["sub_0805D268"] + 0x08000000 + 1, codeBase + version.CodeSymbols["ScrollBreath"] + 1) != 1)
                return $"The {version.Name} ROM's fire breath code isn't where it should be.";
            return null;
        }
    }
}
