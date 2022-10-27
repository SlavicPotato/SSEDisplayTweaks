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
		public BSTEventSink<MenuOpenCloseEvent>
	{
	public:
		virtual EventResult ReceiveEvent(
			const MenuOpenCloseEvent*           evn,
			BSTEventSource<MenuOpenCloseEvent>* dispatcher) override;

		static MenuOpenCloseEventHandler* GetSingleton()
		{
			static MenuOpenCloseEventHandler menuEventHandler;
			return &menuEventHandler;
		}
	};

	typedef void (*EventCallback)(Event, void*);
	typedef bool (*MenuEventCallback)(MenuEvent, const MenuOpenCloseEvent*, BSTEventSource<MenuOpenCloseEvent>*);

	template <class E, class C>
	class EventTriggerDescriptor
	{
	public:
		EventTriggerDescriptor(E m_code, C callback) :
			m_code(m_code),
			m_callback(callback)
		{}

		E m_code;
		C m_callback;
	};

	class MenuEventTrack
	{
		using trackSet_t = std::unordered_set<MenuEvent>;

	public:
		MenuEventTrack() = default;
		MenuEventTrack(const trackSet_t& a_set);

		void SetTracked(const trackSet_t& a_set);

		void Track(MenuEvent a_code, bool a_opening);
		void ClearStack();

	protected:
		std::vector<MenuEvent> m_stack;
		bool                   m_tracked[stl::underlying(MenuEvent::Max)];
	};

	class IEvents :
		protected ILog
	{
		friend class MenuEventTrack;
		friend class MenuOpenCloseEventInitializer;

		typedef void (*inihookproc)(void);
		typedef UIStringHolder* (*PopulateUIStringHolder_t)(void*);

		using mstcMap_t = std::unordered_map<BSFixedString, MenuEvent>;

	public:
		static inline constexpr auto ID = DRIVER_ID::EVENTS;

		struct uistr_desc_t
		{
			UIStringHolder::STRING_INDICES index;
			MenuEvent                      event;
		};

		typedef EventTriggerDescriptor<Event, EventCallback>         _EventTriggerDescriptor;
		typedef EventTriggerDescriptor<MenuEvent, MenuEventCallback> _MenuEventCallbackDescriptor;

		static bool Initialize();
		static void RegisterForEvent(Event a_code, EventCallback a_fn);
		static void RegisterForEvent(MenuEvent a_code, MenuEventCallback a_fn);
		static void TriggerEvent(Event a_code, void* a_args = nullptr);
		static void TriggerMenuEventAny(MenuEvent a_code, const MenuOpenCloseEvent* a_evn, BSTEventSource<MenuOpenCloseEvent>* a_dispatcher);

		void TriggerMenuEventImpl(MenuEvent a_triggercode, MenuEvent a_code, const MenuOpenCloseEvent* a_evn, BSTEventSource<MenuOpenCloseEvent>* a_dispatcher);

		static MenuEvent GetMenuEventCode(const BSFixedString& a_str);

		FN_NAMEPROC("Events");
		FN_ESSENTIAL(true);
		FN_DRVDEF(0);

	private:
		IEvents() = default;

		static void            PostLoadPluginINI_Hook();
		static void            PostLoadPluginINI_AE_Hook(void* a_unk);
		static UIStringHolder* PopulateUIStringHolder_Hook(void* a_dest);

		static void MessageHandler(SKSEMessagingInterface::Message* a_message);

		void CreateMSTCMap();

		std::unordered_map<Event, std::vector<_EventTriggerDescriptor>>          m_events;
		std::unordered_map<MenuEvent, std::vector<_MenuEventCallbackDescriptor>> m_menu_events;

		decltype(&PostLoadPluginINI_Hook)    LoadPluginINI_O;
		decltype(&PostLoadPluginINI_AE_Hook) LoadPluginINI_AE_O;

		PopulateUIStringHolder_t PopulateUIStringHolder_O;

		inline static const auto LoadPluginINI_C          = IAL::Addr(AID::Init0, 36547, Offsets::LoadPluginINI_C, IAL::ver() >= VER_1_6_342 ? IAL::ver() >= VER_1_6_629 ? 0xAB1 : 0xA91 : 0xA71);
		inline static const auto PopulateUIStringHolder_C = IAL::Addr(AID::Init0, 36547, Offsets::PopulateUIStringHolder_C, IAL::ver() >= VER_1_6_342 ? IAL::ver() >= VER_1_6_629 ? 0xEC4 : 0xEA4 : 0xE85);

		mstcMap_t m_mstc_map;

		static IEvents m_Instance;
	};
}