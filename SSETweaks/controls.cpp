#include "pch.h"

using namespace std;

namespace SDT {
    constexpr char* SECTION_CONTROLS = "Controls";

    constexpr char* CKEY_DAMPINGFIX = "ThirdPersonMovementFix";
    constexpr char* CKEY_TCPFTHRESH = "MovementThreshold";

    DControls DControls::m_Instance;

    void DControls::LoadConfig()
    {
        conf.damping_fix = GetConfigValue(SECTION_CONTROLS, CKEY_DAMPINGFIX, false);
        conf.tcpf_threshold = clamp(GetConfigValue(SECTION_CONTROLS, CKEY_TCPFTHRESH, 0.25f), 0.01f, 5.0f);
    }

    void DControls::PostLoadConfig()
    {
        if (conf.damping_fix) {
            Message("Third person movement threshold: %.6g", conf.tcpf_threshold);
        }
    }

    bool DControls::Prepare()
    {
        return true;
    }

    void DControls::Patch()
    {
        if (conf.damping_fix)
        {
            struct MovementThresholdInject : JITASM {
                MovementThresholdInject(uintptr_t retnAddr, void* maxvAddr)
                    : JITASM()
                {
                    Xbyak::Label maxvLabel;
                    Xbyak::Label retnLabel;

                    movss(xmm9, ptr[rip + maxvLabel]);
                    mov(dword[rsp + 0x30], 0x7F7FFFFF);

                    jmp(ptr[rip + retnLabel]);

                    L(retnLabel);
                    dq(retnAddr);

                    L(maxvLabel);
                    db(reinterpret_cast<Xbyak::uint8*>(maxvAddr), 4);
                }
            };

            LogPatchBegin(CKEY_DAMPINGFIX);
            {
                MovementThresholdInject code(MT_Inject + 0x8, &conf.tcpf_threshold);
                g_branchTrampoline.Write6Branch(MT_Inject, code.get());
            }
            LogPatchEnd(CKEY_DAMPINGFIX);
        }
    }
}