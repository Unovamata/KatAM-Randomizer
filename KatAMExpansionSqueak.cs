using System.Text;

namespace KatAMRandomizer {
    /* Kirby: Squeak Squad's graphics, read from the player's own Squeak Squad ROM (Nintendo DS, any release: their graphics are the
     * same), never shipped. An NDS ROM's files are found through its file name table (FNT, header +0x40) and allocation table (FAT,
     * header +0x48). Each character has HAL's own files (see "Documentation/Squeak Squad Notes.md"): .cga tiles (4 bits a pixel),
     * .cla palettes (16 colors each), .cea cells (a list of the DS's OAM pieces) and animations ((cell, frames) steps). Each cell
     * becomes a KatAM frame: drawn on a canvas from its OAM pieces, then cut into KatAM's pieces (CutFrame); */
    internal static partial class KatAMExpansion {
        // The releases (USA, Europe: "Mouse Attack", Japan: "Sanjou! Dorotche Dan");
        static readonly string[] SqueakGameCodes = { "AKWE", "AKWP", "AKWJ" };

        // IsSqueakSquad(); A Kirby: Squeak Squad ROM, whose files the expansion can read;
        public static bool IsSqueakSquad(byte[] rom) {
            try {
                return rom != null && rom.Length > 0x4000 && SqueakGameCodes.Contains(Encoding.ASCII.GetString(rom, 0x0C, 4))
                       && SqueakFile(rom, "enemy/waddledee.cea") != null;
            } catch (Exception exception) when (exception is ArgumentException or IndexOutOfRangeException) {
                return false;
            }
        }

        /* SqueakFile(); A file of the ROM's file system by its path ("enemy/waddledee.cga"), or null. The FNT starts with 8 bytes per
         * directory (its entries' offset from the FNT, its first file's ID, ...), the root's first; an entry is a byte (its name's
         * length, bit 7 a directory), the name, and for a directory its ID (0xF000 + its index); the FAT holds each file's start and
         * end; ignoreCase: names matched without their case (a stage's .mxi names its files in lower case, some stored in upper); */
        static byte[] SqueakFile(byte[] rom, string path, bool ignoreCase = false) {
            int fnt = ReadInt(rom, 0x40), fat = ReadInt(rom, 0x48), directory = 0;
            string[] parts = path.Split('/');

            for (int p = 0; p < parts.Length; p++) {
                int at = fnt + ReadInt(rom, fnt + directory * 8), file = ReadShort(rom, fnt + directory * 8 + 4);
                bool found = false;

                for (int head = rom[at++]; head != 0; head = rom[at++]) {
                    int length = head & 0x7F;
                    bool isDirectory = (head & 0x80) != 0;
                    string name = Encoding.ASCII.GetString(rom, at, length);
                    int sub = 0;

                    at += length;
                    if (isDirectory) {
                        sub = ReadShort(rom, at) & 0xFFF;
                        at += 2;
                    }
                    if (string.Equals(name, parts[p], ignoreCase ? StringComparison.OrdinalIgnoreCase : StringComparison.Ordinal)) {
                        if (p == parts.Length - 1) {
                            if (isDirectory) return null;
                            int start = ReadInt(rom, fat + file * 8), end = ReadInt(rom, fat + file * 8 + 4);
                            return rom[start..end];
                        }
                        if (!isDirectory) return null;
                        directory = sub;
                        found = true;
                        break;
                    }
                    if (!isDirectory) file++;
                }
                if (!found) return null;
            }
            return null;
        }

        // A DS OAM piece of a cell: its attributes (attr2's tile counted in 4-tile steps from the cell's first tile);
        internal sealed record SqueakPiece(int Attr0, int Attr1, int Attr2);
        // A cell: its first tile and pieces;
        internal sealed record SqueakCell(int Tile, List<SqueakPiece> Pieces);
        // An animation step: the cell, how many frames it shows, and where it's drawn (x, y: pixels added to the position, y down);
        internal sealed record SqueakStep(int Cell, int Frames, short X, short Y);
        internal sealed record SqueakAnim(bool Loops, List<SqueakStep> Steps);
        /* A part of a composition (.mca): the .cea animation it plays, the time it plays it at (Absolute: the frames since the
         * composite animation started; else its time within it, 0x01FFA6CC's 0 and 1), and where (pixels, as the character faces
         * right, y down); */
        internal sealed record SqueakPart(int Anim, bool Absolute, short X, short Y);
        // A composite animation: compositions (their parts drawn together, the first on top), each a number of frames;
        internal sealed record SqueakComposite(bool Loops, List<(int Composition, int Frames)> Steps);

        // A character's graphics: tiles, palettes (16 colors each), cells and animations, and its composite ones (.mca), if any;
        internal sealed class SqueakCharacter {
            public byte[] Tiles;
            public List<ushort[]> Palettes;
            public List<SqueakCell> Cells;
            public List<SqueakAnim> Anims;
            public List<List<SqueakPart>> Compositions = new List<List<SqueakPart>>();
            public List<SqueakComposite> Composites = new List<SqueakComposite>();
            // (cells made for an affine animation's steps: a cell's copy drawn turned by its angle, in 1/4096 turns: SqueakCanvas());
            public Dictionary<int, int> Angles = new Dictionary<int, int>();
        }

        /* SqueakMerge(); Two files' graphics as one character: the second's tiles, palettes, cells and animations after the first's (its
         * cells' tiles and pieces' palettes moved on), and its composite animations (.mca) after the first's (their parts' animations
         * and steps' compositions moved on). Palettes that don't all fit share: the second's palettes the same as one of the first's
         * are that one (Mrs. Moley's three files have the same 16); */
        static SqueakCharacter SqueakMerge(SqueakCharacter a, SqueakCharacter b, bool sharePalettes = false) {
            int tiles = a.Tiles.Length / 32, cells = a.Cells.Count, anims = a.Anims.Count, compositions = a.Compositions.Count;
            List<ushort[]> palettes = a.Palettes.ToList();
            int[] paletteOf;

            // (the second's pieces in the first's palettes, by index: one palette object for both, Squeak Squad's);
            if (sharePalettes) {
                paletteOf = Enumerable.Range(0, b.Palettes.Count).Select(p => Math.Min(p, palettes.Count - 1)).ToArray();
            } else if (palettes.Count + b.Palettes.Count <= 16) {
                paletteOf = Enumerable.Range(palettes.Count, b.Palettes.Count).ToArray();
                palettes.AddRange(b.Palettes);
            } else {
                paletteOf = b.Palettes.Select(p => {
                    int same = palettes.FindIndex(q => q.SequenceEqual(p));
                    if (same >= 0) return same;
                    palettes.Add(p);
                    return palettes.Count - 1;
                }).ToArray();
            }
            if (palettes.Count > 16) throw new InvalidOperationException("Too many palettes to merge.");
            return new SqueakCharacter {
                Tiles = a.Tiles.Concat(b.Tiles).ToArray(),
                Palettes = palettes,
                Cells = a.Cells.Concat(b.Cells.Select(c => new SqueakCell(c.Tile + tiles,
                    c.Pieces.Select(p => p with { Attr2 = p.Attr2 & 0xFFF | paletteOf[p.Attr2 >> 12] << 12 }).ToList()))).ToList(),
                Anims = a.Anims.Concat(b.Anims.Select(an => an with { Steps = an.Steps.Select(st => st with { Cell = st.Cell + cells }).ToList() })).ToList(),
                Compositions = a.Compositions.Concat(b.Compositions.Select(c => c.Select(part => part with { Anim = part.Anim + anims }).ToList())).ToList(),
                Composites = a.Composites.Concat(b.Composites.Select(cm => cm with {
                    Steps = cm.Steps.Select(st => (st.Composition + compositions, st.Frames)).ToList() })).ToList(),
            };
        }

        /* SqueakSplitLayer(); The character's cells without the pieces of `palettes` (layer: FALSE) or with only them (TRUE), its
         * animations and composite ones (.mca) kept; `whole` (WholeCells: the layers' groups), a cell drawn wholly with one group's
         * palettes stays whole, the character's own (none of it the layer's); */
        static SqueakCharacter SqueakSplitLayer(SqueakCharacter c, int[] palettes, bool layer, int[][] whole = null) {
            bool Whole(SqueakCell cell) => whole != null && cell.Pieces.Count > 0
                                           && whole.Any(g => cell.Pieces.All(p => g.Contains(p.Attr2 >> 12)));
            return new SqueakCharacter {
                Tiles = c.Tiles, Palettes = c.Palettes, Anims = c.Anims, Compositions = c.Compositions, Composites = c.Composites,
                Cells = c.Cells.Select(cell => new SqueakCell(cell.Tile, Whole(cell) ? (layer ? new List<SqueakPiece>() : cell.Pieces)
                                                                       : cell.Pieces.Where(p => palettes.Contains(p.Attr2 >> 12) == layer).ToList())).ToList(),
            };
        }

        /* SqueakKeepAnims(); The character with only these animations (as its 0, 1...) and the cells they show (renumbered in the order
         * they first show), its tiles and palettes as they were. Its composite animations are kept (keepComposites: Mrs. Moley's, their
         * parts' animations then among those kept, renumbered; those with a part not kept left out, the others renumbered: Meta Knight's),
         * else dropped; */
        /* SqueakRolled(); The character with an animation more: its cell `cell` turned a step of 1/`steps` turn further each step (the
         * rolling boulder's roll: the DS turns its one frame, `yakumono/ball_rock`'s animation 0, as its body rolls; squeak/gimmicks.c
         * picks the step by how far it rolled), each turned copy a cell of its own drawn turned (SqueakCanvas()); */
        static SqueakCharacter SqueakRolled(SqueakCharacter character, int cell, int steps) {
            List<SqueakStep> roll = new List<SqueakStep>();
            for (int s = 0; s < steps; s++) {
                int turned = cell;
                if (s > 0) {
                    turned = character.Cells.Count;
                    character.Cells.Add(character.Cells[cell]);
                    character.Angles[turned] = s * 4096 / steps;
                }
                roll.Add(new SqueakStep(turned, 1, 0, 0));
            }
            character.Anims.Add(new SqueakAnim(true, roll));
            return character;
        }

        static SqueakCharacter SqueakKeepAnims(SqueakCharacter character, int[] anims, bool keepComposites = false) {
            List<int> cells = anims.SelectMany(a => character.Anims[a].Steps.Select(s => s.Cell)).Distinct().ToList();
            List<int> compositions = keepComposites
                ? Enumerable.Range(0, character.Compositions.Count).Where(c => character.Compositions[c].All(part => anims.Contains(part.Anim))).ToList()
                : new List<int>();
            List<SqueakComposite> composites = character.Composites.Where(cm => cm.Steps.All(st => compositions.Contains(st.Composition)))
                .Select(cm => cm with { Steps = cm.Steps.Select(st => (compositions.IndexOf(st.Composition), st.Frames)).ToList() }).ToList();
            return new SqueakCharacter {
                Tiles = character.Tiles, Palettes = character.Palettes, Cells = cells.Select(c => character.Cells[c]).ToList(),
                Anims = anims.Select(a => character.Anims[a] with {
                    Steps = character.Anims[a].Steps.Select(s => s with { Cell = cells.IndexOf(s.Cell) }).ToList() }).ToList(),
                Compositions = compositions.Select(c => character.Compositions[c].Select(part => part with { Anim = Array.IndexOf(anims, part.Anim) }).ToList()).ToList(),
                Composites = keepComposites ? composites : new List<SqueakComposite>(),
            };
        }

        /* SqueakFlattenComposites(); The character's composite animations (.mca) as plain ones, after its own (anim count + k: its composite
         * k), for a gimmick's one sprite (squeak/gimmicks.c plays plain animations only): frame by frame, each composition's parts at
         * their time (an absolute part the frames since the composite started, else since its step did; its animation looped, or held
         * on its last step), their cells' pieces in one cell (the first part's first: on top, as the DS draws them), moved by the part's
         * offset and its step's, the same parts' cells in a row one step. Cells' own tiles must start on 4-tile bounds (the pieces count
         * their tiles from the cell's in 4s); a composite that can't is left out (its animation an empty one); */
        static SqueakCharacter SqueakFlattenComposites(SqueakCharacter character) {
            List<SqueakCell> cells = character.Cells.ToList();
            List<SqueakAnim> anims = character.Anims.ToList();
            Dictionary<string, int> made = new Dictionary<string, int>();
            (int cell, int x, int y) PartAt(SqueakAnim anim, int time) {
                int total = anim.Steps.Sum(st => st.Frames);
                if (anim.Steps.Count == 0 || total <= 0) return (-1, 0, 0);
                if (anim.Loops) time %= total;
                foreach (SqueakStep st in anim.Steps) {
                    if (time < st.Frames) return (st.Cell, st.X, st.Y);
                    time -= st.Frames;
                }
                SqueakStep last = anim.Steps[^1];
                return (last.Cell, last.X, last.Y);
            }
            foreach (SqueakComposite composite in character.Composites) {
                List<SqueakStep> steps = new List<SqueakStep>();
                int time = 0;
                bool ok = true;
                foreach ((int composition, int frames) in composite.Steps) {
                    List<SqueakPart> parts = composition < character.Compositions.Count ? character.Compositions[composition] : new List<SqueakPart>();
                    for (int f = 0; f < frames && ok; f++, time++) {
                        List<(int cell, int x, int y)> drawn = parts.Select(part => {
                            (int cell, int x, int y) at = part.Anim < character.Anims.Count ? PartAt(character.Anims[part.Anim], part.Absolute ? time : f) : (-1, 0, 0);
                            return (at.cell, at.x + part.X, at.y + part.Y);
                        }).ToList();
                        string key = string.Join(";", drawn.Select(d => $"{d.cell},{d.x},{d.y}"));
                        if (!made.TryGetValue(key, out int index)) {
                            List<SqueakPiece> pieces = new List<SqueakPiece>();
                            foreach ((int cell, int x, int y) in drawn) {
                                if (cell < 0 || cell >= character.Cells.Count) continue;
                                SqueakCell c = character.Cells[cell];
                                if (c.Tile % 4 != 0 || character.Angles.ContainsKey(cell)) { ok = false; break; }
                                foreach (SqueakPiece piece in c.Pieces) {
                                    int px = (piece.Attr1 & 0x1FF) >= 256 ? (piece.Attr1 & 0x1FF) - 512 : piece.Attr1 & 0x1FF;
                                    int py = (piece.Attr0 & 0xFF) >= 128 ? (piece.Attr0 & 0xFF) - 256 : piece.Attr0 & 0xFF;
                                    pieces.Add(new SqueakPiece(piece.Attr0 & ~0xFF | (py + y) & 0xFF, piece.Attr1 & ~0x1FF | (px + x) & 0x1FF,
                                                               piece.Attr2 & ~0x3FF | ((piece.Attr2 & 0x3FF) + c.Tile / 4) & 0x3FF));
                                }
                            }
                            index = cells.Count;
                            cells.Add(new SqueakCell(0, pieces));
                            made[key] = index;
                        }
                        if (steps.Count > 0 && steps[^1].Cell == index) steps[^1] = steps[^1] with { Frames = steps[^1].Frames + 1 };
                        else steps.Add(new SqueakStep(index, 1, 0, 0));
                    }
                }
                anims.Add(new SqueakAnim(composite.Loops, ok ? steps : new List<SqueakStep>()));
            }
            return new SqueakCharacter {
                Tiles = character.Tiles, Palettes = character.Palettes, Cells = cells, Anims = anims,
                Compositions = character.Compositions, Composites = character.Composites, Angles = character.Angles,
            };
        }

