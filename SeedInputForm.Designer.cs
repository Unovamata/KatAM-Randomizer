namespace KatAMRandomizer {
    partial class SeedInputForm {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing) {
            if (disposing && (components != null)) {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent() {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(SeedInputForm));
            LabelInputSeed = new Label();
            NumericSeedInput = new NumericUpDown();
            ButtonInputPremadeSeed = new Button();
            ((System.ComponentModel.ISupportInitialize)NumericSeedInput).BeginInit();
            SuspendLayout();
            // 
            // LabelInputSeed
            // 
            LabelInputSeed.AutoSize = true;
            LabelInputSeed.Location = new Point(12, 9);
            LabelInputSeed.Name = "LabelInputSeed";
            LabelInputSeed.Size = new Size(122, 15);
            LabelInputSeed.TabIndex = 0;
            LabelInputSeed.Text = "Input a Seed Number:";
            // 
            // NumericSeedInput
            // 
            NumericSeedInput.Location = new Point(140, 7);
            NumericSeedInput.Maximum = new decimal(new int[] { 9999999, 0, 0, 0 });
            NumericSeedInput.Minimum = new decimal(new int[] { 9999999, 0, 0, int.MinValue });
            NumericSeedInput.Name = "NumericSeedInput";
            NumericSeedInput.Size = new Size(120, 23);
            NumericSeedInput.TabIndex = 1;
            // 
            // ButtonInputPremadeSeed
            // 
            ButtonInputPremadeSeed.Location = new Point(12, 39);
            ButtonInputPremadeSeed.Name = "ButtonInputPremadeSeed";
            ButtonInputPremadeSeed.Size = new Size(248, 23);
            ButtonInputPremadeSeed.TabIndex = 2;
            ButtonInputPremadeSeed.Text = "Enter";
            ButtonInputPremadeSeed.UseVisualStyleBackColor = true;
            ButtonInputPremadeSeed.Click += ButtonInputPremadeSeed_Click;
            // 
            // SeedInputForm
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(276, 74);
            Controls.Add(ButtonInputPremadeSeed);
            Controls.Add(NumericSeedInput);
            Controls.Add(LabelInputSeed);
            FormBorderStyle = FormBorderStyle.FixedDialog;
            Icon = (Icon)resources.GetObject("$this.Icon");
            MaximizeBox = false;
            MinimizeBox = false;
            Name = "SeedInputForm";
            SizeGripStyle = SizeGripStyle.Hide;
            Text = "Input Premade Seed";
            ((System.ComponentModel.ISupportInitialize)NumericSeedInput).EndInit();
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion

        private Label LabelInputSeed;
        private NumericUpDown NumericSeedInput;
        private Button ButtonInputPremadeSeed;
    }
}