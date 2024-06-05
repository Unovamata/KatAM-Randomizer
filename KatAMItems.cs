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
        static Settings settings;
        static int seed;

        static List<Entity> entities = new List<Entity>();
        static List<Entity> chestEntities = new List<Entity>();
        static List<byte> objectIDs = new List<byte>();

        public static void RandomizeItems(Processing system) {
            byte[] romFile = system.ROMData;
            settings = system.Settings;
            seed = settings.Seed;

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
                            case "Behavior": serialized.Behavior = (byte)kvp.Value; break;
                            case "Speed": serialized.Speed = (byte)kvp.Value; break;
                            case "Properties": serialized.Properties = kvp.Value; break;
                            case "Room": serialized.Room = (byte)kvp.Value; break;
                        }
                    }

                    Entity entity = serialized.DeserializeEntity();

                    // If it's a chest or a mirror shard;
                    if (IsVetoedObject(serialized.Name) && !IsProgressionObject(serialized.Address)) {
                        chestEntities.Add(entity);
                        objectIDs.Add(GenerateRandomConsumable());
                    } else {
                        objectIDs.Add(entity.ID);
                    }

                    entities.Add(entity);
                    

                    //Utils.ShowObjectData(entity);
                }
            }

            Console.WriteLine($"Address 0: {entities[0].Address}");

            objectIDs = objectIDs.OrderBy(x => Random.Shared.Next()).ToList();

            Console.WriteLine($"Address Shuffled: {entities[0].Address}");

            bool isRandom = false;
            

            for(int i = 0; i < entities.Count; i++) {
                Entity entity = entities[i];

                // Is name either a chest or a mirror shard? Then veto it;
                /*bool isVetoedItem = IsVetoedObject(entity.Name) || IsVetoedObject(entity.Address);
                if (isVetoedItem) {


                    continue;
                }*/

                if (isRandom) {
                    entity.ID = GenerateRandomConsumable();

                    Utils.WriteObjectToROM(romFile, entity);

                    //Utils.WriteToROM(romFile, address + 12, new byte[] { consumableItems[index] });
                } else {
                    entity.ID = objectIDs[i];

                    Utils.WriteObjectToROM(romFile, entity);
                }
            }
        }

        static bool IsVetoedObject(string name) {
            return name == Processing.itemsDictionary[0x65] ||
                   name == Processing.itemsDictionary[0x80] ||
                   name == Processing.itemsDictionary[0x81];
        }

        static bool IsProgressionObject(int address) {
            return address == 8933912 || address == 8970772 ||
                   address == 9032664 || address == 9049580 ||
                   address == 9056256;
        }

        static byte[] consumableItems = { 0x5E, 0x5F, 0x60, 0x61, 0x62, 0x63, 0x64 };
        static byte GenerateRandomConsumable() {
            int index = Utils.GetRandomNumber(0, consumableItems.Length);

            return consumableItems[index];
        }


        /*Console.WriteLine(formattedJson);*/


        /*

        var json = JsonConvert.DeserializeObject<Dictionary<string, Dataset>>(File.ReadAllText("JSON/items.json"));
        List<Entity> items = new List<Entity>();
        List<Entity> chests = new List<Entity>();

        // Iterate over each key-value pair in the dictionary
        foreach (var kvp in json) {
            Dataset dataset = kvp.Value;

            for (int i = 0; i < dataset.Address.Count; i++) {
                Entity item = new Entity();

                item.Name = kvp.Key;

                item.Address = dataset.Address[i];
                item.Room = dataset.Room[i];
                try { 
                    item.Properties = dataset.Item[i]; 
                } catch {
                    item.Properties = dataset.Item[0];
                }

                //bool isNotOverworldItem = 

                if(item.Name != "")items.Add(item);
            }*/

        /*Console.WriteLine($"Key: {kvp.Key}");

        Console.WriteLine($"Addresses: {string.Join(", ", kvp.Value.Address)}");
        Console.WriteLine(); // Add a blank line for readability

        // Access and print properties of the Entity class
        Console.WriteLine($"Addresses: {string.Join(", ", kvp.Value.Address)}");
        Console.WriteLine($"XY: {string.Join(", ", kvp.Value.XY)}");
        Console.WriteLine($"Rooms: {string.Join(", ", kvp.Value.Room)}");
        Console.WriteLine($"Items: {string.Join(", ", kvp.Value.Item)}");

        Console.WriteLine(); // Add a blank line for readability
        break;
    }*/

        /*foreach(Entity item in items) {
            //bool isIllegalItem = item.Name = 

            if (item.Address == 0) continue;

            byte[] propierties = Utils.LongToBytes(item.Properties);

            Utils.WriteToROM(system.ROMData, item.Address, propierties);



            Console.WriteLine($"{item.Name}: Address {item.Address}");

            string processedBytes = "";

            foreach (byte bit in propierties) {
                processedBytes += bit.ToString() + ",";
            }

            Console.WriteLine(processedBytes);
        }
    }*/

        /*public class Dataset {
            public List<long> Address { get; set; }
            public List<long> XY { get; set; }
            public List<long> Room { get; set; }
            public List<long> Item { get; set; }
        }*/

        /*static List<(string, int)> weightReference = new List<(string, int)>{
        ("Cherry", 4),
        ("Drink", 4),
        ("Meat", 3),
        ("Tomato", 2),
        ("Battery", 2),
        ("1Up", 1),
        ("Candy", 1),
        ("MirrorShards", 2)
    };

        static List<int> weightedItemsToRandomize = new List<int>();

        static Random random = new Random();

        static KatAMItems() {
            foreach (var item in weightReference) {
                for (int i = 0; i < item.Item2; i++) {
                    if (item.Item1 == "MirrorShards") continue;
                    weightedItemsToRandomize.AddRange(GetItem(item.Item1));
                }
            }
        }

        static void ChestlistAppend(List<List<int>> list, int room, int value) {
            if (list[room][0] == 0) {
                list[room][0] = value;
            } else {
                list[room].Add(value);
            }
        }

        public static void RandomizeItems(FileStream romFile, string randomMode) {
            Console.WriteLine("Randomizing chests and items...");

            var items = JsonConvert.DeserializeObject<Dictionary<string, Item>>(File.ReadAllText("JSON/items.json"));
            List<long> itemList = new List<long>();
            List<int> itemAdd = new List<int>();
            List<int> itemXY = new List<int>();
            List<int> itemRoom = new List<int>();
            List<List<int>> chestList = new List<List<int>>(new List<int>[287].Select(x => new List<int> { 0 }));

            foreach (var itemType in new[] { "Cherry", "Drink", "Meat", "Tomato", "Battery", "1Up", "Candy" }) {
                var item = items[itemType];
                foreach (var addr in item.Address) {
                    itemList.Add(randomMode == "Shuffle Items" ? item.ItemAddresses[0] : weightedItemsToRandomize[random.Next(weightedItemsToRandomize.Count)]);
                    itemAdd.Add(addr);
                }
                itemXY.AddRange(item.XY);
                itemRoom.AddRange(item.Room);
            }

            int mirrorWeight = weightReference.First(w => w.Item1 == "MirrorShards").Item2;
            if (mirrorWeight != 0) {
                Console.WriteLine("Adding random shard fragments in the overworld...");
                List<int> mirrorIndexList = new List<int>();
                List<long> mirrorAddressList = GetItem("MirrorShards");

                while (mirrorIndexList.Count < mirrorWeight * 8) {
                    int index = random.Next(itemList.Count);
                    if (!mirrorIndexList.Contains(index)) {
                        mirrorIndexList.Add(index);
                    }
                }

                int currentIndex = 0;
                for (int i = 0; i < mirrorIndexList.Count; i++) {
                    itemList[mirrorIndexList[i]] = mirrorAddressList[currentIndex];
                    if ((i + 1) % mirrorWeight == 0) {
                        currentIndex++;
                    }
                }
            }

            foreach (var itemType in new[] { "BigChest", "SmallChest" }) {
                var item = items[itemType];
                itemList.AddRange(item.ItemAddresses.Select(x => (long)x));
                itemAdd.AddRange(item.Address);
                itemXY.AddRange(item.XY);
                itemRoom.AddRange(item.Room);
            }

            itemList = itemList.OrderBy(x => random.Next()).ToList();

            for (int i = 0; i < itemAdd.Count; i++) {
                WriteValueToRom(romFile, itemAdd[i], itemList[i], 6);
            }

            itemList.AddRange(new long[] { 142932386250752, 142933880078354, 142933880078401, 142933880078410, 142933880078413 });
            itemAdd.AddRange(new[] { 8933912, 8970772, 9032664, 9049580, 9056256 });
            itemXY.AddRange(new long[] { 939810816, 3087036416, 671100928, 3355455488, 805326848 }.Select(x => (int)x));
            itemRoom.AddRange(new[] { 3, 81, 201, 238, 251 });

            for (int i = 0; i < itemRoom.Count; i++) {
                if (itemRoom[i] >= 4) {
                    itemRoom[i]++;
                }
            }

            foreach (var itemType in new[] { "SmallChest", "BigChest", "Unrandomized" }) {
                foreach (var item in items[itemType].ItemAddresses) {
                    int itemIndex = itemList.IndexOf(item);
                    ChestlistAppend(chestList, itemRoom[itemIndex], itemXY[itemIndex]);
                }
            }

            bool eof = false;
            int olposition = 9441164;
            int nlposition = 14745600;
            int nlroomstart = nlposition;
            int roomnumber = 0;

            Console.WriteLine("Creating new treasure table...");
            while (!eof) {
                if (olposition <= 9449747) {
                    bool endofroom = false;
                    int chestcount = 0;
                    nlroomstart = nlposition;

                    if (chestList[roomnumber][0] != 0) {
                        chestcount = chestList[roomnumber].Count;
                        foreach (var chest in chestList[roomnumber]) {
                            romFile.Seek(nlposition, SeekOrigin.Begin);
                            romFile.Write(BitConverter.GetBytes(17367039));
                            romFile.Write(BitConverter.GetBytes(chest));
                            nlposition += 8;
                        }
                    }

                    while (!endofroom) {
                        romFile.Seek(olposition, SeekOrigin.Begin);
                        int readvalue = BitConverter.ToInt32(romFile.ReadBytes(4), 0);

                        if (readvalue == 17367039) {
                            olposition += 8;
                        } else if (readvalue == 34144255) {
                            var copydata = romFile.ReadBytes(8);
                            romFile.Seek(nlposition, SeekOrigin.Begin);
                            romFile.Write(copydata, 0, 8);
                            nlposition += 8;
                            olposition += 8;
                        } else if (readvalue == 65535) {
                            olposition += 12;
                            romFile.Seek(nlposition, SeekOrigin.Begin);
                            romFile.Write(BitConverter.GetBytes(65535), 0, 4);

                            nlposition += 4;
                            romFile.Write(BitConverter.GetBytes(nlroomstart + 134217728), 0, 4);

                            romFile.Seek(13825216 + (4 * roomnumber), SeekOrigin.Begin);
                            romFile.Write(BitConverter.GetBytes(nlposition + 134217728), 0, 4);
                            nlposition += 4;

                            romFile.Seek(nlposition, SeekOrigin.Begin);
                            romFile.WriteByte((byte)chestcount);
                            nlposition += 4;

                            roomnumber++;
                            endofroom = true;
                        }
                    }
                } else {
                    eof = true;
                }
            }
        }

        static List<long> GetItem(string itemName) {
            // Placeholder method. Replace with actual implementation.
            return new List<long> { 1, 2, 3 };
        }

        static void WriteValueToRom(FileStream romFile, int address, long value, int size) {
            // Placeholder method. Replace with actual implementation.
        }

        public class Item {
            public List<int> Address { get; set; }
            public List<int> XY { get; set; }
            public List<int> Room { get; set; }
            public List<int> ItemAddresses { get; set; }
        }*/
    }
}
