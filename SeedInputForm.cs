using KatAMInternal;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace KatAM_Randomizer {
    public partial class SeedInputForm : Form {
        Settings settings;

        public SeedInputForm() {
            settings = Settings.GetInstance();

            InitializeComponent();

            this.StartPosition = FormStartPosition.CenterScreen;

            NumericSeedInput.Value = settings.Seed;
        }

        private void ButtonInputPremadeSeed_Click(object sender, EventArgs e) {
            try { settings.Seed = (int)NumericSeedInput.Value; } catch { settings.GetNewSeed(); }
            KatAMRandomizerMain.Instance.UpdateLabelSeedText();
            this.Close();
        }
    }
}
