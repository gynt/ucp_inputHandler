#pragma once

#include <unordered_map>
#include <functional>

#include "keyEnum.h"
#include "inputHandlerHeader.h"

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
	std::unordered_map<unsigned int, InputHandlerHeader::KeyEventFunc> funcMap{};

public:
	KeyMap() {};
	~KeyMap() {};

	InputHandlerHeader::KeyEventFunc* getHandlerFunc(InputHandlerHeader::KeyEvent ev);
	void registerKeyEvent(InputHandlerHeader::KeyEvent ev, InputHandlerHeader::KeyEventFunc&& func);

private:

};

/* variables*/

inline CrusaderKeyState* crusaderKeyState{ nullptr };


/* functions */

bool InitStructures();

SHORT __stdcall GetAsyncKeyFake(int vKey);

LRESULT __stdcall ProcessInput(int reservedCurrentPrio, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

bool RetranslateToWindowProc(InputHandlerHeader::KeyEvent status, int windowProcPrio, HWND winHandle);