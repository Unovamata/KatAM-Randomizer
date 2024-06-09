using KatAMInternal;

namespace KatAM_Randomizer {
    internal class KatAMItems {
        static Processing System;
        static Settings Settings;
        static int seed;

        static List<Entity> entities;
        static List<Entity> chestEntities;
        static List<byte> objectIDs;
        static GenerationOptions consumableOptions;

        public static void RandomizeItems(Processing system) {
            entities = new List<Entity>();
            chestEntities = new List<Entity>();
            objectIDs = new List<byte>();
            
            // Read the settings, entities, and ROM data;
            System = system;
            Settings = system.Settings;
            consumableOptions = Settings.ConsumablesGenerationType;
            byte[] romFile = System.ROMData;
            seed = Settings.Seed;

            // Deserializing all the entities in the game;
            DeserializeEntities(Utils.JSONToEntities(Utils.itemsJson));

            // Shuffling the object IDs;
            bool isShuffling = consumableOptions == GenerationOptions.Shuffle;

            if (isShuffling) {
                objectIDs = Utils.Shuffle(objectIDs);
            }

            for(int i = 0; i < entities.Count; i++) {
                Entity entity = entities[i];
                
                if (IsProgressionObject(entity)) continue;

                switch (consumableOptions) {
                    // Writing consumables as they come in the OG game;
                    case GenerationOptions.Unchanged:
                    case GenerationOptions.Shuffle:
                        entity.ID = objectIDs[i];
                    break;

                    case GenerationOptions.Random:
                        entity.ID = GenerateRandomConsumable();
                    break;

                    case GenerationOptions.Challenge:
                        bool canSpawnItem = Utils.Dice() == 1;

                        // If the item can spawn, spawn a cherry;
                        if (canSpawnItem) {
                            entity.ID = consumableItems[0];
                        } else { // If not, don't spawn anything;
                            entity.ID = 0x2C;
                        }
                    break;

                    // Spawn no consumables;
                    case GenerationOptions.No:
                        entity.ID = 0x2C;
                    break;
                }

                Utils.WriteObjectToROM(romFile, entity);
            }
        }

        static void DeserializeEntities(dynamic itemsJson) {
            // Reading the JSON dictionary;
            foreach (string key in itemsJson.Keys) {
                List<Dictionary<string, dynamic>> dict = itemsJson[key];

                // Injecting the extracted information on Entities;
                foreach (var item in dict) {
                    EntitySerializable serialized = new EntitySerializable();

                    serialized.Name = key;

                    // Reading the KeyValuePair information;
                    foreach (var kvp in item) {
                        switch (kvp.Key) {
                            case "Address":
                                int address = (int) kvp.Value;
                                serialized.Address = address;
                            break;
                            case "Number": serialized.Number = kvp.Value; break;
                            case "Link": serialized.Link = kvp.Value; break;
                            case "X": serialized.X = kvp.Value; break;
                            case "Y": serialized.Y = kvp.Value; break;
                            case "ID":
                                byte ID = (byte) kvp.Value;
                                serialized.ID = ID;
                            break;
                            case "Behavior": serialized.Behavior = (byte) kvp.Value; break;
                            case "Speed": serialized.Speed = (byte) kvp.Value; break;
                            case "Properties": serialized.Properties = kvp.Value; break;
                            case "Room":
                                serialized.Room = (int) kvp.Value;
                            break;
                        }
                    }

                    Entity entity = serialized.DeserializeEntity();

                    // Save entities to a list if the objects are randomizeable;
                    if (IsVetoedRoom(entity)) continue;
                    if (entity.Name == "Mirror Shard") continue;

                    // If it's a chest save its information for later;
                    if (IsChestObject(entity) || IsProgressionObject(entity)) {
                        chestEntities.Add(new Entity(entity));
                        objectIDs.Add(GenerateRandomConsumable());
                    } else { // If it's a consumable, save it for randomization;
                        objectIDs.Add(entity.ID);
                    }

                    entities.Add(entity);
                }
            }
        }

        // IsVetoedRoom(); Check if the room is not feasible for randomization;
        static bool IsVetoedRoom(Entity entity) {
            int room = entity.Room;

            // Banned rooms like debug, boss endurance, or final boss rooms;
            HashSet<int> vetoedRooms = new HashSet<int>{
                0x0, 0x38D, 0x38E, 0x38F, 0x390, 0x391, 0x392, 0x393,
                0x394, 0x396, 0x397, 0x3B6, 0x3B7, 0x3BB, 0x3BC,
                0x3BD, 0x3C9, 0x3CA
            };

            return vetoedRooms.Contains(room);
        }

        // IsChestObject(); Checks if it's a chest object;
        static bool IsChestObject(Entity entity) {
            string name = entity.Name;

            return name == Processing.itemsDictionary[0x80] || // Small chest;
                   name == Processing.itemsDictionary[0x81]; // Big Chest;
        }

