#pragma once

namespace SDT {
    constexpr size_t MAX_TRAMPOLINE_BRANCH = 256;
    constexpr size_t MAX_TRAMPOLINE_CODEGEN = 256;

    class ISKSE
    {
    public:
        static bool Query(const SKSEInterface* skse, PluginInfo* info);
        static bool Initialize(const SKSEInterface* skse);

        template <typename T>
        static T GetINISettingAddr(const char* name)
        {
            Setting* setting = (*g_iniSettingCollection)->Get(name);
            if (setting) {
                return reinterpret_cast<T>(&setting->data);
            }
            return NULL;
        };

        static Setting* GetINISetting(const char* name)
        {
            return (*g_iniSettingCollection)->Get(name);
        }

        static HMODULE g_moduleHandle;
        static PluginHandle g_pluginHandle;

        static SKSEMessagingInterface* g_messaging;
    protected:
        ISKSE() = default;
    };
}