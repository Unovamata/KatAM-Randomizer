using System;
using System.Collections.Generic;
using System.Linq;

namespace KatAMRandomizer {
    /* KatAMExpansionSqueakSound; Squeak Squad's music and sound effects in KatAM's sound engine (the owner: "Implement the sounds and
     * music and enemy sound effects for both NiDL and Squeak Squad"). The DS's are all in sound/sound_data.sdat (Nintendo's SDAT): its
     * 42 sequences (the music, SSEQ), 22 sequence archives (the sound effects, SSAR), 83 instrument banks (SBNK) and 2 wave archives
     * (SWAR, every sample IMA-ADPCM). They're compiled into KatAM's MP2K songs:
     * • a sample becomes an 8-bit DirectSound sample (halved while over 24 kHz: KatAM mixes at 13 kHz), its instrument's root key
     *   baked into its rate (MP2K plays a sample's rate at key 60), one copy per sample and root key;
     * • a bank becomes a voicegroup: a sampled instrument a DirectSound voice (its pan, the DS's envelope by its times: attack's rise,
     *   decay's and release's fall in decibels a second, sustain's level), a PSG one a square (its duty the nearest), noise noise; a
     *   key split a key split (its table, its voices), a drum set a drum kit (each key its voice, at that key);
     * • a sequence's commands become MP2K's one by one: the DS's 48 ticks a beat kept (TEMPO set to the DS's tempo itself: MP2K's
     *   is half a tempo at 24 a beat), its notes with their length (a note waits its length, the DS's note wait, unless its track's mono/poly is 0: the music's
     *   tracks all set it, so their notes don't wait, as MP2K's; the sound effects never do, so theirs wait)
     *   (a length past MP2K's 223, a tie and its end after it), rests as waits, calls and returns as PATT and PEND (never nested
     *   in Squeak Squad), jumps as GOTO, the end FINE, volume on the DS's curve (amplitude by the square: its decibel table; the
     *   sequence's volume and the notes' velocity as they are, as loud as KatAM's own songs), pan, pitch bend and its range, priority, transpose, modulation, the envelope's changes (XCMD's); each track starts
     *   with the DS's defaults.
     * The songs go after the others in the grown song table: the music ids (the DS's own list of 43, each a sequence:
     * SqueakMusicSequences) first, then every sound effect id (SqueakSoundIds: the DS's table, an id its archive and index), so
     * Squeak Squad's sound n is "ROM Expansion/nidl.h"'s SQUEAK_SE(n) (gExpansionSongs[1]: the first); a room plays its map's
     * music (its header's byte 0x0B; 0 keeps the one before, as on the DS); */
    internal static partial class KatAMExpansion {
        const int SqueakMusicCount = 43;
        // (the first Squeak Squad song in the grown table; -1 not added);
        static int squeakSongBase = -1;

        // The DS's music ids' sequences (USA arm9 0x02159770: 4 bytes an id, the sequence the second u16);
        static readonly int[] SqueakMusicSequences = {
            4, 22, 0, 3, 5, 6, 7, 9, 10, 11, 19, 20, 23, 21, 16, 17, 18, 14, 14, 15, 12, 13, 2, 1, 8,
            24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41,
        };