        // IsProgressionObject(); This will not randomize key progression objects;
        static bool IsProgressionObject(Entity entity) {
            byte behavior = entity.Behavior;

            // Big chest with switch parameter or mirror;
            bool isProgressionObject = (entity.ID == 0x81 && behavior == 0x63) || entity.ID == 0x65;

            return isProgressionObject;
        }

        // GenerateRandomConsumable(); Generates a random consumable ID ;
        static byte[] consumableItems = { 0x5E, 0x5F, 0x60, 0x61, 0x62, 0x63, 0x64 };
        static byte GenerateRandomConsumable() {
            int index = Utils.GetRandomNumber(0, consumableItems.Length);

            return consumableItems[index];
        }

        //ReplaceChestEntity(); Selects a random object to replace it for a chest in the entity list;
        static int ReplaceChestEntity(Entity chest) {
            int selectedEntity = Utils.GetRandomNumber(0, entities.Count);
            Entity entity = entities[selectedEntity];

            // If the entity chosen is a chest, reroll;
            do {
                selectedEntity = Utils.GetRandomNumber(0, entities.Count);
                entity = entities[selectedEntity];
            } while (IsChestObject(entity));

            return selectedEntity;
        }


        //////////////////////////////////////////////////////////////////////////////////////////////
        

        static Dictionary<int, List<Entity>> chestDictionary;
        static GenerationOptions chestOptions;

        // RandomizeChests(); Randomizes chest objects and injects them in the ROM's 9ROM pointers;
        public static void RandomizeChests() {
            byte[] romFile = System.ROMData;
            chestDictionary = new Dictionary<int, List<Entity>>();
            chestOptions = Settings.ChestsGenerationType;

            // Return if no chests must be written;
            if (chestOptions == GenerationOptions.No) return;

            // For every chest in the list, randomize it;
            for (int i = 0; i < chestEntities.Count; i++) {
                Entity chest = chestEntities[i];

                if (IsProgressionObject(chest)) {
                    AddChestToDictionary(chest);
                    continue;
                }

                // Shuffling chests;
                ShuffleChest(chest);
                AddChestToDictionary(chest);
                //Console.WriteLine($"Chest Replaced At Address: {chest.Address}");
                

                Utils.WriteObjectToROM(romFile, chest);
            }

            // Write to the ROM;
            WriteChestTo9ROM(romFile);
        }

        // AddChestToDictionary(); Adds a chest entity to a dictionary for later processing;
        static void AddChestToDictionary(Entity chest) {
            if (!chestDictionary.ContainsKey(chest.Room)) {
                chestDictionary[chest.Room] = new List<Entity>();
            }

            chestDictionary[chest.Room].Add(chest);
        }

        // ShuffleChest(); Take a chest entity and select a random entity to shuffle it to;
        static void ShuffleChest(Entity chest) {
            if (chestOptions == GenerationOptions.Shuffle) {
                int replacedEntityIndex = ReplaceChestEntity(chest);
                Entity oldEntity = entities[replacedEntityIndex];

                chest.Address = oldEntity.Address;
                chest.Number = oldEntity.Number;
                chest.Link = oldEntity.Link;
                chest.X = oldEntity.X;
                chest.Y = oldEntity.Y;
                chest.ID = 0x80; // Assign small chests since big chests fall of the level;
                chest.Room = oldEntity.Room;
            }
        }

        // Pointers;
        static long NineROMStartAddress = 9441164,
                    NineROMEndAddress = 9449752,

        // New pointers list for chest and mirror writing;
                    NewNineROMAddress = 14745600,
                    PointersListAddress = 13825216,
                    NewListPointer = 134217728;

