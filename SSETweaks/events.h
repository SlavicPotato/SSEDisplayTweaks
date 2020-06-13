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

    template <typename T> class BSTEventSinkEx;

    template <typename EventT, typename EventArgT = EventT>
    class EventDispatcherEx
    {
        typedef BSTEventSinkEx<EventT> SinkT;

        tArray<SinkT*>		eventSinks;			// 000
        tArray<SinkT*>		addBuffer;			// 018 - schedule for add
        tArray<SinkT*>		removeBuffer;		// 030 - schedule for remove
        SimpleLock			lock;				// 048
        bool				stateFlag;			// 050 - some internal state changed while sending
        char				pad[7];				// 051

        // Note: in SE there are multiple identical copies of all these functions 
        MEMBER_FN_PREFIX(EventDispatcherEx);
        // 66B1C7AC473D5EA48E4FD620BBFE0A06392C5885+66
        DEFINE_MEMBER_FN(AddEventSink_Internal, void, EventDispatcherEx_offset1, SinkT* eventSink);
        // ??_7BGSProcedureShoutExecState@@6B@ dtor | +43
        DEFINE_MEMBER_FN(RemoveEventSink_Internal, void, EventDispatcherEx_offset2, SinkT* eventSink);
        // D6BA7CEC95B2C2B9C593A9AEE7F0ADFFB2C10E11+456
        DEFINE_MEMBER_FN(SendEvent_Internal, void, EventDispatcherEx_offset3, EventArgT* evn);

    public:

        EventDispatcherEx() : stateFlag(false) {}

        void AddEventSink(SinkT* eventSink) { CALL_MEMBER_FN(this, AddEventSink_Internal)(eventSink); }
        void RemoveEventSink(SinkT* eventSink) { CALL_MEMBER_FN(this, RemoveEventSink_Internal)(eventSink); }
        void SendEvent(EventArgT* evn) { CALL_MEMBER_FN(this, SendEvent_Internal)(evn); }
    private:

        inline static auto EventDispatcherEx_offset1 = IAL::Offset(AID::EventDispatcher_offset1);
        inline static auto EventDispatcherEx_offset2 = IAL::Offset(AID::EventDispatcher_offset2);
        inline static auto EventDispatcherEx_offset3 = IAL::Offset(AID::EventDispatcher_offset3);
    };
    STATIC_ASSERT(sizeof(EventDispatcherEx<void*>) == 0x58);

    // 08 
    template <typename T>
    class BSTEventSinkEx
    {
    public:
        virtual ~BSTEventSinkEx() { };
        virtual	EventResult	ReceiveEvent(T* evn, EventDispatcherEx<T>* dispatcher) { return kEvent_Continue; }; // pure
    //	void	** _vtbl;	// 00
    };

    class MenuOpenCloseEventInitializer : 
        public BSTEventSinkEx <MenuOpenCloseEvent>
    {
    public:
        virtual EventResult	ReceiveEvent(MenuOpenCloseEvent* evn, EventDispatcherEx<MenuOpenCloseEvent>* dispatcher) override;

        static MenuOpenCloseEventInitializer* GetSingleton()
        {
            static MenuOpenCloseEventInitializer menuEventInitializer;
            return &menuEventInitializer;
        }
    };

    class MenuOpenCloseEventHandler :
        public BSTEventSinkEx <MenuOpenCloseEvent>
    {
    public:
        virtual EventResult	ReceiveEvent(MenuOpenCloseEvent* evn, EventDispatcherEx<MenuOpenCloseEvent>* dispatcher) override;

        static MenuOpenCloseEventHandler* GetSingleton()
        {
            static MenuOpenCloseEventHandler menuEventHandler;
            return &menuEventHandler;
        }
    };

    typedef EventDispatcherEx<MenuOpenCloseEvent> EDE_MenuOpenCloseEvent;
    typedef void (*EventCallback)(Event, void*);
    typedef bool (*MenuEventCallback)(MenuEvent, MenuOpenCloseEvent*, EDE_MenuOpenCloseEvent*);

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

        void SetTracked(TrackMap& map)
        {
            m_tracked = map;
        };

        void AddTracked(MenuEvent code)
        {
            m_tracked[code] = true;
        }

        void RemoveTracked(MenuEvent code)
        {
            m_tracked.erase(code);
        }

        //void Dump();

        void Track(MenuEvent code, bool opening)
        {
            if (m_tracked.find(code) == m_tracked.end()) {
                return;
            }

            if (opening) {
                if (std::find(m_stack.begin(), m_stack.end(), code) == m_stack.end()) {
                    m_stack.push_back(code);
                }
            }
            else {
                auto it = std::find(m_stack.begin(), m_stack.end(), code);
                if (it != m_stack.end()) {
                    m_stack.erase(it);
                }
            }
        }

        void ClearTracked()
        {
            if (m_stack.size()) {
                m_stack.clear();
            }
        }

        bool IsTracking()
        {
            return m_stack.size() > 0;
        }

    protected:

        std::vector<MenuEvent> m_stack;
        TrackMap m_tracked;
    };

    class IEvents :
        protected IHook
    {
        friend class MenuEventTrack;
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
        static void TriggerMenuEventAny(MenuEvent m_code, MenuOpenCloseEvent* evn, EDE_MenuOpenCloseEvent* dispatcher);

        void _TriggerMenuEvent(MenuEvent triggercode, MenuEvent code, MenuOpenCloseEvent* evn, EDE_MenuOpenCloseEvent* dispatcher);

        static __inline MenuEvent GetMenuEventCode(const char* str)
        {
            return m_Instance.m_mstc_map[str];
        }

        friend class MenuOpenCloseEventInitializer;

        FN_NAMEPROC("Events")
        FN_ESSENTIAL(true)
        FN_PRIO(0)
        FN_DRVID(DRIVER_EVENTS)
    private:
        IEvents() = default;
        //void TriggerEvent(Event m_code, void* args);

        static void hookLoadPluginINI();
        static void MessageHandler(SKSEMessagingInterface::Message* message);

        void CreateMSTCMap();

        std::map<Event, std::vector<_EventTriggerDescriptor>> m_events;
        std::map<MenuEvent, std::vector<_MenuEventCallbackDescriptor>> m_menu_events;

        inline static auto phookLoadPluginINI = IAL::Addr<inihookproc>(AID::LoadPluginINI);
        inline static auto g_UIStringHolder = IAL::Addr<UIStringHolder**>(AID::UIStringHolder);
        inline static auto LoadPluginINI_C = IAL::Addr(AID::Init0, Offsets::LoadPluginINI_C);

        MenuNameToCodeMap m_mstc_map;

        static IEvents m_Instance;
    };




}