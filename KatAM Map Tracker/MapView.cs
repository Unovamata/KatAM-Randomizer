using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.Linq;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace KatAMMapTracker {
    /* MapView; Draws the discovered part of the map like the in-game pause screen map, with the game's own graphics;
     * • Room squares: yellow once every mirror in the room was used, orange while some weren't, pulsing where Kirby is.
     *   Hubs are big squares and Boss/Goal game rooms big star squares.
     * • Lines join the rooms through the mirrors Kirby used, with 0°, 45° and 90° segments and a few colors so they don't blend.
     *   One-Way mirrors are drawn as arrows, like the game does.
     * • Room clusters are tinted, like the colored regions of the in-game map.
     * • Everything is drawn as pixel art (nearest neighbor), never blurred.
     * • The camera eases its pans and zooms. Following Kirby keeps it on the room he's in.
     * • Reveal All (spoiler) shows the whole map, with what wasn't discovered yet faded.
     * • Hovering a room shows its preview, with its background, and its mirrors (green: used, yellow: not yet).
     * • Mouse wheel zooms around the cursor, dragging pans, double clicking a room centers it; */
    public sealed class MapView : Control {
        const double MinZoom = 0.25, MaxZoom = 8, ZoomStep = 1.2;
        const int AreaRadius = 56, AreaCell = 2, PreviewMaxWidth = 420, PreviewMaxHeight = 280;
        const double PanDuration = 0.45, ZoomDuration = 0.3; // Seconds;
        const float FadedOpacity = 0.35f;

        /* Dynamic scaling: room squares grow by a number of map pixels on every side;
         * • Zoomed in, they grow ZoomedInGrowth pixels, so they read well, while lines keep their in-game thickness to keep
         *   the paths easy to follow (lines are routed 16 pixels away from the squares, so they keep a gap).
         * • Zoomed out, they grow until a small room is at least MinRoomScreenSize screen pixels, so the map can be previewed,
         *   and lines get thicker with them.
         * Small rooms grow up to MaxSmallRoomGrowth (40 pixel squares on the 48 pixel grid) and big rooms up to MaxBigRoomGrowth,
         * so neighbors never touch; */
        const double ZoomedInGrowth = 4, MinRoomScreenSize = 20, MaxSmallRoomGrowth = 12, MaxBigRoomGrowth = 6, LineGrowthShare = 1.0 / 24;

        // Soft colors for the regions behind the room clusters, like the in-game map backdrop;
        static readonly Color[] RegionColors = {
            Color.FromArgb(70, 76, 120), Color.FromArgb(64, 112, 72), Color.FromArgb(120, 92, 60), Color.FromArgb(104, 70, 116),
            Color.FromArgb(56, 108, 116), Color.FromArgb(124, 66, 66), Color.FromArgb(112, 112, 64), Color.FromArgb(66, 82, 128),
            Color.FromArgb(76, 116, 104), Color.FromArgb(128, 104, 48),
        };

        // Scene; What a drawing pass shows: the discovered map, or everything (faded or not where it wasn't discovered);
        readonly record struct Scene(bool ShowAll, bool FadeUndiscovered, double RoomGrowth = 0, double LineScale = 1);

        RomMap map;
        MapPlan plan;
        Dictionary<int, RoomPlacement> layout;
        Dictionary<int, int> regionOf = new Dictionary<int, int>();
        Dictionary<string, int> lineColors = new Dictionary<string, int>();
        MapProgress progress;
        MapGraphics graphics;

        // Camera: the map position at the center of the view, and the zoom;
        double centerX, centerY, zoom = 2;

        // Smooth pan: from where the view was to where it's going (ease-out cubic);
        double panFromX, panFromY, panToX, panToY;
        DateTime panStart;
        bool isPanning;

        // Smooth zoom: around a map point that stays under the same screen point (anchored), or around the center;
        double zoomFrom, zoomTo, zoomAnchorX, zoomAnchorY, zoomAnchorScreenX, zoomAnchorScreenY;
        DateTime zoomStart;
        bool isZooming, isZoomAnchored;

        readonly Dictionary<(Sprite, bool), Bitmap> spriteBitmaps = new Dictionary<(Sprite, bool), Bitmap>();
        readonly Dictionary<int, Bitmap> previews = new Dictionary<int, Bitmap>();
        readonly Queue<int> previewOrder = new Queue<int>();
        readonly Timer pulseTimer = new Timer { Interval = 16 };

        Bitmap areaLayer;
        RectangleF areaLayerBounds;
        (int rooms, bool all) areaLayerKey = (-1, false);

        Point dragStart, mouse;
        bool isDragging, hasDragged;
        int hoveredRoom = -1, pulseFrame, pulseTicks;
        DateTime lastPulse = DateTime.Now;

        public bool Follow { get; set; } = true;

        bool dynamicScaling = true;

        bool showEntities;

        // Shows the in-game sprites of the important objects of every room (bosses, items, treasure...); Off by default;
        public bool ShowEntities {
            get => showEntities;
            set { showEntities = value; Invalidate(); }
        }

        bool showChestContents;

        // Chests show what they hold (spoiler) instead of the chest sprite;
        public bool ShowChestContents {
            get => showChestContents;
            set { showChestContents = value; Invalidate(); }
        }

        HashSet<int> mapRevealedRooms = new HashSet<int>();

        // Rooms revealed by the area maps of the player's save, shown faded like rooms not visited yet;
        public HashSet<int> MapRevealedRooms {
            get => mapRevealedRooms;
            set { mapRevealedRooms = value ?? new HashSet<int>(); areaLayerKey = (-1, false); Invalidate(); }
        }

        /* Entity icons are drawn inside the room square, all the same size: half a small room's square, in a 2x2 grid of up to MaxEntityIcons
         * (a single one in the middle). They're pixel art: nearest neighbor, never smoothed; */
        const int MaxEntityIcons = 4;

        Dictionary<int, List<MapEntity>> roomEntities = new Dictionary<int, List<MapEntity>>();
        readonly Dictionary<EntityIcon, Sprite> entitySprites = new Dictionary<EntityIcon, Sprite>();

        // A short message drawn over the map (e.g. how to leave the minimalist mode), until hintUntil;
        string hint;
        DateTime hintUntil;

        // Zoomed out, rooms and lines are drawn bigger so they stay readable (see MinRoomScreenSize);
        public bool DynamicScaling {
            get => dynamicScaling;
            set { dynamicScaling = value; Invalidate(); }
        }

        public event Action FollowDisabledByUser;

        // Raised when the zoom changes;
        public event Action ViewChanged;

        public MapView() {
            DoubleBuffered = true;
            ResizeRedraw = true;
            BackColor = Color.FromArgb(28, 28, 36);
            SetStyle(ControlStyles.AllPaintingInWmPaint | ControlStyles.UserPaint | ControlStyles.OptimizedDoubleBuffer, true);

            pulseTimer.Tick += (s, e) => {
                AdvanceCamera();
                AdvancePulse();

                if (hint != null && DateTime.Now >= hintUntil) Invalidate();
            };
            pulseTimer.Start();
        }

        public double Zoom => isZooming ? zoomTo : zoom;

        // Map position drawn at the top left corner;
        double OffsetX => centerX - Width / 2.0 / zoom;
        double OffsetY => centerY - Height / 2.0 / zoom;

        bool RevealAll => progress?.RevealAll == true;

        Scene ViewScene => new Scene(RevealAll, true, RoomGrowth, 1 + Math.Min(ZoomedOutGrowth, MaxSmallRoomGrowth) * LineGrowthShare);

        // RoomGrowth(); How many map pixels room squares grow on every side at the current zoom;
        double RoomGrowth => DynamicScaling ? Math.Max(ZoomedInGrowth, ZoomedOutGrowth) : 0;

        // ZoomedOutGrowth(); Growth that keeps small rooms (16 map pixels) at least MinRoomScreenSize on screen; 0 when zoomed in;
        double ZoomedOutGrowth => DynamicScaling ? Math.Max(0, (MinRoomScreenSize / zoom - MapGraphics.TileSize * 2) / 2) : 0;

        // RoomBounds(); Where a room square is drawn, grown around its center (up to MaxSmallRoomGrowth or MaxBigRoomGrowth);
        RectangleF RoomBounds(RoomPlacement placement, double growth) {
            bool isBig = Math.Max(placement.Width, placement.Height) > MapGraphics.TileSize * 2;
            growth = Math.Clamp(growth, 0, isBig ? MaxBigRoomGrowth : MaxSmallRoomGrowth);

            double width = placement.Width + growth * 2, height = placement.Height + growth * 2;
            return new RectangleF((float)(placement.CenterX - width / 2), (float)(placement.CenterY - height / 2), (float)width, (float)height);
        }

        /* SetMap(); Loads a map with its plan (room positions, clusters and connection paths); */
        public void SetMap(RomMap romMap, MapPlan mapPlan, MapProgress mapProgress) {
            ClearImages();

            map = romMap;
            plan = mapPlan;
            layout = mapPlan?.Layout;
            Dictionary<int, int> clusters = mapPlan?.Clusters;
            progress = mapProgress;
            graphics = map == null ? null : new MapGraphics(map);
            lineColors = map == null ? new Dictionary<string, int>() : MapRoutes.AssignLineColors(map);
            roomEntities = map == null ? new Dictionary<int, List<MapEntity>>() : map.Rooms.ToDictionary(kvp => kvp.Key, kvp => MapEntities.EntitiesOf(kvp.Value));
            entitySprites.Clear();

            // Clusters are numbered in order so neighbor clusters tend to get different region colors;
            regionOf = new Dictionary<int, int>();
            Dictionary<int, int> regionIndex = new Dictionary<int, int>();

            foreach ((int room, int cluster) in (clusters ?? new Dictionary<int, int>()).OrderBy(kvp => kvp.Key)) {
                if (!regionIndex.ContainsKey(cluster)) regionIndex[cluster] = regionIndex.Count;
                regionOf[room] = regionIndex[cluster];
            }

            StopCamera();

            if (progress != null && layout != null && layout.ContainsKey(progress.CurrentRoom)) CenterOn(progress.CurrentRoom, false);
            else FitToDiscovered(false);

            Invalidate();
        }

        public void SetProgress(MapProgress mapProgress) {
            progress = mapProgress;
            areaLayerKey = (-1, false);
            Invalidate();
        }

        // RefreshProgress(); Redraws after the progress changed outside of room changes (Reveal All);
        public void RefreshProgress() {
            areaLayerKey = (-1, false);
            Invalidate();
        }

        // OnRoomChanged(); Called when Kirby enters a room, to follow him;
        public void OnRoomChanged() {
            if (Follow && progress != null && layout != null && layout.ContainsKey(progress.CurrentRoom)) CenterOn(progress.CurrentRoom);

            Invalidate();
        }

        /* CenterOn(); Centers the view on a room. With animate, the view eases there (ease-out cubic) instead of jumping; */
        public void CenterOn(int room, bool animate = true) {
            RoomPlacement placement = layout[room];
            PanTo(placement.CenterX, placement.CenterY, animate);
        }

        void PanTo(double x, double y, bool animate) {
            if (!animate) {
                isPanning = false;
                centerX = x;
                centerY = y;
                Invalidate();
                return;
            }

            panFromX = centerX; panFromY = centerY;
            panToX = x; panToY = y;
            panStart = DateTime.Now;
            isPanning = true;
        }

        void StopCamera() {
            isPanning = false;

            if (isZooming) zoom = zoomTo;

            isZooming = false;
        }

        // AdvanceCamera(); Moves the view along the current smooth pan and zoom, and keeps it on Kirby while following him;
        void AdvanceCamera() {
            if (map == null || layout == null) return;

            // Following never loses Kirby: if the view isn't on his room (resizes, a new layout...), it goes back to it;
            if (Follow && !isPanning && !isDragging && progress != null && layout.TryGetValue(progress.CurrentRoom, out RoomPlacement current) &&
                (Math.Abs(centerX - current.CenterX) > 0.5 || Math.Abs(centerY - current.CenterY) > 0.5)) {
                PanTo(current.CenterX, current.CenterY, true);
            }

            if (!isPanning && !isZooming) return;

            DateTime now = DateTime.Now;

            if (isZooming) {
                double t = Math.Min(1, (now - zoomStart).TotalSeconds / ZoomDuration);
                double eased = 1 - Math.Pow(1 - t, 3);

                // Geometric interpolation, so every step of the zoom feels the same;
                zoom = zoomFrom * Math.Pow(zoomTo / zoomFrom, eased);

                if (isZoomAnchored && !isPanning) {
                    centerX = zoomAnchorX - zoomAnchorScreenX / zoom;
                    centerY = zoomAnchorY - zoomAnchorScreenY / zoom;
                }

                if (t >= 1) isZooming = false;

                ViewChanged?.Invoke();
            }

            if (isPanning) {
                double t = Math.Min(1, (now - panStart).TotalSeconds / PanDuration);
                double eased = 1 - Math.Pow(1 - t, 3);

                centerX = panFromX + (panToX - panFromX) * eased;
                centerY = panFromY + (panToY - panFromY) * eased;

                if (t >= 1) isPanning = false;
            }

            Invalidate();
        }

        /* FitToDiscovered(); Shows every room on the map (the discovered ones, or all of them with Reveal All); */
        public void FitToDiscovered() {
            StopFollowing();
            FitToDiscovered(true);
        }

        // StopFollowing(); The user moved the view away from Kirby; Following comes back when he enters another room;
        void StopFollowing() {
            if (!Follow) return;

            Follow = false;
            FollowDisabledByUser?.Invoke();
        }

        void FitToDiscovered(bool animate) {
            if (layout == null) return;

            List<RoomPlacement> rooms = ShownRooms(ViewScene).Select(r => layout[r]).ToList();

            // Nothing discovered yet; Central Circle is where the adventure starts;
            if (rooms.Count == 0 && layout.TryGetValue(RomMap.CentralCircle, out RoomPlacement hub)) rooms.Add(hub);
            if (rooms.Count == 0) return;

            double minX = rooms.Min(r => r.X), minY = rooms.Min(r => r.Y),
                   maxX = rooms.Max(r => r.X + r.Width), maxY = rooms.Max(r => r.Y + r.Height);
            double margin = 48;
            double target = Math.Clamp(Math.Min(Width / (maxX - minX + margin * 2), Height / (maxY - minY + margin * 2)), MinZoom, 4);

            if (!animate || Width == 0) {
                StopCamera();
                zoom = target;
                PanTo((minX + maxX) / 2, (minY + maxY) / 2, false);
                ViewChanged?.Invoke();
                return;
            }

            ZoomTo(target, null);
            PanTo((minX + maxX) / 2, (minY + maxY) / 2, true);
        }

        public void ZoomBy(double factor) => ZoomAt(factor, null);

        // ZoomAt(); Eases the zoom; Around the cursor, unless the view follows Kirby (or pans), which zooms around the center;
        void ZoomAt(double factor, Point? screen) {
            ZoomTo(Math.Clamp(Zoom * factor, MinZoom, MaxZoom), screen);
        }

        void ZoomTo(double target, Point? screen) {
            isZoomAnchored = screen.HasValue && !Follow && !isPanning;

            if (isZoomAnchored) {
                zoomAnchorScreenX = screen.Value.X - Width / 2.0;
                zoomAnchorScreenY = screen.Value.Y - Height / 2.0;
                zoomAnchorX = centerX + zoomAnchorScreenX / zoom;
                zoomAnchorY = centerY + zoomAnchorScreenY / zoom;
            }

            zoomFrom = zoom;
            zoomTo = target;
            zoomStart = DateTime.Now;
            isZooming = true;
            ViewChanged?.Invoke();
        }

        // AdvancePulse(); Plays the in-game square pulse on the current room, with the game's frame durations (1/60 s);
        void AdvancePulse() {
            if (graphics == null || progress == null || !layout.ContainsKey(progress.CurrentRoom)) return;

            DateTime now = DateTime.Now;
            pulseTicks += (int)Math.Round((now - lastPulse).TotalMilliseconds * 60 / 1000);
            lastPulse = now;

            bool changed = false;

            while (pulseTicks >= graphics.PulseDuration(pulseFrame)) {
                pulseTicks -= graphics.PulseDuration(pulseFrame);
                pulseFrame = (pulseFrame + 1) % graphics.PulseFrameCount;
                changed = true;
            }

            if (!changed) return;

            RectangleF bounds = RoomBounds(layout[progress.CurrentRoom], RoomGrowth);

            // The current room's outline reaches past the square;
            if (ShowEntities) bounds = RectangleF.Inflate(bounds, 2, 2);
            PointF topLeft = ToScreen(bounds.X, bounds.Y);
            Invalidate(Rectangle.Inflate(Rectangle.Round(new RectangleF(topLeft, new SizeF((float)(bounds.Width * zoom), (float)(bounds.Height * zoom)))), 4, 4));
        }

        // ShowHint(); Shows a short message at the bottom of the map for a few seconds;
        public void ShowHint(string text, double seconds = 4) {
            hint = text;
            hintUntil = DateTime.Now.AddSeconds(seconds);
            Invalidate();
        }

        void DrawHint(Graphics g) {
            if (hint == null) return;

            if (DateTime.Now >= hintUntil) {
                hint = null;
                return;
            }

            Size size = TextRenderer.MeasureText(hint, Font);
            Rectangle box = new Rectangle((Width - size.Width) / 2 - 10, Height - size.Height - 28, size.Width + 20, size.Height + 10);

            using (SolidBrush background = new SolidBrush(Color.FromArgb(200, 16, 16, 24))) g.FillRectangle(background, box);
            TextRenderer.DrawText(g, hint, Font, box, Color.White, TextFormatFlags.HorizontalCenter | TextFormatFlags.VerticalCenter);
        }

        PointF ToScreen(double x, double y) => new PointF((float)((x - OffsetX) * zoom), (float)((y - OffsetY) * zoom));

        //////////////////////////////////////////////////////////////////////////////////////////////

        // ShownRooms(); Rooms a scene draws: the visited ones, or all of them;
        IEnumerable<int> ShownRooms(Scene scene) =>
            (scene.ShowAll ? (IEnumerable<int>)map.Rooms.Keys : progress.VisitedRooms.Union(mapRevealedRooms)).Where(layout.ContainsKey);

        bool IsShown(Scene scene, int room) => scene.ShowAll || progress.VisitedRooms.Contains(room) || mapRevealedRooms.Contains(room);

        // IsDoorShown(); Found doors, every door with Reveal All, and the doors between rooms an area map reveals;
        bool IsDoorShown(Scene scene, DoorInfo door) =>
            scene.ShowAll || progress.IsDoorFound(door) || (mapRevealedRooms.Contains(door.Room) && IsShown(scene, door.Destination));

        bool IsFaded(Scene scene, int room) => scene.FadeUndiscovered && !progress.VisitedRooms.Contains(room);

        bool IsFaded(Scene scene, DoorInfo door) => scene.FadeUndiscovered && !progress.IsDoorFound(door);

        protected override void OnPaint(PaintEventArgs e) {
            base.OnPaint(e);

            Graphics g = e.Graphics;

            if (map == null || layout == null || progress == null) {
                TextRenderer.DrawText(g, "Open a randomized ROM to load its map (File > Open ROM...).", Font,
                                      ClientRectangle, Color.Gainsboro, TextFormatFlags.HorizontalCenter | TextFormatFlags.VerticalCenter);
                return;
            }

            GraphicsState state = g.Save();

            // Everything is drawn in map pixels; The transform does the zoom and pan;
            g.Transform = new Matrix((float)zoom, 0, 0, (float)zoom, (float)(-OffsetX * zoom), (float)(-OffsetY * zoom));
            DrawScene(g, ViewScene, true);
            g.Restore(state);

            if (hoveredRoom >= 0 && !isDragging) DrawPreview(g, hoveredRoom);

            DrawHint(g);
        }

        /* DrawScene(); Draws the regions, lines and rooms of a scene in map pixels, as pixel art; */
        void DrawScene(Graphics g, Scene scene, bool useAreaCache) {
            g.PixelOffsetMode = PixelOffsetMode.Half;
            g.InterpolationMode = InterpolationMode.NearestNeighbor;
            g.SmoothingMode = SmoothingMode.None;

            DrawAreas(g, scene, useAreaCache);
            DrawConnections(g, scene);

            // Faded rooms first, so the discovered ones are drawn over them;
            foreach (int room in ShownRooms(scene).OrderBy(room => IsFaded(scene, room) ? 0 : 1)) DrawRoom(g, scene, room);

            if (ShowEntities) {
                foreach (int room in ShownRooms(scene).OrderBy(room => IsFaded(scene, room) ? 0 : 1)) DrawEntities(g, scene, room);
            }
        }

        /* DrawEntities(); The sprites of the important objects of a room (one per kind of sprite), inside its square, all the same
         * size: one in the middle, two side by side, three or four in a 2x2 grid. The current room gets a blinking outline; */
        void DrawEntities(Graphics g, Scene scene, int room) {
            if (!roomEntities.TryGetValue(room, out List<MapEntity> entities) || entities.Count == 0) return;

            List<EntityIcon> icons = entities.Select(IconOf).Distinct().Take(MaxEntityIcons).ToList();
            RectangleF square = RoomBounds(layout[room], scene.RoomGrowth);
            bool faded = IsFaded(scene, room);
            float screenScale = g.Transform.Elements[0];

            GraphicsState state = g.Save();
            g.InterpolationMode = InterpolationMode.NearestNeighbor;
            g.PixelOffsetMode = PixelOffsetMode.Half;

            int columns = icons.Count == 1 ? 1 : 2, rows = icons.Count > 2 ? 2 : 1;
            // The same size in every room: half a small room's square;
            float cell = (MapGraphics.TileSize * 2 + 2 * (float)Math.Clamp(scene.RoomGrowth, 0, MaxSmallRoomGrowth)) / 2;
            float left = square.X + square.Width / 2 - columns * cell / 2, top = square.Y + square.Height / 2 - rows * cell / 2;

            for (int i = 0; i < icons.Count; i++) {
                DrawPixelArt(g, GetEntitySprite(icons[i]), left + (i % columns + 0.5f) * cell, top + (i / columns + 0.5f) * cell, cell, screenScale, faded);
            }

            if (room == progress.CurrentRoom && pulseFrame % 2 == 0) {
                using Pen pen = new Pen(Color.White, 2 / screenScale);
                RectangleF outline = RectangleF.Inflate(square, 1, 1);
                g.DrawRectangle(pen, outline.X, outline.Y, outline.Width, outline.Height);
            }

            g.Restore(state);
        }

        // IconOf(); The sprite of an entity: what a chest holds when chest contents are shown, else its own sprite;
        EntityIcon IconOf(MapEntity entity) => ShowChestContents && entity.ContentIcon != null ? entity.ContentIcon.Value : entity.Icon;

        /* DrawPixelArt(); Draws a sprite centered at x, y, fitting exactly in box map pixels, so every icon is the same size.
         * Nearest neighbor only, never smoothed: it stays pixel art; */
        void DrawPixelArt(Graphics g, Sprite sprite, float x, float y, float box, float screenScale, bool faded) {
            if (sprite == null) return;

            float scale = box / Math.Max(sprite.Width, sprite.Height);
            float width = sprite.Width * scale, height = sprite.Height * scale;

            g.DrawImage(GetBitmap(sprite, faded), new RectangleF(x - width / 2, y - height / 2, width, height));
        }

        Sprite GetEntitySprite(EntityIcon icon) {
            if (!entitySprites.TryGetValue(icon, out Sprite sprite)) entitySprites[icon] = sprite = MapEntities.Render(map, icon);

            return sprite;
        }

        void DrawRoom(Graphics g, Scene scene, int room) {
            RoomPlacement placement = layout[room];
            RoomInfo info = map.Rooms[room];

            RoomStyle style = room == progress.CurrentRoom ? RoomStyle.Current
                : info.Doors.All(progress.IsDoorFound) ? RoomStyle.Explored
                : RoomStyle.Unexplored;

            Sprite sprite = graphics.Room(MapGraphics.ShapeOf(info), style, pulseFrame);
            RectangleF bounds = RoomBounds(placement, scene.RoomGrowth);
            g.DrawImage(GetBitmap(sprite, IsFaded(scene, room)), bounds);

            if (room == hoveredRoom) {
                using Pen pen = new Pen(Color.White, (float)(1.5 / zoom));
                RectangleF outline = RectangleF.Inflate(RoomBounds(placement, scene.RoomGrowth), 1, 1);
                g.DrawRectangle(pen, outline.X, outline.Y, outline.Width, outline.Height);
            }
        }

        void DrawConnections(Graphics g, Scene scene) {
            foreach (int room in ShownRooms(scene)) {
                foreach (DoorInfo door in map.Rooms[room].Doors) {
                    if (!IsDoorShown(scene, door) || !layout.ContainsKey(door.Destination)) continue;

                    bool isTwoWay = door.DoorBack != null;

                    // Two-Way mirrors are drawn once; The found side is drawn, so the line isn't faded when only one was used;
                    if (isTwoWay && (scene.ShowAll || progress.IsDoorFound(door.DoorBack))) {
                        bool backDrawnInstead = IsFaded(scene, door) == IsFaded(scene, door.DoorBack) ? door.Room > door.Destination : IsFaded(scene, door);

                        if (backDrawnInstead) continue;
                    }

                    LineSprites lines = graphics.Lines(lineColors.GetValueOrDefault(MapRoutes.ConnectionKey(door.Room, door.Destination)));
                    bool faded = IsFaded(scene, door);

                    foreach (RouteSegment segment in plan.Segments(door.Room, door.Destination, !isTwoWay)) DrawSegment(g, segment, lines, faded, (float)scene.LineScale);
                }
            }
        }

        /* DrawSegment(); Fills a segment with the in-game line (or arrow) tiles, scaled by the line scale;
         * Straight segments use the in-game pieces aligned to the 8 pixel grid. Diagonals rotate the horizontal pieces 45°; */
        void DrawSegment(Graphics g, RouteSegment segment, LineSprites lines, bool faded, float scale) {
            float half = 8 * scale, thickness = 16 * scale;

            if (!segment.IsDiagonal) {
                Sprite sprite = !segment.IsOneWay
                    ? (segment.IsHorizontal ? lines.Horizontal : lines.Vertical)
                    : segment.IsHorizontal ? (segment.DirectionX > 0 ? lines.ArrowRight : lines.ArrowLeft)
                    : (segment.DirectionY > 0 ? lines.ArrowDown : lines.ArrowUp);

                double left = Math.Min(segment.StartX, segment.EndX), top = Math.Min(segment.StartY, segment.EndY);
                RectangleF area = segment.IsHorizontal
                    ? new RectangleF((float)left, (float)segment.StartY - half, (float)Math.Abs(segment.EndX - segment.StartX), thickness)
                    : new RectangleF((float)segment.StartX - half, (float)top, thickness, (float)Math.Abs(segment.EndY - segment.StartY));

                using TextureBrush brush = new TextureBrush(GetBitmap(sprite, faded), WrapMode.Tile) {
                    Transform = new Matrix(scale, 0, 0, scale, area.X, area.Y),
                };
                g.FillRectangle(brush, area);
                return;
            }

            // Diagonal: the horizontal piece (or the arrow pointing right) rotated along the segment;
            Sprite piece = segment.IsOneWay ? lines.ArrowRight : lines.Horizontal;
            GraphicsState state = g.Save();

            g.TranslateTransform((float)segment.StartX, (float)segment.StartY);
            g.RotateTransform((float)segment.Angle);

            using (TextureBrush brush = new TextureBrush(GetBitmap(piece, faded), WrapMode.Tile) { Transform = new Matrix(scale, 0, 0, scale, 0, -half) }) {
                g.FillRectangle(brush, 0, -half, (float)segment.Length, thickness);
            }

            g.Restore(state);
        }

        // DrawAreas(); Colored regions behind the rooms, by cluster, like the in-game map backdrop;
        void DrawAreas(Graphics g, Scene scene, bool useCache) {
            List<int> rooms = ShownRooms(scene).ToList();

            if (rooms.Count == 0) return;

            if (!useCache) {
                (Bitmap layer, RectangleF bounds) = BuildAreaLayer(rooms);
                using (layer) g.DrawImage(layer, bounds);
                return;
            }

            if (areaLayer == null || areaLayerKey != (rooms.Count, scene.ShowAll)) {
                areaLayer?.Dispose();
                (areaLayer, areaLayerBounds) = BuildAreaLayer(rooms);
                areaLayerKey = (rooms.Count, scene.ShowAll);
            }

            g.DrawImage(areaLayer, areaLayerBounds);
        }

        /* BuildAreaLayer(); Every room stamps its region color in a circle around it, in 2 pixel cells.
         * The nearest room wins, which splits the regions like a Voronoi diagram. Cells on the edge of a region get
         * a darker outline, so the regions look crisp, like the in-game map, instead of blurred; */
        (Bitmap, RectangleF) BuildAreaLayer(List<int> rooms) {
            double minX = rooms.Min(r => layout[r].CenterX) - AreaRadius - AreaCell, minY = rooms.Min(r => layout[r].CenterY) - AreaRadius - AreaCell,
                   maxX = rooms.Max(r => layout[r].CenterX) + AreaRadius + AreaCell, maxY = rooms.Max(r => layout[r].CenterY) + AreaRadius + AreaCell;

            int columns = (int)Math.Ceiling((maxX - minX) / AreaCell) + 1, rows = (int)Math.Ceiling((maxY - minY) / AreaCell) + 1;
            double[] distance = new double[columns * rows];
            int[] region = new int[columns * rows];

            Array.Fill(distance, double.MaxValue);
            Array.Fill(region, -1);

            foreach (int room in rooms) {
                RoomPlacement p = layout[room];
                int regionColor = regionOf.GetValueOrDefault(room) % RegionColors.Length;
                int centerColumn = (int)((p.CenterX - minX) / AreaCell), centerRow = (int)((p.CenterY - minY) / AreaCell);
                int reach = AreaRadius / AreaCell + 1;

                for (int row = Math.Max(0, centerRow - reach); row <= Math.Min(rows - 1, centerRow + reach); row++) {
                    for (int column = Math.Max(0, centerColumn - reach); column <= Math.Min(columns - 1, centerColumn + reach); column++) {
                        double dx = minX + (column + 0.5) * AreaCell - p.CenterX, dy = minY + (row + 0.5) * AreaCell - p.CenterY;
                        double d = dx * dx + dy * dy;

                        if (d > AreaRadius * AreaRadius || d >= distance[row * columns + column]) continue;

                        distance[row * columns + column] = d;
                        region[row * columns + column] = regionColor;
                    }
                }
            }

            int[] fill = RegionColors.Select(color => Color.FromArgb(150, color).ToArgb()).ToArray(),
                  outline = RegionColors.Select(color => Color.FromArgb(220, color.R * 3 / 5, color.G * 3 / 5, color.B * 3 / 5).ToArgb()).ToArray();
            int[] pixels = new int[columns * rows];

            for (int row = 0; row < rows; row++) {
                for (int column = 0; column < columns; column++) {
                    int index = row * columns + column, color = region[index];

                    if (color < 0) continue;

                    bool isEdge = column == 0 || row == 0 || column == columns - 1 || row == rows - 1 ||
                                  region[index - 1] != color || region[index + 1] != color ||
                                  region[index - columns] != color || region[index + columns] != color;

                    pixels[index] = isEdge ? outline[color] : fill[color];
                }
            }

            return (ToBitmap(new Sprite(columns, rows, pixels)), new RectangleF((float)minX, (float)minY, columns * AreaCell, rows * AreaCell));
        }

        // DrawPreview(); Shows the hovered room with its in-game graphics and background, and where its mirrors are;
        void DrawPreview(Graphics g, int room) {
            RoomInfo info = map.Rooms[room];
            Bitmap image = GetPreview(room);

            double scale = Math.Min(1, Math.Min((double)PreviewMaxWidth / image.Width, (double)PreviewMaxHeight / image.Height));
            int width = (int)(image.Width * scale), height = (int)(image.Height * scale);

            int found = info.Doors.Count(progress.IsDoorFound);
            string caption = $"Room 0x{room:X}   Mirrors used: {found}/{info.Doors.Count}" + (progress.VisitedRooms.Contains(room) ? "" : "   (not visited)");

            // What's in the room, with counts (e.g. "Cherry x3");
            if (ShowEntities && roomEntities.TryGetValue(room, out List<MapEntity> entities) && entities.Count > 0) {
                caption += "\n" + string.Join(", ", entities.GroupBy(entity => entity.Name).Select(group => group.Count() > 1 ? $"{group.Key} x{group.Count()}" : group.Key));
            }
            Size captionSize = TextRenderer.MeasureText(caption, Font);

            int boxWidth = Math.Max(width, captionSize.Width) + 12, boxHeight = height + captionSize.Height + 14;
            int x = mouse.X + 20, y = mouse.Y + 20;

            if (x + boxWidth > Width) x = Math.Max(0, mouse.X - boxWidth - 20);
            if (y + boxHeight > Height) y = Math.Max(0, mouse.Y - boxHeight - 20);

            using (SolidBrush background = new SolidBrush(Color.FromArgb(235, 16, 16, 24))) g.FillRectangle(background, x, y, boxWidth, boxHeight);
            g.DrawRectangle(Pens.Gold, x, y, boxWidth - 1, boxHeight - 1);
            TextRenderer.DrawText(g, caption, Font, new Point(x + 6, y + 4), Color.White);

            int imageX = x + 6, imageY = y + captionSize.Height + 8;

            // Pixel art, even when the preview is scaled down;
            g.InterpolationMode = InterpolationMode.NearestNeighbor;
            g.PixelOffsetMode = PixelOffsetMode.Half;
            g.DrawImage(image, imageX, imageY, width, height);
            g.PixelOffsetMode = PixelOffsetMode.Default;

            foreach (DoorInfo door in info.Doors) {
                float doorX = imageX + (float)(door.X * scale), doorY = imageY + (float)(door.Y * scale);
                using SolidBrush fill = new SolidBrush(progress.IsDoorFound(door) ? Color.LimeGreen : Color.Gold);
                g.FillRectangle(fill, doorX - 4, doorY - 4, 8, 8);
                g.DrawRectangle(Pens.Black, doorX - 4, doorY - 4, 8, 8);
            }
        }

        Bitmap GetPreview(int room) {
            if (previews.TryGetValue(room, out Bitmap cached)) return cached;

            // Only the last few previews are kept, as full size rooms are big;
            if (previewOrder.Count >= 12) {
                int oldest = previewOrder.Dequeue();
                previews[oldest].Dispose();
                previews.Remove(oldest);
            }

            int[] pixels = map.RenderRoom(room, 8, out int width, out int height);
            Bitmap bitmap = ToBitmap(new Sprite(width, height, pixels));
            previews[room] = bitmap;
            previewOrder.Enqueue(room);

            return bitmap;
        }

        // GetBitmap(); The bitmap of a sprite, faded (see-through) for what wasn't discovered yet;
        Bitmap GetBitmap(Sprite sprite, bool faded = false) {
            if (spriteBitmaps.TryGetValue((sprite, faded), out Bitmap cached)) return cached;

            Sprite source = sprite;

            if (faded) {
                int[] pixels = sprite.Pixels.Select(argb => (int)((uint)(((argb >> 24) & 0xFF) * FadedOpacity) << 24) | (argb & 0xFFFFFF)).ToArray();
                source = new Sprite(sprite.Width, sprite.Height, pixels);
            }

            Bitmap bitmap = ToBitmap(source);
            spriteBitmaps[(sprite, faded)] = bitmap;

            return bitmap;
        }

        static Bitmap ToBitmap(Sprite sprite) {
            Bitmap bitmap = new Bitmap(sprite.Width, sprite.Height, PixelFormat.Format32bppArgb);
            BitmapData data = bitmap.LockBits(new Rectangle(0, 0, sprite.Width, sprite.Height), ImageLockMode.WriteOnly, PixelFormat.Format32bppArgb);

            for (int y = 0; y < sprite.Height; y++) {
                Marshal.Copy(sprite.Pixels, y * sprite.Width, data.Scan0 + y * data.Stride, sprite.Width);
            }

            bitmap.UnlockBits(data);

            return bitmap;
        }

        /* RenderImage(); Draws the map into an image, to save it: the discovered map, or the entire map (spoilers).
         * Pixel art is scaled 2x (nearest neighbor) unless the image would be too big; */
        public Bitmap RenderImage(bool entireMap) {
            if (map == null || layout == null || progress == null) return null;

            Scene scene = new Scene(entireMap, false);
            List<int> rooms = ShownRooms(scene).ToList();

            if (rooms.Count == 0) return null;

            double margin = AreaRadius + 8;
            double minX = rooms.Min(r => layout[r].X) - margin, minY = rooms.Min(r => layout[r].Y) - margin,
                   maxX = rooms.Max(r => layout[r].X + layout[r].Width) + margin, maxY = rooms.Max(r => layout[r].Y + layout[r].Height) + margin;

            int scale = Math.Max(maxX - minX, maxY - minY) * 2 <= 6000 ? 2 : 1;
            Bitmap image = new Bitmap((int)Math.Ceiling(maxX - minX) * scale, (int)Math.Ceiling(maxY - minY) * scale, PixelFormat.Format32bppArgb);

            int previousHover = hoveredRoom;
            hoveredRoom = -1;

            using (Graphics g = Graphics.FromImage(image)) {
                g.Clear(BackColor);
                g.Transform = new Matrix(scale, 0, 0, scale, (float)(-minX * scale), (float)(-minY * scale));
                DrawScene(g, scene, false);
            }

            hoveredRoom = previousHover;

            return image;
        }

        void ClearImages() {
            foreach (Bitmap bitmap in spriteBitmaps.Values.Concat(previews.Values)) bitmap.Dispose();
            spriteBitmaps.Clear();
            previews.Clear();
            previewOrder.Clear();
            areaLayer?.Dispose();
            areaLayer = null;
            areaLayerKey = (-1, false);
        }

        int RoomAt(Point screen) {
            if (layout == null || progress == null) return -1;

            double mapX = OffsetX + screen.X / zoom, mapY = OffsetY + screen.Y / zoom;

            double growth = RoomGrowth;

            foreach (int room in ShownRooms(ViewScene)) {
                if (RoomBounds(layout[room], growth).Contains((float)mapX, (float)mapY)) return room;
            }

            return -1;
        }

        //////////////////////////////////////////////////////////////////////////////////////////////

        protected override void OnMouseWheel(MouseEventArgs e) {
            base.OnMouseWheel(e);
            ZoomAt(Math.Pow(ZoomStep, e.Delta / 120.0), e.Location);
        }

        protected override void OnMouseDown(MouseEventArgs e) {
            base.OnMouseDown(e);
            Focus();

            if (e.Button != MouseButtons.Left) return;

            isDragging = true;
            hasDragged = false;
            dragStart = e.Location;
        }

        protected override void OnMouseMove(MouseEventArgs e) {
            base.OnMouseMove(e);
            mouse = e.Location;

            if (isDragging) {
                int dx = e.X - dragStart.X, dy = e.Y - dragStart.Y;

                if (!hasDragged && Math.Abs(dx) + Math.Abs(dy) < 4) return;

                // Panning by hand stops following Kirby;
                if (!hasDragged) {
                    StopFollowing();
                    StopCamera();
                }

                hasDragged = true;
                centerX -= dx / zoom;
                centerY -= dy / zoom;
                dragStart = e.Location;
                Invalidate();
                return;
            }

            int room = RoomAt(e.Location);

            // The preview follows the cursor;
            if (room != hoveredRoom || room >= 0) Invalidate();

            hoveredRoom = room;
        }

        protected override void OnMouseLeave(EventArgs e) {
            base.OnMouseLeave(e);
            hoveredRoom = -1;
            Invalidate();
        }

        protected override void OnMouseUp(MouseEventArgs e) {
            base.OnMouseUp(e);
            isDragging = false;
        }

        protected override void OnMouseDoubleClick(MouseEventArgs e) {
            base.OnMouseDoubleClick(e);

            int room = RoomAt(e.Location);

            if (room < 0) return;

            if (room != progress.CurrentRoom) StopFollowing();

            CenterOn(room);
        }

        protected override void Dispose(bool disposing) {
            if (disposing) {
                pulseTimer.Dispose();
                ClearImages();
            }

            base.Dispose(disposing);
        }
    }
}
