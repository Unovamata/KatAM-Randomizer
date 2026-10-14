using KatAMInternal;
using System;
using System.Collections.Generic;
using System.Linq;

namespace KatAM_Randomizer {
    /* MirrorTarget; Where a mirror sends Kirby. This is the data written to 8ROM (+8..+12) and 9ROM (+4..+7):
     * the destination room, the X/Y coordinates Kirby lands at, and the facing direction; */
    internal class MirrorTarget {
        public int Destination { get; set; }
        public byte X { get; set; }
        public byte Y { get; set; }
        public byte Facing { get; set; }

        // For Two-Way mirrors, the door Kirby arrives at. Null for One-Way targets;
        public MirrorDoor ArrivalDoor { get; set; }

        // True if the original mirror holding this target was classified as safe;
        public bool IsSafe { get; set; }

        public static MirrorTarget FromMirror(Mirror mirror) {
            return new MirrorTarget {
                Destination = mirror.Destination,
                X = mirror.X,
                Y = mirror.Y,
                Facing = mirror.Facing,
                IsSafe = mirror.ExitSafety == ExitSafety.Safe,
            };
        }
    }

    /* MirrorDoor; A physical door in a room;
     * Wide doors and linked doors are made of several 8ROM objects that share a single 9ROM entry
     * per room/destination. If these get different destinations, the 8ROM and 9ROM data mismatch and the
     * game crashes, so every 8ROM object in a room pointing to the same destination is grouped
     * in one door and always randomized together; */
    internal class MirrorDoor {
        public int Room { get; set; }
        public int Destination { get; set; }
        public List<Mirror> Mirrors { get; set; } = new List<Mirror>();
        public List<long> NineROMAddresses { get; set; } = new List<long>();
        public ExitType ExitType { get; set; }

        // Doors in rooms listed as unsafe may not be reachable, so the logic never relies on them to keep exploring;
        public bool IsReliable { get; set; } = true;

        // Vanilla door at the other side of a Two-Way mirror. Its data is what lands Kirby in front of this door;
        public MirrorDoor Partner { get; set; }

        // Randomized target. Null while the door hasn't been used;
        public MirrorTarget Target { get; set; }

        public Mirror Representative => Mirrors[0];
        public MirrorTarget VanillaTarget => MirrorTarget.FromMirror(Representative);

        // Target that lands Kirby in front of this door;
        public MirrorTarget Arrival {
            get {
                MirrorTarget arrival = Partner.VanillaTarget;
                arrival.ArrivalDoor = this;
                return arrival;
            }
        }
    }

    /* MirrorShuffler; Rewires the mirrors keeping a web of logical navigation like the original map;
     *
     * • Mirrors keep their type: Two-Way doors are paired with other Two-Way doors, and One-Way doors
     *   receive One-Way targets.
     * • Two-Way doors are paired symmetrically. If door A in room 1 leads to door B in room 2, then door B
     *   leads back to door A in room 1.
     * • A randomized queue holds every unused door in the rooms reachable so far. A door is picked at random,
     *   linked, and removed from the queue; Targets are removed from their pool once used. Reaching a new room
     *   adds its doors to the queue.
     * • While unreached rooms remain, every link leads to a new room, and a door is never linked in a way that
     *   leaves the queue without a reliable door to continue from. Dead ends are only placed while other exits
     *   remain open. Once every room is reachable, the doors left in the queue are linked between themselves.
     * • A room never holds two mirrors pointing to the same destination room, as that causes the 8ROM/9ROM mismatch;
     * • Once every door is linked, each room must be reachable from the hub and be able to get back to it,
     *   either through mirrors or through a path ender (Goal, Boss). Otherwise, the generation is attempted again; */
    internal class MirrorShuffler {
        const int MaxAttempts = 500;

        // When the queue holds this many reliable doors or fewer, links that reach new rooms are preferred;
        const int LowFrontier = 2;

        // Node that represents the hub rooms in the logic graph;
        const int HubNode = -2;

