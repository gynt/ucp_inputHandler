local exports = {}


--[[ LookUpTables ]]--

local modifier = {
  SHIFT                 = 0x10,
  CONTROL               = 0x11,
  ALT                   = 0x12,
}

local invModifier = {} -- reverse, int to key string
for keyName, keyId in pairs(invModifier) do
  invModifier[keyId] = keyName
end

local keys = {
  -- NONE                  = 0x00, -- not valid, for invalid stuff
  -- LEFT_MOUSE_BUTTON     = 0x01, -- currently no mouse
  -- RIGHT_MOUSE_BUTTON    = 0x02, -- currently no mouse
  CANCEL                = 0x03,
  -- MIDDLE_MOUSE_BUTTON   = 0x04, -- currently no mouse
  -- EXTRA_MOUSE_BUTTON1   = 0x05, -- currently no mouse
  -- EXTRA_MOUSE_BUTTON2   = 0x06, -- currently no mouse
  BACKSPACE             = 0x08,
  TAB                   = 0x09,
  CLEAR                 = 0x0C,
  ENTER                 = 0x0D,
  -- SHIFT                 = 0x10, -- modifier
  -- CONTROL               = 0x11, -- modifier
  -- ALT                   = 0x12, -- modifier, also called menu
  PAUSE                 = 0x13,
  -- CAPSLOCK              = 0x14, -- state key
  ESCAPE                = 0x1B,
  SPACE                 = 0x20,
  PAGE_UP               = 0x21,
  PAGE_DOWN             = 0x22,
  END                   = 0x23,
  HOME                  = 0x24,
  LEFT                  = 0x25,
  UP                    = 0x26,
  RIGHT                 = 0x27,
  DOWN                  = 0x28,
  SELECT                = 0x29,
  PRINT                 = 0x2A,
  EXECUTE               = 0x2B,
  PRINT_SCREEN          = 0x2C, -- german keys: Druck
  INSERT                = 0x2D,
  DELETE_KEY            = 0x2E,
  HELP                  = 0x2F, -- numbers
  ZERO                  = 0x30,
  ONE                   = 0x31,
  TWO                   = 0x32,
  THREE                 = 0x33,
  FOUR                  = 0x34,
  FIVE                  = 0x35,
  SIX                   = 0x36,
  SEVEN                 = 0x37,
  EIGHT                 = 0x38,
  NINE                  = 0x39,
  A                     = 0x41,
  B                     = 0x42,
  C                     = 0x43,
  D                     = 0x44,
  E                     = 0x45,
  F                     = 0x46,
  G                     = 0x47,
  H                     = 0x48,
  I                     = 0x49,
  J                     = 0x4A,
  K                     = 0x4B,
  L                     = 0x4C,
  M                     = 0x4D,
  N                     = 0x4E,
  O                     = 0x4F,
  P                     = 0x50,
  Q                     = 0x51,
  R                     = 0x52,
  S                     = 0x53,
  T                     = 0x54,
  U                     = 0x55,
  V                     = 0x56,
  W                     = 0x57,
  X                     = 0x58,
  Y                     = 0x59,
  Z                     = 0x5A,
  LEFT_WINDOWS_KEY      = 0x5B, -- reported as additional key, but as left (not general), so right should report also as such
  RIGHT_WINDOWS_KEY     = 0x5C,
  APPLICATIONS_KEY      = 0x5D,
  SLEEP                 = 0x5F,
  NUMPAD0               = 0x60,
  NUMPAD1               = 0x61,
  NUMPAD2               = 0x62,
  NUMPAD3               = 0x63,
  NUMPAD4               = 0x64,
  NUMPAD5               = 0x65,
  NUMPAD6               = 0x66,
  NUMPAD7               = 0x67,
  NUMPAD8               = 0x68,
  NUMPAD9               = 0x69,
  MULTIPLY              = 0x6A, -- num
  ADD                   = 0x6B, -- num
  SEPERATOR             = 0x6C, -- num
  SUBTRACT              = 0x6D, -- num
  DECIMAL               = 0x6E, -- german keys: num comma
  DIVIDE                = 0x6F, -- num
  F1                    = 0x70,
  F2                    = 0x71,
  F3                    = 0x72,
  F4                    = 0x73,
  F5                    = 0x74,
  F6                    = 0x75,
  F7                    = 0x76,
  F8                    = 0x77,
  F9                    = 0x78,
  F10                   = 0x79,
  F11                   = 0x7A,
  F12                   = 0x7B,
  F13                   = 0x7C,
  F14                   = 0x7D,
  F15                   = 0x7E,
  F16                   = 0x7F,
  F17                   = 0x80,
  F18                   = 0x81,
  F19                   = 0x82,
  F20                   = 0x83,
  F21                   = 0x84,
  F22                   = 0x85,
  F23                   = 0x86,
  F24                   = 0x87,
  -- NUMLOCK               = 0x90, -- state key
  -- SCROLL_LOCK           = 0x91, -- state key
  -- LEFT_SHIFT            = 0xA0, -- reported as shift, also modifier
  -- RIGHT_SHIFT           = 0xA1, -- reported as shift, also modifier
  -- LEFT_CONTROL          = 0xA2, -- reported as ctrl, also modifier
  -- RIGHT_CONTROL         = 0xA3, -- reported as ctrl, also modifier
  -- LEFT_MENU             = 0xA4, -- left alt (reported as alt additional key flag), also modifier
  -- RIGHT_MENU            = 0xA5, -- right alt (german keyboard reports as ctrl + alt(with addititional key flag), also modifier
  BROWSER_BACK          = 0xA6,
  BROWSER_FORWARD       = 0xA7,
  BROWSER_REFRESH       = 0xA8,
  BROWSER_STOP          = 0xA9,
  BROWSER_SEARCH        = 0xAA,
  BROWSER_FAVORITES     = 0xAB,
  BROWSER_HOME          = 0xAC,
  VOLUME_MUTE           = 0xAD,
  VOLUME_DOWN           = 0xAE,
  VOLUME_UP             = 0xAF,
  NEXT_TRACK            = 0xB0,
  PREVIOUS_TRACK        = 0xB1,
  STOP_MEDIA            = 0xB2,
  PLAY_PAUSE            = 0xB3,
  LAUNCH_MAIL           = 0xB4,
  SELECT_MEDIA          = 0xB5,
  LAUNCH_APP1           = 0xB6,
  LAUNCH_APP2           = 0xB7,
  OEM1                  = 0xBA, -- german keys: ü
  OEM_PLUS              = 0xBB, -- non num versions
  OEM_COMMA             = 0xBC,
  OEM_MINUS             = 0xBD,
  OEM_PERIOD            = 0xBE,
  OEM2                  = 0xBF, -- german keys: #
  OEM3                  = 0xC0, -- german keys: ö
  OEM4                  = 0xDB, -- german keys: ß
  OEM5                  = 0xDC, -- german keys: ZIRKUMFLEX
  OEM6                  = 0xDD, -- german keys: ´
  OEM7                  = 0xDE, -- german keys: ä
  OEM8                  = 0xDF,
  OEM102                = 0xE2, -- german keys: <
  PROCESS               = 0xE5,
  PACKET                = 0xE7,
  ATTN                  = 0xF6,
  CRSEL                 = 0xF7,
  EXSEL                 = 0xF8,
  ERASEEOF              = 0xF9,
  PLAY                  = 0xFA,
  ZOOM                  = 0xFB,
  PA1                   = 0xFD,
  OEM_CLEAR             = 0xFE,
}

local invKeys = {} -- reverse, int to key string
for keyName, keyId in pairs(keys) do
  invKeys[keyId] = keyName
end

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

local function controlFunc(keyMapName, eventFuncName, state)
  local mapTable = funcTable[keyMapName]
  if mapTable == nil then
    log(WARNING, "[inputHandler]: Unable to react to key event. Requested key table not found.")
    return false
  end
  
  local func = mapTable[eventFuncName]
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


-- throws if not valid
local function keyStringToInfo(keyStr)

  -- split func taken from here: https://stackoverflow.com/a/7615129
  -- however, from the comments
  local strkeys = {}
  local count = 0
  for field, s in string.gmatch(keyStr, "([^%+]*)(%+?)") do -- "([^"..sep.."]*)("..sep.."?)" for generic separator, % is escape
    count = count + 1
    strkeys[count] = field
    if s == "" then
      break
    end
  end
  
  if count < 1 or count > 4 then
    error("Invalid number of keys for combination.", 0)
  end
  
  local key = keys[strkeys[count]]
  if key == nil then
    error("Invalid main key.", 0)
  end
  
  local ctrlState = nil
  local shiftState = nil
  local altState = nil
  
  if count ~= 1 then
    for i = 1, count - 1 do
      local mod = modifier[strkeys[i]]
      if mod == nil then
        error("Invalid modifier key.", 0)
      end
      
      if mod == modifier.CONTROL then
        if ctrlState ~= nil then
          error("Ctrl set twice.", 0)
        else
          ctrlState = true
        end
      elseif mod == modifier.SHIFT then
        if shiftState ~= nil then
          error("Shift set twice.", 0)
        else
          shiftState = true
        end
      elseif mod == modifier.ALT then
        if altState ~= nil then
          error("Alt set twice.", 0)
        else
          altState = true
        end
      end
    end
  end
  
  if ctrlState == nil then
    ctrlState = false
  end
  if shiftState == nil then
    shiftState = false
  end
  if altState == nil then
    altState = false
  end

  return {
    ["ctrl"]    = ctrlState,
    ["shift"]   = shiftState,
    ["alt"]     = altState,
    ["key"]     = key,
  }
end


--[[ Main Func ]]--

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
  
  local arrowKeyStructAddr = core.AOBScan("ff 24 85 ? ? ? 00 89 1d ? ? ? 01 e9 ? ? ? ff", 0x400000)
  if arrowKeyStructAddr == nil then
    print("'inputHandler' was unable to find the arrow key struct of Crusader.")
    error("'inputHandler' can not be initialized.")
  end
  arrowKeyStructAddr = core.readInteger(arrowKeyStructAddr + 9) -- move pointer to address, then read the value
  
  --[[
    Note: Crusader has an additional function that sets key states: 00468a10.
    But the only known reference so far is from a jump table and it is close to some "useless" functions. So maybe unused?
  ]]

  --[[ load module ]]--
  
  local requireTable = require("inputHandler.dll") -- loads the dll in memory and runs luaopen_winProcHandler
  
  for name, addr in pairs(requireTable.funcPtr) do
    self[name] = addr
  end

  self.DEFAULT_KEY_MAP = ""
  self.status = status -- status enums, basically; should not be changed (but they could)
  self.keys = keys -- key enums, basically; should not be changed (but they could)
  self.invKeys = invKeys -- reverse, int to key string
  self.modifier = modifier -- modifier keys
  self.invModifier = invModifier -- inverse modifier keys
  
  requireTable.lua_RegisterControlFunc(controlFunc) -- register lua event handler
  
  -- these do not need to be wrapped
  self.LockKeyMap = requireTable.lua_LockKeyMap
  self.ReleaseKeyMap = requireTable.lua_ReleaseKeyMap

  self.RegisterEvent = function(keyMapName, eventName, asciiTitle, funcToCall)
    if requireTable.lua_RegisterEvent(keyMapName, eventName, asciiTitle) then -- register handle before function
      local mapTable = funcTable[keyMapName]
      if mapTable == nil then
        funcTable[keyMapName] = {}
        mapTable = funcTable[keyMapName]
      end
        
      mapTable[eventName] = funcToCall -- funcs are stored in lua, since I do not want to generate a ton of never lifted references
      return true
    else
      return false
    end
  end

  self.RegisterKeyComb = function(keyMapName, ctrlActive, shiftActive, altActive, virtualKey, eventName)
    local regInt = toIntBoolean(ctrlActive) << 24 | toIntBoolean(shiftActive) << 16 | toIntBoolean(altActive) << 8 | (virtualKey & 0xFF)
    return requireTable.lua_RegisterKeyComb(keyMapName, regInt, eventName)
  end
  
  self.RegisterKeyCombStr = function(keyMapName, eventName, keyStr)
    local status, res = pcall(keyStringToInfo, keyStr)
    if not status then
      log(WARNING, "[inputHandler]: Key combination error: " .. keyStr .. ": " .. res)
      return false
    end
    
    if not self.RegisterKeyComb(keyMapName, res.ctrl, res.shift, res.alt, res.key, eventName) then
      log(WARNING, "[inputHandler]: Unable to register key combination '" .. keyStr .. "' for '" .. eventName .. "' in key map: " .. keyMapName)
      return false
    end
    return true
  end
  
  self.RegisterKeyAlias = function(ofStr, isStr)
  
    local statusOf, resOf = pcall(keyStringToInfo, ofStr)
    if not statusOf then
      log(WARNING, "[inputHandler]: Key 'alias of' error: " .. ofStr .. ": " .. resOf)
      return false
    end
    
    local statusIs, resIs = pcall(keyStringToInfo, isStr)
    if not statusIs then
      log(WARNING, "[inputHandler]: Key 'alias is' error: " .. isStr .. ": " .. resIs)
      return false
    end
    
    local ofKeys = toIntBoolean(resOf.ctrl) << 24 | toIntBoolean(resOf.shift) << 16 | toIntBoolean(resOf.alt) << 8 | (resOf.key & 0xFF)
    requireTable.lua_RegisterKeyAlias(self.DEFAULT_KEY_MAP, ofKeys, ofStr) -- ret not important, since if it fails, we assume already present
    
    if not self.RegisterKeyComb(self.DEFAULT_KEY_MAP, resIs.ctrl, resIs.shift, resIs.alt, resIs.key, ofStr) then
      log(WARNING, "[inputHandler]: Unable to register key alias key combination: " .. ofStr .. " to " .. toStr)
      return false
    end
    return true
  end
  

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
  
  -- gives the address of crusaders key state struct to the module
  core.writeCode(
    requireTable.address_FillWithKeyStateStructAddr,
    {keyStateStructAddr}
  )
  
  -- gives the address of crusaders arrow key state struct to the module
  core.writeCode(
    requireTable.address_FillWithArrowKeyStateStructAddr,
    {arrowKeyStructAddr}
  )
  
  
  --[[ use config ]]--
  
  if moduleConfig.alias ~= nil then
    for keyComb, shouldBe in pairs(moduleConfig.alias) do
      self.RegisterKeyAlias(shouldBe, keyComb)
    end
  end
  
  if moduleConfig.functions ~= nil then
    for keyMapName, funcComb in pairs(moduleConfig.functions) do
      for comb, funcName in pairs(funcComb) do
        self.RegisterKeyCombStr(keyMapName, funcName, comb)
      end
    end
  end


  --[[ test code ]]--

  self.RegisterEvent(self.DEFAULT_KEY_MAP, "secretMsgLua", "Secret Lua Message",
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