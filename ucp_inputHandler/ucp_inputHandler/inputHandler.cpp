#pragma once

#include "pch.h"

#include <winProcHandler.h>
#include "inputHandlerInternal.h"

#include "keyEnum.h"
#include <unordered_map>


static std::unordered_map<std::string, KeyMap> keyMapMap{};
static std::pair<const std::string, KeyMap>* currentKeyMapPair{ nullptr };
static std::pair<const std::string, KeyMap>* defaultKeyMapPair{ nullptr };

static std::unordered_map<VK::VirtualKey, const IHH::KeyEventFunc*> currentEvents{};

static CrusaderKeyState nativeState{};  // half of this is not used
static IHH::KeyEventFunc defaultFunc{ RetranslateToWindowProc };  // to be part of pipeline


bool InitStructures()
{
  auto iter{ keyMapMap.try_emplace(IHH::DEFAULT_KEY_MAP).first }; // no test, but it is first and must work
  currentKeyMapPair = &*iter;
  defaultKeyMapPair = currentKeyMapPair;

  // test
  defaultKeyMapPair->second.registerKeyEvent("secretMsgCpp", "Secret Cpp Message", [](IHH::KeyEvent ev, int, HWND) {
    switch (ev.status)
    {
      case IHH::KeyStatus::RESET:
        LuaLog::Log(LuaLog::LOG_INFO, "I am a secret reset.");
        break;
      case IHH::KeyStatus::KEY_DOWN:
        LuaLog::Log(LuaLog::LOG_INFO, "I am a secret key down.");
        break;
      case IHH::KeyStatus::KEY_HOLD:
        LuaLog::Log(LuaLog::LOG_INFO, "I am a secret key hold.");
        break;
      case IHH::KeyStatus::KEY_UP:
        LuaLog::Log(LuaLog::LOG_INFO, "I am a secret key up.");
        break;
      default:
        break;
    }
    return false;
  });

  return true;
}


// only used for one call
SHORT __stdcall GetAsyncKeyFake(int vKey)
{
  if (vKey == VK::DOWN)
  {
    return static_cast<short>(crusaderKeyState->downArrow);
  }
  else
  {
    LuaLog::Log(LuaLog::LOG_FATAL, "[InputHandler]: 'GetAsyncKeyFake' received key it can not handle.");
    return 0;
  }
}


