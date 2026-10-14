using System;
using System.Collections.Generic;
using System.Linq;

namespace KatAMMapTracker {
    // RoomPlacement; Where a room square is drawn on the map, in map pixels (in-game map tiles are 8 pixels);
    public readonly record struct RoomPlacement(double X, double Y, double Width, double Height) {
        public double CenterX => X + Width / 2;
        public double CenterY => Y + Height / 2;
    }

    /* MapLayout; Pregenerates where every room sits on the map from the ROM's (randomized) mirror connections,
     * so rooms connected by a mirror are drawn next to each other, regardless of where they were in the original game;
     *
     * 1. Clusters: rooms are grouped in communities of rooms that are densely connected between themselves (Louvain method).
     * 2. Each cluster is laid out on a grid, placing every room next to the rooms it's connected to, in one of the 8 directions
     *    when possible, so most connections are straight 0°, 45° or 90° lines like a metro map.
     * 3. Clusters are placed around Central Circle's cluster, pulled together by the mirrors between them and pushed apart
     *    so they never overlap.
     * Every room center lands on the 8 pixel grid, so the in-game line tiles line up. The layout only depends on the ROM data,
     * so it's the same every time the same ROM is opened; */
    public static class MapLayout {
        // Distance between neighbor rooms in a cluster, in map pixels (5 map tiles);
        const int Cell = 40;
        const int Grid = MapGraphics.TileSize;
        const double ClusterMargin = 48;
        const int ForceIterations = 300, OverlapIterations = 400;

        // First tutorial room (0x323 -> 0x324 -> 0x325);
        const int FirstTutorialRoom = 0x323;

        public static Dictionary<int, RoomPlacement> Generate(RomMap map) => Generate(map, out _);

        public static Dictionary<int, RoomPlacement> Generate(RomMap map, out Dictionary<int, int> clusters) {
            Dictionary<int, List<int>> neighbors = BuildNeighbors(map);

            clusters = FindClusters(neighbors);

            Dictionary<int, List<int>> members = clusters
                .GroupBy(kvp => kvp.Value)
                .ToDictionary(group => group.Key, group => group.Select(kvp => kvp.Key).OrderBy(id => id).ToList());

            // Every cluster is laid out on its own grid first;
            Dictionary<int, Dictionary<int, (int x, int y)>> localCells = members.ToDictionary(
                kvp => kvp.Key, kvp => LayoutCluster(kvp.Value, neighbors, map));

            /* Clusters not connected to Central Circle's (rooms without mirrors, reached through cannons or stars,
             * and closed groups like the tutorial rooms) are lined up under the map instead; */
            HashSet<int> connected = new HashSet<int>();
            Queue<int> pending = new Queue<int>();
            int rootCluster = clusters.TryGetValue(RomMap.CentralCircle, out int hub) ? hub : members.Keys.Min();

            connected.Add(rootCluster);
            pending.Enqueue(rootCluster);

            while (pending.Count > 0) {
                foreach (int room in members[pending.Dequeue()]) {
                    foreach (int neighbor in neighbors[room]) {
                        if (connected.Add(clusters[neighbor])) pending.Enqueue(clusters[neighbor]);
                    }
                }
            }

            HashSet<int> isolated = new HashSet<int>(members.Keys.Where(c => !connected.Contains(c)));

            Dictionary<int, (double x, double y)> clusterOffsets = PlaceClusters(map,
                members.Where(kvp => !isolated.Contains(kvp.Key)).ToDictionary(kvp => kvp.Key, kvp => kvp.Value),
                localCells, neighbors, clusters);

            /* The tutorial rooms are where a new game starts, so they sit right above the map, over Central Circle,
             * where players can see the tracker reacting from the very beginning; */
            int? tutorial = clusters.TryGetValue(FirstTutorialRoom, out int tutorialCluster) && isolated.Contains(tutorialCluster)
                ? tutorialCluster : null;

            if (tutorial != null) {
                isolated.Remove(tutorial.Value);
                PlaceAbove(tutorial.Value, clusterOffsets, localCells, rootCluster, clusters);
            }

            PlaceIsolated(isolated.OrderBy(c => members[c][0]).ToList(), clusterOffsets, localCells);

            Dictionary<int, RoomPlacement> placements = new Dictionary<int, RoomPlacement>();

            foreach ((int cluster, Dictionary<int, (int x, int y)> cells) in localCells) {
                (double offsetX, double offsetY) = clusterOffsets[cluster];

                foreach ((int room, (int x, int y)) in cells) {
                    double size = MapGraphics.SizeOf(MapGraphics.ShapeOf(map.Rooms[room]));
                    double centerX = offsetX + x * Cell, centerY = offsetY + y * Cell;
                    placements[room] = new RoomPlacement(centerX - size / 2, centerY - size / 2, size, size);
                }
            }

            return placements;
        }