        /* SqueakMergeParts(); (MergeParts) A composition's parts that play in step (the same time, absolute or not, their animations'
         * steps as long and looping alike), but its last (the body), drawn by one part per run (at most 128 pixels across, and
         * `canvas`): an animation of their own, each step one cell of their cells' pieces at the parts' offsets and their steps' (from
         * the middle of the run's pieces, where the part goes), the first part's on top; the same picture by fewer sprites (each a
         * KatAM effect object, a VRAM slot and its tiles a frame: Dark Daroach's beam, 16 parts, is 2 and its 7 sparkles; the owner:
         * "Massive amounts of lag whenever the Squeak Squad boss invokes stars."). Parts whose cells' tiles don't start on 4-tile bounds
         * (the pieces count their tiles from the cell's in 4s), turned ones (Angles), or whose pieces aren't all in one palette stay as
         * they are; */
        static SqueakCharacter SqueakMergeParts(SqueakCharacter character, int canvas) {
            List<SqueakCell> cells = character.Cells.ToList();
            // (the box the parts' pieces take, in all their steps, from the character's spot: x right, y down);
            (int x0, int x1, int y0, int y1) Extent(List<SqueakPart> parts, IEnumerable<int> members) {
                int x0 = int.MaxValue, x1 = int.MinValue, y0 = int.MaxValue, y1 = int.MinValue;
                foreach (int k in members)
                    foreach (SqueakStep st in character.Anims[parts[k].Anim].Steps)
                        foreach (SqueakPiece piece in character.Cells[st.Cell].Pieces) {
                            (int w, int h) = SqueakPieceSizes[piece.Attr0 >> 14, piece.Attr1 >> 14];
                            int px = ((piece.Attr1 & 0x1FF) >= 256 ? (piece.Attr1 & 0x1FF) - 512 : piece.Attr1 & 0x1FF) + parts[k].X + st.X;
                            int py = ((piece.Attr0 & 0xFF) >= 128 ? (piece.Attr0 & 0xFF) - 256 : piece.Attr0 & 0xFF) + parts[k].Y + st.Y;
                            x0 = Math.Min(x0, px);
                            x1 = Math.Max(x1, px + w);
                            y0 = Math.Min(y0, py);
                            y1 = Math.Max(y1, py + h);
                        }
                return (x0, x1, y0, y1);
            }
            List<SqueakAnim> anims = character.Anims.ToList();
            Dictionary<string, int> made = new Dictionary<string, int>();
            bool Mergeable(SqueakPart part) {
                if (part.Anim >= character.Anims.Count || character.Anims[part.Anim].Steps.Count == 0) return false;
                return character.Anims[part.Anim].Steps.All(st => st.Cell < character.Cells.Count && character.Cells[st.Cell].Tile % 4 == 0
                                                                  && !character.Angles.ContainsKey(st.Cell));
            }
            string Timing(SqueakPart part) {
                SqueakAnim a = character.Anims[part.Anim];
                return $"{part.Absolute}:{a.Loops}:{string.Join(",", a.Steps.Select(st => st.Frames))}";
            }
            IEnumerable<int> Palettes(SqueakPart part) =>
                character.Anims[part.Anim].Steps.SelectMany(st => character.Cells[st.Cell].Pieces.Select(piece => piece.Attr2 >> 12));
            List<List<SqueakPart>> compositions = new List<List<SqueakPart>>();
            foreach (List<SqueakPart> parts in character.Compositions) {
                List<SqueakPart> result = new List<SqueakPart>();
                HashSet<int> done = new HashSet<int>();
                for (int i = 0; i < parts.Count; i++) {
                    if (done.Contains(i)) continue;
                    // (the group: the parts after it in step with it, the body (the last) left alone);
                    List<int> group = new List<int> { i };
                    if (i < parts.Count - 1 && Mergeable(parts[i]))
                        for (int k = i + 1; k < parts.Count - 1; k++)
                            if (!done.Contains(k) && Mergeable(parts[k]) && Timing(parts[k]) == Timing(parts[i])) group.Add(k);
                    if (group.Count > 1 && group.SelectMany(k => Palettes(parts[k])).Distinct().Count() > 1) group = new List<int> { i };
                    if (group.Count == 1) {
                        result.Add(parts[i]);
                        continue;
                    }
                    foreach (int k in group) done.Add(k);
                    /* (cut in runs at most 128 pixels across (and the canvas), left to right, each drawn about the middle of its
                     * pieces: Dark Daroach's beam's parts reach 248 pixels, a 256 canvas about their average cut its end at his wand
                     * off; a sprite of his whole beam, 183 tiles, found no hole that big in KatAM's VRAM heap for half a second, his
                     * stars' and bombs' sprites between, two of 90 do); */
                    List<List<int>> runs = new List<List<int>>();
                    List<int> run = new List<int>();
                    foreach (int k in group.OrderBy(k => parts[k].X)) {
                        List<int> trial = run.Append(k).ToList();
                        (int x0, int x1, int y0, int y1) = Extent(parts, trial);
                        if (run.Count > 0 && (x1 - x0 > Math.Min(canvas - 8, 128) || y1 - y0 > Math.Min(canvas - 8, 128))) {
                            runs.Add(run);
                            run = new List<int> { k };
                        } else run = trial;
                    }
                    runs.Add(run);
                    foreach (List<int> members in runs.Select(r => r.OrderBy(k => k).ToList())) {
                        if (members.Count == 1) {
                            result.Add(parts[members[0]]);
                            continue;
                        }
                        (int minX, int maxX, int minY, int maxY) = Extent(parts, members);
                        int cx = (minX + maxX) / 2, cy = (minY + maxY) / 2;
                        string key = string.Join(";", members.Select(k => $"{parts[k].Anim},{parts[k].X - cx},{parts[k].Y - cy}")) + $"|{Timing(parts[i])}";
                        if (!made.TryGetValue(key, out int anim)) {
                            SqueakAnim first = character.Anims[parts[members[0]].Anim];
                            List<SqueakStep> steps = new List<SqueakStep>();
                            for (int s = 0; s < first.Steps.Count; s++) {
                                List<SqueakPiece> pieces = new List<SqueakPiece>();
                                /* (the merged cell's first tile its first cell's of them all, each piece's tile counted from it: a
                                 * piece's 10 bits reach 4096 tiles on, Daroach's beam's cells past that in his two files' tiles); */
                                int tile = members.Min(k => character.Cells[character.Anims[parts[k].Anim].Steps[s].Cell].Tile);
                                foreach (int k in members) {
                                    SqueakStep st = character.Anims[parts[k].Anim].Steps[s];
                                    SqueakCell c = character.Cells[st.Cell];
                                    int x = parts[k].X - cx + st.X, y = parts[k].Y - cy + st.Y;
                                    foreach (SqueakPiece piece in c.Pieces) {
                                        int px = (piece.Attr1 & 0x1FF) >= 256 ? (piece.Attr1 & 0x1FF) - 512 : piece.Attr1 & 0x1FF;
                                        int py = (piece.Attr0 & 0xFF) >= 128 ? (piece.Attr0 & 0xFF) - 256 : piece.Attr0 & 0xFF;
                                        int at = (piece.Attr2 & 0x3FF) + (c.Tile - tile) / 4;
                                        if (at > 0x3FF) throw new InvalidOperationException("A merged part's cells are too far apart in the tiles.");
                                        pieces.Add(new SqueakPiece(piece.Attr0 & ~0xFF | (py + y) & 0xFF, piece.Attr1 & ~0x1FF | (px + x) & 0x1FF,
                                                                   piece.Attr2 & ~0x3FF | at));
                                    }
                                }
                                steps.Add(new SqueakStep(cells.Count, first.Steps[s].Frames, 0, 0));
                                cells.Add(new SqueakCell(tile, pieces));
                            }
                            anim = anims.Count;
                            anims.Add(new SqueakAnim(first.Loops, steps));
                            made[key] = anim;
                        }
                        result.Add(new SqueakPart(anim, parts[i].Absolute, (short)cx, (short)cy));
                    }
                }
                compositions.Add(result);
            }
            return new SqueakCharacter {
                Tiles = character.Tiles, Palettes = character.Palettes, Cells = cells, Anims = anims,
                Compositions = compositions, Composites = character.Composites, Angles = character.Angles,
            };
        }

        /* SqueakLowParts(); The character's compositions without their parts placed `high` pixels or more over its spot (SqueakEnemy's
         * HighParts: the first left the character's own sprite's part); */
        static SqueakCharacter SqueakLowParts(SqueakCharacter character, int high) {
            return new SqueakCharacter {
                Tiles = character.Tiles, Palettes = character.Palettes, Cells = character.Cells, Anims = character.Anims,
                Compositions = character.Compositions.Select(c => c.Where(part => part.Y > -high).ToList()).ToList(),
                Composites = character.Composites, Angles = character.Angles,
            };
        }

        // SqueakShiftCells(); The character's cells drawn `y` pixels lower (its pieces' y, attr0's low byte);
        static SqueakCharacter SqueakShiftCells(SqueakCharacter character, int y) {
            return new SqueakCharacter {
                Tiles = character.Tiles, Palettes = character.Palettes, Anims = character.Anims, Compositions = character.Compositions,
                Composites = character.Composites,
                Cells = character.Cells.Select(c => c with { Pieces = c.Pieces.Select(p => p with { Attr0 = p.Attr0 & ~0xFF | (p.Attr0 + y) & 0xFF }).ToList() }).ToList(),
            };
        }

        // ReadSqueakCharacter(); A character's graphics from its files (folder/name.cga, .cla, .cea; palette: another .cla, "name_blue");
        static SqueakCharacter ReadSqueakCharacter(byte[] rom, string name, string folder = "enemy", string palette = null) {
            byte[] cga = SqueakFile(rom, $"{folder}/{name}.cga"), cla = SqueakFile(rom, $"{folder}/{palette ?? name}.cla"),
                   cea = SqueakFile(rom, $"{folder}/{name}.cea");

            if (cga == null || cla == null || cea == null) throw new InvalidOperationException($"Squeak Squad's {folder}/{name} is missing.");
            if (Encoding.ASCII.GetString(cga, 0, 4) != "BCGA" || Encoding.ASCII.GetString(cla, 0, 4) != "BCLA" || Encoding.ASCII.GetString(cea, 0, 4) != "BCEA")
                throw new InvalidOperationException($"Squeak Squad's {folder}/{name} isn't HAL's sprite format.");

            SqueakCharacter character = new SqueakCharacter { Tiles = cga[0x14..(0x14 + ReadInt(cga, 0x10))], Palettes = new List<ushort[]>() };
            int colors = ReadInt(cla, 8);
            for (int p = 0; p < colors / 16; p++) character.Palettes.Add(Enumerable.Range(0, 16).Select(c => (ushort)ReadShort(cla, 12 + (p * 16 + c) * 2)).ToArray());

            /* Cells: a count, then each one's entry after an 8-byte head; the pieces' list (a count, then 8 bytes each) is at the head's
             * offset from the cells'. With the header's flag 8 (the characters' files), an entry is 8 bytes (pieces, first piece, first
             * tile, tiles): each cell has its own tiles; without it (the shared effects' files), 4 bytes (pieces, first piece): the
             * pieces count their tiles from the file's first; */
            bool ownTiles = (cea[8] & 8) != 0;
            int cells = ReadInt(cea, 0x0C), anims = ReadInt(cea, 0x10), pieceList = cells + ReadInt(cea, cells + 4), entrySize = ownTiles ? 8 : 4;
            List<SqueakPiece> pieces = Enumerable.Range(0, ReadInt(cea, pieceList))
                .Select(i => new SqueakPiece(ReadShort(cea, pieceList + 4 + i * 8), ReadShort(cea, pieceList + 6 + i * 8), ReadShort(cea, pieceList + 8 + i * 8))).ToList();
            character.Cells = Enumerable.Range(0, ReadShort(cea, cells)).Select(c => {
                int entry = cells + 8 + c * entrySize;
                return new SqueakCell(ownTiles ? ReadShort(cea, entry + 4) : 0, pieces.GetRange(ReadShort(cea, entry + 2), ReadShort(cea, entry)));
            }).ToList();

            // Animations: a count, then each one's offset (from the animations') after an 8-byte head; one: its step count (a byte), loop
            // flag (a byte), 2 bytes, its length, then 8 bytes a step (cell, frames, a word);
            /* (an affine one, its head's third byte set: its steps in pairs, the step drawn and then its angle (1/4096 turns), its x and y
             * scales (0x1000 for 1.0): the falling rock's spin, the boulder's roll; each step's cell at its angle a cell of its own, the
             * cell's copy, turned as it's drawn: SqueakCanvas(); the scales are all 1.0 in the gimmicks'; the gimmicks' graphics only, the
             * characters already ported kept as they were); */
            Dictionary<(int cell, int angle), int> turnedCells = new Dictionary<(int, int), int>();
            character.Anims = Enumerable.Range(0, ReadShort(cea, anims)).Select(a => {
                int at = anims + ReadInt(cea, anims + 8 + a * 4);
                List<SqueakStep> steps = Enumerable.Range(0, cea[at])
                    .Select(s => new SqueakStep(ReadShort(cea, at + 8 + s * 8), ReadShort(cea, at + 10 + s * 8),
                                                   (short)ReadShort(cea, at + 12 + s * 8), (short)ReadShort(cea, at + 14 + s * 8))).ToList();
                if (cea[at + 2] != 0 && folder == "yakumono") {
                    List<SqueakStep> drawn = new List<SqueakStep>();
                    for (int s = 0; s + 1 < steps.Count; s += 2) {
                        SqueakStep step = steps[s];
                        int angle = steps[s + 1].Cell & 0xFFF;
                        if (angle != 0 && step.Cell < character.Cells.Count) {
                            if (!turnedCells.TryGetValue((step.Cell, angle), out int turned)) {
                                turned = character.Cells.Count;
                                character.Cells.Add(character.Cells[step.Cell]);
                                character.Angles[turned] = angle;
                                turnedCells[(step.Cell, angle)] = turned;
                            }
                            step = step with { Cell = turned };
                        }
                        drawn.Add(step);
                    }
                    steps = drawn;
                }
                return new SqueakAnim(cea[at + 1] != 0, steps);
            }).ToList();

            /* Composite animations (.mca, "BMCA"): the compositions at +8 (a count, then each one's parts and their offset from the
             * compositions'; a part: its .cea animation, how it's timed, x, y), the animations at +0xC (a count, then each one's
             * offset from the animations' after an 8-byte head; one: its step count (a byte), loop flag (a byte), 2 bytes, its
             * length, then 8 bytes a step: composition, frames, a word); */
            byte[] mca = SqueakFile(rom, $"{folder}/{name}.mca");
            if (mca != null && Encoding.ASCII.GetString(mca, 0, 4) == "BMCA") {
                int compositions = ReadInt(mca, 8), composites = ReadInt(mca, 0x0C);
                character.Compositions = Enumerable.Range(0, ReadInt(mca, compositions)).Select(c => {
                    int at = compositions + ReadInt(mca, compositions + 8 + c * 8);
                    return Enumerable.Range(0, ReadInt(mca, compositions + 4 + c * 8))
                        .Select(p => new SqueakPart(ReadShort(mca, at + p * 8), ReadShort(mca, at + 2 + p * 8) != 0,
                                                    (short)ReadShort(mca, at + 4 + p * 8), (short)ReadShort(mca, at + 6 + p * 8))).ToList();
                }).ToList();
                character.Composites = Enumerable.Range(0, ReadShort(mca, composites)).Select(a => {
                    int at = composites + ReadInt(mca, composites + 8 + a * 4);
                    return new SqueakComposite(mca[at + 1] != 0, Enumerable.Range(0, mca[at])
                        .Select(s => (ReadShort(mca, at + 8 + s * 8), ReadShort(mca, at + 10 + s * 8))).ToList());
                }).ToList();
            }
            return character;
        }

        /* ReadSqueakRaw(); A character of one still frame from tiles in any of Squeak Squad's files ("folder/file@tiles,palette,width,
         * height", hexadecimal offsets, pixels: 4bpp 8x8 tiles in rows, then 16 colors), its origin at the frame's bottom middle: the
         * stage's own pictures no sprite file has (Dark Nebula's burning floor: the stage's fire tile, map/25_f.bin's and the others'); */
        static SqueakCharacter ReadSqueakRaw(byte[] rom, string spec) {
            string[] at = spec.Split('@'), values = at[1].Split(',');
            byte[] file = SqueakFile(rom, at[0]) ?? throw new InvalidOperationException($"Squeak Squad's {at[0]} is missing.");
            int tiles = Convert.ToInt32(values[0], 16), palette = Convert.ToInt32(values[1], 16), width = int.Parse(values[2]), height = int.Parse(values[3]);
            // (one piece: 16x16, 32x32... a square, its tiles in order as the DS's 1D mapping reads them);
            int size = width switch { 8 => 0, 16 => 1, 32 => 2, _ => 3 };
            if (width != height) throw new InvalidOperationException($"{spec}: only square frames.");
            return new SqueakCharacter {
                Tiles = file[tiles..(tiles + width * height / 2)],
                Palettes = new List<ushort[]> { Enumerable.Range(0, 16).Select(c => (ushort)ReadShort(file, palette + c * 2)).ToArray() },
                Cells = new List<SqueakCell> { new SqueakCell(0, new List<SqueakPiece> {
                    new SqueakPiece(-height & 0xFF, (-width / 2 & 0x1FF) | size << 14, 0) }) },
                Anims = new List<SqueakAnim> { new SqueakAnim(true, new List<SqueakStep> { new SqueakStep(0, 1, 0, 0) }) },
                Compositions = new List<List<SqueakPart>>(), Composites = new List<SqueakComposite>(),
            };
        }

        /* ReadSqueakBlocks(); A character of a stage's own blocks ("folder/map.mxp,folder/tileset@table:main/over+main/over,...",
         * hexadecimal block numbers of the map's block table): each cell a block with the one over it, 16 by 32 pixels, or several such
         * side by side ('+', each 16 pixels on to the right), its origin at the bottom middle of its first, each 8-pixel tile a piece
         * in its tileset palette (flipped as its tilemap entry is): Dark Nebula's ice floor, the front tileset's ice blocks his pillar
         * puts in the floor's top row, 0x022146EC. Each cell given mirrored (each piece on the other side, its flip turned), as
         * SqueakCanvas() mirrors every Squeak Squad cell: the blocks are a stage's, drawn as they are (their left and right ends had
         * come out swapped); */
        static SqueakCharacter ReadSqueakBlocks(byte[] rom, string spec) {
            string[] at = spec.Split('@'), files = at[0].Split(','), cellSpecs = at[1].Split(':');
            SqueakMap map = new SqueakMap(SqueakUnpack(SqueakFileAnyCase(rom, files[0]) ?? throw new InvalidOperationException($"Squeak Squad's {files[0]} is missing.")));
            SqueakTileset tileset = new SqueakTileset(SqueakFileAnyCase(rom, files[1]) ?? throw new InvalidOperationException($"Squeak Squad's {files[1]} is missing."));
            int table = int.Parse(cellSpecs[0]);
            List<byte> tiles = new List<byte>();
            List<SqueakCell> cells = new List<SqueakCell>();
            foreach (string cellSpec in cellSpecs[1].Split(',')) {
                List<SqueakPiece> pieces = new List<SqueakPiece>();
                string[] columns = cellSpec.Split('+');
                for (int k = 0; k < columns.Length; k++) {
                    int[] blocks = columns[k].Split('/').Select(b => Convert.ToInt32(b, 16)).ToArray();
                    for (int b = 0; b < blocks.Length; b++)
                        for (int q = 0; q < 4; q++) {
                            int entry = map.Entry(table, blocks[b], q), tile = entry & 0x3FF;
                            int x = -16 * k - (q % 2) * 8, y = -16 * (b + 1) + q / 2 * 8;
                            int palette = tile < tileset.TilePalettes.Length ? tileset.TilePalettes[tile] & 15 : 0;
                            // (a piece a tile, its tile in a 4-tile step of its own: SqueakCanvas());
                            pieces.Add(new SqueakPiece(y & 0xFF, x & 0x1FF | (entry & 0xC00 ^ 0x400) << 2, tiles.Count / 128 | palette << 12));
                            tiles.AddRange(tile * 32 + 32 <= tileset.Tiles.Length ? tileset.Tiles[(tile * 32)..(tile * 32 + 32)] : new byte[32]);
                            tiles.AddRange(new byte[96]);
                        }
                }
                cells.Add(new SqueakCell(0, pieces));
            }
            return new SqueakCharacter {
                Tiles = tiles.ToArray(),
                Palettes = Enumerable.Range(0, 16).Select(r => Enumerable.Range(0, 16).Select(c => (ushort)(tileset.Colors[r * 16 + c] & 0x7FFF)).ToArray()).ToList(),
                Cells = cells,
                Anims = cells.Select((_, i) => new SqueakAnim(true, new List<SqueakStep> { new SqueakStep(i, 1, 0, 0) })).ToList(),
                Compositions = new List<List<SqueakPart>>(), Composites = new List<SqueakComposite>(),
            };
        }

