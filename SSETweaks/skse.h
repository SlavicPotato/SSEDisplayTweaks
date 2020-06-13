#pragma once

namespace SDT {
    constexpr size_t MAX_TRAMPOLINE_BRANCH = 256;
    constexpr size_t MAX_TRAMPOLINE_CODEGEN = 256;

    class SettingCollectionListEx
    {
    public:
        virtual ~SettingCollectionListEx();

        struct Entry
        {
            Setting* setting;
            Entry* next;
        };

        //	void	** _vtbl;	// 000
        UInt64	pad008[(0x118 - 0x008) >> 3];
        Entry	items;	// 118

        MEMBER_FN_PREFIX(SettingCollectionListEx);
        DEFINE_MEMBER_FN(Get_Internal, Setting*, offset1, const char* name);

        Setting* Addr(const char* name);

    private:
        inline static auto offset1 = IAL::Offset(AID::INISettingsCollection_offset1);
    };
    STATIC_ASSERT(offsetof(SettingCollectionListEx, items) == 0x118);

    class MenuManagerEx :
        public MenuManager
    {
        friend class ISKSE;
    public:
        bool InPausedMenu() {
            return numPauseGame > 0;
        };

        static MenuManagerEx* GetSingleton(void)
        {
            return *MenuManager_ptr;
        }
    private:
        inline static auto MenuManager_ptr = IAL::Addr<MenuManagerEx**>(AID::MenuManager);
    };
    STATIC_ASSERT(sizeof(MenuManagerEx) == 0x1C8);

    class ISKSE
    {
    public:
        static bool Query(const SKSEInterface* skse, PluginInfo* info);
        static bool Initialize(const SKSEInterface* skse);

        template <typename T>
        static T GetINISettingAddr(const char* name)
        {
            Setting* setting = (*iniSettingCollection)->Addr(name);
            if (setting) {
                return reinterpret_cast<T>(&setting->data);
            }
            return NULL;
        };

        static Setting* GetINISetting(const char* name)
        {
            return (*iniSettingCollection)->Addr(name);
        }

        inline static auto iniSettingCollection = IAL::Addr<SettingCollectionListEx**>(AID::INISettingsCollection);

        static HMODULE g_moduleHandle;
        static PluginHandle g_pluginHandle;

        static SKSEMessagingInterface* g_messaging;
    protected:
        ISKSE() = default;
    };
}