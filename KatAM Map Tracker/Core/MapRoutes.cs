using System;
using System.Collections.Generic;
using System.Linq;

namespace KatAMMapTracker {
    /* RouteSegment; A straight piece of a connection, in map pixels, going from Start to End.
     * It's horizontal, vertical or diagonal (45°), and 16 pixels thick like the in-game lines; */
    public readonly record struct RouteSegment(double StartX, double StartY, double EndX, double EndY, bool IsOneWay) {
        public int DirectionX => Math.Sign(EndX - StartX);
        public int DirectionY => Math.Sign(EndY - StartY);
        public bool IsHorizontal => DirectionY == 0;
        public bool IsVertical => DirectionX == 0;
        public bool IsDiagonal => DirectionX != 0 && DirectionY != 0;
        public double Length => Math.Sqrt((EndX - StartX) * (EndX - StartX) + (EndY - StartY) * (EndY - StartY));

        // Angle of the segment in degrees, clockwise from the right (screen coordinates);
        public double Angle => Math.Atan2(EndY - StartY, EndX - StartX) * 180 / Math.PI;
    }

    /* MapRoutes; Routes the connections between rooms with 0°, 45° and 90° lines, like a metro map:
     * rooms that line up get a single straight line; otherwise a straight leg is followed by a 45° leg into the destination.
     * Room centers are on the 8 pixel grid, so every corner is too; */
    public static class MapRoutes {
        // Amount of line colors; Connections touching the same room get different colors when possible;
        public const int LineColorCount = 6;

        public static List<RouteSegment> Route(RoomPlacement from, RoomPlacement to, bool isOneWay) {
            List<RouteSegment> segments = new List<RouteSegment>();
            double fromX = from.CenterX, fromY = from.CenterY, toX = to.CenterX, toY = to.CenterY;
            double dx = toX - fromX, dy = toY - fromY;
            double absX = Math.Abs(dx), absY = Math.Abs(dy);

            if (absX == 0 && absY == 0) return segments;

            List<(double x, double y)> points = new List<(double x, double y)> { (fromX, fromY) };

            if (absX != 0 && absY != 0 && absX != absY) {
                // Straight leg along the longest axis, then the 45° leg into the destination;
                double straight = Math.Max(absX, absY) - Math.Min(absX, absY);
                points.Add(absX > absY ? (fromX + Math.Sign(dx) * straight, fromY) : (fromX, fromY + Math.Sign(dy) * straight));
            }

            points.Add((toX, toY));

            for (int i = 0; i + 1 < points.Count; i++) {
                (double x1, double y1) = points[i];
                (double x2, double y2) = points[i + 1];
                int directionX = Math.Sign(x2 - x1), directionY = Math.Sign(y2 - y1);

                // Lines start and end at the edges of the room squares, not under them;
                if (i == 0) { x1 += directionX * from.Width / 2; y1 += directionY * from.Height / 2; }
                if (i + 2 == points.Count) { x2 -= directionX * to.Width / 2; y2 -= directionY * to.Height / 2; }

                // Skip segments swallowed by the squares;
                if (Math.Sign(x2 - x1) != directionX || Math.Sign(y2 - y1) != directionY) continue;
                if (x1 == x2 && y1 == y2) continue;

                segments.Add(new RouteSegment(x1, y1, x2, y2, isOneWay));
            }

            return segments;
        }

        public static string ConnectionKey(int a, int b) => a < b ? $"{a:X}-{b:X}" : $"{b:X}-{a:X}";

        /* AssignLineColors(); Gives every connection of the ROM a line color, so lines next to each other don't blend;
         * Greedy coloring: each connection takes the first color not used by the other connections of its two rooms; */
        public static Dictionary<string, int> AssignLineColors(RomMap map) {
            Dictionary<string, int> colors = new Dictionary<string, int>();
            Dictionary<int, HashSet<int>> usedByRoom = new Dictionary<int, HashSet<int>>();

            IEnumerable<(int a, int b)> connections = map.Rooms.Values
                .SelectMany(room => room.Doors.Where(d => d.Destination != room.Id && map.Rooms.ContainsKey(d.Destination))
                                              .Select(d => (a: Math.Min(room.Id, d.Destination), b: Math.Max(room.Id, d.Destination))))
                .Distinct()
                .OrderBy(c => c.a).ThenBy(c => c.b);

            HashSet<int> Used(int room) {
                if (!usedByRoom.TryGetValue(room, out HashSet<int> used)) usedByRoom[room] = used = new HashSet<int>();
                return used;
            }

            foreach ((int a, int b) in connections) {
                HashSet<int> usedA = Used(a), usedB = Used(b);
                int color = Enumerable.Range(0, LineColorCount).FirstOrDefault(c => !usedA.Contains(c) && !usedB.Contains(c));

                // Every color is taken around these rooms; Spread them evenly instead;
                if (usedA.Contains(color) || usedB.Contains(color)) color = (a + b) % LineColorCount;

                colors[ConnectionKey(a, b)] = color;
                usedA.Add(color);
                usedB.Add(color);
            }

            return colors;
        }
    }
}
