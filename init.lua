local exports = {}

--[[ LookUpTables ]]--

local status = {
  RESET     = 0,  -- reset is not consumed by this module and does not receive valid key states
  KEY_DOWN  = 1,
  KEY_HOLD  = 2,
  KEY_UP    = 3,
}

--[[ Event Object ]]--

local Event = {}

function Event:ctrlActive()
  return (self.eventInt & 0x01000000) ~= 0
end

function Event:shiftActive()
  return (self.eventInt & 0x00010000) ~= 0
end

function Event:altActive()
  return (self.eventInt & 0x00000100) ~= 0
end

function Event:virtualKeyNum()
  return (self.eventInt & 0xFF)
end

function Event:statusNum()
  return (self.eventInt & 0xFF00000000) >> 32
end

function Event:new(eventInt)
  local obj = {}
  setmetatable(obj, self)
  self.__index = self
  obj.eventInt = eventInt
  return obj
end

--[[ Event Handling Func ]]--

local funcTable = {}

local function controlFunc(keyMapName, refNum, state)
  local mapTable = funcTable[keyMapName]
  if mapTable == nil then
    log(WARNING, "[inputHandler]: Unable to react to key event. Requested key table not found.")
    return false
  end
  
  local func = mapTable[refNum]
  if func == nil then
    log(WARNING, "[inputHandler]: Unable to react to key event. Requested event function not found.")
    return false
  end

  return func(Event:new(state)) -- call with new event object
end

--[[ Helper Func ]]--

local function toIntBoolean(value)
  if value == true then
    return 1
  else
    return 0
  end
end

exports.enable = function(self, moduleConfig, globalConfig)

  --[[ get addresses ]]--

  local asyncKeyStateFuncAddr = core.AOBScan("53 56 57 8b 3d ? ? ? 00 8b f1", 0x400000)
  if asyncKeyStateFuncAddr == nil then
    print("'inputHandler' was unable to find the address of the key state function and the jump address of 'GetAsyncKeyState'.")
    error("'inputHandler' can not be initialized.")
  end
  local asyncKeyFuncJumpAddr = core.readInteger(asyncKeyStateFuncAddr + 5) -- move to address and then read actual address
  
  local keyStateStructAddr = core.AOBScan("89 2d ? ? ? 00 0f 87 ? ? ? ff", 0x400000)
  if keyStateStructAddr == nil then
    print("'inputHandler' was unable to find the key state struct of Crusader.")
    error("'inputHandler' can not be initialized.")
  end
  keyStateStructAddr = core.readInteger(keyStateStructAddr + 2) -- move pointer to address, then read the value
  
  --[[
    Note: Crusader has an additional function that sets key states: 00468a10.
    But the only known reference so far is from a jump table and it is close to some "useless" functions. So maybe unused?
  ]]

  --[[ load module ]]--
  
  local requireTable = require("inputHandler.dll") -- loads the dll in memory and runs luaopen_winProcHandler
  
  for name, addr in pairs(requireTable.funcPtr) do
    self[name] = addr
  end
  
  requireTable.lua_RegisterControlFunc(controlFunc) -- register lua event handler
  
  -- these do not need to be wrapped
  self.LockKeyMap = requireTable.lua_LockKeyMap
  self.ReleaseKeyMap = requireTable.lua_ReleaseKeyMap
  
  -- TODO: there should be a table with keys, maybe
  self.status = status -- status enums, basically

  self.RegisterEvent = function(keyMapName, ctrlActive, shiftActive, altActive, virtualKey, funcToCall)
    local regInt = toIntBoolean(ctrlActive) << 24 | toIntBoolean(shiftActive) << 16 | toIntBoolean(altActive) << 8 | (virtualKey & 0xFF)

    if requireTable.lua_RegisterEvent(keyMapName, regInt) then -- register handle before function
      local mapTable = funcTable[keyMapName]
      if mapTable == nil then
        funcTable[keyMapName] = {}
        mapTable = funcTable[keyMapName]
      end
        
      mapTable[regInt] = funcToCall -- funcs are stored in lua, since I do not want to generate a ton of never lifted references
      return true
    else
      return false
    end
  end
  
  self.DEFAULT_KEY_MAP = ""
  
  --[[ modify code ]]--
  
  -- get main state set function to return, the handler takes care of key ups and resets
  core.writeCode(
    asyncKeyStateFuncAddr,
    {0xC3}  -- ret
  )
  
  -- jump address of crusaders GetAsyncKeyState, replace address with own -> only for one key down call
  core.writeCode(
    asyncKeyFuncJumpAddr,
    {requireTable.funcAddress_GetAsyncKeyState}
  )
  
  -- gives the address of crusaders key state table to the module
  core.writeCode(
    requireTable.address_FillWithKeyStateStructAddr,
    {keyStateStructAddr}
  )
  
  --[[ test code ]]--

  self.RegisterEvent(self.DEFAULT_KEY_MAP, true, true, false, 0x20,
    function(event)
      if event:statusNum() == self.status.KEY_DOWN then
        log(INFO, "I am a secret LUA message.")
      end
      
      return false
    end
  )
end

exports.disable = function(self, moduleConfig, globalConfig) error("not implemented") end

return exports