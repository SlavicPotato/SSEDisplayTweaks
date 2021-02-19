#pragma once

namespace SDT
{
    constexpr uint32_t DXGI_CAP_FLIP_DISCARD = 0x00000001U;
    constexpr uint32_t DXGI_CAP_FLIP_SEQUENTIAL = 0x00000002U;
    constexpr uint32_t DXGI_CAP_TEARING = 0x00000004U;

    constexpr uint32_t DXGI_CAPS_ALL = 0xFFFFFFFFU;

    struct MenuFramerateLimitDescriptor
    {
        MenuFramerateLimitDescriptor() : enabled(false) {};
        MenuFramerateLimitDescriptor(
            bool a_disable_vsync,
            long long a_limit)
            :
            enabled(true),
            disable_vsync(a_disable_vsync),
            limit(a_limit)
        {};

        bool enabled;
        bool disable_vsync;
        long long limit;
    };

    class MenuFramerateLimit :
        public MenuEventTrack
    {
        //using data_t = stl::unordered_map<MenuEvent, MenuFramerateLimitDescriptor>;
    public:

        using MenuEventTrack::MenuEventTrack;

        void SetLimit(MenuEvent code, float limit, bool disable_vsync);
        bool GetLimit(MenuEvent code, MenuFramerateLimitDescriptor& limit) const;
        bool HasLimits() const;
        bool HasLimit(MenuEvent code) const;
        bool GetCurrentLimit(MenuFramerateLimitDescriptor& limit) const;
    private:

        MenuFramerateLimitDescriptor m_limits[Enum::Underlying(MenuEvent::Max)];
        bool m_hasLimits;
    };

