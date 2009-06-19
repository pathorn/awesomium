#include "KeyboardHook.h"

LRESULT CALLBACK GetMessageProc(int nCode, WPARAM wParam, LPARAM lParam);

KeyboardHook* KeyboardHook::instance = 0;

KeyboardHook::KeyboardHook(HookListener* listener) : listener(listener)
{
	instance = this;

	HINSTANCE hInstance = GetModuleHandle(0);

	getMsgHook = SetWindowsHookEx(WH_GETMESSAGE, GetMessageProc, hInstance, GetCurrentThreadId());
}

KeyboardHook::~KeyboardHook()
{
	UnhookWindowsHookEx(getMsgHook);

	instance = 0;
}

void KeyboardHook::handleHook(UINT msg, HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_CHAR:
	case WM_DEADCHAR:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_SYSDEADCHAR:
	case WM_SYSCHAR:
	case WM_IME_CHAR:
	case WM_IME_COMPOSITION:
	case WM_IME_COMPOSITIONFULL:
	case WM_IME_CONTROL:
	case WM_IME_ENDCOMPOSITION:
	case WM_IME_KEYDOWN:
	case WM_IME_KEYUP:
	case WM_IME_NOTIFY:
	case WM_IME_REQUEST:
	case WM_IME_SELECT:
	case WM_IME_SETCONTEXT:
	case WM_IME_STARTCOMPOSITION:
	case WM_HELP:
	case WM_CANCELMODE: 
		{
			if(listener)
				listener->handleKeyMessage(hwnd, msg, wParam, lParam);

			break;
		}
	}
}

LRESULT CALLBACK GetMessageProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if(nCode == HC_ACTION)
	{
		MSG *msg = (MSG*)lParam;
		if(wParam & PM_REMOVE)
			KeyboardHook::instance->handleHook(msg->message, msg->hwnd, msg->wParam, msg->lParam);
	}

	return CallNextHookEx(KeyboardHook::instance->getMsgHook, nCode, wParam, lParam);
}