using KatAMInternal;
using Newtonsoft.Json;
using System.Collections;
using System.Reflection;
using System.Reflection.Metadata;
using System.Reflection.Metadata.Ecma335;
using System.Text;
using System.Text.Json;

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

        // https://www.tapatalk.com/groups/lighthouse_of_yoshi/kirby-and-the-amazing-mirror-hacking-t741.html
        static Dictionary<byte, string> enemiesDictionary = new Dictionary<byte, string>{
            { 0x00, "Waddle Dee" }, 
            { 0x01, "Bronto Burt" }, 
            { 0x02, "Blipper"},
            { 0x03, "Glunk"}, 
            { 0x04, "Squishy" }, 
            { 0x05, "Scarfy" },
            { 0x06, "Gordo" }, 
            { 0x07, "Snooter" }, 
            { 0x08, "Chip"},
            { 0x09, "Soarar"}, 
            { 0x0A, "Haley"}, 
            { 0x0B, "Roly-Poly"}, 
            { 0x0C, "Cupie"}, 
            { 0x0D, "Blockin"}, 
            { 0x0E, "Snooter 2"},
            { 0x0F, "Leap"},
            { 0x10, "Jack" },
            { 0x11, "Big Waddle Dee" },
            { 0x12, "Waddle Doo"},
            { 0x13, "Flamer"},
            { 0x14, "Hot Head" },
            { 0x15, "Laser Ball" },
            { 0x16, "Pengy" },
            { 0x17, "Rocky" },
            { 0x18, "Sir Kibble"},
            { 0x19, "Sparky"},
            { 0x1A, "Sword Knight"},
            { 0x1B, "UFO"},
            { 0x1C, "Twister"},
            { 0x1D, "Wheelie"},
            { 0x1E, "Noddy"},
            { 0x1F, "Golem Press"},
            { 0x20, "Golem Roll" },
            { 0x21, "Golem Punch" },
            { 0x22, "Foley"},
            { 0x23, "Shooty"},
            { 0x24, "Scarfy 2" },
            { 0x25, "Boxin" },
            { 0x26, "Cookin" },
            { 0x27, "Minny" },
            { 0x28, "Bomber"},
            { 0x29, "Heavy Knight"},
            { 0x2A, "Giant Rocky"},
            { 0x2B, "Metal Guardian"},
            { 0x2C, "Nothing"},
            { 0x2D, "Batty"},
            { 0x2E, "Foley Automatic"},
            { 0x2F, "Bang-Bang"},
            { 0x30, "Explosion" },
            { 0x31, "Nothing" },
            { 0x32, "Droppy"},
            { 0x33, "Prank"},
            { 0x34, "Mirra" },
            { 0x35, "Shotzo" },
            { 0x36, "Nothing" },
            { 0x37, "Waddle Dee 2" },
            { 0x70, "Shotzo 2"},
            { 0xA2, "Parasol"}
        };

        static Dictionary<byte, string> minibossesDictionary = new Dictionary<byte, string>{
            { 0x38, "Mr. Frosty" },
            { 0x39, "Bonkers" },
            { 0x3A, "Phan Phan"},
            { 0x3B, "Batafire"},
            { 0x3C, "Box Boxer" },
            { 0x3D, "Boxy" },
            { 0x3E, "Master Hand" },
            { 0x3F, "Bombar" },
        };

        static Dictionary<byte, string> bossesDictionary = new Dictionary<byte, string>{
            { 0x45, "Kracko" },
            { 0x46, "King Golem" },
            { 0x47, "Master Hand (Boss)"},
            { 0x48, "Gobbler"},
            { 0x49, "Wiz" },
            { 0x4A, "Moley" },
            { 0x4B, "Mega Titan" },
            { 0x4C, "Titan Head" },
            { 0x4D, "Crazy Hand" },
            { 0x4E, "Dark Meta Knight (Boss)" },
            { 0x4F, "Dark Mind (1)"},
            { 0x50, "Dark Mind (2)"},
            { 0x51, "Dark Mind (Shooting)" },
            { 0x52, "Dark Meta Knight (Radish)" },
        };

        static Dictionary<byte, string> itemsDictionary = new Dictionary<byte, string>{
            { 0x5E, "Cherry" },
            { 0x5F, "Pep Brew" },
            { 0x60, "Meat"},
            { 0x61, "Maximum Tomato"},
            { 0x62, "Battery" },
            { 0x63, "1UP" },
            { 0x64, "Lollipop" },
            { 0x65, "Mirror Shard" },
            { 0x66, "Nothing"},
            { 0x67, "Nothing"},
            { 0x68, "Nothing"},
            { 0x69, "Nothing"},
            { 0x6A, "Nothing"},
            { 0x6B, "Nothing"},
            { 0x6C, "Nothing"},
            { 0x80, "Small Chest"},
            { 0x81, "Big Chest"}
        };

        static Dictionary<byte, string> mirrorsDictionary = new Dictionary<byte, string>{
            { 0x6F, "Mirror" },
            { 0x8C, "Special Hub Mirror"},
        };

        static Dictionary<byte, string> abilityStandsDictionary = new Dictionary<byte, string>{
            { 0x92, "Ability Stand" },
            { 0x93, "Ability Stand" },
            { 0x94, "Ability Stand" },
            { 0x95, "Ability Stand" },
            { 0x96, "Random Ability Stand" },
        };

        static Dictionary<byte, string> mapElementsDictionary = new Dictionary<byte, string>{
            { 0x7D, "Large Star Stone Block (No Collision)" },
            { 0x7E, "Large Star Stone Block"},
            { 0x88, "Large Star Stone Block"},
            { 0x89, "Large Star Stone Block"},
        };



        public static void ReadROMData(Processing system) {
            byte[] romFile = system.ROMData;
            settings = system.Settings;
            seed = settings.Seed;

            string startAddress = "884C64", endAddress = "8A630D";
            int roomDataStartAddress = Convert.ToInt32(startAddress, 16),
                roomDataEndAddress = Convert.ToInt32(endAddress, 16);
            
            List<Entity> enemies = new List<Entity>(),
                        minibosses = new List<Entity>(),
                        bosses = new List<Entity>(),
                        items = new List<Entity>(),
                        mirrors = new List<Entity>(),
                        abilityStands = new List<Entity>(),
                        miscellaneous = new List<Entity>();

            byte objectByte1 = 0x01, objectByte2 = 0x24, roomLimit = 0xFF;
            int currentRoomIndex = 0;
            bool isInConsole = false;
            int itemsInRoom = 0;

            for (int i = roomDataStartAddress; i < romFile.Length; i++) {
                if (i > roomDataEndAddress || i + 10 >= romFile.Length) break;

                long currentRoom;
                try { currentRoom = roomIds[currentRoomIndex]; } catch { break;  }

                /*if (!isInConsole) {
                    Console.WriteLine($"Room: {currentRoom} / {Utils.ConvertLongToHex(currentRoom)}");
                    isInConsole = true;
                }*/

                if (romFile[i] == objectByte1 && romFile[i + 1] == objectByte2) {
                    byte[] objectDefinition = new byte[36];
                    Entity entity = new Entity();

                    for(int k = 0; k < 36; k++) {
                        int index = i + k;

                        objectDefinition[k] = romFile[index];
                    }

                    entity.Definition = objectDefinition;
                    entity.Address = i;
                    entity.Number = new byte[] { romFile[i + 2], romFile[i + 3] };
                    entity.Link = new byte[] { romFile[i + 4], romFile[i + 5] };
                    entity.X = new byte[] { romFile[i + 6], romFile[i + 7] };
                    entity.Y = new byte[] { romFile[i + 8], romFile[i + 9] };

                    byte ID = romFile[i + 12];
                    entity.ID = ID;

                    entity.Behavior = romFile[i + 14];
                    entity.Speed = romFile[i + 16];
                    Array.Copy(objectDefinition, 18, entity.Properties, 0, 18);
                    entity.Room = (byte) currentRoom;

                    if (enemiesDictionary.ContainsKey(ID)) {
                        entity.Name = enemiesDictionary[ID];

                        AreAllPropertiesZeroes(entity);

                        enemies.Add(entity);
                    } else if (minibossesDictionary.ContainsKey(ID)) {
                        entity.Name = minibossesDictionary[ID];
                        minibosses.Add(entity);
                    } else if (bossesDictionary.ContainsKey(ID)) {
                        entity.Name = bossesDictionary[ID];
                        bosses.Add(entity);
                    } else if (itemsDictionary.ContainsKey(ID)) {
                        entity.Name = itemsDictionary[ID];
                        items.Add(entity);
                    } else if (mirrorsDictionary.ContainsKey(ID)) {
                        entity.Name = mirrorsDictionary[ID];

                        AreAllPropertiesZeroes(entity);

                        mirrors.Add(entity);
                    } else if (abilityStandsDictionary.ContainsKey(ID)) {
                        entity.Name = abilityStandsDictionary[ID];
                        abilityStands.Add(entity);
                    } else if (mapElementsDictionary.ContainsKey(ID)) {
                        entity.Name = mapElementsDictionary[ID];

                        AreAllPropertiesZeroes(entity);

                        miscellaneous.Add(entity);
                    }

                    //Console.WriteLine($"Object Found at Address: {i} / {i.ToString("X")} {ID}");
                    itemsInRoom++;

                    i += 35;
                    continue;
                }

                int emptyBytes = 0;

                if (romFile[i] == roomLimit) {
                    for (int j = 0; j < 10; i++) {
                        if (romFile[i + j] != roomLimit) {
                            //Console.WriteLine($"Objects found in room: {itemsInRoom}");

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

            static void AreAllPropertiesZeroes(Entity entity) {
                if (entity.Properties.All(x => x == 0)) {
                    entity.Properties = new byte[] { 0 };
                }
            }

            Console.WriteLine("Enemies saved: " + enemies.Count);
            Console.WriteLine("Minibosses saved: " + minibosses.Count);
            Console.WriteLine("Bosses saved: " + bosses.Count);
            Console.WriteLine("Items saved: " + items.Count);
            Console.WriteLine("Mirrors saved: " + mirrors.Count);
            Console.WriteLine("Ability Stands saved: " + abilityStands.Count);
            Console.WriteLine("World Map Objects saved: " + miscellaneous.Count);

            // Serialize the dictionary to JSON using Newtonsoft.Json
            SaveJSON(enemies, "Enemies.json");
            SaveJSON(minibosses, "Minibosses.json");
            SaveJSON(bosses, "Bosses.json");
            SaveJSON(items, "Items.json");
            SaveJSON(mirrors, "Mirrors.json");
            SaveJSON(abilityStands, "AbilityStands.json");
            SaveJSON(miscellaneous, "WorldMapObjects.json");
        }

        static string jsonRoute = "JSON\\";

        static void SaveJSON(dynamic list, string filename) {
            string json = JsonConvert.SerializeObject(EntitiesToJSON(list), Formatting.Indented);

            File.WriteAllText(jsonRoute + filename, json);
        }

        static dynamic EntitiesToJSON(List<Entity> list) {
            var groupedEntities = new Dictionary<string, List<Dictionary<string, dynamic>>>();

            foreach (Entity entity in list) {
                EntitySerializable serialized = entity.SerializeEntity();

                if (!groupedEntities.ContainsKey(serialized.Name)) {
                    groupedEntities[serialized.Name] = new List<Dictionary<string, dynamic>>();
                }

                var entityDict = new Dictionary<string, dynamic>();

                // Use reflection to loop through all properties
                string key = serialized.Name;

                //entityDict["Definition"] = serialized.Definition;
                entityDict["Address"] = serialized.Address;
                entityDict["Number"] = serialized.Number;
                entityDict["Link"] = serialized.Link;
                entityDict["X"] = serialized.X;
                entityDict["Y"] = serialized.Y;
                entityDict["ID"] = serialized.ID;
                entityDict["Behaviour"] = serialized.Behavior;
                entityDict["Speed"] = serialized.Speed;
                entityDict["Properties"] = serialized.Properties;
                entityDict["Room"] = serialized.Room;

                groupedEntities[serialized.Name].Add(entityDict);
            }

            return groupedEntities;
        }
    }

    class Entity {
        public string Name { get; set; }
        public byte[] Definition { get; set; }
        public int Address { get; set; }

        // Object Number in a room;
        public byte[] Number { get; set; }

        // Object to which this entity is linked with; For doors or buttons;
        public byte[] Link { get; set; }
        public byte[] X { get; set; }
        public byte[] Y { get; set; }
        public byte ID { get; set; }
        public byte Behavior { get; set; }
        public byte Speed { get; set; }
        public byte[] Properties { get; set; }
        public byte Room { get; set; }

        public Entity() {
            Link = new byte[2];
            X = new byte[2];
            Y = new byte[2];
            Properties = new byte[18];
        }

        public EntitySerializable SerializeEntity() {
            return new EntitySerializable(this);
        }
    }

    class EntitySerializable : Entity {
        public string Definition { get; set; }
        public string Number { get; set; }
        public string Link { get; set; }
        public string X { get; set; }
        public string Y { get; set; }
        public string Properties { get; set; }

        public EntitySerializable(Entity entity) {
            Definition = ByteArrayToHexString(entity.Definition);
            Number = ByteArrayToHexString(entity.Number);
            Link = ByteArrayToHexString(entity.Link);
            X = ByteArrayToHexString(entity.X);
            Y = ByteArrayToHexString(entity.Y);
            Properties = ByteArrayToHexString(entity.Properties);

            Name = entity.Name;
            Address = entity.Address;
            ID = entity.ID;
            Behavior = entity.Behavior;
            Speed = entity.Speed;
            Room = entity.Room;
        }

        public static string ByteArrayToString(byte[] byteArray) {
            // Convert byte array to a comma-separated string
            string result = "[" + string.Join(", ", byteArray) + "]";

            return result;
        }

        public static string ByteArrayToHexString(byte[] byteArray) {
            string result = "";
            
            foreach (byte bit in byteArray) {
                result += bit.ToString("X2");
            }

            return result;
        }

        public Entity DeserializeEntity() {
            Entity entity = new Entity();

            entity.Definition = StringToByteArray(Definition);
            entity.Number = StringToByteArray(Number);
            entity.Link = StringToByteArray(Link);
            entity.X = StringToByteArray(X);
            entity.Y = StringToByteArray(Y);
            entity.Properties = StringToByteArray(Properties);

            entity.Name = Name;
            entity.Address = Address;
            entity.ID = ID;
            entity.Behavior = Behavior;
            entity.Speed = Speed;

            return entity;
        }

        public static byte[] StringToByteArray(string input) {
            // Remove the square brackets
            input = input.Trim(new char[] { '[', ']' });

            // Split the string by commas
            string[] stringArray = input.Split(',');

            // Convert each string element to a byte
            byte[] byteArray = stringArray.Select(byte.Parse).ToArray();

            return byteArray;
        }
    }
}
