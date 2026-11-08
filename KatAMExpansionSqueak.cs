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
        }

        /* SqueakMerge(); Two files' graphics as one character: the second's tiles, palettes, cells and animations after the first's (its
         * cells' tiles and pieces' palettes moved on), and its composite animations (.mca) after the first's (their parts' animations
         * and steps' compositions moved on). Palettes that don't all fit share: the second's palettes the same as one of the first's
         * are that one (Mrs. Moley's three files have the same 16); */
        static SqueakCharacter SqueakMerge(SqueakCharacter a, SqueakCharacter b) {
            int tiles = a.Tiles.Length / 32, cells = a.Cells.Count, anims = a.Anims.Count, compositions = a.Compositions.Count;
            List<ushort[]> palettes = a.Palettes.ToList();
            int[] paletteOf;

            if (palettes.Count + b.Palettes.Count <= 16) {
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
         * animations and composite ones (.mca) kept; */
        static SqueakCharacter SqueakSplitLayer(SqueakCharacter c, int[] palettes, bool layer) =>
            new SqueakCharacter {
                Tiles = c.Tiles, Palettes = c.Palettes, Anims = c.Anims, Compositions = c.Compositions, Composites = c.Composites,
                Cells = c.Cells.Select(cell => new SqueakCell(cell.Tile, cell.Pieces.Where(p => palettes.Contains(p.Attr2 >> 12) == layer).ToList())).ToList(),
            };

        /* SqueakKeepAnims(); The character with only these animations (as its 0, 1...) and the cells they show (renumbered in the order
         * they first show), its tiles and palettes as they were. Its composite animations are kept (keepComposites: Mrs. Moley's, their
         * parts' animations then among those kept, renumbered), else dropped; */
        static SqueakCharacter SqueakKeepAnims(SqueakCharacter character, int[] anims, bool keepComposites = false) {
            List<int> cells = anims.SelectMany(a => character.Anims[a].Steps.Select(s => s.Cell)).Distinct().ToList();
            int Kept(int anim) {
                int at = Array.IndexOf(anims, anim);
                if (at < 0) throw new InvalidOperationException($"A composite animation's part plays animation {anim}, not kept.");
                return at;
            }
            return new SqueakCharacter {
                Tiles = character.Tiles, Palettes = character.Palettes, Cells = cells.Select(c => character.Cells[c]).ToList(),
                Anims = anims.Select(a => character.Anims[a] with {
                    Steps = character.Anims[a].Steps.Select(s => s with { Cell = cells.IndexOf(s.Cell) }).ToList() }).ToList(),
                Compositions = keepComposites
                    ? character.Compositions.Select(c => c.Select(part => part with { Anim = Kept(part.Anim) }).ToList()).ToList()
                    : new List<List<SqueakPart>>(),
                Composites = keepComposites ? character.Composites : new List<SqueakComposite>(),
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
            character.Anims = Enumerable.Range(0, ReadShort(cea, anims)).Select(a => {
                int at = anims + ReadInt(cea, anims + 8 + a * 4);
                return new SqueakAnim(cea[at + 1] != 0, Enumerable.Range(0, cea[at])
                    .Select(s => new SqueakStep(ReadShort(cea, at + 8 + s * 8), ReadShort(cea, at + 10 + s * 8),
                                                   (short)ReadShort(cea, at + 12 + s * 8), (short)ReadShort(cea, at + 14 + s * 8))).ToList());
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
         * composite animations (their parts' animations among those kept: Mrs. Moley's, from her other two files); */
        internal sealed record SqueakEnemy(int Index, string Name, string File, int[] HurtCells, (int type, int letter)[] Glyphs, int GraphicsOf = -1,
                                            byte MidbossType = 0, int[] OnlyAnims = null, string[] Palettes = null,
                                            (ushort from, ushort to)[] MergeColors = null, string ExtraFile = null,
                                            int[][] Layers = null, int Canvas = 128, bool BodyLast = false, bool KeepComposites = false);
        /* AnimPalettes: a character whose cells are in Squeak Squad's different palettes (Doc's things), per animation the palette its
         * sprite takes (SqueakPaletteKey); */
        internal sealed record SqueakBuilt(SqueakEnemy Enemy, int AnimId, SqueakCharacter Character, int Tiles, List<SqueakLayerBuilt> Layers = null,
                                           List<SqueakPaletteKey> AnimPalettes = null, List<int> CellTiles = null);
        // A character's layer (Layers): its anim, the most tiles a frame takes, per cell its frame + 1 (0: none; | 0x80: in front);
        internal sealed record SqueakLayerBuilt(int AnimId, int Tiles, byte[] Cells, SqueakPaletteKey Palette);
        /* A palette's slot (squeak.c's SqueakLoadPalette): the key and frame of the first sprite built with it, those after it with the same
         * colors taking it too (one of the 7 palette slots KatAM keeps for a room's enemies, not one each: sub_0803DFAC); */
        internal sealed record SqueakPaletteKey(int Key, int Variant);

        /* Squeak Squad's effects the ported characters show (squeak.h's SQUEAK_FX_*): an animation of <File>.* ("folder/name"; 0x02021900's
         * types name effect/efc_cmn* ones through the table at 0x02157DF8), each its own anim of the cells it shows (in Squeak Squad's
         * palette, its slot shared with any other sprite in it: SqueakPaletteKey); */
        internal sealed record SqueakEffect(int Index, string File, int Anim);
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
        };

        /* SqueakEffectsBuild(); Each effect's cells (those its animation shows, in its order) as KatAM frames and their palette, the anim
         * added by `addAnim` (palettes, frames: its anim ID), its palette's slot by `share` (its palette, anim ID, frame); */
        static List<SqueakEffectBuilt> SqueakEffectsBuild(byte[] squeak, List<SqueakEffect> effects,
                                                          Func<List<byte[]>, List<(byte[] tiles, ushort[] oams, short[] attributes, int palette)>, int> addAnim,
                                                          Func<byte[], int, int, SqueakPaletteKey> share) {
            List<SqueakEffectBuilt> built = new List<SqueakEffectBuilt>();
            foreach (SqueakEffect effect in effects) {
                string[] path = effect.File.Split('/');
                SqueakCharacter character = ReadSqueakCharacter(squeak, path[1], path[0]);
                SqueakAnim anim = character.Anims[effect.Anim];
                List<int> cells = anim.Steps.Select(s => s.Cell).Distinct().ToList();
                (List<(byte[] tiles, ushort[] oams, short[] attributes, int palette)> frames, List<byte[]> palettes) = SqueakFrames(character, cells: cells);
                int animId = addAnim(palettes, frames), first = cells.IndexOf(anim.Steps[0].Cell);
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

        static readonly (int type, int letter)[] DaroachGlyphs = { (0x00, 6), (0x14, 5), (0x01, 1), (0x01, 2), (0x14, 5), (0x17, 2), (0x08, 1) };

        // "S" (Squishy), "p", "i" (Chip), "n", "n" (Bronto Burt), "i" (Chip);
        static readonly (int type, int letter)[] SpinniGlyphs = { (0x04, 0), (0x08, 3), (0x08, 2), (0x01, 3), (0x01, 3), (0x08, 2) };

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
            new SqueakEnemy(40, "Mecha-Kracko", "kracko", new[] { 1, 1, 1 }, MechaKrackoGlyphs, MidbossType: 0x40, Canvas: 288),
            /* Its eye (0x0213025C: kracko's animations 8-15, looking at Kirby; only those, not drawn with its graphics: its sprite takes
             * an eye's VRAM, as type 0x2C's characters take the biggest frame's of theirs), no name; */
            new SqueakEnemy(41, "Mecha-Kracko's eye", "kracko", new[] { 0, 0, 0 }, new (int, int)[0],
                            OnlyAnims: new[] { 8, 9, 10, 11, 12, 13, 14, 15 }),
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
            foreach (SqueakBuilt midboss in built.Where(b => b.Enemy.MidbossType != 0))
                if (!minibossTiles.TryGetValue(midboss.Enemy.MidbossType, out int typeTiles) || midboss.Tiles > typeTiles)
                    return $"{midboss.Enemy.Name}'s frames don't fit mini-boss type 0x{midboss.Enemy.MidbossType:X2}'s VRAM.";
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
             * its layer frame + 1, | 0x80 in front; none: 0; its palette's key and frame); */
            int layerTable = Allocate(ref localCursor, SqueakDefinitions.Count * 24, 4);
            for (int i = 0; i < SqueakDefinitions.Count * 24; i++) rom[layerTable + i] = 0;
            foreach (SqueakBuilt b in built.Where(b => b.Layers != null)) {
                for (int k = 0; k < b.Layers.Count && k < 2; k++) {
                    int cellsAt = Allocate(ref localCursor, b.Layers[k].Cells.Length, 4), at = layerTable + b.Enemy.Index * 24 + k * 12;
                    b.Layers[k].Cells.CopyTo(rom, cellsAt);
                    WriteShorts(rom, at, b.Layers[k].AnimId, b.Layers[k].Tiles);
                    WriteInt(rom, at + 4, cellsAt + 0x08000000);
                    WriteShorts(rom, at + 8, b.Layers[k].Palette.Key, b.Layers[k].Palette.Variant);
                }
            }
            WriteInt(rom, data + 8 + SqueakDefinitions.Count * SqueakGraphicsSize, layerTable + 0x08000000);
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
                                                                                                                                List<int> cells = null,
                                                                                                                                List<List<ushort[]>> alternates = null,
                                                                                                                                List<byte[]> alternatePalettes = null,
                                                                                                                                (ushort from, ushort to)[] merge = null) {
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
                    (byte[] tiles, ushort[] oams, short[] attributes) = CutFrame(SqueakCanvas(character, cell, colorMap, size), int.MaxValue, size);
                    frames.Add((tiles, oams, attributes, 0));
                }
            } else {
                if (alternates != null) throw new InvalidOperationException("A character with other colors needs one palette.");
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
