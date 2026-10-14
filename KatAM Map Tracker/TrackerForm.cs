using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Net.Sockets;
using System.Windows.Forms;

namespace KatAMMapTracker {
    /* TrackerForm; The tracker window. Loads the map of a (randomized) ROM, listens for the emulator script
     * and reveals the map as Kirby explores, saving the progress after every discovery; */
    public sealed class TrackerForm : Form {
        readonly MapView mapView = new MapView { Dock = DockStyle.Fill };
        readonly ToolStripButton followButton = new ToolStripButton("Follow Kirby") { CheckOnClick = true, Checked = true };
        readonly ToolStripStatusLabel connectionLabel = new ToolStripStatusLabel(),
                                      discoveryLabel = new ToolStripStatusLabel { Spring = true, TextAlign = ContentAlignment.MiddleRight };

        readonly TrackerServer server;

        RomMap map;
        Dictionary<int, RoomPlacement> layout;
        MapProgress progress;
        string romPath, progressPath;
        int previousRoom = -1;

        static readonly string SettingsFolder = Path.Combine(
            Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), "KatAM Map Tracker");
        static readonly string LastRomFile = Path.Combine(SettingsFolder, "last_rom.txt");

        public TrackerForm(string startupRom) {
            Text = "KatAM Map Tracker";
            Size = new Size(1100, 800);
            StartPosition = FormStartPosition.CenterScreen;
            KeyPreview = true;

            BuildMenus();

            Controls.Add(mapView);
            Controls.Add(BuildToolbar());
            Controls.Add(BuildStatusBar());
            mapView.BringToFront();

            mapView.FollowDisabledByUser += () => followButton.Checked = false;
            mapView.ViewChanged += UpdateDiscoveryLabel;

            server = new TrackerServer();
            server.RoomReported += (room, sessionStart) => BeginInvokeSafe(() => OnRoomReported(room, sessionStart));
            server.ConnectionChanged += connected => BeginInvokeSafe(() => SetConnected(connected));

            try {
                server.Start();
                SetConnected(false);
            }
            catch (SocketException exception) {
                connectionLabel.Text = $"Port {server.Port} is in use ({exception.Message}). Is another tracker open?";
            }

            string rom = startupRom ?? ReadLastRom();

            if (rom != null && File.Exists(rom)) Shown += (s, e) => LoadRom(rom);
        }

        void BuildMenus() {
            MenuStrip menu = new MenuStrip();
            ToolStripMenuItem file = new ToolStripMenuItem("File");
            file.DropDownItems.Add("Open ROM...", null, (s, e) => OpenRomDialog());
            file.DropDownItems.Add("Reset Progress...", null, (s, e) => ResetProgress());
            file.DropDownItems.Add(new ToolStripSeparator());
            file.DropDownItems.Add("Exit", null, (s, e) => Close());

            ToolStripMenuItem help = new ToolStripMenuItem("Help");
            help.DropDownItems.Add("How to use...", null, (s, e) => ShowHelp());

            menu.Items.Add(file);
            menu.Items.Add(help);
            MainMenuStrip = menu;
            Controls.Add(menu);
        }

        ToolStrip BuildToolbar() {
            ToolStrip toolbar = new ToolStrip { GripStyle = ToolStripGripStyle.Hidden };

            followButton.CheckedChanged += (s, e) => {
                mapView.Follow = followButton.Checked;
                if (followButton.Checked) mapView.OnRoomChanged();
            };

            toolbar.Items.Add(followButton);
            toolbar.Items.Add(new ToolStripSeparator());
            toolbar.Items.Add("Zoom In (+)", null, (s, e) => mapView.ZoomBy(1.25));
            toolbar.Items.Add("Zoom Out (-)", null, (s, e) => mapView.ZoomBy(0.8));
            toolbar.Items.Add("Fit Map (F)", null, (s, e) => mapView.FitToDiscovered());

            return toolbar;
        }

        StatusStrip BuildStatusBar() {
            StatusStrip status = new StatusStrip();
            status.Items.Add(connectionLabel);
            status.Items.Add(discoveryLabel);
            return status;
        }

        protected override void OnKeyDown(KeyEventArgs e) {
            base.OnKeyDown(e);

            switch (e.KeyCode) {
                case Keys.Add: case Keys.Oemplus: mapView.ZoomBy(1.25); break;
                case Keys.Subtract: case Keys.OemMinus: mapView.ZoomBy(0.8); break;
                case Keys.F: mapView.FitToDiscovered(); break;
                case Keys.Space: followButton.Checked = !followButton.Checked; break;
                default: return;
            }

            e.Handled = true;
        }

        //////////////////////////////////////////////////////////////////////////////////////////////

        void OpenRomDialog() {
            using OpenFileDialog dialog = new OpenFileDialog {
                Title = "Open the randomized ROM you're playing",
                Filter = "GBA ROM (*.gba)|*.gba|All files (*.*)|*.*",
            };

            if (dialog.ShowDialog(this) == DialogResult.OK) LoadRom(dialog.FileName);
        }

