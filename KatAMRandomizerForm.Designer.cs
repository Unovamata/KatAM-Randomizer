namespace KatAM_Randomizer
{
    partial class KatAMRandomizerMain
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent() {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(KatAMRandomizerMain));
            ButtonConsoleSend = new Button();
            ButtonLoadFile = new Button();
            ButtonSaveFile = new Button();
            ButtonInputSeed = new Button();
            button5 = new Button();
            button6 = new Button();
            GroupSettings = new GroupBox();
            checkBox1 = new CheckBox();
            GroupRomInfo = new GroupBox();
            LabelGameRegion = new Label();
            LabelGameID = new Label();
            LabelInternalName = new Label();
            LabelROMName = new Label();
            tabControl1 = new TabControl();
            tabPage1 = new TabPage();
            tabPage2 = new TabPage();
            Chests = new GroupBox();
            RadioChestsNo = new RadioButton();
            GroupChestsProperties = new GroupBox();
            RadioChestsPropertiesRemoveHealing = new RadioButton();
            RadioChestsPropertiesRandom = new RadioButton();
            RadioChestsPropertiesUnchanged = new RadioButton();
            RadioChestsShuffle = new RadioButton();
            RadioChestsUnchanged = new RadioButton();
            groupBox2 = new GroupBox();
            RadioConsumablesChallenge = new RadioButton();
            RadioConsumablesNo = new RadioButton();
            button1 = new Button();
            RadioConsumablesCustom = new RadioButton();
            GroupConsumablesMirrorShards = new GroupBox();
            RadioMirrorShardsRandom = new RadioButton();
            GroupMirrorShardsAmount = new GroupBox();
            TrackBarMirrorShardsAmount = new TrackBar();
            label5 = new Label();
            label1 = new Label();
            label4 = new Label();
            label2 = new Label();
            label3 = new Label();
            RadioMirrorShardsCustom = new RadioButton();
            RadioMirrorShardsUnchanged = new RadioButton();
            RadioConsumablesRandom = new RadioButton();
            RadioConsumablesShuffle = new RadioButton();
            RadioConsumablesUnchanged = new RadioButton();
            tabPage3 = new TabPage();
            tabPage4 = new TabPage();
            tabPage5 = new TabPage();
            TabMiscellaneous = new TabPage();
            GroupMiscSprayPalettes = new GroupBox();
            GroupSprayOutlines = new GroupBox();
            RadioOutlinesRandom = new RadioButton();
            RadioOutlinesAll = new RadioButton();
            RadioOutlinesUnchanged = new RadioButton();
            RadioSprayPresets = new RadioButton();
            RadioSprayRandomAndPresets = new RadioButton();
            RadioSprayRandom = new RadioButton();
            RadioSprayUnchanged = new RadioButton();
            ButtonRefreshSeed = new Button();
            LabelSeed = new Label();
            GroupSettings.SuspendLayout();
            GroupRomInfo.SuspendLayout();
            tabControl1.SuspendLayout();
            tabPage2.SuspendLayout();
            Chests.SuspendLayout();
            GroupChestsProperties.SuspendLayout();
            groupBox2.SuspendLayout();
            GroupConsumablesMirrorShards.SuspendLayout();
            GroupMirrorShardsAmount.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)TrackBarMirrorShardsAmount).BeginInit();
            TabMiscellaneous.SuspendLayout();
            GroupMiscSprayPalettes.SuspendLayout();
            GroupSprayOutlines.SuspendLayout();
            SuspendLayout();
            // 
            // ButtonConsoleSend
            // 
            ButtonConsoleSend.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            ButtonConsoleSend.Location = new Point(13, 109);
            ButtonConsoleSend.Name = "ButtonConsoleSend";
            ButtonConsoleSend.Size = new Size(158, 23);
            ButtonConsoleSend.TabIndex = 0;
            ButtonConsoleSend.Text = "Console Send Message";
            ButtonConsoleSend.UseVisualStyleBackColor = true;
            ButtonConsoleSend.Click += ButtonConsoleSend_Click;
            // 
            // ButtonLoadFile
            // 
            ButtonLoadFile.Location = new Point(626, 12);
            ButtonLoadFile.Name = "ButtonLoadFile";
            ButtonLoadFile.Size = new Size(162, 23);
            ButtonLoadFile.TabIndex = 1;
            ButtonLoadFile.Text = "Open ROM";
            ButtonLoadFile.UseVisualStyleBackColor = true;
            ButtonLoadFile.Click += ButtonLoadFile_Click;
            // 
            // ButtonSaveFile
            // 
            ButtonSaveFile.Enabled = false;
            ButtonSaveFile.Location = new Point(626, 41);
            ButtonSaveFile.Name = "ButtonSaveFile";
            ButtonSaveFile.Size = new Size(162, 23);
            ButtonSaveFile.TabIndex = 2;
            ButtonSaveFile.Text = "Randomize (Save)";
            ButtonSaveFile.UseVisualStyleBackColor = true;
            ButtonSaveFile.Click += ButtonSaveFile_Click;
            // 
            // ButtonInputSeed
            // 
            ButtonInputSeed.Enabled = false;
            ButtonInputSeed.Location = new Point(626, 70);
            ButtonInputSeed.Name = "ButtonInputSeed";
            ButtonInputSeed.Size = new Size(162, 23);
            ButtonInputSeed.TabIndex = 3;
            ButtonInputSeed.Text = "Input Premade Seed";
            ButtonInputSeed.UseVisualStyleBackColor = true;
            ButtonInputSeed.Click += ButtonInputSeed_Click;
            // 
            // button5
            // 
            button5.Font = new Font("Segoe UI", 9F);
            button5.Location = new Point(13, 51);
            button5.Name = "button5";
            button5.Size = new Size(158, 23);
            button5.TabIndex = 6;
            button5.Text = "Load Settings";
            button5.UseVisualStyleBackColor = true;
            // 
            // button6
            // 
            button6.Font = new Font("Segoe UI", 9F);
            button6.Location = new Point(13, 80);
            button6.Name = "button6";
            button6.Size = new Size(158, 23);
            button6.TabIndex = 7;
            button6.Text = "Save Settings";
            button6.UseVisualStyleBackColor = true;
            // 
            // GroupSettings
            // 
            GroupSettings.Controls.Add(checkBox1);
            GroupSettings.Controls.Add(ButtonConsoleSend);
            GroupSettings.Controls.Add(button6);
            GroupSettings.Controls.Add(button5);
            GroupSettings.Font = new Font("Segoe UI", 9F, FontStyle.Bold, GraphicsUnit.Point, 0);
            GroupSettings.Location = new Point(12, 12);
            GroupSettings.Name = "GroupSettings";
            GroupSettings.Size = new Size(177, 143);
            GroupSettings.TabIndex = 8;
            GroupSettings.TabStop = false;
            GroupSettings.Text = "General Options";
            // 
            // checkBox1
            // 
            checkBox1.AutoSize = true;
            checkBox1.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            checkBox1.Location = new Point(13, 25);
            checkBox1.Name = "checkBox1";
            checkBox1.Size = new Size(85, 19);
            checkBox1.TabIndex = 12;
            checkBox1.Text = "Race Mode";
            checkBox1.UseVisualStyleBackColor = true;
            // 
            // GroupRomInfo
            // 
            GroupRomInfo.Controls.Add(LabelGameRegion);
            GroupRomInfo.Controls.Add(LabelGameID);
            GroupRomInfo.Controls.Add(LabelInternalName);
            GroupRomInfo.Controls.Add(LabelROMName);
            GroupRomInfo.Font = new Font("Segoe UI", 9F, FontStyle.Bold, GraphicsUnit.Point, 0);
            GroupRomInfo.Location = new Point(195, 12);
            GroupRomInfo.Name = "GroupRomInfo";
            GroupRomInfo.Size = new Size(271, 143);
            GroupRomInfo.TabIndex = 9;
            GroupRomInfo.TabStop = false;
            GroupRomInfo.Text = "ROM Information";
            // 
            // LabelGameRegion
            // 
            LabelGameRegion.AutoSize = true;
            LabelGameRegion.Font = new Font("Segoe UI", 9F);
            LabelGameRegion.Location = new Point(6, 113);
            LabelGameRegion.Name = "LabelGameRegion";
            LabelGameRegion.Size = new Size(47, 15);
            LabelGameRegion.TabIndex = 11;
            LabelGameRegion.Text = "Region:";
            // 
            // LabelGameID
            // 
            LabelGameID.AutoSize = true;
            LabelGameID.Font = new Font("Segoe UI", 9F);
            LabelGameID.Location = new Point(6, 84);
            LabelGameID.Name = "LabelGameID";
            LabelGameID.Size = new Size(55, 15);
            LabelGameID.TabIndex = 10;
            LabelGameID.Text = "Game ID:";
            // 
            // LabelInternalName
            // 
            LabelInternalName.AutoSize = true;
            LabelInternalName.Font = new Font("Segoe UI", 9F);
            LabelInternalName.Location = new Point(6, 55);
            LabelInternalName.Name = "LabelInternalName";
            LabelInternalName.Size = new Size(85, 15);
            LabelInternalName.TabIndex = 9;
            LabelInternalName.Text = "Internal Name:";
            // 
            // LabelROMName
            // 
            LabelROMName.AutoSize = true;
            LabelROMName.Font = new Font("Segoe UI", 9F);
            LabelROMName.Location = new Point(6, 26);
            LabelROMName.Name = "LabelROMName";
            LabelROMName.Size = new Size(95, 15);
            LabelROMName.TabIndex = 8;
            LabelROMName.Text = "No ROM Loaded";
            // 
            // tabControl1
            // 
            tabControl1.Controls.Add(tabPage1);
            tabControl1.Controls.Add(tabPage2);
            tabControl1.Controls.Add(tabPage3);
            tabControl1.Controls.Add(tabPage4);
            tabControl1.Controls.Add(tabPage5);
            tabControl1.Controls.Add(TabMiscellaneous);
            tabControl1.ItemSize = new Size(100, 20);
            tabControl1.Location = new Point(12, 161);
            tabControl1.Name = "tabControl1";
            tabControl1.SelectedIndex = 0;
            tabControl1.Size = new Size(776, 277);
            tabControl1.SizeMode = TabSizeMode.Fixed;
            tabControl1.TabIndex = 10;
            // 
            // tabPage1
            // 
            tabPage1.Location = new Point(4, 24);
            tabPage1.Name = "tabPage1";
            tabPage1.Padding = new Padding(3);
            tabPage1.Size = new Size(768, 249);
            tabPage1.TabIndex = 0;
            tabPage1.Text = "Mirrors";
            tabPage1.UseVisualStyleBackColor = true;
            // 
            // tabPage2
            // 
            tabPage2.Controls.Add(Chests);
            tabPage2.Controls.Add(groupBox2);
            tabPage2.Location = new Point(4, 24);
            tabPage2.Name = "tabPage2";
            tabPage2.Padding = new Padding(3);
            tabPage2.Size = new Size(768, 249);
            tabPage2.TabIndex = 1;
            tabPage2.Text = "Chests & Items";
            tabPage2.UseVisualStyleBackColor = true;
            // 
            // Chests
            // 
            Chests.Controls.Add(RadioChestsNo);
            Chests.Controls.Add(GroupChestsProperties);
            Chests.Controls.Add(RadioChestsShuffle);
            Chests.Controls.Add(RadioChestsUnchanged);
            Chests.Font = new Font("Segoe UI", 9F, FontStyle.Bold, GraphicsUnit.Point, 0);
            Chests.Location = new Point(397, 6);
            Chests.Name = "Chests";
            Chests.Size = new Size(365, 237);
            Chests.TabIndex = 3;
            Chests.TabStop = false;
            Chests.Text = "Chests";
            // 
            // RadioChestsNo
            // 
            RadioChestsNo.AutoSize = true;
            RadioChestsNo.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            RadioChestsNo.Location = new Point(6, 72);
            RadioChestsNo.Name = "RadioChestsNo";
            RadioChestsNo.Size = new Size(79, 19);
            RadioChestsNo.TabIndex = 15;
            RadioChestsNo.Text = "No Chests";
            RadioChestsNo.UseVisualStyleBackColor = true;
            RadioChestsNo.CheckedChanged += RadioChestsNo_CheckedChanged;
            // 
            // GroupChestsProperties
            // 
            GroupChestsProperties.Controls.Add(RadioChestsPropertiesRemoveHealing);
            GroupChestsProperties.Controls.Add(RadioChestsPropertiesRandom);
            GroupChestsProperties.Controls.Add(RadioChestsPropertiesUnchanged);
            GroupChestsProperties.Enabled = false;
            GroupChestsProperties.Font = new Font("Segoe UI", 9F, FontStyle.Bold, GraphicsUnit.Point, 0);
            GroupChestsProperties.Location = new Point(139, 12);
            GroupChestsProperties.Name = "GroupChestsProperties";
            GroupChestsProperties.Size = new Size(220, 219);
            GroupChestsProperties.TabIndex = 5;
            GroupChestsProperties.TabStop = false;
            GroupChestsProperties.Text = "Properties";
            // 
            // RadioChestsPropertiesRemoveHealing
            // 
            RadioChestsPropertiesRemoveHealing.AutoSize = true;
            RadioChestsPropertiesRemoveHealing.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            RadioChestsPropertiesRemoveHealing.Location = new Point(6, 47);
            RadioChestsPropertiesRemoveHealing.Name = "RadioChestsPropertiesRemoveHealing";
            RadioChestsPropertiesRemoveHealing.Size = new Size(144, 19);
            RadioChestsPropertiesRemoveHealing.TabIndex = 4;
            RadioChestsPropertiesRemoveHealing.Text = "Remove Healing Items";
            RadioChestsPropertiesRemoveHealing.UseVisualStyleBackColor = true;
            RadioChestsPropertiesRemoveHealing.CheckedChanged += RadioChestsPropertiesRemoveHealing_CheckedChanged;
            // 
            // RadioChestsPropertiesRandom
            // 
            RadioChestsPropertiesRandom.AutoSize = true;
            RadioChestsPropertiesRandom.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            RadioChestsPropertiesRandom.Location = new Point(6, 72);
            RadioChestsPropertiesRandom.Name = "RadioChestsPropertiesRandom";
            RadioChestsPropertiesRandom.Size = new Size(70, 19);
            RadioChestsPropertiesRandom.TabIndex = 1;
            RadioChestsPropertiesRandom.Text = "Random";
            RadioChestsPropertiesRandom.UseVisualStyleBackColor = true;
            RadioChestsPropertiesRandom.CheckedChanged += RadioChestsPropertiesRandom_CheckedChanged;
            // 
            // RadioChestsPropertiesUnchanged
            // 
            RadioChestsPropertiesUnchanged.AutoSize = true;
            RadioChestsPropertiesUnchanged.Checked = true;
            RadioChestsPropertiesUnchanged.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            RadioChestsPropertiesUnchanged.Location = new Point(6, 22);
            RadioChestsPropertiesUnchanged.Name = "RadioChestsPropertiesUnchanged";
            RadioChestsPropertiesUnchanged.Size = new Size(86, 19);
            RadioChestsPropertiesUnchanged.TabIndex = 0;
            RadioChestsPropertiesUnchanged.TabStop = true;
            RadioChestsPropertiesUnchanged.Text = "Unchanged";
            RadioChestsPropertiesUnchanged.UseVisualStyleBackColor = true;
            RadioChestsPropertiesUnchanged.CheckedChanged += RadioChestsPropertiesUnchanged_CheckedChanged;
            // 
            // RadioChestsShuffle
            // 
            RadioChestsShuffle.AutoSize = true;
            RadioChestsShuffle.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            RadioChestsShuffle.Location = new Point(6, 47);
            RadioChestsShuffle.Name = "RadioChestsShuffle";
            RadioChestsShuffle.Size = new Size(62, 19);
            RadioChestsShuffle.TabIndex = 14;
            RadioChestsShuffle.Text = "Shuffle";
            RadioChestsShuffle.UseVisualStyleBackColor = true;
            RadioChestsShuffle.CheckedChanged += RadioChestsShuffle_CheckedChanged;
            // 
            // RadioChestsUnchanged
            // 
            RadioChestsUnchanged.AutoSize = true;
            RadioChestsUnchanged.Checked = true;
            RadioChestsUnchanged.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            RadioChestsUnchanged.Location = new Point(6, 22);
            RadioChestsUnchanged.Name = "RadioChestsUnchanged";
            RadioChestsUnchanged.Size = new Size(86, 19);
            RadioChestsUnchanged.TabIndex = 14;
            RadioChestsUnchanged.TabStop = true;
            RadioChestsUnchanged.Text = "Unchanged";
            RadioChestsUnchanged.UseVisualStyleBackColor = true;
            RadioChestsUnchanged.CheckedChanged += RadioChestsUnchanged_CheckedChanged;
            // 
            // groupBox2
            // 
            groupBox2.Controls.Add(RadioConsumablesChallenge);
            groupBox2.Controls.Add(RadioConsumablesNo);
            groupBox2.Controls.Add(button1);
            groupBox2.Controls.Add(RadioConsumablesCustom);
            groupBox2.Controls.Add(GroupConsumablesMirrorShards);
            groupBox2.Controls.Add(RadioConsumablesRandom);
            groupBox2.Controls.Add(RadioConsumablesShuffle);
            groupBox2.Controls.Add(RadioConsumablesUnchanged);
            groupBox2.Font = new Font("Segoe UI", 9F, FontStyle.Bold, GraphicsUnit.Point, 0);
            groupBox2.Location = new Point(9, 6);
            groupBox2.Name = "groupBox2";
            groupBox2.Size = new Size(382, 237);
            groupBox2.TabIndex = 2;
            groupBox2.TabStop = false;
            groupBox2.Text = "Consumables";
            // 
            // RadioConsumablesChallenge
            // 
            RadioConsumablesChallenge.AutoSize = true;
            RadioConsumablesChallenge.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            RadioConsumablesChallenge.Location = new Point(10, 97);
            RadioConsumablesChallenge.Name = "RadioConsumablesChallenge";
            RadioConsumablesChallenge.Size = new Size(78, 19);
            RadioConsumablesChallenge.TabIndex = 15;
            RadioConsumablesChallenge.Text = "Challenge";
            RadioConsumablesChallenge.UseVisualStyleBackColor = true;
            RadioConsumablesChallenge.CheckedChanged += RadioConsumablesChallenge_CheckedChanged;
            // 
            // RadioConsumablesNo
            // 
            RadioConsumablesNo.AutoSize = true;
            RadioConsumablesNo.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            RadioConsumablesNo.Location = new Point(10, 122);
            RadioConsumablesNo.Name = "RadioConsumablesNo";
            RadioConsumablesNo.Size = new Size(116, 19);
            RadioConsumablesNo.TabIndex = 14;
            RadioConsumablesNo.Text = "No Consumables";
            RadioConsumablesNo.UseVisualStyleBackColor = true;
            RadioConsumablesNo.CheckedChanged += RadioConsumablesNo_CheckedChanged;
            // 
            // button1
            // 
            button1.Enabled = false;
            button1.Font = new Font("Segoe UI", 9F);
            button1.Location = new Point(10, 172);
            button1.Name = "button1";
            button1.Size = new Size(114, 23);
            button1.TabIndex = 13;
            button1.Text = "Edit Probability";
            button1.UseVisualStyleBackColor = true;
            button1.Visible = false;
            // 
            // RadioConsumablesCustom
            // 
            RadioConsumablesCustom.AutoSize = true;
            RadioConsumablesCustom.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            RadioConsumablesCustom.Location = new Point(10, 147);
            RadioConsumablesCustom.Name = "RadioConsumablesCustom";
            RadioConsumablesCustom.Size = new Size(67, 19);
            RadioConsumablesCustom.TabIndex = 4;
            RadioConsumablesCustom.Text = "Custom";
            RadioConsumablesCustom.UseVisualStyleBackColor = true;
            RadioConsumablesCustom.Visible = false;
            RadioConsumablesCustom.CheckedChanged += RadioConsumablesCustom_CheckedChanged;
            // 
            // GroupConsumablesMirrorShards
            // 
            GroupConsumablesMirrorShards.Controls.Add(RadioMirrorShardsRandom);
            GroupConsumablesMirrorShards.Controls.Add(GroupMirrorShardsAmount);
            GroupConsumablesMirrorShards.Controls.Add(RadioMirrorShardsCustom);
            GroupConsumablesMirrorShards.Controls.Add(RadioMirrorShardsUnchanged);
            GroupConsumablesMirrorShards.Enabled = false;
            GroupConsumablesMirrorShards.Font = new Font("Segoe UI", 9F, FontStyle.Bold, GraphicsUnit.Point, 0);
            GroupConsumablesMirrorShards.Location = new Point(159, 12);
            GroupConsumablesMirrorShards.Name = "GroupConsumablesMirrorShards";
            GroupConsumablesMirrorShards.Size = new Size(217, 219);
            GroupConsumablesMirrorShards.TabIndex = 3;
            GroupConsumablesMirrorShards.TabStop = false;
            GroupConsumablesMirrorShards.Text = "Mirror Shards";
            // 
            // RadioMirrorShardsRandom
            // 
            RadioMirrorShardsRandom.AutoSize = true;
            RadioMirrorShardsRandom.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            RadioMirrorShardsRandom.Location = new Point(6, 47);
            RadioMirrorShardsRandom.Name = "RadioMirrorShardsRandom";
            RadioMirrorShardsRandom.Size = new Size(70, 19);
            RadioMirrorShardsRandom.TabIndex = 4;
            RadioMirrorShardsRandom.Text = "Random";
            RadioMirrorShardsRandom.UseVisualStyleBackColor = true;
            RadioMirrorShardsRandom.CheckedChanged += RadioMirrorShardsRandom_CheckedChanged;
            // 
            // GroupMirrorShardsAmount
            // 
            GroupMirrorShardsAmount.Controls.Add(TrackBarMirrorShardsAmount);
            GroupMirrorShardsAmount.Controls.Add(label5);
            GroupMirrorShardsAmount.Controls.Add(label1);
            GroupMirrorShardsAmount.Controls.Add(label4);
            GroupMirrorShardsAmount.Controls.Add(label2);
            GroupMirrorShardsAmount.Controls.Add(label3);
            GroupMirrorShardsAmount.Enabled = false;
            GroupMirrorShardsAmount.Location = new Point(6, 97);
            GroupMirrorShardsAmount.Name = "GroupMirrorShardsAmount";
            GroupMirrorShardsAmount.Size = new Size(205, 116);
            GroupMirrorShardsAmount.TabIndex = 3;
            GroupMirrorShardsAmount.TabStop = false;
            GroupMirrorShardsAmount.Text = "Amount";
            // 
            // TrackBarMirrorShardsAmount
            // 
            TrackBarMirrorShardsAmount.Location = new Point(6, 40);
            TrackBarMirrorShardsAmount.Maximum = 5;
            TrackBarMirrorShardsAmount.Minimum = 1;
            TrackBarMirrorShardsAmount.Name = "TrackBarMirrorShardsAmount";
            TrackBarMirrorShardsAmount.RightToLeft = RightToLeft.No;
            TrackBarMirrorShardsAmount.Size = new Size(193, 45);
            TrackBarMirrorShardsAmount.TabIndex = 2;
            TrackBarMirrorShardsAmount.TickStyle = TickStyle.TopLeft;
            TrackBarMirrorShardsAmount.Value = 1;
            // 
            // label5
            // 
            label5.AutoSize = true;
            label5.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            label5.Location = new Point(179, 22);
            label5.Name = "label5";
            label5.Size = new Size(13, 15);
            label5.TabIndex = 7;
            label5.Text = "5";
            // 
            // label1
            // 
            label1.AutoSize = true;
            label1.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            label1.Location = new Point(13, 22);
            label1.Name = "label1";
            label1.Size = new Size(13, 15);
            label1.TabIndex = 3;
            label1.Text = "1";
            // 
            // label4
            // 
            label4.AutoSize = true;
            label4.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            label4.Location = new Point(138, 22);
            label4.Name = "label4";
            label4.Size = new Size(13, 15);
            label4.TabIndex = 6;
            label4.Text = "4";
            // 
            // label2
            // 
            label2.AutoSize = true;
            label2.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            label2.Location = new Point(56, 22);
            label2.Name = "label2";
            label2.Size = new Size(13, 15);
            label2.TabIndex = 4;
            label2.Text = "2";
            // 
            // label3
            // 
            label3.AutoSize = true;
            label3.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            label3.Location = new Point(96, 22);
            label3.Name = "label3";
            label3.Size = new Size(13, 15);
            label3.TabIndex = 5;
            label3.Text = "3";
            // 
            // RadioMirrorShardsCustom
            // 
            RadioMirrorShardsCustom.AutoSize = true;
            RadioMirrorShardsCustom.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            RadioMirrorShardsCustom.Location = new Point(6, 72);
            RadioMirrorShardsCustom.Name = "RadioMirrorShardsCustom";
            RadioMirrorShardsCustom.Size = new Size(67, 19);
            RadioMirrorShardsCustom.TabIndex = 1;
            RadioMirrorShardsCustom.Text = "Custom";
            RadioMirrorShardsCustom.UseVisualStyleBackColor = true;
            RadioMirrorShardsCustom.CheckedChanged += RadioMirrorShardsCustom_CheckedChanged;
            // 
            // RadioMirrorShardsUnchanged
            // 
            RadioMirrorShardsUnchanged.AutoSize = true;
            RadioMirrorShardsUnchanged.Checked = true;
            RadioMirrorShardsUnchanged.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            RadioMirrorShardsUnchanged.Location = new Point(6, 22);
            RadioMirrorShardsUnchanged.Name = "RadioMirrorShardsUnchanged";
            RadioMirrorShardsUnchanged.Size = new Size(86, 19);
            RadioMirrorShardsUnchanged.TabIndex = 0;
            RadioMirrorShardsUnchanged.TabStop = true;
            RadioMirrorShardsUnchanged.Text = "Unchanged";
            RadioMirrorShardsUnchanged.UseVisualStyleBackColor = true;
            RadioMirrorShardsUnchanged.CheckedChanged += RadioMirrorShardsUnchanged_CheckedChanged;
            // 
            // RadioConsumablesRandom
            // 
            RadioConsumablesRandom.AutoSize = true;
            RadioConsumablesRandom.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            RadioConsumablesRandom.Location = new Point(10, 72);
            RadioConsumablesRandom.Name = "RadioConsumablesRandom";
            RadioConsumablesRandom.Size = new Size(70, 19);
            RadioConsumablesRandom.TabIndex = 2;
            RadioConsumablesRandom.Text = "Random";
            RadioConsumablesRandom.UseVisualStyleBackColor = true;
            RadioConsumablesRandom.CheckedChanged += RadioConsumablesRandom_CheckedChanged;
            // 
            // RadioConsumablesShuffle
            // 
            RadioConsumablesShuffle.AutoSize = true;
            RadioConsumablesShuffle.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            RadioConsumablesShuffle.Location = new Point(10, 47);
            RadioConsumablesShuffle.Name = "RadioConsumablesShuffle";
            RadioConsumablesShuffle.Size = new Size(62, 19);
            RadioConsumablesShuffle.TabIndex = 1;
            RadioConsumablesShuffle.Text = "Shuffle";
            RadioConsumablesShuffle.UseVisualStyleBackColor = true;
            RadioConsumablesShuffle.CheckedChanged += RadioConsumablesShuffle_CheckedChanged;
            // 
            // RadioConsumablesUnchanged
            // 
            RadioConsumablesUnchanged.AutoSize = true;
            RadioConsumablesUnchanged.Checked = true;
            RadioConsumablesUnchanged.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            RadioConsumablesUnchanged.Location = new Point(10, 22);
            RadioConsumablesUnchanged.Name = "RadioConsumablesUnchanged";
            RadioConsumablesUnchanged.Size = new Size(86, 19);
            RadioConsumablesUnchanged.TabIndex = 0;
            RadioConsumablesUnchanged.TabStop = true;
            RadioConsumablesUnchanged.Text = "Unchanged";
            RadioConsumablesUnchanged.UseVisualStyleBackColor = true;
            RadioConsumablesUnchanged.CheckedChanged += RadioConsumablesUnchanged_CheckedChanged;
            // 
            // tabPage3
            // 
            tabPage3.Location = new Point(4, 24);
            tabPage3.Name = "tabPage3";
            tabPage3.Padding = new Padding(3);
            tabPage3.Size = new Size(768, 249);
            tabPage3.TabIndex = 2;
            tabPage3.Text = "Enemies";
            tabPage3.UseVisualStyleBackColor = true;
            // 
            // tabPage4
            // 
            tabPage4.Location = new Point(4, 24);
            tabPage4.Name = "tabPage4";
            tabPage4.Padding = new Padding(3);
            tabPage4.Size = new Size(768, 249);
            tabPage4.TabIndex = 3;
            tabPage4.Text = "Minibosses";
            tabPage4.UseVisualStyleBackColor = true;
            // 
            // tabPage5
            // 
            tabPage5.Location = new Point(4, 24);
            tabPage5.Name = "tabPage5";
            tabPage5.Padding = new Padding(3);
            tabPage5.Size = new Size(768, 249);
            tabPage5.TabIndex = 4;
            tabPage5.Text = "Abilities";
            tabPage5.UseVisualStyleBackColor = true;
            // 
            // TabMiscellaneous
            // 
            TabMiscellaneous.Controls.Add(GroupMiscSprayPalettes);
            TabMiscellaneous.Location = new Point(4, 24);
            TabMiscellaneous.Name = "TabMiscellaneous";
            TabMiscellaneous.Padding = new Padding(3);
            TabMiscellaneous.Size = new Size(768, 249);
            TabMiscellaneous.TabIndex = 5;
            TabMiscellaneous.Text = "Miscellaneous";
            TabMiscellaneous.UseVisualStyleBackColor = true;
            // 
            // GroupMiscSprayPalettes
            // 
            GroupMiscSprayPalettes.Controls.Add(GroupSprayOutlines);
            GroupMiscSprayPalettes.Controls.Add(RadioSprayPresets);
            GroupMiscSprayPalettes.Controls.Add(RadioSprayRandomAndPresets);
            GroupMiscSprayPalettes.Controls.Add(RadioSprayRandom);
            GroupMiscSprayPalettes.Controls.Add(RadioSprayUnchanged);
            GroupMiscSprayPalettes.Font = new Font("Segoe UI", 9F, FontStyle.Bold, GraphicsUnit.Point, 0);
            GroupMiscSprayPalettes.Location = new Point(9, 6);
            GroupMiscSprayPalettes.Name = "GroupMiscSprayPalettes";
            GroupMiscSprayPalettes.Size = new Size(279, 237);
            GroupMiscSprayPalettes.TabIndex = 0;
            GroupMiscSprayPalettes.TabStop = false;
            GroupMiscSprayPalettes.Text = "Spray Palettes";
            // 
            // GroupSprayOutlines
            // 
            GroupSprayOutlines.Controls.Add(RadioOutlinesRandom);
            GroupSprayOutlines.Controls.Add(RadioOutlinesAll);
            GroupSprayOutlines.Controls.Add(RadioOutlinesUnchanged);
            GroupSprayOutlines.Location = new Point(142, 12);
            GroupSprayOutlines.Name = "GroupSprayOutlines";
            GroupSprayOutlines.Size = new Size(131, 219);
            GroupSprayOutlines.TabIndex = 5;
            GroupSprayOutlines.TabStop = false;
            GroupSprayOutlines.Text = "Outlines";
            // 
            // RadioOutlinesRandom
            // 
            RadioOutlinesRandom.AutoSize = true;
            RadioOutlinesRandom.Font = new Font("Segoe UI", 9F);
            RadioOutlinesRandom.Location = new Point(6, 72);
            RadioOutlinesRandom.Name = "RadioOutlinesRandom";
            RadioOutlinesRandom.Size = new Size(70, 19);
            RadioOutlinesRandom.TabIndex = 8;
            RadioOutlinesRandom.Text = "Random";
            RadioOutlinesRandom.UseVisualStyleBackColor = true;
            RadioOutlinesRandom.CheckedChanged += RadioOutlinesRandom_CheckedChanged;
            // 
            // RadioOutlinesAll
            // 
            RadioOutlinesAll.AutoSize = true;
            RadioOutlinesAll.Font = new Font("Segoe UI", 9F);
            RadioOutlinesAll.Location = new Point(6, 47);
            RadioOutlinesAll.Name = "RadioOutlinesAll";
            RadioOutlinesAll.Size = new Size(83, 19);
            RadioOutlinesAll.TabIndex = 7;
            RadioOutlinesAll.Text = "All Palettes";
            RadioOutlinesAll.UseVisualStyleBackColor = true;
            RadioOutlinesAll.CheckedChanged += RadioOutlinesAll_CheckedChanged;
            // 
            // RadioOutlinesUnchanged
            // 
            RadioOutlinesUnchanged.AutoSize = true;
            RadioOutlinesUnchanged.Checked = true;
            RadioOutlinesUnchanged.Font = new Font("Segoe UI", 9F);
            RadioOutlinesUnchanged.Location = new Point(6, 22);
            RadioOutlinesUnchanged.Name = "RadioOutlinesUnchanged";
            RadioOutlinesUnchanged.Size = new Size(86, 19);
            RadioOutlinesUnchanged.TabIndex = 6;
            RadioOutlinesUnchanged.TabStop = true;
            RadioOutlinesUnchanged.Text = "Unchanged";
            RadioOutlinesUnchanged.UseVisualStyleBackColor = true;
            RadioOutlinesUnchanged.CheckedChanged += RadioOutlinesUnchanged_CheckedChanged;
            // 
            // RadioSprayPresets
            // 
            RadioSprayPresets.AutoSize = true;
            RadioSprayPresets.Font = new Font("Segoe UI", 9F);
            RadioSprayPresets.Location = new Point(10, 47);
            RadioSprayPresets.Name = "RadioSprayPresets";
            RadioSprayPresets.Size = new Size(90, 19);
            RadioSprayPresets.TabIndex = 3;
            RadioSprayPresets.Text = "Presets Only";
            RadioSprayPresets.UseVisualStyleBackColor = true;
            RadioSprayPresets.CheckedChanged += RadioSprayPresets_CheckedChanged;
            // 
            // RadioSprayRandomAndPresets
            // 
            RadioSprayRandomAndPresets.AutoSize = true;
            RadioSprayRandomAndPresets.Font = new Font("Segoe UI", 9F);
            RadioSprayRandomAndPresets.Location = new Point(10, 72);
            RadioSprayRandomAndPresets.Name = "RadioSprayRandomAndPresets";
            RadioSprayRandomAndPresets.Size = new Size(121, 19);
            RadioSprayRandomAndPresets.TabIndex = 2;
            RadioSprayRandomAndPresets.Text = "Random + Presets";
            RadioSprayRandomAndPresets.UseVisualStyleBackColor = true;
            RadioSprayRandomAndPresets.CheckedChanged += RadioSprayRandomAndPresets_CheckedChanged;
            // 
            // RadioSprayRandom
            // 
            RadioSprayRandom.AutoSize = true;
            RadioSprayRandom.Font = new Font("Segoe UI", 9F);
            RadioSprayRandom.Location = new Point(10, 97);
            RadioSprayRandom.Name = "RadioSprayRandom";
            RadioSprayRandom.Size = new Size(70, 19);
            RadioSprayRandom.TabIndex = 1;
            RadioSprayRandom.Text = "Random";
            RadioSprayRandom.UseVisualStyleBackColor = true;
            RadioSprayRandom.CheckedChanged += RadioSprayRandom_CheckedChanged;
            // 
            // RadioSprayUnchanged
            // 
            RadioSprayUnchanged.AutoSize = true;
            RadioSprayUnchanged.Checked = true;
            RadioSprayUnchanged.Font = new Font("Segoe UI", 9F);
            RadioSprayUnchanged.Location = new Point(10, 22);
            RadioSprayUnchanged.Name = "RadioSprayUnchanged";
            RadioSprayUnchanged.Size = new Size(86, 19);
            RadioSprayUnchanged.TabIndex = 0;
            RadioSprayUnchanged.TabStop = true;
            RadioSprayUnchanged.Text = "Unchanged";
            RadioSprayUnchanged.UseVisualStyleBackColor = true;
            RadioSprayUnchanged.CheckedChanged += RadioSprayUnchanged_CheckedChanged;
            // 
            // ButtonRefreshSeed
            // 
            ButtonRefreshSeed.Enabled = false;
            ButtonRefreshSeed.Location = new Point(626, 99);
            ButtonRefreshSeed.Name = "ButtonRefreshSeed";
            ButtonRefreshSeed.Size = new Size(162, 23);
            ButtonRefreshSeed.TabIndex = 11;
            ButtonRefreshSeed.Text = "Refresh Seed";
            ButtonRefreshSeed.UseVisualStyleBackColor = true;
            ButtonRefreshSeed.Click += ButtonRefreshSeed_Click;
            // 
            // LabelSeed
            // 
            LabelSeed.AutoSize = true;
            LabelSeed.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            LabelSeed.Location = new Point(626, 125);
            LabelSeed.Name = "LabelSeed";
            LabelSeed.Size = new Size(35, 15);
            LabelSeed.TabIndex = 12;
            LabelSeed.Text = "Seed:";
            LabelSeed.Visible = false;
            // 
            // KatAMRandomizerMain
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(800, 450);
            Controls.Add(LabelSeed);
            Controls.Add(ButtonRefreshSeed);
            Controls.Add(tabControl1);
            Controls.Add(GroupRomInfo);
            Controls.Add(GroupSettings);
            Controls.Add(ButtonInputSeed);
            Controls.Add(ButtonSaveFile);
            Controls.Add(ButtonLoadFile);
            Icon = (Icon)resources.GetObject("$this.Icon");
            Name = "KatAMRandomizerMain";
            Text = "KatAM Randomizer";
            GroupSettings.ResumeLayout(false);
            GroupSettings.PerformLayout();
            GroupRomInfo.ResumeLayout(false);
            GroupRomInfo.PerformLayout();
            tabControl1.ResumeLayout(false);
            tabPage2.ResumeLayout(false);
            Chests.ResumeLayout(false);
            Chests.PerformLayout();
            GroupChestsProperties.ResumeLayout(false);
            GroupChestsProperties.PerformLayout();
            groupBox2.ResumeLayout(false);
            groupBox2.PerformLayout();
            GroupConsumablesMirrorShards.ResumeLayout(false);
            GroupConsumablesMirrorShards.PerformLayout();
            GroupMirrorShardsAmount.ResumeLayout(false);
            GroupMirrorShardsAmount.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)TrackBarMirrorShardsAmount).EndInit();
            TabMiscellaneous.ResumeLayout(false);
            GroupMiscSprayPalettes.ResumeLayout(false);
            GroupMiscSprayPalettes.PerformLayout();
            GroupSprayOutlines.ResumeLayout(false);
            GroupSprayOutlines.PerformLayout();
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion

        private Button ButtonConsoleSend;
        private Button ButtonLoadFile;
        private Button ButtonSaveFile;
        private Button ButtonInputSeed;
        private Button button5;
        private Button button6;
        private GroupBox GroupSettings;
        private GroupBox GroupRomInfo;
        private Label LabelGameID;
        private Label LabelInternalName;
        private Label LabelROMName;
        private TabControl tabControl1;
        private TabPage tabPage1;
        private TabPage tabPage2;
        private TabPage tabPage3;
        private TabPage tabPage4;
        private TabPage tabPage5;
        private TabPage TabMiscellaneous;
        private Label LabelGameRegion;
        private CheckBox checkBox1;
        private GroupBox GroupMiscSprayPalettes;
        private RadioButton RadioSprayUnchanged;
        private RadioButton RadioSprayRandom;
        private RadioButton RadioSprayPresets;
        private RadioButton RadioSprayRandomAndPresets;
        private GroupBox GroupSprayOutlines;
        private RadioButton RadioOutlinesRandom;
        private RadioButton RadioOutlinesAll;
        private RadioButton RadioOutlinesUnchanged;
        private Button ButtonRefreshSeed;
        private GroupBox groupBox2;
        private GroupBox GroupConsumablesMirrorShards;
        private RadioButton RadioMirrorShardsCustom;
        private RadioButton RadioMirrorShardsUnchanged;
        private RadioButton RadioConsumablesRandom;
        private RadioButton RadioConsumablesShuffle;
        private RadioButton RadioConsumablesUnchanged;
        private TrackBar TrackBarMirrorShardsAmount;
        private Label label1;
        private GroupBox GroupMirrorShardsAmount;
        private Label label5;
        private Label label4;
        private Label label2;
        private Label label3;
        private RadioButton RadioMirrorShardsRandom;
        private Label LabelSeed;
        private GroupBox Chests;
        private Button button1;
        private RadioButton RadioConsumablesCustom;
        private RadioButton RadioChestsShuffle;
        private RadioButton RadioChestsUnchanged;
        private RadioButton RadioChestsNo;
        private GroupBox GroupChestsProperties;
        private RadioButton RadioChestsPropertiesRemoveHealing;
        private RadioButton RadioChestsPropertiesRandom;
        private RadioButton RadioChestsPropertiesUnchanged;
        private RadioButton RadioConsumablesNo;
        private RadioButton RadioConsumablesChallenge;
    }
}
