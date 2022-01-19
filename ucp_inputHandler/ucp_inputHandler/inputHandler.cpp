#pragma once

#include "pch.h"

#include <winProcHandler.h>
#include "inputHandlerInternal.h"

#include "keyEnum.h"
#include <unordered_map>

static std::unordered_map<std::string, KeyMap> keyMapMap{};

LRESULT __stdcall ProcessInput(int reservedCurrentPrio, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_SYSKEYUP:
		case WM_KEYUP:
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			if (wParam == VK::BACKSPACE)
			{
				keyMapMap.try_emplace("");
				KeyMap* keyMap{ &keyMapMap.find("")->second };
				keyMap->registerKeyEvent(true, true, true, VK::A,
					[](InputHandlerHeader::KeyEvent) -> bool
					{
						return true;
					});
				InputHandlerHeader::KeyEventFunc* shouldBeNull{ keyMap->getHandlerFunc(true, true, true, VK::B) };
				InputHandlerHeader::KeyEventFunc* shouldBeFunc{ keyMap->getHandlerFunc(true, true, true, VK::A) };
				InputHandlerHeader::KeyEvent test{ InputHandlerHeader::KeyStatus::KEY_HOLD };
				bool res{ (*shouldBeFunc)(test) };
				int a{ 1 };
			}

			break;
		}
		case WM_CHAR:
		default:
			break;
	}
	

  return WinProcHeader::CallNextProc(reservedCurrentPrio, hwnd, uMsg, wParam, lParam); // dummy currently
}

InputHandlerHeader::KeyEventFunc* KeyMap::getHandlerFunc(bool ctrl, bool shift, bool alt, VK::VirtualKey key)
{
	int mapKey{ctrl << 24 | shift << 16 | alt << 8 | key};	// duplicate? would try to evade another func call, or not
	auto res{ funcMap.find(mapKey) };
	return res != funcMap.end() ? &res->second : nullptr;
}

void KeyMap::registerKeyEvent(bool ctrl, bool shift, bool alt, VK::VirtualKey key, InputHandlerHeader::KeyEventFunc&& func)
{
	int mapKey{ ctrl << 24 | shift << 16 | alt << 8 | key }; // duplicate? would try to evade another func call, or not
	funcMap.insert_or_assign(mapKey, std::forward<InputHandlerHeader::KeyEventFunc>(func)); // should i trust the move stuff?
}