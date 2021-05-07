#include "pch.h"
#include "Data/PluginInfo.h"

namespace SDT
{
    static constexpr const char* CKEY_SKIPMISSINGINI = "SkipMissingPluginINI";
    static constexpr const char* CKEY_LSF = "LoadScreenFilter";
    static constexpr const char* CKEY_LSF_ALLOW = "LoadScreenAllow";
    static constexpr const char* CKEY_LSF_BLOCK = "LoadScreenBlock";
    static constexpr const char* CKEY_DISABLE_WEATHER_LENSFLARE = "DisableWeatherLensFlare";

    DMisc DMisc::m_Instance;

    DMisc::DMisc()
        : m_dlsDenyAll(false)
    {
    }

    void DMisc::LoadConfig()
    {
        m_conf.skipmissingini = GetConfigValue(CKEY_SKIPMISSINGINI, true);
        m_conf.loadscreen_filter = GetConfigValue(CKEY_LSF, false);
        m_conf.dls_allow = GetConfigValue(CKEY_LSF_ALLOW, "");
        m_conf.dls_deny = GetConfigValue(CKEY_LSF_BLOCK, "All");
        m_conf.disable_lens_flare = GetConfigValue(CKEY_DISABLE_WEATHER_LENSFLARE, false);
    }

    void Structures::_SettingCollectionList::LoadIni_Hook()
    {
        DWORD dwAttrib = ::GetFileAttributesA(inipath);
        if (dwAttrib == INVALID_FILE_ATTRIBUTES ||
            (dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) {
            return;
        }

        this->LoadINI();
    }

    void DMisc::PostLoadConfig()
    {
        if (m_conf.skipmissingini) {
            Message("Disabling processing of missing plugin INIs");
        }
    }

    void DMisc::Patch()
    {
        if (m_conf.skipmissingini)
        {
            Patching::safe_write(
                SkipNoINI,
                static_cast<const void*>(Payloads::SkipNoINI),
                sizeof(Payloads::SkipNoINI));
        }

        if (m_conf.loadscreen_filter)
        {
            stl::vector<std::string> elems;
            StrHelpers::SplitString(m_conf.dls_deny, ',', elems);

            constexpr auto kall = "All";

            for (auto& e : elems)
            {
                StrHelpers::trim(e);
                if (e.empty()) {
                    continue;
                }

                if (StrHelpers::iequal(e.c_str(), kall)) {
                    m_dlsDenyAll = true;
                    break;
                }

                m_dlsBlock.emplace(e);
            }

            elems.clear();
            StrHelpers::SplitString(m_conf.dls_allow, ',', elems);

            for (auto& e : elems)
            {
                StrHelpers::trim(e);
                if (e.empty()) {
                    continue;
                }

                if (StrHelpers::iequal(e.c_str(), kall)) {
                    break;
                }

                m_dlsAllow.emplace(e);
            }

            m_conf.dls_allow.clear();
            m_conf.dls_deny.clear();

            m_Instance.m_pluginData = std::make_unique<IPluginInfo>();

            struct LoadScreenLoadFormInject : JITASM::JITASM {
                LoadScreenLoadFormInject(
                    std::uintptr_t targetAddr)
                    : JITASM()
                {
                    Xbyak::Label retnOKLabel;
                    Xbyak::Label retnSkipLabel;
                    Xbyak::Label callHookFnLabel;
                    Xbyak::Label callUnkFnLabel;

                    Xbyak::Label load;

                    mov(rcx, r14);
                    call(ptr[rip + callHookFnLabel]);
                    test(al, al);
                    jne(load);
                    jmp(ptr[rip + retnSkipLabel]);
                    L(load);
                    mov(rcx, rbp);
                    call(ptr[rip + callUnkFnLabel]);
                    jmp(ptr[rip + retnOKLabel]);

                    L(retnOKLabel);
                    dq(targetAddr + 0x8);

                    L(retnSkipLabel);
                    dq(targetAddr + 0x4EC);

                    L(callHookFnLabel);
                    dq(std::uintptr_t(TESLoadScreen_LoadForm_Hook));

                    L(callUnkFnLabel);
                    dq(Sub14017D910);
                }
            };

            auto target(TESLoadScreen_LoadForm + 0x36);
            LoadScreenLoadFormInject code(target);
            g_branchTrampoline.Write6Branch(target, code.get());

            //IEvents::RegisterForEvent(Event::OnMessage, MessageHandler);

            Message("Loadscreen filter installed");
        }
    }

    void DMisc::RegisterHooks()
    {
        if (m_conf.skipmissingini)
        {
            RegisterHook(
                SkipNoINI + 0x3,
                GetFnAddr(&Structures::_SettingCollectionList::LoadIni_Hook),
                HookDescriptor::HookType::kWR6Call
            );
        }

        if (m_conf.disable_lens_flare)
        {
            IEvents::RegisterForEvent(Event::OnMessage, MessageHandler);
        }
    }

    bool DMisc::Prepare()
    {
        return true;
    }

    bool DMisc::TESLoadScreen_LoadForm_Hook(TESLoadScreen* a_form)
    {
        if (m_Instance.m_dlsDenyAll && m_Instance.m_dlsAllow.empty()) {
            return false;
        }

        if (!m_Instance.m_pluginData->IsPopulated()) {
            m_Instance.m_pluginData->Populate();
        }

        UInt32 modIndex;
        if (a_form->formID.GetPluginPartialIndex(modIndex))
        {
            auto pi = m_Instance.m_pluginData->Lookup(modIndex);
            if (pi)
            {
                if (m_Instance.m_dlsAllow.contains(pi->name))
                {
                    return true;
                }

                if (m_Instance.m_dlsDenyAll || m_Instance.m_dlsBlock.contains(pi->name))
                {
                    return false;
                }
            }
        }

        return true;
    }

    void DMisc::RemoveLensFlareFromWeathers()
    {
        auto dh = DataHandler::GetSingleton();
        if (dh)
        {
            using size_type = decltype(dh->arrWTHR.count);

            std::size_t count(0);

            for (size_type i = 0; i < dh->arrWTHR.count; i++)
            {
                auto weather = dh->arrWTHR[i];
                if (weather && weather->lensFlare)
                {
                    weather->lensFlare = nullptr;
                    count++;
                }
            }

            m_Instance.Message("Removed lens flare from %zu/%zu weather form(s)",
                count, static_cast<std::size_t>(dh->arrWTHR.count));
        }
    }

    void DMisc::MessageHandler(Event, void* a_args)
    {
        auto message = static_cast<SKSEMessagingInterface::Message*>(a_args);

        if (message->type == SKSEMessagingInterface::kMessage_DataLoaded)
        {
            /*m_Instance.m_pluginData.reset();
            m_Instance.m_dlsBlock.swap(decltype(m_Instance.m_dlsBlock)());
            m_Instance.m_dlsAllow.swap(decltype(m_Instance.m_dlsAllow)());*/

            RemoveLensFlareFromWeathers();
        }
    }

}
