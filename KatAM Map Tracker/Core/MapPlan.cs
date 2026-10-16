using System;
using System.Collections.Generic;
using System.Linq;

namespace KatAMMapTracker {
    /* MapPlan; The complete map of a ROM: where every room sits and the path of every connection;
     *
     * Connections are routed around the rooms, so no line touches or hides behind a room square:
     * • Paths run on the 8 pixel grid in the 8 directions (0°, 45°, 90°), found with A*.
     * • Every room but the two being connected blocks the cells around it, leaving a gap between lines and squares.
     * • Diagonals cost the same as straight steps per cell, turns cost a little and sharp turns are not allowed,
     *   so lines bend around rooms with 45° turns. Cells already used by other lines cost a bit more, to spread them.
     * • If some connection can't be routed, the rooms and clusters are spread further apart and everything is retried; */
    public sealed class MapPlan {
        public Dictionary<int, RoomPlacement> Layout { get; private set; }
        public Dictionary<int, int> Clusters { get; private set; }

        // Path of every connection, from the room with the lowest ID to the other one, as points in map pixels;
        public Dictionary<string, List<(double x, double y)>> Paths { get; private set; } = new Dictionary<string, List<(double x, double y)>>();

        // Connections that couldn't be routed around the rooms (drawn with a direct route instead);
        public int UnroutedConnections { get; private set; }

        const int Grid = MapGraphics.TileSize;

        // Line half thickness (8) plus a gap, so lines never touch the squares;
        const int Clearance = 16;

        const double TurnCost45 = 0.6, TurnCost90 = 2.5, UsedCellCost = 0.35, OutsideCost = 0.25;
        const int MaxExpansions = 400000;

        static readonly (int x, int y)[] Directions = {
            (1, 0), (1, 1), (0, 1), (-1, 1), (-1, 0), (-1, -1), (0, -1), (1, -1),
        };

        // Whether the rooms sit where the game's own maps draw them (the ROM's mirrors are the original ones, see MapVanilla);
        public bool IsOriginalLayout { get; private set; }

        /* Generate(); Three kinds of map: an original ROM's is the game's (MapVanilla); a randomized one's is laid out from its mirrors
         * (MapLayout); an expanded ROM's (ROM Expansion) is its KatAM's, one of those two, with the Nightmare in Dream Land levels next to
         * it, laid out by stage (MapExpansion). The spacing grows until every line can be routed between the squares; */
        public static MapPlan Generate(RomMap map) {
            bool original = MapVanilla.IsUnmodified(map);
            RomMap katam = map.IsExpanded ? map.Subset(r => !r.IsNightmareRoom) : map;
            List<Func<(Dictionary<int, RoomPlacement> layout, Dictionary<int, int> clusters)>> attempts =
                new List<Func<(Dictionary<int, RoomPlacement>, Dictionary<int, int>)>>();

            if (original) {
                // The game's spacing first, wider when some lines can't be routed between the squares;
                foreach (double scale in new[] { 1.0, 1.5, 2.0, 2.5 })
                    attempts.Add(() => (MapVanilla.Generate(katam, scale, out Dictionary<int, int> areas), areas));
            } else {
                foreach ((int cell, double margin) in new[] {
                    (MapLayout.DefaultCell, MapLayout.DefaultClusterMargin),
                    (MapLayout.DefaultCell, MapLayout.DefaultClusterMargin * 2),
                    (MapLayout.DefaultCell + 8, MapLayout.DefaultClusterMargin * 2),
                    (MapLayout.DefaultCell + 16, MapLayout.DefaultClusterMargin * 3),
                }) attempts.Add(() => (MapLayout.Generate(katam, out Dictionary<int, int> clusters, cell, margin), clusters));
            }

            // The spacing is KatAM's lines' (the few long ones to the NiDL levels would spread KatAM apart for nothing);
            MapPlan best = null;

            foreach (var attempt in attempts) {
                (Dictionary<int, RoomPlacement> layout, Dictionary<int, int> clusters) = attempt();
                MapPlan plan = new MapPlan { Layout = layout, Clusters = clusters, IsOriginalLayout = original };
                plan.RouteAll(katam);

                if (best == null || plan.UnroutedConnections < best.UnroutedConnections) best = plan;
                if (plan.UnroutedConnections == 0) break;
            }

            if (map.IsExpanded) {
                MapExpansion.Place(map, best.Layout, best.Clusters);
                best.Paths.Clear();
                best.RouteAll(map);
            }

            return best;
        }

