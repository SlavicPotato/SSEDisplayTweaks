#include "pch.h"

namespace SDT
{
    static constexpr const char* CKEY_DAMPINGFIX = "ThirdPersonMovementFix";
    static constexpr const char* CKEY_TCPFTHRESH = "MovementThreshold";
    static constexpr const char* CKEY_FSHS = "SittingHorizontalLookSensitivityFix";
    static constexpr const char* CKEY_MAP_KB_MOVEMENT = "MapMoveKeyboardSpeedFix";
    static constexpr const char* CKEY_MAP_KB_MOVEMENT_SPEED = "MapMoveKeyboardSpeedMult";
    static constexpr const char* CKEY_AUTO_VANITY_CAMERA = "AutoVanityCameraSpeedFix";
    static constexpr const char* CKEY_PC_DIALOGUE_LOOK = "DialogueLookSpeedFix";
    static constexpr const char* CKEY_PC_DIALOGUE_LOOK_SE = "DialogueLookSmoothEdge";
    static constexpr const char* CKEY_GP_CURSOR = "GamepadCursorSpeedFix";
    static constexpr const char* CKEY_LOCKPICK_ROTATION = "LockpickRotationSpeedFix";
    static constexpr const char* CKEY_FREECAM_VERTICALSENS = "FreeCameraVerticalSensitivityFix";
    static constexpr const char* CKEY_FREECAM_TRANSLATION = "FreeCameraMovementSpeedFix";

    DControls DControls::m_Instance;

    void DControls::LoadConfig()
    {
        m_conf.damping_fix = GetConfigValue(CKEY_DAMPINGFIX, true);
        m_conf.tcpf_threshold = std::clamp(GetConfigValue(CKEY_TCPFTHRESH, 0.25f), 0.01f, 5.0f);
        m_conf.fp_mount_horiz_sens = GetConfigValue(CKEY_FSHS, true);
        m_conf.map_kb_movement = GetConfigValue(CKEY_MAP_KB_MOVEMENT, true);
        m_conf.map_kb_movement_speedmult = std::clamp(GetConfigValue(CKEY_MAP_KB_MOVEMENT_SPEED, 1.0f), -20.0f, 20.0f);
        m_conf.auto_vanity_camera = GetConfigValue(CKEY_AUTO_VANITY_CAMERA, true);
        m_conf.dialogue_look = GetConfigValue(CKEY_PC_DIALOGUE_LOOK, true);
        m_conf.dialogue_look_se = GetConfigValue(CKEY_PC_DIALOGUE_LOOK_SE, false);
        m_conf.gamepad_cursor_speed = GetConfigValue(CKEY_GP_CURSOR, true);
        m_conf.lockpick_rotation = GetConfigValue(CKEY_LOCKPICK_ROTATION, true);
        m_conf.freecam_verticalsens = GetConfigValue(CKEY_FREECAM_VERTICALSENS, true);
        m_conf.freecam_translation = GetConfigValue(CKEY_FREECAM_TRANSLATION, true);
    }

    void DControls::PostLoadConfig()
    {
    }

    bool DControls::Prepare()
    {
        return true;
    }

    void DControls::Patch()
    {
        if (m_conf.damping_fix) {
            Patch_Damping();
        }

        if (m_conf.fp_mount_horiz_sens) {
            Patch_FPMountHorizontalSens();
        }

        if (m_conf.map_kb_movement) {
            Patch_MapKBMovement();
        }

        if (m_conf.auto_vanity_camera) {
            Patch_AutoVanityCamera();
        }

        if (m_conf.dialogue_look) {
            Patch_DialogueLook();
        }

        if (m_conf.dialogue_look_se) {
            Patch_DialogueLook_Edge();
        }

        if (m_conf.gamepad_cursor_speed) {
            Patch_GamepadCursor();
        }

        if (m_conf.lockpick_rotation) {
            Patch_LockpickRotation();
        }

        if (m_conf.freecam_verticalsens) {
            Patch_FreecamVerticalSens();
        }

        if (m_conf.freecam_translation) {
            Patch_FreecamTranslation();
        }

    }

