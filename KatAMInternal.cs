using Newtonsoft.Json;
using System.ComponentModel;
using System.Collections.Generic;
using KatAMRandomizer;
using KatAM_Randomizer;
using System.Net;

namespace KatAMInternal
{
    public enum GenerationOptions {
        Unchanged = 0,
        Presets = 1,
        RandomAndPresets = 2,
        Random = 3,
        Shuffle = 4,
        All = 5,
        No = 6,
        Challenge = 7,
        Custom = 8,
        Remove = 9
    }

    internal class Processing {
        public string ROMPath { get; set; }
        public string ROMDirectory { get; set; }
        public string ROMFilename { get; set; }
        public byte[] ROMData { get; set; }
        public Settings Settings { get; set; }

        public Processing() {
            Settings = new Settings();

            MergeSpecialDictionaries(parameters, enemiesDictionary);
            MergeSpecialDictionaries(parameters, minibossesDictionary);
            MergeDictionaries(parameters, bossesDictionary);
            MergeDictionaries(parameters, itemsDictionary);
            MergeDictionaries(parameters, mirrorsDictionary);
            MergeDictionaries(parameters, abilityStandsDictionary);
            MergeDictionaries(parameters, mapElementsDictionary);
        }

        public void MergeSpecialDictionaries(Dictionary<byte, string> destination, Dictionary<byte, Tuple<string, byte, bool>> source) {
            foreach (var item in source) {
                destination[item.Key] = item.Value.Item1;
            }
        }

        public void MergeDictionaries(Dictionary<byte, string> destination, Dictionary<byte, string> source) {
            foreach(var item in source) {
                destination[item.Key] = item.Value;
            }
        }

        // By Fyyyyik; https://github.com/Fyyyyik/KatAM-Object-Editor/blob/main/Editor/LevelDataManager.cs
        public static List<int> roomIds = new List<int>(){
            0x321, 0x323, 0x324, 0x325, 0x0, 0x3D4, 0x3D5, 0x3D6,
            0xB4, 0xB5, 0x214, 0x216, 0x217, 0x218, 0x219, 0x21A,
            0x21E, 0x21F, 0x23A, 0x23B, 0x24E, 0x65, 0x67, 0x68,
            0x6A, 0x6B, 0x6C, 0x6E, 0x8C, 0x8D, 0x8E, 0x8F, 0xBE,
            0x7F, 0x88, 0x89, 0xC2, 0x1F8, 0x1FB, 0x22A, 0x22B,
            0x1F7, 0x22C, 0x22D, 0x7E, 0x73, 0x75, 0x76, 0x77,
            0x8A, 0x8B, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB0, 0xB1,
            0xB2, 0xC0, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0xC3,
            0x2BC, 0x2C0, 0x2C1, 0x2C2, 0x2C3, 0x2C4, 0x2C5, 0x2C6,
            0x2C7, 0x2C8, 0x2E4, 0x2E5, 0x2E6, 0x316, 0x2BE, 0x2E7,
            0x2E8, 0x2E9, 0xC8, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xD0,
            0xD1, 0xD2, 0xD3, 0xD6, 0xDC, 0xE2, 0xE3, 0xE4, 0xE5,
            0xE6, 0xE7, 0xE8, 0x122, 0xD4, 0xD7, 0xD8, 0xD9, 0xDA,
            0xDB, 0xDD, 0xDE, 0xDF, 0xE0, 0xE1, 0x123, 0x215, 0x21B,
            0x21C, 0x21D, 0x220, 0x221, 0x222, 0x223, 0x12C, 0x130,
            0x134, 0x136, 0x138, 0x139, 0x13B, 0x13C, 0x13D,
            0x13E, 0x141, 0x142, 0x143, 0x144, 0x145, 0x146, 0x186,
            0x190, 0x191, 0x192, 0x193, 0x194, 0x195, 0x196, 0x197,
            0x198, 0x199, 0x19B, 0x19C, 0x19D, 0x19E, 0x19F,
            0x1A0, 0x1A1, 0x1A2, 0x1A3, 0x1A4, 0xA15, 0x1A6,
            0x1A7, 0x1A8, 0x1A9, 0x1AA, 0x1AB, 0x1EA, 0x1F4,
            0x1FC, 0x1FD, 0x1FE, 0x204, 0x210, 0x1F5, 0x1FF,
            0x200, 0x201, 0x202, 0x203, 0x205, 0x20B, 0x20F,
            0x211, 0x212, 0x213, 0x24F, 0xBF, 0x82, 0x83, 0x84,
            0x85, 0x86, 0x87, 0x227, 0x229, 0x32B, 0x32E, 0x32F,
            0x337, 0x338, 0x339, 0x33A, 0x33B, 0x33C, 0x33D,
            0x33E, 0x33F, 0x340, 0x37A, 0x320, 0x322, 0x32A,
            0x330, 0x333, 0x335, 0x336, 0x250, 0x259, 0x25B,
            0x25C, 0x25D, 0x262, 0x263, 0x26C, 0x26D, 0x26E,
            0x26F, 0x260, 0x261, 0x264, 0x265, 0x266, 0x268,
            0x269, 0x26A, 0x26B, 0x2B2, 0x1F6, 0x20C, 0x20D,
            0x20E, 0x66, 0x71, 0x72, 0x81, 0x78, 0x79, 0x7A, 0xAA,
            0xC1, 0x258, 0x2CA, 0x2CC, 0x2CD, 0x2CF, 0x2DC,
            0x2DD, 0x2DE, 0x2DF, 0x2E0, 0x2E1, 0x2E2, 0x2E3,
            0x2D0, 0x2DA, 0x2DB, 0x38D, 0x38E, 0x38F, 0x390,
            0x391, 0x392, 0x393, 0x394, 0x396, 0x397, 0x3B6,
            0x3B7, 0x3BB, 0x3BC, 0x3BD, 0x3C9, 0x0, 0x3CA,
        };

