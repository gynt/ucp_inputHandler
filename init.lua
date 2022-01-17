local exports = {}

exports.enable = function(self, moduleConfig, globalConfig)

  --[[ load module ]]--
  
  local requireTable = require("inputHandler.dll") -- loads the dll in memory and runs luaopen_winProcHandler

end

exports.disable = function(self, moduleConfig, globalConfig) error("not implemented") end

return exports