using System;
using System.Collections.Generic;
using System.Linq;

namespace KatAMRandomizer {
    /* KatAMExpansionSqueakSound; Squeak Squad's music and sound effects in KatAM's sound engine (the owner: "Implement the sounds and
     * music and enemy sound effects for both NiDL and Squeak Squad"). The DS's are all in sound/sound_data.sdat (Nintendo's SDAT): its
     * 42 sequences (the music, SSEQ), 22 sequence archives (the sound effects, SSAR), 83 instrument banks (SBNK) and 2 wave archives
     * (SWAR, every sample IMA-ADPCM). They're compiled into KatAM's MP2K songs:
     * • a sample becomes an 8-bit DirectSound sample (halved while over 24 kHz: KatAM mixes at 15768 Hz), the DS's rate (16756991 /
     *   its timer) and its instrument's root key baked into its rate (MP2K plays a sample's rate at key 60), one copy per sample and
     *   root key;
     * • a bank becomes a voicegroup: a sampled instrument a DirectSound voice (its pan, the DS's envelope by its times: attack's rise,
     *   decay's and release's fall in decibels a second, sustain's level), a PSG one a square (its duty the nearest; its notes moved
     *   by its root: the DS's square plays middle C there), noise noise (its notes at MP2K's nearest noise rate, NoiseKey()); a key
     *   split a key split (its table, its voices), a drum set a drum kit (each key its voice, at that key);
     * • a sequence becomes an MP2K song track by track (TrackCompiler): each DS track run as the DS's own sequencer runs it (NitroSDK's
     *   SND_seq.c and SND_exChannel.c, decompiled in pret's pokediamond) and written out tick for tick, its calls and loops played
     *   out, its jumps back MP2K's GOTO once it's back in the same state: the same notes on the same ticks at the same pitch (checked
     *   against a model of the DS's driver: the 42 songs' notes over 15000 ticks, their pitch and the 638 sounds' over their first
     *   ticks), its tie mode, portamento and bends in MP2K's BEND, TUNE and KEYSH, its volume, expression and velocities on the DS's
     *   decibel curve, its vibrato, its envelope's changes;
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

        /* Sound effects whose notes' release (the MP2K envelope's, per frame) is at most this: Dark Daroach's beam (0x1AF) plays 42
         * one-tick notes a tick apart, each ringing about 30 frames on its release (0xEB), so KatAM mixed up to ten voices for it, a
         * fifth of a frame (the owner: "Massive amounts of lag whenever the Squeak Squad boss invokes stars."); 0x60 rings each for 4,
         * the same notes, pans and bends; 0x30 for 2 (Daroach's beam plays it too: still 4 voices at 0x60; the owner: "Daroach ray
         * causes lag"). Dark Nebula's star (0x1D2, each star he shoots: his rain's, his eye's) the same: 8 voices of it at once after
         * his rain, some 100,000 cycles of KatAM's mixing a frame, every other frame dropped (the owner: "Dark nebula stars are still
         * lagging the game"; his rain's frames dropped: 18 of 140, 2 at 0x60, 1 at 0x30); */
        static readonly Dictionary<int, byte> SqueakSoundReleaseCaps = new Dictionary<int, byte> { [0x1AF] = 0x30, [0x1D2] = 0x30 };
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
            readonly int silentSample;
            // (a sound effect's commands, in its archive's file, and the most its release may be: SqueakSoundReleaseCaps);
            public readonly List<(int fileKey, int start, int end, byte cap)> ReleaseCaps = new List<(int, int, int, byte)>();

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
                // (its rate the DS plays it at: its timer's, 16756991 / timer, as SND_CalcTimer() does; the header's rounded rate if none);
                int encoding = swar[at], timer = ReadShort(swar, at + 4), loopWords = ReadShort(swar, at + 6), lengthWords = ReadInt(swar, at + 8);
                double rate = timer > 0 ? 16756991.0 / timer : ReadShort(swar, at + 2);
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
            void Voice(int to, int type, byte[] bank, int def, int[] waves, int key, bool drum = false) {
                Array.Clear(rom, to, 12);
                int wave = ReadShort(bank, def), archive = ReadShort(bank, def + 2), root = bank[def + 4], pan = bank[def + 9];
                byte[] envelope = Envelope(bank[def + 5], bank[def + 6], bank[def + 7], bank[def + 8]);
                // (a drum's key, MP2K playing the voice at it: a PSG one's moved so it sounds as the DS's, middle C at its root);
                rom[to + 1] = (byte)(drum && type == 2 ? Math.Clamp(key + 60 - root, 0, 127) : drum && type == 3 ? NoiseKey(key, root) : key);
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
                            else Voice(kit + k * 12, bank[at + 2 + (k - low) * 12], bank, at + 4 + (k - low) * 12, waves, k, true);
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

            // (an amplitude on the DS's curve: its decibel tables' square of a 0-127 value);
            static double Square(int value) => Math.Pow(Math.Clamp(value, 0, 127) / 127.0, 2);
            /* Level(); An MP2K VOL for an amplitude (the DS's volume, expression, a held tie's velocity: each squared, multiplied), times
             * the sequence's volume (as it is: the DS's square of it made the songs about half as loud as KatAM's own); */
            static byte Level(double amplitude, int sequenceVolume) =>
                (byte)Math.Clamp((int)Math.Round(127.0 * amplitude * (sequenceVolume / 127.0)), 0, 127);

            static int VarLen(byte[] d, ref int at) {
                int value = 0;
                for (int i = 0; i < 4 && at < d.Length; i++) {
                    byte b = d[at++];
                    value = value << 7 | (b & 0x7F);
                    if ((b & 0x80) == 0) break;
                }
                return value;
            }

            readonly Dictionary<int, byte[]> bankFiles = new Dictionary<int, byte[]>();
            /* Instrument(); What a bank's program plays a key with: its type (1 a sample, 2 a PSG square, 3 noise; 0 none), the bank file
             * and its definition's offset (a drum set's key's, a key split's region's), the bank's wave archives; */
            (int type, byte[] bank, int def, int[] waves) Instrument(int bankId, int program, int key) {
                (int file, int[] waves) = sdat.Bank(bankId);
                if (file < 0) return (0, null, 0, null);
                if (!bankFiles.TryGetValue(file, out byte[] bank)) bankFiles[file] = bank = sdat.File(file);
                if (program < 0 || program >= ReadInt(bank, 0x38)) return (0, null, 0, null);
                int type = bank[0x3C + program * 4], at = ReadShort(bank, 0x3C + program * 4 + 1);
                if (type == 16) {
                    int low = bank[at], high = bank[at + 1];
                    if (key < low || key > high) return (0, null, 0, null);
                    return (bank[at + 2 + (key - low) * 12], bank, at + 4 + (key - low) * 12, waves);
                }
                if (type == 17) {
                    int r = 0;
                    while (r < 7 && bank[at + r + 1] != 0 && key > bank[at + r]) r++;
                    return (bank[at + 8 + r * 12], bank, at + 10 + r * 12, waves);
                }
                return (type, bank, at, waves);
            }
            /* PsgKey(); The key MP2K's square plays a DS PSG note with: the DS's square plays middle C (its timer 8006) at the instrument's
             * root key, MP2K's at key 60; a sample's key as it is (its root baked into its rate, Sample()); */
            // (a program that is itself a PSG or noise voice; a drum set's or key split's notes pick their voice by their key, its
            // drums moved in the drum kit itself, Voice());
            int PlayedKey(int bankId, int program, int key) {
                var (type, bank, def, _) = Instrument(bankId, program, key);
                if (type is not (2 or 3) || bank[0x3C + program * 4] is 16 or 17) return key;
                return type == 2 ? Math.Clamp(key + 60 - bank[def + 4], 0, 127) : NoiseKey(key, bank[def + 4]);
            }
            /* NoiseKey(); The key MP2K's noise plays the DS's noise with: the DS's steps its noise at 16756991 / 8006 a second (2093) at
             * the instrument's root key, a semitone's ratio a key; MP2K's at keys 21-80 by gNoiseTable (s << 4 | r: 524288 / r / 2^(s + 1)
             * a second, r 0 as 0.5): the nearest; */
            static readonly byte[] NoiseTable = {
                0xD7, 0xD6, 0xD5, 0xD4, 0xC7, 0xC6, 0xC5, 0xC4, 0xB7, 0xB6, 0xB5, 0xB4, 0xA7, 0xA6, 0xA5, 0xA4, 0x97, 0x96, 0x95, 0x94,
                0x87, 0x86, 0x85, 0x84, 0x77, 0x76, 0x75, 0x74, 0x67, 0x66, 0x65, 0x64, 0x57, 0x56, 0x55, 0x54, 0x47, 0x46, 0x45, 0x44,
                0x37, 0x36, 0x35, 0x34, 0x27, 0x26, 0x25, 0x24, 0x17, 0x16, 0x15, 0x14, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
            };
            static int NoiseKey(int key, int root) {
                double ds = 16756991.0 / 8006 * Math.Pow(2, (key - root) / 12.0);
                int best = 60;
                double bestError = double.MaxValue;
                for (int k = 21; k <= 80; k++) {
                    int v = NoiseTable[k - 21], r = v & 7, sh = v >> 4;
                    double rate = 524288 / (r == 0 ? 0.5 : r) / Math.Pow(2, sh + 1), error = Math.Abs(Math.Log(rate / ds));
                    if (error < bestError) { bestError = error; best = k; }
                }
                return best;
            }
            /* SampleSeconds(); How long a note of a one-shot sample sounds at a key (its samples at the DS's rate, 16756991 / its timer,
             * shifted by the key from its root), the DS's channel ending with it; infinity for a looped one, a PSG or noise; */
            double SampleSeconds(int bankId, int program, int key) {
                var (type, bank, def, waves) = Instrument(bankId, program, key);
                if (type != 1) return double.PositiveInfinity;
                int wave = ReadShort(bank, def), archive = ReadShort(bank, def + 2), root = bank[def + 4];
                if (archive >= 4 || waves[archive] == 0xFFFF) return double.PositiveInfinity;
                byte[] swar = WaveArchive(waves[archive]);
                if (wave < 0 || wave >= ReadInt(swar, 0x38)) return double.PositiveInfinity;
                int at = ReadInt(swar, 0x3C + wave * 4), encoding = swar[at], timer = ReadShort(swar, at + 4);
                if (swar[at + 1] != 0 || timer == 0) return double.PositiveInfinity;
                int words = ReadShort(swar, at + 6) + ReadInt(swar, at + 8);
                int samples = encoding == 2 ? (words * 4 - 4) * 2 : encoding == 1 ? words * 2 : words * 4;
                return samples / (16756991.0 / timer * Math.Pow(2, (key - root) / 12.0));
            }

            /* DsTrack; A DS track's state as the sequencer keeps it (SND_seq.c's TrackInit: its defaults), what the compiled MP2K stream
             * depends on; */
            sealed class DsTrack {
                public int Volume = 127, Expression = 127, Bend, BendRange = 2, Transpose, Program, PortaKey = 60, PortaTime, Sweep;
                public int ModDepth, ModSpeed = 16, ModTarget, ModRange = 1, ModDelay;
                public bool NoteWait = true, Tie, Porta;
                public DsTrack Copy() => (DsTrack)MemberwiseClone();
                public string Key(bool porta) => $"{Volume},{Expression},{Bend},{BendRange},{Transpose},{Program},{(Porta || Tie ? PortaKey : 0)},{PortaTime},{Sweep}," +
                                                 $"{ModDepth},{ModSpeed},{ModTarget},{ModRange},{ModDelay},{NoteWait},{Tie},{Porta}";
            }

            /* TrackCompiler; One DS track run as the DS's sequencer runs it (NitroSDK's SND_seq.c, TrackStepTicks(), and SND_exChannel.c's
             * pitch: its commands one by one, its calls and loops played out, its jumps back a GOTO once the track comes back to a spot in
             * the same state) and written as MP2K's commands, tick for tick:
             * • the DS's 48 ticks a beat kept, MP2K's tempo set to match (TempoValue());
             * • its notes with their length (a note waits its length while the track's note wait is on, its mono/poly: the music turns it
             *   off, the sound effects keep it), a length past MP2K's 223 a tie and its end; a length of 0 a note held (the DS's -1);
             * • its pitch exact (1/64 of a semitone): its bend (pitchBend * bendRange * 64 / 128), a tie's key change (the DS's tie mode
             *   changes the held channel's key: the note held, its pitch moved) and the portamento's sweep (from the last key, over the
             *   note's length or the portamento time's, SND_exChannel.c's ExChannelSweepUpdate(), tick by tick) in MP2K's BEND with its
             *   range 64 (a semitone a step) and TUNE (the rest, 1/64 of a semitone a step);
             * • its volume, expression and a tie's velocity in VOL (each squared, the DS's decibel tables), its velocities squared too,
             *   the loudest note of the song at 127 (`maxVelocity`);
             * • its modulation (SND_exChannel.c's LFO: a sine, `speed` << 6 a 192nd of a second over 128 << 8, `depth` * `range`) in
             *   MP2K's (a triangle, `speed` a tick over 256): MOD, LFOS by the tempo, LFODL from 192nds of a second to ticks, MODT;
             * • its envelope's changes (XCMD's), pan, priority, program; */
            sealed class TrackCompiler {
                readonly SqueakSoundCompiler c;
                readonly byte[] d;
                readonly int dataStart, dataEnd, volume, bankId, fileKey, maxVelocity;
                readonly Func<int, int> tempoAt;
                public readonly List<byte> O = new List<byte>();
                public readonly List<(int at, int target)> Gotos = new List<(int, int)>();
                public readonly List<(int tick, int tempo)> Tempos = new List<(int, int)>();
                public int MaxVelocity;
                readonly HashSet<int> targets;
                readonly Dictionary<string, int> visits = new Dictionary<string, int>();
                readonly Dictionary<int, int> visitCount = new Dictionary<int, int>();
                DsTrack t = new DsTrack();
                int tick;
                // (what MP2K has now, so a command goes only on a change: VOL, the pitch in 1/64 semitones, the LFO's four, the voice);
                int vol = -1, pitch = int.MinValue, bendSet, tuneSet, shiftSet, mod = -1, lfos = -1, lfodl = -1, modt = -1, voice = -1;
                // (a held tie: its key as MP2K plays it, its velocity, the tick its one-shot sample ends; the notes held for good, the
                // long notes' ends: key, ticks left);
                int heldKey = -1, heldVelocity, heldUntil = int.MaxValue;
                readonly List<int> heldForever = new List<int>();
                readonly List<(int key, int left)> longNotes = new List<(int, int)>();
                // (the sweep: its pitch, length, counter, whether by 192nds of a second (the portamento time's) or ticks; the tie's key offset);
                int sweepPitch, sweepLength, sweepCounter;
                bool autoSweep;
                int sweepTick, sweepTempo = 120;
                int tieOffset;

                public TrackCompiler(SqueakSoundCompiler c, byte[] d, int dataStart, int dataEnd, int volume, int bankId, int fileKey,
                                     int maxVelocity, Func<int, int> tempoAt, HashSet<int> targets) {
                    (this.c, this.d, this.dataStart, this.dataEnd, this.volume, this.bankId, this.fileKey, this.maxVelocity, this.tempoAt, this.targets) =
                        (c, d, dataStart, dataEnd, volume, bankId, fileKey, Math.Max(1, maxVelocity), tempoAt, targets);
                }

                void Add(params int[] bytes) { foreach (int b in bytes) O.Add((byte)b); }
                int Tempo => Math.Max(1, tempoAt(tick));

                void SetVolume() {
                    double amplitude = Square(t.Volume) * Square(t.Expression) * (heldKey >= 0 ? Velocity(heldVelocity) / 127.0 : 1);
                    int v = Level(amplitude, volume);
                    if (v != vol) { Add(0xBE, v); vol = v; }
                }
                int Velocity(int v) => Math.Clamp((int)Math.Round(127.0 * Math.Pow(Math.Min(v, maxVelocity) / (double)maxVelocity, 2)), 0, 127);
                /* (the sweep's counter: by ticks, or by the DS's frames since the note's: tick n of the sequence comes in its frame
                 * ceil(240 * n / tempo), the tempo counter starting at 240, SND_seq.c's PlayerInit()); */
                int SweepCounter() {
                    if (!autoSweep) return sweepCounter;
                    long Frame(long n) => (240 * n + sweepTempo - 1) / sweepTempo;
                    return (int)(Frame(sweepTick + sweepCounter) - Frame(sweepTick));
                }
                int SweepNow() {
                    if (sweepPitch == 0 || sweepLength <= 0) return 0;
                    int counter = SweepCounter();
                    return counter >= sweepLength ? 0 : (int)((long)sweepPitch * (sweepLength - counter) / sweepLength);
                }
                bool Sweeping => sweepPitch != 0 && sweepLength > 0 && SweepCounter() < sweepLength;
                void SetPitch() {
                    int p = (t.Bend * (t.BendRange << 6) >> 7) + tieOffset + SweepNow();
                    if (p == pitch) return;
                    // (past BEND's 64 semitones either way (a tie's long glide, 0x92's), the rest in KEYSH, which moves held notes too);
                    int semitones = (int)Math.Floor(p / 64.0), bend = Math.Clamp(semitones, -64, 63), tune = p - semitones * 64;
                    int shift = Math.Clamp(semitones - bend, -127, 127);
                    if (shift != shiftSet) { Add(0xBC, shift & 0xFF); shiftSet = shift; }
                    if (bend != bendSet) { Add(0xC0, 64 + bend); bendSet = bend; }
                    if (tune != tuneSet) { Add(0xC8, 64 + tune); tuneSet = tune; }
                    pitch = p;
                }
                void SetModulation() {
                    int depth = t.ModDepth * t.ModRange, m;
                    if (t.ModTarget == 1) {
                        double db = 127.0 * depth * 60 / 16384 / 10;
                        m = (int)Math.Round(128 * (1 - Math.Pow(10, -db / 20)));
                    } else if (t.ModTarget == 2) m = (int)Math.Round(127.0 * depth / 128);
                    else m = (int)Math.Round(127.0 * depth / 1024);
                    m = Math.Clamp(m, 0, 127);
                    if (m != mod) { Add(0xC4, m); mod = m; }
                    if (m == 0) return;
                    int s = Math.Clamp((int)Math.Round(120.0 * t.ModSpeed / Tempo), 0, 255), dl = Math.Clamp((int)Math.Round(t.ModDelay * Tempo / 240.0), 0, 255);
                    int target = Math.Min(t.ModTarget, 2);
                    if (s != lfos) { Add(0xC2, s); lfos = s; }
                    if (dl != lfodl) { Add(0xC3, dl); lfodl = dl; }
                    if (target != modt) { Add(0xC5, target); modt = target; }
                }

                // Wait(); `ticks` ticks on: MP2K's waits, a long note's end where it falls, the sweep's pitch a tick at a time while it runs;
                void Wait(int ticks) {
                    while (ticks > 0) {
                        int part = Sweeping ? 1 : ticks;
                        foreach (var note in longNotes) part = Math.Min(part, note.left);
                        for (int done = 0; part - done > 0;) {
                            int i = ClockIndex(part - done);
                            Add(0x80 + i);
                            done += Clocks[i];
                        }
                        ticks -= part;
                        tick += part;
                        if (sweepPitch != 0) { sweepCounter += part; SetPitch(); }
                        for (int i = longNotes.Count - 1; i >= 0; i--) {
                            longNotes[i] = (longNotes[i].key, longNotes[i].left - part);
                            if (longNotes[i].left <= 0) { Add(0xCE, longNotes[i].key); longNotes.RemoveAt(i); }
                        }
                    }
                }
                // ReleaseAll(); The track's notes let go (the DS's TrackReleaseChannels(): the tie mode's change, the track's end);
                void ReleaseAll() {
                    if (heldKey >= 0) { Add(0xCE, heldKey); heldKey = -1; }
                    foreach (int key in heldForever) Add(0xCE, key);
                    heldForever.Clear();
                    foreach (var note in longNotes) Add(0xCE, note.key);
                    longNotes.Clear();
                    tieOffset = 0;
                    SetVolume();
                }

                void Note(int key, int velocity, int length) {
                    key = Math.Clamp(key + t.Transpose, 0, 127);
                    int played = c.PlayedKey(bankId, t.Program, key);
                    // (the sweep: the DS's sweep pitch, and from the portamento's key, over the note's length or the portamento time's);
                    int sweep = t.Sweep + (t.Porta ? (t.PortaKey - key) << 6 : 0);
                    bool held = t.Tie && heldKey >= 0 && tick < heldUntil;
                    if (t.Tie && heldKey >= 0 && !held) ReleaseAll();
                    sweepPitch = sweep;
                    sweepCounter = 0;
                    sweepTick = tick;
                    sweepTempo = Tempo;
                    if (t.PortaTime == 0) { sweepLength = length > 0 ? length : -1; autoSweep = false; }
                    else {
                        sweepLength = (int)((long)t.PortaTime * t.PortaTime * Math.Abs(sweep) >> 11);
                        autoSweep = true;
                    }
                    if (held) {
                        // (the tie mode's held note: its key and velocity changed, the channel playing on);
                        tieOffset = (played - heldKey) * 64;
                        heldVelocity = velocity;
                        SetVolume();
                        SetPitch();
                    } else if (t.Tie || length <= 0) {
                        tieOffset = 0;
                        SetPitch();
                        if (t.Tie) {
                            heldKey = played;
                            heldVelocity = velocity;
                            double seconds = c.SampleSeconds(bankId, t.Program, key);
                            heldUntil = double.IsInfinity(seconds) ? int.MaxValue : tick + Math.Max(1, (int)Math.Ceiling(seconds * Tempo * 0.8));
                            SetVolume();
                            Add(0xCF, played, 127);
                        } else {
                            SetVolume();
                            Add(0xCF, played, Velocity(velocity));
                            heldForever.Add(played);
                        }
                    } else {
                        tieOffset = 0;
                        SetVolume();
                        SetPitch();
                        int v = Velocity(velocity);
                        if (length > 223) {
                            Add(0xCF, played, v);
                            longNotes.Add((played, length));
                        } else {
                            int i = ClockIndex(Math.Min(length, 96));
                            Add(0xCF + i, played, v);
                            if (length - Clocks[i] > 0) Add(length - Clocks[i]);
                        }
                    }
                    MaxVelocity = Math.Max(MaxVelocity, velocity);
                    t.PortaKey = key;
                    if (t.NoteWait) {
                        if (length > 0) Wait(length);
                        else {
                            // (a note of length 0 waits till its channel is done: a one-shot sample's end, else for good);
                            double seconds = c.SampleSeconds(bankId, t.Program, key);
                            if (!double.IsInfinity(seconds)) Wait(Math.Max(1, (int)Math.Ceiling(seconds * Tempo * 0.8)));
                            else { int here = O.Count; Add(0x80 + 48); Add(0xB2); Gotos.Add((O.Count, here)); Add(0, 0, 0, 0); end = true; }
                        }
                    }
                }

                bool end;
                // Compile(); The track from `start` (MP2K's defaults first: the DS's TrackInit()'s);
                public void Compile(int start) {
                    Add(0xBD, 0, 0xC1, 64, 0xC0, 64, 0xC8, 64, 0xBF, 64);
                    pitch = 0; voice = 0;
                    SetVolume();
                    Add(0xC4, 0); mod = 0;
                    var stack = new List<(int ret, int count, bool loop)>();
                    int at = start, steps = 0;
                    while (!end && at < dataEnd && O.Count < 0x40000 && steps++ < 400000) {
                        // (a spot a jump comes back to: MP2K's GOTO there once the track is back in the same state);
                        if (targets.Contains(at)) {
                            string key = at + "|" + string.Join(";", stack.Select(s => $"{s.ret},{s.count},{s.loop}")) + "|" + t.Key(true) + $"|{heldKey},{heldVelocity},{tieOffset},{sweepPitch},{Sweeping},{heldForever.Count},{longNotes.Count},{Tempo}";
                            if (visits.TryGetValue(key, out int pos)) {
                                Add(0xB2);
                                Gotos.Add((O.Count, pos));
                                Add(0, 0, 0, 0);
                                return;
                            }
                            visitCount[at] = visitCount.GetValueOrDefault(at) + 1;
                            if (visitCount[at] > 8) { Add(0xB1); return; }
                            visits[key] = O.Count;
                        }
                        int commandAt = at;
                        byte cmd = d[at++];
                        bool run = true, random = false, variable = false;
                        if (cmd == 0xA2) { cmd = d[at++]; }
                        if (cmd == 0xA0) { cmd = d[at++]; random = true; }
                        if (cmd == 0xA1) { cmd = d[at++]; variable = true; }
                        // (a value: a random one's middle, a variable's (none in Squeak Squad's) 0);
                        int Value(Func<int> plain) {
                            if (random) { int lo = (short)ReadShort(d, at), hi = (short)ReadShort(d, at + 2); at += 4; return (lo + hi) / 2; }
                            if (variable) { at++; return 0; }
                            return plain();
                        }
                        if (cmd < 0x80) {
                            int velocity = d[at++];
                            int length = Value(() => VarLen(d, ref at));
                            if (run) Note(cmd, velocity, length);
                            continue;
                        }
                        switch (cmd & 0xF0) {
                        case 0x80: {
                            int value = Value(() => VarLen(d, ref at));
                            if (cmd == 0x80) Wait(value);
                            else if (cmd == 0x81 && value < 128) { t.Program = value; if (value != voice) { Add(0xBD, value); voice = value; } }
                            break;
                        }
                        case 0x90: {
                            if (cmd == 0x93) { at += 4; break; }
                            int target = dataStart + (d[at] | d[at + 1] << 8 | d[at + 2] << 16);
                            at += 3;
                            if (cmd == 0x94) at = target;
                            else if (cmd == 0x95 && stack.Count < 3) { stack.Add((at, 0, false)); at = target; }
                            break;
                        }
                        case 0xC0: case 0xD0: {
                            int value = Value(() => d[at++]) & 0xFF;
                            sbyte signed = (sbyte)value;
                            switch (cmd) {
                            case 0xC0: Add(0xBF, Math.Min(value, 127)); break;
                            case 0xC1: t.Volume = value; SetVolume(); break;
                            case 0xD5: t.Expression = value; SetVolume(); break;
                            case 0xC3: t.Transpose = signed; break;
                            case 0xC4: t.Bend = signed; SetPitch(); break;
                            case 0xC5: t.BendRange = value; SetPitch(); break;
                            case 0xC6: Add(0xBA, value); break;
                            case 0xC7: t.NoteWait = value != 0; break;
                            case 0xC8: t.Tie = value != 0; ReleaseAll(); break;
                            case 0xC9: t.PortaKey = (value + t.Transpose) & 0xFF; t.Porta = true; break;
                            case 0xCE: t.Porta = value != 0; break;
                            case 0xCF: t.PortaTime = value; break;
                            case 0xCA: t.ModDepth = value; SetModulation(); break;
                            case 0xCB: t.ModSpeed = value; SetModulation(); break;
                            case 0xCC: t.ModTarget = value; SetModulation(); break;
                            case 0xCD: t.ModRange = value; SetModulation(); break;
                            case 0xD0: case 0xD1: case 0xD2: case 0xD3: {
                                if (value == 0xFF) break;
                                byte set = Envelope(value, value, value, value)[cmd - 0xD0];
                                if (cmd == 0xD3)
                                    foreach (var (key2, from, to, cap) in c.ReleaseCaps)
                                        if (key2 == fileKey && commandAt >= from && commandAt < to) set = Math.Min(set, cap);
                                Add(0xCD, 0x04 + cmd - 0xD0, set);
                                break;
                            }
                            case 0xD4: if (stack.Count < 3) stack.Add((at, value, true)); break;
                            }
                            break;
                        }
                        case 0xE0: {
                            int value = (short)Value(() => { int v = ReadShort(d, at); at += 2; return v; });
                            if (cmd == 0xE3) t.Sweep = value;
                            else if (cmd == 0xE1) { Tempos.Add((tick, value & 0xFFFF)); Add(0xBB, TempoValue(value & 0xFFFF)); }
                            else if (cmd == 0xE0) { t.ModDelay = value & 0xFFFF; SetModulation(); }
                            break;
                        }
                        case 0xB0: at++; Value(() => { at += 2; return 0; }); break;
                        case 0xF0:
                            if (cmd == 0xFD) {
                                // (a call's return; inside a loop of the DS's, its loop's return spot: as the DS's stack);
                                if (stack.Count > 0) { at = stack[^1].ret; stack.RemoveAt(stack.Count - 1); }
                            } else if (cmd == 0xFC) {
                                if (stack.Count == 0) break;
                                var top = stack[^1];
                                int count = top.count;
                                if (count != 0) {
                                    count--;
                                    if (count == 0) { stack.RemoveAt(stack.Count - 1); break; }
                                }
                                stack[^1] = (top.ret, count, top.loop);
                                at = top.ret;
                            } else if (cmd == 0xFE) at += 2;
                            else if (cmd == 0xFF) end = true;
                            break;
                        default:
                            end = true;
                            break;
                        }
                    }
                    if (O.Count > 0 && O[^5] == 0xB2 && Gotos.Count > 0 && Gotos[^1].at == O.Count - 4) return;
                    ReleaseAll();
                    Add(0xB1);
                }
            }

            /* TempoValue(); MP2K's TEMPO for a DS tempo at 48 ticks a beat: the DS ticks 191.96 times a second, `tempo` / 240 ticks
             * each (0.79983 * tempo a second), MP2K 59.7275 times a second, 2 * TEMPO / 150 ticks each (0.79637 * TEMPO a second); */
            static int TempoValue(int tempo) => Math.Clamp((int)Math.Round(tempo * (191.958 / 240) / (59.7275 * 2 / 150)), 1, 255);

            /* Song(); A sequence (its tracks opened at `start`: 0xFE, then 0x93 each, track 0 after them) as an MP2K song, each track a
             * TrackCompiler's (run twice: first for the tempo changes and the loudest velocity of the song, then for good); */
            public int Song(byte[] d, int dataStart, int dataEnd, int start, int volume, int bankId, int priority, int fileKey) {
                byte[] padded = new byte[d.Length + 32];
                Array.Copy(d, padded, d.Length);
                d = padded;
                List<int> starts = new List<int>();
                int at = start;
                if (d[at] == 0xFE) at += 3;
                while (at + 5 <= dataEnd && d[at] == 0x93) {
                    starts.Add(dataStart + (d[at + 2] | d[at + 3] << 8 | d[at + 4] << 16));
                    at += 5;
                }
                starts.Insert(0, at);
                starts = starts.Take(16).ToList();
                // (the spots jumps and loops go back to: the GOTOs' possible targets);
                HashSet<int> targets = new HashSet<int>();
                foreach (int s in starts) JumpTargets(d, dataStart, dataEnd, s, targets);
                List<(int tick, int tempo)> tempos = new List<(int, int)>();
                int maxVelocity = 1;
                foreach (int s in starts) {
                    TrackCompiler dry = new TrackCompiler(this, d, dataStart, dataEnd, volume, bankId, fileKey, 127, _ => 120, targets);
                    dry.Compile(s);
                    tempos.AddRange(dry.Tempos);
                    maxVelocity = Math.Max(maxVelocity, dry.MaxVelocity);
                }
                tempos = tempos.OrderBy(x => x.tick).ToList();
                int TempoAt(int tick) {
                    int tempo = 120;
                    foreach (var (when, value) in tempos) { if (when > tick) break; tempo = value; }
                    return tempo;
                }
                List<TrackCompiler> tracks = new List<TrackCompiler>();
                foreach (int s in starts) {
                    TrackCompiler track = new TrackCompiler(this, d, dataStart, dataEnd, volume, bankId, fileKey, maxVelocity, TempoAt, targets);
                    track.Compile(s);
                    tracks.Add(track);
                }
                int header = allocate(8 + tracks.Count * 4, 4);
                rom[header] = (byte)tracks.Count;
                rom[header + 1] = 0;
                rom[header + 2] = (byte)Math.Min(priority, 127);
                rom[header + 3] = 0;
                WriteInt(rom, header + 4, Group(bankId) + 0x08000000);
                for (int k = 0; k < tracks.Count; k++) {
                    List<byte> o = tracks[k].O;
                    int block = allocate(o.Count, 4);
                    for (int i = 0; i < o.Count; i++) rom[block + i] = o[i];
                    foreach (var (fix, target) in tracks[k].Gotos) WriteInt(rom, block + fix, block + target + 0x08000000);
                    WriteInt(rom, header + 8 + k * 4, block + 0x08000000);
                }
                return header;
            }

            // JumpTargets(); The spots a track's jumps (0x94) and endless loops (0xD4 0) go back to, from its start through its calls;
            static void JumpTargets(byte[] d, int dataStart, int dataEnd, int start, HashSet<int> targets) {
                HashSet<int> seen = new HashSet<int>();
                Stack<int> todo = new Stack<int>();
                todo.Push(start);
                while (todo.Count > 0) {
                    int at = todo.Pop();
                    while (at < dataEnd && seen.Add(at)) {
                        byte cmd = d[at++];
                        int extra = 0;
                        while (cmd is 0xA0 or 0xA1 or 0xA2) {
                            if (cmd == 0xA0) extra = 4; else if (cmd == 0xA1) extra = 1;
                            cmd = d[at++];
                        }
                        if (cmd < 0x80) { at++; if (extra > 0) at += extra; else VarLen(d, ref at); continue; }
                        if (cmd is 0x80 or 0x81) { if (extra > 0) at += extra; else VarLen(d, ref at); continue; }
                        if (cmd == 0x93) { at += 4; continue; }
                        if (cmd is 0x94 or 0x95) {
                            int target = dataStart + (d[at] | d[at + 1] << 8 | d[at + 2] << 16);
                            at += 3;
                            if (cmd == 0x94) { targets.Add(target); at = target; }
                            else todo.Push(target);
                            continue;
                        }
                        if (cmd == 0xD4) { if (extra == 0 && d[at] == 0) targets.Add(at + 1); at += extra > 0 ? extra : 1; continue; }
                        if (cmd >= 0xB0 && cmd <= 0xBF) { at += 1 + (extra > 0 ? extra : 2); continue; }
                        if (cmd >= 0xC0 && cmd <= 0xDF) { at += extra > 0 ? extra : 1; continue; }
                        if (cmd >= 0xE0 && cmd <= 0xEF) { at += extra > 0 ? extra : 2; continue; }
                        if (cmd == 0xFE) { at += 2; continue; }
                        if (cmd is 0xFF or 0xFD) break;
                        if (cmd >= 0xF0) continue;
                        break;
                    }
                }
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
                /* (each capped sound's commands: from its start to the next sequence's in its archive, the archive's sequences one after
                 * the other; before any of its archive's are written, the archive's commands written once for all of them); */
                foreach ((int id, byte cap) in SqueakSoundReleaseCaps) {
                    (int archive, int index) = SqueakSoundId(id);
                    if (archive < 0) continue;
                    byte[] ssar = sdat.File(sdat.ArchiveFile(archive));
                    int dataStart = ReadInt(ssar, 0x18), dataEnd = Math.Min(ssar.Length, 0x10 + ReadInt(ssar, 0x14)), count = ReadInt(ssar, 0x1C);
                    if (index >= count) continue;
                    int offset = ReadInt(ssar, 0x20 + index * 12);
                    int next = Enumerable.Range(0, count).Select(k => ReadInt(ssar, 0x20 + k * 12)).Where(o => o != -1 && o > offset)
                                         .DefaultIfEmpty(dataEnd - dataStart).Min();
                    compiler.ReleaseCaps.Add((0x10000 + sdat.ArchiveFile(archive), dataStart + offset, dataStart + next, cap));
                }
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
            /* (KatAM's mixer at MP2K's most, 12 sampled voices, not its 10: the DS has 16, its songs sounding up to 13 notes at once (the
             * Squeak hideout's, the last boss's), MP2K cutting the oldest; its sound mode, m4aSoundInit()'s and GameInit()'s literal: 8-bit
             * output, 15768 Hz, volume 15, the channels); */
            const int KatAMSoundMode = 0x00900000 | 0x00050000 | 15 << 12, KatAMChannels = 10, SqueakChannels = 12;
            for (int at = 0x140000; at + 4 <= 0x160000; at += 4)
                if (ReadInt(rom, at) == (KatAMSoundMode | KatAMChannels << 8)) WriteInt(rom, at, KatAMSoundMode | SqueakChannels << 8);
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
