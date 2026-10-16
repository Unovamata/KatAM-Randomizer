using System;
using System.Collections.Generic;
using System.Linq;

namespace KatAMMapTracker {
    /* MapExpansion; The ROM Expansion's Nightmare in Dream Land rooms, laid out like the game's area maps (MapVanilla), one map per
     * NiDL level, next to KatAM's map (original or randomized: the randomizer doesn't shuffle NiDL's own doors):
     * • A level: its switch room (a hub square) at the top left, then a row per stage (0-7, as KatAMExpansionRooms.cs records them, see
     *   RoomInfo.NightmareStage), its rooms in the order Kirby goes through them: each at its door distance from the stage's first
     *   room (its lowest room number), the rooms at the same distance stacked, on the game's grid (small rooms 2 tiles, 4 apart).
     * • The levels in two columns of four (Vegetable Valley to Grape Garden, Yogurt Yard to the Fountain of Dreams), under the NiDL
     *   hub, to the right of KatAM's map. Each level is a cluster of its own (11 + level, as its area), the hub too (19);
     * Rooms of an older expanded ROM (no stages) are laid out the same way, the whole level as one stage; */
    public static class MapExpansion {
        const int Grid = MapGraphics.TileSize;
        const int Step = 4 * Grid, StageGap = 2 * Grid, LevelMargin = 64, BlockMargin = 128;
        public const int HubCluster = 19;
        public static int LevelCluster(int level) => 11 + level;

        // Place(); Adds the NiDL rooms of 'map' to 'layout' (KatAM's rooms, placed) and their clusters to 'clusters';
        public static void Place(RomMap map, Dictionary<int, RoomPlacement> layout, Dictionary<int, int> clusters) {
            List<RoomInfo> nightmare = map.Rooms.Values.Where(r => r.IsNightmareRoom).ToList();
            if (nightmare.Count == 0) return;

            Dictionary<int, List<int>> neighbors = nightmare.ToDictionary(r => r.Id, r => new List<int>());
            foreach (RoomInfo room in nightmare) {
                foreach (DoorInfo door in room.Doors) {
                    if (door.Destination == room.Id || !neighbors.ContainsKey(door.Destination)) continue;
                    if (!neighbors[room.Id].Contains(door.Destination)) neighbors[room.Id].Add(door.Destination);
                    if (!neighbors[door.Destination].Contains(room.Id)) neighbors[door.Destination].Add(room.Id);
                }
            }

            double SizeOf(RoomInfo room) => MapGraphics.SizeOf(MapGraphics.ShapeOf(room));

            // Each level on its own canvas: room centers relative to its top left;
            Dictionary<int, Dictionary<int, (double x, double y)>> levels = new Dictionary<int, Dictionary<int, (double x, double y)>>();
            foreach (IGrouping<int, RoomInfo> level in nightmare.Where(r => r.NightmareLevel >= 0).GroupBy(r => r.NightmareLevel)) {
                Dictionary<int, (double x, double y)> centers = levels[level.Key] = new Dictionary<int, (double x, double y)>();
                List<RoomInfo> switchRooms = level.Where(r => r.IsNightmareSwitchRoom).ToList();
                double top = 0, left = 0;

                foreach (RoomInfo room in switchRooms) {
                    centers[room.Id] = (2 * Grid, top + 2 * Grid);
                    top += 4 * Grid + StageGap;
                }
                if (switchRooms.Count > 0) { left = 4 * Grid + Step; top = 0; }

                foreach (IGrouping<int, RoomInfo> stage in level.Where(r => !r.IsNightmareSwitchRoom).GroupBy(r => r.NightmareStage).OrderBy(g => g.Key)) {
                    HashSet<int> members = stage.Select(r => r.Id).ToHashSet();
                    Dictionary<int, int> depth = new Dictionary<int, int>();
                    Queue<int> queue = new Queue<int>();

                    // From the stage's first room; rooms its doors don't reach start again after the deepest;
                    foreach (RoomInfo start in stage.OrderBy(r => r.NightmareIndex).ThenBy(r => r.Id)) {
                        if (depth.ContainsKey(start.Id)) continue;
                        depth[start.Id] = depth.Count == 0 ? 0 : depth.Values.Max() + 1;
                        queue.Enqueue(start.Id);
                        while (queue.Count > 0) {
                            int room = queue.Dequeue();
                            foreach (int next in neighbors[room].Where(members.Contains).OrderBy(n => map.Rooms[n].NightmareIndex).ThenBy(n => n)) {
                                if (depth.ContainsKey(next)) continue;
                                depth[next] = depth[room] + 1;
                                queue.Enqueue(next);
                            }
                        }
                    }

                    int rows = 1;
                    foreach (IGrouping<int, int> column in depth.Keys.GroupBy(r => depth[r])) {
                        int row = 0;
                        foreach (int room in column.OrderBy(r => map.Rooms[r].NightmareIndex).ThenBy(r => r)) {
                            centers[room] = (left + column.Key * Step + Grid, top + row * Step + Grid);
                            row++;
                        }
                        rows = Math.Max(rows, row);
                    }
                    top += rows * Step + StageGap;
                }
            }

            // The block: the hub, then the levels in two columns, right of KatAM's map;
            double blockX = layout.Count == 0 ? 0 : Snap(layout.Values.Max(p => p.X + p.Width) + BlockMargin);
            double blockY = layout.Count == 0 ? 0 : Snap(layout.Values.Min(p => p.Y));
            double y = blockY;

            foreach (RoomInfo hub in nightmare.Where(r => r.IsNightmareHub)) {
                Put(hub, blockX + SizeOf(hub) / 2, y + SizeOf(hub) / 2, HubCluster);
                y += SizeOf(hub) + LevelMargin;
            }

            (double width, double height) Bounds(Dictionary<int, (double x, double y)> centers) =>
                (centers.Max(c => c.Value.x + SizeOf(map.Rooms[c.Key]) / 2), centers.Max(c => c.Value.y + SizeOf(map.Rooms[c.Key]) / 2));

            List<int> order = levels.Keys.OrderBy(l => l).ToList();
            int perColumn = (order.Count + 1) / 2;
            double x = blockX, columnTop = y;
            for (int i = 0; i < order.Count; i++) {
                if (i == perColumn) {
                    x = Snap(order.Take(perColumn).Max(l => Bounds(levels[l]).width) + blockX + LevelMargin);
                    y = columnTop;
                }
                Dictionary<int, (double x, double y)> centers = levels[order[i]];
                foreach ((int room, (double cx, double cy)) in centers) Put(map.Rooms[room], x + cx, y + cy, LevelCluster(order[i]));
                y = Snap(y + Bounds(centers).height + LevelMargin);
            }

            void Put(RoomInfo room, double centerX, double centerY, int cluster) {
                double size = SizeOf(room);
                layout[room.Id] = new RoomPlacement(Snap(centerX) - size / 2, Snap(centerY) - size / 2, size, size);
                clusters[room.Id] = cluster;
            }
        }

        static double Snap(double value) => Math.Round(value / Grid) * Grid;
    }
}