        readonly List<MirrorDoor> doors;
        readonly Dictionary<int, List<MirrorDoor>> doorsByRoom;
        readonly Dictionary<int, List<int>> frozenEdges;
        readonly Dictionary<int, HashSet<int>> reservedDestinations;
        readonly HashSet<int> roomsWithMirrors;
        readonly Func<int, bool> isHubRoom;

        HashSet<int> reachedRooms, pendingRooms, entryRooms;
        Dictionary<int, HashSet<int>> usedDestinations;
        List<MirrorDoor> doorQueue, twoWayPool;
        List<MirrorTarget> oneWayPool;
        int reliableDoorsInQueue;

        public MirrorTarget StartTarget { get; private set; }
        public int Attempts { get; private set; }
        public int StrandedRooms { get; private set; }

        /* doors: Every door to randomize;
         * frozenEdges: Destinations of the mirrors that won't be randomized, per room. Mirrors in hub rooms are excluded,
         *              as most of them are locked until their area switch is pressed;
         * reservedDestinations: Destinations used by non randomized mirrors, per room;
         * roomsWithMirrors: Every room that holds at least one mirror;
         * isHubRoom: Whether a room is a hub (vetoed) room; */
        public MirrorShuffler(List<MirrorDoor> doors, Dictionary<int, List<int>> frozenEdges,
                              Dictionary<int, HashSet<int>> reservedDestinations, HashSet<int> roomsWithMirrors,
                              Func<int, bool> isHubRoom) {
            this.doors = doors;
            this.frozenEdges = frozenEdges;
            this.reservedDestinations = reservedDestinations;
            this.roomsWithMirrors = roomsWithMirrors;
            this.isHubRoom = isHubRoom;

            doorsByRoom = doors.GroupBy(door => door.Room).ToDictionary(group => group.Key, group => group.ToList());
        }

        /* Shuffle(); Returns true if every room can be reached from the hub and get back to it;
         * hubRoom: Room holding the starting mirror;
         * fixedStart: Forced target for the starting mirror (8-Way Carrot Castle start). Null to pick a random safe One-Way target;
         * vanillaStart: Vanilla target of the starting mirror; Joins the One-Way pool when the start is random; */
        public bool Shuffle(int hubRoom, MirrorTarget fixedStart, MirrorTarget vanillaStart) {
            Dictionary<MirrorDoor, MirrorTarget> bestAssignment = null;
            MirrorTarget bestStart = null;
            int bestStranded = int.MaxValue;

            /* Entry rooms; Rooms no randomized mirror can lead to, as they have no Two-Way doors and no One-Way target
             * lands in them. The game enters these through hub mirrors or non mirror objects (Ex: Rooms 0xB4, 0xB5),
             * which aren't randomized, so the logic doesn't need to reach them. Their doors are still randomized; */
            HashSet<int> oneWayDestinations = new HashSet<int>(doors
                .Where(door => door.ExitType == ExitType.OneWay)
                .Select(door => door.Destination));

            if (fixedStart == null) oneWayDestinations.Add(vanillaStart.Destination);

            entryRooms = new HashSet<int>(doorsByRoom
                .Where(kvp => !oneWayDestinations.Contains(kvp.Key) && kvp.Value.All(door => door.ExitType != ExitType.TwoWay))
                .Select(kvp => kvp.Key));

            for (Attempts = 1; Attempts <= MaxAttempts; Attempts++) {
                if (!TryLinkDoors(hubRoom, fixedStart, vanillaStart)) continue;

                int stranded = CountStrandedRooms();

                if (stranded < bestStranded) {
                    bestStranded = stranded;
                    bestStart = StartTarget;
                    bestAssignment = doors.ToDictionary(door => door, door => door.Target);
                }

                if (stranded == 0) break;
            }

            Attempts = Math.Min(Attempts, MaxAttempts);

            if (bestAssignment == null) {
                foreach (MirrorDoor door in doors) door.Target = null;
                StartTarget = null;
                StrandedRooms = -1;
                return false;
            }

            foreach (MirrorDoor door in doors) door.Target = bestAssignment[door];
            StartTarget = bestStart;
            StrandedRooms = bestStranded;

            return bestStranded == 0;
        }

