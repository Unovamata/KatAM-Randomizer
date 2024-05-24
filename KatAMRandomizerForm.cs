using KatAMEntities;
using KatAMInternal;
using System.ComponentModel;
using System.Text;
using System.Windows.Forms;

namespace KatAM_Randomizer
{
    public partial class KatAMRandomizerMain : Form {
        public KatAMRandomizerMain() {
            InitializeComponent();
        }

        private void ButtonConsoleSend_Click(object sender, EventArgs e) {
            Entity entity = new Entity("Entity", 1, 1, 1, 1, 1, 1, 1, 1, 1);

            Console.Clear();
            ShowObjectData(entity);
            Console.WriteLine();
            ShowObjectData(new Entity());
        }

        void ShowObjectData(Entity entity) {
            //Console.Clear();

            foreach (PropertyDescriptor descriptor in TypeDescriptor.GetProperties(entity)) {
                string name = descriptor.Name;
                object value = descriptor.GetValue(entity);
                Console.WriteLine("{0}={1}", name, value);
            }
        }

        private void ButtonLoadFile_Click(object sender, EventArgs e) {
            OpenFileDialog fileDialog = new OpenFileDialog();

            fileDialog.Filter = "GBA ROM Files (*.gba)|*.gba";
            fileDialog.FilterIndex = 1;
            fileDialog.RestoreDirectory = true;

            if(fileDialog.ShowDialog() == DialogResult.OK) {
                try {
                    string filePath = fileDialog.FileName;
                    int nameLastIndex = filePath.LastIndexOf('\u005C');
                    string gameName = "";

                    Console.WriteLine(filePath);

                    if(nameLastIndex != -1) {
                        gameName = filePath.Substring(nameLastIndex + 1)
                            .Replace(".gba", "");

                        LabelROMName.Text = gameName;
                    }

                    if (gameName == "") return;
                    
                    byte[] fileContents = File.ReadAllBytes(filePath);

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
                } catch (Exception ex) {
                    MessageBox.Show($"Error loading file: {ex.Message}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }

        }
    }
}
