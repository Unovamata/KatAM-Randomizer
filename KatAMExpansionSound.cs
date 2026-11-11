using System;
using System.Collections.Generic;
using System.Linq;

namespace KatAMRandomizer {
    /* KatAMExpansionSound; The expansion's songs and sound effects in KatAM's own sound engine (MP2K, "m4a"): KatAM plays a song by its
     * number in its song table (gSongTable: 8 bytes a song, its header, its music player, 0 the music's, 1-3 the effects', and a u16),
     * which its code reads from literal pools. The table is moved and grown (GrowSongTable()): KatAM's songs keep their numbers, the
     * expansion's come after them.
     * • NiDL's (the owner: "Implement the sounds and music and enemy sound effects for both NiDL and Squeak Squad"): NiDL plays its
     *   songs with the same engine, so its whole song table (579 songs at 0x0860B460 in the USA ROM, found as KatAM's is: FindSongTable();
     *   its 4 players as KatAM's) is
     *   copied (Mp2kCopier): each song's tracks (their jumps and calls moved with them), its voicegroup (the instruments its tracks
     *   pick, key splits and drum kits with theirs), its samples and wave shapes. NiDL's song n is KatAM's song NidlSongBase + n:
     *   the NiDL rooms play their own room's music (its record's +4: 0x100 | the song, 0x1FF none: the boss fights'), the switch rooms
     *   their level hub's (NiDL's hub lists' first room), and the ported code plays NiDL's sound effects by their NiDL numbers
     *   ("ROM Expansion/nidl.h"'s NIDL_SE: gExpansionSongs, written here); */
    internal static partial class KatAMExpansion {
        // (the song table as the ROM has it now: KatAM's, then grown; -1 not looked up yet this build);
        static int songTableAt = -1, songTableCount;
        // NiDL's songs' first number in the grown table (-1: not imported), and how many;
        static int nidlSongBase = -1;
        // (NiDL's song count: its table's, 579 in the USA ROM, at 0x0860B460; NiDL's rooms' songs are under 0x100);
        static int nidlSongCount;

        /* FindSongTable(); KatAM's song table: the longest run of entries whose header is a song's (its track count 0-16, its voicegroup
         * and tracks in the ROM), from a word KatAM's m4a code names (the first run of 500 entries or more); */
        static (int table, int count) FindSongTable(byte[] rom, int limit) {
            bool InRom(int value) => value >= 0x08000000 && value < 0x08000000 + limit;
            bool Header(int at) {
                if (at < 0 || at + 8 > limit) return false;
                int tracks = rom[at];
                if (tracks > 16) return false;
                if (tracks > 0 && !InRom(ReadInt(rom, at + 4))) return false;
                for (int t = 0; t < tracks; t++) if (!InRom(ReadInt(rom, at + 8 + t * 4))) return false;
                return true;
            }
            bool Entry(int at) => InRom(ReadInt(rom, at)) && Header(ReadInt(rom, at) - 0x08000000);
            for (int at = 0; at + 8 <= limit; at += 4) {
                if (!Entry(at)) continue;
                int end = at;
                while (end + 8 <= limit && Entry(end)) end += 8;
                if ((end - at) / 8 >= 500) return (at, (end - at) / 8);
                at = end;
            }
            throw new InvalidOperationException("The song table wasn't found.");
        }

        /* GrowSongTable(); The song table moved to the expansion's space with `extra` after its songs, KatAM's code pointed at it (every
         * aligned word naming it in KatAM's own part of the ROM: m4a's literal pools) and the expansion's code too (`codeLength` bytes
         * at CodeAddress, written before: its PlaySfx() reads gSongTable, linked at KatAM's address, for the song's player and
         * priority; left there it read past KatAM's 620 songs for every NiDL and Squeak Squad sound); returns the first new song's
         * number; */
        static int GrowSongTable(byte[] rom, List<(int header, ushort player, ushort unused)> extra, ref int cursor, int codeLength) {
            if (songTableAt < 0) (songTableAt, songTableCount) = FindSongTable(rom, CodeAddress);
            int table = Allocate(ref cursor, (songTableCount + extra.Count) * 8, 4);
            Array.Copy(rom, songTableAt, rom, table, songTableCount * 8);
            for (int i = 0; i < extra.Count; i++) {
                WriteInt(rom, table + (songTableCount + i) * 8, extra[i].header + 0x08000000);
                WriteShort(rom, table + (songTableCount + i) * 8 + 4, extra[i].player);
                WriteShort(rom, table + (songTableCount + i) * 8 + 6, extra[i].unused);
            }
            int old = songTableAt + 0x08000000, repointed = 0;
            for (int at = 0; at + 4 <= CodeAddress; at += 4)
                if (ReadInt(rom, at) == old) { WriteInt(rom, at, table + 0x08000000); repointed++; }
            for (int at = CodeAddress; at + 4 <= CodeAddress + codeLength; at += 4)
                if (ReadInt(rom, at) == old) WriteInt(rom, at, table + 0x08000000);
            if (repointed == 0) throw new InvalidOperationException("Nothing in KatAM's code names its song table.");
            int first = songTableCount;
            songTableAt = table;
            songTableCount += extra.Count;
            return first;
        }