        bool TryLinkDoors(int hubRoom, MirrorTarget fixedStart, MirrorTarget vanillaStart) {
            foreach (MirrorDoor door in doors) door.Target = null;

            usedDestinations = reservedDestinations.ToDictionary(kvp => kvp.Key, kvp => new HashSet<int>(kvp.Value));
            twoWayPool = doors.Where(door => door.ExitType == ExitType.TwoWay).ToList();
            oneWayPool = doors.Where(door => door.ExitType == ExitType.OneWay).Select(door => door.VanillaTarget).ToList();
            reachedRooms = new HashSet<int>();
            pendingRooms = new HashSet<int>(doorsByRoom.Keys.Where(room => !entryRooms.Contains(room)));
            doorQueue = new List<MirrorDoor>();
            reliableDoorsInQueue = 0;

            // Selecting the starting mirror's target;
            if (fixedStart != null) StartTarget = fixedStart;
            else {
                // The vanilla start target takes the place of the One-Way target taken by the starting mirror;
                oneWayPool.Add(vanillaStart);

                List<MirrorTarget> startCandidates = oneWayPool
                    .Where(target => target.IsSafe && CanLink(hubRoom, target.Destination) && NewReliableDoors(target.Destination, null) > 0)
                    .ToList();

                if (startCandidates.Count == 0) return false;

                StartTarget = startCandidates[Utils.GetRandomRange(0, startCandidates.Count)];
                oneWayPool.Remove(StartTarget);
            }

            MarkDestinationUsed(hubRoom, StartTarget.Destination);
            ReachRoom(StartTarget.Destination);

            /* Exploration; Doors are picked at random from the queue and linked to unreached rooms,
             * until every room is reachable. Doors that can't reach a new room yet stay in the queue; */
            while (pendingRooms.Count > 0) {
                if (!LinkDoorToNewRoom()) return false;
            }

            // Entry rooms are reachable through their vanilla entrances, so their doors join the queue now;
            foreach (int entryRoom in entryRooms) ReachRoom(entryRoom);

            /* Every room is reachable now. The doors left in the queue are picked at random
             * and linked to any of the remaining doors/targets; */
            while (doorQueue.Count > 0) {
                MirrorDoor door = TakeFromQueue(Utils.GetRandomRange(0, doorQueue.Count));

                // Two-Way doors may have been linked already from the other side;
                if (door.Target != null) continue;

                if (!LinkDoor(door, false)) return false;
            }

            // Every door must be used;
            return doors.All(door => door.Target != null);
        }

        /* LinkDoorToNewRoom(); Links a random door from the queue to an unreached room;
         * Doors in unsafe rooms are only used to explore when no other door can; */
        bool LinkDoorToNewRoom() {
            foreach (bool allowUnsafeDoors in new[] { false, true }) {
                // Lazy Fisher-Yates shuffle over the queue, so doors are visited in random order;
                for (int k = 0; k < doorQueue.Count; k++) {
                    int j = Utils.GetRandomRange(k, doorQueue.Count);
                    (doorQueue[k], doorQueue[j]) = (doorQueue[j], doorQueue[k]);

                    MirrorDoor door = doorQueue[k];

                    if (door.Target != null || door.IsReliable == allowUnsafeDoors) continue;

                    if (LinkDoor(door, true)) {
                        doorQueue.RemoveAt(k);
                        return true;
                    }
                }
            }

            return false;
        }

        MirrorDoor TakeFromQueue(int index) {
            MirrorDoor door = doorQueue[index];

            doorQueue[index] = doorQueue[doorQueue.Count - 1];
            doorQueue.RemoveAt(doorQueue.Count - 1);

            return door;
        }

