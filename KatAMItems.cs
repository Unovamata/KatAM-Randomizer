using KatAMInternal;
using KatAM_Randomizer;

namespace KatAMRandomizer
{
    internal class KatAMItems : KatAMRandomizerComponent, IKatAMRandomizer {
        List<Entity> chestEntities;
        List<byte> objectIDs;
        GenerationOptions consumableOptions;

        public KatAMItems(Processing system) {
            InitializeComponents(system);

            RandomizeItems(this);
            RandomizeChests();
        }

        public void RandomizeItems(KatAMItems Instance) {
            entities = new List<Entity>();
            chestEntities = new List<Entity>();
            objectIDs = new List<byte>();

            // Read the settings, entities, and ROM data;
            byte[] romFile = System.ROMData;
            consumableOptions = Settings.ConsumablesGenerationType;

            // Deserializing all the entities in the game;
            Utils.DeserializeEntitiesJSON(Utils.JSONToObjects(Utils.itemsJson), entities, Instance);

            // Shuffling the object IDs;
            bool isShuffling = consumableOptions == GenerationOptions.Shuffle;

            if (isShuffling) {
                objectIDs = Utils.Shuffle(objectIDs);
            }

            // Inject 
            for(int i = 0; i < Settings.amountOfMirrorShardsToAdd; i++) {
                // Randomize the mirror shard behaviours;
                for (int j = 0; j < 8; j++) {
                    ShuffleMirrorShards((byte) j);
                }
            }


            for (int i = 0; i < entities.Count; i++) {
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
                        // Big chest at the start, if removed it can softlock the entire seed;
                        if (entity.ID == 0x81 && entity.Behavior == 0x0A) continue;

                        bool canSpawnItem = Utils.Dice() == 1;

                        // If the item can spawn, spawn a cherry;
                        if (canSpawnItem) {
                            entity.ID = consumableItems[0];
                        } else { // If not, don't spawn anything;
                            entity.ID = Utils.Nothing;
                        }
                    break;

                    // Spawn no consumables;
                    case GenerationOptions.No:
                        entity.ID = Utils.Nothing;
                    break;
                }

                Utils.WriteObjectToROM(romFile, entity);
            }
        }

        public bool FilterEntities(Entity entity) {
            // Save entities to a list if the objects are randomizeable;
            if (Utils.IsVetoedRoom(entity)) return false;
            if (entity.Name == "Mirror Shard") return false;

            // If it's a chest save its information for later;
            if (IsChestObject(entity) || IsProgressionObject(entity)) {
                chestEntities.Add(new Entity(entity));
                objectIDs.Add(GenerateRandomConsumable());
            } else { // If it's a consumable, save it for randomization;
                objectIDs.Add(entity.ID);
            }

            return true;
        }

        // ShuffleMirrorShard(); Take an entity and convert it to a mirror shard;
        void ShuffleMirrorShards(byte currentShardBehaviour) {
            if (currentShardBehaviour >= 8) return;

            int index = ReturnReplaceableEntity();

            Entity oldEntity = entities[index];
            Entity mirrorShard = new Entity(oldEntity);

            mirrorShard.Name = "Mirror Shard";
            mirrorShard.ID = 0x65; // Mirror shard;
            mirrorShard.Behavior = currentShardBehaviour; //currentShardBehaviour; // 0x0 - 0x7 behaviors = collectable shards;

            entities[index] = mirrorShard;
            objectIDs[index] = mirrorShard.ID;
            currentShardBehaviour++;
        }

        //ReplaceChestEntity(); Selects a random object to replace it for a chest in the entity list;
        int ReturnReplaceableEntity() {
            int selectedEntity = Utils.GetRandomNumber(0, entities.Count);
            Entity entity = entities[selectedEntity];

            // If the entity chosen is a chest, reroll;
            do {
                selectedEntity = Utils.GetRandomNumber(0, entities.Count);
                entity = entities[selectedEntity];
            } while (IsChestObject(entity) || IsMirrorObject(entity));

            return selectedEntity;
        }

