using KatAMInternal;
using KatAM_Randomizer;

namespace KatAMRandomizer
{
    internal class KatAMPedestals : KatAMRandomizerComponent, IKatAMRandomizer {
        GenerationOptions pedestalsOptions;

        public KatAMPedestals(Processing system) {
            InitializeComponents(system);

            pedestalsOptions = Settings.PedestalsGenerationType;

            bool isRandomizing = pedestalsOptions != GenerationOptions.Unchanged;

            if (isRandomizing) RandomizeAbilityPedestals(this);
        }

        // Pedestal ID 0x92;
        List<byte> pedestal146AbilitiesList = new List<byte>() {
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
        List<byte> pedestal147AbilitiesList = new List<byte>() {
            0x00,  // Wheel
            0x01,  // Ice
            0x02,  // Electric
            0x03,  // Missile
            0x04,  // Smash
            0x05,  // Throw
            0x06   // Hammer
        };

        // Pedestal ID 0x94;
        List<byte> pedestal148AbilitiesList = new List<byte>() {
            0x00   // Parasol
        };

        // Pedestal ID 0x95;
        List<byte> pedestal149AbilitiesList = new List<byte>() {
            0x00,  // Sword
            0x01,  // Cutter
            0x02   // Cupid
        };

        // Pedestal ID 0x96;
        List<byte> pedestal150AbilitiesList = new List<byte>() {
            0x00   // Random
        };

        List<KeyValuePair<byte, byte>> unlockPathAbilities = new List<KeyValuePair<byte, byte>>() {
            new KeyValuePair<byte, byte>(0x92, 0x01),
            new KeyValuePair<byte, byte>(0x92, 0x03),
            new KeyValuePair<byte, byte>(0x92, 0x04),
            new KeyValuePair<byte, byte>(0x92, 0x06),
            new KeyValuePair<byte, byte>(0x93, 0x04),
            new KeyValuePair<byte, byte>(0x93, 0x06),
            new KeyValuePair<byte, byte>(0x94, 0x00),
            new KeyValuePair<byte, byte>(0x94, 0x01),
        };

        // Store ID with Behaviour;
        List<KeyValuePair<byte, byte>> objectIDs = new List<KeyValuePair<byte, byte>>();

        int maxIndex = 4;
        bool isParasolBanned, isAddingRandomPedestal;

        void RandomizeAbilityPedestals(IKatAMRandomizer Instance) {
            entities = new List<Entity>();
            objectIDs = new List<KeyValuePair<byte, byte>>();

            byte[] romFile = System.ROMData;

            Utils.DeserializeJSON(Utils.JSONToEntities(Utils.abilityStandsJson), entities, Instance);

            isParasolBanned = Settings.isBanningParasol;
            isAddingRandomPedestal = Settings.isAddingRandomPedestal;

            if (pedestalsOptions == GenerationOptions.Shuffle) {
                objectIDs = Utils.Shuffle(objectIDs);
            } else if (pedestalsOptions == GenerationOptions.Random) {
                if (isAddingRandomPedestal) maxIndex = 5;
            }

            

            for (int i = 0; i < entities.Count; i++) {
                Entity entity = entities[i];

                if (Utils.IsVetoedRoom(entity)) continue;

                switch (pedestalsOptions) {
                    case GenerationOptions.Shuffle:
                        entity.ID = objectIDs[i].Key;
                        entity.Behavior = objectIDs[i].Value;
                    break;

                    case GenerationOptions.Random:
                        SelectRandomPedestalAbility(entity);
                    break;

                    case GenerationOptions.Challenge:
                        int chance = Utils.Dice();
                        bool isSpawningPedestal = chance <= 3;

                        if (isSpawningPedestal) {
                            SelectRandomPedestalAbility(entity);
                        } else {
                            entity.ID = Utils.Nothing;
                        }
                    break;

                    // Unlock Path Abilities Only;
                    case GenerationOptions.Presets:
                        int randomIndex = Utils.GetRandomNumber(0, unlockPathAbilities.Count);
                        KeyValuePair<byte, byte> kvp = unlockPathAbilities[randomIndex];

                        entity.ID = kvp.Key;
                        entity.Behavior = kvp.Value;
                    break;

                    // Spawn nothing;
                    case GenerationOptions.No:
                        entity.ID = Utils.Nothing;
                    break;

                    case GenerationOptions.Custom: break;
                }

                Utils.WriteObjectToROM(romFile, entity);
            }
        }

        void SelectRandomPedestalAbility(Entity entity) {
            int selectedPedestal = Utils.GetRandomNumber(0, maxIndex);
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
                    if (isParasolBanned) {
                        List<byte> unbannedPedestals = new List<byte>() { 0x92, 0x93, 0x95 };

                        if (isAddingRandomPedestal) unbannedPedestals.Add(0x96);

                        selectedPedestal = Utils.GetRandomNumber(0, unbannedPedestals.Count);
                        selectedID = unbannedPedestals[selectedPedestal];

                        switch (selectedID) {
                            case 0x92:
                                selectedAbility = SelectPedestalAbility(pedestal148AbilitiesList);
                            break;

                            case 0x93:
                                selectedAbility = SelectPedestalAbility(pedestal148AbilitiesList);
                            break;

                            case 0x95:
                                selectedAbility = SelectPedestalAbility(pedestal148AbilitiesList);
                            break;

                            case 0x96:
                                selectedAbility = SelectPedestalAbility(pedestal148AbilitiesList);
                            break;
                        }
                    } else {
                        selectedID = 0x94;
                        selectedAbility = SelectPedestalAbility(pedestal148AbilitiesList);
                    }
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
        }

        byte SelectPedestalAbility(List<byte> list) {
            int index = (byte) Utils.GetRandomNumber(0, list.Count);

            return list[index];
        }

        //FilterEntities(); Filtering the objects for the randomization process;
        public bool FilterEntities(Entity entity) {
            objectIDs.Add(new KeyValuePair<byte, byte>(entity.ID, entity.Behavior));

            return true;
        }
    }
}
