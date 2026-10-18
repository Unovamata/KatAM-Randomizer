using System;
using System.Collections.Generic;
using System.Linq;

namespace KatAMRandomizer {
    /* KatAMExpansionChests; Mirror shards in chests ("ROM Expansion/chest.c"; KatAMItems puts the shards it adds in small chests
     * holding KatAMItems.ShardChest + n): a chest's opening (sub_0800AFC8) detoured to chest.c's, which lets shard n out of those
     * and runs the original for the others. The shards it puts in rooms instead become Maxim Tomatoes once owned (OwnedFieldShards).
     * Both need the expansion's code, which they put in themselves when no NiDL ROM is loaded (PatchWithCode); */
    internal static partial class KatAMExpansion {
        // AddShardChests(); See above. Returns an error, or null (the ROM unchanged on an error);
        public static string AddShardChests(ref byte[] rom) => PatchWithCode(ref rom, "Mirror shards in chests", (patched, version) => {
            // The trampoline goes in chest.c's room for it (gNidlChestTrampolines), its address in gNidlChest;
            int cursor = CodeAddress + version.CodeSymbols["gNidlChestTrampolines"], chest = CodeAddress + version.CodeSymbols["gNidlChest"];
            if (ReadInt(patched, chest) != 0) return null;  // (applied already);
            string hooked = HookShards(patched, version);
            if (hooked != null) return hooked;
            int open = Detour(patched, version.RomSymbols["sub_0800AFC8"], CodeAddress + 0x08000000 + version.CodeSymbols["NidlChestOpen"], ref cursor);
            if (open < 0) return $"The {version.Name} ROM's chest code doesn't match.";
            WriteInt(patched, chest, open + 0x08000000 + 1);
            return null;
        });

        /* OwnedFieldShards(); For the shards the randomizer puts in rooms (not in chests): their spawn (object 0x65's, the spawn table's)
         * becomes chest.c's NidlCreateShard, which makes one already owned a Maxim Tomato (the original's address in gNidlShard).
         * Returns an error, or null (the ROM unchanged on an error); */
        public static string OwnedFieldShards(ref byte[] rom) => PatchWithCode(ref rom, "Owned mirror shards as tomatoes", HookShards);

        /* HookShards(); Object 0x65's spawn becomes NidlCreateShard (owned field shards as tomatoes, bosses' shard numbers), and the
         * shard's step after its cutscene (sub_0801C558) jumps to NidlShardDone, which does its work itself (only bosses' shards take
         * Kirby out of the level: a room's or a chest's leaves him where he is, without the victory dance; no trampoline: its first
         * instructions keep the shard in r3, which the jump back would need); */
        static string HookShards(byte[] patched, KatAMExpansionCode.Version version) {
            int entry = version.SpawnTable + 0x65 * 4, shard = CodeAddress + version.CodeSymbols["gNidlShard"];
            int create = CodeAddress + 0x08000000 + version.CodeSymbols["NidlCreateShard"] + 1;
            if (ReadInt(patched, entry) == create) return null;  // (applied already);
            if (ReadInt(patched, entry) >> 24 != 0x08) return $"The {version.Name} ROM's mirror shard code doesn't match.";
            int step = version.RomSymbols["sub_0801C558"];
            if ((step & 3) != 0 || ReadShort(patched, step) != 0xB5F0) return $"The {version.Name} ROM's mirror shard code doesn't match.";
            WriteShorts(patched, step, 0x4B00, 0x4718);    // ldr r3, [pc, #0]; bx r3;
            WriteInt(patched, step + 4, CodeAddress + 0x08000000 + version.CodeSymbols["NidlShardDone"] + 1);
            WriteInt(patched, shard, ReadInt(patched, entry));
            WriteInt(patched, entry, create);
            return null;
        }

        /* RandomizeBossShards(); The randomizer's "Bosses Give Random Shards": KatAM's 8 bosses leave the 8 shards in a shuffled order (all
         * of them still, so the game is beaten as before), then the ROM Expansion's NiDL bosses leave them again, spread as evenly as
         * they go (each shard at most once more than another, in a second shuffled order). For KatAM's: each boss's defeat (gUnk_08350E58's
         * handlers) asks HasShard about its own shard as a constant right before the call (movs r0, #n; bl HasShard), as King Golem's
         * harder rematch does: every such constant n becomes the boss's new shard, so an owned one still leaves a tomato; the shard it
         * makes is given its new number by chest.c (gNidlShard +4, by the game's shard; HookShards); the pause screen's area maps show
         * a boss room's shard icon until the area's shard is owned (gAreaMapRoomIdsAndShards: {map room, boss room, shard} for each
         * area): the area's new one. For NiDL's: the low nibble of their templates' behavior (type 0x43 or 0x44 in the rooms' object
         * lists: nidl_boss.c's NidlBossDefeat). BossShardLog lists who leaves what (the spoiler log). Returns an error, or null; */
        static readonly string[] KatAMShardBosses = { "Kracko", "King Golem", "Master Hand & Crazy Hand", "Gobbler", "Wiz", "Moley", "Mega Titan", "Dark Meta Knight" };
        static readonly string[] NidlBossNames = { "Whispy Woods", "Kracko (NiDL)", "Mr. Shine & Mr. Bright", "Meta Knight", "Paint Roller", "Heavy Mole",
                                                   "King Dedede", "Nightmare's Power Orb", "Nightmare" };
        public static List<string> BossShardLog { get; } = new List<string>();

