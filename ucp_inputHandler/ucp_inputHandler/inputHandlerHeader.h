
#ifndef INPUT_HANDLER_HEADER
#define INPUT_HANDLER_HEADER

#include <functional>
#include <lua.hpp>

namespace InputHandlerHeader
{
  enum class KeyStatus : unsigned char
  {
    NONE = 255, // unused for events, only used in managing
    RESET = 0,  // reset is not consumed by this module and does not receive a valid windowProc priority, HWND or key states
    KEY_DOWN = 1,
    KEY_HOLD = 2,
    KEY_UP = 3
  };

  struct KeyEvent
  {
    unsigned char virtualKey : 8;
    KeyStatus status : 2;
    unsigned int ctrlActive : 1;
    unsigned int shiftActive : 1;
    unsigned int altActive : 1;
  };

  using RawKeyEventFunc = bool(KeyEvent, int windowProcPrio, HWND winHandle);
  using KeyEventFunc = std::function<RawKeyEventFunc>;

  
  // Cpp API

  using FuncKeyMap = bool(__stdcall*)(const char* name);
  using FuncRegisterKeyComb = bool(__stdcall*)(const char* keyMapName, bool ctrl, bool shift,
    bool alt, unsigned char virtualKey, const char* eventName);
  using FuncRegisterEvent = bool(__stdcall*)(const char* keyMapName, const char* eventName,
    const char* asciiTitle, KeyEventFunc&& func);
  using FuncRegisterRawEvent = bool(__stdcall*)(const char* keyMapName, const char* eventName,
    const char* asciiTitle, RawKeyEventFunc* func);

  inline constexpr char const* NAME_VERSION{ "0.1.0" };

  inline constexpr char const* NAME_MODULE{ "inputHandler" };
  inline constexpr char const* NAME_LOCK_KEY_MAP{ "_LockKeyMap@4" };
  inline constexpr char const* NAME_RELEASE_KEY_MAP{ "_ReleaseKeyMap@4" };
  inline constexpr char const* NAME_REGISTER_KEY_COMB{ "_RegisterKeyComb@24" };
  inline constexpr char const* NAME_REGISTER_EVENT{ "_RegisterEvent@16" };
  inline constexpr char const* NAME_REGISTER_RAW_EVENT{ "_RegisterRawEvent@16" };

  inline FuncKeyMap LockKeyMap{ nullptr };
  inline FuncKeyMap ReleaseKeyMap{ nullptr };
  inline FuncRegisterKeyComb RegisterKeyComb{ nullptr };
  inline FuncRegisterEvent RegisterEvent{ nullptr };
  inline FuncRegisterRawEvent RegisterRawEvent{ nullptr };

  inline constexpr char const* DEFAULT_KEY_MAP{ "" };

  // returns true if the function variables of this header were successfully filled
  inline bool initModuleFunctions(lua_State* L)
  {
    if (LockKeyMap && ReleaseKeyMap && RegisterKeyComb && RegisterEvent && RegisterRawEvent) // assumed to not change during runtime
    {
      return true;
    }

    if (lua_getglobal(L, "modules") != LUA_TTABLE)
    {
      lua_pop(L, 1);  // remove value
      return false;
    }

    if (lua_getfield(L, -1, NAME_MODULE) != LUA_TTABLE)
    {
      lua_pop(L, 2);  // remove table and value
      return false;
    }

    LockKeyMap = (lua_getfield(L, -1, NAME_LOCK_KEY_MAP) == LUA_TNUMBER) ? (FuncKeyMap)lua_tointeger(L, -1) : nullptr;
    lua_pop(L, 1);  // remove value
    ReleaseKeyMap = (lua_getfield(L, -1, NAME_RELEASE_KEY_MAP) == LUA_TNUMBER) ? (FuncKeyMap)lua_tointeger(L, -1) : nullptr;
    lua_pop(L, 1);  // remove value
    RegisterKeyComb = (lua_getfield(L, -1, NAME_REGISTER_KEY_COMB) == LUA_TNUMBER) ? (FuncRegisterKeyComb)lua_tointeger(L, -1) : nullptr;
    lua_pop(L, 1);  // remove value
    RegisterEvent = (lua_getfield(L, -1, NAME_REGISTER_EVENT) == LUA_TNUMBER) ? (FuncRegisterEvent)lua_tointeger(L, -1) : nullptr;
    lua_pop(L, 1);  // remove value
    RegisterRawEvent = (lua_getfield(L, -1, NAME_REGISTER_RAW_EVENT) == LUA_TNUMBER) ? (FuncRegisterRawEvent)lua_tointeger(L, -1) : nullptr;
    lua_pop(L, 3);  // remove value and all tables

    return LockKeyMap && ReleaseKeyMap && RegisterKeyComb && RegisterEvent && RegisterRawEvent;
  }
}

#endif //INPUT_HANDLER_HEADER