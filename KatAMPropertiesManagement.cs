using KatAMInternal;
using KatAMRandomizer;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Linq;

namespace KatAM_Randomizer {
    internal class KatAMPropertiesManagement : KatAMRandomizerComponent, IKatAMRandomizer {

        public KatAMPropertiesManagement(Processing system) {
            InitializeComponents(system);

            ManageProperties(this);
        }

        Dictionary<byte, Properties> propertiesDictionary = new Dictionary<byte, Properties>();
        List<Properties> modifiedProperties = new List<Properties>();

        List<byte> abilityIndexes = new List<byte>() {
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

        GenerationOptions EnemiesInhaleAbilityType,
                          MinibossesInhaleAbilityType;
                          

        public bool isEnemyIncludingNormalInhaleAbility,
                    isEnemyIncludingMasterInhaleAbility,
                    isEnemyIncludingMixInhaleAbility,
                    isMinibossIncludingNormalInhaleAbility,
                    isMinibossIncludingMasterInhaleAbility,
                    isMinibossIncludingMixInhaleAbility;

        void ManageProperties(IKatAMRandomizer Instance) {
            propertiesDictionary = new Dictionary<byte, Properties>();

            byte[] romFile = System.ROMData;

            DeserializePropertiesJSON(Utils.JSONToObjects(Utils.propertiesJson), Instance);

            EnemiesInhaleAbilityType = Settings.EnemiesInhaleAbilityType;
            isEnemyIncludingNormalInhaleAbility = Settings.isEnemyIncludingNormalInhaleAbility;
            isEnemyIncludingMasterInhaleAbility = Settings.isEnemyIncludingMasterInhaleAbility;
            isEnemyIncludingMixInhaleAbility = Settings.isEnemyIncludingMixInhaleAbility;

            MinibossesInhaleAbilityType = Settings.MinibossesInhaleAbilityType;
            isMinibossIncludingNormalInhaleAbility = Settings.isMinibossIncludingNormalInhaleAbility;
            isMinibossIncludingMasterInhaleAbility = Settings.isMinibossIncludingMasterInhaleAbility;
            isMinibossIncludingMixInhaleAbility = Settings.isMinibossIncludingMixInhaleAbility;

            List<byte> enemyAbilityIndexes = new List<byte>(abilityIndexes);

            if (isEnemyIncludingNormalInhaleAbility) enemyAbilityIndexes.Add(0x00);
            if (isEnemyIncludingMasterInhaleAbility) enemyAbilityIndexes.Add(0x1A);
            if (isEnemyIncludingMixInhaleAbility) enemyAbilityIndexes.Add(0x1B);

            if(EnemiesInhaleAbilityType != GenerationOptions.Unchanged) {
                foreach (byte id in Processing.enemiesDictionary.Keys) {
                    Properties properties = propertiesDictionary[id];

                    switch (EnemiesInhaleAbilityType) {
                        case GenerationOptions.Shuffle:

                        break;

                        case GenerationOptions.Random:
                            int index = Utils.GetRandomNumber(0, enemyAbilityIndexes.Count);

                            properties.CopyAbility = enemyAbilityIndexes[index];
                        break;
                    }

                    modifiedProperties.Add(properties);
                }
            }
            

            List<byte> minibossAbilityIndexes = new List<byte>(abilityIndexes);

            if (isMinibossIncludingNormalInhaleAbility) minibossAbilityIndexes.Add(0x00);
            if (isMinibossIncludingMasterInhaleAbility) minibossAbilityIndexes.Add(0x1A);
            if (isMinibossIncludingMixInhaleAbility) minibossAbilityIndexes.Add(0x1B);

            foreach(Properties property in modifiedProperties) {
                Utils.WritePropertiesToROM(romFile, property);
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

        public bool FilterEntities(Entity a) {
            return true;
        }
    }
}