        static Dictionary<int, List<int>> BuildNeighbors(RomMap map) {
            Dictionary<int, List<int>> neighbors = map.Rooms.Keys.ToDictionary(id => id, id => new List<int>());

            foreach (RoomInfo room in map.Rooms.Values) {
                foreach (DoorInfo door in room.Doors.OrderBy(d => d.X).ThenBy(d => d.Y)) {
                    if (door.Destination == room.Id || !map.Rooms.ContainsKey(door.Destination)) continue;

                    if (!neighbors[room.Id].Contains(door.Destination)) neighbors[room.Id].Add(door.Destination);
                    if (!neighbors[door.Destination].Contains(room.Id)) neighbors[door.Destination].Add(room.Id);
                }
            }

            return neighbors;
        }

        //////////////////////////////////////////////////////////////////////////////////////////////

        /* FindClusters(); Louvain community detection (local moving phase), visiting rooms in ID order so it's deterministic;
         * Very small clusters are then merged into the neighbor cluster they share the most mirrors with; */
        static Dictionary<int, int> FindClusters(Dictionary<int, List<int>> neighbors) {
            List<int> rooms = neighbors.Keys.OrderBy(id => id).ToList();
            Dictionary<int, int> cluster = rooms.ToDictionary(id => id, id => id);
            Dictionary<int, double> clusterDegree = rooms.ToDictionary(id => id, id => (double)neighbors[id].Count);
            double totalEdges = rooms.Sum(id => neighbors[id].Count) / 2.0;

            if (totalEdges == 0) return cluster;

            for (int pass = 0; pass < 20; pass++) {
                bool moved = false;

                foreach (int room in rooms) {
                    int current = cluster[room];
                    double degree = neighbors[room].Count;

                    if (degree == 0) continue;

                    Dictionary<int, int> linksToCluster = new Dictionary<int, int>();

                    foreach (int neighbor in neighbors[room]) {
                        int c = cluster[neighbor];
                        linksToCluster[c] = linksToCluster.GetValueOrDefault(c) + 1;
                    }

                    clusterDegree[current] -= degree;

                    int best = current;
                    double bestGain = Gain(linksToCluster.GetValueOrDefault(current), clusterDegree[current], degree, totalEdges);

                    foreach ((int candidate, int links) in linksToCluster.OrderBy(kvp => kvp.Key)) {
                        double gain = Gain(links, clusterDegree[candidate], degree, totalEdges);

                        if (gain > bestGain + 1e-12) {
                            bestGain = gain;
                            best = candidate;
                        }
                    }

                    clusterDegree[best] += degree;

                    if (best != current) {
                        cluster[room] = best;
                        moved = true;
                    }
                }

                if (!moved) break;
            }

            MergeSmallClusters(cluster, neighbors);

            return cluster;
        }

        // Gain(); Modularity gain of moving a room with the given degree into a cluster it has 'links' mirrors to;
        static double Gain(int links, double clusterDegree, double degree, double totalEdges) {
            return links / totalEdges - clusterDegree * degree / (2 * totalEdges * totalEdges);
        }