        public static string RandomizeBossShards(ref byte[] rom, Random random) => PatchWithCode(ref rom, "Bosses Give Random Shards", (patched, version) => {
            string error = HookShards(patched, version);
            if (error != null) return error;
            int[] shards = Enumerable.Range(0, 8).OrderBy(_ => random.Next()).ToArray();

            // (every "movs r0, #n; bl HasShard" with n a shard: the 8 bosses' defeats and King Golem's rematch);
            int hasShard = version.RomSymbols["HasShard"] + 0x08000000;
            List<int> sites = new List<int>();
            for (int at = 2; at + 4 <= 0x400000; at += 2) {
                int h1 = ReadShort(patched, at), h2 = ReadShort(patched, at + 2);
                if (h1 >> 11 != 0x1E || h2 >> 11 != 0x1F) continue;
                int offset = (h1 & 0x7FF) << 12 | (h2 & 0x7FF) << 1;
                if ((offset & 0x400000) != 0) offset -= 0x800000;
                int before = ReadShort(patched, at - 2);
                if (at + 4 + offset + 0x08000000 == hasShard && before >> 8 == 0x20 && (before & 0xFF) < 8) sites.Add(at - 2);
            }
            if (sites.Count != 9 || sites.Select(s => patched[s]).Distinct().Count() != 8) return $"The {version.Name} ROM's bosses don't leave their shards as expected.";

            int areaMaps = -1;
            byte[] firstArea = { 0xC3, 0x00, 0xC6, 0x02, 0x01, 0x00 };
            for (int at = 0x800000; at + 6 <= 0x1000000 && areaMaps < 0; at += 2)
                if (patched.AsSpan(at, 6).SequenceEqual(firstArea)) areaMaps = at;
            if (areaMaps < 0) return $"The {version.Name} ROM's area maps don't match.";

            foreach (int site in sites) patched[site] = (byte)shards[patched[site]];
            for (int area = 0; area < 8; area++) {
                int shard = ReadShort(patched, areaMaps + area * 6 + 4);
                if (shard < 8) WriteShort(patched, areaMaps + area * 6 + 4, shards[shard]);
            }
            for (int n = 0; n < 8; n++) patched[CodeAddress + version.CodeSymbols["gNidlShard"] + 4 + n] = (byte)shards[n];

            BossShardLog.Clear();
            for (int n = 0; n < 8; n++) BossShardLog.Add($"{KatAMShardBosses[n]}: Mirror Shard #{shards[n] + 1}");

            // The NiDL bosses, in room order, a shard each from a second shuffle, round and round;
            if (patched.AsSpan(MapRoomsBlock, 2).SequenceEqual(new byte[] { (byte)'N', (byte)'R' })) {
                int lists = ReadInt(patched, MapRoomsBlock + 4 + 5 * 4) - 0x08000000, props = version.RomSymbols["gRoomProps"];
                int[] again = Enumerable.Range(0, 8).OrderBy(_ => random.Next()).ToArray();
                int count = 0;
                for (int room = 0; room < 0x3D7; room++) {
                    int objects = ReadShort(patched, props + room * 0x28 + 32);
                    if (objects == 0xFFFF || ReadShort(patched, props + room * 0x28) == 0xFFFF) continue;
                    int list = ReadInt(patched, lists + objects * 4) - 0x08000000;
                    if (list < 0 || list + 4 > patched.Length) continue;
                    int at = ReadInt(patched, list) - 0x08000000;
                    for (int k = 0; at >= 0 && at + 36 <= patched.Length && (patched[at] != 0 || patched[at + 1] != 0) && k < 256; at += 36, k++) {
                        if (!IsBossType(patched[at + 12])) continue;
                        int boss = patched[at + 14] >> 4, shard = again[count++ % 8];
                        patched[at + 14] = (byte)(boss << 4 | shard);
                        BossShardLog.Add($"{(boss < NidlBossNames.Length ? NidlBossNames[boss] : $"NiDL boss {boss}")} (room 0x{room:X}): Mirror Shard #{shard + 1}");
                    }
                }
            }
            return null;
        });
    }
}
