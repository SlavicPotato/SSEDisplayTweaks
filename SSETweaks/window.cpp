#include "pch.h"

using namespace std;

namespace SDT
{
    constexpr const char* SECTION_WINDOW = "Window";

    constexpr const char* CKEY_GHOSTING = "DisableProcessWindowsGhosting";
    constexpr const char* CKEY_LOCKCURSOR = "LockCursor";
    constexpr const char* CKEY_FORCEMIN = "ForceMinimize";

    DWindow::CreateWindowExA_T DWindow::CreateWindowExA_O;

    DWindow DWindow::m_Instance;

    DWindow::DWindow()
    {
        upscaling.hWnd = NULL;
    }

    void DWindow::LoadConfig()
    {
        conf.disable_ghosting = GetConfigValue(SECTION_WINDOW, CKEY_GHOSTING, false);
        conf.lock_cursor = GetConfigValue(SECTION_WINDOW, CKEY_LOCKCURSOR, false);
        conf.force_minimize = GetConfigValue(SECTION_WINDOW, CKEY_FORCEMIN, false);

        auto rd = IDDispatcher::GetDriver<DRender>(DRIVER_RENDER);
        conf.upscale = rd && rd->IsOK() && rd->conf.upscale;
    }

    void DWindow::PostLoadConfig()
    {
        if (conf.disable_ghosting) {
            ::DisableProcessWindowsGhosting();
            Message("Ghosting disabled");
        }

        if (conf.lock_cursor) {
            SetupCursorLockMP();
            Message("Cursor locking enabled");
        }

        if (conf.force_minimize) {
            SetupForceMinimizeMP();
            Message("Forcing minimize on focus loss");
        }
    }

