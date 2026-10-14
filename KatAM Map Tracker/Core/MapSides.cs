using System;
using System.Collections.Generic;
using System.Linq;

namespace KatAMMapTracker {
    /* DoorSide; Where a connection leaves a room square: one of the 8 directions (screen coordinates, Y down),
     * and how far along that side the door is (-0.5 to 0.5, perpendicular to a straight direction; 0 for diagonals); */
    public readonly record struct DoorSide(int DX, int DY, double Along);

    /* MapSides; The side of a room each connection's door is on, like the game's own area maps: a door at the left end of a
     * room is on the left, one in the ceiling on top. The original layout uses it to place the rooms the area maps don't show;
     * • A room's end of a connection is its door(s) leading to the other room, or, for a One-Way mirror arriving there,
     *   the spot where Kirby lands.
     * • Positions are taken relative to the room's size, so long corridors have their side doors on the ends. Doors near
     *   two edges (corners) leave diagonally; doors in the middle of a room have no side; */
    public static class MapSides {
        // How far from the center a door must be (0.5 = the edge) to have a side, and how close both axes must be to be diagonal;
        const double MinimumOffset = 0.12, DiagonalRatio = 0.8, DiagonalMinimum = 0.3;

        /* Of(); Side of 'room' for its connection with 'other', or null when the door is in the middle of the room
         * (or the room has no door to it and no landing from it); */
        public static DoorSide? Of(RomMap map, int room, int other) {
            if (!map.Rooms.TryGetValue(room, out RoomInfo info) || info.Width <= 0 || info.Height <= 0) return null;

            List<(double x, double y)> points = info.Doors.Where(d => d.Destination == other).Select(d => ((double)d.X, (double)d.Y)).ToList();

            if (points.Count == 0 && map.Rooms.TryGetValue(other, out RoomInfo from)) {
                points = from.Doors.Where(d => d.Destination == room).Select(d => ((double)d.LandX, (double)d.LandY)).ToList();
            }

            if (points.Count == 0) return null;

            double u = points.Average(p => p.x) / (info.Width * 8.0) - 0.5, v = points.Average(p => p.y) / (info.Height * 8.0) - 0.5;
            u = Math.Clamp(u, -0.5, 0.5);
            v = Math.Clamp(v, -0.5, 0.5);

            double au = Math.Abs(u), av = Math.Abs(v);

            if (Math.Max(au, av) < MinimumOffset) return null;

            if (Math.Min(au, av) >= DiagonalMinimum && Math.Min(au, av) >= DiagonalRatio * Math.Max(au, av)) return new DoorSide(Math.Sign(u), Math.Sign(v), 0);

            return au >= av ? new DoorSide(Math.Sign(u), 0, v) : new DoorSide(0, Math.Sign(v), u);
        }

        // Octant(); Index (0-7, clockwise from right) of a direction, for comparing directions;
        public static int Octant(double dx, double dy) {
            double angle = Math.Atan2(dy, dx);
            return ((int)Math.Round(angle / (Math.PI / 4)) % 8 + 8) % 8;
        }

        // Difference(); Steps between two octants (0 same, 4 opposite);
        public static int Difference(int a, int b) {
            int d = Math.Abs(a - b) % 8;
            return Math.Min(d, 8 - d);
        }
    }
}
