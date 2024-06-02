﻿using KatAMInternal;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using KatAMInternal;
using System.Reflection;

namespace KatAM_Randomizer {
    internal class KatAMROMReader {
        static Settings settings;
        static int seed;

        // By Fyyyyik; https://github.com/Fyyyyik/KatAM-Object-Editor/blob/main/Editor/LevelDataManager.cs
        static List<long> roomIds = new List<long>(){
            0x321, 0x323, 0x324, 0x325, 0x0, 0x3D4, 0x3D5, 0x3D6,
            0xB4, 0xB5, 0x214, 0x216, 0x217, 0x218, 0x219, 0x21A,
            0x21E, 0x21F, 0x23A, 0x23B, 0x24E, 0x65, 0x67, 0x68,
            0x6A, 0x6B, 0x6C, 0x6E, 0x8C, 0x8D, 0x8E, 0x8F, 0xBE,
            0x7F, 0x88, 0x89, 0xC2, 0x1F8, 0x1FB, 0x22A, 0x22B,
            0x1F7, 0x22C, 0x22D, 0x7E, 0x73, 0x75, 0x76, 0x77,
            0x8A, 0x8B, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB0, 0xB1,
            0xB2, 0xC0, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0xC3,
            0x2BC, 0x2C0, 0x2C1, 0x2C2, 0x2C3, 0x2C4, 0x2C5, 0x2C6,
            0x2C7, 0x2C8, 0x2E4, 0x2E5, 0x2E6, 0x316, 0x2BE, 0x2E7,
            0x2E8, 0x2E9, 0xC8, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xD0,
            0xD1, 0xD2, 0xD3, 0xD6, 0xDC, 0xE2, 0xE3, 0xE4, 0xE5,
            0xE6, 0xE7, 0xE8, 0x122, 0xD4, 0xD7, 0xD8, 0xD9, 0xDA,
            0xDB, 0xDD, 0xDE, 0xDF, 0xE0, 0xE1, 0x123, 0x215, 0x21B,
            0x21C, 0x21D, 0x220, 0x221, 0x222, 0x223, 0x12C, 0x130,
            0x134, 0x136, 0x138, 0x139, 0x13B, 0x13C, 0x13D,
            0x13E, 0x141, 0x142, 0x143, 0x144, 0x145, 0x146, 0x186,
            0x190, 0x191, 0x192, 0x193, 0x194, 0x195, 0x196, 0x197,
            0x198, 0x199, 0x19B, 0x19C, 0x19D, 0x19E, 0x19F,
            0x1A0, 0x1A1, 0x1A2, 0x1A3, 0x1A4, 0xA15, 0x1A6,
            0x1A7, 0x1A8, 0x1A9, 0x1AA, 0x1AB, 0x1EA, 0x1F4,
            0x1FC, 0x1FD, 0x1FE, 0x204, 0x210, 0x1F5, 0x1FF,
            0x200, 0x201, 0x202, 0x203, 0x205, 0x20B, 0x20F,
            0x211, 0x212, 0x213, 0x24F, 0xBF, 0x82, 0x83, 0x84,
            0x85, 0x86, 0x87, 0x227, 0x229, 0x32B, 0x32E, 0x32F,
            0x337, 0x338, 0x339, 0x33A, 0x33B, 0x33C, 0x33D,
            0x33E, 0x33F, 0x340, 0x37A, 0x320, 0x322, 0x32A,
            0x330, 0x333, 0x335, 0x336, 0x250, 0x259, 0x25B,
            0x25C, 0x25D, 0x262, 0x263, 0x26C, 0x26D, 0x26E,
            0x26F, 0x260, 0x261, 0x264, 0x265, 0x266, 0x268,
            0x269, 0x26A, 0x26B, 0x2B2, 0x1F6, 0x20C, 0x20D,
            0x20E, 0x66, 0x71, 0x72, 0x81, 0x78, 0x79, 0x7A, 0xAA,
            0xC1, 0x258, 0x2CA, 0x2CC, 0x2CD, 0x2CF, 0x2DC,
            0x2DD, 0x2DE, 0x2DF, 0x2E0, 0x2E1, 0x2E2, 0x2E3,
            0x2D0, 0x2DA, 0x2DB, 0x38D, 0x38E, 0x38F, 0x390,
            0x391, 0x392, 0x393, 0x394, 0x396, 0x397, 0x3B6,
            0x3B7, 0x3BB, 0x3BC, 0x3BD, 0x3C9, 0x0, 0x3CA,
        };

        public static void ReadROMData(Processing system) {
            string startAddress = "884C64", endAddress = "8A630D";
            int roomDataStartAddress = Convert.ToInt32(startAddress, 16),
                roomDataEndAddress = Convert.ToInt32(endAddress, 16);

            byte[] romFile = system.ROMData;
            settings = system.Settings;
            seed = settings.Seed;

            byte objectByte1 = 0x01, objectByte2 = 0x24, roomLimit = 0xFF;
            int currentRoomIndex = 0;
            bool isInConsole = false;
            int itemsInRoom = 0;

            for (int i = roomDataStartAddress; i < romFile.Length; i++) {
                if (i > roomDataEndAddress || i + 10 >= romFile.Length) return;

                long currentRoom;
                try { currentRoom = roomIds[currentRoomIndex]; } catch { return;  }

                if (!isInConsole) {
                    Console.WriteLine($"Room: {currentRoom} / {Utils.ConvertLongToHex(currentRoom)}");
                    isInConsole = true;
                }

                if (romFile[i] == objectByte1 && romFile[i + 1] == objectByte2) {
                    byte[] objectDefinition = new byte[36];

                    for(int k = 0; k < 36; k++) {
                        int index = i + k;

                        objectDefinition[k] = romFile[index];
                    }

                    Console.WriteLine($"Object Found at Address: {i} / {i.ToString("X")}");
                    itemsInRoom++;

                    i += 35;
                    continue;
                }

                int emptyBytes = 0;

                if (romFile[i] == roomLimit) {
                    for (int j = 0; j < 10; i++) {
                        if (romFile[i + j] != roomLimit) {
                            Console.WriteLine($"Objects found in room: {itemsInRoom}");

                            if (emptyBytes >= 10) {
                                currentRoomIndex += 1;
                                i += 9;
                                isInConsole = false;
                                itemsInRoom = 0;
                            }
                            break;
                        } else {
                            emptyBytes++;
                        }
                    }                    
                }
            }
        }
    }
}
