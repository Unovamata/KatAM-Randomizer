using KatAMInternal;
using KatAM_Randomizer;
using System.Reflection;
using System.Text;

namespace KatAMRandomizer
{
    /* KatAMSpoilerLog; Writes what the randomizer changed, comparing the original ROM against the randomized one:
     * • The seed, the ROM and every setting used.
     * • Every mirror (door) and the room it leads to now, and where the Boss & Goal game rooms are reached from.
     * • Enemies, mini-bosses, chests, items, ability pedestals and map elements that changed.
     * The entities come from the JSON files the ROM reader writes from the original ROM, re-read from the randomized one; */
    internal class KatAMSpoilerLog : KatAMRandomizerComponent, IKatAMRandomizer {
        // Room data (Japanese addresses, translated to the loaded ROM's region; See RomRegion);
        const long LongRoomParameters = 0x903338, CoverDataTable = 0xD2E74C;
        const int LongRoomParameterSize = 0x28, MaxLongRoom = 0x3D7, MaxShortRoom = 0x11F, DoorObjectType = 3,
                  HubArea = 4, BossAndGoalArea = 0x25;

        readonly byte[] originalROM, randomizedROM;
        readonly StringBuilder log = new StringBuilder();

        // Chest contents (behavior of chest objects); 0x00 - 0x05 are the consumables, in the same order as their item IDs;
        static readonly Dictionary<byte, string> chestTreasures = new Dictionary<byte, string> {
            { 0x06, "Vitality (HP Up)" },
            { 0x0A, "Area Map" }, { 0x0B, "Area Map" }, { 0x0C, "Area Map" }, { 0x0D, "Area Map" }, { 0x0E, "Area Map" }, { 0x0F, "Area Map" },
        };

        // Abilities of the ability pedestals, by pedestal ID and behavior;
        static readonly Dictionary<byte, string[]> pedestalAbilities = new Dictionary<byte, string[]> {
            { 0x92, new[] { "Beam", "Flame", "Laser", "Fire", "Rock", "Fighter", "Bomb", "Tornado" } },
            { 0x93, new[] { "Wheel", "Ice", "Electric", "Missile", "Smash", "Throw", "Hammer" } },
            { 0x94, new[] { "Parasol" } },
            { 0x95, new[] { "Sword", "Cutter", "Cupid" } },
            { 0x96, new[] { "Random" } },
        };

        public KatAMSpoilerLog(Processing system, byte[] original) {
            InitializeComponents(system);

            originalROM = original;
            randomizedROM = Processing.ROMData;

            WriteHeader();
            WriteSettings();
            WriteMirrors();
            WriteEntities("Enemies", Utils.enemiesJson, false, false);
            WriteEntities("Mini-Bosses", Utils.minibossesJson, false, false);
            WriteEntities("Chests & Items", Utils.itemsJson, true, true);
            WriteEntities("Ability Pedestals", Utils.abilityStandsJson, true, true);
            WriteEntities("Map Elements", Utils.worldMapObjectsJson, false, true);
        }

        public bool FilterEntities(Entity entity) => true;

