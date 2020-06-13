#include "pch.h"

namespace SDT
{
    constexpr char* SECTION_MISC = "Miscellaneous";

    constexpr char* CKEY_SKIPMISSINGINI = "SkipMissingPluginINI";

    DMisc DMisc::m_Instance;

    void DMisc::LoadConfig()
    {
        conf.skipmissingini = GetConfigValue(SECTION_MISC, CKEY_SKIPMISSINGINI, false);
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
        if (conf.skipmissingini) {
            Message("Disabling processing of missing plugin INIs");
        }
    }

    void DMisc::Patch()
    {
        if (conf.skipmissingini) {
            safe_write(
                SkipNoINI,
                reinterpret_cast<const void*>(Payloads::SkipNoINI),
                sizeof(Payloads::SkipNoINI));
        }
    }

    void DMisc::RegisterHooks()
    {
        if (conf.skipmissingini) {
            RegisterHook(
                SkipNoINI + 0x3,
                GetFnAddr(&Structures::_SettingCollectionList::LoadIni_Hook),
                HookDescriptor::kWR6Call
            );
        }
    }

    bool DMisc::Prepare()
    {
        return true;
    }
}
