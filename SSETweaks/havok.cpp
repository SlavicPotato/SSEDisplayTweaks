#include "pch.h"

namespace SDT
{
    static constexpr const char* SECTION_GENERAL = "General";

    static constexpr const char* CKEY_HAVOKON = "DynamicMaxTimeScaling";
    static constexpr const char* CKEY_HAVOKENABLED = "Enabled";
    static constexpr const char* CKEY_MAXFPS = "MaximumFramerate";
    static constexpr const char* CKEY_MINFPS = "MinimumFramerate";
    static constexpr const char* CKEY_FMTCOFFSET = "MaxTimeComplexOffset";
    static constexpr const char* CKEY_STATSON = "OSDStatsEnabled";

    static constexpr const char* CKEY_ADJUSTINICFG = "AdjustGameSettings";

    DHavok DHavok::m_Instance;

    DHavok::DHavok()
    {
        m_Instance.bufStats[0] = 0x0;
    }

    void DHavok::LoadConfig()
    {
        m_conf.havok_enabled = GetConfigValue(CKEY_HAVOKENABLED, false);
        m_conf.havok_on = GetConfigValue(CKEY_HAVOKON, true);
        m_conf.fmt_min = GetConfigValue(CKEY_MINFPS, 60.0f);
        m_conf.fmt_max = GetConfigValue(CKEY_MAXFPS, 0.0f);
        m_conf.fmtc_offset = std::clamp(GetConfigValue(CKEY_FMTCOFFSET, 30.0f), 0.0f, 30.0f);
        m_conf.stats_enabled = GetConfigValue(CKEY_STATSON, false);

        m_conf.adjust_ini = IConfigS(SECTION_GENERAL).GetConfigValue(CKEY_ADJUSTINICFG, true);
    }

    void DHavok::PostLoadConfig()
    {
        if (m_conf.havok_enabled)
        {
            auto rd = IDDispatcher::GetDriver<DRender>();

            if (m_conf.havok_on && !rd || !rd->IsOK()) {
                m_conf.havok_on = false;
                Error("Render driver unavailable, disabling dynamic scaling");
            }

            if (m_conf.fmt_max > 0.0f) {
                if (m_conf.fmt_max < HAVOK_MAXTIME_MIN) {
                    m_conf.fmt_max = HAVOK_MAXTIME_MIN;
                }

                fmt_min = 1.0f / m_conf.fmt_max;
            }

            if (m_conf.fmt_min < HAVOK_MAXTIME_MIN) {
                m_conf.fmt_min = HAVOK_MAXTIME_MIN;
            }

            fmt_max = 1.0f / m_conf.fmt_min;

            if (m_conf.fmt_max > 0.0f)
            {
                if (m_conf.fmt_min == m_conf.fmt_max) {
                    m_conf.havok_on = false;
                }
            }
        }
    }

    void DHavok::RegisterHooks()
    {
        if (m_conf.havok_enabled) {
            IEvents::RegisterForEvent(Event::OnD3D11PreCreate, OnD3D11PreCreate_Havok);

            if (m_conf.havok_on) {

                m_OSDDriver = IDDispatcher::GetDriver<DOSD>();

                bool regOSDEvent;

                uintptr_t hf;
                if (m_OSDDriver && m_OSDDriver->IsOK() && m_OSDDriver->m_conf.enabled && m_conf.stats_enabled)
                {
                    regOSDEvent = true;
                    hf = reinterpret_cast<uintptr_t>(hookRTHStats);
                }
                else {
                    regOSDEvent = false;
                    hf = reinterpret_cast<uintptr_t>(hookRTH);
                }

                if (!Hook::Call5(PhysCalcMaxTime, hf, PhysCalcMaxTime_O))
                {
                    m_conf.havok_on = false;
                    Error("Couldn't hook physics calc function");
                    return;
                }

                if (regOSDEvent) {
                    IEvents::RegisterForEvent(Event::OnD3D11PostCreate, OnD3D11PostCreate_Havok);
                }
            }
        }
    }

    bool DHavok::Prepare()
    {
        fMaxTime = ISKSE::GetINISettingAddr<float>("fMaxTime:HAVOK");
        if (!fMaxTime) {
            return false;
        }

        fMaxTimeComplex = ISKSE::GetINISettingAddr<float>("fMaxTimeComplex:HAVOK");
        if (!fMaxTimeComplex) {
            return false;
        }

        uMaxNumPhysicsStepsPerUpdate = ISKSE::GetINISettingAddr<uint32_t>("uMaxNumPhysicsStepsPerUpdate:HAVOK");
        if (!uMaxNumPhysicsStepsPerUpdate) {
            return false;
        }

        uMaxNumPhysicsStepsPerUpdateComplex = ISKSE::GetINISettingAddr<uint32_t>("uMaxNumPhysicsStepsPerUpdateComplex:HAVOK");
        if (!uMaxNumPhysicsStepsPerUpdateComplex) {
            return false;
        }

        return true;
    }

