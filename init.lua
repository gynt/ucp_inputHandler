local exports = {}

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

end

exports.disable = function(self, moduleConfig, globalConfig) error("not implemented") end

return exports