#pragma once

namespace SDT
{
    constexpr float HAVOK_MAXTIME_MIN = 30.0f;

    class DHavok :
        public IDriver,
        IConfig
    {
        typedef void(*RTProcR) (void);
    public:
        FN_NAMEPROC("HAVOK")
        FN_ESSENTIAL(false)
        FN_PRIO(4)
        FN_DRVID(DRIVER_HAVOK)
    private:
        DHavok();

        virtual void LoadConfig() override;
        virtual void PostLoadConfig() override;
        virtual void RegisterHooks() override;
        virtual bool Prepare() override;

        bool HavokHasPossibleIssues(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, float t);
        void ApplyHavokSettings(DXGI_SWAP_CHAIN_DESC* pSwapChainDesc);
        float AutoGetMaxTime(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, float def);

        void __forceinline CalculateHavokValues();
        void __forceinline UpdateHavokStats();

        static void hookRTH();
        static void hookRTHStats();

        static const wchar_t* StatsRendererCallback1();
        static const wchar_t* StatsRendererCallback2();

        static void OnD3D11PreCreate_Havok(Event code, void* data);
        static void OnD3D11PostCreate_Havok(Event code, void* data);

        struct {
            bool havok_enabled;
            bool havok_on;
            float fmt_max;
            float fmt_min;
            float fmtc_offset;
            bool stats_enabled;
            bool adjust_ini;
        } conf;

        long long fts;

        float fmt_max;
        float fmt_min;

        float* fMaxTime;
        float* fMaxTimeComplex;
        uint32_t* uMaxNumPhysicsStepsPerUpdate;
        uint32_t* uMaxNumPhysicsStepsPerUpdateComplex;

        inline static auto RTUnk0_GM_C = IAL::Addr(AID::RT0, Offsets::RTUnk0_GM_C);
        inline static auto RTUnk0_UI_C = IAL::Addr(AID::RT0, Offsets::RTUnk0_UI_C);
        inline static auto PhysFuncUnk0_O = IAL::Addr<RTProcR>(AID::FMTProc);
        inline static auto isComplex = IAL::Addr<uint32_t*>(AID::IsComplex);

        DOSD* OSDDriver;

        wchar_t bufStats1[128];
        wchar_t bufStats2[128];

        static DHavok m_Instance;
    };

}