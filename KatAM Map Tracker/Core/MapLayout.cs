using System;
using System.Collections.Generic;
using System.Linq;

namespace KatAMMapTracker {
    // RoomPlacement; Where a room is drawn on the map, in map units. A map unit is one thumbnail pixel;
    public readonly record struct RoomPlacement(double X, double Y, double Width, double Height) {
        public double CenterX => X + Width / 2;
        public double CenterY => Y + Height / 2;
    }

    /* MapLayout; Pregenerates where every room sits on the map from the ROM's (randomized) mirror connections,
     * so rooms connected by a mirror are drawn next to each other, regardless of where they were in the original game;
     *
     * • Central Circle is the root. Rooms are spread in rings by how many mirrors away they are from it.
     * • A force pass pulls connected rooms together and pushes the rest apart.
     * • A final pass removes overlaps between room thumbnails.
     * The layout only depends on the ROM data, so it's the same every time the same ROM is opened; */
    public static class MapLayout {
        // Thumbnail pixels per 8x8 tile;
        public const int PixelsPerTile = 4;

        const double RingGap = 350,
                     EdgeLength = 220,
                     RoomMargin = 40;

        const int ForceIterations = 400,
                  OverlapIterations = 300;

        public static Dictionary<int, RoomPlacement> Generate(RomMap map) {
            Dictionary<int, List<int>> neighbors = BuildNeighbors(map);
            Dictionary<int, (double x, double y)> centers = new Dictionary<int, (double x, double y)>();
            Dictionary<int, (double w, double h)> sizes = map.Rooms.ToDictionary(
                kvp => kvp.Key,
                kvp => ((double)kvp.Value.Width * PixelsPerTile, (double)kvp.Value.Height * PixelsPerTile));

            // Main component from Central Circle, then any room the mirrors don't reach (entered through cannons, stars, etc.);
            List<List<int>> components = new List<List<int>>();
            HashSet<int> placed = new HashSet<int>();

            IEnumerable<int> roots = new[] { RomMap.CentralCircle }.Concat(map.Rooms.Keys.OrderBy(id => id));

            foreach (int root in roots) {
                if (!map.Rooms.ContainsKey(root) || placed.Contains(root)) continue;

                List<int> component = PlaceRadial(root, neighbors, placed, centers);
                RelaxForces(component, neighbors, centers);
                components.Add(component);
            }

            Dictionary<int, RoomPlacement> placements = new Dictionary<int, RoomPlacement>();

            foreach (int room in centers.Keys) {
                (double w, double h) = sizes[room];
                placements[room] = new RoomPlacement(centers[room].x - w / 2, centers[room].y - h / 2, w, h);
            }

            foreach (List<int> component in components) RemoveOverlaps(component, placements);

            ArrangeComponents(components, placements);

            return placements;
        }

        static Dictionary<int, List<int>> BuildNeighbors(RomMap map) {
            Dictionary<int, List<int>> neighbors = map.Rooms.Keys.ToDictionary(id => id, id => new List<int>());

            foreach (RoomInfo room in map.Rooms.Values) {
                // Doors are visited left to right, so the layout keeps a bit of the room's own orientation;
                foreach (DoorInfo door in room.Doors.OrderBy(d => d.X).ThenBy(d => d.Y)) {
                    if (door.Destination == room.Id || !map.Rooms.ContainsKey(door.Destination)) continue;

                    if (!neighbors[room.Id].Contains(door.Destination)) neighbors[room.Id].Add(door.Destination);
                    if (!neighbors[door.Destination].Contains(room.Id)) neighbors[door.Destination].Add(room.Id);
                }
            }

            return neighbors;
        }

        // PlaceRadial(); Places a component in rings around its root, giving each subtree an angle proportional to its size;
        static List<int> PlaceRadial(int root, Dictionary<int, List<int>> neighbors, HashSet<int> placed,
                                     Dictionary<int, (double x, double y)> centers) {
            Dictionary<int, List<int>> children = new Dictionary<int, List<int>>();
            Dictionary<int, int> depth = new Dictionary<int, int> { [root] = 0 };
            List<int> order = new List<int> { root };
            Queue<int> queue = new Queue<int>();

            placed.Add(root);
            queue.Enqueue(root);

            while (queue.Count > 0) {
                int room = queue.Dequeue();
                children[room] = new List<int>();

                foreach (int next in neighbors[room]) {
                    if (!placed.Add(next)) continue;

                    depth[next] = depth[room] + 1;
                    children[room].Add(next);
                    order.Add(next);
                    queue.Enqueue(next);
                }
            }

            Dictionary<int, int> leaves = new Dictionary<int, int>();

            for (int i = order.Count - 1; i >= 0; i--) {
                int room = order[i];
                leaves[room] = Math.Max(1, children[room].Sum(child => leaves[child]));
            }

            void Assign(int room, double startAngle, double endAngle) {
                double angle = (startAngle + endAngle) / 2, radius = depth[room] * RingGap;
                centers[room] = (Math.Cos(angle) * radius, Math.Sin(angle) * radius);

                double current = startAngle, span = endAngle - startAngle;

                foreach (int child in children[room]) {
                    double childSpan = span * leaves[child] / leaves[room];
                    Assign(child, current, current + childSpan);
                    current += childSpan;
                }
            }

            Assign(root, 0, Math.PI * 2);

            return order;
        }