        // https://www.tapatalk.com/groups/lighthouse_of_yoshi/kirby-and-the-amazing-mirror-hacking-t741.html
        /*Prop(Enemy Name, Ability (Default: true), Is Inhalable? (Default: true))*/
        public static Dictionary<byte, Tuple<string, byte, bool>> enemiesDictionary = new Dictionary<byte, Tuple<string, byte, bool>>{
            { 0x00, Prop("Waddle Dee")},
            { 0x01, Prop("Bronto Burt")},
            { 0x02, Prop("Blipper")},
            { 0x03, Prop("Glunk")},
            { 0x04, Prop("Squishy")},
            { 0x05, Prop("Scarfy", 0x00, false)},
            { 0x06, Prop("Gordo", 0x00, false)},
            { 0x07, Prop("Snooter")},
            { 0x08, Prop("Chip")},
            { 0x09, Prop("Soarar")},
            { 0x0A, Prop("Haley")},
            { 0x0B, Prop("Roly-Poly")},
            { 0x0C, Prop("Cupie", 0x13)},
            { 0x0D, Prop("Blockin", 0x00, false)},
            { 0x0E, Prop("Snooter 2")},
            { 0x0F, Prop("Leap")},
            { 0x10, Prop("Jack")},
            { 0x11, Prop("Big Waddle Dee")},
            { 0x12, Prop("Waddle Doo", 0x07)},
            { 0x13, Prop("Flamer", 0x03)},
            { 0x14, Prop("Hot Head", 0x01)},
            { 0x15, Prop("Laser Ball", 0x0D) },
            { 0x16, Prop("Pengy", 0x02)},
            { 0x17, Prop("Rocky", 0x08)},
            { 0x18, Prop("Sir Kibble", 0x06)},
            { 0x19, Prop("Sparky", 0x0F)},
            { 0x1A, Prop("Sword Knight", 0x12)},
            { 0x1B, Prop("UFO", 0x0E)},
            { 0x1C, Prop("Twister", 0x10)},
            { 0x1D, Prop("Wheelie", 0x04)},
            { 0x1E, Prop("Noddy", 0x0B)},
            { 0x1F, Prop("Golem Press", 0x08)},
            { 0x20, Prop("Golem Roll", 0x04)},
            { 0x21, Prop("Golem Punch", 0x14) },
            { 0x22, Prop("Foley", 0x09)},
            { 0x23, Prop("Shooty")},
            { 0x24, Prop("Scarfy 2", 0x00, false)},
            { 0x25, Prop("Boxin", 0x14)},
            { 0x26, Prop("Cookin", 0x0C)},
            { 0x27, Prop("Minny", 0x17)},
            { 0x28, Prop("Bomber", 0x18)},
            { 0x29, Prop("Heavy Knight", 0x12)},
            { 0x2A, Prop("Giant Rocky", 0x08)},
            { 0x2B, Prop("Metal Guardian", 0x0D)},
            { 0x2C, Prop("Nothing", 0x00, false)},
            { 0x2D, Prop("Batty")},
            { 0x2E, Prop("Foley Automatic", 0x09)},
            { 0x2F, Prop("Bang-Bang", 0x19)},
            { 0x30, Prop("Explosion", 0x00, false)},
            { 0x31, Prop("Nothing", 0x00, false)},
            { 0x32, Prop("Droppy")},
            { 0x33, Prop("Prank")},
            { 0x34, Prop("Mirra", 0x00, false) },
            { 0x35, Prop("Shotzo", 0x00, false)},
            /*{ 0x36, "Nothing" },*/ // Shadow Kirby with behavior 0 is nothing;
            { 0x37, Prop("Waddle Dee 2")},
            { 0x70, Prop("Shotzo 2", 0x00, false)},
            { 0xA2, Prop("Parasol", 0x05)}
        };

