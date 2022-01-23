#pragma once

#include "pch.h"

#include <winProcHandler.h>
#include "inputHandlerInternal.h"

#include "keyEnum.h"
#include <unordered_map>

static std::unordered_map<std::string, KeyMap> keyMapMap{};
static KeyMap* currentKeyMap{ nullptr };
static KeyMap* defaultKeyMap{ nullptr };

static std::unordered_map<VK::VirtualKey, IHH::KeyEventFunc*> currentEvents{};

static CrusaderKeyState nativeState{};	// half of this is not used
static IHH::KeyEventFunc defaultFunc{ RetranslateToWindowProc };	// to be part of pipeline


bool InitStructures()
{
	auto iter{ keyMapMap.try_emplace(IHH::DEFAULT_KEY_MAP).first };	// no test, but it is first and must work
	currentKeyMap = &iter->second;
	defaultKeyMap = currentKeyMap;

	// test
	IHH::KeyEvent ev{ VK::SPACE, IHH::KeyStatus::RESET, 1, 1, 1};
	defaultKeyMap->registerKeyEvent(ev, [](IHH::KeyEvent, int, HWND) {
		LuaLog::Log(LuaLog::LOG_INFO, "I am a secret message.");
		return false;
	});

	// here would be the place to create the redefines
	// other funcs need to register themselves

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
			action = lParam & 0x40000000 ? IHH::KeyStatus::KEY_HOLD : IHH::KeyStatus::KEY_DOWN;
			break;
		case WM_ACTIVATEAPP:
			if (wParam)	// at the moment only on activation (like all input)
			{
				action = IHH::KeyStatus::RESET;
			}
			break;
		case WM_CHAR:
		default:
			break;
	}

	if (action == IHH::KeyStatus::NONE)	// nothing to do, pass through
	{
		return WinProcHeader::CallNextProc(reservedCurrentPrio, hwnd, uMsg, wParam, lParam); // dummy currently
	}


	// a lot safety catching needs to happen here



	if (wParam <= VK::ALT && wParam >= VK::SHIFT)
	{
		BOOL modifier{ action == IHH::KeyStatus::KEY_HOLD || action == IHH::KeyStatus::KEY_DOWN };	// the others will be 0
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

	if (action == IHH::KeyStatus::RESET)	// dummy at the moment
	{
		ResetEventsAndKeyState();
		return WinProcHeader::CallNextProc(reservedCurrentPrio, hwnd, uMsg, wParam, lParam); // send further
	}

	// create KeyEvent
	IHH::KeyEvent currentEvent{ static_cast<unsigned char>(wParam), action, static_cast<unsigned int>(nativeState.ctrl),
		static_cast<unsigned int>(nativeState.shift), static_cast<unsigned int>(nativeState.alt) };

	// handling existing actions here

	if (action != IHH::KeyStatus::KEY_DOWN)	// not sure if this should be the gate, or rather the key search
	{
		auto runningEvent{ currentEvents.find(static_cast<VK::VirtualKey>(wParam)) };
		if (runningEvent == currentEvents.end())
		{
			return 0;
		}

		bool res{ (*runningEvent->second)(currentEvent, reservedCurrentPrio, hwnd) };
		if (res || action == IHH::KeyStatus::KEY_UP)
		{
			currentEvents.erase(runningEvent);	// currently erasing -> if it turns out this is too slow, maybe switch to some approach that keeps the registered

			if (res)
			{
				return ProcessInput(reservedCurrentPrio, hwnd, WM_KEYDOWN, wParam, 0);	// sets it to a new keyDown, other stuff is context generated
			}
		}
		return 0;
	}

	// only key_downs from here

	IHH::KeyEventFunc* currentFunc{ currentKeyMap->getHandlerFunc(currentEvent) };
	if (!currentFunc)
	{
		if (currentKeyMap == defaultKeyMap)
		{
			currentFunc = &defaultFunc;
		}
		else
		{
			return 0;	// special KeyMap selected, devour
		}
	}

	currentEvents.insert_or_assign(static_cast<VK::VirtualKey>(wParam), currentFunc);
	(*currentFunc)(currentEvent, reservedCurrentPrio, hwnd);
  return 0; // consume
}


void ResetEventsAndKeyState()
{
	IHH::KeyEvent resetEvent{ 0, IHH::KeyStatus::RESET, 0, 0, 0 };	// reset does not receive status
	auto iter{ currentEvents.begin() };
	while (iter != currentEvents.end())
	{
		(*iter->second)(resetEvent, 0, 0);	// ignore output, we are resetting
		iter = currentEvents.erase(iter);	// delete, since we are done
	}
	ZeroMemory(crusaderKeyState, sizeof(CrusaderKeyState));	// zero key state object of stronghold should be enough
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
		lParam |= 0x40000000;	// ignore other parameters for now, they seem unused
	}

	WinProcHeader::CallNextProc(windowProcPrio, winHandle, message, status.virtualKey, lParam);

	crusaderKeyState->ctrl = nativeState.ctrl;
	crusaderKeyState->shift = nativeState.shift;
	crusaderKeyState->alt = nativeState.alt;

	return false;	// input is only cared for on KEY_HOLD, freeing the key and sending a key down to the process again, launching the other key
}


