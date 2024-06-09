using KatAMInternal;

namespace KatAM_Randomizer {
    internal class KatAMItems {
        static Processing System;
        static Settings Settings;
        static int seed;

        static List<Entity> entities;
        static List<Entity> chestEntities;
        static List<byte> objectIDs;
        static Dictionary<int, List<Entity>> chestDictionary;

        public static void RandomizeItems(Processing system) {
            entities = new List<Entity>();
            chestEntities = new List<Entity>();
            objectIDs = new List<byte>();
            chestDictionary = new Dictionary<int, List<Entity>>();

            // Read the settings, entities, and ROM data;
            System = system;
            Settings = system.Settings;
            byte[] romFile = System.ROMData;
            seed = Settings.Seed;

            // Deserializing all the entities in the game;
            DeserializeEntities(Utils.JSONToEntities(Utils.itemsJson));

            // Shuffling the object IDs;
            objectIDs = Utils.Shuffle(objectIDs);

            bool isRandom = false;

            for(int i = 0; i < entities.Count; i++) {
                Entity entity = entities[i];
                
                if (IsProgressionObject(entity)) continue;

                if (isRandom) {
                    entity.ID = GenerateRandomConsumable();

                    Utils.WriteObjectToROM(romFile, entity);
                } else {
                    entity.ID = objectIDs[i];

                    Utils.WriteObjectToROM(romFile, entity);
                }
            }

            for(int i = 0; i < chestEntities.Count; i++) {
                Entity chest = chestEntities[i];

                if (IsProgressionObject(chest)) {
                    AddChestToDictionary(chest);

                    continue;
                }

                int replacedEntityIndex = ReplaceChestEntity(chest);
                Entity oldEntity = entities[replacedEntityIndex];

                chest.Address = oldEntity.Address;
                chest.Number = oldEntity.Number;
                chest.Link = oldEntity.Link;
                chest.X = oldEntity.X;
                chest.Y = oldEntity.Y;
                chest.ID = 0x80; // Assign small chests since big chests fall of the level;
                chest.Room = oldEntity.Room;

                AddChestToDictionary(chest);

                //Console.WriteLine($"Chest Replaced At Address: {chest.Address}");

                Utils.WriteObjectToROM(romFile, chest);
            }

            /*foreach(Entity entity in entities) {
                Entity chest = new Entity();

                chest.Address = entity.Address;
                chest.Number = entity.Number;
                chest.Link = entity.Link;
                chest.X = entity.X;
                chest.Y = entity.Y;
                chest.ID = 0x80; // Assign small chests since big chests fall of the level;
                chest.Room = entity.Room;
                chest.Behavior = entity.Behavior;

                Utils.WriteObjectToROM(romFile, chest);
                AddChestToDictionary(chest);
            }*/

            RandomizeChests();
        }

        static void DeserializeEntities(dynamic itemsJson) {
            foreach (string key in itemsJson.Keys) {
                List<Dictionary<string, dynamic>> dict = itemsJson[key];

                // Print the contents of the list
                foreach (var item in dict) {
                    EntitySerializable serialized = new EntitySerializable();

                    serialized.Name = key;

                    foreach (var kvp in item) {
                        switch (kvp.Key) {
                            case "Address":
                            int address = (int)kvp.Value;

                            serialized.Address = address;
                            break;
                            case "Number": serialized.Number = kvp.Value; break;
                            case "Link": serialized.Link = kvp.Value; break;
                            case "X": serialized.X = kvp.Value; break;
                            case "Y": serialized.Y = kvp.Value; break;
                            case "ID":
                            byte ID = (byte)kvp.Value;
                            serialized.ID = ID;
                            break;
                            case "Behavior": serialized.Behavior = (byte)kvp.Value; break;
                            case "Speed": serialized.Speed = (byte)kvp.Value; break;
                            case "Properties": serialized.Properties = kvp.Value; break;
                            case "Room":
                            serialized.Room = (int)kvp.Value;
                            break;
                        }
                    }

                    Entity entity = serialized.DeserializeEntity();

                    // If it's not a progression object or the room is not unused, save entities to a list;
                    if (entity.Room == 0x0) continue;

                    // If it's a chest save its information for later;
                    if (IsVetoedObject(entity) || IsProgressionObject(entity)) {
                        chestEntities.Add(new Entity(entity));
                        objectIDs.Add(GenerateRandomConsumable());
                    } else { // If it's a consumable, save it for randomization;
                        objectIDs.Add(entity.ID);
                    }

                    entities.Add(entity);
                }
            }
        }

        static bool IsVetoedObject(Entity entity) {
            string name = entity.Name;

            return name == Processing.itemsDictionary[0x80] ||
                   name == Processing.itemsDictionary[0x81];
        }

        // This will detect big chest items that are either switches or passage-ways to not randomize them;
        static bool IsProgressionObject(Entity entity) {
            byte behavior = entity.Behavior;

            bool isProgressionObject = entity.ID == 0x81 && behavior == 0x63,
                 isDimensionMirror = entity.ID == 0x65 && behavior == 0x08;

            return isProgressionObject || isDimensionMirror;
        }

        static byte[] consumableItems = { 0x5E, 0x5F, 0x60, 0x61, 0x62, 0x63, 0x64 };
        static byte GenerateRandomConsumable() {
            int index = Utils.GetRandomNumber(0, consumableItems.Length);

            return consumableItems[index];
        }

