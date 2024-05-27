using KatAMInternal;
using System.Collections.Generic;
using System.Drawing;
using System.Security.Cryptography.Xml;

namespace KatAMRandomizer {
    internal class KatAMSprays {
        static Random random;

        public static void RandomizeSpray(Processing system) {
            byte[] romFile = system.ROMData;
            random = system.Settings.RandomEntity;

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
            /*for (int i = 0; i <= 21; i++) {
                if (i < 2) currentPalette.Add(0);
                else currentPalette.Add(255);
            }
            
            byte[] bytesPalette = currentPalette.ToArray();
             */

            List<byte[]> presetSprays = LoadSprayPresets();

            byte[] kirbyColorPalette = RandomizePalette();

            // Injecting the color palettes in the ROM;
            for (int x = 0; x < 14; x++) {
                int spraySelected = Utils.GetRandomNumber(random, 0, presetSprays.Count);

                if (x < 13) {
                    byte[] kirbySupportPalette = RandomizePalette(); /*presetSprays[spraySelected]*/

                    // Support Kirbys Palettes;
                    Utils.WriteToROM(romFile, 4846298 + (x * 32), kirbySupportPalette);
                    Utils.WriteToROM(romFile, 3343126 + (x * 32), GenerateHUDPalette(kirbySupportPalette)/*presetSprays[spraySelected]*/);
                } else {
                    

                    // Kirby Base Palette;
                    Utils.WriteToROM(romFile, 4846170, kirbyColorPalette);
                    Utils.WriteToROM(romFile, 3343094, GenerateHUDPalette(kirbyColorPalette)); // HUD palettes (lives + vitality).
                    //WriteToROM(romFile, 4847100, bytesPalette); //Title Screen Palette, Supposedly;
                }

                presetSprays.RemoveAt(spraySelected);
            }
        }

        static byte[] GenerateHUDPalette(byte[] colorPalette) {
            int[] colorPaletteReferences = new int[] { 4, 5, 8, 9, 8, 9, 12, 13, 8, 9, 12, 13, 0, 1 };

            List<byte> hudPalette = new List<byte>();

            for (int i = 0; i < colorPaletteReferences.Length; i++) {
                int index = colorPaletteReferences[i];

                hudPalette.Add(colorPalette[index]);
            }

            return hudPalette.ToArray();

        }

        //LoadSprayPresets(); Loading all the spray presets for the system to pick one if needed;
        static List<byte[]> LoadSprayPresets() {
            List<byte[]> presetSprays = new List<byte[]>();

            presetSprays.Add(DefaultPinkKirby());
            presetSprays.Add(DefaultYellowKirby());
            presetSprays.Add(DefaultRedKirby());
            presetSprays.Add(DefaultGreenKirby());
            presetSprays.Add(DefaultSnowKirby());
            presetSprays.Add(DefaultCarbonKirby());
            presetSprays.Add(DefaultOceanKirby());
            presetSprays.Add(DefaultSapphireKirby());
            presetSprays.Add(DefaultGrapeKirby());
            presetSprays.Add(DefaultEmeraldKirby());
            presetSprays.Add(DefaultOrangeKirby());
            presetSprays.Add(DefaultChocolateKirby());
            presetSprays.Add(DefaultCherryKirby());
            presetSprays.Add(DefaultChalkKirby());
            presetSprays.Add(DefaultShadowKirby());
            presetSprays.Add(KDL3Kirby());
            presetSprays.Add(AdvanceIceKirby());
            presetSprays.Add(AdvanceStoneKirby());
            presetSprays.Add(AdvanceMetaKnightKirby());
            presetSprays.Add(OriginalWhiteKirby());

            return presetSprays;
        }

