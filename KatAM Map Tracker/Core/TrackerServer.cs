using System;
using System.Globalization;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Threading;

namespace KatAMMapTracker {
    /* TrackerServer; Receives the room reports sent by the emulator script (Lua/katam_tracker.lua);
     * The script connects to 127.0.0.1 on the given port and sends one line per room change: "ROOM <hex id>".
     * Events are raised on a background thread; */
    public sealed class TrackerServer : IDisposable {
        public const int DefaultPort = 8765;

        // Raised for every valid line. sessionStart is true for the first report of a connection;
        public event Action<int, bool> RoomReported;
        public event Action<bool> ConnectionChanged;

        public int Port { get; }

        readonly TcpListener listener;
        Thread thread;
        volatile bool running;
        TcpClient currentClient;

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
                    bool sessionStart = true;
                    string line;

                    while (running && (line = reader.ReadLine()) != null) {
                        if (!TryParseRoom(line, out int room)) continue;

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

        public static bool TryParseRoom(string line, out int room) {
            room = -1;
            string[] parts = line.Trim().Split(' ', StringSplitOptions.RemoveEmptyEntries);

            return parts.Length == 2 && parts[0] == "ROOM" &&
                   int.TryParse(parts[1], NumberStyles.HexNumber, CultureInfo.InvariantCulture, out room);
        }

        public void Dispose() {
            running = false;
            listener.Stop();
            currentClient?.Close();
        }
    }
}