        // The DS's sound ids (USA arm9 0x02159DDC, 12 bytes an id: its archive the low u16, its index in it the high): 4 hex digits an
        // id, the archive and index, FFFF none;
        const string SqueakSoundIds =
            "0000000100020003000400050006000700080009000A15000100010101020103010C010401050106010701080109010A010B010D020002010202020302040205" +
            "020602070300030103020303030403050305030703080309030A030B030C030D030E030F0310031103120313031403150316031703180319031A031B031C031D" +
            "031E031F0320032103220323032403250326032703280329032A032B032C032D032F0330033103320333033403350336033703380339033A033B033C033D033E" +
            "033F0340034103420343034403460347032E0345034803490400040104020403040404050406040704080409040A040B040C040D040E040F0410041104120413" +
            "041404150416041704180419041A041B041C041D041E041F0500050105020503050405050506050705080509050A050B050C050D050E050F0510051105120513" +
            "051405150516051705180519051A051B051C051D051E051F0520052105220523052505260524052705280529052A052B052C052D052E052F0530053105320533" +
            "053405350536053705380539053A053B053C053D053E053F05400541054205430544054505460547054805490549054A054B054C054D054E054F055005510552" +
            "0553055405550556055705580559055A055B055C055D055E055F0560056105620563056405650567056605680700070107020703070407050706070707080709" +
            "070A070B070C070D070E070F071007110713071207020600060106020603060406050606060706080609060A060B060C060D060E060F06100611061206130614" +
            "0615061606170618061A061C061D061E061F062006210622062306240626062706280629062A062B062C062D062E062F06300631063206330634063506360637" +
            "06380639063A063B063C063D06400641064206430644064506460619061B0625063E063F080008010803080408050806080708080809080A080B080C080D080E" +
            "080F0810081108120813081408150816081708180819081A081B081C081D081F08200821082208230824082508260827081E0802082805440545054609000901" +
            "0902090309040905090609070A000A010A020A030A040A050A060A070A080A090A0A0B000B010B020B030B040B050B060B070B080B090C000C010C030C050C06" +
            "0C080C020C040C070D000D010D020D030D040D050E000E010E020E050E070E080E090E0A0E0B0E0C0E030E040E060F000F040F050F060F070F0C0F0D0F0E0F0F" +
            "0F010F020F030F080F090F0A0F0B1000100110021003100410051006100710081009100A100B100C100D100E100F101010111012101311001101110211031104" +
            "110511061107110800010705070D0207020406301200120112021203120405221205120612070346130007081301130213031304130513060539020214001401" +
            "1402140314041405110811061304140714081409140AFFFFFFFF0000FFFF0000FFFFFFFFFFFF0000FFFF0600090009000D0012001500FFFF0000FFFF0000FFFF" +
            "0000FFFF0000FFFF0000FFFF030007000000FFFFFFFFFFFFFFFF0000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF" +
            "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF" +
            "FFFFFFFF01000000FFFF0000FFFF0000FFFF0000FFFF0000FFFF0000FFFF0000FFFF0000FFFF0000FFFF0000FFFF0000FFFF0000FFFF0000FFFF0000FFFF0000" +
            "FFFF0100FFFF0100FFFF0100FFFF0100FFFF0100FFFF0100FFFF0100FFFF0100FFFF00000000000001000000FFFF0000FFFF0000000001000200020000000200" +
            "FFFF0100FFFF0000000000000100010000000100FFFF0000000000000000FFFF0000FFFF0000FFFF0000FFFF0000FFFF0000FFFF0100FFFF0100FFFF0100FFFF" +
            "0100FFFF0200FFFF0200FFFF0200FFFF0200FFFF0200FFFF0200FFFF0200FFFF000000000000FFFFFFFFFFFFFFFF00000000010700000000FFFF010002000500" +
            "0200080002000D0001000B00020002000200FFFF020006000200020002000E00FFFF000000000000FFFF000000000000";

        static int SqueakSoundCount => SqueakSoundIds.Length / 4;
        static (int archive, int index) SqueakSoundId(int id) {
            int value = Convert.ToInt32(SqueakSoundIds.Substring(id * 4, 4), 16);
            return value == 0xFFFF ? (-1, -1) : (value >> 8, value & 0xFF);
        }

        /* Sdat; Nintendo's SDAT: its FAT (each file's offset and size), its INFO's records (sequences: file, bank, volume, priorities,
         * player; sequence archives, banks with their wave archives, wave archives: their files); */
        sealed class Sdat {
            public readonly byte[] Data;
            readonly int info, fat;
            public Sdat(byte[] data) {
                Data = data;
                if (data.Length < 0x40 || data[0] != 'S' || data[1] != 'D' || data[2] != 'A' || data[3] != 'T') throw new InvalidOperationException("sound/sound_data.sdat isn't an SDAT.");
                info = ReadInt(data, 0x18);
                fat = ReadInt(data, 0x20);
            }
            int Records(int kind) => ReadInt(Data, info + ReadInt(Data, info + 8 + kind * 4));
            int Record(int kind, int i) {
                int list = info + ReadInt(Data, info + 8 + kind * 4);
                if (i < 0 || i >= ReadInt(Data, list)) return -1;
                int offset = ReadInt(Data, list + 4 + i * 4);
                return offset == 0 ? -1 : info + offset;
            }
            public byte[] File(int id) {
                int offset = ReadInt(Data, fat + 12 + id * 16), size = ReadInt(Data, fat + 16 + id * 16);
                byte[] file = new byte[size];
                Array.Copy(Data, offset, file, 0, size);
                return file;
            }
            public int SequenceCount => Records(0);
            public (int file, int bank, int volume, int priority, int player) Sequence(int i) {
                int at = Record(0, i);
                if (at < 0) return (-1, -1, 0, 0, 0);
                return (ReadShort(Data, at), ReadShort(Data, at + 4), Data[at + 6], Data[at + 8], Data[at + 9]);
            }
            public int ArchiveFile(int i) { int at = Record(1, i); return at < 0 ? -1 : ReadShort(Data, at); }
            public (int file, int[] waves) Bank(int i) {
                int at = Record(2, i);
                if (at < 0) return (-1, null);
                return (ReadShort(Data, at), Enumerable.Range(0, 4).Select(k => ReadShort(Data, at + 4 + k * 2)).ToArray());
            }
            public int WaveArchiveFile(int i) { int at = Record(3, i); return at < 0 ? -1 : ReadShort(Data, at); }
        }

