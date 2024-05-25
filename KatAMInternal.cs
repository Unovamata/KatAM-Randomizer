using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace KatAMInternal {
    internal class Settings {
        public string ROMPath { get; set; }
        public string ROMDirectory { get; set; }
        public string ROMFilename { get; set; }
        public byte [] ROMData { get; set; }
        public int Seed { get; set; }
        public bool IsRace { get; set; }
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
    }
}
