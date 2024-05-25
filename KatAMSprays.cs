using KatAMInternal;

namespace KatAMRandomizer {
    internal class KatAMSprays {
        public static void RandomizeSpray(Processing system) {
            byte[] romFile = system.ROMData;

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
            }*/

            List<byte[]> presetSprays = LoadSprayPresets();
            //byte[] bytesPalette = currentPalette.ToArray();

            for (int x = 0; x < 4; x++) {
                int spraySelected = Utils.GetRandomNumber(system.Settings, 0, presetSprays.Count);

                if(x < 3) {
                    // Support Kirbys Palettes;
                    Utils.WriteToROM(romFile, 4846298 + (x * 32), presetSprays[spraySelected]);
                } else {
                    // Kirby Base Palette;
                    Utils.WriteToROM(romFile, 4846170, presetSprays[spraySelected]);
                    //WriteToROM(romFile, 4847100, bytesPalette); //Title Screen Palette, Supposedly;
                }

                presetSprays.RemoveAt(spraySelected);
            }
        }

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

            AddColorRGB(colourPalettes, 0, 0, 0); //0;
            AddColorRGB(colourPalettes, 0, 0, 0); //1;
            AddColorRGB(colourPalettes, 0, 0, 0); //2;
            AddColorRGB(colourPalettes, 0, 0, 0); //3;
            AddColorRGB(colourPalettes, 0, 0, 0); //4;
            AddColorRGB(colourPalettes, 0, 0, 0); //5;
            AddColorRGB(colourPalettes, 0, 0, 0); //6;
            AddColorRGB(colourPalettes, 0, 0, 0); //7;
            AddColorRGB(colourPalettes, 0, 0, 0); //8;
            AddColorRGB(colourPalettes, 0, 0, 0); //9;
            AddColorRGB(colourPalettes, 0, 0, 0); //10;

