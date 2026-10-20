using KatAMInternal;

namespace KatAMRandomizer {
    /* KatAMTitleScreen; The characters parading across the intro (after the HAL Laboratory / Nintendo / Flagship logos, before the
     * title) are randomized.
     * • gUnk_08387814 (USA 0x08387814, 56 entries of 0x18 bytes, the same bytes in every version): { u32 frame; u8 type, variant,
     *   group, y; u8 order, unk9; u16 speedX; s16 speedY; ... }, one character each, spawning at its frame off the screen's right
     *   edge and moving left (sub_0814669C). Its type indexes gUnk_08387348 (0x10 bytes: palette slot, y offset, width, update
     *   functions) and gUnk_08D62530 (animation, variant, tiles: 0 tiles spawns nothing).
     * • Each type's look and movement come with it: walkers (sub_08146CE4) and walkers that loop a sound effect (sub_08146D80, set by
     *   the type's own init) take only their position and speed from the entry, so they can stand in for each other. Types with
     *   scripted moves (flocks, hoppers, the ones that stop and wait for the logo), the four Kirbys, and the two pairs sharing a
     *   palette slot (0x05/0x17, 0x24/0x25) are kept.
     * • A type is swapped for one of the same kind (small and big ground walkers, fliers), counting 7 drawable types the intro never
     *   uses (0x07, 0x0B, 0x1A, 0x1D, 0x1F, 0x23, 0x34). Each type is swapped for a different one wherever it appears (a one-to-one
     *   remap). The parade fills all 16 sprite palettes at its busiest (one a type, sub_0814682C has no 17th: the counter runs away
     *   and colours go wrong), and a wider type stays on screen a few frames longer, so each remap is checked: every entry is on
     *   screen from its frame until it leaves the left edge, crossing the screen and its own width at its speed (speedX / 256
     *   pixels a frame, rounded up: the original's 16 has no frame to spare), and a remap with more than 16 palette slots, or more
     *   than MaxSegments of object VRAM (366 of 512 segments in the original), in use at once is drawn again. The estimate gives
     *   the original exactly its 16 and 366;
     * • The birds, fish and stars flying behind are tiles of two background layers (BG2, BG3), not characters, and stay;
     * • With the ROM Expansion, the ported NiDL enemies join (AddNidlTypes): both tables move to its free space with a type more for
     *   each (types 0x46 on), a walker drawing the enemy's looping walk or flight (an extra variant of its anim), its own palette
     *   slot; the moved tables start after a header ("PK", how many, each one's kind), and their kinds join the remap's; */
    internal static class KatAMTitleScreen {
        // (Code: where the intro's code starts, its literal pools within CodeSize; TypesSite, SpritesSite: a pointer to each table there);
        sealed record Version(string GameCode, int Parade, int Types, int Sprites, int Code, int TypesSite, int SpritesSite);
        const int CodeSize = 0x4000;

        static readonly Version[] Versions = {
            new Version("B8KE", 0x387814, 0x387348, 0xD62530, 0x145B64, 0x14680C, 0x146720),
            new Version("B8KP", 0x3917C0, 0x3912F4, 0xD6CB40, 0x145C4C, 0x1468F4, 0x146808),
            new Version("B8KJ", 0x36BF8C, 0x36BAC0, 0xD2D960, 0x145B00, 0x1467A8, 0x1466BC),
        };

        const int Entries = 56, EntrySize = 0x18, TypeSize = 0x10, SpriteSize = 6, MaxSegments = 480, MaxPalettes = 16, ScreenWidth = 240;
        // TypeCount: the types (both tables' entries); HeaderSize: the moved tables' header; WalkerTemplate: a walker whose update the new types take;
        const int TypeCount = 0x46, HeaderSize = 0x10, WalkerTemplate = 0x0C, MaxNidlTypes = HeaderSize - 3;

        /* NidlType; A ported enemy in the parade: its kind (Kinds' index), anim and variant (its loop), tiles (its biggest frame), width
         * and lift (from the line it walks on to its sprite's origin); */
        public sealed record NidlType(int Kind, int AnimId, int Variant, int Tiles, int Width, int Lift);

        // NidlTypesSize(); The space AddNidlTypes needs for these many types;
        public static int NidlTypesSize(int count) => HeaderSize + (TypeCount + count) * (TypeSize + SpriteSize);

        /* AddNidlTypes(); Moves the types and sprites tables to `at` (NidlTypesSize free bytes), adding these types; every pointer to the
         * tables in the intro's code (its literal pools, near the parade's own) is changed to the moved ones. Does nothing on an unknown ROM; */
        public static void AddNidlTypes(byte[] rom, int at, List<NidlType> types) {
            Version version = VersionOf(rom);

            if (version == null || types.Count == 0) return;
            types = types.Take(MaxNidlTypes).ToList();

            int count = TypeCount + types.Count, typesAt = at + HeaderSize, spritesAt = typesAt + count * TypeSize;
            int walker = BitConverter.ToInt32(rom, version.Types + WalkerTemplate * TypeSize + 8);

            rom[at] = (byte)'P';
            rom[at + 1] = (byte)'K';
            rom[at + 2] = (byte)types.Count;
            for (int i = 0; i < types.Count; i++) rom[at + 3 + i] = (byte)types[i].Kind;
            Array.Copy(rom, version.Types, rom, typesAt, TypeCount * TypeSize);
            Array.Copy(rom, version.Sprites, rom, spritesAt, TypeCount * SpriteSize);
            for (int i = 0; i < types.Count; i++) {
                int type = typesAt + (TypeCount + i) * TypeSize, sprite = spritesAt + (TypeCount + i) * SpriteSize;

                Array.Clear(rom, type, TypeSize);
                rom[type] = (byte)(TypeCount + i);  // Its own palette slot;
                rom[type + 1] = rom[type + 3] = (byte)types[i].Lift;
                rom[type + 2] = (byte)types[i].Width;
                BitConverter.GetBytes(walker).CopyTo(rom, type + 8);
                BitConverter.GetBytes((ushort)types[i].AnimId).CopyTo(rom, sprite);
                BitConverter.GetBytes((ushort)types[i].Variant).CopyTo(rom, sprite + 2);
                BitConverter.GetBytes((ushort)types[i].Tiles).CopyTo(rom, sprite + 4);
            }
            for (int i = version.Code; i < version.Code + CodeSize; i += 4) {
                int value = BitConverter.ToInt32(rom, i);
                if (value == version.Types + 0x08000000) BitConverter.GetBytes(typesAt + 0x08000000).CopyTo(rom, i);
                else if (value == version.Sprites + 0x08000000) BitConverter.GetBytes(spritesAt + 0x08000000).CopyTo(rom, i);
            }
        }

