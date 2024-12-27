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
        Dictionary<int, List<Mirror>> EightROMMirrors = new Dictionary<int, List<Mirror>>(),
                                      NineROMMirrors = new Dictionary<int, List<Mirror>>();

        int currentRoomIndex = 0;
        bool is9ROMResetted = false;
        string addressType = "8";
        int ROMAdd = 13, breakerAdd = 11, dataToExtract = 14;

        List<int> roomIDs = Processing.roomIDs;

        public void ReadROMData() {
            for(long i = Processing.EightROMStartAddress; i < Processing.NineROMEndAddress; i++) {
                if(!is9ROMResetted) {
                    if(i >= Processing.NineROMStartAddress) {
                        is9ROMResetted = true;
                        currentRoomIndex = 0;
                        addressType = "9";
                        ROMAdd = 7;
                        breakerAdd = 11;
                        dataToExtract = 8;
                        /*Console.WriteLine("");
                        Console.WriteLine("9ROM Address Start!");
                        Console.WriteLine("");*/
                    }
                }

                bool isWithinRange = i >= Processing.EightROMStartAddress && i <= Processing.EightROMEndAddress ||
                                     i >= Processing.NineROMStartAddress && i <= Processing.NineROMEndAddress;

                if(!isWithinRange) continue;

                byte[] bytes = Processing.ExtractROMData(i, dataToExtract);

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

                int currentRoom = roomIDs[currentRoomIndex];

                if(currentRoom == 0x0) currentRoom = roomIDs[currentRoomIndex + 1];

                if(isMirror) {
                    int warpRoomID = MirrorGetRoomID(bytes, addressType);

                    if(Processing.roomIDs.Contains(currentRoom)) {
                        byte x = bytes[6],
                             y = bytes[7];

                        Mirror mirror = new Mirror(addressType, i, x, y, currentRoom, warpRoomID);
                        mirror.MirrorData = Utils.ByteArrayToHexString(bytes, " ");

                        switch(addressType) {
                            case "8":
                                mirror.Facing = bytes[12];
                                mirrorList.Add(mirror);

                                if(!EightROMMirrors.ContainsKey(currentRoom)) {
                                    EightROMMirrors[currentRoom] = new List<Mirror>();
                                    EightROMMirrors[currentRoom].Add(mirror);
                                }
                                else {
                                    EightROMMirrors[currentRoom].Add(mirror);
                                }
                            break;

                            case "9":
                                if(!NineROMMirrors.ContainsKey(currentRoom)) {
                                    NineROMMirrors[currentRoom] = new List<Mirror>();
                                    NineROMMirrors[currentRoom].Add(mirror);
                                }
                                else {
                                    NineROMMirrors[currentRoom].Add(mirror);
                                }
                            break;
                        }

                        /*Utils.ShowObjectData(mirror);
                        Console.WriteLine(" ");*/
                    }

                    i += ROMAdd;
                } 
                else if(isChest) { 
                    switch(addressType) {
                        case "8": i += breakerAdd; break;
                        case "9": i += ROMAdd; break;
                    }
                }
                else if(isEndOfRoom) {
                    /*Console.WriteLine($"Address: {i.ToString("X2")} Checked room: {currentRoom.ToString("X2")}");
                    Console.WriteLine(" ");*/

                    currentRoomIndex++;
                    i += breakerAdd;
                }
            }

            foreach(int key in EightROMMirrors.Keys) {
                //Console.WriteLine($"Checking room {key}...");

                List<Mirror> mirrorsInRoom = EightROMMirrors[key]; 

                foreach(Mirror mirror in mirrorsInRoom) {
                    int destinationRoom = mirror.Destination;

                    List<Mirror> dest8ROMRooms = new List<Mirror>(),
                                 dest9ROMRooms = new List<Mirror>();

                    try {
                        dest8ROMRooms = EightROMMirrors[destinationRoom];
                        dest9ROMRooms = NineROMMirrors[destinationRoom];
                    } 
                    /* If the destination room has no mirrors, it's a One-Way mirror;
                     * Though, it can be either a Goal, One-Way, or Boss warp;
                     */
                    catch {
                        ClasifyWarpType(mirror);
                        continue;
                    }

                    for(int i = 0; i < dest8ROMRooms.Count; i++) {
                        Mirror destinationMirror = dest8ROMRooms[i];
                        int destination = destinationMirror.Destination;

                        if(destination == mirror.InRoom) {
                            mirror.Warp = destinationMirror;
                            destinationMirror.Warp = mirror;
                            mirror.MirrorWarpType = Exits.TwoWay;
                            //Console.WriteLine($"Found destination {destinationRoom} for mirror {mirror.Address8ROM}");
                            break;
                        }
                    }

                    if(mirror.Warp == null) {
                        mirror.MirrorWarpType = Exits.OneWay;
                        //Console.WriteLine("One-Way Mirror Detected!");
                    } else {
                        //Console.WriteLine($"{mirror.Destination} -> {mirror.Warp.Destination}");
                    }

                    
                    //Utils.ShowObjectData(mirror.Warp);
                    //Console.WriteLine("");
                }

                //return;
            }

            void ClasifyWarpType(Mirror mirror) {
                int destinationRoom = mirror.Destination;

                // For Goal Minigame warps;
                if(IsGoalMinigameRoom(destinationRoom)) {
                    Console.WriteLine("Minigame Goal Door Found!");
                    mirror.MirrorWarpType = Exits.Goal;
                }
                // For Boss room warps;
                else if(IsBossRoom(destinationRoom)) {
                    Console.WriteLine("Boss Door Found!");
                    mirror.MirrorWarpType = Exits.Boss;
                }
                // For warps with no doors that come from a One-Way mirror;
                else {
                    mirror.MirrorWarpType = Exits.OneWay;
                    Console.WriteLine("One-Way Door Found!");
                }
            }

            bool IsGoalMinigameRoom(int destination) {
                // Goal rooms are 0x3D4, 0x3D5, 0x3D6;
                return destination == 0x3D4 ||
                       destination == 0x3D5 ||
                       destination == 0x3D6;
            }

            bool IsBossRoom(int destination) {
                // Goal rooms are 0x3D4, 0x3D5, 0x3D6;
                return destination == 0x2C6 ||
                       destination == 0xDB ||
                       destination == 0x144 || // Has 2 entrances;
                       destination == 0x1A1 ||
                       destination == 0x204 ||
                       destination == 0x336 ||
                       destination == 0x81 ||
                       destination == 0x2E2;
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

        int MirrorGetRoomID(byte[] mirrorData, string ROMSpace) {
            int roomID = 0;

            switch(ROMSpace) {
                case "8":
                    roomID = BitConverter.ToUInt16(new byte[] { mirrorData[8], mirrorData[9] }, 0);
                break;

                case "9":
                    roomID = BitConverter.ToUInt16(new byte[] { mirrorData[4], mirrorData[5] }, 0);
                break;
            }

            // Little endian notation conversion;
            return roomID;
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