        /* SqueakShrink(); A picture drawn at num/den of its size, its outlines a pixel wide and whole: each old pixel goes to the new
         * one its middle is in (num/den as far from the origin, the new pixels' corners on the origin's; 1, 2 or 4 old pixels a new
         * one), the new pixel an outline's (its darkest: a weighted brightness under 300 of 2550) if one of them is, so an outline
         * stays one pixel across, unbroken (taking every new pixel an outline crosses a quarter of, as before, drew them one and two
         * pixels thick, in jagged stairs); else transparent when most of them are, else the color most of them have (ties: the one
         * nearest its middle). Then each stair's doubled corner taken out: an outline pixel between two others at a right angle
         * (one across, one up or down; the diagonal between them and its other two sides not outlines), the diagonal joining them,
         * given the color on its other side. `at`: the old picture's value at a pixel (-1: none), `brightness` a value's; the new
         * pixels by their place from the origin; */
        static Dictionary<(int x, int y), int> SqueakShrink(Func<int, int, int> at, int width, int height, int originX, int originY, int num, int den,
                                                             Func<int, int> brightness) {
            int FloorDiv(int a, int b) => a >= 0 ? a / b : -((-a + b - 1) / b);
            int New(int pixel, int origin) => FloorDiv((2 * (pixel - origin) + 1) * num, 2 * den);
            bool Dark(int value) => brightness(value) < 300;

            // (the old picture's drawn box, and its pixels around it: those not drawn count against a new pixel's being drawn);
            int minX = width, maxX = -1, minY = height, maxY = -1;
            for (int y = 0; y < height; y++)
                for (int x = 0; x < width; x++)
                    if (at(x, y) >= 0) {
                        minX = Math.Min(minX, x);
                        maxX = Math.Max(maxX, x);
                        minY = Math.Min(minY, y);
                        maxY = Math.Max(maxY, y);
                    }
            Dictionary<(int, int), int> counts = new Dictionary<(int, int), int>();
            Dictionary<(int, int), List<(int x, int y, int value)>> drawn = new Dictionary<(int, int), List<(int, int, int)>>();
            for (int y = Math.Max(0, minY - 2); y <= Math.Min(height - 1, maxY + 2); y++)
                for (int x = Math.Max(0, minX - 2); x <= Math.Min(width - 1, maxX + 2); x++) {
                    (int, int) key = (New(x, originX), New(y, originY));
                    counts[key] = counts.GetValueOrDefault(key) + 1;
                    int value = at(x, y);
                    if (value < 0) continue;
                    if (!drawn.TryGetValue(key, out List<(int, int, int)> list)) drawn[key] = list = new List<(int, int, int)>();
                    list.Add((x, y, value));
                }
            Dictionary<(int x, int y), int> shrunk = new Dictionary<(int, int), int>();
            foreach (((int i, int j) key, List<(int x, int y, int value)> list) in drawn) {
                List<(int x, int y, int value)> outline = list.Where(p => Dark(p.value)).ToList();
                if (outline.Count > 0) {
                    shrunk[key] = outline.OrderBy(p => brightness(p.value)).First().value;
                    continue;
                }
                if (list.Count * 2 < counts[key]) continue;
                // (its middle in the old picture's pixels, twice over: an old pixel's middle is 2x + 1);
                double middleX = 2 * originX + (2 * key.i + 1) * den / (double)num, middleY = 2 * originY + (2 * key.j + 1) * den / (double)num;
                shrunk[key] = list.GroupBy(p => p.value).OrderByDescending(g => g.Count())
                                  .ThenBy(g => g.Sum(p => Math.Pow(2 * p.x + 1 - middleX, 2) + Math.Pow(2 * p.y + 1 - middleY, 2))).First().Key;
            }
            // (the stairs' doubled corners);
            bool Outline(int x, int y) => shrunk.TryGetValue((x, y), out int value) && Dark(value);
            Dictionary<(int x, int y), int> cleaned = new Dictionary<(int, int), int>(shrunk);
            foreach (((int x, int y) key, int value) in shrunk) {
                if (!Dark(value)) continue;
                bool done = false;
                foreach (int across in new[] { -1, 1 }) {
                    foreach (int up in new[] { -1, 1 }) {
                        if (!Outline(key.x + across, key.y) || !Outline(key.x, key.y + up) || Outline(key.x - across, key.y) || Outline(key.x, key.y - up)
                            || Outline(key.x + across, key.y + up)) continue;
                        if (shrunk.TryGetValue((key.x - across, key.y - up), out int side)) {
                            if (!Dark(side)) cleaned[key] = side;
                        } else {
                            cleaned.Remove(key);
                        }
                        done = true;
                        break;
                    }
                    if (done) break;
                }
            }
            return cleaned;
        }

        // SqueakBrightness(); A color's weighted brightness (0 to 2550: green 6, red 3, blue 1);
        static int SqueakBrightness(int color) => (color & 31) * 8 * 3 + (color >> 5 & 31) * 8 * 6 + (color >> 10 & 31) * 8;

        /* SqueakScaled(); A character's frames at num/den of its size (Scale: SqueakFrames' cells drawn smaller, SqueakShrink) need its
         * animations' and compositions' offsets as much smaller (rounded, the same either way: they turn with it); */
        static SqueakCharacter SqueakScaled(SqueakCharacter character, int num, int den) {
            short Scaled(short v) => (short)Math.Round(v * num / (double)den, MidpointRounding.AwayFromZero);
            character.Anims = character.Anims.Select(a => a with { Steps = a.Steps.Select(st => st with { X = Scaled(st.X), Y = Scaled(st.Y) }).ToList() }).ToList();
            character.Compositions = character.Compositions.Select(c => c.Select(part => part with { X = Scaled(part.X), Y = Scaled(part.Y) }).ToList()).ToList();
            return character;
        }

        /* Yadgaine at three quarters of the DS's size (the owner's pick: at the DS's, its body's own pixels beside a shell that can't be
         * as big in KatAM's sprite memory, it was too detailed beside it; shrunk, the GBA's smaller screen shows it as the DS's did):
         * its body and legs (SqueakEnemy.Scale), its shell (SqueakShell) and, in yadgaine.c (SIZE_NUM, SIZE_DEN), its boxes, its spots
         * and how far it moves; */
        const int YadgaineScaleNum = 3, YadgaineScaleDen = 4;

        /* SqueakShell(); Yadgaine's shell, which Squeak Squad's stage draws on a background layer from its own map and tiles (0x0220B0B4;
         * KatAM's backgrounds are full in a fight), as one KatAM frame at three quarters of its size, as its body (YadgaineScale:
         * SqueakShrink; 174 tiles of KatAM's 512-tile sprite heap, drawn pixel for pixel by yadgaine.c's Shell). Its stage's map,
         * map/a4s7s2.mxp (RLE), has its sections' offsets at +0xC: the map's width (the first's first byte), its blocks (the third's,
         * after 2 bytes: 16x16 pixels, 4 tile entries each: tile, flips, palette - 6) and the shell's 16 by 13 blocks (the fifth's top
         * left); their tiles and palettes are map/30_b.bin's (+0x210, +0x10). Only what its fight ever shows (columns 14 on: the rest is
         * left of the screen at its spot furthest in, the map's edge there; rows to 171: under the floor after). The origin (the body's
         * feet: column 128, row 171) a pixel's corner, at the canvas' middle; it faces as on the DS (it never turns); */
        const string YadgaineShellFile = "stage:a4s7s2";
        const int ShellLeft = 14, ShellTop = 0, ShellBottom = 171, ShellOriginX = 128, ShellOriginY = 171;
        static (List<(byte[] tiles, ushort[] oams, short[] attributes, int palette)> frames, List<byte[]> palettes) SqueakShell(byte[] rom) {
            byte[] packed = SqueakFile(rom, "map/a4s7s2.mxp"), bg = SqueakFile(rom, "map/30_b.bin");
            if (packed == null || bg == null || packed[0] != 0x30) throw new InvalidOperationException("Squeak Squad's map/a4s7s2 is missing.");

            // (RLE: a flag byte, bit 7 a run of the next byte, (its low bits) + 3 times, else (its low bits) + 1 bytes as they are);
            byte[] map = new byte[ReadInt(packed, 0) >> 8];
            for (int at = 4, length = 0; length < map.Length;) {
                int flag = packed[at++];
                if ((flag & 0x80) != 0) {
                    for (int n = (flag & 0x7F) + 3; n > 0 && length < map.Length; n--) map[length++] = packed[at];
                    at++;
                } else {
                    for (int n = (flag & 0x7F) + 1; n > 0 && length < map.Length; n--) map[length++] = packed[at++];
                }
            }
            int width = map[ReadInt(map, 0x0C)], blocks = ReadInt(map, 0x0C + 2 * 4) + 2, grid = ReadInt(map, 0x0C + 4 * 4);
            const int Columns = 16, Rows = 13, Width = Columns * 16, Height = Rows * 16;
            int?[,] picture = new int?[Width, Height];
            for (int by = 0; by < Rows; by++)
                for (int bx = 0; bx < Columns; bx++) {
                    int block = ReadShort(map, grid + (by * width + bx) * 2);
                    for (int k = 0; k < 4; k++) {
                        int entry = ReadShort(map, blocks + block * 8 + k * 2), tile = entry & 0x3FF, palette = (entry >> 12) + 6;
                        bool hflip = (entry & 0x400) != 0, vflip = (entry & 0x800) != 0;
                        for (int y = 0; y < 8; y++)
                            for (int x = 0; x < 8; x++) {
                                int color = bg[0x210 + tile * 32 + y * 4 + x / 2] >> (x % 2 * 4) & 15;
                                int px = bx * 16 + k % 2 * 8 + (hflip ? 7 - x : x), py = by * 16 + k / 2 * 8 + (vflip ? 7 - y : y);
                                if (color != 0 && px >= ShellLeft && py >= ShellTop && py <= ShellBottom)
                                    picture[px, py] = ReadShort(bg, 0x10 + (palette * 16 + color) * 2);
                            }
                    }
                }

            const int Size = 256, Center = Size / 2;
            Dictionary<(int x, int y), int> shrunk = SqueakShrink((x, y) => picture[x, y] ?? -1, Width, Height, ShellOriginX, ShellOriginY,
                                                                   YadgaineScaleNum, YadgaineScaleDen, SqueakBrightness);
            List<ushort> colors = new List<ushort> { 0 };
            int[,] canvas = new int[Size, Size];
            foreach (((int i, int j), int value) in shrunk) {
                if (Center + i < 0 || Center + j < 0 || Center + i >= Size || Center + j >= Size) throw new InvalidOperationException("Yadgaine's shell is too big.");
                int index = colors.IndexOf((ushort)value, 1);
                if (index < 0) {
                    index = colors.Count;
                    colors.Add((ushort)value);
                }
                canvas[Center + i, Center + j] = index;
            }
            if (colors.Count > 16) throw new InvalidOperationException("Yadgaine's shell has more than 15 colors.");

            byte[] paletteBytes = new byte[32];
            for (int c = 0; c < colors.Count; c++) WriteShort(paletteBytes, c * 2, colors[c]);
            (byte[] tiles, ushort[] oams, short[] attributes) = CutFrame(canvas, int.MaxValue, Size);
            return (new List<(byte[], ushort[], short[], int)> { (tiles, oams, attributes, 0) }, new List<byte[]> { paletteBytes });
        }

        // The size of squeak.h's SqueakGraphics, an entry of gSqueakData.enemies;
        const int SqueakGraphicsSize = 24;

        // The type Squeak Squad's characters share (squeak.h's SQUEAK_TYPE: KatAM's last free regular enemy type);
        public const byte SqueakID = 0x2C;
        /* KatAM's own enemies given Squeak Squad's extra variants (squeak/natives.c): their object type, and the spawn and resume that
         * take over their type's (KatAM's own kept in gSqueakNatives, in this order; no resume: KatAM's stays); */
        static readonly (byte type, string spawn, string start)[] SqueakNativeTypes = {
            (0x00, "SqueakCreateWaddleDee", "SqueakWaddleDeeStart"),    // Waddle Dee;
            (0x03, "SqueakCreateGlunk", "SqueakGlunkStart"),            // Glunk;
            (0x06, "SqueakCreateGordo", "SqueakGordoStart"),            // Gordo;
            (0x07, "SqueakCreateSnooter", null),                        // Snooter (its resume, its rush, stays KatAM's);
        };
        // The hurt frames every character's anim starts with (squeak.h's SQUEAK_HURT_VARIANTS: the type's hurt indexes 0 and 1, and 2);
        const int SqueakHurtVariants = 3;

        /* A Squeak Squad character: its index (squeak.h's SQUEAK_*), name, files (enemy/<File>.*, or "folder/name"; OnlyAnims: only those
         * animations of them, as its 0, 1..., and the cells they show, in order), the cells its hurt frames show (its
         * hurt animations': SqueakHurtVariants), and its HP bar name, made of the letters of KatAM's names (object type, letter); a thing
 * a character throws (spawned as a character of its own) can be drawn with that character's graphics (GraphicsOf: its index).
         * A character in other colors (Palettes: the other .cla files of its folder, "chun_blue") gets one palette more each, in a frame
         * of its own after its cells (squeak.c's SqueakUsePalette); MergeColors: colors drawn as others (from, to), so a character a
         * color or two over KatAM's 15 gets one palette. A character drawn from more files (ExtraFile, comma-separated: the other sprites
         * Squeak Squad's body switches to, its +0xE4) has their cells and animations after the first's, in that order (OnlyAnims
         * counting them on: the files before's counts plus theirs) and their palettes after the first's; Layers (up to two groups of those merged palettes' indexes):
         * the pieces drawn with a group go to a layer of their own, another sprite with its own palette shown with the cell (squeak.c's
         * SqueakShowLayer: a weapon in a character's hands in other colors than its own, Doc's saucer); Canvas: the frames' canvas
         * (pixels a side), its spot in the middle (Storo's raised mallet, 96 pixels over its feet, needs 192); BodyLast: its .mca's
         * compositions list its body last (King Dedede's: its sparkles first, drawn over it), moved first here, the part the
         * character's own sprite shows (with its layers), the others behind it; KeepComposites: OnlyAnims keeps its files' .mca
         * composite animations (their parts' animations among those kept: Mrs. Moley's, from her other two files); SharePalettes:
         * its other files' cells drawn with the first's palettes, their indexes kept (Squeak Squad's body loading every sprite into one
         * palette object: Daroach's); PaletteFile: every file's cells in another .cla's colors (Dark Daroach's, dancho01's);
         * WholeCells: a cell drawn wholly with one layer's palettes stays the character's own (its composites' parts, KatAM's hurt
         * frames show no layer), and the palettes its cells draw with together are merged into one when their colors fit in 15 (Meta
         * Knight's: his body the character's own, his sword a layer; his sword's flourishes alone in its frames, his wave's two
         * palettes one); Scale: drawn at Scale.Num / Scale.Den of the DS's size (its cells, SqueakShrink, and its animations' offsets:
         * Yadgaine's, YadgaineScale); Roll: an animation more, that cell turned a sixteenth more each step (SqueakRolled(): the
         * rolling boulder's, which the DS turns as its body rolls); OnlyPalettes: only the pieces drawn with these of its .cla's palettes
         * (the others left out: SqueakSplitLayer()), a frame's palette the one of its first piece (the pedestals' icons); MapIcon: a
         * mini-boss's or boss's look in the map tracker, these cells drawn on each other with its layers (none: its first animation's
         * first cell, KatAMExpansion.WriteSqueakMapEntries()); DamagePalette: its hurt flash's colors (Squeak Squad's `enemy/<name>.cla`,
         * Daroach's dan_dmg: the DS's body +0x33C, shown by its vtable +0x20): per animation its palette in those colors, frames of an
         * anim of their own (a character in several palettes: its .cla's palette n the damage file's n; squeak.c's SqueakDamageColors);
         * MergeParts: its composite animations' parts that play in step drawn by one sprite (SqueakMergeParts()); FitVram: its sprite's
         * VRAM as many tiles as the frame it shows takes, more got as a bigger one shows (squeak.c's SqueakFitVram: a thing drawn with a
         * boss's graphics took his biggest frame's, Daroach's stars 103 tiles for 16, KatAM's VRAM full with three); HighParts: its
         * composites' parts placed this many pixels or more over its spot left out (SqueakLowParts(): their frames drawn up from their
         * place, at most 128 high, wholly over KatAM's view, 16 pixels shorter than the DS's: Dark Nebula's fire wall's top row); */
        internal sealed record SqueakEnemy(int Index, string Name, string File, int[] HurtCells, (int type, int letter)[] Glyphs, int GraphicsOf = -1,
                                            byte MidbossType = 0, int[] OnlyAnims = null, string[] Palettes = null,
                                            (ushort from, ushort to)[] MergeColors = null, string ExtraFile = null,
                                            int[][] Layers = null, int Canvas = 128, bool BodyLast = false, bool KeepComposites = false,
                                            bool SharePalettes = false, string PaletteFile = null, bool WholeCells = false,
                                            (int Num, int Den) Scale = default, int ShiftY = 0, int Rotate = 0, int Roll = -1,
                                            bool FlattenComposites = false, int[] OnlyPalettes = null, int[] MapIcon = null,
                                            string DamagePalette = null, bool MergeParts = false, bool FitVram = false, int HighParts = 0);
        /* AnimPalettes: a character whose cells are in Squeak Squad's different palettes (Doc's things), per animation the palette its
         * sprite takes (SqueakPaletteKey); */
        internal sealed record SqueakBuilt(SqueakEnemy Enemy, int AnimId, SqueakCharacter Character, int Tiles, List<SqueakLayerBuilt> Layers = null,
                                           List<SqueakPaletteKey> AnimPalettes = null, List<int> CellTiles = null,
                                           List<SqueakPaletteKey> DamagePalettes = null);
        // A character's layer (Layers): its anim, the most tiles a frame takes, per cell its frame + 1 (0: none; | 0x8000: in front);
        internal sealed record SqueakLayerBuilt(int AnimId, int Tiles, ushort[] Cells, SqueakPaletteKey Palette);
        /* A palette's slot (squeak.c's SqueakLoadPalette): the key and frame of the first sprite built with it, those after it with the same
         * colors taking it too (one of the 7 palette slots KatAM keeps for a room's enemies, not one each: sub_0803DFAC); */
        internal sealed record SqueakPaletteKey(int Key, int Variant);

        /* Squeak Squad's effects the ported characters show (squeak.h's SQUEAK_FX_*): an animation of <File>.* ("folder/name"; 0x02021900's
         * types name effect/efc_cmn* ones through the table at 0x02157DF8), each its own anim of the cells it shows (in Squeak Squad's
         * palette, its slot shared with any other sprite in it: SqueakPaletteKey); */
        /* (Skip: cells its animation shows that aren't drawn, their steps shown as nothing (their cell 0xFFFF, squeak.h's
         * SQUEAK_FX_HIDDEN): Kirby's own in the Ability Scrolls' Spark bursts, the DS's flickering him in them, KatAM's Kirby drawn
         * as he is. Once: played once, though the DS's animation loops, as the DS's effect plays it: Spark Kirby's glint); */
        internal sealed record SqueakEffect(int Index, string File, int Anim, int[] Skip = null, bool Once = false);
        internal sealed record SqueakEffectBuilt(SqueakEffect Effect, int AnimId, List<int> Cells, SqueakAnim Anim, int Tiles, SqueakPaletteKey Palette);

