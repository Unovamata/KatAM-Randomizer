using System;
using System.Globalization;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Threading;

namespace KatAMMapTracker {
    /* TrackerServer; Receives the room reports sent by the emulator script (Lua/katam_tracker.lua);
     * The script connects to 127.0.0.1 on the given port, sends "HELLO <script version>", then one line per room change: "ROOM <hex id>",
     * and the area maps of the loaded save whenever they change: "MAPS <hex bits>" (bit n: the map of area n).
     * Old copies of the script don't send HELLO; Their first room is reported as script version 0;
     * Debug: the tracker sends the script "WARP <hex room> <x> <y>" (Send()), and the script moves Kirby there (x, y: the spawn tile, as a
     * door's landing; -1 -1: the room's own default spawn);
     * Events are raised on a background thread; */
    public sealed class TrackerServer : IDisposable {
        public const int DefaultPort = 8765;

        // Version of Lua/katam_tracker.lua that goes with this tracker (SCRIPT_VERSION in the script);
        public const int ScriptVersion = 4;

        // Raised once per connection with the version of the connected script (0: an old script without HELLO);
        public event Action<int> ScriptVersionReported;

        // Raised with the area maps of the loaded save (bit n: the map of area n);
        public event Action<int> MapsReported;

        // Raised for every valid line. sessionStart is true for the first report of a connection;
        public event Action<int, bool> RoomReported;
        public event Action<bool> ConnectionChanged;

        public int Port { get; }

        readonly TcpListener listener;
        Thread thread;
        volatile bool running;
        TcpClient currentClient;
        readonly object sendLock = new object();

        public TrackerServer(int port = DefaultPort) {
            Port = port;
            listener = new TcpListener(IPAddress.Loopback, port);
        }

        public void Start() {
            listener.Start();
            running = true;
            thread = new Thread(Run) { IsBackground = true, Name = "KatAM Tracker Server" };
            thread.Start();
        }

        void Run() {
            while (running) {
                TcpClient client;

                try { client = listener.AcceptTcpClient(); }
                catch (SocketException) { break; }
                catch (ObjectDisposedException) { break; }

                currentClient = client;
                ConnectionChanged?.Invoke(true);

                try {
                    using StreamReader reader = new StreamReader(client.GetStream());
                    bool sessionStart = true, versionKnown = false;
                    string line;

                    while (running && (line = reader.ReadLine()) != null) {
                        if (!versionKnown && TryParseHello(line, out int version)) {
                            versionKnown = true;
                            ScriptVersionReported?.Invoke(version);
                            continue;
                        }

                        if (TryParseMaps(line, out int maps)) {
                            MapsReported?.Invoke(maps);
                            continue;
                        }

                        if (!TryParseRoom(line, out int room)) continue;

                        if (!versionKnown) {
                            versionKnown = true;
                            ScriptVersionReported?.Invoke(0);
                        }

                        RoomReported?.Invoke(room, sessionStart);
                        sessionStart = false;
                    }
                }
                catch (IOException) { }
                catch (ObjectDisposedException) { }
                finally {
                    client.Close();
                    currentClient = null;
                    ConnectionChanged?.Invoke(false);
                }
            }
        }

        // Send(); A line to the connected script (false when none is connected or the connection failed);
        public bool Send(string line) {
            lock (sendLock) {
                try {
                    TcpClient client = currentClient;
                    if (client == null || !client.Connected) return false;
                    byte[] bytes = System.Text.Encoding.ASCII.GetBytes(line + "\n");
                    client.GetStream().Write(bytes, 0, bytes.Length);
                    return true;
                }
                catch (Exception exception) when (exception is IOException || exception is ObjectDisposedException || exception is InvalidOperationException) {
                    return false;
                }
            }
        }

        public static bool TryParseRoom(string line, out int room) {
            room = -1;
            string[] parts = line.Trim().Split(' ', StringSplitOptions.RemoveEmptyEntries);

            return parts.Length == 2 && parts[0] == "ROOM" &&
                   int.TryParse(parts[1], NumberStyles.HexNumber, CultureInfo.InvariantCulture, out room);
        }

        public static bool TryParseMaps(string line, out int maps) {
            maps = 0;
            string[] parts = line.Trim().Split(' ', StringSplitOptions.RemoveEmptyEntries);

            return parts.Length == 2 && parts[0] == "MAPS" && int.TryParse(parts[1], NumberStyles.HexNumber, CultureInfo.InvariantCulture, out maps);
        }

        public static bool TryParseHello(string line, out int version) {
            version = 0;
            string[] parts = line.Trim().Split(' ', StringSplitOptions.RemoveEmptyEntries);

            return parts.Length == 2 && parts[0] == "HELLO" && int.TryParse(parts[1], NumberStyles.Integer, CultureInfo.InvariantCulture, out version);
        }

        public void Dispose() {
            running = false;
            listener.Stop();
            currentClient?.Close();
        }
    }
}