        /* LinkDoor(); Links a door to a random valid candidate of its type. Returns false if there's none;
         * mustReachNewRoom: Only candidates leading to unreached rooms are valid; */
        bool LinkDoor(MirrorDoor door, bool mustReachNewRoom) {
            // The door leaves the queue once it's linked;
            int doorsInQueue = reliableDoorsInQueue - (door.IsReliable ? 1 : 0);

            return door.ExitType == ExitType.TwoWay
                ? LinkTwoWayDoor(door, doorsInQueue, mustReachNewRoom)
                : LinkOneWayDoor(door, doorsInQueue, mustReachNewRoom);
        }

        bool LinkTwoWayDoor(MirrorDoor door, int doorsInQueue, bool mustReachNewRoom) {
            int index = PickCandidate(twoWayPool.Count, mustReachNewRoom, i => {
                MirrorDoor other = twoWayPool[i];

                if (other == door || other.Target != null || other.Room == door.Room) return null;
                if (!CanLink(door.Room, other.Room) || !CanLink(other.Room, door.Room)) return null;
                if (mustReachNewRoom && reachedRooms.Contains(other.Room)) return null;

                // If the other door is waiting in the queue, linking it takes it out of the queue as well;
                int lostDoors = reachedRooms.Contains(other.Room) && other.IsReliable ? 1 : 0;

                return doorsInQueue - lostDoors + NewReliableDoors(other.Room, other);
            });

            if (index < 0) return false;

            MirrorDoor partner = twoWayPool[index];

            twoWayPool.Remove(door);
            twoWayPool.Remove(partner);

            // Entering this door lands Kirby at the partner door, and vice versa;
            SetTarget(door, partner.Arrival);
            SetTarget(partner, door.Arrival);

            ReachRoom(partner.Room);

            return true;
        }

        bool LinkOneWayDoor(MirrorDoor door, int doorsInQueue, bool mustReachNewRoom) {
            int index = PickCandidate(oneWayPool.Count, mustReachNewRoom, i => {
                MirrorTarget target = oneWayPool[i];

                if (target.Destination == door.Room || !CanLink(door.Room, target.Destination)) return null;
                if (mustReachNewRoom && reachedRooms.Contains(target.Destination)) return null;

                return doorsInQueue + NewReliableDoors(target.Destination, null);
            });

            if (index < 0) return false;

            MirrorTarget chosen = oneWayPool[index];

            oneWayPool[index] = oneWayPool[oneWayPool.Count - 1];
            oneWayPool.RemoveAt(oneWayPool.Count - 1);

            SetTarget(door, chosen);
            ReachRoom(chosen.Destination);

            return true;
        }

        void SetTarget(MirrorDoor door, MirrorTarget target) {
            door.Target = target;
            MarkDestinationUsed(door.Room, target.Destination);

            // Every unused door in a reached room is waiting in the queue;
            if (door.IsReliable && reachedRooms.Contains(door.Room)) reliableDoorsInQueue--;
        }

        /* PickCandidate(); Visits the pool in random order and returns the index of a valid candidate, or -1;
         * evaluate returns null for invalid candidates, or the amount of reliable doors left in the queue after the link;
         * While exploring, the queue can't be left without a reliable door, unless it's the last unreached room.
         * When the queue runs low, candidates that leave more doors open are preferred; */
        int PickCandidate(int poolSize, bool isExploring, Func<int, int?> evaluate) {
            int minimumDoors = isExploring && pendingRooms.Count > 1 ? 1 : 0;
            int preferredDoors = isExploring ? LowFrontier + 1 : 0;

            int[] order = Enumerable.Range(0, poolSize).ToArray();
            int fallback = -1;

            // Lazy Fisher-Yates shuffle, so only the visited candidates are shuffled;
            for (int k = 0; k < poolSize; k++) {
                int j = Utils.GetRandomRange(k, poolSize);
                (order[k], order[j]) = (order[j], order[k]);

                int? doorsLeft = evaluate(order[k]);

                if (doorsLeft == null || doorsLeft < minimumDoors) continue;

                if (doorsLeft >= preferredDoors) return order[k];

                if (fallback < 0) fallback = order[k];
            }

            return fallback;
        }

