using System;
using System.Windows.Forms;

namespace KatAMMapTracker {
    internal static class Program {
        // Usage: "KatAM Map Tracker.exe" [path to the randomized ROM];
        [STAThread]
        static void Main(string[] args) {
            Application.SetHighDpiMode(HighDpiMode.SystemAware);
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new TrackerForm(args.Length > 0 ? args[0] : null));
        }
    }
}
