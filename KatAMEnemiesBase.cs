using KatAMInternal;
using KatAM_Randomizer;
using Newtonsoft.Json.Linq;

namespace KatAMRandomizer {
    public interface IKatAMEnemiesBase {
        protected void LoadSettings();
    }

    internal class KatAMEnemiesBase : KatAMRandomizerComponent {
        // Adding unused behaviors from the start;
        protected Dictionary<byte, List<byte>> behaviorDictionary = new Dictionary<byte, List<byte>>();
        protected Dictionary<byte, List<byte>> speedDictionary = new Dictionary<byte, List<byte>>();

        protected GenerationOptions enemiesOptions,
                          enemiesSpeedOptions,
                          enemiesBehaviorOptions,
                          enemiesInhaleAbilityOptions,
                          enemiesHPOptions;

        protected bool isRandomizingFlyingEnemiesIntelligently,
             isRandomizingUnderwaterEnemiesIntelligently,
             isIncludingMiniBosses,
             isUsingUnusedBehaviors,
             isIncludingMasterInhaleAbility,
             isHPPercentageModified,
             isRandomizingExcludedEnemies,
             isAddingNightmareEnemies;
        protected List<byte> nightmareBehaviors = new List<byte>(), nightmareFlyingBehaviors = new List<byte>();
        // ROM Expansion mini-bosses (object types 0x40-0x42), one behavior list per mini-boss, by type;
        protected bool isAddingNightmareMinibosses;
        protected Dictionary<byte, List<List<byte>>> nightmareMinibossGroups = new Dictionary<byte, List<List<byte>>>();
        protected List<List<byte>> nightmareGroups = new List<List<byte>>(), nightmareFlyingGroups = new List<List<byte>>(),
                                   nightmareWaterGroups = new List<List<byte>>();

        protected List<byte> underwaterEnemyIDs = new List<byte>();
        protected List<byte> flyingEnemyIDs = new List<byte>();
        protected HashSet<byte> progressionEnemyIDs = new HashSet<byte>() {
            0x10, // Jack;
            0x27, // Minny;
            0x34, // Mirra;
        };

        // Rooms whose enemies are never randomized, as the game crashes consistently when they are;
        protected static readonly HashSet<int> roomsWithoutEnemyRandomization = new HashSet<int>() {
            0x6B,
        };

        protected void LoadEnemyDataset(IKatAMRandomizer Instance, string file) {
            entities = new List<Entity>();

            // Deserializing all the entities in the game;
            Utils.DeserializeEntitiesJSON(Utils.JSONToObjects(file), entities, Instance);
        }

