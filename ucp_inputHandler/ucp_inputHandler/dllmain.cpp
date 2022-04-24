
#include "pch.h"

// lua
#include "lua.hpp"

#include <winProcHandler.h>
#include "inputHandlerInternal.h"

// lua module load
extern "C" __declspec(dllexport) int __cdecl luaopen_inputHandler(lua_State * L)
{
  if (!LuaLog::initLog(L))
  {
    luaL_error(L, "[InputHandler]: Failed to receive Log functions.");
  }

  if (!WinProcHeader::initModuleFunctions(L))
  {
    luaL_error(L, "[InputHandler]: Failed to receive WinProcHandler functions.");
  }
  WinProcHeader::RegisterProc(ProcessInput, -50000);

  if (!InitStructures())
  {
    LuaLog::Log(LuaLog::LOG_FATAL, "[InputHandler]: Failed to initialize the handler.");
  }

  luaState = L; // keep ref to lua

  lua_newtable(L); // push a new table on the stack

  // simple replace
  lua_pushinteger(L, (DWORD)GetAsyncKeyFake);
  lua_setfield(L, -2, "funcAddress_GetAsyncKeyState");

  // address
  lua_pushinteger(L, (DWORD)&crusaderKeyState);
  lua_setfield(L, -2, "address_FillWithKeyStateStructAddr");

  // address
  lua_pushinteger(L, (DWORD)&crusaderArrowKeyState);
  lua_setfield(L, -2, "address_FillWithArrowKeyStateStructAddr");

  // add functions
  lua_newtable(L); // push function table
  lua_pushinteger(L, (DWORD)LockKeyMap);
  lua_setfield(L, -2, InputHandlerHeader::NAME_LOCK_KEY_MAP);
  lua_pushinteger(L, (DWORD)ReleaseKeyMap);
  lua_setfield(L, -2, InputHandlerHeader::NAME_RELEASE_KEY_MAP);
  lua_pushinteger(L, (DWORD)RegisterKeyComb);
  lua_setfield(L, -2, InputHandlerHeader::NAME_REGISTER_KEY_COMB);
  lua_pushinteger(L, (DWORD)RegisterEvent);
  lua_setfield(L, -2, InputHandlerHeader::NAME_REGISTER_EVENT);
  lua_pushinteger(L, (DWORD)RegisterRawEvent);
  lua_setfield(L, -2, InputHandlerHeader::NAME_REGISTER_RAW_EVENT);

  // add table
  lua_setfield(L, -2, "funcPtr");

  // return lua funcs

  lua_pushcfunction(L, lua_RegisterControlFunc);
  lua_setfield(L, -2, "lua_RegisterControlFunc");

  lua_pushcfunction(L, lua_LockKeyMap);
  lua_setfield(L, -2, "lua_LockKeyMap");

  lua_pushcfunction(L, lua_ReleaseKeyMap);
  lua_setfield(L, -2, "lua_ReleaseKeyMap");

  lua_pushcfunction(L, lua_RegisterKeyComb);
  lua_setfield(L, -2, "lua_RegisterKeyComb");

  lua_pushcfunction(L, lua_RegisterEvent);
  lua_setfield(L, -2, "lua_RegisterEvent");

  lua_pushcfunction(L, lua_RegisterKeyAlias);
  lua_setfield(L, -2, "lua_RegisterKeyAlias");

  return 1;
}

// entry point
BOOL APIENTRY DllMain(HMODULE,
  DWORD  ul_reason_for_call,
  LPVOID
)
{
  switch (ul_reason_for_call)
  {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
      break;
  }
  return TRUE;
}