        /* SqueakSoundCompiler; Squeak Squad's banks, samples and sequences written as MP2K's (see above), each piece once; */
        sealed class SqueakSoundCompiler {
            readonly Sdat sdat;
            readonly byte[] rom;
            readonly Func<int, int, int> allocate;
            readonly Dictionary<int, byte[]> waveArchives = new Dictionary<int, byte[]>();
            readonly Dictionary<(int archive, int wave, int root), int> samples = new Dictionary<(int, int, int), int>();
            readonly Dictionary<int, int> groups = new Dictionary<int, int>();
            readonly Dictionary<(int file, int volume), (int block, Dictionary<int, int> map)> blocks = new Dictionary<(int, int), (int, Dictionary<int, int>)>();
            readonly int silentSample;

            public SqueakSoundCompiler(Sdat sdat, byte[] rom, Func<int, int, int> allocate) {
                (this.sdat, this.rom, this.allocate) = (sdat, rom, allocate);
                silentSample = allocate(16 + 2, 4);
                WriteInt(rom, silentSample + 4, 13379 * 1024);
                WriteInt(rom, silentSample + 12, 1);
            }

            byte[] WaveArchive(int id) {
                if (!waveArchives.TryGetValue(id, out byte[] swar)) waveArchives[id] = swar = sdat.File(sdat.WaveArchiveFile(id));
                return swar;
            }

            static readonly int[] ImaSteps = {
                7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45, 50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130, 143,
                157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408, 449, 494, 544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552,
                1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487,
                12635, 13899, 15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767,
            };
            static readonly int[] ImaIndex = { -1, -1, -1, -1, 2, 4, 6, 8 };

            /* Sample(); A wave of a wave archive as an MP2K sample, its root key baked into its rate (0x7FFF... a missing one: silent):
             * its encoding (0 PCM8, 1 PCM16, 2 IMA-ADPCM: a 4-byte head, the first sample and step index, then nibbles, low first),
             * loop and rate, its loop start and length in words; */
            public int Sample(int archive, int wave, int root) {
                if (samples.TryGetValue((archive, wave, root), out int done)) return done;
                byte[] swar = WaveArchive(archive);
                if (wave < 0 || wave >= ReadInt(swar, 0x38)) return samples[(archive, wave, root)] = silentSample;
                int at = ReadInt(swar, 0x3C + wave * 4);
                int encoding = swar[at], rate = ReadShort(swar, at + 2), loopWords = ReadShort(swar, at + 6), lengthWords = ReadInt(swar, at + 8);
                bool loops = swar[at + 1] != 0;
                int data = at + 12, bytes = (loopWords + lengthWords) * 4;
                List<int> pcm = new List<int>();
                int loopStart;
                if (encoding == 2) {
                    int sample = (short)ReadShort(swar, data), index = Math.Clamp((int)swar[data + 2], 0, 88);
                    for (int b = 4; b < bytes && data + b < swar.Length; b++) {
                        for (int half = 0; half < 2; half++) {
                            int nibble = half == 0 ? swar[data + b] & 15 : swar[data + b] >> 4, step = ImaSteps[index], diff = step >> 3;
                            if ((nibble & 1) != 0) diff += step >> 2;
                            if ((nibble & 2) != 0) diff += step >> 1;
                            if ((nibble & 4) != 0) diff += step;
                            sample = Math.Clamp((nibble & 8) != 0 ? sample - diff : sample + diff, -32767, 32767);
                            index = Math.Clamp(index + ImaIndex[nibble & 7], 0, 88);
                            pcm.Add(sample);
                        }
                    }
                    loopStart = Math.Max(0, (loopWords * 4 - 4) * 2);
                } else if (encoding == 1) {
                    for (int b = 0; b + 1 < bytes && data + b + 1 < swar.Length; b += 2) pcm.Add((short)ReadShort(swar, data + b));
                    loopStart = loopWords * 2;
                } else {
                    for (int b = 0; b < bytes && data + b < swar.Length; b++) pcm.Add((sbyte)swar[data + b] * 256);
                    loopStart = loopWords * 4;
                }
                if (pcm.Count == 0) return samples[(archive, wave, root)] = silentSample;
                // (KatAM mixes at about 13 kHz: rates over 24 kHz are halved, every two samples averaged);
                while (rate > 24000 && pcm.Count > 64) {
                    List<int> half = new List<int>(pcm.Count / 2 + 1);
                    for (int i = 0; i < pcm.Count; i += 2) half.Add(i + 1 < pcm.Count ? (pcm[i] + pcm[i + 1]) / 2 : pcm[i]);
                    pcm = half;
                    rate /= 2;
                    loopStart /= 2;
                }
                if (!loops || loopStart >= pcm.Count) loopStart = 0;
                int size = pcm.Count;
                int sampleAt = allocate(16 + size + 1, 4);
                WriteShort(rom, sampleAt, 0);
                WriteShort(rom, sampleAt + 2, loops ? 0x4000 : 0);
                WriteInt(rom, sampleAt + 4, (int)Math.Round(rate * 1024.0 * Math.Pow(2, (60 - root) / 12.0)));
                WriteInt(rom, sampleAt + 8, loopStart);
                WriteInt(rom, sampleAt + 12, size);
                for (int i = 0; i < size; i++) rom[sampleAt + 16 + i] = (byte)(sbyte)Math.Clamp(pcm[i] >> 8, -128, 127);
                rom[sampleAt + 16 + size] = loops ? rom[sampleAt + 16 + loopStart] : rom[sampleAt + 16 + size - 1];
                return samples[(archive, wave, root)] = sampleAt;
            }