        protected void RandomizeEnemies(IKatAMRandomizer Instance, bool isRandomizingMiniBosses = false) {
            List<Entity> minibosses = new List<Entity>();

            if(!isRandomizingMiniBosses && isIncludingMiniBosses) {
                Utils.DeserializeEntitiesJSON(Utils.JSONToObjects(Utils.minibossesJson), minibosses, Instance);
            }

            List<byte> allEntitiesIDs = LoadAllSpeedAndBehaviorData();

            // ROM Expansion enemies join the pool as object type 0x31, the behavior picking which one (and speed 0);
            if(!isRandomizingMiniBosses && isAddingNightmareEnemies) {
                behaviorDictionary[KatAMExpansion.NightmareID] = new List<byte>(nightmareBehaviors);
                speedDictionary[KatAMExpansion.NightmareID] = NewProperty(0x00);

                // The ones that fly can take flying enemies' places;
                if(nightmareFlyingBehaviors.Count > 0 && !flyingEnemyIDs.Contains(KatAMExpansion.NightmareID)) flyingEnemyIDs.Add(KatAMExpansion.NightmareID);

                // Every ported enemy counts as much as one of the game's own: type 0x31 is in the pools once per enemy;
                for(int i = 1; i < nightmareFlyingGroups.Count; i++) flyingEnemyIDs.Add(KatAMExpansion.NightmareID);

                // The ones that swim, underwater enemies' places, the same way;
                for(int i = 0; i < nightmareWaterGroups.Count; i++) underwaterEnemyIDs.Add(KatAMExpansion.NightmareID);
            }

            // ROM Expansion mini-bosses join the mini-boss pool as object types 0x40-0x42 the same way;
            if(isAddingNightmareMinibosses) {
                foreach((byte type, List<List<byte>> groups) in nightmareMinibossGroups) {
                    behaviorDictionary[type] = groups.SelectMany(group => group).ToList();
                    speedDictionary[type] = NewProperty(0x00);
                }
            }

            if(enemiesOptions == GenerationOptions.Shuffle) allEntitiesIDs = Utils.Shuffle(allEntitiesIDs);

            bool isRandomizingIDs = enemiesOptions != GenerationOptions.Unchanged,
                 isRandomizingSpeed = enemiesSpeedOptions != GenerationOptions.Unchanged,
                 isRandomizingBehaviors = enemiesBehaviorOptions != GenerationOptions.Unchanged;

            /* Extracting from a dictionary the list of IDs available for randomizing.
             * Compared to the "allEntitiesIDs" list, this list has no repeats; */
            List<byte> availableEnemyIDs = behaviorDictionary.Keys.ToList();

            if(!isRandomizingMiniBosses && isAddingNightmareEnemies) {
                for(int i = 1; i < nightmareGroups.Count; i++) availableEnemyIDs.Add(KatAMExpansion.NightmareID);
            }

            // Every ported mini-boss counts as much as one of the game's own;
            if(isAddingNightmareMinibosses) {
                foreach((byte type, List<List<byte>> groups) in nightmareMinibossGroups)
                    for(int i = 1; i < groups.Count; i++) availableEnemyIDs.Add(type);
            }

            for(int i = 0; i < entities.Count; i++) {
                Entity entity = entities[i];
                byte id = entity.ID;

                bool isIDAssigned = false,
                     isFlyingPlace = isRandomizingFlyingEnemiesIntelligently && flyingEnemyIDs.Contains(id),
                     isUnderwaterPlace = isRandomizingUnderwaterEnemiesIntelligently && underwaterEnemyIDs.Contains(id);

                // Enemies in these rooms keep their vanilla data;
                if(roomsWithoutEnemyRandomization.Contains(entity.Room)) continue;

                // Underwater or Key Progression enemies;
                if(!isRandomizingMiniBosses && IsVetoedEnemy(entity)) {
                    bool isProgressionEntity = progressionEnemyIDs.Contains(id);

                    if(isProgressionEntity) continue;
                    else {
                        bool isRandomizingSelectEnemies = isRandomizingFlyingEnemiesIntelligently || isRandomizingUnderwaterEnemiesIntelligently || isRandomizingExcludedEnemies;

                        // Underwater enemies will be unchanged if the "Randomize Excluded Enemies" option is not active;
                        if(enemiesOptions == GenerationOptions.Shuffle && !isRandomizingExcludedEnemies) continue;
                        // If it's randomizing underwater enemies, select valid underwater replacements;
                        else if(isRandomizingSelectEnemies) {

                            if(isRandomizingFlyingEnemiesIntelligently && flyingEnemyIDs.Contains(id)) {
                                int flyingEnemyIndex = Utils.GetRandomRange(0, flyingEnemyIDs.Count);

                                entity.ID = flyingEnemyIDs[flyingEnemyIndex];
                                isIDAssigned = true;
                            }

                            else if(isRandomizingUnderwaterEnemiesIntelligently && underwaterEnemyIDs.Contains(id)) {
                                int underwaterEnemyIndex = Utils.GetRandomRange(0, underwaterEnemyIDs.Count);

                                entity.ID = underwaterEnemyIDs[underwaterEnemyIndex];
                                isIDAssigned = true;
                            }
                        }
                    }
                }

                // Randomizing enemy IDs;
                if(isRandomizingIDs) {
                    switch(enemiesOptions) {
                        // Shuffling enemies;
                        case GenerationOptions.Shuffle:
                            if(!isIDAssigned) {
                                byte selectedID = allEntitiesIDs[i];
                                entity.ID = allEntitiesIDs[i];

                                do {
                                    int rerollIndex = Utils.GetRandomRange(0, availableEnemyIDs.Count);

                                    entity.ID = availableEnemyIDs[rerollIndex];
                                } while(entity.ID == 0x34 || entity.ID == 0x10);

                                
                            }

                            break;

                        // Randomizing enemies;
                        case GenerationOptions.Random:
                            if(!isIDAssigned) {
                                int idIndex = Utils.GetRandomRange(0, availableEnemyIDs.Count);

                                entity.ID = availableEnemyIDs[idIndex];
                            }
                            break;

                        // Removing enemies;
                        case GenerationOptions.No:
                            entity.ID = Utils.Nothing;
                            break;
                    }


                    if(!isRandomizingMiniBosses && enemiesOptions != GenerationOptions.No) {
                        if(isIncludingMiniBosses) {
                            // Only where the camera can't leave them behind (see MinibossScreens);
                            (int x, int y, int width, int height)? screen = MinibossScreens.ScreenOf(entity);
                            bool canSpawnMiniboss = screen != null && Utils.Dice(1, 20) == 1;

                            if(canSpawnMiniboss) {
                                int selectedMinibossIndex = Utils.GetRandomRange(0, minibosses.Count);
                                Entity miniboss = minibosses[selectedMinibossIndex];

                                entity.ID = miniboss.ID;
                                entity.Link = new byte[2] { 0x0, 0x0 };
                                entity.Behavior = miniboss.Behavior;
                                entity.Speed = miniboss.Speed;
                                entity.Properties = (byte[])miniboss.Properties.Clone();
                                MinibossScreens.SetArea(entity, screen.Value);
                            }
                        }
                    }
                }

                // ROM Expansion enemies: the behavior picks which one, so it's always one of theirs;
                if(entity.ID == KatAMExpansion.NightmareID && isAddingNightmareEnemies) {
                    // An enemy first, then one of its variants, so enemies with many variants aren't more common;
                    List<List<byte>> groups = isFlyingPlace && nightmareFlyingGroups.Count > 0 ? nightmareFlyingGroups :
                                              isUnderwaterPlace && nightmareWaterGroups.Count > 0 ? nightmareWaterGroups : nightmareGroups;
                    List<byte> behaviors = groups[Utils.GetRandomRange(0, groups.Count)];
                    entity.Behavior = behaviors[Utils.GetRandomRange(0, behaviors.Count)];
                }

                // ROM Expansion mini-bosses: the behavior picks which one (a mini-boss first, then one of its variants);
                bool isNightmareMiniboss = isAddingNightmareMinibosses && nightmareMinibossGroups.ContainsKey(entity.ID);
                if(isNightmareMiniboss) {
                    List<List<byte>> groups = nightmareMinibossGroups[entity.ID];
                    List<byte> behaviors = groups[Utils.GetRandomRange(0, groups.Count)];
                    entity.Behavior = behaviors[Utils.GetRandomRange(0, behaviors.Count)];
                }

                // Randomizing enemy speeds;
                if(isRandomizingSpeed) {
                    if(speedDictionary.ContainsKey(entity.ID)) {
                        List<byte> availableEnemySpeeds = speedDictionary[entity.ID];
                        int speedsIndex = Utils.GetRandomRange(0, availableEnemySpeeds.Count);

                        entity.Speed = availableEnemySpeeds[speedsIndex];
                    }
                }

                // Randomizing enemy behaviors (ROM Expansion enemies already have theirs);
                if(isRandomizingBehaviors && !(entity.ID == KatAMExpansion.NightmareID && isAddingNightmareEnemies) && !isNightmareMiniboss) {
                    if(behaviorDictionary.ContainsKey(entity.ID)) {
                        List<byte> availableEnemyBehaviors = behaviorDictionary[entity.ID];
                        int behaviorIndex = Utils.GetRandomRange(0, availableEnemyBehaviors.Count);

                        entity.Behavior = availableEnemyBehaviors[behaviorIndex];
                    }
                }

                // Batafire flies across the arena from one side (its own places are all by one), so elsewhere it goes there;
                if(entity.ID == MinibossScreens.Batafire && id != MinibossScreens.Batafire) MinibossScreens.MoveToSide(entity);

                Utils.WriteObjectToROM(entity);
            }
        }

