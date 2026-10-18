using System;
using System.Collections.Generic;
using System.Linq;

namespace KatAMMapTracker {
    /* MapExpansion; The ROM Expansion's Nightmare in Dream Land rooms, laid out like the game's area maps (MapVanilla), one map per
     * NiDL level, next to KatAM's map (original or randomized: the randomizer doesn't shuffle NiDL's own doors):
     * • A level (LayoutLevel()): from its way in (its first stage's first room) outwards, each room where the door to it is in the
     *   room before it, as the game's area maps draw rooms where their exits lead (a door on a room's right edge: the next room to
     *   its right; one in its ceiling: above it), as far as KatAM's linked rooms sit on its own maps (a step of a 6 tile grid: 4
     *   tiles between two small rooms, KatAM's most common gap), two steps past a room long that way; in the free cell that best
     *   keeps the directions of every door it shares with the rooms placed, never nearer another room than KatAM's rooms ever are
     *   (MinGap: 3 tiles between their edges); so a level grows the branches, loops and long corridors its doors make, not rows,
     *   spaced as KatAM's.
     * • The levels packed tight around KatAM's map (Pack()): each in the free spot that keeps the whole map the most square, then the
     *   closest to its middle, its rooms RoomMargin clear of every other cluster's, so none is left out on its own; the hub beside
     *   Central Circle. KatAM's original map (its original mirrors) stays exactly the pause screen's, one block the levels go around,
     *   never in its gaps; a randomized one's clusters leave gaps the levels fill. A room linked to none of its level's rooms (only to other levels': a swapped door, a switch room's) sits by
     *   its stage's rooms, else by the level's way in, so a stage's map stays together.
     *   Each level is a cluster of its own (11 + level, as its area), the hub too (19).
     * • KatAM's rooms the map lines up under itself (the tutorial and the game's end: rooms no mirror links to Central Circle's
     *   map) go after the levels, so the map still ends with them; */
    public static class MapExpansion {
        const int Grid = MapGraphics.TileSize;
        const int Step = 6 * Grid, MinGap = 3 * Grid, LevelMargin = 128, LongRoom = 60, RoomMargin = 72, Fit = 2 * Grid;
        public const int HubCluster = 19;
        public static int LevelCluster(int level) => 11 + level;
        public static bool IsLevelCluster(int cluster) => cluster >= 11 && cluster < 19;

