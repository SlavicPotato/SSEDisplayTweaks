#pragma once

namespace SDT
{
	static constexpr std::uint32_t DXGI_CAP_FLIP_DISCARD    = 0x00000001U;
	static constexpr std::uint32_t DXGI_CAP_FLIP_SEQUENTIAL = 0x00000002U;
	static constexpr std::uint32_t DXGI_CAP_TEARING         = 0x00000004U;

	static constexpr std::uint32_t DXGI_CAPS_ALL = 0xFFFFFFFFU;

	struct MenuFramerateLimitDescriptor
	{
		MenuFramerateLimitDescriptor() :
			enabled(false){};
		MenuFramerateLimitDescriptor(
			bool      a_disable_vsync,
			long long a_limit) :
			enabled(true),
			disable_vsync(a_disable_vsync),
			limit(a_limit){};

		bool      enabled;
		bool      disable_vsync;
		long long limit;
	};

	class MenuFramerateLimit :
		public MenuEventTrack
	{
		//using data_t = std::unordered_map<MenuEvent, MenuFramerateLimitDescriptor>;
	public:
		using MenuEventTrack::MenuEventTrack;

		void SetLimit(MenuEvent code, float limit, bool disable_vsync);
		bool GetLimit(MenuEvent code, MenuFramerateLimitDescriptor& limit) const;
		bool HasLimits() const;
		bool HasLimit(MenuEvent code) const;
		bool GetCurrentLimit(MenuFramerateLimitDescriptor& limit) const;

	private:
		MenuFramerateLimitDescriptor m_limits[stl::underlying(MenuEvent::Max)];
		bool                         m_hasLimits;
	};

	class FramerateLimiter;

