﻿using KatAMInternal;
using System;
using System.Collections.Generic;
using System.Diagnostics.Eventing.Reader;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using static Mirror;
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

        List<Mirror> mirrors = new List<Mirror>(),
                     bossMirrors = new List<Mirror>(),
                     goalMirrors = new List<Mirror>();
        Dictionary<int, List<Mirror>> EightROMMirrors = new Dictionary<int, List<Mirror>>(),
                                      NineROMMirrors = new Dictionary<int, List<Mirror>>();

        int currentRoomIndex = 0;
        bool is9ROMResetted = false;
        string addressType = "8";
        int ROMAdd = 13, breakerAdd = 11, dataToExtract = 14;
        int xIndex = 10, yIndex = 11;

        List<int> roomIDs = Processing.roomIDs;

        public void ReadROMData() {
            Utils.UniteVetoedRoomHashSets();

            mirrors = new List<Mirror>();
            bossMirrors = new List<Mirror>();
            goalMirrors = new List<Mirror>();
            EightROMMirrors = new Dictionary<int, List<Mirror>>();
            NineROMMirrors = new Dictionary<int, List<Mirror>>();

            for (long i = Processing.EightROMStartAddress; i < Processing.NineROMEndAddress; i++) {
                if(!is9ROMResetted) {
                    if(i >= Processing.NineROMStartAddress) {
                        is9ROMResetted = true;

                        addressType = "9";

                        currentRoomIndex = 0;
                        
                        ROMAdd = 7;
                        breakerAdd = 11;
                        dataToExtract = 8;

                        xIndex = 6;
                        yIndex = 7;
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
                        byte x = bytes[xIndex],
                             y = bytes[yIndex];

                        Mirror mirror = new Mirror(addressType, i, x, y, currentRoom, warpRoomID);
                        mirror.MirrorData = Utils.ByteArrayToHexString(bytes, " ");

                        switch(addressType) {
                            case "8":
                                mirror.Facing = bytes[12];

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

            // Here starts Mirror Randomization
            foreach(int key in EightROMMirrors.Keys) {
                List<Mirror> mirrorsInRoom = EightROMMirrors[key]; 

                foreach(Mirror mirror in mirrorsInRoom) {
                    int x = mirror.X,
                        y = mirror.Y,
                        destination = mirror.Destination,
                        inRoom = mirror.InRoom;

                    List<Mirror> destination8ROMRooms = new List<Mirror>(),
                                 current9ROMRooms = new List<Mirror>();

                    current9ROMRooms = NineROMMirrors[inRoom];

                    /* To save on memory space, the 9ROM warp data is stored in a per-room
                     * basis, where if multiple warps with the same destination exist,
                     * the ROM will assign the same pointers to 1 set of data. So there
                     * may be 15 warps all pointing to the same 9ROM address, but with a different
                     * 8ROM address. Since there's only 1 9ROM reference per warp per room,
                     * it's as easy as finding the corresponding matching warp and link these 
                     * data points together;
                     */
                    foreach(Mirror NineROMMirror in current9ROMRooms) {
                        long address9ROM = NineROMMirror.Address9ROM;
                        int x9ROM = NineROMMirror.X,
                            y9ROM = NineROMMirror.Y,
                            destination9ROM = NineROMMirror.Destination;

                        if(x == x9ROM && y == y9ROM && destination == destination9ROM) {
                            mirror.Address9ROM = address9ROM;
                        }
                    }

                    try {
                        destination8ROMRooms = EightROMMirrors[destination];
                    } 
                    /* If the destination room has no mirrors, it's a One-Way mirror;
                     * Though, it can be either a Goal, One-Way, or Boss warp;
                     */
                    catch {
                        ClasifyWarpType(mirror);
                        mirrors.Add(mirror);
                        continue;
                    }

                    for(int i = 0; i < destination8ROMRooms.Count; i++) {
                        Mirror destinationMirror = destination8ROMRooms[i];
                        int destination8ROM = destinationMirror.Destination;

                        if(destination8ROM == mirror.InRoom) {
                            mirror.Warp = destinationMirror;
                            destinationMirror.Warp = mirror;
                            if(Utils.IsVetoedRoom(mirror) && Utils.IsVetoedRoom(destinationMirror)){
                                mirror.MirrorWarpType = Exits.Hub;
                                destinationMirror.MirrorWarpType = Exits.Hub;
                                //Console.WriteLine($"Found Hub mirror {mirror.Destination:X} -> {destinationMirror.Destination:X} for mirror {mirror.Address8ROM}");
                            } else if (mirror.Destination == mirror.InRoom){
                                mirror.MirrorWarpType = Exits.OneWay;
                            } else {
                                mirror.MirrorWarpType = Exits.TwoWay;
                            }
                            //Console.WriteLine($"Found destination {destinationRoom} for mirror {mirror.Address8ROM}");
                            break;
                        }
                    }

                    if(mirror.Warp == null) mirror.MirrorWarpType = Exits.OneWay;

                    mirrors.Add(mirror);

                    /*Utils.ShowObjectData(mirror);
                    Console.WriteLine("");*/
                }
            }

            void ClasifyWarpType(Mirror mirror) {
                int destinationRoom = mirror.Destination;

                // For Goal Minigame warps;
                if(IsGoalMinigameRoom(destinationRoom)) {
                    //Console.WriteLine("Minigame Goal Door Found!");
                    mirror.MirrorWarpType = Exits.Goal;
                    goalMirrors.Add(mirror);
                }
                // For Boss room warps;
                else if(IsBossRoom(destinationRoom)) {
                    //Console.WriteLine("Boss Door Found!");
                    mirror.MirrorWarpType = Exits.Boss;
                    bossMirrors.Add(mirror);
                }
                // For warps with no doors that come from a One-Way mirror;
                else {
                    mirror.MirrorWarpType = Exits.OneWay;
                    //Console.WriteLine("One-Way Door Found!");
                }
            }

            bool IsGoalMinigameRoom(int destination) {
                // Goal rooms are 0x3D4, 0x3D5, 0x3D6;
                return destination == 0x3D4 ||
                       destination == 0x3D5 ||
                       destination == 0x3D6;
            }

            bool IsBossRoom(int destination) {
                return destination == 0x2C6 ||
                       destination == 0xDB ||
                       destination == 0x144 || // Has 2 entrances;
                       destination == 0x1A1 ||
                       destination == 0x204 ||
                       destination == 0x336 ||
                       destination == 0x81 ||
                       destination == 0x2E2;
            }

            Console.WriteLine("Mirrors found: " + mirrors.Count);

            /* Some rooms have wide doors/mirrors or mirrors linked to the same destination 
             * room warp but at different coordinates. 100% of the time, one warp will function
             * normally, while other will crash the game because of a mismatch in 
             * 8ROM and 9ROM data;
             * 
             * For the randomization to work effectively you must:
             * • Make sure the object is a wide door/linked mirror.
             * • These mirrors have the SAME warp destination in 8ROM and 9ROM respectively, destination x/y coordinates don't matter.
             * • Do not override the 9ROM warp as the 8ROM will have nowhere to point to;
             * 
             * Here's a list of everything that can be considered a wide door/linked mirror:
             * • Goal Minigame Mirrors. (2 tile warp);
             * • Wide Doors. (Ex: Carrot Castle Entrance, Ice Palace Entrance);
             * • Linked Doors pointing to the same room. (Ex: Kracko Boss Room Warp Mirrors);
             * 
             * Remember that for every 8ROM reference, there's only 1 9ROM reference warp.
             * If there's 15 8ROM objects with the same warp location, there will only be 1 9ROM 
             * data point for all these objects.
             */

            // B4 and B5 Rooms should be included;

            // We create another list to properly filter out duplicate mirrors in a room so its easier to shuffle them;
            EightROMMirrors = EightROMMirrors
            .ToDictionary(
                kvp => kvp.Key,
                kvp => kvp.Value
                    .Where(m => m.MirrorWarpType != Exits.Hub)
                    .Where(m => m.InRoom != 0x2E0 && m.Destination != 0x2E0)
                    .Where(m => !Utils.IsVetoedRoom(m.InRoom))
                    .ToList()
            );

            // Step 1: Filter out Hub mirrors and remove duplicates based on (Destination, X, Y)
            List<Mirror> filteredMirrorList = EightROMMirrors
                .SelectMany(kvp => kvp.Value)
                .GroupBy(m => (m.Destination, m.X, m.Y))  // Group by unique key
                .Select(g => g.First())                   // Keep only the first from each group
                .ToList();

            // Step 3: Prepare shuffled mirror lists
            List<Mirror> OneWayMirrors = Utils.Shuffle(filteredMirrorList.Where(m => m.MirrorWarpType == Exits.OneWay || m.MirrorWarpType == Exits.Boss).ToList());
            int oneWayMirrorsCount = OneWayMirrors.Count;
            int currentOneWayIndex = 0;

            List<Mirror> TwoWayMirrors = Utils.Shuffle(filteredMirrorList.Where(m => m.MirrorWarpType == Exits.TwoWay).ToList());
            int currentTwoWayIndex = 0;

            int entries = 1;

            // Starts in a random room and starts randomizing;
            List<int> keys = Utils.Shuffle(EightROMMirrors.Keys.ToList());

            foreach (int key in keys) {
                List<Mirror> mirrorsInRoom = EightROMMirrors[key];

                Mirror lastMirror = new Mirror("", -1, 0, 0, -1, -1);
                if(mirrorsInRoom.Count > 0) Console.WriteLine($"* Room {key:X} *");

                foreach (Mirror mirror in mirrorsInRoom) {
                    if (mirror.IsRandomized) continue;

                    bool isLastMirrorValid = lastMirror.Destination != -1;
                    bool isDifferentMirror = lastMirror.Destination != mirror.Destination || (lastMirror.X != mirror.X || lastMirror.Y != mirror.Y);

                    switch (mirror.MirrorWarpType) {
                        case Exits.OneWay:
                        case Exits.Boss:
                            Mirror newMirror = new Mirror(mirror);
                            Mirror randomMirror = new Mirror(mirror);

                            try {
                                randomMirror = OneWayMirrors[currentOneWayIndex];
                            } catch {
                                currentOneWayIndex = 0;
                                randomMirror = OneWayMirrors[currentOneWayIndex];
                                Console.WriteLine("Out of Bounds!!! Resetting Count!") ;
                            }

                            newMirror.Destination = randomMirror.Destination;
                            newMirror.X = randomMirror.X;
                            newMirror.Y = randomMirror.Y;
                            newMirror.Facing = randomMirror.Facing;
                            
                            Utils.WriteMirrorToROM(newMirror);

                            if (!isLastMirrorValid || isDifferentMirror) {
                                lastMirror = mirror;
                                currentOneWayIndex++;
                                Console.Write(">>>");
                            }
                        
                            Console.WriteLine($"Warp 0x{mirror.InRoom:X} -> 0x{mirror.Destination:X}! X: {mirror.X} Y: {mirror.Y} Type: {mirror.MirrorWarpType}");
                           
                        break;
                        
                        case Exits.TwoWay:
                            /*currentTwoWayIndex++;
                            Console.WriteLine($"Warp 0x{mirror.InRoom:X} -> 0x{mirror.Destination:X}! X: {mirror.X} Y: {mirror.Y} Type: {mirror.MirrorWarpType}");*/
                        break;

                        default: continue; break;
                    }
                }
            }

            /*Console.WriteLine($"{oneWayMirrorsCount}/{currentOneWayIndex} One-Way Mirrors Used");
            Console.WriteLine("Leftover Mirrors:");*/

            /*while (currentOneWayIndex < oneWayMirrorsCount) {
                Mirror mirror = OneWayMirrors[currentOneWayIndex];
                Console.WriteLine($"Warp 0x{mirror.InRoom:X} -> 0x{mirror.Destination:X}! X: {mirror.X} Y: {mirror.Y} Type: {mirror.MirrorWarpType}");
                currentOneWayIndex++;
            }*/

            // Swap Goal Mirrors for Boss Warp Mirrors;
            /*bool isRandomizingMirrors = Settings.MirrorRandomization != GenerationOptions.Unchanged;

            if (isRandomizingMirrors) RandomizeGoalMirrorsToBossMirrors();*/
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

        int goalMirrorIndex = 0;

        void RandomizeGoalMirrorsToBossMirrors() {
            bool isRandomizingGoalMirrors = Settings.GoalMirrorRandomization != GenerationOptions.Unchanged,
                 isSettingGoalMirrorsAsBossRooms = Settings.GoalMirrorWarpTypeRandomization == GenerationOptions.Custom;

            if(!isRandomizingGoalMirrors){
                foreach(Mirror goalMirror in goalMirrors) {
                    Utils.WriteMirrorToROM(goalMirror);
                }

                return;
            }

            Dictionary<int, List<Mirror>> goalMirrorsDictionary = goalMirrors
                .GroupBy(mirror => mirror.InRoom) // Group mirrors by the `InRoom` property
                .ToDictionary(
                    group => group.Key,           // Key selector: the room number
                    group => group.ToList()       // Value selector: list of mirrors in that room
                );

            var keys = goalMirrorsDictionary.Keys.ToList();

            if (isSettingGoalMirrorsAsBossRooms) {
                List<Mirror> shuffledBossMirrors = Utils.Shuffle(new List<Mirror>(bossMirrors));

                // Removing the duplicate Kracko warp as there are 2;
                Mirror duplicateKrackoWarp = shuffledBossMirrors.FirstOrDefault(x => x.Destination == 0x144);
                shuffledBossMirrors.Remove(duplicateKrackoWarp);

                for (int i = 0; i < keys.Count; i++) {
                    if (i % 2 == 0 && i > 0) goalMirrorIndex++;

                    int key = keys[i];

                    List<Mirror> goalMirrorsInRoom = goalMirrorsDictionary[key];

                    Mirror bossMirror = shuffledBossMirrors[goalMirrorIndex];

                    foreach (Mirror goalMirrorToReplace in goalMirrorsInRoom) {
                        goalMirrorToReplace.Destination = bossMirror.Destination;
                        goalMirrorToReplace.X = bossMirror.X;
                        goalMirrorToReplace.Y = bossMirror.Y;
                        goalMirrorToReplace.Facing = bossMirror.Facing;
                        goalMirrorToReplace.Warp = bossMirror.Warp;
                        goalMirrorToReplace.MirrorWarpType = bossMirror.MirrorWarpType;

                        Utils.WriteMirrorToROM(goalMirrorToReplace);
                    }
                }
            } else {
                List<Mirror> shuffledGoalRooms = Utils.Shuffle(new List<Mirror>(goalMirrors));

                for (int i = 0; i < keys.Count; i++) {
                    if (i % 2 == 0 && i > 0) goalMirrorIndex++;

                    int key = keys[i];

                    List<Mirror> goalMirrorsInRoom = goalMirrorsDictionary[key];

                    Mirror goalMirror = shuffledGoalRooms[goalMirrorIndex];

                    foreach (Mirror goalMirrorToReplace in goalMirrorsInRoom) {
                        goalMirrorToReplace.Destination = goalMirror.Destination;
                        goalMirrorToReplace.X = goalMirror.X;
                        goalMirrorToReplace.Y = goalMirror.Y;
                        goalMirrorToReplace.Facing = goalMirror.Facing;
                        goalMirrorToReplace.Warp = goalMirror.Warp;
                        goalMirrorToReplace.MirrorWarpType = goalMirror.MirrorWarpType;

                        Utils.WriteMirrorToROM(goalMirrorToReplace);
                    }
                }
            }
        }
    }
}
