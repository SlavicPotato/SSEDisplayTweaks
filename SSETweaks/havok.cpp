#include "pch.h"

#include "osd.h"

namespace SDT
{
	static constexpr const char* SECTION_GENERAL = "General";

	static constexpr const char* CKEY_HAVOKON = "DynamicMaxTimeScaling";
	static constexpr const char* CKEY_HAVOKENABLED = "Enabled";
	static constexpr const char* CKEY_MAXFPS = "MaximumFramerate";
	static constexpr const char* CKEY_MINFPS = "MinimumFramerate";
	static constexpr const char* CKEY_FMTCOFFSET = "MaxTimeComplexOffset";
	static constexpr const char* CKEY_STATSON = "OSDStatsEnabled";
	static constexpr const char* CKEY_PERFMODE = "PerformanceMode";
	static constexpr const char* CKEY_PHYSDAMAGE = "PhysicsDamagePatch";
	static constexpr const char* CKEY_PHYSDAMAGE_MULT = "PhysicsDamageMult";

	static constexpr const char* CKEY_ADJUSTINICFG = "AdjustGameSettings";

	DHavok DHavok::m_Instance;

	void DHavok::LoadConfig()
	{
		m_conf.havok_enabled = GetConfigValue(CKEY_HAVOKENABLED, false);
		m_conf.havok_dyn = GetConfigValue(CKEY_HAVOKON, true);
		m_conf.fmt_min = GetConfigValue(CKEY_MINFPS, 60.0f);
		m_conf.fmt_max = GetConfigValue(CKEY_MAXFPS, 0.0f);
		m_conf.fmtc_offset = std::clamp(GetConfigValue(CKEY_FMTCOFFSET, 30.0f), 0.0f, 30.0f);
		m_conf.stats_enabled = GetConfigValue(CKEY_STATSON, false);
		m_conf.perf_mode = GetConfigValue(CKEY_PERFMODE, false);
		m_conf.phys_damage_patch = GetConfigValue(CKEY_PHYSDAMAGE, true);
		m_conf.phys_damage_mult = std::clamp(GetConfigValue(CKEY_PHYSDAMAGE_MULT, 1.0f), 0.0f, 1.0f);

		m_conf.adjust_ini = IConfigS(SECTION_GENERAL).GetConfigValue(CKEY_ADJUSTINICFG, true);
	}

	void DHavok::PostLoadConfig()
	{
		if (m_conf.havok_enabled)
		{
			auto rd = IDDispatcher::GetDriver<DRender>();

			if (m_conf.havok_dyn && !rd || !rd->IsOK())
			{
				m_conf.havok_dyn = false;

				Error("Render driver unavailable, disabling dynamic scaling");
			}

			if (m_conf.fmt_max > 0.0f)
			{
				m_conf.fmt_max = std::max(m_conf.fmt_max, HAVOK_MAXTIME_MIN);
				fmt_min = 1.0f / m_conf.fmt_max;
			}

			m_conf.fmt_min = std::max(m_conf.fmt_min, HAVOK_MAXTIME_MIN);

			fmt_max = 1.0f / m_conf.fmt_min;

			if (m_conf.fmt_max > 0.0f)
			{
				if (m_conf.fmt_min == m_conf.fmt_max)
				{
					m_conf.havok_dyn = false;

					Message(
						"%s == %s, disabling dynamic scaling",
						CKEY_MINFPS,
						CKEY_MAXFPS);
				}
			}

			if (m_conf.perf_mode)
			{
				Message("Performance mode enabled");
			}
		}
	}

	void DHavok::RegisterHooks()
	{
		if (m_conf.havok_enabled)
		{
			IEvents::RegisterForEvent(Event::OnD3D11PreCreate, OnD3D11PreCreate_Havok);

			if (m_conf.havok_dyn)
			{
				m_OSDDriver = IDDispatcher::GetDriver<DOSD>();

				bool regOSDEvent;

				std::uintptr_t hf;
				if (m_OSDDriver &&
				    m_OSDDriver->IsOK() &&
				    m_OSDDriver->m_conf.enabled &&
				    m_conf.stats_enabled)
				{
					regOSDEvent = true;
					hf = reinterpret_cast<std::uintptr_t>(hookRTHStats);
				}
				else
				{
					regOSDEvent = false;
					hf = reinterpret_cast<std::uintptr_t>(hookRTH);
				}

				if (!Hook::Call5(
						ISKSE::GetBranchTrampoline(),
						PhysCalcMaxTime,
						hf,
						PhysCalcMaxTime_O))
				{
					m_conf.havok_dyn = false;
					Error("Couldn't hook physics calc function");
					return;
				}

				if (regOSDEvent)
				{
					IEvents::RegisterForEvent(Event::OnD3D11PostCreate, OnD3D11PostCreate_Havok);
				}
			}
		}
	}