        /* Mp2kCopier; Songs of another GBA game's MP2K engine copied into the ROM, each piece once: a song's tracks (a block from its first
         * track to its header, MP2K's layout: its GOTO, PATT and REPT targets inside it, moved with it), its header, its voicegroup (as
         * many instruments as its tracks pick, VOICE), a key split's table and voicegroup (as many as its table names), a drum kit's 128,
         * DirectSound samples (16-byte head, size + 1 bytes), wave shapes (16 bytes); an instrument naming nothing in the ROM becomes a
         * silent square; */
        sealed class Mp2kCopier {
            readonly byte[] src, rom;
            readonly Func<int, int, int> allocate;
            readonly Dictionary<int, int> samples = new Dictionary<int, int>(), shapes = new Dictionary<int, int>(), tables = new Dictionary<int, int>();
            readonly Dictionary<(int at, int count), int> groups = new Dictionary<(int, int), int>();
            public Mp2kCopier(byte[] src, byte[] rom, Func<int, int, int> allocate) => (this.src, this.rom, this.allocate) = (src, rom, allocate);

            int Pointer(int at) {
                if (at < 0 || at + 4 > src.Length) return -1;
                int value = ReadInt(src, at) - 0x08000000;
                return value >= 0 && value < src.Length ? value : -1;
            }

            // Decode(); A track block's commands, each pointer's spot (a GOTO, PATT or REPT target, an XCMD wave) and the programs VOICE picks;
            static void Decode(byte[] data, int from, int to, List<int> pointers, HashSet<int> programs) {
                int at = from;
                while (at < to) {
                    byte b = data[at];
                    if (b is 0xB2 or 0xB3) { pointers.Add(at + 1); at += 5; continue; }
                    if (b == 0xB5) { pointers.Add(at + 2); at += 6; continue; }
                    if (b == 0xB9) { at += 4; continue; }
                    if (b == 0xCD && at + 1 < to && data[at + 1] == 0x01) { pointers.Add(at + 2); at += 6; continue; }
                    if (b == 0xBD) {
                        at++;
                        while (at < to && data[at] < 0x80) programs?.Add(data[at++]);
                        continue;
                    }
                    at++;
                }
            }

            public int CopySong(int header) {
                int tracks = src[header];
                if (tracks == 0) {
                    int empty = allocate(8, 4);
                    Array.Copy(src, header, rom, empty, 4);
                    WriteInt(rom, empty + 4, 0);
                    return empty;
                }
                int[] starts = Enumerable.Range(0, tracks).Select(t => Pointer(header + 8 + t * 4)).ToArray();
                int from = starts.Min();
                if (from < 0 || from > header || header - from > 0x40000) throw new InvalidOperationException($"A song at 0x{header:X} isn't laid out as MP2K's.");
                List<int> pointers = new List<int>();
                HashSet<int> programs = new HashSet<int>();
                Decode(src, from, header, pointers, programs);
                int block = allocate(header - from + 8 + tracks * 4, 4);
                Array.Copy(src, from, rom, block, header - from);
                foreach (int at in pointers) {
                    int target = Pointer(at);
                    // (a target outside the block: an XCMD's wave, copied as a wave shape; anything else stays where it pointed, 0);
                    int moved = target >= from && target < header ? block + target - from : target >= 0 ? CopyShape(target) : -1;
                    WriteInt(rom, block + at - from, moved < 0 ? 0 : moved + 0x08000000);
                }
                int newHeader = block + header - from;
                Array.Copy(src, header, rom, newHeader, 4);
                int group = Pointer(header + 4);
                int count = programs.Count == 0 ? 1 : programs.Max() + 1;
                WriteInt(rom, newHeader + 4, group < 0 ? 0 : CopyGroup(group, count) + 0x08000000);
                for (int t = 0; t < tracks; t++) WriteInt(rom, newHeader + 8 + t * 4, block + starts[t] - from + 0x08000000);
                return newHeader;
            }

