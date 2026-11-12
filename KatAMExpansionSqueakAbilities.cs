namespace KatAMRandomizer {
    /* KatAMExpansion's Squeak Squad abilities: Ninja, Animal, Bubble and Metal, which KatAM lacks, as abilities 35-38 (after NiDL's;
     * "ROM Expansion/abilities.h"). The owner's pick for their look: Kirby drawn as the DS draws him with them, in every motion.
     * • Each ability's motions are its kirby/<ability>_cmn file's animations (its .mca's composites as animations of their own,
     *   SqueakFlattenComposites: the slide), shown at the KatAM animation indexes of the same motions (SqueakMotions, found by playing
     *   both games: KatAM's indexes in its emulator, the DS's animations by its states). The rest of KatAM's indexes keep Normal's frames
     *   (Normal's own body, no hat: hurt, cutscenes, and what isn't matched yet).
     * • A DS frame's pieces in its palette 0 are Kirby's body: its colors 1-11 are KatAM's Kirby's palette's (the same colors at the
     *   same places), so the body anim draws them with the player's palette, spray colors included, in his body sprite (16 tiles of
     *   VRAM). The other pieces (the hat, Animal's hood and paws) are the hat anim's, drawn by his hat sprite (48 tiles) in the hat's
     *   own palette (the DS's palette 1, loaded by the hat anim's GetPalette, as KatAM's hats). Metal Kirby's body is all metal (his
     *   palette 0 is grey): all of him goes to the hat sprite, in that palette.
     * • Each ability gets a body table (an AnimInfo per KatAM animation index, Normal's copied, the motions' put in) its setup copies
     *   for the player ("ROM Expansion/abilities.c"'s SqueakAbilitySetup, gSqueakAbilityBodies), and a hat table (gUnk_08D60FDC's
     *   entry: the motions' hat frames, nothing elsewhere); */
    internal static partial class KatAMExpansion {
        static readonly string[] SqueakAbilityFiles = { "ninja", "animal", "bubble", "metal" };
        const int SqueakAbilityMetal = 3;
        const int AnimationIndexCount = 0xB9;      // KatAM's body tables' entries (sub_0806E4EC copies 0xB9);

        /* KatAM's animation indexes and the DS motions shown there: the ability's _cmn animation (>= 0), or ~k its .mca's composite k
         * (an animation after its own). KatAM's (its emulator): 0 standing, 1 crouching, 2-9 on slopes (standing, crouching, four
         * slopes), 11 walking, 13 sliding, 14 its end, 15 the jump's rise, 16 its flip, 17 falling, 22 braking to turn, 24 a high jump's
         * top, 25 dashing, 31 puffing up, 32 flapping, 33 floating, 36 letting the air out, 41 landing. The DS's: each ability's own
         * numbers, from the animation its states play (Kirby's state forced in the DS's emulator: 0x60 standing, 0x64 crouching, 0x68
         * walking, 0x6C dashing, 0x70 braking, 0x74 the jump (its rise, flip at 0x7C, fall at 0xB8, landing), 0x80 puffing up, 0x84
         * floating, 0x8C sliding, 0x9C letting the air out), its slopes the eight after its crouch, its flap the one before its float;
         * per ability: standing, crouching, walking, dashing, braking, sliding, rising, flipping, falling, landing, puffing up,
         * flapping, floating, letting the air out (Metal's dash is its walk: the DS's Metal Kirby doesn't dash); */
        static readonly int[][] SqueakAbilityMotions = {
            new[] { 1, 3, 12, 15, 13, ~0, ~1, 21, 22, 3, 32, 33, 34, 36 },                     // Ninja;
            new[] { 1, 3, 12, 15, 13, 16, 18, 19, 20, 3, 29, 30, 31, 28 },                     // Animal;
            new[] { ~0, ~2, ~11, ~14, ~12, ~15, ~16, 20, ~17, ~2, 32, ~22, ~22, 31 },           // Bubble;
            new[] { 1, 3, 12, 12, 14, 15, 22, 23, 24, 3, 30, 31, 32, 29 },                     // Metal;
        };

        // SqueakMotions(); The (KatAM animation index, DS motion) pairs of ability `a` (see above);
        static (int index, int motion)[] SqueakMotions(int a) {
            int[] m = SqueakAbilityMotions[a];
            int crouch = m[1];
            List<(int, int)> pairs = new List<(int, int)> { (0, m[0]), (1, crouch) };
            // (the slopes: the eight after the crouch, the same kind, plain or composite);
            for (int i = 0; i < 8; i++) pairs.Add((2 + i, crouch >= 0 ? crouch + 1 + i : ~(~crouch + 1 + i)));
            pairs.AddRange(new[] { (11, m[2]), (25, m[3]), (22, m[4]), (13, m[5]), (14, crouch), (15, m[6]), (16, m[7]), (24, m[8]), (17, m[8]),
                                   (41, m[9]), (31, m[10]), (32, m[11]), (33, m[12]), (36, m[13]) });
            return pairs.ToArray();
        }

        /* The Squeak Squad abilities' moves' looks ("ROM Expansion/squeak_abilities.h"'s SQUEAK_LOOK_*, in that order): an animation of the
         * ability's kirby/<ability>_ext file (or kirby/<ability>_cmn's), or with Composite its .mca's composite animation (its parts as one,
         * SqueakFlatten), drawn by the moves' pieces ("ROM Expansion/squeak_abilities.c") while Kirby's own sprite is hidden, as the
         * Hammer and Magic scrolls draw theirs: split by palette into effects, one sprite each (SqueakEffect's OnlyPalettes): its
         * palette 0's pieces first (Kirby's body, drawn with the player's own palette: his spray colors), then each other one in its
         * own (the headband, the blade, the sparks), SqueakLookLayers at most; gSqueakAbilityLooks holds each look's effects; */
        internal sealed record SqueakMoveLook(string File, int Anim = -1, int Composite = -1);
        const int SqueakLookLayers = 4;
        static readonly SqueakMoveLook[] SqueakMoveLooks = {
            // Ninja (ninja_ext): the kunai throw and its end (composites 0, 1), the dash's start (its grab: composite 4), the slash and
            // its end (5, 6), the dive's start, the dive and its landing (28, composite 7, 31), on a wall: clinging (43), crouching to
            // jump off (44), the wall jump's flip (composite 11), the kunai throw from it and its end (composites 9, 10), the grab (32),
            // the rise (34) and fall (35) with what it holds and the bounce off (composite 8), running on water (53), the kunai (48)
            // and stuck (composite 3), the scroll's art: its start, charged, let go (composites 13, 14, 15; on water 18, 19, 20) and its
            // flame pillar (16; on water the geyser, 21);
            new("kirby/ninja_ext", Composite: 0), new("kirby/ninja_ext", Composite: 1), new("kirby/ninja_ext", Composite: 4),
            new("kirby/ninja_ext", Composite: 5), new("kirby/ninja_ext", Composite: 6), new("kirby/ninja_ext", 28),
            new("kirby/ninja_ext", Composite: 7), new("kirby/ninja_ext", 31), new("kirby/ninja_ext", 43), new("kirby/ninja_ext", 44),
            new("kirby/ninja_ext", Composite: 11), new("kirby/ninja_ext", Composite: 9), new("kirby/ninja_ext", Composite: 10),
            new("kirby/ninja_ext", 32), new("kirby/ninja_ext", 34), new("kirby/ninja_ext", 35), new("kirby/ninja_ext", Composite: 8),
            new("kirby/ninja_ext", 53), new("kirby/ninja_ext", 48), new("kirby/ninja_ext", Composite: 3),
            new("kirby/ninja_ext", Composite: 13), new("kirby/ninja_ext", Composite: 14), new("kirby/ninja_ext", Composite: 15),
            new("kirby/ninja_ext", Composite: 16), new("kirby/ninja_ext", Composite: 18), new("kirby/ninja_ext", Composite: 19),
            new("kirby/ninja_ext", Composite: 20), new("kirby/ninja_ext", Composite: 21),
            // Animal (animal_ext): the scratch (34), the dash's crouch and leap (21, 22), the held flurry and the throw (24, 25), digging
            // ahead, down and up (30, 31, 32), looking up (33); the scroll's roll (35, 36), its end (37), and its spin (38, 39);
            new("kirby/animal_ext", 34), new("kirby/animal_ext", 21), new("kirby/animal_ext", 22), new("kirby/animal_ext", 24),
            new("kirby/animal_ext", 25), new("kirby/animal_ext", 30), new("kirby/animal_ext", 31), new("kirby/animal_ext", 32),
            new("kirby/animal_ext", 33), new("kirby/animal_ext", 35), new("kirby/animal_ext", 36), new("kirby/animal_ext", 37),
            new("kirby/animal_ext", 38), new("kirby/animal_ext", 39),
            // Bubble (bubble_ext): the start (22), blowing ahead, up and down (composites 16, 14, 18), their ends (17, 15, 19), the scroll's
            // big bubble blown (20); the bubbles (35, 36), the scroll's big one (37), a bubble's pop (38);
            new("kirby/bubble_ext", 22), new("kirby/bubble_ext", Composite: 16), new("kirby/bubble_ext", Composite: 14),
            new("kirby/bubble_ext", Composite: 18), new("kirby/bubble_ext", Composite: 17), new("kirby/bubble_ext", Composite: 15),
            new("kirby/bubble_ext", Composite: 19), new("kirby/bubble_ext", Composite: 20), new("kirby/bubble_ext", 35),
            new("kirby/bubble_ext", 36), new("kirby/bubble_ext", 37), new("kirby/bubble_ext", 38),
            // Metal (metal_ext): the stomp's start, fall and bounce (26, 27, 28); the scroll's roll: its start (23), fast, slow and in the
            // air (24, 34, 35), its end (25). All of Metal Kirby is metal, his palette 0 grey: drawn in its own (MoveStep);
            new("kirby/metal_ext", 26), new("kirby/metal_ext", 27), new("kirby/metal_ext", 28), new("kirby/metal_ext", 23),
            new("kirby/metal_ext", 24), new("kirby/metal_ext", 34), new("kirby/metal_ext", 35), new("kirby/metal_ext", 25),
        };

        /* SqueakMoveLookEffects(); The looks' effects (their indexes from `first` on, after SqueakEffectDefinitions') and the table of each
         * look's (SqueakLookLayers each: the body's first, 0xFFFF none, then the other palettes'); */
        static (List<SqueakEffect> effects, ushort[] table) SqueakMoveLookEffects(byte[] squeak, int first) {
            List<SqueakEffect> effects = new List<SqueakEffect>();
            ushort[] table = Enumerable.Repeat((ushort)0xFFFF, SqueakMoveLooks.Length * SqueakLookLayers).ToArray();
            Dictionary<string, SqueakCharacter> characters = new Dictionary<string, SqueakCharacter>();
            for (int i = 0; i < SqueakMoveLooks.Length; i++) {
                SqueakMoveLook look = SqueakMoveLooks[i];
                string[] path = look.File.Split('/');
                if (!characters.TryGetValue(look.File, out SqueakCharacter character))
                    characters[look.File] = character = ReadSqueakCharacter(squeak, path[1], path[0]);
                SqueakCharacter flat = character;
                SqueakAnim anim;
                if (look.Composite >= 0) (flat, anim) = SqueakFlatten(character, look.Composite, null);
                else anim = character.Anims[look.Anim];
                List<int> palettes = anim.Steps.SelectMany(st => flat.Cells[st.Cell].Pieces.Select(piece => piece.Attr2 >> 12)).Distinct()
                                         .OrderBy(pal => pal).ToList();
                // (slot 0 the body's, palette 0, none when the look has none of it; the others' after it);
                if (palettes.Count(pal => pal != 0) > SqueakLookLayers - 1)
                    throw new ArgumentException($"{look.File}'s look {i} is in more than {SqueakLookLayers - 1} palettes besides its body's");
                for (int k = 0; k < palettes.Count; k++) {
                    int slot = palettes[k] == 0 ? 0 : palettes.Contains(0) ? k : k + 1;
                    table[i * SqueakLookLayers + slot] = (ushort)(first + effects.Count);
                    effects.Add(new SqueakEffect(first + effects.Count, look.File, Math.Max(0, look.Anim), Composite: look.Composite,
                                                 OnlyPalettes: new[] { palettes[k] }));
                }
            }
            return (effects, table);
        }

        /* The Squeak Squad abilities' HUD portraits (KatAM's 40x48 cards, 5x6 tiles row by row, NidlAbilityPortrait's): the name band as
         * KatAM's (rows 0-2 and 14-15 the band's color, 0; 3 and 13 black rules; the name on black in rows 4-12, in the narrow letters of
         * KatAM's "NORMAL" and "MISSILE", 2-pixel strokes, drawn here), a black rule (16), then the picture (rows 17-47, black sides): a
         * burst in the ability's colors behind Kirby as the DS draws him with it, standing (his _cmn's standing motion, its first step).
         * Colors: 0 the band's, 1 black, 2 the name's, 3-5 the burst's (dark, mid, light), 6-15 Kirby's (his own, merged to fit); */
        static readonly Dictionary<char, string[]> PortraitLetters = new Dictionary<char, string[]> {
            ['A'] = new[] { ".###.", "##.##", "##.##", "##.##", "#####", "##.##", "##.##", "##.##", "##.##" },
            ['B'] = new[] { "####.", "##.##", "##.##", "##.##", "####.", "##.##", "##.##", "##.##", "####." },
            ['E'] = new[] { "#####", "##...", "##...", "##...", "####.", "##...", "##...", "##...", "#####" },
            ['I'] = Enumerable.Repeat("##", 9).ToArray(),
            ['J'] = Enumerable.Repeat("...##", 6).Concat(new[] { "##.##", "##.##", ".###." }).ToArray(),
            ['L'] = Enumerable.Repeat("##..", 8).Append("####").ToArray(),
            ['M'] = new[] { "##...##", "###.###", "#######", "##.#.##", "##.#.##", "##...##", "##...##", "##...##", "##...##" },
            ['N'] = new[] { "##..##", "##..##", "###.##", "###.##", "######", "##.###", "##.###", "##..##", "##..##" },
            ['T'] = Enumerable.Repeat("..##..", 8).Prepend("######").ToArray(),
            ['U'] = Enumerable.Repeat("##.##", 8).Append(".###.").ToArray(),
        };
        // (per ability: its name, then its colors as 5-bit (r, g, b): the band, the name, the burst's dark, mid and light);
        static readonly (string name, (int r, int g, int b)[] colors)[] PortraitThemes = {
            ("NINJA", new[] { (12, 4, 20), (24, 16, 31), (4, 3, 12), (8, 6, 18), (14, 11, 24) }),
            ("ANIMAL", new[] { (24, 12, 2), (31, 22, 8), (16, 8, 3), (24, 14, 5), (30, 21, 10) }),
            ("BUBBLE", new[] { (4, 18, 28), (16, 28, 31), (8, 18, 26), (14, 24, 30), (22, 29, 31) }),
            ("METAL", new[] { (12, 13, 16), (24, 26, 29), (8, 9, 12), (13, 14, 18), (19, 21, 25) }),
        };
        const int PortraitKirbyColors = 10;

        // SqueakAbilityPortraits(); Each Squeak Squad ability's portrait: its 16 colors and its 30 tiles (see above);
        static (ushort[] palette, byte[] tiles)[] SqueakAbilityPortraits(byte[] squeak) {
            (ushort[], byte[])[] portraits = new (ushort[], byte[])[SqueakAbilityFiles.Length];
            static ushort Rgb((int r, int g, int b) c) => (ushort)(c.r | c.g << 5 | c.b << 10);
            for (int a = 0; a < SqueakAbilityFiles.Length; a++) {
                (string name, (int r, int g, int b)[] colors) = PortraitThemes[a];
                int[,] card = new int[40, 48];      // (color indexes; 0 the band's color everywhere at first);

                // The name band;
                for (int x = 1; x < 39; x++) card[x, 3] = card[x, 13] = card[x, 16] = 1;
                for (int y = 4; y <= 12; y++)
                    for (int x = 0; x < 40; x++) card[x, y] = 1;
                int width = name.Sum(ch => PortraitLetters[ch][0].Length) + name.Length - 1, left = 1 + (38 - width) / 2;
                foreach (char ch in name) {
                    string[] letter = PortraitLetters[ch];
                    for (int y = 0; y < letter.Length; y++)
                        for (int x = 0; x < letter[y].Length; x++)
                            if (letter[y][x] == '#') card[left + x, 4 + y] = 2;
                    left += letter[0].Length + 1;
                }

                // The burst: 16 rays about a point under Kirby, alternately dark and mid, a light glow around him;
                const double centerX = 19.5, centerY = 33;
                for (int y = 17; y < 48; y++)
                    for (int x = 0; x < 40; x++) {
                        if (x == 0 || x == 39) { card[x, y] = 1; continue; }
                        double angle = Math.Atan2(y - centerY, x - centerX);
                        int ray = (int)((angle + Math.PI) / (2 * Math.PI) * 16) % 2;
                        card[x, y] = Math.Sqrt((x - centerX) * (x - centerX) + (y - centerY) * 1.2 * ((y - centerY) * 1.2)) < 9 ? 5 : ray == 1 ? 4 : 3;
                    }

                // Kirby: his standing motion's first step, as the DS draws him (every palette's colors, see SqueakCanvas());
                SqueakCharacter character = SqueakFlattenComposites(ReadSqueakCharacter(squeak, SqueakAbilityFiles[a] + "_cmn", "kirby"));
                int standing = SqueakAbilityMotions[a][0], plain = character.Anims.Count - character.Composites.Count;
                SqueakStep step = character.Anims[standing >= 0 ? standing : plain + ~standing].Steps.First(st => st.Frames > 0);
                Dictionary<(int palette, int color), int> colorMap = new Dictionary<(int, int), int>();
                List<ushort> rgb = new List<ushort>();
                for (int p = 0; p < character.Palettes.Count; p++)
                    for (int c = 1; c < 16; c++) { colorMap[(p, c)] = rgb.Count + 1; rgb.Add(character.Palettes[p][c]); }
                int[,] canvas = SqueakCanvas(character, step.Cell, colorMap);
                int size = canvas.GetLength(0), minX = size, maxX = -1, maxY = -1;
                for (int x = 0; x < size; x++)
                    for (int y = 0; y < size; y++)
                        if (canvas[x, y] != 0) { minX = Math.Min(minX, x); maxX = Math.Max(maxX, x); maxY = Math.Max(maxY, y); }
                if (maxX < 0) throw new InvalidOperationException($"Squeak Squad's {SqueakAbilityFiles[a]} Kirby has no standing frame.");

                /* His colors cut to PortraitKirbyColors (and black, the card's): the closest two (by their pixels' counts' weighted
                 * average) merged until they fit; a color merged with black is black; */
                Dictionary<int, int> counts = new Dictionary<int, int>();
                foreach (int value in canvas) if (value != 0) counts[value] = counts.GetValueOrDefault(value) + 1;
                List<(double r, double g, double b, double weight, bool black, List<int> members)> groups = counts.Select(kv => {
                    ushort c = rgb[kv.Key - 1];
                    return ((double)(c & 31), (double)(c >> 5 & 31), (double)(c >> 10 & 31), (double)kv.Value, false, new List<int> { kv.Key });
                }).ToList();
                groups.Add((0, 0, 0, 1e9, true, new List<int>()));
                while (groups.Count(g => !g.black) > PortraitKirbyColors) {
                    (int i, int j, double d) closest = (-1, -1, double.MaxValue);
                    for (int i = 0; i < groups.Count; i++)
                        for (int j = i + 1; j < groups.Count; j++) {
                            double d = Math.Pow(groups[i].r - groups[j].r, 2) + Math.Pow(groups[i].g - groups[j].g, 2) + Math.Pow(groups[i].b - groups[j].b, 2);
                            if (d < closest.d) closest = (i, j, d);
                        }
                    var (gi, gj) = (groups[closest.i], groups[closest.j]);
                    double w = gi.weight + gj.weight;
                    var merged = gi.black || gj.black ? (0.0, 0.0, 0.0, w, true, gi.members.Concat(gj.members).ToList())
                               : ((gi.r * gi.weight + gj.r * gj.weight) / w, (gi.g * gi.weight + gj.g * gj.weight) / w,
                                  (gi.b * gi.weight + gj.b * gj.weight) / w, w, false, gi.members.Concat(gj.members).ToList());
                    groups.RemoveAt(closest.j);
                    groups[closest.i] = merged;
                }
                ushort[] palette = new ushort[16];
                palette[0] = Rgb(colors[0]); palette[1] = 0; palette[2] = Rgb(colors[1]);
                for (int k = 0; k < 3; k++) palette[3 + k] = Rgb(colors[2 + k]);
                Dictionary<int, int> index = new Dictionary<int, int>();
                int next = 6;
                foreach (var g in groups) {
                    int at = g.black ? 1 : next++;
                    if (!g.black) palette[at] = (ushort)((int)Math.Round(g.r) | (int)Math.Round(g.g) << 5 | (int)Math.Round(g.b) << 10);
                    foreach (int member in g.members) index[member] = at;
                }
                // (his feet on row 46, in the middle);
                int offsetX = 20 - (minX + maxX + 1) / 2, offsetY = 46 - maxY;
                for (int x = minX; x <= maxX; x++)
                    for (int y = 0; y <= maxY; y++) {
                        int cx = x + offsetX, cy = y + offsetY;
                        if (canvas[x, y] != 0 && cx >= 1 && cx < 39 && cy >= 17 && cy < 48) card[cx, cy] = index[canvas[x, y]];
                    }

                byte[] tiles = new byte[30 * 32];
                for (int x = 0; x < 40; x++)
                    for (int y = 0; y < 48; y++)
                        tiles[(y / 8 * 5 + x / 8) * 32 + y % 8 * 4 + x % 8 / 2] |= (byte)(card[x, y] << (x % 2 * 4));
                portraits[a] = (palette, tiles);
            }
            return portraits;
        }

        /* BuildSqueakAbilityLooks(); Each Squeak Squad ability's body and hat anims (added to the anim lists) and tables (see above):
         * returns the body tables and hat tables, as ROM pointers; `normalBody` is Normal's body table (gUnk_0834C5D4); */
        static (int[] bodies, int[] hats) BuildSqueakAbilityLooks(byte[] rom, byte[] squeak, int normalBody, int tilesBase, int paletteBase, ref int cursor,
                                                                  int firstAnimId, List<int> animVariants, List<int> animAttributes, List<int> animOams) {
            int place = cursor;     // (Local functions can't use `cursor`);
            int[] bodies = new int[SqueakAbilityFiles.Length], hats = new int[SqueakAbilityFiles.Length];

            for (int a = 0; a < SqueakAbilityFiles.Length; a++) {
                SqueakCharacter character = SqueakFlattenComposites(ReadSqueakCharacter(squeak, SqueakAbilityFiles[a] + "_cmn", "kirby"));
                int plain = character.Anims.Count - character.Composites.Count;

                // The motions: each its steps' cells (a step drawn moved, its cell's copy with its pieces moved: the frames hold no offset);
                (int index, int motion)[] pairs = SqueakMotions(a);
                List<int> motions = pairs.Select(m => m.motion).Distinct().ToList();
                Dictionary<(int, int, int), int> moved = new Dictionary<(int, int, int), int>();
                int CellAt(SqueakStep step) {
                    if (step.X == 0 && step.Y == 0) return step.Cell;
                    if (moved.TryGetValue((step.Cell, step.X, step.Y), out int cell)) return cell;
                    SqueakCell c = character.Cells[step.Cell];
                    character.Cells.Add(new SqueakCell(c.Tile, c.Pieces.Select(piece => {
                        int x = (piece.Attr1 & 0x1FF) >= 256 ? (piece.Attr1 & 0x1FF) - 512 : piece.Attr1 & 0x1FF;
                        int y = (piece.Attr0 & 0xFF) >= 128 ? (piece.Attr0 & 0xFF) - 256 : piece.Attr0 & 0xFF;
                        return new SqueakPiece(piece.Attr0 & ~0xFF | (y + step.Y) & 0xFF, piece.Attr1 & ~0x1FF | (x + step.X) & 0x1FF, piece.Attr2);
                    }).ToList()));
                    return moved[(step.Cell, step.X, step.Y)] = character.Cells.Count - 1;
                }
                List<(List<(int cell, int frames)> steps, bool loops)> motionSteps = motions.Select(m => {
                    SqueakAnim anim = character.Anims[m >= 0 ? m : plain + ~m];
                    return (anim.Steps.Where(st => st.Frames > 0).Select(st => (CellAt(st), st.Frames)).ToList(), anim.Loops);
                }).ToList();
                List<int> cells = motionSteps.SelectMany(m => m.steps.Select(st => st.cell)).Distinct().ToList();

                // Body and hat frames: palette 0's pieces and the others' (Metal: all the hat's);
                int[] bodyPalettes = a == SqueakAbilityMetal ? new int[0] : new[] { 0 };
                var (bodyFrames, _) = SqueakFrames(SqueakSplitLayer(character, bodyPalettes, true), cells: cells);
                var (hatFrames, hatPalettes) = SqueakFrames(SqueakSplitLayer(character, bodyPalettes, false), cells: cells);
                if (hatPalettes.Count != 1) throw new ArgumentException($"Squeak Squad's {SqueakAbilityFiles[a]} hat isn't in one palette");
                if (bodyFrames.Any(f => f.tiles.Length / 32 > BodyTiles) || hatFrames.Any(f => f.tiles.Length / 32 > MotionHatTiles))
                    throw new ArgumentException($"Squeak Squad's {SqueakAbilityFiles[a]} frames don't fit Kirby's VRAM");
                // (the hat's frame 0 is none, for KatAM's indexes it has no frames for);
                hatFrames.Insert(0, (new byte[0], new ushort[0], new short[] { 0, 0, 0, 0, 0, 0 }, 0));

                int hatPaletteAt = Allocate(ref place, 32, 32, paletteBase);
                hatPalettes[0].CopyTo(rom, hatPaletteAt);
                int hatPalette = (hatPaletteAt - paletteBase) / 32;

                // Data: per anim, tiles (aligned to the tiles base), attributes and OAM (as BuildMotions());
                (int tileIndex, int[] frameTile) WriteFrames(List<(byte[] tiles, ushort[] oams, short[] attributes, int palette)> frames) {
                    int tilesAt = Allocate(ref place, Math.Max(32, frames.Sum(f => f.tiles.Length)), 32, tilesBase);
                    int attributesAt = Allocate(ref place, frames.Count * 12, 4), oamsAt = Allocate(ref place, Math.Max(2, frames.Sum(f => f.oams.Length) * 2), 4);
                    int[] frameTile = new int[frames.Count];
                    for (int i = 0, tile = 0, oam = 0; i < frames.Count; i++) {
                        (byte[] tiles, ushort[] oams, short[] attributes, int _) = frames[i];
                        tiles.CopyTo(rom, tilesAt + tile * 32);
                        WriteShorts(rom, attributesAt + i * 12, (short)(oam / 3), attributes[1], attributes[2], attributes[3], attributes[4], attributes[5]);
                        for (int o = 0; o < oams.Length; o++) WriteShort(rom, oamsAt + (oam + o) * 2, oams[o]);
                        frameTile[i] = tile;
                        tile += tiles.Length / 32;
                        oam += oams.Length;
                    }
                    animAttributes.Add(attributesAt);
                    animOams.Add(oamsAt);
                    return ((tilesAt - tilesBase) / 32, frameTile);
                }

                /* Commands per motion: the body's GetPalette (as KatAM's) or the hat's own, GetTiles when the frame has any, ShowFrame; then a
                 * JumpBack to the first (a looping animation) or End (held on its last frame); then (the hat's) none, frame 0, four times (the warp
                 * star's riders: KatAM adds the player's number to the variant of index 0x5A, sub_0803F790); */
                int WriteVariants(List<(byte[] tiles, ushort[] oams, short[] attributes, int palette)> frames, (int tileIndex, int[] frameTile) data, bool hat) {
                    int variantsAt = Allocate(ref place, (motions.Count + 4) * 4, 4), shift = hat ? 1 : 0;
                    for (int m = 0; m < motions.Count + 4; m++) {
                        List<int> words = new List<int> { -2, hat ? hatPalette : 0, 16 };
                        if (m >= motions.Count) {
                            words.AddRange(new[] { 1, 0, -4 });
                        } else {
                            int first = words.Count;
                            foreach ((int cell, int delay) in motionSteps[m].steps) {
                                int index = cells.IndexOf(cell) + shift;
                                if (frames[index].tiles.Length > 0) words.AddRange(new[] { -1, data.tileIndex + data.frameTile[index], frames[index].tiles.Length / 32 });
                                words.AddRange(new[] { delay, index });
                            }
                            if (motionSteps[m].loops && motionSteps[m].steps.Count > 1) words.AddRange(new[] { -3, words.Count - first });
                            else words.Add(-4);
                        }
                        int at = Allocate(ref place, words.Count * 4, 4);
                        for (int w = 0; w < words.Count; w++) WriteInt(rom, at + w * 4, words[w]);
                        WriteInt(rom, variantsAt + m * 4, at + 0x08000000);
                    }
                    return variantsAt;
                }

                int bodyAnim = firstAnimId + animVariants.Count;
                var bodyData = WriteFrames(bodyFrames);
                animVariants.Add(WriteVariants(bodyFrames, bodyData, false));
                int hatAnim = firstAnimId + animVariants.Count;
                var hatData = WriteFrames(hatFrames);
                animVariants.Add(WriteVariants(hatFrames, hatData, true));

                // The tables: Normal's body with the motions (Metal's too: his body sprite's frames are empty there), the hat's motions or none;
                int bodyTable = Allocate(ref place, AnimationIndexCount * 4, 4), hatTable = Allocate(ref place, AnimationIndexCount * 4, 4);
                Array.Copy(rom, normalBody, rom, bodyTable, AnimationIndexCount * 4);
                for (int i = 0; i < AnimationIndexCount; i++) {
                    WriteShort(rom, hatTable + i * 4, hatAnim);
                    rom[hatTable + i * 4 + 2] = (byte)motions.Count;
                }
                foreach ((int index, int motion) in pairs) {
                    int m = motions.IndexOf(motion);
                    WriteShort(rom, bodyTable + index * 4, bodyAnim);
                    rom[bodyTable + index * 4 + 2] = (byte)m;
                    rom[hatTable + index * 4 + 2] = (byte)m;
                }
                bodies[a] = bodyTable + 0x08000000;
                hats[a] = hatTable + 0x08000000;
            }
            cursor = place;
            return (bodies, hats);
        }
    }
}
