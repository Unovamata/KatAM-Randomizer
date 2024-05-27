using KatAMEntities;
using KatAMInternal;
using System.ComponentModel;
using System.Text;
using System.Windows.Forms;
using KatAMRandomizer;

namespace KatAM_Randomizer
{
    public partial class KatAMRandomizerMain : Form {
        Processing system = new Processing();
        Settings settings;

        public KatAMRandomizerMain() {
            settings = system.Settings;

            InitializeComponent();
        }

        private void ButtonConsoleSend_Click(object sender, EventArgs e) {
            Entity entity = new Entity("Entity", 1, 1, 1, 1, 1, 1, 1, 1, 1);

            Console.Clear();
            ShowObjectData(system);
            Console.WriteLine();
            ShowObjectData(entity);
            Console.WriteLine();

            Console.WriteLine((int)settings.SprayGeneration);
            Console.WriteLine(settings.SprayOutlineGenerationType);

            /*int[] randomColor = KatAMSprays.RandomRGB(system.Settings);
            int[] color = KatAMSprays.RGBToHSV(209, 255, 149);
            int[] hsv = KatAMSprays.HSVToRGB(86, 105, 255);

            Console.WriteLine($"{(byte)randomColor[0]}, {(byte)randomColor[1]}, {(byte)randomColor[2]}");
            Console.WriteLine($"{color[0]}, {color[1]}, {color[2]}");
            Console.WriteLine($"{hsv[0]}, {hsv[1]}, {hsv[2]}");*/

        }

        void ShowObjectData(object inputObject) {
            //Console.Clear();

            foreach (PropertyDescriptor descriptor in TypeDescriptor.GetProperties(inputObject)) {
                string name = descriptor.Name;
                object value = descriptor.GetValue(inputObject);
                Console.WriteLine("{0}={1}", name, value);
            }
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
                } catch (Exception ex) {
                    MessageBox.Show($"Error loading file: {ex.Message}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }

        }

        private void ButtonSaveFile_Click(object sender, EventArgs e) {
            string newFileName = "KatAM.gba",
            destinationPath = Path.Combine(system.ROMDirectory, newFileName);
            settings.RandomEntity = new Random(settings.Seed);

            KatAMSprays.RandomizeSpray(system);

            File.WriteAllBytes(destinationPath, system.ROMData);
        }

        private void RadioSprayUnchanged_CheckedChanged(object sender, EventArgs e) {
            settings.SprayGeneration = SprayGen.Unchanged;
        }

        private void RadioSprayPresets_CheckedChanged(object sender, EventArgs e) {
            settings.SprayGeneration = SprayGen.Presets;
        }

        private void RadioSprayRandomAndPresets_CheckedChanged(object sender, EventArgs e) {
            settings.SprayGeneration = SprayGen.RandomAndPresets;
        }

        private void RadioSprayRandom_CheckedChanged(object sender, EventArgs e) {
            settings.SprayGeneration = SprayGen.Random;
        }

        private void RadioOutlinesUnchanged_CheckedChanged(object sender, EventArgs e) {
            settings.SprayOutlineGenerationType = SprayGen.Unchanged;
        }

        private void RadioOutlinesAll_CheckedChanged(object sender, EventArgs e) {
            settings.SprayOutlineGenerationType = SprayGen.All;
        }

        private void RadioOutlinesRandom_CheckedChanged(object sender, EventArgs e) {
            settings.SprayOutlineGenerationType = SprayGen.Random;
        }

        private void ButtonRefreshSeed_Click(object sender, EventArgs e) {
            settings.GetNewSeed();
        }
    }
}
