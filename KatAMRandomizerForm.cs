using KatAMInternal;
using System.ComponentModel;
using System.Text;
using System.Windows.Forms;
using KatAMRandomizer;
using KatAMRandomizer;
using KatAM_Randomizer;

namespace KatAMRandomizer
{
    public partial class KatAMRandomizerMain : Form {
        Processing system = new Processing();
        Settings settings;
        public static KatAMRandomizerMain Instance;

        public KatAMRandomizerMain() {
            settings = system.Settings;

            InitializeComponent();
            Instance = this;

            this.StartPosition = FormStartPosition.CenterScreen;
        }

        public void UpdateLabelSeedText() {
            LabelSeed.Text = $"Seed: {settings.Seed}";
        }

        private void ButtonConsoleSend_Click(object sender, EventArgs e) {
            Console.Clear();
            Utils.ShowObjectData(system);
            Console.WriteLine();
        }

        private void ButtonLoadFile_Click(object sender, EventArgs e) {
            OpenFileDialog fileDialog = new OpenFileDialog();

            fileDialog.Filter = "GBA ROM Files (*.gba)|*.gba";
            fileDialog.FilterIndex = 1;
            fileDialog.RestoreDirectory = true;

            if(fileDialog.ShowDialog() == DialogResult.OK) {
                try {
                    string filePath = fileDialog.FileName;
                    system.ROMPath = filePath;
                    system.ROMDirectory = Path.GetDirectoryName(system.ROMPath);

                    string gameName = Path.GetFileNameWithoutExtension(system.ROMPath);
                    system.ROMFilename = gameName;
                    LabelROMName.Text = system.ROMFilename;

                    byte[] fileContents = File.ReadAllBytes(system.ROMPath);
                    Processing.ROMData = new byte[fileContents.Length];
                    Array.Copy(fileContents, Processing.ROMData, fileContents.Length);

                    // Read 12 bytes starting at offset 0xA0 (160)
                    byte[] nameHandleBytes = new byte[4],
                    internalNameBytes = new byte[8],
                    gameIdBytes = new byte[4];

                    // Reading ROM Headers;
                    Array.Copy(fileContents, 0xA0, nameHandleBytes, 0, nameHandleBytes.Length);
                    Array.Copy(fileContents, 0xA4, internalNameBytes, 0, internalNameBytes.Length);
                    Array.Copy(fileContents, 0xAC, gameIdBytes, 0, gameIdBytes.Length);

                    string nameHandle = Encoding.ASCII.GetString(nameHandleBytes),
                    internalName = nameHandle + Encoding.ASCII.GetString(internalNameBytes),
                    gameId = nameHandle.Replace(" ", "-") + Encoding.ASCII.GetString(gameIdBytes);

                    LabelInternalName.Text = $"Internal Name: {internalName}";
                    LabelGameID.Text = $"Game ID: {gameId}";

                    string region = "";

                    switch(gameId) {
                        // Japanese Version;
                        case "AGB-B8KJ":
                            region = "(JAP)";
                            LabelCompatibility.ForeColor = Color.Green;
                            LabelCompatibility.Text = "Compatibility: Compatible";
                            LabelStability.ForeColor = Color.Green;
                            LabelStability.Text = "Stability: Perfectly Playable";
                            break;

                        case "AGB-B8KP":
                            region = "(EUR)";
                            LabelCompatibility.ForeColor = Color.Red;
                            LabelCompatibility.Text = "Compatibility: Incompatible";
                            LabelStability.ForeColor = Color.Red;
                            LabelStability.Text = "Stability: Unsupported";
                            break;

                        case "AGB-B8KE":
                            region = "(USA)";
                            LabelCompatibility.ForeColor = Color.Red;
                            LabelCompatibility.Text = "Compatibility: Incompatible";
                            LabelStability.ForeColor = Color.Red;
                            LabelStability.Text = "Stability: Unsupported";
                            break;

                        default:
                            LabelCompatibility.ForeColor = Color.Red;
                            LabelCompatibility.Text = "Compatibility: Incompatible";
                            LabelStability.ForeColor = Color.Red;
                            LabelStability.Text = "Stability: Unsupported";
                            break;
                    }

                    LabelGameRegion.Text = $"Region: {region}";

                    ButtonSaveFile.Enabled = true;
                    ButtonInputSeed.Enabled = true;
                    ButtonRefreshSeed.Enabled = true;
                    LabelSeed.Visible = true;
                    LabelCompatibility.Visible = true;
                    LabelStability.Visible = true;
                    UpdateLabelSeedText();
                }
                catch(Exception ex) {
                    MessageBox.Show($"Error loading file: {ex.Message}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }

        }

        // Control Panel;

        // Randomize (Save);
        private void ButtonSaveFile_Click(object sender, EventArgs e) {
            byte[] fileContents = File.ReadAllBytes(system.ROMPath);
            Processing.ROMData = new byte[fileContents.Length];
            Array.Copy(fileContents, Processing.ROMData, fileContents.Length);

            string newFileName = "KatAM.gba",
            destinationPath = Path.Combine(system.ROMDirectory, newFileName);
            settings.RandomEntity = new Random(settings.Seed);

            new KatAMROMReader(system);
            new KatAMMirrors(system);
            new KatAMSprays(system);
            new KatAMPedestals(system);
            new KatAMEnemies(system);
            new KatAMMinibosses(system);
            new KatAMMapElements(system);
            new KatAMItems(system);
            new KatAMPropertiesManagement(system);

            // Randomize mirrors;
            // Veto rooms without at least one exit for the first mirror;
            // Randomize HP values on Enemies;
            // Randomize HP values on Bosses;
            // Fix the way behaviors and speeds are loaded when including minibosses in enemy generation;

            File.WriteAllBytes(destinationPath, Processing.ROMData);
        }

        // Refresh Seed;
        private void ButtonRefreshSeed_Click(object sender, EventArgs e) {
            settings.GetNewSeed();
            UpdateLabelSeedText();
        }

        // Input Premade Seed;
        private void ButtonInputSeed_Click(object sender, EventArgs e) {
            SeedInputForm form = new SeedInputForm();
            form.ShowDialog();
        }


        //////////////////////////////////////////////////////////////////////////////////////////////

        // Mirrors;
        // Warp Stars;
        private void RadioWarpsStarsUnchanged_CheckedChanged(object sender, EventArgs e) {
            settings.WarpStarsGenerationType = GenerationOptions.Unchanged;
        }

        private void RadioWarpsStarsShuffle_CheckedChanged(object sender, EventArgs e) {
            settings.WarpStarsGenerationType = GenerationOptions.Shuffle;
        }

        // Fuse Cannons;
        private void RadioWarpsFuseCannonsUnchanged_CheckedChanged(object sender, EventArgs e) {
            settings.WarpStarsGenerationType = GenerationOptions.Unchanged;
        }

        private void RadioWarpsFuseCannonsShuffle_CheckedChanged(object sender, EventArgs e) {
            settings.WarpStarsGenerationType = GenerationOptions.Shuffle;
        }



        //////////////////////////////////////////////////////////////////////////////////////////////

        // Chests & Items;
        // Consumables;
        private void RadioConsumablesUnchanged_CheckedChanged(object sender, EventArgs e) {
            settings.ConsumablesGenerationType = GenerationOptions.Unchanged;
            GroupConsumablesMirrorShards.Enabled = false;
            RadioMirrorShardsUnchanged.Checked = true;
        }

        private void RadioConsumablesShuffle_CheckedChanged(object sender, EventArgs e) {
            settings.ConsumablesGenerationType = GenerationOptions.Shuffle;
            GroupConsumablesMirrorShards.Enabled = true;
        }

        private void RadioConsumablesRandom_CheckedChanged(object sender, EventArgs e) {
            settings.ConsumablesGenerationType = GenerationOptions.Random;
            GroupConsumablesMirrorShards.Enabled = true;
        }

        private void RadioConsumablesNo_CheckedChanged(object sender, EventArgs e) {
            settings.ConsumablesGenerationType = GenerationOptions.No;
            GroupConsumablesMirrorShards.Enabled = false;
            RadioMirrorShardsUnchanged.Checked = true;
        }

        private void RadioConsumablesChallenge_CheckedChanged(object sender, EventArgs e) {
            settings.ConsumablesGenerationType = GenerationOptions.Challenge;
            GroupConsumablesMirrorShards.Enabled = true;
        }

        private void RadioConsumablesCustom_CheckedChanged(object sender, EventArgs e) {

        }

        // Mirror Shards;
        private void RadioMirrorShardsUnchanged_CheckedChanged(object sender, EventArgs e) {
            GroupMirrorShardsAmount.Enabled = false;
            settings.MirrorShardsGenerationType = GenerationOptions.Unchanged;
            settings.amountOfMirrorShardsToAdd = 0;
        }

        private void RadioMirrorShardsRandom_CheckedChanged(object sender, EventArgs e) {
            GroupMirrorShardsAmount.Enabled = false;
            settings.MirrorShardsGenerationType = GenerationOptions.Random;
            settings.amountOfMirrorShardsToAdd = Utils.GetRandomRange(1, 5);
        }

        private void RadioMirrorShardsCustom_CheckedChanged(object sender, EventArgs e) {
            GroupMirrorShardsAmount.Enabled = true;
            settings.MirrorShardsGenerationType = GenerationOptions.Custom;
        }

        private void TrackBarMirrorShardsAmount_Scroll(object sender, EventArgs e) {
            settings.amountOfMirrorShardsToAdd = TrackBarMirrorShardsAmount.Value;
        }

        // Chests;
        private void RadioChestsUnchanged_CheckedChanged(object sender, EventArgs e) {
            settings.ChestsGenerationType = GenerationOptions.Unchanged;
            GroupChestsProperties.Enabled = GroupChestsProperties.Enabled = false;
            RadioChestsPropertiesUnchanged.Checked = true;
        }

        private void RadioChestsShuffle_CheckedChanged(object sender, EventArgs e) {
            settings.ChestsGenerationType = GenerationOptions.Shuffle;
            GroupChestsProperties.Enabled = true;
        }

        private void RadioChestsNo_CheckedChanged(object sender, EventArgs e) {
            settings.ChestsGenerationType = GenerationOptions.No;
            GroupChestsProperties.Enabled = false;
            RadioChestsPropertiesUnchanged.Checked = true;
            CheckboxChestsMoreHPUP.Checked = false;
            TrackBarHPUP.Value = 1;
            settings.HPUpsAdded = 0;
        }

        // Properties;
        private void RadioChestsPropertiesUnchanged_CheckedChanged(object sender, EventArgs e) {
            settings.ChestsPropertiesType = GenerationOptions.Unchanged;
        }

        private void RadioChestsPropertiesRemoveHealing_CheckedChanged(object sender, EventArgs e) {
            settings.ChestsPropertiesType = GenerationOptions.Remove;
        }

        private void RadioChestsPropertiesRandomWithoutConsumables_CheckedChanged(object sender, EventArgs e) {
            settings.ChestsPropertiesType = GenerationOptions.RandomAndPresets;
        }

        private void RadioChestsPropertiesRandom_CheckedChanged(object sender, EventArgs e) {
            settings.ChestsPropertiesType = GenerationOptions.Random;
        }

        // HP Ups;
        private void CheckboxChestsMoreHPUP_CheckedChanged(object sender, EventArgs e) {
            bool isAddingMoreHPUPs = CheckboxChestsMoreHPUP.Checked;

            if(!isAddingMoreHPUPs) {
                Label1HPUP.Enabled = false;
                Label2HPUP.Enabled = false;
                Label3HPUP.Enabled = false;
                TrackBarHPUP.Enabled = false;
                TrackBarHPUP.Value = 1;
                settings.HPUpsAdded = 0;
            }
            else {
                Label1HPUP.Enabled = true;
                Label2HPUP.Enabled = true;
                Label3HPUP.Enabled = true;
                TrackBarHPUP.Enabled = true;
                settings.HPUpsAdded = TrackBarHPUP.Value;
                settings.isAddingMoreHPUps = isAddingMoreHPUPs;
            }
        }

        private void TrackBarHPUP_Scroll(object sender, EventArgs e) {
            settings.HPUpsAdded = TrackBarHPUP.Value;
        }


        //////////////////////////////////////////////////////////////////////////////////////////////


        // Enemies;
        private void RadioEnemiesUnchanged_CheckedChanged(object sender, EventArgs e) {
            settings.EnemiesGenerationType = GenerationOptions.Unchanged;
            HideCheckboxEnemiesRandomizeExcluded();
            HideEnemiesOptions();
        }

        private void RadioEnemiesShuffle_CheckedChanged(object sender, EventArgs e) {
            settings.EnemiesGenerationType = GenerationOptions.Shuffle;
            CheckboxEnemiesRandomizeExcluded.Visible = true;
            ShowEnemiesOptions();
        }

        private void RadioEnemiesRandom_CheckedChanged(object sender, EventArgs e) {
            settings.EnemiesGenerationType = GenerationOptions.Random;
            ShowEnemiesOptions();
            HideCheckboxEnemiesRandomizeExcluded();
        }

        private void RadioEnemiesNo_CheckedChanged(object sender, EventArgs e) {
            settings.EnemiesGenerationType = GenerationOptions.No;
            HideEnemiesOptions();
        }

        private void HideCheckboxEnemiesRandomizeExcluded() {
            CheckboxEnemiesRandomizeExcluded.Visible = false;
            CheckboxEnemiesRandomizeExcluded.Checked = false;
            settings.isRandomizingExcludedEnemies = false;
        }

        private void HideEnemiesOptions() {
            HideCheckboxEnemiesRandomizeExcluded();


            CheckboxEnemiesRandomizeMinibosses.Visible = false;
            settings.isIncludingMiniBosses = false;

            CheckboxEnemiesRandomizeUnderwaterIntelligent.Visible = false;
            CheckboxEnemiesRandomizeFlyingIntelligent.Visible = false;
            settings.isRandomizingFlyingEnemiesIntelligently = false;
            settings.isRandomizingUnderwaterEnemiesIntelligently = false;
        }

        private void ShowEnemiesOptions() {
            CheckboxEnemiesRandomizeMinibosses.Visible = true;
            settings.isIncludingMiniBosses = false;

            CheckboxEnemiesRandomizeUnderwaterIntelligent.Visible = true;
            CheckboxEnemiesRandomizeFlyingIntelligent.Visible = true;
            settings.isRandomizingFlyingEnemiesIntelligently = false;
            settings.isRandomizingUnderwaterEnemiesIntelligently = false;
        }

        private void CheckboxEnemiesRandomizeExcluded_CheckedChanged(object sender, EventArgs e) {
            settings.isRandomizingExcludedEnemies = CheckboxEnemiesRandomizeExcluded.Checked;
        }

        private void CheckboxEnemiesRandomizeMinibosses_CheckedChanged(object sender, EventArgs e) {
            settings.isIncludingMiniBosses = CheckboxEnemiesRandomizeMinibosses.Checked;
        }

        private void CheckboxEnemiesRandomizeFlyingIntelligent_CheckedChanged(object sender, EventArgs e) {
            settings.isRandomizingFlyingEnemiesIntelligently = CheckboxEnemiesRandomizeFlyingIntelligent.Checked;
        }

        private void CheckboxEnemiesRandomizeUnderwaterIntelligent_CheckedChanged(object sender, EventArgs e) {
            settings.isRandomizingUnderwaterEnemiesIntelligently = CheckboxEnemiesRandomizeUnderwaterIntelligent.Checked;
        }

        // Speed;
        private void RadioEnemyPropertiesSpeedUnchanged_CheckedChanged(object sender, EventArgs e) {
            settings.EnemiesPropertiesSpeedType = GenerationOptions.Unchanged;
        }

        private void RadioEnemyPropertiesSpeedRandom_CheckedChanged(object sender, EventArgs e) {
            settings.EnemiesPropertiesSpeedType = GenerationOptions.Random;
        }

        // Behavior;
        private void RadioEnemyPropertiesBehaviorUnchanged_CheckedChanged(object sender, EventArgs e) {
            settings.EnemiesPropertiesBehaviorType = GenerationOptions.Unchanged;
        }

        private void RadioEnemyPropertiesBehaviorRandom_CheckedChanged(object sender, EventArgs e) {
            settings.EnemiesPropertiesBehaviorType = GenerationOptions.Random;
        }

        private void CheckboxEnemyPropertiesUnusedBehaviors_CheckedChanged(object sender, EventArgs e) {
            settings.isUsingUnusedBehaviors = CheckboxEnemyPropertiesUnusedBehaviors.Checked;
        }

        // Abilities;
        private void RadioEnemyPropertiesAbilityUnchanged_CheckedChanged(object sender, EventArgs e) {
            settings.EnemiesInhaleAbilityType = GenerationOptions.Unchanged;
        }

        private void RadioEnemyPropertiesAbilityShuffle_CheckedChanged(object sender, EventArgs e) {
            settings.EnemiesInhaleAbilityType = GenerationOptions.Shuffle;
        }

        private void RadioEnemyPropertiesAbilityRandom_CheckedChanged(object sender, EventArgs e) {
            settings.EnemiesInhaleAbilityType = GenerationOptions.Random;
        }

        private void CheckboxEnemyPropertiesAbilityNormal_CheckedChanged(object sender, EventArgs e) {
            settings.isEnemyIncludingNormalInhaleAbility = CheckboxEnemyPropertiesAbilityNormal.Checked;
        }

        private void CheckboxEnemyPropertiesAbilityMaster_CheckedChanged(object sender, EventArgs e) {
            settings.isEnemyIncludingMasterInhaleAbility = CheckboxEnemyPropertiesAbilityMaster.Checked;
        }

        private void CheckboxEnemyPropertiesAbilityMix_CheckedChanged(object sender, EventArgs e) {
            settings.isEnemyIncludingMixInhaleAbility = CheckboxEnemyPropertiesAbilityMix.Checked;
        }

        // HP;
        private void RadioEnemyPropertiesHPUnchanged_CheckedChanged(object sender, EventArgs e) {
            settings.EnemiesHPType = GenerationOptions.Unchanged;
        }

        private void RadioEnemyPropertiesHPShuffle_CheckedChanged(object sender, EventArgs e) {
            settings.EnemiesHPType = GenerationOptions.Shuffle;
        }

        private void RadioEnemyPropertiesHPRandom_CheckedChanged(object sender, EventArgs e) {
            settings.EnemiesHPType = GenerationOptions.Random;
        }

        private void CheckboxEnemyPropertiesHPPercentage_CheckedChanged(object sender, EventArgs e) {

        }

        private void NumericEnemyPropertiesHPMin_ValueChanged(object sender, EventArgs e) {

        }

        private void NumericEnemyPropertiesHPMax_ValueChanged(object sender, EventArgs e) {

        }


        //////////////////////////////////////////////////////////////////////////////////////////////


        // Mini-Bosses;

        private void RadioMinibossesUnchanged_CheckedChanged(object sender, EventArgs e) {
            settings.MinibossesGenerationType = GenerationOptions.Unchanged;
        }

        private void RadioMinibossesShuffle_CheckedChanged(object sender, EventArgs e) {
            settings.MinibossesGenerationType = GenerationOptions.Shuffle;
        }

        private void RadioMinibossesRandom_CheckedChanged(object sender, EventArgs e) {
            settings.MinibossesGenerationType = GenerationOptions.Random;
        }

        private void RadioMinibossesNo_CheckedChanged(object sender, EventArgs e) {
            settings.MinibossesGenerationType = GenerationOptions.No;
        }

        private void RadioMinibossesCustom_CheckedChanged(object sender, EventArgs e) {
            settings.MinibossesGenerationType = GenerationOptions.Custom;
        }

        // Speed;
        private void RadioMinibossPropertiesSpeedUnchanged_CheckedChanged(object sender, EventArgs e) {
            settings.MinibossesPropertiesSpeedType = GenerationOptions.Unchanged;
        }

        private void RadioMinibossPropertiesSpeedRandom_CheckedChanged(object sender, EventArgs e) {
            settings.MinibossesPropertiesSpeedType = GenerationOptions.Random;
        }

        // Behavior;
        private void RadioMinibossPropertiesBehaviorUnchanged_CheckedChanged(object sender, EventArgs e) {
            settings.MinibossesPropertiesBehaviorType = GenerationOptions.Unchanged;
        }

        private void RadioMinibossPropertiesBehaviorRandom_CheckedChanged(object sender, EventArgs e) {
            settings.MinibossesPropertiesBehaviorType = GenerationOptions.Random;
        }

        // Ability;
        private void RadioMinibossPropertiesAbilityUnchanged_CheckedChanged(object sender, EventArgs e) {
            settings.MinibossesInhaleAbilityType = GenerationOptions.Unchanged;
        }

        private void RadioMinibossPropertiesAbilityShuffle_CheckedChanged(object sender, EventArgs e) {
            settings.MinibossesInhaleAbilityType = GenerationOptions.Shuffle;
        }

        private void RadioMinibossPropertiesAbilityRandom_CheckedChanged(object sender, EventArgs e) {
            settings.MinibossesInhaleAbilityType = GenerationOptions.Random;
        }

        private void CheckboxMinibossPropertiesAbilityMaster_CheckedChanged(object sender, EventArgs e) {
            settings.MinibossesInhaleAbilityType = GenerationOptions.Shuffle;
        }

        // HP;
        private void RadioMinibossPropertiesHPUnchanged_CheckedChanged(object sender, EventArgs e) {
            settings.MinibossesHPType = GenerationOptions.Unchanged;
        }

        private void RadioMinibossPropertiesHPShuffle_CheckedChanged(object sender, EventArgs e) {
            settings.MinibossesHPType = GenerationOptions.Shuffle;
        }

        private void RadioMinibossPropertiesHPRandom_CheckedChanged(object sender, EventArgs e) {
            settings.MinibossesHPType = GenerationOptions.Random;
        }

        private void CheckboxMinibossPropertiesHPPercentage_CheckedChanged(object sender, EventArgs e) {
            settings.isEnemyHPPercentageModified = CheckboxMinibossPropertiesHPPercentage.Checked;
        }



        //////////////////////////////////////////////////////////////////////////////////////////////


        // Ability Pedestals;
        private void RadioAbilityPedestalsUnchanged_CheckedChanged(object sender, EventArgs e) {
            settings.PedestalsGenerationType = GenerationOptions.Unchanged;
        }

        private void RadioAbilityPedestalShuffle_CheckedChanged(object sender, EventArgs e) {
            settings.PedestalsGenerationType = GenerationOptions.Shuffle;
        }

        private void RadioAbilityPedestalsRandom_CheckedChanged(object sender, EventArgs e) {
            settings.PedestalsGenerationType = GenerationOptions.Random;
        }

        private void RadioAbilityPedestalsUnlockPath_CheckedChanged(object sender, EventArgs e) {
            settings.PedestalsGenerationType = GenerationOptions.Presets;
        }

        private void RadioAbilityPedestalsChallenge_CheckedChanged(object sender, EventArgs e) {
            settings.PedestalsGenerationType = GenerationOptions.Challenge;
        }

        private void RadioAbilityPedestalsNo_CheckedChanged(object sender, EventArgs e) {
            settings.PedestalsGenerationType = GenerationOptions.No;
        }

        private void RadioAbilityPedestalsCustom_CheckedChanged(object sender, EventArgs e) {
            settings.PedestalsGenerationType = GenerationOptions.Custom;
        }

        private void CheckboxIncludeRandomPedestal_CheckedChanged(object sender, EventArgs e) {
            settings.isAddingRandomPedestal = CheckboxIncludeRandomPedestal.Checked;
        }

        private void CheckboxBanParasol_CheckedChanged(object sender, EventArgs e) {
            settings.isBanningParasol = CheckboxBanParasol.Checked;
        }


        //////////////////////////////////////////////////////////////////////////////////////////////

        // Miscellaneous;
        // Spray Color Presets;
        private void RadioSprayUnchanged_CheckedChanged(object sender, EventArgs e) {
            settings.SprayGeneration = GenerationOptions.Unchanged;
        }

        private void RadioSprayPresets_CheckedChanged(object sender, EventArgs e) {
            settings.SprayGeneration = GenerationOptions.Presets;
        }

        private void RadioSprayRandomAndPresets_CheckedChanged(object sender, EventArgs e) {
            settings.SprayGeneration = GenerationOptions.RandomAndPresets;
        }

        private void RadioSprayRandom_CheckedChanged(object sender, EventArgs e) {
            settings.SprayGeneration = GenerationOptions.Random;
        }

        // Outline Colors;
        private void RadioOutlinesUnchanged_CheckedChanged(object sender, EventArgs e) {
            settings.SprayOutlineGenerationType = GenerationOptions.Unchanged;
        }

        private void RadioOutlinesAll_CheckedChanged(object sender, EventArgs e) {
            settings.SprayOutlineGenerationType = GenerationOptions.All;
        }

        private void RadioOutlinesRandom_CheckedChanged(object sender, EventArgs e) {
            settings.SprayOutlineGenerationType = GenerationOptions.Random;
        }

        // Map Elements;
        private void MapElementsStoneDoorsUnchanged_CheckedChanged(object sender, EventArgs e) {
            settings.StoneDoorGenerationType = GenerationOptions.Unchanged;
        }

        private void MapElementsStoneDoorsRemoveAll_CheckedChanged(object sender, EventArgs e) {
            settings.StoneDoorGenerationType = GenerationOptions.All;
        }

        private void MapElementsStoneDoorsRemoveTutorial_CheckedChanged(object sender, EventArgs e) {
            settings.StoneDoorGenerationType = GenerationOptions.Presets;
        }

        private void CheckboxMapElementsStoneBlocks_CheckedChanged(object sender, EventArgs e) {
            settings.isRemovingStoneBlocks = CheckboxMapElementsAllStoneBlocks.Checked;
        }

        // Mirrors;
        private void RadioMirrorsUnchanged_CheckedChanged(object sender, EventArgs e) {
            GroupMirrorsGoalGameMirrors.Enabled = false;
            RadioGoalGameMirrorsUnchanged.Checked = true;
            settings.MirrorRandomization = GenerationOptions.Unchanged;
            settings.GoalMirrorRandomization = GenerationOptions.Unchanged;
            settings.GoalMirrorWarpTypeRandomization = GenerationOptions.Unchanged;
        }

        private void RadioMirrorsShuffle_CheckedChanged(object sender, EventArgs e) {
            GroupMirrorsGoalGameMirrors.Enabled = true;
            settings.MirrorRandomization = GenerationOptions.Shuffle;
        }

        private void RadioMirrorsRandom_CheckedChanged(object sender, EventArgs e) {
            GroupMirrorsGoalGameMirrors.Enabled = true;
            settings.MirrorRandomization = GenerationOptions.Random;
        }

        private void RadioGoalGameMirrorsUnchanged_CheckedChanged(object sender, EventArgs e) {
            GroupGoalGameMirrorsWarpType.Enabled = false;
            settings.GoalMirrorRandomization = GenerationOptions.Unchanged;
            settings.GoalMirrorWarpTypeRandomization = GenerationOptions.Unchanged;
            RadioWarpTypeShuffle.Checked = true;
        }

        private void RadioGoalGameMirrorsShuffle_CheckedChanged(object sender, EventArgs e) {
            GroupGoalGameMirrorsWarpType.Enabled = true;
            settings.GoalMirrorRandomization = GenerationOptions.Shuffle;
        }

        private void RadioGoalGameMirrorsRandom_CheckedChanged(object sender, EventArgs e) {
            GroupGoalGameMirrorsWarpType.Enabled = true;
            settings.GoalMirrorRandomization = GenerationOptions.Random;
        }

        private void RadioWarpTypeShuffle_CheckedChanged(object sender, EventArgs e) {
            settings.GoalMirrorWarpTypeRandomization = GenerationOptions.Unchanged;
        }

        private void RadioWarpTypeCustom_CheckedChanged(object sender, EventArgs e) {
            settings.GoalMirrorWarpTypeRandomization = GenerationOptions.Custom;
        }
    }
}
