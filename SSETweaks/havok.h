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
		virtual void LoadConfig() override;
		virtual void PostLoadConfig() override;
		virtual void RegisterHooks() override;
		virtual void Patch() override;
		virtual bool Prepare() override;
		virtual void OnGameConfigLoaded() override;

		void Patch_PhysicsDamage() const;

		bool  HavokHasPossibleIssues(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, float t) const;
		void  ApplyHavokSettings(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc);
		float AutoGetMaxTime(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, float def) const;

		static float GetMaxTimeComplex(float a_interval);
		void         CalculateHavokValues(bool a_isComplex) const;
		void         UpdateHavokStats() const;

		static void hookRTH(float a_time, bool a_isComplex, std::uint8_t a_unk0);
		static void hookRTHStats(float a_time, bool a_isComplex, std::uint8_t a_unk0);

		static const wchar_t* StatsRendererCallback();

		static void OnD3D11PreCreate_Havok(Event code, void* data);
		static void OnD3D11PostCreate_Havok(Event code, void* data);

		struct
		{
			bool  havok_enabled;
			bool  havok_dyn;
			float fmt_max;
			float fmt_min;
			float fmtc_offset;
			bool  stats_enabled;
			bool  perf_mode;
			bool  adjust_ini;
			bool  phys_damage_patch;
			float phys_damage_mult;
		} m_conf;

		float fmt_max{ 0.0f };
		float fmt_min{ 0.0f };

		struct
		{
			float*         fMaxTime{ nullptr };
			float*         fMaxTimeComplex{ nullptr };
			std::uint32_t* uMaxNumPhysicsStepsPerUpdate{ nullptr };
			std::uint32_t* uMaxNumPhysicsStepsPerUpdateComplex{ nullptr };
		} m_gv;

		RTProcR PhysCalcMaxTime_O;

		inline static auto PhysCalcMaxTime   = IAL::Addr(AID::FMTProc, 36577, Offsets::PhysCalcHT, 0xA6);
		inline static auto PhysCalc_AE_patch = IAL::Addr<std::uintptr_t>(0, 77850, 0, 0x75);
		inline static auto isComplex         = IAL::Addr<std::uint32_t*>(AID::IsComplex, 403438);
		inline static auto PhysDamageCalc    = IAL::Addr<std::uintptr_t>(25478, 26018, 0x5F, 0x5F);

		DOSD* m_OSDDriver{ nullptr };

		wchar_t bufStats[128]{ 0 };

		StatsCounter m_stats_counters[2];

		static DHavok m_Instance;
	};

}