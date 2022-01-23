#pragma once

#include <unordered_map>
#include <functional>

#include "keyEnum.h"
#include "inputHandlerHeader.h"

namespace IHH = InputHandlerHeader; // easier to use

/* classes and objects */

struct CrusaderKeyState
{
	BOOL unknown;		// for examples set to 1 if alt encountered, usage unknown, maybe debug originally?
	BOOL ctrl;
	BOOL shift;
	BOOL alt;
	BOOL downArrow;
	BOOL v;
};

class KeyMap
{
private:
	std::unordered_map<unsigned int, IHH::KeyEventFunc> funcMap{};

public:
	KeyMap() {};
	~KeyMap() {};

	IHH::KeyEventFunc* getHandlerFunc(IHH::KeyEvent ev);
	bool registerKeyEvent(IHH::KeyEvent ev, IHH::KeyEventFunc&& func);
};

/* variables*/

inline CrusaderKeyState* crusaderKeyState{ nullptr };

inline lua_State* luaState{ nullptr };	// needed for lua API
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
extern "C" __declspec(dllexport) bool __stdcall RegisterEvent(const char* keyMapName, bool ctrl, bool shift,
	bool alt, unsigned char virtualKey, IHH::KeyEventFunc&& func);

/* LUA */

bool handleLuaEvents(const char* mapRef, unsigned int mapInt, IHH::KeyEvent ev);

// need to be called with func ptr, will receive the events
extern "C" __declspec(dllexport) int __cdecl lua_RegisterControlFunc(lua_State * L);

extern "C" __declspec(dllexport) int __cdecl lua_LockKeyMap(lua_State * L);
extern "C" __declspec(dllexport) int __cdecl lua_ReleaseKeyMap(lua_State * L);
extern "C" __declspec(dllexport) int __cdecl lua_RegisterEvent(lua_State * L);