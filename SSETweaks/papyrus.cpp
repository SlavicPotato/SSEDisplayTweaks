#include "pch.h"

namespace SDT
{
    constexpr char* SECTION_PAPYRUS = "Papyrus";

    constexpr char* CKEY_SEOFIX = "SetExpressionOverridePatch";
    constexpr char* CKEY_DYNBUDGETENABLED = "DynamicUpdateBudget";
    constexpr char* CKEY_UPDATEBUDGET = "UpdateBudgetBase";
    constexpr char* CKEY_MAXTIME = "BudgetMaxFPS";
    constexpr char* CKEY_STATSON = "OSDStatsEnabled";

    using namespace Patching;

    DPapyrus DPapyrus::m_Instance;

    void DPapyrus::LoadConfig()
    {
        conf.seo_fix = GetConfigValue(SECTION_PAPYRUS, CKEY_SEOFIX, false);
        conf.dynbudget_enabled = GetConfigValue(SECTION_PAPYRUS, CKEY_DYNBUDGETENABLED, false);
        conf.dynbudget_fps_min = 60.0f;
        conf.dynbudget_fps_max = std::clamp(GetConfigValue(SECTION_PAPYRUS, CKEY_MAXTIME, 144.0f), conf.dynbudget_fps_min, 300.0f);
        conf.dynbudget_base = std::clamp(GetConfigValue(SECTION_PAPYRUS, CKEY_UPDATEBUDGET, 1.2f), 0.1f, 4.0f);
        conf.stats_enabled = GetConfigValue(SECTION_PAPYRUS, CKEY_STATSON, false);
    }

    void DPapyrus::PostLoadConfig()
    {
        if (conf.dynbudget_enabled)
        {
            if (conf.dynbudget_fps_max == conf.dynbudget_fps_min) {
                Warning("dynbudget_fps_max == dynbudget_fps_min, disabling..");
                conf.dynbudget_enabled = false;
            }
            else {
                fts = PerfCounter::Query();

                OSDDriver = IDDispatcher::GetDriver<DOSD>(DRIVER_OSD);

                enable_stats = OSDDriver && OSDDriver->IsOK() &&
                    OSDDriver->conf.enabled && conf.stats_enabled;

                bmult = conf.dynbudget_base / (1.0f / 60.0f * 1000.0f) * 1000.0f;
                t_max = 1.0f / conf.dynbudget_fps_min;
                t_min = 1.0f / conf.dynbudget_fps_max;

                Message("UpdateBudgetBase: %.6g ms (%.6g - %.6g)",
                    conf.dynbudget_base, t_min * bmult, t_max * bmult);

            }
        }
    }

    void DPapyrus::Patch()
    {
        if (conf.seo_fix)
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

        if (conf.dynbudget_enabled)
        {
            struct UpdateBudgetInject : JITASM {
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
        if (conf.dynbudget_enabled && enable_stats) {
            IEvents::RegisterForEvent(Event::OnD3D11PostCreate, OnD3D11PostCreate_Papyrus);
        }
    }

    bool DPapyrus::Prepare()
    {
        fUpdateBudgetMS = ISKSE::GetINISettingAddr<float*>("fUpdateBudgetMS:Papyrus");
        if (!fUpdateBudgetMS) {
            return false;
        }

        return true;
    }

    float DPapyrus::CalculateUpdateBudget()
    {
        long long fte = PerfCounter::Query();
        float cft = PerfCounter::delta(m_Instance.fts, fte);
        m_Instance.fts = fte;

        if (cft > m_Instance.t_max) {
            cft = m_Instance.t_max;
        }
        else if (cft < m_Instance.t_min) {
            cft = m_Instance.t_min;
        }

        cft *= m_Instance.bmult;
        *m_Instance.fUpdateBudgetMS = cft;

        return cft;
    }

    float DPapyrus::CalculateUpdateBudgetStats()
    {
        float cft = CalculateUpdateBudget();

        IStats::Accum(3, cft);

        return cft;
    }

    const wchar_t* DPapyrus::StatsRendererCallback()
    {
        double val;
        if (IStats::Addr(3, val)) {
            _snwprintf_s(m_Instance.bufStats1,
                _TRUNCATE, L"fUpdateBudgetMS: %.4g", val);
        }

        return m_Instance.bufStats1;
    }

    void DPapyrus::OnD3D11PostCreate_Papyrus(Event, void*)
    {
        //auto info = reinterpret_cast<D3D11CreateEventPost*>(data);

        m_Instance.bufStats1[0] = 0x0;
        m_Instance.OSDDriver->AddStatsCallback(StatsRendererCallback);
    }

}