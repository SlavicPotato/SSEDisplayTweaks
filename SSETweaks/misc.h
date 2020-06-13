#pragma once

namespace SDT
{
    class DMisc :
        public IDriver,
        IConfig
    {
    public:
        FN_NAMEPROC("Miscellaneous")
        FN_ESSENTIAL(false)
        FN_PRIO(6)
        FN_DRVID(DRIVER_MISC)
    private:
        DMisc() = default;

        virtual void LoadConfig() override;
        virtual void PostLoadConfig() override;
        virtual void Patch() override;
        virtual void RegisterHooks() override;
        virtual bool Prepare() override;

        struct {
            bool skipmissingini;
        }conf;

        inline static auto SkipNoINI = IAL::Addr(AID::INIProc0, Offsets::SkipNoINI);

        static DMisc m_Instance;
    };
}