        //RandomizePalette(); Creates a new color palette taking Kirby's base color as a reference;
        static byte[] RandomizePalette() {
            /* Color Index To Kirby Sprite:
             * 0: Outline;
             * 1: Shoe Shine;
             * 2: Body Main Color;
             * 3: Body Soft Shadows;
             * 4: Body Darker Shadows;
             * 5: Body Dark Highlights;
             * 6: Body Darker Highlights;
             * 7: Body Outline Pixel Transitions;
             * 8: Shoes/Cheeks Main Color;
             * 9: Shoes Shadows;
             * 10: Shoes Darkest Shadows;
             * https://aquova.net/kirby-color/
             */

            List<int[]> selectedPalette = pinkKirbyColors;

            // Isolating the body colors;
            List<int[]> bodyColors = selectedPalette.GetRange(0, 8);

            // Creating a HSV tone to color the pixels;
            int bodyH = Utils.GetRandomNumber(random, 0, 360),
            bodyS = 0, //Utils.GetRandomNumber(random, 0, 0),
            bodyV = Utils.GetRandomNumber(random, -10, -10);

            // Coloring the body color palette;
            List<byte> bodyPalette = AdjustHSV(bodyColors, bodyH, bodyS, bodyV, true);

            // Isolating the shoes and cheeks colors;
            List<int[]> shoesAndCheeksColors = selectedPalette.GetRange(8, 3);

            // Creating a HSV tone to color the pixels;
            int shoesH = Utils.GetRandomNumber(random, 0, 360),
            shoesS = 0, //Utils.GetRandomNumber(random, 0, 0),
            shoesV = Utils.GetRandomNumber(random, -30, -10);

            // Coloring the body color palette;
            List<byte> shoesPalette = AdjustHSV(shoesAndCheeksColors, shoesH, shoesS, shoesV);

            // Combining both color palettes
            bodyPalette.AddRange(shoesPalette);

            return bodyPalette.ToArray();
        }

        // Method to adjust HSV values based on random number
        public static List<byte> AdjustHSV(List<int[]> rgbColors, int randomH, int randomS, int randomV, bool hasOutline = false) {
            List<byte> adjustedColors = new List<byte>();

            for (int i = 0; i < rgbColors.Count; i++) {
                int[] color = rgbColors[i];

                if(i == 0 && hasOutline) {
                    color = rgbColors[7];
                }

                int[] hsvColor = RGBToHSV(color[0], color[1], color[2]);

                int h = hsvColor[0] + randomH;
                if (h > 359) h -= 360; // Wrap around if h goes above 360

                int s = hsvColor[1] + randomS;
                if (s > 255) s = 256; // Wrap around if s goes above 255

                int v = hsvColor[2] + randomV;
                if (v > 255) v -= 256; // Wrap around if v goes above 255

                int[] RGBColor = HSVToRGB(new int[] { h, s, v });

                AddColorRGB(adjustedColors, (byte)RGBColor[0], (byte)RGBColor[1], (byte)RGBColor[2]);
            }
            
            return adjustedColors;
        }

        public static int[] RGBToHSV(int r, int g, int b) {
            // Scale RGB values to [0, 1]
            double rd = r / 255.0;
            double gd = g / 255.0;
            double bd = b / 255.0;

            double cmax = Math.Max(rd, Math.Max(gd, bd));
            double cmin = Math.Min(rd, Math.Min(gd, bd));
            double delta = cmax - cmin;

            // Calculate hue
            double hue = 0;
            if (delta != 0) {
                if (cmax == rd)
                    hue = 60 * (((gd - bd) / delta) % 6);
                else if (cmax == gd)
                    hue = 60 * (((bd - rd) / delta) + 2);
                else if (cmax == bd)
                    hue = 60 * (((rd - gd) / delta) + 4);
            }

            // Calculate saturation
            double saturation = (cmax == 0) ? 0 : (delta / cmax);

            // Calculate value
            double value = cmax;

            // Scale hue to [0, 360]
            hue = (hue < 0) ? hue + 360 : hue;

            // Scale saturation and value to [0, 255]
            int hueInt = (int)Math.Round(hue);
            int saturationInt = (int)Math.Round(saturation * 255);
            int valueInt = (int)Math.Round(value * 255);

            return new int[] { hueInt, saturationInt, valueInt };
        }