        static readonly List<SqueakEffect> SqueakEffectDefinitions = new List<SqueakEffect> {
            new SqueakEffect(0, "effect/efc_cmn0", 19),     // Type 3: the dust puff (Gold Waddle Dee's steps, Perara's launch);
            /* The splashes of the stage's water (its vtable +0xB4, 0x02215A10: type 0 into the water, 1 out of it; the stage's m_efc file's
             * animations 4 and 5, the same in clay, sand and snow); */
            new SqueakEffect(1, "m_efc/clay", 4),
            new SqueakEffect(2, "m_efc/clay", 5),
            new SqueakEffect(3, "effect/efc_cmn1", 1),     // 0x020B48E0's type 6: a thing bursting (Crimp's rock);
            new SqueakEffect(4, "effect/efc_cmn0", 7),     // 0x020B48E0's type 3: a star burst (Bio Spark's kunai stuck in a wall);
            new SqueakEffect(5, "effect/efc_cmn0", 23),    // 0x020219C4's type 1: a mid-boss's landing puff (Gao Gao's);
            new SqueakEffect(6, "effect/efc_cmn0", 22),    // 0x020219C4's type 2: its big landing puff;
            /* Doc's composite animations' other parts (doc_at1.mca: its compositions 1 and 2, the ball and rings of its gun's shots;
             * 0, the bomb in its claw; 3, its cannon's smoke, looping while it fires): doc_at1's animations 4, 5, 11, 12, 18 and 26 (in
             * its palettes 3, 2, 3, 2, 4 and 3: their slots shared with Doc's layer and things in them); */
            new SqueakEffect(7, "enemy/doc_at1", 4),
            new SqueakEffect(8, "enemy/doc_at1", 5),
            new SqueakEffect(9, "enemy/doc_at1", 11),
            new SqueakEffect(10, "enemy/doc_at1", 12),
            new SqueakEffect(11, "enemy/doc_at1", 18),
            new SqueakEffect(12, "enemy/doc_at1", 26),
            new SqueakEffect(13, YadgaineShellFile, 0),     // Yadgaine's shell (the stage's: SqueakShell);
            // Gimmick 0x19's (squeak/gimmicks.c's Emitter): its look 1, a lava spark (the volcano's lava), 0 a water bubble;
            new SqueakEffect(14, "yakumono/spark", 0),
            new SqueakEffect(15, "yakumono/bubble", 0),
            // Gimmick 0x18's (squeak/gimmicks.c's Streaks): its look 0, the wind streaks' 3 animations, 1 the snowflakes' 2;
            new SqueakEffect(16, "yakumono/stream", 0),
            new SqueakEffect(17, "yakumono/stream", 1),
            new SqueakEffect(18, "yakumono/stream", 2),
            new SqueakEffect(19, "yakumono/snow", 0),
            new SqueakEffect(20, "yakumono/snow", 1),
            // Dark Nebula's defeat ("ROM Expansion/squeak/dark_nebula.c"): darkZERO_ext's 13, the sparks thrown off him (0x0211CBAA);
            new SqueakEffect(21, "enemy/darkZERO_ext", 13),
            // Dark Daroach's defeat ("ROM Expansion/squeak/daroach_boss.c"): efc_cmn0's 10, the burst at him (0x02039B28);
            new SqueakEffect(22, "effect/efc_cmn0", 10),
            /* The Spark Ability Scroll's ("ROM Expansion/scrolls.c"): kirby/spark_ext's animations its composite ones draw (spark_ext.mca):
             * 30, the charged aura's lightning (level 1, 0x0100001E); 22, 23 and 31, its level 2's (composite 3: lightning and the
             * ball; the thrown ball, composite 4, the same with 32, 31's cells); 26 and 25, the level-1 attack (composite 1: its lightning
             * ring, the burst); 27, 28 and 38, the level-2 attack's (composite 2); the bursts without Kirby's own cells (5, 8, 11, 14:
             * the DS shows him in them every other frame); then kirby/spark_cmn's 58, the glint on Spark Kirby every 61 frames (once,
             * its empty cell 0 shown as nothing); */
            new SqueakEffect(23, "kirby/spark_ext", 30),
            new SqueakEffect(24, "kirby/spark_ext", 22),
            new SqueakEffect(25, "kirby/spark_ext", 23),
            new SqueakEffect(26, "kirby/spark_ext", 31),
            new SqueakEffect(27, "kirby/spark_ext", 26),
            new SqueakEffect(28, "kirby/spark_ext", 25, Skip: new[] { 5, 8, 11, 14 }),
            new SqueakEffect(29, "kirby/spark_ext", 27),
            new SqueakEffect(30, "kirby/spark_ext", 28),
            new SqueakEffect(31, "kirby/spark_ext", 38, Skip: new[] { 5, 8, 11, 14 }),
            new SqueakEffect(32, "kirby/spark_cmn", 58, Skip: new[] { 0 }, Once: true),
            // The Beam scroll's whip ("ROM Expansion/scrolls.c": beam_ext.mca's composite 1, its whip; its last step empty);
            new SqueakEffect(33, "kirby/beam_ext", 25, Skip: new[] { 0 }),
        };

        /* SqueakEffectsBuild(); Each effect's cells (those its animation shows, in its order) as KatAM frames and their palette, the anim
         * added by `addAnim` (palettes, frames: its anim ID), its palette's slot by `share` (its palette, anim ID, frame); */
        static List<SqueakEffectBuilt> SqueakEffectsBuild(byte[] squeak, List<SqueakEffect> effects,
                                                          Func<List<byte[]>, List<(byte[] tiles, ushort[] oams, short[] attributes, int palette)>, int> addAnim,
                                                          Func<byte[], int, int, SqueakPaletteKey> share) {
            List<SqueakEffectBuilt> built = new List<SqueakEffectBuilt>();
            foreach (SqueakEffect effect in effects) {
                // (Yadgaine's shell: the stage's picture, one still frame);
                if (effect.File == YadgaineShellFile) {
                    (List<(byte[] tiles, ushort[] oams, short[] attributes, int palette)> shell, List<byte[]> shellPalettes) = SqueakShell(squeak);
                    int shellAnim = addAnim(shellPalettes, shell);
                    built.Add(new SqueakEffectBuilt(effect, shellAnim, new List<int> { 0 }, new SqueakAnim(true, new List<SqueakStep> { new SqueakStep(0, 1, 0, 0) }),
                                                    shell[0].tiles.Length / 32, share(shellPalettes[0], shellAnim, 0)));
                    continue;
                }
                string[] path = effect.File.Split('/');
                SqueakCharacter character = ReadSqueakCharacter(squeak, path[1], path[0]);
                SqueakAnim anim = character.Anims[effect.Anim];
                if (effect.Once) anim = anim with { Loops = false };
                List<int> cells = anim.Steps.Select(s => s.Cell).Where(c => effect.Skip == null || !effect.Skip.Contains(c)).Distinct().ToList();
                (List<(byte[] tiles, ushort[] oams, short[] attributes, int palette)> frames, List<byte[]> palettes) = SqueakFrames(character, cells: cells);
                int animId = addAnim(palettes, frames), first = cells.IndexOf(anim.Steps.First(s => cells.Contains(s.Cell)).Cell);
                built.Add(new SqueakEffectBuilt(effect, animId, cells, anim, frames.Max(f => f.tiles.Length / 32),
                                                share(palettes[frames[first].palette], animId, first)));
            }
            return built;
        }

        // "C", "i", "p" (Chip), "r" (Bronto Burt), "m" (Bomber);
        static readonly (int type, int letter)[] CrimpGlyphs = { (0x08, 0), (0x01, 1), (0x08, 2), (0x28, 2), (0x08, 3) };

        // "C" (Chip), "r" (Bronto Burt), "e", "t" (Hot Head);
        static readonly (int type, int letter)[] CretGlyphs = { (0x08, 0), (0x01, 1), (0x14, 4), (0x14, 2) };

        // "B", "o", "r" (Bronto Burt), "i" (Minny), " ", "S" (Squishy), "p" (Chip), "a" (Hot Head), "k" (Glunk);
        static readonly (int type, int letter)[] BioSparkGlyphs = { (0x01, 0), (0x27, 1), (0x01, 2), (-1, 0), (0x04, 0), (0x08, 3), (0x14, 5),
                                                                    (0x01, 1), (0x03, 4) };

        // "B" (Bronto Burt), "u", "l" (Glunk), "b" (Bomber), "e", " ", "H", "e", "a", "d" (Hot Head);
        static readonly (int type, int letter)[] BubbleHeadGlyphs = { (0x01, 0), (0x03, 2), (0x28, 3), (0x28, 3), (0x03, 1), (0x14, 4), (-1, 0),
                                                                      (0x14, 0), (0x14, 4), (0x14, 5), (0x14, 6) };

        // "A" (Rocky's R: the names' font draws R and A alike, as KatAM's Spanish "Anemonia"), "c" (Rocky), "h", "i" (Chip);
        static readonly (int type, int letter)[] AcchiGlyphs = { (0x17, 0), (0x17, 2), (0x17, 2), (0x08, 1), (0x08, 2) };

        // "T" (Twister), "e", "d" (Waddle Dee), "h" (Chip), "a" (Hot Head), "u" (Glunk), "n" (Bronto Burt);
        static readonly (int type, int letter)[] TedhaunGlyphs = { (0x1C, 0), (0x14, 4), (0x00, 2), (0x08, 1), (0x14, 5), (0x03, 2), (0x01, 3) };

        // "B" (Bronto Burt), "u" (Glunk), "b" (Bomber), "o", "o" (Bronto Burt);
        static readonly (int type, int letter)[] BubooGlyphs = { (0x01, 0), (0x03, 2), (0x28, 3), (0x01, 2), (0x01, 2) };

        // "S", "q", "u" (Squishy), "e", "a" (Hot Head), "k" (Glunk), "e" (Hot Head), "r" (Bronto Burt);
        static readonly (int type, int letter)[] SqueakerGlyphs = { (0x04, 0), (0x04, 1), (0x04, 2), (0x14, 4), (0x14, 5), (0x03, 4), (0x14, 4),
                                                                    (0x01, 1) };

        // "D" (Waddle Dee), "o" (Bronto Burt), "c" (Rocky);
        static readonly (int type, int letter)[] DocGlyphs = { (0x00, 6), (0x01, 2), (0x17, 2) };

        // "S" (Squishy), "t", "o" (Hot Head), "r" (Bronto Burt), "o" (Hot Head);
        static readonly (int type, int letter)[] StoroGlyphs = { (0x04, 0), (0x14, 2), (0x14, 1), (0x01, 1), (0x14, 1) };

        // "K" (Sir Kibble), "i" (Chip), "n" (Bronto Burt), "g" (Pengy), " ", "D", "e", "d", "e", "d", "e" (Waddle Dee, Hot Head);
        static readonly (int type, int letter)[] KingDededeGlyphs = { (0x18, 3), (0x08, 2), (0x01, 3), (0x16, 3), (-1, 0), (0x00, 6), (0x14, 4),
                                                                      (0x00, 2), (0x14, 4), (0x00, 2), (0x14, 4) };

        // "M", "r" (Mr. Frosty), "s" (Squishy), "." (Mr. Frosty), " ", "Moley" (KatAM's Moley);
        static readonly (int type, int letter)[] MrsMoleyGlyphs = { (0x38, 0), (0x38, 1), (0x04, 4), (0x38, 2), (-1, 0), (0x4A, 0), (0x4A, 1),
                                                                    (0x4A, 2), (0x4A, 3), (0x4A, 4) };

        // "D" (Waddle Dee), "a" (Hot Head), "r", "o" (Bronto Burt), "a" (Hot Head), "c" (Rocky), "h" (Chip);
        // "M" (Mr. Frosty), "e" (Hot Head), "c" (Rocky), "h" (Chip), "a" (Hot Head), "-" (Bang-Bang), "Kracko" (KatAM's Kracko);
        static readonly (int type, int letter)[] MechaKrackoGlyphs = { (0x38, 0), (0x14, 4), (0x17, 2), (0x08, 1), (0x14, 5), (0x2F, 4),
                                                                       (0x45, 0), (0x45, 1), (0x45, 2), (0x45, 3), (0x45, 4), (0x45, 5) };

        // "Y" (drawn: no name of the game's has one), "a", "d" (Hot Head), "g" (Pengy), "a" (Hot Head), "i" (Chip), "n" (Bronto Burt), "e";
        static readonly (int type, int letter)[] YadgaineGlyphs = { (HandDrawn, 0), (0x14, 5), (0x14, 6), (0x16, 3), (0x14, 5), (0x08, 2),
                                                                    (0x01, 3), (0x14, 4) };

        // "B", "o" (Bronto Burt), "h" (Chip), "b" (Sir Kibble), "o" (Bronto Burt), "h" (Chip);
        static readonly (int type, int letter)[] BohbohGlyphs = { (0x01, 0), (0x01, 2), (0x08, 1), (0x18, 5), (0x01, 2), (0x08, 1) };

        static readonly (int type, int letter)[] DaroachGlyphs = { (0x00, 6), (0x14, 5), (0x01, 1), (0x01, 2), (0x14, 5), (0x17, 2), (0x08, 1) };
        // "M" (Mr. Frosty), "eta" (Hot Head), " ", "Knight" (Sword Knight), as NiDL's Meta Knight's;
        static readonly (int type, int letter)[] MetaKnightGlyphs = { (0x38, 0), (0x14, 4), (0x14, 2), (0x14, 5), (-1, -1), (0x1A, 5),
                                                                      (0x1A, 6), (0x1A, 7), (0x1A, 8), (0x1A, 9), (0x1A, 10) };
        // "D" (Waddle Dee), "a" (Hot Head), "r" (Bronto Burt), "k" (KatAM's Kracko), " ", "Daroach";
        // "D" (Waddle Dee), "a" (Hot Head), "r" (Bronto Burt), "k" (KatAM's Kracko), " ", "N" (drawn: no name of the game's has one),
        // "e" (Hot Head), "b" (Bomber), "u", "l" (Glunk), "a" (Hot Head);
        static readonly (int type, int letter)[] DarkNebulaGlyphs = { (0x00, 6), (0x14, 5), (0x01, 1), (0x45, 4), (-1, 0), (HandDrawn, 1),
                                                                      (0x14, 4), (0x28, 3), (0x03, 2), (0x03, 1), (0x14, 5) };
        static readonly (int type, int letter)[] DarkDaroachGlyphs = { (0x00, 6), (0x14, 5), (0x01, 1), (0x45, 4), (-1, 0), (0x00, 6), (0x14, 5),
                                                                       (0x01, 1), (0x01, 2), (0x14, 5), (0x17, 2), (0x08, 1) };

        // "S" (Squishy), "p", "i" (Chip), "n", "n" (Bronto Burt), "i" (Chip);
        static readonly (int type, int letter)[] SpinniGlyphs = { (0x04, 0), (0x08, 3), (0x08, 2), (0x01, 3), (0x01, 3), (0x08, 2) };

        /* The switch's (squeak.h's SQUEAK_SWITCH), in the hammer peg's 4 tiles (object 0x8D's: chest.c's SqueakCreateSwitch); (the
         * platinum chest's, 69 before it, is gone: Squeak Squad's big chests are KatAM's small chests, the owner: "We should drop the
         * big platinum chests from the ROM and use KatAM's small chests instead."); */
        const int SqueakSwitch = 69, HammerPegTiles = 4;
        /* The shutters' (squeak.h's SQUEAK_SHUTTER on: gimmick 2's looks, 0x02227EA4, turned as the DS turns them; their objects take
         * VRAM of their own, squeak/gimmicks.c); */
        const int SqueakShutter = 70, SqueakShutter90 = 71, SqueakShutter270 = 72, SqueakNeedle = 73, SqueakNeedle180 = 74;
        // (the fire columns', squeak.h's SQUEAK_FIRE on: gimmick 7's looks, 0x02227B30, turned as the DS turns them);
        const int SqueakFireColumn = 75, SqueakFire180 = 76, SqueakFire270 = 77, SqueakFire90 = 78;
        // (the grass tufts' and the barrel's, squeak.h's SQUEAK_GRASS on and SQUEAK_BARREL: gimmicks 0xE and 0xF);
        const int SqueakGrass = 79, SqueakBarrel = 85, SqueakIcicle = 86, SqueakRock = 87, SqueakPlatformLook = 88, SqueakThunderLook = 89,
                  SqueakBoulderLook = 90, SqueakCandleLook = 91;
        /* The Squeak Squad ability room's pedestals (squeak.h's SQUEAK_PEDESTAL and SQUEAK_PEDESTAL_ICON: item/copy, the pedestal item's
         * 0x0203A8A0, its animation per ability the table at 0x021582D8's; SqueakPedestalAnims: abilities 1-26's, the room's): the stand
         * and the pieces in its palette (the DS's 0), and the icon's other pieces (palettes 1-3: one each, more colors than one of
         * KatAM's palettes holds), drawn by a sprite of their own over it (squeak/gimmicks.c's SqueakCreatePedestal); */
        internal const int SqueakPedestalLook = 92, SqueakPedestalIconLook = 93, SqueakIceFloorLook = 94, SqueakScrollIconLook = 95;
        internal static readonly int[] SqueakPedestalAnims = { 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2C, 0x2D, 0x2E, 0x2F, 0x30,
                                                               0x31, 0x32, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F };

