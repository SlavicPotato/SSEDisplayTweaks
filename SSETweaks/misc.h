#pragma once

namespace SDT
{
    class DMisc :
        public IDriver,
        IConfig
    {
    public:
        static inline constexpr auto ID = DRIVER_ID::MISC;

        FN_NAMEPROC("Miscellaneous")
        FN_ESSENTIAL(false)
        FN_DRVDEF(6)
    private:
        DMisc() = default;

        virtual void LoadConfig() override;
        virtual void PostLoadConfig() override;
        virtual void Patch() override;
        virtual void RegisterHooks() override;
        virtual bool Prepare() override;

        struct {
            bool skipmissingini;
        }m_conf;

        inline static auto SkipNoINI = IAL::Addr(AID::INIProc0, Offsets::SkipNoINI);

        static DMisc m_Instance;
    };
}
