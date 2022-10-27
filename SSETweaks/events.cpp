#include "pch.h"

namespace SDT
{
	IEvents IEvents::m_Instance;

	static IEvents::uistr_desc_t s_mstc_map_desc[] = {
		{ UIStringHolder::STRING_INDICES::kloadingMenu, MenuEvent::OnLoadingMenu },
		{ UIStringHolder::STRING_INDICES::kmainMenu, MenuEvent::OnMainMenu },
		{ UIStringHolder::STRING_INDICES::kconsole, MenuEvent::OnConsoleMenu },
		{ UIStringHolder::STRING_INDICES::kmapMenu, MenuEvent::OnMapMenu },
		{ UIStringHolder::STRING_INDICES::kinventoryMenu, MenuEvent::OnInventoryMenu },
		{ UIStringHolder::STRING_INDICES::kcloseMenu, MenuEvent::OnCloseMenu },
		{ UIStringHolder::STRING_INDICES::kfavoritesMenu, MenuEvent::OnFavoritesMenu },
		{ UIStringHolder::STRING_INDICES::kbarterMenu, MenuEvent::OnBarterMenu },
		{ UIStringHolder::STRING_INDICES::kbookMenu, MenuEvent::OnBookMenu },
		{ UIStringHolder::STRING_INDICES::kcraftingMenu, MenuEvent::OnCraftingMenu },
		{ UIStringHolder::STRING_INDICES::kcreditsMenu, MenuEvent::OnCreditsMenu },
		{ UIStringHolder::STRING_INDICES::kdialogueMenu, MenuEvent::OnDialogueMenu },
		{ UIStringHolder::STRING_INDICES::klockpickingMenu, MenuEvent::OnLockpickingMenu },
		{ UIStringHolder::STRING_INDICES::ktrainingMenu, MenuEvent::OnTrainingMenu },
		{ UIStringHolder::STRING_INDICES::ktutorialMenu, MenuEvent::OnTutorialMenu },
		{ UIStringHolder::STRING_INDICES::kgiftMenu, MenuEvent::OnGiftMenu },
		{ UIStringHolder::STRING_INDICES::kstatsMenu, MenuEvent::OnStatsMenu },
		{ UIStringHolder::STRING_INDICES::ktopMenu, MenuEvent::OnTopMenu },
		{ UIStringHolder::STRING_INDICES::khudMenu, MenuEvent::OnHUDMenu },
		{ UIStringHolder::STRING_INDICES::kjournalMenu, MenuEvent::OnJournalMenu },
		{ UIStringHolder::STRING_INDICES::kcursorMenu, MenuEvent::OnCursorMenu },
		{ UIStringHolder::STRING_INDICES::kkinectMenu, MenuEvent::OnKinectMenu },
		{ UIStringHolder::STRING_INDICES::klevelUpMenu, MenuEvent::OnLevelUpMenu },
		{ UIStringHolder::STRING_INDICES::kmagicMenu, MenuEvent::OnMagicMenu },
		{ UIStringHolder::STRING_INDICES::kmistMenu, MenuEvent::OnMistMenu },
		{ UIStringHolder::STRING_INDICES::kraceSexMenu, MenuEvent::OnRaceSexMenu },
		{ UIStringHolder::STRING_INDICES::ksleepWaitMenu, MenuEvent::OnSleepWaitMenu },
		{ UIStringHolder::STRING_INDICES::kmodManagerMenu, MenuEvent::OnModManagerMenu },
		{ UIStringHolder::STRING_INDICES::kquantityMenu, MenuEvent::OnQuantityMenu },
		{ UIStringHolder::STRING_INDICES::krefreshMenu, MenuEvent::OnRefreshMenu },
		{ UIStringHolder::STRING_INDICES::kfaderMenu, MenuEvent::OnFaderMenu },
		{ UIStringHolder::STRING_INDICES::kloadWaitSpinner, MenuEvent::OnLoadWaitSpinner },
		{ UIStringHolder::STRING_INDICES::kstreamingInstallMenu, MenuEvent::OnStreamingInstallMenu },
		{ UIStringHolder::STRING_INDICES::kdebugTextMenu, MenuEvent::OnDebugTextMenu },
		{ UIStringHolder::STRING_INDICES::ktweenMenu, MenuEvent::OnTweenMenu },
		{ UIStringHolder::STRING_INDICES::koverlayMenu, MenuEvent::OnOverlayMenu },
		{ UIStringHolder::STRING_INDICES::koverlayInteractionMenu, MenuEvent::OnOverlayInteractionMenu },
		{ UIStringHolder::STRING_INDICES::ktitleSequenceMenu, MenuEvent::OnTitleSequenceMenu },
		{ UIStringHolder::STRING_INDICES::kconsoleNativeUIMenu, MenuEvent::OnConsoleNativeUIMenu },
		{ UIStringHolder::STRING_INDICES::kcontainerMenu, MenuEvent::OnContainerMenu },
		{ UIStringHolder::STRING_INDICES::kmessageBoxMenu, MenuEvent::OnMessageBoxMenu }
	};

