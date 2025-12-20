using KatAMInternal;
using System;
using System.Collections;
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
            ReadROMData();
            RandomizeMirrors();
            RandomizeWarpStars();
            RandomizeWarpCannons();
            RandomizeGoalMirrorsToBossMirrors();
        }

        Mirror startingMirror = NullMirror(),
               carrotCastle8CannonMirror = NullMirror();
        List<Mirror> mirrors = new List<Mirror>(),
                     safeOneWayMirrors = new List<Mirror>(),
                     deadEndOneWayMirrors = new List<Mirror>(),
                     pathEndMirrors = new List<Mirror>(),
                     safeTwoWayMirrors = new List<Mirror>(),
                     deadEndTwoWayMirrors = new List<Mirror>(),
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
            safeOneWayMirrors = new List<Mirror>();
            deadEndOneWayMirrors = new List<Mirror>();
            safeTwoWayMirrors = new List<Mirror>();
            deadEndTwoWayMirrors = new List<Mirror>();
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

            // Here starts Mirror Classification;
            foreach(int key in EightROMMirrors.Keys) {
                List<Mirror> mirrorsInRoom = EightROMMirrors[key];

                foreach(Mirror mirror in mirrorsInRoom) {
                    int x = mirror.X,
                        y = mirror.Y,
                        destination = mirror.Destination,
                        inRoom = mirror.InRoom;

                    //Console.WriteLine($"Warp 0x{mirror.InRoom:X} -> 0x{mirror.Destination:X}! X: {mirror.X} Y: {mirror.Y} Type: {mirror.ExitType} Safety: {mirror.ExitSafety} Exits: {mirror.Exits.Count}");

                    List<Mirror> destination8ROMRooms = new List<Mirror>(),
                                 current9ROMRooms = new List<Mirror>();

                    current9ROMRooms = NineROMMirrors[inRoom];

                    /* To save on memory space, the 9ROM warp data is stored in a per-room
                     * basis, where if multiple warps with the same destination exist,
                     * the ROM will assign the same pointers to 1 set of data. So there
                     * may be 15 warps all pointing to the same 9ROM address, but with a different
                     * 8ROM address. Since there's only 1 9ROM reference per warp per room,
                     * it's as easy as finding the corresponding matching warp and link these 
                     * data points together;*/
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
                     * Though, it can be either a Goal, One-Way, or Boss warp;*/
                    catch {
                        ClasifyWarpType(mirror);
                        mirrors.Add(mirror);
                        continue;
                    }

                    // Reading all current room's mirrors;
                    for (int i = 0; i < destination8ROMRooms.Count; i++) {
                        Mirror destinationMirror = destination8ROMRooms[i];
                        int destination8ROM = destinationMirror.Destination;

                        mirror.Exits = EightROMMirrors[destination]
                            .GroupBy(m => new { m.InRoom, m.Destination, m.X, m.Y, m.Facing })
                            .Select(g => g.First())
                            .ToList();

                        // STEP 1: Resolve warp pairing
                        if (destination8ROM == mirror.InRoom) {
                            mirror.Warp = destinationMirror;
                            destinationMirror.Warp = mirror;
                        }

                        // STEP 2: Precompute facts
                        bool hasWarp = mirror.Warp != null;
                        bool isVetoed = Utils.IsVetoedRoom(mirror);
                        bool destVetoed = hasWarp && Utils.IsVetoedRoom(mirror.Warp);
                        bool isSelfWarp = mirror.Destination == mirror.InRoom;
                        bool isDeadEnd = mirror.Exits.Count <= 1;
                        bool unsafeMirror = IsUnsafeMirror(mirror);
                        bool mirraRoom = IsRoomWithMirra(mirror);

                        // STEP 3: Decide ExitType (single assignment)
                        if (!hasWarp) mirror.ExitType = ExitType.OneWay;
                        else if (isVetoed && destVetoed) mirror.ExitType = ExitType.Hub;
                        else if (isSelfWarp) mirror.ExitType = ExitType.OneWay;
                        else mirror.ExitType = ExitType.TwoWay;

                        // STEP 4: Decide ExitSafety (explicit priority)
                        if (mirraRoom) mirror.ExitSafety = ExitSafety.Safe;
                        else if (unsafeMirror) mirror.ExitSafety = ExitSafety.DeadEnd;
                        else if (isVetoed) mirror.ExitSafety = ExitSafety.Vetoed;
                        else if (isDeadEnd) mirror.ExitSafety = ExitSafety.DeadEnd;
                        else mirror.ExitSafety = ExitSafety.Safe;

                        mirrors.Add(mirror);

                        //
                        /*if (destination8ROM == mirror.InRoom) {
                            mirror.Warp = destinationMirror;
                            destinationMirror.Warp = mirror;

                            if(Utils.IsVetoedRoom(mirror) && Utils.IsVetoedRoom(destinationMirror)){
                                mirror.ExitType = ExitType.Hub;
                                destinationMirror.ExitType = ExitType.Hub;
                            } 
                            
                            else if (mirror.Destination == mirror.InRoom){
                                mirror.ExitType = ExitType.OneWay;
                            } 
                            
                            else {
                                mirror.ExitType = ExitType.TwoWay;
                            }
                            break;
                        }
                    }

                    if (Utils.IsVetoedRoom(mirror)) mirror.ExitSafety = ExitSafety.Vetoed;
                    else if(mirror.Exits.Count <= 1) {
                        mirror.ExitSafety = ExitSafety.DeadEnd;
                    }

                    if (mirror.Warp == null) {
                        mirror.ExitType = ExitType.OneWay;
                    }

                    if(IsUnsafeMirror(mirror)) mirror.ExitSafety = ExitSafety.DeadEnd;

                    if (IsRoomWithMirra(mirror)) mirror.ExitSafety = ExitSafety.Safe;*/
                    }
                }
            }
            
            /* Processing mirrors' safety;
             * While this can be done in the loop above. It is simpler to process it
             * in a single for loop for maintainability and readability in the future. */
            foreach (Mirror mirror in mirrors) {
                if (startingMirror.Destination == -1) {
                    bool isStartingMirror = mirror.Destination == 0x65 && mirror.InRoom == 0x321;

                    if (isStartingMirror) {
                        startingMirror = new Mirror(mirror);
                        Console.WriteLine($"Warp 0x{mirror.InRoom:X} -> 0x{mirror.Destination:X}! X: {mirror.X} Y: {mirror.Y} Type: {mirror.ExitType} Safety: {mirror.ExitSafety} Exits: {mirror.Exits.Count}");
                    }
                }

                // 8-Way Carrot Castle Start;
                if (Settings.EightWayCarrotCastleStart && carrotCastle8CannonMirror.Destination == -1) {
                    bool is8CannonMirror = mirror.Destination == 0x2E0 && mirror.InRoom == 0x2E0 && mirror.X == 0x18 && mirror.Y == 0x0E;

                    if (is8CannonMirror) {
                        if (carrotCastle8CannonMirror.Destination == -1) carrotCastle8CannonMirror = new Mirror(mirror);
                        continue;
                    }
                }
                

                switch (mirror.ExitType) {
                    case ExitType.Goal:
                    case ExitType.Boss:
                        pathEndMirrors.Add(mirror);
                    break;
                    case ExitType.OneWay:
                        if (mirror.ExitSafety == ExitSafety.Safe) safeOneWayMirrors.Add(mirror);
                        else if (mirror.ExitSafety != ExitSafety.Vetoed) deadEndOneWayMirrors.Add(mirror);
                        //Console.WriteLine($"Warp 0x{mirror.InRoom:X} -> 0x{mirror.Destination:X}! X: {mirror.X} Y: {mirror.Y} Type: {mirror.ExitType} Safety: {mirror.ExitSafety} Exits: {mirror.Exits.Count}");
                        break;

                    case ExitType.TwoWay:
                        if (mirror.ExitSafety == ExitSafety.Safe) safeTwoWayMirrors.Add(mirror);
                        else if(mirror.ExitSafety != ExitSafety.Vetoed) deadEndTwoWayMirrors.Add(mirror);
                        //Console.WriteLine($"Warp 0x{mirror.InRoom:X} -> 0x{mirror.Destination:X}! X: {mirror.X} Y: {mirror.Y} Type: {mirror.ExitType} Safety: {mirror.ExitSafety} Exits: {mirror.Exits.Count}");
                        break;
                }
            }

            bool IsRoomWithMirra(Mirror mirror) {
                return Processing.roomsWithMirras.Contains(mirror.InRoom);
            }

            bool IsUnsafeMirror(Mirror mirror) {
                return Processing.unsafeMirrors.Contains(mirror.InRoom);
            }

            void ClasifyWarpType(Mirror mirror) {
                int destinationRoom = mirror.Destination;

                // For Goal Minigame warps;
                if(IsGoalMinigameRoom(destinationRoom)) {
                    //Console.WriteLine("Minigame Goal Door Found!");
                    mirror.ExitSafety = ExitSafety.DeadEnd;
                    mirror.ExitType = ExitType.Goal;
                    goalMirrors.Add(mirror);
                }
                // For Boss room warps;
                else if(IsBossRoom(destinationRoom)) {
                    //Console.WriteLine("Boss Door Found!");
                    mirror.ExitSafety = ExitSafety.DeadEnd;
                    mirror.ExitType = ExitType.Boss;
                    bossMirrors.Add(mirror);
                }
                // For warps with no mirrors that come from a One-Way mirror;
                else {
                    mirror.ExitSafety = ExitSafety.DeadEnd;
                    mirror.ExitType = ExitType.OneWay;
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
            // Add a failsafe so it doesn't teleport you to the same room.
            // Make an algorithm to check if the seed is beatable.

            // We create another list to properly filter out duplicate mirrors in a room so its easier to shuffle them;
            EightROMMirrors = EightROMMirrors
            .ToDictionary(
                kvp => kvp.Key,
                kvp => kvp.Value
                    .Where(m => m.ExitType != ExitType.Hub)
                    .Where(m => m.InRoom != 0x2E0 && m.Destination != 0x2E0)
                    .Where(m => !Utils.IsVetoedRoom(m.InRoom))
                    .ToList()
            );
            

            /*EightROMMirrors = EightROMMirrors
            .ToDictionary(
                kvp => kvp.Key,
                kvp => {
                    var filtered = kvp.Value
                        .Where(m => m.MirrorWarpType != Exits.Hub)
                        .Where(m => m.InRoom != 0x2E0 && m.Destination != 0x2E0)
                        .Where(m => !Utils.IsVetoedRoom(m.InRoom))
                        .ToList();

                    var counts = filtered
                        .GroupBy(m => (m.Destination, m.X, m.Y))
                        .ToDictionary(g => g.Key, g => g.Count());

                    foreach (var mirror in filtered) {
                        mirror.Occurrences = counts[(mirror.Destination, mirror.X, mirror.Y)];
                    }

                    return filtered;
                }
            );*/


            // Step 1: Filter out Hub mirrors and remove duplicates based on (Destination, X, Y)
            /*List<Mirror> filteredMirrorList = EightROMMirrors
                .SelectMany(kvp => kvp.Value)
                .ToList();*/

            // Step 2: Randomize random rooms;
            /*List<int> keys = Utils.Shuffle(EightROMMirrors.Keys.ToList());

            // Step 3: Prepare shuffled mirror lists
            List<Mirror> OneWayMirrors = Utils.Shuffle(filteredMirrorList.Where(m => m.MirrorWarpType == Exits.OneWay || m.MirrorWarpType == Exits.Boss).ToList());
            int oneWayMirrorsCount = OneWayMirrors.Count;
            int currentOneWayIndex = 0;

            foreach (int key in keys) {
                List<Mirror> mirrorsInRoom = EightROMMirrors[key];

                Mirror lastMirror = NullMirror();
                if (mirrorsInRoom.Count > 0) Console.WriteLine($"* Room {key:X} *");

                foreach (Mirror mirror in mirrorsInRoom) {
                    bool isLastMirrorValid = lastMirror.Destination != -1;
                    bool isDifferentMirror = lastMirror.Address9ROM != mirror.Address9ROM;

                    Mirror newMirror = new Mirror(mirror);
                    Mirror randomMirror = new Mirror(mirror);

                    switch (mirror.MirrorWarpType) {
                        case Exits.OneWay:
                        case Exits.Boss:
                            try {
                                randomMirror = OneWayMirrors[currentOneWayIndex];
                            } catch {
                                currentOneWayIndex = 0;
                                randomMirror = OneWayMirrors[currentOneWayIndex];
                            }

                            newMirror.Destination = randomMirror.Destination;
                            newMirror.X = randomMirror.X;
                            newMirror.Y = randomMirror.Y;
                            newMirror.Facing = randomMirror.Facing;

                            Utils.WriteMirrorToROM(newMirror);

                            if (!isLastMirrorValid && isDifferentMirror) {
                                lastMirror = mirror;
                                currentOneWayIndex++;
                            }
                            break;

                        default: continue; break;
                    }
                }
            }

            List<Mirror> TwoWayMirrors = Utils.Shuffle(filteredMirrorList.Where(m => m.MirrorWarpType == Exits.TwoWay).ToList());
            int twoWayMirrorsCount = TwoWayMirrors.Count;
            int currentTwoWayIndex = 0;
            int mirrorsAlreadyRandomized = 0;*/


            /*foreach (int key in keys) {
                List<Mirror> mirrorsInRoom = EightROMMirrors[key];

                Mirror lastMirror = NullMirror();
                if(mirrorsInRoom.Count > 0) Console.WriteLine($"* Room {key:X} *");

                foreach (Mirror mirror in mirrorsInRoom) {
                    bool isLastMirrorValid = lastMirror.Destination != -1;
                    bool isDifferentMirror = lastMirror.Address9ROM != mirror.Address9ROM;

                    Mirror newMirror = new Mirror(mirror);
                    Mirror randomMirror = new Mirror(mirror);

                    switch (mirror.MirrorWarpType) {
                        case Exits.OneWay:
                        case Exits.Boss:
                            try {
                                randomMirror = OneWayMirrors[currentOneWayIndex];
                            } catch {
                                currentOneWayIndex = 0;
                                randomMirror = OneWayMirrors[currentOneWayIndex];
                            }

                            newMirror.Destination = randomMirror.Destination;
                            newMirror.X = randomMirror.X;
                            newMirror.Y = randomMirror.Y;
                            newMirror.Facing = randomMirror.Facing;
                            
                            Utils.WriteMirrorToROM(newMirror);

                            if (!isLastMirrorValid && isDifferentMirror) {
                                lastMirror = mirror;
                                currentOneWayIndex++;
                            }   
                        break;
                        
                        case Exits.TwoWay:
                            if (isDifferentMirror) {
                                lastMirror = mirror;
                                currentTwoWayIndex++;
                            }

                            randomMirror = TwoWayMirrors[currentTwoWayIndex];
                            
                            while (randomMirror.IsRandomized) {
                                currentTwoWayIndex++;
                                randomMirror = TwoWayMirrors[currentTwoWayIndex];
                            }

                            Mirror A = new Mirror(mirror), B = new Mirror(randomMirror.Warp);

                            A.UpdateMirrorData(randomMirror);
                            B.UpdateMirrorData(mirror.Warp);

                            Utils.WriteMirrorToROM(A);
                            Utils.WriteMirrorToROM(B);

                            Console.WriteLine($"{currentTwoWayIndex}: Warp 0x{mirror.InRoom:X} -> 0x{mirror.Destination:X}! X: {mirror.X} Y: {mirror.Y} Type: {mirror.MirrorWarpType} Occurences: {mirror.Occurrences}");
                            break;

                        default: continue; break;
                    }
                }
            }

            Console.WriteLine($"{currentTwoWayIndex}/{twoWayMirrorsCount} Two-Way Mirrors Used");
            Console.WriteLine($"Mirrors already randomized: {mirrorsAlreadyRandomized}");
            //Console.WriteLine("Leftover Mirrors:");

            /*while (currentTwoWayIndex < twoWayMirrorsCount) {
                Mirror mirror = TwoWayMirrors[currentTwoWayIndex];
                Console.WriteLine($"Warp 0x{mirror.InRoom:X} -> 0x{mirror.Destination:X}! X: {mirror.X} Y: {mirror.Y} Type: {mirror.MirrorWarpType}");
                currentTwoWayIndex++;
            }*/
        }

        int currentExitIndex = 0;
        Queue<Mirror> mirrorQueue = new Queue<Mirror>();

        void RandomizeMirrors() {
            List<Mirror> ShuffledSafeOneWayMirrors = Utils.Shuffle(safeOneWayMirrors);

            // Selecting the starting mirror based on the user's options;
            /*startingMirror = new Mirror(startingMirror);
             
            // If selected, the starting mirror would be the 8-way cannon at Carrot Castle;
            if(Settings.EightWayCarrotCastleStart) startingMirror.UpdateMirrorData(carrotCastle8CannonMirror);
            else {
                // Else, we get a random one way safe mirror;
                startingMirror.UpdateMirrorData(ShuffledSafeOneWayMirrors[currentExitIndex]);
                currentExitIndex++;
            }*/

            Utils.WriteMirrorToROM(startingMirror);
            Console.WriteLine($"{startingMirror.X:X2}:{startingMirror.Y:X2} Warp 0x{startingMirror.InRoom:X} -> 0x{startingMirror.Destination:X}! X: {startingMirror.X} Y: {startingMirror.Y} Type: {startingMirror.ExitType}");
            
            // Add starting mirror to the queue for traversal;
            mirrorQueue.Enqueue(startingMirror);

            // Queue that traverses all exits;
            while (mirrorQueue.Count > 0) {
                Mirror mirror = mirrorQueue.Dequeue();

                // If the mirror has already been randomized, then continue;
                if (mirror.IsQueued)
                    continue;

                mirror.SetQueuedFlag();

                // 
                foreach (Mirror exit in mirror.Exits) {
                    switch (exit.ExitType) {
                        case ExitType.OneWay:

                        break;

                        case ExitType.TwoWay:

                        break;
                    }

                    mirrorQueue.Enqueue(exit);

                    Console.WriteLine($"{exit.X:X2}:{exit.Y:X2} Warp 0x{exit.InRoom:X} -> 0x{exit.Destination:X}! X: {exit.X} Y: {exit.Y} Type: {exit.ExitType} Safety: {exit.ExitSafety}");
                }
            }
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
            bool isRandomizingMirrors = Settings.MirrorRandomization != GenerationOptions.Unchanged;

            if (!isRandomizingMirrors) return;

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
                        goalMirrorToReplace.ExitType = bossMirror.ExitType;

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
                        goalMirrorToReplace.ExitType = goalMirror.ExitType;

                        Utils.WriteMirrorToROM(goalMirrorToReplace);
                    }
                }
            }
        }
    }
}
