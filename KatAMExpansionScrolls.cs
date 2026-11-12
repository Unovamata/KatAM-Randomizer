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
         * • Beam's whip (its hit box sub_0807CFE4 and its chain sub_0807D2D0, sub_0807D4B0, detoured): ScrollBeamStart, the DS's
         *   upgraded whip, ScrollBeamChain and ScrollBeamChain2, none (KatAM's whip without the scroll);
         * • Tornado's attack as it starts (sub_0807A10C, detoured): ScrollTornadoStart, the piece giving it an element;
         * • Parasol's twirl's attack as it starts (sub_08084188, detoured): ScrollParasolTwirl, the piece of the DS's stars and boxes;
         * • Cutter's throw (sub_08078EFC, detoured): ScrollCutterThrow, the cutter drawn as the DS's upgraded one, its box grown;
         * • Cupid's shot (sub_080680C0, detoured): ScrollCupidShot, the DS's volley of three in its arrows' place;
         * • Bomb's throw (sub_08082380) and a blast (sub_08074240), detoured: ScrollBombThrow, the piece giving his bombs an element and
         *   the DS's look, and ScrollBombBlast, his blast that element's;
         * the detoured functions' trampolines (running them as they were) in gScroll*Original. Returns an error, or null; */
        static string AddScrollUpgrades(byte[] rom, KatAMExpansionCode.Version version, ref int cursor) {
            int codeBase = CodeAddress + 0x08000000;
            if (Repoint(rom, version.RomSymbols["sub_0805D268"] + 0x08000000 + 1, codeBase + version.CodeSymbols["ScrollBreath"] + 1) != 1)
                return $"The {version.Name} ROM's fire breath code isn't where it should be.";
            int input = Detour(rom, version.RomSymbols["sub_0803F324"], codeBase + version.CodeSymbols["ScrollKirbyInput"], ref cursor),
                spark = Detour(rom, version.RomSymbols["sub_08074AB0"], codeBase + version.CodeSymbols["ScrollSparkAttack"], ref cursor);
            if (input < 0 || spark < 0) return $"The {version.Name} ROM's Kirby input or spark code doesn't match.";
            WriteInt(rom, CodeAddress + version.CodeSymbols["gScrollInputOriginal"], input + 0x08000000 + 1);
            WriteInt(rom, CodeAddress + version.CodeSymbols["gScrollSparkOriginal"], spark + 0x08000000 + 1);
            int whip = Detour(rom, version.RomSymbols["sub_0807CFE4"], codeBase + version.CodeSymbols["ScrollBeamStart"], ref cursor),
                chain = Detour(rom, version.RomSymbols["sub_0807D2D0"], codeBase + version.CodeSymbols["ScrollBeamChain"], ref cursor),
                chain2 = Detour(rom, version.RomSymbols["sub_0807D4B0"], codeBase + version.CodeSymbols["ScrollBeamChain2"], ref cursor);
            if (whip < 0 || chain < 0 || chain2 < 0) return $"The {version.Name} ROM's beam code doesn't match.";
            WriteInt(rom, CodeAddress + version.CodeSymbols["gScrollBeamOriginal"], whip + 0x08000000 + 1);
            WriteInt(rom, CodeAddress + version.CodeSymbols["gScrollChainOriginal"], chain + 0x08000000 + 1);
            WriteInt(rom, CodeAddress + version.CodeSymbols["gScrollChain2Original"], chain2 + 0x08000000 + 1);
            int tornado = Detour(rom, version.RomSymbols["sub_0807A10C"], codeBase + version.CodeSymbols["ScrollTornadoStart"], ref cursor);
            if (tornado < 0) return $"The {version.Name} ROM's tornado code doesn't match.";
            WriteInt(rom, CodeAddress + version.CodeSymbols["gScrollTornadoOriginal"], tornado + 0x08000000 + 1);
            int twirl = Detour(rom, version.RomSymbols["sub_08084188"], codeBase + version.CodeSymbols["ScrollParasolTwirl"], ref cursor);
            if (twirl < 0) return $"The {version.Name} ROM's parasol code doesn't match.";
            WriteInt(rom, CodeAddress + version.CodeSymbols["gScrollParasolOriginal"], twirl + 0x08000000 + 1);
            int cutter = Detour(rom, version.RomSymbols["sub_08078EFC"], codeBase + version.CodeSymbols["ScrollCutterThrow"], ref cursor);
            if (cutter < 0) return $"The {version.Name} ROM's cutter code doesn't match.";
            WriteInt(rom, CodeAddress + version.CodeSymbols["gScrollCutterOriginal"], cutter + 0x08000000 + 1);
            int cupid = Detour(rom, version.RomSymbols["sub_080680C0"], codeBase + version.CodeSymbols["ScrollCupidShot"], ref cursor);
            if (cupid < 0) return $"The {version.Name} ROM's cupid code doesn't match.";
            WriteInt(rom, CodeAddress + version.CodeSymbols["gScrollCupidOriginal"], cupid + 0x08000000 + 1);
            int bomb = Detour(rom, version.RomSymbols["sub_08082380"], codeBase + version.CodeSymbols["ScrollBombThrow"], ref cursor),
                blast = Detour(rom, version.RomSymbols["sub_08074240"], codeBase + version.CodeSymbols["ScrollBombBlast"], ref cursor);
            if (bomb < 0 || blast < 0) return $"The {version.Name} ROM's bomb code doesn't match.";
            WriteInt(rom, CodeAddress + version.CodeSymbols["gScrollBombOriginal"], bomb + 0x08000000 + 1);
            WriteInt(rom, CodeAddress + version.CodeSymbols["gScrollBlastOriginal"], blast + 0x08000000 + 1);
            return null;
        }
    }
}