        public static int[] HSVToRGB(int[] colorInput) {
            int hue = colorInput[0], saturation = colorInput[1], value = colorInput[2];

            double h = hue / 360.0;       // Scale hue to [0, 1]
            double s = saturation / 255.0; // Scale saturation to [0, 1]
            double v = value / 255.0;      // Scale value to [0, 1]

            double r = 0, g = 0, b = 0;

            int i = (int)(h * 6);
            double f = (h * 6) - i;
            double p = v * (1 - s);
            double q = v * (1 - f * s);
            double t = v * (1 - (1 - f) * s);

            switch (i % 6) {
                case 0:
                    r = v;
                    g = t;
                    b = p;
                    break;
                case 1:
                    r = q;
                    g = v;
                    b = p;
                    break;
                case 2:
                    r = p;
                    g = v;
                    b = t;
                    break;
                case 3:
                    r = p;
                    g = q;
                    b = v;
                    break;
                case 4:
                    r = t;
                    g = p;
                    b = v;
                    break;
                case 5:
                    r = v;
                    g = p;
                    b = q;
                    break;
            }

            // Convert to integer values in range [0, 255]
            int red = (int)(r * 255);
            int green = (int)(g * 255);
            int blue = (int)(b * 255);

            return new int[] { red, green, blue };
        }


        /////////////////////////////////////////////////////////////////////////////////////////////////////


        // AddColorRGB(); Converting a RGB color to a GBA color, 0 min - 255 max;
        static void AddColorRGB(List<byte> reference, byte red, byte green, byte blue) {
            byte[] gbaColor = ConvertRGBToGBAColorBytes(red, green, blue);

            reference.Add(gbaColor[0]);
            reference.Add(gbaColor[1]);
        }
        
        public static byte[] ConvertRGBToGBAColorBytes(byte red, byte green, byte blue) {
            ushort gbaColor = ConvertRGBToGBAColor(red, green, blue);

            // Convert the 16-bit integer to a byte array (little-endian)
            byte[] colorBytes = new byte[2];
            colorBytes[0] = (byte)(gbaColor & 0xFF);      // LSB
            colorBytes[1] = (byte)((gbaColor >> 8) & 0xFF); // MSB

            return colorBytes;
        }
        
        public static ushort ConvertRGBToGBAColor(byte red, byte green, byte blue) {
            // Convert RGB to GBA color (15-bit RGB format)
            ushort gbaColor = (ushort)(((red >> 3) & 31) | (((green >> 3) & 31) << 5) | (((blue >> 3) & 31) << 10));
            return gbaColor;
        }


        /////////////////////////////////////////////////////////////////////////////////////////////////////

        // Preset Colors;

        static List<int[]> pinkKirbyColors = new List<int[]>(){
            new int[] { 0, 0, 0 },
            new int[] { 255, 211, 247 },
            new int[] { 255, 162, 222 },
            new int[] { 255, 146, 198 },
            new int[] { 247, 113, 165 },
            new int[] { 222, 73, 115 },
            new int[] { 181, 32, 57 },
            new int[] { 99, 16, 16 },
            new int[] { 255, 24, 132 },
            new int[] { 214, 0, 82 },
            new int[] { 181, 0, 41 }
        };

        static void ConvertColorPalette(List<byte> destination, List<int[]> colorReference) {
            foreach (int[] color in colorReference) {
                AddColorRGB(destination, (byte)color[0], (byte)color[1], (byte)color[2]);
            }
        }

        static byte[] DefaultPinkKirby() {
            List<byte> result = new List<byte>();

            ConvertColorPalette(result, pinkKirbyColors);

            return result.ToArray();
        }

        static List<int[]> yellowKirbyColors = new List<int[]>(){
            new int[] { 0, 0, 0 },
            new int[] { 255, 251, 0 },
            new int[] { 255, 211, 0 },
            new int[] { 255, 178, 0 },
            new int[] { 231, 170, 0 },
            new int[] { 173, 121, 0 },
            new int[] { 132, 97, 0 },
            new int[] { 90, 73, 0 },
            new int[] { 255, 24, 0 },
            new int[] { 222, 16, 0 },
            new int[] { 173, 0, 0 }
        };

        static byte[] DefaultYellowKirby() {
            List<byte> result = new List<byte>();

            ConvertColorPalette(result, yellowKirbyColors);

            return result.ToArray();
        }

        static List<int[]> redKirbyColors = new List<int[]>(){
            new int[] { 0, 0, 0 },
            new int[] { 255, 162, 165 },
            new int[] { 255, 0, 57 },
            new int[] { 222, 0, 41 },
            new int[] { 181, 8, 24 },
            new int[] { 181, 8, 24 },
            new int[] { 107, 8, 8 },
            new int[] { 74, 0, 0 },
            new int[] { 255, 0, 165 },
            new int[] { 222, 0, 107 },
            new int[] { 181, 0, 41 }
        };