        /* Segments(); The segments of a connection, oriented from 'from' to 'to', trimmed at the edges of both squares; */
        public List<RouteSegment> Segments(int from, int to, bool isOneWay) {
            if (!Paths.TryGetValue(MapRoutes.ConnectionKey(from, to), out List<(double x, double y)> path)) {
                return MapRoutes.Route(Layout[from], Layout[to], isOneWay);
            }

            List<(double x, double y)> points = from < to ? path : Enumerable.Reverse(path).ToList();
            points = ClipInside(points, Layout[from]);
            points.Reverse();
            points = ClipInside(points, Layout[to]);
            points.Reverse();

            List<RouteSegment> segments = new List<RouteSegment>();

            for (int i = 0; i + 1 < points.Count; i++) {
                if (points[i] == points[i + 1]) continue;
                segments.Add(new RouteSegment(points[i].x, points[i].y, points[i + 1].x, points[i + 1].y, isOneWay));
            }

            return segments;
        }

        // ClipInside(); Removes the start of a path that's under the given room square;
        static List<(double x, double y)> ClipInside(List<(double x, double y)> points, RoomPlacement room) {
            bool Inside((double x, double y) p) =>
                p.x > room.X && p.x < room.X + room.Width && p.y > room.Y && p.y < room.Y + room.Height;

            for (int i = 0; i + 1 < points.Count; i++) {
                if (Inside(points[i + 1])) continue;

                // The path leaves the square on this segment; Walk it pixel by pixel to the edge;
                (double x1, double y1) = points[i];
                (double x2, double y2) = points[i + 1];
                double steps = Math.Max(Math.Abs(x2 - x1), Math.Abs(y2 - y1));
                double stepX = (x2 - x1) / steps, stepY = (y2 - y1) / steps;
                (double x, double y) edge = points[i];

                for (int k = 0; k <= steps && Inside(edge); k++) edge = (x1 + stepX * k, y1 + stepY * k);

                List<(double x, double y)> clipped = new List<(double x, double y)> { edge };
                clipped.AddRange(points.Skip(i + 1));

                return clipped;
            }

            return points;
        }

        //////////////////////////////////////////////////////////////////////////////////////////////

        int originX, originY, columns, rows;
        int[] blockOwnerA, blockOwnerB; // Up to two rooms blocking a cell (-1 none, -2 more than two);
        float[] usedCost, outsideCost;

        // A* state arrays, reused by every search; A state is valid for the current search when its stamp matches;
        float[] stateCost;
        int[] stateFrom, stateStamp;
        int searchStamp;