	bool IEvents::Initialize()
	{
		if (IAL::IsAE())
		{
			if (!hook::call5(
					ISKSE::GetBranchTrampoline(),
					LoadPluginINI_C,
					reinterpret_cast<std::uintptr_t>(PostLoadPluginINI_AE_Hook),
					m_Instance.LoadPluginINI_AE_O))
			{
				m_Instance.FatalError("Could not install event hooks");
				return false;
			}
		}
		else
		{
			if (!hook::call5(
					ISKSE::GetBranchTrampoline(),
					LoadPluginINI_C,
					reinterpret_cast<std::uintptr_t>(PostLoadPluginINI_Hook),
					m_Instance.LoadPluginINI_O))
			{
				m_Instance.FatalError("Could not install event hooks");
				return false;
			}
		}

		if (!hook::call5(
				ISKSE::GetBranchTrampoline(),
				PopulateUIStringHolder_C,
				reinterpret_cast<std::uintptr_t>(PopulateUIStringHolder_Hook),
				m_Instance.PopulateUIStringHolder_O))
		{
			m_Instance.Error("Could not install PopulateUIStringHolder hook, menu event tracking won't work properly");
		}

		m_Instance.Debug("Installed event hooks");

		auto& skse = ISKSE::GetSingleton();

		auto messagingInterface = skse.GetInterface<SKSEMessagingInterface>();

		m_Instance.Debug("Registering SKSE message listener..");
		messagingInterface->RegisterListener(skse.GetPluginHandle(), "SKSE", MessageHandler);
		m_Instance.Debug("OK");

		return true;
	}

	void IEvents::RegisterForEvent(Event a_code, EventCallback a_fn)
	{
		m_Instance.m_events.try_emplace(a_code).first->second.emplace_back(a_code, a_fn);
	}

	void IEvents::RegisterForEvent(MenuEvent a_code, MenuEventCallback a_fn)
	{
		m_Instance.m_menu_events.try_emplace(a_code).first->second.emplace_back(a_code, a_fn);
	}

	void IEvents::TriggerEvent(Event a_code, void* a_args)
	{
		const auto it = m_Instance.m_events.find(a_code);
		if (it == m_Instance.m_events.end())
			return;

		for (const auto& evtd : it->second)
			evtd.m_callback(a_code, a_args);
	}

	void IEvents::TriggerMenuEventAny(
		MenuEvent                           a_code,
		const MenuOpenCloseEvent*           a_evn,
		BSTEventSource<MenuOpenCloseEvent>* a_dispatcher)
	{
		m_Instance.TriggerMenuEventImpl(MenuEvent::OnAnyMenu, a_code, a_evn, a_dispatcher);
	}

	void IEvents::TriggerMenuEventImpl(
		MenuEvent                           a_triggercode,
		MenuEvent                           a_code,
		const MenuOpenCloseEvent*           a_evn,
		BSTEventSource<MenuOpenCloseEvent>* a_dispatcher)
	{
		auto it = m_menu_events.find(a_triggercode);
		if (it == m_menu_events.end())
		{
			return;
		}

		auto it2 = it->second.begin();
		while (it2 != it->second.end())
		{
			if (it2->m_callback(a_code, a_evn, a_dispatcher))
			{
				++it2;
			}
			else
			{
				it2 = it->second.erase(it2);
			}
		}
	}

