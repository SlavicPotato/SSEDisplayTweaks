#pragma once

namespace SDT
{

    class StatsRenderer
    {
    public:
        typedef const wchar_t* (*Callback)(void);

        enum Align {
            TOP_LEFT = 0,
            TOP_RIGHT,
            BOTTOM_LEFT,
            BOTTOM_RIGHT
        };

        StatsRenderer(
            ID3D11Device* pDevice,
            ID3D11DeviceContext* pDeviceContext,
            UINT bufferX, UINT bufferY,
            const DirectX::XMFLOAT2& offset = DirectX::XMFLOAT2(4.0f, 4.0f),
            float outlineSize = 1.0f,
            Align al = TOP_LEFT,
            const DirectX::XMFLOAT2& scale = DirectX::XMFLOAT2(1.0f, 1.0f),
            const DirectX::XMVECTORF32& fontColor = DirectX::Colors::White,
            const DirectX::XMVECTORF32& outlineColor = DirectX::Colors::Black
        );

        bool Load(int resource);
        bool Load(const wchar_t* filename);

        bool IsLoaded() { return isLoaded; }

        void AddCallback(Callback cb);
        size_t GetNumCallbacks();
        void MulScale(float mul);
        void MulScale(const DirectX::XMFLOAT2& mul);

        void DrawStrings() const;
        void Update();
        void UpdateStrings();
        void AdjustPosition();
        void AdjustOutline();

    private:
        std::unique_ptr<DirectX::CommonStates> m_commonStates;
        std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
        std::unique_ptr<DirectX::SpriteFont> m_font;

        except::descriptor e_last;

        stl::vector<Callback> callbacks;
        std::wostringstream ss;
        std::wstring s;

        DirectX::XMFLOAT2 pos, opos[4], origin, scale, off;
        DirectX::XMVECTORF32 fcol, ocol;

        float bx, by, ol;
        Align align;

        bool isLoaded;

        ID3D11Device* m_pDevice;
        ID3D11BlendState* m_blendState;
    };

    class DOSD :
        public IDriver,
        IConfig
    {
        enum Font
        {
            DroidSans = 1,
            FontMax
        };

        class KeyPressHandler : public KeyEventHandler
        {
        public:
            virtual void ReceiveEvent(KeyEvent, UInt32) override;
        private:
            bool combo_down = false;
        };
    public:
        static inline constexpr auto ID = DRIVER_ID::OSD;

        void AddStatsCallback(StatsRenderer::Callback cb)
        {
            if (statsRenderer.get() != nullptr) {
                statsRenderer->AddCallback(cb);
            }
        }

        struct {
            bool enabled;
            bool initial;
            float interval;
            Font font;
            std::string font_file;
            std::string font_color;
            std::string font_outline_color;
            std::string font_scale;
            bool font_autoscale;
            std::string offset;
            std::string items;
            float outline_size;
            uint32_t combo_key;
            uint32_t key;
            StatsRenderer::Align align;
        } m_conf;

        FN_NAMEPROC("OSD")
        FN_ESSENTIAL(false)
        FN_DRVDEF(3)
    private:
        DOSD();

        virtual void LoadConfig() override;
        virtual void PostLoadConfig() override;
        virtual void RegisterHooks() override;
        virtual bool Prepare() override;

        static StatsRenderer::Align ConfigGetStatsRendererAlignment(int32_t param);
        static int ConfigGetFontResource(Font font);
        static void ConfigParseColors(const std::string& in, DirectX::XMVECTORF32& out);
        static void ConfigParseScale(const std::string& in, DirectX::XMFLOAT2& out);
        static void ConfigParseVector2(const std::string& in, DirectX::XMFLOAT2& out);
        static void ConfigGetFlags(const std::string& in, uint32_t& out);
        static uint32_t ConfigGetComboKey(int32_t param);

        typedef stl::iunordered_map<std::string, uint32_t> ItemToFlag_T;
        static ItemToFlag_T ItemToFlag;

        static const wchar_t* StatsRendererCallback_FPS();
        static const wchar_t* StatsRendererCallback_SimpleFPS();
        static const wchar_t* StatsRendererCallback_Frametime();
        static const wchar_t* StatsRendererCallback_SimpleFrametime();
        static const wchar_t* StatsRendererCallback_Counter();
        static const wchar_t* StatsRendererCallback_VRAM();

        static void OnD3D11PostCreate_OSD(Event code, void* data);
        static void OnD3D11PostPostCreate_OSD(Event code, void* data);

        static HRESULT STDMETHODCALLTYPE StatsPresent_Hook(
            IDXGISwapChain* pSwapChain,
            UINT SyncInterval,
            UINT PresentFlags);

        /*typedef HRESULT(STDMETHODCALLTYPE* Present_T)(
            IDXGISwapChain* pSwapChain,
            UINT SyncInterval,
            UINT PresentFlags);

        Present_T Present_O;*/

        struct
        {
            long long lastUpdate;
            uint64_t lastFrameCount, frameCounter;
            volatile bool draw;
            volatile uint32_t warmup;
            uint32_t flags;
            long long interval;
            struct {
                DirectX::XMVECTORF32 font;
                DirectX::XMVECTORF32 outline;
            }colors;
            DirectX::XMFLOAT2 scale;
            DirectX::XMFLOAT2 offset;
            struct {
                double fps;
                double frametime;
            } cur;
        } stats;

        KeyPressHandler inputEventHandler;

        wchar_t bufStats1[64];
        wchar_t bufStats2[64];
        wchar_t bufStats3[64];
        wchar_t bufStats4[64];

        std::unique_ptr<StatsRenderer> statsRenderer;

        DRender* m_dRender;

        Microsoft::WRL::ComPtr<IDXGIAdapter3> m_adapter;

        inline static auto presentAddr = IAL::Addr(AID::Present, Offsets::Present);

        static DOSD m_Instance;
    };
}