namespace KatAMRandomizer {
    internal static partial class KatAMExpansion {
        /* KatAM's own chases (the window's Add Chase Sequences to KatAM; the owner: "I would also like to add chase sequences to KatAM, these
         * last random KatAM spawns should be random and optional with a toggle in the randomizer. Since KatAM's chests can not be stolen like
         * Squeak Squad's, the main cast mouses should be faster, more aggresive and go for Kirby to incentivize this chase aspect"): of
         * KatAM's rooms with a big chest (no fight in them, not the tutorial's), one in KatamChaseShare picked from the seed, each a chase
         * of its own (squeak.h's "The chases": its key KatamChaseFirstKey on, its gimmick list in squeak/gimmicks.c's gSqueakChaseRooms):
         * a wall at its chest, and one to three Squeaks (Spinni, Doc, Storo, a Squeaker of any color, picked from the seed) at its own
         * enemies' spots at least KatamChaseNear pixels from the chest (none: its other objects' spots, items' and doors', places KatAM's
         * own put things), out once the wall's passed, after Kirby at their angry pace; */
        const int KatamChaseShare = 2, KatamChaseNear = 96, KatamChaseMost = 3, KatamChaseFirstKey = 0x80, TemplateSize = 36;
        static readonly (byte character, byte[] variants)[] KatamChaseSqueaks = {
            (29, new byte[] { 0 }), (31, new byte[] { 0 }), (33, new byte[] { 0, 1, 2 }), (27, new byte[] { 0, 1, 2, 3 }),
        };
        internal sealed record KatamChaseRoom(int Room, int ChestX, int ChestY, List<(int x, int y)> Spots);

        /* KatamChaseCandidates(); KatAM's rooms (its own, as the ROM comes: before the ROM Expansion adds its rooms) with a big chest
         * (type 0x81) and an enemy spot (types 0-0x37) far enough from it, no mini-boss or boss (types 0x38 on) in them; */
        static List<KatamChaseRoom> KatamChaseCandidates(byte[] rom, KatAMExpansionCode.Version version) {
            List<KatamChaseRoom> rooms = new List<KatamChaseRoom>();
            int roomProps = version.RomSymbols["gRoomProps"], lists = version.RomSymbols["gLevelObjLists"];
            for (int room = 1; room < KatAMRoomCount; room++) {
                if (room == TutorialChestRoom) continue;
                int index = ReadShort(rom, roomProps + room * 0x28 + 16 * 2);
                if (index == 0xFFFF) continue;
                int list = ReadInt(rom, lists + index * 4) - 0x08000000;
                if (list < 0 || list + 4 > rom.Length) continue;
                int at = ReadInt(rom, list) - 0x08000000;
                if (at < 0 || at >= rom.Length) continue;
                (int x, int y)? chest = null;
                List<(int x, int y)> enemies = new List<(int, int)>(), others = new List<(int, int)>();
                bool fight = false;
                for (; at + TemplateSize <= rom.Length && (rom[at] != 0 || rom[at + 1] != 0); at += TemplateSize) {
                    byte type = rom[at + 12];
                    (int x, int y) spot = ((short)ReadShort(rom, at + 6), (short)ReadShort(rom, at + 8));
                    if (type == KatAMBigChestType) chest ??= spot;
                    else if (type < 0x38) enemies.Add(spot);
                    else if (type <= 0x52) fight = true;
                    else others.Add(spot);
                }
                if (chest is not (int cx, int cy) || fight) continue;
                bool Far((int x, int y) e) => Math.Abs(e.x - cx) >= KatamChaseNear || Math.Abs(e.y - cy) >= KatamChaseNear;
                List<(int x, int y)> spots = enemies.Where(Far).Distinct().ToList();
                if (spots.Count == 0) spots = others.Where(Far).Distinct().ToList();
                if (spots.Count > 0) rooms.Add(new KatamChaseRoom(room, cx, cy, spots));
            }
            return rooms;
        }

        /* AddKatamChases(); The picked rooms' chases: each one's gimmick list (squeak/gimmicks.c: count, key, then 24 bytes an entry: its
         * wall, its Squeaks) and gSqueakChaseRooms (8 bytes a room: its id, its list); the rooms picked kept for the spoiler log; */
        public static List<(int room, List<string> squeaks)> KatamChases { get; } = new List<(int, List<string>)>();
        static string AddKatamChases(byte[] rom, KatAMExpansionCode.Version version, List<KatamChaseRoom> candidates, int seed, ref int cursor) {
            KatamChases.Clear();
            if (candidates == null || candidates.Count == 0) return null;
            Random random = new Random(seed ^ 0x5C4A5E);
            List<KatamChaseRoom> picked = candidates.OrderBy(_ => random.Next()).Take(Math.Min(KatamChaseFirstKey - 1, (candidates.Count + KatamChaseShare - 1) / KatamChaseShare))
                                                    .OrderBy(r => r.Room).ToList();
            int table = Allocate(ref cursor, picked.Count * 8, 4);
            for (int k = 0; k < picked.Count; k++) {
                KatamChaseRoom r = picked[k];
                List<(int x, int y)> spots = r.Spots.OrderBy(_ => random.Next()).Take(random.Next(1, KatamChaseMost + 1)).ToList();
                List<(int x, int y, byte character, byte variant)> spawns = new List<(int, int, byte, byte)>();
                List<string> names = new List<string>();
                foreach ((int x, int y) in spots) {
                    (byte character, byte[] variants) = KatamChaseSqueaks[random.Next(KatamChaseSqueaks.Length)];
                    spawns.Add((x, y, character, variants[random.Next(variants.Length)]));
                    names.Add(SqueakPoolName(character));
                }
                List<byte> list = new List<byte> { 0, 0, 0, 0 };
                AddChaseEntries(list, new SqueakChase((byte)(KatamChaseFirstKey + k), new List<(int, int)> { (r.ChestX, r.ChestY) }, spawns), 0);
                int listAt = Allocate(ref cursor, list.Count, 4);
                list.ToArray().CopyTo(rom, listAt);
                WriteShort(rom, table + k * 8, r.Room);
                WriteShort(rom, table + k * 8 + 2, 0);
                WriteInt(rom, table + k * 8 + 4, listAt + 0x08000000);
                KatamChases.Add((r.Room, names));
            }
            WriteInts(rom, CodeAddress + version.CodeSymbols["gSqueakChaseRooms"], table + 0x08000000, picked.Count);
            return null;
        }
    }
}
