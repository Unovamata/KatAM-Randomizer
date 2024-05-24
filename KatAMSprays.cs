using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Intrinsics.Arm;
using System.Text;
using System.Threading.Tasks;
using KatAMInternal;
using static System.Windows.Forms.VisualStyles.VisualStyleElement.Rebar;

namespace KatAMRandomizer {
    internal class KatAMSprays {
        public static void RandomizeSpray(byte[] romFile) {
            Console.WriteLine("Randomizing spray colours...");
            List<byte> currentPalette = new List<byte>();
            /*WriteToROM(romFile, 4846172, currentPalette[0] >> 32, 20); // Normal palette. We don't want to use the last two colours, since that's for UFO only.
            WriteToROM(romFile, 4849948, currentPalette[0], 24); // UFO palette.
            WriteToROM(romFile, 3343094, currentPalette[1], 12); // HUD palettes (lives + vitality).
            for (int x = 0; x < 13; x++) {
                currentPalette = RandomizePalette();
                WriteToROM(romFile, 4846300 + (x * 32), currentPalette[0] >> 32, 20);
                WriteToROM(romFile, 4850076 + (x * 32), currentPalette[0], 24);
                WriteToROM(romFile, 3343126 + (x * 32), currentPalette[1], 12);
            }*/

            /* Colors are stored in pairs of 3. Every index correlates to their RGB values;
             * Every Kirby palette consists of 11 colors
             */
            for (int i = 0; i < 23; i++) {
                if (i < 2) currentPalette.Add(0);
                else currentPalette.Add(255);
            }

            byte[] bytesPalette = currentPalette.ToArray();

            for (int x = 0; x < 13; x++) {
                if (x == 0) {
                    WriteToROM(romFile, 4846170, bytesPalette);
                    //WriteToROM(romFile, 4847100, bytesPalette);
                }
                
                WriteToROM(romFile, 4846298 + (x * 32), bytesPalette);
            }
        }

        public static byte[] RandomizePalette() {
            List<byte> colourPalettes = new List<byte>();

            return colourPalettes.ToArray();
        }

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