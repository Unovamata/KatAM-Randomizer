using System;
using System.Collections.Generic;
using System.Linq;

namespace KatAMMapTracker {
    /* MapVanilla; The game's own map layout, used when the ROM's mirrors are the original ones (see IsUnmodified),
     * so the tracker looks like the pause screen's area maps:
     * • Every room on an area map sits where the game draws it: its square's top-left map tile (sAreaMapRoomInfos in the
     *   KatAM decompilation, src/pause_area_map.c; room IDs and positions are the same in every version).
     * • Each area map is its own canvas, so the areas are arranged like the world map arranges them (the average spot of
     *   their rooms in gWorldMapKirbysCoor*, src/pause_world_map.c), spread apart until they don't overlap.
     * • Rooms the area maps don't show (tutorial, boss and goal game rooms, the last rooms...) are put next to a room they
     *   connect to, on the side its door is on. Rooms connected to nothing placed are lined up above the map; */
    public static class MapVanilla {
        // (room, area, type: N normal, B big, S star, map tile column, map tile row);
        const char N = 'N', B = 'B', S = 'S';
        static readonly (int room, int area, char type, int column, int row)[] Rooms = {
            (0x7E, 1, N, 26, 24), (0x22D, 1, B, 57, 33), (0xBE, 1, B, 53, 23), (0xC2, 1, B, 25, 33), (0x321, 1, B, 29, 17), (0xB0, 1, B, 1, 33),
            (0xC0, 1, B, 9, 17), (0x24E, 1, B, 33, 3), (0xB1, 1, B, 3, 3), (0x217, 1, N, 48, 4), (0x21A, 1, N, 48, 8), (0x218, 1, N, 40, 12),
            (0x219, 1, N, 44, 8), (0x21E, 1, N, 48, 12), (0x21F, 1, N, 60, 12), (0x8D, 1, N, 64, 16), (0x8F, 1, N, 64, 24), (0x22C, 1, N, 58, 30),
            (0x22B, 1, N, 54, 30), (0x22A, 1, N, 50, 26), (0x6C, 1, N, 46, 22), (0x8C, 1, N, 50, 22), (0x67, 1, N, 50, 18), (0x68, 1, N, 46, 18),
            (0x6B, 1, N, 42, 18), (0x6A, 1, N, 38, 18), (0x65, 1, N, 34, 18), (0x6E, 1, N, 46, 26), (0x1F7, 1, N, 42, 30), (0x1FB, 1, N, 38, 34),
            (0x89, 1, N, 32, 34), (0x88, 1, N, 26, 28), (0x7F, 1, N, 22, 34), (0x1F8, 1, N, 18, 30), (0xAE, 1, N, 14, 30), (0x77, 1, N, 14, 26),
            (0x76, 1, N, 10, 26), (0xAD, 1, N, 10, 30), (0xAF, 1, N, 6, 34), (0xAC, 1, N, 6, 30), (0xAB, 1, N, 6, 26), (0x75, 1, N, 6, 22),
            (0x73, 1, N, 10, 12), (0x8B, 1, N, 14, 8), (0x216, 1, N, 42, 4), (0x23A, 1, N, 26, 4), (0x23B, 1, N, 22, 4), (0x214, 1, N, 18, 4),
            (0x8A, 1, N, 14, 4), (0xB2, 1, N, 10, 4), (0x8E, 1, S, 53, 17), (0x2BC, 2, N, 56, 32), (0x90, 2, N, 6, 18), (0x93, 2, N, 18, 18),
            (0x92, 2, N, 12, 24), (0x91, 2, N, 6, 24), (0x95, 2, N, 24, 24), (0x2E4, 2, N, 36, 20), (0x2C4, 2, N, 36, 24), (0x2E7, 2, N, 36, 28),
            (0xB4, 2, N, 36, 36), (0x2C2, 2, N, 44, 24), (0x2BE, 2, N, 44, 28), (0x2E8, 2, N, 44, 32), (0x2C5, 2, N, 48, 28), (0x2C3, 2, N, 48, 24),
            (0x2C0, 2, N, 52, 28), (0x2C7, 2, N, 56, 28), (0x2C8, 2, N, 52, 20), (0x2E5, 2, N, 56, 16), (0x2E6, 2, N, 52, 12), (0x316, 2, B, 39, 19),
            (0x2E9, 2, B, 43, 35), (0x2C6, 2, B, 55, 35), (0x2C1, 2, S, 51, 5), (0xC3, 2, B, 29, 23), (0x94, 2, B, 17, 23), (0x1F4, 3, N, 16, 22),
            (0x1FC, 3, N, 10, 4), (0x210, 3, N, 10, 10), (0x20F, 3, N, 10, 18), (0x203, 3, N, 16, 18), (0x201, 3, N, 16, 10), (0x1FD, 3, N, 16, 4),
            (0x1FE, 3, N, 22, 4), (0x1F5, 3, N, 22, 10), (0x202, 3, N, 28, 10), (0x205, 3, N, 28, 16), (0x212, 3, N, 42, 16), (0x20B, 3, N, 48, 16),
            (0x211, 3, N, 48, 10), (0x200, 3, N, 48, 4), (0x1FF, 3, N, 42, 4), (0x250, 3, B, 5, 13), (0x204, 3, B, 15, 25), (0x24F, 3, B, 33, 15),
            (0x213, 3, B, 35, 3), (0xBF, 3, B, 53, 21), (0x138, 4, N, 0, 16), (0x13D, 4, N, 6, 22), (0x139, 4, N, 8, 16), (0x145, 4, N, 2, 26),
            (0x215, 4, N, 6, 30), (0x130, 4, N, 22, 20), (0x143, 4, N, 14, 10), (0x13B, 4, N, 20, 10), (0x142, 4, N, 26, 10), (0x13C, 4, N, 32, 10),
            (0x134, 4, N, 36, 14), (0x141, 4, N, 36, 20), (0x222, 4, N, 44, 20), (0x221, 4, N, 44, 32), (0x220, 4, N, 36, 32), (0x21C, 4, N, 30, 32),
            (0x21D, 4, N, 30, 28), (0x21B, 4, N, 24, 32), (0x144, 4, B, 13, 1), (0x146, 4, B, 13, 15), (0x136, 4, B, 13, 27), (0x186, 4, B, 25, 23),
            (0x223, 4, B, 49, 19), (0x13E, 4, S, 5, 9), (0x12C, 4, N, 14, 6), (0x258, 5, N, 8, 20), (0x2CC, 5, N, 12, 8), (0x2DA, 5, N, 20, 8),
            (0x2D0, 5, N, 30, 8), (0x2DE, 5, N, 30, 14), (0x2DD, 5, N, 24, 12), (0x2CD, 5, N, 16, 12), (0x2E1, 5, N, 12, 20), (0x2E0, 5, N, 18, 20),
            (0x2DF, 5, N, 24, 20), (0x2CA, 5, N, 30, 20), (0x2CF, 5, N, 36, 14), (0xAA, 5, N, 40, 14), (0xB5, 5, N, 40, 24), (0x7A, 5, N, 50, 14),
            (0x78, 5, N, 56, 14), (0x79, 5, N, 50, 8), (0xE2, 5, N, 44, 2), (0x2DB, 5, S, 7, 3), (0x2DC, 5, B, 7, 9), (0x2E2, 5, B, 1, 19),
            (0x2E3, 5, B, 17, 25), (0xC1, 5, B, 43, 13), (0x82, 6, N, 0, 2), (0x83, 6, N, 4, 2), (0x33A, 6, N, 4, 8), (0x84, 6, N, 8, 2),
            (0x85, 6, N, 12, 2), (0x86, 6, N, 18, 2), (0x33B, 6, N, 12, 6), (0x33C, 6, N, 16, 10), (0x337, 6, N, 22, 10), (0x338, 6, N, 48, 4),
            (0x33F, 6, N, 54, 10), (0x32E, 6, N, 48, 10), (0x33D, 6, N, 40, 10), (0x32B, 6, N, 30, 10), (0x33E, 6, N, 30, 14), (0x227, 6, N, 34, 14),
            (0x229, 6, N, 44, 18), (0x340, 6, N, 48, 18), (0x32A, 6, N, 30, 20), (0x322, 6, N, 34, 24), (0x333, 6, N, 44, 24), (0x335, 6, N, 48, 24),
            (0x336, 6, B, 55, 23), (0x330, 6, B, 39, 23), (0x32F, 6, B, 39, 13), (0x37A, 6, B, 25, 9), (0x339, 6, B, 53, 3), (0x87, 6, B, 23, 1),
            (0x320, 6, N, 52, 24), (0xE0, 7, B, 9, 27), (0xDB, 7, B, 1, 19), (0xE6, 7, S, 55, 1), (0xD6, 7, B, 59, 9), (0xD1, 7, B, 53, 13),
            (0x122, 7, B, 45, 17), (0x123, 7, B, 35, 27), (0xCE, 7, N, 54, 10), (0xE5, 7, N, 50, 2), (0xE4, 7, N, 50, 6), (0xCD, 7, N, 50, 10),
            (0xD2, 7, N, 50, 14), (0xCA, 7, N, 46, 28), (0xD0, 7, N, 46, 24), (0xD3, 7, N, 46, 32), (0xDE, 7, N, 42, 28), (0xE3, 7, N, 46, 14),
            (0xCB, 7, N, 46, 10), (0xCC, 7, N, 42, 10), (0xDC, 7, N, 38, 10), (0xE1, 7, N, 16, 10), (0xE8, 7, N, 32, 10), (0xE7, 7, N, 32, 16),
            (0xD7, 7, N, 32, 24), (0xD4, 7, N, 26, 24), (0xD8, 7, N, 20, 24), (0xDF, 7, N, 16, 28), (0xD9, 7, N, 16, 24), (0xDD, 7, N, 16, 20),
            (0xDA, 7, N, 10, 20), (0xC8, 7, N, 6, 20), (0x26E, 8, B, 47, 7), (0x262, 8, B, 41, 21), (0x2B2, 8, B, 33, 17), (0x269, 8, B, 25, 5),
            (0x20E, 8, B, 35, 1), (0x81, 8, B, 1, 1), (0x25B, 8, N, 38, 12), (0x26F, 8, N, 42, 12), (0x25C, 8, N, 42, 8), (0x25E, 8, N, 46, 12),
            (0x26C, 8, N, 56, 18), (0x26D, 8, N, 50, 18), (0x25D, 8, N, 46, 18), (0x259, 8, N, 42, 18), (0x263, 8, N, 38, 18), (0x264, 8, N, 30, 18),
            (0x261, 8, N, 22, 18), (0x266, 8, N, 18, 14), (0x26A, 8, N, 18, 8), (0x268, 8, N, 22, 8), (0x265, 8, N, 22, 14), (0x260, 8, N, 26, 14),
            (0x26B, 8, N, 26, 10), (0x20D, 8, N, 30, 2), (0x1F6, 8, N, 26, 2), (0x20C, 8, N, 22, 2), (0x71, 8, N, 18, 2), (0x66, 8, N, 14, 2),
            (0x72, 8, N, 10, 2), (0x199, 9, N, 0, 38), (0x1A9, 9, N, 8, 34), (0x1AB, 9, N, 8, 30), (0x194, 9, N, 12, 30), (0x197, 9, N, 18, 30),
            (0x191, 9, N, 24, 30), (0x198, 9, N, 32, 38), (0x195, 9, N, 20, 22), (0x19B, 9, N, 26, 22), (0x1A2, 9, N, 26, 18), (0x19E, 9, N, 22, 14),
            (0x192, 9, N, 30, 18), (0x1A3, 9, N, 34, 18), (0x1A8, 9, N, 42, 14), (0x1A6, 9, N, 42, 10), (0x19F, 9, N, 38, 10), (0x1A7, 9, N, 48, 10),
            (0x1A5, 9, N, 52, 10), (0x1A0, 9, N, 56, 10), (0x19D, 9, B, 15, 13), (0x193, 9, B, 11, 21), (0x1EA, 9, B, 21, 25), (0x196, 9, B, 21, 33),
            (0x1AA, 9, B, 33, 23), (0x1A4, 9, B, 39, 23), (0x1A1, 9, B, 55, 1), (0x19C, 9, S, 37, 17), (0x190, 9, N, 56, 6),        };