            // The DS's envelope (its sound driver's tables; updated 192 times a second, in 1/1280 decibel) as MP2K's DirectSound one;
            static int DsAttackRate(int a) {
                int[] high = { 0x00, 0x01, 0x05, 0x0E, 0x1A, 0x26, 0x33, 0x3F, 0x49, 0x54, 0x5C, 0x64, 0x6D, 0x74, 0x7B, 0x7F, 0x84, 0x89, 0x8F };
                return a >= 109 ? high[127 - Math.Min(a, 127)] : 255 - a;
            }
            static int DsFallRate(int d) => d >= 127 ? 0xFFFF : d == 126 ? 0x3C00 : d < 50 ? d * 2 + 1 : 0x1E00 / (126 - d);
            static double DsSustainDb(int s) => s <= 0 ? -72.3 : 40 * Math.Log10(Math.Min(s, 127) / 127.0);
            static byte Attack(int a) {
                int rate = DsAttackRate(a), updates = 0;
                for (long level = -92544; level < -1280 && updates < 2000; updates++) level = level * rate >> 8;
                double frames = updates / 192.0 * 60;
                return frames < 1 ? (byte)255 : (byte)Math.Clamp((int)Math.Round(255 / frames), 1, 254);
            }
            static byte Fall(int d) {
                int rate = DsFallRate(d);
                if (rate >= 0xFFFF) return 0;
                double dbPerFrame = rate / 1280.0 * 192 / 60;
                return (byte)Math.Clamp((int)Math.Round(256 * Math.Pow(10, -dbPerFrame / 20)), 0, 255);
            }
            static byte Sustain(int s) => (byte)Math.Clamp((int)Math.Round(255 * Math.Pow(10, DsSustainDb(s) / 20)), 0, 255);
            public static byte[] Envelope(int a, int d, int s, int r) => new[] { Attack(a), Fall(d), Sustain(s), Fall(r) };

            /* Voice(); One of a bank's note definitions (type: 1 a sample, 2 a PSG square, 3 noise) as a 12-byte MP2K voice at `to`, played
             * at `key` (a drum's; 60 otherwise); */
            void Voice(int to, int type, byte[] bank, int def, int[] waves, int key) {
                Array.Clear(rom, to, 12);
                int wave = ReadShort(bank, def), archive = ReadShort(bank, def + 2), root = bank[def + 4], pan = bank[def + 9];
                byte[] envelope = Envelope(bank[def + 5], bank[def + 6], bank[def + 7], bank[def + 8]);
                rom[to + 1] = (byte)key;
                if (type == 1 && archive < 4 && waves[archive] != 0xFFFF) {
                    rom[to] = 0x00;
                    rom[to + 3] = pan == 64 ? (byte)0 : (byte)(0x80 | Math.Min(pan, 127));
                    WriteInt(rom, to + 4, Sample(waves[archive], wave, root) + 0x08000000);
                    Array.Copy(envelope, 0, rom, to + 8, 4);
                } else if (type == 2 || type == 3) {
                    // (a PSG: the GBA's square 2, its duty the nearest of 12.5, 25, 50, 75 %; noise: its noise channel; their envelope
                    // the CGB's: no attack, the sustain's level of 15);
                    rom[to] = type == 2 ? (byte)0x02 : (byte)0x04;
                    WriteInt(rom, to + 4, type == 2 ? (wave <= 0 ? 0 : wave == 1 ? 1 : wave <= 4 ? 2 : 3) : 0);
                    rom[to + 8] = 0;
                    rom[to + 9] = 0;
                    rom[to + 10] = (byte)Math.Clamp(envelope[2] * 15 / 255, 0, 15);
                    rom[to + 11] = 2;
                } else {
                    rom[to] = 0x00;
                    WriteInt(rom, to + 4, silentSample + 0x08000000);
                    rom[to + 8] = 255;
                    rom[to + 10] = 0;
                }
            }

