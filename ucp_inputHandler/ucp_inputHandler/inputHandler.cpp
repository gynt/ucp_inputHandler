#pragma once

#include "pch.h"

#include <winProcHandler.h>
#include "inputHandlerInternal.h"

#include "keyEnum.h"
#include <unordered_map>

namespace IHH = InputHandlerHeader; // easier to use

static std::unordered_map<std::string, KeyMap> keyMapMap{};

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
		{
			action = lParam & 0x40000000 ? IHH::KeyStatus::KEY_HOLD : IHH::KeyStatus::KEY_DOWN;
			break;
			// example
			//if (wParam == VK::BACKSPACE)
			//{
			//	keyMapMap.try_emplace("");
			//	KeyMap* keyMap{ &keyMapMap.find("")->second };
			//	keyMap->registerKeyEvent(true, true, true, VK::A,
			//		[](IHH::KeyEvent) -> bool
			//		{
			//			return true;
			//		});
			//	IHH::KeyEventFunc* shouldBeNull{ keyMap->getHandlerFunc(true, true, true, VK::B) };
			//	IHH::KeyEventFunc* shouldBeFunc{ keyMap->getHandlerFunc(true, true, true, VK::A) };
			//	IHH::KeyEvent test{ IHH::KeyStatus::KEY_HOLD };
			//	bool res{ (*shouldBeFunc)(test) };
			//	int a{ 1 };
			//}
		}
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
	
	const char* test{ nullptr };
	switch (action)
	{
		case InputHandlerHeader::KeyStatus::RESET:
			test = "RESET";
			break;
		case InputHandlerHeader::KeyStatus::KEY_DOWN:
			test = "KEY_DOWN";
			break;
		case InputHandlerHeader::KeyStatus::KEY_HOLD:
			test = "KEY_HOLD";
			break;
		case InputHandlerHeader::KeyStatus::KEY_UP:
			test = "KEY_UP";
			break;
		case InputHandlerHeader::KeyStatus::NONE:
		default:
			break;
	}
	if (test) {
		LuaLog::Log(LuaLog::LOG_INFO, test);
	}

  return WinProcHeader::CallNextProc(reservedCurrentPrio, hwnd, uMsg, wParam, lParam); // dummy currently
}

IHH::KeyEventFunc* KeyMap::getHandlerFunc(bool ctrl, bool shift, bool alt, VK::VirtualKey key)
{
	int mapKey{ctrl << 24 | shift << 16 | alt << 8 | key};	// duplicate? would try to evade another func call, or not
	auto res{ funcMap.find(mapKey) };
	return res != funcMap.end() ? &res->second : nullptr;
}

void KeyMap::registerKeyEvent(bool ctrl, bool shift, bool alt, VK::VirtualKey key, IHH::KeyEventFunc&& func)
{
	int mapKey{ ctrl << 24 | shift << 16 | alt << 8 | key }; // duplicate? would try to evade another func call, or not
	funcMap.insert_or_assign(mapKey, std::forward<IHH::KeyEventFunc>(func)); // should i trust the move stuff?
}