        // Place(); Adds the NiDL rooms of 'map' to 'layout' (KatAM's rooms, placed) and their clusters to 'clusters';
        public static void Place(RomMap map, Dictionary<int, RoomPlacement> layout, Dictionary<int, int> clusters, bool original = false) {
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

            /* Each level on its own canvas: room centers relative to its top left; beside KatAM's original map, laid out as its area maps
             * are (LayoutLevel()); on a randomized map, a cluster as compact as KatAM's clusters there (ClusterLevel()), and packed as
             * close to the others as they are to each other (ClusterMargin); */
            Dictionary<int, Dictionary<int, (double x, double y)>> levels = nightmare.Where(r => r.NightmareLevel >= 0).GroupBy(r => r.NightmareLevel)
                .ToDictionary(level => level.Key, level => original ? LayoutLevel(level.ToList(), neighbors, SizeOf) : ClusterLevel(level.ToList(), neighbors, map, SizeOf));
            double roomMargin = original ? RoomMargin : MapLayout.DefaultClusterMargin;

            (double width, double height) Bounds(Dictionary<int, (double x, double y)> centers) =>
                (centers.Max(c => c.Value.x + SizeOf(map.Rooms[c.Key]) / 2), centers.Max(c => c.Value.y + SizeOf(map.Rooms[c.Key]) / 2));

            // KatAM's map, without the rooms lined up under it (UnderKatAM(): not linked to Central Circle's map);
            HashSet<int> under = UnderKatAM(map, layout);
            List<RoomPlacement> katam = layout.Where(p => !under.Contains(p.Key)).Select(p => p.Value).ToList();
            (double left, double top, double right, double bottom) main = katam.Count == 0 ? (0, 0, 0, 0)
                : (katam.Min(p => p.X), katam.Min(p => p.Y), katam.Max(p => p.X + p.Width), katam.Max(p => p.Y + p.Height));

            // The hub beside Central Circle, clear of KatAM's areas;
            List<(double left, double top, double right, double bottom)> taken = clusters.Where(c => layout.ContainsKey(c.Key) && !under.Contains(c.Key)).GroupBy(c => c.Value)
                .Select(area => (area.Min(c => layout[c.Key].X), area.Min(c => layout[c.Key].Y), area.Max(c => layout[c.Key].X + layout[c.Key].Width), area.Max(c => layout[c.Key].Y + layout[c.Key].Height)))
                .ToList();
            foreach (RoomInfo hub in nightmare.Where(r => r.IsNightmareHub)) {
                double size = SizeOf(hub);
                (double x, double y) anchor = layout.TryGetValue(RomMap.CentralCircle, out RoomPlacement circle) ? (circle.CenterX, circle.CenterY)
                    : ((main.left + main.right) / 2, (main.top + main.bottom) / 2);
                (double left, double top) best = (anchor.x - size / 2, anchor.y - size / 2);
                bool Free(double left, double top) => taken.All(t => left + size + LevelMargin <= t.left || left >= t.right + LevelMargin || top + size + LevelMargin <= t.top || top >= t.bottom + LevelMargin);
                for (int ring = 0; ring < 400 && !Free(best.left, best.top); ring++) {
                    foreach ((int dx, int dy) in Enumerable.Range(-ring, 2 * ring + 1).SelectMany(k => new[] { (k, -ring), (k, ring), (-ring, k), (ring, k) })
                                                     .OrderBy(d => Math.Abs(d.Item1) + Math.Abs(d.Item2))) {
                        (double left, double top) spot = (Snap(anchor.x - size / 2 + dx * 2 * Grid), Snap(anchor.y - size / 2 + dy * 2 * Grid));
                        if (Free(spot.left, spot.top)) { best = spot; break; }
                    }
                }
                taken.Add((best.left, best.top, best.left + size, best.top + size));
                Put(hub, best.left + size / 2, best.top + size / 2, HubCluster);
                main = (Math.Min(main.left, best.left), Math.Min(main.top, best.top), Math.Max(main.right, best.left + size), Math.Max(main.bottom, best.top + size));
            }

            /* The levels packed tight around KatAM's map, the whole map kept square (as KatAM's world map: no long strip to scroll, no
             * level left out on its own, none of the gaps between clusters left empty): each level goes in the free spot (each of its
             * rooms RoomMargin clear of every room placed, of KatAM's and the levels before it, on a Fit grid) that keeps the square
             * around it all the smallest, then the closest to the map's middle; the levels in their order and the biggest first are both
             * tried, the tighter map kept. KatAM's rooms lined up under its map come last, under it all (Under), so the map still ends
             * with them; */
            const int Under = -1;
            (double left, double top, double right, double bottom) underBox = under.Count == 0 ? default
                : (under.Min(r => layout[r].X), under.Min(r => layout[r].Y), under.Max(r => layout[r].X + layout[r].Width), under.Max(r => layout[r].Y + layout[r].Height));
            List<RoomPlacement> katamRooms = layout.Where(p => !under.Contains(p.Key)).Select(p => p.Value).ToList();
            List<(int level, double left, double top)> Pack(IEnumerable<int> order, out double score) {
                // (the cells of a Fit grid no room of a level may stand in: within RoomMargin of a room placed);
                double reach = roomMargin + nightmare.Max(SizeOf) / 2;
                double originX = main.left - 4096, originY = main.top - 4096;
                HashSet<(int, int)> blocked = new HashSet<(int, int)>();
                void Block(RoomPlacement p) {
                    for (int cx = (int)Math.Floor((p.X - reach - originX) / Fit); cx <= (int)Math.Ceiling((p.X + p.Width + reach - originX) / Fit); cx++)
                        for (int cy = (int)Math.Floor((p.Y - reach - originY) / Fit); cy <= (int)Math.Ceiling((p.Y + p.Height + reach - originY) / Fit); cy++)
                            blocked.Add((cx, cy));
                }
                // (KatAM's original map is one block: the levels go around it, never in its gaps; a randomized one's rooms each);
                if (original) Block(new RoomPlacement(main.left, main.top, main.right - main.left, main.bottom - main.top));
                else foreach (RoomPlacement p in katamRooms) Block(p);
                List<(int level, double left, double top)> spots = new List<(int, double, double)>();
                (double left, double top, double right, double bottom) all = main;
                double middleX = (main.left + main.right) / 2, middleY = (main.top + main.bottom) / 2;
                double Square(double width, double height) => Math.Max(width, height) + Math.Abs(width - height) / 2;
                foreach (int level in order) {
                    (double width, double height) = Bounds(levels[level]);
                    List<(double x, double y)> centers = levels[level].Values.ToList();
                    bool Free(double left, double top) => centers.All(c => !blocked.Contains(((int)Math.Round((left + c.x - originX) / Fit), (int)Math.Round((top + c.y - originY) / Fit))));
                    (double left, double top) best = (Snap(all.right + LevelMargin), all.top);
                    (double size, double distance) bestScore = (double.MaxValue, double.MaxValue);
                    for (double x = Snap(all.left - width - LevelMargin); x <= all.right + LevelMargin; x += Fit)
                        for (double y = Snap(all.top - height - LevelMargin); y <= all.bottom + LevelMargin; y += Fit) {
                            double size = Square(Math.Max(all.right, x + width) - Math.Min(all.left, x), Math.Max(all.bottom, y + height) - Math.Min(all.top, y));
                            if (size > bestScore.size + 0.5) continue;
                            double distance = Math.Abs(x + width / 2 - middleX) + Math.Abs(y + height / 2 - middleY);
                            if (Math.Abs(size - bestScore.size) <= 0.5 && distance >= bestScore.distance) continue;
                            if (!Free(x, y)) continue;
                            bestScore = (size, distance); best = (x, y);
                        }
                    spots.Add((level, best.left, best.top));
                    foreach ((double cx, double cy) in centers) { double s = 16; Block(new RoomPlacement(best.left + cx - s / 2, best.top + cy - s / 2, s, s)); }
                    all = (Math.Min(all.left, best.left), Math.Min(all.top, best.top), Math.Max(all.right, best.left + width), Math.Max(all.bottom, best.top + height));
                }
                if (under.Count > 0) {
                    spots.Add((Under, all.left, Snap(all.bottom + LevelMargin)));
                    all.bottom = Snap(all.bottom + LevelMargin) + underBox.bottom - underBox.top;
                    all.right = Math.Max(all.right, all.left + underBox.right - underBox.left);
                }
                score = Square(all.right - all.left, all.bottom - all.top);
                return spots;
            }
            List<int> inOrder = levels.Keys.OrderBy(l => l).ToList();
            List<int> biggestFirst = levels.Keys.OrderByDescending(l => Bounds(levels[l]).width * Bounds(levels[l]).height).ThenBy(l => l).ToList();
            List<(int level, double left, double top)> packed = Pack(biggestFirst, out double biggestScore);
            List<(int level, double left, double top)> ordered = Pack(inOrder, out double orderedScore);
            if (orderedScore <= biggestScore) packed = ordered;
            foreach ((int level, double left, double top) in packed) {
                if (level == Under) {
                    // (KatAM's rooms lined up under its map: the same arrangement, moved there);
                    foreach (int room in under) {
                        RoomPlacement p = layout[room];
                        layout[room] = new RoomPlacement(p.X + left - underBox.left, p.Y + top - underBox.top, p.Width, p.Height);
                    }
                    continue;
                }
                foreach ((int room, (double cx, double cy)) in levels[level]) Put(map.Rooms[room], left + cx, top + cy, LevelCluster(level));
            }

            void Put(RoomInfo room, double centerX, double centerY, int cluster) {
                double size = SizeOf(room);
                layout[room.Id] = new RoomPlacement(Snap(centerX) - size / 2, Snap(centerY) - size / 2, size, size);
                clusters[room.Id] = cluster;
            }
        }