        public static Dictionary<byte, Tuple<string, byte, bool>> minibossesDictionary = new Dictionary<byte, Tuple<string, byte, bool>>{
            { 0x38, Prop("Mr. Frosty", 0x02)},
            { 0x39, Prop("Bonkers", 0x11)},
            { 0x3A, Prop("Phan Phan", 0x0A)},
            { 0x3B, Prop("Batafire", 0x03)},
            { 0x3C, Prop("Box Boxer", 0x14)},
            { 0x3D, Prop("Boxy", 0x15)},
            { 0x3E, Prop("Master Hand", 0x16)},
            { 0x3F, Prop("Bombar", 0x19)},
        };

        public static Dictionary<byte, string> bossesDictionary = new Dictionary<byte, string>{
            { 0x45, "Kracko" },
            { 0x46, "King Golem" },
            { 0x47, "Master Hand (Boss)"},
            { 0x48, "Gobbler"},
            { 0x49, "Wiz" },
            { 0x4A, "Moley" },
            { 0x4B, "Mega Titan" },
            { 0x4C, "Titan Head" },
            { 0x4D, "Crazy Hand" },
            { 0x4E, "Dark Meta Knight (Boss)" },
            { 0x4F, "Dark Mind (1)"},
            { 0x50, "Dark Mind (2)"},
            { 0x51, "Dark Mind (Shooting)" },
            { 0x52, "Dark Meta Knight (Radish)" },
        };

        public static Dictionary<byte, string> itemsDictionary = new Dictionary<byte, string>{
            { 0x5E, "Cherry" },
            { 0x5F, "Pep Brew" },
            { 0x60, "Meat"},
            { 0x61, "Maximum Tomato"},
            { 0x62, "Battery" },
            { 0x63, "1UP" },
            { 0x64, "Lollipop" },
            { 0x65, "Mirror Shard" },
            { 0x66, "Nothing"},
            { 0x67, "Nothing"},
            { 0x68, "Nothing"},
            { 0x69, "Nothing"},
            { 0x6A, "Nothing"},
            { 0x6B, "Nothing"},
            { 0x6C, "Nothing"},
            { 0x80, "Small Chest"},
            { 0x81, "Big Chest"}
        };

        public static Dictionary<byte, string> mirrorsDictionary = new Dictionary<byte, string>{
            { 0x6F, "Mirror" },
            { 0x84, "Warp Star" },
            { 0x85, "Warp Star [Goal Game]" },
            { 0x8C, "Special Hub Mirror"},
        };

        public static Dictionary<byte, string> abilityStandsDictionary = new Dictionary<byte, string>{
            { 0x92, "Ability Stand" },
            { 0x93, "Ability Stand" },
            { 0x94, "Ability Stand" },
            { 0x95, "Ability Stand" },
            { 0x96, "Random Ability Stand" },
        };

        public static Dictionary<byte, string> mapElementsDictionary = new Dictionary<byte, string>{
            { 0x6D, "Small Button"},
            { 0x71, "Stone Sliding Door"},
            { 0x79, "Star Stone Block"}, // Small;
            { 0x7D, "Star Stone Block"},
            { 0x7E, "Star Stone Block"},
            { 0x86, "Big Switch" },
            { 0x88, "Star Stone Block"},
            { 0x89, "Star Stone Block"},
        };

