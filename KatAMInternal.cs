using System.ComponentModel;

namespace KatAMInternal {
    public enum SprayGen {
        Unchanged = 0,
        Presets = 1,
        RandomAndPresets = 2,
        Random = 3,
        All = 4
    }

    internal class Processing {
        public string ROMPath { get; set; }
        public string ROMDirectory { get; set; }
        public string ROMFilename { get; set; }
        public byte[] ROMData { get; set; }
        public Settings Settings { get; set; }

        public Processing() {
            Settings = new Settings();
        }
    }

    public class Settings {
        private static Settings Instance;
        public Random RandomEntity { get; set; }
        public int Seed { get; set; }
        public int MinSeed = -9999999, MaxSeed = 9999999;
        public bool IsRace { get; set; }
        public SprayGen SprayGeneration { get; set; }
        public SprayGen SprayOutlineGenerationType { get; set; }

        public Settings() {
            Random seedGenerator = new Random();
            Seed = seedGenerator.Next(MinSeed, MaxSeed);
            RandomEntity = new Random(Seed);
            Instance = this;
        }

        public static Settings GetInstance() {
            return Instance;
        }

        public void GetNewSeed() {
            Random seedGenerator = new Random();
            Seed = seedGenerator.Next(MinSeed, MaxSeed);
            RandomEntity = new Random(Seed);
        }
    }

    public static class Utils {
        public static void WriteToROM(byte[] romData, long address, byte[] valueBytes) {
            int bytesLength = valueBytes.Length;

            if (address < 0 || address + bytesLength > romData.Length) {
                throw new ArgumentOutOfRangeException(nameof(address), "Address is out of bounds of the file contents.");
            }

            for (int i = 0; i < bytesLength; i++) {
                romData[address + i] = valueBytes[i];
            }
        }

        public static int GetRandomNumber(int min, int max) {
            Settings settings = Settings.GetInstance();

            return settings.RandomEntity.Next(min, max);
        }

        public static int Dice() {
            Settings settings = Settings.GetInstance();

            return settings.RandomEntity.Next(1, 7);
        }

        public static string ConvertIntToHex(int input) {
            string hex = input.ToString("X");

            return hex;
        }

        public static string ConvertLongToHex(long input) {
            string hex = input.ToString("X");

            return hex;
        }

        public static byte[] LongToBytes(long input) {
            string hex = ConvertLongToHex(input);

            // Ensure the input string length is even
            if (hex.Length % 2 != 0)
                throw new ArgumentException("Hex string must have an even length.");

            byte[] bytes = new byte[hex.Length / 2];
            for (int i = 0; i < hex.Length; i += 2) {
                bytes[i / 2] = Convert.ToByte(hex.Substring(i, 2), 16);
            }
            return bytes;
        }
    }
}