LRESULT __stdcall ProcessInput(int reservedCurrentPrio, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  IHH::KeyStatus action{ IHH::KeyStatus::NONE };
  switch (uMsg)
  {
    case WM_SYSKEYUP:
    case WM_KEYUP:
      action = IHH::KeyStatus::KEY_UP;
      break;
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
      if (wParam == VK::NUMLOCK)
      {
        action = IHH::KeyStatus::RESET; // numlock usage forces reset
      }
      else
      {
        action = lParam & 0x40000000 ? IHH::KeyStatus::KEY_HOLD : IHH::KeyStatus::KEY_DOWN;
      }
      break;
    case WM_ACTIVATEAPP:
      if (wParam) // at the moment only on activation (like all input)
      {
        action = IHH::KeyStatus::RESET;
      }
      break;
    case WM_CHAR:
    default:
      break;
  }

  if (action == IHH::KeyStatus::NONE) // nothing to do, pass through
  {
    return WinProcHeader::CallNextProc(reservedCurrentPrio, hwnd, uMsg, wParam, lParam); // dummy currently
  }


  // a lot safety catching needs to happen here
  // num key changes also leads to locked keys


  if (wParam <= VK::ALT && wParam >= VK::SHIFT)
  {
    BOOL modifier{ action == IHH::KeyStatus::KEY_HOLD || action == IHH::KeyStatus::KEY_DOWN };  // the others will be 0
    switch (wParam)
    {
      case VK::CONTROL:
      {
        crusaderKeyState->ctrl = modifier;
        nativeState.ctrl = modifier;
        break;
      }
      case VK::SHIFT:
      {
        crusaderKeyState->shift = modifier;
        nativeState.shift = modifier;
        break;
      }
      case VK::ALT:
      {
        crusaderKeyState->alt = modifier;
        nativeState.alt = modifier;
        
        // Crusader returns 1 in case of ALT; for the moment do the same
        // (Test with graphicsAPIReplacer was the same, but still keep it)
        // also, a windows sound plays if the graphicsAPIReplacer is used
        return 1;
      }
      default:
        break;
    }

    return 0; // consumed key
  }

  // reset here

  if (action == IHH::KeyStatus::RESET)  // dummy at the moment
  {
    ResetEventsAndKeyState();
    return WinProcHeader::CallNextProc(reservedCurrentPrio, hwnd, uMsg, wParam, lParam); // send further
  }

  // create KeyEvent
  IHH::KeyEvent currentEvent{ static_cast<unsigned char>(wParam), action, static_cast<unsigned int>(nativeState.ctrl),
    static_cast<unsigned int>(nativeState.shift), static_cast<unsigned int>(nativeState.alt) };

  // handling existing actions here

  if (action != IHH::KeyStatus::KEY_DOWN) // not sure if this should be the gate, or rather the key search
  {
    auto runningEvent{ currentEvents.find(static_cast<VK::VirtualKey>(wParam)) };
    if (runningEvent == currentEvents.end())
    {
      return 0;
    }

    bool res{ (*runningEvent->second)(currentEvent, reservedCurrentPrio, hwnd) };
    if (res || action == IHH::KeyStatus::KEY_UP)
    {
      currentEvents.erase(runningEvent);  // currently erasing -> if it turns out this is too slow, maybe switch to some approach that keeps the registered

      if (res)
      {
        return ProcessInput(reservedCurrentPrio, hwnd, WM_KEYDOWN, wParam, 0);  // sets it to a new keyDown, other stuff is context generated
      }
    }
    return 0;
  }

  // only key_downs from here

  const IHH::KeyEventFunc* currentFunc{ currentKeyMapPair->second.getHandlerFunc(currentEvent) };
  if (!currentFunc)
  {
    if (currentKeyMapPair == defaultKeyMapPair)
    {
      currentFunc = &defaultFunc;
    }
    else
    {
      return 0; // special KeyMap selected, devour
    }
  }

  currentEvents.insert_or_assign(static_cast<VK::VirtualKey>(wParam), currentFunc);
  (*currentFunc)(currentEvent, reservedCurrentPrio, hwnd);
  return 0; // consume
}


void ResetEventsAndKeyState()
{
  IHH::KeyEvent resetEvent{ 0, IHH::KeyStatus::RESET, 0, 0, 0 };  // reset does not receive status
  auto iter{ currentEvents.begin() };
  while (iter != currentEvents.end())
  {
    (*iter->second)(resetEvent, 0, 0);  // ignore output, we are resetting
    iter = currentEvents.erase(iter); // delete, since we are done
  }
  // zero key state objects of stronghold
  ZeroMemory(crusaderKeyState, sizeof(CrusaderKeyState));
  ZeroMemory(crusaderArrowKeyState, sizeof(CrusaderArrowKeyState));
}


bool RetranslateToWindowProc(IHH::KeyEvent status, int windowProcPrio, HWND winHandle)
{
  if (status.status == IHH::KeyStatus::RESET)
  {
    return false; // there is no other handling of resets in the default key handler
  }

  crusaderKeyState->ctrl = status.ctrlActive;
  crusaderKeyState->shift = status.shiftActive;
  crusaderKeyState->alt = status.altActive;

  UINT message{ 0 };
  long lParam{ 0 };
  if (status.status == IHH::KeyStatus::KEY_UP)
  {
    message = status.altActive ? WM_SYSKEYUP : WM_KEYUP;

    // the keys are set by the normal func, but the release is done here
    // NOTE, however, that this ignores ONE case where the down arrow state is only gathered through GetAsyncKeyState
    switch (status.virtualKey)
    {
      case VK::DOWN:
        crusaderKeyState->downArrow = 0;
        break;
      case VK::V:
        crusaderKeyState->v = 0;
        break;
      default:
        break;
    }
  }
  else
  {
    message = status.altActive ? WM_SYSKEYDOWN : WM_KEYDOWN;
  }
  if (status.status != IHH::KeyStatus::KEY_DOWN)
  {
    lParam |= 0x40000000; // ignore other parameters for now, they seem unused
  }

  WinProcHeader::CallNextProc(windowProcPrio, winHandle, message, status.virtualKey, lParam);

  crusaderKeyState->ctrl = nativeState.ctrl;
  crusaderKeyState->shift = nativeState.shift;
  crusaderKeyState->alt = nativeState.alt;

  return false; // input is only cared for on KEY_HOLD, freeing the key and sending a key down to the process again, launching the other key
}


