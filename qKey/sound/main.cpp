#pragma comment(lib,"winmm.lib")

#include <windows.h>
#include <mmsystem.h>
#include <string>

NOTIFYICONDATA Icon = { 0 };
HWND window;
short indexOfHotkeys = 0;
short indexOfKeys = 0;
short firstPressing = 0;
LRESULT CALLBACK SoftwareMainProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
char arrayOfKeys[2];
HHOOK keyboardHook;

class Queue
{
public:
	char array[2];
	short index = 0;

	void push(short item)
	{
		if (array[0] != item && array[1] != item)
		{
			if (index == 2)
			{
				array[0] = array[1];
				array[1] = item;
				index = 0;
			}
			else if (index == 0 || index == 1)
			{
				if (array[1] != -1)
				{
					array[0] = array[1];
					array[1] = item;
				}
				else
					array[index] = item;
			}
			else
				array[index] = item;

			++index;
		}
	}
};

Queue q;

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR args, int ncmdsh)
{
	MSG SoftwareMainMessage = { 0 };

	WNDCLASSEX main = { 0 };
	main.cbSize = sizeof(WNDCLASSEX);
	main.hInstance = hInst;
	main.lpszClassName = TEXT("Main");
	main.lpfnWndProc = SoftwareMainProcedure;
	RegisterClassEx(&main);

	window = CreateWindowEx(0, TEXT("Main"), NULL, 0, 0, 0, 0, 0, NULL, NULL, hInst, NULL);

	Icon.cbSize = sizeof(NOTIFYICONDATA);
	Icon.hWnd = window;
	Icon.uVersion = NOTIFYICON_VERSION;
	Icon.uCallbackMessage = WM_USER;
	Icon.hIcon = (HICON)LoadImage(NULL, TEXT("golds.ico"), IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
	Icon.uFlags = NIF_MESSAGE | NIF_ICON;
	Shell_NotifyIcon(NIM_ADD, &Icon);

	keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);

	RegisterHotKey(NULL, 1, NULL, 0x51);

	INPUT inputs[1];
	UINT ret;

	inputs[0].type = INPUT_KEYBOARD;
	inputs[0].ki.wScan = 0;
	inputs[0].ki.dwFlags = 0;
	inputs[0].ki.time = 0;
	inputs[0].ki.dwExtraInfo = 0;

	while (GetMessage(&SoftwareMainMessage, NULL, NULL, NULL))
	{
		if (SoftwareMainMessage.message == WM_HOTKEY && firstPressing == 2) // firstPressing 
		{
			if (indexOfHotkeys == 2)
				indexOfHotkeys = 0;

			inputs[0].ki.wVk = q.array[indexOfHotkeys];
			ret = SendInput(1, inputs, sizeof(INPUT));

			++indexOfHotkeys;
		}

		TranslateMessage(&SoftwareMainMessage);
		DispatchMessage(&SoftwareMainMessage);
	}
	return 0;
}


LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) 
{
	if (nCode >= 0 && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN))
	{
		KBDLLHOOKSTRUCT* pKeyStruct = (KBDLLHOOKSTRUCT*)lParam;

		if (pKeyStruct->vkCode == 0x31 || pKeyStruct->vkCode == 0x32 || pKeyStruct->vkCode == 0x33)
		{
			q.push(pKeyStruct->vkCode);

			if (firstPressing < 2)
				++firstPressing;
		}
	}

	return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

LRESULT CALLBACK SoftwareMainProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_CREATE:
		break;

	case WM_USER:
		if (lp == WM_RBUTTONDOWN)
			if (MessageBox(NULL, TEXT("Delete process?"), TEXT("Tray"), MB_YESNO) == IDYES)
				DestroyWindow(window);
		break;

	case WM_DESTROY:
		Shell_NotifyIcon(NIM_DELETE, &Icon);
		PostQuitMessage(0);
		break;

	default: return DefWindowProc(hWnd, msg, wp, lp);
	}
}