        // IsChestObject(); Checks if it's a chest object;
        bool IsChestObject(Entity entity) {
            string name = entity.Name;

            return name == Processing.itemsDictionary[0x80] || // Small chest;
                   name == Processing.itemsDictionary[0x81]; // Big Chest;
        }

        // IsProgressionObject(); This will not randomize key progression objects;
        bool IsProgressionObject(Entity entity) {
            byte behavior = entity.Behavior;

            // Big chest with switch parameter or mirror;
            bool isProgressionObject = (entity.ID == 0x81 && behavior == 0x63) || (entity.ID == 0x65 && entity.Behavior == 0x08);

            return isProgressionObject;
        }

        // GenerateRandomConsumable(); Generates a random consumable ID ;
        byte[] consumableItems = { 0x5E, 0x5F, 0x60, 0x61, 0x62, 0x63, 0x64 };
        byte GenerateRandomConsumable() {
            int index = Utils.GetRandomNumber(0, consumableItems.Length);

            return consumableItems[index];
        }

        // IsMirrorObject(); Checks if it's a mirror object;
        bool IsMirrorObject(Entity entity) {
            string name = entity.Name;

            return name == Processing.itemsDictionary[0x65]; // Mirror Shard;
        }


        //////////////////////////////////////////////////////////////////////////////////////////////


        Dictionary<int, List<Entity>> chestDictionary;
        GenerationOptions chestGeneration, chestProperties;
        bool isAddingHPUps = false;
        int HPUpsToAdd = 0;
        int HPUpsAdded = 0;

        // Banned rooms like debug, boss endurance, or final boss rooms;
        List<byte> chestConsumableIDs = new List<byte>{
                0x0, 0x1, 0x2, 0x3, 0x4, 0x5
        };

        List<byte> chestTreasureIDs = new List<byte>{
                0x06, // HP UPs;
                0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, // Maps;
                0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, // Sprays
                0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, // Sprays
                0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F // Music
        };

        List<byte> combinedTreasureIDs = new List<byte>();

        // RandomizeChests(); Randomizes chest objects and injects them in the ROM's 9ROM pointers;
        public void RandomizeChests() {
            byte[] romFile = System.ROMData;
            chestDictionary = new Dictionary<int, List<Entity>>();
            chestGeneration = Settings.ChestsGenerationType;
            chestProperties = Settings.ChestsPropertiesType;
            isAddingHPUps = Settings.isAddingMoreHPUps;

            if (isAddingHPUps) HPUpsToAdd = Settings.HPUpsAdded;

            // Return if no chests must be written;
            if (chestGeneration == GenerationOptions.No) return;
            else if (chestProperties == GenerationOptions.Random) {
                combinedTreasureIDs.AddRange(chestConsumableIDs);
                combinedTreasureIDs.AddRange(chestTreasureIDs);
            }

            // For every chest in the list, randomize it;
            for (int i = 0; i < chestEntities.Count; i++) {
                Entity chest = chestEntities[i];

                if (IsProgressionObject(chest)) {
                    AddChestToDictionary(chest);
                    continue;
                }

                // If the randomizer is removing healing items, then don't process the chest;
                if (chestProperties == GenerationOptions.Remove) {
                    bool isContainingVetoedItem = chestConsumableIDs.Contains(chest.Behavior);

                    if (isContainingVetoedItem) {
                        // Adding HP Ups;
                        bool canAddHPUps = isAddingHPUps && HPUpsAdded < HPUpsToAdd,
                             isHPUp = chest.Behavior == 0x06,
                             isTreasure = chestTreasureIDs.Contains(chest.Behavior);

                        if (canAddHPUps && !isTreasure) {
                            chest.Behavior = 0x06;
                            HPUpsAdded++;
                            AddChestToDictionary(chest);
                        }

                        continue;
                    }

                    // Shuffling chests;
                    ShuffleChest(chest);
                }

                AddChestToDictionary(chest);
                Utils.WriteObjectToROM(romFile, chest);

                // Write to the ROM;
                WriteChestTo9ROM(romFile);
            }
        }

