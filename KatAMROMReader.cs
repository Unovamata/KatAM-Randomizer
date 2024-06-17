using KatAM_Randomizer;
using KatAMInternal;

namespace KatAMRandomizer {
    internal class KatAMROMReader : KatAMRandomizerComponent {
        static Settings settings;
        static int seed;

        public KatAMROMReader(Processing system) {
            InitializeComponents(system);

            ReadObjectParameterData(System.ROMData);

            ReadObjectData(System.ROMData);
        }

        List<Properties> properties = new List<Properties>();

        void ReadObjectParameterData(byte[] romFile) {
            int parameterStartAddress = 0x335E1C,
                parameterEndAddress = 0x3372A4;

            int currentObjectID = 0, currentByteCount = 0;

            Properties property = new Properties();

            // Read the object parameter data;
            for (int i = parameterStartAddress; i <= parameterEndAddress; i++) {
                // Object parameters are 24 bytes long;
                if(currentByteCount >= 24) {
                    // Specify the object parameters and add them to the property list for future processing;
                    byte[] definition = property.Definition;

                    property.Name = Processing.parameters[(byte) currentObjectID];
                    property.ID = (byte) currentObjectID;
                    property.DamageSprites = new byte[] { definition[0], definition[1] };
                    property.HP = definition[4];
                    property.CopyAbility = definition[6];
                    property.Palette = definition[8];

                    properties.Add(property);

                    currentByteCount = 0;
                    currentObjectID++;
                } 
                
                // For new object parameters, define the address;
                if(currentByteCount == 0) {
                    property = new Properties();

                    property.Address = i;

                    property.Definition[currentByteCount] = romFile[i];
                }
                
                // Store the definition data;
                property.Definition[currentByteCount] = romFile[i];

                currentByteCount++;
            }

            Console.WriteLine("Object Properties saved: " + properties.Count);

            Utils.SaveJSON(properties, Utils.propertiesJson);
        }

        // Calling the data linking information;
        List<int> roomIds = Processing.roomIds;
        Dictionary<byte, Tuple<string, byte, bool>> enemiesDictionary = Processing.enemiesDictionary,
                                                    minibossesDictionary = Processing.minibossesDictionary;
        Dictionary<byte, string> bossesDictionary = Processing.bossesDictionary,
                                 itemsDictionary = Processing.itemsDictionary,
                                 mirrorsDictionary = Processing.mirrorsDictionary,
                                 abilityStandsDictionary = Processing.abilityStandsDictionary,
                                 mapElementsDictionary = Processing.mapElementsDictionary;

        List<Entity> enemies = new List<Entity>(),
                     minibosses = new List<Entity>(),
                     bosses = new List<Entity>(),
                     items = new List<Entity>(),
                     mirrors = new List<Entity>(),
                     abilityStands = new List<Entity>(),
                     miscellaneous = new List<Entity>(),
                     undefined = new List<Entity>();