        public static Dictionary<byte, string> parameters = new Dictionary<byte, string>() {
            { 0x36, "Nothing" }, // Shadow Kirby with behavior 0 is nothing;
            { 0x40, "???" },
            { 0x41, "???" },
            { 0x42, "???" },
            { 0x43, "???" },
            { 0x44, "???" },
            { 0x53, "Nothing" },
            { 0x54, "Nothing" },
            { 0x55, "Nothing" },
            { 0x56, "Nothing" },
            { 0x57, "Nothing" },
            { 0x58, "Nothing" },
            { 0x59, "Nothing" },
            { 0x5A, "Nothing" },
            { 0x5B, "Nothing" },
            { 0x5C, "Nothing" },
            { 0x5D, "Nothing" },
            { 0x6E, "Small Flame" },
            { 0x70, "Unused Shotzo Variant" },
            { 0x71, "Sliding Pillar" },
            { 0x72, "Boss Endurance Mirror" },
            { 0x73, "???" },
            { 0x74, "Crumbling Block" },
            { 0x75, "???" },
            { 0x76, "???" },
            { 0x77, "Camera Locker Object" },
            { 0x78, "Fuse" },
            { 0x7A, "???" },
            { 0x7B, "Star Platform" },
            { 0x7C, "Wall Flame" },
            { 0x7F, "CPU mover" },
            { 0x82, "???" },
            { 0x83, "???" },
            { 0x87, "???" },
            { 0x8A, "Fuse Cannon" },
            { 0x8B, "Manual Direction Cannon" },
            { 0x8D, "Stake" },
            { 0x8E, "Cutscene Loader" },
            { 0x8F, "???" },
            { 0x90, "Air Current Effect" },
            { 0x91, "???" },
            { 0x97, "Freezes the Game" },
            { 0x98, "Resets the Game" },
            { 0x99, "???" },
            { 0x9A, "Resets the Game" },
            { 0x9B, "King Golem's Rock" },
            { 0x9C, "King Golem's Gordo" },
            { 0x9D, "Foley's Propeller Leaves" },
            { 0x9E, "Frying Pan" },
            { 0x9F, "Pacriel" },
            { 0xA0, "Sir Kibble's Cutter Projectile" },
            { 0xA1, "???" },
            { 0xA2, "Parasol Enemy" },
            { 0xA3, "Ability Star" },
            { 0xA4, "Master Sword Ability Star" },
            { 0xA5, "Impact Star" },
            { 0xA6, "Mr. Frosty's Ice Cube" },
            { 0xA7, "Mr. Frosty's Big Ice Cube" },
            { 0xA8, "Bonker's Coconut" },
            { 0xA9, "Bonker's Big Coconut" },
            { 0xAA, "Phan Phan's Apple" },
            { 0xAB, "Prank's Fireball" },
            { 0xAC, "Prank's Ice Cube" },
            { 0xAD, "Prank's Bomb" },
            { 0xAE, "Prank's Frying Pan" },
            { 0xAF, "Banana Peel" },
            { 0xB0, "Boxy's Present Box" },
            { 0xB1, "Exhaled Star" },
            { 0xB2, "Bombar's Bomb" },
            { 0xB3, "Bombar's Missile" },
            { 0xB4, "Box Boxer's Hadouken" },
            { 0xB5, "Wiz' Rugby Ball" },
            { 0xB6, "Wiz' Mini-car" },
            { 0xB7, "Wiz' Balloon" },
            { 0xB8, "Wiz' Bomb" },
            { 0xB9, "Wiz' Cloud" },
            { 0xBA, "Poisonous Apple" },
            { 0xBB, "Wiz' Droppy Variant" },
            { 0xBC, "Mega Titan Arm 1" },
            { 0xBD, "Mega Titan Arm 2" },
            { 0xBE, "Mega Titan Arm 3" },
            { 0xBF, "Mega Titan Arm 4" },
            { 0xC0, "Titan Head's Missile" },
            { 0xC1, "Moley's Boulder" },
            { 0xC2, "Moley's Screw" },
            { 0xC3, "Moley's Tire" },
            { 0xC4, "Moley's Big Bomb" },
            { 0xC5, "Moley's Massive Boulder" },
            { 0xC6, "Moley's Oil Barrel" },
            { 0xC7, "Moley's Spike Ball" },
            { 0xC8, "Master Hand's Fire Bullet" },
            { 0xC9, "Small Bomb" },
            { 0xCA, "???" },
            { 0xCB, "Dark Mind's Red Star" },
            { 0xCC, "Dark Mind's Blue Star" },
            { 0xCD, "Dark Mind's Purple Star" },
            { 0xCE, "Dark Mind's Green Star" },
            { 0xCF, "???" },
            { 0xD0, "???" },
            { 0xD1, "???" },
            { 0xD2, "Dark Mind's Laser" },
            { 0xD3, "Dark Mind's Shooter Cutter" },
            { 0xD4, "Crashes the Game" },
            { 0xD5, "Cutter Projectile" },
            { 0xD6, "Glunk's Bullet" },
            { 0xD7, "Shotzo's Bullet" },
            { 0xD8, "Cupie's Arrow" },
            { 0xD9, "???" },
            { 0xDA, "Shooty's Bomb" },
            { 0xDB, "Resets the Game" }
        };

