// Kenarlıksız Win32 penceresi + Direct3D 11. Tema dosyaları (src\gui) ve uygulama
// kabuğu (src\app\shell.cpp) bu kabuğun içinde yaşar; buradaki tek sorumluluk
// pencere ve grafik yaşam döngüsü. Çizilecek içerik app::content()'ten gelir ve
// link adımında seçilir: src\app\my_app.cpp (şablon) ya da src\demo.cpp (vitrin).
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "app/shell.hpp"
#include "window_hit.hpp"
#include "gui/theme.hpp"
#include "gui/logo.hpp"
#include <d3d11.h>
#include <dwmapi.h>
#include <shellapi.h>
#include <cstdio>
#include <cstdlib>
#include <cwchar>
#include <string>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dwmapi.lib")

static ID3D11Device*           g_device    = nullptr;
static ID3D11DeviceContext*    g_context   = nullptr;
static IDXGISwapChain*         g_swapChain = nullptr;
static ID3D11RenderTargetView* g_rtv       = nullptr;
static float                   g_initialScale = 1.0f;
static bool                    g_rounded  = false;
static int                     g_winW     = 0, g_winH = 0;
static UINT                    g_resizeW = 0, g_resizeH = 0;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Köşe yuvarlatma isteği bazı Windows 10 derlemelerinde sessizce yok sayılır;
// ölçüt sürüm numarasıdır.
static bool IsWindows11OrGreater()
{
    using RtlGetVersionFn = LONG(WINAPI*)(PRTL_OSVERSIONINFOW);
    RTL_OSVERSIONINFOW vi = { sizeof(vi) };
    if (HMODULE ntdll = ::GetModuleHandleW(L"ntdll.dll"))
        if (auto fn = (RtlGetVersionFn)(void*)::GetProcAddress(ntdll, "RtlGetVersion"))
            fn(&vi);
    return vi.dwBuildNumber >= 22000;
}

static void CreateRenderTarget()
{
    ID3D11Texture2D* back = nullptr;
    g_swapChain->GetBuffer(0, IID_PPV_ARGS(&back));
    if (back)
    {
        g_device->CreateRenderTargetView(back, nullptr, &g_rtv);
        back->Release();
    }
}

static void CleanupRenderTarget()
{
    if (g_rtv) { g_rtv->Release(); g_rtv = nullptr; }
}

static bool CreateDeviceD3D(HWND hwnd)
{
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount                        = 2;
    sd.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator   = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags                              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow                       = hwnd;
    sd.SampleDesc.Count                   = 1;
    sd.Windowed                           = TRUE;
    sd.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;

    const D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
    D3D_FEATURE_LEVEL level;
    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, levels, 2, D3D11_SDK_VERSION,
                                               &sd, &g_swapChain, &g_device, &level, &g_context);
    if (hr == DXGI_ERROR_UNSUPPORTED) // donanım sürücüsü yok: yazılım çizim yolu
        hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, levels, 2, D3D11_SDK_VERSION,
                                           &sd, &g_swapChain, &g_device, &level, &g_context);
    if (FAILED(hr)) return false;

    CreateRenderTarget();
    return true;
}