        // Moved(); Where the tables are (moved by AddNidlTypes, or not) and the added types' kinds;
        static (int types, int sprites, byte[] kinds) Moved(byte[] rom, Version version) {
            int types = BitConverter.ToInt32(rom, version.TypesSite) - 0x08000000, sprites = BitConverter.ToInt32(rom, version.SpritesSite) - 0x08000000;
            int header = types - HeaderSize;

            if (types == version.Types || header < 0 || rom[header] != 'P' || rom[header + 1] != 'K') return (version.Types, version.Sprites, new byte[0]);
            return (types, sprites, rom.Skip(header + 3).Take(rom[header + 2]).ToArray());
        }

        static Version VersionOf(byte[] rom) => Versions.FirstOrDefault(v => v.GameCode == System.Text.Encoding.ASCII.GetString(rom, 0xAC, 4));

        static readonly byte[][] Kinds = {
            new byte[] { 0x0C, 0x10, 0x19, 0x1B, 0x1D, 0x22, 0x23, 0x2A, 0x2C, 0x2D, 0x37 },              // Small ground walkers;
            new byte[] { 0x16, 0x1F, 0x2B, 0x2E, 0x38, 0x3D, 0x3E, 0x3F, 0x41, 0x43 },                    // Big ground walkers;
            new byte[] { 0x07, 0x0A, 0x0B, 0x0F, 0x12, 0x14, 0x1A, 0x20, 0x21, 0x30, 0x32, 0x34 },        // Fliers;
        };

        // RandomizeParade(); Swaps the intro parade's characters. Does nothing on an unknown ROM or one whose parade was changed;
        public static void RandomizeParade(byte[] rom, Random random) {
            Version version = VersionOf(rom);

            if (version == null) return;

            (int typesAt, int spritesAt, byte[] nidlKinds) = Moved(rom, version);
            byte[][] kinds = Kinds.Select((kind, k) => kind.Concat(Enumerable.Range(0, nidlKinds.Length).Where(i => nidlKinds[i] == k)
                                                                                 .Select(i => (byte)(TypeCount + i))).ToArray()).ToArray();
            byte[] original = Enumerable.Range(0, Entries).Select(i => rom[version.Parade + i * EntrySize + 4]).ToArray();

            if (!Fits(rom, version, typesAt, spritesAt, original)) return;

            for (int attempt = 0; attempt < 100; attempt++) {
                Dictionary<byte, byte> remap = new Dictionary<byte, byte>();

                foreach (byte[] kind in kinds) {
                    byte[] used = kind.Where(original.Contains).ToArray(),
                           targets = kind.OrderBy(_ => random.Next()).ToArray();

                    for (int i = 0; i < used.Length; i++) remap[used[i]] = targets[i];
                }

                byte[] types = original.Select(type => remap.TryGetValue(type, out byte to) ? to : type).ToArray();

                if (!Fits(rom, version, typesAt, spritesAt, types)) continue;

                for (int i = 0; i < Entries; i++) rom[version.Parade + i * EntrySize + 4] = types[i];
                return;
            }
        }

        // Fits(); Whether the parade with these types never needs more than MaxPalettes palette slots and MaxSegments VRAM segments at once;
        static bool Fits(byte[] rom, Version version, int typesAt, int spritesAt, byte[] types) {
            List<(int start, int end, int segments, byte palette)> spans = new List<(int, int, int, byte)>();

            for (int i = 0; i < Entries; i++) {
                int entry = version.Parade + i * EntrySize, type = typesAt + types[i] * TypeSize,
                    tiles = ReadU16(rom, spritesAt + types[i] * SpriteSize + 4), speed = Math.Max(1, (int)ReadU16(rom, entry + 0xA)),
                    start = ReadU16(rom, entry);

                if (tiles > 0) spans.Add((start, start + ((ScreenWidth + rom[type + 2]) * 256 + speed - 1) / speed, (tiles + 1) / 2, rom[type]));
            }

            return spans.All(at => {
                var shown = spans.Where(s => s.start <= at.start && at.start < s.end).ToList();

                return shown.Sum(s => s.segments) <= MaxSegments && shown.Select(s => s.palette).Distinct().Count() <= MaxPalettes;
            });
        }

        static ushort ReadU16(byte[] rom, int address) => (ushort)(rom[address] | rom[address + 1] << 8);
    }
}