        static readonly List<SqueakEnemy> SqueakDefinitions = new List<SqueakEnemy> {
            // Perara: hurt animations 7 and 8 (0x02021A88's 0x1000007, 0x1000008);
            new SqueakEnemy(0, "Perara", "perara", new[] { 18, 19, 19 },
                            new[] { (0x16, 0), (0x14, 4), (0x01, 1), (0x14, 5), (0x01, 1), (0x14, 5) }),   // "P" (Pengy), "e", "a" (Hot Head), "r" (Bronto Burt);
            // Gold Waddle Dee: hurt animations 13 and 14 (0x020B6764's 0x100000D, 0x100000E);
            new SqueakEnemy(1, "Gold Waddle Dee", "gwaddledee", new[] { 9, 10, 10 },
                            new[] { (0x03, 0), (0x01, 2), (0x03, 1), (0x00, 2), (-1, 0), (0x00, 0), (0x00, 1), (0x00, 2), (0x00, 3), (0x00, 4),
                                    (0x00, 5), (-1, 0), (0x00, 6), (0x00, 7), (0x00, 8) }),   // "G" (Glunk), "o" (Bronto Burt), "l" (Glunk), "d", "Waddle Dee";
            // Gussa: hurt animations 17 and 18 (0x0206E364's 0x1000011, 0x1000012);
            new SqueakEnemy(2, "Gussa", "gussa", new[] { 11, 12, 12 },
                            new[] { (0x03, 0), (0x03, 2), (0x04, 4), (0x04, 4), (0x14, 5) }),   // "G", "u" (Glunk), "s" (Squishy), "a" (Hot Head);
            // Crimp: hurt animations 9 and 10 (0x020CE310's 0x1000009, 0x100000A); its rock (0x020CBF94) is drawn with its graphics;
            new SqueakEnemy(3, "Crimp", "climb", new[] { 10, 11, 11 }, CrimpGlyphs),
            new SqueakEnemy(4, "Crimp", "climb", new[] { 22, 22, 22 }, CrimpGlyphs, GraphicsOf: 3),
            // Maiga: hurt cells 11 and 12 (its animations 6 and 7, and 13 and 14 the same);
            new SqueakEnemy(5, "Maiga", "maiga", new[] { 11, 12, 12 },
                            new[] { (0x27, 0), (0x14, 5), (0x27, 1), (0x16, 3), (0x14, 5) }),   // "M", "i" (Minny), "a" (Hot Head), "g" (Pengy);
            // Cret: hurt cells 14 and 15 (its animations 6 and 7: 0x02073044's 0x1000006, 0x1000007); its ball (0x02073764) is drawn
            // with its graphics (cret.cea's animations 9 and 11, cells 16-31);
            new SqueakEnemy(6, "Cret", "cret", new[] { 14, 15, 15 }, CretGlyphs),
            new SqueakEnemy(7, "Cret", "cret", new[] { 16, 16, 16 }, CretGlyphs, GraphicsOf: 6),
            // Uja: hurt cell 60 (the puff's animation 24, 0x020840C8's 0x1000018; squeak/uja.c shows the glove's and sword's, cells 58 and
            // 59, 45 and 55, as its state set them);
            new SqueakEnemy(8, "Uja", "uja", new[] { 60, 60, 60 },
                            new[] { (0x1B, 0), (0x10, 0), (0x14, 5) }),   // "U" (UFO), "J" (Jack: KatAM's names have no small j), "a" (Hot Head);
            // Bio Spark: hurt animations 21 and 22 (0x0207CF18's 0x1000015, 0x1000016: cells 2 and 10); its kunai and leaf (0x0207D770,
            // the class it throws both with) are drawn with its graphics (biospark.cea's animations 15 and 20);
            new SqueakEnemy(9, "Bio Spark", "biospark", new[] { 2, 10, 10 }, BioSparkGlyphs),
            new SqueakEnemy(10, "Bio Spark", "biospark", new[] { 11, 11, 11 }, BioSparkGlyphs, GraphicsOf: 9),
            new SqueakEnemy(11, "Bio Spark", "biospark", new[] { 12, 12, 12 }, BioSparkGlyphs, GraphicsOf: 9),
            // Starman: hurt animations 10 and 11 (0x020877B8's 0x100000A, 0x100000B: cells 15 and 17);
            new SqueakEnemy(12, "Starman", "starman", new[] { 15, 17, 17 },
                            new[] { (0x04, 0), (0x14, 2), (0x14, 5), (0x01, 1), (0x28, 2), (0x14, 5), (0x01, 3) }),   // "S" (Squishy), "t", "a" (Hot Head), "r", "n" (Bronto Burt), "m" (Bomber);
            // Bubble Head: hurt animations 8 and 9 (0x020968D0's 0x1000008, 0x1000009: cells 13 and 14); its bubbles (0x0209702C) are
            // drawn with its graphics (bubblehead.cea's animation 25, cells 52-55);
            new SqueakEnemy(13, "Bubble Head", "bubblehead", new[] { 13, 14, 14 }, BubbleHeadGlyphs),
            new SqueakEnemy(14, "Bubble Head", "bubblehead", new[] { 53, 53, 53 }, BubbleHeadGlyphs, GraphicsOf: 13),
            // Gaw Gaw: hurt animation 24 (0x020B0068's 0x1000018, both: cell 53);
            new SqueakEnemy(15, "Gaw Gaw", "gawgaw", new[] { 53, 53, 53 },
                            new[] { (0x03, 0), (0x14, 5), (0x1A, 1), (-1, 0), (0x03, 0), (0x14, 5), (0x1A, 1) }),   // "G" (Glunk), "a" (Hot Head), "w" (Sword Knight);
            // Acchi: hurt animations 8 and 9 (0x0206B1F0's 0x1000008, 0x1000009: cells 17 and 18); its fire (0x0206BCA0) is drawn with its
            // graphics (acchi.cea's animations 11 and 13-15, cells 23-49);
            new SqueakEnemy(16, "Acchi", "acchi", new[] { 17, 18, 18 }, AcchiGlyphs),
            new SqueakEnemy(17, "Acchi", "acchi", new[] { 23, 23, 23 }, AcchiGlyphs, GraphicsOf: 16),
            // Bun: hurt animations 10 and 11 (0x02056AA4's 0x100000A, 0x100000B: cells 22 and 23);
            new SqueakEnemy(18, "Bun", "bun", new[] { 22, 23, 23 },
                            new[] { (0x01, 0), (0x03, 2), (0x01, 3) }),   // "B", "n" (Bronto Burt), "u" (Glunk);
            // Metalun: hurt animations 7 and 8 (0x020526AC's 0x1000007, 0x1000008: cells 24 and 25);
            new SqueakEnemy(19, "Metalun", "metalum", new[] { 24, 25, 25 },
                            new[] { (0x2B, 0), (0x14, 4), (0x14, 2), (0x14, 5), (0x03, 1), (0x03, 2), (0x01, 3) }),   // "M" (Metal Guardian), "e", "t", "a" (Hot Head), "l", "u" (Glunk), "n" (Bronto Burt);
            /* Big Metalun (a mid-boss, on mini-boss type 0x41, squeak.h's SQUEAK_MIDBOSS): its hurt animations both 10 (0x0210C4F8's
             * 0x100000A: cell 32), the defeated one too; */
            new SqueakEnemy(20, "Big Metalun", "bigmetalum", new[] { 32, 32, 32 },
                            new[] { (0x11, 0), (0x11, 1), (0x11, 2), (-1, 0), (0x2B, 0), (0x14, 4), (0x14, 2), (0x14, 5), (0x03, 1), (0x03, 2),
                                    (0x01, 3) }, MidbossType: 0x41),   // "Big" (Big Waddle Dee), " ", "Metalun" (as Metalun's);
            // Gao Gao (a mid-boss, on mini-boss type 0x40): its hurt animations both 9 (0x02100300's 0x1000009: cell 28);
            new SqueakEnemy(21, "Gao Gao", "gaogao", new[] { 28, 28, 28 },
                            new[] { (0x03, 0), (0x14, 5), (0x01, 2), (-1, 0), (0x03, 0), (0x14, 5), (0x01, 2) },
                            MidbossType: 0x40),   // "G" (Glunk), "a" (Hot Head), "o" (Bronto Burt);
            /* The star a mid-boss knocks out (0x0212389C's kinds 1 and 2, 0x021236A0: efc_cmn0's animations 11-13, the cells they show;
             * its hurt frames its first), and the one King Dedede spits Kirby out in (kind 0: efc_cmn0's animation 5, its 3); */
            new SqueakEnemy(22, "Star", "effect/efc_cmn0", new[] { 0, 0, 0 },
                            new[] { (0x04, 0), (0x14, 2), (0x14, 5), (0x01, 1) }, OnlyAnims: new[] { 11, 12, 13, 5 }),   // as Starman's;
            // Tedhaun (a mid-boss, on mini-boss type 0x42): its hurt animations both 6 (0x02103B4C's 0x1000006: cell 24); its ghosts (0x02104E34) drawn with its graphics (its animation 9);
            new SqueakEnemy(23, "Tedhaun", "tedhaun", new[] { 24, 24, 24 }, TedhaunGlyphs, MidbossType: 0x42),
            new SqueakEnemy(24, "Tedhaun's ghost", "tedhaun", new[] { 29, 29, 29 }, new (int, int)[0], GraphicsOf: 23),   // (no name: the HP bar a hit ghost takes shows none);
            // Buboo (a mid-boss, on mini-boss type 0x41): its hurt animations both 17 (0x02108EAC's 0x1000011: cell 39); its bubbles (0x02109E00) drawn with its graphics;
            new SqueakEnemy(25, "Buboo", "buboo", new[] { 39, 39, 39 }, BubooGlyphs, MidbossType: 0x41),
            new SqueakEnemy(26, "Buboo's bubble", "buboo", new[] { 52, 52, 52 }, new (int, int)[0], GraphicsOf: 25),   // (no name, as Tedhaun's ghosts);
            /* Squeakers (the Squeak Squad's, enemy 0x40: 0x020EC504): the animations its code plays (squeak/squeakers.c's, in its order),
             * its hurt animations 28 and 29 (0x100001C, 0x100001D: cells 46 and 47, here 27 and 28), in four colors (their .cla files:
             * red, then the spawn byte's 1-3); the near-white of the bomb in its hands (0x7FBC) drawn white, for one palette; */
            new SqueakEnemy(27, "Squeaker", "chun", new[] { 27, 28, 28 }, SqueakerGlyphs,
                            OnlyAnims: new[] { 1, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 24, 25, 26, 27, 28, 29, 35, 36, 37, 38, 48 },
                            Palettes: new[] { "chun_blue", "chun_green", "chun_yellow" }, MergeColors: new (ushort, ushort)[] { (0x7FBC, 0x7FFF) }),
            // Their bombs (0x020ED208, drawn with chun's: its animations 33 and 39-41, cells 55, 67 and 68), no name;
            new SqueakEnemy(28, "Squeaker's bomb", "chun", new[] { 0, 0, 0 }, new (int, int)[0], OnlyAnims: new[] { 33, 39, 40, 41 }),
            /* Spinni (the Squeak Squad's, enemy 0x42: 0x020AB6D0), drawn from spin and spin_at (its attacks and hurt poses: the sprite
             * its body switches to, here spin's animations 0-28, then spin_at's from 29): the animations its code plays (squeak/spinni.c's,
             * in its order), its hurt animations spin_at's 13 and 14 (0x100000D, 0x100000E: cells 42 and 43 of the two), the
             * shuriken in its hands (spin_at's palettes 1 and 2, the merged 8 and 9) a layer of its own; */
            new SqueakEnemy(29, "Spinni", "spin", new[] { 42, 43, 43 }, SpinniGlyphs, ExtraFile: "spin_at",
                            OnlyAnims: new[] { 1, 2, 4, 5, 6, 7, 8, 14, 15, 17, 18, 19, 20, 21, 27, 28, 30, 31, 33, 34, 35, 36, 37, 39, 40, 43, 44, 56 },
                            Layers: new[] { new[] { 8, 9 } }),
            // What it throws (0x020ABE08, drawn with spin_at's: its shuriken 16, slash 28, claws 12 and 13), no name;
            new SqueakEnemy(30, "Spinni's shuriken", "spin_at", new[] { 0, 0, 0 }, new (int, int)[0], OnlyAnims: new[] { 16, 28, 12, 13 }),
            /* Doc (the Squeak Squad's, enemy 0x43: 0x020DD82C), drawn from doc and doc_at1 (its robot arm's attacks: the sprite its body
             * switches to, here doc's animations 0-22, then doc_at1's from 23): the animations its code plays (squeak/doc.c's, in its
             * order; doc_at1.mca's composites as its own part, their other parts as effects), its hurt animations doc's 3 and 4
             * (0x1000003, 0x1000004: cells 5 and 6), its saucer (doc's palette 1, doc_at1's: the merged 1 and 9) and its arm's parts
             * (doc_at1's palette 3: 11) layers of their own; */
            new SqueakEnemy(31, "Doc", "doc", new[] { 5, 6, 6 }, DocGlyphs, ExtraFile: "doc_at1",
                            OnlyAnims: new[] { 1, 2, 3, 4, 13, 14, 16, 17, 18, 21, 22, 24, 25, 26, 31, 32, 33, 40, 42, 43, 47, 48, 50, 51 },
                            Layers: new[] { new[] { 1, 9 }, new[] { 11 } }),
            // What it makes (0x020DF354, drawn from doc_at1, then doc: its bolts 6 and 13, missile 15 and 16, bomb 21 and 29, lid 20, saucer 19), no name;
            new SqueakEnemy(32, "Doc's thing", "doc_at1", new[] { 0, 0, 0 }, new (int, int)[0], ExtraFile: "doc",
                            OnlyAnims: new[] { 6, 13, 15, 16, 21, 29, 50, 49 }),
            /* Storo: stron's animations 1-13, stron_atack's 3 (its mallet), stron_dmg's 1 and 2 (its hurt poses, KatAM's hurt frames
             * too); its body in the three files' palette 0 (the same colors), the mallet's wood (stron_atack's 1) and stron's sparkles
             * (1 and 3) layers; */
            new SqueakEnemy(33, "Storo", "stron", new[] { 50, 51, 51 }, StoroGlyphs, ExtraFile: "stron_atack,stron_dmg",
                            OnlyAnims: new[] { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 17, 19, 20 },
                            Layers: new[] { new[] { 5 }, new[] { 1, 3 } }, Canvas: 192),
            /* Daroach in a stage's scene (enemy 0x45: 0x02134DA8, made by 0x020A51A8 with enemy/demo_dancho): its animations 1-5 (hovering,
             * his cape, held, pointing, flying off); never hurt (no hurt animations: his hovering frame); */
            new SqueakEnemy(34, "Daroach", "demo_dancho", new[] { 1, 1, 1 }, DaroachGlyphs, OnlyAnims: new[] { 1, 2, 3, 4, 5 }),
            /* King Dedede (a boss, enemy 0x38: 0x020E7E58, on mini-boss type 0x40 as the mid-bosses): all of dedede's animations (its .mca's
             * inhale and summon use its sparkles' ones), its hurt animation 38 (0x1000026: cell 41), the pieces in its palette 1 (its
             * hands, hammer, face: 27 colors with palette 0's) a layer, a frame big enough for its hammer over its head; */
            new SqueakEnemy(35, "King Dedede", "dedede", new[] { 41, 41, 41 }, KingDededeGlyphs, MidbossType: 0x40,
                            Layers: new[] { new[] { 1 } }, Canvas: 192, BodyLast: true),
            // The air it lets out after floating (0x0212EAA0: its animation 20), no name;
            new SqueakEnemy(36, "King Dedede's puff", "dedede", new[] { 28, 28, 28 }, new (int, int)[0], GraphicsOf: 35),
            /* Mrs. Moley (a boss, enemy 0x3A: 0x0212CF40, on mini-boss type 0x40 as the mid-bosses), drawn from mrsMoley_cmn, _ext00 and
             * _ext01 (the sprites her body switches to: 0x02022F00's slots 0-2; their palettes the same, shared), the animations her code
             * plays (squeak/mrs_moley.c's, in its order: cmn's 15, her hurt animation (0x100000F: cells 36-38, here 0-2), 3-13; ext00's
             * 1-8 and 13-16; ext01's 1-3 and the things in her hand), with both .mca's composites (ext00's dive and landing, ext01's
             * throws: the thing first, moved behind her); a frame big enough for her plunge (ext00's cell 15, 79 pixels tall); */
            new SqueakEnemy(37, "Mrs. Moley", "mrsMoley_cmn", new[] { 0, 0, 0 }, MrsMoleyGlyphs, MidbossType: 0x40,
                            ExtraFile: "mrsMoley_ext00,mrsMoley_ext01",
                            OnlyAnims: new[] { 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 17, 18, 19, 20, 21, 22, 23, 24, 29, 30, 31, 32, 34, 35, 36,
                                               37, 39, 41, 43, 46, 48, 50 },
                            Canvas: 192, BodyLast: true, KeepComposites: true),
            /* What she throws (0x021292EC, drawn from mrsMoley_ext01: by Squeak Squad's kind, its animations 5, 14, 21, 7, 9, 11, 16, 18,
             * the pebbles 19 and 20, and the bomb's flashing 12), no name; */
            new SqueakEnemy(38, "Mrs. Moley's thing", "mrsMoley_ext01", new[] { 0, 0, 0 }, new (int, int)[0],
                            OnlyAnims: new[] { 5, 14, 21, 7, 9, 11, 16, 18, 19, 20, 12 }),
            // Her dirt (0x0212A404, drawn from mrsMoley_ext00: the petals' burst 8, the petals 9 and 10, her shadow 11 and 12), no name;
            new SqueakEnemy(39, "Mrs. Moley's dirt", "mrsMoley_ext00", new[] { 0, 0, 0 }, new (int, int)[0],
                            OnlyAnims: new[] { 8, 9, 10, 11, 12 }),
            /* Mecha-Kracko (a boss, enemy 0x39: 0x02132D7C, on mini-boss type 0x40 as the mid-bosses), drawn with enemy/kracko: all its
             * animations, its .mca's composites (its cloud over its core, its field, its bolts: the first part drawn on top, the
             * character's own), its hurt look its cloud (cell 1: Squeak Squad's is .mca's 2, 0x3000002); a frame big enough for its bolts
             * (139 pixels under it); */
            new SqueakEnemy(40, "Mecha-Kracko", "kracko", new[] { 1, 1, 1 }, MechaKrackoGlyphs, MidbossType: 0x40, Canvas: 288, MapIcon: new[] { 5 }),
            /* Its eye (0x0213025C: kracko's animations 8-15, looking at Kirby; only those, not drawn with its graphics: its sprite takes
             * an eye's VRAM, as type 0x2C's characters take the biggest frame's of theirs), no name; */
            new SqueakEnemy(41, "Mecha-Kracko's eye", "kracko", new[] { 0, 0, 0 }, new (int, int)[0],
                            OnlyAnims: new[] { 8, 9, 10, 11, 12, 13, 14, 15 }),
            /* Yadgaine (a boss, enemy 0x3B: 0x0211A4E8, on mini-boss type 0x40 as the mid-bosses), drawn with enemy/yadgaine: all its
             * animations, its .mca's composites (its body 0 and 1, its legs 2-15); its hurt look its head (cell 3); a frame big enough
             * for its raised legs (114 pixels over it, 65 either side). Its shell isn't in its sprites (the stage draws it): its own effect,
             * SqueakShell; */
            new SqueakEnemy(42, "Yadgaine", "yadgaine", new[] { 3, 3, 3 }, YadgaineGlyphs, MidbossType: 0x40, Canvas: 256,
                            Scale: (YadgaineScaleNum, YadgaineScaleDen), MapIcon: new[] { 3 }),
            // Its legs (0x021185FC, 0x02117C44: its .mca's composites 2-15), drawn with its graphics, no name;
            new SqueakEnemy(43, "Yadgaine's leg", "yadgaine", new[] { 9, 9, 9 }, new (int, int)[0], GraphicsOf: 42),
            /* What it throws and knocks down (0x0212E248: its big and small rocks 32 and 33, a block 34, its shot's puff 35 and capsule
             * 36), no name; */
            new SqueakEnemy(44, "Yadgaine's thing", "yadgaine", new[] { 0, 0, 0 }, new (int, int)[0], OnlyAnims: new[] { 32, 33, 34, 35, 36 }),
            // A gordo falling in its fight (0x0212E198's kind 7: enemy/gordo's animation 3), no name;
            new SqueakEnemy(45, "Yadgaine's gordo", "gordo", new[] { 0, 0, 0 }, new (int, int)[0], OnlyAnims: new[] { 3 }),
            // Daroach bringing it in (0x0212E198's kind 8: demo_dancho's animations 1-5), drawn with his graphics, no name;
            new SqueakEnemy(46, "Daroach (Yadgaine's)", "demo_dancho", new[] { 1, 1, 1 }, new (int, int)[0], GraphicsOf: 34),
            /* Bohboh (a boss, enemy 0x3C: 0x02111370, on mini-boss type 0x40 as the mid-bosses), drawn with enemy/bohboh: all its
             * animations, its .mca's composites (flying, carrying Kirby, rising out of the lava, its flare: its flames' part drawn on top,
             * the character's own); its hurt look its dive (cell 29) and in the lava (31); */
            new SqueakEnemy(47, "Bohboh", "bohboh", new[] { 29, 31, 29 }, BohbohGlyphs, MidbossType: 0x40),
            // What it brings in (0x02127750: its fire, bohboh's animation 12; a fire from the top, 11; the lava's hurt draws nothing), no name;
            new SqueakEnemy(48, "Bohboh's thing", "bohboh", new[] { 0, 0, 0 }, new (int, int)[0], OnlyAnims: new[] { 12, 11 }),
            /* Daroach (a boss, enemy 0x3D: 0x02126F48, on mini-boss type 0x40 as the mid-bosses), drawn from dancho00 and dancho01 (his
             * wand's poses and beam: the sprite his body switches to, its +0xE4, here dancho00's animations 0-29, then dancho01's from 30;
             * both in dancho00's palettes, Squeak Squad's one palette object), their .mca's composites (dancho00's 0-7: his star throw,
             * his bombs' fire, the star; dancho01's from 8: his wand's charge and beam, its body listed last, as the others'); his hurt
             * look his float (0x1000001: cell 2). A frame 256 across: his beam's first piece (dancho01's 11 and 13, 48 ahead of him) reaches
             * 104 pixels on from there, where its rest (152 on) starts; 128's 64 cut it in the middle (the owner: "The ray of light
             * summoned from Daroach's wand attack is cutoff in the middle"); */
            new SqueakEnemy(49, "Daroach", "dancho00", new[] { 2, 2, 2 }, DaroachGlyphs, MidbossType: 0x40, ExtraFile: "dancho01",
                            SharePalettes: true, BodyLast: true, Canvas: 256, DamagePalette: "dan_dmg", MergeParts: true),
            // What he brings in (his stars 0x02124894, bombs 0x02124290, his beam's reach), drawn with his graphics, no name;
            new SqueakEnemy(50, "Daroach's thing", "dancho00", new[] { 0, 0, 0 }, new (int, int)[0], GraphicsOf: 49, FitVram: true),
            // Dark Daroach (0x3D with its parameter 1, a8s4s2's: dancho01's palette for both sprites, 0x02126F48's +0x1F8), as Daroach;
            new SqueakEnemy(51, "Dark Daroach", "dancho00", new[] { 2, 2, 2 }, DarkDaroachGlyphs, MidbossType: 0x40, ExtraFile: "dancho01",
                            SharePalettes: true, BodyLast: true, PaletteFile: "dancho01", Canvas: 256, DamagePalette: "dan_dmg",
                            MergeParts: true),
            new SqueakEnemy(52, "Dark Daroach's thing", "dancho00", new[] { 0, 0, 0 }, new (int, int)[0], GraphicsOf: 51, FitVram: true),
            /* Meta Knight (a boss, enemy 0x37: 0x02116C7C, on mini-boss type 0x40 as the mid-bosses), drawn from metaknight_cmn and
             * metaknight_ext (the sprite his body switches to, its +0xE4: his sword's big moves), both in cmn's palettes (Squeak Squad's
             * one palette object): the animations his code plays (squeak/meta_knight_kss.c's, in its order: cmn's 1, 2, 4-14, 16, 18-25,
             * 28; ext's 1-11, 13-17, 19-25: under KatAM's 253 frames), their .mca's composites (cmn's 0, 1 and 3, which he doesn't play;
             * ext's 0-3: his thrust's landing, his beams' ready, his sword raised, his call; the body listed last, as the others'); his
             * body (palette 0) his own, his sword (palette 1) a layer and his wave's light (palettes 2 and 3) another, a frame drawn
             * wholly with one of those his own (his sword's flourishes); his hurt look cmn's 25 (cell 98, here too); */
            new SqueakEnemy(53, "Meta Knight", "metaknight_cmn", new[] { 98, 98, 98 }, MetaKnightGlyphs, MidbossType: 0x40,
                            ExtraFile: "metaknight_ext", SharePalettes: true, BodyLast: true, Canvas: 192, KeepComposites: true,
                            OnlyAnims: new[] { 1, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 16, 18, 19, 20, 21, 22, 23, 24, 25, 28,
                                               40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 52, 53, 54, 55, 56, 58, 59, 60, 61, 62, 63, 64 },
                            Layers: new[] { new[] { 1 }, new[] { 2, 3 } }, WholeCells: true, MapIcon: new[] { 31 }),
            /* His tornado (0x02111CFC: metaknight_ext.mca's composites 4 and 5, their parts 26-30 and 32-36), no name; a frame big
             * enough for it (128 up); */
            new SqueakEnemy(54, "Meta Knight's tornado", "metaknight_ext", new[] { 0, 0, 0 }, new (int, int)[0], Canvas: 256,
                            KeepComposites: true, OnlyAnims: new[] { 26, 27, 28, 29, 30, 32, 33, 34, 35, 36 }),
            /* His beam (0x02111F90: metaknight_ext's 12), his wave (0x02111E44: its 18, its two palettes one) and the sparkles about Kirby
             * in his tornado (0x021115C4: its 31), each its own (its VRAM its own frames', not the tornado's), no name; */
            new SqueakEnemy(55, "Meta Knight's beam", "metaknight_ext", new[] { 0, 0, 0 }, new (int, int)[0], OnlyAnims: new[] { 12 }),
            new SqueakEnemy(56, "Meta Knight's wave", "metaknight_ext", new[] { 0, 0, 0 }, new (int, int)[0], OnlyAnims: new[] { 18 },
                            WholeCells: true),
            new SqueakEnemy(57, "Meta Knight's sparkle", "metaknight_ext", new[] { 0, 0, 0 }, new (int, int)[0], OnlyAnims: new[] { 31 }),
            /* Dark Nebula (the last boss, enemies 0x46 and 0x3F: 0x021376EC, 0x02122CD0, on mini-boss type 0x40 as the mid-bosses), drawn
             * from darkZERO_cmn and darkZERO_ext (one palette set: the files' are the same): the animations his code plays (squeak/
             * dark_nebula.c's, in its order: cmn's 1, 2, 6, 7, 8, ext's 22; then ext's 19-21 and 23-25, his eye opening's parts; then
             * ext's 14 and 15, his defeat's black star shaking and breaking up), ext's .mca 1-3 (his eye opening, looking, glinting); his
             * hurt look the black star (cmn's cell 17); */
            new SqueakEnemy(58, "Dark Nebula", "darkZERO_cmn", new[] { 16, 16, 16 }, DarkNebulaGlyphs, MidbossType: 0x40,
                            ExtraFile: "darkZERO_ext", SharePalettes: true, KeepComposites: true,
                            OnlyAnims: new[] { 1, 2, 6, 7, 8, 44, 41, 42, 43, 45, 46, 47, 36, 37 }, MapIcon: new[] { 94, 65 }),
            /* His eye (0x0211C4E0: cmn's 3, ext's 17 and 18), its twinkle (cmn's 5), his flames (0x0211B98C: cmn's 4), each its own (its
             * VRAM its own frames', his and theirs together under KatAM's 512 tiles), no name; */
            new SqueakEnemy(59, "Dark Nebula's eye", "darkZERO_cmn", new[] { 0, 0, 0 }, new (int, int)[0], ExtraFile: "darkZERO_ext",
                            SharePalettes: true, OnlyAnims: new[] { 3, 39, 40 }),
            new SqueakEnemy(60, "Dark Nebula's twinkle", "darkZERO_cmn", new[] { 0, 0, 0 }, new (int, int)[0], OnlyAnims: new[] { 5 }),
            new SqueakEnemy(61, "Dark Nebula's flames", "darkZERO_cmn", new[] { 0, 0, 0 }, new (int, int)[0], OnlyAnims: new[] { 4 }),
            /* What he brings in (0x0212FDC4's kinds): his stars (0), sparks (8) and flames (3) (cmn's 9 and 11), his fireball (1: cmn's 10),
             * its burst (2: cmn's 12 and 19, the fire wall cmn.mca's 0: its parts 13-18, up to 128 high), his ice pillar (7: cmn's 20, 128
             * high), his bolts (5: ext's 1-4) and their cage (6: ext.mca's 0, its parts ext's 5-12), no name; */
            new SqueakEnemy(62, "Dark Nebula's star", "darkZERO_cmn", new[] { 0, 0, 0 }, new (int, int)[0], OnlyAnims: new[] { 9, 11 }),
            new SqueakEnemy(63, "Dark Nebula's fireball", "darkZERO_cmn", new[] { 0, 0, 0 }, new (int, int)[0], OnlyAnims: new[] { 10 }),
            new SqueakEnemy(64, "Dark Nebula's fire", "darkZERO_cmn", new[] { 0, 0, 0 }, new (int, int)[0], Canvas: 256, KeepComposites: true,
                            OnlyAnims: new[] { 12, 19, 13, 14, 15, 16, 17, 18 }, FitVram: true, HighParts: 128),
            new SqueakEnemy(65, "Dark Nebula's ice", "darkZERO_cmn", new[] { 0, 0, 0 }, new (int, int)[0], Canvas: 256, OnlyAnims: new[] { 20 }),
            new SqueakEnemy(66, "Dark Nebula's bolt", "darkZERO_ext", new[] { 0, 0, 0 }, new (int, int)[0], KeepComposites: true,
                            OnlyAnims: new[] { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 }, MergeParts: true),
            /* The floor his flames set burning (the stage's fire tile, 0x02216BD8: a 16x16 picture of the stages' own tiles, the same in
             * every stage's map/*_f.bin, its palette 3 there: 0x70); */
            new SqueakEnemy(67, "Burning floor", "raw:map/25_f.bin@5490,70,16,16", new[] { 0, 0, 0 }, new (int, int)[0]),
            /* The dark star Kirby chases before Dark Nebula (enemy 0x3E: 0x0211B680), darkZERO_cmn's animation 1 (the small spinning
             * star), no name (nothing hits it); */
            new SqueakEnemy(68, "Dark star", "darkZERO_cmn", new[] { 0, 0, 0 }, new (int, int)[0], OnlyAnims: new[] { 1 }),
            /* The small star switch (gimmick 0, the DS's look 0: yakumono/switch_small, its states' animations 0 up, 1 going down, 2
             * down, 3 coming up, 0x02227F30): its animations 0 and 2, its variants 3 (up) and 4 (pressed), shown by object 0x8D
             * where a Squeak Squad room has one (squeak.c's SqueakAnimFor(), chest.c's SqueakCreateSwitch()); no name; */
            new SqueakEnemy(SqueakSwitch, "Switch", "yakumono/switch_small", new[] { 0, 0, 0 }, new (int, int)[0], OnlyAnims: new[] { 0, 2 }),
            /* The shutters (gimmick 2: its parameter's low 3 bits its look, 0x02227EA4: 0-2 yakumono/shutter, a stone pillar, at 0, 90
             * and 270 degrees, 3-6 yakumono/shutter_needle, spikes, at 0, 180, 270 and 90; the rooms use 0-4): every cell, the DS's
             * animations 0 shut, 1 opening, 2 open, 3 shutting; no name; */
            new SqueakEnemy(SqueakShutter, "Shutter", "yakumono/shutter", new[] { 0, 0, 0 }, new (int, int)[0]),
            new SqueakEnemy(SqueakShutter90, "Shutter", "yakumono/shutter", new[] { 0, 0, 0 }, new (int, int)[0], Rotate: 90),
            new SqueakEnemy(SqueakShutter270, "Shutter", "yakumono/shutter", new[] { 0, 0, 0 }, new (int, int)[0], Rotate: 270),
            new SqueakEnemy(SqueakNeedle, "Spikes", "yakumono/shutter_needle", new[] { 0, 0, 0 }, new (int, int)[0]),
            new SqueakEnemy(SqueakNeedle180, "Spikes", "yakumono/shutter_needle", new[] { 0, 0, 0 }, new (int, int)[0], Rotate: 180),
            /* The fire columns (gimmick 7: its parameter's low 4 bits its look, 0x02227B30, yakumono/fire_long at 0, 180, 270 and 90
             * degrees): every cell, the DS's animations 0 the pilot flame, 1 flaring up, 2 the column, 3 dying down; no name; */
            new SqueakEnemy(SqueakFireColumn, "Fire", "yakumono/fire_long", new[] { 0, 0, 0 }, new (int, int)[0]),
            new SqueakEnemy(SqueakFire180, "Fire", "yakumono/fire_long", new[] { 0, 0, 0 }, new (int, int)[0], Rotate: 180),
            new SqueakEnemy(SqueakFire270, "Fire", "yakumono/fire_long", new[] { 0, 0, 0 }, new (int, int)[0], Rotate: 270),
            new SqueakEnemy(SqueakFire90, "Fire", "yakumono/fire_long", new[] { 0, 0, 0 }, new (int, int)[0], Rotate: 90),
            /* The grass tufts (gimmick 0xE: its table's first byte's low nibble its look, 0x02227C28: yakumono/grass_A_S, grass_A_L,
             * grass_B_S, grass_B_L, grass_C_S, grass_C_L, small and large in turn; the animations squeak/gimmicks.c plays: 0 still,
             * 1 rustling, 5 its blades flying as it's cut, 6 its stub going) and the barrel (0xF, yakumono/barrel: 0 open, 2 shut,
             * 4 shaking, 5 bursting): every cell; no name; */
            new SqueakEnemy(SqueakGrass, "Grass", "yakumono/grass_A_S", new[] { 0, 0, 0 }, new (int, int)[0]),
            new SqueakEnemy(SqueakGrass + 1, "Grass", "yakumono/grass_A_L", new[] { 0, 0, 0 }, new (int, int)[0]),
            new SqueakEnemy(SqueakGrass + 2, "Grass", "yakumono/grass_B_S", new[] { 0, 0, 0 }, new (int, int)[0]),
            new SqueakEnemy(SqueakGrass + 3, "Grass", "yakumono/grass_B_L", new[] { 0, 0, 0 }, new (int, int)[0]),
            new SqueakEnemy(SqueakGrass + 4, "Grass", "yakumono/grass_C_S", new[] { 0, 0, 0 }, new (int, int)[0]),
            new SqueakEnemy(SqueakGrass + 5, "Grass", "yakumono/grass_C_L", new[] { 0, 0, 0 }, new (int, int)[0]),
            new SqueakEnemy(SqueakBarrel, "Barrel", "yakumono/barrel", new[] { 0, 0, 0 }, new (int, int)[0]),
            // The icicle (gimmick 9, yakumono/icicle: 0 hanging, 1 shaking, 2 falling, 3 shattering): every cell; no name;
            new SqueakEnemy(SqueakIcicle, "Icicle", "yakumono/icicle", new[] { 0, 0, 0 }, new (int, int)[0]),
            // The falling rock (gimmick 8, yakumono/rock: 0 appearing, 1 falling, spinning, 2 breaking): every cell; no name;
            new SqueakEnemy(SqueakRock, "Rock", "yakumono/rock", new[] { 0, 0, 0 }, new (int, int)[0]),
            // The moving platform (gimmick 4, yakumono/floor: looks 0 and 1 its animations 0 and 3, still; 2 and 5 a dip): every cell; no name;
            new SqueakEnemy(SqueakPlatformLook, "Floor", "yakumono/floor", new[] { 0, 0, 0 }, new (int, int)[0]),
            /* The thunder cloud (gimmick 5, yakumono/thunder_cloud): every cell, and its .mca's 4 composites as animations 14-17 (the
             * DS plays only those, 0x3000000-3: 14 resting, the cloud bobbing with its sparkles; 15 the zap, its green burst and sparks;
             * 16 shaking before it breaks, flashing; 17 breaking up in puffs); no name; */
            new SqueakEnemy(SqueakThunderLook, "Thunder Cloud", "yakumono/thunder_cloud", new[] { 0, 0, 0 }, new (int, int)[0],
                            Canvas: 160, FlattenComposites: true),
            /* The rolling boulder (gimmick 6, yakumono/ball_rock: 0 the rock, 1 and 2 its pieces spinning; 3 here its roll, cell 1 turned
             * a sixteenth a step): every cell; no name; */
            new SqueakEnemy(SqueakBoulderLook, "Boulder", "yakumono/ball_rock", new[] { 0, 0, 0 }, new (int, int)[0], Roll: 1),
            // The candle (gimmick 0xA, yakumono/candle: 0 lit, its flame flickering, 1 out): every cell; no name;
            new SqueakEnemy(SqueakCandleLook, "Candle", "yakumono/candle", new[] { 0, 0, 0 }, new (int, int)[0]),
            // The ability room's pedestals (see SqueakPedestalLook): the stand's look, the icons' (their pieces only); no name;
            new SqueakEnemy(SqueakPedestalLook, "Pedestal", "item/copy", new[] { 0, 0, 0 }, new (int, int)[0], OnlyAnims: SqueakPedestalAnims,
                            OnlyPalettes: new[] { 0 }),
            new SqueakEnemy(SqueakPedestalIconLook, "Pedestal", "item/copy", new[] { 0, 0, 0 }, new (int, int)[0], OnlyAnims: SqueakPedestalAnims,
                            OnlyPalettes: new[] { 1, 2, 3 }),
            /* Dark Nebula's ice floor (his pillar's, 0x0212EBC4: the stage's vtable +0x8C, 0x022146EC, the floor's top row an ice block
             * at each cell it passes, the cell over it its top, both from a8s4s6's front block table: 0xC0 alone, + 0x60 with one on its
             * right, + 0xC0 with one on its left, + the cell's x % 3; the top 0x20 before): cells 0-2 one with both, 3-5 alone, 6-8 the
             * left end, 9-11 the right end, each by x % 3, 12 the three with both side by side (x % 3 0, 1, 2: one sprite for three of
             * a long row's, a third of the drawing) (squeak/dark_nebula.c's IceFloor()); no name; */
            new SqueakEnemy(SqueakIceFloorLook, "Ice", "blocks:map/a8s4s6.mxp,map/27_f.bin@1:1E0/1C0,1E1/1C1,1E2/1C2,C0/A0,C1/A1,C2/A2," +
                                                       "120/100,121/101,122/102,180/160,181/161,182/162,1E0/1C0+1E1/1C1+1E2/1C2",
                            new[] { 0, 0, 0 }, new (int, int)[0]),
            /* The Ability Scrolls' icons (the Collection screen's, collection/skill: 23 cells and animations, one a scroll in the
             * treasures' order, 28-50, each in its own palette row): an Ability Scroll chest's popup (chest.c's NidlScrollPopup); no name; */
            new SqueakEnemy(SqueakScrollIconLook, "Scroll", "collection/skill", new[] { 0, 0, 0 }, new (int, int)[0]),
        };

