#include "pch.h"

namespace SDT
{
    IEvents IEvents::m_Instance;

    bool IEvents::Initialize()
    {
        m_Instance.RegisterHook(LoadPluginINI_C, reinterpret_cast<uintptr_t>(hookLoadPluginINI));

        if (!m_Instance.InstallHooks()) {
            m_Instance.FatalError("Could not install event hooks");
            return false;
        }

        m_Instance.Message("Installed event hooks");

        ISKSE::g_messaging->RegisterListener(ISKSE::g_pluginHandle, "SKSE", MessageHandler);

        return true;
    }

    void IEvents::RegisterForEvent(Event code, EventCallback fn)
    {
        m_Instance.m_events[code].push_back(
            _EventTriggerDescriptor(code, fn)
        );
    }

    void IEvents::RegisterForEvent(MenuEvent code, MenuEventCallback fn)
    {
        m_Instance.m_menu_events[code].push_back(
            _MenuEventCallbackDescriptor(code, fn)
        );
    }

    void IEvents::TriggerEvent(Event m_code, void* args)
    {
        for (auto const& evtd : m_Instance.m_events[m_code]) {
            evtd.m_callback(m_code, args);
        }
    }

    /*void IEvents::TriggerMenuEvent(MenuEvent m_code, MenuOpenCloseEvent* evn, EDE_MenuOpenCloseEvent* dispatcher)
    {
        m_Instance._TriggerMenuEvent(m_code, m_code, evn, dispatcher);
    }*/

    void IEvents::TriggerMenuEventAny(MenuEvent m_code, MenuOpenCloseEvent* evn, EventDispatcher<MenuOpenCloseEvent>* dispatcher)
    {
        m_Instance._TriggerMenuEvent(MenuEvent::OnAnyMenu, m_code, evn, dispatcher);
    }

    void IEvents::_TriggerMenuEvent(MenuEvent triggercode, MenuEvent code, MenuOpenCloseEvent* evn, EventDispatcher<MenuOpenCloseEvent>* dispatcher)
    {
        auto& tl = m_menu_events[triggercode];
        auto it = tl.begin();
        while (it != tl.end()) {
            auto& evtd = *it;
            if (evtd.m_callback(code, evn, dispatcher)) {
                ++it;
            }
            else {
                it = tl.erase(it);
            }
        }
    }

    void IEvents::hookLoadPluginINI()
    {
        m_Instance.phookLoadPluginINI();
        m_Instance.TriggerEvent(OnConfigLoad, nullptr);
    }

    void IEvents::MessageHandler(SKSEMessagingInterface::Message* message)
    {
        if (message->type == SKSEMessagingInterface::kMessage_InputLoaded) {
            auto mm = MenuManager::GetSingleton();
            if (mm) {
                auto dispatcher = mm->MenuOpenCloseEventDispatcher();
                dispatcher->AddEventSink(MenuOpenCloseEventInitializer::GetSingleton());
                dispatcher->AddEventSink(MenuOpenCloseEventHandler::GetSingleton());
            }
            else {
                m_Instance.Error("Could not add menu open/close event sinks");
            }
        }

        m_Instance.TriggerEvent(OnMessage, reinterpret_cast<void*>(message));
    }

