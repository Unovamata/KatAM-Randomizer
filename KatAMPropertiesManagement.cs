using KatAMInternal;
using KatAMRandomizer;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Xml.Linq;

namespace KatAM_Randomizer {
    internal class KatAMPropertiesManagement : KatAMRandomizerComponent, IKatAMRandomizer {
        byte[] romFile = Processing.ROMData;
        public KatAMPropertiesManagement(Processing system) {
            InitializeComponents(system);

            ManageProperties(this);
        }

        Dictionary<byte, Properties> propertiesDictionary = new Dictionary<byte, Properties>();
        List<Properties> modifiedProperties = new List<Properties>();

        Dictionary<byte, Data> enemiesDictionary;

        GenerationOptions EnemiesInhaleAbilityType,
                          MinibossesInhaleAbilityType;
                          

        public bool isEnemyIncludingNormalInhaleAbility,
                    isEnemyIncludingMasterInhaleAbility,
                    isEnemyIncludingMixInhaleAbility,
                    isMinibossIncludingNormalInhaleAbility,
                    isMinibossIncludingMasterInhaleAbility,
                    isMinibossIncludingMixInhaleAbility;

        List<byte> InitializeAbilityList() {
            return new List<byte>() {
                0x00, // Normal*/
                0x01, // Fire;
                0x02, // Ice;
                0x03, // Burning;
                0x04, // Wheel;
                0x05, // Parasol;
                0x06, // Cutter;
                0x07, // Beam;
                0x08, // Stone;
                0x09, // Bomb;
                0x0A, // Throw;
                0x0B, // Sleep;
                0x0C, // Cook;
                0x0D, // Laser;
                0x0E, // UFO;
                0x0F, // Spark;
                0x10, // Tornado;
                0x11, // Hammer;
                0x12, // Sword
                0x13, // Cupid;
                0x14, // Fighter;
                0x15, // Magic;
                0x16, // Smash;
                0x17, // Mini;
                0x18, // Crash;
                0x19, // Missile;
                /*0x1A, // Master;
                0x1B, // WAIT / Mix;*/
            };
        }

        void LoadRandomizationSettings() {
            EnemiesInhaleAbilityType = Settings.EnemiesInhaleAbilityType;
            isEnemyIncludingNormalInhaleAbility = Settings.isEnemyIncludingNormalInhaleAbility;
            isEnemyIncludingMasterInhaleAbility = Settings.isEnemyIncludingMasterInhaleAbility;
            isEnemyIncludingMixInhaleAbility = Settings.isEnemyIncludingMixInhaleAbility;

            MinibossesInhaleAbilityType = Settings.MinibossesInhaleAbilityType;
            isMinibossIncludingNormalInhaleAbility = Settings.isMinibossIncludingNormalInhaleAbility;
            isMinibossIncludingMasterInhaleAbility = Settings.isMinibossIncludingMasterInhaleAbility;
            isMinibossIncludingMixInhaleAbility = Settings.isMinibossIncludingMixInhaleAbility;
        }