        // RelaxForces(); Fruchterman-Reingold style pass: mirrors pull rooms together, every room pushes the others away;
        static void RelaxForces(List<int> component, Dictionary<int, List<int>> neighbors, Dictionary<int, (double x, double y)> centers) {
            if (component.Count < 3) return;

            int count = component.Count;
            Dictionary<int, int> index = new Dictionary<int, int>();
            double[] x = new double[count], y = new double[count];

            for (int i = 0; i < count; i++) {
                index[component[i]] = i;
                (x[i], y[i]) = centers[component[i]];
            }

            double temperature = RingGap;

            for (int iteration = 0; iteration < ForceIterations; iteration++) {
                double[] dx = new double[count], dy = new double[count];

                for (int i = 0; i < count; i++) {
                    for (int j = i + 1; j < count; j++) {
                        double ox = x[i] - x[j], oy = y[i] - y[j];
                        double distanceSquared = Math.Max(ox * ox + oy * oy, 1);
                        double force = EdgeLength * EdgeLength / distanceSquared;

                        dx[i] += ox * force; dy[i] += oy * force;
                        dx[j] -= ox * force; dy[j] -= oy * force;
                    }
                }

                for (int i = 0; i < count; i++) {
                    foreach (int neighbor in neighbors[component[i]]) {
                        int j = index[neighbor];
                        double ox = x[j] - x[i], oy = y[j] - y[i];
                        double distance = Math.Max(Math.Sqrt(ox * ox + oy * oy), 1);
                        double force = distance / EdgeLength;

                        dx[i] += ox * force; dy[i] += oy * force;
                    }
                }

                for (int i = 0; i < count; i++) {
                    // The root stays in place so the map is always centered on Central Circle;
                    if (i == 0) continue;

                    double length = Math.Sqrt(dx[i] * dx[i] + dy[i] * dy[i]);

                    if (length < 1e-9) continue;

                    double move = Math.Min(length, temperature);
                    x[i] += dx[i] / length * move;
                    y[i] += dy[i] / length * move;
                }

                temperature = Math.Max(temperature * 0.97, 5);
            }

            for (int i = 0; i < count; i++) centers[component[i]] = (x[i], y[i]);
        }

        // RemoveOverlaps(); Pushes overlapping room thumbnails apart along the axis where they overlap the least;
        static void RemoveOverlaps(List<int> component, Dictionary<int, RoomPlacement> placements) {
            for (int iteration = 0; iteration < OverlapIterations; iteration++) {
                bool moved = false;

                for (int i = 0; i < component.Count; i++) {
                    for (int j = i + 1; j < component.Count; j++) {
                        RoomPlacement a = placements[component[i]], b = placements[component[j]];

                        double overlapX = Math.Min(a.X + a.Width, b.X + b.Width) - Math.Max(a.X, b.X) + RoomMargin,
                               overlapY = Math.Min(a.Y + a.Height, b.Y + b.Height) - Math.Max(a.Y, b.Y) + RoomMargin;

                        if (overlapX <= 0 || overlapY <= 0) continue;

                        moved = true;

                        double pushX = 0, pushY = 0;

                        if (overlapX < overlapY) pushX = (a.CenterX < b.CenterX ? -1 : 1) * overlapX / 2;
                        else pushY = (a.CenterY < b.CenterY ? -1 : 1) * overlapY / 2;

                        // Central Circle stays in place;
                        double shareA = i == 0 && component[0] == RomMap.CentralCircle ? 0 : 1,
                               shareB = 2 - shareA;

                        placements[component[i]] = a with { X = a.X + pushX * shareA, Y = a.Y + pushY * shareA };
                        placements[component[j]] = b with { X = b.X - pushX * shareB, Y = b.Y - pushY * shareB };
                    }
                }

                if (!moved) break;
            }
        }

        // ArrangeComponents(); Lines up the components that aren't connected to Central Circle under the main one;
        static void ArrangeComponents(List<List<int>> components, Dictionary<int, RoomPlacement> placements) {
            if (components.Count < 2) return;

            (double minX, double minY, double maxX, double maxY) Bounds(List<int> component) => (
                component.Min(r => placements[r].X), component.Min(r => placements[r].Y),
                component.Max(r => placements[r].X + placements[r].Width), component.Max(r => placements[r].Y + placements[r].Height));

            var main = Bounds(components[0]);
            double cursorX = main.minX, cursorY = main.maxY + RingGap, rowHeight = 0;

            foreach (List<int> component in components.Skip(1)) {
                var bounds = Bounds(component);
                double width = bounds.maxX - bounds.minX, height = bounds.maxY - bounds.minY;

                if (cursorX + width > main.maxX && cursorX > main.minX) {
                    cursorX = main.minX;
                    cursorY += rowHeight + RoomMargin * 2;
                    rowHeight = 0;
                }

                foreach (int room in component) {
                    RoomPlacement p = placements[room];
                    placements[room] = p with { X = p.X - bounds.minX + cursorX, Y = p.Y - bounds.minY + cursorY };
                }

                cursorX += width + RoomMargin * 2;
                rowHeight = Math.Max(rowHeight, height);
            }
        }
    }
}
