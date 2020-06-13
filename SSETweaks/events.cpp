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

        auto dispatcher = reinterpret_cast<EDE_MenuOpenCloseEvent*>(MenuManagerEx::GetSingleton()->MenuOpenCloseEventDispatcher());
        if (dispatcher) {
            dispatcher->AddEventSink(MenuOpenCloseEventInitializer::GetSingleton());
            dispatcher->AddEventSink(MenuOpenCloseEventHandler::GetSingleton());
        }
        else {
            return false;
        }

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

    void IEvents::TriggerMenuEventAny(MenuEvent m_code, MenuOpenCloseEvent* evn, EDE_MenuOpenCloseEvent* dispatcher)
    {
        m_Instance._TriggerMenuEvent(MenuEvent::OnAnyMenu, m_code, evn, dispatcher);
    }

    void IEvents::_TriggerMenuEvent(MenuEvent triggercode, MenuEvent code, MenuOpenCloseEvent* evn, EDE_MenuOpenCloseEvent* dispatcher)
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
        m_Instance.TriggerEvent(OnMessage, reinterpret_cast<void*>(message));
    }

    void IEvents::CreateMSTCMap()
    {
        //_DMESSAGE("> %s   |   %s", __FUNCTION__, (*g_UIStringHolder)->console.c_str());

        m_mstc_map[(*g_UIStringHolder)->loadingMenu.c_str()] = MenuEvent::OnLoadingMenu;
        m_mstc_map[(*g_UIStringHolder)->mainMenu.c_str()] = MenuEvent::OnMainMenu;
        m_mstc_map[(*g_UIStringHolder)->console.c_str()] = MenuEvent::OnConsoleMenu;
        m_mstc_map[(*g_UIStringHolder)->mapMenu.c_str()] = MenuEvent::OnMapMenu;
        m_mstc_map[(*g_UIStringHolder)->inventoryMenu.c_str()] = MenuEvent::OnInventoryMenu;
        m_mstc_map[(*g_UIStringHolder)->closeMenu.c_str()] = MenuEvent::OnCloseMenu;
        m_mstc_map[(*g_UIStringHolder)->favoritesMenu.c_str()] = MenuEvent::OnFavoritesMenu;
        m_mstc_map[(*g_UIStringHolder)->barterMenu.c_str()] = MenuEvent::OnBarterMenu;
        m_mstc_map[(*g_UIStringHolder)->bookMenu.c_str()] = MenuEvent::OnBookMenu;
        m_mstc_map[(*g_UIStringHolder)->craftingMenu.c_str()] = MenuEvent::OnCraftingMenu;
        m_mstc_map[(*g_UIStringHolder)->creditsMenu.c_str()] = MenuEvent::OnCreditsMenu;
        m_mstc_map[(*g_UIStringHolder)->dialogueMenu.c_str()] = MenuEvent::OnDialogueMenu;
        m_mstc_map[(*g_UIStringHolder)->lockpickingMenu.c_str()] = MenuEvent::OnLockpickingMenu;
        m_mstc_map[(*g_UIStringHolder)->trainingMenu.c_str()] = MenuEvent::OnTrainingMenu;
        m_mstc_map[(*g_UIStringHolder)->tutorialMenu.c_str()] = MenuEvent::OnTutorialMenu;
        m_mstc_map[(*g_UIStringHolder)->giftMenu.c_str()] = MenuEvent::OnGiftMenu;
        m_mstc_map[(*g_UIStringHolder)->statsMenu.c_str()] = MenuEvent::OnStatsMenu;
        m_mstc_map[(*g_UIStringHolder)->topMenu.c_str()] = MenuEvent::OnTopMenu;
        m_mstc_map[(*g_UIStringHolder)->hudMenu.c_str()] = MenuEvent::OnHUDMenu;
        m_mstc_map[(*g_UIStringHolder)->journalMenu.c_str()] = MenuEvent::OnJournalMenu;
        m_mstc_map[(*g_UIStringHolder)->cursorMenu.c_str()] = MenuEvent::OnCursorMenu;
        m_mstc_map[(*g_UIStringHolder)->kinectMenu.c_str()] = MenuEvent::OnKinectMenu;
        m_mstc_map[(*g_UIStringHolder)->levelUpMenu.c_str()] = MenuEvent::OnLevelUpMenu;
        m_mstc_map[(*g_UIStringHolder)->magicMenu.c_str()] = MenuEvent::OnMagicMenu;
        m_mstc_map[(*g_UIStringHolder)->mistMenu.c_str()] = MenuEvent::OnMistMenu;
        m_mstc_map[(*g_UIStringHolder)->raceSexMenu.c_str()] = MenuEvent::OnRaceSexMenu;
        m_mstc_map[(*g_UIStringHolder)->sleepWaitMenu.c_str()] = MenuEvent::OnSleepWaitMenu;
        m_mstc_map[(*g_UIStringHolder)->modManagerMenu.c_str()] = MenuEvent::OnModManagerMenu;
        m_mstc_map[(*g_UIStringHolder)->quantityMenu.c_str()] = MenuEvent::OnQuantityMenu;
        m_mstc_map[(*g_UIStringHolder)->refreshMenu.c_str()] = MenuEvent::OnRefreshMenu;
        m_mstc_map[(*g_UIStringHolder)->faderMenu.c_str()] = MenuEvent::OnFaderMenu;
        m_mstc_map[(*g_UIStringHolder)->loadWaitSpinner.c_str()] = MenuEvent::OnLoadWaitSpinner;
        m_mstc_map[(*g_UIStringHolder)->streamingInstallMenu.c_str()] = MenuEvent::OnStreamingInstallMenu;
        m_mstc_map[(*g_UIStringHolder)->debugTextMenu.c_str()] = MenuEvent::OnDebugTextMenu;
        m_mstc_map[(*g_UIStringHolder)->tweenMenu.c_str()] = MenuEvent::OnTweenMenu;
        m_mstc_map[(*g_UIStringHolder)->overlayMenu.c_str()] = MenuEvent::OnOverlayMenu;
        m_mstc_map[(*g_UIStringHolder)->overlayInteractionMenu.c_str()] = MenuEvent::OnOverlayInteractionMenu;
        m_mstc_map[(*g_UIStringHolder)->titleSequenceMenu.c_str()] = MenuEvent::OnTitleSequenceMenu;
        m_mstc_map[(*g_UIStringHolder)->consoleNativeUIMenu.c_str()] = MenuEvent::OnConsoleNativeUIMenu;
        m_mstc_map[(*g_UIStringHolder)->containerMenu.c_str()] = MenuEvent::OnContainerMenu;
        m_mstc_map[(*g_UIStringHolder)->messageBoxMenu.c_str()] = MenuEvent::OnMessageBoxMenu;
        m_mstc_map["CustomMenu"] = MenuEvent::OnCustomMenu;
    }

    auto MenuOpenCloseEventHandler::ReceiveEvent(MenuOpenCloseEvent* evn, EventDispatcherEx<MenuOpenCloseEvent>* dispatcher)
        -> EventResult
    {
        if (evn) {
            MenuEvent code = IEvents::GetMenuEventCode(evn->menuName.c_str());

            //_DMESSAGE(">>>>>>>>>>>> %d   [%s]  opening:%d | %d  ", code, evn->menuName.c_str(), evn->opening, MenuManagerEx::GetSingleton()->InPausedMenu());

            IEvents::TriggerMenuEventAny(code, evn, dispatcher);
            //IEvents::TriggerMenuEvent(code, evn, reinterpret_cast<EDE_MenuOpenCloseEvent*>(dispatcher));
        }

        return EventResult::kEvent_Continue;
    }

    auto MenuOpenCloseEventInitializer::ReceiveEvent(MenuOpenCloseEvent* evn, EventDispatcherEx<MenuOpenCloseEvent>* dispatcher)
        -> EventResult
    {
        IEvents::m_Instance.CreateMSTCMap();
        dispatcher->RemoveEventSink(this);
        return EventResult::kEvent_Continue;
    }


    /*void MenuEventTrack::Dump()
    {
        _DMESSAGE("\n(");
        for (const auto& m : m_stack)
        {
            bool found = false;
            for (auto it = IEvents::m_Instance.m_mstc_map.begin(); it != IEvents::m_Instance.m_mstc_map.end(); ++it)
                if (it->second == m) {
                    _DMESSAGE("\t %s", it->first.c_str());
                    found = true;
                    break;
                }

            if (!found) {
                _DMESSAGE("\t %d", m);
            }
        }
        _DMESSAGE(")\n");
    }*/

}