            return colourPalettes.ToArray();
        }

        // InsertColorRGB(); RGB colors, 0 min - 255 max;
        static void AddColorRGB(List<byte> reference, byte red, byte green, byte blue) {
            byte[] gbaColor = ConvertRgbToGbaColorBytes(red, green, blue);

            reference.Add(gbaColor[0]);
            reference.Add(gbaColor[1]);
        }

        public static ushort ConvertRGBToGBAColor(byte red, byte green, byte blue) {
            // Convert RGB to GBA color (15-bit RGB format)
            ushort gbaColor = (ushort)(((red >> 3) & 31) | (((green >> 3) & 31) << 5) | (((blue >> 3) & 31) << 10));
            return gbaColor;
        }

        public static byte[] ConvertRgbToGbaColorBytes(byte red, byte green, byte blue) {
            ushort gbaColor = ConvertRGBToGBAColor(red, green, blue);

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

        static byte[] DefaultYellowKirby() {
            List<byte> reference = new List<byte>();

            AddColorRGB(reference, 0, 0, 0);
            AddColorRGB(reference, 255, 251, 0);
            AddColorRGB(reference, 255, 211, 0);
            AddColorRGB(reference, 255, 178, 0);
            AddColorRGB(reference, 231, 170, 0);
            AddColorRGB(reference, 173, 121, 0);
            AddColorRGB(reference, 132, 97, 0);
            AddColorRGB(reference, 90, 73, 0);
            AddColorRGB(reference, 255, 24, 0);
            AddColorRGB(reference, 222, 16, 0);
            AddColorRGB(reference, 173, 0, 0);

            return reference.ToArray();
        }

        static byte[] DefaultRedKirby() {
            List<byte> reference = new List<byte>();

            AddColorRGB(reference, 0, 0, 0);
            AddColorRGB(reference, 255, 162, 165);
            AddColorRGB(reference, 255, 0, 57);
            AddColorRGB(reference, 222, 0, 41);
            AddColorRGB(reference, 181, 8, 24);
            AddColorRGB(reference, 181, 8, 24);
            AddColorRGB(reference, 107, 8, 8);
            AddColorRGB(reference, 74, 0, 0);
            AddColorRGB(reference, 255, 0, 165);
            AddColorRGB(reference, 222, 0, 107);
            AddColorRGB(reference, 181, 0, 41);

            return reference.ToArray();
        }

        static byte[] DefaultGreenKirby() {
            List<byte> reference = new List<byte>();

            AddColorRGB(reference, 0, 0, 0);
            AddColorRGB(reference, 198, 251, 156);
            AddColorRGB(reference, 123, 251, 41);
            AddColorRGB(reference, 115, 235, 24);
            AddColorRGB(reference, 99, 211, 0);
            AddColorRGB(reference, 57, 154, 0);
            AddColorRGB(reference, 41, 121, 0);
            AddColorRGB(reference, 16, 89, 0);
            AddColorRGB(reference, 222, 113, 0);
            AddColorRGB(reference, 206, 73, 0);
            AddColorRGB(reference, 140, 56, 0);

            return reference.ToArray();
        }

        static byte[] DefaultSnowKirby() {
            List<byte> reference = new List<byte>();

            AddColorRGB(reference, 0, 0, 0);
            AddColorRGB(reference, 255, 251, 255);
            AddColorRGB(reference, 255, 251, 255);
            AddColorRGB(reference, 222, 219, 222);
            AddColorRGB(reference, 189, 186, 189);
            AddColorRGB(reference, 148, 146, 148);
            AddColorRGB(reference, 115, 113, 115);
            AddColorRGB(reference, 82, 81, 82);
            AddColorRGB(reference, 255, 73, 99);
            AddColorRGB(reference, 214, 32, 66);
            AddColorRGB(reference, 181, 0, 41);

            return reference.ToArray();
        }

        static byte[] DefaultCarbonKirby() {
            List<byte> reference = new List<byte>();

            AddColorRGB(reference, 0, 0, 0);
            AddColorRGB(reference, 132, 130, 132);
            AddColorRGB(reference, 115, 113, 115);
            AddColorRGB(reference, 99, 97, 99);
            AddColorRGB(reference, 82, 81, 82);
            AddColorRGB(reference, 66, 65, 66);
            AddColorRGB(reference, 57, 56, 57);
            AddColorRGB(reference, 41, 40, 41);
            AddColorRGB(reference, 255, 154, 8);
            AddColorRGB(reference, 255, 121, 0);
            AddColorRGB(reference, 206, 81, 0);

            return reference.ToArray();
        }

        static byte[] DefaultOceanKirby() {
            List<byte> reference = new List<byte>();

            AddColorRGB(reference, 0, 0, 0);
            AddColorRGB(reference, 165, 251, 255);
            AddColorRGB(reference, 140, 219, 255);
            AddColorRGB(reference, 107, 186, 255);
            AddColorRGB(reference, 82, 154, 255);
            AddColorRGB(reference, 41, 121, 255);
            AddColorRGB(reference, 16, 97, 198);
            AddColorRGB(reference, 41, 48, 123);
            AddColorRGB(reference, 99, 89, 255);
            AddColorRGB(reference, 74, 65, 222);
            AddColorRGB(reference, 49, 24, 181);

            return reference.ToArray();
        }

        static byte[] DefaultSapphireKirby() {
            List<byte> reference = new List<byte>();

            AddColorRGB(reference, 0, 0, 0);
            AddColorRGB(reference, 181, 203, 255);
            AddColorRGB(reference, 123, 154, 239);
            AddColorRGB(reference, 99, 130, 222);
            AddColorRGB(reference, 82, 113, 206);
            AddColorRGB(reference, 0, 81, 181);
            AddColorRGB(reference, 0, 56, 156);
            AddColorRGB(reference, 0, 32, 123);
            AddColorRGB(reference, 82, 0, 189);
            AddColorRGB(reference, 66, 0, 148);
            AddColorRGB(reference, 57, 0, 107);

            return reference.ToArray();
        }

        static byte[] DefaultGrapeKirby() {
            List<byte> reference = new List<byte>();

            AddColorRGB(reference, 0, 0, 0);
            AddColorRGB(reference, 206, 186, 255);
            AddColorRGB(reference, 181, 154, 255);
            AddColorRGB(reference, 165, 113, 255);
            AddColorRGB(reference, 140, 81, 255);
            AddColorRGB(reference, 115, 8, 214);
            AddColorRGB(reference, 99, 40, 156);
            AddColorRGB(reference, 74, 32, 107);
            AddColorRGB(reference, 181, 65, 132);
            AddColorRGB(reference, 148, 32, 107);
            AddColorRGB(reference, 123, 0, 82);

            return reference.ToArray();
        }

        static byte[] DefaultEmeraldKirby() {
            List<byte> reference = new List<byte>();

            AddColorRGB(reference, 0, 0, 0);
            AddColorRGB(reference, 189, 251, 222);
            AddColorRGB(reference, 115, 251, 189);
            AddColorRGB(reference, 57, 211, 148);
            AddColorRGB(reference, 0, 162, 107);
            AddColorRGB(reference, 0, 130, 99);
            AddColorRGB(reference, 0, 105, 82);
            AddColorRGB(reference, 0, 81, 57);
            AddColorRGB(reference, 255, 162, 74);
            AddColorRGB(reference, 222, 121, 33);
            AddColorRGB(reference, 189, 89, 0);

            return reference.ToArray();
        }

        static byte[] DefaultOrangeKirby() {
            List<byte> reference = new List<byte>();

            AddColorRGB(reference, 0, 0, 0);
            AddColorRGB(reference, 255, 235, 0);
            AddColorRGB(reference, 255, 178, 24);
            AddColorRGB(reference, 255, 154, 24);
            AddColorRGB(reference, 255, 121, 33);
            AddColorRGB(reference, 255, 89, 41);
            AddColorRGB(reference, 198, 65, 24);
            AddColorRGB(reference, 132, 40, 16);
            AddColorRGB(reference, 255, 73, 0);
            AddColorRGB(reference, 198, 48, 0);
            AddColorRGB(reference, 132, 40, 16);

            return reference.ToArray();
        }

        static byte[] DefaultChocolateKirby() {
            List<byte> reference = new List<byte>();

            AddColorRGB(reference, 0, 0, 0);
            AddColorRGB(reference, 214, 154, 140);
            AddColorRGB(reference, 214, 130, 99);
            AddColorRGB(reference, 189, 105, 82);
            AddColorRGB(reference, 156, 81, 66);
            AddColorRGB(reference, 132, 56, 49);
            AddColorRGB(reference, 107, 40, 33);
            AddColorRGB(reference, 82, 24, 24);
            AddColorRGB(reference, 156, 32, 24);
            AddColorRGB(reference, 132, 16, 8);
            AddColorRGB(reference, 115, 0, 0);

            return reference.ToArray();
        }

        static byte[] DefaultCherryKirby() {
            List<byte> reference = new List<byte>();

            AddColorRGB(reference, 0, 0, 0);
            AddColorRGB(reference, 255, 211, 247);
            AddColorRGB(reference, 255, 146, 222);
            AddColorRGB(reference, 255, 146, 198);
            AddColorRGB(reference, 247, 113, 165);
            AddColorRGB(reference, 222, 73, 115);
            AddColorRGB(reference, 181, 32, 57);
            AddColorRGB(reference, 99, 16, 16);
            AddColorRGB(reference, 24, 162, 90);
            AddColorRGB(reference, 49, 130, 82);
            AddColorRGB(reference, 41, 105, 57);

            return reference.ToArray();
        }

        static byte[] DefaultChalkKirby() {
            List<byte> reference = new List<byte>();

            AddColorRGB(reference, 0, 0, 0);
            AddColorRGB(reference, 255, 251, 255);
            AddColorRGB(reference, 222, 219, 222);
            AddColorRGB(reference, 189, 186, 189);
            AddColorRGB(reference, 156, 154, 156);
            AddColorRGB(reference, 115, 113, 115);
            AddColorRGB(reference, 82, 81, 82);
            AddColorRGB(reference, 49, 48, 49);
            AddColorRGB(reference, 123, 121, 123);
            AddColorRGB(reference, 90, 89, 90);
            AddColorRGB(reference, 66, 65, 66);

            return reference.ToArray();
        }

        static byte[] DefaultShadowKirby() {
            List<byte> reference = new List<byte>();

            AddColorRGB(reference, 0, 0, 0);
            AddColorRGB(reference, 140, 138, 140);
            AddColorRGB(reference, 132, 130, 132);
            AddColorRGB(reference, 115, 113, 115);
            AddColorRGB(reference, 99, 97, 99);
            AddColorRGB(reference, 82, 81, 82);
            AddColorRGB(reference, 66, 65, 66);
            AddColorRGB(reference, 49, 48, 49);
            AddColorRGB(reference, 82, 81, 82);
            AddColorRGB(reference, 57, 56, 57);
            AddColorRGB(reference, 41, 40, 41);

            return reference.ToArray();
        }

        static byte[] KDL3Kirby() {
            List<byte> reference = new List<byte>();

            AddColorRGB(reference, 168, 80, 72);
            AddColorRGB(reference, 232, 120, 128);
            AddColorRGB(reference, 240, 224, 232);
            AddColorRGB(reference, 232, 208, 208);
            AddColorRGB(reference, 240, 160, 184);
            AddColorRGB(reference, 232, 120, 128);
            AddColorRGB(reference, 208, 120, 128);
            AddColorRGB(reference, 168, 112, 112);
            AddColorRGB(reference, 232, 80, 72);
            AddColorRGB(reference, 224, 32, 24);
            AddColorRGB(reference, 176, 24, 16);

            return reference.ToArray();
        }

        static byte[] AdvanceIceKirby() {
            List<byte> reference = new List<byte>();

            AddColorRGB(reference, 0, 0, 0);
            AddColorRGB(reference, 72, 208, 248);
            AddColorRGB(reference, 144, 240, 248);
            AddColorRGB(reference, 72, 208, 248);
            AddColorRGB(reference, 0, 160, 248);
            AddColorRGB(reference, 88, 48, 248);
            AddColorRGB(reference, 80, 0, 48);
            AddColorRGB(reference, 48, 48, 48);
            AddColorRGB(reference, 120, 0, 136);
            AddColorRGB(reference, 104, 0, 88);
            AddColorRGB(reference, 80, 0, 48);

            return reference.ToArray();
        }

        static byte[] AdvanceStoneKirby() {
            List<byte> reference = new List<byte>();

            AddColorRGB(reference, 0, 0, 0);
            AddColorRGB(reference, 239, 214, 198);
            AddColorRGB(reference, 239, 214, 198);
            AddColorRGB(reference, 204, 153, 153);
            AddColorRGB(reference, 204, 153, 153);
            AddColorRGB(reference, 153, 102, 102);
            AddColorRGB(reference, 102, 51, 51);
            AddColorRGB(reference, 57, 57, 57);
            AddColorRGB(reference, 255, 80, 80);
            AddColorRGB(reference, 204, 51, 51);
            AddColorRGB(reference, 153, 51, 51);

            return reference.ToArray();
        }

        static byte[] AdvanceMetaKnightKirby() {
            List<byte> reference = new List<byte>();

            AddColorRGB(reference, 0, 0, 0);
            AddColorRGB(reference, 248, 248, 248);
            AddColorRGB(reference, 0, 0, 248);
            AddColorRGB(reference, 0, 0, 248);
            AddColorRGB(reference, 32, 0, 176);
            AddColorRGB(reference, 32, 0, 176);
            AddColorRGB(reference, 64, 0, 88);
            AddColorRGB(reference, 64, 0, 88);
            AddColorRGB(reference, 232, 0, 152);
            AddColorRGB(reference, 192, 0, 144);
            AddColorRGB(reference, 152, 0, 120);

            return reference.ToArray();
        }

        static byte[] OriginalWhiteKirby() {
            List<byte> reference = new List<byte>();

            AddColorRGB(reference, 0, 0, 0);
            AddColorRGB(reference, 255, 251, 255);
            AddColorRGB(reference, 255, 251, 255);
            AddColorRGB(reference, 222, 219, 222);
            AddColorRGB(reference, 189, 186, 189);
            AddColorRGB(reference, 148, 146, 148);
            AddColorRGB(reference, 115, 113, 115);
            AddColorRGB(reference, 82, 81, 82);
            AddColorRGB(reference, 222, 219, 222);
            AddColorRGB(reference, 189, 186, 189);
            AddColorRGB(reference, 148, 146, 148);

            return reference.ToArray();
        }
    }
}