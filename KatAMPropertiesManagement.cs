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

        List<Properties> properties = new List<Properties>();

        void ManageProperties(IKatAMRandomizer Instance) {
            properties = new List<Properties>();

            Console.WriteLine(properties.Count);

            byte[] romFile = System.ROMData;

            DeserializePropertiesJSON(Utils.JSONToObjects(Utils.propertiesJson), Instance);

            Console.WriteLine(properties.Count);
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

                    properties.Add(property);
                }
            }
        }

        public bool FilterEntities(Entity a) {
            return true;
        }
    }
}
