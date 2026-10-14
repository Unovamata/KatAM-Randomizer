using System;
using System.Collections.Generic;

namespace KatAMMapTracker {
    public enum RoomShape { Small, Hub, PathEnder }

    // Explored: every mirror of the room was used. Unexplored: some weren't. Current: Kirby is here (pulses);
    public enum RoomStyle { Explored, Unexplored, Current }

    // Sprite; An image made of in-game map tiles, as 0xAARRGGBB pixels (transparent where the game draws color 0);
    public sealed record Sprite(int Width, int Height, int[] Pixels);

    // LineSprites; The in-game line and arrow pieces in one color. Diagonals are drawn by rotating the horizontal pieces;
    public sealed record LineSprites(Sprite Horizontal, Sprite Vertical, Sprite ArrowRight, Sprite ArrowLeft, Sprite ArrowDown, Sprite ArrowUp);

    /* MapGraphics; The pause screen map graphics of the game, read from the ROM, to draw the tracker's map like the game does.
     * Addresses are the Japanese ones (see RomVersion);
     *
     * • Tiles: 256 8bpp tiles at 0x33F688 (the map is an affine BG layer).
     *   Room squares are 2x2 (small) or 4x4 (big) tile blocks, in row order:
     *     Small rooms: 1 (pulsing), 5 (yellow), 9 (orange). Big rooms: 16, 32, 48. Star rooms: 64, 80, 96.
     *   Lines: 160 over 176 (horizontal), 163 next to 164 (vertical). Diagonals rotate the horizontal pieces.
     *   One-Way arrows: 197/213 (right), 193/209 (left), 246|247 (down), 226|227 (up).
     * • Palette: 16 colors at 0x33F588, used by palette rows 0 and 2. Pulsing squares use row 0.
     * • Square pulse: 8 frames at 0x3458AE of [duration, 5 colors] written to colors 2-6 (durations in 1/60 s).
     * • Line colors: frames at 0x34591E of [duration, 4 colors] written to colors 18-21. The first frame is used; */
    public sealed class MapGraphics {
        const int TileCount = 256,
                  SquarePulseFrames = 8;

        // Addresses of the Japanese version, translated to the ROM's region;
        readonly int TileData, BasePalette, SquarePulse, LineColors;

        // Map tiles are drawn at 8 pixels per tile; Small rooms are 16 pixels, big rooms 32;
        public const int TileSize = 8;

        readonly byte[] tiles;
        readonly int[] basePalette = new int[256];
        readonly List<(int duration, int[] colors)> pulseFrames = new List<(int duration, int[] colors)>();
        readonly Dictionary<(RoomShape, RoomStyle, int), Sprite> roomCache = new Dictionary<(RoomShape, RoomStyle, int), Sprite>();

        // Hues of the line colors; The first one is the game's own purple. The others re-hue it, keeping its shading;
        static readonly double[] LineHues = { double.NaN, 205, 170, 325, 40, 115 };

        readonly Dictionary<int, LineSprites> lineCache = new Dictionary<int, LineSprites>();

        public int PulseFrameCount => pulseFrames.Count;

        public MapGraphics(RomMap map) {
            byte[] rom = map.Rom;
            int Read16(int address) => rom[address] | (rom[address + 1] << 8);

            int shift = map.Version.MapScreenShift;
            TileData = 0x33F688 + shift;
            BasePalette = 0x33F588 + shift;
            SquarePulse = 0x3458AE + shift;
            LineColors = 0x34591E + shift;

            tiles = new byte[TileCount * 64];
            Array.Copy(rom, TileData, tiles, 0, tiles.Length);

            for (int i = 0; i < 16; i++) {
                int color = RomMap.ToArgb(Read16(BasePalette + i * 2));
                basePalette[i] = color;      // Row 0;
                basePalette[32 + i] = color; // Row 2;
            }

            for (int i = 0; i < 4; i++) basePalette[18 + i] = RomMap.ToArgb(Read16(LineColors + 2 + i * 2));

            for (int frame = 0; frame < SquarePulseFrames; frame++) {
                int address = SquarePulse + frame * 12;
                int[] colors = new int[5];

                for (int i = 0; i < 5; i++) colors[i] = RomMap.ToArgb(Read16(address + 2 + i * 2));

                pulseFrames.Add((Math.Max(1, Read16(address)), colors));
            }

        }