static void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_swapChain) { g_swapChain->Release(); g_swapChain = nullptr; }
    if (g_context)   { g_context->Release();   g_context   = nullptr; }
    if (g_device)    { g_device->Release();    g_device    = nullptr; }
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;
    switch (msg)
    {
    case WM_SIZE:
        if (g_device && wParam != SIZE_MINIMIZED)
        {
            g_resizeW = (UINT)LOWORD(lParam);
            g_resizeH = (UINT)HIWORD(lParam);
            g_winW = (int)g_resizeW;
            g_winH = (int)g_resizeH;
            // DWM köşe yuvarlatması yoksa bölge tek geçici ölçüttür; yeniden
            // boyutlandırmada bölge de pencereyle birlikte yenilenmeli.
            if (!g_rounded)
            {
                // Başarılı çağrıda bölge yönetiminin sahipliği sisteme geçer;
                // önceki bölge de sistem tarafından bırakılır.
                ::SetWindowRgn(hWnd,
                    ::CreateRoundRectRgn(0, 0, g_winW + 1, g_winH + 1,
                                         (int)(12.0f * g_initialScale * 2.0f),
                                         (int)(12.0f * g_initialScale * 2.0f)), TRUE);
            }
        }
        return 0;
    case WM_GETMINMAXINFO:
    {
        // WS_THICKFRAME küçültme tutamağını Win32'ye bırakıyor; alt sınır
        // burada veriliyor ki yerleşim negatif genişliğe düşmesin.
        MINMAXINFO* mmi = (MINMAXINFO*)lParam;
        mmi->ptMinTrackSize.x = (LONG)(720 * g_initialScale);
        mmi->ptMinTrackSize.y = (LONG)(480 * g_initialScale);
        return 0;
    }
    case WM_NCCALCSIZE:
        // Kalın çerçeve yalnızca boyutlandırma döngüsü için isteniyor; istem
        // alanından pay biçmesine izin verilmez, böylece kenarlık çizilmez.
        if (wParam) return 0;
        break;
    case WM_NCHITTEST:
    {
        // Kenar şeritleri Win32'nin kendi boyutlandırmasına devredilir; imleç
        // biçimi ve sürükleme davranışı elle taklit edilmez.
        const LRESULT hit = ::DefWindowProcW(hWnd, msg, wParam, lParam);
        if (hit != HTCLIENT) return hit;
        const POINT pt{ (LONG)(short)LOWORD(lParam), (LONG)(short)HIWORD(lParam) };
        RECT rc;
        if (!::GetWindowRect(hWnd, &rc)) return HTCLIENT;
        return win::HitTestEdge(rc, pt, (int)(10.0f * g_initialScale));
    }
    case WM_DPICHANGED:
    {
        // Yazı tipleri yalnızca başlangıçta rasterleştirildiği için yeni ölçek,
        // yazı ölçeği çarpanı üzerinden taşınır.
        RECT* rc = (RECT*)lParam;
        const float s = HIWORD(wParam) / 96.0f;
        const float ratio = s / g_initialScale;
        theme::scale = s;
        ImGui::GetStyle().FontScaleDpi = ImGui::GetStyle().FontScaleDpi * ratio;
        ImGui::GetStyle().ScaleAllSizes(ratio);
        ::SetWindowPos(hWnd, nullptr, rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top,
                       SWP_NOZORDER | SWP_NOACTIVATE);
        return 0;
    }
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Alt menüsü arayüzü çizerken araya girmesin
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

static std::wstring ToWide(const char* utf8)
{
    if (!utf8 || !*utf8) return std::wstring();
    const int n = ::MultiByteToWideChar(CP_UTF8, 0, utf8, -1, nullptr, 0);
    std::wstring out((size_t)(n > 0 ? n - 1 : 0), L'\0');
    if (n > 0) ::MultiByteToWideChar(CP_UTF8, 0, utf8, -1, &out[0], n);
    return out;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    ImGui_ImplWin32_EnableDpiAwareness();
    const float scale = ImGui_ImplWin32_GetDpiScaleForMonitor(
        ::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));
    g_initialScale = scale > 0.0f ? scale : 1.0f;

    const app::Content& content = app::content();
    const std::wstring  title   = ToWide(content.brand);

    const wchar_t* kClass = L"TENGRI_ThemeApp";
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0, 0, hInstance,
                       ::LoadIconW(nullptr, IDI_APPLICATION), ::LoadCursorW(nullptr, IDC_ARROW),
                       nullptr, nullptr, kClass, nullptr };
    ::RegisterClassExW(&wc);

    // -size 1440x900 / -page 3 / -toast 2 / -modal: düzeni ve belirli bir durumu
    // tıklamadan ekrana getirir (ekran görüntüsü alma ve doküman için).
    int logicalW = 1060, logicalH = 700;
    app::Options opt;
    {
        int     argc = 0;
        LPWSTR* argv = ::CommandLineToArgvW(::GetCommandLineW(), &argc);
        for (int i = 1; argv && i + 1 < argc; ++i)
        {
            if (::wcscmp(argv[i], L"-size") == 0 || ::wcscmp(argv[i], L"--size") == 0)
            {
                int pw = 0, ph = 0;
                if (::swscanf_s(argv[i + 1], L"%dx%d", &pw, &ph) == 2)
                {
                    logicalW = pw < 720 ? 720 : pw;
                    logicalH = ph < 480 ? 480 : ph;
                }
            }
            else if (::wcscmp(argv[i], L"-page") == 0 || ::wcscmp(argv[i], L"--page") == 0)
                opt.start_page = ::_wtoi(argv[i + 1]);
            else if (::wcscmp(argv[i], L"-toast") == 0 || ::wcscmp(argv[i], L"--toast") == 0)
                opt.toast = ::_wtoi(argv[i + 1]);
        }
        // bayraksız seçenekler
        for (int i = 1; argv && i < argc; ++i)
            if (::wcscmp(argv[i], L"-modal") == 0 || ::wcscmp(argv[i], L"--modal") == 0)
                opt.modal = true;
        if (argv) ::LocalFree(argv);
    }
    const int w = (int)(logicalW * g_initialScale), h = (int)(logicalH * g_initialScale);
    g_winW = w; g_winH = h;

    RECT wa;
    ::SystemParametersInfoW(SPI_GETWORKAREA, 0, &wa, 0);
    HWND hwnd = ::CreateWindowExW(WS_EX_APPWINDOW, kClass, title.c_str(),
                                  WS_POPUP | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME,
                                  wa.left + (wa.right - wa.left - w) / 2,
                                  wa.top  + (wa.bottom - wa.top - h) / 2,
                                  w, h, nullptr, nullptr, wc.hInstance, nullptr);
    if (!hwnd)
    {
        ::UnregisterClassW(kClass, wc.hInstance);
        ::MessageBoxW(nullptr, L"Pencere oluşturulamadı.", title.c_str(), MB_ICONERROR);
        return 1;
    }

    float corner = 8.0f * g_initialScale;
    if (IsWindows11OrGreater())
    {
        DWORD pref = 2; // yuvarlak köşe tercihi
        if (SUCCEEDED(::DwmSetWindowAttribute(hwnd, 33, &pref, sizeof(pref))))
        {
            g_rounded = true;
            COLORREF border = RGB(32, 32, 34);
            ::DwmSetWindowAttribute(hwnd, 34, &border, sizeof(border));
        }
    }
    if (!g_rounded)
    {
        corner = 12.0f * g_initialScale;
        ::SetWindowRgn(hwnd, ::CreateRoundRectRgn(0, 0, w + 1, h + 1, (int)(corner * 2.0f), (int)(corner * 2.0f)), TRUE);
    }

    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(kClass, wc.hInstance);
        ::MessageBoxW(nullptr, L"Direct3D 11 başlatılamadı.", title.c_str(), MB_ICONERROR);
        return 1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    theme::Init(g_initialScale);
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_device, g_context);
    logo::Init(g_device);

    app::Init(hwnd, corner, content, opt);

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    bool done = false;
    while (!done)
    {
        MSG msg;
        while (::PeekMessageW(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessageW(&msg);
            if (msg.message == WM_QUIT) done = true;
        }
        if (done) break;

        if (g_resizeW != 0 && g_resizeH != 0)
        {
            CleanupRenderTarget();
            g_swapChain->ResizeBuffers(0, g_resizeW, g_resizeH, DXGI_FORMAT_UNKNOWN, 0);
            g_resizeW = g_resizeH = 0;
            CreateRenderTarget();
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        app::Frame();

        ImGui::Render();
        const float clear[4] = { 0.016f, 0.016f, 0.02f, 1.0f };
        g_context->OMSetRenderTargets(1, &g_rtv, nullptr);
        g_context->ClearRenderTargetView(g_rtv, clear);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g_swapChain->Present(1, 0); // dikey eşitleme
    }

    // Doku, cihaz kapatılmadan önce serbest bırakılmalı.
    logo::Shutdown();
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(kClass, wc.hInstance);
    return 0;
}
