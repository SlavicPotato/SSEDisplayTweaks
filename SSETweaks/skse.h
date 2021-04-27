#pragma once

namespace SDT
{
    static inline constexpr std::size_t MAX_TRAMPOLINE_BRANCH = 256;
    static inline constexpr std::size_t MAX_TRAMPOLINE_CODEGEN = 580;

    class ISKSE
    {
    public:
        static bool Query(const SKSEInterface* skse, PluginInfo* info);
        static bool Initialize(const SKSEInterface* skse);

        template <typename T>
        [[nodiscard]] static auto GetINISettingAddr(const char* name)
        {
            using type = std::remove_all_extents_t<T>;

            Setting* setting = (*g_iniSettingCollection)->Get(name);
            if (setting) {
                return reinterpret_cast<type*>(&setting->data);
            }

            return static_cast<type*>(nullptr);
        };

        template <typename T>
        [[nodiscard]] static auto GetINIPrefSettingAddr(const char* name)
        {
            using type = std::remove_all_extents_t<T>;

            Setting* setting = (*g_iniPrefSettingCollection)->Get(name);
            if (setting) {
                return reinterpret_cast<type*>(&setting->data);
            }

            return static_cast<type*>(nullptr);
        };

        static HMODULE moduleHandle;
        static PluginHandle pluginHandle;

        static SKSEMessagingInterface* messaging;

        static std::size_t branchTrampolineSize;
        static std::size_t localTrampolineSize;
    protected:
        ISKSE() = default;
    };
}