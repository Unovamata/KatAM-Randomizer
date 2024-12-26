using KatAMInternal;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using static System.Runtime.InteropServices.JavaScript.JSType;

namespace KatAM_Randomizer {
    internal class KatAMMirrors : KatAMRandomizerComponent, IKatAMRandomizer {
        byte[] romFile = Processing.ROMData;

        List<Entity> mirrorEntities,
                     warpStarEntities,
                     cannonEntities,
                     hubMirrorEntities;

        Dictionary<byte, string> mirrorDictionary = Processing.mirrorsDictionary;

        public KatAMMirrors(Processing system) {
            mirrorEntities = new List<Entity>();
            warpStarEntities = new List<Entity>();
            cannonEntities = new List<Entity>();
            hubMirrorEntities = new List<Entity>();

            LoadMirrorDataset(this, Utils.mirrorsJson);

            RandomizeWarps();
        }
        
        protected void LoadMirrorDataset(IKatAMRandomizer Instance, string file) {
            entities = new List<Entity>();

            // Deserializing all the entities in the game;
            Utils.DeserializeEntitiesJSON(Utils.JSONToObjects(file), entities, Instance);
        }

        public bool FilterEntities(Entity entity) {
            // Separate mirror data as different parameters have different randomization criteria;
            if(IsRegularMirror(entity)) {
                mirrorEntities.Add(new Entity(entity));
            }
            else if(IsWarpStar(entity)) {
                warpStarEntities.Add(new Entity(entity));
            }
            else if(IsFuseCannon(entity)) {
                cannonEntities.Add(new Entity(entity));
            }
            else if(IsHubMirror(entity)) {
                hubMirrorEntities.Add(new Entity(entity));
            }

            return true;
        }

        // IsRegularMirror(); Checks if it's a traversable mirror object;
        bool IsRegularMirror(Entity entity) {
            return entity.Name == mirrorDictionary[0x6F]; // Small chest;
        }

        // IsWarpStar(); Checks if it's a warp star object;
        bool IsWarpStar(Entity entity) {
            return entity.Name == mirrorDictionary[0x84]; // Small chest;
        }

        // IsFuseCannon(); Checks if it's a star cannon object;
        bool IsFuseCannon(Entity entity) {
            return entity.Name == mirrorDictionary[0x8A]; // Small chest;
        }

        // IsHubMirror(); Checks if it's a chest object;
        bool IsHubMirror(Entity entity) {
            return entity.Name == mirrorDictionary[0x8C]; // Small chest;
        }


        //////////////////////////////////////////////////////////////////////////////////////////////
        

        void RandomizeWarps() {
            RandomizeMirrors();
            RandomizeWarpStars();
            RandomizeWarpCannons();
        }

        // 
        void RandomizeMirrors() {
            ReadROMData();
        }

        List<Mirror> mirrorList = new List<Mirror>();
        Dictionary<int, List<Mirror>> mirrorWarpDictionary = new Dictionary<int, List<Mirror>>();

        int currentRoomIndex = 0;
        bool is9ROMResetted = false;
        string addressType = "8";