        // World map position of each area (tutorial, Rainbow Route... Candy Constellation), in world map pixels;
        static readonly (int x, int y)[] AreaAnchors = {
            (116, 84), (124, 86), (172, 76), (123, 131), (133, 21), (57, 74), (206, 140), (50, 37), (46, 122), (205, 17),
        };

        // Fingerprint of the original mirrors (see Fingerprint); The Japanese, American and European versions share it;
        const ulong OriginalMirrors = 0xD7553C6CD8543A94;

        const int Grid = MapGraphics.TileSize;
        const double AreaMargin = 64, NeighborStep = 48;

        /* Fingerprint(); Hash of every mirror of the ROM: room, destination, door position and landing spot.
         * Anything else in the ROM (enemies, items, colors) can change without changing the layout; */
        public static ulong Fingerprint(RomMap map) {
            ulong hash = 14695981039346656037UL;

            void Add(int value) {
                for (int i = 0; i < 4; i++) {
                    hash ^= (byte)(value >> (i * 8));
                    hash *= 1099511628211UL;
                }
            }

            foreach (RoomInfo room in map.Rooms.Values.OrderBy(r => r.Id)) {
                foreach (DoorInfo door in room.Doors.Where(d => d.Kind == DoorKind.Mirror).OrderBy(d => d.Destination).ThenBy(d => d.X).ThenBy(d => d.Y)) {
                    Add(room.Id); Add(door.Destination); Add(door.X); Add(door.Y); Add(door.LandX); Add(door.LandY);
                }
            }

            return hash;
        }