        static Tuple<string, byte, bool> Prop(string name, byte abilityID = 0x00, bool isInhalable = true) {
            return new Tuple<string, byte, bool>(name, abilityID, isInhalable);
        }

        static Tuple<string, byte, bool> Null(string name, byte abilityID = 0x00, bool isInhalable = false) {
            return new Tuple<string, byte, bool>(name, abilityID, isInhalable);
        }
    }

    public class Settings {
        private static Settings Instance;
        public Random RandomEntity { get; set; }
        public int Seed { get; set; }
        public int MinSeed = -9999999, MaxSeed = 9999999;
        public bool IsRace { get; set; }
        public GenerationOptions ConsumablesGenerationType { get; set; }
        public GenerationOptions MirrorShardsGenerationType { get; set; }
        public int amountOfMirrorShardsToAdd = 0;
        public GenerationOptions ChestsGenerationType { get; set; }
        public GenerationOptions ChestsPropertiesType { get; set; }
        public bool isAddingMoreHPUps { get; set; }
        public int HPUpsAdded { get; set; }
        public GenerationOptions EnemiesGenerationType { get; set; }
        public bool isRandomizingExcludedEnemies { get; set; }
        public bool isIncludingMiniBosses { get; set; }
        public bool isRandomizingEnemiesIntelligently { get; set; }
        public GenerationOptions EnemiesPropertiesSpeedType { get; set; }
        public GenerationOptions EnemiesPropertiesBehaviorType { get; set; }
        public bool isUsingUnusedBehaviors { get; set; }
        public GenerationOptions EnemiesInhaleAbilityType { get; set; }
        public bool isEnemyIncludingNormalInhaleAbility { get; set; }
        public bool isEnemyIncludingMasterInhaleAbility { get; set; }
        public bool isEnemyIncludingMixInhaleAbility { get; set; }
        public GenerationOptions EnemiesHPType { get; set; }
        public bool isEnemyHPPercentageModified { get; set; }
        public GenerationOptions MinibossesGenerationType { get; set; }
        public GenerationOptions MinibossesPropertiesSpeedType { get; set; }
        public GenerationOptions MinibossesPropertiesBehaviorType { get; set; }
        public GenerationOptions MinibossesInhaleAbilityType { get; set; }
        public bool isMinibossIncludingNormalInhaleAbility { get; set; }
        public bool isMinibossIncludingMasterInhaleAbility { get; set; }
        public bool isMinibossIncludingMixInhaleAbility { get; set; }
        public GenerationOptions MinibossesHPType { get; set; }
        public bool isMinibossHPPercentageModified { get; set; }
        public GenerationOptions PedestalsGenerationType { get; set; }
        public bool isAddingRandomPedestal;
        public bool isBanningParasol;
        public GenerationOptions SprayGeneration { get; set; }
        public GenerationOptions SprayOutlineGenerationType { get; set; }
        public GenerationOptions StoneDoorGenerationType { get; set; }
        public bool isRemovingStoneBlocks = false;

        public Settings() {
            Random seedGenerator = new Random();
            Seed = seedGenerator.Next(MinSeed, MaxSeed);
            RandomEntity = new Random(Seed);
            Instance = this;
        }

        public static Settings GetInstance() {
            return Instance;
        }

        public void GetNewSeed() {
            Random seedGenerator = new Random();
            Seed = seedGenerator.Next(MinSeed, MaxSeed);
            RandomEntity = new Random(Seed);
        }
    }

    public static class Utils {
        public static void ShowObjectData(object inputObject) {
            //Console.Clear();

            foreach (PropertyDescriptor descriptor in TypeDescriptor.GetProperties(inputObject)) {
                string name = descriptor.Name;
                object value = descriptor.GetValue(inputObject);
                Console.WriteLine("{0}={1}", name, value);
            }
        }