        static void MergeSmallClusters(Dictionary<int, int> cluster, Dictionary<int, List<int>> neighbors) {
            const int MinimumSize = 3;

            for (int pass = 0; pass < 5; pass++) {
                Dictionary<int, int> sizes = cluster.Values.GroupBy(c => c).ToDictionary(g => g.Key, g => g.Count());
                bool merged = false;

                foreach (int c in sizes.Where(kvp => kvp.Value < MinimumSize).Select(kvp => kvp.Key).OrderBy(c => c).ToList()) {
                    List<int> rooms = cluster.Where(kvp => kvp.Value == c).Select(kvp => kvp.Key).ToList();

                    int target = rooms
                        .SelectMany(room => neighbors[room])
                        .Select(neighbor => cluster[neighbor])
                        .Where(other => other != c)
                        .GroupBy(other => other)
                        .OrderByDescending(g => g.Count()).ThenBy(g => g.Key)
                        .Select(g => g.Key)
                        .DefaultIfEmpty(c)
                        .First();

                    if (target == c) continue;

                    foreach (int room in rooms) cluster[room] = target;
                    merged = true;
                }

                if (!merged) break;
            }
        }

        //////////////////////////////////////////////////////////////////////////////////////////////

        static readonly (int x, int y)[] Directions = {
            (1, 0), (0, 1), (-1, 0), (0, -1), (1, 1), (-1, 1), (-1, -1), (1, -1),
        };

        /* LayoutCluster(); Places the rooms of a cluster on a grid of cells, one room per cell;
         * Rooms are placed in breadth-first order from the most connected one, each in the free cell that best
         * lines up with the rooms it's connected to: straight or 45° neighbors are cheap, anything else costs more; */
        static Dictionary<int, (int x, int y)> LayoutCluster(List<int> rooms, Dictionary<int, List<int>> neighbors, RomMap map) {
            HashSet<int> inCluster = new HashSet<int>(rooms);
            Dictionary<int, (int x, int y)> cells = new Dictionary<int, (int x, int y)>();
            HashSet<(int x, int y)> used = new HashSet<(int x, int y)>();

            int Links(int room) => neighbors[room].Count(inCluster.Contains);

            int start = rooms.Contains(RomMap.CentralCircle) ? RomMap.CentralCircle
                : rooms.OrderByDescending(Links).ThenBy(id => id).First();

            List<int> order = new List<int>();
            HashSet<int> queued = new HashSet<int> { start };
            Queue<int> queue = new Queue<int>();
            queue.Enqueue(start);

            while (order.Count < rooms.Count) {
                if (queue.Count == 0) {
                    // Rooms of the cluster not connected to the rest of it (rare);
                    int next = rooms.First(r => !queued.Contains(r));
                    queued.Add(next);
                    queue.Enqueue(next);
                }

                int room = queue.Dequeue();
                order.Add(room);

                foreach (int neighbor in neighbors[room].Where(inCluster.Contains).OrderByDescending(Links).ThenBy(id => id)) {
                    if (queued.Add(neighbor)) queue.Enqueue(neighbor);
                }
            }

            foreach (int room in order) {
                List<(int x, int y)> placedNeighbors = neighbors[room].Where(cells.ContainsKey).Select(n => cells[n]).ToList();

                if (cells.Count == 0) {
                    cells[room] = (0, 0);
                    used.Add((0, 0));
                    continue;
                }

                // Candidate cells around the placed neighbors (or around the cluster when it has none placed yet);
                IEnumerable<(int x, int y)> anchors = placedNeighbors.Count > 0 ? placedNeighbors : cells.Values;
                HashSet<(int x, int y)> candidates = new HashSet<(int x, int y)>();

                for (int ring = 1; candidates.Count == 0 && ring <= 6; ring++) {
                    foreach ((int ax, int ay) in anchors) {
                        foreach ((int dx, int dy) in Directions) {
                            (int x, int y) cell = (ax + dx * ring, ay + dy * ring);
                            if (!used.Contains(cell)) candidates.Add(cell);
                        }
                    }
                }

                if (candidates.Count == 0) {
                    // Fallback: spiral out from the origin;
                    for (int r = 1; candidates.Count == 0; r++) {
                        for (int x = -r; x <= r; x++) for (int y = -r; y <= r; y++) if (!used.Contains((x, y))) candidates.Add((x, y));
                    }
                }

                (int x, int y) best = candidates
                    .OrderBy(cell => CellCost(cell, placedNeighbors, cells.Values))
                    .ThenBy(cell => Math.Abs(cell.x) + Math.Abs(cell.y))
                    .ThenBy(cell => cell.y).ThenBy(cell => cell.x)
                    .First();

                cells[room] = best;
                used.Add(best);
            }

            return cells;
        }

