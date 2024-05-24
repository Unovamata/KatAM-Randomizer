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
            button1 = new Button();
            button2 = new Button();
            button5 = new Button();
            button6 = new Button();
            GroupSettings = new GroupBox();
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
            tabPage6 = new TabPage();
            checkBox1 = new CheckBox();
            GroupSettings.SuspendLayout();
            GroupRomInfo.SuspendLayout();
            tabControl1.SuspendLayout();
            SuspendLayout();
            // 
            // ButtonConsoleSend
            // 
            ButtonConsoleSend.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            ButtonConsoleSend.Location = new Point(6, 109);
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
            // button1
            // 
            button1.Location = new Point(626, 41);
            button1.Name = "button1";
            button1.Size = new Size(162, 23);
            button1.TabIndex = 2;
            button1.Text = "Randomize (Save)";
            button1.UseVisualStyleBackColor = true;
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
            button5.Location = new Point(6, 51);
            button5.Name = "button5";
            button5.Size = new Size(158, 23);
            button5.TabIndex = 6;
            button5.Text = "Load Settings";
            button5.UseVisualStyleBackColor = true;
            // 
            // button6
            // 
            button6.Font = new Font("Segoe UI", 9F);
            button6.Location = new Point(6, 80);
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
            tabControl1.Controls.Add(tabPage6);
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
            // tabPage6
            // 
            tabPage6.Location = new Point(4, 24);
            tabPage6.Name = "tabPage6";
            tabPage6.Padding = new Padding(3);
            tabPage6.Size = new Size(768, 249);
            tabPage6.TabIndex = 5;
            tabPage6.Text = "Miscellaneous";
            tabPage6.UseVisualStyleBackColor = true;
            // 
            // checkBox1
            // 
            checkBox1.AutoSize = true;
            checkBox1.Font = new Font("Segoe UI", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
            checkBox1.Location = new Point(6, 25);
            checkBox1.Name = "checkBox1";
            checkBox1.Size = new Size(85, 19);
            checkBox1.TabIndex = 12;
            checkBox1.Text = "Race Mode";
            checkBox1.UseVisualStyleBackColor = true;
            // 
            // KatAMRandomizerMain
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(800, 450);
            Controls.Add(tabControl1);
            Controls.Add(GroupRomInfo);
            Controls.Add(GroupSettings);
            Controls.Add(button2);
            Controls.Add(button1);
            Controls.Add(ButtonLoadFile);
            Icon = (Icon)resources.GetObject("$this.Icon");
            Name = "KatAMRandomizerMain";
            Text = "KatAM Randomizer";
            GroupSettings.ResumeLayout(false);
            GroupSettings.PerformLayout();
            GroupRomInfo.ResumeLayout(false);
            GroupRomInfo.PerformLayout();
            tabControl1.ResumeLayout(false);
            ResumeLayout(false);
        }

        #endregion

        private Button ButtonConsoleSend;
        private Button ButtonLoadFile;
        private Button button1;
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
        private TabPage tabPage6;
        private Label LabelGameRegion;
        private CheckBox checkBox1;
    }
}
