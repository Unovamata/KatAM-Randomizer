namespace KatAMInternal {
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
        public Random RandomEntity { get; set; }
        public int Seed { get; set; }
        public int MinSeed = -9999999, MaxSeed = 9999999;
        public bool IsRace { get; set; }
        public int SprayGeneration { get; set; }
        public int SprayOutlineGeneration { get; set; }

        public Settings() {
            RandomEntity = new Random();
            Seed = RandomEntity.Next(MinSeed, MaxSeed);
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

        public static int GetRandomNumber(Random random, int min, int max) {
            return random.Next(min, max);
        }
    }
}
