using KatAMInternal;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace KatAM_Randomizer {
    internal class KatAMMirrors : KatAMRandomizerComponent, IKatAMRandomizer {

        public KatAMMirrors(Processing system) {
            LoadEnemyDataset(this, Utils.mirrorsJson);

            foreach(Entity entity in entities) {
                Console.WriteLine($"Address: {entity.Address}");
            }
        }
        
        protected void LoadEnemyDataset(IKatAMRandomizer Instance, string file) {
            entities = new List<Entity>();

            // Deserializing all the entities in the game;
            Utils.DeserializeEntitiesJSON(Utils.JSONToObjects(file), entities, Instance);
        }

        public bool FilterEntities(Entity entity) { return true; }
    }
}