    void DWindow::SetupCursorLockMP()
    {
        mp.Add(MsgProc::MsgList(
            { WM_SETFOCUS , WM_CAPTURECHANGED }),
            [&](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
            {
                if (::GetFocus() == hWnd && ::GetActiveWindow() == hWnd) {
                    CaptureCursor(hWnd, true);
                }
            });

        mp.Add(MsgProc::MsgList(
            { WM_WINDOWPOSCHANGED, WM_SIZING }),
            [&](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
            {
                CaptureCursor(hWnd, true);
            });

        mp.Add(WM_ACTIVATE,
            [&](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
            {
                BOOL fMinimized = static_cast<BOOL>(HIWORD(wParam));
                WORD fActive = LOWORD(wParam);

                if (fActive == WA_ACTIVE) {
                    if (!fMinimized) {
                        if (::GetFocus() == hWnd) {
                            CaptureCursor(hWnd, true);
                        }
                    }
                }
                else if (fActive == WA_INACTIVE) {
                    CaptureCursor(hWnd, false);
                }
            });

        mp.Add(MsgProc::MsgList(
            { WM_KILLFOCUS, WM_DESTROY }),
            [&](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
            {
                CaptureCursor(hWnd, false);
            });
    }

    void DWindow::SetupForceMinimizeMP()
    {
        mp.Add(WM_KILLFOCUS,
            [&](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
            {
                Debug("[0x%llX] Window minimized", hWnd);
                ::ShowWindow(hWnd, SW_MINIMIZE);
            });
    }

    void DWindow::RegisterHooks()
    {
        if (mp.HasProcessors() || conf.upscale) {
            if (!Hook::Call6(CreateWindowEx_C, reinterpret_cast<uintptr_t>(CreateWindowExA_Hook), CreateWindowExA_O)) {
                Error("CreateWindowExA hook failed");
                SetOK(false);
                return;
            }
        }

        if (conf.upscale) {
            RegisterHook(
                GetClientRect1,
                reinterpret_cast<uintptr_t>(GetClientRect_Hook),
                HookDescriptor::kWR6Call
            );

            IEvents::RegisterForEvent(Event::OnD3D11PreCreate, OnD3D11PreCreate_Upscale);
        }
    }

    bool DWindow::Prepare()
    {
        return true;
    }

    bool DWindow::SetCursorLock(HWND hwnd)
    {
        RECT rect;
        if (::GetWindowRect(hwnd, &rect)) {
            return static_cast<bool>(::ClipCursor(&rect));
        }
        return false;
    }

    void DWindow::CaptureCursor(HWND hwnd, bool sw)
    {
        if (sw) {
            SetCursorLock(hwnd);
        }
        else {
            ::ClipCursor(NULL);
        }
    }

    HWND WINAPI DWindow::CreateWindowExA_Hook(
        _In_ DWORD dwExStyle,
        _In_opt_ LPCSTR lpClassName,
        _In_opt_ LPCSTR lpWindowName,
        _In_ DWORD dwStyle,
        _In_ int X,
        _In_ int Y,
        _In_ int nWidth,
        _In_ int nHeight,
        _In_opt_ HWND hWndParent,
        _In_opt_ HMENU hMenu,
        _In_opt_ HINSTANCE hInstance,
        _In_opt_ LPVOID lpParam)
    {
        HWND hWnd = CreateWindowExA_O(
            dwExStyle, lpClassName, lpWindowName, dwStyle,
            X, Y, nWidth, nHeight,
            hWndParent, hMenu, hInstance, lpParam);

        if (hWnd == NULL) {
            m_Instance.FatalError(
                "CreateWindowExA failed: %lu", ::GetLastError());
            ::abort();
        }

        if (m_Instance.mp.HasProcessors()) {
            m_Instance.pfnWndProc = reinterpret_cast<WNDPROC>(
                ::SetWindowLongPtrA(
                    hWnd,
                    GWLP_WNDPROC,
                    reinterpret_cast<LONG_PTR>(WndProc_Hook))
                );

            m_Instance.Message(
                "[0x%llX] Message hook installed", hWnd);
        }

        if (m_Instance.conf.upscale)
        {
            HMONITOR hMonitor = ::MonitorFromWindow(
                hWnd, MONITOR_DEFAULTTOPRIMARY);

            MONITORINFO mi;
            mi.cbSize = sizeof(mi);
            if (::GetMonitorInfoA(hMonitor, &mi))
            {
                int newWidth = mi.rcMonitor.right - mi.rcMonitor.left;
                int newHeight = mi.rcMonitor.bottom - mi.rcMonitor.top;

                if (::SetWindowPos(
                    hWnd,
                    HWND_TOP,
                    mi.rcMonitor.left,
                    mi.rcMonitor.top,
                    newWidth,
                    newHeight,
                    SWP_NOSENDCHANGING | SWP_ASYNCWINDOWPOS))
                {
                    X = mi.rcMonitor.left;
                    Y = mi.rcMonitor.top;
                    nWidth = newWidth;
                    nHeight = newHeight;
                    m_Instance.Message(
                        "[0x%llX] Window stretched across the screen", hWnd);
                }
                else {
                    m_Instance.Error(
                        "[0x%llX] SetWindowPos failed", hWnd);
                }
            }
            else {
                m_Instance.Error(
                    "GetMonitorInfo failed, upscaling won't work");
            }
        }

        m_Instance.Debug(
            "[0x%llX] Window created [%s] (%d,%d,%d,%d)",
            hWnd, lpWindowName,
            X, Y, nWidth, nHeight);

        return hWnd;
    }

    LRESULT CALLBACK DWindow::WndProc_Hook(
        HWND hWnd,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam)
    {
        LRESULT lr = ::CallWindowProcA(m_Instance.pfnWndProc, hWnd, uMsg, wParam, lParam);
        m_Instance.mp.Process(hWnd, uMsg, wParam, lParam);
        return lr;
    }

    BOOL WINAPI DWindow::GetClientRect_Hook(
        _In_ HWND hWnd,
        _Out_ LPRECT lpRect)
    {
        if (m_Instance.upscaling.hWnd != NULL &&
            m_Instance.upscaling.hWnd == hWnd)
        {
            *lpRect = m_Instance.upscaling.resolution;
            return TRUE;
        }

        return ::GetClientRect(hWnd, lpRect);
    }

    void DWindow::OnD3D11PreCreate_Upscale(Event code, void* data)
    {
        auto info = reinterpret_cast<D3D11CreateEventPre*>(data);

        if (info->m_pSwapChainDesc->Windowed == TRUE)
        {
            m_Instance.upscaling.resolution.top = 0;
            m_Instance.upscaling.resolution.left = 0;
            m_Instance.upscaling.resolution.right = info->m_pSwapChainDesc->BufferDesc.Width;
            m_Instance.upscaling.resolution.bottom = info->m_pSwapChainDesc->BufferDesc.Height;
            m_Instance.upscaling.hWnd = info->m_pSwapChainDesc->OutputWindow;
        }
    }
}