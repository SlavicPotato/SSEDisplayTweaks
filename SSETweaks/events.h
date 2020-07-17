#pragma once

namespace SDT
{
    enum Event {
        OnConfigLoad = 1,
        OnD3D11PreCreate,
        OnD3D11PostCreate,
        OnD3D11PostPostCreate,
        OnCreateWindowEx,
        OnMenuEvent,
        OnMessage
    };

    enum MenuEvent {
        OnAnyMenu = -1,
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
        OnCustomMenu
    };
       

    class MenuOpenCloseEventInitializer : 
        public BSTEventSink <MenuOpenCloseEvent>
    {
    public:
        virtual EventResult	ReceiveEvent(MenuOpenCloseEvent* evn, EventDispatcher<MenuOpenCloseEvent>* dispatcher) override;

        static MenuOpenCloseEventInitializer* GetSingleton()
        {
            static MenuOpenCloseEventInitializer menuEventInitializer;
            return &menuEventInitializer;
        }
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

        int m_code;
        C m_callback;
    };

    class MenuEventTrack
    {
    public:
        typedef std::unordered_map<MenuEvent, bool> TrackMap;

        void SetTracked(const TrackMap& map);
        void AddTracked(MenuEvent code);
        void RemoveTracked(MenuEvent code);
        void Track(MenuEvent code, bool opening);
        void ClearTracked();
        bool IsTracking();

    protected:

        std::vector<MenuEvent> m_stack;
        TrackMap m_tracked;
    };

    class IEvents :
        protected IHook
    {
        friend class MenuEventTrack;
        friend class MenuOpenCloseEventInitializer;

        typedef void(*inihookproc) (void);
    public:

        typedef std::unordered_map<std::string, MenuEvent> MenuNameToCodeMap;

        typedef EventTriggerDescriptor<Event, EventCallback> _EventTriggerDescriptor;
        typedef EventTriggerDescriptor<MenuEvent, MenuEventCallback> _MenuEventCallbackDescriptor;

        static bool Initialize();
        static void RegisterForEvent(Event m_code, EventCallback fn);
        static void RegisterForEvent(MenuEvent m_code, MenuEventCallback fn);
        static void TriggerEvent(Event m_code, void* args);
        //static void TriggerMenuEvent(MenuEvent m_code, MenuOpenCloseEvent* evn, EDE_MenuOpenCloseEvent* dispatcher);
        static void TriggerMenuEventAny(MenuEvent m_code, MenuOpenCloseEvent* evn, EventDispatcher<MenuOpenCloseEvent>* dispatcher);

        void _TriggerMenuEvent(MenuEvent triggercode, MenuEvent code, MenuOpenCloseEvent* evn, EventDispatcher<MenuOpenCloseEvent>* dispatcher);

        __forceinline static MenuEvent GetMenuEventCode(const char* str) {
            return m_Instance.m_mstc_map[str];
        }

        FN_NAMEPROC("Events")
        FN_ESSENTIAL(true)
        FN_PRIO(0)
        FN_DRVID(DRIVER_EVENTS)
    private:
        IEvents() = default;
        //void TriggerEvent(Event m_code, void* args);

        static void PostLoadPluginINI_Hook();
        static void MessageHandler(SKSEMessagingInterface::Message* message);

        void CreateMSTCMap();

        std::map<Event, std::vector<_EventTriggerDescriptor>> m_events;
        std::map<MenuEvent, std::vector<_MenuEventCallbackDescriptor>> m_menu_events;

        inline static auto phookLoadPluginINI = IAL::Addr<inihookproc>(AID::LoadPluginINI);
        inline static auto LoadPluginINI_C = IAL::Addr(AID::Init0, Offsets::LoadPluginINI_C);

        MenuNameToCodeMap m_mstc_map;

        static IEvents m_Instance;
    };




}