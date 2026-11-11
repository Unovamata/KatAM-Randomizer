namespace KatAMRandomizer {
    internal static partial class KatAMExpansion {
        /* The Squeak Lairs' nests: the DS's own, standing at a door that leads into a Squeak Lair (PlanSqueakLevels(); the owner: "The
         * original entrace 'nests' things should be placed where the door where this Squeak Lair area leads to, to provide the player
         * a warning, that, that will be a miniboss fight"). The DS keeps them in every room's front block table (map table 1, the front
         * layer's), 5 blocks across and 3 down: blocks 0x110-0x114 (the flag and the top), 0x130-0x134 and 0x150-0x154 (its feet), drawn
         * with the room's own front tileset (map/NN_f.bin's tiles), each world's look: a stump, a grassy mound, a rock pile, a castle,
         * an igloo, a submarine, a factory, a UFO..., the doorway in the middle, its flag on top (a second copy at 0x115 on, the same).
         * SqueakNests(); A room's map with a nest on each of `doors` (the cell Kirby stands in to take it): its front layer (grid 3)
         * holding the nest's blocks, the doorway's column the door's, its feet the door's row, cut where the room ends. A Squeak Lair's
         * own room (`lair`) loses its third layer (grid 5), which holds only the hideout's black doorway (blocks 604 over 796, in every
         * one of them): its way out is KatAM's one-way door, shown once its mini-boss is beaten (the owner: "The black exit of the
         * Squeak Lairs should be replaced with the 1 way door"). The DS's map itself untouched (a copy); */
        static readonly int[] SqueakNestRows = { 0x110, 0x130, 0x150 };
        const int SqueakNestWidth = 5;
        /* SqueakNestKey(); A room's nest blocks' tilemap entries, as one key (most rooms share one: the nest; a few have other blocks
         * there, 8-3-9's; null where its front block table is too short); SqueakNestDrawn(), whether its front tileset draws a nest with
         * them: 30 to 80% of its 80 by 48 pixels drawn (the DS's nests 42-58%; 20_f, 29_f, 2C_f, 2D_f and 3B_f fill them, 3E_f and
         * 3F_f draw nothing). Randomize Squeak Lairs puts a lair's door only where both hold (PlanSqueakLevels()); */
        static string SqueakNestKey(SqueakMap map) =>
            map.TableCount(1) <= SqueakNestRows[^1] + SqueakNestWidth - 1 ? null
            : string.Join(",", SqueakNestRows.SelectMany(row => Enumerable.Range(row, SqueakNestWidth)).SelectMany(b => Enumerable.Range(0, 4).Select(q => map.Entry(1, b, q))));
        static bool SqueakNestDrawn(SqueakMap map, SqueakTileset front) {
            if (front == null || map.TableCount(1) <= SqueakNestRows[^1] + SqueakNestWidth - 1) return false;
            int drawn = 0, all = SqueakNestRows.Length * SqueakNestWidth * 256;
            foreach (int b in SqueakNestRows.SelectMany(row => Enumerable.Range(row, SqueakNestWidth)))
                for (int q = 0; q < 4; q++) {
                    int tile = map.Entry(1, b, q) & 0x3FF;
                    if (tile >= front.Count) continue;
                    for (int y = 0; y < 8; y++)
                        for (int x = 0; x < 8; x++) if (front.Pixel(tile, x, y) != 0) drawn++;
                }
            return drawn * 10 >= all * 3 && drawn * 10 <= all * 8;
        }
        static SqueakMap SqueakNests(SqueakMap map, IEnumerable<(int x, int y)> doors, bool lair = false) {
            List<(int x, int y)> list = map.TableCount(1) > SqueakNestRows[^1] + SqueakNestWidth - 1 ? doors.ToList() : new List<(int, int)>();
            if (list.Count == 0 && !lair) return map;
            byte[] data = (byte[])map.Data.Clone();
            int grid = map.Section(3);
            if (lair) Array.Clear(data, map.Section(5), map.Width * map.Height * 2);
            foreach ((int x, int y) in list)
                for (int row = 0; row < SqueakNestRows.Length; row++)
                    for (int column = 0; column < SqueakNestWidth; column++) {
                        int cx = x - SqueakNestWidth / 2 + column, cy = y - (SqueakNestRows.Length - 1) + row;
                        if (cx >= 0 && cx < map.Width && cy >= 0 && cy < map.Height) WriteShort(data, grid + (cy * map.Width + cx) * 2, SqueakNestRows[row] + column);
                    }
            return new SqueakMap(data);
        }
    }
}