    class DRender :
        public IDriver,
        IConfig
    {
        typedef stl::iunordered_map<std::string, int> SEMap;
        static SEMap cfgSwapEffectMap;

        typedef stl::iunordered_map<std::string, DXGI_MODE_SCALING> SMMap;
        static SMMap cfgScalingModeMap;

        typedef HRESULT(WINAPI* CreateDXGIFactory_T)(REFIID riid, _COM_Outptr_ void** ppFactory);

    public:
        static inline constexpr auto ID = DRIVER_ID::RENDER;

        typedef void(*RTProcR) (void);
        typedef void(*PhysCalcR) (void*, int32_t);

        struct {
            uint8_t fullscreen;
            uint8_t borderless;
            bool upscale;
            bool upscale_select_primary_monitor;
            bool disablebufferresize;
            bool disabletargetresize;
            bool vsync_on;
            uint32_t vsync_present_interval;
            int swap_effect;
            DXGI_MODE_SCALING scaling_mode;
            int32_t max_rr;
            int32_t buffer_count;
            int32_t max_frame_latency;
            bool enable_tearing;
            int32_t resolution[2];
            float resolution_scale;

            struct {
                float game;

                float ui;
                float ui_loadscreen;
                float ui_map;
                float ui_inventory;
                float ui_journal;
                float ui_custom;
                float ui_main;
                float ui_race;
                float ui_perk;
                float ui_book;
                float ui_lockpick;
                float ui_console;
                float ui_tween;
                float ui_sw;

                bool ui_dv;
                bool ui_loadscreen_dv;
                bool ui_map_dv;
                bool ui_inventory_dv;
                bool ui_journal_dv;
                bool ui_custom_dv;
                bool ui_main_dv;
                bool ui_race_dv;
                bool ui_perk_dv;
                bool ui_book_dv;
                bool ui_lockpick_dv;
                bool ui_console_dv;
                bool ui_tween_dv;
                bool ui_sw_dv;

                int32_t ui_loadscreenex;
                int32_t ui_initialloadex;
            } limits;

            bool adjust_ini;
        } m_conf;

        [[nodiscard]] float GetMaxFramerate(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc) const;
        [[nodiscard]] bool IsLimiterInstalled() { return limiter_installed; }

        [[nodiscard]] bool QueryVideoMemoryInfo(
            IDXGISwapChain* a_swapChain,
            DXGI_QUERY_VIDEO_MEMORY_INFO& a_out) const;

        FN_NAMEPROC("Render")
        FN_ESSENTIAL(false)
        FN_DRVDEF(2)
    private:
        DRender();

        virtual void LoadConfig() override;
        virtual void PostLoadConfig() override;
        virtual void Patch() override;
        virtual void RegisterHooks() override;
        virtual bool Prepare() override;

        static DXGI_SWAP_EFFECT GetSwapEffect(int a_code);
        static const char* GetMenuDescription(MenuEvent a_event);
        static const char* GetSwapEffectOption(DXGI_SWAP_EFFECT a_swapEffect);

        void UISetLimit(MenuEvent code, float limit, bool disable_vsync);

        bool ConfigTranslateSwapEffect(const std::string& param, int& out) const;
        bool ConfigTranslateScalingMode(const std::string& param, DXGI_MODE_SCALING& out) const;
        static bool ConfigParseResolution(const std::string& in, int32_t(&a_out)[2]);

        bool ValidateDisplayMode(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc) const;
        UINT GetRefreshRate(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc) const;
        DXGI_SWAP_EFFECT AutoGetSwapEffect(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc) const;
        DXGI_SWAP_EFFECT ManualGetSwapEffect(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc);
        void ApplyD3DSettings(DXGI_SWAP_CHAIN_DESC* pSwapChainDesc);

        static void Throttle();

        static void OnD3D11PreCreate(IDXGIAdapter* pAdapter, const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc);
        static void OnD3D11PostCreate(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, ID3D11Device** ppDevice);

        static HRESULT WINAPI D3D11CreateDeviceAndSwapChain_Hook(_In_opt_ IDXGIAdapter* pAdapter,
            D3D_DRIVER_TYPE DriverType,
            HMODULE Software,
            UINT Flags,
            _In_reads_opt_(FeatureLevels) CONST D3D_FEATURE_LEVEL* pFeatureLevels,
            UINT FeatureLevels,
            UINT SDKVersion,
            _In_opt_ CONST DXGI_SWAP_CHAIN_DESC* pSwapChainDesc,
            _COM_Outptr_opt_ IDXGISwapChain** ppSwapChain,
            _COM_Outptr_opt_ ID3D11Device** ppDevice,
            _Out_opt_ D3D_FEATURE_LEVEL* pFeatureLevel,
            _COM_Outptr_opt_ ID3D11DeviceContext** ppImmediateContext);

        static HRESULT WINAPI CreateDXGIFactory_Hook(REFIID riid, _COM_Outptr_ void** ppFactory);

        IDXGIFactory *DXGI_GetFactory() const;
        void DXGI_GetCapabilities();
        bool HasWindowedHWCompositionSupport(IDXGIAdapter* adapter) const;


        static void MessageHandler(Event m_code, void* args);
        static void OnConfigLoad(Event m_code, void* args);

        bool HandleMenuEvent(MenuEvent a_code, MenuOpenCloseEvent* a_evn);
        static bool OnMenuEvent(MenuEvent m_code, MenuOpenCloseEvent* evn, EventDispatcher<MenuOpenCloseEvent>* dispatcher);

        void SetFPSLimitOverride(long long max, bool disable_vsync);
        void SetFPSLimitPost(long long a_max, long long a_expire);
        void ResetFPSLimitOverride();

        void QueueFPSLimitOverride(long long max, bool disable_vsync);
        void QueueFPSLimitPost(long long a_max, long long a_expire);
        void QueueFPSLimitOverrideReset();

        long long tts;
        int fps_limit;
        bool has_swap_effect;
        bool has_scaling_mode;
        uint32_t vsync;
        float fmt_max;
        float fmt_min;
        long long fps_max;
        bool tearing_enabled;
        long long current_fps_max, oo_current_fps_max, oo_expire_time;
        long long lslExtraTime, lslPostLoadExtraTime;
        uint8_t gameLoadState;
        bool has_fl_override;
        bool limiter_installed;

        MenuFramerateLimit m_fl;

        SKMP_FORCEINLINE bool HasLimits() {
            return fps_limit == 1 || m_fl.HasLimits();
        }

        SKMP_FORCEINLINE bool IsFlipOn(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc) {
            return pSwapChainDesc->SwapEffect == DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL ||
                pSwapChainDesc->SwapEffect == DXGI_SWAP_EFFECT_FLIP_DISCARD;
        }

        UINT present_flags;

        uint8_t* bLockFramerate;
        int32_t* iFPSClamp;
        int32_t* iSizeW;
        int32_t* iSizeH;

        IDXGIFactory* m_dxgiFactory;

        PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN D3D11CreateDeviceAndSwapChain_O;
        CreateDXGIFactory_T CreateDXGIFactory_O;

        inline static auto CreateDXGIFactory_C = IAL::Addr(AID::D3D11Create, Offsets::CreateDXGIFactory_C);
        inline static auto D3D11CreateDeviceAndSwapChain_C = IAL::Addr(AID::D3D11Create, Offsets::D3D11CreateDeviceAndSwapChain_C);
        inline static auto Present_Limiter = IAL::Addr(AID::Present, Offsets::Present_Limiter);
        inline static auto Present_Flags_Inject = IAL::Addr(AID::Present, Offsets::Present_Flags_Inject);

        inline static auto bFullscreen_Patch = IAL::Addr(AID::Init0, Offsets::bFullscreen_Patch);
        inline static auto bBorderless_Patch = IAL::Addr(AID::Init0, Offsets::bBorderless_Patch);
        inline static auto iSizeW_Patch = IAL::Addr(AID::Init0, Offsets::iSizeW_Patch);
        inline static auto iSizeH_Patch = IAL::Addr(AID::Init0, Offsets::iSizeH_Patch);
        inline static auto DisplayRefreshRate = IAL::Addr(AID::Init0, Offsets::DisplayRefreshRate);

        inline static auto DXGIData = IAL::Addr<Structures::IDXGIData**>(AID::DXGIData);

        inline static auto MaxFrameLatency = IAL::Addr(AID::D3DInit, Offsets::MaxFrameLatency);
        inline static auto ResizeBuffers_Inject = IAL::Addr(AID::WindowSwapChainAdjust, Offsets::ResizeBuffers_Inject);
        inline static auto ResizeBuffersDisable = IAL::Addr(AID::WindowSwapChainAdjust, Offsets::ResizeBuffersDisable);
        inline static auto ResizeTargetDisable = IAL::Addr(AID::WindowSwapChain2, Offsets::ResizeTargetDisable);
        inline static auto ResizeTarget = IAL::Addr(AID::WindowSwapChain2, Offsets::ResizeTarget);

        struct {
            uint32_t width;
            uint32_t height;
            uint32_t flags;
        }swapchain;

        DXGI_MODE_DESC modeDesc;

        struct
        {
            uint32_t caps;
        } dxgi;

        TaskQueue m_afTasks;

        static DRender m_Instance;
    };

