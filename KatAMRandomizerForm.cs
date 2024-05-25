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

        public KatAMRandomizerMain() {
            InitializeComponent();
        }

        private void ButtonConsoleSend_Click(object sender, EventArgs e) {
            Entity entity = new Entity("Entity", 1, 1, 1, 1, 1, 1, 1, 1, 1);

            Console.Clear();
            ShowObjectData(system);
            Console.WriteLine();
            ShowObjectData(entity);
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

        private void ButtonSaveRandomized_Click(object sender, EventArgs e) {

        }

        private void ButtonSaveFile_Click(object sender, EventArgs e) {
            string newFileName = "KatAM.gba",
            destinationPath = Path.Combine(system.ROMDirectory, newFileName);

            Console.WriteLine($"Are the settings consistent? {system.ROMData.SequenceEqual(File.ReadAllBytes(system.ROMPath))}");

            KatAMSprays.RandomizeSpray(system);

            Console.WriteLine(destinationPath);

            File.WriteAllBytes(destinationPath, system.ROMData);

            Console.WriteLine($"Are the settings consistent? {system.ROMData.SequenceEqual(File.ReadAllBytes(system.ROMPath))} ");
        }
    }
}
