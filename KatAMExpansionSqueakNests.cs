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
