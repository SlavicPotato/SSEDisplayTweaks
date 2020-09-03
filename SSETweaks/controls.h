#pragma once

namespace SDT
{
    class DControls :
        public IDriver,
        IConfig
    {
    public:
        FN_NAMEPROC("Controls")
        FN_ESSENTIAL(false)
        FN_PRIO(6)
        FN_DRVID(DRIVER_CONTROLS)
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
        } conf;

        static void MouseSens_Hook(PlayerControls* p1, FirstPersonState* p2);

        float* fMouseHeadingXScale;
        float* fMouseHeadingSensitivity;

        inline static auto MT_Inject = IAL::Addr(AID::UnkMovFunc0, Offsets::MT_Inject);

        inline static auto UnkFloat0 = IAL::Addr<float*>(AID::UnkFloat0);
        inline static auto FMHS_Inject = IAL::Addr(AID::UnkMM0, Offsets::FMHS_Inject);

        static DControls m_Instance;
    };
}