        protected List<byte> LoadAllSpeedAndBehaviorData() {
            List<byte> allEntitiesIDs = new List<byte>();

            foreach(Entity entity in entities) {
                allEntitiesIDs.Add(entity.ID);

                if(!behaviorDictionary.ContainsKey(entity.ID)) {
                    behaviorDictionary.Add(entity.ID, NewProperty(entity.Behavior));
                }

                if(!speedDictionary.ContainsKey(entity.ID)) {
                    speedDictionary.Add(entity.ID, NewProperty(entity.Speed));
                }

                List<byte> behaviorsList = behaviorDictionary[entity.ID];

                if(!behaviorsList.Contains(entity.Behavior)) {
                    behaviorsList.Add(entity.Behavior);
                }

                List<byte> speedsList = speedDictionary[entity.ID];

                if(!speedsList.Contains(entity.Speed)) {
                    speedsList.Add(entity.Speed);
                }
            }

            return allEntitiesIDs;
        }

        protected List<byte> NewProperty(byte input) {
            return new List<byte>() { input };
        }

        bool IsVetoedEnemy(Entity entity) {
            byte id = entity.ID;

            return progressionEnemyIDs.Contains(id) ||
                   underwaterEnemyIDs.Contains(id) ||
                   flyingEnemyIDs.Contains(id);
        }
    }

