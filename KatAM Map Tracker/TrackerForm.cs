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
        readonly ToolStripMenuItem recentMenu = new ToolStripMenuItem("Open Recent"),
                                   revealAllItem = new ToolStripMenuItem("Reveal Entire Map (Spoilers)") { CheckOnClick = true },
                                   minimalistItem = new ToolStripMenuItem("Minimalist Mode") { ShortcutKeyDisplayString = "M" },
                                   dynamicScalingItem = new ToolStripMenuItem("Dynamic Scaling") { CheckOnClick = true, Checked = true },
                                   entitiesItem = new ToolStripMenuItem("Show on Map"),
                                   chestContentsItem = new ToolStripMenuItem("Show Chest Contents (Spoilers)") { CheckOnClick = true },
                                   mapsItem = new ToolStripMenuItem("Reveal Areas of Found Maps") { CheckOnClick = true },
                                   // DEBUG: clicking a room warps Kirby into it (DebugWarp()); to be removed for the release;
                                   debugWarpItem = new ToolStripMenuItem("Click a Room to Warp Kirby There") { CheckOnClick = true };
        MenuStrip menu;
        ToolStrip toolbar;
        StatusStrip statusBar;
        readonly ToolStripStatusLabel connectionLabel = new ToolStripStatusLabel(),
                                      discoveryLabel = new ToolStripStatusLabel { Spring = true, TextAlign = ContentAlignment.MiddleRight };

        readonly TrackerServer server;

        RomMap map;
        Dictionary<int, RoomPlacement> layout;
        MapProgress progress;
        string romPath, progressPath;
        int previousRoom = -1, lastReportedRoom = -1;

        // The kinds of objects that can be shown in the rooms, with their menu names and settings keys; Off by default but the cannons, stars
        // and Amazing Mirrors;
        static readonly (EntityKind kind, string name, string key)[] EntityCategories = {
            (EntityKind.Boss, "Bosses", "bosses"),
            (EntityKind.MiniBoss, "Mini-Bosses", "minibosses"),
            (EntityKind.BigSwitch, "Big Switches", "big_switches"),
            (EntityKind.MirrorShard, "Mirror Shards", "mirror_shards"),
            (EntityKind.AmazingMirror, "Amazing Mirrors", "amazing_mirrors"),
            (EntityKind.Treasure, "Chests", "chests"),
            (EntityKind.Item, "Items (Food, 1UPs...)", "items"),
            (EntityKind.FuseCannon, "Fuse Cannons", "fuse_cannons"),
            (EntityKind.WarpStar, "Warp Stars", "warp_stars"),
        };

        readonly Dictionary<EntityKind, ToolStripMenuItem> entityKindItems = new Dictionary<EntityKind, ToolStripMenuItem>();

        // Area maps the emulator's loaded save holds (bit n: the map of area n), as last reported by the script;
        int mapFlags;
        bool isConnected;

        static readonly string SettingsFolder = Path.Combine(
            Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), "KatAM Map Tracker");
        static readonly string LastRomFile = Path.Combine(SettingsFolder, "last_rom.txt"),
                               RecentRomsFile = Path.Combine(SettingsFolder, "recent_roms.txt"),
                               ViewSettingsFile = Path.Combine(SettingsFolder, "view_settings.txt");
        const int MaxRecentRoms = 10;

        public TrackerForm(string startupRom) {
            Text = "KatAM Map Tracker";
            Size = new Size(1100, 800);
            StartPosition = FormStartPosition.CenterScreen;
            KeyPreview = true;
            Icon = LoadIcon();

            BuildMenus();

            Controls.Add(mapView);
            Controls.Add(toolbar = BuildToolbar());
            Controls.Add(statusBar = BuildStatusBar());
            mapView.BringToFront();
            mapView.ContextMenuStrip = BuildContextMenu();

            LoadViewSettings();

            mapView.FollowDisabledByUser += () => followButton.Checked = false;
            mapView.ViewChanged += UpdateDiscoveryLabel;
            mapView.RoomClicked += DebugWarp;

            server = new TrackerServer();
            server.RoomReported += (room, sessionStart) => BeginInvokeSafe(() => OnRoomReported(room, sessionStart));
            server.ConnectionChanged += connected => BeginInvokeSafe(() => SetConnected(connected));
            server.ScriptVersionReported += version => BeginInvokeSafe(() => OnScriptVersion(version));
            server.MapsReported += maps => BeginInvokeSafe(() => OnMapsReported(maps));

            try {
                server.Start();
                SetConnected(false);
            }
            catch (SocketException exception) {
                connectionLabel.Text = $"Port {server.Port} is in use ({exception.Message}). Is another tracker open?";
            }

            AcceptRomDrops(this);
            AcceptRomDrops(mapView);

            string rom = startupRom ?? ReadRecentRoms().FirstOrDefault();

            if (rom != null && File.Exists(rom)) Shown += (s, e) => LoadRom(rom);
        }

        // LoadIcon(); The window icon (the star room square of the in-game map), embedded in the executable;
        static Icon LoadIcon() {
            using Stream stream = typeof(TrackerForm).Assembly.GetManifestResourceStream("MapTracker.ico");

            return stream == null ? null : new Icon(stream);
        }

        void BuildMenus() {
            menu = new MenuStrip();
            ToolStripMenuItem file = new ToolStripMenuItem("File");
            file.DropDownItems.Add("Open ROM...", null, (s, e) => OpenRomDialog());
            file.DropDownItems.Add(recentMenu);
            file.DropDownItems.Add(new ToolStripSeparator());

            ToolStripMenuItem export = new ToolStripMenuItem("Export Map Image");
            export.DropDownItems.Add("Discovered Map...", null, (s, e) => ExportImage(false));
            export.DropDownItems.Add("Entire Map (Spoilers)...", null, (s, e) => ExportImage(true));
            file.DropDownItems.Add(export);

            file.DropDownItems.Add("Reset Progress...", null, (s, e) => ResetProgress());
            file.DropDownItems.Add(new ToolStripSeparator());
            file.DropDownItems.Add("Exit", null, (s, e) => Close());
            file.DropDownOpening += (s, e) => UpdateRecentMenu();
            UpdateRecentMenu();

            ToolStripMenuItem view = new ToolStripMenuItem("View");
            revealAllItem.Click += (s, e) => SetRevealAll(revealAllItem.Checked);
            view.DropDownItems.Add(revealAllItem);
            view.DropDownItems.Add(new ToolStripSeparator());
            minimalistItem.Click += (s, e) => SetMinimalist(true);
            view.DropDownItems.Add(minimalistItem);
            dynamicScalingItem.CheckedChanged += (s, e) => {
                mapView.DynamicScaling = dynamicScalingItem.Checked;
                SaveViewSettings();
            };
            view.DropDownItems.Add(dynamicScalingItem);
            BuildEntityMenu();
            view.DropDownItems.Add(entitiesItem);
            chestContentsItem.CheckedChanged += (s, e) => {
                mapView.ShowChestContents = chestContentsItem.Checked;
                SaveViewSettings();
            };
            view.DropDownItems.Add(chestContentsItem);
            mapsItem.CheckedChanged += (s, e) => {
                UpdateMapRevealedRooms();
                SaveViewSettings();
            };
            view.DropDownItems.Add(mapsItem);

            // DEBUG menu (to be removed for the release);
            ToolStripMenuItem debug = new ToolStripMenuItem("Debug");
            debugWarpItem.CheckedChanged += (s, e) => SaveViewSettings();
            debug.DropDownItems.Add(debugWarpItem);

            ToolStripMenuItem help = new ToolStripMenuItem("Help");
            help.DropDownItems.Add("How to use...", null, (s, e) => ShowHelp());

            menu.Items.Add(file);
            menu.Items.Add(view);
            menu.Items.Add(debug);
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

        /* BuildEntityMenu(); View > Show on Map: which kinds of objects are drawn in the rooms, each on its own, off by default but
         * Fuse Cannons, Warp Stars and Amazing Mirrors (they are ways between rooms), plus Show All and Hide All; */
        void BuildEntityMenu() {
            foreach ((EntityKind kind, string name, string key) in EntityCategories) {
                ToolStripMenuItem item = new ToolStripMenuItem(name) { CheckOnClick = true, Checked = kind is EntityKind.FuseCannon or EntityKind.WarpStar or EntityKind.AmazingMirror };
                item.CheckedChanged += (s, e) => {
                    UpdateShownEntityKinds();
                    SaveViewSettings();
                };
                entityKindItems[kind] = item;
                entitiesItem.DropDownItems.Add(item);
            }

            entitiesItem.DropDownItems.Add(new ToolStripSeparator());
            entitiesItem.DropDownItems.Add("Show All", null, (s, e) => SetAllEntityKinds(true));
            entitiesItem.DropDownItems.Add("Hide All", null, (s, e) => SetAllEntityKinds(false));
            UpdateShownEntityKinds();
        }

        void SetAllEntityKinds(bool shown) {
            foreach (ToolStripMenuItem item in entityKindItems.Values) item.Checked = shown;
        }

        void UpdateShownEntityKinds() {
            mapView.ShownEntityKinds = new HashSet<EntityKind>(entityKindItems.Where(kvp => kvp.Value.Checked).Select(kvp => kvp.Key));
        }

        /* BuildContextMenu(); Right clicking the map; It's the way to reach the options in the minimalist mode,
         * where the menus, toolbar and status bar are hidden; */
        ContextMenuStrip BuildContextMenu() {
            ContextMenuStrip context = new ContextMenuStrip();
            ToolStripMenuItem minimalist = new ToolStripMenuItem("Minimalist Mode", null, (s, e) => SetMinimalist(!IsMinimalist)) { ShortcutKeyDisplayString = "M" },
                              follow = new ToolStripMenuItem("Follow Kirby", null, (s, e) => followButton.Checked = !followButton.Checked) { ShortcutKeyDisplayString = "Space" },
                              reveal = new ToolStripMenuItem("Reveal Entire Map (Spoilers)", null, (s, e) => {
                                  revealAllItem.Checked = !revealAllItem.Checked;
                                  SetRevealAll(revealAllItem.Checked);
                              }),
                              scaling = new ToolStripMenuItem("Dynamic Scaling", null, (s, e) => dynamicScalingItem.Checked = !dynamicScalingItem.Checked),
                              entities = new ToolStripMenuItem("Show on Map"),
                              contents = new ToolStripMenuItem("Show Chest Contents (Spoilers)", null, (s, e) => chestContentsItem.Checked = !chestContentsItem.Checked),
                              maps = new ToolStripMenuItem("Reveal Areas of Found Maps", null, (s, e) => mapsItem.Checked = !mapsItem.Checked);

            context.Items.Add(minimalist);
            context.Items.Add(new ToolStripSeparator());
            context.Items.Add(follow);
            context.Items.Add(new ToolStripMenuItem("Fit Map", null, (s, e) => mapView.FitToDiscovered()) { ShortcutKeyDisplayString = "F" });
            context.Items.Add(reveal);
            context.Items.Add(scaling);
            context.Items.Add(entities);
            context.Items.Add(contents);
            context.Items.Add(maps);
            context.Items.Add(new ToolStripSeparator());
            context.Items.Add("Open ROM...", null, (s, e) => OpenRomDialog());

            // The same categories as View > Show on Map, toggling them;
            foreach ((EntityKind kind, string name, string key) in EntityCategories) {
                ToolStripMenuItem item = new ToolStripMenuItem(name, null, (s, e) => entityKindItems[kind].Checked = !entityKindItems[kind].Checked);
                entities.DropDownItems.Add(item);
                entities.DropDownOpening += (s, e) => item.Checked = entityKindItems[kind].Checked;
            }

            context.Opening += (s, e) => {
                minimalist.Checked = IsMinimalist;
                follow.Checked = followButton.Checked;
                reveal.Checked = revealAllItem.Checked;
                reveal.Enabled = progress != null;
                scaling.Checked = dynamicScalingItem.Checked;
                entities.Checked = entityKindItems.Values.Any(item => item.Checked);
                contents.Checked = chestContentsItem.Checked;
                maps.Checked = mapsItem.Checked;
            };

            return context;
        }

        // Kept apart from the controls' Visible, which reads false until the window is shown;
        bool IsMinimalist { get; set; }

        /* SetMinimalist(); The minimalist mode hides everything but the map (menus, toolbar and status bar), so it can sit next to the game
         * without clutter. M toggles it, Esc leaves it, and right clicking the map has every option; */
        void SetMinimalist(bool minimalist) {
            IsMinimalist = minimalist;
            menu.Visible = toolbar.Visible = statusBar.Visible = !minimalist;
            minimalistItem.Checked = minimalist;

            if (minimalist) mapView.ShowHint("Minimalist mode: press M or Esc to leave it, or right click the map for options.");

            SaveViewSettings();
        }

        /* DebugWarp(); DEBUG (to be removed for the release): with Debug > Click a Room to Warp Kirby There on, a click on a room sends the
         * emulator script "WARP <room> <x> <y>", and it moves Kirby there as a door would: onto where a mirror into that room lands him,
         * else the room's own default spawn (-1 -1); */
        void DebugWarp(int room) {
            if (!debugWarpItem.Checked || map == null || room == progress?.CurrentRoom) return;
            DoorInfo door = map.Rooms.Values.SelectMany(r => r.Doors)
                .Where(d => d.Destination == room && d.Room != room && d.LandX > 0)
                .OrderBy(d => d.Kind == DoorKind.Mirror ? 0 : 1).ThenBy(d => d.Room).FirstOrDefault();
            (int x, int y) spawn = door != null && door.Kind == DoorKind.Mirror ? ((door.LandX - 8) / 16, (door.LandY - 8) / 16) : (-1, -1);
            bool sent = server.Send($"WARP {RomMap.RealRoom(room):X} {spawn.x} {spawn.y}");
            connectionLabel.Text = sent ? $"Debug: warping Kirby to room {RomMap.RealRoom(room):X3}..." : "Debug: the emulator script isn't connected, so Kirby can't be warped.";
        }

        // View settings (minimalist mode and dynamic scaling), as "key=value" lines in the settings folder;
        void LoadViewSettings() {
            try {
                if (!File.Exists(ViewSettingsFile)) return;

                Dictionary<string, string> values = File.ReadAllLines(ViewSettingsFile)
                    .Select(line => line.Split('=', 2)).Where(parts => parts.Length == 2)
                    .ToDictionary(parts => parts[0].Trim(), parts => parts[1].Trim());

                if (values.TryGetValue("dynamic_scaling", out string scaling)) dynamicScalingItem.Checked = scaling != "0";
                foreach ((EntityKind kind, string name, string key) in EntityCategories) {
                    if (values.TryGetValue("show_" + key, out string shown)) entityKindItems[kind].Checked = shown == "1";
                }
                if (values.TryGetValue("chest_contents", out string contents)) chestContentsItem.Checked = contents == "1";
                if (values.TryGetValue("area_maps", out string maps)) mapsItem.Checked = maps == "1";
                if (values.TryGetValue("debug_warp", out string warp)) debugWarpItem.Checked = warp == "1";
                if (values.TryGetValue("minimalist", out string minimalist) && minimalist == "1") Shown += (s, e) => SetMinimalist(true);
            }
            catch (Exception exception) when (exception is IOException || exception is UnauthorizedAccessException || exception is ArgumentException) { }
        }

        void SaveViewSettings() {
            try {
                Directory.CreateDirectory(SettingsFolder);
                File.WriteAllLines(ViewSettingsFile, new[] {
                    $"minimalist={(IsMinimalist ? 1 : 0)}",
                    $"dynamic_scaling={(dynamicScalingItem.Checked ? 1 : 0)}",
                    $"chest_contents={(chestContentsItem.Checked ? 1 : 0)}",
                    $"area_maps={(mapsItem.Checked ? 1 : 0)}",
                    $"debug_warp={(debugWarpItem.Checked ? 1 : 0)}",
                }.Concat(EntityCategories.Select(category => $"show_{category.key}={(entityKindItems[category.kind].Checked ? 1 : 0)}")));
            }
            catch (Exception exception) when (exception is IOException || exception is UnauthorizedAccessException) { }
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
                case Keys.M: SetMinimalist(!IsMinimalist); break;
                case Keys.Escape when IsMinimalist: SetMinimalist(false); break;
                default: return;
            }

            e.Handled = true;
        }

        //////////////////////////////////////////////////////////////////////////////////////////////

        // AcceptRomDrops(); A ROM dropped on the control (from Explorer, the first file if several) opens like File > Open ROM;
        void AcceptRomDrops(Control control) {
            control.AllowDrop = true;
            control.DragEnter += (s, e) =>
                e.Effect = e.Data?.GetData(DataFormats.FileDrop) is string[] files && files.Length > 0 && File.Exists(files[0]) ? DragDropEffects.Copy : DragDropEffects.None;
            control.DragDrop += (s, e) => {
                if (e.Data?.GetData(DataFormats.FileDrop) is string[] files && files.Length > 0 && File.Exists(files[0])) {
                    Activate();
                    BeginInvokeSafe(() => LoadRom(files[0]));
                }
            };
        }

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
                if (!File.Exists(path)) RemoveRecentRom(path);
                MessageBox.Show(this, $"The ROM couldn't be read:\n{exception.Message}", Text, MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }
            finally {
                Cursor = Cursors.Default;
            }

            if (!loaded.IsSupportedGame) {
                RemoveRecentRom(path);
                MessageBox.Show(this, $"This ROM ({loaded.GameCode}) isn't Kirby & the Amazing Mirror (B8KJ, B8KE or B8KP). The map can't be read.",
                                Text, MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            map = loaded;
            romPath = path;
            // Laying out the map and routing every connection around the rooms takes a moment;
            Cursor = Cursors.WaitCursor;
            MapPlan plan = MapPlan.Generate(map);
            Cursor = Cursors.Default;
            layout = plan.Layout;
            progressPath = MapProgress.PathForRom(path);
            progress = MapProgress.Load(progressPath, map.Crc32);
            previousRoom = -1;

            // Unmodified mirrors get the game's own map layout (see MapVanilla);
            Text = $"KatAM Map Tracker - {Path.GetFileName(path)}" + (plan.IsOriginalLayout ? " (original layout)" : "") + (map.IsExpanded ? " (ROM Expansion)" : "");
            revealAllItem.Checked = progress.RevealAll;
            mapView.SetMap(map, plan, progress);
            UpdateDiscoveryLabel();
            AddRecentRom(path);

            // The emulator may already be in a room; The map starts there instead of waiting for the next room change;
            if (isConnected && lastReportedRoom >= 0) OnRoomReported(lastReportedRoom, true);
        }

        void OnRoomReported(int room, bool sessionStart) {
            lastReportedRoom = room;

            if (map == null) return;

            // The way Kirby reached the first room reported after connecting is unknown;
            if (sessionStart) previousRoom = -1;

            // Rooms that aren't in the ROM (title screen, menus) are ignored, keeping the last real room; a goal game, the copy of it
            // beside the room Kirby came from (RomMap.SplitGoalRooms());
            room = map.ResolveRoom(room, previousRoom);
            if (!map.Rooms.ContainsKey(room)) return;

            bool changed = progress.EnterRoom(map, room, previousRoom);
            previousRoom = room;

            if (changed) SaveProgress();

            // Every room change brings the camera back to Kirby, even if the view was moved by hand;
            mapView.Follow = true;
            followButton.Checked = true;
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
            revealAllItem.Checked = false;
            SaveProgress();
            mapView.SetProgress(progress);
            mapView.FitToDiscovered();
            UpdateDiscoveryLabel();
        }

        /* SetRevealAll(); Shows (or hides again) every room and mirror of the seed, for players who want to go straight
         * to the bosses. It's remembered with the progress of the ROM; */
        void SetRevealAll(bool reveal) {
            if (progress == null) {
                revealAllItem.Checked = false;
                return;
            }

            if (reveal && !progress.RevealAll) {
                DialogResult answer = MessageBox.Show(this, "Reveal every room and mirror of this seed? Rooms and mirrors you haven't found yet are shown faded.",
                                                      Text, MessageBoxButtons.YesNo, MessageBoxIcon.Question);

                if (answer != DialogResult.Yes) {
                    revealAllItem.Checked = false;
                    return;
                }
            }

            progress.RevealAll = reveal;
            SaveProgress();
            mapView.RefreshProgress();
        }

        // ExportImage(); Saves the discovered map, or the entire map, as a PNG image;
        void ExportImage(bool entireMap) {
            if (map == null) return;

            using SaveFileDialog dialog = new SaveFileDialog {
                Title = entireMap ? "Save the entire map" : "Save the discovered map",
                Filter = "PNG image (*.png)|*.png",
                FileName = Path.GetFileNameWithoutExtension(romPath) + (entireMap ? " - Entire Map.png" : " - Discovered Map.png"),
                InitialDirectory = Path.GetDirectoryName(romPath),
            };

            if (dialog.ShowDialog(this) != DialogResult.OK) return;

            try {
                Cursor = Cursors.WaitCursor;
                using Bitmap image = mapView.RenderImage(entireMap);

                if (image == null) {
                    MessageBox.Show(this, "No room has been discovered yet.", Text, MessageBoxButtons.OK, MessageBoxIcon.Information);
                    return;
                }

                image.Save(dialog.FileName, System.Drawing.Imaging.ImageFormat.Png);
            }
            catch (Exception exception) when (exception is IOException || exception is UnauthorizedAccessException || exception is System.Runtime.InteropServices.ExternalException) {
                MessageBox.Show(this, $"The image couldn't be saved:\n{exception.Message}", Text, MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            finally {
                Cursor = Cursors.Default;
            }
        }

        /* OnScriptVersion(); An outdated copy of the emulator script (e.g. one copied elsewhere before an update) still tracks rooms,
         * but may miss fixes; The window says which file to load instead; */
        void OnScriptVersion(int version) {
            if (version >= TrackerServer.ScriptVersion) return;

            string script = Path.Combine(AppContext.BaseDirectory, "Lua", "katam_tracker.lua");
            string message = $"mGBA is running an outdated copy of the tracker script. Load this one instead: {script}";

            connectionLabel.Text = message;
            connectionLabel.ForeColor = Color.Firebrick;
            mapView.ShowHint(message, 12);
        }

        /* OnMapsReported(); The loaded save's area maps changed (a map was found, or another save was loaded). The rooms they
         * reveal aren't saved with the progress: they follow whatever save is loaded, so switching saves never reveals areas
         * the player hasn't explored in that save; */
        void OnMapsReported(int maps) {
            mapFlags = maps;
            UpdateMapRevealedRooms();
        }

        void UpdateMapRevealedRooms() {
            mapView.MapRevealedRooms = mapsItem.Checked ? AreaMaps.RoomsOf(mapFlags) : new HashSet<int>();
            UpdateDiscoveryLabel();
        }

        void SetConnected(bool connected) {
            isConnected = connected;
            if (!connected) lastReportedRoom = -1;

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
            string maps = mapsItem.Checked ? $"   Maps: {AreaMaps.CountOf(mapFlags)}" : "";
            discoveryLabel.Text = $"Rooms: {progress.VisitedRooms.Count}/{map.Rooms.Count}   Exits: {progress.FoundDoors.Count}/{doors}{maps}   Zoom: {mapView.Zoom:P0}";
        }

        void ShowHelp() {
            MessageBox.Show(this,
                "1. Open the randomized ROM you're playing (File > Open ROM...).\n" +
                "2. In mGBA 0.10 or newer: Tools > Scripting... > File > Load script, and pick Lua/katam_tracker.lua.\n" +
                "3. Play! Rooms appear as you enter them, and lines as you go through their mirrors.\n\n" +
                "Yellow rooms: every mirror used. Orange rooms: some mirrors left. The pulsing room is where Kirby is.\n" +
                "Dotted lines: Two-Way mirrors. Arrows: One-Way mirrors.\n\n" +
                "Mouse wheel: zoom.   Drag: move the map.   Hover: room preview.   Double click: center a room.\n" +
                "Space: follow Kirby on/off (it comes back on when Kirby changes rooms).   F: fit the discovered map.   +/-: zoom.\n" +
                "M: minimalist mode (only the map; Esc leaves it).   Right click the map: options.\n" +
                "View > Dynamic Scaling keeps rooms and lines readable when zoomed out.\n" +
                "View > Show on Map draws the bosses, mini-bosses, switches, shards, chests, items... of every room, each kind on its own.\n" +
                "View > Show Chest Contents shows what chests hold instead of the chest (spoilers).\n" +
                "View > Reveal Areas of Found Maps shows the rooms of every area whose map the loaded save holds.\n\n" +
                "View > Reveal Entire Map shows the whole seed (spoilers). File > Export Map Image saves the map as a PNG.\n\n" +
                "Progress is saved next to the ROM as <rom>.tracker.json.",
                "How to use", MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

        // ReadRecentRoms(); The most recently opened ROMs, newest first (older versions only saved the last one);
        static List<string> ReadRecentRoms() {
            try {
                if (File.Exists(RecentRomsFile)) return File.ReadAllLines(RecentRomsFile).Select(line => line.Trim()).Where(line => line.Length > 0).ToList();
                if (File.Exists(LastRomFile)) return new List<string> { File.ReadAllText(LastRomFile).Trim() };
            }
            catch (Exception exception) when (exception is IOException || exception is UnauthorizedAccessException) { }

            return new List<string>();
        }

        static void SaveRecentRoms(List<string> roms) {
            try {
                Directory.CreateDirectory(SettingsFolder);
                File.WriteAllLines(RecentRomsFile, roms.Take(MaxRecentRoms));
            }
            catch (Exception exception) when (exception is IOException || exception is UnauthorizedAccessException) { }
        }

        static void AddRecentRom(string path) {
            List<string> roms = ReadRecentRoms();
            roms.RemoveAll(rom => string.Equals(rom, path, StringComparison.OrdinalIgnoreCase));
            roms.Insert(0, path);
            SaveRecentRoms(roms);
        }

        static void RemoveRecentRom(string path) {
            List<string> roms = ReadRecentRoms();

            if (roms.RemoveAll(rom => string.Equals(rom, path, StringComparison.OrdinalIgnoreCase)) > 0) SaveRecentRoms(roms);
        }

        // UpdateRecentMenu(); Lists the recent ROMs under File > Open Recent;
        void UpdateRecentMenu() {
            recentMenu.DropDownItems.Clear();
            List<string> roms = ReadRecentRoms();

            for (int i = 0; i < roms.Count; i++) {
                string rom = roms[i];
                ToolStripMenuItem item = new ToolStripMenuItem($"&{(i + 1) % 10} {Path.GetFileName(rom)}") { ToolTipText = rom };

                item.Click += (s, e) => {
                    if (File.Exists(rom)) LoadRom(rom);
                    else {
                        RemoveRecentRom(rom);
                        MessageBox.Show(this, $"The ROM wasn't found:\n{rom}", Text, MessageBoxButtons.OK, MessageBoxIcon.Warning);
                    }
                };

                recentMenu.DropDownItems.Add(item);
            }

            if (roms.Count == 0) {
                recentMenu.DropDownItems.Add(new ToolStripMenuItem("(No recent ROMs)") { Enabled = false });
                return;
            }

            recentMenu.DropDownItems.Add(new ToolStripSeparator());
            recentMenu.DropDownItems.Add("Clear Recent ROMs", null, (s, e) => SaveRecentRoms(new List<string>()));
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
