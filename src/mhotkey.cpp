// #define _WIN32_WINNT 0x0400
#pragma comment( lib, "user32.lib" )

#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <thread>
#include <format>
#include <functional>
#include <WinUser.h>
#include "camera/camera.hpp"

extern std::function<void()> on_hotKey_0;
extern std::function<void()> g_on_close;

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
    

    WNDPROC g_pfnOldWndProc = NULL;
    LRESULT CALLBACK WndProcCallback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        DWORD SHIFT_key = 0;
        DWORD CTRL_key = 0;
        DWORD ALT_key = 0;
        DWORD SPACE_key = 0;
        DWORD UP_key = 0;
        DWORD DOWN_key = 0;
        DWORD LEFT_key = 0;
        DWORD RIGHT_key = 0;

        switch (uMsg) {
        case WM_INPUT: {
            RAWINPUT rawInput;
            UINT size = sizeof(RAWINPUT);
            if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &rawInput, &size, sizeof(RAWINPUTHEADER)) == size) {

                if (rawInput.header.dwType == RIM_TYPEMOUSE)
                {
                    switch (rawInput.data.mouse.ulButtons) {
                    case 0: {  // move
                        SCCamera::mouseMove(rawInput.data.mouse.lLastX, rawInput.data.mouse.lLastY, 3);
                    }; break;
                    case 4: {  // press
                        SCCamera::mouseMove(0, 0, 1);
                    }; break;
                    case 8: {  // release
                        SCCamera::mouseMove(0, 0, 2);
                    }; break;
                    default: break;
                    }

                    if (rawInput.data.mouse.usButtonFlags == RI_MOUSE_WHEEL) {
                        if (rawInput.data.mouse.usButtonData == 120) {
                            SCCamera::mouseMove(0, 1, 4);
                        }
                        else {
                            SCCamera::mouseMove(0, -1, 4);
                        }
                    }

                }
            }
        }; break;

        case WM_SYSKEYUP:
        case WM_KEYUP: {
            int key = wParam;
            if (mKeyBoardRawCallBack != nullptr) mKeyBoardRawCallBack(uMsg, key);
        }; break;

        case WM_SYSKEYDOWN:
        case WM_KEYDOWN: {
            int key = wParam;

            if (mKeyBoardRawCallBack != nullptr) mKeyBoardRawCallBack(uMsg, key);
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
        }; break;
        case WM_NCACTIVATE: {
            if (!wParam) {
                SCCamera::onKillFocus();
                return FALSE;
            }
        }; break;
        case WM_KILLFOCUS: {
            SCCamera::onKillFocus();
            return FALSE;
        }; break;
        case WM_CLOSE: {
            if (g_on_close) g_on_close();
        }; break;
        default: break;
        }

        return CallWindowProc(g_pfnOldWndProc, hWnd, uMsg, wParam, lParam);
    }

    void InstallWndProcHook()
    {
        g_pfnOldWndProc = (WNDPROC)GetWindowLongPtr(FindWindowW(L"UnityWndClass", L"imasscprism"), GWLP_WNDPROC);
        SetWindowLongPtr(FindWindowW(L"UnityWndClass", L"imasscprism"), GWLP_WNDPROC, (LONG_PTR)WndProcCallback);
    }

    void UninstallWndProcHook(HWND hWnd)
    {
        SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)g_pfnOldWndProc);
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
        InstallWndProcHook();

        return 1;

    }
}