	void DHavok::Patch()
	{
		if (m_conf.havok_enabled)
		{
			if (IAL::IsAE())
			{
				constexpr std::uint8_t payload[] = {
					0xEB,
					0x1F,
					0x90,
					0x90,
					0x90,
					0x90,
					0x90
				};

				Patching::safe_write(PhysCalc_AE_patch, payload);
			}

			if (m_conf.phys_damage_patch)
			{
				Patch_PhysicsDamage();
			}
		}
	}

	bool DHavok::Prepare()
	{
		m_gv.fMaxTime = ISKSE::GetINISettingAddr<float>("fMaxTime:HAVOK");
		if (!m_gv.fMaxTime)
		{
			return false;
		}

		m_gv.fMaxTimeComplex = ISKSE::GetINISettingAddr<float>("fMaxTimeComplex:HAVOK");
		if (!m_gv.fMaxTimeComplex)
		{
			return false;
		}

		m_gv.uMaxNumPhysicsStepsPerUpdate = ISKSE::GetINISettingAddr<std::uint32_t>("uMaxNumPhysicsStepsPerUpdate:HAVOK");
		if (!m_gv.uMaxNumPhysicsStepsPerUpdate)
		{
			return false;
		}

		m_gv.uMaxNumPhysicsStepsPerUpdateComplex = ISKSE::GetINISettingAddr<std::uint32_t>("uMaxNumPhysicsStepsPerUpdateComplex:HAVOK");
		if (!m_gv.uMaxNumPhysicsStepsPerUpdateComplex)
		{
			return false;
		}

		return true;
	}

	void DHavok::Patch_PhysicsDamage() const
	{
		struct Assembly : JITASM::JITASM
		{
			Assembly(const float* a_mult) :
				JITASM(ISKSE::GetLocalTrampoline())
			{
				Xbyak::Label timerLabel;
				Xbyak::Label magicLabel;
				Xbyak::Label multLabel;

				mulss(xmm0, xmm2);
				movss(xmm1, dword[rip + magicLabel]);
				mov(rcx, ptr[rip + timerLabel]);
				mulss(xmm1, dword[rcx]);
				mulss(xmm0, xmm1);
				mulss(xmm0, ptr[rip + multLabel]);
				ret();

				L(timerLabel);
				dq(std::uintptr_t(Game::g_frameTimerSlow));

				L(magicLabel);
				dd(0x42700000);  // 60.0f

				L(multLabel);
				db(reinterpret_cast<const Xbyak::uint8*>(a_mult), sizeof(float));
			}
		};

		LogPatchBegin(CKEY_PHYSDAMAGE);
		{
			Assembly code(std::addressof(m_conf.phys_damage_mult));
			ISKSE::GetBranchTrampoline().Write5Branch(PhysDamageCalc, code.get());
		}
		LogPatchEnd(CKEY_PHYSDAMAGE);
	}

	float DHavok::GetMaxTimeComplex(float a_interval)
	{
		return 1.0f / std::max(1.0f / a_interval - m_Instance.m_conf.fmtc_offset, HAVOK_MAXTIME_MIN);
	}

	void DHavok::CalculateHavokValues(bool a_isComplex) const
	{
		float interval = std::clamp(*Game::g_frameTimer, fmt_min, fmt_max);

		if (m_conf.perf_mode)
		{
			auto fmtc = GetMaxTimeComplex(interval);

			*m_gv.fMaxTime = fmtc;

			if (a_isComplex)
			{
				*m_gv.fMaxTimeComplex = fmtc;
			}
		}
		else
		{
			*m_gv.fMaxTime = interval;

			if (a_isComplex)
			{
				*m_gv.fMaxTimeComplex = GetMaxTimeComplex(interval);
			}
		}
	}

	void DHavok::UpdateHavokStats() const
	{
		m_Instance.m_stats_counters[0].accum(static_cast<double>(*m_gv.fMaxTime));
		m_Instance.m_stats_counters[1].accum(static_cast<double>(*m_gv.fMaxTimeComplex));
	}

	float DHavok::AutoGetMaxTime(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, float def) const
	{
		auto rd = IDDispatcher::GetDriver<DRender>();

		float maxt = rd->GetMaxFramerate(pSwapChainDesc);
		if (maxt > std::numeric_limits<float>::epsilon())
		{
			maxt = 1.0f / maxt;
		}
		else
		{
			maxt = 1.0f / def;
			Warning("Unable to calculate optimal fMaxTime, using %.6g", maxt);
		}

		return maxt;
	}

	bool DHavok::HavokHasPossibleIssues(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, float t) const
	{
		auto rd = IDDispatcher::GetDriver<DRender>();

		float maxfr = rd->GetMaxFramerate(pSwapChainDesc);
		return (maxfr > 0.0f && t > 1.0f / maxfr);
	}

