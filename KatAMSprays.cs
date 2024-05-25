using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Intrinsics.Arm;
using System.Security.Cryptography.Xml;
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
            for (int i = 0; i <= 21; i++) {
                if (i < 2) currentPalette.Add(0);
                else currentPalette.Add(255);
            }

            byte[] bytesPalette = currentPalette.ToArray();

            for (int x = 0; x < 4; x++) {
                if (x == 0) {
                    // Kirby Base Palette;
                    Utils.WriteToROM(romFile, 4846170, DefaultPinkKirby());
                    //WriteToROM(romFile, 4847100, bytesPalette); //Title Screen Palette, Supposedly;
                } else {
                    // Support Kirbys Palettes;
                    Utils.WriteToROM(romFile, 4846298 + (x * 32), bytesPalette);
                }
            }
        }

        public static byte[] RandomizePalette() {
            /* Color Index To Kirby Sprite:
             * 0: Outline;
             * 1: Shoe Shine;
             * 2: Main Body Color;
             * 3: Body Soft Shadows;
             * 4: Body Darker Shadows;
             * 5: Body Dark Highlights;
             * 6: Body Darker Highlights;
             * 7: Outline Pixel Transitions;
             * 8: Cheeks and Shoes Main Color;
             * 9: Shoes Shadows;
             * 10: Shoes Darkest Shadows;
             * https://aquova.net/kirby-color/
             */
            List<byte> colourPalettes = new List<byte>();

            AddColorRGB(colourPalettes, 0, 0, 0);
            AddColorRGB(colourPalettes, 0, 0, 0);
            AddColorRGB(colourPalettes, 0, 0, 0);
            AddColorRGB(colourPalettes, 0, 0, 0);
            AddColorRGB(colourPalettes, 0, 0, 0);
            AddColorRGB(colourPalettes, 0, 0, 0);
            AddColorRGB(colourPalettes, 0, 0, 0);
            AddColorRGB(colourPalettes, 0, 0, 0);
            AddColorRGB(colourPalettes, 0, 0, 0);
            AddColorRGB(colourPalettes, 0, 0, 0);
            AddColorRGB(colourPalettes, 0, 0, 0);

            return colourPalettes.ToArray();
        }

        // InsertColorRGB(); RGB colors, 0 min - 255 max;
        static void AddColorRGB(List<byte> reference, byte red, byte green, byte blue) {
            byte[] gbaColor = ConvertRgbToGbaColorBytes(red, green, blue);

            reference.Add(gbaColor[0]);
            reference.Add(gbaColor[1]);
        }

        public static ushort ConvertRgbToGbaColor(byte red, byte green, byte blue) {
            // Convert RGB to GBA color (15-bit RGB format)
            ushort gbaColor = (ushort)(((red >> 3) & 31) | (((green >> 3) & 31) << 5) | (((blue >> 3) & 31) << 10));
            return gbaColor;
        }

        public static byte[] ConvertRgbToGbaColorBytes(byte red, byte green, byte blue) {
            ushort gbaColor = ConvertRgbToGbaColor(red, green, blue);

            // Convert the 16-bit integer to a byte array (little-endian)
            byte[] colorBytes = new byte[2];
            colorBytes[0] = (byte)(gbaColor & 0xFF);      // LSB
            colorBytes[1] = (byte)((gbaColor >> 8) & 0xFF); // MSB

            return colorBytes;
        }

        /////////////////////////////////////////////////////////////////////////////////////////////////////
        
        // Preset Colors;

        static byte[] DefaultPinkKirby() {
            List<byte> reference = new List<byte>();

            AddColorRGB(reference, 0, 0, 0);
            AddColorRGB(reference, 255, 211, 247);
            AddColorRGB(reference, 255, 162, 222);
            AddColorRGB(reference, 255, 146, 198);
            AddColorRGB(reference, 247, 113, 165);
            AddColorRGB(reference, 222, 73, 115);
            AddColorRGB(reference, 181, 32, 57);
            AddColorRGB(reference, 99, 16, 16);            
            AddColorRGB(reference, 255, 24, 132);
            AddColorRGB(reference, 214, 0, 82);
            AddColorRGB(reference, 181, 0, 41);

            return reference.ToArray();
        }

        
    }
}