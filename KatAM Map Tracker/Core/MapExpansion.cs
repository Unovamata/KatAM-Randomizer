using System;
using System.Collections.Generic;
using System.Linq;

namespace KatAMMapTracker {
    /* MapExpansion; The ROM Expansion's Nightmare in Dream Land rooms, laid out like the game's area maps (MapVanilla), one map per
     * NiDL level, next to KatAM's map (original or randomized: the randomizer doesn't shuffle NiDL's own doors):
     * • A level (LayoutLevel()): from its way in (its first stage's first room) outwards, each room where the door to it is in the
     *   room before it, as the game's area maps draw rooms where their exits lead (a door on a room's right edge: the next room to
     *   its right; one in its ceiling: above it), one grid step away (MapVanilla's NeighborStep: small rooms 2 tiles, 6 apart), two
     *   past a room long that way; in the free cell that best keeps the directions of every door it shares with the rooms placed;
     *   so a level grows the branches, loops and long corridors its doors make, not rows.
     * • The levels around the NiDL hub, as KatAM's areas around Central Circle: Vegetable Valley top left, then clockwise, right of
     *   KatAM's map. Each level is a cluster of its own (11 + level, as its area), the hub too (19); */
    public static class MapExpansion {
        const int Grid = MapGraphics.TileSize;
        const int Step = 6 * Grid, LevelMargin = 128, BlockMargin = 128, LongRoom = 60;
        // Where each level sits around the hub (column, row of a 3 x 3 grid): clockwise from the top left;
        static readonly (int column, int row)[] LevelSlots = { (0, 0), (1, 0), (2, 0), (2, 1), (2, 2), (1, 2), (0, 2), (0, 1) };
        public const int HubCluster = 19;
        public static int LevelCluster(int level) => 11 + level;
        public static bool IsLevelCluster(int cluster) => cluster >= 11 && cluster < 19;

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
            Dictionary<int, Dictionary<int, (double x, double y)>> levels = nightmare.Where(r => r.NightmareLevel >= 0).GroupBy(r => r.NightmareLevel)
                .ToDictionary(level => level.Key, level => LayoutLevel(level.ToList(), neighbors, SizeOf));

            (double width, double height) Bounds(Dictionary<int, (double x, double y)> centers) =>
                (centers.Max(c => c.Value.x + SizeOf(map.Rooms[c.Key]) / 2), centers.Max(c => c.Value.y + SizeOf(map.Rooms[c.Key]) / 2));

            // The block, right of KatAM's map: a 3 x 3 grid, the hub in the middle, the levels around it clockwise from the top left;
            double blockX = layout.Count == 0 ? 0 : Snap(layout.Values.Max(p => p.X + p.Width) + BlockMargin);
            double blockY = layout.Count == 0 ? 0 : Snap(layout.Values.Min(p => p.Y));
            List<RoomInfo> hubs = nightmare.Where(r => r.IsNightmareHub).ToList();
            double hubSize = hubs.Count == 0 ? 0 : hubs.Max(SizeOf);
            Dictionary<int, (int column, int row)> slots = levels.Keys.ToDictionary(level => level, level => LevelSlots[level % LevelSlots.Length]);
            double[] widths = new double[3], heights = new double[3];
            widths[1] = heights[1] = hubSize;
            foreach ((int level, (int column, int row)) in slots) {
                (double width, double height) = Bounds(levels[level]);
                widths[column] = Math.Max(widths[column], width);
                heights[row] = Math.Max(heights[row], height);
            }
            double ColumnX(int column) => blockX + Enumerable.Range(0, column).Sum(c => widths[c] + LevelMargin);
            double RowY(int row) => blockY + Enumerable.Range(0, row).Sum(r => heights[r] + LevelMargin);