        static void AddChestToDictionary(Entity chest) {
            if (!chestDictionary.ContainsKey(chest.Room)) {
                chestDictionary[chest.Room] = new List<Entity>();
            }

            Console.WriteLine($"Chest Type: {chest.Behavior}");

            chestDictionary[chest.Room].Add(chest);
        }
        static int ReplaceChestEntity(Entity chest) {
            int selectedEntity = Utils.GetRandomNumber(0, entities.Count);
            Entity entity = entities[selectedEntity];

            do {
                selectedEntity = Utils.GetRandomNumber(0, entities.Count);
                entity = entities[selectedEntity];
            } while (IsVetoedObject(entity));

            return selectedEntity;
        }


        static long NineROMStartAddress = 9441164,
                    NineROMEndAddress = 9449752,
                    NewNineROMAddress = 14745600,
                    PointersListAddress = 13825216,
                    NewListPointer = 134217728;

        public static void RandomizeChests() {
            byte[] romFile = System.ROMData;
            int currentRoomIndex = 0,
                chestsInRoomCount = 0,
                lastRoomAdded = -1;
            long currentRoomAddress = NewNineROMAddress,
                 newListAddress = NewNineROMAddress;
            byte[] chestBytes = new byte[] { 0x01, 0x08, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, };

            for (long i = NineROMStartAddress; i < romFile.Length; i += 1) {
                if (i >= NineROMEndAddress || i >= romFile.Length) return;
                int currentRoom;

                try { currentRoom = Processing.roomIds[currentRoomIndex]; } catch { return; }

                if (chestDictionary.ContainsKey(currentRoom)) {
                    if (currentRoom != lastRoomAdded) {
                        List<Entity> chestsInRoom = chestDictionary[currentRoom];
                        chestsInRoomCount = chestsInRoom.Count;

                        for (int k = 0; k < chestsInRoom.Count; k++) {
                            Entity chest = chestsInRoom[k];
                            byte[] x = chest.X, y = chest.Y;

                            chestBytes[4] = x[0];
                            chestBytes[5] = x[1];
                            chestBytes[6] = y[0];
                            chestBytes[7] = y[1];

                            Utils.WriteToROM(romFile, newListAddress, chestBytes);

                            //Console.WriteLine($"Chest At Address: {newListAddress}");

                            newListAddress += 8;
                        }

                        lastRoomAdded = currentRoom;
                    }
                } else {
                    chestsInRoomCount = 0;
                }


                byte byte1 = romFile[i],
                     byte2 = romFile[i + 1],
                     byte3 = romFile[i + 2],
                     byte4 = romFile[i + 3];

                if (IsChest(byte1, byte2, byte3, byte4)) {
                    //Console.WriteLine($"Chest 9ROM Found at {i} address!");

                    i += 7;
                } else if (IsMirror(byte1, byte2, byte3, byte4)) {
                    //Console.WriteLine($"Mirror 9ROM Found at {i} address!");

                    byte[] mirrorData = ExtractNineROMData(romFile, i, 8);

                    Utils.WriteToROM(romFile, newListAddress, mirrorData);

                    i += 7;
                    newListAddress += 8;
                } else if (IsEndOfRoom(byte1, byte2, byte3, byte4)) {
                    //Console.WriteLine($"End of Room {Processing.roomIds[currentRoomIndex]} / {Utils.ConvertIntToHex(Processing.roomIds[currentRoomIndex])} 9ROM Found at {i} address!");

                    byte[] endOfRoomData = ExtractNineROMData(romFile, i, 12),
                           writeRoomData = BitConverter.GetBytes(currentRoomAddress + NewListPointer);

                    for (int j = 4; j < writeRoomData.Length + 4; j += 1) {
                        byte bit = writeRoomData[j - 4];

                        endOfRoomData[j] = bit;
                    }

                    endOfRoomData[8] = (byte)chestsInRoomCount;

                    Utils.WriteToROM(romFile, newListAddress, endOfRoomData);

                    // Overwritting Pointers;
                    long pointerIndex = PointersListAddress + (4 * currentRoomIndex);
                    byte[] pointerInformation = BitConverter.GetBytes((newListAddress + 4) + NewListPointer),
                           pointerToWrite = new byte[4];

                    for (int l = 0; l < 4; l++) {
                        pointerToWrite[l] = pointerInformation[l];
                    }

                    Utils.WriteToROM(romFile, pointerIndex, pointerToWrite);

                    currentRoomIndex++;
                    i += 11;
                    newListAddress += 12;
                    currentRoomAddress = newListAddress;
                }
            }
        }

        static bool IsChest(byte first, byte second, byte third, byte forth) {
            return (first == 0x01) && (second == 0x08) && (third == 0xFF) && (forth == 0xFF);
        }

        static bool IsMirror(byte first, byte second, byte third, byte forth) {
            return (first == 0x02) && (second == 0x08) && (third == 0xFF) && (forth == 0xFF);
        }

        static bool IsEndOfRoom(byte first, byte second, byte third, byte forth) {
            return (first == 0x00) && (second == 0x00) && (third == 0xFF) && (forth == 0xFF);
        }

        static byte[] ExtractNineROMData(byte[] romFile, long i, int arraySize) {
            byte[] data = new byte[arraySize];

            for (long j = 0; j < arraySize; j++) {
                long index = i + j;
                data[j] = romFile[index];
            }

            return data;
        }
    }
}
