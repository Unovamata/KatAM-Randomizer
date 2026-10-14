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
     * • Following Kirby pans smoothly to the room he enters.
     * • Hovering a room shows its preview, with its background, and its mirrors (green: used, yellow: not yet).
     * • Mouse wheel zooms around the cursor, dragging pans, double clicking a room centers it; */
    public sealed class MapView : Control {
        const double MinZoom = 0.25, MaxZoom = 8, ZoomStep = 1.2;
        const int AreaRadius = 56, AreaCell = 8, PreviewMaxWidth = 420, PreviewMaxHeight = 280;
        const double PanDuration = 0.45; // Seconds;

        // Soft colors for the regions behind the room clusters, like the in-game map backdrop;
        static readonly Color[] RegionColors = {
            Color.FromArgb(70, 76, 120), Color.FromArgb(64, 112, 72), Color.FromArgb(120, 92, 60), Color.FromArgb(104, 70, 116),
            Color.FromArgb(56, 108, 116), Color.FromArgb(124, 66, 66), Color.FromArgb(112, 112, 64), Color.FromArgb(66, 82, 128),
            Color.FromArgb(76, 116, 104), Color.FromArgb(128, 104, 48),
        };

        RomMap map;
        Dictionary<int, RoomPlacement> layout;
        Dictionary<int, int> regionOf = new Dictionary<int, int>();
        Dictionary<string, int> lineColors = new Dictionary<string, int>();
        MapProgress progress;
        MapGraphics graphics;

        // Smooth pan: from where the view was to where it's going;
        double panFromX, panFromY, panToX, panToY;
        DateTime panStart;
        bool isPanning;

        readonly Dictionary<Sprite, Bitmap> spriteBitmaps = new Dictionary<Sprite, Bitmap>();
        readonly Dictionary<int, Bitmap> previews = new Dictionary<int, Bitmap>();
        readonly Queue<int> previewOrder = new Queue<int>();
        readonly Timer pulseTimer = new Timer { Interval = 16 };

        Bitmap areaLayer;
        RectangleF areaLayerBounds;
        int areaLayerRoomCount = -1;

        double zoom = 2, offsetX, offsetY; // offset: map position drawn at the top left corner;
        Point dragStart, mouse;
        bool isDragging, hasDragged;
        int hoveredRoom = -1, pulseFrame, pulseTicks;
        DateTime lastPulse = DateTime.Now;

        public bool Follow { get; set; } = true;

        public event Action FollowDisabledByUser;

        // Raised when the zoom changes;
        public event Action ViewChanged;

        public MapView() {
            DoubleBuffered = true;
            ResizeRedraw = true;
            BackColor = Color.FromArgb(28, 28, 36);
            SetStyle(ControlStyles.AllPaintingInWmPaint | ControlStyles.UserPaint | ControlStyles.OptimizedDoubleBuffer, true);

            pulseTimer.Tick += (s, e) => { AdvancePan(); AdvancePulse(); };
            pulseTimer.Start();
        }

        public double Zoom => zoom;

        /* SetMap(); Loads a map. clusters: cluster of every room (from MapLayout), tinted as regions; */
        public void SetMap(RomMap romMap, Dictionary<int, RoomPlacement> roomLayout, Dictionary<int, int> clusters, MapProgress mapProgress) {
            ClearImages();

            map = romMap;
            layout = roomLayout;
            progress = mapProgress;
            graphics = map == null ? null : new MapGraphics(map);
            lineColors = map == null ? new Dictionary<string, int>() : MapRoutes.AssignLineColors(map);

            // Clusters are numbered in order so neighbor clusters tend to get different region colors;
            regionOf = new Dictionary<int, int>();
            Dictionary<int, int> regionIndex = new Dictionary<int, int>();

            foreach ((int room, int cluster) in (clusters ?? new Dictionary<int, int>()).OrderBy(kvp => kvp.Key)) {
                if (!regionIndex.ContainsKey(cluster)) regionIndex[cluster] = regionIndex.Count;
                regionOf[room] = regionIndex[cluster];
            }

            isPanning = false;

            if (progress != null && layout != null && layout.ContainsKey(progress.CurrentRoom)) CenterOn(progress.CurrentRoom, false);
            else FitToDiscovered();

            Invalidate();
        }

        public void SetProgress(MapProgress mapProgress) {
            progress = mapProgress;
            areaLayerRoomCount = -1;
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
            double targetX = placement.CenterX - Width / 2.0 / zoom, targetY = placement.CenterY - Height / 2.0 / zoom;

            if (!animate) {
                isPanning = false;
                offsetX = targetX;
                offsetY = targetY;
                Invalidate();
                return;
            }

            panFromX = offsetX; panFromY = offsetY;
            panToX = targetX; panToY = targetY;
            panStart = DateTime.Now;
            isPanning = true;
        }

        // AdvancePan(); Moves the view along the current smooth pan;
        void AdvancePan() {
            if (!isPanning) return;

            double t = Math.Min(1, (DateTime.Now - panStart).TotalSeconds / PanDuration);
            double eased = 1 - Math.Pow(1 - t, 3);

            offsetX = panFromX + (panToX - panFromX) * eased;
            offsetY = panFromY + (panToY - panFromY) * eased;

            if (t >= 1) isPanning = false;

            Invalidate();
        }

        public void FitToDiscovered() {
            if (layout == null) return;

            List<RoomPlacement> rooms = (progress?.VisitedRooms ?? new HashSet<int>())
                .Where(layout.ContainsKey).Select(r => layout[r]).ToList();

            // Nothing discovered yet; Central Circle is where the adventure starts;
            if (rooms.Count == 0 && layout.TryGetValue(RomMap.CentralCircle, out RoomPlacement hub)) rooms.Add(hub);
            if (rooms.Count == 0) return;

            double minX = rooms.Min(r => r.X), minY = rooms.Min(r => r.Y),
                   maxX = rooms.Max(r => r.X + r.Width), maxY = rooms.Max(r => r.Y + r.Height);
            double margin = 48;

            isPanning = false;
            zoom = Math.Clamp(Math.Min(Width / (maxX - minX + margin * 2), Height / (maxY - minY + margin * 2)), MinZoom, 4);
            offsetX = (minX + maxX) / 2 - Width / 2.0 / zoom;
            offsetY = (minY + maxY) / 2 - Height / 2.0 / zoom;
            Invalidate();
            ViewChanged?.Invoke();
        }

        public void ZoomBy(double factor) => ZoomAt(factor, new Point(Width / 2, Height / 2));

        void ZoomAt(double factor, Point screen) {
            // Zooming during a pan would fight it; The pan target is kept relative to the new zoom instead;
            if (isPanning) {
                double centerX = panToX + Width / 2.0 / zoom, centerY = panToY + Height / 2.0 / zoom;
                double newZoom = Math.Clamp(zoom * factor, MinZoom, MaxZoom);
                panToX = centerX - Width / 2.0 / newZoom;
                panToY = centerY - Height / 2.0 / newZoom;
            }

            double mapX = offsetX + screen.X / zoom, mapY = offsetY + screen.Y / zoom;

            zoom = Math.Clamp(zoom * factor, MinZoom, MaxZoom);
            offsetX = mapX - screen.X / zoom;
            offsetY = mapY - screen.Y / zoom;
            Invalidate();
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

            RoomPlacement current = layout[progress.CurrentRoom];
            PointF topLeft = ToScreen(current.X, current.Y);
            Invalidate(Rectangle.Inflate(Rectangle.Round(new RectangleF(topLeft, new SizeF((float)(current.Width * zoom), (float)(current.Height * zoom)))), 4, 4));
        }

        PointF ToScreen(double x, double y) => new PointF((float)((x - offsetX) * zoom), (float)((y - offsetY) * zoom));

        //////////////////////////////////////////////////////////////////////////////////////////////

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
            g.Transform = new Matrix((float)zoom, 0, 0, (float)zoom, (float)(-offsetX * zoom), (float)(-offsetY * zoom));
            g.PixelOffsetMode = PixelOffsetMode.Half;

            DrawAreas(g);

            g.InterpolationMode = InterpolationMode.NearestNeighbor;

            DrawConnections(g);

            foreach (int room in progress.VisitedRooms) {
                if (layout.ContainsKey(room)) DrawRoom(g, room);
            }

            g.Restore(state);

            if (hoveredRoom >= 0 && !isDragging) DrawPreview(g, hoveredRoom);
        }

        void DrawRoom(Graphics g, int room) {
            RoomPlacement placement = layout[room];
            RoomInfo info = map.Rooms[room];

            RoomStyle style = room == progress.CurrentRoom ? RoomStyle.Current
                : info.Doors.All(progress.IsDoorFound) ? RoomStyle.Explored
                : RoomStyle.Unexplored;

            Sprite sprite = graphics.Room(MapGraphics.ShapeOf(info), style, pulseFrame);
            g.DrawImage(GetBitmap(sprite), (float)placement.X, (float)placement.Y, sprite.Width, sprite.Height);

            if (room == hoveredRoom) {
                using Pen pen = new Pen(Color.White, (float)(1.5 / zoom));
                g.DrawRectangle(pen, (float)placement.X - 1, (float)placement.Y - 1, (float)placement.Width + 2, (float)placement.Height + 2);
            }
        }

        void DrawConnections(Graphics g) {
            foreach (int room in progress.VisitedRooms) {
                if (!layout.ContainsKey(room)) continue;

                foreach (DoorInfo door in map.Rooms[room].Doors) {
                    if (!progress.IsDoorFound(door) || !layout.TryGetValue(door.Destination, out RoomPlacement destination)) continue;

                    bool isTwoWay = door.DoorBack != null;

                    // Two-Way mirrors are drawn once;
                    if (isTwoWay && progress.IsDoorFound(door.DoorBack) && door.Room > door.Destination) continue;

                    LineSprites lines = graphics.Lines(lineColors.GetValueOrDefault(MapRoutes.ConnectionKey(door.Room, door.Destination)));

                    foreach (RouteSegment segment in MapRoutes.Route(layout[room], destination, !isTwoWay)) DrawSegment(g, segment, lines);
                }
            }
        }

        /* DrawSegment(); Fills a segment with the in-game line (or arrow) tiles;
         * Straight segments use the in-game pieces aligned to the 8 pixel grid. Diagonals rotate the horizontal pieces 45°; */
        void DrawSegment(Graphics g, RouteSegment segment, LineSprites lines) {
            if (!segment.IsDiagonal) {
                Sprite sprite = !segment.IsOneWay
                    ? (segment.IsHorizontal ? lines.Horizontal : lines.Vertical)
                    : segment.IsHorizontal ? (segment.DirectionX > 0 ? lines.ArrowRight : lines.ArrowLeft)
                    : (segment.DirectionY > 0 ? lines.ArrowDown : lines.ArrowUp);

                double left = Math.Min(segment.StartX, segment.EndX), top = Math.Min(segment.StartY, segment.EndY);
                RectangleF area = segment.IsHorizontal
                    ? new RectangleF((float)left, (float)segment.StartY - 8, (float)Math.Abs(segment.EndX - segment.StartX), 16)
                    : new RectangleF((float)segment.StartX - 8, (float)top, 16, (float)Math.Abs(segment.EndY - segment.StartY));

                using TextureBrush brush = new TextureBrush(GetBitmap(sprite), WrapMode.Tile);
                brush.TranslateTransform(area.X, area.Y);
                g.FillRectangle(brush, area);
                return;
            }

            // Diagonal: the horizontal piece (or the arrow pointing right) rotated along the segment;
            Sprite piece = segment.IsOneWay ? lines.ArrowRight : lines.Horizontal;
            GraphicsState state = g.Save();

            g.TranslateTransform((float)segment.StartX, (float)segment.StartY);
            g.RotateTransform((float)segment.Angle);

            using (TextureBrush brush = new TextureBrush(GetBitmap(piece), WrapMode.Tile)) {
                brush.TranslateTransform(0, -8);
                g.FillRectangle(brush, 0, -8, (float)segment.Length, 16);
            }

            g.Restore(state);
        }

        // DrawAreas(); Soft colored regions behind the discovered rooms, by cluster, like the in-game map backdrop;
        void DrawAreas(Graphics g) {
            List<int> visited = progress.VisitedRooms.Where(layout.ContainsKey).ToList();

            if (visited.Count == 0) return;

            if (areaLayer == null || areaLayerRoomCount != visited.Count) BuildAreaLayer(visited);

            InterpolationMode previous = g.InterpolationMode;
            g.InterpolationMode = InterpolationMode.Bilinear;
            g.DrawImage(areaLayer, areaLayerBounds);
            g.InterpolationMode = previous;
        }

        void BuildAreaLayer(List<int> visited) {
            areaLayer?.Dispose();

            double minX = visited.Min(r => layout[r].CenterX) - AreaRadius, minY = visited.Min(r => layout[r].CenterY) - AreaRadius,
                   maxX = visited.Max(r => layout[r].CenterX) + AreaRadius, maxY = visited.Max(r => layout[r].CenterY) + AreaRadius;

            int columns = (int)Math.Ceiling((maxX - minX) / AreaCell) + 1, rows = (int)Math.Ceiling((maxY - minY) / AreaCell) + 1;
            double[] distance = new double[columns * rows];
            int[] colors = new int[columns * rows];

            Array.Fill(distance, double.MaxValue);

            // Every room stamps its color around it; The nearest room wins, which splits the regions like a Voronoi diagram;
            foreach (int room in visited) {
                RoomPlacement p = layout[room];
                Color color = RegionColors[regionOf.GetValueOrDefault(room) % RegionColors.Length];
                int argb = Color.FromArgb(150, color).ToArgb();
                int centerColumn = (int)((p.CenterX - minX) / AreaCell), centerRow = (int)((p.CenterY - minY) / AreaCell);
                int reach = AreaRadius / AreaCell;

                for (int row = Math.Max(0, centerRow - reach); row <= Math.Min(rows - 1, centerRow + reach); row++) {
                    for (int column = Math.Max(0, centerColumn - reach); column <= Math.Min(columns - 1, centerColumn + reach); column++) {
                        double dx = minX + column * AreaCell - p.CenterX, dy = minY + row * AreaCell - p.CenterY;
                        double d = Math.Sqrt(dx * dx + dy * dy);

                        if (d > AreaRadius || d >= distance[row * columns + column]) continue;

                        distance[row * columns + column] = d;
                        colors[row * columns + column] = argb;
                    }
                }
            }

            areaLayer = ToBitmap(new Sprite(columns, rows, colors));
            areaLayerBounds = new RectangleF((float)minX, (float)minY, columns * AreaCell, rows * AreaCell);
            areaLayerRoomCount = visited.Count;
        }

        // DrawPreview(); Shows the hovered room with its in-game graphics and background, and where its mirrors are;
        void DrawPreview(Graphics g, int room) {
            RoomInfo info = map.Rooms[room];
            Bitmap image = GetPreview(room);

            double scale = Math.Min(1, Math.Min((double)PreviewMaxWidth / image.Width, (double)PreviewMaxHeight / image.Height));
            int width = (int)(image.Width * scale), height = (int)(image.Height * scale);

            int found = info.Doors.Count(progress.IsDoorFound);
            string caption = $"Room 0x{room:X}   Mirrors used: {found}/{info.Doors.Count}";
            Size captionSize = TextRenderer.MeasureText(caption, Font);

            int boxWidth = Math.Max(width, captionSize.Width) + 12, boxHeight = height + captionSize.Height + 14;
            int x = mouse.X + 20, y = mouse.Y + 20;

            if (x + boxWidth > Width) x = Math.Max(0, mouse.X - boxWidth - 20);
            if (y + boxHeight > Height) y = Math.Max(0, mouse.Y - boxHeight - 20);

            using (SolidBrush background = new SolidBrush(Color.FromArgb(235, 16, 16, 24))) g.FillRectangle(background, x, y, boxWidth, boxHeight);
            g.DrawRectangle(Pens.Gold, x, y, boxWidth - 1, boxHeight - 1);
            TextRenderer.DrawText(g, caption, Font, new Point(x + 6, y + 4), Color.White);

            int imageX = x + 6, imageY = y + captionSize.Height + 8;

            g.InterpolationMode = scale >= 1 ? InterpolationMode.NearestNeighbor : InterpolationMode.HighQualityBilinear;
            g.DrawImage(image, imageX, imageY, width, height);

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

        Bitmap GetBitmap(Sprite sprite) {
            if (spriteBitmaps.TryGetValue(sprite, out Bitmap cached)) return cached;

            Bitmap bitmap = ToBitmap(sprite);
            spriteBitmaps[sprite] = bitmap;

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

        void ClearImages() {
            foreach (Bitmap bitmap in spriteBitmaps.Values.Concat(previews.Values)) bitmap.Dispose();
            spriteBitmaps.Clear();
            previews.Clear();
            previewOrder.Clear();
            areaLayer?.Dispose();
            areaLayer = null;
            areaLayerRoomCount = -1;
        }

        int RoomAt(Point screen) {
            if (layout == null || progress == null) return -1;

            double mapX = offsetX + screen.X / zoom, mapY = offsetY + screen.Y / zoom;

            foreach (int room in progress.VisitedRooms) {
                if (!layout.TryGetValue(room, out RoomPlacement p)) continue;
                if (mapX >= p.X && mapX <= p.X + p.Width && mapY >= p.Y && mapY <= p.Y + p.Height) return room;
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

                if (!hasDragged && Follow) {
                    // Panning by hand stops following Kirby;
                    Follow = false;
                    FollowDisabledByUser?.Invoke();
                }

                hasDragged = true;
                isPanning = false;
                offsetX -= dx / zoom;
                offsetY -= dy / zoom;
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

            if (room >= 0) CenterOn(room);
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
