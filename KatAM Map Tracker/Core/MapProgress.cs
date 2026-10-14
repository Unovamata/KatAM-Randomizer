using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.Json;

namespace KatAMMapTracker {
    /* MapProgress; What the player has discovered so far. Saved next to the ROM as "<rom>.tracker.json",
     * so progress isn't lost when the emulator or the tracker are closed;
     *
     * • A room is discovered once Kirby enters it. Its doors are drawn from then on, as unexplored exits.
     * • A door is discovered once Kirby goes through it. Arriving through a Two-Way mirror also discovers
     *   the door back, since Kirby is standing on it; */
    public sealed class MapProgress {
        public uint RomCrc32 { get; set; }
        public HashSet<int> VisitedRooms { get; set; } = new HashSet<int>();
        public HashSet<string> FoundDoors { get; set; } = new HashSet<string>();
        public int CurrentRoom { get; set; } = -1;

        // The player chose to see the whole map of this seed (spoiler), e.g. to go straight to the bosses;
        public bool RevealAll { get; set; }

        static readonly JsonSerializerOptions JsonOptions = new JsonSerializerOptions { WriteIndented = true };

        public static string PathForRom(string romPath) => romPath + ".tracker.json";

        // Load(); Loads the progress of a ROM. Progress saved for a different ROM (another seed) is discarded;
        public static MapProgress Load(string path, uint romCrc32) {
            try {
                if (File.Exists(path)) {
                    MapProgress progress = JsonSerializer.Deserialize<MapProgress>(File.ReadAllText(path));

                    if (progress != null && progress.RomCrc32 == romCrc32) return progress;
                }
            }
            catch (Exception exception) when (exception is IOException || exception is JsonException) { }

            return new MapProgress { RomCrc32 = romCrc32 };
        }

        public void Save(string path) {
            // Written to a temporary file first, so a crash mid-write can't corrupt the saved progress;
            string temporary = path + ".tmp";
            File.WriteAllText(temporary, JsonSerializer.Serialize(this, JsonOptions));
            File.Move(temporary, path, true);
        }

        public bool IsDoorFound(DoorInfo door) => FoundDoors.Contains(door.Key);

        /* EnterRoom(); Registers that Kirby is now in a room. Returns true if anything new was discovered;
         * previousRoom: Room Kirby came from, or -1 when unknown (first room reported after connecting); */
        public bool EnterRoom(RomMap map, int room, int previousRoom) {
            if (!map.Rooms.ContainsKey(room)) return false;

            bool changed = VisitedRooms.Add(room) || CurrentRoom != room;

            CurrentRoom = room;

            if (previousRoom == room || !map.Rooms.TryGetValue(previousRoom, out RoomInfo from)) return changed;

            // Rooms never point twice to the same room, so the door Kirby went through is the one leading here;
            DoorInfo door = from.Doors.FirstOrDefault(d => d.Destination == room);

            if (door == null) return changed; // Nothing leads there: death, menus...

            changed |= FoundDoors.Add(door.Key);

            if (door.DoorBack != null) changed |= FoundDoors.Add(door.DoorBack.Key);

            return changed;
        }

        public void Reset() {
            VisitedRooms.Clear();
            FoundDoors.Clear();
            CurrentRoom = -1;
            RevealAll = false;
        }
    }
}
