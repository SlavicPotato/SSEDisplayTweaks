#pragma once

namespace SDT
{
    class DControls :
        public IDriver,
        IConfig
    {
        typedef void (*writeCameraPos_t)(MapCamera*, float, float, float);
    public:
        static inline constexpr auto ID = DRIVER_ID::CONTROLS;

        FN_NAMEPROC("Controls");
        FN_ESSENTIAL(false);
        FN_DRVDEF(6);
    private:
        DControls() = default;

        virtual void LoadConfig() override;
        virtual void PostLoadConfig() override;
        virtual void Patch() override;
        virtual void RegisterHooks() override;
        virtual bool Prepare() override;

        struct {
            bool damping_fix;
            float tcpf_threshold;
            float fp_mount_horiz_sens;
            bool map_kb_movement;
            float map_kb_movement_speedmult;
            bool auto_vanity_camera;
            bool dialogue_look;
        } m_conf;

        void WriteKBMovementPatchDir(std::uintptr_t a_address, bool a_isY = false);

        static void MouseSens_Hook(PlayerControls* a_controls, FirstPersonState* a_fpState);
        static void AddMapCameraPos_Hook(MapCamera* a_camera, float x, float y, float z);

        struct {
            float* fMouseHeadingXScale;
            float* fMouseHeadingSensitivity;
        } m_gv;

        writeCameraPos_t addCameraPos_o;

        inline static auto MT_Inject = IAL::Addr(AID::UnkMovFunc0, Offsets::MT_Inject);

        inline static auto UnkFloat0 = IAL::Addr<float*>(AID::UnkFloat0);
        inline static auto FMHS_Inject = IAL::Addr(AID::UnkMM0, Offsets::FMHS_Inject);
        inline static auto MapLookHandler_ProcessButton = IAL::Addr<std::uintptr_t>(AID::MapLookHandler_ProcessButton);
        inline static auto AutoVanityState_Update = IAL::Addr<std::uintptr_t>(AID::AutoVanityState_Update);
        inline static auto PlayerControls_InputEvent_ProcessEvent = IAL::Addr<std::uintptr_t>(AID::PlayerControls_InputEvent_ProcessEvent);

        static DControls m_Instance;
    };
}