        void ReadObjectData(byte[] romFile) {
            // Memory locations;
            string startAddress = "884C64", endAddress = "8A630D";
            int roomDataStartAddress = Convert.ToInt32(startAddress, 16),
                roomDataEndAddress = Convert.ToInt32(endAddress, 16);

            // Data pointers;
            byte objectByte1 = 0x01, objectByte2 = 0x24, // Start of object definition;
                 roomLimit = 0xFF; // Separator byte;
            int currentRoomIndex = 0;
            int itemsInRoom = 0;
            //bool isInConsole = false;

            // Checking all rooms for objects;
            for (int i = roomDataStartAddress; i < romFile.Length; i++) {
                if (i > roomDataEndAddress || i + 10 >= romFile.Length) break;

                int currentRoom; // Extracting the current room ID;
                try { currentRoom = roomIds[currentRoomIndex]; } catch { break; }

                /*if (!isInConsole) {
                    Console.WriteLine($"Room: {currentRoom} / {Utils.ConvertLongToHex(currentRoom)}");
                    isInConsole = true;
                }*/

                bool isAnObject = romFile[i] == objectByte1 && romFile[i + 1] == objectByte2;

                if (isAnObject) {
                    // Create a new Entity reference;
                    byte[] objectDefinition = new byte[36];
                    Entity entity = new Entity();

                    // Extract all the bytes for the object; 01 24 00 00 00 00 00 00 ...
                    for (int k = 0; k < 36; k++) {
                        int index = i + k;

                        objectDefinition[k] = romFile[index];
                    }

                    // Extract the information in Hex from the ROM;
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
                    Array.Copy(objectDefinition, 17, entity.Properties, 0, entity.Properties.Length);
                    entity.Room = currentRoom;

                    // Mapping the enemies to a specific name type;
                    bool isEnemy = enemiesDictionary.ContainsKey(ID);
                    bool isMiniboss = minibossesDictionary.ContainsKey(ID);
                    bool isBoss = bossesDictionary.ContainsKey(ID);
                    bool isItem = itemsDictionary.ContainsKey(ID);
                    bool isMirror = mirrorsDictionary.ContainsKey(ID);
                    bool isAbilityStand = abilityStandsDictionary.ContainsKey(ID);
                    bool isMapElement = mapElementsDictionary.ContainsKey(ID);

                    // Enemy references;
                    if (isEnemy) {
                        entity.Name = enemiesDictionary[ID].Item1;

                        entity.AreAllPropertiesZeroes();

                        enemies.Add(entity);
                    }

                    // Miniboss references;
                    else if (isMiniboss) {
                        entity.Name = minibossesDictionary[ID].Item1;
                        minibosses.Add(entity);
                    }

                    // Boss references;
                    else if (isBoss) {
                        entity.Name = bossesDictionary[ID];
                        bosses.Add(entity);
                    }

                    // Item references;
                    else if (isItem) {
                        entity.Name = itemsDictionary[ID];

                        entity.AreAllPropertiesZeroes();

                        items.Add(entity);
                    }

                    // Mirror references;
                    else if (isMirror) {
                        entity.Name = mirrorsDictionary[ID];

                        entity.AreAllPropertiesZeroes();

                        mirrors.Add(entity);
                    }

                    // Ability Stand references;
                    else if (isAbilityStand) {
                        entity.Name = abilityStandsDictionary[ID];
                        abilityStands.Add(entity);
                    }

                    // Map Element references;
                    else if (isMapElement) {
                        entity.Name = mapElementsDictionary[ID];

                        entity.AreAllPropertiesZeroes();

                        miscellaneous.Add(entity);
                    }

                    // Unassigned references;
                    else {
                        undefined.Add(entity);
                    }

                    //Console.WriteLine($"Object Found at Address: {i} / {i.ToString("X")} {ID}");
                    itemsInRoom++;

                    // Progress 35 bytes because object definitions are 36 bytes long;
                    i += 35;
                    continue;
                }

                int emptyBytes = 0;

                bool isInObjectLimitByte = romFile[i] == roomLimit;

                if (isInObjectLimitByte) {
                    // If the room limit byte repeats 10 times, then this is a room end definition;
                    for (int j = 0; j < 10; i++) {
                        bool isNotARoomLimit = romFile[i + j] != roomLimit;

                        if (isNotARoomLimit) {
                            //Console.WriteLine($"Objects found in room: {itemsInRoom}");

                            // If the room separator has been reached, increment the data points;
                            if (emptyBytes >= 10) {
                                currentRoomIndex += 1;
                                i += 9;
                                //isInConsole = false;
                                itemsInRoom = 0;
                            }

                            break;
                        } else emptyBytes++;
                    }
                }
            }

            Console.WriteLine("Enemies saved: " + enemies.Count);
            Console.WriteLine("Minibosses saved: " + minibosses.Count);
            Console.WriteLine("Bosses saved: " + bosses.Count);
            Console.WriteLine("Items saved: " + items.Count);
            Console.WriteLine("Mirrors saved: " + mirrors.Count);
            Console.WriteLine("Ability Stands saved: " + abilityStands.Count);
            Console.WriteLine("World Map Objects saved: " + miscellaneous.Count);
            Console.WriteLine("Objects Unassigned saved: " + undefined.Count);

            // Serialize the dictionary to JSON using Newtonsoft.Json
            Utils.SaveJSON(enemies, Utils.enemiesJson);
            Utils.SaveJSON(minibosses, Utils.minibossesJson);
            Utils.SaveJSON(bosses, Utils.bossesJson);
            Utils.SaveJSON(items, Utils.itemsJson);
            Utils.SaveJSON(mirrors, Utils.mirrorsJson);
            Utils.SaveJSON(abilityStands, Utils.abilityStandsJson);
            Utils.SaveJSON(miscellaneous, Utils.worldMapObjectsJson);
        }
    }
}
