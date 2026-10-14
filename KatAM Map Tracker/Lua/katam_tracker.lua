-- KatAM Map Tracker - mGBA script (mGBA 0.10 or newer)
--
-- Reports the room Kirby is in to the KatAM Map Tracker window, which draws the map as you explore.
-- Usage: open the tracker window first, then in mGBA go to Tools > Scripting... > File > Load script
-- and pick this file. The script reconnects on its own if the tracker is opened later or restarted.
--
-- Works with the Japanese (AGB-B8KJ), American (AGB-B8KE) and European (AGB-B8KP) versions.

local HOST = "127.0.0.1"
local PORT = 8765                 -- Must match the port of the tracker window;
local ROOM_ADDRESS = 0x02020F40   -- Current room ID of Kirby (player 1), the same in every version;
local RETRY_FRAMES = 180          -- Frames to wait before trying to connect again (~3 seconds);
local SCRIPT_VERSION = 3          -- Sent to the tracker, which warns about outdated copies of this script;
local MAPS_ADDRESS = 0x0203897C   -- Area maps the loaded save holds (gTreasures.bigChestField), one bit per area;

local connection = nil
local sentRoom = -1
local sentMaps = -1
local retryTimer = 0
local warnedGame = false

local function log(message)
  console:log("[KatAM Tracker] " .. message)
end

local function disconnect()
  if connection then connection:close() end
  connection = nil
  sentRoom = -1
  sentMaps = -1
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
  sentMaps = -1
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

local function connectAndGreet()
  connect()
  if connection then send("HELLO " .. SCRIPT_VERSION) end
end

callbacks:add("frame", function()
  if not emu then return end

  if not warnedGame then
    warnedGame = true
    local ok, code = pcall(function() return emu:getGameCode() end)
    if ok and code then
      local version = string.find(code, "B8KJ") and "Japanese" or string.find(code, "B8KE") and "American"
                      or string.find(code, "B8KP") and "European" or nil
      if version then
        log("Game: Kirby & the Amazing Mirror, " .. version .. " version (" .. code .. ").")
      else
        log("Warning: this doesn't look like a KatAM ROM (" .. code .. "). Rooms may not be tracked.")
      end
    end
  end

  if not connection then
    retryTimer = retryTimer - 1
    if retryTimer > 0 then return end
    retryTimer = RETRY_FRAMES
    connectAndGreet()
    if not connection then return end
  end

  local room = emu:read16(ROOM_ADDRESS)

  if room ~= sentRoom and send(string.format("ROOM %X", room)) then
    sentRoom = room
  end

  -- The maps change when one is found, or when another save is loaded;
  if connection then
    local maps = emu:read32(MAPS_ADDRESS) & 0x3FF
    if maps ~= sentMaps and send(string.format("MAPS %X", maps)) then
      sentMaps = maps
    end
  end
end)

callbacks:add("reset", function()
  sentRoom = -1
end)

log("Loaded (script version " .. SCRIPT_VERSION .. "). Waiting for the tracker window on port " .. PORT .. "...")
