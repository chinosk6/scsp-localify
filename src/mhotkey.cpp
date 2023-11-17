#define _WIN32_WINNT 0x0400
#pragma comment( lib, "user32.lib" )

#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <thread>
#include <format>
#include <functional>

extern std::function<void()> on_hotKey_0;

namespace MHotkey{
    namespace {
        HHOOK hKeyboardHook;
        bool is_uma = false;
        char hotk = 'u';
        std::string extPluginPath = "";
        std::string umaArgs = "";
        bool openPluginSuccess = false;
        DWORD pluginPID = -1;
        int tlgport = 43215;
        bool ext_server_start = false;

        std::function<void(int, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD)> mKeyBoardCallBack = nullptr;
        std::function<void(int, int)> mKeyBoardRawCallBack = nullptr;
        bool hotKeyThreadStarted = false;
    }

    bool get_is_plugin_open() {
        return openPluginSuccess && ext_server_start;
    }

    void set_ext_server_start(const bool status)
    {
        ext_server_start = status;
    }

    void SetKeyCallBack(std::function<void(int, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD)> callbackfun) {
        mKeyBoardCallBack = callbackfun;
    }

    bool check_file_exist(const std::string& name) {
        struct stat buffer;
        return (stat(name.c_str(), &buffer) == 0);
    }
    
    bool setWindowPosOffset(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags) {
        RECT* windowR = new RECT;
        RECT* clientR = new RECT;
        GetWindowRect(hWnd, windowR);
        GetClientRect(hWnd, clientR);

        return SetWindowPos(hWnd, hWndInsertAfter, X, Y, cx + windowR->right - windowR->left - clientR->right,
            cy + windowR->bottom - windowR->top - clientR->bottom, uFlags);
    }

    void setMKeyBoardRawCallBack(std::function<void(int, int)> cbfunc) {
        mKeyBoardRawCallBack = cbfunc;
    }
    
    __declspec(dllexport) LRESULT CALLBACK KeyboardEvent(int nCode, WPARAM wParam, LPARAM lParam)
    {
        DWORD SHIFT_key = 0;
        DWORD CTRL_key = 0;
        DWORD ALT_key = 0;
        DWORD SPACE_key = 0;
        DWORD UP_key = 0;
        DWORD DOWN_key = 0;
        DWORD LEFT_key = 0;
        DWORD RIGHT_key = 0;


        if (nCode == HC_ACTION)
        {
            if ((wParam == WM_SYSKEYUP) || wParam == WM_KEYUP) {
                KBDLLHOOKSTRUCT hooked_key = *((KBDLLHOOKSTRUCT*)lParam);
                int key = hooked_key.vkCode;
                if (mKeyBoardRawCallBack != nullptr) mKeyBoardRawCallBack(wParam, key);
            }
            else if ((wParam == WM_SYSKEYDOWN) || (wParam == WM_KEYDOWN)) {
                KBDLLHOOKSTRUCT hooked_key = *((KBDLLHOOKSTRUCT*)lParam);
                DWORD dwMsg = 1;
                dwMsg += hooked_key.scanCode << 16;
                dwMsg += hooked_key.flags << 24;
                char lpszKeyName[1024] = { 0 };
                lpszKeyName[0] = '[';

                int i = GetKeyNameText(dwMsg, (lpszKeyName + 1), 0xFF) + 1;
                lpszKeyName[i] = ']';

                int key = hooked_key.vkCode;

                if (mKeyBoardRawCallBack != nullptr) mKeyBoardRawCallBack(wParam, key);
                SHIFT_key = GetAsyncKeyState(VK_SHIFT);
                CTRL_key = GetAsyncKeyState(VK_CONTROL);
                ALT_key = GetAsyncKeyState(VK_MENU);
                SPACE_key = GetAsyncKeyState(VK_SPACE);

                UP_key = GetAsyncKeyState(VK_UP);
                DOWN_key = GetAsyncKeyState(VK_DOWN);
                LEFT_key = GetAsyncKeyState(VK_LEFT);
                RIGHT_key = GetAsyncKeyState(VK_RIGHT);

                if (mKeyBoardCallBack != nullptr) {
                    mKeyBoardCallBack(key, SHIFT_key, CTRL_key, ALT_key, SPACE_key, UP_key, DOWN_key, LEFT_key, RIGHT_key);
                }

                if (key >= 'A' && key <= 'Z')
                {

                    if (GetAsyncKeyState(VK_SHIFT) >= 0) key += 32;

                    if (CTRL_key != 0 && key == hotk)
                    {
                        // fopenExternalPlugin(tlgport);
                        printf("hotKey pressed.\n");
                        if (on_hotKey_0) on_hotKey_0();
                    }

                    SHIFT_key = 0;
                    CTRL_key = 0;
                    ALT_key = 0;
                    SPACE_key = 0;
                    DWORD UP_key = 0;
                    DWORD DOWN_key = 0;
                    DWORD LEFT_key = 0;
                    DWORD RIGHT_key = 0;
                }
            }

        }
        return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
    }

    void MessageLoop()
    {
        MSG message;
        while (GetMessage(&message, NULL, 0, 0))
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
    }

    DWORD WINAPI my_HotKey(LPVOID lpParm)
    {
        HINSTANCE hInstance = GetModuleHandle(NULL);
        if (!hInstance) hInstance = LoadLibrary((LPCSTR)lpParm);
        if (!hInstance) return 1;

        hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)KeyboardEvent, hInstance, NULL);
        MessageLoop();
        UnhookWindowsHookEx(hKeyboardHook);
        return 0;
    }

    bool get_uma_stat() {
        return MHotkey::is_uma;
    }
    void set_uma_stat(bool s) {
        MHotkey::is_uma = s;
    }

    void setExtPluginPath(std::string ppath) {
        MHotkey::extPluginPath = ppath;
    }
    void setUmaCommandLine(std::string args) {
        MHotkey::umaArgs = args;
    }
    void setTlgPort(int port) {
        tlgport = port;
    }

    int start_hotkey(char sethotk='u')
    {
        MHotkey::hotk = sethotk;
        if (hotKeyThreadStarted) return 1;

        HANDLE hThread;
        DWORD dwThread;

        hotKeyThreadStarted = true;
        hThread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)my_HotKey, (LPVOID)"", NULL, &dwThread);

        // ShowWindow(FindWindowA("ConsoleWindowClass", NULL), false);
        return 1;

        if (hThread) return WaitForSingleObject(hThread, INFINITE);
        else return 1;

    }
}
