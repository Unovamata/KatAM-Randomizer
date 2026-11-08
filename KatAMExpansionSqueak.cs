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
         * end; */
        static byte[] SqueakFile(byte[] rom, string path) {
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
                    if (name == parts[p]) {
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

        // A character's graphics: tiles, palettes (16 colors each), cells and animations;
        internal sealed class SqueakCharacter {
            public byte[] Tiles;
            public List<ushort[]> Palettes;
            public List<SqueakCell> Cells;
            public List<SqueakAnim> Anims;
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
            character.Anims = Enumerable.Range(0, ReadShort(cea, anims)).Select(a => {
                int at = anims + ReadInt(cea, anims + 8 + a * 4);
                return new SqueakAnim(cea[at + 1] != 0, Enumerable.Range(0, cea[at])
                    .Select(s => new SqueakStep(ReadShort(cea, at + 8 + s * 8), ReadShort(cea, at + 10 + s * 8),
                                                   (short)ReadShort(cea, at + 12 + s * 8), (short)ReadShort(cea, at + 14 + s * 8))).ToList());
            }).ToList();
            return character;
        }

        // The type Squeak Squad's characters share (squeak.h's SQUEAK_TYPE: KatAM's last free regular enemy type);
        public const byte SqueakID = 0x2C;
        // The hurt frames every character's anim starts with (squeak.h's SQUEAK_HURT_VARIANTS: the type's hurt indexes 0 and 1, and 2);
        const int SqueakHurtVariants = 3;

        /* A Squeak Squad character: its index (squeak.h's SQUEAK_*), name, files (enemy/<File>.*), the cells its hurt frames show (its
         * hurt animations': SqueakHurtVariants), and its HP bar name, made of the letters of KatAM's names (object type, letter); */
        internal sealed record SqueakEnemy(int Index, string Name, string File, int[] HurtCells, (int type, int letter)[] Glyphs);
        internal sealed record SqueakBuilt(SqueakEnemy Enemy, int AnimId, SqueakCharacter Character, int Tiles);

        /* Squeak Squad's effects the ported characters show (squeak.h's SQUEAK_FX_*): an animation of effect/<File>.* (0x02021900's types
         * name them through the table at 0x02157DF8), each its own anim of the cells it shows; */
        internal sealed record SqueakEffect(int Index, string File, int Anim);
        internal sealed record SqueakEffectBuilt(SqueakEffect Effect, int AnimId, List<int> Cells, SqueakAnim Anim, int Tiles);

        static readonly List<SqueakEffect> SqueakEffectDefinitions = new List<SqueakEffect> {
            new SqueakEffect(0, "efc_cmn0", 19),    // Type 3: the dust puff (Gold Waddle Dee's steps, Perara's launch);
        };

        /* SqueakEffectBuild(); An effect's cells (those its animation shows, in its order) as KatAM frames and their palettes, the anim
         * added by `addAnim` (palettes, frames: its anim ID and the most tiles a frame takes); */
        static SqueakEffectBuilt SqueakEffectBuild(byte[] squeak, SqueakEffect effect,
                                                   Func<List<byte[]>, List<(byte[] tiles, ushort[] oams, short[] attributes, int palette)>, int> addAnim) {
            SqueakCharacter character = ReadSqueakCharacter(squeak, effect.File, "effect");
            SqueakAnim anim = character.Anims[effect.Anim];
            List<int> cells = anim.Steps.Select(s => s.Cell).Distinct().ToList();
            (List<(byte[] tiles, ushort[] oams, short[] attributes, int palette)> frames, List<byte[]> palettes) = SqueakFrames(character, cells: cells);
            return new SqueakEffectBuilt(effect, addAnim(palettes, frames), cells, anim, frames.Max(f => f.tiles.Length / 32));
        }

        static readonly List<SqueakEnemy> SqueakDefinitions = new List<SqueakEnemy> {
            // Perara: hurt animations 7 and 8 (0x02021A88's 0x1000007, 0x1000008);
            new SqueakEnemy(0, "Perara", "perara", new[] { 18, 19, 19 },
                            new[] { (0x16, 0), (0x14, 4), (0x01, 1), (0x14, 5), (0x01, 1), (0x14, 5) }),   // "P" (Pengy), "e", "a" (Hot Head), "r" (Bronto Burt);
            // Gold Waddle Dee: hurt animations 13 and 14 (0x020B6764's 0x100000D, 0x100000E);
            new SqueakEnemy(1, "Gold Waddle Dee", "gwaddledee", new[] { 9, 10, 10 },
                            new[] { (0x03, 0), (0x01, 2), (0x03, 1), (0x00, 2), (-1, 0), (0x00, 0), (0x00, 1), (0x00, 2), (0x00, 3), (0x00, 4),
                                    (0x00, 5), (-1, 0), (0x00, 6), (0x00, 7), (0x00, 8) }),   // "G" (Glunk), "o" (Bronto Burt), "l" (Glunk), "d", "Waddle Dee";
        };

        public static IReadOnlyList<(int index, string name)> SqueakEnemies => SqueakDefinitions.Select(e => (e.Index, e.Name)).ToList();

        /* WriteSqueak(); Object type 0x2C as type 0x31 is for NiDL's: spawn function and parameters (hurt frames 0 & 1, Waddle Dee's score
         * and HP, no ability, no palette key: each character loads its own, VRAM for the biggest frame, behavior start, one AnimInfo:
         * ObjectInitSprite reads it before the character's own frames show). Each character's animations as squeak.h's SqueakAnim
         * tables, gSqueakData, the HP bar names, and the object drawing (sub_0809A7A4) patched to ask SqueakAnimFor which anim and
         * variant to show: at +0x80, where it read the type's AnimInfo, `adds r0, r7, #0; ldr r1, =SqueakAnimFor; bl _call_via_r1;
         * lsls r1, r0, #16; lsrs r1, r1, #16; mov r8, r1; strh r1, [r6, #12]; lsrs r0, r0, #16; strb r0, [r6, #26]; b +0x38`;
         * the same in every version; */
        static string WriteSqueak(byte[] rom, KatAMExpansionCode.Version version, ref int cursor, List<SqueakBuilt> built,
                                  List<SqueakEffectBuilt> effects) {
            int codeBase = CodeAddress + 0x08000000, parameters = version.ObjectParameters + SqueakID * 0x18;
            int site = version.RomSymbols["sub_0809A7A4"] + 0x80;
            byte[] original = Convert.FromHexString("3D1C8235297848004018C000143280183B1C8333197800688900091809888846B181297848004018C000801819780068890009188878B076");

            if (!rom.AsSpan(site, original.Length).SequenceEqual(original)) return $"The {version.Name} ROM's object drawing doesn't match.";

            WriteInt(rom, version.SpawnTable + SqueakID * 4, codeBase + version.CodeSymbols["CreateSqueakEnemy"] + 1);
            rom[parameters] = 0;
            rom[parameters + 1] = 1;
            Array.Copy(rom, version.ObjectParameters + 2, rom, parameters + 2, 4);
            WriteShort(rom, parameters + 6, 0);
            WriteShort(rom, parameters + 8, 0);
            WriteShort(rom, parameters + 10, 0);
            WriteInt(rom, parameters + 12, built.Max(b => b.Tiles));
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

            /* gSqueakData: names, the effects (anim ID, tiles, their animation: its cells are the effect's frames), then per character
             * (anim ID, cells, animations, its SqueakAnim table); */
            int data = CodeAddress + version.CodeSymbols["gSqueakData"], namesAt = Allocate(ref localCursor, SqueakDefinitions.Count * 0x100, 4);
            WriteInt(rom, data, namesAt + 0x08000000);
            int effectTable = Allocate(ref localCursor, Math.Max(1, SqueakEffectDefinitions.Count) * 12, 4);
            WriteInt(rom, data + 4, effectTable + 0x08000000);
            foreach (SqueakEffectBuilt e in effects) {
                int at = effectTable + e.Effect.Index * 12;
                WriteShorts(rom, at, e.AnimId, e.Tiles);
                WriteAnim(at + 4, e.Anim, c => e.Cells.IndexOf(c));
            }
            foreach (SqueakBuilt b in built) {
                ComposeName(rom, version, b.Enemy.Glyphs).CopyTo(rom, namesAt + b.Enemy.Index * 0x100);
                List<SqueakAnim> anims = b.Character.Anims;
                int table = Allocate(ref localCursor, anims.Count * 8, 4);
                for (int a = 0; a < anims.Count; a++) WriteAnim(table + a * 8, anims[a], c => c);
                int entry = data + 8 + b.Enemy.Index * 8;
                WriteShort(rom, entry, b.AnimId);
                rom[entry + 2] = (byte)b.Character.Cells.Count;
                rom[entry + 3] = (byte)anims.Count;
                WriteInt(rom, entry + 4, table + 0x08000000);
            }
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
            return canvas;
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

        /* SqueakFrames(); A character's cells (all, or `cells`) as KatAM frames (tiles, OAM pieces, attributes, palette index) and the palettes they use
         * (32 bytes each). Characters whose cells draw with at most 15 colors in all get one palette (the colors of every palette they
         * use, merged); the others one per palette of the .cla, each cell with its own (cells mixing palettes keep their first's colors,
         * the pieces of the others left out); */
        static (List<(byte[] tiles, ushort[] oams, short[] attributes, int palette)> frames, List<byte[]> palettes) SqueakFrames(SqueakCharacter character,
                                                                                                                                int size = 128,
                                                                                                                                List<int> cells = null) {
            cells ??= Enumerable.Range(0, character.Cells.Count).ToList();
            List<(int palette, int color)> used = SqueakUsedColors(character, cells).Where(u => u.palette < character.Palettes.Count).ToList();
            List<byte[]> palettes = new List<byte[]>();
            List<(byte[], ushort[], short[], int)> frames = new List<(byte[], ushort[], short[], int)>();

            byte[] PaletteBytes(IEnumerable<ushort> colors) {
                byte[] bytes = new byte[32];
                int i = 0;
                foreach (ushort color in colors) WriteShort(bytes, i++ * 2, color);
                return bytes;
            }

            if (used.Select(u => character.Palettes[u.palette][u.color]).Distinct().Count() <= 15) {
                // One palette: color 0 (transparent) the first palette's, then each color used once;
                List<ushort> colors = new List<ushort> { character.Palettes[0][0] };
                Dictionary<(int, int), int> colorMap = new Dictionary<(int, int), int>();
                foreach ((int palette, int color) in used) {
                    ushort value = character.Palettes[palette][color];
                    int index = colors.IndexOf(value, 1);
                    if (index < 0) {
                        index = colors.Count;
                        colors.Add(value);
                    }
                    colorMap[(palette, color)] = index;
                }
                // (a character with all its colors in its first palette keeps them where they are);
                if (used.All(u => u.palette == 0)) {
                    colors = character.Palettes[0].ToList();
                    colorMap = used.ToDictionary(u => u, u => u.color);
                }
                palettes.Add(PaletteBytes(colors));
                foreach (int cell in cells) {
                    (byte[] tiles, ushort[] oams, short[] attributes) = CutFrame(SqueakCanvas(character, cell, colorMap, size), int.MaxValue, size);
                    frames.Add((tiles, oams, attributes, 0));
                }
            } else {
                foreach (ushort[] palette in character.Palettes) palettes.Add(PaletteBytes(palette));
                foreach (int cell in cells) {
                    int palette = character.Cells[cell].Pieces.Count > 0 ? character.Cells[cell].Pieces[0].Attr2 >> 12 : 0;
                    Dictionary<(int, int), int> colorMap = Enumerable.Range(1, 15).ToDictionary(c => (palette, c), c => c);
                    (byte[] tiles, ushort[] oams, short[] attributes) = CutFrame(SqueakCanvas(character, cell, colorMap, size), int.MaxValue, size);
                    frames.Add((tiles, oams, attributes, palette));
                }
            }
            return (frames, palettes);
        }
    }
}