        public void ReadROMData() {
            for(long i = Processing.EightROMStartAddress; i < Processing.NineROMEndAddress; i++) {
                if(!is9ROMResetted) {
                    if(i >= Processing.NineROMStartAddress) {
                        is9ROMResetted = true;
                        currentRoomIndex = 0;
                        addressType = "9";
                        Console.WriteLine("");
                        Console.WriteLine("9ROM Address Start!");
                        Console.WriteLine("");
                    }
                }

                bool isWithinRange = i >= Processing.EightROMStartAddress && i <= Processing.EightROMEndAddress ||
                                     i >= Processing.NineROMStartAddress && i <= Processing.NineROMEndAddress;

                if(!isWithinRange) continue;

                byte[] bytes = Processing.ExtractROMData(i, 14);

                bool isMirror = false, isChest = false, isEndOfRoom = false;

                switch(addressType) {
                    case "8":
                        isMirror = Processing.Is8ROMMirror(bytes);
                        isChest = Processing.Is8ROMChest(bytes);
                        isEndOfRoom = Processing.Is8ROMEmpty(bytes);
                        break;

                    case "9":
                        isMirror = Processing.Is9ROMMirror(bytes);
                        isChest = Processing.Is9ROMChest(bytes);
                        isEndOfRoom = Processing.Is9ROMEndOfRoom(bytes);
                    break;
                }

                int roomIndexChecked = Processing.roomIds[currentRoomIndex];

                if(isMirror) {
                    int warpRoomID = MirrorGetRoomID(bytes);

                    if(Processing.roomIds.Contains(warpRoomID)) {
                        byte x = bytes[6],
                             y = bytes[7];

                        Mirror mirror = new Mirror(i, x, y, warpRoomID);
                        mirror.MirrorData = Utils.ByteArrayToHexString(bytes, " ");
                        Console.WriteLine(mirror.MirrorData);

                        mirrorList.Add(mirror);

                        if(!mirrorWarpDictionary.ContainsKey(warpRoomID)) {
                            mirrorWarpDictionary[warpRoomID] = new List<Mirror>();
                            mirrorWarpDictionary[warpRoomID].Add(mirror);
                        }
                        else {
                            mirrorWarpDictionary[warpRoomID].Add(mirror);
                        }
                    }

                    i += 13;
                } else if(isChest) {
                    //Console.WriteLine($"Chest At: {Utils.ByteArrayToHexString(bytes, " ")}");

                    i += 11;
                }
                
                else if(isEndOfRoom) {
                    Console.WriteLine($"Address: {i.ToString("X2")} Checked room: {roomIndexChecked.ToString("X2")}");
                    currentRoomIndex++;
                    Console.WriteLine(" ");
                }
            }

            /*Console.WriteLine($"Total Rooms: {Processing.roomIds.Count}");
            Console.WriteLine($"Total Mirrors Found: {mirrorList.Count}");

            foreach(Mirror mirror in mirrorList) {
                int warpRoomID = mirror.RoomID;
                Mirror warpMirror = new Mirror(0, 0, 0, 0);

                Console.WriteLine($"Main Mirror: {mirror.MirrorData}");

                foreach(Mirror warp in mirrorWarpDictionary[warpRoomID]) {
                    Console.WriteLine(warp.MirrorData);
                }

                Console.WriteLine($"Mirror in room: {mirror.RoomID} warps to room: {warpMirror.RoomID}");
                Console.WriteLine($"Mirror in room: {warpMirror.RoomID} warps to room: {mirror.RoomID}");


                return;
            }*/
        }

        int MirrorGetRoomID(byte[] mirrorData) {
            // Little endian notation conversion;
            return BitConverter.ToUInt16(new byte[] { mirrorData[8], mirrorData[9] }, 0);
        }

        void RandomizeWarpStars() {
            if(Settings.WarpStarsGenerationType == GenerationOptions.Unchanged) return;

            List<Entity> warpStars = Utils.Shuffle(new List<Entity>(warpStarEntities));

            for(int i = 0; i < warpStars.Count; i++) {
                Entity entity = new Entity(warpStarEntities[i]);
                Entity shuffledEntity = warpStars[i];
                
                entity.Behavior = shuffledEntity.Behavior;
                entity.Properties = shuffledEntity.Properties;
                
                Utils.WriteObjectToROM(entity);
            }
        }

        void RandomizeWarpCannons() {
            if(Settings.FuseCannonsGenerationType == GenerationOptions.Unchanged) return;

            List<Entity> warpCannons = Utils.Shuffle(new List<Entity>(cannonEntities));

            for(int i = 0; i < warpCannons.Count; i++) {
                Entity entity = new Entity(cannonEntities[i]);
                Entity shuffledEntity = warpCannons[i];
                
                entity.Properties = shuffledEntity.Properties;

                Utils.WriteObjectToROM(entity);
            }
        }
    }
}
