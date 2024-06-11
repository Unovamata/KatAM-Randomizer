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

        void RandomizeEnemies(IKatAMRandomizer Instance) {
            entities = new List<Entity>();

            // Read the settings, entities, and ROM data;
            byte[] romFile = System.ROMData;

            // Deserializing all the entities in the game;
            Utils.DeserializeJSON(Utils.JSONToEntities(Utils.enemiesJson), entities, Instance);

            foreach(Entity entity in entities) {
                Console.WriteLine(entity.Name);
            }
        }

        public bool FilterEntities(Entity entity) {

            return true;
        }
    }
}
