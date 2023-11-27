#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#include "stdinclude.hpp"
#include "scgui/scGUILoop.hpp"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

// Data
static ID3D11Device* g_pd3dDevice = NULL;
static ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
static IDXGISwapChain* g_pSwapChain = NULL;
static ID3D11RenderTargetView* g_mainRenderTargetView = NULL;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool guiDone = true;

bool attachToGame = false;

HWND hwnd;
RECT cacheRect{ 100, 100, 600, 500 };

void SetGuiDone(bool isDone) {
    guiDone = isDone;
}

void SetWindowTop()
{
    ::SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
}

void CancelWindowTop()
{
    ::SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
}

bool lastTopStat = false;
bool nowTopStat = false;
void changeTopState() {
    if (lastTopStat == nowTopStat) return;
    lastTopStat = nowTopStat;
    if (nowTopStat) {
        SetWindowTop();
    }
    else {
        CancelWindowTop();
    }

}

bool getUmaGuiDone() {
    return guiDone;
}

// Main code
void guimain()
{
    guiDone = false;
    WNDCLASSW wc;
    RECT WindowRectangle;
    HWND GameHwnd;
    int WindowWide;
    int WindowHeight;

    // GameHwnd = GetConsoleWindow();
    // if (GameHwnd) attachToGame = true;

    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    if (!attachToGame) {
        WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"IM@S SCSP Localify", NULL };
        ::RegisterClassExW(&wc);
        hwnd = ::CreateWindowW(wc.lpszClassName, L"IM@S SCSP GUI", WS_OVERLAPPEDWINDOW, cacheRect.left, cacheRect.top,
            cacheRect.right - cacheRect.left, cacheRect.bottom - cacheRect.top, NULL, NULL, wc.hInstance, NULL);
        if (nowTopStat) SetWindowTop();
    }
    else {
        // 注册窗体类
        // 附加到指定窗体上
        wc.cbClsExtra = NULL;
        wc.cbWndExtra = NULL;
        wc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0));
        wc.hCursor = LoadCursor(0, IDC_ARROW);
        wc.hIcon = LoadIcon(0, IDI_APPLICATION);
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpfnWndProc = (WNDPROC)WndProc;
        wc.lpszClassName = L" ";
        wc.lpszMenuName = L" ";
        wc.style = CS_VREDRAW | CS_HREDRAW;

        RegisterClassW(&wc);

        // 得到窗口句柄
        WindowRectangle;   
        GameHwnd = FindWindowW(L"UnityWndClass", L"imasscprism");
        GetWindowRect(GameHwnd, &WindowRectangle);
        WindowWide = WindowRectangle.right - WindowRectangle.left;
        WindowHeight = WindowRectangle.bottom - WindowRectangle.top;

        // 创建窗体
       //  HWND hwnd = ::CreateWindowW(WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW, L" ", L" ", WS_POPUP, 1, 1, WindowWide, WindowHeight, 0, 0, wc.hInstance, 0);
        hwnd = ::CreateWindowW(wc.lpszClassName, L"", WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_LAYERED | WS_POPUP, 1, 1, WindowWide, WindowHeight, NULL, NULL, wc.hInstance, NULL);
        LONG lWinStyleEx = GetWindowLong(hwnd, GWL_EXSTYLE);
        lWinStyleEx = lWinStyleEx | WS_EX_LAYERED;

        SetWindowLong(hwnd, GWL_EXSTYLE, lWinStyleEx);
        SetLayeredWindowAttributes(hwnd, 0, 0, LWA_ALPHA);
        // 去掉标题栏及边框
        LONG_PTR Style = GetWindowLongPtr(hwnd, GWL_STYLE);
        Style = Style & ~WS_CAPTION & ~WS_SYSMENU & ~WS_SIZEBOX;
        SetWindowLongPtr(hwnd, GWL_STYLE, Style);
        // 至顶层窗口 最大化
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, WindowWide, WindowHeight, SWP_SHOWWINDOW);
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
        // 设置窗体可穿透鼠标
        // SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_TRANSPARENT | WS_EX_LAYERED);

    }

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        printf("init D3D failed\n");
        CleanupDeviceD3D();
        if (attachToGame) ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return;
    }

    if (attachToGame) {
        // Show the window
        SetLayeredWindowAttributes(hwnd, 0, 0, LWA_ALPHA);
        SetLayeredWindowAttributes(hwnd, 0, 0, LWA_COLORKEY);
    }

    ::ShowWindow(hwnd, SW_SHOW);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    ImFontGlyphRangesBuilder builder;

    ImFontConfig config;
    builder.AddRanges(io.Fonts->GetGlyphRangesJapanese());
    builder.AddRanges(io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
    builder.AddRanges(io.Fonts->GetGlyphRangesChineseFull());
    builder.AddRanges(io.Fonts->GetGlyphRangesKorean());
    builder.AddRanges(io.Fonts->GetGlyphRangesDefault());
    builder.AddText("○◎△×☆");
    ImVector<ImWchar> glyphRanges;
    builder.BuildRanges(&glyphRanges);
    config.GlyphRanges = glyphRanges.Data;

    if (std::filesystem::exists("c:\\Windows\\Fonts\\msyh.ttc")) {
        io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\msyh.ttc", 18.0f, &config);
    }
    else {
        io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f, &config);
    }

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 0.00f);

    // Main loop
    while (!guiDone)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                guiDone = true;
        }
        if (guiDone)
            break;


        if (attachToGame) {
            // 每次都将窗体置顶并跟随游戏窗体移动
            GetWindowRect(GameHwnd, &WindowRectangle);
            WindowWide = (WindowRectangle.right) - (WindowRectangle.left);
            WindowHeight = (WindowRectangle.bottom) - (WindowRectangle.top);
            DWORD dwStyle = GetWindowLong(GameHwnd, GWL_STYLE);
            if (dwStyle & WS_BORDER)
            {
                WindowRectangle.top += 23;
                WindowHeight -= 23;
            }

            // 跟随窗口移动
            MoveWindow(hwnd, WindowRectangle.left + 9, WindowRectangle.top + 9, WindowWide - 18, WindowHeight - 18, true);

        }

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();

        ImGui::NewFrame();
        SCGUILoop::mainLoop();

        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0); // Present with vsync
        // g_pSwapChain->Present(0, 0); // Present without vsync
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_WARP, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SIZING: {
        RECT* rect = reinterpret_cast<RECT*>(lParam);
        cacheRect.left = rect->left;
        cacheRect.right = rect->right;
        cacheRect.top = rect->top;
        cacheRect.bottom = rect->bottom;
    }; break;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}