            /* Group(); A bank as a voicegroup (its programs' count): a single definition one voice, a key split (17: 8 bytes, each
             * region's top key, then a definition each) a key split with its table and voices, a drum set (16: its lowest and highest
             * key, a definition each) a drum kit of 128; */
            public int Group(int bankId) {
                if (groups.TryGetValue(bankId, out int done)) return done;
                (int file, int[] waves) = sdat.Bank(bankId);
                if (file < 0) return groups[bankId] = 0;
                byte[] bank = sdat.File(file);
                int count = Math.Min(ReadInt(bank, 0x38), 128);
                int group = allocate(Math.Max(1, count) * 12, 4);
                for (int p = 0; p < count; p++) {
                    int type = bank[0x3C + p * 4], at = ReadShort(bank, 0x3C + p * 4 + 1), to = group + p * 12;
                    if (type == 0) { Voice(to, 0, bank, 0, waves, 60); continue; }
                    if (type < 16) { Voice(to, type, bank, at, waves, 60); continue; }
                    if (type == 16) {
                        int low = bank[at], high = bank[at + 1];
                        int kit = allocate(128 * 12, 4);
                        for (int k = 0; k < 128; k++) {
                            if (k < low || k > high) Voice(kit + k * 12, 0, bank, at, waves, k);
                            else Voice(kit + k * 12, bank[at + 2 + (k - low) * 12], bank, at + 4 + (k - low) * 12, waves, k);
                        }
                        Array.Clear(rom, to, 12);
                        rom[to] = 0x80;
                        WriteInt(rom, to + 4, kit + 0x08000000);
                        continue;
                    }
                    if (type == 17) {
                        int[] tops = Enumerable.Range(0, 8).Select(r => (int)bank[at + r]).TakeWhile(top => top != 0).ToArray();
                        if (tops.Length == 0) { Voice(to, 0, bank, at, waves, 60); continue; }
                        int sub = allocate(tops.Length * 12, 4), table = allocate(128, 4);
                        for (int r = 0; r < tops.Length; r++) Voice(sub + r * 12, bank[at + 8 + r * 12], bank, at + 10 + r * 12, waves, 60);
                        for (int k = 0; k < 128; k++) {
                            int r = 0;
                            while (r < tops.Length - 1 && k > tops[r]) r++;
                            rom[table + k] = (byte)r;
                        }
                        Array.Clear(rom, to, 12);
                        rom[to] = 0x40;
                        WriteInt(rom, to + 4, sub + 0x08000000);
                        WriteInt(rom, to + 8, table + 0x08000000);
                        continue;
                    }
                    Voice(to, 0, bank, at, waves, 60);
                }
                return groups[bankId] = group;
            }

            // MP2K's note and wait lengths (gClockTable: index 0-48);
            static readonly int[] Clocks = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 28, 30,
                32, 36, 40, 42, 44, 48, 52, 54, 56, 60, 64, 66, 68, 72, 76, 78, 80, 84, 88, 90, 92, 96 };
            static int ClockIndex(int ticks) { int i = Clocks.Length - 1; while (Clocks[i] > ticks) i--; return i; }

            /* Level(); A track's volume on the DS's curve (its amplitude the value's square: its decibel table) times the sequence's
             * volume (as it is: the DS's square of both made the songs about half as loud as KatAM's own); */
            static byte Level(int value, int sequenceVolume, int floor) =>
                (byte)Math.Clamp((int)Math.Round(127.0 * Math.Pow(value / 127.0, 2) * (sequenceVolume / 127.0)), floor, 127);