IHH::KeyEventFunc* KeyMap::getHandlerFunc(IHH::KeyEvent ev)
{
	unsigned int mapKey{ev.ctrlActive << 24 | ev.shiftActive << 16 | ev.altActive << 8 | ev.virtualKey };	// duplicate? would try to evade another func call, or not
	auto res{ funcMap.find(mapKey) };
	return res != funcMap.end() ? &res->second : nullptr;
}

bool KeyMap::registerKeyEvent(IHH::KeyEvent ev, IHH::KeyEventFunc&& func)
{
	unsigned int mapKey{ ev.ctrlActive << 24 | ev.shiftActive << 16 | ev.altActive << 8 | ev.virtualKey };	// duplicate? would try to evade another func call, or not
	funcMap.insert_or_assign(mapKey, std::forward<IHH::KeyEventFunc>(func)); // should i trust the move stuff?
	return true;	// could later return reject
}


/* exports */

extern "C" __declspec(dllexport) bool __stdcall LockKeyMap(const char* name)
{
	if (currentKeyMap != defaultKeyMap || strcmp(name, IHH::DEFAULT_KEY_MAP) == 0)	// only if currently default and not default name
	{
		return false;
	}

	auto iter{ keyMapMap.try_emplace(name).first };
	ResetEventsAndKeyState();
	currentKeyMap = &(iter->second);
	return true;
}

extern "C" __declspec(dllexport) bool __stdcall ReleaseKeyMap(const char* name)
{
	if (currentKeyMap == defaultKeyMap || strcmp(name, IHH::DEFAULT_KEY_MAP) == 0 )	// only if not default and not default name
	{
		return false;
	}

	auto iter{ keyMapMap.find(name) };
	if (iter == keyMapMap.end() || &(iter->second) != currentKeyMap)	// if this is not the current map, do not release
	{
		return false;
	}

	ResetEventsAndKeyState();
	currentKeyMap = defaultKeyMap;
	return true;
}

extern "C" __declspec(dllexport) bool __stdcall RegisterEvent(const char * keyMapName, bool ctrl, bool shift,
	bool alt, unsigned char virtualKey, IHH::KeyEventFunc&& func)
{
	if (keyMapName == nullptr || func == nullptr)	// nullptr not allowed (hopefully it does not create move problems)
	{
		return false;
	}

	auto iter{ keyMapMap.try_emplace(keyMapName).first };

	// since the action is overwriting, there is no "false" return at the moment
	// later, this might reject invalid keys
	IHH::KeyEvent createEvent{ virtualKey, IHH::KeyStatus::RESET, ctrl, shift, alt };	// reset is just dummy
	return (iter->second).registerKeyEvent(createEvent, std::forward<IHH::KeyEventFunc>(func));
}

/* LUA */


bool handleLuaEvents(const char* mapRef, unsigned int mapInt, IHH::KeyEvent ev)
{
	if (luaState == nullptr || luaControlFuncIndex == 0)
	{
		LuaLog::Log(LuaLog::LOG_ERROR, "[inputHandler]: Missing lua state or handling func. Can not handle lua key event.");
		return false;
	}

	lua_rawgeti(luaState, LUA_REGISTRYINDEX, luaControlFuncIndex);
	lua_pushstring(luaState, mapRef);
	lua_pushinteger(luaState, mapInt);

	// basically like internal, but uses long long integer by givving the status at another position
	lua_pushinteger(luaState,
		static_cast<long long>(ev.status) << 32 |
		static_cast<long long>(ev.ctrlActive) << 24 |
		static_cast<long long>(ev.shiftActive) << 16 |
		static_cast<long long>(ev.altActive) << 8 |
		static_cast<long long>(ev.virtualKey));

	if (lua_pcall(luaState, 3, 1, 0) != 0)	// call and log potential error (hopefully this does not break anything)
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

	if (!lua_isfunction(L, 1))	// only one thing left
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
		luaL_error(L, "[inputHandler]: lua_RegisterControlFunc: Invalid number of args.");
	}

	if (!lua_isstring(L, 1))
	{
		luaL_error(L, "[inputHandler]: lua_RegisterControlFunc: Wrong input fields.");
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

extern "C" __declspec(dllexport) int __cdecl lua_RegisterEvent(lua_State * L)
{
	int n{ lua_gettop(L) };    /* number of arguments */
	if (n != 2)
	{
		luaL_error(L, "[inputHandler]: lua_RegisterEvent: Invalid number of args.");
	}

	if (!lua_isstring(L, 1) || !lua_isinteger(L, 2))
	{
		luaL_error(L, "[inputHandler]: lua_ReleaseKeyMap: Wrong input fields.");
	}

	// the given integer reflects the register structure
	// it is only very loosely coupled, since overwrites can happen all the time
	// the functions will remain on the lua side, but C will simply stop calling them once they are overwritten

	// ctrl << 24 | shift << 16 | alt << 8 | virtualKey
	const char* mapName{ lua_tostring(L, 1) };
	unsigned int regInt{ static_cast<unsigned int>(lua_tointeger(L, 2)) };
	bool res{ RegisterEvent(mapName, regInt & 0x01000000, regInt & 0x00010000, regInt & 0x00000100, regInt & 0x000000FF,
		[mapName, regInt](IHH::KeyEvent ev, int, HWND) {	// stores string copy -> could get a bit heavy, but for now ok
			return handleLuaEvents(mapName, regInt, ev);
		}
	)};
	lua_pushboolean(L, res);
	return 1;
}