const IHH::KeyEventFunc* KeyMap::getHandlerFunc(IHH::KeyEvent ev)
{
  unsigned int mapKey{ev.ctrlActive << 24 | ev.shiftActive << 16 | ev.altActive << 8 | ev.virtualKey }; // duplicate? would try to evade another func call, or not
  
  auto combIter{ keyCombMap.find(mapKey) };
  if (combIter == keyCombMap.end())
  {
    return nullptr;
  }

  auto eventIter{ eventMap.find(combIter->second) };
  if (eventIter == eventMap.end())
  {
    // this may or may not be a bad idea
    LuaLog::Log(LuaLog::LOG_WARNING, "Encountered registered key combination without event.");
    return nullptr;
  }
  return &eventIter->second.eventFunc;
}

// key combinations are overwritten without mercy
bool KeyMap::registerKeyCombination(IHH::KeyEvent structure, const char* eventName)
{
  unsigned int mapKey{ structure.ctrlActive << 24 | structure.shiftActive << 16 |
    structure.altActive << 8 | structure.virtualKey };  // duplicate? would try to evade another func call, or not
  
  auto nameIter{ KeyMap::nameSet.emplace(eventName).first };  // may discard created object, might need to happen anyway
  keyCombMap.insert_or_assign(mapKey, &*nameIter);
  return true;  // could later return reject
}

// event names however need to be unique, so it might get rejected
bool KeyMap::registerKeyEvent(const char* eventName, const char* asciiTitle, IHH::KeyEventFunc&& func)
{
  auto nameIter{ KeyMap::nameSet.emplace(eventName).first };  // may discard created object, might need to happen anyway
  auto res{ eventMap.try_emplace(&*nameIter, asciiTitle, std::forward<IHH::KeyEventFunc>(func)) }; // should i trust the move stuff?
  return res.second;  // true if placed, false if one already present
}

bool KeyMap::registerLuaKeyEvent(const std::string* mapKey, const char* eventName, const char* asciiTitle)
{
  const std::string* namePtr{ &*KeyMap::nameSet.emplace(eventName).first }; // may discard created object, might need to happen anyway
  auto res{ eventMap.try_emplace(namePtr, asciiTitle,
    [mapKey, namePtr](IHH::KeyEvent ev, int, HWND) // should i trust the move stuff?
    {
      return handleLuaEvents(mapKey, namePtr, ev);
    }
  )};
  return res.second;  // true if placed, false if one already present
}


/* exports */

extern "C" __declspec(dllexport) bool __stdcall LockKeyMap(const char* name)
{
  if (currentKeyMapPair != defaultKeyMapPair || strcmp(name, IHH::DEFAULT_KEY_MAP) == 0)  // only if currently default and not default name
  {
    return false;
  }

  auto iter{ keyMapMap.try_emplace(name).first };
  ResetEventsAndKeyState();
  currentKeyMapPair = &*iter;
  return true;
}

extern "C" __declspec(dllexport) bool __stdcall ReleaseKeyMap(const char* name)
{
  if (currentKeyMapPair == defaultKeyMapPair || strcmp(name, IHH::DEFAULT_KEY_MAP) == 0 ) // only if not default and not default name
  {
    return false;
  }

  auto iter{ keyMapMap.find(name) };
  if (iter == keyMapMap.end() || &*iter != currentKeyMapPair) // if this is not the current map, do not release
  {
    return false;
  }

  ResetEventsAndKeyState();
  currentKeyMapPair = defaultKeyMapPair;
  return true;
}

