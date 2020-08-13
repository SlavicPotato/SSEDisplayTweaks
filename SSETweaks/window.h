#pragma once

#define WMSG_INVOKELOCK		"mfd_invokelock"

namespace SDT
{
    class MsgProc
    {
        typedef std::function<void(HWND, UINT, WPARAM, LPARAM)> MsgProcFunc;
        typedef std::unordered_map<UINT, std::vector<MsgProcFunc>> MPMap;
    public:
        typedef std::vector<UINT> MsgList;

        void Add(UINT msg, const MsgProcFunc& f)
        {
            m_map[msg].push_back(f);
        }

        void Add(UINT msg, MsgProcFunc&& f)
        {
            m_map[msg].emplace_back(std::forward<MsgProcFunc>(f));
        }

        void Add(const MsgList& l, const MsgProcFunc& f)
        {
            for (const auto msg : l)
                Add(msg, f);
        }

        void Add(const MsgList& l, MsgProcFunc&& f)
        {
            for (const auto msg : l)
                Add(msg, std::forward<MsgProcFunc>(f));
        }

        inline bool HasProcessors() const noexcept
        {
            return m_map.size() != 0;
        }

        void Process(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
        {
            auto it = m_map.find(uMsg);
            if (it != m_map.end()) {
                for (const auto& f : it->second) {
                    f(hWnd, uMsg, wParam, lParam);
                }
            }
        }
    private:
        MPMap m_map;
    };

    class DWindow :
        public IDriver,
        IConfig
    {
        typedef HWND (WINAPI* CreateWindowExA_T)(
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
            _In_opt_ LPVOID lpParam);

    public:
        FN_NAMEPROC("Window")
        FN_ESSENTIAL(false)
        FN_PRIO(4)
        FN_DRVID(DRIVER_WINDOW)

    private:
        DWindow();

        typedef BOOL
        (WINAPI* GetClientRect_pfn)(
            _In_ HWND hWnd,
            _Out_ LPRECT lpRect);

        /*typedef HWND(WINAPI* PFN_CreateWindowExA)
            (	_In_ DWORD dwExStyle,
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
                _In_opt_ LPVOID lpParam);
                */
        static HWND WINAPI CreateWindowExA_Hook(
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
            _In_opt_ LPVOID lpParam);

        static CreateWindowExA_T CreateWindowExA_O;

        virtual void LoadConfig() override;
        virtual void PostLoadConfig() override;
        virtual void RegisterHooks() override;
        virtual bool Prepare() override;

        void SetupCursorLockMP();
        void SetupForceMinimizeMP();

        bool SetCursorLock(HWND hwnd);
        void CaptureCursor(HWND hwnd, bool sw);

        struct {
            HWND hWnd;
            RECT MonitorRes, resolution;
        } upscaling;

        WNDPROC pfnWndProc;
        static void OnD3D11PreCreate_Upscale(Event code, void* data);

        static BOOL WINAPI
            GetClientRect_Hook(
                _In_ HWND hWnd,
                _Out_ LPRECT lpRect);

        static LRESULT CALLBACK WndProc_Hook(
            HWND hWnd,
            UINT uMsg,
            WPARAM wParam,
            LPARAM lParam);

        struct {
            bool disable_ghosting;
            bool lock_cursor;
            bool force_minimize;
            bool upscale;
        }conf;

        MsgProc mp;

        inline static uintptr_t CreateWindowEx_C = IAL::Addr(AID::WindowCreate, Offsets::WindowCreate);
        inline static uintptr_t GetClientRect1 = IAL::Addr(AID::WinFunc0, Offsets::GetClientRect1);

        static DWindow m_Instance;
    };
}