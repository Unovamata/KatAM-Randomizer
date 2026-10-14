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
            InitializeComponents(system);

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
                                      NineROMMirrors = new Dictionary<int, List<Mirror>>(),
                                      allRoomMirrors = new Dictionary<int, List<Mirror>>();

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
            
            // Mirrors are added once per mirror in their destination room, so duplicates are removed;
            mirrors = mirrors.Distinct().ToList();

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

            // Every room's mirrors, before filtering. Used to build the doors and the non randomized paths;
            allRoomMirrors = EightROMMirrors.ToDictionary(kvp => kvp.Key, kvp => kvp.Value.ToList());

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
        }

        // Rooms the generation doesn't touch;
        const int CarrotCastle8CannonRoom = 0x2E0;

        /* Mini hubs; Hub rooms with mirrors to the levels of an area, reached from Central Circle (0x321)
         * or from another mini hub. Their mirrors to the levels are randomized, so they lead to a random room
         * that brings Kirby back to the same mini hub. Mirrors between hubs are never randomized;
         * 
         * Central Circle -> 0x24E, 0xBE, 0xC2, 0xC0, 0x316, 0x122, 0x1EA, 0xBF, 0x250
         * 0x24E -> 0x186, 0xC2 -> 0x24F, 0xC0 -> 0xC1, 0x122 -> 0x123, 0xBF -> 0x37A, 0x250 -> 0x2B2 */
        static readonly HashSet<int> miniHubRooms = new HashSet<int> {
            0x24E, 0xBE, 0xC2, 0xC0, 0x316, 0x122, 0x1EA, 0xBF, 0x250,
            0x186, 0x24F, 0xC1, 0x123, 0x37A, 0x2B2,
        };

        // IsMainHubRoom(); Vetoed rooms that aren't mini hubs (Central Circle, tutorial, debug & boss endurance rooms);
        static bool IsMainHubRoom(int room) {
            return Utils.IsVetoedRoom(room) && !miniHubRooms.Contains(room);
        }

        // Goal and Boss mirrors join the One-Way shuffle when the Goal Game Mirrors option is randomized;
        bool arePathEndersShuffled = false;

        bool IsShufflingPathEnders() {
            return Settings.GoalMirrorRandomization != GenerationOptions.Unchanged &&
                   Settings.GoalMirrorWarpTypeRandomization != GenerationOptions.Custom;
        }

        /* RandomizeMirrors(); Rewires the mirrors through a randomized queue. See MirrorShuffler for the logic;
         * Mirrors in Central Circle (except the starting mirror) and mirrors between hubs keep their vanilla destination; */
        void RandomizeMirrors() {
            if (Settings.MirrorRandomization == GenerationOptions.Unchanged) return;

            if (startingMirror.Destination == -1) {
                /* The starting mirror is detected by its vanilla warp (0x321 -> 0x65). If it's missing,
                 * the ROM has most likely been randomized already, so a clean ROM must be used; */
                Console.WriteLine("Starting mirror (0x321 -> 0x65) not found, mirrors won't be randomized. Please use a clean, unrandomized ROM.");
                return;
            }

            int hubRoom = startingMirror.InRoom;

            // The starting mirror may be made of several 8ROM objects sharing its 9ROM entry;
            List<Mirror> startingMirrorGroup = allRoomMirrors[hubRoom]
                .Where(m => m.Destination == startingMirror.Destination)
                .ToList();

            List<MirrorDoor> doors = BuildMirrorDoors();
            HashSet<Mirror> randomizedMirrors = new HashSet<Mirror>(doors.SelectMany(door => door.Mirrors));
            randomizedMirrors.UnionWith(startingMirrorGroup);

            Dictionary<int, List<int>> frozenEdges = new Dictionary<int, List<int>>();
            Dictionary<int, HashSet<int>> reservedDestinations = new Dictionary<int, HashSet<int>>();

            foreach (Mirror mirror in allRoomMirrors.Values.SelectMany(m => m)) {
                if (randomizedMirrors.Contains(mirror)) continue;

                // A room can't point twice to the same destination, so non randomized mirrors keep theirs reserved;
                if (!reservedDestinations.ContainsKey(mirror.InRoom)) reservedDestinations[mirror.InRoom] = new HashSet<int>();
                reservedDestinations[mirror.InRoom].Add(mirror.Destination);

                /* Central Circle mirrors are locked until their area switch is pressed, so they aren't
                 * considered as paths in the logic; */
                if (IsMainHubRoom(mirror.InRoom)) continue;

                if (!frozenEdges.ContainsKey(mirror.InRoom)) frozenEdges[mirror.InRoom] = new List<int>();
                frozenEdges[mirror.InRoom].Add(mirror.Destination);
            }

            MirrorShuffler shuffler = new MirrorShuffler(doors, frozenEdges, reservedDestinations,
                                                         new HashSet<int>(allRoomMirrors.Keys), IsMainHubRoom);

            // If selected, the starting mirror would be the 8-way cannon at Carrot Castle;
            MirrorTarget fixedStart = null;

            if (Settings.EightWayCarrotCastleStart && carrotCastle8CannonMirror.Destination != -1) {
                fixedStart = MirrorTarget.FromMirror(carrotCastle8CannonMirror);
            }

            // Else, a random safe One-Way target is used, and the vanilla starting target joins the One-Way pool;
            MirrorTarget vanillaStart = MirrorTarget.FromMirror(startingMirror);
            vanillaStart.IsSafe = true;

            bool isFullyConnected = shuffler.Shuffle(hubRoom, fixedStart, vanillaStart);

            if (shuffler.StartTarget == null) {
                Console.WriteLine($"Mirrors couldn't be linked after {shuffler.Attempts} attempts, mirrors won't be randomized.");
                return;
            }

            if (!isFullyConnected) {
                Console.WriteLine($"Warning: {shuffler.StrandedRooms} rooms can't get back to the hub in the best generated map.");
            }

            Console.WriteLine($"Mirrors linked in {shuffler.Attempts} attempts. Doors randomized: {doors.Count}");

            List<long> startingMirrorNineROMAddresses = NineROMMirrors[hubRoom]
                .Where(m => m.Destination == startingMirror.Destination)
                .Select(m => m.Address9ROM)
                .ToList();

            WriteDoorToROM(startingMirrorGroup, startingMirrorNineROMAddresses, shuffler.StartTarget);
            Console.WriteLine($"Start: Warp 0x{hubRoom:X} -> 0x{shuffler.StartTarget.Destination:X}! X: {shuffler.StartTarget.X} Y: {shuffler.StartTarget.Y}");

            foreach (MirrorDoor door in doors) {
                WriteDoorToROM(door.Mirrors, door.NineROMAddresses, door.Target);
                Console.WriteLine($"Warp 0x{door.Room:X} -> 0x{door.Target.Destination:X} (Vanilla: 0x{door.Destination:X})! X: {door.Target.X} Y: {door.Target.Y} Type: {door.ExitType}");
            }

            arePathEndersShuffled = doors.Any(door => door.Mirrors.Any(m => m.ExitType == ExitType.Goal || m.ExitType == ExitType.Boss));
        }

        /* BuildMirrorDoors(); Groups the randomizable mirrors in doors;
         * Every 8ROM object in a room pointing to the same destination shares a 9ROM entry,
         * so they are grouped together and always get the same target; */
        List<MirrorDoor> BuildMirrorDoors() {
            HashSet<Mirror> candidates = new HashSet<Mirror>(mirrors.Where(IsRandomizableMirror));

            Dictionary<(int room, int destination), MirrorDoor> doorsByKey = new Dictionary<(int room, int destination), MirrorDoor>();

            var groups = allRoomMirrors.Values
                .SelectMany(m => m)
                .Distinct()
                .GroupBy(m => (room: m.InRoom, destination: m.Destination));

            foreach (var group in groups) {
                List<Mirror> groupMirrors = group.ToList();

                // If any 8ROM object of the door can't be randomized, the whole door stays vanilla;
                if (!groupMirrors.All(candidates.Contains)) continue;

                // Goal and Boss mirrors are One-Way doors; Their targets can be placed on any One-Way door;
                ExitType ShuffleType(Mirror m) => m.ExitType == ExitType.Goal || m.ExitType == ExitType.Boss ? ExitType.OneWay : m.ExitType;

                ExitType exitType = ShuffleType(groupMirrors[0]);

                if (exitType != ExitType.OneWay && exitType != ExitType.TwoWay) continue;
                if (groupMirrors.Any(m => ShuffleType(m) != exitType)) continue;

                // Without its 9ROM entry, the 8ROM data has nowhere to point to;
                List<long> nineROMAddresses = new List<long>();

                if (NineROMMirrors.TryGetValue(group.Key.room, out List<Mirror> nineROMMirrorsInRoom)) {
                    nineROMAddresses = nineROMMirrorsInRoom
                        .Where(m => m.Destination == group.Key.destination && m.Address9ROM > 0)
                        .Select(m => m.Address9ROM)
                        .Distinct()
                        .ToList();
                }

                if (nineROMAddresses.Count == 0) continue;

                doorsByKey[group.Key] = new MirrorDoor {
                    Room = group.Key.room,
                    Destination = group.Key.destination,
                    Mirrors = groupMirrors,
                    NineROMAddresses = nineROMAddresses,
                    ExitType = exitType,
                    IsReliable = !Processing.unsafeMirrors.Contains(group.Key.room),
                    IsHubDoor = miniHubRooms.Contains(group.Key.room),
                };
            }

            List<MirrorDoor> doors = new List<MirrorDoor>();

            foreach (MirrorDoor door in doorsByKey.Values) {
                if (door.ExitType == ExitType.TwoWay) {
                    /* Two-Way doors need their vanilla partner door to be randomized as well, as the partner's data
                     * is what lands Kirby in front of this door. Otherwise, both stay vanilla; */
                    if (!doorsByKey.TryGetValue((door.Destination, door.Room), out MirrorDoor partner)) continue;
                    if (partner.ExitType != ExitType.TwoWay) continue;

                    door.Partner = partner;
                }

                doors.Add(door);
            }

            return doors;
        }

        bool IsRandomizableMirror(Mirror mirror) {
            bool isPathEnder = mirror.ExitType == ExitType.Goal || mirror.ExitType == ExitType.Boss;

            if (isPathEnder && !IsShufflingPathEnders()) return false;

            return mirror.ExitType != ExitType.Hub &&
                   !IsMainHubRoom(mirror.InRoom) &&
                   !IsMainHubRoom(mirror.Destination) &&
                   // Mirrors between hubs are never randomized;
                   !(miniHubRooms.Contains(mirror.InRoom) && miniHubRooms.Contains(mirror.Destination)) &&
                   mirror.InRoom != CarrotCastle8CannonRoom &&
                   mirror.Destination != CarrotCastle8CannonRoom &&
                   // Mirrors leading to other rooms without mirrors exit through cannons or stars, so they stay vanilla;
                   (isPathEnder || allRoomMirrors.ContainsKey(mirror.Destination));
        }

        // WriteDoorToROM(); Writes the same target to every 8ROM object of a door and its 9ROM entries;
        void WriteDoorToROM(IEnumerable<Mirror> doorMirrors, IEnumerable<long> nineROMAddresses, MirrorTarget target) {
            byte[] mirrorData = BitConverter.GetBytes(target.Destination); // Little endian;

            mirrorData[2] = target.X;
            mirrorData[3] = target.Y;

            foreach (Mirror mirror in doorMirrors) {
                Utils.WriteToROM(mirror.Address8ROM + 8, mirrorData);
                Utils.WriteToROM(mirror.Address8ROM + 12, new byte[] { target.Facing });
            }

            foreach (long nineROMAddress in nineROMAddresses.Where(address => address > 0).Distinct()) {
                Utils.WriteToROM(nineROMAddress + 4, mirrorData);
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

            // Goal and Boss mirrors were already shuffled with the rest of the One-Way mirrors;
            if (arePathEndersShuffled) return;

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
