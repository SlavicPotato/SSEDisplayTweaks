#include "pch.h"

namespace SDT
{
    IEvents IEvents::m_Instance;

    bool IEvents::Initialize()
    {
        m_Instance.RegisterHook(LoadPluginINI_C, reinterpret_cast<uintptr_t>(PostLoadPluginINI_Hook));

        if (!m_Instance.InstallHooks()) {
            m_Instance.FatalError("Could not install event hooks");
            return false;
        }

        m_Instance.Debug("Installed event hooks");

        ISKSE::g_messaging->RegisterListener(ISKSE::g_pluginHandle, "SKSE", MessageHandler);

        return true;
    }

    void IEvents::RegisterForEvent(Event code, EventCallback a_fn)
    {
        m_Instance.m_events[code].emplace_back(
            _EventTriggerDescriptor(code, a_fn)
        );
    }

    void IEvents::RegisterForEvent(MenuEvent code, MenuEventCallback a_fn)
    {
        m_Instance.m_menu_events[code].emplace_back(
            _MenuEventCallbackDescriptor(code, a_fn)
        );
    }

    void IEvents::TriggerEvent(Event a_code, void* a_args)
    {
        for (const auto & evtd : m_Instance.m_events[a_code]) {
            evtd.m_callback(a_code, a_args);
        }
    }

    void IEvents::TriggerMenuEventAny(MenuEvent a_code, MenuOpenCloseEvent* evn, EventDispatcher<MenuOpenCloseEvent>* dispatcher)
    {
        m_Instance.TriggerMenuEventImpl(MenuEvent::OnAnyMenu, a_code, evn, dispatcher);
    }

    void IEvents::TriggerMenuEventImpl(MenuEvent triggercode, MenuEvent code, MenuOpenCloseEvent* evn, EventDispatcher<MenuOpenCloseEvent>* dispatcher)
    {
        auto it = m_menu_events.find(triggercode);
        if (it == m_menu_events.end()) {
            return;
        }

        auto it2 = it->second.begin();
        while (it2 != it->second.end()) {
            if (it2->m_callback(code, evn, dispatcher)) {
                ++it2;
            }
            else {
                it2 = it->second.erase(it2);
            }
        }
    }

    void IEvents::PostLoadPluginINI_Hook()
    {
        m_Instance.phookLoadPluginINI();
        m_Instance.TriggerEvent(OnConfigLoad);
    }

    void IEvents::MessageHandler(SKSEMessagingInterface::Message* message)
    {
        if (message->type == SKSEMessagingInterface::kMessage_InputLoaded) {
            auto mm = MenuManager::GetSingleton();
            if (mm) {
                auto dispatcher = mm->MenuOpenCloseEventDispatcher();
                dispatcher->AddEventSink(MenuOpenCloseEventInitializer::GetSingleton());
                dispatcher->AddEventSink(MenuOpenCloseEventHandler::GetSingleton());
                m_Instance.Debug("Added menu event sinks");
            }
            else {
                m_Instance.Error("Could not add menu open/close event sinks");
            }
        }

        m_Instance.TriggerEvent(OnMessage, static_cast<void*>(message));
    }

