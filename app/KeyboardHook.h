#ifndef __KeyboardHook_H__
#define __KeyboardHook_H__

#include <windows.h>


class HookListener
{
public:
	virtual void handleKeyMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) = 0;
};

class KeyboardHook
{
public:
	HHOOK getMsgHook;
	static KeyboardHook* instance;
	HookListener* listener;

	KeyboardHook(HookListener* listener);
	~KeyboardHook();

	void handleHook(UINT msg, HWND hwnd, WPARAM wParam, LPARAM lParam);
};


#endif