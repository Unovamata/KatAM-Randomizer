using KatAMInternal;
using KatAMRandomizer;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace KatAM_Randomizer {
    internal class KatAMEnemies : KatAMRandomizerComponent, IKatAMRandomizer{

        public KatAMEnemies(Processing system) {
            InitializeComponents(system);

            RandomizeEnemies(this);
        }

        // Adding unused behaviors from the start;
        Dictionary<byte, List<byte>> enemyBehaviorDictionary = new Dictionary<byte, List<byte>>();
        Dictionary<byte, List<byte>> enemySpeedDictionary = new Dictionary<byte, List<byte>>();

        GenerationOptions enemiesOptions, 
                          enemiesSpeedOptions, 
                          enemiesBehaviorOptions, 
                          enemiesInhaleAbilityOptions, 
                          enemiesHPOptions;

        bool isRandomizingEnemiesIntelligently,
             isIncludingMasterInhaleAbility,
             isHPPercentageModified,
             isRandomizingExcludedEnemies;

        List<byte> underwaterVetoedEnemyIDs = new List<byte>() {
            0x02, // Blipper;
            0x03, // Glunk;
            0x04, // Squishy;
            0x06, // Gordo;
        };

        List<byte> underwaterValidEnemyIDs = new List<byte>() {
            0x01, //Bronto 
            0x23, //Shooty 
            0x0F, //Leap
            0x15, //Laser
            0x09, //Soarar
            0x1B, //UFO
            0x0A //Haley
        };

        List<byte> underwaterEnemyIDs = new List<byte>();

        List<byte> progressionEnemyIDs = new List<byte>() {
            0x10, // Jack;
            0x27, // Minny;
            0x34, // Mirra;
        };

        void RandomizeEnemies(IKatAMRandomizer Instance) {
            // Calling the settings data;
            enemiesOptions = Settings.EnemiesGenerationType;
            enemiesSpeedOptions = Settings.EnemiesPropertiesSpeedType;
            enemiesBehaviorOptions = Settings.EnemiesPropertiesBehaviorType;
            enemiesInhaleAbilityOptions = Settings.EnemiesInhaleAbilityType;
            enemiesHPOptions = Settings.EnemiesHPType;

            isRandomizingEnemiesIntelligently = Settings.isRandomizingEnemiesIntelligently;
            isIncludingMasterInhaleAbility = Settings.isIncludingMasterInhaleAbility;
            isHPPercentageModified = Settings.isHPPercentageModified;
            isRandomizingExcludedEnemies = Settings.isRandomizingExcludedEnemies;

            // Adding underwater enemies together for randomization;
            underwaterEnemyIDs.AddRange(underwaterVetoedEnemyIDs);
            underwaterEnemyIDs.AddRange(underwaterValidEnemyIDs);

            InitializeBehaviorDictionary();

            entities = new List<Entity>();

            // Read the settings, entities, and ROM data;
            byte[] romFile = System.ROMData;

            // Deserializing all the entities in the game;
            Utils.DeserializeJSON(Utils.JSONToEntities(Utils.enemiesJson), entities, Instance);

            List<byte> enemyIDs = new List<byte>();

            foreach(Entity entity in entities) {
                enemyIDs.Add(entity.ID);

                if (!enemyBehaviorDictionary.ContainsKey(entity.ID)) {
                    enemyBehaviorDictionary.Add(entity.ID, NewProperty(entity.Behavior));
                }

                if (!enemySpeedDictionary.ContainsKey(entity.ID)) {
                    enemySpeedDictionary.Add(entity.ID, NewProperty(entity.Speed));
                }

                List<byte> behaviorsList = enemyBehaviorDictionary[entity.ID];

                if (!behaviorsList.Contains(entity.Behavior)) {
                    behaviorsList.Add(entity.Behavior);
                }

                List<byte> speedsList = enemySpeedDictionary[entity.ID];

                if (!speedsList.Contains(entity.Speed)) {
                    speedsList.Add(entity.Speed);
                }
            }

            if (enemiesOptions == GenerationOptions.Shuffle) enemyIDs = Utils.Shuffle(enemyIDs);

            List<byte> enemyKeysIDs = enemyBehaviorDictionary.Keys.ToList();

            bool isRandomizingIDs = enemiesOptions != GenerationOptions.Unchanged,
                 isRandomizingSpeed = enemiesSpeedOptions != GenerationOptions.Unchanged,
                 isRandomizingBehaviors = enemiesBehaviorOptions != GenerationOptions.Unchanged;

            for (int i = 0; i < entities.Count; i++) {
                bool isIDAssigned = false;
                Entity entity = entities[i];
                
                List<byte> speedsList = enemySpeedDictionary[entity.ID];

                if (IsVetoedEnemy(entity)) {
                    bool isProgressionEntity = progressionEnemyIDs.Contains(entity.ID);

                    if (isProgressionEntity) continue;
                    else {
                        // IDs will not change if entities are being shuffled and the user is not randomizing the underwater enemies;
                        if (enemiesOptions == GenerationOptions.Shuffle && !isRandomizingExcludedEnemies) continue;
                        else if (isRandomizingEnemiesIntelligently || isRandomizingExcludedEnemies) {
                            int underwaterEnemyIndex = Utils.GetRandomNumber(0, underwaterEnemyIDs.Count);

                            entity.ID = underwaterEnemyIDs[underwaterEnemyIndex];
                            isIDAssigned = true;
                        }
                    }
                }

                if (isRandomizingIDs) {
                    switch (enemiesOptions) {
                        case GenerationOptions.Shuffle:
                            if (!isIDAssigned) {
                                byte selectedID = enemyIDs[i];

                                entity.ID = enemyIDs[i];

                                // If it will shuffle into a Mirra, reroll the enemy;
                                do {
                                    int rerollIndex = Utils.GetRandomNumber(0, enemyKeysIDs.Count);

                                    entity.ID = enemyKeysIDs[rerollIndex];
                                } while (entity.ID == 0x34); // Mirra;
                            }
                        break;

                        case GenerationOptions.Random:
                            if (!isIDAssigned) {
                                int idIndex = Utils.GetRandomNumber(0, enemyKeysIDs.Count);

                                entity.ID = enemyKeysIDs[idIndex];
                            }
                        break;

                        case GenerationOptions.No:
                            entity.ID = Utils.Nothing;
                        break;
                    }
                    
                }

                if (isRandomizingSpeed) {
                    int speedsIndex = Utils.GetRandomNumber(0, speedsList.Count);

                    entity.Speed = speedsList[speedsIndex];
                }

                if (isRandomizingBehaviors) {
                    List<byte> behaviorsList = enemyBehaviorDictionary[entity.ID];
                    int behaviorIndex = Utils.GetRandomNumber(0, behaviorsList.Count);

                    entity.Behavior = behaviorsList[behaviorIndex];
                }

                Utils.WriteObjectToROM(romFile, entity);
            }
        }

        void InitializeBehaviorDictionary() {
            enemyBehaviorDictionary = new Dictionary<byte, List<byte>>();

            // Squishy;
            enemyBehaviorDictionary.Add(0x04, NewProperty(0x01)); // Appear and launch upwards;
            enemyBehaviorDictionary[0x04].Add(0x02); // Hop and chase Kirby;

            // Scarfy;
            enemyBehaviorDictionary.Add(0x05, NewProperty(0x03)); // Rise from below and places itself at Kirby's current Y;

            // Gordo;
            enemyBehaviorDictionary.Add(0x06, NewProperty(0x03)); // Hover up and down slowly;

            // Haley;
            enemyBehaviorDictionary.Add(0x0A, NewProperty(0x01)); // Fly with regular speed instead of slowing down;

            // Cupie;
            enemyBehaviorDictionary.Add(0x0C, NewProperty(0x02)); // Fly in 8 pattern;
            enemyBehaviorDictionary[0x0C].Add(0x03); // Run away from Kirby, shoot, disappear out of the screen;

            // Leap;
            enemyBehaviorDictionary.Add(0x0F, NewProperty(0x01)); // Hover up and down slowly;

            // Big Waddle Dee;
            enemyBehaviorDictionary.Add(0x11, NewProperty(0x02)); // Jump;

            // Golem Press;
            enemyBehaviorDictionary.Add(0x1F, NewProperty(0x01)); // Do all the Golem attacks;

            // Golem Roll;
            enemyBehaviorDictionary.Add(0x20, NewProperty(0x01)); // Do all the Golem attacks;

            // Boxin;
            enemyBehaviorDictionary.Add(0x25, NewProperty(0x03)); // Stand in place;

            // Cookin;
            enemyBehaviorDictionary.Add(0x26, NewProperty(0x01)); // Walking;

            // Heavy Knight;
            enemyBehaviorDictionary.Add(0x29, NewProperty(0x02)); // Stand in place;

            // Giant Rocky;
            enemyBehaviorDictionary.Add(0x2A, NewProperty(0x01)); // Stand in place;

            // Batty;
            enemyBehaviorDictionary.Add(0x2D, NewProperty(0x01)); // Chase Kirby and return to its spawn point;

            // Shotzo;
            enemyBehaviorDictionary.Add(0x35, NewProperty(0x01)); // Fire at the upper left section of the screen;

        }

        List<byte> NewProperty(byte input) {
            return new List<byte>() { input };
        }

        bool IsVetoedEnemy(Entity entity) {
            byte id = entity.ID;

            return progressionEnemyIDs.Contains(id) || underwaterEnemyIDs.Contains(id);
        }

        public bool FilterEntities(Entity entity) {

            return true;
        }
    }
}