	class DRender :
		public IDriver,
		IConfig
	{
		typedef stl::iunordered_map<std::string, int> SEMap;
		static SEMap                                  cfgSwapEffectMap;

		typedef stl::iunordered_map<std::string, DXGI_MODE_SCALING> SMMap;
		static SMMap                                                cfgScalingModeMap;

		typedef HRESULT(WINAPI* CreateDXGIFactory_T)(REFIID riid, _COM_Outptr_ void** ppFactory);

		typedef void (*presentCallback_t)(IDXGISwapChain* pSwapChain);

	public:
		static inline constexpr auto ID = DRIVER_ID::RENDER;

		typedef void (*RTProcR)(void);
		typedef void (*PhysCalcR)(void*, std::int32_t);

		struct
		{
			std::uint8_t      fullscreen;
			std::uint8_t      borderless;
			bool              upscale;
			bool              upscale_select_primary_monitor;
			bool              disablebufferresize;
			bool              disabletargetresize;
			bool              vsync_on;
			std::uint32_t     vsync_present_interval;
			int               swap_effect;
			DXGI_MODE_SCALING scaling_mode;
			std::int32_t      max_rr;
			std::int32_t      buffer_count;
			std::int32_t      max_frame_latency;
			bool              enable_tearing;
			std::int32_t      resolution[2];
			float             resolution_scale;
			std::uint8_t      limit_mode;

			struct
			{
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

				std::int32_t ui_loadscreenex;
				std::int32_t ui_initialloadex;
			} limits;

			bool adjust_ini;
		} m_conf;

		[[nodiscard]] float GetMaxFramerate(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc) const;
		[[nodiscard]] bool  IsLimiterInstalled() { return limiter_installed; }

		SKMP_FORCEINLINE void AddPresentCallbackPre(presentCallback_t f)
		{
			m_presentCallbacksPre.emplace_back(f);
		}

		SKMP_FORCEINLINE void AddPresentCallbackPost(presentCallback_t f)
		{
			m_presentCallbacksPost.emplace_back(f);
		}

		FN_NAMEPROC("Render");
		FN_ESSENTIAL(false);
		FN_DRVDEF(2);

	private:
		DRender();

		virtual void LoadConfig() override;
		virtual void PostLoadConfig() override;
		virtual void Patch() override;
		virtual void RegisterHooks() override;
		virtual bool Prepare() override;
		virtual void PostInit() override;
		virtual void OnGameConfigLoaded() override;

		std::uint8_t GetScreenModeSetting(IConfigGame& a_gameConfig, const char* a_key, const char* a_prefkey, bool a_default);

		static DXGI_SWAP_EFFECT GetSwapEffect(int a_code);
		static const char*      GetMenuDescription(MenuEvent a_event);
		static const char*      GetSwapEffectOption(DXGI_SWAP_EFFECT a_swapEffect);

		void UISetLimit(MenuEvent code, float limit, bool disable_vsync);

		bool        ConfigTranslateSwapEffect(const std::string& param, int& out) const;
		bool        ConfigTranslateScalingMode(const std::string& param, DXGI_MODE_SCALING& out) const;
		static bool ConfigParseResolution(const std::string& in, std::int32_t (&a_out)[2]);

		bool             ValidateDisplayMode(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc) const;
		UINT             GetRefreshRate(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc) const;
		DXGI_SWAP_EFFECT AutoGetSwapEffect(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc) const;
		DXGI_SWAP_EFFECT ManualGetSwapEffect(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc);
		void             ApplyD3DSettings(DXGI_SWAP_CHAIN_DESC* pSwapChainDesc);

		SKMP_FORCEINLINE static long long GetCurrentFramerateLimit();
		static void                       Throttle(IDXGISwapChain*);

		void OnD3D11PreCreate(IDXGIAdapter* pAdapter, const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc);
		//static void OnD3D11PostCreate(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, ID3D11Device** ppDevice);

		static HRESULT WINAPI D3D11CreateDeviceAndSwapChain_Hook(_In_opt_ IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, _In_reads_opt_(FeatureLevels) CONST D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, _In_opt_ CONST DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, _COM_Outptr_opt_ IDXGISwapChain** ppSwapChain, _COM_Outptr_opt_ ID3D11Device** ppDevice, _Out_opt_ D3D_FEATURE_LEVEL* pFeatureLevel, _COM_Outptr_opt_ ID3D11DeviceContext** ppImmediateContext);

		static HRESULT WINAPI            CreateDXGIFactory_Hook(REFIID riid, _COM_Outptr_ void** ppFactory);
		static HRESULT STDMETHODCALLTYPE Present_Hook(
			IDXGISwapChain4* pSwapChain,
			UINT             SyncInterval,
			UINT             PresentFlags);

		IDXGIFactory* DXGI_GetFactory() const;
		void          DXGI_GetCapabilities();
		bool          HasWindowedHWCompositionSupport(IDXGIAdapter* adapter) const;

		static void MessageHandler(Event m_code, void* args);
		static void OnConfigLoad(Event m_code, void* args);

		bool        HandleMenuEvent(MenuEvent a_code, const MenuOpenCloseEvent* a_evn);
		static bool OnMenuEvent(MenuEvent m_code, const MenuOpenCloseEvent* evn, BSTEventSource<MenuOpenCloseEvent>* dispatcher);

		void SetFPSLimitOverride(long long max, bool disable_vsync);
		void SetFPSLimitPost(long long a_max, long long a_expire);
		void ResetFPSLimitOverride();

		void QueueFPSLimitOverride(long long max, bool disable_vsync);
		void QueueFPSLimitPost(long long a_max, long long a_expire);
		void QueueFPSLimitOverrideReset();

		long long    tts;
		int          fps_limit;
		bool         has_swap_effect;
		bool         has_scaling_mode;
		float        fmt_max;
		float        fmt_min;
		long long    fps_max;
		bool         tearing_enabled;
		long long    current_fps_max, oo_current_fps_max, oo_expire_time;
		long long    lslExtraTime, lslPostLoadExtraTime;
		std::uint8_t gameLoadState;
		bool         has_fl_override;
		bool         limiter_installed;

		MenuFramerateLimit m_fl;

		SKMP_FORCEINLINE bool HasLimits()
		{
			return fps_limit == 1 || m_fl.HasLimits();
		}

		SKMP_FORCEINLINE bool IsFlipOn(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc)
		{
			return pSwapChainDesc->SwapEffect == DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL ||
			       pSwapChainDesc->SwapEffect == DXGI_SWAP_EFFECT_FLIP_DISCARD;
		}

		UINT m_vsync_present_interval;
		UINT m_current_vsync_present_interval;
		UINT m_present_flags;

		std::int64_t m_originalResW{ 0 };
		std::int64_t m_originalResH{ 0 };

		struct
		{
			std::uint8_t* bLockFramerate{ nullptr };
			std::int32_t* iFPSClamp{ nullptr };
			std::int32_t* iSizeW{ nullptr };
			std::int32_t* iSizeH{ nullptr };
		} m_gv;

		IDXGIFactory* m_dxgiFactory;

		PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN m_D3D11CreateDeviceAndSwapChain_O;
		CreateDXGIFactory_T                    m_createDXGIFactory_O;

		std::vector<presentCallback_t> m_presentCallbacksPre;
		std::vector<presentCallback_t> m_presentCallbacksPost;

		inline static auto CreateDXGIFactory_C             = IAL::Addr(AID::D3D11Create, 77396, Offsets::CreateDXGIFactory_C, 0x25);
		inline static auto D3D11CreateDeviceAndSwapChain_C = IAL::Addr(AID::D3D11Create, 77396, Offsets::D3D11CreateDeviceAndSwapChain_C, 0x2C0);
		inline static auto Present_Flags_Inject            = IAL::Addr(AID::Present, 77246, Offsets::Present_Flags_Inject, 0x8E);
		inline static auto presentAddr                     = IAL::Addr(AID::Present, 77246, Offsets::Present, 0x9F);

		inline static auto bFullscreen_Patch  = IAL::Addr(AID::Init0, 36547, Offsets::bFullscreen_Patch, IAL::ver() >= VER_1_6_342 ? IAL::ver() >= VER_1_6_629 ? 0xCEF : 0xCCF : 0xCB0);
		inline static auto bBorderless_Patch  = IAL::Addr(AID::Init0, 36547, Offsets::bBorderless_Patch, IAL::ver() >= VER_1_6_342 ? IAL::ver() >= VER_1_6_629 ? 0xCFA : 0xCDA : 0xCBB);
		inline static auto iSizeW_Patch       = IAL::Addr(AID::Init0, 36547, Offsets::iSizeW_Patch, IAL::ver() >= VER_1_6_342 ? IAL::ver() >= VER_1_6_629 ? 0xD05 : 0xCE5 : 0xCC6);
		inline static auto iSizeH_Patch       = IAL::Addr(AID::Init0, 36547, Offsets::iSizeH_Patch, IAL::ver() >= VER_1_6_342 ? IAL::ver() >= VER_1_6_629 ? 0xD0F : 0xCEF : 0xCD0);
		inline static auto DisplayRefreshRate = IAL::Addr(AID::Init0, 36547, Offsets::DisplayRefreshRate, IAL::ver() >= VER_1_6_342 ? IAL::ver() >= VER_1_6_629 ? 0xD2D : 0xD0D : 0xCEE);

		//inline static auto DXGIData = IAL::Addr<Structures::IDXGIData**>(AID::DXGIData);

		inline static auto MaxFrameLatency      = IAL::Addr(AID::D3DInit, 77226, Offsets::MaxFrameLatency, 0x2FE);
		inline static auto ResizeBuffers_Inject = IAL::Addr(AID::WindowSwapChainAdjust, 77238, Offsets::ResizeBuffers_Inject, 0x2C4);
		inline static auto ResizeBuffersDisable = IAL::Addr(AID::WindowSwapChainAdjust, 77238, Offsets ::ResizeBuffersDisable, 0x26);
		inline static auto ResizeTargetDisable  = IAL::Addr(AID::WindowSwapChain2, 77239, Offsets::ResizeTargetDisable, 0x24);
		inline static auto ResizeTarget         = IAL::Addr(AID::WindowSwapChain2, 77239, Offsets::ResizeTarget, 0xF9);

		struct
		{
			std::uint32_t width;
			std::uint32_t height;
			std::uint32_t flags;
		} m_swapchain;

		DXGI_MODE_DESC modeDesc;

		struct
		{
			std::uint32_t caps;
		} m_dxgi;

		TaskQueue                         m_afTasks;
		std::unique_ptr<FramerateLimiter> m_limiter;

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
			ID3D11Device*               pDevice,
			ID3D11DeviceContext*        pImmediateContext,
			IDXGISwapChain*             pSwapChain,
			IDXGIAdapter*               pAdapter) :
			D3D11CreateEvent(pSwapChainDesc),
			m_pDevice(pDevice),
			m_pImmediateContext(pImmediateContext),
			m_pSwapChain(pSwapChain),
			m_pAdapter(pAdapter)
		{}

		ID3D11Device* const        m_pDevice;
		ID3D11DeviceContext* const m_pImmediateContext;
		IDXGISwapChain* const      m_pSwapChain;
		IDXGIAdapter* const        m_pAdapter;
	};

}