        public static void WriteToROM(byte[] romData, long address, byte[] valueBytes) {
            int bytesLength = valueBytes.Length;

            if (address < 0 || address + bytesLength > romData.Length) {
                throw new ArgumentOutOfRangeException(nameof(address), "Address is out of bounds of the file contents.");
            }

            for (int i = 0; i < bytesLength; i++) {
                romData[address + i] = valueBytes[i];
            }
        }

        public static void WriteObjectToROM(byte[] romFile, Entity entity) {
            int address = entity.Address;
            byte id = entity.ID;
            byte behavior = entity.Behavior;
            byte speed = entity.Speed;
            byte[] properties = entity.Properties;

            WriteToROM(romFile, address + 12, new byte[] { id });
            WriteToROM(romFile, address + 14, new byte[] { behavior });
            WriteToROM(romFile, address + 16, new byte[] { speed });
            WriteToROM(romFile, address + 17, properties);
        }

        public static void WritePropertiesToROM(byte[] romFile, Properties properties) {
            int address = properties.Address;

            WriteToROM(romFile, address, properties.DamageSprites);
            WriteToROM(romFile, address + 4, new byte[] { properties.HP });
            WriteToROM(romFile, address + 6, new byte[] { properties.CopyAbility });
            WriteToROM(romFile, address + 8, new byte[] { properties.Palette });
        }

        public static int GetNextRandom() {
            Settings settings = Settings.GetInstance();

            return settings.RandomEntity.Next();
        }

        public static int GetRandomNumber(int min, int max) {
            Settings settings = Settings.GetInstance();

            return settings.RandomEntity.Next(min, max);
        }

        public static int Dice() {
            Settings settings = Settings.GetInstance();

            return settings.RandomEntity.Next(1, 7);
        }

        public static int Dice(int min, int max) {
            Settings settings = Settings.GetInstance();

            return settings.RandomEntity.Next(min, max + 1);
        }

        public static string ConvertIntToHex(int input) {
            string hex = input.ToString("X2");

            return hex;
        }

        public static string ConvertLongToHex(long input) {
            string hex = input.ToString("X2");

            return hex;
        }

        public static byte[] LongToBytes(long input) {
            string hex = ConvertLongToHex(input);

            // Ensure the input string length is even
            if (hex.Length % 2 != 0)
                throw new ArgumentException("Hex string must have an even length.");

            byte[] bytes = new byte[hex.Length / 2];
            for (int i = 0; i < hex.Length; i += 2) {
                bytes[i / 2] = Convert.ToByte(hex.Substring(i, 2), 16);
            }
            return bytes;
        }

        public static string jsonRoute = "JSON\\",
                             propertiesJson = "Properties",
                             enemiesJson = "Enemies",
                             minibossesJson = "Minibosses",
                             bossesJson = "Bosses",
                             itemsJson = "Items",
                             mirrorsJson = "Mirrors",
                             abilityStandsJson = "AbilityStands",
                             worldMapObjectsJson = "WorldMapObjects";

        public static void SaveJSON(dynamic list, string filename) {
            string json = JsonConvert.SerializeObject(ObjectsToJSON(list), Formatting.Indented);
            File.WriteAllText(jsonRoute + filename + ".json", json);
        }

        public static dynamic ObjectsToJSON(List<Entity> list) {
            var groupedEntities = new Dictionary<string, List<Dictionary<string, dynamic>>>();

            foreach (Entity entity in list) {
                EntitySerializable serialized = entity.SerializeEntity();
                if (!groupedEntities.ContainsKey(serialized.Name)) {
                    groupedEntities[serialized.Name] = new List<Dictionary<string, dynamic>>();
                }

                var entityDict = new Dictionary<string, dynamic> {
                    ["Address"] = serialized.Address,
                    ["Number"] = serialized.Number,
                    ["Link"] = serialized.Link,
                    ["X"] = serialized.X,
                    ["Y"] = serialized.Y,
                    ["ID"] = serialized.ID,
                    ["Behavior"] = serialized.Behavior,
                    ["Speed"] = serialized.Speed,
                    ["Properties"] = serialized.Properties,
                    ["Room"] = serialized.Room
                };

                groupedEntities[serialized.Name].Add(entityDict);
            }

            return groupedEntities;
        }