    internal class KatAMEnemies : KatAMEnemiesBase, IKatAMEnemiesBase, IKatAMRandomizer {
        public KatAMEnemies(Processing system) {
            InitializeComponents(system);

            LoadSettings();

            if(isUsingUnusedBehaviors) InitializeBehaviorDictionary();

            LoadEnemyDataset(this, Utils.enemiesJson);

            GroupEnemies();

            RandomizeEnemies(this);
        }

        public void LoadSettings() {
            // Calling the settings data;
            enemiesOptions = Settings.EnemiesGenerationType;
            enemiesSpeedOptions = Settings.EnemiesPropertiesSpeedType;
            enemiesBehaviorOptions = Settings.EnemiesPropertiesBehaviorType;
            enemiesInhaleAbilityOptions = Settings.EnemiesInhaleAbilityType;
            enemiesHPOptions = Settings.EnemiesHPType;

            isRandomizingFlyingEnemiesIntelligently = Settings.isRandomizingFlyingEnemiesIntelligently;
            isRandomizingUnderwaterEnemiesIntelligently = Settings.isRandomizingUnderwaterEnemiesIntelligently;
            isIncludingMiniBosses = Settings.isIncludingMiniBosses;
            isUsingUnusedBehaviors = Settings.isUsingUnusedBehaviors;
            isIncludingMasterInhaleAbility = Settings.isEnemyIncludingMasterInhaleAbility;
            isHPPercentageModified = Settings.isEnemyHPPercentageModified;
            isRandomizingExcludedEnemies = Settings.isRandomizingExcludedEnemies;
            isAddingNightmareEnemies = Settings.NightmareEnemies.Count > 0;
            nightmareBehaviors = KatAMExpansion.BehaviorsOf(Settings.NightmareEnemies);
            nightmareFlyingBehaviors = KatAMExpansion.BehaviorsOf(Settings.NightmareEnemies, true);
            nightmareGroups = KatAMExpansion.BehaviorGroupsOf(Settings.NightmareEnemies);
            nightmareFlyingGroups = KatAMExpansion.BehaviorGroupsOf(Settings.NightmareEnemies, true);
            nightmareWaterGroups = KatAMExpansion.BehaviorGroupsOf(Settings.NightmareEnemies, water: true);
        }

        public void GroupEnemies() {
            foreach(Entity enemy in entities) {
                if(enemy.IsUnderwater && !underwaterEnemyIDs.Contains(enemy.ID)) {
                    underwaterEnemyIDs.Add(enemy.ID);
                }

                if(enemy.IsFlying && !flyingEnemyIDs.Contains(enemy.ID)) {
                    flyingEnemyIDs.Add(enemy.ID);
                }
            }
        }