	void DHavok::ApplyHavokSettings(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc)
	{
		float maxt;

		if (m_conf.havok_dyn)
		{
			if (m_conf.fmt_max <= 0.0f)
			{
				fmt_min = AutoGetMaxTime(pSwapChainDesc, 240.0f);
			}

			if (fmt_min > fmt_max)
			{
				fmt_max = fmt_min;
			}

			if (fmt_max == fmt_min)
			{
				Warning("Dynamic fMaxTime scaling is enabled but MinimumFramerate is equal to MaximumFramerate. Adjust your configuration.");
			}

			maxt = fmt_min;

			Message("(DYNAMIC) fMaxTime=%.6g-%.6g fMaxTimeComplexOffset=%.6g (Max FPS = %.6g)", fmt_min, fmt_max, m_conf.fmtc_offset, 1.0f / fmt_min);
		}
		else
		{
			if (m_conf.fmt_max > 0.0f)
			{
				maxt = fmt_min;
			}
			else
			{
				maxt = AutoGetMaxTime(pSwapChainDesc, 60.0f);
			}

			float maxtc = 1.0f / std::max(1.0f / maxt - m_conf.fmtc_offset, HAVOK_MAXTIME_MIN);

			if (m_conf.perf_mode)
			{
				*m_gv.fMaxTime = maxtc;
			}
			else
			{
				*m_gv.fMaxTime = maxt;
			}

			*m_gv.fMaxTimeComplex = maxtc;

			Message("(STATIC) fMaxTime=%.6g fMaxTimeComplex=%.6g (Max FPS = %.6g)", maxt, maxtc, 1.0f / maxt);
		}

		if (HavokHasPossibleIssues(pSwapChainDesc, maxt))
		{
			Warning("With the current configuration frame times could fall below fMaxTime. You may experience physics issues.");
		}

		if (m_conf.perf_mode)
		{
			*m_gv.uMaxNumPhysicsStepsPerUpdate = 1;
		}
		else
		{
			if (!*m_gv.uMaxNumPhysicsStepsPerUpdate)
			{
				*m_gv.uMaxNumPhysicsStepsPerUpdate = 3;
				Warning("uMaxNumPhysicsStepsPerUpdate is 0, adjusting to 3");
			}
			else if (*m_gv.uMaxNumPhysicsStepsPerUpdate != 3)
			{
				if (m_conf.adjust_ini)
				{
					m_Instance.Message("Setting uMaxNumPhysicsStepsPerUpdate=3");
					*m_gv.uMaxNumPhysicsStepsPerUpdate = 3;
				}
				else
				{
					Warning("uMaxNumPhysicsStepsPerUpdate != 3, recommend resetting to default");
				}
			}
		}

		if (!*m_gv.uMaxNumPhysicsStepsPerUpdateComplex)
		{
			*m_gv.uMaxNumPhysicsStepsPerUpdateComplex = 1;
			Warning("uMaxNumPhysicsStepsPerUpdateComplex is 0, adjusting to 1");
		}
		else if (*m_gv.uMaxNumPhysicsStepsPerUpdateComplex != 1)
		{
			if (m_conf.adjust_ini)
			{
				m_Instance.Message("Setting uMaxNumPhysicsStepsPerUpdateComplex=1");
				*m_gv.uMaxNumPhysicsStepsPerUpdateComplex = 1;
			}
			else
			{
				Warning("uMaxNumPhysicsStepsPerUpdateComplex != 1, recommend resetting to default");
			}
		}
	}

	void DHavok::hookRTH(float a_time, bool a_isComplex, std::uint8_t a_unk0)
	{
		m_Instance.CalculateHavokValues(a_isComplex);
		m_Instance.PhysCalcMaxTime_O(a_time, a_isComplex, a_unk0);
	}

	void DHavok::hookRTHStats(float a_time, bool a_isComplex, std::uint8_t a_unk0)
	{
		m_Instance.CalculateHavokValues(a_isComplex);
		m_Instance.UpdateHavokStats();
		m_Instance.PhysCalcMaxTime_O(a_time, a_isComplex, a_unk0);
	}

	const wchar_t* DHavok::StatsRendererCallback()
	{
		double val;

		if (!*m_Instance.isComplex)
		{
			if (m_Instance.m_stats_counters[0].get(val))
			{
				_snwprintf_s(m_Instance.bufStats, _TRUNCATE, L"fMaxTime: %.4g", val);
			}
		}
		else
		{
			if (m_Instance.m_stats_counters[1].get(val))
			{
				_snwprintf_s(m_Instance.bufStats, _TRUNCATE, L"fMaxTimeComplex: %.4g", val);
			}
		}

		return m_Instance.bufStats;
	}

	void DHavok::OnD3D11PreCreate_Havok(Event code, void* data)
	{
		auto info = reinterpret_cast<D3D11CreateEventPre*>(data);

		m_Instance.ApplyHavokSettings(info->m_pSwapChainDesc);
	}

	void DHavok::OnD3D11PostCreate_Havok(Event code, void* data)
	{
		m_Instance.m_OSDDriver->AddStatsCallback(StatsRendererCallback);
	}
}
