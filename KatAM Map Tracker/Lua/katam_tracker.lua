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
local SCRIPT_VERSION = 4          -- Sent to the tracker, which warns about outdated copies of this script;
local MAPS_ADDRESS = 0x0203897C   -- Area maps the loaded save holds (gTreasures.bigChestField), one bit per area;

-- DEBUG (to be removed for the release): the tracker's "WARP <hex room> <x> <y>" moves Kirby (player 1) into that room, as the
-- game's own warps do (sub_0805BDF4): his destination room and spawn tile, then his state set to the room transition's start
-- (sub_0805545C, per version); x, y -1: the room's default spawn (gUnk_0893CB44, per version). Kirby's struct (gKirbys[0]) is at
-- the same address in every version;
local KIRBY = 0x02020EE0
local TRANSITION = { B8KE = 0x0805545C, B8KP = 0x080554A4, B8KJ = 0x080553D4 }
local DEFAULT_SPAWNS = { B8KE = 0x0893CB44, B8KP = 0x08946FA8, B8KJ = 0x0890CCD0 }
local TRANSITION_START = { 0xF0, 0xB5, 0x57, 0x46, 0x4E, 0x46, 0x45, 0x46 }   -- sub_0805545C's first bytes, checked before calling it;
local WARP_WAIT = 600                                                        -- Frames a warp waits for Kirby to be free to move;

local connection = nil
local received = ""
local pendingWarp = nil
local gameCode = nil
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
  received = ""
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

-- warpBlocked(); Why Kirby can't be warped right now (nil: he can), as the game's own warps refuse (sub_080551FC);
local function warpBlocked(room)
  if (emu:read8(0x03000514) & 0x11) ~= 0 then return "the game is paused or in a cutscene" end
  local hp = emu:read8(KIRBY + 0x100)
  if hp == 0 or hp >= 0x80 then return "Kirby has no HP" end
  local anim = emu:read16(KIRBY + 0xD4)
  if anim == 39 or anim > 122 then return "Kirby is busy" end
  if emu:read32(KIRBY + 0x110) ~= 0 then return "Kirby is busy" end
  if (emu:read32(KIRBY + 0x08) & 0x3800B00) ~= 0 then return "Kirby is busy" end
  if emu:read16(KIRBY + 0x60) == room then return "Kirby is already there" end
  return nil
end

-- doWarp(); Kirby into the room, onto the spawn tile (as sub_0805BDF4: the flags, the hub's return spot, the room and tile, then
-- the transition), true once done;
local function doWarp(warp)
  local code = gameCode or ""
  local transition = TRANSITION[string.sub(code, -4)] or TRANSITION[code]
  if not transition then log("Debug warp: unknown game version."); return true end
  for i, byte in ipairs(TRANSITION_START) do
    if emu:read8(transition + i - 1) ~= byte then log("Debug warp: this ROM's room transition isn't where it's expected."); return true end
  end
  local reason = warpBlocked(warp.room)
  if reason then
    warp.wait = warp.wait - 1
    if warp.wait <= 0 then log("Debug warp to room " .. string.format("%X", warp.room) .. " cancelled: " .. reason .. ".") ; return true end
    return false
  end
  local x, y = warp.x, warp.y
  if x < 0 or y < 0 then
    local table = DEFAULT_SPAWNS[string.sub(code, -4)] or DEFAULT_SPAWNS[code]
    x = emu:read8(table + warp.room * 2); y = emu:read8(table + warp.room * 2 + 1)
  end
  local unkC = emu:read32(KIRBY + 0x0C)
  emu:write32(KIRBY + 0x0C, (unkC | 0x480000) & ~0x40000)
  if emu:read16(KIRBY + 0x60) == 0x321 then
    emu:write8(KIRBY + 0xF2, (emu:read32(KIRBY + 0x40) >> 12) & 0xFF)
    emu:write8(KIRBY + 0xF3, (emu:read32(KIRBY + 0x44) >> 12) & 0xFF)
  end
  emu:write8(KIRBY + 0x62, emu:read8(KIRBY + 0x62) & ~0x10)   -- (not through the door he may stand at);
  emu:write8(KIRBY + 0xD8, 0)                                -- (flyTimer: else the transition sends him to his respawn spot);
  emu:write16(KIRBY + 0x106, warp.room)
  emu:write16(KIRBY + 0x108, x)
  emu:write16(KIRBY + 0x10A, y)
  emu:write32(KIRBY + 0x78, transition | 1)
  log(string.format("Debug warp: Kirby to room %X (%d, %d).", warp.room, x, y))
  return true
end

-- receive(); The tracker's lines: "WARP <hex room> <x> <y>" (debug);
local function receive()
  while connection do
    local ok, ready = pcall(function() return connection:hasdata() end)
    if not ok or not ready then break end
    local data = connection:receive(1024)
    if not data or #data == 0 then break end
    received = received .. data
  end
  while true do
    local newline = string.find(received, "\n")
    if not newline then break end
    local line = string.sub(received, 1, newline - 1)
    received = string.sub(received, newline + 1)
    local room, x, y = string.match(line, "^WARP (%x+) (%-?%d+) (%-?%d+)")
    if room then pendingWarp = { room = tonumber(room, 16), x = tonumber(x), y = tonumber(y), wait = WARP_WAIT } end
  end
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
      gameCode = code
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

  receive()
  if pendingWarp and doWarp(pendingWarp) then pendingWarp = nil end

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
