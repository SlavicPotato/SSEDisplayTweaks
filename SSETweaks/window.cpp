#include "pch.h"

namespace SDT
{
	static constexpr const char* CKEY_GHOSTING   = "DisableProcessWindowsGhosting";
	static constexpr const char* CKEY_LOCKCURSOR = "LockCursor";
	static constexpr const char* CKEY_FORCEMIN   = "ForceMinimize";
	static constexpr const char* CKEY_CENTER     = "AutoCenter";
	static constexpr const char* CKEY_OFFSETX    = "OffsetX";
	static constexpr const char* CKEY_OFFSETY    = "OffsetY";

	DWindow DWindow::m_Instance;

	bool MonitorInfo::GetPrimary(HMONITOR& a_handle)
	{
		FindDesc fd;

		::EnumDisplayMonitors(NULL, NULL, FindPrimary, reinterpret_cast<LPARAM>(&fd));

		if (fd.found)
			a_handle = fd.handle;

		return fd.found;
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

				fd->found  = true;
				fd->handle = a_handle;
				return FALSE;
			}
		}

		return TRUE;
	}

	DWindow::DWindow() :
		m_gv{ .iLocationX = nullptr, .iLocationY = nullptr },
		m_upscaling{ .resized = false, .hWnd = nullptr }
	{
	}

	void DWindow::LoadConfig()
	{
		m_conf.disable_ghosting = GetConfigValue(CKEY_GHOSTING, false);
		m_conf.lock_cursor      = GetConfigValue(CKEY_LOCKCURSOR, false);
		m_conf.force_minimize   = GetConfigValue(CKEY_FORCEMIN, false);
		m_conf.offset_x         = GetConfigValue<int>(CKEY_OFFSETX, 0);
		m_conf.offset_y         = GetConfigValue<int>(CKEY_OFFSETY, 0);

		auto rd        = IDDispatcher::GetDriver<DRender>();
		m_conf.upscale = rd && rd->IsOK() && rd->m_conf.upscale;

		if (rd->m_conf.fullscreen)
		{
			m_conf.center_window = false;
		}
		else
		{
			m_conf.center_window = GetConfigValue(CKEY_CENTER, false);
		}
	}

	void DWindow::PostLoadConfig()
	{
		if (m_conf.disable_ghosting)
		{
			::DisableProcessWindowsGhosting();
			Message("Ghosting disabled");
		}

		if (m_conf.lock_cursor)
		{
			SetupCursorLockMP();
			Message("Cursor locking enabled");
		}

		if (m_conf.force_minimize)
		{
			SetupForceMinimizeMP();
			Message("Forcing minimize on focus loss");
		}
	}

	void DWindow::SetupCursorLockMP()
	{
		m_mp.Add(
			{ WM_SETFOCUS },
			[&](HWND hWnd, UINT, WPARAM, LPARAM) {
				CaptureCursor(hWnd, true);
			});

		m_mp.Add(
			{ WM_WINDOWPOSCHANGED, WM_SIZING },
			[&](HWND hWnd, UINT, WPARAM, LPARAM) {
				CaptureCursor(hWnd, true);
			});

		m_mp.Add(WM_ACTIVATE, [&](HWND hWnd, UINT, WPARAM wParam, LPARAM) {
			auto fMinimized = static_cast<BOOL>(HIWORD(wParam));
			WORD fActive    = LOWORD(wParam);

			if (fActive == WA_ACTIVE)
			{
				if (!fMinimized)
				{
					if (::GetFocus() == hWnd)
					{
						CaptureCursor(hWnd, true);
					}
				}
			}
			else if (fActive == WA_INACTIVE)
			{
				CaptureCursor(hWnd, false);
			}
		});

		m_mp.Add(
			{ WM_KILLFOCUS, WM_DESTROY },
			[&](HWND hWnd, UINT, WPARAM, LPARAM) {
				CaptureCursor(hWnd, false);
			});
	}

	void DWindow::SetupForceMinimizeMP()
	{
		m_mp.Add(WM_KILLFOCUS, [&](HWND hWnd, UINT, WPARAM, LPARAM) {
			Debug("[0x%llX] Window minimized", hWnd);
			::ShowWindow(hWnd, SW_MINIMIZE);
		});
	}

	void DWindow::RegisterHooks()
	{
		if (m_mp.HasProcessors() || m_conf.upscale || m_conf.center_window)
		{
			if (!hook::call6(
					ISKSE::GetBranchTrampoline(),
					CreateWindowEx_C,
					reinterpret_cast<std::uintptr_t>(CreateWindowExA_Hook),
					m_createWindowExA_O))
			{
				Error("CreateWindowExA hook failed");
				SetOK(false);
				return;
			}
		}

		if (m_conf.upscale)
		{
			if (hook::call6(
					ISKSE::GetBranchTrampoline(),
					GetClientRect1_C,
					reinterpret_cast<std::uintptr_t>(GetClientRect_Hook),
					m_getClientRect_O))
			{
				IEvents::RegisterForEvent(Event::OnD3D11PreCreate, OnD3D11PreCreate_Upscale);
			}
			else
			{
				Error("GetClientRect hook failed, upscaling disabled");
				m_conf.upscale = false;
			}
		}

		if (m_conf.offset_x != 0 || m_conf.offset_y != 0)
		{
			IEvents::RegisterForEvent(Event::OnConfigLoad, PostConfigLoad);
		}
	}

	bool DWindow::Prepare()
	{
		return true;
	}

	void DWindow::OnGameConfigLoaded()
	{
		if (m_conf.upscale || m_conf.center_window || m_conf.offset_x != 0 || m_conf.offset_y != 0)
		{
			m_gv.iLocationX = ISKSE::GetINISettingAddr<int>("iLocation X:Display");
			ASSERT(m_gv.iLocationX);
			m_gv.iLocationY = ISKSE::GetINISettingAddr<int>("iLocation Y:Display");
			ASSERT(m_gv.iLocationY);
		}
	}

	void DWindow::PostConfigLoad(Event code, void* data)
	{
		if (m_Instance.m_conf.offset_x != 0 && m_Instance.m_gv.iLocationX)
		{
			*m_Instance.m_gv.iLocationX = m_Instance.m_conf.offset_x;
		}

		if (m_Instance.m_conf.offset_y != 0 && m_Instance.m_gv.iLocationY)
		{
			*m_Instance.m_gv.iLocationY = m_Instance.m_conf.offset_y;
		}
	}

	bool DWindow::SetCursorLock(HWND hwnd)
	{
		RECT rect;
		if (::GetWindowRect(hwnd, &rect))
		{
			return static_cast<bool>(::ClipCursor(&rect));
		}
		return false;
	}

	void DWindow::CaptureCursor(HWND hwnd, bool sw)
	{
		if (sw)
		{
			SetCursorLock(hwnd);
		}
		else
		{
			::ClipCursor(NULL);
		}
	}

	static bool GetMI(HWND a_windowHandle, bool a_primary, MONITORINFO* a_out)
	{
		HMONITOR hMonitor;
		bool     gotHandle(false);

		if (a_primary)
			gotHandle = MonitorInfo::GetPrimary(hMonitor);

		if (!gotHandle)
			hMonitor = ::MonitorFromWindow(a_windowHandle, MONITOR_DEFAULTTOPRIMARY);

		a_out->cbSize = sizeof(MONITORINFO);
		return ::GetMonitorInfoA(hMonitor, a_out);
	}

	void DWindow::DoUpscale(HWND a_windowHandle, int& X, int& Y, int& nWidth, int& nHeight)
	{
		auto rd = IDDispatcher::GetDriver<DRender>();

		MONITORINFO mi;
		if (!GetMI(a_windowHandle, rd->m_conf.upscale_select_primary_monitor, std::addressof(mi)))
		{
			Error("[0x%llX] [%s] GetMonitorInfo failed", a_windowHandle, __FUNCTION__);
			return;
		}

		int offsetx = 0, offsety = 0;

		if (m_gv.iLocationX &&
		    m_gv.iLocationY)
		{
			offsetx = *m_gv.iLocationX;
			offsety = *m_gv.iLocationY;
		}

		int monWidth  = static_cast<int>(mi.rcMonitor.right - mi.rcMonitor.left);
		int monHeight = static_cast<int>(mi.rcMonitor.bottom - mi.rcMonitor.top);

		int newX = static_cast<int>(mi.rcMonitor.left) + offsetx;
		int newY = static_cast<int>(mi.rcMonitor.top) + offsety;

		if (newX == X && newY == Y && monWidth == nWidth && monHeight == nHeight)
		{
			Debug("[0x%llX] [%s] Window position and dimensions unchanged", a_windowHandle, __FUNCTION__);
			return;
		}

		if (::SetWindowPos(a_windowHandle, HWND_TOP, newX, newY, monWidth, monHeight, SWP_NOSENDCHANGING | SWP_ASYNCWINDOWPOS))
		{
			X       = newX;
			Y       = newY;
			nWidth  = monWidth;
			nHeight = monHeight;

			m_upscaling.resized = true;

			Message("[0x%llX] Window stretched across the screen", a_windowHandle);
		}
		else
		{
			Error("[0x%llX] [%s] SetWindowPos failed", a_windowHandle, __FUNCTION__);
		}
	}

	void DWindow::DoCenter(HWND a_windowHandle, int& X, int& Y, int nWidth, int nHeight)
	{
		MONITORINFO mi;
		if (!GetMI(a_windowHandle, false, std::addressof(mi)))
		{
			Error("[0x%llX] [%s] GetMonitorInfo failed", a_windowHandle, __FUNCTION__);
			return;
		}

		int monWidth  = static_cast<int>(mi.rcMonitor.right - mi.rcMonitor.left);
		int monHeight = static_cast<int>(mi.rcMonitor.bottom - mi.rcMonitor.top);

		int newX = static_cast<int>(mi.rcMonitor.left) + (monWidth - nWidth) / 2;
		int newY = static_cast<int>(mi.rcMonitor.top) + (monHeight - nHeight) / 2;

		if (newX == X && newY == Y)
		{
			Debug("[0x%llX] [%s] Window position unchanged", a_windowHandle, __FUNCTION__);
			return;
		}

		if (::SetWindowPos(a_windowHandle, HWND_TOP, newX, newY, nWidth, nHeight, SWP_NOSENDCHANGING | SWP_ASYNCWINDOWPOS))
		{
			X = newX;
			Y = newY;

			Message("[0x%llX] Window centered", a_windowHandle);
		}
		else
		{
			Error("[0x%llX] [%s] SetWindowPos failed", a_windowHandle, __FUNCTION__);
		}
	}

	HWND WINAPI DWindow::CreateWindowExA_Hook(
		_In_ DWORD         dwExStyle,
		_In_opt_ LPCSTR    lpClassName,
		_In_opt_ LPCSTR    lpWindowName,
		_In_ DWORD         dwStyle,
		_In_ int           X,
		_In_ int           Y,
		_In_ int           nWidth,
		_In_ int           nHeight,
		_In_opt_ HWND      hWndParent,
		_In_opt_ HMENU     hMenu,
		_In_opt_ HINSTANCE hInstance,
		_In_opt_ LPVOID    lpParam)
	{
		HWND hWnd = m_Instance.m_createWindowExA_O(
			dwExStyle,
			lpClassName,
			lpWindowName,
			dwStyle,
			X,
			Y,
			nWidth,
			nHeight,
			hWndParent,
			hMenu,
			hInstance,
			lpParam);

		if (hWnd == NULL)
		{
			m_Instance.FatalError(
				"CreateWindowExA failed: %lu",
				::GetLastError());
			::abort();
		}

		if (m_Instance.m_mp.HasProcessors())
		{
			m_Instance.pfnWndProc = reinterpret_cast<WNDPROC>(
				::SetWindowLongPtrA(
					hWnd,
					GWLP_WNDPROC,
					reinterpret_cast<LONG_PTR>(WndProc_Hook)));

			if (m_Instance.pfnWndProc != 0)
			{
				m_Instance.Debug("[0x%llX] Message hook installed", hWnd);
			}
			else
			{
				m_Instance.Error("[0x%llX] SetWindowLongPtrA failed, window event capture won't work", hWnd);
			}
		}

		if (m_Instance.m_conf.upscale)
		{
			m_Instance.DoUpscale(hWnd, X, Y, nWidth, nHeight);
		}

		if (m_Instance.m_conf.center_window)
		{
			m_Instance.DoCenter(hWnd, X, Y, nWidth, nHeight);
		}

		m_Instance.Message(
			"[0x%llX] Window created [%s] (%d,%d,%d,%d)",
			hWnd,
			lpWindowName,
			X,
			Y,
			nWidth,
			nHeight);

		return hWnd;
	}

	LRESULT CALLBACK DWindow::WndProc_Hook(
		HWND   hWnd,
		UINT   uMsg,
		WPARAM wParam,
		LPARAM lParam)
	{
		LRESULT lr = ::CallWindowProcA(m_Instance.pfnWndProc, hWnd, uMsg, wParam, lParam);
		m_Instance.m_mp.Process(hWnd, uMsg, wParam, lParam);
		return lr;
	}

	BOOL WINAPI DWindow::GetClientRect_Hook(
		_In_ HWND    hWnd,
		_Out_ LPRECT lpRect)
	{
		if (m_Instance.m_upscaling.hWnd != nullptr &&
		    m_Instance.m_upscaling.hWnd == hWnd)
		{
			*lpRect = m_Instance.m_upscaling.resolution;
			return TRUE;
		}

		return ::GetClientRect(hWnd, lpRect);
	}

	void DWindow::OnD3D11PreCreate_Upscale(Event code, void* data)
	{
		if (!m_Instance.m_upscaling.resized)
			return;

		auto info = reinterpret_cast<D3D11CreateEventPre*>(data);

		if (info->m_pSwapChainDesc->Windowed == TRUE)
		{
			m_Instance.m_upscaling.resolution.top    = 0;
			m_Instance.m_upscaling.resolution.left   = 0;
			m_Instance.m_upscaling.resolution.right  = info->m_pSwapChainDesc->BufferDesc.Width;
			m_Instance.m_upscaling.resolution.bottom = info->m_pSwapChainDesc->BufferDesc.Height;
			m_Instance.m_upscaling.hWnd              = info->m_pSwapChainDesc->OutputWindow;
		}
	}
}