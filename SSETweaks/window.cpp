#include "pch.h"

namespace SDT
{
    static constexpr const char* CKEY_GHOSTING = "DisableProcessWindowsGhosting";
    static constexpr const char* CKEY_LOCKCURSOR = "LockCursor";
    static constexpr const char* CKEY_FORCEMIN = "ForceMinimize";

    DWindow::CreateWindowExA_T DWindow::CreateWindowExA_O;

    DWindow DWindow::m_Instance;

    bool MonitorInfo::GetPrimary(HMONITOR& a_handle)
    {
        FindDesc fd;

        fd.found = false;

        ::EnumDisplayMonitors(NULL, NULL, FindPrimary, reinterpret_cast<LPARAM>(&fd));

        bool found = fd.found;

        if (found)
            a_handle = fd.handle;

        return found;
    }

    BOOL CALLBACK MonitorInfo::FindPrimary(HMONITOR a_handle, HDC, LPRECT, LPARAM a_out)
    {
        MONITORINFO mi;
        mi.cbSize = sizeof(mi);
        if (::GetMonitorInfoA(a_handle, &mi))
        {
            if (mi.dwFlags & MONITORINFOF_PRIMARY)
            {
                auto fd = reinterpret_cast<FindDesc*>(a_out);

                fd->found = true;
                fd->handle = a_handle;
                return FALSE;
            }
        }

        return TRUE;
    }

    DWindow::DWindow() :
        iLocationX(nullptr),
        iLocationY(nullptr)
    {
        upscaling.hWnd = NULL;
    }

    void DWindow::LoadConfig()
    {
        m_conf.disable_ghosting = GetConfigValue(CKEY_GHOSTING, false);
        m_conf.lock_cursor = GetConfigValue(CKEY_LOCKCURSOR, false);
        m_conf.force_minimize = GetConfigValue(CKEY_FORCEMIN, false);

        auto rd = IDDispatcher::GetDriver<DRender>();
        m_conf.upscale = rd && rd->IsOK() && rd->m_conf.upscale;
    }

    void DWindow::PostLoadConfig()
    {
        if (m_conf.disable_ghosting) {
            ::DisableProcessWindowsGhosting();
            Message("Ghosting disabled");
        }

        if (m_conf.lock_cursor) {
            SetupCursorLockMP();
            Message("Cursor locking enabled");
        }

        if (m_conf.force_minimize) {
            SetupForceMinimizeMP();
            Message("Forcing minimize on focus loss");
        }

        if (m_conf.upscale) {
            iLocationX = ISKSE::GetINISettingAddr<int>("iLocation X:Display");
            iLocationY = ISKSE::GetINISettingAddr<int>("iLocation Y:Display");
        }
    }

    void DWindow::SetupCursorLockMP()
    {
        mp.Add(
            { WM_SETFOCUS , WM_CAPTURECHANGED },
            [&](HWND hWnd, UINT, WPARAM, LPARAM)
            {
                if (::GetFocus() == hWnd && ::GetActiveWindow() == hWnd) {
                    CaptureCursor(hWnd, true);
                }
            });

        mp.Add(
            { WM_WINDOWPOSCHANGED, WM_SIZING },
            [&](HWND hWnd, UINT, WPARAM, LPARAM)
            {
                CaptureCursor(hWnd, true);
            });

        mp.Add(WM_ACTIVATE,
            [&](HWND hWnd, UINT, WPARAM wParam, LPARAM)
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

        mp.Add(
            { WM_KILLFOCUS, WM_DESTROY },
            [&](HWND hWnd, UINT, WPARAM, LPARAM)
            {
                CaptureCursor(hWnd, false);
            });
    }

    void DWindow::SetupForceMinimizeMP()
    {
        mp.Add(WM_KILLFOCUS,
            [&](HWND hWnd, UINT, WPARAM, LPARAM)
            {
                Debug("[0x%llX] Window minimized", hWnd);
                ::ShowWindow(hWnd, SW_MINIMIZE);
            });
    }

    void DWindow::RegisterHooks()
    {
        if (mp.HasProcessors() || m_conf.upscale) {
            if (!Hook::Call6(CreateWindowEx_C, reinterpret_cast<uintptr_t>(CreateWindowExA_Hook), CreateWindowExA_O)) {
                Error("CreateWindowExA hook failed");
                SetOK(false);
                return;
            }
        }

        if (m_conf.upscale) {
            RegisterHook(
                GetClientRect1,
                reinterpret_cast<uintptr_t>(GetClientRect_Hook),
                HookDescriptor::HookType::kWR6Call
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

        if (m_Instance.m_conf.upscale)
        {
            int offsetx = 0;
            int offsety = 0;

            if (m_Instance.iLocationX)
                offsetx = *m_Instance.iLocationX;

            if (m_Instance.iLocationY)
                offsety = *m_Instance.iLocationY;

            HMONITOR hMonitor;
            bool gotHandle(false);

            auto rd = IDDispatcher::GetDriver<DRender>();

            if (rd->m_conf.upscale_select_primary_monitor)
                gotHandle = MonitorInfo::GetPrimary(hMonitor);

            if (!gotHandle)
                hMonitor = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);

            MONITORINFO mi;
            mi.cbSize = sizeof(mi);

            if (::GetMonitorInfoA(hMonitor, &mi))
            {
                int newX = static_cast<int>(mi.rcMonitor.left) + offsetx;
                int newY = static_cast<int>(mi.rcMonitor.top) + offsety;

                int newWidth = static_cast<int>(mi.rcMonitor.right - mi.rcMonitor.left);
                int newHeight = static_cast<int>(mi.rcMonitor.bottom - mi.rcMonitor.top);

                if (::SetWindowPos(
                    hWnd,
                    HWND_TOP,
                    newX,
                    newY,
                    newWidth,
                    newHeight,
                    SWP_NOSENDCHANGING | SWP_ASYNCWINDOWPOS))
                {
                    X = newX;
                    Y = newY;
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