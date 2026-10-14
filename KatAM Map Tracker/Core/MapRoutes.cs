using System;
using System.Collections.Generic;

namespace KatAMMapTracker {
    // Direction the arrows of a One-Way segment point to. None for Two-Way segments;
    public enum RouteDirection { None, Right, Left, Down, Up }

    /* RouteSegment; A straight horizontal or vertical piece of a connection, in map pixels.
     * Horizontal segments are centered on Y, vertical ones on X, and are 16 pixels thick like the in-game lines; */
    public readonly record struct RouteSegment(bool IsHorizontal, double Start, double End, double Position, RouteDirection Direction);

    /* MapRoutes; Routes the connections between rooms like the in-game map: a horizontal leg and a vertical leg
     * that meet at a corner, along the rooms' centers. Room centers are on the 8 pixel grid, so the segments
     * line up with the in-game line tiles; */
    public static class MapRoutes {
        public static List<RouteSegment> Route(RoomPlacement from, RoomPlacement to, bool isOneWay) {
            List<RouteSegment> segments = new List<RouteSegment>();
            double fromX = from.CenterX, fromY = from.CenterY, toX = to.CenterX, toY = to.CenterY;

            // The longest distance is covered first, so the corner sits next to the destination;
            bool horizontalFirst = Math.Abs(toX - fromX) >= Math.Abs(toY - fromY);
            double cornerX = horizontalFirst ? toX : fromX, cornerY = horizontalFirst ? fromY : toY;

            void Add(double x1, double y1, double x2, double y2, RoomPlacement? startRoom, RoomPlacement? endRoom) {
                bool horizontal = y1 == y2;
                double start = horizontal ? x1 : y1, end = horizontal ? x2 : y2;

                if (start == end) return;

                double sign = Math.Sign(end - start);

                // Segments start and end at the edges of the room squares, not under them;
                if (startRoom is RoomPlacement s) start += sign * (horizontal ? s.Width : s.Height) / 2;
                if (endRoom is RoomPlacement e) end -= sign * (horizontal ? e.Width : e.Height) / 2;

                if ((end - start) * sign <= 0) return;

                RouteDirection direction = !isOneWay ? RouteDirection.None
                    : horizontal ? (sign > 0 ? RouteDirection.Right : RouteDirection.Left)
                    : (sign > 0 ? RouteDirection.Down : RouteDirection.Up);

                segments.Add(new RouteSegment(horizontal, Math.Min(start, end), Math.Max(start, end), horizontal ? y1 : x1, direction));
            }

            bool hasCorner = cornerX != fromX || cornerY != fromY;
            bool reachesEnd = cornerX != toX || cornerY != toY;

            Add(fromX, fromY, cornerX, cornerY, from, reachesEnd ? null : to);
            if (hasCorner) Add(cornerX, cornerY, toX, toY, null, to);

            return segments;
        }
    }
}