        /* The randomizer's Squeak Squad pools (the ROM Expansion tab). SqueakPool; A character the randomizer places: its index (SQUEAK_*
         * in squeak.h), its variants (the template's unk12: KatAMEnemiesBase writes it in its Properties[1]) in a ground spot, in a
         * flying enemy's spot, in an underwater one (none: not there), and, for a mid-boss, its mini-boss type (MidbossType; its
         * behavior byte SQUEAK_MIDBOSS | its index). The rest of the cast (what a character throws or brings in, Daroach's stage scene,
         * the dark star's route) isn't placed on its own, nor are the bosses: in KatAM's mini-boss rooms some don't fit the camera
         * (Yadgaine and Dark Nebula mostly off-screen in the arenas tried), so they wait for rooms of their own (the owner's call);
         * • Enemies (squeak/*.c, each file's "Brain"): Perara floats where it's placed; Gold Waddle Dee 0-3 and 5 (4 is 0 again) and
         *   swims (0 forward, 5 still); Gussa only in water (out of it it falls); Crimp, Maiga, Spinni one brain; Cret 0-3 and Uja 0-1 fly
         *   from wherever they are, in a flying enemy's spot too; Bio Spark 0-2; Starman 1-3 on the ground, 0 hovers; Bubble Head 0-2 (5 is 1 again); Gaw Gaw 0, 1, 3
         *   (2 is 0); Acchi, Bun, Metalun 0-2; the Squeakers in their four colors; Doc (it flies, but
         *   only from ground spots: the flying spots' pool is small, and a Squeak would fill it); Storo 0-2;
         * • Mid-bosses: Big Metalun, Gao Gao, Tedhaun, Buboo; */
        sealed record SqueakPool(int Index, byte[] Variants, byte[] Flying = null, byte[] Water = null);
        static readonly byte[] SqueakV0 = { 0 }, SqueakV01 = { 0, 1 }, SqueakV012 = { 0, 1, 2 }, SqueakV0123 = { 0, 1, 2, 3 };
        static readonly List<SqueakPool> SqueakEnemyPool = new List<SqueakPool> {
            new SqueakPool(0, SqueakV0, Flying: SqueakV0),                                      // Perara;
            new SqueakPool(1, new byte[] { 0, 1, 2, 3, 5 }, Water: new byte[] { 0, 5 }),        // Gold Waddle Dee;
            new SqueakPool(2, new byte[0], Water: SqueakV01),                                   // Gussa;
            new SqueakPool(3, SqueakV0),                                                        // Crimp;
            new SqueakPool(5, SqueakV0),                                                        // Maiga;
            new SqueakPool(6, SqueakV0123, Flying: SqueakV0123),                                // Cret;
            new SqueakPool(8, SqueakV01, Flying: SqueakV01),                                    // Uja;
            new SqueakPool(9, SqueakV012),                                                      // Bio Spark;
            new SqueakPool(12, new byte[] { 1, 2, 3 }, Flying: SqueakV0),                       // Starman;
            new SqueakPool(13, SqueakV012),                                                     // Bubble Head;
            new SqueakPool(15, new byte[] { 0, 1, 3 }),                                         // Gaw Gaw;
            new SqueakPool(16, SqueakV012),                                                     // Acchi;
            new SqueakPool(18, SqueakV012),                                                     // Bun;
            new SqueakPool(19, SqueakV012),                                                     // Metalun;
            new SqueakPool(27, SqueakV0123),                                                    // Squeaker;
            new SqueakPool(29, SqueakV0),                                                       // Spinni;
            new SqueakPool(31, SqueakV0),                                                       // Doc (flies from a ground spot: a Squeak, not one of the few flyers);
            new SqueakPool(33, SqueakV012),                                                     // Storo;
        };
        /* The mid-bosses (Big Metalun, Gao Gao, Tedhaun, Buboo) and the chases' Squeaks as mini-bosses (Spinni, Doc, Storo: their fight
         * with Kirby, as the DS's hideouts', SqueakAsMidbossType; the owner: "including the 'chase sequence' mouses as minibosses"), and
         * the bosses (the owner: "Add Squeak Squad bosses to randomizer pools"; on a KatAM or NiDL mini-boss spot the flying ones keep
         * to its screen: squeak.c's SqueakBossFloor, Yadgaine pushes no one into its walls); */
        static readonly int[] SqueakMidbossPool = { 20, 21, 23, 25, 29, 31, 33 };
        static readonly int[] SqueakBossPool = { 35, 37, 40, 42, 47, 49, 51, 53, 58 };
        const byte SqueakAsMidboss = 0x40;
        static byte SqueakMidbossTypeOf(SqueakEnemy e) => e.MidbossType != 0 ? e.MidbossType : SqueakMidbossPool.Contains(e.Index) ? SqueakAsMidboss : (byte)0;
        // (the HP options' mid-bosses: the four; the Squeaks' HP are the enemies' pool's, the bosses' their own rooms');
        static readonly int[] SqueakMidbossHpPool = { 20, 21, 23, 25 };

