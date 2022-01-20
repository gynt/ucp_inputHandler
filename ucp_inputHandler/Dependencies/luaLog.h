
#ifndef LUA_LOG
#define LUA_LOG

// lua
#include "lua.hpp"
#include <functional>

namespace LuaLog
{
  // normal enum, to allow easier transform to int 
  enum LogLevel
  {
    LOG_NONE = 99, // for control stuff in the dll
    LOG_FATAL = -3,
    LOG_ERROR = -2,
    LOG_WARNING = -1,
    LOG_INFO = 0,
    LOG_DEBUG = 1,
  };

  using LogFunc = std::function<void(LogLevel, const char*)>;
  inline LogFunc Log{ nullptr };

  inline bool initLog(lua_State* L)
  {
    if (Log)
    {
      return true;  // already received, so hopefully this stays
    }

    lua_getglobal(L, "log");  // get log
    if (!lua_isfunction(L, -1))
    {
      lua_pop(L, 1);
      return false;
    }
    // stores index to function, also pops value, currently not removed for lifetime of program
    int luaLogFuncIndex{ luaL_ref(L, LUA_REGISTRYINDEX) };

    Log = [L, luaLogFuncIndex](LogLevel level, const char* message)
    {
      lua_rawgeti(L, LUA_REGISTRYINDEX, luaLogFuncIndex);
      lua_pushinteger(L, level);
      lua_pushstring(L, message);
      lua_call(L, 2, 0);
    };
    return true;
  }
}

#endif //LUA_LOG