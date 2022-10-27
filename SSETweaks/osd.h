#pragma once

namespace SDT
{
	class D3D11CreateEventPost;
	class DRender;

	class SKMP_ALIGN(16) StatsRenderer
	{
	public:
		SKMP_ALIGNED_REDEFINE_NEW_MM(16);

		typedef const wchar_t* (*Callback)(void);

		enum Align
		{
			TOP_LEFT = 0,
			TOP_RIGHT,
			BOTTOM_LEFT,
			BOTTOM_RIGHT
		};

		StatsRenderer(
			ID3D11Device*               a_pDevice,
			ID3D11DeviceContext*        a_pDeviceContext,
			UINT                        a_bufferX,
			UINT                        a_bufferY,
			const DirectX::XMFLOAT2A&   a_offset       = { 4.0f, 4.0f },
			float                       a_outlineSize  = 1.0f,
			Align                       a_al           = TOP_LEFT,
			const DirectX::XMFLOAT2A&   a_scale        = { 1.0f, 1.0f },
			const DirectX::XMVECTORF32& a_fontColor    = DirectX::Colors::White,
			const DirectX::XMVECTORF32& a_outlineColor = DirectX::Colors::Black);

		bool Load(int resource);
		bool Load(const wchar_t* filename);

		SKMP_FORCEINLINE bool IsLoaded() const { return m_isLoaded; }

		SKMP_FORCEINLINE void AddCallback(Callback cb)
		{
			m_callbacks.emplace_back(cb);
		}

		SKMP_FORCEINLINE bool RemoveCallback(Callback cb)
		{
			auto it = std::find(m_callbacks.begin(), m_callbacks.end(), cb);
			if (it != m_callbacks.end())
			{
				m_callbacks.erase(it);
				return true;
			}
			return false;
		}

		SKMP_FORCEINLINE auto GetNumCallbacks() const
		{
			return m_callbacks.size();
		}

		SKMP_FORCEINLINE void MulScale(float mul)
		{
			m_scale = _mm_mul_ps(m_scale, _mm_set_ps1(mul));
		}

		SKMP_FORCEINLINE void MulScale(const DirectX::XMFLOAT2A& mul)
		{
			m_scale = _mm_mul_ps(m_scale, DirectX::XMLoadFloat2A(std::addressof(mul)));
		}

		void DrawStrings();
		void Update();
		void UpdateStrings();
		void AdjustPosition();
		void AdjustOutline();

	private:
		std::unique_ptr<DirectX::CommonStates> m_commonStates;
		std::unique_ptr<DirectX::SpriteBatch>  m_spriteBatch;
		std::unique_ptr<DirectX::SpriteFont>   m_font;

		except::descriptor m_lastException;
		bool               m_isLoaded;

		std::vector<Callback> m_callbacks;
		std::wstring          m_drawString;

		DirectX::XMFLOAT2A   m_offset, m_bufferSize;
		DirectX::XMVECTORF32 m_fontColor, m_outlineColor;

		DirectX::XMVECTOR m_outlinePos[4], m_pos, m_origin, m_scale;
		DirectX::XMVECTOR m_outlineSize;

		Align m_alignment;

		ID3D11Device*        m_pDevice;
		ID3D11DeviceContext* m_pDeviceContext;

		ID3D11BlendState* m_blendState;

		D3D11StateBackupImpl m_backup;
	};

	class SKMP_ALIGN(16) DOSD :
		public IDriver,
		IConfig
	{
		enum Font
		{
			DroidSans = 1,
			FontMax
		};

		class KeyPressHandler : public ComboKeyPressHandler
		{
		public:
			virtual void OnKeyPressed() override;

		private:
			bool combo_down = false;
		};

	public:
		static inline constexpr auto ID = DRIVER_ID::OSD;

		void AddStatsCallback(StatsRenderer::Callback cb)
		{
			if (m_statsRenderer.get() != nullptr)
			{
				m_statsRenderer->AddCallback(cb);
			}
		}

		struct
		{
			bool                 enabled;
			bool                 initial;
			float                interval;
			Font                 font;
			std::string          font_file;
			std::string          font_color;
			std::string          font_outline_color;
			std::string          font_scale;
			bool                 font_autoscale;
			std::string          offset;
			std::string          items;
			float                outline_size;
			std::uint32_t        combo_key;
			std::uint32_t        key;
			StatsRenderer::Align align;
			bool                 scale_to_window;
		} m_conf;

		FN_NAMEPROC("OSD");
		FN_ESSENTIAL(false);
		FN_DRVDEF(3);

	private:
		using itemToFlag_t = stl::iunordered_map<std::string, std::uint32_t>;

		DOSD();

		virtual void LoadConfig() override;
		virtual void PostLoadConfig() override;
		virtual void RegisterHooks() override;
		virtual bool Prepare() override;

		static StatsRenderer::Align ConfigGetStatsRendererAlignment(std::int32_t param);
		static int                  ConfigGetFontResource(Font font);
		static void                 ConfigParseColors(const std::string& in, DirectX::XMVECTORF32& out);
		static void                 ConfigParseScale(const std::string& in, DirectX::XMFLOAT2A& out);
		static void                 ConfigParseVector2(const std::string& in, DirectX::XMFLOAT2A& out);
		static void                 ConfigGetFlags(const std::string& in, std::uint32_t& out);
		static std::uint32_t        ConfigGetComboKey(std::int32_t param);

		static const wchar_t* StatsRendererCallback_FPS();
		static const wchar_t* StatsRendererCallback_SimpleFPS();
		static const wchar_t* StatsRendererCallback_Frametime();
		static const wchar_t* StatsRendererCallback_SimpleFrametime();
		static const wchar_t* StatsRendererCallback_Counter();
		static const wchar_t* StatsRendererCallback_VRAM();

		void Initialize(D3D11CreateEventPost* a_data);

		static void OnD3D11PostCreate_OSD(Event code, void* data);
		static void OnD3D11PostPostCreate_OSD(Event code, void* data);

		static void Present_Pre(IDXGISwapChain* pSwapChain);
		static void Present_Post(IDXGISwapChain* pSwapChain);

		static HRESULT STDMETHODCALLTYPE StatsPresent_Hook(
			IDXGISwapChain* pSwapChain,
			UINT            SyncInterval,
			UINT            PresentFlags);

		/*typedef HRESULT(STDMETHODCALLTYPE* Present_T)(
            IDXGISwapChain* pSwapChain,
            UINT SyncInterval,
            UINT PresentFlags);

        Present_T Present_O;*/

		struct SKMP_ALIGN(16)
		{
			long long              lastUpdate;
			uint64_t               lastFrameCount, frameCounter;
			volatile bool          draw;
			volatile std::uint32_t warmup;
			std::uint32_t          flags;
			long long              interval;
			struct
			{
				DirectX::XMVECTORF32 font;
				DirectX::XMVECTORF32 outline;
			} colors;
			DirectX::XMFLOAT2A scale;
			DirectX::XMFLOAT2A offset;
			struct
			{
				long double frametime;
			} cur;
		} m_stats;

		KeyPressHandler m_inputEventHandler;

		wchar_t m_bufStats1[64];
		wchar_t m_bufStats2[64];
		wchar_t m_bufStats3[64];
		wchar_t m_bufStats4[64];

		std::unique_ptr<StatsRenderer> m_statsRenderer;

		DRender* m_dRender;

		Microsoft::WRL::ComPtr<IDXGIAdapter3> m_adapter;

		static itemToFlag_t m_itemToFlag;

		static DOSD m_Instance;
	};
}