            foreach (RoomInfo hub in hubs) Put(hub, ColumnX(1) + widths[1] / 2, RowY(1) + heights[1] / 2, HubCluster);
            foreach ((int level, (int column, int row)) in slots) {
                (double width, double height) = Bounds(levels[level]);
                // (each level against the hub's side of its cell, so the levels gather around it);
                double x = column == 0 ? ColumnX(0) + widths[0] - width : column == 1 ? ColumnX(1) + (widths[1] - width) / 2 : ColumnX(2);
                double y = row == 0 ? RowY(0) + heights[0] - height : row == 1 ? RowY(1) + (heights[1] - height) / 2 : RowY(2);
                foreach ((int room, (double cx, double cy)) in levels[level]) Put(map.Rooms[room], Snap(x) + cx, Snap(y) + cy, LevelCluster(level));
            }

            void Put(RoomInfo room, double centerX, double centerY, int cluster) {
                double size = SizeOf(room);
                layout[room.Id] = new RoomPlacement(Snap(centerX) - size / 2, Snap(centerY) - size / 2, size, size);
                clusters[room.Id] = cluster;
            }
        }

        static readonly (int x, int y)[] Directions = { (1, 0), (1, 1), (0, 1), (-1, 1), (-1, 0), (-1, -1), (0, -1), (1, -1) };

