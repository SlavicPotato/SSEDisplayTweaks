#pragma once

namespace SDT
{
    class DControls :
        public IDriver,
        IConfig
    {
    public:
        static inline constexpr auto ID = DRIVER_ID::CONTROLS;

        FN_NAMEPROC("Controls")
        FN_ESSENTIAL(false)
        FN_DRVDEF(6)
    private:
        DControls() = default;

        virtual void LoadConfig() override;
        virtual void PostLoadConfig() override;
        virtual void Patch() override;
        virtual bool Prepare() override;

        struct {
            bool damping_fix;
            float tcpf_threshold;
            float fp_mount_horiz_sens;
        } m_conf;

        static void MouseSens_Hook(PlayerControls* a_controls, FirstPersonState* a_fpState);

        float* fMouseHeadingXScale;
        float* fMouseHeadingSensitivity;

        inline static auto MT_Inject = IAL::Addr(AID::UnkMovFunc0, Offsets::MT_Inject);

        inline static auto UnkFloat0 = IAL::Addr<float*>(AID::UnkFloat0);
        inline static auto FMHS_Inject = IAL::Addr(AID::UnkMM0, Offsets::FMHS_Inject);

        static DControls m_Instance;
    };
}