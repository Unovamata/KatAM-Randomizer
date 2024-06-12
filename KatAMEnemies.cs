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
        Dictionary<byte, List<byte>> enemyDictionary = new Dictionary<byte, List<byte>>();

        void RandomizeEnemies(IKatAMRandomizer Instance) {
            InitializeEnemyDictionary();

            entities = new List<Entity>();

            // Read the settings, entities, and ROM data;
            byte[] romFile = System.ROMData;

            // Deserializing all the entities in the game;
            Utils.DeserializeJSON(Utils.JSONToEntities(Utils.enemiesJson), entities, Instance);

            foreach(Entity entity in entities) {
                if (!enemyDictionary.ContainsKey(entity.ID)) {
                    enemyDictionary.Add(entity.ID, NewBehavior(entity.Behavior));
                } 

                List<byte> behaviorsList = enemyDictionary[entity.ID];

                if (!behaviorsList.Contains(entity.Behavior)) {
                    behaviorsList.Add(entity.Behavior);
                }
            }

            foreach(KeyValuePair<byte, List<byte>> kvp in enemyDictionary) {
                Console.WriteLine(Processing.enemiesDictionary[kvp.Key]);

                foreach(byte bit in kvp.Value) {
                    Console.WriteLine($"Behaviour: {bit}");
                }
            }
        }

        void InitializeEnemyDictionary() {
            enemyDictionary = new Dictionary<byte, List<byte>>();

            // Squishy;
            enemyDictionary.Add(0x04, NewBehavior(0x01)); // Appear and launch upwards;
            enemyDictionary[0x04].Add(0x02); // Hop and chase Kirby;

            // Scarfy;
            enemyDictionary.Add(0x05, NewBehavior(0x03)); // Rise from below and places itself at Kirby's current Y;

            // Gordo;
            enemyDictionary.Add(0x06, NewBehavior(0x03)); // Hover up and down slowly;

            // Haley;
            enemyDictionary.Add(0x0A, NewBehavior(0x01)); // Fly with regular speed instead of slowing down;

            // Cupie;
            enemyDictionary.Add(0x0C, NewBehavior(0x02)); // Fly in 8 pattern;
            enemyDictionary[0x0C].Add(0x03); // Run away from Kirby, shoot, disappear out of the screen;

            // Leap;
            enemyDictionary.Add(0x0F, NewBehavior(0x01)); // Hover up and down slowly;

            // Big Waddle Dee;
            enemyDictionary.Add(0x11, NewBehavior(0x02)); // Jump;

            // Golem Press;
            enemyDictionary.Add(0x1F, NewBehavior(0x01)); // Do all the Golem attacks;

            // Golem Roll;
            enemyDictionary.Add(0x20, NewBehavior(0x01)); // Do all the Golem attacks;

            // Boxin;
            enemyDictionary.Add(0x25, NewBehavior(0x03)); // Stand in place;

            // Cookin;
            enemyDictionary.Add(0x26, NewBehavior(0x01)); // Walking;

            // Heavy Knight;
            enemyDictionary.Add(0x29, NewBehavior(0x02)); // Stand in place;

            // Giant Rocky;
            enemyDictionary.Add(0x2A, NewBehavior(0x01)); // Stand in place;

            // Batty;
            enemyDictionary.Add(0x2D, NewBehavior(0x01)); // Chase Kirby and return to its spawn point;

            // Shotzo;
            enemyDictionary.Add(0x35, NewBehavior(0x01)); // Fire at the upper left section of the screen;

        }

        List<byte> NewBehavior(byte input) {
            return new List<byte>() { input };
        }

        public bool FilterEntities(Entity entity) {

            return true;
        }
    }
}