extern "C" __declspec(dllexport) bool __stdcall RegisterKeyComb(const char* keyMapName, bool ctrl, bool shift,
  bool alt, unsigned char virtualKey, const char* eventName)
{
  if (!(keyMapName && eventName)) // nullptr not allowed
  {
    return false;
  }
  auto iter{ keyMapMap.try_emplace(keyMapName).first };

  IHH::KeyEvent createKeyComb{ virtualKey, IHH::KeyStatus::RESET, ctrl, shift, alt }; // reset is just dummy
  return (iter->second).registerKeyCombination(createKeyComb, eventName);
}

extern "C" __declspec(dllexport) bool __stdcall RegisterEvent(const char* keyMapName, const char* eventName,
  const char* asciiTitle, IHH::KeyEventFunc && func)
{
  // nullptr not allowed (hopefully it does not create move problems)
  if (! (keyMapName && eventName && asciiTitle && func))
  {
    return false;
  }
  auto iter{ keyMapMap.try_emplace(keyMapName).first };
  return (iter->second).registerKeyEvent(eventName, asciiTitle, std::forward<IHH::KeyEventFunc>(func));
}



/* LUA */

bool handleLuaEvents(const std::string* mapRef, const std::string* eventName, IHH::KeyEvent ev)
{
  if (luaState == nullptr || luaControlFuncIndex == 0)
  {
    LuaLog::Log(LuaLog::LOG_ERROR, "[inputHandler]: Missing lua state or handling func. Can not handle lua key event.");
    return false;
  }

  lua_rawgeti(luaState, LUA_REGISTRYINDEX, luaControlFuncIndex);
  lua_pushstring(luaState, mapRef->c_str());  // still copies of the string
  lua_pushstring(luaState, eventName->c_str());

  // basically like internal, but uses long long integer by giving the status at another position
  lua_pushinteger(luaState,
    static_cast<long long>(ev.status) << 32 |
    static_cast<long long>(ev.ctrlActive) << 24 |
    static_cast<long long>(ev.shiftActive) << 16 |
    static_cast<long long>(ev.altActive) << 8 |
    static_cast<long long>(ev.virtualKey));

  if (lua_pcall(luaState, 3, 1, 0) != 0)  // call and log potential error (hopefully this does not break anything)
  {
    std::string err{ "[inputHandler]: Lua key event caused error: " };
    err.append(lua_tostring(luaState, -1));
    LuaLog::Log(LuaLog::LOG_ERROR, err.c_str());
    return false;
  }

  if (!lua_isboolean(luaState, -1))
  {
    LuaLog::Log(LuaLog::LOG_ERROR, "[inputHandler]: Lua key event did not return a boolean.");
  }
  bool res{ static_cast<bool>(lua_toboolean(luaState, -1)) };
  lua_pop(luaState, 1);  // pop returned value
  return res;
}

// need to be called with func ptr, will receive the events
extern "C" __declspec(dllexport) int __cdecl lua_RegisterControlFunc(lua_State * L)
{
  int n{ lua_gettop(L) };    /* number of arguments */
  if (n != 1)
  {
    luaL_error(L, "[inputHandler]: lua_RegisterControlFunc: Invalid number of args.");
  }

  if (!lua_isfunction(L, 1))  // only one thing left
  {
    luaL_error(L, "[inputHandler]: lua_RegisterControlFunc: Received no function.");
  }
  // stores index to function, also pops value, currently not removed for lifetime of program

  luaControlFuncIndex = luaL_ref(L, LUA_REGISTRYINDEX);
  return 0;
}

extern "C" __declspec(dllexport) int __cdecl lua_LockKeyMap(lua_State * L)
{
  int n{ lua_gettop(L) };    /* number of arguments */
  if (n != 1)
  {
    luaL_error(L, "[inputHandler]: lua_LockKeyMap: Invalid number of args.");
  }

  if (!lua_isstring(L, 1))
  {
    luaL_error(L, "[inputHandler]: lua_LockKeyMap: Wrong input fields.");
  }

  bool res{ LockKeyMap(lua_tostring(L, 1)) };
  lua_pushboolean(L, res);
  return 1;
}