        void LoadRom(string path) {
            RomMap loaded;

            try {
                Cursor = Cursors.WaitCursor;
                loaded = RomMap.Load(path);
            }
            catch (Exception exception) when (exception is IOException || exception is UnauthorizedAccessException || exception is IndexOutOfRangeException) {
                MessageBox.Show(this, $"The ROM couldn't be read:\n{exception.Message}", Text, MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }
            finally {
                Cursor = Cursors.Default;
            }

            if (!loaded.IsSupportedGame) {
                MessageBox.Show(this, $"This ROM ({loaded.GameCode}) isn't the Japanese Kirby & the Amazing Mirror (B8KJ). The map can't be read.",
                                Text, MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            map = loaded;
            romPath = path;
            layout = MapLayout.Generate(map);
            progressPath = MapProgress.PathForRom(path);
            progress = MapProgress.Load(progressPath, map.Crc32);
            previousRoom = -1;

            Text = $"KatAM Map Tracker - {Path.GetFileName(path)}";
            mapView.SetMap(map, layout, progress);
            UpdateDiscoveryLabel();
            SaveLastRom(path);
        }

        void OnRoomReported(int room, bool sessionStart) {
            if (map == null) return;

            // The way Kirby reached the first room reported after connecting is unknown;
            if (sessionStart) previousRoom = -1;

            // Rooms that aren't in the ROM (title screen, menus) are ignored, keeping the last real room;
            if (!map.Rooms.ContainsKey(room)) return;

            bool changed = progress.EnterRoom(map, room, previousRoom);
            previousRoom = room;

            if (changed) SaveProgress();

            mapView.OnRoomChanged();
            UpdateDiscoveryLabel();
        }

        void SaveProgress() {
            try { progress.Save(progressPath); }
            catch (Exception exception) when (exception is IOException || exception is UnauthorizedAccessException) {
                connectionLabel.Text = $"Progress couldn't be saved: {exception.Message}";
            }
        }

        void ResetProgress() {
            if (progress == null) return;

            DialogResult answer = MessageBox.Show(this, "Forget every room and mirror discovered in this ROM?", Text,
                                                  MessageBoxButtons.YesNo, MessageBoxIcon.Warning);

            if (answer != DialogResult.Yes) return;

            progress.Reset();
            previousRoom = -1;
            SaveProgress();
            mapView.SetProgress(progress);
            mapView.FitToDiscovered();
            UpdateDiscoveryLabel();
        }

        void SetConnected(bool connected) {
            connectionLabel.Text = connected
                ? "Emulator connected."
                : $"Waiting for mGBA (load Lua/katam_tracker.lua, port {server.Port})...";
            connectionLabel.ForeColor = connected ? Color.DarkGreen : Color.DarkGoldenrod;
        }

        void UpdateDiscoveryLabel() {
            if (map == null || progress == null) {
                discoveryLabel.Text = "";
                return;
            }

            int doors = map.Rooms.Values.Sum(r => r.Doors.Count);
            discoveryLabel.Text = $"Rooms: {progress.VisitedRooms.Count}/{map.Rooms.Count}   Mirrors: {progress.FoundDoors.Count}/{doors}   Zoom: {mapView.Zoom:P0}";
        }

        void ShowHelp() {
            MessageBox.Show(this,
                "1. Open the randomized ROM you're playing (File > Open ROM...).\n" +
                "2. In mGBA 0.10 or newer: Tools > Scripting... > File > Load script, and pick Lua/katam_tracker.lua.\n" +
                "3. Play! Rooms appear as you enter them, and lines as you go through their mirrors.\n\n" +
                "Yellow rooms: every mirror used. Orange rooms: some mirrors left. The pulsing room is where Kirby is.\n" +
                "Dotted lines: Two-Way mirrors. Arrows: One-Way mirrors.\n\n" +
                "Mouse wheel: zoom.   Drag: move the map.   Hover: room preview.   Double click: center a room.\n" +
                "Space: follow Kirby on/off.   F: fit the discovered map.   +/-: zoom.\n\n" +
                "Progress is saved next to the ROM as <rom>.tracker.json.",
                "How to use", MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

        static string ReadLastRom() {
            try { return File.Exists(LastRomFile) ? File.ReadAllText(LastRomFile).Trim() : null; }
            catch (IOException) { return null; }
        }

        static void SaveLastRom(string path) {
            try {
                Directory.CreateDirectory(SettingsFolder);
                File.WriteAllText(LastRomFile, path);
            }
            catch (Exception exception) when (exception is IOException || exception is UnauthorizedAccessException) { }
        }

        void BeginInvokeSafe(Action action) {
            if (IsDisposed || !IsHandleCreated) return;

            try { BeginInvoke(action); }
            catch (InvalidOperationException) { } // The window is closing;
        }

        protected override void OnFormClosed(FormClosedEventArgs e) {
            server.Dispose();
            base.OnFormClosed(e);
        }
    }
}