        void InitializeBehaviorDictionary() {
            behaviorDictionary = new Dictionary<byte, List<byte>>();

            // Squishy;
            behaviorDictionary.Add(0x04, NewProperty(0x01)); // Appear and launch upwards;
            behaviorDictionary[0x04].Add(0x02); // Hop and chase Kirby;

            // Scarfy;
            behaviorDictionary.Add(0x05, NewProperty(0x03)); // Rise from below and places itself at Kirby's current Y;

            // Gordo;
            behaviorDictionary.Add(0x06, NewProperty(0x03)); // Hover up and down slowly;

            // Haley;
            behaviorDictionary.Add(0x0A, NewProperty(0x01)); // Fly with regular speed instead of slowing down;

            // Cupie;
            behaviorDictionary.Add(0x0C, NewProperty(0x02)); // Fly in 8 pattern;
            behaviorDictionary[0x0C].Add(0x03); // Run away from Kirby, shoot, disappear out of the screen;

            // Leap;
            behaviorDictionary.Add(0x0F, NewProperty(0x01)); // Hover up and down slowly;

            // Big Waddle Dee;
            behaviorDictionary.Add(0x11, NewProperty(0x02)); // Jump;

            // Golem Press;
            behaviorDictionary.Add(0x1F, NewProperty(0x01)); // Do all the Golem attacks;

            // Golem Roll;
            behaviorDictionary.Add(0x20, NewProperty(0x01)); // Do all the Golem attacks;

            // Boxin;
            behaviorDictionary.Add(0x25, NewProperty(0x03)); // Stand in place;

            // Cookin;
            behaviorDictionary.Add(0x26, NewProperty(0x01)); // Walking;

            // Heavy Knight;
            behaviorDictionary.Add(0x29, NewProperty(0x02)); // Stand in place;

            // Giant Rocky;
            behaviorDictionary.Add(0x2A, NewProperty(0x01)); // Stand in place;

            // Batty;
            behaviorDictionary.Add(0x2D, NewProperty(0x01)); // Chase Kirby and return to its spawn point;

            // Shotzo;
            behaviorDictionary.Add(0x35, NewProperty(0x01)); // Fire at the upper left section of the screen;
        }

        public bool FilterEntities(Entity entity) { return true; }
    }

    internal class KatAMMinibosses : KatAMEnemiesBase, IKatAMEnemiesBase, IKatAMRandomizer {

        public KatAMMinibosses(Processing system) {
            InitializeComponents(system);

            LoadSettings();

            LoadEnemyDataset(this, Utils.minibossesJson);

            RandomizeEnemies(this);
        }

        public void LoadSettings() {
            // Calling the settings data;
            enemiesOptions = Settings.MinibossesGenerationType;
            enemiesSpeedOptions = Settings.MinibossesPropertiesSpeedType;
            enemiesBehaviorOptions = Settings.MinibossesPropertiesBehaviorType;
            enemiesInhaleAbilityOptions = Settings.MinibossesInhaleAbilityType;
            enemiesHPOptions = Settings.MinibossesHPType;

            isIncludingMasterInhaleAbility = Settings.isMinibossIncludingMasterInhaleAbility;
            isHPPercentageModified = Settings.isMinibossHPPercentageModified;
            isAddingNightmareMinibosses = Settings.NightmareMinibosses.Count > 0;
            nightmareMinibossGroups = KatAMExpansion.MinibossIDs.ToDictionary(type => type, type => KatAMExpansion.MinibossBehaviorGroupsOf(Settings.NightmareMinibosses, type))
                                                               .Where(entry => entry.Value.Count > 0).ToDictionary(entry => entry.Key, entry => entry.Value);
        }

        public bool FilterEntities(Entity entity) { return true; }
    }

    /* MinibossScreens; Keeping randomized mini-bosses on the screen. In their own rooms a scroll lock (object 0x77, its area: template
     * +0x1A-0x21, like the mini-boss's) holds the camera on the arena, and KatAM's mini-bosses stay inside it (ObjXSomething, the ROM
     * Expansion's too); elsewhere the camera follows Kirby and they'd wander off. So a mini-boss only takes an enemy's place where the
     * camera can't leave it: inside a scroll lock's area, or in a room the size of the screen (256x208 at most), and its fight area
     * becomes that one. Batafire doesn't keep itself inside: it flies across from one side of the arena, so in another's place it
     * goes to the side that one was nearer to, where its own are (29 pixels in, 60 down from the top); */
    internal static class MinibossScreens {
        public const byte Batafire = 0x3B;
        const byte ScrollLock = 0x77;
        const int MaxScreenWidth = 256, MaxScreenHeight = 208, BatafireSide = 29, BatafireTop = 60, BatafireWidth = 32;

        static readonly Dictionary<int, List<(int x, int y, int width, int height)>> screens = new Dictionary<int, List<(int, int, int, int)>>();
        static byte[] screensOf;

