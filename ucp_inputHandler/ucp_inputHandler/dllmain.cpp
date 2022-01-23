
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

  lua_newtable(L); // push a new table on the stack

  // simple replace
  lua_pushinteger(L, (DWORD)GetAsyncKeyFake);
  lua_setfield(L, -2, "funcAddress_GetAsyncKeyState");

  // address
  lua_pushinteger(L, (DWORD)&crusaderKeyState);
  lua_setfield(L, -2, "address_FillWithKeyStateStructAddr");

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