        void RouteAll(RomMap map) {
            Paths = new Dictionary<string, List<(double x, double y)>>();
            UnroutedConnections = 0;

            const int Border = 160;

            originX = (int)Math.Floor((Layout.Values.Min(p => p.X) - Border) / Grid) * Grid;
            originY = (int)Math.Floor((Layout.Values.Min(p => p.Y) - Border) / Grid) * Grid;
            columns = (int)Math.Ceiling((Layout.Values.Max(p => p.X + p.Width) + Border - originX) / Grid) + 1;
            rows = (int)Math.Ceiling((Layout.Values.Max(p => p.Y + p.Height) + Border - originY) / Grid) + 1;

            blockOwnerA = new int[columns * rows];
            blockOwnerB = new int[columns * rows];
            usedCost = new float[columns * rows];
            stateCost = new float[columns * rows * 9];
            stateFrom = new int[columns * rows * 9];
            stateStamp = new int[columns * rows * 9];
            searchStamp = 0;
            Array.Fill(blockOwnerA, -1);
            Array.Fill(blockOwnerB, -1);

            foreach ((int room, RoomPlacement p) in Layout) BlockRoom(room, p);

            /* Lines may go around the map when it's crowded, but every cell outside the rooms' box costs more the farther out it is,
             * so they hug the map instead of looping far away; */
            outsideCost = new float[columns * rows];
            int boxLeft = (int)Math.Floor((Layout.Values.Min(p => p.X) - originX) / Grid), boxTop = (int)Math.Floor((Layout.Values.Min(p => p.Y) - originY) / Grid);
            int boxRight = (int)Math.Ceiling((Layout.Values.Max(p => p.X + p.Width) - originX) / Grid), boxBottom = (int)Math.Ceiling((Layout.Values.Max(p => p.Y + p.Height) - originY) / Grid);

            for (int y = 0; y < rows; y++) {
                for (int x = 0; x < columns; x++) {
                    int outside = Math.Max(Math.Max(boxLeft - x, x - boxRight), Math.Max(boxTop - y, y - boxBottom));
                    if (outside > 0) outsideCost[y * columns + x] = (float)(OutsideCost * outside);
                }
            }

            // Shorter connections are routed first, so they get the straightest lines;
            List<(int a, int b)> connections = map.Rooms.Values
                .SelectMany(room => room.Doors.Where(d => d.Destination != room.Id && Layout.ContainsKey(d.Destination) && Layout.ContainsKey(room.Id))
                                              .Select(d => (a: Math.Min(room.Id, d.Destination), b: Math.Max(room.Id, d.Destination))))
                .Distinct()
                .OrderBy(c => Distance(Layout[c.a], Layout[c.b])).ThenBy(c => c.a).ThenBy(c => c.b)
                .ToList();

            foreach ((int a, int b) in connections) {
                List<(int x, int y)> cells = FindPath(a, b);

                if (cells == null) {
                    UnroutedConnections++;
                    continue;
                }

                foreach ((int x, int y) in cells) usedCost[y * columns + x] += (float)UsedCellCost;

                Paths[MapRoutes.ConnectionKey(a, b)] = Simplify(cells).Select(c => ((double)originX + c.x * Grid, (double)originY + c.y * Grid)).ToList();
            }
        }

        static double Distance(RoomPlacement a, RoomPlacement b) => Math.Max(Math.Abs(a.CenterX - b.CenterX), Math.Abs(a.CenterY - b.CenterY));

        // BlockRoom(); Marks the cells where a line would touch the room square;
        void BlockRoom(int room, RoomPlacement p) {
            double reach = p.Width / 2 + Clearance;

            int fromX = (int)Math.Floor((p.CenterX - reach - originX) / Grid), toX = (int)Math.Ceiling((p.CenterX + reach - originX) / Grid);
            int fromY = (int)Math.Floor((p.CenterY - reach - originY) / Grid), toY = (int)Math.Ceiling((p.CenterY + reach - originY) / Grid);

            for (int y = Math.Max(0, fromY); y <= Math.Min(rows - 1, toY); y++) {
                for (int x = Math.Max(0, fromX); x <= Math.Min(columns - 1, toX); x++) {
                    double dx = Math.Abs(originX + x * Grid - p.CenterX), dy = Math.Abs(originY + y * Grid - p.CenterY);

                    if (Math.Max(dx, dy) >= reach) continue;

                    int i = y * columns + x;

                    if (blockOwnerA[i] == -1) blockOwnerA[i] = room;
                    else if (blockOwnerB[i] == -1) blockOwnerB[i] = room;
                    else blockOwnerA[i] = blockOwnerB[i] = -2;
                }
            }
        }