        /* LayoutLevel(); A level's rooms on a grid of Step cells (centers relative to the level's top left): breadth first from its way
         * in (the lowest stage's first room), in NiDL's order; each room in the free cell around the room that first leads to it (its
         * corridor's) that best keeps the direction its door to it leaves it (Toward()) and the distance (a cell more per LongRoom
         * tiles that room is long that way), straight or 45° (as MapLayout's: the in-game lines' angles), a cell away from rooms it
         * isn't linked to; the other rooms it's linked to (loops) pull it only a little, their lines going around as KatAM's do; */
        static Dictionary<int, (double x, double y)> LayoutLevel(List<RoomInfo> rooms, Dictionary<int, List<int>> neighbors, Func<RoomInfo, double> sizeOf) {
            Dictionary<int, RoomInfo> byId = rooms.ToDictionary(r => r.Id);
            Dictionary<int, (int x, int y)> cells = new Dictionary<int, (int x, int y)>();
            HashSet<(int x, int y)> used = new HashSet<(int x, int y)>();
            List<int> Linked(int room) => neighbors[room].Where(byId.ContainsKey).ToList();
            IEnumerable<RoomInfo> Ordered(IEnumerable<RoomInfo> list) => list.OrderBy(r => r.IsNightmareSwitchRoom ? 1 : 0).ThenBy(r => r.NightmareStage).ThenBy(r => r.NightmareIndex).ThenBy(r => r.Id);

            // (the direction a door from 'from' to 'to' leaves 'from': where it is in the room, from its middle; a door in the room's
            // middle says nothing, then the door back, the other way; snapped to the 8 directions; null without one);
            (int x, int y)? Toward(RoomInfo from, RoomInfo to) {
                (double x, double y)? Where(RoomInfo room, int destination) {
                    List<DoorInfo> doors = room.Doors.Where(d => d.Destination == destination).ToList();
                    if (doors.Count == 0 || room.Width == 0 || room.Height == 0) return null;
                    double x = doors.Average(d => d.X) / (room.Width * 8.0) - 0.5, y = doors.Average(d => d.Y) / (room.Height * 8.0) - 0.5;
                    return Math.Abs(x) < 0.15 && Math.Abs(y) < 0.15 ? null : (x, y);
                }
                (double x, double y)? v = Where(from, to.Id);
                if (v == null && Where(to, from.Id) is (double bx, double by)) v = (-bx, -by);
                if (v is not (double vx, double vy)) return null;
                int k = (int)Math.Round(Math.Atan2(vy, vx) / (Math.PI / 4));
                return Directions[((k % 8) + 8) % 8];
            }
            int Reach(RoomInfo from, (int x, int y) d) => 1 + Math.Min(2, (Math.Abs(d.x) * from.Width + Math.Abs(d.y) * from.Height) / LongRoom);

            List<RoomInfo> order = new List<RoomInfo>();
            Dictionary<int, RoomInfo> parent = new Dictionary<int, RoomInfo>();
            HashSet<int> queued = new HashSet<int>();
            Queue<RoomInfo> queue = new Queue<RoomInfo>();
            while (order.Count < rooms.Count) {
                if (queue.Count == 0) {
                    RoomInfo next = Ordered(rooms.Where(r => !queued.Contains(r.Id))).First();
                    queued.Add(next.Id);
                    queue.Enqueue(next);
                }
                RoomInfo room = queue.Dequeue();
                order.Add(room);
                foreach (RoomInfo next in Ordered(Linked(room.Id).Select(n => byId[n])))
                    if (queued.Add(next.Id)) { parent[next.Id] = room; queue.Enqueue(next); }
            }

            foreach (RoomInfo room in order) {
                if (cells.Count == 0) { cells[room.Id] = (0, 0); used.Add((0, 0)); continue; }
                List<RoomInfo> placed = Linked(room.Id).Where(cells.ContainsKey).Select(n => byId[n]).ToList();
                // (around the room that leads to it first, its corridor's; the other rooms it's linked to, loops, only pull a little);
                RoomInfo from = parent.TryGetValue(room.Id, out RoomInfo first) ? first : null;
                IEnumerable<(int x, int y)> anchors = from != null ? new[] { cells[from.Id] } : placed.Count > 0 ? placed.Select(p => cells[p.Id]) : cells.Values;
                HashSet<(int x, int y)> candidates = new HashSet<(int x, int y)>();
                for (int ring = 1; ring <= 6 || candidates.Count == 0; ring++)
                    foreach ((int ax, int ay) in anchors)
                        foreach ((int dx, int dy) in Directions)
                            if (!used.Contains((ax + dx * ring, ay + dy * ring))) candidates.Add((ax + dx * ring, ay + dy * ring));

                double Cost((int x, int y) cell) {
                    double cost = 0;
                    foreach (RoomInfo p in placed) {
                        (int px, int py) = cells[p.Id];
                        int dx = cell.x - px, dy = cell.y - py, steps = Math.Max(Math.Abs(dx), Math.Abs(dy));
                        bool straight = dx == 0 || dy == 0 || Math.Abs(dx) == Math.Abs(dy);
                        if (p != from) { cost += 0.2 * steps; continue; }
                        cost += steps + (straight ? 0 : 1.5) + (dx != 0 && dy != 0 ? 0.5 : 0);
                        if (Toward(p, room) is (int tx, int ty) && straight) {
                            int turn = Math.Abs(Array.IndexOf(Directions, (Math.Sign(dx), Math.Sign(dy))) - Array.IndexOf(Directions, (tx, ty)));
                            cost += Math.Min(turn, 8 - turn) * 1.2 + Math.Abs(steps - Reach(p, (tx, ty))) * 0.6;
                        }
                    }
                    // (room to breathe, as KatAM's area maps: rooms it isn't linked to are kept a cell or two away, linked ones crowd a little);
                    HashSet<(int x, int y)> mine = placed.Select(p => cells[p.Id]).ToHashSet();
                    int Near((int x, int y) o) => Math.Max(Math.Abs(o.x - cell.x), Math.Abs(o.y - cell.y));
                    int strangers = cells.Values.Count(o => !mine.Contains(o) && Near(o) <= 1), farStrangers = cells.Values.Count(o => !mine.Contains(o) && Near(o) == 2);
                    int friends = mine.Count(o => Near(o) <= 1);
                    return cost + strangers * 2.5 + farStrangers * 0.8 + friends * 0.3;
                }

                (int x, int y) best = candidates.OrderBy(Cost).ThenBy(c => c.y).ThenBy(c => c.x).First();
                cells[room.Id] = best;
                used.Add(best);
            }

            int minX = cells.Values.Min(c => c.x), minY = cells.Values.Min(c => c.y);
            double margin = rooms.Max(sizeOf) / 2;
            return cells.ToDictionary(c => c.Key, c => (margin + (c.Value.x - minX) * Step, margin + (c.Value.y - minY) * Step));
        }

        static double Snap(double value) => Math.Round(value / Grid) * Grid;
    }
}
