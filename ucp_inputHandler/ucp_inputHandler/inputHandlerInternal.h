#pragma once

#include <unordered_map>
#include <functional>

#include "keyEnum.h"
#include "inputHandlerHeader.h"

class KeyMap
{
private:
	std::unordered_map<int, InputHandlerHeader::KeyEventFunc> funcMap{};

public:
	KeyMap() {};
	~KeyMap() {};

	InputHandlerHeader::KeyEventFunc* getHandlerFunc(bool ctrl, bool shift, bool alt, VK::VirtualKey key);
	void registerKeyEvent(bool ctrl, bool shift, bool alt, VK::VirtualKey key, InputHandlerHeader::KeyEventFunc&& func);

private:

};


LRESULT __stdcall ProcessInput(int reservedCurrentPrio, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);