    void IEvents::CreateMSTCMap()
    {
        auto uiStrHolder = UIStringHolder::GetSingleton();

        ASSERT(uiStrHolder != nullptr);

        m_mstc_map[uiStrHolder->loadingMenu.c_str()] = MenuEvent::OnLoadingMenu;
        m_mstc_map[uiStrHolder->mainMenu.c_str()] = MenuEvent::OnMainMenu;
        m_mstc_map[uiStrHolder->console.c_str()] = MenuEvent::OnConsoleMenu;
        m_mstc_map[uiStrHolder->mapMenu.c_str()] = MenuEvent::OnMapMenu;
        m_mstc_map[uiStrHolder->inventoryMenu.c_str()] = MenuEvent::OnInventoryMenu;
        m_mstc_map[uiStrHolder->closeMenu.c_str()] = MenuEvent::OnCloseMenu;
        m_mstc_map[uiStrHolder->favoritesMenu.c_str()] = MenuEvent::OnFavoritesMenu;
        m_mstc_map[uiStrHolder->barterMenu.c_str()] = MenuEvent::OnBarterMenu;
        m_mstc_map[uiStrHolder->bookMenu.c_str()] = MenuEvent::OnBookMenu;
        m_mstc_map[uiStrHolder->craftingMenu.c_str()] = MenuEvent::OnCraftingMenu;
        m_mstc_map[uiStrHolder->creditsMenu.c_str()] = MenuEvent::OnCreditsMenu;
        m_mstc_map[uiStrHolder->dialogueMenu.c_str()] = MenuEvent::OnDialogueMenu;
        m_mstc_map[uiStrHolder->lockpickingMenu.c_str()] = MenuEvent::OnLockpickingMenu;
        m_mstc_map[uiStrHolder->trainingMenu.c_str()] = MenuEvent::OnTrainingMenu;
        m_mstc_map[uiStrHolder->tutorialMenu.c_str()] = MenuEvent::OnTutorialMenu;
        m_mstc_map[uiStrHolder->giftMenu.c_str()] = MenuEvent::OnGiftMenu;
        m_mstc_map[uiStrHolder->statsMenu.c_str()] = MenuEvent::OnStatsMenu;
        m_mstc_map[uiStrHolder->topMenu.c_str()] = MenuEvent::OnTopMenu;
        m_mstc_map[uiStrHolder->hudMenu.c_str()] = MenuEvent::OnHUDMenu;
        m_mstc_map[uiStrHolder->journalMenu.c_str()] = MenuEvent::OnJournalMenu;
        m_mstc_map[uiStrHolder->cursorMenu.c_str()] = MenuEvent::OnCursorMenu;
        m_mstc_map[uiStrHolder->kinectMenu.c_str()] = MenuEvent::OnKinectMenu;
        m_mstc_map[uiStrHolder->levelUpMenu.c_str()] = MenuEvent::OnLevelUpMenu;
        m_mstc_map[uiStrHolder->magicMenu.c_str()] = MenuEvent::OnMagicMenu;
        m_mstc_map[uiStrHolder->mistMenu.c_str()] = MenuEvent::OnMistMenu;
        m_mstc_map[uiStrHolder->raceSexMenu.c_str()] = MenuEvent::OnRaceSexMenu;
        m_mstc_map[uiStrHolder->sleepWaitMenu.c_str()] = MenuEvent::OnSleepWaitMenu;
        m_mstc_map[uiStrHolder->modManagerMenu.c_str()] = MenuEvent::OnModManagerMenu;
        m_mstc_map[uiStrHolder->quantityMenu.c_str()] = MenuEvent::OnQuantityMenu;
        m_mstc_map[uiStrHolder->refreshMenu.c_str()] = MenuEvent::OnRefreshMenu;
        m_mstc_map[uiStrHolder->faderMenu.c_str()] = MenuEvent::OnFaderMenu;
        m_mstc_map[uiStrHolder->loadWaitSpinner.c_str()] = MenuEvent::OnLoadWaitSpinner;
        m_mstc_map[uiStrHolder->streamingInstallMenu.c_str()] = MenuEvent::OnStreamingInstallMenu;
        m_mstc_map[uiStrHolder->debugTextMenu.c_str()] = MenuEvent::OnDebugTextMenu;
        m_mstc_map[uiStrHolder->tweenMenu.c_str()] = MenuEvent::OnTweenMenu;
        m_mstc_map[uiStrHolder->overlayMenu.c_str()] = MenuEvent::OnOverlayMenu;
        m_mstc_map[uiStrHolder->overlayInteractionMenu.c_str()] = MenuEvent::OnOverlayInteractionMenu;
        m_mstc_map[uiStrHolder->titleSequenceMenu.c_str()] = MenuEvent::OnTitleSequenceMenu;
        m_mstc_map[uiStrHolder->consoleNativeUIMenu.c_str()] = MenuEvent::OnConsoleNativeUIMenu;
        m_mstc_map[uiStrHolder->containerMenu.c_str()] = MenuEvent::OnContainerMenu;
        m_mstc_map[uiStrHolder->messageBoxMenu.c_str()] = MenuEvent::OnMessageBoxMenu;
        m_mstc_map["CustomMenu"] = MenuEvent::OnCustomMenu;
    }

    auto MenuOpenCloseEventHandler::ReceiveEvent(MenuOpenCloseEvent* evn, EventDispatcher<MenuOpenCloseEvent>* dispatcher)
        -> EventResult
    {
        if (evn) {
            MenuEvent code = IEvents::GetMenuEventCode(evn->menuName.c_str());

            //_DMESSAGE(">>>>>>>>>>>> %d   [%s]  opening:%d | %d  ", code, evn->menuName.c_str(), evn->opening, MenuManager::GetSingleton()->InPausedMenu());

            IEvents::TriggerMenuEventAny(code, evn, dispatcher);
        }

        return EventResult::kEvent_Continue;
    }

    auto MenuOpenCloseEventInitializer::ReceiveEvent(MenuOpenCloseEvent* evn, EventDispatcher<MenuOpenCloseEvent>* dispatcher)
        -> EventResult
    {
        IEvents::m_Instance.CreateMSTCMap();
        dispatcher->RemoveEventSink(this);
        return EventResult::kEvent_Continue;
    }

    void MenuEventTrack::SetTracked(const trackSet_t& map)
    {
        m_tracked = map;
    };
    
    void MenuEventTrack::SetTracked(trackSet_t&& map)
    {
        m_tracked = std::forward<trackSet_t>(map);
    };

    void MenuEventTrack::AddTracked(MenuEvent code)
    {
        m_tracked.insert(code);
    }

    void MenuEventTrack::RemoveTracked(MenuEvent code)
    {
        m_tracked.erase(code);
    }

    void MenuEventTrack::Track(MenuEvent code, bool opening)
    {
        if (!m_tracked.contains(code))
            return;

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

    void MenuEventTrack::ClearTracked()
    {
        if (m_stack.size()) {
            m_stack.clear();
        }
    }

    bool MenuEventTrack::IsTracking()
    {
        return m_stack.size() != 0;
    }


}