        // WriteChestTo9ROM(); Writing the object data in a format the ROM can understand;
        static void WriteChestTo9ROM(byte[] romFile) {
            // Pointers to inspect the data correctly;
            int currentRoomIndex = 0,
                chestsInRoomCount = 0,
                lastRoomAdded = -1;

            long currentRoomAddress = NewNineROMAddress,
                 newListAddress = NewNineROMAddress;

            // Base empty chest;
            byte[] chestBytes = new byte[] { 0x01, 0x08, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, };

            // Read and Overwritte the data for all 9ROM addresses found;
            for (long i = NineROMStartAddress; i < romFile.Length; i += 1) {
                if (i >= NineROMEndAddress || i >= romFile.Length) return;

                int currentRoom;

                try { currentRoom = Processing.roomIds[currentRoomIndex]; } catch { return; }

                // If the room has any chests, process the data;
                if (chestDictionary.ContainsKey(currentRoom)) {
                    // And the chests haven't been added to the 9ROM data;
                    if (currentRoom != lastRoomAdded) {
                        List<Entity> chestsInRoom = chestDictionary[currentRoom];
                        chestsInRoomCount = chestsInRoom.Count;

                        /* Parse the information in the 9ROM's requested notation;
                         * Chest Notation: 01 08 FF FF XX XX YY YY
                         * XX = X coordinate pair
                         * YY = Y coordinate pair */
                        for (int k = 0; k < chestsInRoom.Count; k++) {
                            Entity chest = chestsInRoom[k];
                            byte[] x = chest.X, y = chest.Y;

                            // Inserting the coordinate data in 9ROM format;
                            chestBytes[4] = x[0];
                            chestBytes[5] = x[1];
                            chestBytes[6] = y[0];
                            chestBytes[7] = y[1];

                            Utils.WriteToROM(romFile, newListAddress, chestBytes);
                            //Console.WriteLine($"Chest At Address: {newListAddress}");

                            // If there are more chests in the room, continue writing them after this one;
                            newListAddress += 8;
                        }

                        // This room has been added successfully, continue;
                        lastRoomAdded = currentRoom;
                    }
                } else { // If not, the room has no chests;
                    chestsInRoomCount = 0;
                }

                // Read the next 4 bytes to detect the 9ROM reference instance;
                byte byte1 = romFile[i],
                     byte2 = romFile[i + 1],
                     byte3 = romFile[i + 2],
                     byte4 = romFile[i + 3];

                if (IsChest(byte1, byte2, byte3, byte4)) {
                    //Console.WriteLine($"Chest 9ROM Found at {i} address!");
                    i += 7;
                }
                
                else if (IsMirror(byte1, byte2, byte3, byte4)) {
                    //Console.WriteLine($"Mirror 9ROM Found at {i} address!");

                    // Read the 9ROM mirror data and inject it untouched to the ROM;
                    byte[] mirrorData = ExtractNineROMData(romFile, i, 8);

                    Utils.WriteToROM(romFile, newListAddress, mirrorData);

                    i += 7;
                    newListAddress += 8;
                } 
                
                // If it's the end of the room, inject all the chests to their respective pointers;
                else if (IsEndOfRoom(byte1, byte2, byte3, byte4)) {
                    //Console.WriteLine($"End of Room {Processing.roomIds[currentRoomIndex]} / {Utils.ConvertIntToHex(Processing.roomIds[currentRoomIndex])} 9ROM Found at {i} address!");

                    // Extract the room data and move the pointers to the new list;
                    byte[] endOfRoomData = ExtractNineROMData(romFile, i, 12),
                           writeRoomData = BitConverter.GetBytes(currentRoomAddress + NewListPointer);

                    /* Leave the "00 00 FF FF" bytes untouched and replace everything
                     * with the writeRoomData information */
                    for (int j = 4; j < writeRoomData.Length + 4; j += 1) {
                        byte bit = writeRoomData[j - 4];

                        endOfRoomData[j] = bit;
                    }

                    // Change the byte that tracks the number of chests in a room;
                    endOfRoomData[8] = (byte)chestsInRoomCount;

                    Utils.WriteToROM(romFile, newListAddress, endOfRoomData);

                    // Overwritting Pointers;
                    long pointerIndex = PointersListAddress + (4 * currentRoomIndex);
                    byte[] pointerInformation = BitConverter.GetBytes((newListAddress + 4) + NewListPointer),
                           pointerToWrite = new byte[4];

                    // Telling the pointer data to look for the new chest table;
                    for (int l = 0; l < 4; l++) {
                        pointerToWrite[l] = pointerInformation[l];
                    }

                    Utils.WriteToROM(romFile, pointerIndex, pointerToWrite);

                    // Check for the next room and continue writing in the next addresses;
                    currentRoomIndex++;
                    i += 11;
                    newListAddress += 12;
                    currentRoomAddress = newListAddress;
                }
            }
        }

        // IsChest(); Checks if an array of bytes is equal to a chest 9ROM notation;
        static bool IsChest(byte first, byte second, byte third, byte forth) {
            return (first == 0x01) && (second == 0x08) && (third == 0xFF) && (forth == 0xFF);
        }

        // IsMirror(); Checks if an array of bytes is equal to a mirror door 9ROM notation;
        static bool IsMirror(byte first, byte second, byte third, byte forth) {
            return (first == 0x02) && (second == 0x08) && (third == 0xFF) && (forth == 0xFF);
        }

        // IsMirror(); Checks if an array of bytes is equal to a end of the room 9ROM notation;
        static bool IsEndOfRoom(byte first, byte second, byte third, byte forth) {
            return (first == 0x00) && (second == 0x00) && (third == 0xFF) && (forth == 0xFF);
        }

        // ExtractNineROMData(); A function to spit out 9ROM data;
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