        // Lines(); Line and arrow pieces in one of the line colors (0 is the in-game purple);
        public LineSprites Lines(int color) {
            color = ((color % LineHues.Length) + LineHues.Length) % LineHues.Length;

            if (lineCache.TryGetValue(color, out LineSprites cached)) return cached;

            int[] palette = (int[])basePalette.Clone();

            if (!double.IsNaN(LineHues[color])) {
                for (int i = 18; i < 22; i++) palette[i] = WithHue(basePalette[i], LineHues[color]);
            }

            LineSprites sprites = new LineSprites(
                Compose(new[,] { { 160 }, { 176 } }, palette),
                Compose(new[,] { { 163, 164 } }, palette),
                Compose(new[,] { { 197 }, { 213 } }, palette),
                Compose(new[,] { { 193 }, { 209 } }, palette),
                Compose(new[,] { { 246, 247 } }, palette),
                Compose(new[,] { { 226, 227 } }, palette));

            lineCache[color] = sprites;

            return sprites;
        }

        // WithHue(); Same color with another hue, keeping its saturation and brightness;
        static int WithHue(int argb, double hue) {
            double r = ((argb >> 16) & 0xFF) / 255.0, g = ((argb >> 8) & 0xFF) / 255.0, b = (argb & 0xFF) / 255.0;
            double max = Math.Max(r, Math.Max(g, b)), min = Math.Min(r, Math.Min(g, b));
            double saturation = max == 0 ? 0 : (max - min) / max, value = max;

            double h = hue / 60, c = value * saturation, x = c * (1 - Math.Abs(h % 2 - 1)), m = value - c;
            (double rr, double gg, double bb) = (int)h switch {
                0 => (c, x, 0.0), 1 => (x, c, 0.0), 2 => (0.0, c, x), 3 => (0.0, x, c), 4 => (x, 0.0, c), _ => (c, 0.0, x),
            };

            return unchecked((int)0xFF000000) | ((int)((rr + m) * 255) << 16) | ((int)((gg + m) * 255) << 8) | (int)((bb + m) * 255);
        }

        public static RoomShape ShapeOf(RoomInfo room) => room.IsHub ? RoomShape.Hub : room.IsPathEnder ? RoomShape.PathEnder : RoomShape.Small;

        public static int SizeOf(RoomShape shape) => shape == RoomShape.Small ? 2 * TileSize : 4 * TileSize;

        // PulseDuration(); How long a pulse frame lasts, in 1/60 seconds;
        public int PulseDuration(int frame) => pulseFrames[frame % pulseFrames.Count].duration;

        // Room(); Room square sprite. pulseFrame only matters for the current room;
        public Sprite Room(RoomShape shape, RoomStyle style, int pulseFrame = 0) {
            if (style != RoomStyle.Current) pulseFrame = 0;
            else pulseFrame %= pulseFrames.Count;

            if (roomCache.TryGetValue((shape, style, pulseFrame), out Sprite cached)) return cached;

            int styleOffset = style switch { RoomStyle.Current => 0, RoomStyle.Explored => 1, _ => 2 };
            int blockSize = shape == RoomShape.Small ? 2 : 4;
            int firstTile = shape switch {
                RoomShape.Small => 1 + styleOffset * 4,
                RoomShape.Hub => 16 + styleOffset * 16,
                _ => 64 + styleOffset * 16,
            };

            int[] palette = basePalette;

            if (style == RoomStyle.Current) {
                palette = (int[])basePalette.Clone();
                Array.Copy(pulseFrames[pulseFrame].colors, 0, palette, 2, 5);
            }

            int[,] block = new int[blockSize, blockSize];

            for (int i = 0; i < blockSize * blockSize; i++) block[i / blockSize, i % blockSize] = firstTile + i;

            Sprite sprite = Compose(block, palette);
            roomCache[(shape, style, pulseFrame)] = sprite;

            return sprite;
        }

        // Compose(); Builds a sprite from a grid of tile indices [row, column];
        Sprite Compose(int[,] grid, int[] palette) {
            int rows = grid.GetLength(0), columns = grid.GetLength(1);
            int width = columns * TileSize, height = rows * TileSize;
            int[] pixels = new int[width * height];

            for (int row = 0; row < rows; row++) {
                for (int column = 0; column < columns; column++) {
                    int tile = grid[row, column];

                    for (int y = 0; y < 8; y++) {
                        for (int x = 0; x < 8; x++) {
                            int index = tiles[tile * 64 + y * 8 + x];

                            // Color 0 is transparent;
                            pixels[(row * 8 + y) * width + column * 8 + x] = index == 0 ? 0 : palette[index];
                        }
                    }
                }
            }

            return new Sprite(width, height, pixels);
        }
    }
}