        bool IsFree(int x, int y, int from, int to) {
            if (x < 0 || y < 0 || x >= columns || y >= rows) return false;

            int i = y * columns + x, a = blockOwnerA[i], b = blockOwnerB[i];

            // The two rooms being connected don't block their own line;
            return (a == -1 || a == from || a == to) && (b == -1 || b == from || b == to);
        }

        (int x, int y) CellOf(RoomPlacement p) => ((int)Math.Round((p.CenterX - originX) / Grid), (int)Math.Round((p.CenterY - originY) / Grid));

        /* FindPath(); A* over (cell, direction) states, so turns can be priced. Returns the cells of the path, or null; */
        List<(int x, int y)> FindPath(int from, int to) {
            (int startX, int startY) = CellOf(Layout[from]);
            (int goalX, int goalY) = CellOf(Layout[to]);

            int stamp = ++searchStamp;
            PriorityQueue<int, float> open = new PriorityQueue<int, float>();

            float Heuristic(int x, int y) {
                int dx = Math.Abs(x - goalX), dy = Math.Abs(y - goalY);
                return Math.Max(dx, dy);
            }

            // State = cell * 9 + direction (8 = no direction yet, at the start);
            int start = (startY * columns + startX) * 9 + 8;
            stateCost[start] = 0;
            stateStamp[start] = stamp;
            open.Enqueue(start, Heuristic(startX, startY));

            int expansions = 0, goalState = -1;

            while (open.Count > 0 && expansions++ < MaxExpansions) {
                int state = open.Dequeue();
                int cell = state / 9, direction = state % 9;
                int x = cell % columns, y = cell / columns;

                if (x == goalX && y == goalY) { goalState = state; break; }

                float current = stateCost[state];

                for (int d = 0; d < 8; d++) {
                    // No U-turns or sharp (135°) turns;
                    int turn = direction == 8 ? 0 : Math.Min((d - direction + 8) % 8, (direction - d + 8) % 8);
                    if (turn > 2) continue;

                    (int dx, int dy) = Directions[d];
                    int nx = x + dx, ny = y + dy;

                    if (!IsFree(nx, ny, from, to)) continue;

                    // Diagonal steps can't squeeze between two blocked cells;
                    if (dx != 0 && dy != 0 && (!IsFree(x + dx, y, from, to) || !IsFree(x, y + dy, from, to))) continue;

                    float step = 1 + usedCost[ny * columns + nx] + outsideCost[ny * columns + nx] + (turn == 1 ? (float)TurnCost45 : turn == 2 ? (float)TurnCost90 : 0);
                    int next = (ny * columns + nx) * 9 + d;
                    float total = current + step;

                    if (stateStamp[next] == stamp && stateCost[next] <= total) continue;

                    stateStamp[next] = stamp;
                    stateCost[next] = total;
                    stateFrom[next] = state;
                    open.Enqueue(next, total + Heuristic(nx, ny));
                }
            }

            if (goalState < 0) return null;

            List<(int x, int y)> cells = new List<(int x, int y)>();

            for (int s = goalState; ; s = stateFrom[s]) {
                int c = s / 9;
                cells.Add((c % columns, c / columns));
                if (s == start) break;
            }

            cells.Reverse();
            return cells;
        }

        // Simplify(); Keeps only the cells where the path changes direction;
        static List<(int x, int y)> Simplify(List<(int x, int y)> cells) {
            List<(int x, int y)> points = new List<(int x, int y)> { cells[0] };

            for (int i = 1; i + 1 < cells.Count; i++) {
                (int ax, int ay) = (cells[i].x - cells[i - 1].x, cells[i].y - cells[i - 1].y);
                (int bx, int by) = (cells[i + 1].x - cells[i].x, cells[i + 1].y - cells[i].y);

                if (ax != bx || ay != by) points.Add(cells[i]);
            }

            if (cells.Count > 1) points.Add(cells[^1]);

            return points;
        }
    }
}
