#include "pch.h"

namespace SDT
{
    static constexpr const char* CKEY_SEOFIX = "SetExpressionOverridePatch";
    static constexpr const char* CKEY_DYNBUDGETENABLED = "DynamicUpdateBudget";
    static constexpr const char* CKEY_UPDATEBUDGET = "UpdateBudgetBase";
    static constexpr const char* CKEY_MAXTIME = "BudgetMaxFPS";
    static constexpr const char* CKEY_STATSON = "OSDStatsEnabled";

    using namespace Patching;

    DPapyrus DPapyrus::m_Instance;

    void DPapyrus::LoadConfig()
    {
        m_conf.seo_fix = GetConfigValue(CKEY_SEOFIX, false);
        m_conf.dynbudget_enabled = GetConfigValue(CKEY_DYNBUDGETENABLED, false);
        m_conf.dynbudget_fps_min = 60.0f;
        m_conf.dynbudget_fps_max = std::clamp(GetConfigValue(CKEY_MAXTIME, 144.0f), m_conf.dynbudget_fps_min, 300.0f);
        m_conf.dynbudget_base = std::clamp(GetConfigValue(CKEY_UPDATEBUDGET, 1.2f), 0.1f, 4.0f);
        m_conf.stats_enabled = GetConfigValue(CKEY_STATSON, false);
    }

    void DPapyrus::PostLoadConfig()
    {
        if (m_conf.dynbudget_enabled)
        {
            if (m_conf.dynbudget_fps_max == m_conf.dynbudget_fps_min) {
                Warning("dynbudget_fps_max == dynbudget_fps_min, disabling..");
                m_conf.dynbudget_enabled = false;
            }
            else
            {
                m_OSDDriver = IDDispatcher::GetDriver<DOSD>();

                enable_stats = m_OSDDriver && m_OSDDriver->IsOK() &&
                    m_OSDDriver->m_conf.enabled && m_conf.stats_enabled;

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
                UpdateBudgetInject code(UpdateBudgetGame, enable_stats);
                g_branchTrampoline.Write6Branch(UpdateBudgetGame, code.get());

                safe_memset(UpdateBudgetGame + 0x6, 0xCC, 2);
            }
            LogPatchEnd("UpdateBudget (game)");

            LogPatchBegin("UpdateBudget (UI)");
            {
                UpdateBudgetInject code(UpdateBudgetUI, enable_stats);
                g_branchTrampoline.Write6Branch(UpdateBudgetUI, code.get());

                safe_memset(UpdateBudgetUI + 0x6, 0xCC, 2);
            }
            LogPatchEnd("UpdateBudget (UI)");
        }
    }

    void DPapyrus::RegisterHooks()
    {
        if (m_conf.dynbudget_enabled && enable_stats) {
            IEvents::RegisterForEvent(Event::OnD3D11PostCreate, OnD3D11PostCreate_Papyrus);
        }
    }

    bool DPapyrus::Prepare()
    {
        fUpdateBudgetMS = ISKSE::GetINISettingAddr<float>("fUpdateBudgetMS:Papyrus");
        if (!fUpdateBudgetMS) {
            return false;
        }

        return true;
    }

    float DPapyrus::CalculateUpdateBudget()
    {
        float interval = std::clamp(*Game::frameTimer, m_Instance.t_min, m_Instance.t_max);

        interval *= m_Instance.bmult;
        *m_Instance.fUpdateBudgetMS = interval;

        return interval;
    }

    float DPapyrus::CalculateUpdateBudgetStats()
    {
        float cft = CalculateUpdateBudget();

        m_Instance.m_stats_counter.accum(cft);

        return cft;
    }

    const wchar_t* DPapyrus::StatsRendererCallback()
    {
        double val;
        if (m_Instance.m_stats_counter.get(val)) {
            ::_snwprintf_s(m_Instance.bufStats1,
                _TRUNCATE, L"fUpdateBudgetMS: %.4g", val);
        }

        return m_Instance.bufStats1;
    }

    void DPapyrus::OnD3D11PostCreate_Papyrus(Event, void*)
    {
        m_Instance.bufStats1[0] = 0x0;
        m_Instance.m_OSDDriver->AddStatsCallback(StatsRendererCallback);
    }

}