        void ManageProperties(IKatAMRandomizer Instance) {
            List<byte> abilityIndexes = InitializeAbilityList();
            propertiesDictionary = new Dictionary<byte, Properties>();
            modifiedProperties = new List<Properties>();
            enemiesDictionary = new Dictionary<byte, Data>();

            DeserializePropertiesJSON(Utils.JSONToObjects(Utils.propertiesJson), Instance);

            LoadRandomizationSettings();

            List<byte> enemyAbilityIndexes = new List<byte>(abilityIndexes);
        
            if (isEnemyIncludingMasterInhaleAbility) enemyAbilityIndexes.Add(0x1A);
            if (isEnemyIncludingMixInhaleAbility) enemyAbilityIndexes.Add(0x1B);

            // Removing non inhalable enemies;
            List<byte> shuffleAbilities = new List<byte>();

            if (EnemiesInhaleAbilityType == GenerationOptions.Shuffle) {
                foreach (KeyValuePair<byte, Data> kvp in Processing.enemiesDictionary) {
                    byte key = kvp.Key;
                    Data data = kvp.Value;

                    bool isInhalable = data.IsInhalable;

                    // Filtering non-inhalable items so abilities don't go to uninhalable enemies;
                    if (isInhalable) {
                        enemiesDictionary[key] = data;
                        
                        shuffleAbilities.Add(data.AbilityID);
                    }
                }

                shuffleAbilities = Utils.Shuffle(shuffleAbilities);
            } else { 
                enemiesDictionary = Processing.enemiesDictionary;

                if (isEnemyIncludingNormalInhaleAbility) enemyAbilityIndexes.Add(0x00);
            }

            RandomizeAbilityProperties(EnemiesInhaleAbilityType, Processing.enemiesDictionary,
                                       enemyAbilityIndexes, shuffleAbilities);


            // Minibosses;
            List<byte> minibossAbilityIndexes = new List<byte>(abilityIndexes);

            if (isMinibossIncludingNormalInhaleAbility) minibossAbilityIndexes.Add(0x00);
            if (isMinibossIncludingMasterInhaleAbility) minibossAbilityIndexes.Add(0x1A);
            if (isMinibossIncludingMixInhaleAbility) minibossAbilityIndexes.Add(0x1B);

            shuffleAbilities = new List<byte>();

            if (MinibossesInhaleAbilityType == GenerationOptions.Shuffle) {
                foreach (KeyValuePair<byte, Data> kvp in Processing.enemiesDictionary) {
                    byte key = kvp.Key;
                    Data data = kvp.Value;

                    enemiesDictionary[key] = data;

                    shuffleAbilities.Add(data.AbilityID);
                }

                shuffleAbilities = Utils.Shuffle(shuffleAbilities);
            }

            RandomizeAbilityProperties(MinibossesInhaleAbilityType, Processing.minibossesDictionary, 
                                       minibossAbilityIndexes, shuffleAbilities);

            foreach (Properties property in modifiedProperties) {
                Utils.WritePropertiesToROM(property);
            }
        }

        void DeserializePropertiesJSON(dynamic json, IKatAMRandomizer component) {
            // Reading the JSON dictionary;
            foreach (string key in json.Keys) {
                List<Dictionary<string, dynamic>> dict = json[key];

                // Injecting the extracted information on Entities;
                foreach (var item in dict) {
                    PropertiesSerializable serialized = new PropertiesSerializable();

                    serialized.Name = key;

                    // Reading the KeyValuePair information;
                    foreach (var kvp in item) {
                        switch (kvp.Key) {
                            case "Definition": serialized.Definition = kvp.Value; break;
                            case "ID":
                                byte ID = (byte) kvp.Value;
                                serialized.ID = ID;
                            break;
                            case "Address":
                                int address = (int) kvp.Value;
                                serialized.Address = address;
                            break;
                            case "Damage Sprites": serialized.DamageSprites = kvp.Value; break;
                            case "HP": serialized.HP = (byte) kvp.Value; break;
                            case "Copy Ability": serialized.CopyAbility = (byte) kvp.Value; break;
                            case "Palette": serialized.Palette = (byte) kvp.Value; break;
                        }
                    }
                    Properties property = serialized.DeserializeProperties();

                    propertiesDictionary[property.ID] = property;
                }
            }
        }

        void RandomizeAbilityProperties(GenerationOptions inhaleType, 
                                        Dictionary<byte, Data> dictionary, List<byte> abilities, 
                                        List<byte> shuffledAbilities) {
            if (inhaleType == GenerationOptions.Unchanged) return;

            int currentAbility = 0;

            foreach (byte id in dictionary.Keys) {
                Properties properties = propertiesDictionary[id];

                switch (inhaleType) {
                    case GenerationOptions.Shuffle:
                        properties.CopyAbility = shuffledAbilities[currentAbility];

                        currentAbility++;
                    break;

                    case GenerationOptions.Random:
                        int index = Utils.GetRandomNumber(0, abilities.Count);

                        properties.CopyAbility = abilities[index];
                    break;
                }

                modifiedProperties.Add(properties);
            }
        }

        public bool FilterEntities(Entity a) {
            return true;
        }
    }
}