        static int Read16(byte[] rom, long address) => rom[address] | (rom[address + 1] << 8);

        static long Pointer(byte[] rom, long address) {
            uint pointer = BitConverter.ToUInt32(rom, (int)address);
            return pointer >= 0x08000000 && pointer < 0x0A000000 ? pointer & 0x01FFFFFF : -1;
        }

        // Screens(); A room's areas the camera stays on: its scroll locks', or the whole room if it's the size of the screen;
        static List<(int x, int y, int width, int height)> Screens(int room) {
            byte[] rom = Processing.ROMData;

            if (screensOf != rom) {
                screens.Clear();
                screensOf = rom;
            }
            if (screens.TryGetValue(room, out var cached)) return cached;

            List<(int, int, int, int)> list = new List<(int, int, int, int)>();
            RomRegion region = RomRegion.Current;
            int shortIndex = Read16(rom, region.LevelData(0x903338) + room * 0x28 + 0x24);
            long appearance = Pointer(rom, region.PointerTable(0xD2F93C) + shortIndex * 4), objects = Pointer(rom, region.PointerTable(0xD2EBC4) + shortIndex * 4);

            if (appearance >= 0 && Read16(rom, appearance) * 8 <= MaxScreenWidth && Read16(rom, appearance + 2) * 8 <= MaxScreenHeight)
                list.Add((0, 0, Read16(rom, appearance) * 8, Read16(rom, appearance + 2) * 8));
            for (long at = objects + 16; objects >= 0 && rom[at] == 0x01 && rom[at + 1] == 0x24; at += 36) {
                if (rom[at + 12] != ScrollLock) continue;
                list.Add(((short)Read16(rom, at + 6) + (short)Read16(rom, at + 0x1A), (short)Read16(rom, at + 8) + (short)Read16(rom, at + 0x1C),
                          Read16(rom, at + 0x1E), Read16(rom, at + 0x20)));
            }
            return screens[room] = list;
        }

        static int X(Entity entity) => (short)(entity.X[0] | (entity.X[1] << 8));
        static int Y(Entity entity) => (short)(entity.Y[0] | (entity.Y[1] << 8));

        // ScreenOf(); The area the camera stays on around an enemy's place, or null if it follows Kirby there;
        public static (int x, int y, int width, int height)? ScreenOf(Entity entity) {
            int x = X(entity), y = Y(entity);

            foreach (var screen in Screens(entity.Room))
                if (x >= screen.x && x <= screen.x + screen.width && y >= screen.y && y <= screen.y + screen.height) return screen;
            return null;
        }

        // SetArea(); The mini-boss's fight area (Properties 9-16: template +0x1A-0x21, relative to its place) becomes the screen;
        public static void SetArea(Entity entity, (int x, int y, int width, int height) screen) {
            WriteArea(entity, screen.x - X(entity), screen.y - Y(entity), screen.width, screen.height);
        }

        static void WriteArea(Entity entity, int left, int top, int width, int height) {
            BitConverter.GetBytes((short)left).CopyTo(entity.Properties, 9);
            BitConverter.GetBytes((short)top).CopyTo(entity.Properties, 11);
            BitConverter.GetBytes((ushort)width).CopyTo(entity.Properties, 13);
            BitConverter.GetBytes((ushort)height).CopyTo(entity.Properties, 15);
        }

        // MoveToSide(); Batafire to the side of its fight area its place is nearer to; the area stays where it is;
        public static void MoveToSide(Entity entity) {
            int left = BitConverter.ToInt16(entity.Properties, 9), top = BitConverter.ToInt16(entity.Properties, 11), width = BitConverter.ToUInt16(entity.Properties, 13);

            if (width == 0) return;

            int fromLeft = -left, toLeft = fromLeft * 2 > width ? width - BatafireWidth : BatafireSide, dx = toLeft - fromLeft, dy = BatafireTop + top;

            entity.X = BitConverter.GetBytes((short)(X(entity) + dx));
            entity.Y = BitConverter.GetBytes((short)(Y(entity) + dy));
            WriteArea(entity, left - dx, top - dy, width, BitConverter.ToUInt16(entity.Properties, 15));
            entity.IsMoved = true;
        }
    }
}