        public static dynamic ObjectsToJSON(List<Properties> list) {
            var groupedProperties = new Dictionary<string, List<Dictionary<string, dynamic>>>();

            foreach (Properties property in list) {
                PropertiesSerializable serialized = property.SerializeEntity();

                if (!groupedProperties.ContainsKey(serialized.Name)) {
                    groupedProperties[serialized.Name] = new List<Dictionary<string, dynamic>>();
                }

                var entityDict = new Dictionary<string, dynamic> {
                    ["Definition"] = serialized.Definition,
                    ["ID"] = serialized.ID,
                    ["Address"] = serialized.Address,
                    ["Name"] = serialized.Name,
                    ["Damage Sprites"] = serialized.DamageSprites,
                    ["HP"] = serialized.HP,
                    ["Copy Ability"] = serialized.CopyAbility,
                    ["Palette"] = serialized.Palette
                };

                groupedProperties[serialized.Name].Add(entityDict);
            }

            return groupedProperties;
        }

        public static dynamic JSONToObjects(string filename) {
            string json = File.ReadAllText(jsonRoute + filename + ".json");

            var result = JsonConvert.DeserializeObject<Dictionary<string, List<Dictionary<string, dynamic>>>>(json);

            return result;
        }

        public static byte[] StringToByteArray(string hexString) {
            // Initialize the byte array
            byte[] byteArray = new byte[hexString.Length / 2];

            // Convert each pair of characters to a byte
            for (int i = 0; i < hexString.Length; i += 2) {
                string byteString = hexString.Substring(i, 2);
                byteArray[i / 2] = Convert.ToByte(byteString, 16);
            }

            return byteArray;
        }

        public static List<T> Shuffle<T>(List<T> list) { 
            return list.OrderBy(x => Utils.GetNextRandom()).ToList();
        }

        public static void ChangeStatusBarText(string text) {
            KatAMRandomizerMain.Instance.StatusStripRandomizer.Text = text;
        }

        public static void DeserializeEntitiesJSON(dynamic itemsJson, List<Entity> entities, IKatAMRandomizer component) {
            // Reading the JSON dictionary;
            foreach (string key in itemsJson.Keys) {
                List<Dictionary<string, dynamic>> dict = itemsJson[key];

                // Injecting the extracted information on Entities;
                foreach (var item in dict) {
                    EntitySerializable serialized = new EntitySerializable();

                    serialized.Name = key;

                    // Reading the KeyValuePair information;
                    foreach (var kvp in item) {
                        switch (kvp.Key) {
                            case "Address":
                            int address = (int)kvp.Value;
                            serialized.Address = address;
                            break;
                            case "Number": serialized.Number = kvp.Value; break;
                            case "Link": serialized.Link = kvp.Value; break;
                            case "X": serialized.X = kvp.Value; break;
                            case "Y": serialized.Y = kvp.Value; break;
                            case "ID":
                            byte ID = (byte)kvp.Value;
                            serialized.ID = ID;
                            break;
                            case "Behavior": serialized.Behavior = (byte)kvp.Value; break;
                            case "Speed": serialized.Speed = (byte)kvp.Value; break;
                            case "Properties": serialized.Properties = kvp.Value; break;
                            case "Room":
                            serialized.Room = (int)kvp.Value;
                            break;
                        }
                    }

                    Entity entity = serialized.DeserializeEntity();

                    bool shouldAddEntity = component.FilterEntities(entity);

                    if(shouldAddEntity) entities.Add(entity);
                }
            }
        }

        // IsVetoedRoom(); Check if the room is not feasible for randomization;
        public static bool IsVetoedRoom(Entity entity) {
            int room = entity.Room;

            // Banned rooms like debug, boss endurance, or final boss rooms;
            HashSet<int> vetoedRooms = new HashSet<int>{
                0x0, 0x38D, 0x38E, 0x38F, 0x390, 0x391, 0x392, 0x393,
                0x394, 0x396, 0x397, 0x3B6, 0x3B7, 0x3BB, 0x3BC,
                0x3BD, 0x3C9, 0x3CA
            };

            return vetoedRooms.Contains(room);
        }

        public static byte Nothing = 0x2C;

        public static string ByteArrayToHexString(byte[] byteArray) {
            string result = "";

            foreach (byte bit in byteArray) {
                result += bit.ToString("X2");
            }

            return result;
        }
    }
}