    void DControls::RegisterHooks()
    {
        if (m_conf.map_kb_movement)
        {
            if (!Hook::Call5(
                MapLookHandler_ProcessButton + Offsets::MapLookHandler_ProcessButton_Add,
                std::uintptr_t(AddMapCameraPos_Hook),
                addCameraPos_o))
            {
                Warning("%s: cam pos normalization hook failed", CKEY_MAP_KB_MOVEMENT);
            }
        }
    }

    void DControls::Patch_Damping()
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

    void DControls::Patch_FPMountHorizontalSens()
    {
        m_gv.fMouseHeadingXScale = ISKSE::GetINISettingAddr<float>("fMouseHeadingXScale:Controls");
        m_gv.fMouseHeadingSensitivity = ISKSE::GetINIPrefSettingAddr<float>("fMouseHeadingSensitivity:Controls");

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

    void DControls::Patch_MapKBMovement()
    {
        LogPatchBegin(CKEY_MAP_KB_MOVEMENT);

        WriteKBMovementPatchDir(MapLookHandler_ProcessButton + Offsets::MapLookHandler_ProcessButton_Up, true);
        WriteKBMovementPatchDir(MapLookHandler_ProcessButton + Offsets::MapLookHandler_ProcessButton_Down, true);
        WriteKBMovementPatchDir(MapLookHandler_ProcessButton + Offsets::MapLookHandler_ProcessButton_Left);
        WriteKBMovementPatchDir(MapLookHandler_ProcessButton + Offsets::MapLookHandler_ProcessButton_Right);

        LogPatchEnd(CKEY_MAP_KB_MOVEMENT);
    }

    void DControls::Patch_AutoVanityCamera()
    {
        auto fAutoVanityIncrement = ISKSE::GetINISettingAddr<float>("fAutoVanityIncrement:Camera");

        if (fAutoVanityIncrement)
        {
            struct AutoVanityStateUpdate : JITASM::JITASM {
                AutoVanityStateUpdate(
                    std::uintptr_t a_targetAddr,
                    std::uintptr_t a_fAutoVanityIncrementAddr)
                    : JITASM()
                {
                    Xbyak::Label retnLabel;
                    Xbyak::Label timerLabel;
                    Xbyak::Label magicLabel;
                    Xbyak::Label fAutoVanityIncrementLabel;

                    mov(rcx, ptr[rip + fAutoVanityIncrementLabel]);
                    movss(xmm1, dword[rcx]);
                    mulss(xmm1, dword[rip + magicLabel]);
                    mov(rcx, ptr[rip + timerLabel]);
                    mulss(xmm1, dword[rcx]);
                    subss(xmm0, xmm1);
                    jmp(ptr[rip + retnLabel]);

                    L(retnLabel);
                    dq(a_targetAddr + 0x8);

                    L(timerLabel);
                    dq(std::uintptr_t(Game::g_frameTimer));

                    L(magicLabel);
                    dd(0x42700000); // 60.0f

                    L(fAutoVanityIncrementLabel);
                    dq(a_fAutoVanityIncrementAddr);
                }
            };

            LogPatchBegin(CKEY_AUTO_VANITY_CAMERA);
            {
                auto addr(AutoVanityState_Update + Offsets::AutoVanityState_Update_IncrementAngle);
                AutoVanityStateUpdate code(addr, std::uintptr_t(fAutoVanityIncrement));
                g_branchTrampoline.Write6Branch(addr, code.get());
            }
            LogPatchEnd(CKEY_AUTO_VANITY_CAMERA);
        }
        else {
            Error("%s: could not apply patch", CKEY_AUTO_VANITY_CAMERA);
        }
    }

    void DControls::Patch_DialogueLook()
    {
        auto fPCDialogueLookSpeed = ISKSE::GetINISettingAddr<float>("fPCDialogueLookSpeed:Controls");

        if (fPCDialogueLookSpeed)
        {
            struct DialogueLookSpeedUpdate : JITASM::JITASM {
                DialogueLookSpeedUpdate(
                    std::uintptr_t a_targetAddr,
                    std::uintptr_t a_fPCDialogueLookSpeedAddr)
                    : JITASM::JITASM()
                {
                    Xbyak::Label retnLabel;
                    Xbyak::Label timerLabel;
                    Xbyak::Label magicLabel;
                    Xbyak::Label fPCDialogueLookSpeedLabel;

                    mov(rcx, ptr[rip + fPCDialogueLookSpeedLabel]);
                    movss(xmm1, dword[rcx]);
                    mulss(xmm1, dword[rip + magicLabel]);
                    mov(rcx, ptr[rip + timerLabel]);
                    mulss(xmm1, dword[rcx]);
                    jmp(ptr[rip + retnLabel]);

                    L(retnLabel);
                    dq(a_targetAddr + 0x8);

                    L(timerLabel);
                    dq(std::uintptr_t(Game::g_frameTimer));

                    L(magicLabel);
                    dd(0x42700000); // 60.0f

                    L(fPCDialogueLookSpeedLabel);
                    dq(a_fPCDialogueLookSpeedAddr);
                }
            };

            LogPatchBegin(CKEY_PC_DIALOGUE_LOOK);
            {
                auto addr(
                    PlayerControls_InputEvent_ProcessEvent +
                    Offsets::PlayerControls_InputEvent_ProcessEvent_LoadDLSpeed);

                DialogueLookSpeedUpdate code(addr, std::uintptr_t(fPCDialogueLookSpeed));
                g_branchTrampoline.Write6Branch(addr, code.get());
            }
            LogPatchEnd(CKEY_PC_DIALOGUE_LOOK);
        }
        else {
            Error("Could not apply patch: %s", CKEY_PC_DIALOGUE_LOOK);
        }
    }

    void DControls::Patch_DialogueLook_Edge()
    {
        m_gv.fPCDialogueLookStart = ISKSE::GetINISettingAddr<float>("fPCDialogueLookStart:Controls");

        if (m_gv.fPCDialogueLookStart)
        {
            struct DialogueLookSmooth : JITASM::JITASM {
                DialogueLookSmooth(
                    std::uintptr_t a_targetAddr)
                    : JITASM()
                {
                    Xbyak::Label retnLabel;
                    Xbyak::Label callLabel;

                    movss(dword[rbx + 0x2C], xmm0);
                    mov(rcx, rbx);
                    call(ptr[rip + callLabel]);
                    jmp(ptr[rip + retnLabel]);

                    L(retnLabel);
                    dq(a_targetAddr + 0xD);

                    L(callLabel);
                    dq(std::uintptr_t(PlayerControls_InputEvent_ProcessEvent_140707110_Hook));
                }
            };

            LogPatchBegin(CKEY_PC_DIALOGUE_LOOK_SE);
            {
                auto addr(
                    PlayerControls_InputEvent_ProcessEvent +
                    Offsets::PlayerControls_InputEvent_ProcessEvent_movssix);

                DialogueLookSmooth code(addr);
                g_branchTrampoline.Write5Branch(addr, code.get());
            }
            LogPatchEnd(CKEY_PC_DIALOGUE_LOOK_SE);
        }
        else
        {
            Error("%s: could not apply patch", CKEY_PC_DIALOGUE_LOOK_SE);
        }
    }

    void DControls::Patch_GamepadCursor()
    {
        struct GamepadCursorSpeed : JITASM::JITASM {
            GamepadCursorSpeed(
                std::uintptr_t a_targetAddr)
                : JITASM()
            {
                Xbyak::Label retnLabel;
                Xbyak::Label timerLabel;
                Xbyak::Label magicLabel;

                mulss(xmm4, dword[rcx + 0x1C]);
                mulss(xmm4, dword[rip + magicLabel]);
                mov(rax, ptr[rip + timerLabel]);
                mulss(xmm4, dword[rax]);
                jmp(ptr[rip + retnLabel]);

                L(retnLabel);
                dq(a_targetAddr + 0x6);

                L(timerLabel);
                dq(std::uintptr_t(Game::g_frameTimer));

                L(magicLabel);
                dd(0x42700000); // 60.0f
            }
        };

        LogPatchBegin(CKEY_GP_CURSOR);
        {
            auto addr(
                CursorMenu_MenuEventHandler_ProcessThumbstick_Sub140ED3120 +
                Offsets::CursorMenu_MenuEventHandler_ProcessThumbstick_MulCS);

            GamepadCursorSpeed code(addr);
            g_branchTrampoline.Write6Branch(addr, code.get());
        }
        LogPatchEnd(CKEY_GP_CURSOR);
    }

    void DControls::Patch_LockpickRotation()
    {
        // TODO: check ProcessThumbstick too

        struct LockpickRotationSpeedMouse : JITASM::JITASM {
            LockpickRotationSpeedMouse(
                std::uintptr_t a_targetAddr)
                : JITASM()
            {
                Xbyak::Label retnLabel;
                Xbyak::Label magicLabel;

                mulss(xmm1, dword[rip + magicLabel]);
                jmp(ptr[rip + retnLabel]);

                L(retnLabel);
                dq(a_targetAddr + 0x8);

                L(magicLabel);
                dd(0x3c88893b); // 0.016667f
            }
        };

        LogPatchBegin(CKEY_LOCKPICK_ROTATION);
        {
            auto addr(
                LockpickingMenu_ProcessMouseMove +
                Offsets::LockpickingMenu_ProcessMouseMove_MulFT);

            LockpickRotationSpeedMouse code(addr);
            g_branchTrampoline.Write6Branch(addr, code.get());
        }
        LogPatchEnd(CKEY_LOCKPICK_ROTATION);
    }

    void DControls::Patch_FreecamVerticalSens()
    {
        struct FreeCameraVerticalSensitivity : JITASM::JITASM {
            FreeCameraVerticalSensitivity(
                std::uintptr_t a_targetAddr)
                : JITASM()
            {
                Xbyak::Label retnLabel;
                Xbyak::Label magicLabel;

                mulss(xmm0, xmm1);
                mulss(xmm2, dword[rip + magicLabel]);
                jmp(ptr[rip + retnLabel]);

                L(retnLabel);
                dq(a_targetAddr + 0x8);

                L(magicLabel);
                dd(0x3d6a0ea1); // 17.5f
            }
        };

        LogPatchBegin(CKEY_FREECAM_VERTICALSENS);
        {
            auto addr(FreeCameraState_Update_Sub140848AA0 + 0x97);
            FreeCameraVerticalSensitivity code(addr);
            g_branchTrampoline.Write6Branch(addr, code.get());
        }
        LogPatchEnd(CKEY_FREECAM_VERTICALSENS);
    }

    void DControls::Patch_FreecamTranslation()
    {
        struct FreeCameraTranslationSpeedFwdBack : JITASM::JITASM {
            FreeCameraTranslationSpeedFwdBack(
                std::uintptr_t a_targetAddr)
                : JITASM()
            {
                Xbyak::Label retnLabel;
                Xbyak::Label magicLabel;

                movss(xmm10, dword[rax + 0x24]);
                mulss(xmm8, dword[rip + magicLabel]);
                mulss(xmm8, xmm1); // xmm1 = no slow timer
                jmp(ptr[rip + retnLabel]);

                L(retnLabel);
                dq(a_targetAddr + 0x6);

                L(magicLabel);
                dd(0x42700000); // 60.0f
            }
        };

        LogPatchBegin("FreeCameraMovementSpeedFix (forward/back)");
        {
            auto addr(FreeCameraState_Update_Sub140848AA0 + 0x74);
            FreeCameraTranslationSpeedFwdBack code(addr);
            g_branchTrampoline.Write6Branch(addr, code.get());
        }
        LogPatchEnd("FreeCameraMovementSpeedFix (forward/back)");

        auto fFreeCameraRunSpeed = ISKSE::GetINISettingAddr<float>("fFreeCameraRunSpeed:Camera");

        if (fFreeCameraRunSpeed)
        {
            struct FreeCameraTranslationSpeedUpDown : JITASM::JITASM {
                FreeCameraTranslationSpeedUpDown(
                    std::uintptr_t a_targetAddr,
                    std::uintptr_t a_fFreeCameraRunSpeedAddr)
                    : JITASM()
                {
                    Xbyak::Label retnLabel;
                    Xbyak::Label timerLabel;
                    Xbyak::Label magicLabel;
                    Xbyak::Label fFreeCameraRunSpeedLabel;

                    Xbyak::Label notRunningLabel;

                    movaps(xmm3, xmm0);
                    je(notRunningLabel);
                    mov(rcx, ptr[rip + fFreeCameraRunSpeedLabel]);
                    mulss(xmm3, ptr[rcx]);
                    L(notRunningLabel);
                    mulss(xmm3, dword[rip + magicLabel]);
                    mov(rcx, ptr[rip + timerLabel]);
                    mulss(xmm3, dword[rcx]);
                    jmp(ptr[rip + retnLabel]);

                    L(retnLabel);
                    dq(a_targetAddr + 0x5);

                    L(timerLabel);
                    dq(std::uintptr_t(Game::g_frameTimer));

                    L(magicLabel);
                    dd(0x42700000); // 60.0f

                    L(fFreeCameraRunSpeedLabel);
                    dq(a_fFreeCameraRunSpeedAddr);
                }
            };

            LogPatchBegin("FreeCameraMovementSpeedFix (up/down)");
            {
                auto addr(FreeCameraState_Update_Sub140848AA0 + 0x285);
                FreeCameraTranslationSpeedUpDown code(addr, std::uintptr_t(fFreeCameraRunSpeed));
                g_branchTrampoline.Write5Branch(addr, code.get());
            }
            LogPatchEnd("FreeCameraMovementSpeedFix (up/down)");
        }
        else {
            Error("%s: up/down patch failed", CKEY_FREECAM_TRANSLATION);
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
                const float* a_speedMult,
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

        a_fpState->unk68[0] = *UnkFloat0 * (a_controls->lookinput.x / (f / interval) * (f * 30.0f)) + a_fpState->unk68[0];
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

    void DControls::PlayerControls_InputEvent_ProcessEvent_140707110_Hook(PlayerControls* a_controls)
    {
        Sub_140707110(a_controls);

        float lookStart = *m_Instance.m_gv.fPCDialogueLookStart;
        if (lookStart <= 0.0f) {
            return;
        }

        auto data = *unkCoordData;

        float pRight = data->bottomRight.x - lookStart;
        float pLeft = data->topLeft.x + lookStart;
        float pTop = data->topLeft.y + lookStart;
        float pBottom = data->bottomRight.y - lookStart;

        bool isRight = data->cursorPos.x > pRight;
        bool isLeft = data->cursorPos.x < pLeft;
        bool isTop = data->cursorPos.y < pTop;
        bool isBottom = data->cursorPos.y > pBottom;

        float m;

        if (isRight)
        {
            m = Math::NormalizeSafeClamp(data->cursorPos.x, pRight, data->bottomRight.x);

            if (isTop)
            {
                auto mb = 1.0f - Math::NormalizeSafeClamp(data->cursorPos.y, data->topLeft.y, pTop);
                if (mb > m) m = mb;
            }
            else if (isBottom)
            {
                auto mb = Math::NormalizeSafeClamp(data->cursorPos.y, pBottom, data->bottomRight.y);
                if (mb > m) m = mb;
            }
        }
        else if (isLeft)
        {
            m = 1.0f - Math::NormalizeSafeClamp(data->cursorPos.x, data->topLeft.x, pLeft);

            if (isTop)
            {
                auto mb = 1.0f - Math::NormalizeSafeClamp(data->cursorPos.y, data->topLeft.y, pTop);
                if (mb > m) m = mb;
            }
            else if (isBottom)
            {
                auto mb = Math::NormalizeSafeClamp(data->cursorPos.y, pBottom, data->bottomRight.y);
                if (mb > m) m = mb;
            }
        }
        else if (isTop)
        {
            m = 1.0f - Math::NormalizeSafeClamp(data->cursorPos.y, data->topLeft.y, pTop);
        }
        else if (isBottom)
        {
            m = Math::NormalizeSafeClamp(data->cursorPos.y, pBottom, data->bottomRight.y);
        }
        else {
            return;
        }

        m = m * m;

        a_controls->lookinput.y *= m;
        a_controls->lookinput.x *= m;
    }
}