    void IEvents::CreateMSTCMap()
    {
        auto UIStringHolder_ptr = UIStringHolder::GetSingleton();

        m_mstc_map[UIStringHolder_ptr->loadingMenu.c_str()] = MenuEvent::OnLoadingMenu;
        m_mstc_map[UIStringHolder_ptr->mainMenu.c_str()] = MenuEvent::OnMainMenu;
        m_mstc_map[UIStringHolder_ptr->console.c_str()] = MenuEvent::OnConsoleMenu;
        m_mstc_map[UIStringHolder_ptr->mapMenu.c_str()] = MenuEvent::OnMapMenu;
        m_mstc_map[UIStringHolder_ptr->inventoryMenu.c_str()] = MenuEvent::OnInventoryMenu;
        m_mstc_map[UIStringHolder_ptr->closeMenu.c_str()] = MenuEvent::OnCloseMenu;
        m_mstc_map[UIStringHolder_ptr->favoritesMenu.c_str()] = MenuEvent::OnFavoritesMenu;
        m_mstc_map[UIStringHolder_ptr->barterMenu.c_str()] = MenuEvent::OnBarterMenu;
        m_mstc_map[UIStringHolder_ptr->bookMenu.c_str()] = MenuEvent::OnBookMenu;
        m_mstc_map[UIStringHolder_ptr->craftingMenu.c_str()] = MenuEvent::OnCraftingMenu;
        m_mstc_map[UIStringHolder_ptr->creditsMenu.c_str()] = MenuEvent::OnCreditsMenu;
        m_mstc_map[UIStringHolder_ptr->dialogueMenu.c_str()] = MenuEvent::OnDialogueMenu;
        m_mstc_map[UIStringHolder_ptr->lockpickingMenu.c_str()] = MenuEvent::OnLockpickingMenu;
        m_mstc_map[UIStringHolder_ptr->trainingMenu.c_str()] = MenuEvent::OnTrainingMenu;
        m_mstc_map[UIStringHolder_ptr->tutorialMenu.c_str()] = MenuEvent::OnTutorialMenu;
        m_mstc_map[UIStringHolder_ptr->giftMenu.c_str()] = MenuEvent::OnGiftMenu;
        m_mstc_map[UIStringHolder_ptr->statsMenu.c_str()] = MenuEvent::OnStatsMenu;
        m_mstc_map[UIStringHolder_ptr->topMenu.c_str()] = MenuEvent::OnTopMenu;
        m_mstc_map[UIStringHolder_ptr->hudMenu.c_str()] = MenuEvent::OnHUDMenu;
        m_mstc_map[UIStringHolder_ptr->journalMenu.c_str()] = MenuEvent::OnJournalMenu;
        m_mstc_map[UIStringHolder_ptr->cursorMenu.c_str()] = MenuEvent::OnCursorMenu;
        m_mstc_map[UIStringHolder_ptr->kinectMenu.c_str()] = MenuEvent::OnKinectMenu;
        m_mstc_map[UIStringHolder_ptr->levelUpMenu.c_str()] = MenuEvent::OnLevelUpMenu;
        m_mstc_map[UIStringHolder_ptr->magicMenu.c_str()] = MenuEvent::OnMagicMenu;
        m_mstc_map[UIStringHolder_ptr->mistMenu.c_str()] = MenuEvent::OnMistMenu;
        m_mstc_map[UIStringHolder_ptr->raceSexMenu.c_str()] = MenuEvent::OnRaceSexMenu;
        m_mstc_map[UIStringHolder_ptr->sleepWaitMenu.c_str()] = MenuEvent::OnSleepWaitMenu;
        m_mstc_map[UIStringHolder_ptr->modManagerMenu.c_str()] = MenuEvent::OnModManagerMenu;
        m_mstc_map[UIStringHolder_ptr->quantityMenu.c_str()] = MenuEvent::OnQuantityMenu;
        m_mstc_map[UIStringHolder_ptr->refreshMenu.c_str()] = MenuEvent::OnRefreshMenu;
        m_mstc_map[UIStringHolder_ptr->faderMenu.c_str()] = MenuEvent::OnFaderMenu;
        m_mstc_map[UIStringHolder_ptr->loadWaitSpinner.c_str()] = MenuEvent::OnLoadWaitSpinner;
        m_mstc_map[UIStringHolder_ptr->streamingInstallMenu.c_str()] = MenuEvent::OnStreamingInstallMenu;
        m_mstc_map[UIStringHolder_ptr->debugTextMenu.c_str()] = MenuEvent::OnDebugTextMenu;
        m_mstc_map[UIStringHolder_ptr->tweenMenu.c_str()] = MenuEvent::OnTweenMenu;
        m_mstc_map[UIStringHolder_ptr->overlayMenu.c_str()] = MenuEvent::OnOverlayMenu;
        m_mstc_map[UIStringHolder_ptr->overlayInteractionMenu.c_str()] = MenuEvent::OnOverlayInteractionMenu;
        m_mstc_map[UIStringHolder_ptr->titleSequenceMenu.c_str()] = MenuEvent::OnTitleSequenceMenu;
        m_mstc_map[UIStringHolder_ptr->consoleNativeUIMenu.c_str()] = MenuEvent::OnConsoleNativeUIMenu;
        m_mstc_map[UIStringHolder_ptr->containerMenu.c_str()] = MenuEvent::OnContainerMenu;
        m_mstc_map[UIStringHolder_ptr->messageBoxMenu.c_str()] = MenuEvent::OnMessageBoxMenu;
        m_mstc_map["CustomMenu"] = MenuEvent::OnCustomMenu;
    }

    auto MenuOpenCloseEventHandler::ReceiveEvent(MenuOpenCloseEvent* evn, EventDispatcher<MenuOpenCloseEvent>* dispatcher)
        -> EventResult
    {
        if (evn) {
            MenuEvent code = IEvents::GetMenuEventCode(evn->menuName.c_str());

            //_DMESSAGE(">>>>>>>>>>>> %d   [%s]  opening:%d | %d  ", code, evn->menuName.c_str(), evn->opening, MenuManagerEx::GetSingleton()->InPausedMenu());

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
}