            // CopyGroup(); A voicegroup's first `count` instruments (a group asked for again with more gets copied again: rare);
            int CopyGroup(int at, int count) {
                if (groups.TryGetValue((at, count), out int copied)) return copied;
                int group = allocate(count * 12, 4);
                groups[(at, count)] = group;
                for (int i = 0; i < count; i++) {
                    int entry = at + i * 12, to = group + i * 12;
                    if (entry + 12 > src.Length) { Silent(to); continue; }
                    Array.Copy(src, entry, rom, to, 12);
                    byte type = src[entry];
                    if ((type & 0x40) != 0) {
                        int sub = Pointer(entry + 4), table = Pointer(entry + 8);
                        if (sub < 0 || table < 0 || table + 128 > src.Length) { Silent(to); continue; }
                        int subCount = Enumerable.Range(0, 128).Max(k => src[table + k]) + 1;
                        WriteInt(rom, to + 4, CopyGroup(sub, Math.Min(subCount, 128)) + 0x08000000);
                        WriteInt(rom, to + 8, CopyTable(table) + 0x08000000);
                    } else if ((type & 0x80) != 0) {
                        int sub = Pointer(entry + 4);
                        if (sub < 0) { Silent(to); continue; }
                        WriteInt(rom, to + 4, CopyGroup(sub, 128) + 0x08000000);
                    } else if ((type & 7) == 0) {
                        int sample = Pointer(entry + 4);
                        int moved = sample < 0 ? -1 : CopySample(sample);
                        if (moved < 0) Silent(to);
                        else WriteInt(rom, to + 4, moved + 0x08000000);
                    } else if ((type & 7) == 3) {
                        int shape = Pointer(entry + 4);
                        if (shape < 0) Silent(to);
                        else WriteInt(rom, to + 4, CopyShape(shape) + 0x08000000);
                    }
                }
                return group;
            }

            // Silent(); A square wave voice at no volume, for an instrument that names nothing;
            void Silent(int to) {
                Array.Clear(rom, to, 12);
                rom[to] = 0x01;
                rom[to + 1] = 60;
            }

            int CopySample(int at) {
                if (samples.TryGetValue(at, out int copied)) return copied;
                if (at + 16 > src.Length) return -1;
                int size = ReadInt(src, at + 12);
                if (size < 0 || size > 0x200000 || at + 16 + size + 1 > src.Length) return -1;
                int sample = allocate(16 + size + 1, 4);
                Array.Copy(src, at, rom, sample, 16 + size + 1);
                return samples[at] = sample;
            }

            int CopyShape(int at) {
                if (shapes.TryGetValue(at, out int copied)) return copied;
                int shape = allocate(16, 4);
                Array.Copy(src, at, rom, shape, Math.Min(16, src.Length - at));
                return shapes[at] = shape;
            }

            int CopyTable(int at) {
                if (tables.TryGetValue(at, out int copied)) return copied;
                int table = allocate(128, 4);
                Array.Copy(src, at, rom, table, 128);
                return tables[at] = table;
            }
        }

        /* AddNidlSounds(); NiDL's songs after KatAM's (see above); the base written for the ported code (gExpansionSongs[0]); */
        static string AddNidlSounds(byte[] rom, byte[] nightmare, KatAMExpansionCode.Version version, ref int cursor) {
            int here = cursor;
            Mp2kCopier copier = new Mp2kCopier(nightmare, rom, (size, align) => Allocate(ref here, size, align));
            List<(int header, ushort player, ushort unused)> songs = new List<(int, ushort, ushort)>();
            try {
                (int table, int count) = FindSongTable(nightmare, nightmare.Length);
                nidlSongCount = count;
                for (int i = 0; i < count; i++) {
                    int entry = table + i * 8;
                    songs.Add((copier.CopySong(ReadInt(nightmare, entry) - 0x08000000), (ushort)ReadShort(nightmare, entry + 4), (ushort)ReadShort(nightmare, entry + 6)));
                }
                nidlSongBase = GrowSongTable(rom, songs, ref here, version.Code.Length);
            } catch (InvalidOperationException e) {
                return "NiDL's sounds: " + e.Message;
            }
            WriteShort(rom, CodeAddress + version.CodeSymbols["gExpansionSongs"], nidlSongBase);
            cursor = here;
            return null;
        }

        /* NidlRoomSong(); A NiDL room's music: NiDL's own (its record's +4, 0x100 | the song; a switch room its level's hub's, NiDL's
         * hub list's first room), else KatAM's song for its level (LevelSongs: no NiDL sounds, or a room that keeps the music); */
        static int NidlRoomSong(byte[] nightmare, NightmareRegion region, NightmareRoom r) {
            if (nidlSongBase < 0) return LevelSongs[r.Level];
            int record = r.Record;
            if (r.IsSwitchRoom) {
                int list = ReadInt(nightmare, region.RoomTable + 0x100 + r.Level * 4);
                if (list >> 24 != 0x08) return LevelSongs[r.Level];
                record = ReadInt(nightmare, list - 0x08000000) & 0x1FFFFFF;
            }
            int field = ReadShort(nightmare, record + 4);
            return (field & 0x100) != 0 && (field & 0xFF) != 0xFF && (field & 0xFF) < nidlSongCount ? nidlSongBase + (field & 0xFF) : LevelSongs[r.Level];
        }
    }
}
