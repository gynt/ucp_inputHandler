#pragma once

#include <unordered_map>
#include <unordered_set>
#include <functional>

#include "keyEnum.h"
#include "inputHandlerHeader.h"

namespace IHH = InputHandlerHeader; // easier to use

/* classes and objects */

struct CrusaderKeyState
{
  BOOL unknown;   // for examples set to 1 if alt encountered, usage unknown, maybe debug originally?
  BOOL ctrl;
  BOOL shift;
  BOOL alt;
  BOOL downArrow;
  BOOL v;
};

// used to reset also them
struct CrusaderArrowKeyState
{
  BOOL right;
  BOOL left;
  BOOL down;
  BOOL up;
};

struct KeyEventReceiver
{
  const std::string asciiTitle;
  const IHH::KeyEventFunc eventFunc;

  KeyEventReceiver(const char* asciiTitle, IHH::KeyEventFunc&& func) :
    asciiTitle{ asciiTitle }, eventFunc{ std::forward<IHH::KeyEventFunc>(func) } {};
  ~KeyEventReceiver() {};
};


class KeyMap
{
private:
  inline static std::unordered_set<std::string> nameSet{};  // will contain names

  // a delete is more complicated and involves invalidating all stored ptr
  // at the moment ignored, but maybe for later
  std::unordered_map<const std::string*, KeyEventReceiver> eventMap{};
  std::unordered_map<unsigned int, const std::string*> keyCombMap{};

public:
  KeyMap() {};
  ~KeyMap() {};

  const IHH::KeyEventFunc* getHandlerFunc(IHH::KeyEvent ev);

  // key combinations are overwritten without mercy, but maybe the keys are invalid
  bool registerKeyCombination(IHH::KeyEvent structure, const char* eventName);

  // event names however need to be unique, so it might get rejected
  bool registerKeyEvent(const char* eventName, const char* asciiTitle, IHH::KeyEventFunc&& func);

  // treated differently, to avoid double querying
  bool registerLuaKeyEvent(const std::string* mapKey, const char* eventName, const char* asciiTitle);
};

/* variables*/

inline CrusaderKeyState* crusaderKeyState{ nullptr };
inline CrusaderArrowKeyState* crusaderArrowKeyState{ nullptr };

inline lua_State* luaState{ nullptr };  // needed for lua API
inline int luaControlFuncIndex{ 0 };

/* functions */

bool InitStructures();

SHORT __stdcall GetAsyncKeyFake(int vKey);

LRESULT __stdcall ProcessInput(int reservedCurrentPrio, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void ResetEventsAndKeyState();

bool RetranslateToWindowProc(IHH::KeyEvent status, int windowProcPrio, HWND winHandle);

/* exports */

extern "C" __declspec(dllexport) bool __stdcall LockKeyMap(const char* name);
extern "C" __declspec(dllexport) bool __stdcall ReleaseKeyMap(const char* name);
extern "C" __declspec(dllexport) bool __stdcall RegisterKeyComb(const char* keyMapName, bool ctrl, bool shift,
  bool alt, unsigned char virtualKey, const char* eventName);
extern "C" __declspec(dllexport) bool __stdcall RegisterEvent(const char* keyMapName, const char* eventName,
  const char* asciiTitle, IHH::KeyEventFunc&& func);
extern "C" __declspec(dllexport) bool __stdcall RegisterRawEvent(const char* keyMapName, const char* eventName,
  const char* asciiTitle, IHH::RawKeyEventFunc* func);

/* LUA */

bool handleLuaEvents(const std::string* mapRef, const std::string* eventName, IHH::KeyEvent ev);

// need to be called with func ptr, will receive the events
extern "C" __declspec(dllexport) int __cdecl lua_RegisterControlFunc(lua_State * L);

extern "C" __declspec(dllexport) int __cdecl lua_LockKeyMap(lua_State * L);
extern "C" __declspec(dllexport) int __cdecl lua_ReleaseKeyMap(lua_State * L);
extern "C" __declspec(dllexport) int __cdecl lua_RegisterKeyComb(lua_State * L);
extern "C" __declspec(dllexport) int __cdecl lua_RegisterEvent(lua_State * L);

extern "C" __declspec(dllexport) int __cdecl lua_RegisterKeyAlias(lua_State * L);