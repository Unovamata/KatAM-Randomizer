-- KatAM Map Tracker - mGBA script (mGBA 0.10 or newer)
--
-- Reports the room Kirby is in to the KatAM Map Tracker window, which draws the map as you explore.
-- Usage: open the tracker window first, then in mGBA go to Tools > Scripting... > File > Load script
-- and pick this file. The script reconnects on its own if the tracker is opened later or restarted.
--
-- Only the Japanese version (Hoshi no Kirby: Kagami no Daimeikyuu, AGB-B8KJ) is supported.

local HOST = "127.0.0.1"
local PORT = 8765                 -- Must match the port of the tracker window;
local ROOM_ADDRESS = 0x02020F40   -- Current room ID of Kirby (player 1);
local RETRY_FRAMES = 180          -- Frames to wait before trying to connect again (~3 seconds);

local connection = nil
local sentRoom = -1
local retryTimer = 0
local warnedGame = false

local function log(message)
  console:log("[KatAM Tracker] " .. message)
end

local function disconnect()
  if connection then connection:close() end
  connection = nil
  sentRoom = -1
end

local function connect()
  local sock, err = socket.connect(HOST, PORT)

  if not sock then return end

  sock:add("error", function()
    log("Connection lost, retrying...")
    disconnect()
  end)

  connection = sock
  sentRoom = -1
  log("Connected to the tracker on port " .. PORT .. ".")
end

local function send(line)
  local sent, err = connection:send(line .. "\n")

  if not sent then
    log("Connection lost (" .. tostring(err) .. "), retrying...")
    disconnect()
    return false
  end

  return true
end

callbacks:add("frame", function()
  if not emu then return end

  if not warnedGame then
    warnedGame = true
    local ok, code = pcall(function() return emu:getGameCode() end)
    if ok and code and not string.find(code, "B8KJ") then
      log("Warning: this doesn't look like the Japanese KatAM ROM (" .. code .. "). Rooms may not be tracked.")
    end
  end

  if not connection then
    retryTimer = retryTimer - 1
    if retryTimer > 0 then return end
    retryTimer = RETRY_FRAMES
    connect()
    if not connection then return end
  end

  local room = emu:read16(ROOM_ADDRESS)

  if room ~= sentRoom and send(string.format("ROOM %X", room)) then
    sentRoom = room
  end
end)

callbacks:add("reset", function()
  sentRoom = -1
end)

log("Loaded. Waiting for the tracker window on port " .. PORT .. "...")
