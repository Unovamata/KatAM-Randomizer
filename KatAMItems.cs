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

        static List<Entity> entities = new List<Entity>();
        static List<Entity> chestEntities = new List<Entity>();
        static List<byte> objectIDs = new List<byte>();

        public static void RandomizeItems(Processing system) {
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
                            byte ID = (byte)kvp.Value;

                            serialized.ID = ID;
                            break;
                            case "Behavior": serialized.Behavior = (byte)kvp.Value; break;
                            case "Speed": serialized.Speed = (byte)kvp.Value; break;
                            case "Properties": serialized.Properties = kvp.Value; break;
                            case "Room": 
                                serialized.Room = (int) kvp.Value;


                            break;
                        }
                    }

                    Entity entity = serialized.DeserializeEntity();

                    Console.WriteLine(entity.Room);

                    // If it's not a progression object or the room is not unused, save entities to a list;
                    if (!IsProgressionObject(entity) || entity.Room != 0x0) {
                        // If it's a chest save its information for later;
                        if (IsVetoedObject(serialized.Name)) {
                            chestEntities.Add(entity);
                            objectIDs.Add(GenerateRandomConsumable());
                        } else { // If it's a consumable, save it for randomization;
                            objectIDs.Add(entity.ID);
                        }

                        entities.Add(entity);
                    }
                    

                    //Utils.ShowObjectData(entity);
                }
            }

            Console.WriteLine($"Address 0: {entities[0].Address}");

            objectIDs = objectIDs.OrderBy(x => Random.Shared.Next()).ToList();

            Console.WriteLine($"Address Shuffled: {entities[0].Address}");

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
        }

        static bool IsVetoedObject(string name) {
            return name == Processing.itemsDictionary[0x80] ||
                   name == Processing.itemsDictionary[0x81];
        }

        // This will detect big chest items that are either switches or passage-ways to not randomize them;
        static bool IsProgressionObject(Entity entity) {
            byte behavior = entity.Behavior;

            bool isProgressionObject = entity.ID == 0x81 && behavior == 0x63,
                 isMirror = entity.ID == 0x65 && behavior == 0x08;

            return isProgressionObject || isMirror;
        }

        static byte[] consumableItems = { 0x5E, 0x5F, 0x60, 0x61, 0x62, 0x63, 0x64 };
        static byte GenerateRandomConsumable() {
            int index = Utils.GetRandomNumber(0, consumableItems.Length);

            return consumableItems[index];
        }

        static long NineROMStartAddress = 9441164;
        static long NineROMEndAddress = 9441164;
        static long NewNineROMAddress = 14745600;

        public static void RandomizeChests() {
            byte[] romFile = System.ROMData;

            for(int i = 0; i < romFile.Length; i++) {
                if (i >= NineROMEndAddress) return;


            }

        }
    }
}