        // ReachRoom(); Adds the room's unused doors to the queue, and follows the non randomized mirrors;
        void ReachRoom(int room) {
            Stack<int> rooms = new Stack<int>();
            rooms.Push(room);

            while (rooms.Count > 0) {
                int current = rooms.Pop();

                if (isHubRoom(current) || !reachedRooms.Add(current)) continue;

                pendingRooms.Remove(current);

                if (doorsByRoom.TryGetValue(current, out List<MirrorDoor> roomDoors)) {
                    foreach (MirrorDoor roomDoor in roomDoors) {
                        if (roomDoor.Target != null) continue;

                        doorQueue.Add(roomDoor);
                        if (roomDoor.IsReliable) reliableDoorsInQueue++;
                    }
                }

                if (frozenEdges.TryGetValue(current, out List<int> destinations)) {
                    foreach (int destination in destinations) {
                        if (roomsWithMirrors.Contains(destination)) rooms.Push(destination);
                    }
                }
            }
        }

        // NewReliableDoors(); Amount of reliable doors added to the queue if the room is reached;
        int NewReliableDoors(int room, MirrorDoor excluded) {
            if (reachedRooms.Contains(room) || isHubRoom(room)) return 0;
            if (!doorsByRoom.TryGetValue(room, out List<MirrorDoor> roomDoors)) return 0;

            return roomDoors.Count(door => door != excluded && door.Target == null && door.IsReliable);
        }

        // CanLink(); A room can't point to the same destination room twice, nor to a hub room;
        bool CanLink(int room, int destination) {
            if (isHubRoom(destination) || !roomsWithMirrors.Contains(destination)) return false;

            return !usedDestinations.TryGetValue(room, out HashSet<int> used) || !used.Contains(destination);
        }

        void MarkDestinationUsed(int room, int destination) {
            if (!usedDestinations.TryGetValue(room, out HashSet<int> used)) {
                used = new HashSet<int>();
                usedDestinations[room] = used;
            }

            used.Add(destination);
        }

        /* CountStrandedRooms(); Builds the final map and counts the rooms that either can't be reached
         * from the hub, or can't get back to it; */
        int CountStrandedRooms() {
            Dictionary<int, List<int>> edges = new Dictionary<int, List<int>>();
            Dictionary<int, List<int>> reverseEdges = new Dictionary<int, List<int>>();

            void AddEdge(int from, int to) {
                int source = isHubRoom(from) ? HubNode : from;
                /* Rooms without mirrors are path enders (Goal minigames, Boss rooms, cannons or stars);
                 * Once finished, they bring Kirby back to the hub; */
                int destination = isHubRoom(to) || !roomsWithMirrors.Contains(to) ? HubNode : to;

                if (!edges.ContainsKey(source)) edges[source] = new List<int>();
                if (!reverseEdges.ContainsKey(destination)) reverseEdges[destination] = new List<int>();

                edges[source].Add(destination);
                reverseEdges[destination].Add(source);
            }

            AddEdge(HubNode, StartTarget.Destination);

            foreach (int entryRoom in entryRooms) AddEdge(HubNode, entryRoom);

            foreach (MirrorDoor door in doors) AddEdge(door.Room, door.Target.Destination);

            foreach (KeyValuePair<int, List<int>> kvp in frozenEdges) {
                foreach (int destination in kvp.Value) AddEdge(kvp.Key, destination);
            }

            HashSet<int> reachable = Traverse(edges, HubNode),
                         canReturn = Traverse(reverseEdges, HubNode);

            int stranded = reachable.Count(room => !canReturn.Contains(room));
            stranded += doorsByRoom.Keys.Count(room => !reachable.Contains(room));

            return stranded;
        }

        static HashSet<int> Traverse(Dictionary<int, List<int>> edges, int start) {
            HashSet<int> visited = new HashSet<int> { start };
            Queue<int> queue = new Queue<int>();
            queue.Enqueue(start);

            while (queue.Count > 0) {
                int room = queue.Dequeue();

                if (!edges.TryGetValue(room, out List<int> next)) continue;

                foreach (int destination in next) {
                    if (visited.Add(destination)) queue.Enqueue(destination);
                }
            }

            return visited;
        }
    }
}
