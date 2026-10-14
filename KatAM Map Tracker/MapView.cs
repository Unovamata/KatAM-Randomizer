using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.Linq;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace KatAMMapTracker {
    /* MapView; Draws the discovered part of the map with the rooms' in-game graphics;
     * • Mouse wheel zooms around the cursor, dragging pans, double clicking a room centers it.
     * • Only visited rooms are drawn. Their doors show as markers: yellow if unexplored, green once used.
     * • Lines join the doors Kirby went through: blue for Two-Way mirrors, orange arrows for One-Way ones; */
    public sealed class MapView : Control {
        const double MinZoom = 0.02, MaxZoom = 4, ZoomStep = 1.2;
        const int FullResolution = 8;

        RomMap map;
        Dictionary<int, RoomPlacement> layout;
        MapProgress progress;

        readonly Dictionary<int, Bitmap> thumbnails = new Dictionary<int, Bitmap>(),
                                         fullImages = new Dictionary<int, Bitmap>();
        readonly ToolTip toolTip = new ToolTip();

        double zoom = 0.5, offsetX, offsetY; // offset: map position drawn at the top left corner;
        Point dragStart;
        bool isDragging, hasDragged;
        int hoveredRoom = -1;

        public bool Follow { get; set; } = true;

        public event Action FollowDisabledByUser;

        // Raised when the zoom changes;
        public event Action ViewChanged;

        public MapView() {
            DoubleBuffered = true;
            ResizeRedraw = true;
            BackColor = Color.FromArgb(24, 24, 32);
            SetStyle(ControlStyles.AllPaintingInWmPaint | ControlStyles.UserPaint | ControlStyles.OptimizedDoubleBuffer, true);
        }

        public double Zoom => zoom;

        public void SetMap(RomMap romMap, Dictionary<int, RoomPlacement> roomLayout, MapProgress mapProgress) {
            ClearImages();

            map = romMap;
            layout = roomLayout;
            progress = mapProgress;

            if (progress != null && layout != null && layout.ContainsKey(progress.CurrentRoom)) CenterOn(progress.CurrentRoom);
            else FitToDiscovered();

            Invalidate();
        }

        public void SetProgress(MapProgress mapProgress) {
            progress = mapProgress;
            Invalidate();
        }

        // OnRoomChanged(); Called when Kirby enters a room, to follow him;
        public void OnRoomChanged() {
            if (Follow && progress != null && layout != null && layout.ContainsKey(progress.CurrentRoom)) CenterOn(progress.CurrentRoom);

            Invalidate();
        }

        public void CenterOn(int room) {
            RoomPlacement placement = layout[room];
            offsetX = placement.CenterX - Width / 2.0 / zoom;
            offsetY = placement.CenterY - Height / 2.0 / zoom;
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
            double margin = 100;

            zoom = Math.Clamp(Math.Min(Width / (maxX - minX + margin * 2), Height / (maxY - minY + margin * 2)), MinZoom, 1);
            offsetX = (minX + maxX) / 2 - Width / 2.0 / zoom;
            offsetY = (minY + maxY) / 2 - Height / 2.0 / zoom;
            Invalidate();
            ViewChanged?.Invoke();
        }

        public void ZoomBy(double factor) => ZoomAt(factor, new Point(Width / 2, Height / 2));

        void ZoomAt(double factor, Point screen) {
            double mapX = offsetX + screen.X / zoom, mapY = offsetY + screen.Y / zoom;

            zoom = Math.Clamp(zoom * factor, MinZoom, MaxZoom);
            offsetX = mapX - screen.X / zoom;
            offsetY = mapY - screen.Y / zoom;
            Invalidate();
            ViewChanged?.Invoke();
        }

        PointF ToScreen(double x, double y) => new PointF((float)((x - offsetX) * zoom), (float)((y - offsetY) * zoom));

        // DoorPoint(); Position of a door (in room pixels) on the map;
        static PointF DoorPoint(RoomPlacement room, int x, int y) {
            double scale = MapLayout.PixelsPerTile / 8.0;
            return new PointF((float)(room.X + x * scale), (float)(room.Y + y * scale));
        }

        //////////////////////////////////////////////////////////////////////////////////////////////

        protected override void OnPaint(PaintEventArgs e) {
            base.OnPaint(e);

            Graphics g = e.Graphics;

            if (map == null || layout == null || progress == null) {
                TextRenderer.DrawText(g, "Open a randomized ROM to load its map (File > Open ROM...).", Font,
                                      ClientRectangle, Color.Gainsboro, TextFormatFlags.HorizontalCenter | TextFormatFlags.VerticalCenter);
                return;
            }

            g.SmoothingMode = SmoothingMode.AntiAlias;
            g.InterpolationMode = zoom >= 1 ? InterpolationMode.NearestNeighbor : InterpolationMode.Bilinear;
            g.PixelOffsetMode = PixelOffsetMode.Half;

            List<int> visibleRooms = progress.VisitedRooms.Where(layout.ContainsKey).ToList();

            DrawConnections(g);

            foreach (int room in visibleRooms) DrawRoom(g, room);

            foreach (int room in visibleRooms) DrawDoors(g, room);

            if (layout.TryGetValue(progress.CurrentRoom, out RoomPlacement current)) {
                PointF topLeft = ToScreen(current.X, current.Y);
                using Pen highlight = new Pen(Color.Gold, 3);
                g.DrawRectangle(highlight, topLeft.X - 3, topLeft.Y - 3, (float)(current.Width * zoom) + 6, (float)(current.Height * zoom) + 6);
            }
        }

        void DrawRoom(Graphics g, int room) {
            RoomPlacement placement = layout[room];
            PointF topLeft = ToScreen(placement.X, placement.Y);
            RectangleF bounds = new RectangleF(topLeft.X, topLeft.Y, (float)(placement.Width * zoom), (float)(placement.Height * zoom));

            if (!bounds.IntersectsWith(ClientRectangle)) return;

            // Full resolution graphics are only used when zoomed in enough to see them;
            Bitmap image = zoom > 1.2 ? GetImage(fullImages, room, FullResolution) : GetImage(thumbnails, room, MapLayout.PixelsPerTile);

            g.DrawImage(image, bounds);

            using Pen border = new Pen(room == hoveredRoom ? Color.White : Color.FromArgb(160, 200, 200, 200), 1);
            g.DrawRectangle(border, bounds.X, bounds.Y, bounds.Width, bounds.Height);

            if (zoom >= 0.35) {
                using Font font = new Font(Font.FontFamily, 8, FontStyle.Bold);
                string label = $"0x{room:X}";
                SizeF size = g.MeasureString(label, font);
                g.FillRectangle(Brushes.Black, bounds.X, bounds.Y - size.Height, size.Width, size.Height);
                g.DrawString(label, font, Brushes.White, bounds.X, bounds.Y - size.Height);
            }
        }

        void DrawDoors(Graphics g, int room) {
            RoomPlacement placement = layout[room];
            float size = (float)Math.Clamp(16 * zoom, 4, 14);

            foreach (DoorInfo door in map.Rooms[room].Doors) {
                PointF point = DoorPoint(placement, door.X, door.Y);
                PointF screen = ToScreen(point.X, point.Y);
                bool found = progress.IsDoorFound(door);

                using Brush fill = new SolidBrush(found ? Color.LimeGreen : Color.Gold);
                g.FillRectangle(fill, screen.X - size / 2, screen.Y - size / 2, size, size);
                g.DrawRectangle(Pens.Black, screen.X - size / 2, screen.Y - size / 2, size, size);
            }
        }

        void DrawConnections(Graphics g) {
            using Pen twoWay = new Pen(Color.FromArgb(220, 110, 160, 255), 2);
            using Pen oneWay = new Pen(Color.FromArgb(220, 255, 170, 60), 2) { CustomEndCap = new AdjustableArrowCap(4, 5) };

            foreach (int room in progress.VisitedRooms) {
                if (!layout.ContainsKey(room)) continue;

                foreach (DoorInfo door in map.Rooms[room].Doors) {
                    if (!progress.IsDoorFound(door) || !layout.TryGetValue(door.Destination, out RoomPlacement destination)) continue;

                    // Two-Way mirrors are drawn once, from the room with the lowest ID;
                    if (door.DoorBack != null && progress.IsDoorFound(door.DoorBack) && door.Room > door.Destination) continue;

                    PointF from = DoorPoint(layout[room], door.X, door.Y);
                    PointF to = door.DoorBack != null
                        ? DoorPoint(destination, door.DoorBack.X, door.DoorBack.Y)
                        : DoorPoint(destination, door.LandX, door.LandY);

                    g.DrawLine(door.DoorBack != null ? twoWay : oneWay, ToScreen(from.X, from.Y), ToScreen(to.X, to.Y));
                }
            }
        }

        Bitmap GetImage(Dictionary<int, Bitmap> cache, int room, int pixelsPerTile) {
            if (cache.TryGetValue(room, out Bitmap cached)) return cached;

            int[] pixels = map.RenderRoom(room, pixelsPerTile, out int width, out int height);
            Bitmap bitmap = new Bitmap(width, height, PixelFormat.Format32bppArgb);
            BitmapData data = bitmap.LockBits(new Rectangle(0, 0, width, height), ImageLockMode.WriteOnly, PixelFormat.Format32bppArgb);

            for (int y = 0; y < height; y++) {
                Marshal.Copy(pixels, y * width, data.Scan0 + y * data.Stride, width);
            }

            bitmap.UnlockBits(data);
            cache[room] = bitmap;

            return bitmap;
        }

        void ClearImages() {
            foreach (Bitmap bitmap in thumbnails.Values.Concat(fullImages.Values)) bitmap.Dispose();
            thumbnails.Clear();
            fullImages.Clear();
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

            if (isDragging) {
                int dx = e.X - dragStart.X, dy = e.Y - dragStart.Y;

                if (!hasDragged && Math.Abs(dx) + Math.Abs(dy) < 4) return;

                if (!hasDragged && Follow) {
                    // Panning by hand stops following Kirby;
                    Follow = false;
                    FollowDisabledByUser?.Invoke();
                }

                hasDragged = true;
                offsetX -= dx / zoom;
                offsetY -= dy / zoom;
                dragStart = e.Location;
                Invalidate();
                return;
            }

            int room = RoomAt(e.Location);

            if (room == hoveredRoom) return;

            hoveredRoom = room;
            toolTip.SetToolTip(this, room < 0 ? "" : RoomToolTip(room));
            Invalidate();
        }

        string RoomToolTip(int room) {
            RoomInfo info = map.Rooms[room];
            int found = info.Doors.Count(progress.IsDoorFound);

            return $"Room 0x{room:X}\nMirrors explored: {found}/{info.Doors.Count}";
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
                ClearImages();
                toolTip.Dispose();
            }

            base.Dispose(disposing);
        }
    }
}