        static byte[] DefaultRedKirby() {
            List<byte> result = new List<byte>();

            ConvertColorPalette(result, redKirbyColors);

            return result.ToArray();
        }

        static List<int[]> greenKirbyColors = new List<int[]>(){
            new int[] { 0, 0, 0 },
            new int[] { 198, 251, 156 },
            new int[] { 123, 251, 41 },
            new int[] { 115, 235, 24 },
            new int[] { 99, 211, 0 },
            new int[] { 57, 154, 0 },
            new int[] { 41, 121, 0 },
            new int[] { 16, 89, 0 },
            new int[] { 222, 113, 0 },
            new int[] { 206, 73, 0 },
            new int[] { 140, 56, 0 }
        };

        static byte[] DefaultGreenKirby() {
            List<byte> result = new List<byte>();

            ConvertColorPalette(result, greenKirbyColors);

            return result.ToArray();
        }

        static List<int[]> defaultSnowKirbyColors = new List<int[]>(){
            new int[] { 0, 0, 0 },
            new int[] { 255, 251, 255 },
            new int[] { 255, 251, 255 },
            new int[] { 222, 219, 222 },
            new int[] { 189, 186, 189 },
            new int[] { 148, 146, 148 },
            new int[] { 115, 113, 115 },
            new int[] { 82, 81, 82 },
            new int[] { 255, 73, 99 },
            new int[] { 214, 32, 66 },
            new int[] { 181, 0, 41 }
        };

        static byte[] DefaultSnowKirby() {
            List<byte> reference = new List<byte>();

            ConvertColorPalette(reference, defaultSnowKirbyColors);

            return reference.ToArray();
        }

        static List<int[]> defaultCarbonKirbyColors = new List<int[]>(){
            new int[] { 0, 0, 0 },
            new int[] { 132, 130, 132 },
            new int[] { 115, 113, 115 },
            new int[] { 99, 97, 99 },
            new int[] { 82, 81, 82 },
            new int[] { 66, 65, 66 },
            new int[] { 57, 56, 57 },
            new int[] { 41, 40, 41 },
            new int[] { 255, 154, 8 },
            new int[] { 255, 121, 0 },
            new int[] { 206, 81, 0 }
        };

        static byte[] DefaultCarbonKirby() {
            List<byte> reference = new List<byte>();

            ConvertColorPalette(reference, defaultCarbonKirbyColors);

            return reference.ToArray();
        }

        static List<int[]> oceanKirbyColors = new List<int[]>(){
            new int[] { 0, 0, 0 },
            new int[] { 165, 251, 255 },
            new int[] { 140, 219, 255 },
            new int[] { 107, 186, 255 },
            new int[] { 82, 154, 255 },
            new int[] { 41, 121, 255 },
            new int[] { 16, 97, 198 },
            new int[] { 41, 48, 123 },
            new int[] { 99, 89, 255 },
            new int[] { 74, 65, 222 },
            new int[] { 49, 24, 181 }
        };

        static byte[] DefaultOceanKirby() {
            List<byte> reference = new List<byte>();

            ConvertColorPalette(reference, oceanKirbyColors);

            return reference.ToArray();
        }

        static List<int[]> sapphireKirbyColors = new List<int[]>(){
            new int[] { 0, 0, 0 },
            new int[] { 181, 203, 255 },
            new int[] { 123, 154, 239 },
            new int[] { 99, 130, 222 },
            new int[] { 82, 113, 206 },
            new int[] { 0, 81, 181 },
            new int[] { 0, 56, 156 },
            new int[] { 0, 32, 123 },
            new int[] { 82, 0, 189 },
            new int[] { 66, 0, 148 },
            new int[] { 57, 0, 107 }
        };

        static byte[] DefaultSapphireKirby() {
            List<byte> reference = new List<byte>();

            ConvertColorPalette(reference, sapphireKirbyColors);

            return reference.ToArray();
        }

        static List<int[]> grapeKirbyColors = new List<int[]>(){
            new int[] { 0, 0, 0 },
            new int[] { 206, 186, 255 },
            new int[] { 181, 154, 255 },
            new int[] { 165, 113, 255 },
            new int[] { 140, 81, 255 },
            new int[] { 115, 8, 214 },
            new int[] { 99, 40, 156 },
            new int[] { 74, 32, 107 },
            new int[] { 181, 65, 132 },
            new int[] { 148, 32, 107 },
            new int[] { 123, 0, 82 }
        };