        static SqueakEnemy SqueakDefinitionOf(int index) => SqueakDefinitions.First(e => e.Index == index);
        static string SqueakPoolName(int index) => SqueakDefinitionOf(index).Name;

        // The pools' characters (index, name) for the settings and the window: its enemies and mid-bosses;
        public static IReadOnlyList<(int index, string name)> SqueakEnemies => SqueakEnemyPool.Select(p => (p.Index, SqueakPoolName(p.Index))).ToList();
        public static IReadOnlyList<(int index, string name)> SqueakMidbosses => SqueakMidbossPool.Select(i => (i, SqueakPoolName(i))).ToList();
        public static IReadOnlyList<(int index, string name)> SqueakBossList => SqueakBossPool.Select(i => (i, SqueakPoolName(i))).ToList();

        /* SqueakGroupsOf(); The characters picked as the randomizer places them in type 0x2C's spots: one (behavior, its variants) per
         * character that can be there (a ground spot, a flying enemy's or an underwater one's); */
        public static List<(byte behavior, byte[] variants)> SqueakGroupsOf(IEnumerable<int> enemies, bool flying = false, bool water = false) =>
            SqueakEnemyPool.Where(p => enemies.Contains(p.Index))
                           .Select(p => ((byte)p.Index, (water ? p.Water : flying ? p.Flying : p.Variants) ?? Array.Empty<byte>()))
                           .Where(group => group.Item2.Length > 0).ToList();

        // SqueakMidbossGroupsOf(); The mid-bosses picked on a mini-boss type: their behavior bytes (SQUEAK_MIDBOSS | index), variant 0;
        public static List<(byte behavior, byte[] variants)> SqueakMidbossGroupsOf(IEnumerable<int> characters, byte type) =>
            SqueakDefinitions.Where(e => SqueakMidbossTypeOf(e) == type && characters.Contains(e.Index)
                                         && (SqueakMidbossPool.Contains(e.Index) || SqueakBossPool.Contains(e.Index)))
                             .Select(e => ((byte)(SqueakMidbossFlag | e.Index), SqueakV0)).ToList();
        const int SqueakMidbossFlag = 0x80;     // SQUEAK_MIDBOSS (squeak.h);

        // IsSqueakMidbossBehavior(); A mini-boss type's behavior byte that picks a Squeak Squad mid-boss or boss (NiDL's are under 0x60);
        public static bool IsSqueakMidbossBehavior(byte behavior) => behavior >= SqueakMidbossFlag;

        // SqueakNameOf(); The Squeak Squad character a behavior byte picks (type 0x2C's, or a mini-boss type's SQUEAK_MIDBOSS | index);
        public static string SqueakNameOf(byte behavior) {
            int index = behavior & ~SqueakMidbossFlag;
            return SqueakDefinitions.FirstOrDefault(e => e.Index == index)?.Name ?? SqueakDefinitions[0].Name;
        }

        /* SqueakNativeBehaviors; The behaviors Squeak Squad gives KatAM's own enemies that KatAM's lack ("ROM Expansion/squeak/natives.c":
         * their subtype1): Waddle Dee's 5-7 (patrol, a jump at Kirby, a jump on a timer), Glunk's 5-6 (Squeak Squad's two beats), Gordo's
         * 5 (circles), Snooter's 5 (stands facing Kirby); */
        public static readonly IReadOnlyDictionary<byte, byte[]> SqueakNativeBehaviors = new Dictionary<byte, byte[]> {
            [0x00] = new byte[] { 5, 6, 7 },
            [0x03] = new byte[] { 5, 6 },
            [0x06] = new byte[] { 5 },
            [0x07] = new byte[] { 5 },
        };

        /* The HP options: the pools' characters' HP (gSqueakEnemyHp, one HP for any number of players), by pool (KatAMHP.cs): its enemies
         * and mid-bosses; */
        public static IReadOnlyList<int> SqueakEnemyHpIndexes => SqueakEnemyPool.Select(p => p.Index).ToList();
        public static IReadOnlyList<int> SqueakMidbossHpIndexes => SqueakMidbossHpPool;

        /* WriteSqueak(); Object type 0x2C as type 0x31 is for NiDL's: spawn function and parameters (hurt frames 0 & 1, Waddle Dee's score
         * and HP, no ability, no palette key: each character loads its own, VRAM for the biggest frame, behavior start, one AnimInfo:
         * ObjectInitSprite reads it before the character's own frames show). Each character's animations as squeak.h's SqueakAnim
         * tables, gSqueakData, the HP bar names, and the object drawing (sub_0809A7A4) patched to ask SqueakAnimFor which anim and
         * variant to show: at +0x80, where it read the type's AnimInfo, `adds r0, r7, #0; ldr r1, =SqueakAnimFor; bl _call_via_r1;
         * lsls r1, r0, #16; lsrs r1, r1, #16; mov r8, r1; strh r1, [r6, #12]; lsrs r0, r0, #16; strb r0, [r6, #26]; b +0x38`;
         * the same in every version; */
        static string WriteSqueak(byte[] rom, KatAMExpansionCode.Version version, ref int cursor, List<SqueakBuilt> built,
                                  IReadOnlyDictionary<byte, int> minibossTiles,
                                  List<SqueakEffectBuilt> effects) {
            int codeBase = CodeAddress + 0x08000000, parameters = version.ObjectParameters + SqueakID * 0x18;
            int site = version.RomSymbols["sub_0809A7A4"] + 0x80;
            byte[] original = Convert.FromHexString("3D1C8235297848004018C000143280183B1C8333197800688900091809888846B181297848004018C000801819780068890009188878B076");

            if (!rom.AsSpan(site, original.Length).SequenceEqual(original)) return $"The {version.Name} ROM's object drawing doesn't match.";

            WriteInt(rom, version.SpawnTable + SqueakID * 4, codeBase + version.CodeSymbols["CreateSqueakEnemy"] + 1);
            /* KatAM's own enemies with Squeak Squad's extra behaviors (squeak/natives.c): each one's spawn (SqueakNativeTypes) and its
             * type's resume after a hit or bump (its parameters' +16) become the expansion's, KatAM's own kept in gSqueakNatives for them
             * to call; */
            for (int n = 0; n < SqueakNativeTypes.Length; n++) {
                (byte type, string spawn, string start) = SqueakNativeTypes[n];
                int spawnAt = version.SpawnTable + type * 4, startAt = version.ObjectParameters + type * 0x18 + 16;
                if (ReadInt(rom, spawnAt) >> 24 != 0x08 || ReadInt(rom, startAt) >> 24 != 0x08)
                    return $"The {version.Name} ROM's spawn table doesn't match.";
                WriteInt(rom, CodeAddress + version.CodeSymbols["gSqueakNatives"] + n * 8, ReadInt(rom, spawnAt));
                WriteInt(rom, CodeAddress + version.CodeSymbols["gSqueakNatives"] + n * 8 + 4, ReadInt(rom, startAt));
                WriteInt(rom, spawnAt, codeBase + version.CodeSymbols[spawn] + 1);
                if (start != null) WriteInt(rom, startAt, codeBase + version.CodeSymbols[start] + 1);
            }
            rom[parameters] = 0;
            rom[parameters + 1] = 1;
            Array.Copy(rom, version.ObjectParameters + 2, rom, parameters + 2, 4);
            WriteShort(rom, parameters + 6, 0);
            WriteShort(rom, parameters + 8, 0);
            WriteShort(rom, parameters + 10, 0);
            // (type 0x2C's VRAM for its biggest frame: the mid-bosses' are their mini-bosses types', which have to hold theirs);
            WriteInt(rom, parameters + 12, built.Where(b => b.Enemy.MidbossType == 0).Max(b => b.Tiles));
            foreach (SqueakBuilt midboss in built.Where(b => SqueakMidbossTypeOf(b.Enemy) != 0))
                if (!minibossTiles.TryGetValue(SqueakMidbossTypeOf(midboss.Enemy), out int typeTiles) || midboss.Tiles > typeTiles)
                    return $"{midboss.Enemy.Name}'s frames don't fit mini-boss type 0x{SqueakMidbossTypeOf(midboss.Enemy):X2}'s VRAM.";
            WriteInt(rom, parameters + 16, codeBase + version.CodeSymbols["SqueakStart"] + 1);
            int animInfo = Allocate(ref cursor, 4, 4);
            WriteShort(rom, animInfo, built[0].AnimId);
            rom[animInfo + 2] = SqueakHurtVariants;
            rom[animInfo + 3] = 0;
            WriteInt(rom, parameters + 20, animInfo + 0x08000000);
            Array.Copy(rom, version.ObjectParameters2, rom, version.ObjectParameters2 + SqueakID * 8, 8);

            // A SqueakAnim (8 bytes: steps, loops, its steps' address) at `at`, its steps' cells through `cell`;
            int localCursor = cursor;
            void WriteAnim(int at, SqueakAnim anim, Func<int, int> cell) {
                int steps = Allocate(ref localCursor, Math.Max(1, anim.Steps.Count) * 8, 4);
                for (int i = 0; i < anim.Steps.Count; i++) {
                    SqueakStep step = anim.Steps[i];
                    WriteShorts(rom, steps + i * 8, cell(step.Cell), step.Frames, step.X, step.Y);
                }
                rom[at] = (byte)anim.Steps.Count;
                rom[at + 1] = (byte)(anim.Loops ? 1 : 0);
                WriteShort(rom, at + 2, 0);
                WriteInt(rom, at + 4, steps + 0x08000000);
            }

            /* gSqueakData: names, the effects (16 bytes: anim ID, tiles, their animation: its cells are the effect's frames, their palette's
             * key and frame), then per character squeak.h's SqueakGraphics (24 bytes: anim ID, cells, animations, its SqueakAnim table, then
             * compositions, composite animations, the most tiles a frame takes, the SqueakComposition and SqueakComposite tables, its
             * animations' palettes); */
            int data = CodeAddress + version.CodeSymbols["gSqueakData"], namesAt = Allocate(ref localCursor, SqueakDefinitions.Count * 0x100, 4);
            WriteInt(rom, data, namesAt + 0x08000000);
            int effectTable = Allocate(ref localCursor, Math.Max(1, SqueakEffectDefinitions.Count) * 16, 4);
            WriteInt(rom, data + 4, effectTable + 0x08000000);
            foreach (SqueakEffectBuilt e in effects) {
                int at = effectTable + e.Effect.Index * 16;
                WriteShorts(rom, at, e.AnimId, e.Tiles);
                WriteAnim(at + 4, e.Anim, c => e.Cells.IndexOf(c));
                WriteShorts(rom, at + 12, e.Palette.Key, e.Palette.Variant);
            }
            foreach (SqueakBuilt b in built) {
                ComposeName(rom, version, b.Enemy.Glyphs).CopyTo(rom, namesAt + b.Enemy.Index * 0x100);
                List<SqueakAnim> anims = b.Character.Anims;
                int table = Allocate(ref localCursor, anims.Count * 8, 4);
                for (int a = 0; a < anims.Count; a++) WriteAnim(table + a * 8, anims[a], c => c);
                int entry = data + 8 + b.Enemy.Index * SqueakGraphicsSize;
                WriteShort(rom, entry, b.AnimId);
                rom[entry + 2] = (byte)b.Character.Cells.Count;
                rom[entry + 3] = (byte)anims.Count;
                WriteInt(rom, entry + 4, table + 0x08000000);

                // Compositions (8 bytes: parts, 3 bytes, its parts: 8 bytes each, anim, timing, 1 byte, x, y) and composite animations
                // (8 bytes: steps, loops, 2 bytes, its steps: 4 bytes each, composition, frames);
                List<List<SqueakPart>> compositions = b.Character.Compositions;
                List<SqueakComposite> composites = b.Character.Composites;
                int compositionTable = 0, compositeTable = 0;
                if (b.Enemy.BodyLast)
                    compositions = compositions.Select(c => c.Count > 1 ? new[] { c[^1] }.Concat(c.Take(c.Count - 1)).ToList() : c).ToList();
                /* (each part's sprite takes the tiles its own frames need: squeak.c's SqueakCreatePart, a part sprite too small for
                 * another composition's part in its place made again; its own sprite, part 0, the character's); */
                int PartTiles(SqueakPart part) => b.Character.Anims[part.Anim].Steps
                    .Select(step => b.CellTiles != null && step.Cell < b.CellTiles.Count ? b.CellTiles[step.Cell] : b.Tiles).DefaultIfEmpty(0).Max();
                if (compositions.Count > 0) {
                    compositionTable = Allocate(ref localCursor, compositions.Count * 8, 4);
                    for (int c = 0; c < compositions.Count; c++) {
                        int parts = Allocate(ref localCursor, Math.Max(1, compositions[c].Count) * 8, 4);
                        for (int i = 0; i < compositions[c].Count; i++) {
                            SqueakPart part = compositions[c][i];
                            WriteShorts(rom, parts + i * 8, part.Anim, (part.Absolute ? 1 : 0) | Math.Min(255, Math.Max(1, PartTiles(part))) << 8, part.X, part.Y);
                        }
                        rom[compositionTable + c * 8] = (byte)compositions[c].Count;
                        WriteInt(rom, compositionTable + c * 8 + 4, parts + 0x08000000);
                    }
                    compositeTable = Allocate(ref localCursor, Math.Max(1, composites.Count) * 8, 4);
                    for (int a = 0; a < composites.Count; a++) {
                        int steps = Allocate(ref localCursor, Math.Max(1, composites[a].Steps.Count) * 4, 4);
                        for (int i = 0; i < composites[a].Steps.Count; i++)
                            WriteShorts(rom, steps + i * 4, composites[a].Steps[i].Composition, composites[a].Steps[i].Frames);
                        rom[compositeTable + a * 8] = (byte)composites[a].Steps.Count;
                        rom[compositeTable + a * 8 + 1] = (byte)(composites[a].Loops ? 1 : 0);
                        WriteInt(rom, compositeTable + a * 8 + 4, steps + 0x08000000);
                    }
                }
                rom[entry + 8] = (byte)compositions.Count;
                rom[entry + 9] = (byte)composites.Count;
                WriteShort(rom, entry + 10, b.Tiles);
                WriteInt(rom, entry + 12, compositionTable == 0 ? 0 : compositionTable + 0x08000000);
                WriteInt(rom, entry + 16, compositeTable == 0 ? 0 : compositeTable + 0x08000000);
                // (its animations' palettes: 4 bytes each, key, frame; none: its own);
                int animPalettes = 0;
                if (b.AnimPalettes != null) {
                    animPalettes = Allocate(ref localCursor, b.AnimPalettes.Count * 4, 4);
                    for (int a = 0; a < b.AnimPalettes.Count; a++) WriteShorts(rom, animPalettes + a * 4, b.AnimPalettes[a].Key, b.AnimPalettes[a].Variant);
                }
                WriteInt(rom, entry + 20, animPalettes == 0 ? 0 : animPalettes + 0x08000000);
            }
            /* The layers (squeak.h's SqueakLayer, two of 12 bytes per character, after the characters' entries: anim ID, tiles, per cell
             * (a u16) its layer frame + 1, | 0x8000 in front; none: 0; its palette's key and frame); */
            int layerTable = Allocate(ref localCursor, SqueakDefinitions.Count * 24, 4);
            for (int i = 0; i < SqueakDefinitions.Count * 24; i++) rom[layerTable + i] = 0;
            foreach (SqueakBuilt b in built.Where(b => b.Layers != null)) {
                for (int k = 0; k < b.Layers.Count && k < 2; k++) {
                    int cellsAt = Allocate(ref localCursor, b.Layers[k].Cells.Length * 2, 4), at = layerTable + b.Enemy.Index * 24 + k * 12;
                    for (int c = 0; c < b.Layers[k].Cells.Length; c++) WriteShort(rom, cellsAt + c * 2, b.Layers[k].Cells[c]);
                    WriteShorts(rom, at, b.Layers[k].AnimId, b.Layers[k].Tiles);
                    WriteInt(rom, at + 4, cellsAt + 0x08000000);
                    WriteShorts(rom, at + 8, b.Layers[k].Palette.Key, b.Layers[k].Palette.Variant);
                }
            }
            WriteInt(rom, data + 8 + SqueakDefinitions.Count * SqueakGraphicsSize, layerTable + 0x08000000);
            /* The hurt flashes' colors (squeak.h's SqueakData.damage, after the layers': a pointer per character, NULL or its animations'
             * damage palettes, 4 bytes each, key and frame: DamagePalette); */
            int damageTable = Allocate(ref localCursor, SqueakDefinitions.Count * 4, 4);
            for (int i = 0; i < SqueakDefinitions.Count * 4; i++) rom[damageTable + i] = 0;
            foreach (SqueakBuilt b in built.Where(b => b.DamagePalettes != null)) {
                int keys = Allocate(ref localCursor, b.DamagePalettes.Count * 4, 4);
                for (int a = 0; a < b.DamagePalettes.Count; a++) WriteShorts(rom, keys + a * 4, b.DamagePalettes[a].Key, b.DamagePalettes[a].Variant);
                WriteInt(rom, damageTable + b.Enemy.Index * 4, keys + 0x08000000);
            }
            WriteInt(rom, data + 8 + SqueakDefinitions.Count * SqueakGraphicsSize + 4, damageTable + 0x08000000);
            /* The tiles each frame takes (squeak.h's SqueakData.cellTiles, after the hurt flashes': a pointer per character, NULL or its
             * count and then per variant, its hurt frames' (its graphics' owner's) and its cells', a u16 each: FitVram); */
            int cellTilesTable = Allocate(ref localCursor, SqueakDefinitions.Count * 4, 4);
            for (int i = 0; i < SqueakDefinitions.Count * 4; i++) rom[cellTilesTable + i] = 0;
            foreach (SqueakBuilt b in built.Where(b => b.Enemy.FitVram && b.CellTiles != null)) {
                SqueakEnemy owner = b.Enemy.GraphicsOf >= 0 ? built.First(o => o.Enemy.Index == b.Enemy.GraphicsOf).Enemy : b.Enemy;
                List<int> perVariant = owner.HurtCells.Select(c => b.CellTiles[c]).Concat(b.CellTiles).ToList();
                int at = Allocate(ref localCursor, (perVariant.Count + 1) * 2, 2);
                WriteShort(rom, at, perVariant.Count);
                for (int k = 0; k < perVariant.Count; k++) WriteShort(rom, at + 2 + k * 2, perVariant[k]);
                WriteInt(rom, cellTilesTable + b.Enemy.Index * 4, at + 0x08000000);
            }
            WriteInt(rom, data + 8 + SqueakDefinitions.Count * SqueakGraphicsSize + 8, cellTilesTable + 0x08000000);
            cursor = localCursor;

            int call = version.RomSymbols["_call_via_r1"] - (site + 4 + 4);
            WriteShorts(rom, site, 0x1C38, 0x4905, 0xF000 | (call >> 12 & 0x7FF), 0xF800 | (call >> 1 & 0x7FF), 0x0401, 0x0C09, 0x4688, 0x81B1, 0x0C00,
                        0x76B0, 0xE010, 0x46C0);
            WriteInt(rom, site + 0x18, codeBase + version.CodeSymbols["SqueakAnimFor"] + 1);
            for (int at = site + 0x1C; at < site + 0x38; at += 2) WriteShort(rom, at, 0x46C0);
            return null;
        }