	MenuEvent IEvents::GetMenuEventCode(const BSFixedString& a_str)
	{
		auto it = m_Instance.m_mstc_map.find(a_str);
		if (it != m_Instance.m_mstc_map.end())
			return it->second;

		return MenuEvent::OnUnknownMenu;
	}

	void IEvents::PostLoadPluginINI_Hook()
	{
		m_Instance.LoadPluginINI_O();
		IDDispatcher::DriversOnGameConfigLoaded();
		m_Instance.TriggerEvent(Event::OnConfigLoad);
	}

	void IEvents::PostLoadPluginINI_AE_Hook(void* a_unk)
	{
		m_Instance.LoadPluginINI_AE_O(a_unk);
		IDDispatcher::DriversOnGameConfigLoaded();
		m_Instance.TriggerEvent(Event::OnConfigLoad);
	}

	UIStringHolder* IEvents::PopulateUIStringHolder_Hook(void* a_dst)
	{
		auto dst = m_Instance.PopulateUIStringHolder_O(a_dst);
		m_Instance.CreateMSTCMap();
		return dst;
	}

	void IEvents::MessageHandler(SKSEMessagingInterface::Message* a_message)
	{
		if (a_message->type == SKSEMessagingInterface::kMessage_InputLoaded)
		{
			if (auto mm = MenuManager::GetSingleton())
			{
				mm->GetMenuOpenCloseEventDispatcher().AddEventSink(MenuOpenCloseEventHandler::GetSingleton());
				m_Instance.Debug("Added menu event sink");
			}
			else
			{
				m_Instance.Error("Could not add menu open/close event sink");
			}
		}
		else if (a_message->type == SKSEMessagingInterface::kMessage_PostPostLoad)
		{
			IDDispatcher::InitializeDriversPost();
		}

		m_Instance.TriggerEvent(Event::OnMessage, static_cast<void*>(a_message));
	}

	void IEvents::CreateMSTCMap()
	{
		if (auto sh = UIStringHolder::GetSingleton())
		{
			for (auto& e : s_mstc_map_desc)
			{
				auto& str = sh->GetString(e.index);
				m_mstc_map.try_emplace(str, e.event);
			}
		}

		BSFixedString customMenu("CustomMenu");
		m_mstc_map.try_emplace(customMenu, MenuEvent::OnCustomMenu);
	}

	auto MenuOpenCloseEventHandler::ReceiveEvent(
		const MenuOpenCloseEvent*           a_evn,
		BSTEventSource<MenuOpenCloseEvent>* a_dispatcher)
		-> EventResult
	{
		if (a_evn)
		{
			auto code = IEvents::GetMenuEventCode(a_evn->menuName);
			IEvents::TriggerMenuEventAny(code, a_evn, a_dispatcher);
		}

		return EventResult::kContinue;
	}

	MenuEventTrack::MenuEventTrack(const trackSet_t& a_set)
	{
		SetTracked(a_set);
	}

	void MenuEventTrack::SetTracked(const trackSet_t& a_set)
	{
		for (auto& e : m_tracked)
		{
			e = false;
		}

		for (auto& e : a_set)
		{
			m_tracked[stl::underlying(e)] = true;
		}
	};

	void MenuEventTrack::Track(MenuEvent a_code, bool a_opening)
	{
		if (!m_tracked[stl::underlying(a_code)])
			return;

		if (a_opening)
		{
			if (std::find(m_stack.begin(), m_stack.end(), a_code) == m_stack.end())
				m_stack.emplace_back(a_code);
		}
		else
		{
			auto it = std::find(m_stack.begin(), m_stack.end(), a_code);
			if (it != m_stack.end())
				m_stack.erase(it);
		}
	}

	void MenuEventTrack::ClearStack()
	{
		if (!m_stack.empty())
			m_stack.clear();
	}

}
