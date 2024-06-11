using KatAMInternal;
using System.ComponentModel;
using System.Text;
using System.Windows.Forms;
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

            if (fileDialog.ShowDialog() == DialogResult.OK) {
                try {
                    string filePath = fileDialog.FileName;
                    system.ROMPath = filePath;
                    system.ROMDirectory = Path.GetDirectoryName(system.ROMPath);

                    string gameName = Path.GetFileNameWithoutExtension(system.ROMPath);
                    system.ROMFilename = gameName;
                    LabelROMName.Text = system.ROMFilename;

                    byte[] fileContents = File.ReadAllBytes(system.ROMPath);
                    system.ROMData = new byte[fileContents.Length];
                    Array.Copy(fileContents, system.ROMData, fileContents.Length);

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

                    switch (gameId) {
                        // Japanese Version;
                        case "AGB-B8KJ":
                        region = "(JAP)";
                        break;

                        case "AGB-B8KP":
                        region = "(EUR)";
                        break;

                        case "AGB-B8KE":
                        region = "(USA)";
                        break;
                    }

                    LabelGameRegion.Text = $"Region: {region}";

                    ButtonSaveFile.Enabled = true;
                    ButtonInputSeed.Enabled = true;
                    ButtonRefreshSeed.Enabled = true;
                    LabelSeed.Visible = true;
                    UpdateLabelSeedText();
                } catch (Exception ex) {
                    MessageBox.Show($"Error loading file: {ex.Message}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }

        }

        // Control Panel;

        // Randomize (Save);
        private void ButtonSaveFile_Click(object sender, EventArgs e) {
            string newFileName = "KatAM.gba",
            destinationPath = Path.Combine(system.ROMDirectory, newFileName);
            settings.RandomEntity = new Random(settings.Seed);

            //KatAMROMReader.ReadROMData(system);
            new KatAMSprays(system);
            new KatAMItems(system);
            new KatAMPedestals(system);
            new KatAMEnemies(system);


            File.WriteAllBytes(destinationPath, system.ROMData);
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
            settings.amountOfMirrorShardsToAdd = Utils.GetRandomNumber(1, 5);
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

            if (!isAddingMoreHPUPs) {
                Label1HPUP.Enabled = false;
                Label2HPUP.Enabled = false;
                Label3HPUP.Enabled = false;
                TrackBarHPUP.Enabled = false;
                TrackBarHPUP.Value = 1;
                settings.HPUpsAdded = 0;
            } else {
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
    }
}
