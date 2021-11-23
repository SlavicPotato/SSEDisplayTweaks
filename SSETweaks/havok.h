#pragma once

#include "stats.h"

namespace SDT
{
	constexpr float HAVOK_MAXTIME_MIN = 30.0f;

	class DOSD;

	class DHavok :
		public IDriver,
		IConfig
	{
		typedef void (*RTProcR)(float a_time, bool a_isComplex, std::uint8_t a_unk0);

	public:
		static inline constexpr auto ID = DRIVER_ID::HAVOK;

		FN_NAMEPROC("HAVOK");
		FN_ESSENTIAL(false);
		FN_DRVDEF(4);

	private:
		DHavok();

		virtual void LoadConfig() override;
		virtual void PostLoadConfig() override;
		virtual void RegisterHooks() override;
		virtual void Patch() override;
		virtual bool Prepare() override;

		bool HavokHasPossibleIssues(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, float t) const;
		void ApplyHavokSettings(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc);
		float AutoGetMaxTime(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, float def) const;

		SKMP_FORCEINLINE static float GetMaxTimeComplex(float a_interval);
		SKMP_FORCEINLINE void CalculateHavokValues(bool a_isComplex) const;
		SKMP_FORCEINLINE void UpdateHavokStats() const;

		static void hookRTH(float a_time, bool a_isComplex, std::uint8_t a_unk0);
		static void hookRTHStats(float a_time, bool a_isComplex, std::uint8_t a_unk0);

		static const wchar_t* StatsRendererCallback();

		static void OnD3D11PreCreate_Havok(Event code, void* data);
		static void OnD3D11PostCreate_Havok(Event code, void* data);

		struct
		{
			bool havok_enabled;
			bool havok_on;
			float fmt_max;
			float fmt_min;
			float fmtc_offset;
			bool stats_enabled;
			bool perf_mode;
			bool adjust_ini;
		} m_conf;

		float fmt_max;
		float fmt_min;

		struct
		{
			float* fMaxTime;
			float* fMaxTimeComplex;
			std::uint32_t* uMaxNumPhysicsStepsPerUpdate;
			std::uint32_t* uMaxNumPhysicsStepsPerUpdateComplex;
		} m_gv;

		RTProcR PhysCalcMaxTime_O;

		inline static auto PhysCalcMaxTime = IAL::Addr(AID::FMTProc, 36577, Offsets::PhysCalcHT, 0xA6);
		inline static auto PhysCalc_AE_patch = IAL::Addr<std::uintptr_t>(0, 77850, 0, 0x75);
		inline static auto isComplex = IAL::Addr<std::uint32_t*>(AID::IsComplex, 403438);

		DOSD* m_OSDDriver;

		wchar_t bufStats[128];

		StatsCounter m_stats_counters[2];

		static DHavok m_Instance;
	};

}