        // The DS's (and GBA's) OAM sizes in pixels, by shape and size;
        static readonly (int w, int h)[,] SqueakPieceSizes = {
            { (8, 8), (16, 16), (32, 32), (64, 64) }, { (16, 8), (32, 8), (32, 16), (64, 32) }, { (8, 16), (8, 32), (16, 32), (32, 64) },
        };

        /* SqueakCanvas(); One of a character's cells drawn on a canvas (size x size, the character's origin at its center; values:
         * colors + 1 of `colorMap`, which maps (palette, color) to the KatAM palette's color; 0 transparent), the pieces drawn back to
         * front as the DS does (the first piece on top). Pieces of palettes `colorMap` lacks are left out. Mirrored: Squeak Squad's cells
         * face right, KatAM's enemies' frames left (KatAM flips them, about the origin, when the object faces right); */
        static int[,] SqueakCanvas(SqueakCharacter character, int cell, Dictionary<(int palette, int color), int> colorMap, int size = 128) {
            int[,] canvas = new int[size, size];
            int center = size / 2;
            SqueakCell squeakCell = character.Cells[cell];

            for (int p = squeakCell.Pieces.Count - 1; p >= 0; p--) {
                SqueakPiece piece = squeakCell.Pieces[p];
                (int w, int h) = SqueakPieceSizes[piece.Attr0 >> 14, piece.Attr1 >> 14];
                int x = piece.Attr1 & 0x1FF, y = piece.Attr0 & 0xFF, palette = piece.Attr2 >> 12;
                bool hflip = (piece.Attr1 & 0x1000) != 0, vflip = (piece.Attr1 & 0x2000) != 0;
                int firstTile = squeakCell.Tile + (piece.Attr2 & 0x3FF) * 4;

                if (x >= 256) x -= 512;
                if (y >= 128) y -= 256;
                for (int py = 0; py < h; py++)
                    for (int px = 0; px < w; px++) {
                        int tile = firstTile + py / 8 * (w / 8) + px / 8, at = tile * 32 + py % 8 * 4 + px % 8 / 2;
                        int value = at < character.Tiles.Length ? character.Tiles[at] >> (px % 2 * 4) & 15 : 0;
                        if (value == 0 || !colorMap.TryGetValue((palette, value), out int color)) continue;
                        int cx = center - 1 - (x + (hflip ? w - 1 - px : px)), cy = center + y + (vflip ? h - 1 - py : py);
                        if (cx >= 0 && cy >= 0 && cx < size && cy < size) canvas[cx, cy] = color;
                    }
            }
            if (!character.Angles.TryGetValue(cell, out int angle)) return canvas;
            // (an affine step's cell: turned about the middle by its angle, each pixel from the one it comes from; the canvas mirrored,
            // the DS's turn the other way here);
            int[,] turnedCanvas = new int[size, size];
            double radians = angle * 2 * Math.PI / 4096, cos = Math.Cos(radians), sin = Math.Sin(radians);
            for (int x = 0; x < size; x++)
                for (int y = 0; y < size; y++) {
                    double dx = x + 0.5 - center, dy = y + 0.5 - center;
                    int fx = (int)Math.Floor(center + dx * cos - dy * sin), fy = (int)Math.Floor(center + dx * sin + dy * cos);
                    if (fx >= 0 && fy >= 0 && fx < size && fy < size) turnedCanvas[x, y] = canvas[fx, fy];
                }
            return turnedCanvas;
        }

        // SqueakCellPalettes(); The palettes a cell's pieces use;
        static HashSet<int> SqueakCellPalettes(SqueakCharacter character, int cell) => character.Cells[cell].Pieces.Select(p => p.Attr2 >> 12).ToHashSet();

        /* SqueakUsedColors(); The (palette, color) pairs the cells draw with (color 0 is transparent), in the order of their palettes and
         * colors; */
        static List<(int palette, int color)> SqueakUsedColors(SqueakCharacter character, IEnumerable<int> cells) {
            HashSet<(int, int)> used = new HashSet<(int, int)>();
            foreach (int cell in cells)
                foreach (SqueakPiece piece in character.Cells[cell].Pieces) {
                    (int w, int h) = SqueakPieceSizes[piece.Attr0 >> 14, piece.Attr1 >> 14];
                    int firstTile = character.Cells[cell].Tile + (piece.Attr2 & 0x3FF) * 4;
                    for (int at = firstTile * 32; at < (firstTile + w * h / 64) * 32 && at < character.Tiles.Length; at++) {
                        if ((character.Tiles[at] & 15) != 0) used.Add((piece.Attr2 >> 12, character.Tiles[at] & 15));
                        if (character.Tiles[at] >> 4 != 0) used.Add((piece.Attr2 >> 12, character.Tiles[at] >> 4));
                    }
                }
            return used.OrderBy(u => u.Item1).ThenBy(u => u.Item2).ToList();
        }

        /* SqueakTurned(); A cell's canvas turned about its middle by `degrees` clockwise (90, 180, 270; 0 as it is), as the DS turns a
         * gimmick's sprite (its look's angle: a shutter's 0x5A, 90 degrees, puts the top of its pillar on its right: SqueakFrames()'s
         * `rotate`, its canvas mirrored, turns it the other way, the object shown facing right); a pixel's square (dx, dy) to (dx + 1,
         * dy + 1) from the middle goes to (-dy - 1, dx) at 90 degrees; */
        static int[,] SqueakTurned(int[,] canvas, int degrees) {
            if (degrees % 360 == 0) return canvas;
            int size = canvas.GetLength(0), c = size / 2;
            int[,] turned = new int[size, size];
            for (int x = 0; x < size; x++)
                for (int y = 0; y < size; y++) {
                    int dx = x - c, dy = y - c;
                    (int nx, int ny) = (degrees % 360) switch { 90 => (c - dy - 1, c + dx), 180 => (c - dx - 1, c - dy - 1), _ => (c + dy, c - dx - 1) };
                    if (nx >= 0 && ny >= 0 && nx < size && ny < size) turned[nx, ny] = canvas[x, y];
                }
            return turned;
        }

        /* SqueakFrames(); A character's cells (all, or `cells`) as KatAM frames (tiles, OAM pieces, attributes, palette index) and the palettes they use
         * (32 bytes each). Characters whose cells draw with at most 15 colors in all get one palette (the colors of every palette they
         * use, merged); the others one per palette of the .cla, each cell with its own (cells mixing palettes keep their first's colors,
         * the pieces of the others left out; `mergeShared`: palettes a cell draws with together merged into one, after the .cla's, when
         * their colors fit in 15); */
        static (List<(byte[] tiles, ushort[] oams, short[] attributes, int palette)> frames, List<byte[]> palettes) SqueakFrames(SqueakCharacter character,
                                                                                                                                int size = 128,
                                                                                                                                List<int> cells = null,
                                                                                                                                List<List<ushort[]>> alternates = null,
                                                                                                                                List<byte[]> alternatePalettes = null,
                                                                                                                                (ushort from, ushort to)[] merge = null,
                                                                                                                                bool mergeShared = false,
                                                                                                                                (int num, int den) scale = default,
                                                                                                                                int rotate = 0) {
            cells ??= Enumerable.Range(0, character.Cells.Count).ToList();
            List<(int palette, int color)> used = SqueakUsedColors(character, cells).Where(u => u.palette < character.Palettes.Count).ToList();
            List<byte[]> palettes = new List<byte[]>();
            List<(byte[], ushort[], short[], int)> frames = new List<(byte[], ushort[], short[], int)>();

            /* (a cell drawn, at `scale` of its size if it's drawn smaller: SqueakShrink, its pixels by its frame's palette's
             * brightness); */
            int[,] Drawn(int cell, Dictionary<(int, int), int> colorMap, byte[] palette) {
                // (the canvas is the DS's look mirrored, KatAM's frames facing left: turned the other way, then shown facing right);
                int[,] canvas = SqueakTurned(SqueakCanvas(character, cell, colorMap, size), (360 - rotate) % 360);
                if (scale.num == 0) return canvas;
                int center = size / 2;
                int[,] small = new int[size, size];
                foreach (((int i, int j), int value) in SqueakShrink((x, y) => canvas[x, y] == 0 ? -1 : canvas[x, y], size, size, center, center,
                                                                     scale.num, scale.den, c => SqueakBrightness(ReadShort(palette, c * 2))))
                    if (center + i >= 0 && center + j >= 0 && center + i < size && center + j < size) small[center + i, center + j] = value;
                return small;
            }
            byte[] PaletteBytes(IEnumerable<ushort> colors) {
                byte[] bytes = new byte[32];
                int i = 0;
                foreach (ushort color in colors) WriteShort(bytes, i++ * 2, color);
                return bytes;
            }

            // (a color as drawn: `merge`'s other color for it);
            ushort Value(int palette, int color) {
                ushort value = character.Palettes[palette][color];
                foreach ((ushort from, ushort to) in merge ?? new (ushort, ushort)[0])
                    if (value == from) return to;
                return value;
            }
            if (used.Select(u => Value(u.palette, u.color)).Distinct().Count() <= 15) {
                // One palette: color 0 (transparent) the first palette's, then each color used once;
                List<ushort> colors = new List<ushort> { character.Palettes[0][0] };
                Dictionary<(int, int), int> colorMap = new Dictionary<(int, int), int>();
                foreach ((int palette, int color) in used) {
                    ushort value = Value(palette, color);
                    int index = colors.IndexOf(value, 1);
                    if (index < 0) {
                        index = colors.Count;
                        colors.Add(value);
                    }
                    colorMap[(palette, color)] = index;
                }
                /* (a character with all its colors in its first palette keeps them where they are; so does one in any one palette, or in
                 * palettes of the same colors, without other colors: Squeak Squad's palette as it is, shared with others in it); */
                int keptPalette = used.All(u => u.palette == 0) ? 0
                                  : alternates == null && used.All(u => character.Palettes[u.palette].SequenceEqual(character.Palettes[used[0].palette])) ? used[0].palette : -1;
                bool kept = keptPalette >= 0;
                if (kept) {
                    colors = character.Palettes[keptPalette].ToList();
                    colorMap = used.ToDictionary(u => u, u => u.color);
                }
                palettes.Add(PaletteBytes(colors));
                // Its other colors: the same places, each the value of its other palette file's (the first one's where colors are one);
                foreach (List<ushort[]> alternate in alternates ?? new List<List<ushort[]>>()) {
                    if (kept) {
                        alternatePalettes.Add(PaletteBytes(alternate[0]));
                        continue;
                    }
                    ushort[] other = colors.ToArray();
                    HashSet<int> set = new HashSet<int>();
                    other[0] = alternate[0][0];
                    foreach ((int palette, int color) in used)
                        if (set.Add(colorMap[(palette, color)])) other[colorMap[(palette, color)]] = alternate[palette][color];
                    alternatePalettes.Add(PaletteBytes(other));
                }
                foreach (int cell in cells) {
                    (byte[] tiles, ushort[] oams, short[] attributes) = CutFrame(Drawn(cell, colorMap, palettes[0]), int.MaxValue, size);
                    frames.Add((tiles, oams, attributes, 0));
                }
            } else {
                if (alternates != null) throw new InvalidOperationException("A character with other colors needs one palette.");
                foreach (ushort[] palette in character.Palettes) palettes.Add(PaletteBytes(palette));
                /* (mergeShared: the palettes drawn together in a cell, a group each, while their colors fit in 15: the group's palette after
                 * the .cla's, its colors each used once); */
                int[] group = Enumerable.Range(0, character.Palettes.Count).ToArray();
                int Root(int p) => group[p] == p ? p : Root(group[p]);
                List<(int palette, int color)> GroupColors(int root) => used.Where(u => Root(u.palette) == root).ToList();
                if (mergeShared) {
                    foreach (int cell in cells) {
                        List<int> drawn = character.Cells[cell].Pieces.Select(p => p.Attr2 >> 12).Where(p => p < character.Palettes.Count).Distinct().ToList();
                        for (int i = 1; i < drawn.Count; i++) {
                            int a = Root(drawn[0]), b = Root(drawn[i]);
                            if (a == b) continue;
                            if (GroupColors(a).Concat(GroupColors(b)).Select(u => Value(u.palette, u.color)).Distinct().Count() > 15) continue;
                            group[b] = a;
                        }
                    }
                }
                Dictionary<int, (int palette, Dictionary<(int, int), int> colorMap)> merged = new Dictionary<int, (int, Dictionary<(int, int), int>)>();
                foreach (int root in Enumerable.Range(0, character.Palettes.Count).Where(p => Root(p) == p
                                                                                              && Enumerable.Range(0, character.Palettes.Count).Count(q => Root(q) == p) > 1)) {
                    List<ushort> colors = new List<ushort> { character.Palettes[root][0] };
                    Dictionary<(int, int), int> colorMap = new Dictionary<(int, int), int>();
                    foreach ((int palette, int color) in GroupColors(root)) {
                        ushort value = Value(palette, color);
                        int index = colors.IndexOf(value, 1);
                        if (index < 0) {
                            index = colors.Count;
                            colors.Add(value);
                        }
                        colorMap[(palette, color)] = index;
                    }
                    palettes.Add(PaletteBytes(colors));
                    merged[root] = (palettes.Count - 1, colorMap);
                }
                foreach (int cell in cells) {
                    int palette = character.Cells[cell].Pieces.Count > 0 ? character.Cells[cell].Pieces[0].Attr2 >> 12 : 0;
                    Dictionary<(int, int), int> colorMap = Enumerable.Range(1, 15).ToDictionary(c => (palette, c), c => c);
                    if (palette < character.Palettes.Count && merged.TryGetValue(Root(palette), out var m)) (palette, colorMap) = m;
                    (byte[] tiles, ushort[] oams, short[] attributes) = CutFrame(Drawn(cell, colorMap, palettes[Math.Min(palette, palettes.Count - 1)]), int.MaxValue, size);
                    frames.Add((tiles, oams, attributes, palette));
                }
            }
            return (frames, palettes);
        }
    }
}
