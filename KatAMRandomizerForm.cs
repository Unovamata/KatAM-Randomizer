using KatAMInternal;
using System.ComponentModel;
using System.Text;
using System.Windows.Forms;
using KatAMRandomizer;
using KatAMRandomizer;
using KatAM_Randomizer;
using System.Reflection;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

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

                    // Every version of the game is supported; Each one keeps its data at different addresses;
                    RomRegion romRegion = RomRegion.Detect(fileContents);
                    string region = romRegion?.Name ?? "";

                    if (romRegion != null) {
                        RomRegion.Use(romRegion);
                        LabelCompatibility.ForeColor = Color.Green;
                        LabelCompatibility.Text = "Compatibility: Compatible";
                        LabelStability.ForeColor = Color.Green;
                        LabelStability.Text = "Stability: Perfectly Playable";
                    } else {
                        LabelCompatibility.ForeColor = Color.Red;
                        LabelCompatibility.Text = "Compatibility: Incompatible";
                        LabelStability.ForeColor = Color.Red;
                        LabelStability.Text = "Stability: Unsupported";
                    }

                    LabelGameRegion.Text = $"Region: {region}";

                    ButtonSaveFile.Enabled = romRegion != null;
                    ButtonInputSeed.Enabled = true;
                    ButtonRefreshSeed.Enabled = true;
                    LabelSeed.Visible = true;
                    LabelCompatibility.Visible = true;
                    LabelStability.Visible = true;
                    UpdateLabelSeedText();
                } catch (Exception ex) {
                    MessageBox.Show($"Error loading file: {ex.Message}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }

        }

        // Control Panel;

        // AddNightmareEnemies(); The ROM Expansion, which also expands the ROM to 32 MB (so ROMData becomes the bigger copy);
        private string AddNightmareEnemies() {
            byte[] rom = Processing.ROMData;
            string error = KatAMExpansion.AddEnemies(ref rom, nightmareROM);

            Processing.ROMData = rom;
            return error;
        }

        // Randomize (Save);
        private void ButtonSaveFile_Click(object sender, EventArgs e) {
            byte[] fileContents = File.ReadAllBytes(system.ROMPath),
                   originalROM = (byte[])fileContents.Clone();
            Processing.ROMData = new byte[fileContents.Length];
            Array.Copy(fileContents, Processing.ROMData, fileContents.Length);
            RomRegion.Use(RomRegion.Detect(fileContents));

            string newFileName = "KatAM.gba",
            destinationPath = Path.Combine(system.ROMDirectory, newFileName);
            settings.RandomEntity = new Random(settings.Seed);

            // ROM Expansion goes first, so the ROM reader sees the new enemies' parameters like any other's;
            if (settings.NightmareEnemies.Count > 0) {
                string error = nightmareROM == null ? "Load a Nightmare in Dream Land ROM in the ROM Expansion tab, or turn its enemies off."
                                                    : AddNightmareEnemies();

                if (error != null) {
                    MessageBox.Show(error, "ROM Expansion", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }

                Processing.parameters[KatAMExpansion.NightmareID] = "Nightmare in Dream Land Enemy";
            } else {
                Processing.parameters[KatAMExpansion.NightmareID] = Processing.enemiesDictionary[KatAMExpansion.NightmareID].Name;
            }

            new KatAMROMReader(system);
            new KatAMEnemies(system);
            new KatAMMirrors(system);
            new KatAMSprays(system);
            new KatAMPedestals(system);
            new KatAMMinibosses(system);
            new KatAMMapElements(system);
            new KatAMItems(system);
            new KatAMPropertiesManagement(system);

            // Randomize mirrors;
            // Veto rooms without at least one exit for the first mirror;
            // Randomize HP values on Enemies;
            // Randomize HP values on Bosses;
            // Fix the way behaviors and speeds are loaded when including minibosses in enemy generation;
            // Consider Mirras for mirror randomization;

            File.WriteAllBytes(destinationPath, Processing.ROMData);

            // The spoiler log is written next to the randomized ROM;
            if (settings.SpoilerLog) {
                string spoilerLogPath = Path.Combine(system.ROMDirectory, Path.GetFileNameWithoutExtension(newFileName) + " - Spoiler Log.txt");
                new KatAMSpoilerLog(system, originalROM).Save(spoilerLogPath);
            }
        }

        //////////////////////////////////////////////////////////////////////////////////////////////

        // General Options;
        private void CheckboxOptionsSpoilerLog_CheckedChanged(object sender, EventArgs e) {
            settings.SpoilerLog = CheckboxOptionsSpoilerLog.Checked;
        }

        /* Settings files; The state of every option (radio buttons, checkboxes, track bars and numeric boxes, by name) and the seed.
         * Loading sets the options back as if they were clicked, so their handlers update the settings like they always do; */
        const int SettingsFileVersion = 1;

        private void ButtonSaveSettings_Click(object sender, EventArgs e) {
            using SaveFileDialog fileDialog = new SaveFileDialog {
                Filter = "KatAM Randomizer Settings (*.json)|*.json",
                FileName = "KatAM Randomizer Settings.json",
                RestoreDirectory = true,
            };

            if (fileDialog.ShowDialog() != DialogResult.OK) return;

            JObject options = new JObject();

            foreach (Control control in GetOptionControls(this)) {
                switch (control) {
                    case RadioButton radio: options[radio.Name] = radio.Checked; break;
                    case CheckBox checkbox: options[checkbox.Name] = checkbox.Checked; break;
                    case TrackBar trackBar: options[trackBar.Name] = trackBar.Value; break;
                    case NumericUpDown numeric: options[numeric.Name] = numeric.Value; break;
                }
            }

            JObject file = new JObject {
                ["Version"] = SettingsFileVersion,
                ["Seed"] = settings.Seed,
                ["Options"] = options,
            };

            try {
                File.WriteAllText(fileDialog.FileName, file.ToString(Formatting.Indented));
            } catch (Exception ex) {
                MessageBox.Show($"Error saving settings: {ex.Message}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void ButtonLoadSettings_Click(object sender, EventArgs e) {
            using OpenFileDialog fileDialog = new OpenFileDialog {
                Filter = "KatAM Randomizer Settings (*.json)|*.json",
                RestoreDirectory = true,
            };

            if (fileDialog.ShowDialog() != DialogResult.OK) return;

            JObject file, options;

            try {
                file = JObject.Parse(File.ReadAllText(fileDialog.FileName));
                options = file["Options"] as JObject ?? throw new InvalidDataException("The file has no options.");
            } catch (Exception ex) {
                MessageBox.Show($"Error loading settings: {ex.Message}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            try {
                ApplyOptions(options);
            } catch (Exception ex) when (ex is InvalidCastException || ex is FormatException || ex is ArgumentException || ex is OverflowException) {
                MessageBox.Show($"Some settings couldn't be loaded: {ex.Message}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }

            if (file["Seed"]?.Type == JTokenType.Integer) {
                settings.Seed = (int)Math.Clamp((long)file["Seed"], settings.MinSeed, settings.MaxSeed);
                settings.RandomEntity = new Random(settings.Seed);
                UpdateLabelSeedText();
            }
        }

        // ApplyOptions(); Sets every option of a settings file;
        void ApplyOptions(JObject options) {
            List<Control> controls = GetOptionControls(this).Where(control => options.ContainsKey(control.Name)).ToList();

            /* Two passes: an option's handler may reset the options that depend on it (e.g. chests set to "No" reset the HP Ups),
             * and the second pass restores them. Each handler is run even if the option didn't change, so the settings always match; */
            for (int pass = 0; pass < 2; pass++) {
                // Radio buttons first, since they enable or reset the other options;
                foreach (RadioButton radio in controls.OfType<RadioButton>().Where(radio => (bool)options[radio.Name])) {
                    if (radio.Checked) RaiseControlEvent(radio, "OnCheckedChanged");
                    else radio.Checked = true;
                }

                foreach (CheckBox checkbox in controls.OfType<CheckBox>()) {
                    bool isChecked = (bool)options[checkbox.Name];

                    if (checkbox.Checked == isChecked) RaiseControlEvent(checkbox, "OnCheckedChanged");
                    else checkbox.Checked = isChecked;
                }

                foreach (TrackBar trackBar in controls.OfType<TrackBar>()) {
                    trackBar.Value = Math.Clamp((int)options[trackBar.Name], trackBar.Minimum, trackBar.Maximum);
                    RaiseControlEvent(trackBar, "OnScroll");
                }

                foreach (NumericUpDown numeric in controls.OfType<NumericUpDown>()) {
                    numeric.Value = Math.Clamp((decimal)options[numeric.Name], numeric.Minimum, numeric.Maximum);
                }
            }
        }

        // GetOptionControls(); Every option control of the form, in the order they're laid out;
        static IEnumerable<Control> GetOptionControls(Control parent) {
            foreach (Control control in parent.Controls.Cast<Control>().OrderBy(control => control.TabIndex)) {
                if (control is RadioButton || control is CheckBox || control is TrackBar || control is NumericUpDown) yield return control;

                foreach (Control child in GetOptionControls(control)) yield return child;
            }
        }

        // RaiseControlEvent(); Runs a control's event (e.g. OnCheckedChanged) as if the user had changed it;
        static void RaiseControlEvent(Control control, string method) {
            control.GetType().GetMethod(method, BindingFlags.Instance | BindingFlags.NonPublic, null, new[] { typeof(EventArgs) }, null)
                   ?.Invoke(control, new object[] { EventArgs.Empty });
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

        // ROM Expansion;
        byte[] nightmareROM;

        private void ButtonExpansionLoadNightmare_Click(object sender, EventArgs e) {
            OpenFileDialog fileDialog = new OpenFileDialog {
                Filter = "GBA ROM Files (*.gba)|*.gba",
                RestoreDirectory = true
            };

            if (fileDialog.ShowDialog() != DialogResult.OK) return;

            try {
                byte[] rom = File.ReadAllBytes(fileDialog.FileName);

                if (!KatAMExpansion.IsNightmareInDreamLand(rom)) {
                    MessageBox.Show("That isn't a Kirby: Nightmare in Dream Land ROM (USA, Japanese or European).", "ROM Expansion", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }

                nightmareROM = rom;
                LabelExpansionNightmare.Text = Path.GetFileName(fileDialog.FileName);
                CheckedListExpansionEnemies.Enabled = true;

                if (CheckedListExpansionEnemies.Items.Count == 0) {
                    foreach ((int index, string name) in KatAMExpansion.Enemies) CheckedListExpansionEnemies.Items.Add(name);
                }
            } catch (Exception ex) {
                MessageBox.Show($"Error loading file: {ex.Message}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        // The enemies picked; ItemCheck comes before the item's state changes;
        private void CheckedListExpansionEnemies_ItemCheck(object sender, ItemCheckEventArgs e) {
            int index = KatAMExpansion.Enemies[e.Index].index;

            settings.NightmareEnemies.Remove(index);
            if (e.NewValue == CheckState.Checked) settings.NightmareEnemies.Add(index);
        }

        // Mirrors;
        private void RadioMirrorsUnchanged_CheckedChanged(object sender, EventArgs e) {
            GroupMirrorsGoalGameMirrors.Enabled = false;
            RadioGoalGameMirrorsUnchanged.Checked = true;
            Checkbox8WayCannonStart.Enabled = false;
            Checkbox8WayCannonStart.Checked = false;
            settings.MirrorRandomization = GenerationOptions.Unchanged;
            settings.EightWayCarrotCastleStart = false;
            settings.GoalMirrorRandomization = GenerationOptions.Unchanged;
            settings.GoalMirrorWarpTypeRandomization = GenerationOptions.Unchanged;
        }

        private void RadioMirrorsShuffle_CheckedChanged(object sender, EventArgs e) {
            GroupMirrorsGoalGameMirrors.Enabled = true;
            Checkbox8WayCannonStart.Enabled = true;
            settings.MirrorRandomization = GenerationOptions.Shuffle;
        }

        private void RadioMirrorsRandom_CheckedChanged(object sender, EventArgs e) {
            GroupMirrorsGoalGameMirrors.Enabled = true;
            Checkbox8WayCannonStart.Enabled = true;
            settings.MirrorRandomization = GenerationOptions.Random;
            
        }

        private void Checkbox8WayCannonStart_CheckedChanged(object sender, EventArgs e) {
            settings.EightWayCarrotCastleStart = Checkbox8WayCannonStart.Checked;
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