        // AddChestToDictionary(); Adds a chest entity to a dictionary for later processing;
        void AddChestToDictionary(Entity chest) {
            if (!chestDictionary.ContainsKey(chest.Room)) {
                chestDictionary[chest.Room] = new List<Entity>();
            }

            chestDictionary[chest.Room].Add(chest);
        }

        // ShuffleChest(); Take a chest entity and select a random entity to shuffle it to;
        void ShuffleChest(Entity chest) {
            if (chestGeneration != GenerationOptions.Shuffle) return;

            int replacedEntityIndex = ReturnReplaceableEntity();
            Entity oldEntity = entities[replacedEntityIndex];

            chest.Address = oldEntity.Address;
            chest.Number = oldEntity.Number;
            chest.Link = oldEntity.Link;
            chest.X = oldEntity.X;
            chest.Y = oldEntity.Y;
            chest.ID = 0x80; // Assign small chests since big chests fall of the level;
            chest.Room = oldEntity.Room;

            // Do not shuffle the base HP Ups IDs;
            if (chest.Behavior == 0x06) return;

            switch (chestProperties) {
                case GenerationOptions.RandomAndPresets:
                    int selectedTreasureIndex = Utils.GetRandomNumber(0, chestTreasureIDs.Count);

                    chest.Behavior = chestTreasureIDs[selectedTreasureIndex];
                break;

                case GenerationOptions.Random:
                    int selectedIndex = Utils.GetRandomNumber(0, combinedTreasureIDs.Count);

                    chest.Behavior = combinedTreasureIDs[selectedIndex];
                break;
            }
        }

        // Pointers;
        long NewNineROMAddress = 14745600,
             PointersListAddress = 13825216,
             NewListPointer = 134217728;

        // WriteChestTo9ROM(); Writing the object data in a format the ROM can understand;
        void WriteChestTo9ROM(byte[] romFile) {
            // Pointers to inspect the data correctly;
            int currentRoomIndex = 0,
                chestsInRoomCount = 0,
                lastRoomAdded = -1;

            long currentRoomAddress = NewNineROMAddress,
                 newListAddress = NewNineROMAddress;

            // Base empty chest;
            byte[] chestBytes = new byte[] { 0x01, 0x08, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, };

            // Read and Overwritte the data for all 9ROM addresses found;
            for (long i = Processing.NineROMStartAddress; i < romFile.Length; i += 1) {
                if (i >= Processing.NineROMEndAddress || i >= romFile.Length) return;

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

                if (Processing.IsChest(byte1, byte2, byte3, byte4)) {
                    //Console.WriteLine($"Chest 9ROM Found at {i} address!");
                    i += 7;
                }
                
                else if (Processing.IsMirror(byte1, byte2, byte3, byte4)) {
                    //Console.WriteLine($"Mirror 9ROM Found at {i} address!");

                    // Read the 9ROM mirror data and inject it untouched to the ROM;
                    byte[] mirrorData = Processing.ExtractNineROMData(romFile, i, 8);

                    Utils.WriteToROM(romFile, newListAddress, mirrorData);

                    i += 7;
                    newListAddress += 8;
                } 
                
                // If it's the end of the room, inject all the chests to their respective pointers;
                else if (Processing.IsEndOfRoom(byte1, byte2, byte3, byte4)) {
                    //Console.WriteLine($"End of Room {Processing.roomIds[currentRoomIndex]} / {Utils.ConvertIntToHex(Processing.roomIds[currentRoomIndex])} 9ROM Found at {i} address!");

                    // Extract the room data and move the pointers to the new list;
                    byte[] endOfRoomData = Processing.ExtractNineROMData(romFile, i, 12),
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
    }
}