        // IsUnmodified(); Whether the ROM's mirrors are the original game's;
        public static bool IsUnmodified(RomMap map) => Fingerprint(map) == OriginalMirrors;

        /* Generate(); The original layout, with the area maps scaled by 'scale' (1 = the game's spacing).
         * clusters: the area of every room, so each area gets its own region color; */
        public static Dictionary<int, RoomPlacement> Generate(RomMap map, double scale, out Dictionary<int, int> areaOf) {
            Dictionary<int, (double x, double y)> centers = new Dictionary<int, (double x, double y)>();
            Dictionary<int, int> clusters = areaOf = new Dictionary<int, int>();

            double Snap(double value) => Math.Round(value / Grid) * Grid;
            double SizeOf(int room) => MapGraphics.SizeOf(MapGraphics.ShapeOf(map.Rooms[room]));

            // Area maps;
            foreach ((int room, int area, char type, int column, int row) in Rooms) {
                if (!map.Rooms.ContainsKey(room) || centers.ContainsKey(room)) continue;

                double size = type == N ? 2 * Grid : 4 * Grid;
                centers[room] = (Snap((column * Grid + size / 2) * scale), Snap((row * Grid + size / 2) * scale));
                clusters[room] = area;
            }

            // Areas are placed like the world map places them, then pushed apart until they don't overlap;
            List<int> areas = clusters.Values.Distinct().OrderBy(a => a).ToList();
            Dictionary<int, (double minX, double minY, double maxX, double maxY)> bounds = areas.ToDictionary(a => a, a => {
                List<int> members = clusters.Where(kvp => kvp.Value == a).Select(kvp => kvp.Key).ToList();
                return (members.Min(r => centers[r].x - SizeOf(r) / 2), members.Min(r => centers[r].y - SizeOf(r) / 2),
                        members.Max(r => centers[r].x + SizeOf(r) / 2), members.Max(r => centers[r].y + SizeOf(r) / 2));
            });

            double spread = 6 * scale;
            Dictionary<int, (double x, double y)> offsets = areas.ToDictionary(a => a, a => {
                (double minX, double minY, double maxX, double maxY) = bounds[a];
                return (AreaAnchors[a].x * spread - (minX + maxX) / 2, AreaAnchors[a].y * spread - (minY + maxY) / 2);
            });

            for (int iteration = 0; iteration < 500; iteration++) {
                bool moved = false;

                for (int i = 0; i < areas.Count; i++) {
                    for (int j = i + 1; j < areas.Count; j++) {
                        int a = areas[i], b = areas[j];
                        var (ax0, ay0, ax1, ay1) = bounds[a];
                        var (bx0, by0, bx1, by1) = bounds[b];
                        (double oax, double oay) = offsets[a];
                        (double obx, double oby) = offsets[b];

                        double overlapX = Math.Min(ax1 + oax, bx1 + obx) - Math.Max(ax0 + oax, bx0 + obx) + AreaMargin;
                        double overlapY = Math.Min(ay1 + oay, by1 + oby) - Math.Max(ay0 + oay, by0 + oby) + AreaMargin;

                        if (overlapX <= 0 || overlapY <= 0) continue;

                        // Pushed apart along the axis that needs the least movement, away from each other's centers;
                        double centerAX = (ax0 + ax1) / 2 + oax, centerBX = (bx0 + bx1) / 2 + obx;
                        double centerAY = (ay0 + ay1) / 2 + oay, centerBY = (by0 + by1) / 2 + oby;

                        if (overlapX < overlapY) {
                            double push = Snap(overlapX / 2 + Grid) * (centerAX <= centerBX ? 1 : -1);
                            offsets[a] = (oax - push, oay);
                            offsets[b] = (obx + push, oby);
                        } else {
                            double push = Snap(overlapY / 2 + Grid) * (centerAY <= centerBY ? 1 : -1);
                            offsets[a] = (oax, oay - push);
                            offsets[b] = (obx, oby + push);
                        }

                        moved = true;
                    }
                }

                if (!moved) break;
            }

            foreach (int room in centers.Keys.ToList()) {
                (double ox, double oy) = offsets[clusters[room]];
                centers[room] = (Snap(centers[room].x + ox), Snap(centers[room].y + oy));
            }

            PlaceRest(map, centers, clusters, SizeOf, Snap);

            return centers.ToDictionary(kvp => kvp.Key, kvp => {
                double size = SizeOf(kvp.Key);
                return new RoomPlacement(kvp.Value.x - size / 2, kvp.Value.y - size / 2, size, size);
            });
        }