        public void Save(string path) {
            try {
                File.WriteAllText(path, log.ToString());
            } catch (Exception ex) {
                MessageBox.Show($"Error saving the spoiler log: {ex.Message}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        //////////////////////////////////////////////////////////////////////////////////////////////

        void WriteSection(string title) {
            log.AppendLine();
            log.AppendLine(new string('=', 80));
            log.AppendLine(title);
            log.AppendLine(new string('=', 80));
        }

        void WriteHeader() {
            log.AppendLine("Kirby & the Amazing Mirror Randomizer - Spoiler Log");
            log.AppendLine($"Generated: {DateTime.Now:yyyy-MM-dd HH:mm:ss}");
            log.AppendLine($"Original ROM: {System.ROMFilename} {RomRegion.Current.Name}");
            log.AppendLine($"Seed: {Seed}");
        }

        // WriteSettings(); Every setting, read from the Settings class so new settings show up on their own;
        void WriteSettings() {
            WriteSection("Settings");

            HashSet<string> ignored = new HashSet<string> { "RandomEntity", "Seed", "MinSeed", "MaxSeed", "SpoilerLog" };
            IEnumerable<MemberInfo> members = typeof(Settings).GetProperties(BindingFlags.Public | BindingFlags.Instance).Cast<MemberInfo>()
                .Concat(typeof(Settings).GetFields(BindingFlags.Public | BindingFlags.Instance));

            foreach (MemberInfo member in members.Where(member => !ignored.Contains(member.Name))) {
                object value = member is PropertyInfo property ? property.GetValue(Settings) : ((FieldInfo)member).GetValue(Settings);
                log.AppendLine($"{member.Name}: {value}");
            }
        }

        //////////////////////////////////////////////////////////////////////////////////////////////

        // Room; A room's area and its doors (object address and destination), as read from a ROM;
        record Room(int ID, int Area, List<(int address, int destination)> Doors);

        static int Read16(byte[] rom, long address) => rom[address] | (rom[address + 1] << 8);

        static long ReadPointer(byte[] rom, long address) {
            if (address < 0 || address + 4 > rom.Length) return -1;

            uint pointer = BitConverter.ToUInt32(rom, (int)address);

            if (pointer < 0x08000000 || pointer >= 0x0A000000) return -1;

            long offset = pointer & 0x01FFFFFF;

            return offset < rom.Length ? offset : -1;
        }

        /* ReadRooms(); Reads every room and its doors. Doors are objects of type 3 in the room's cover data:
         * +4 object size, +8 destination room. Several long rooms may share a short room; The first one is the one the game uses; */
        static Dictionary<int, Room> ReadRooms(byte[] rom) {
            Dictionary<int, Room> rooms = new Dictionary<int, Room>();
            HashSet<int> shortRooms = new HashSet<int>();

            for (int room = 0; room < MaxLongRoom; room++) {
                long parameters = RomRegion.Current.LevelData(LongRoomParameters) + room * LongRoomParameterSize;
                int shortIndex = Read16(rom, parameters + 0x24);

                if (shortIndex >= MaxShortRoom || !shortRooms.Add(shortIndex)) continue;

                long header = ReadPointer(rom, RomRegion.Current.PointerTable(CoverDataTable) + shortIndex * 4);
                long address = header < 0 ? -1 : ReadPointer(rom, header + 4);

                if (address < 0) continue;

                List<(int address, int destination)> doors = new List<(int address, int destination)>();

                for (int i = 0; i < 1024 && address + 12 <= rom.Length; i++) {
                    int size = Read16(rom, address + 4);

                    if (size == 0) break;
                    if (rom[address] == DoorObjectType) doors.Add(((int)address, Read16(rom, address + 8)));

                    address += size;
                }

                rooms[room] = new Room(room, Read16(rom, parameters), doors);
            }

            return rooms;
        }

        static string RoomName(Dictionary<int, Room> rooms, int room) {
            string kind = !rooms.TryGetValue(room, out Room info) ? "" : info.Area == HubArea ? " (Hub)" : info.Area == BossAndGoalArea ? " (Boss / Goal Game)" : "";

            return $"0x{room:X3}{kind}";
        }

        /* WriteMirrors(); Every mirror of every room, with the room it leads to now. A mirror made of several door objects
         * is listed once. Doors are paired by object address, since the randomizer rewrites them in place; */
        void WriteMirrors() {
            WriteSection("Mirrors");

            Dictionary<int, Room> original = ReadRooms(originalROM), randomized = ReadRooms(randomizedROM);
            List<(int from, int to)> bossAndGoalEntrances = new List<(int from, int to)>();
            int changed = 0;

            foreach (Room room in randomized.Values.OrderBy(room => room.ID)) {
                if (room.Doors.Count == 0) continue;

                Dictionary<int, int> vanilla = original.TryGetValue(room.ID, out Room originalRoom)
                    ? originalRoom.Doors.ToDictionary(door => door.address, door => door.destination) : new Dictionary<int, int>();

                var mirrors = room.Doors
                    .Select(door => (destination: door.destination, vanilla: vanilla.TryGetValue(door.address, out int before) ? before : door.destination))
                    .Distinct()
                    .OrderBy(mirror => mirror.vanilla)
                    .ToList();

                log.AppendLine($"Room {RoomName(randomized, room.ID)}:");

                foreach ((int destination, int before) in mirrors) {
                    bool isChanged = destination != before;
                    if (isChanged) changed++;

                    log.AppendLine($"    -> {RoomName(randomized, destination)}" + (isChanged ? $"   (vanilla: 0x{before:X3})" : "   (unchanged)"));

                    if (randomized.TryGetValue(destination, out Room target) && target.Area == BossAndGoalArea) bossAndGoalEntrances.Add((room.ID, destination));
                }
            }

            log.AppendLine();
            log.AppendLine($"Mirrors changed: {changed}");

            WriteSection("Boss & Goal Game Rooms");

            foreach (var group in bossAndGoalEntrances.Distinct().GroupBy(entrance => entrance.to).OrderBy(group => group.Key)) {
                log.AppendLine($"0x{group.Key:X3} is reached from: {string.Join(", ", group.Select(entrance => RoomName(randomized, entrance.from)))}");
            }
        }

        //////////////////////////////////////////////////////////////////////////////////////////////

        static string ObjectName(byte id) => Processing.parameters.TryGetValue(id, out string name) ? name : $"Object 0x{id:X2}";

        // Describe(); Name of an object, with what it holds or gives when that depends on its behavior;
        static string Describe(byte id, byte behavior) {
            switch (id) {
                case 0x80: case 0x81: return $"{ObjectName(id)} [{ChestContents(behavior)}]";
                case 0x65: return $"{ObjectName(id)} #{behavior + 1}";
            }

            // ROM Expansion enemies: the behavior picks which one;
            if (id == KatAMExpansion.NightmareID && Settings.GetInstance()?.NightmareEnemies.Count > 0) return KatAMExpansion.NameOf(behavior);
            if (id == KatAMExpansion.MinibossID && Settings.GetInstance()?.NightmareMinibosses.Count > 0) return KatAMExpansion.MinibossNameOf(behavior);

            if (pedestalAbilities.TryGetValue(id, out string[] abilities)) {
                return behavior < abilities.Length ? $"{ObjectName(id)} [{abilities[behavior]}]" : $"{ObjectName(id)} [0x{behavior:X2}]";
            }

            return ObjectName(id);
        }

        static string ChestContents(byte behavior) {
            if (behavior <= 0x05) return ObjectName((byte)(0x5E + behavior));
            if (chestTreasures.TryGetValue(behavior, out string treasure)) return treasure;
            if (behavior >= 0x10 && behavior <= 0x21) return "Spray Paint";
            if (behavior >= 0x28 && behavior <= 0x2F) return "Sound Player Track";

            return $"0x{behavior:X2}";
        }

        /* WriteEntities(); Lists the entities of a JSON file that changed (or all of them, for chests, items and pedestals,
         * so their contents can be looked up), with their room and position. Debug and vetoed rooms are left out;
         * isBehaviorContent: the behavior is what the object holds (chests, shards, pedestals). For enemies it's only how they act; */
        void WriteEntities(string title, string json, bool listAll, bool isBehaviorContent) {
            WriteSection(title);

            List<Entity> list = new List<Entity>();

            try {
                Utils.DeserializeEntitiesJSON(Utils.JSONToObjects(json), list, this);
            } catch (Exception ex) when (ex is IOException || ex is UnauthorizedAccessException) {
                log.AppendLine($"Couldn't be read: {ex.Message}");
                return;
            }

            int changed = 0;

            list.RemoveAll(Utils.IsVetoedEntityRoom);

            foreach (Entity entity in list.OrderBy(entity => entity.Room).ThenBy(entity => entity.Address)) {
                byte id = randomizedROM[entity.Address + 12], behavior = randomizedROM[entity.Address + 14];
                bool isChanged = id != entity.ID || (isBehaviorContent && behavior != entity.Behavior);

                if (isChanged) changed++;
                if (!isChanged && !listAll) continue;

                string position = $"Room 0x{entity.Room:X3} (X: {entity.X[0] | (entity.X[1] << 8)}, Y: {entity.Y[0] | (entity.Y[1] << 8)})";
                string now = Describe(id, behavior), before = Describe(entity.ID, entity.Behavior);

                log.AppendLine(isChanged ? $"{position}: {before} -> {now}" : $"{position}: {now}   (unchanged)");
            }

            log.AppendLine();
            log.AppendLine($"Changed: {changed} of {list.Count}");
        }
    }
}