    class D3D11CreateEvent
    {
    public:
        D3D11CreateEvent(
            CONST DXGI_SWAP_CHAIN_DESC* pSwapChainDesc) :
            m_pSwapChainDesc(pSwapChainDesc)
        {}

        CONST DXGI_SWAP_CHAIN_DESC* const m_pSwapChainDesc;
    };

    typedef D3D11CreateEvent D3D11CreateEventPre;

    class D3D11CreateEventPost :
        public D3D11CreateEvent
    {
    public:
        D3D11CreateEventPost(
            CONST DXGI_SWAP_CHAIN_DESC* pSwapChainDesc,
            ID3D11Device* pDevice,
            ID3D11DeviceContext* pImmediateContext,
            IDXGISwapChain* pSwapChain,
            IDXGIAdapter* pAdapter
        ) :
            D3D11CreateEvent(pSwapChainDesc),
            m_pDevice(pDevice),
            m_pImmediateContext(pImmediateContext),
            m_pSwapChain(pSwapChain),
            m_pAdapter(pAdapter)
        {}

        ID3D11Device* const m_pDevice;
        ID3D11DeviceContext* const m_pImmediateContext;
        IDXGISwapChain* const m_pSwapChain;
        IDXGIAdapter* const m_pAdapter;
    };

}