        static byte[] DefaultGrapeKirby() {
            List<byte> reference = new List<byte>();

            ConvertColorPalette(reference, grapeKirbyColors);

            return reference.ToArray();
        }

        static List<int[]> emeraldKirbyColors = new List<int[]>(){
            new int[] { 0, 0, 0 },
            new int[] { 189, 251, 222 },
            new int[] { 115, 251, 189 },
            new int[] { 57, 211, 148 },
            new int[] { 0, 162, 107 },
            new int[] { 0, 130, 99 },
            new int[] { 0, 105, 82 },
            new int[] { 0, 81, 57 },
            new int[] { 255, 162, 74 },
            new int[] { 222, 121, 33 },
            new int[] { 189, 89, 0 }
        };

        static byte[] DefaultEmeraldKirby() {
            List<byte> reference = new List<byte>();

            ConvertColorPalette(reference, emeraldKirbyColors);

            return reference.ToArray();
        }

        static List<int[]> orangeKirbyColors = new List<int[]>(){
            new int[] { 0, 0, 0 },
            new int[] { 255, 235, 0 },
            new int[] { 255, 178, 24 },
            new int[] { 255, 154, 24 },
            new int[] { 255, 121, 33 },
            new int[] { 255, 89, 41 },
            new int[] { 198, 65, 24 },
            new int[] { 132, 40, 16 },
            new int[] { 255, 73, 0 },
            new int[] { 198, 48, 0 },
            new int[] { 132, 40, 16 }
        };

        static byte[] DefaultOrangeKirby() {
            List<byte> reference = new List<byte>();

            ConvertColorPalette(reference, orangeKirbyColors);

            return reference.ToArray();
        }

        static List<int[]> chocolateKirbyColors = new List<int[]>() {
            new int[] { 0, 0, 0 },
            new int[] { 214, 154, 140 },
            new int[] { 214, 130, 99 },
            new int[] { 189, 105, 82 },
            new int[] { 156, 81, 66 },
            new int[] { 132, 56, 49 },
            new int[] { 107, 40, 33 },
            new int[] { 82, 24, 24 },
            new int[] { 156, 32, 24 },
            new int[] { 132, 16, 8 },
            new int[] { 115, 0, 0 }
        };

        static byte[] DefaultChocolateKirby() {
            List<byte> reference = new List<byte>();

            ConvertColorPalette(reference, chocolateKirbyColors);

            return reference.ToArray();
        }

        static List<int[]> cherryKirbyColors = new List<int[]>(){
            new int[] { 0, 0, 0 },
            new int[] { 255, 211, 247 },
            new int[] { 255, 146, 222 },
            new int[] { 255, 146, 198 },
            new int[] { 247, 113, 165 },
            new int[] { 222, 73, 115 },
            new int[] { 181, 32, 57 },
            new int[] { 99, 16, 16 },
            new int[] { 24, 162, 90 },
            new int[] { 49, 130, 82 },
            new int[] { 41, 105, 57 }
        };

        static byte[] DefaultCherryKirby() {
            List<byte> reference = new List<byte>();

            ConvertColorPalette(reference, cherryKirbyColors);

            return reference.ToArray();
        }

        static List<int[]> defaultChalkKirbyColors = new List<int[]>(){
            new int[] { 0, 0, 0 },
            new int[] { 255, 251, 255 },
            new int[] { 222, 219, 222 },
            new int[] { 189, 186, 189 },
            new int[] { 156, 154, 156 },
            new int[] { 115, 113, 115 },
            new int[] { 82, 81, 82 },
            new int[] { 49, 48, 49 },
            new int[] { 123, 121, 123 },
            new int[] { 90, 89, 90 },
            new int[] { 66, 65, 66 }
        };

        static byte[] DefaultChalkKirby() {
            List<byte> reference = new List<byte>();

            ConvertColorPalette(reference, defaultChalkKirbyColors);

            return reference.ToArray();
        }

