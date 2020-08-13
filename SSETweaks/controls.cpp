#include "pch.h"

namespace SDT 
{
    constexpr char* SECTION_CONTROLS = "Controls";

    constexpr char* CKEY_DAMPINGFIX = "ThirdPersonMovementFix";
    constexpr char* CKEY_TCPFTHRESH = "MovementThreshold";
    constexpr char* CKEY_FSHS = "SittingHorizontalLookSensitivityFix";

    DControls DControls::m_Instance;

    void DControls::LoadConfig()
    {
        conf.damping_fix = GetConfigValue(SECTION_CONTROLS, CKEY_DAMPINGFIX, true);
        conf.tcpf_threshold = std::clamp(GetConfigValue(SECTION_CONTROLS, CKEY_TCPFTHRESH, 0.25f), 0.01f, 5.0f);
        conf.fp_mount_horiz_sens = GetConfigValue(SECTION_CONTROLS, CKEY_FSHS, true);
    }

    void DControls::PostLoadConfig()
    {
        if (conf.damping_fix) {
            Message("Third person movement threshold: %.6g", conf.tcpf_threshold);
        }
    }

    bool DControls::Prepare()
    {
        fMouseHeadingXScale = ISKSE::GetINISettingAddr<float*>("fMouseHeadingXScale:Controls");
        fMouseHeadingSensitivity = ISKSE::GetINIPrefSettingAddr<float*>("fMouseHeadingSensitivity:Controls");

        return true;
    }

    void DControls::Patch()
    {
        if (conf.damping_fix)
        {
            struct MovementThresholdInject : JITASM {
                MovementThresholdInject(uintptr_t retnAddr, float* maxvAddr)
                    : JITASM()
                {
                    Xbyak::Label maxvLabel;
                    Xbyak::Label retnLabel;

                    movss(xmm9, dword[rip + maxvLabel]);
                    mov(dword[rsp + 0x30], 0x7F7FFFFF);

                    jmp(ptr[rip + retnLabel]);

                    L(retnLabel);
                    dq(retnAddr);

                    L(maxvLabel);
                    db(reinterpret_cast<Xbyak::uint8*>(maxvAddr), sizeof(float));
                }
            };
  
            LogPatchBegin(CKEY_DAMPINGFIX);
            {
                MovementThresholdInject code(MT_Inject + 0x8, &conf.tcpf_threshold);
                g_branchTrampoline.Write6Branch(MT_Inject, code.get());

                Patching::safe_memset(MT_Inject + 0x6, 0xCC, 0x2);
            }
            LogPatchEnd(CKEY_DAMPINGFIX);
        }

        if (conf.fp_mount_horiz_sens)
        {
            if (fMouseHeadingXScale && fMouseHeadingSensitivity)
            {
                struct FirstPersonSitHorizontal : JITASM {
                    FirstPersonSitHorizontal(uintptr_t retnAddr, uintptr_t callAddr)
                        : JITASM()
                    {
                        Xbyak::Label retnLabel;
                        Xbyak::Label callLabel;

                        mov(rcx, rax); // PlayerControls
                        mov(rdx, rbx); // FirstPersonState
                        call(ptr[rip + callLabel]);
                        jmp(ptr[rip + retnLabel]);

                        L(retnLabel);
                        dq(retnAddr);

                        L(callLabel);
                        dq(callAddr);
                    }
                };

                LogPatchBegin(CKEY_FSHS);
                {
                    FirstPersonSitHorizontal code(FMHS_Inject + 0x17, uintptr_t(MouseSens_Hook));
                    g_branchTrampoline.Write6Branch(FMHS_Inject, code.get());
                }
                LogPatchEnd(CKEY_FSHS);
            }
            else {
                Error("%s: could not apply patch", CKEY_FSHS);
            }
        }
    }

    void DControls::MouseSens_Hook(PlayerControls* p1, FirstPersonState* p2)
    {
        if (*FrameTimer == 0.0f)
            return;

        auto f = *m_Instance.fMouseHeadingXScale * *m_Instance.fMouseHeadingSensitivity;
        p2->unk68[0] = *UnkFloat0 * (p1->unk02C / (f / *FrameTimer) * (f * 30.0f)) + p2->unk68[0];
    }

}