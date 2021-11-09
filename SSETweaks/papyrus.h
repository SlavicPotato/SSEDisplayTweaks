#pragma once

namespace SDT
{
	class DPapyrus :
		public IDriver,
		IConfig
	{
	public:
		static inline constexpr auto ID = DRIVER_ID::PAPYRUS;

		FN_NAMEPROC("Papyrus");
		FN_ESSENTIAL(false);
		FN_DRVDEF(6);

	private:
		DPapyrus();

		virtual void LoadConfig() override;
		virtual void PostLoadConfig() override;
		virtual void Patch() override;
		virtual bool Prepare() override;
		virtual void RegisterHooks() override;

		static float SKMP_FORCEINLINE CalculateUpdateBudget();
		static float CalculateUpdateBudgetStats();

		static const wchar_t* StatsRendererCallback1();
		static const wchar_t* StatsRendererCallback2();

		static void OnD3D11PostCreate_Papyrus(Event code, void* data);

		struct
		{
			bool seo_fix;
			bool dynbudget_enabled;
			float dynbudget_fps_min;
			float dynbudget_fps_max;
			float dynbudget_base;
			bool stats_enabled;
			bool warn_overstressed;
		} m_conf;

		inline static auto SetExpressionOverride_lea = IAL::Addr(AID::SetExpressionOverride, Offsets::SetExpressionOverride_lea);
		inline static auto SetExpressionOverride_cmp = IAL::Addr(AID::SetExpressionOverride, Offsets::SetExpressionOverride_cmp);
		inline static auto UpdateBudgetGame = IAL::Addr(AID::ScriptRunGame, Offsets::ScriptUpdateBudgetGame);
		inline static auto UpdateBudgetUI = IAL::Addr(AID::ScriptRunUI, Offsets::ScriptUpdateBudgetUI);

		float m_lastInterval;

		struct
		{
			float* fUpdateBudgetMS;
		} m_gv;

		float m_bmult;
		float m_t_max;
		float m_t_min;

		wchar_t m_bufStats1[64];

		DOSD* m_OSDDriver;

		StatsCounter m_stats_counter;

		static DPapyrus m_Instance;
	};
}