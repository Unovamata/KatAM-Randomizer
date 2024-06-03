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
            button2 = new Button();
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
            groupBox2 = new GroupBox();
            button1 = new Button();
            radioButton3 = new RadioButton();
            groupBox1 = new GroupBox();
            RadioMirrorShardsRandom = new RadioButton();
            GroupMirrorShardsAmount = new GroupBox();
            TrackBarMirrorShardsAmount = new TrackBar();
            label5 = new Label();
            label1 = new Label();
            label4 = new Label();
            label2 = new Label();
            label3 = new Label();
            RadioMirrorShardsFixed = new RadioButton();
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
            GroupSettings.SuspendLayout();
            GroupRomInfo.SuspendLayout();
            tabControl1.SuspendLayout();
            tabPage2.SuspendLayout();
            groupBox2.SuspendLayout();
            groupBox1.SuspendLayout();
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
            // button2
            // 
            button2.Location = new Point(626, 70);
            button2.Name = "button2";
            button2.Size = new Size(162, 23);
            button2.TabIndex = 3;
            button2.Text = "Input Premade Seed";
            button2.UseVisualStyleBackColor = true;
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
            tabPage2.Controls.Add(groupBox2);
            tabPage2.Location = new Point(4, 24);
            tabPage2.Name = "tabPage2";
            tabPage2.Padding = new Padding(3);
            tabPage2.Size = new Size(768, 249);
            tabPage2.TabIndex = 1;
            tabPage2.Text = "Chests & Items";
            tabPage2.UseVisualStyleBackColor = true;
            // 
            // groupBox2
            // 
            groupBox2.Controls.Add(button1);
            groupBox2.Controls.Add(radioButton3);
            groupBox2.Controls.Add(groupBox1);
            groupBox2.Controls.Add(RadioConsumablesRandom);
            groupBox2.Controls.Add(RadioConsumablesShuffle);
            groupBox2.Controls.Add(RadioConsumablesUnchanged);
            groupBox2.Font = new Font("Segoe UI", 9F, FontStyle.Bold, GraphicsUnit.Point, 0);
            groupBox2.Location = new Point(9, 6);
            groupBox2.Name = "groupBox2";
            groupBox2.Size = new Size(313, 237);
            groupBox2.TabIndex = 2;
            groupBox2.TabStop = false;
            groupBox2.Text = "Consumables";
            // 
            // button1
            // 
            button1.Enabled = false;
            button1.Font = new Font("Segoe UI", 9F);
            button1.Location = new Point(10, 122);
            button1.Name = "button1";
            button1.Size = new Size(114, 23);
            button1.TabIndex = 13;
            button1.Text = "Edit Probability";
            button1.UseVisualStyleBackColor = true;
            // 
            // radioButton3
            // 
            radioButton3.AutoSize = true;
            radioButton3.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            radioButton3.Location = new Point(10, 97);
            radioButton3.Name = "radioButton3";
            radioButton3.Size = new Size(67, 19);
            radioButton3.TabIndex = 4;
            radioButton3.TabStop = true;
            radioButton3.Text = "Custom";
            radioButton3.UseVisualStyleBackColor = true;
            // 
            // groupBox1
            // 
            groupBox1.Controls.Add(RadioMirrorShardsRandom);
            groupBox1.Controls.Add(GroupMirrorShardsAmount);
            groupBox1.Controls.Add(RadioMirrorShardsFixed);
            groupBox1.Controls.Add(RadioMirrorShardsUnchanged);
            groupBox1.Font = new Font("Segoe UI", 9F, FontStyle.Bold, GraphicsUnit.Point, 0);
            groupBox1.Location = new Point(130, 12);
            groupBox1.Name = "groupBox1";
            groupBox1.Size = new Size(177, 219);
            groupBox1.TabIndex = 3;
            groupBox1.TabStop = false;
            groupBox1.Text = "Mirror Shards";
            // 
            // RadioMirrorShardsRandom
            // 
            RadioMirrorShardsRandom.AutoSize = true;
            RadioMirrorShardsRandom.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            RadioMirrorShardsRandom.Location = new Point(6, 47);
            RadioMirrorShardsRandom.Name = "RadioMirrorShardsRandom";
            RadioMirrorShardsRandom.Size = new Size(70, 19);
            RadioMirrorShardsRandom.TabIndex = 4;
            RadioMirrorShardsRandom.TabStop = true;
            RadioMirrorShardsRandom.Text = "Random";
            RadioMirrorShardsRandom.UseVisualStyleBackColor = true;
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
            GroupMirrorShardsAmount.Location = new Point(6, 121);
            GroupMirrorShardsAmount.Name = "GroupMirrorShardsAmount";
            GroupMirrorShardsAmount.Size = new Size(165, 92);
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
            TrackBarMirrorShardsAmount.Size = new Size(153, 45);
            TrackBarMirrorShardsAmount.TabIndex = 2;
            TrackBarMirrorShardsAmount.TickStyle = TickStyle.TopLeft;
            TrackBarMirrorShardsAmount.Value = 1;
            // 
            // label5
            // 
            label5.AutoSize = true;
            label5.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            label5.Location = new Point(140, 22);
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
            label4.Location = new Point(109, 22);
            label4.Name = "label4";
            label4.Size = new Size(13, 15);
            label4.TabIndex = 6;
            label4.Text = "4";
            // 
            // label2
            // 
            label2.AutoSize = true;
            label2.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            label2.Location = new Point(46, 22);
            label2.Name = "label2";
            label2.Size = new Size(13, 15);
            label2.TabIndex = 4;
            label2.Text = "2";
            // 
            // label3
            // 
            label3.AutoSize = true;
            label3.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            label3.Location = new Point(77, 22);
            label3.Name = "label3";
            label3.Size = new Size(13, 15);
            label3.TabIndex = 5;
            label3.Text = "3";
            // 
            // RadioMirrorShardsFixed
            // 
            RadioMirrorShardsFixed.AutoSize = true;
            RadioMirrorShardsFixed.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            RadioMirrorShardsFixed.Location = new Point(6, 72);
            RadioMirrorShardsFixed.Name = "RadioMirrorShardsFixed";
            RadioMirrorShardsFixed.Size = new Size(53, 19);
            RadioMirrorShardsFixed.TabIndex = 1;
            RadioMirrorShardsFixed.TabStop = true;
            RadioMirrorShardsFixed.Text = "Fixed";
            RadioMirrorShardsFixed.UseVisualStyleBackColor = true;
            // 
            // RadioMirrorShardsUnchanged
            // 
            RadioMirrorShardsUnchanged.AutoSize = true;
            RadioMirrorShardsUnchanged.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            RadioMirrorShardsUnchanged.Location = new Point(6, 22);
            RadioMirrorShardsUnchanged.Name = "RadioMirrorShardsUnchanged";
            RadioMirrorShardsUnchanged.Size = new Size(86, 19);
            RadioMirrorShardsUnchanged.TabIndex = 0;
            RadioMirrorShardsUnchanged.TabStop = true;
            RadioMirrorShardsUnchanged.Text = "Unchanged";
            RadioMirrorShardsUnchanged.UseVisualStyleBackColor = true;
            // 
            // RadioConsumablesRandom
            // 
            RadioConsumablesRandom.AutoSize = true;
            RadioConsumablesRandom.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            RadioConsumablesRandom.Location = new Point(10, 72);
            RadioConsumablesRandom.Name = "RadioConsumablesRandom";
            RadioConsumablesRandom.Size = new Size(70, 19);
            RadioConsumablesRandom.TabIndex = 2;
            RadioConsumablesRandom.TabStop = true;
            RadioConsumablesRandom.Text = "Random";
            RadioConsumablesRandom.UseVisualStyleBackColor = true;
            // 
            // RadioConsumablesShuffle
            // 
            RadioConsumablesShuffle.AutoSize = true;
            RadioConsumablesShuffle.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            RadioConsumablesShuffle.Location = new Point(10, 47);
            RadioConsumablesShuffle.Name = "RadioConsumablesShuffle";
            RadioConsumablesShuffle.Size = new Size(62, 19);
            RadioConsumablesShuffle.TabIndex = 1;
            RadioConsumablesShuffle.TabStop = true;
            RadioConsumablesShuffle.Text = "Shuffle";
            RadioConsumablesShuffle.UseVisualStyleBackColor = true;
            // 
            // RadioConsumablesUnchanged
            // 
            RadioConsumablesUnchanged.AutoSize = true;
            RadioConsumablesUnchanged.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            RadioConsumablesUnchanged.Location = new Point(10, 22);
            RadioConsumablesUnchanged.Name = "RadioConsumablesUnchanged";
            RadioConsumablesUnchanged.Size = new Size(86, 19);
            RadioConsumablesUnchanged.TabIndex = 0;
            RadioConsumablesUnchanged.TabStop = true;
            RadioConsumablesUnchanged.Text = "Unchanged";
            RadioConsumablesUnchanged.UseVisualStyleBackColor = true;
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
            GroupMiscSprayPalettes.Size = new Size(279, 138);
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
            GroupSprayOutlines.Size = new Size(131, 120);
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
            ButtonRefreshSeed.Location = new Point(626, 99);
            ButtonRefreshSeed.Name = "ButtonRefreshSeed";
            ButtonRefreshSeed.Size = new Size(162, 23);
            ButtonRefreshSeed.TabIndex = 11;
            ButtonRefreshSeed.Text = "Refresh Seed";
            ButtonRefreshSeed.UseVisualStyleBackColor = true;
            ButtonRefreshSeed.Click += ButtonRefreshSeed_Click;
            // 
            // KatAMRandomizerMain
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(800, 450);
            Controls.Add(ButtonRefreshSeed);
            Controls.Add(tabControl1);
            Controls.Add(GroupRomInfo);
            Controls.Add(GroupSettings);
            Controls.Add(button2);
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
            groupBox2.ResumeLayout(false);
            groupBox2.PerformLayout();
            groupBox1.ResumeLayout(false);
            groupBox1.PerformLayout();
            GroupMirrorShardsAmount.ResumeLayout(false);
            GroupMirrorShardsAmount.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)TrackBarMirrorShardsAmount).EndInit();
            TabMiscellaneous.ResumeLayout(false);
            GroupMiscSprayPalettes.ResumeLayout(false);
            GroupMiscSprayPalettes.PerformLayout();
            GroupSprayOutlines.ResumeLayout(false);
            GroupSprayOutlines.PerformLayout();
            ResumeLayout(false);
        }

        #endregion

        private Button ButtonConsoleSend;
        private Button ButtonLoadFile;
        private Button ButtonSaveFile;
        private Button button2;
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
        private GroupBox groupBox1;
        private RadioButton RadioMirrorShardsFixed;
        private RadioButton RadioMirrorShardsUnchanged;
        private RadioButton RadioConsumablesRandom;
        private RadioButton RadioConsumablesShuffle;
        private RadioButton RadioConsumablesUnchanged;
        private Button button1;
        private RadioButton radioButton3;
        private TrackBar TrackBarMirrorShardsAmount;
        private Label label1;
        private GroupBox GroupMirrorShardsAmount;
        private Label label5;
        private Label label4;
        private Label label2;
        private Label label3;
        private RadioButton RadioMirrorShardsRandom;
    }
}
