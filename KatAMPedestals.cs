using KatAMInternal;

namespace KatAMRandomizer {
    internal class KatAMPedestals : KatAMRandomizerComponent, IKatAMRandomizer {
        public KatAMPedestals(Processing system) {
            InitializeComponents(system);
            RandomizeAbilityPedestals(this);
        }

        // Pedestal ID 0x92;
        static List<byte> pedestal146AbilitiesList = new List<byte>() {
            0x00,  // Beam
            0x01,  // Flame
            0x02,  // Laser
            0x03,  // Fire
            0x04,  // Rock
            0x05,  // Fighter
            0x06,  // Bomb
            0x07   // Tornado
        };

        // Pedestal ID 0x93;
        static List<byte> pedestal147AbilitiesList = new List<byte>() {
            0x00,  // Wheel
            0x01,  // Ice
            0x02,  // Electric
            0x03,  // Missile
            0x04,  // Smash
            0x05,  // Throw
            0x06   // Hammer
        };

        // Pedestal ID 0x94;
        static List<byte> pedestal148AbilitiesList = new List<byte>() {
            0x00   // Parasol
        };

        // Pedestal ID 0x95;
        static List<byte> pedestal149AbilitiesList = new List<byte>() {
            0x00,  // Sword
            0x01,  // Cutter
            0x02   // Cupid
        };

        // Pedestal ID 0x96;
        static List<byte> pedestal150AbilitiesList = new List<byte>() {
            0x00   // Random
        };

        public static void RandomizeAbilityPedestals(IKatAMRandomizer Instance) {
            entities = new List<Entity>();

            byte[] romFile = System.ROMData;

            Utils.DeserializeJSON(Utils.JSONToEntities(Utils.abilityStandsJson), entities, Instance);

            foreach (Entity entity in entities) {
                if (Utils.IsVetoedRoom(entity)) continue;

                int selectedPedestal = Utils.GetRandomNumber(0, 4);
                byte selectedID = 0, selectedAbility = 0;

                switch (selectedPedestal) {
                    case 0:
                        selectedID = 0x92;
                        selectedAbility = SelectPedestalAbility(pedestal146AbilitiesList);
                    break;

                    case 1:
                        selectedID = 0x93;
                        selectedAbility = SelectPedestalAbility(pedestal147AbilitiesList);
                    break;

                    case 2:
                        selectedID = 0x94;
                        selectedAbility = SelectPedestalAbility(pedestal148AbilitiesList);
                    break;

                    case 3:
                        selectedID = 0x95;
                        selectedAbility = SelectPedestalAbility(pedestal149AbilitiesList);
                    break;

                    case 4:
                        selectedID = 0x96;
                        selectedAbility = SelectPedestalAbility(pedestal150AbilitiesList);
                    break;
                }

                entity.ID = selectedID;
                entity.Behavior = selectedAbility;

                Utils.WriteObjectToROM(romFile, entity);
            }
        }

        static byte SelectPedestalAbility(List<byte> list) {
            int index = (byte) Utils.GetRandomNumber(0, list.Count);

            return list[index];
        }

        //DeserializeEntitiesForRandomization(); Preparing the objects for the randomization process;
        public bool FilterEntities(Entity entity) {
            return true;
        }
    }
}
