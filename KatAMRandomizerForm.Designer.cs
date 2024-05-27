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
            tabPage2.Location = new Point(4, 24);
            tabPage2.Name = "tabPage2";
            tabPage2.Padding = new Padding(3);
            tabPage2.Size = new Size(768, 249);
            tabPage2.TabIndex = 1;
            tabPage2.Text = "Chests & Items";
            tabPage2.UseVisualStyleBackColor = true;
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
            GroupSprayOutlines.Size = new Size(124, 118);
            GroupSprayOutlines.TabIndex = 5;
            GroupSprayOutlines.TabStop = false;
            GroupSprayOutlines.Text = "Outlines";
            // 
            // RadioOutlinesRandom
            // 
            RadioOutlinesRandom.AutoSize = true;
            RadioOutlinesRandom.Font = new Font("Segoe UI", 9F);
            RadioOutlinesRandom.Location = new Point(6, 75);
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
            RadioOutlinesAll.Location = new Point(6, 50);
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
    }
}