            static int VarLen(byte[] d, ref int at) {
                int value = 0;
                for (int i = 0; i < 4 && at < d.Length; i++) {
                    byte b = d[at++];
                    value = value << 7 | (b & 0x7F);
                    if ((b & 0x80) == 0) break;
                }
                return value;
            }

            /* Block(); A sequence file's commands (from `start` to `end`) as MP2K's, at the sequence's volume: every DS command's spot's
             * MP2K spot (a track starts, a jump or a call lands there); */
            (int block, Dictionary<int, int> map) Block(byte[] d, int start, int end, int volume, int fileKey) {
                if (blocks.TryGetValue((fileKey, volume), out var done)) return done;
                // (read past its end as zeros: the padding after its last command);
                byte[] padded = new byte[d.Length + 16];
                Array.Copy(d, padded, d.Length);
                d = padded;
                List<byte> o = new List<byte>();
                Dictionary<int, int> map = new Dictionary<int, int>();
                List<(int at, int target)> fixups = new List<(int, int)>();
                List<(int key, int left)> ties = new List<(int, int)>();
                int modRange = 1;
                /* (the DS's note wait: a note waits its length unless the track's mono/poly, 0xC7, is 0: the music's tracks all set it 0,
                 * the sound effects never do, so their notes play one after the other; where the last wait or call was written: a jump
                 * back to a spot after both loops with no wait, which hangs MP2K, so a tick's wait goes before it); */
                bool noteWait = true;
                int lastWait = -1, lastCall = -1;
                void Wait(int ticks) {
                    if (ticks > 0) lastWait = o.Count;
                    while (ticks > 0) {
                        int part = ticks;
                        foreach (var tie in ties) part = Math.Min(part, tie.left);
                        int done2 = 0;
                        while (part - done2 > 0) {
                            int i = ClockIndex(part - done2);
                            o.Add((byte)(0x80 + i));
                            done2 += Clocks[i];
                        }
                        ticks -= part;
                        for (int i = ties.Count - 1; i >= 0; i--) {
                            ties[i] = (ties[i].key, ties[i].left - part);
                            if (ties[i].left <= 0) { o.Add(0xCE); o.Add((byte)ties[i].key); ties.RemoveAt(i); }
                        }
                    }
                }
                void EndTies() {
                    foreach (var tie in ties) { o.Add(0xCE); o.Add((byte)tie.key); }
                    ties.Clear();
                }
                int at = start;
                while (at < end) {
                    map[at] = o.Count;
                    byte c = d[at++];
                    if (c < 0x80) {
                        int velocity = d[at++], length = VarLen(d, ref at);
                        byte vel = (byte)Math.Clamp(velocity, 1, 127);
                        if (length > 223) {
                            o.Add(0xCF); o.Add(c); o.Add(vel);
                            ties.Add((c, length));
                        } else {
                            length = Math.Max(1, length);
                            int i = ClockIndex(Math.Min(length, 96));
                            o.Add((byte)(0xCF + i)); o.Add(c); o.Add(vel);
                            if (length - Clocks[i] > 0) o.Add((byte)(length - Clocks[i]));
                        }
                        if (noteWait) Wait(length);
                        continue;
                    }
                    switch (c) {
                    case 0x80: Wait(VarLen(d, ref at)); break;
                    case 0x81: { int program = VarLen(d, ref at); if (program < 128) { o.Add(0xBD); o.Add((byte)program); } break; }
                    case 0x93: at += 4; break;
                    case 0x94: case 0x95: {
                        int target = start + (d[at] | d[at + 1] << 8 | d[at + 2] << 16);
                        at += 3;
                        EndTies();
                        if (c == 0x94 && map.TryGetValue(target, out int back) && lastWait < back && lastCall < back) o.Add(0x81);
                        if (c == 0x95) lastCall = o.Count;
                        o.Add(c == 0x94 ? (byte)0xB2 : (byte)0xB3);
                        fixups.Add((o.Count, target));
                        o.AddRange(new byte[4]);
                        break;
                    }
                    case 0xC0: o.Add(0xBF); o.Add(Math.Min(d[at++], (byte)127)); break;
                    case 0xC1: o.Add(0xBE); o.Add(Level(d[at++], volume, 0)); break;
                    case 0xC3: o.Add(0xBC); o.Add(d[at++]); break;
                    case 0xC4: o.Add(0xC0); o.Add((byte)Math.Clamp(64 + (sbyte)d[at++] / 2, 0, 127)); break;
                    case 0xC5: o.Add(0xC1); o.Add(d[at++]); break;
                    case 0xC6: o.Add(0xBA); o.Add(d[at++]); break;
                    case 0xC7: noteWait = d[at++] != 0; break;
                    case 0xCA: o.Add(0xC4); o.Add((byte)Math.Clamp(d[at++] * modRange / 2, 0, 127)); break;
                    case 0xCB: o.Add(0xC2); o.Add(d[at++]); break;
                    case 0xCC: o.Add(0xC5); o.Add((byte)Math.Min((int)d[at++], 2)); break;
                    case 0xCD: modRange = Math.Max(1, (int)d[at++]); break;
                    case 0xD0: case 0xD1: case 0xD2: case 0xD3: {
                        int value = d[at++];
                        if (value == 0xFF) break;
                        byte[] env = Envelope(value, value, value, value);
                        o.Add(0xCD); o.Add((byte)(0x04 + c - 0xD0)); o.Add(env[c - 0xD0]);
                        break;
                    }
                    case 0xE0: { int delay = (short)ReadShort(d, at); at += 2; o.Add(0xC3); o.Add((byte)Math.Clamp(delay, 0, 127)); break; }
                    case 0xE1: { int tempo = ReadShort(d, at); at += 2; o.Add(0xBB); o.Add((byte)Math.Clamp(tempo, 1, 255)); break; }
                    case 0xE3: at += 2; break;
                    case 0xFC: break;
                    case 0xFD: EndTies(); o.Add(0xB4); break;
                    case 0xFE: at += 2; noteWait = true; break;
                    case 0xFF: EndTies(); o.Add(0xB1); noteWait = true; break;
                    default:
                        if (c >= 0xB0 && c <= 0xBD) { at += 3; break; }
                        if (c >= 0xC0 && c <= 0xD6) { at++; break; }
                        if (c == 0xA0 || c == 0xA1 || c == 0xA2) { at = end; break; }
                        at = end;   // (past the commands: padding);
                        break;
                    }
                }
                EndTies();
                o.Add(0xB1);
                int block = allocate(o.Count, 4);
                for (int i = 0; i < o.Count; i++) rom[block + i] = o[i];
                foreach (var (fix, target) in fixups) {
                    int moved = map.TryGetValue(target, out int m) ? m : o.Count - 1;
                    WriteInt(rom, block + fix, block + moved + 0x08000000);
                }
                return blocks[(fileKey, volume)] = (block, map);
            }

