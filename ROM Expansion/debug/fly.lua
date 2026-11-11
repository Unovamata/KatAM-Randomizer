-- debug/fly.lua: a debug fly mode for mGBA (its scripting window, or a headless run's LUA) (the owner: "a way to move around as kirby so kirby ignores collisions and floats
-- at fast speeds"): after each frame, Kirby (player 0, KIRBY env, USA/EU/JP 0x02020EE0) is put where the held D-pad takes him, SPEED
-- pixels a frame (env, default 8), his speeds zeroed: walls and floors don't stop him and he doesn't fall. FLY=0 turns it off. Logs
-- his spot every LOGEVERY frames (env) as "fly f x y room". In mGBA: Tools, Scripting, File, Load script; then hold the D-pad.
-- Testing only: it writes the game's memory every frame, nothing in the ROM.
local getenv = (os and os.getenv) or function() return nil end
local K = tonumber(getenv("KIRBY") or "0x02020EE0")
local SPEED = tonumber(getenv("SPEED") or "8") * 256
local EVERY = tonumber(getenv("LOGEVERY") or "0")
local x, y, room, f = nil, nil, nil, 0
local function s32(v) if v >= 0x80000000 then return v - 0x100000000 end return v end
callbacks:add("frame", function()
  f = f + 1
  if getenv("FLY") == "0" then return end
  local r = emu:read16(K + 0x60)
  if x == nil or r ~= room then x = s32(emu:read32(K + 0x40)); y = s32(emu:read32(K + 0x44)); room = r end
  local keys = emu:getKeys()
  if (keys & 0x10) ~= 0 then x = x + SPEED end
  if (keys & 0x20) ~= 0 then x = x - SPEED end
  if (keys & 0x40) ~= 0 then y = y - SPEED end
  if (keys & 0x80) ~= 0 then y = y + SPEED end
  emu:write32(K + 0x40, x); emu:write32(K + 0x44, y); emu:write32(K + 0x48, x); emu:write32(K + 0x4C, y)
  emu:write16(K + 0x50, 0); emu:write16(K + 0x52, 0)
  -- (his HP kept full: enemies still hit him, he doesn't die);
  emu:write8(K + 0x100, emu:read8(K + 0x101))
  if EVERY > 0 and f % EVERY == 0 then console:log(string.format("KatAM fly %d %d %d %x", f, x // 256, y // 256, r)) end
end)
