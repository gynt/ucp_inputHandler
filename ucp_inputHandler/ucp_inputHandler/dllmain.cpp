
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
    luaL_error(L, "Input handler failed to receive Log functions.");
  }

  if (!WinProcHeader::initModuleFunctions(L))
  {
    luaL_error(L, "Input handler failed to receive WinProcHandler functions.");
  }
  WinProcHeader::RegisterProc(ProcessInput, -50000);

  lua_newtable(L); // push a new table on the stack

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