    void DHavok::CalculateHavokValues(bool a_isComplex) const
    {
        float interval = std::clamp(*Game::frameTimer, fmt_min, fmt_max);

        *fMaxTime = interval;

        if (a_isComplex)
            *fMaxTimeComplex = 1.0f / max(1.0f / interval - m_conf.fmtc_offset, HAVOK_MAXTIME_MIN);
    }

    void DHavok::UpdateHavokStats() const
    {
        m_Instance.m_stats_counters[0].accum(static_cast<double>(*fMaxTime));
        m_Instance.m_stats_counters[1].accum(static_cast<double>(*fMaxTimeComplex));
    }

    float DHavok::AutoGetMaxTime(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, float def) const
    {
        auto rd = IDDispatcher::GetDriver<DRender>();

        float maxt = rd->GetMaxFramerate(pSwapChainDesc);
        if (maxt > 0.0f) {
            maxt = 1.0f / maxt;
        }
        else {
            maxt = 1.0f / def;
            Warning("Unable to calculate optimal fMaxTime, using %.6g", maxt);
        }

        return maxt;
    }

    bool DHavok::HavokHasPossibleIssues(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, float t) const
    {
        auto rd = IDDispatcher::GetDriver<DRender>();

        float maxfr = rd->GetMaxFramerate(pSwapChainDesc);
        if (maxfr > 0.0f && t > 1.0f / maxfr) {
            return true;
        }
        return false;
    }

    void DHavok::ApplyHavokSettings(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc)
    {
        float maxt;

        if (m_conf.havok_on) {
            if (m_conf.fmt_max <= 0.0f) {
                fmt_min = AutoGetMaxTime(pSwapChainDesc, 240.0f);
            }

            if (fmt_min > fmt_max) {
                fmt_max = fmt_min;
            }

            if (fmt_max == fmt_min) {
                Warning("Dynamic fMaxTime scaling is enabled but MinimumFramerate is equal to MaximumFramerate. Adjust your configuration.");
            }

            maxt = fmt_min;

            Message("(DYNAMIC) fMaxTime=%.6g-%.6g fMaxTimeComplexOffset=%.6g (Max FPS = %.6g)",
                fmt_min, fmt_max, m_conf.fmtc_offset, 1.0f / fmt_min);
        }
        else {
            if (m_conf.fmt_max > 0.0f) {
                maxt = fmt_min;
            }
            else {
                maxt = AutoGetMaxTime(pSwapChainDesc, 60.0f);
            }

            float maxtc = 1.0f / max(1.0f / maxt - m_conf.fmtc_offset, HAVOK_MAXTIME_MIN);

            *fMaxTime = maxt;
            *fMaxTimeComplex = maxtc;

            Message("(STATIC) fMaxTime=%.6g fMaxTimeComplex=%.6g (Max FPS = %.6g)", maxt, maxtc, 1.0f / maxt);
        }

        if (HavokHasPossibleIssues(pSwapChainDesc, maxt)) {
            Warning("With the current configuration frame times could fall below fMaxTime. You may experience physics issues.");
        }

        if (!*uMaxNumPhysicsStepsPerUpdate) {
            *uMaxNumPhysicsStepsPerUpdate = 3;
            Warning("uMaxNumPhysicsStepsPerUpdate is 0, adjusting to 3");
        }
        else if (*uMaxNumPhysicsStepsPerUpdate != 3) {
            if (m_conf.adjust_ini) {
                m_Instance.Message("Setting uMaxNumPhysicsStepsPerUpdate=3");
                *uMaxNumPhysicsStepsPerUpdate = 3;
            }
            else {
                Warning("uMaxNumPhysicsStepsPerUpdate != 3, recommend resetting to default");
            }
        }

        if (!*uMaxNumPhysicsStepsPerUpdateComplex) {
            *uMaxNumPhysicsStepsPerUpdateComplex = 1;
            Warning("uMaxNumPhysicsStepsPerUpdateComplex is 0, adjusting to 1");
        }
        else if (*uMaxNumPhysicsStepsPerUpdateComplex != 1) {
            if (m_conf.adjust_ini) {
                m_Instance.Message("Setting uMaxNumPhysicsStepsPerUpdateComplex=1");
                *uMaxNumPhysicsStepsPerUpdateComplex = 1;
            }
            else {
                Warning("uMaxNumPhysicsStepsPerUpdateComplex != 1, recommend resetting to default");
            }
        }
    }

    void DHavok::hookRTH(float a_time, bool a_isComplex, uint8_t a_unk0)
    {
        m_Instance.CalculateHavokValues(a_isComplex);
        m_Instance.PhysCalcMaxTime_O(a_time, a_isComplex, a_unk0);
    }

    void DHavok::hookRTHStats(float a_time, bool a_isComplex, uint8_t a_unk0)
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
            if (m_Instance.m_stats_counters[0].get(val)) {
                _snwprintf_s(m_Instance.bufStats, _TRUNCATE,
                    L"fMaxTime: %.4g", val);
            }
        }
        else
        {
            if (m_Instance.m_stats_counters[1].get(val)) {
                _snwprintf_s(m_Instance.bufStats, _TRUNCATE,
                    L"fMaxTimeComplex: %.4g", val);
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
