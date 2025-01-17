using KatAMInternal;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using static System.Windows.Forms.VisualStyles.VisualStyleElement;
using KatAM_Randomizer;

namespace KatAMRandomizer
{
    internal class KatAMMapElements : KatAMRandomizerComponent, IKatAMRandomizer {

        public KatAMMapElements(Processing system) {
            InitializeComponents(system);

            ManageMapElements(this);
        }

        GenerationOptions stoneDoorOptions;
        bool isRemovingStoneBlocks;

        void ManageMapElements(IKatAMRandomizer Instance) {
            entities = new List<Entity>();

            stoneDoorOptions = Settings.StoneDoorGenerationType;
            isRemovingStoneBlocks = Settings.isRemovingStoneBlocks;

            // Read the settings, entities, and ROM data;
            byte[] romFile = Processing.ROMData;

            // Deserializing all the entities in the game;
            Utils.DeserializeEntitiesJSON(Utils.JSONToObjects(Utils.worldMapObjectsJson), entities, Instance);

            foreach (Entity entity in entities) {
                bool isButtonOrDoor = (entity.ID == 0x6D || entity.ID == 0x71);

                switch (stoneDoorOptions) {
                    case GenerationOptions.All:
                        entity.ID = Utils.Nothing;
                    break;

                    case GenerationOptions.Presets:
                        if(isButtonOrDoor && entity.Room == 804) entity.ID = Utils.Nothing;
                    break;
                }
                
                if (isRemovingStoneBlocks) {
                    /* Removing this specific block can lead to a softlocked run.
                     * This block unlocks a door that can make the difference between a 
                     * beatable and an unbeatable run. Do not remove this object at any cost; */
                    if(entity.Room == 0x13B) continue;

                    bool isStoneBlock = Processing.mapElementsDictionary.ContainsKey(entity.ID) &&
                                        Processing.mapElementsDictionary[entity.ID] == "Star Stone Block";

                    if (isStoneBlock) entity.ID = Utils.Nothing;
                }

                Utils.WriteObjectToROM(entity);
            }
        }

        public bool FilterEntities(Entity entity) {
            return true;
        }
    }
}
