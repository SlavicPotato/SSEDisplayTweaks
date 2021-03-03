#include "pch.h"

namespace SDT
{
    static constexpr const char* CKEY_SKIPMISSINGINI = "SkipMissingPluginINI";

    DMisc DMisc::m_Instance;

    void DMisc::LoadConfig()
    {
        m_conf.skipmissingini = GetConfigValue(CKEY_SKIPMISSINGINI, true);
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
    }

    bool DMisc::Prepare()
    {
        return true;
    }
}