        // CellCost(); Lower is better: short connections in the 8 directions, and staying compact;
        static double CellCost((int x, int y) cell, List<(int x, int y)> neighbors, IEnumerable<(int x, int y)> all) {
            double cost = 0;

            foreach ((int nx, int ny) in neighbors) {
                int dx = Math.Abs(cell.x - nx), dy = Math.Abs(cell.y - ny);
                bool octilinear = dx == 0 || dy == 0 || dx == dy;

                cost += Math.Max(dx, dy) + (octilinear ? 0 : 1.5);
            }

            // Crowded cells (many rooms around) make the lines harder to follow;
            List<(int x, int y)> placed = all.ToList();
            int crowd = placed.Count(o => Math.Abs(o.x - cell.x) <= 1 && Math.Abs(o.y - cell.y) <= 1);

            // Staying close to the middle of the cluster keeps it compact instead of a long chain;
            double centerX = placed.Average(o => o.x), centerY = placed.Average(o => o.y);
            double spread = Math.Sqrt((cell.x - centerX) * (cell.x - centerX) + (cell.y - centerY) * (cell.y - centerY));

            return cost + crowd * 0.15 + spread * 0.35;
        }

        // PlaceAbove(); Places a cluster above the rest of the map, centered over Central Circle;
        static void PlaceAbove(int cluster, Dictionary<int, (double x, double y)> offsets,
                               Dictionary<int, Dictionary<int, (int x, int y)>> localCells, int rootCluster, Dictionary<int, int> clusterOf) {
            double minY = double.MaxValue;

            foreach ((int other, (double x, double y)) in offsets) {
                foreach ((int cx, int cy) in localCells[other].Values) minY = Math.Min(minY, y + cy * Cell);
            }

            if (minY == double.MaxValue) minY = 0;

            (double rootX, double rootY) = offsets.GetValueOrDefault(rootCluster);
            (int hubX, int hubY) = localCells.TryGetValue(rootCluster, out var rootCells) && rootCells.TryGetValue(RomMap.CentralCircle, out var hubCell)
                ? hubCell : (0, 0);

            IEnumerable<(int x, int y)> cells = localCells[cluster].Values;
            int left = cells.Min(c => c.x), right = cells.Max(c => c.x), bottom = cells.Max(c => c.y);
            double centerX = rootX + hubX * Cell;

            offsets[cluster] = (Math.Round((centerX - (left + right) / 2.0 * Cell) / Grid) * Grid,
                                Math.Round((minY - Cell * 2 - bottom * Cell) / Grid) * Grid);
        }

        // PlaceIsolated(); Lines up the rooms without mirrors in rows under the rest of the map;
        static void PlaceIsolated(List<int> isolated, Dictionary<int, (double x, double y)> offsets,
                                  Dictionary<int, Dictionary<int, (int x, int y)>> localCells) {
            if (isolated.Count == 0) return;

            double minX = 0, maxX = 0, maxY = 0;

            foreach ((int cluster, (double x, double y)) in offsets) {
                foreach ((int cx, int cy) in localCells[cluster].Values) {
                    minX = Math.Min(minX, x + cx * Cell); maxX = Math.Max(maxX, x + cx * Cell); maxY = Math.Max(maxY, y + cy * Cell);
                }
            }

            double cursorX = minX, cursorY = maxY + Cell * 2, rowHeight = 0;

            foreach (int cluster in isolated) {
                IEnumerable<(int x, int y)> cells = localCells[cluster].Values;
                int left = cells.Min(c => c.x), right = cells.Max(c => c.x), top = cells.Min(c => c.y), bottom = cells.Max(c => c.y);

                if (cursorX > minX && cursorX + (right - left) * Cell > maxX) {
                    cursorX = minX;
                    cursorY += rowHeight + Cell;
                    rowHeight = 0;
                }

                offsets[cluster] = (Math.Round((cursorX - left * Cell) / Grid) * Grid, Math.Round((cursorY - top * Cell) / Grid) * Grid);
                cursorX += (right - left + 1) * Cell;
                rowHeight = Math.Max(rowHeight, (bottom - top) * Cell);
            }
        }

