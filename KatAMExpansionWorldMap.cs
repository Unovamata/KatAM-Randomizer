namespace KatAMRandomizer {
    /* KatAMExpansion's world map: a door for each NiDL level (see "ROM Expansion/worldmap.c").
     * • KatAM's big switch (object 0x86) runs gUnk_08357B8C[its template's +0x14]: entries 1-15 call sub_08039600(9 + n), which runs
     *   gUnk_0834BD94[9 + n] (WorldMapUnlock*: CreateWorldMap(n), the unlocking animation). Both tables are copied with 8 more entries:
     *   gUnk_08357B8C[17 + k] calls sub_08039600(27 + k), gUnk_0834BD94[27 + k] is NidlWorldMapUnlock<k> (CreateWorldMap(16 + k));
     * • The tables the animation indexes by door (sWorldMapLineCoors, its dot counts, gWorldMapDotCoors) are copied with 24 entries,
     *   entries 16-23 from gNidlWorldMapDoors (with the dots, from the NiDL door toward where its path leads);
     * • WorldMapUnlockSave, WorldMapGetCoorFromRoom, WorldMapDrawKirbys and GetKirbyRoomFlagIndex (the pause menu: NiDL rooms have no
 *   area map, Select switches between the help and the world map) are detoured to the C code; */
    internal static partial class KatAMExpansion {
        const int WorldMapDoors = 16, NidlWorldMapDoors = 8, NidlDoorSize = 88, NidlMaxDots = 20;
        const int BigSwitchEntries = 17, UnlockEntries = 27;

        // NidlBigSwitchId(); The big switch template's +0x14 of NiDL level `level`;
        static int NidlBigSwitchId(int level) => BigSwitchEntries + level;

        static string AddWorldMap(byte[] rom, KatAMExpansionCode.Version version, ref int cursor) {
            int codeBase = CodeAddress + 0x08000000;
            int doors = CodeAddress + version.CodeSymbols["gNidlWorldMapDoors"];
            int lineCoors = version.RomSymbols["sWorldMapLineCoors"], dotCounts = lineCoors + WorldMapDoors * 4, dotCoors = version.RomSymbols["gWorldMapDotCoors"];
            if (ReadInt(rom, lineCoors + 4) != 0x50705898) return $"The {version.Name} ROM's world map isn't where it should be.";

            // The paths' dots (4 bytes each, as struct WorldMapDotCoor's, which agbcc pads): about every 6.5 pixels, from 5 pixels off the NiDL door to 9 (17: Central Circle) before where it leads, as KatAM's (the doors are placed so their paths are straight, level or 45 degrees, and clear of KatAM's paths and the map's captions);
            for (int k = 0; k < NidlWorldMapDoors; k++) {
                int door = doors + k * NidlDoorSize;
                double x = rom[door], y = rom[door + 1], destX = rom[door + 2], destY = rom[door + 3];
                double length = Math.Sqrt((destX - x) * (destX - x) + (destY - y) * (destY - y));
                double end = length - (destX == 112 && destY == 80 ? 17 : 9) + 0.01;
                int count = 0;
                for (double t = 5; t <= end && count < NidlMaxDots; t += 6.5, count++) {
                    rom[door + 8 + count * 4] = (byte)Math.Round(x + (destX - x) * t / length);
                    rom[door + 9 + count * 4] = (byte)Math.Round(y + (destY - y) * t / length + 2);
                }
                rom[door + 5] = (byte)count;
                // Its door tile in the map's tilemap (as sWorldMapDoorTilemapOffsets');
                WriteShort(rom, door + 6, ((int)y / 8 + 1) * 32 + (int)x / 8 + 1);
            }

            int newLines = Allocate(ref cursor, (WorldMapDoors + NidlWorldMapDoors) * 4, 4),
                newCounts = Allocate(ref cursor, WorldMapDoors + NidlWorldMapDoors, 4),
                newDots = Allocate(ref cursor, (WorldMapDoors + NidlWorldMapDoors) * 4, 4);
            Array.Copy(rom, lineCoors, rom, newLines, WorldMapDoors * 4);
            Array.Copy(rom, dotCounts, rom, newCounts, WorldMapDoors);
            Array.Copy(rom, dotCoors, rom, newDots, WorldMapDoors * 4);
            for (int k = 0; k < NidlWorldMapDoors; k++) {
                int door = doors + k * NidlDoorSize, entry = WorldMapDoors + k;
                Array.Copy(rom, door, rom, newLines + entry * 4, 4);
                rom[newCounts + entry] = rom[door + 5];
                WriteInt(rom, newDots + entry * 4, door + 8 + 0x08000000);
            }
            if (Repoint(rom, lineCoors + 0x08000000, newLines + 0x08000000) != 2 || Repoint(rom, dotCounts + 0x08000000, newCounts + 0x08000000) != 1
                || Repoint(rom, dotCoors + 0x08000000, newDots + 0x08000000) != 1) return $"The {version.Name} ROM's world map code doesn't match.";

            // The unlocking functions, then the big switch's: movs r0, #(27 + k); ldr r1, =sub_08039600; bx r1;
            int unlocks = version.RomSymbols["gUnk_0834BD94"], newUnlocks = Allocate(ref cursor, (UnlockEntries + NidlWorldMapDoors) * 4, 4);
            Array.Copy(rom, unlocks, rom, newUnlocks, UnlockEntries * 4);
            for (int k = 0; k < NidlWorldMapDoors; k++)
                WriteInt(rom, newUnlocks + (UnlockEntries + k) * 4, codeBase + version.CodeSymbols[$"NidlWorldMapUnlock{k}"] + 1);

            int switches = version.RomSymbols["gUnk_08357B8C"], newSwitches = Allocate(ref cursor, (BigSwitchEntries + NidlWorldMapDoors) * 4, 4);
            Array.Copy(rom, switches, rom, newSwitches, BigSwitchEntries * 4);
            for (int k = 0; k < NidlWorldMapDoors; k++) {
                int thunk = Allocate(ref cursor, 12, 4);
                WriteShorts(rom, thunk, 0x2000 | (UnlockEntries + k), 0x4901, 0x4708, 0x46C0);
                WriteInt(rom, thunk + 8, version.RomSymbols["sub_08039600"] + 0x08000000 + 1);
                WriteInt(rom, newSwitches + (BigSwitchEntries + k) * 4, thunk + 0x08000000 + 1);
            }
            if (Repoint(rom, unlocks + 0x08000000, newUnlocks + 0x08000000) != 1 || Repoint(rom, switches + 0x08000000, newSwitches + 0x08000000) != 1)
                return $"The {version.Name} ROM's big switch code doesn't match.";

            if (Detour(rom, version.RomSymbols["WorldMapUnlockSave"], codeBase + version.CodeSymbols["NidlWorldMapUnlockSave"], ref cursor) < 0
                || Detour(rom, version.RomSymbols["WorldMapGetCoorFromRoom"], codeBase + version.CodeSymbols["NidlWorldMapGetCoorFromRoom"], ref cursor) < 0
                || Detour(rom, version.RomSymbols["WorldMapDrawKirbys"], codeBase + version.CodeSymbols["NidlWorldMapDrawKirbys"], ref cursor) < 0)
                return $"The {version.Name} ROM's world map code doesn't match.";

            int roomFlags = Detour(rom, version.RomSymbols["GetKirbyRoomFlagIndex"], codeBase + version.CodeSymbols["NidlGetKirbyRoomFlagIndex"], ref cursor);
            if (roomFlags < 0) return $"The {version.Name} ROM's pause menu code doesn't match.";
            WriteInt(rom, CodeAddress + version.CodeSymbols["gNidlRoomFlagIndexOriginal"], roomFlags + 0x08000000 + 1);
            return null;
        }
    }
}