            /* Song(); A sequence (its commands from `start`: its tracks opened there, 0xFE then 0x93 each, track 0 after them) as an MP2K
             * song: each track a start with the DS's defaults (volume 127, pan in the middle, bend range 2, program 0) then GOTO its
             * commands; */
            public int Song(byte[] d, int dataStart, int dataEnd, int start, int volume, int bankId, int priority, int fileKey) {
                (int block, Dictionary<int, int> map) = Block(d, dataStart, dataEnd, volume, fileKey);
                List<int> starts = new List<int>();
                int at = start;
                if (d[at] == 0xFE) at += 3;
                while (at + 5 <= dataEnd && d[at] == 0x93) {
                    starts.Add(dataStart + (d[at + 2] | d[at + 3] << 8 | d[at + 4] << 16));
                    at += 5;
                }
                starts.Insert(0, at);
                starts = starts.Take(16).ToList();
                int header = allocate(8 + starts.Count * 4 + starts.Count * 16, 4);
                rom[header] = (byte)starts.Count;
                rom[header + 1] = 0;
                rom[header + 2] = (byte)Math.Min(priority, 127);
                rom[header + 3] = 0;
                WriteInt(rom, header + 4, Group(bankId) + 0x08000000);
                for (int t = 0; t < starts.Count; t++) {
                    int lead = header + 8 + starts.Count * 4 + t * 16;
                    byte[] defaults = { 0xBD, 0x00, 0xBE, Level(127, volume, 0), 0xBF, 64, 0xC1, 2, 0xC0, 64, 0xB2 };
                    Array.Copy(defaults, 0, rom, lead, defaults.Length);
                    int target = map.TryGetValue(starts[t], out int moved) ? block + moved : block;
                    WriteInt(rom, lead + defaults.Length, target + 0x08000000);
                    WriteInt(rom, header + 8 + t * 4, lead + 0x08000000);
                }
                return header;
            }
        }

