using KatAMInternal;
using Newtonsoft.Json;
using System.Collections;
using System.Collections.Generic;
using System.Net;
using System.Reflection;
using System.Transactions;
using System.Xml.Linq;
using static System.Windows.Forms.VisualStyles.VisualStyleElement.Rebar;

namespace KatAM_Randomizer {
    internal class KatAMItems {
        static Processing System;
        static Settings Settings;
        static int seed;

        static List<Entity> entities;
        static List<Entity> chestEntities;
        static List<byte> objectIDs;

        public static void RandomizeItems(Processing system) {
            // Reset the lists if needed;
            entities = new List<Entity>();
            chestEntities = new List<Entity>();
            objectIDs = new List<byte>();

            System = system;
            Settings = system.Settings;
            byte[] romFile = System.ROMData;
            seed = Settings.Seed;

            dynamic itemsJson = Utils.JSONToEntities(Utils.itemsJson);

            /*string formattedJson = JsonConvert.SerializeObject(itemsJson, Formatting.Indented);*/

            // Print each key
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

                    // If it's not a progression object or the room is not unused, save entities to a list;
                    if (entity.Room == 0x0 || IsProgressionObject(entity)) continue;

                    // If it's a chest save its information for later;
                    if (IsVetoedObject(entity)) {
                        chestEntities.Add(new Entity(entity));
                        objectIDs.Add(GenerateRandomConsumable());
                    } else { // If it's a consumable, save it for randomization;
                        objectIDs.Add(entity.ID);
                    }

                    entities.Add(entity);
                }
            }

            objectIDs = objectIDs.OrderBy(x => Random.Shared.Next()).ToList();

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

            Console.WriteLine(chestEntities.Count);

            for(int i = 0; i < chestEntities.Count; i++) {
                Entity chest = chestEntities[i];

                int replacedEntityIndex = ReplaceChestEntity(chest);
                Entity oldEntity = entities[replacedEntityIndex];

                chest.Address = oldEntity.Address;
                chest.Number = oldEntity.Number;
                chest.Link = oldEntity.Link;
                chest.X = oldEntity.X;
                chest.Y = oldEntity.Y;
                chest.Room = oldEntity.Room;

                entities[replacedEntityIndex] = chest;

                Console.WriteLine($"Chest Replaced At Address: {chest.Address}");
                Utils.ShowObjectData(chest);

                Utils.WriteObjectToROM(romFile, chest);
            }
        }

        static int ReplaceChestEntity(Entity chest) {
            int selectedEntity = Utils.GetRandomNumber(0, entities.Count);
            Entity entity = entities[selectedEntity];

            if (IsVetoedObject(entity)) {
                selectedEntity = ReplaceChestEntity(chest);
            } 
            
            return selectedEntity;
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

        static long NineROMStartAddress = 9441164;
        static long NineROMEndAddress = 9449747;
        static long NewNineROMAddress = 14745600;
        

        public static void RandomizeChests() {
            byte[] romFile = System.ROMData;
            int currentRoom = 0,
                currentChestsInRoom = 0;
            long currentRoomAddress = NineROMStartAddress,
                 newListAddress = NewNineROMAddress;

            for (long i = NineROMStartAddress; i < romFile.Length; i += 1) {
                if (i >= NineROMEndAddress || i >= romFile.Length) return;

                byte byte1 = romFile[i],
                     byte2 = romFile[i + 1],
                     byte3 = romFile[i + 2],
                     byte4 = romFile[i + 3];

                if (IsChest(byte1, byte2, byte3, byte4)) {
                    //Console.WriteLine($"Chest 9ROM Found at {i} address!");
                    i += 7;
                } else if(IsMirror(byte1, byte2, byte3, byte4)) {
                    //Console.WriteLine($"Mirror 9ROM Found at {i} address!");
                    i += 7;
                } else if(IsEndOfRoom(byte1, byte2, byte3, byte4)){
                    //Console.WriteLine($"End of Room {Processing.roomIds[currentRoom]} / {Utils.ConvertIntToHex(Processing.roomIds[currentRoom])} 9ROM Found at {i} address!");
                    currentRoom++;
                    i += 11;
                    currentRoomAddress = i;
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
    }
}