        /* UnderKatAM(); KatAM's rooms its map lines up under itself (MapVanilla's and MapLayout's rows: the tutorial, the game's end...):
         * the ones no door links to Central Circle's map through KatAM's own rooms (NiDL's links left out); */
        static HashSet<int> UnderKatAM(RomMap map, Dictionary<int, RoomPlacement> layout) {
            HashSet<int> katam = layout.Keys.Where(id => map.Rooms.TryGetValue(id, out RoomInfo r) && !r.IsNightmareRoom).ToHashSet();
            if (!katam.Contains(RomMap.CentralCircle)) return new HashSet<int>();
            Dictionary<int, HashSet<int>> links = katam.ToDictionary(id => id, id => new HashSet<int>());
            foreach (int id in katam)
                foreach (DoorInfo door in map.Rooms[id].Doors)
                    if (door.Destination != id && katam.Contains(door.Destination)) { links[id].Add(door.Destination); links[door.Destination].Add(id); }
            HashSet<int> reached = new HashSet<int> { RomMap.CentralCircle };
            Queue<int> queue = new Queue<int>(reached);
            while (queue.Count > 0)
                foreach (int next in links[queue.Dequeue()])
                    if (reached.Add(next)) queue.Enqueue(next);
            return katam.Where(id => !reached.Contains(id)).ToHashSet();
        }