extern "C" __declspec(dllexport) int __cdecl lua_ReleaseKeyMap(lua_State * L)
{
  int n{ lua_gettop(L) };    /* number of arguments */
  if (n != 1)
  {
    luaL_error(L, "[inputHandler]: lua_ReleaseKeyMap: Invalid number of args.");
  }

  if (!lua_isstring(L, 1))
  {
    luaL_error(L, "[inputHandler]: lua_ReleaseKeyMap: Wrong input fields.");
  }

  bool res{ LockKeyMap(lua_tostring(L, 1)) };
  lua_pushboolean(L, res);
  return 1;
}

extern "C" __declspec(dllexport) int __cdecl lua_RegisterKeyComb(lua_State * L)
{
  int n{ lua_gettop(L) };    /* number of arguments */
  if (n != 3)
  {
    luaL_error(L, "[inputHandler]: lua_RegisterKeyComb: Invalid number of args.");
  }

  if (!(lua_isstring(L, 1) && lua_isinteger(L, 2) && lua_isstring(L, 3)))
  {
    luaL_error(L, "[inputHandler]: lua_RegisterKeyComb: Wrong input fields.");
  }

  // the given integer reflects the register structure
  // it is only very loosely coupled, since overwrites can happen all the time

  unsigned int regInt{ static_cast<unsigned int>(lua_tointeger(L, 2)) };
  bool res{ RegisterKeyComb(lua_tostring(L, 1), regInt & 0x01000000, regInt & 0x00010000,
    regInt & 0x00000100, regInt & 0x000000FF, lua_tostring(L, 3)) };
  lua_pushboolean(L, res);
  return 1;
}

extern "C" __declspec(dllexport) int __cdecl lua_RegisterEvent(lua_State * L)
{
  int n{ lua_gettop(L) };    /* number of arguments */
  if (n != 3)
  {
    luaL_error(L, "[inputHandler]: lua_RegisterEvent: Invalid number of args.");
  }

  if (!(lua_isstring(L, 1) && lua_isstring(L, 2) && lua_isstring(L, 3)))
  {
    luaL_error(L, "[inputHandler]: lua_ReleaseKeyMap: Wrong input fields.");
  }

  auto iter{ keyMapMap.try_emplace(lua_tostring(L, 1)).first };
  bool res{ iter->second.registerLuaKeyEvent(&iter->first, lua_tostring(L, 2), lua_tostring(L, 3)) };
  lua_pushboolean(L, res);
  return 1;
}

// TODO?: C endpoint? or rather only for input module
extern "C" __declspec(dllexport) int __cdecl lua_RegisterKeyAlias(lua_State * L)
{
  int n{ lua_gettop(L) };    /* number of arguments */
  if (n != 3)
  {
    luaL_error(L, "[inputHandler]: lua_RegisterKeyAlias: Invalid number of args.");
  }

  if (!(lua_isstring(L, 1) && lua_isinteger(L, 2) && lua_isstring(L, 3)))
  {
    luaL_error(L, "[inputHandler]: lua_RegisterKeyAlias: Wrong input fields.");
  }

  auto iter{ keyMapMap.try_emplace(lua_tostring(L, 1)).first };
  bool res{ iter->second.registerKeyEvent(lua_tostring(L, 3), lua_tostring(L, 3), 
    [toEv{ lua_tointeger(L, 2) }](IHH::KeyEvent ev, int prio, HWND hwnd) // should i trust the move stuff?
    {
      ev.ctrlActive = toEv & 0x01000000 ? 1 : 0;
      ev.shiftActive = toEv & 0x00010000 ? 1 : 0;
      ev.altActive = toEv & 0x00000100 ? 1 : 0;
      ev.virtualKey = toEv & 0x000000FF;
      return RetranslateToWindowProc(ev, prio, hwnd);
    } 
  )};
  lua_pushboolean(L, res);
  return 1;
}