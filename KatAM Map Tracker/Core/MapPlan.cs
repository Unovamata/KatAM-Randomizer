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

        const double TurnCost45 = 0.6, TurnCost90 = 2.5, UsedCellCost = 0.35;
        const int MaxExpansions = 400000, MaxSidedExpansions = 6000;

        // Sided paths: at most this much longer than the distance between the rooms (in cells), and this many 45° turns in total;
        const double MaxDetour = 1.3;
        const int DetourSlack = 6, MaxTurns = 4;

        // Connections whose lines leave both rooms from their door's side (see FindSidedPath);
        public int SidedConnections { get; private set; }

        static readonly (int x, int y)[] Directions = {
            (1, 0), (1, 1), (0, 1), (-1, 1), (-1, 0), (-1, -1), (0, -1), (1, -1),
        };

        // Whether the rooms sit where the game's own maps draw them (the ROM's mirrors are the original ones, see MapVanilla);
        public bool IsOriginalLayout { get; private set; }

        public static MapPlan Generate(RomMap map) {
            if (MapVanilla.IsUnmodified(map)) {
                MapPlan original = null;

                // The game's spacing first, wider when some lines can't be routed between the squares;
                foreach (double scale in new[] { 1.0, 1.5, 2.0, 2.5 }) {
                    MapPlan plan = new MapPlan { Layout = MapVanilla.Generate(map, scale, out Dictionary<int, int> areas), IsOriginalLayout = true };
                    plan.Clusters = areas;
                    plan.RouteAll(map);

                    if (original == null || plan.UnroutedConnections < original.UnroutedConnections) original = plan;
                    if (plan.UnroutedConnections == 0) break;
                }

                return original;
            }

            (int cell, double margin)[] attempts = {
                (MapLayout.DefaultCell, MapLayout.DefaultClusterMargin),
                (MapLayout.DefaultCell, MapLayout.DefaultClusterMargin * 2),
                (MapLayout.DefaultCell + 8, MapLayout.DefaultClusterMargin * 2),
                (MapLayout.DefaultCell + 16, MapLayout.DefaultClusterMargin * 3),
            };

            MapPlan best = null;

            foreach ((int cell, double margin) in attempts) {
                MapPlan plan = new MapPlan { Layout = MapLayout.Generate(map, out Dictionary<int, int> clusters, cell, margin) };
                plan.Clusters = clusters;
                plan.RouteAll(map);

                if (best == null || plan.UnroutedConnections < best.UnroutedConnections) best = plan;
                if (plan.UnroutedConnections == 0) break;
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
        float[] usedCost;

        // A* state arrays, reused by every search; A state is valid for the current search when its stamp matches;
        float[] stateCost;
        int[] stateFrom, stateStamp;
        int searchStamp;

        void RouteAll(RomMap map) {
            Paths = new Dictionary<string, List<(double x, double y)>>();
            UnroutedConnections = 0;
            SidedConnections = 0;

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

            // Shorter connections are routed first, so they get the straightest lines;
            List<(int a, int b)> connections = map.Rooms.Values
                .SelectMany(room => room.Doors.Where(d => d.Destination != room.Id && Layout.ContainsKey(d.Destination) && Layout.ContainsKey(room.Id))
                                              .Select(d => (a: Math.Min(room.Id, d.Destination), b: Math.Max(room.Id, d.Destination))))
                .Distinct()
                .OrderBy(c => Distance(Layout[c.a], Layout[c.b])).ThenBy(c => c.a).ThenBy(c => c.b)
                .ToList();

            foreach ((int a, int b) in connections) {
                List<(int x, int y)> cells = FindSidedPath(map, a, b);

                if (cells != null) SidedConnections++;
                else cells = FindPath(a, b);

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

        /* FindSidedPath(); A path that leaves each room from the side its door is on (see MapSides):
         * it goes straight out of the square to a port cell past the room's clearance, is routed between the two ports
         * starting in the port's direction and arriving within 90° of the other port's, then goes straight into the other room.
         * Doors on a straight side of a big square leave from where they are along it. A room without a side keeps the
         * usual route from its center. Returns null when the ports are blocked or can't be joined; */
        List<(int x, int y)> FindSidedPath(RomMap map, int a, int b) {
            DoorSide? sideA = Facing(map, a, b), sideB = Facing(map, b, a);

            if (sideA == null && sideB == null) return null;

            // Both sides, else only one of them, as long as the line stays short and simple (see IsDirect);
            foreach ((DoorSide? useA, DoorSide? useB) in new[] { (sideA, sideB), (sideA, null), (null, sideB) }) {
                if (useA == null && useB == null) continue;

                List<(int x, int y)> cells = FindSidedPath(a, b, useA, useB);

                if (cells != null && IsDirect(cells, a, b)) return cells;
                if (sideA == null || sideB == null) break;
            }

            return null;
        }

        // Facing(); Side of 'room' for its connection with 'other', unless it faces away from 'other' (a line leaving it would loop back);
        DoorSide? Facing(RomMap map, int room, int other) {
            if (MapSides.Of(map, room, other) is not DoorSide side) return null;

            RoomPlacement from = Layout[room], to = Layout[other];
            int toward = MapSides.Octant(to.CenterX - from.CenterX, to.CenterY - from.CenterY);

            return MapSides.Difference(MapSides.Octant(side.DX, side.DY), toward) <= 2 ? side : null;
        }

        /* IsDirect(); A sided path is only worth it when it isn't much longer than the distance between the rooms
         * and doesn't turn more than half a turn in total (no loops around rooms); */
        bool IsDirect(List<(int x, int y)> cells, int a, int b) {
            (int ax, int ay) = CellOf(Layout[a]);
            (int bx, int by) = CellOf(Layout[b]);
            int distance = Math.Max(Math.Abs(ax - bx), Math.Abs(ay - by)), turns = 0;

            if (cells.Count - 1 > distance * MaxDetour + DetourSlack) return false;

            for (int i = 1; i + 1 < cells.Count; i++) {
                int d1 = Array.IndexOf(Directions, (cells[i].x - cells[i - 1].x, cells[i].y - cells[i - 1].y));
                int d2 = Array.IndexOf(Directions, (cells[i + 1].x - cells[i].x, cells[i + 1].y - cells[i].y));

                turns += MapSides.Difference(d1, d2);
            }

            return turns <= MaxTurns;
        }

        List<(int x, int y)> FindSidedPath(int a, int b, DoorSide? sideA, DoorSide? sideB) {

            (List<(int x, int y)> lead, int direction)? Port(int room, DoorSide? side) {
                if (side is not DoorSide s) return null;

                RoomPlacement p = Layout[room];
                (int cx, int cy) = CellOf(p);
                int reach = (int)Math.Ceiling((p.Width / 2 + Clearance) / Grid);
                int maxOffset = Math.Max(0, (int)Math.Floor((p.Width / 2 - 4) / Grid));
                int offset = s.DX != 0 && s.DY != 0 ? 0 : Math.Clamp((int)Math.Round(s.Along * p.Width / Grid), -maxOffset, maxOffset);

                // Perpendicular to a straight side: a vertical side moves along Y, a horizontal one along X;
                (int sx, int sy) = s.DX != 0 && s.DY == 0 ? (cx, cy + offset) : s.DY != 0 && s.DX == 0 ? (cx + offset, cy) : (cx, cy);
                List<(int x, int y)> lead = new List<(int x, int y)>();

                for (int k = 0; k <= reach; k++) lead.Add((sx + s.DX * k, sy + s.DY * k));

                (int px, int py) = lead[^1];
                if (!IsFree(px, py, -1, -1)) return null;

                return (lead, Array.IndexOf(Directions, (s.DX, s.DY)));
            }

            var portA = Port(a, sideA);
            var portB = Port(b, sideB);

            if ((sideA != null && portA == null) || (sideB != null && portB == null)) return null;

            (int x, int y) start = portA?.lead[^1] ?? CellOf(Layout[a]);
            (int x, int y) goal = portB?.lead[^1] ?? CellOf(Layout[b]);

            // Arriving at b's port heading into the room, the opposite of its side's direction;
            int arrival = portB is var (_, outward) ? (outward + 4) % 8 : -1;

            List<(int x, int y)> middle = FindPath(start, portA?.direction ?? 8, goal, arrival,
                                                   portA == null ? a : -1, portB == null ? b : -1, MaxSidedExpansions);

            if (middle == null) return null;

            List<(int x, int y)> cells = new List<(int x, int y)>();

            if (portA is var (leadA, _)) cells.AddRange(leadA.Take(leadA.Count - 1));
            cells.AddRange(middle);
            if (portB is var (leadB, _)) cells.AddRange(Enumerable.Reverse(leadB).Skip(1));

            return cells;
        }

        // FindPath(); Path from the center of a room to the center of another, each one not blocking its own line;
        List<(int x, int y)> FindPath(int from, int to) => FindPath(CellOf(Layout[from]), 8, CellOf(Layout[to]), -1, from, to, MaxExpansions);

        /* FindPath(); A* over (cell, direction) states, so turns can be priced. Returns the cells of the path, or null;
         * startDirection: direction the path is already heading at the start (8 = none).
         * arrivalDirection: the path must arrive heading within 90° of it (-1 = any) and then turn into it.
         * from, to: rooms that don't block the path (-1 = none); */
        List<(int x, int y)> FindPath((int x, int y) startCell, int startDirection, (int x, int y) goalCell, int arrivalDirection, int from, int to,
                                        int maxExpansions) {
            (int startX, int startY) = startCell;
            (int goalX, int goalY) = goalCell;

            int stamp = ++searchStamp;
            PriorityQueue<int, float> open = new PriorityQueue<int, float>();

            float Heuristic(int x, int y) {
                int dx = Math.Abs(x - goalX), dy = Math.Abs(y - goalY);
                return Math.Max(dx, dy);
            }

            // State = cell * 9 + direction (8 = no direction yet, at the start);
            int start = (startY * columns + startX) * 9 + startDirection;
            stateCost[start] = 0;
            stateStamp[start] = stamp;
            open.Enqueue(start, Heuristic(startX, startY));

            int expansions = 0, goalState = -1;

            while (open.Count > 0 && expansions++ < maxExpansions) {
                int state = open.Dequeue();
                int cell = state / 9, direction = state % 9;
                int x = cell % columns, y = cell / columns;

                if (x == goalX && y == goalY) {
                    int turnIn = arrivalDirection < 0 || direction == 8 ? 0
                        : Math.Min((arrivalDirection - direction + 8) % 8, (direction - arrivalDirection + 8) % 8);

                    if (turnIn <= 2) { goalState = state; break; }
                }

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

                    float step = 1 + usedCost[ny * columns + nx] + (turn == 1 ? (float)TurnCost45 : turn == 2 ? (float)TurnCost90 : 0);
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
