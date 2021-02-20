#include "pch.h"

namespace SDT
{
    static constexpr const char* CKEY_SEOFIX = "SetExpressionOverridePatch";
    static constexpr const char* CKEY_DYNBUDGETENABLED = "DynamicUpdateBudget";
    static constexpr const char* CKEY_UPDATEBUDGET = "UpdateBudgetBase";
    static constexpr const char* CKEY_MAXTIME = "BudgetMaxFPS";
    static constexpr const char* CKEY_STATSON = "OSDStatsEnabled";
    static constexpr const char* CKEY_SHOWVMOVERSTRESSES = "OSDWarnVMOverstressed";

    using namespace Patching;

    DPapyrus DPapyrus::m_Instance;

    DPapyrus::DPapyrus() :
        m_lastInterval(1.0f / 60.0f),
        fUpdateBudgetMS(nullptr)
    {
        bufStats1[0] = 0x0;
        bufStats2[0] = 0x0;
    }

    void DPapyrus::LoadConfig()
    {
        m_conf.seo_fix = GetConfigValue(CKEY_SEOFIX, false);
        m_conf.dynbudget_enabled = GetConfigValue(CKEY_DYNBUDGETENABLED, false);
        m_conf.dynbudget_fps_min = 60.0f;
        m_conf.dynbudget_fps_max = std::clamp(GetConfigValue(CKEY_MAXTIME, 144.0f), m_conf.dynbudget_fps_min, 300.0f);
        m_conf.dynbudget_base = std::clamp(GetConfigValue(CKEY_UPDATEBUDGET, 1.2f), 0.1f, 4.0f);
        m_conf.stats_enabled = GetConfigValue(CKEY_STATSON, false);
        m_conf.warn_overstressed = GetConfigValue(CKEY_SHOWVMOVERSTRESSES, true);
    }

    void DPapyrus::PostLoadConfig()
    {
        if (!fUpdateBudgetMS) {
            m_conf.dynbudget_enabled = false;
        }

        m_OSDDriver = IDDispatcher::GetDriver<DOSD>();
        if (!m_OSDDriver || !m_OSDDriver->IsOK())
        {
            m_conf.stats_enabled = false;
            m_conf.warn_overstressed = false;
        }

        if (m_conf.dynbudget_enabled)
        {
            if (m_conf.dynbudget_fps_max == m_conf.dynbudget_fps_min) {
                Warning("dynbudget_fps_max == dynbudget_fps_min, disabling..");
                m_conf.dynbudget_enabled = false;
            }
            else
            {
                bmult = m_conf.dynbudget_base / (1.0f / 60.0f * 1000.0f) * 1000.0f;
                t_max = 1.0f / m_conf.dynbudget_fps_min;
                t_min = 1.0f / m_conf.dynbudget_fps_max;

                Message("UpdateBudgetBase: %.6g ms (%.6g - %.6g)",
                    m_conf.dynbudget_base, t_min * bmult, t_max * bmult);

            }
        }
    }

    void DPapyrus::Patch()
    {
        if (m_conf.seo_fix)
        {
            safe_write(
                SetExpressionOverride_lea,
                reinterpret_cast<const void*>(Payloads::seoFix_lea),
                sizeof(Payloads::seoFix_lea)
            );
            safe_write(
                SetExpressionOverride_cmp,
                Payloads::seoFix_cmp
            );

            Message("Actor.SetExpressionOverride patch applied");
        }

        if (m_conf.dynbudget_enabled)
        {
            struct UpdateBudgetInject : JITASM::JITASM {
                UpdateBudgetInject(uintptr_t targetAddr, bool enable_stats)
                    : JITASM()
                {
                    Xbyak::Label callLabel;
                    Xbyak::Label retnLabel;

                    call(ptr[rip + callLabel]);
                    movss(xmm6, xmm0);
                    jmp(ptr[rip + retnLabel]);

                    L(retnLabel);
                    dq(targetAddr + 0x8);

                    L(callLabel);
                    if (enable_stats) {
                        dq(uintptr_t(DPapyrus::CalculateUpdateBudgetStats));
                    }
                    else {
                        dq(uintptr_t(DPapyrus::CalculateUpdateBudget));
                    }
                }
            };

            LogPatchBegin("UpdateBudget (game)");
            {
                UpdateBudgetInject code(UpdateBudgetGame, m_conf.stats_enabled);
                g_branchTrampoline.Write6Branch(UpdateBudgetGame, code.get());

                safe_memset(UpdateBudgetGame + 0x6, 0xCC, 2);
            }
            LogPatchEnd("UpdateBudget (game)");

            LogPatchBegin("UpdateBudget (UI)");
            {
                UpdateBudgetInject code(UpdateBudgetUI, m_conf.stats_enabled);
                g_branchTrampoline.Write6Branch(UpdateBudgetUI, code.get());

                safe_memset(UpdateBudgetUI + 0x6, 0xCC, 2);
            }
            LogPatchEnd("UpdateBudget (UI)");
        }
    }

    void DPapyrus::RegisterHooks()
    {
        if ((m_conf.dynbudget_enabled && m_conf.stats_enabled) || m_conf.warn_overstressed) {
            IEvents::RegisterForEvent(Event::OnD3D11PostCreate, OnD3D11PostCreate_Papyrus);
        }
    }

    bool DPapyrus::Prepare()
    {
        fUpdateBudgetMS = ISKSE::GetINISettingAddr<float>("fUpdateBudgetMS:Papyrus");
        
        return true;
    }

    float DPapyrus::CalculateUpdateBudget()
    {
        float interval = std::clamp(*Game::frameTimer, m_Instance.t_min, m_Instance.t_max);

        if (interval <= m_Instance.m_lastInterval) {
            m_Instance.m_lastInterval = interval;
        }
        else {
            m_Instance.m_lastInterval = std::min(m_Instance.m_lastInterval + interval * 0.0075f, interval);
        }

        interval = m_Instance.m_lastInterval * m_Instance.bmult;

        *m_Instance.fUpdateBudgetMS = interval;

        return interval;
    }

    float DPapyrus::CalculateUpdateBudgetStats()
    {
        float cft = CalculateUpdateBudget();

        m_Instance.m_stats_counter.accum(cft);

        return cft;
    }

    const wchar_t* DPapyrus::StatsRendererCallback1()
    {
        double val;
        if (m_Instance.m_stats_counter.get(val)) {
            ::_snwprintf_s(m_Instance.bufStats1,
                _TRUNCATE, L"fUpdateBudgetMS: %.4g", val);
        }

        return m_Instance.bufStats1;
    }

    const wchar_t* DPapyrus::StatsRendererCallback2()
    {
        auto vm = (*g_skyrimVM)->GetClassRegistry();

        if (vm->overstressed) {
            _snwprintf_s(m_Instance.bufStats2,
                _TRUNCATE, L"VM Overstressed");
        }
        else {
            m_Instance.bufStats2[0] = 0x0;
        }
         
        return m_Instance.bufStats2;
    }

    void DPapyrus::OnD3D11PostCreate_Papyrus(Event, void*)
    {
        if (m_Instance.m_conf.stats_enabled)
            m_Instance.m_OSDDriver->AddStatsCallback(StatsRendererCallback1);

        if (m_Instance.m_conf.warn_overstressed)
            m_Instance.m_OSDDriver->AddStatsCallback(StatsRendererCallback2);
    }

}