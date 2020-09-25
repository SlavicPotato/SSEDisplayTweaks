#pragma once

namespace SDT
{
    class DPapyrus :
        public IDriver,
        IConfig
    {
    public:
        FN_NAMEPROC("Papyrus")
        FN_ESSENTIAL(false)
        FN_PRIO(6)
        FN_DRVID(DRIVER_PAPYRUS)
    private:
        DPapyrus() = default;

        virtual void LoadConfig() override;
        virtual void PostLoadConfig() override;
        virtual void Patch() override;
        virtual bool Prepare() override;
        virtual void RegisterHooks() override;

        static float __forceinline CalculateUpdateBudget();
        static float CalculateUpdateBudgetStats();

        static const wchar_t* StatsRendererCallback();

        static void OnD3D11PostCreate_Papyrus(Event code, void* data);

        struct {
            bool seo_fix;
            bool dynbudget_enabled;
            float dynbudget_fps_min;
            float dynbudget_fps_max;
            float dynbudget_base;
            bool stats_enabled;
        }conf;

        inline static auto SetExpressionOverride_lea = IAL::Addr(AID::SetExpressionOverride, Offsets::SetExpressionOverride_lea);
        inline static auto SetExpressionOverride_cmp = IAL::Addr(AID::SetExpressionOverride, Offsets::SetExpressionOverride_cmp);
        inline static auto UpdateBudgetGame = IAL::Addr(AID::ScriptRunGame, Offsets::ScriptUpdateBudgetGame);
        inline static auto UpdateBudgetUI = IAL::Addr(AID::ScriptRunUI, Offsets::ScriptUpdateBudgetUI);

        float* fUpdateBudgetMS;

        float bmult;
        float t_max;
        float t_min;

        wchar_t bufStats1[128];

        DOSD* OSDDriver;

        bool enable_stats;

        static DPapyrus m_Instance;
    };
}