        static readonly (int x, int y)[] Directions = { (1, 0), (1, 1), (0, 1), (-1, 1), (-1, 0), (-1, -1), (0, -1), (1, -1) };

        /* LayoutLevel(); A level's rooms on a grid of Step cells (centers relative to the level's top left): breadth first from its way
         * in (the lowest stage's first room), in NiDL's order; each room in the free cell around the room that first leads to it (its
         * corridor's) that best keeps the direction its door to it leaves it (Toward()) and the distance (a cell, a second past a
         * room LongRoom tiles long that way), straight or 45° (as MapLayout's: the in-game lines' angles), a cell away from rooms it
         * isn't linked to, and never nearer a room than MinGap edge to edge (two big rooms side by side); the other rooms it's linked
         * to (loops) pull it only a little, their lines going around as KatAM's do; */
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
            int Reach(RoomInfo from, (int x, int y) d) => 1 + Math.Min(1, (Math.Abs(d.x) * from.Width + Math.Abs(d.y) * from.Height) / LongRoom);

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
                // (around the room that leads to it first, its corridor's; the other rooms it's linked to, loops, only pull a little; one
                // linked to none of the level's rooms placed, only to other levels' (a swapped door, the switch room's), by its stage's
                // rooms, else the level's way in: kept close, its stage's map together);
                RoomInfo from = parent.TryGetValue(room.Id, out RoomInfo first) ? first : null;
                List<(int x, int y)> kin = placed.Count > 0 || from != null ? new List<(int x, int y)>()
                    : cells.Where(c => !room.IsNightmareSwitchRoom && byId[c.Key].NightmareStage == room.NightmareStage && !byId[c.Key].IsNightmareSwitchRoom).Select(c => c.Value).ToList();
                if (placed.Count == 0 && from == null && kin.Count == 0) kin.Add(cells[order[0].Id]);
                IEnumerable<(int x, int y)> anchors = from != null ? new[] { cells[from.Id] } : placed.Count > 0 ? placed.Select(p => cells[p.Id]) : kin;
                // (a cell is free when it's MinGap clear of every room placed, edge to edge);
                double size = sizeOf(room);
                bool Clear((int x, int y) cell) => !used.Contains(cell) && cells.All(c =>
                    Math.Max(Math.Abs(c.Value.x - cell.x), Math.Abs(c.Value.y - cell.y)) * Step - (size + sizeOf(byId[c.Key])) / 2 >= MinGap);
                HashSet<(int x, int y)> candidates = new HashSet<(int x, int y)>();
                for (int ring = 1; ring <= 8 || candidates.Count == 0; ring++)
                    foreach ((int ax, int ay) in anchors)
                        foreach ((int dx, int dy) in Directions)
                            if (Clear((ax + dx * ring, ay + dy * ring))) candidates.Add((ax + dx * ring, ay + dy * ring));

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
                    if (kin.Count > 0) cost += kin.Min(Near);
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

        // ClusterLevel(); A level's rooms as one of a randomized map's clusters (MapLayout.LayoutCluster(): its grid, compact), centers
        // relative to the level's top left;
        static Dictionary<int, (double x, double y)> ClusterLevel(List<RoomInfo> rooms, Dictionary<int, List<int>> neighbors, RomMap map, Func<RoomInfo, double> sizeOf) {
            Dictionary<int, (int x, int y)> cells = MapLayout.LayoutCluster(rooms.Select(r => r.Id).OrderBy(id => id).ToList(), neighbors, map);
            int minX = cells.Values.Min(c => c.x), minY = cells.Values.Min(c => c.y);
            double margin = rooms.Max(sizeOf) / 2;
            return cells.ToDictionary(c => c.Key, c => (margin + (c.Value.x - minX) * MapLayout.DefaultCell, margin + (c.Value.y - minY) * MapLayout.DefaultCell));
        }

        static double Snap(double value) => Math.Round(value / Grid) * Grid;
    }
}
