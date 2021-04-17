#include "pch.h"

namespace SDT 
{
    static constexpr const char* CKEY_DAMPINGFIX = "ThirdPersonMovementFix";
    static constexpr const char* CKEY_TCPFTHRESH = "MovementThreshold";
    static constexpr const char* CKEY_FSHS = "SittingHorizontalLookSensitivityFix";
    static constexpr const char* CKEY_MAP_KB_MOVEMENT = "MapMoveKeyboardSpeedFix";
    static constexpr const char* CKEY_MAP_KB_MOVEMENT_SPEED = "MapMoveKeyboardSpeedMult";

    DControls DControls::m_Instance;

    void DControls::LoadConfig()
    {
        m_conf.damping_fix = GetConfigValue(CKEY_DAMPINGFIX, true);
        m_conf.tcpf_threshold = std::clamp(GetConfigValue(CKEY_TCPFTHRESH, 0.25f), 0.01f, 5.0f);
        m_conf.fp_mount_horiz_sens = GetConfigValue(CKEY_FSHS, true);
        m_conf.map_kb_movement = GetConfigValue(CKEY_MAP_KB_MOVEMENT, true);
        m_conf.map_kb_movement_speedmult = std::clamp(GetConfigValue(CKEY_MAP_KB_MOVEMENT_SPEED, 1.0f), -20.0f, 20.0f);
    }

    void DControls::PostLoadConfig()
    {
        if (m_conf.damping_fix) {
            Message("Third person movement threshold: %.6g", m_conf.tcpf_threshold);
        }
    }

    bool DControls::Prepare()
    {
        m_gv.fMouseHeadingXScale = ISKSE::GetINISettingAddr<float>("fMouseHeadingXScale:Controls");
        m_gv.fMouseHeadingSensitivity = ISKSE::GetINIPrefSettingAddr<float>("fMouseHeadingSensitivity:Controls");

        return true;
    }

    void DControls::Patch()
    {
        if (m_conf.damping_fix)
        {
            struct MovementThresholdInject : JITASM::JITASM {
                MovementThresholdInject(std::uintptr_t retnAddr, float* maxvAddr)
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
                MovementThresholdInject code(MT_Inject + 0x8, &m_conf.tcpf_threshold);
                g_branchTrampoline.Write6Branch(MT_Inject, code.get());

                Patching::safe_memset(MT_Inject + 0x6, 0xCC, 0x2);
            }
            LogPatchEnd(CKEY_DAMPINGFIX);
        }

        if (m_conf.fp_mount_horiz_sens)
        {
            if (m_gv.fMouseHeadingXScale && m_gv.fMouseHeadingSensitivity)
            {
                struct FirstPersonSitHorizontal : JITASM::JITASM {
                    FirstPersonSitHorizontal(std::uintptr_t retnAddr, std::uintptr_t callAddr)
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
                    FirstPersonSitHorizontal code(FMHS_Inject + 0x17, std::uintptr_t(MouseSens_Hook));
                    g_branchTrampoline.Write6Branch(FMHS_Inject, code.get());
                }
                LogPatchEnd(CKEY_FSHS);
            }
            else {
                Error("%s: could not apply patch", CKEY_FSHS);
            }
        }

        if (m_conf.map_kb_movement)
        {
            LogPatchBegin(CKEY_MAP_KB_MOVEMENT);

            WriteKBMovementPatchDir(MapLookHandler_ProcessButton_Up, true);
            WriteKBMovementPatchDir(MapLookHandler_ProcessButton_Down, true);
            WriteKBMovementPatchDir(MapLookHandler_ProcessButton_Left);
            WriteKBMovementPatchDir(MapLookHandler_ProcessButton_Right);

            LogPatchEnd(CKEY_MAP_KB_MOVEMENT);
        }
    }

    void DControls::RegisterHooks()
    {
        if (m_conf.map_kb_movement)
        {
            if (!Hook::Call5(
                MapLookHandler_ProcessButton_Add,
                std::uintptr_t(AddMapCameraPos_Hook),
                addCameraPos_o))
            {
                Warning("%s: cam pos normalization hook failed", CKEY_MAP_KB_MOVEMENT);
            }
        }
    }

    void DControls::WriteKBMovementPatchDir(
        std::uintptr_t a_address,
        bool a_isY
    )
    {
        struct MapKeyboardMovementSpeedInject : JITASM::JITASM 
        {
            MapKeyboardMovementSpeedInject(
                std::uintptr_t a_targetAddr,
                const float *a_speedMult,
                bool a_isY)
                : JITASM()
            {
                Xbyak::Label retnLabel;
                Xbyak::Label timerLabel;
                Xbyak::Label speedLabel;

                mov(rcx, ptr[rip + timerLabel]);

                if (a_isY)
                {
                    movss(xmm2, ptr[rcx]);
                    movss(xmm1, ptr[rip + speedLabel]);
                    mulss(xmm2, xmm1);
                }
                else
                {
                    movss(xmm1, ptr[rcx]);
                    movss(xmm2, ptr[rip + speedLabel]);
                    mulss(xmm1, xmm2);
                }

                jmp(ptr[rip + retnLabel]);

                L(retnLabel);
                dq(a_targetAddr + 0x8);

                L(timerLabel);
                dq(std::uintptr_t(Game::g_frameTimer));

                L(speedLabel);
                db(reinterpret_cast<const Xbyak::uint8*>(a_speedMult), sizeof(float));
            }
        };

        MapKeyboardMovementSpeedInject code(
            a_address, 
            std::addressof(m_conf.map_kb_movement_speedmult), 
            a_isY);

        g_branchTrampoline.Write6Branch(a_address, code.get());
    }

    void DControls::MouseSens_Hook(PlayerControls* a_controls, FirstPersonState* a_fpState)
    {
        float interval = *Game::g_frameTimer;

        if (interval < _EPSILON)
            return;

        auto f = *m_Instance.m_gv.fMouseHeadingXScale * *m_Instance.m_gv.fMouseHeadingSensitivity;
        auto d = f / interval;

        if (d < _EPSILON)
            return;

        a_fpState->unk68[0] = *UnkFloat0 * (a_controls->unk02C / d * (f * 30.0f)) + a_fpState->unk68[0];
    }

    void DControls::AddMapCameraPos_Hook(
        MapCamera* a_camera,
        float a_x,
        float a_y,
        float a_z // 0
    )
    {
        m_Instance.addCameraPos_o(a_camera, a_x, a_y, a_z);

        // normalize
        auto pl = a_camera->pos.x * a_camera->pos.x + a_camera->pos.y * a_camera->pos.y;
        if (pl >= _EPSILON * _EPSILON)
        {
            auto ps = 1.0f / std::sqrtf(pl);
            a_camera->pos.x *= std::fabsf(a_camera->pos.x) * ps;
            a_camera->pos.y *= std::fabsf(a_camera->pos.y) * ps;
        }
    }

}