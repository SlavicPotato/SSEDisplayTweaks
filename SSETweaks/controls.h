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
        } conf;

        inline static auto MT_Inject = IAL::Addr(AID::UnkMovFunc0, Offsets::MT_Inject);

        static DControls m_Instance;
    };
}