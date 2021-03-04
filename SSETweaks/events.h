#pragma once

namespace SDT
{
    enum class Event : std::uint32_t
    {
        OnConfigLoad,
        OnD3D11PreCreate,
        OnD3D11PostCreate,
        OnD3D11PostPostCreate,
        OnCreateWindowEx,
        OnMenuEvent,
        OnMessage,
        OnGameSave,
        OnGameLoad,
        OnFormDelete,
        OnRevert,
        OnLogMessage,
        OnGameShutdown,
        OnExit
    };

    enum class MenuEvent : std::uint32_t
    {
        OnAnyMenu,
        OnUnknownMenu,
        OnLoadingMenu,
        OnMainMenu,
        OnConsoleMenu,
        OnMapMenu,
        OnInventoryMenu,
        OnCloseMenu,
        OnFavoritesMenu,
        OnBarterMenu,
        OnBookMenu,
        OnCraftingMenu,
        OnCreditsMenu,
        OnDialogueMenu,
        OnLockpickingMenu,
        OnTrainingMenu,
        OnTutorialMenu,
        OnGiftMenu,
        OnStatsMenu,
        OnTopMenu,
        OnHUDMenu,
        OnJournalMenu,
        OnCursorMenu,
        OnKinectMenu,
        OnLevelUpMenu,
        OnMagicMenu,
        OnMistMenu,
        OnRaceSexMenu,
        OnSleepWaitMenu,
        OnModManagerMenu,
        OnQuantityMenu,
        OnRefreshMenu,
        OnFaderMenu,
        OnLoadWaitSpinner,
        OnStreamingInstallMenu,
        OnDebugTextMenu,
        OnTweenMenu,
        OnOverlayMenu,
        OnOverlayInteractionMenu,
        OnTitleSequenceMenu,
        OnConsoleNativeUIMenu,
        OnContainerMenu,
        OnMessageBoxMenu,
        OnCustomMenu,
        Max
    };

    class MenuOpenCloseEventHandler :
        public BSTEventSink <MenuOpenCloseEvent>
    {
    public:
        virtual EventResult	ReceiveEvent(MenuOpenCloseEvent* evn, EventDispatcher<MenuOpenCloseEvent>* dispatcher) override;

        static MenuOpenCloseEventHandler* GetSingleton()
        {
            static MenuOpenCloseEventHandler menuEventHandler;
            return &menuEventHandler;
        }
    };

    typedef void (*EventCallback)(Event, void*);
    typedef bool (*MenuEventCallback)(MenuEvent, MenuOpenCloseEvent*, EventDispatcher<MenuOpenCloseEvent>*);

    template <typename E, typename C>
    class EventTriggerDescriptor
    {
    public:
        EventTriggerDescriptor(E m_code, C callback) :
            m_code(m_code), m_callback(callback)
        {}

        E m_code;
        C m_callback;
    };

    class MenuEventTrack
    {
        using trackSet_t = stl::unordered_set<MenuEvent>;

    public:

        MenuEventTrack() = default;
        MenuEventTrack(const trackSet_t& a_set);

        void SetTracked(const trackSet_t& a_set);

        void Track(MenuEvent a_code, bool a_opening);
        void ClearStack();

    protected:

        std::vector<MenuEvent> m_stack;
        bool m_tracked[Enum::Underlying(MenuEvent::Max)];
    };


    class IEvents :
        protected IHook
    {
        friend class MenuEventTrack;
        friend class MenuOpenCloseEventInitializer;

        typedef void(*inihookproc) (void);
        typedef UIStringHolder* (*PopulateUIStringHolder_t) (UIStringHolder*);

        using mstcMap_t = std::unordered_map<const char*, MenuEvent>;
    public:

        static inline constexpr auto ID = DRIVER_ID::EVENTS;

        struct uistr_desc_t
        {
            UIStringHolder::STRING_INDICES index;
            MenuEvent event;
        };

        typedef stl::unordered_map<std::string, MenuEvent> MenuNameToCodeMap;

        typedef EventTriggerDescriptor<Event, EventCallback> _EventTriggerDescriptor;
        typedef EventTriggerDescriptor<MenuEvent, MenuEventCallback> _MenuEventCallbackDescriptor;

        static bool Initialize();
        static void RegisterForEvent(Event a_code, EventCallback a_fn);
        static void RegisterForEvent(MenuEvent a_code, MenuEventCallback a_fn);
        static void TriggerEvent(Event a_code, void* a_args = nullptr);
        static void TriggerMenuEventAny(MenuEvent a_code, MenuOpenCloseEvent* a_evn, EventDispatcher<MenuOpenCloseEvent>* a_dispatcher);

        void TriggerMenuEventImpl(MenuEvent a_triggercode, MenuEvent a_code, MenuOpenCloseEvent* a_evn, EventDispatcher<MenuOpenCloseEvent>* a_dispatcher);

        static MenuEvent GetMenuEventCode(const BSFixedString& a_str);

        FN_NAMEPROC("Events");
        FN_ESSENTIAL(true);
        FN_DRVDEF(0);
    private:
        IEvents() = default;

        static void PostLoadPluginINI_Hook();
        static UIStringHolder* PopulateUIStringHolder_Hook(UIStringHolder* a_dest);

        static void MessageHandler(SKSEMessagingInterface::Message* a_message);

        void CreateMSTCMap(UIStringHolder* a_holder);

        std::unordered_map<Event, std::vector<_EventTriggerDescriptor>> m_events;
        std::unordered_map<MenuEvent, std::vector<_MenuEventCallbackDescriptor>> m_menu_events;

        inihookproc LoadPluginINI_O;
        PopulateUIStringHolder_t PopulateUIStringHolder_O;

        inline static auto LoadPluginINI_C = IAL::Addr(AID::Init0, Offsets::LoadPluginINI_C);
        inline static auto PopulateUIStringHolder_C = IAL::Addr(AID::Init0, Offsets::PopulateUIStringHolder);

        MenuNameToCodeMap m_mstc_map;

        static IEvents m_Instance;
    };
}