        //////////////////////////////////////////////////////////////////////////////////////////////

        /* PlaceClusters(); Positions every cluster (offset of its local grid). Clusters connected by mirrors attract each other,
         * all clusters repel, and a final pass pushes apart overlapping cluster boxes. Central Circle's cluster stays at 0,0; */
        static Dictionary<int, (double x, double y)> PlaceClusters(RomMap map, Dictionary<int, List<int>> members,
                                                                  Dictionary<int, Dictionary<int, (int x, int y)>> localCells,
                                                                  Dictionary<int, List<int>> neighbors, Dictionary<int, int> clusterOf) {
            List<int> ids = members.Keys.OrderBy(c => c).ToList();
            int root = clusterOf.TryGetValue(RomMap.CentralCircle, out int hubCluster) ? hubCluster : ids[0];

            ids.Remove(root);
            ids.Insert(0, root);

            int count = ids.Count;
            Dictionary<int, int> index = new Dictionary<int, int>();
            for (int i = 0; i < count; i++) index[ids[i]] = i;

            // Bounding box of every cluster, relative to its local origin;
            double[] minX = new double[count], minY = new double[count], maxX = new double[count], maxY = new double[count];

            for (int i = 0; i < count; i++) {
                IEnumerable<(int x, int y)> cells = localCells[ids[i]].Values;
                minX[i] = cells.Min(c => c.x) * Cell - Cell / 2.0; maxX[i] = cells.Max(c => c.x) * Cell + Cell / 2.0;
                minY[i] = cells.Min(c => c.y) * Cell - Cell / 2.0; maxY[i] = cells.Max(c => c.y) * Cell + Cell / 2.0;
            }

            double Radius(int i) => Math.Sqrt((maxX[i] - minX[i]) * (maxX[i] - minX[i]) + (maxY[i] - minY[i]) * (maxY[i] - minY[i])) / 2;

            // Mirrors between clusters;
            Dictionary<(int, int), int> weights = new Dictionary<(int, int), int>();

            foreach ((int room, List<int> list) in neighbors) {
                if (!index.TryGetValue(clusterOf[room], out int a)) continue;

                foreach (int other in list) {
                    if (index.TryGetValue(clusterOf[other], out int b) && a < b) weights[(a, b)] = weights.GetValueOrDefault((a, b)) + 1;
                }
            }

            // Initial positions: breadth-first rings around the root cluster;
            double[] x = new double[count], y = new double[count];
            Dictionary<int, int> depth = new Dictionary<int, int> { [0] = 0 };
            List<int> bfs = new List<int> { 0 };

            for (int k = 0; k < bfs.Count; k++) {
                foreach (((int a, int b), int w) in weights) {
                    int other = a == bfs[k] ? b : b == bfs[k] ? a : -1;
                    if (other >= 0 && !depth.ContainsKey(other)) { depth[other] = depth[bfs[k]] + 1; bfs.Add(other); }
                }
            }

            for (int i = 0; i < count; i++) if (!depth.ContainsKey(i)) { depth[i] = 1 + depth.Values.Max(); bfs.Add(i); }

            Dictionary<int, int> ringCount = new Dictionary<int, int>(), ringIndex = new Dictionary<int, int>();
            foreach (int i in bfs) ringCount[depth[i]] = ringCount.GetValueOrDefault(depth[i]) + 1;

            foreach (int i in bfs) {
                int d = depth[i];
                int k = ringIndex.GetValueOrDefault(d);
                ringIndex[d] = k + 1;
                double angle = 2 * Math.PI * k / ringCount[d] + d * 0.7;
                x[i] = Math.Cos(angle) * d * 260;
                y[i] = Math.Sin(angle) * d * 260;
            }

            // Forces between cluster centers;
            double temperature = 200;

            for (int iteration = 0; iteration < ForceIterations; iteration++) {
                double[] fx = new double[count], fy = new double[count];

                for (int i = 0; i < count; i++) {
                    for (int j = i + 1; j < count; j++) {
                        double dx = x[i] - x[j], dy = y[i] - y[j];
                        double distance = Math.Max(Math.Sqrt(dx * dx + dy * dy), 1);
                        double ideal = Radius(i) + Radius(j) + ClusterMargin;
                        double force = ideal * ideal / distance;

                        fx[i] += dx / distance * force; fy[i] += dy / distance * force;
                        fx[j] -= dx / distance * force; fy[j] -= dy / distance * force;
                    }
                }

                foreach (((int a, int b), int w) in weights) {
                    double dx = x[b] - x[a], dy = y[b] - y[a];
                    double distance = Math.Max(Math.Sqrt(dx * dx + dy * dy), 1);
                    double ideal = Radius(a) + Radius(b) + ClusterMargin;
                    double force = distance * distance / ideal * (0.5 + 0.5 * Math.Min(w, 4));

                    fx[a] += dx / distance * force; fy[a] += dy / distance * force;
                    fx[b] -= dx / distance * force; fy[b] -= dy / distance * force;
                }

                for (int i = 1; i < count; i++) {
                    // A little gravity keeps unconnected clusters from drifting away;
                    fx[i] -= x[i] * 0.02; fy[i] -= y[i] * 0.02;

                    double length = Math.Sqrt(fx[i] * fx[i] + fy[i] * fy[i]);
                    if (length < 1e-9) continue;

                    double move = Math.Min(length, temperature);
                    x[i] += fx[i] / length * move;
                    y[i] += fy[i] / length * move;
                }

                temperature = Math.Max(temperature * 0.98, 2);
            }

            // Convert centers to offsets of the local grids, then push overlapping cluster boxes apart;
            double[] offsetX = new double[count], offsetY = new double[count];

            for (int i = 0; i < count; i++) {
                offsetX[i] = x[i] - (minX[i] + maxX[i]) / 2;
                offsetY[i] = y[i] - (minY[i] + maxY[i]) / 2;
            }

            offsetX[0] = 0; offsetY[0] = 0;

            for (int iteration = 0; iteration < OverlapIterations; iteration++) {
                bool moved = false;

                for (int i = 0; i < count; i++) {
                    for (int j = i + 1; j < count; j++) {
                        double overlapX = Math.Min(offsetX[i] + maxX[i], offsetX[j] + maxX[j]) - Math.Max(offsetX[i] + minX[i], offsetX[j] + minX[j]) + ClusterMargin,
                               overlapY = Math.Min(offsetY[i] + maxY[i], offsetY[j] + maxY[j]) - Math.Max(offsetY[i] + minY[i], offsetY[j] + minY[j]) + ClusterMargin;

                        if (overlapX <= 0 || overlapY <= 0) continue;

                        moved = true;

                        double centerXi = offsetX[i] + (minX[i] + maxX[i]) / 2, centerXj = offsetX[j] + (minX[j] + maxX[j]) / 2,
                               centerYi = offsetY[i] + (minY[i] + maxY[i]) / 2, centerYj = offsetY[j] + (minY[j] + maxY[j]) / 2;

                        double shareI = i == 0 ? 0 : 0.5, shareJ = i == 0 ? 1 : 0.5;

                        if (overlapX < overlapY) {
                            double push = (centerXi < centerXj ? -1 : 1) * overlapX;
                            offsetX[i] += push * shareI; offsetX[j] -= push * shareJ;
                        }
                        else {
                            double push = (centerYi < centerYj ? -1 : 1) * overlapY;
                            offsetY[i] += push * shareI; offsetY[j] -= push * shareJ;
                        }
                    }
                }

                if (!moved) break;
            }

            // Offsets land on the 8 pixel grid (cells are multiples of 8 too), so every room center does;
            Dictionary<int, (double x, double y)> offsets = new Dictionary<int, (double x, double y)>();

            for (int i = 0; i < count; i++) {
                offsets[ids[i]] = (Math.Round(offsetX[i] / Grid) * Grid, Math.Round(offsetY[i] / Grid) * Grid);
            }

            return offsets;
        }
    }
}
