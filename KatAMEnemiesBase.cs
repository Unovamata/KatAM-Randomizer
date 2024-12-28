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
             isRandomizingExcludedEnemies;

        protected List<byte> underwaterEnemyIDs = new List<byte>();
        protected List<byte> flyingEnemyIDs = new List<byte>();
        protected HashSet<byte> progressionEnemyIDs = new HashSet<byte>() {
            0x10, // Jack;
            0x27, // Minny;
            0x34, // Mirra;
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

            if(enemiesOptions == GenerationOptions.Shuffle) allEntitiesIDs = Utils.Shuffle(allEntitiesIDs);

            bool isRandomizingIDs = enemiesOptions != GenerationOptions.Unchanged,
                 isRandomizingSpeed = enemiesSpeedOptions != GenerationOptions.Unchanged,
                 isRandomizingBehaviors = enemiesBehaviorOptions != GenerationOptions.Unchanged;

            /* Extracting from a dictionary the list of IDs available for randomizing.
             * Compared to the "allEntitiesIDs" list, this list has no repeats; */
            List<byte> availableEnemyIDs = behaviorDictionary.Keys.ToList();

            for(int i = 0; i < entities.Count; i++) {
                Entity entity = entities[i];
                byte id = entity.ID;

                bool isIDAssigned = false;

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
                                int flyingEnemyIndex = Utils.GetRandomNumber(0, flyingEnemyIDs.Count);

                                entity.ID = flyingEnemyIDs[flyingEnemyIndex];
                                isIDAssigned = true;
                            }

                            else if(isRandomizingUnderwaterEnemiesIntelligently && underwaterEnemyIDs.Contains(id)) {
                                int underwaterEnemyIndex = Utils.GetRandomNumber(0, underwaterEnemyIDs.Count);

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
                                    int rerollIndex = Utils.GetRandomNumber(0, availableEnemyIDs.Count);

                                    entity.ID = availableEnemyIDs[rerollIndex];

                                    if(entity.ID == 0x34 || entity.ID == 0x10) {
                                        Console.WriteLine(entity.ID);
                                    }
                                } while(entity.ID == 0x34 || entity.ID == 0x10);

                                
                            }

                            break;

                        // Randomizing enemies;
                        case GenerationOptions.Random:
                            if(!isIDAssigned) {
                                int idIndex = Utils.GetRandomNumber(0, availableEnemyIDs.Count);

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
                            bool canSpawnMiniboss = Utils.Dice(1, 10) == 1;

                            if(canSpawnMiniboss) {
                                int selectedMinibossIndex = Utils.GetRandomNumber(0, minibosses.Count);
                                Entity miniboss = minibosses[selectedMinibossIndex];

                                entity.ID = miniboss.ID;
                                entity.Link = new byte[2] { 0x0, 0x0 };
                                entity.Behavior = miniboss.Behavior;
                                entity.Speed = miniboss.Speed;
                                entity.Properties = miniboss.Properties;
                            }
                        }
                    }
                }

                // Randomizing enemy speeds;
                if(isRandomizingSpeed) {
                    if(speedDictionary.ContainsKey(entity.ID)) {
                        List<byte> availableEnemySpeeds = speedDictionary[entity.ID];
                        int speedsIndex = Utils.GetRandomNumber(0, availableEnemySpeeds.Count);

                        entity.Speed = availableEnemySpeeds[speedsIndex];
                    }
                }

                // Randomizing enemy behaviors;
                if(isRandomizingBehaviors) {
                    if(behaviorDictionary.ContainsKey(entity.ID)) {
                        List<byte> availableEnemyBehaviors = behaviorDictionary[entity.ID];
                        int behaviorIndex = Utils.GetRandomNumber(0, availableEnemyBehaviors.Count);

                        entity.Behavior = availableEnemyBehaviors[behaviorIndex];
                    }
                }

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
        }

        public bool FilterEntities(Entity entity) { return true; }
    }
}