        static List<int[]> defaultShadowKirbyColors = new List<int[]>(){
            new int[] { 0, 0, 0 },
            new int[] { 140, 138, 140 },
            new int[] { 132, 130, 132 },
            new int[] { 115, 113, 115 },
            new int[] { 99, 97, 99 },
            new int[] { 82, 81, 82 },
            new int[] { 66, 65, 66 },
            new int[] { 49, 48, 49 },
            new int[] { 82, 81, 82 },
            new int[] { 57, 56, 57 },
            new int[] { 41, 40, 41 }
        };

        static byte[] DefaultShadowKirby() {
            List<byte> reference = new List<byte>();

            ConvertColorPalette(reference, defaultShadowKirbyColors);

            return reference.ToArray();
        }

        static List<int[]> KDL3KirbyColors = new List<int[]>(){
            new int[] { 168, 80, 72 },
            new int[] { 232, 120, 128 },
            new int[] { 240, 224, 232 },
            new int[] { 232, 208, 208 },
            new int[] { 240, 160, 184 },
            new int[] { 232, 120, 128 },
            new int[] { 208, 120, 128 },
            new int[] { 168, 112, 112 },
            new int[] { 232, 80, 72 },
            new int[] { 224, 32, 24 },
            new int[] { 176, 24, 16 }
        };

        static byte[] KDL3Kirby() {
            List<byte> reference = new List<byte>();

            ConvertColorPalette(reference, KDL3KirbyColors);

            return reference.ToArray();
        }

        static List<int[]> advanceIceKirbyColors = new List<int[]>(){
            new int[] { 0, 0, 0 },
            new int[] { 72, 208, 248 },
            new int[] { 144, 240, 248 },
            new int[] { 72, 208, 248 },
            new int[] { 0, 160, 248 },
            new int[] { 88, 48, 248 },
            new int[] { 80, 0, 48 },
            new int[] { 48, 48, 48 },
            new int[] { 120, 0, 136 },
            new int[] { 104, 0, 88 },
            new int[] { 80, 0, 48 }
        };

        static byte[] AdvanceIceKirby() {
            List<byte> reference = new List<byte>();

            ConvertColorPalette(reference, advanceIceKirbyColors);

            return reference.ToArray();
        }

        static List<int[]> advanceStoneKirbyColors = new List<int[]>(){
            new int[] { 0, 0, 0 },
            new int[] { 239, 214, 198 },
            new int[] { 239, 214, 198 },
            new int[] { 204, 153, 153 },
            new int[] { 204, 153, 153 },
            new int[] { 153, 102, 102 },
            new int[] { 102, 51, 51 },
            new int[] { 57, 57, 57 },
            new int[] { 255, 80, 80 },
            new int[] { 204, 51, 51 },
            new int[] { 153, 51, 51 }
        };

        static byte[] AdvanceStoneKirby() {
            List<byte> reference = new List<byte>();

            ConvertColorPalette(reference, advanceStoneKirbyColors);

            return reference.ToArray();
        }

        static List<int[]> advanceMetaKnightKirbyColors = new List<int[]>(){
            new int[] { 0, 0, 0 },
            new int[] { 248, 248, 248 },
            new int[] { 0, 0, 248 },
            new int[] { 0, 0, 248 },
            new int[] { 32, 0, 176 },
            new int[] { 32, 0, 176 },
            new int[] { 64, 0, 88 },
            new int[] { 64, 0, 88 },
            new int[] { 232, 0, 152 },
            new int[] { 192, 0, 144 },
            new int[] { 152, 0, 120 }
        };

        static byte[] AdvanceMetaKnightKirby() {
            List<byte> reference = new List<byte>();

            ConvertColorPalette(reference, advanceMetaKnightKirbyColors);

            return reference.ToArray();
        }

        static List<int[]> originalWhiteKirbyColors = new List<int[]>(){
            new int[] { 0, 0, 0 },
            new int[] { 255, 251, 255 },
            new int[] { 255, 251, 255 },
            new int[] { 222, 219, 222 },
            new int[] { 189, 186, 189 },
            new int[] { 148, 146, 148 },
            new int[] { 115, 113, 115 },
            new int[] { 82, 81, 82 },
            new int[] { 222, 219, 222 },
            new int[] { 189, 186, 189 },
            new int[] { 148, 146, 148 }
        };

        static byte[] OriginalWhiteKirby() {
            List<byte> reference = new List<byte>();

            ConvertColorPalette(reference, originalWhiteKirbyColors);

            return reference.ToArray();
        }
    }
}