        /* AddSqueakSounds(); Squeak Squad's music ids and sound effect ids as songs after the others (see above); the first written for
         * the ported code (gExpansionSongs[1]); */
        static string AddSqueakSounds(byte[] rom, byte[] squeak, KatAMExpansionCode.Version version, ref int cursor) {
            int here = cursor;
            byte[] file = SqueakFile(squeak, "sound/sound_data.sdat");
            if (file == null) return "Squeak Squad's sound/sound_data.sdat wasn't found.";
            List<(int header, ushort player, ushort unused)> songs = new List<(int, ushort, ushort)>();
            try {
                Sdat sdat = new Sdat(file);
                SqueakSoundCompiler compiler = new SqueakSoundCompiler(sdat, rom, (size, align) => Allocate(ref here, size, align));
                Dictionary<int, int> sequenceSongs = new Dictionary<int, int>();
                foreach (int sequence in SqueakMusicSequences) {
                    if (!sequenceSongs.ContainsKey(sequence)) {
                        var (seqFile, bank, volume, priority, _) = sdat.Sequence(sequence);
                        byte[] d = sdat.File(seqFile);
                        int dataStart = ReadInt(d, 0x18), dataEnd = Math.Min(d.Length, 0x10 + ReadInt(d, 0x14));
                        sequenceSongs[sequence] = compiler.Song(d, dataStart, dataEnd, dataStart, volume, bank, 0, seqFile);
                    }
                    songs.Add((sequenceSongs[sequence], 0, 0));
                }
                int empty = Allocate(ref here, 8, 4);
                Dictionary<int, byte[]> archives = new Dictionary<int, byte[]>();
                for (int id = 0; id < SqueakSoundCount; id++) {
                    (int archive, int index) = SqueakSoundId(id);
                    if (archive < 0) { songs.Add((empty, 1, 1)); continue; }
                    if (!archives.TryGetValue(archive, out byte[] ssar)) archives[archive] = ssar = sdat.File(sdat.ArchiveFile(archive));
                    int dataStart = ReadInt(ssar, 0x18), dataEnd = Math.Min(ssar.Length, 0x10 + ReadInt(ssar, 0x14)), record = 0x20 + index * 12;
                    if (index >= ReadInt(ssar, 0x1C) || record + 12 > ssar.Length || ReadInt(ssar, record) == -1) { songs.Add((empty, 1, 1)); continue; }
                    int offset = ReadInt(ssar, record);
                    int bank = ReadShort(ssar, record + 4), volume = ssar[record + 6], priority = ssar[record + 8], player = ssar[record + 9];
                    int header = compiler.Song(ssar, dataStart, dataEnd, dataStart + offset, volume, bank, priority, 0x10000 + sdat.ArchiveFile(archive));
                    ushort mp2kPlayer = (ushort)(1 + player % 3);
                    songs.Add((header, mp2kPlayer, mp2kPlayer));
                }
                squeakSongBase = GrowSongTable(rom, songs, ref here, version.Code.Length);
            } catch (Exception e) when (e is InvalidOperationException or IndexOutOfRangeException or ArgumentException) {
                return "Squeak Squad's sounds: " + e.Message;
            }
            WriteShort(rom, CodeAddress + version.CodeSymbols["gExpansionSongs"] + 2, squeakSongBase);
            cursor = here;
            return null;
        }

        /* SqueakRoomSong(); A Squeak Squad room's music: its map's (the header's byte 0x0B, a music id; 0 keeps the one before: `last`,
         * at first its stage's, SqueakStageMusic()), else KatAM's for its area (no Squeak Squad sounds); */
        static int SqueakRoomSong(byte[] map, int area, ref int last) {
            if (squeakSongBase < 0) return LevelSongs[(area - 1) % LevelSongs.Length];
            int id = map != null && map.Length > 0x0B ? map[0x0B] : 0;
            if (id > 0 && id < SqueakMusicCount) last = id;
            return squeakSongBase + (last > 0 ? last : 1);
        }

        /* SqueakStageMusic(); A stage's music, for its rooms that set none (0): its first room's, else the one most of its rooms set
         * (not 24, the side rooms' music), else 1; */
        static int SqueakStageMusic(byte[] squeak, List<SqueakRoomFiles> files, int area, int stage) {
            List<int> ids = new List<int>();
            foreach (SqueakRoomFiles f in files.Where(r => r.Area == area && r.Stage == stage)) {
                byte[] map = SqueakUnpack(SqueakFileAnyCase(squeak, f.Map));
                ids.Add(map != null && map.Length > 0x0B ? map[0x0B] : 0);
            }
            if (ids.Count > 0 && ids[0] > 0 && ids[0] != 24 && ids[0] < SqueakMusicCount) return ids[0];
            var common = ids.Where(id => id > 0 && id != 24 && id < SqueakMusicCount).GroupBy(id => id).OrderByDescending(g => g.Count()).FirstOrDefault();
            return common?.Key ?? 1;
        }
    }
}