        /* PlaceRest(); Rooms not on the area maps: next to a placed room they connect to (in breadth-first order), on the side
         * of its door, at the first free spot going outwards. Rooms connected to nothing placed are lined up in a row above the map; */
        static void PlaceRest(RomMap map, Dictionary<int, (double x, double y)> centers, Dictionary<int, int> clusters,
                              Func<int, double> sizeOf, Func<double, double> snap) {
            Dictionary<int, List<int>> neighbors = map.Rooms.Keys.ToDictionary(id => id, id => new List<int>());

            foreach (RoomInfo room in map.Rooms.Values) {
                foreach (DoorInfo door in room.Doors) {
                    if (door.Destination == room.Id || !map.Rooms.ContainsKey(door.Destination)) continue;
                    if (!neighbors[room.Id].Contains(door.Destination)) neighbors[room.Id].Add(door.Destination);
                    if (!neighbors[door.Destination].Contains(room.Id)) neighbors[door.Destination].Add(room.Id);
                }
            }

            bool IsFree(double x, double y, double size) => centers.All(kvp =>
                Math.Max(Math.Abs(kvp.Value.x - x), Math.Abs(kvp.Value.y - y)) >= (sizeOf(kvp.Key) + size) / 2 + 24);

            (int dx, int dy)[] directions = { (1, 0), (0, 1), (-1, 0), (0, -1), (1, 1), (-1, 1), (-1, -1), (1, -1) };
            Queue<int> queue = new Queue<int>(centers.Keys.OrderBy(id => id));
            int nextCluster = 100;

            while (true) {
                while (queue.Count > 0) {
                    int placed = queue.Dequeue();

                    foreach (int room in neighbors[placed].OrderBy(id => id)) {
                        if (centers.ContainsKey(room)) continue;

                        double size = sizeOf(room);
                        DoorSide? side = MapSides.Of(map, placed, room);
                        IEnumerable<(int dx, int dy)> order = side is DoorSide s
                            ? directions.OrderBy(d => MapSides.Difference(MapSides.Octant(d.dx, d.dy), MapSides.Octant(s.DX, s.DY)))
                            : directions;

                        (double x, double y)? spot = null;

                        for (int ring = 1; spot == null && ring <= 12; ring++) {
                            foreach ((int dx, int dy) in order) {
                                double x = snap(centers[placed].x + dx * NeighborStep * ring), y = snap(centers[placed].y + dy * NeighborStep * ring);
                                if (IsFree(x, y, size)) { spot = (x, y); break; }
                            }
                        }

                        if (spot == null) continue;

                        centers[room] = spot.Value;
                        clusters[room] = clusters[placed];
                        queue.Enqueue(room);
                    }
                }

                // Rooms connected to nothing placed (the tutorial, the last rooms...): one row above the map, in ID order;
                List<int> rest = map.Rooms.Keys.Where(id => !centers.ContainsKey(id)).OrderBy(id => id).ToList();

                if (rest.Count == 0) break;

                double rowY = snap((centers.Count == 0 ? 0 : centers.Values.Min(c => c.y)) - 128);
                double rowX = snap(centers.Count == 0 ? 0 : centers.Values.Min(c => c.x));

                foreach (int room in rest) {
                    if (centers.ContainsKey(room)) continue;

                    centers[room] = (rowX, rowY);
                    clusters[room] = nextCluster++;
                    rowX += NeighborStep;

                    // Its own connections (the tutorial's rooms) follow it in the row;
                    foreach (int next in neighbors[room].Where(n => !centers.ContainsKey(n)).OrderBy(n => n)) {
                        centers[next] = (rowX, rowY);
                        clusters[next] = clusters[room];
                        rowX += NeighborStep;
                    }
                }
            }
        }
    }
}
