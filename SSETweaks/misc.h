#pragma once

namespace SDT
{
    class IPluginInfo;

    class DMisc :
        public IDriver,
        IConfig
    {
    public:
        static inline constexpr auto ID = DRIVER_ID::MISC;

        FN_NAMEPROC("Miscellaneous");
        FN_ESSENTIAL(false);
        FN_DRVDEF(6);
    private:
        DMisc();

        virtual void LoadConfig() override;
        virtual void PostLoadConfig() override;
        virtual void Patch() override;
        virtual void RegisterHooks() override;
        virtual bool Prepare() override;

        static bool TESLoadScreen_LoadForm_Hook(TESLoadScreen* a_form);
        static void MessageHandler(Event, void*);

        static void RemoveLensFlareFromWeathers();

        struct {
            bool skipmissingini;
            bool loadscreen_filter;
            bool disable_lens_flare;
            std::string dls_deny;
            std::string dls_allow;
        }m_conf;

        std::unique_ptr<IPluginInfo> m_pluginData;
        stl::iunordered_set<std::string> m_dlsAllow;
        stl::iunordered_set<std::string> m_dlsBlock;
        bool m_dlsDenyAll;

        inline static auto SkipNoINI = IAL::Addr(AID::INIProc0, Offsets::SkipNoINI);
        inline static auto TESLoadScreen_LoadForm = IAL::Addr<std::uintptr_t>(AID::TESLoadScreen_LoadForm);
        inline static auto Sub14017D910 = IAL::Addr<std::uintptr_t>(AID::Sub_14017D910);

        static DMisc m_Instance;
    };
}
