#include "pch.h"

#include <ext/Patching.cpp>

namespace SDT
{
	static constexpr const char* CKEY_SKIPMISSINGINI = "SkipMissingPluginINI";
	static constexpr const char* CKEY_LSF = "LoadScreenFilter";
	static constexpr const char* CKEY_LSF_ALLOW = "LoadScreenAllow";
	static constexpr const char* CKEY_LSF_BLOCK = "LoadScreenBlock";
	static constexpr const char* CKEY_DISABLE_WEATHER_LENSFLARE = "DisableWeatherLensFlare";

	using namespace Patching;

	DMisc DMisc::m_Instance;

	DMisc::DMisc()
	{
	}

	void DMisc::LoadConfig()
	{
		m_conf.skipmissingini = GetConfigValue(CKEY_SKIPMISSINGINI, true);
		m_conf.loadscreen_filter = GetConfigValue(CKEY_LSF, false);
		if (m_conf.loadscreen_filter)
		{
			ParseLoadscreenRules();
		}
		m_conf.disable_lens_flare = GetConfigValue(CKEY_DISABLE_WEATHER_LENSFLARE, false);
		m_conf.disable_actor_fade = GetConfigValue("DisableActorFade", false);
		m_conf.disable_player_fade = GetConfigValue("DisablePlayerFade", false);
	}

	void DMisc::ParseLoadscreenRules()
	{
		std::vector<std::string> elems;
		StrHelpers::SplitString(GetConfigValue("LoadScreenBlock", "All"), ',', elems);

		std::string kall("All");

		for (auto& e : elems)
		{
			StrHelpers::trim(e);
			if (e.empty())
			{
				continue;
			}

			if (StrHelpers::iequal(e, kall))
			{
				m_conf.dls_deny_all = true;
				break;
			}

			m_conf.dls_deny.emplace(e);
		}

		elems.clear();
		StrHelpers::SplitString(GetConfigValue("LoadScreenAllow", ""), ',', elems);

		for (auto& e : elems)
		{
			StrHelpers::trim(e);
			if (e.empty())
			{
				continue;
			}

			if (StrHelpers::iequal(e, kall))
			{
				break;
			}

			m_conf.dls_allow.emplace(e);
		}
	}

	void Structures::_SettingCollectionList::LoadIni_Hook()
	{
		DWORD dwAttrib = ::GetFileAttributesA(inipath);
		if (dwAttrib == INVALID_FILE_ATTRIBUTES ||
		    (dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
		{
			return;
		}

		this->LoadINI();
	}

	void DMisc::PostLoadConfig()
	{
		if (m_conf.skipmissingini)
		{
			Message("Disabling processing of missing plugin INIs");
		}
	}

	void DMisc::Patch()
	{
		if (m_conf.skipmissingini)
		{
			if (IAL::IsAE())
			{
				Patching::safe_write(
					SkipNoINI,
					static_cast<const void*>(Payloads::SkipNoINI_AE),
					sizeof(Payloads::SkipNoINI_AE));
			}
			else
			{
				Patching::safe_write(
					SkipNoINI,
					static_cast<const void*>(Payloads::SkipNoINI),
					sizeof(Payloads::SkipNoINI));
			}
		}

		if (m_conf.loadscreen_filter)
		{
			m_Instance.m_pluginData = std::make_unique<IPluginInfo>();

			struct LoadScreenLoadFormInject : JITASM::JITASM
			{
				LoadScreenLoadFormInject(
					std::uintptr_t targetAddr) :
					JITASM(ISKSE::GetLocalTrampoline())
				{
					Xbyak::Label retnOKLabel;
					Xbyak::Label retnSkipLabel;
					Xbyak::Label callHookFnLabel;
					Xbyak::Label callUnkFnLabel;

					Xbyak::Label load;

					mov(rcx, r14);
					call(ptr[rip + callHookFnLabel]);
					test(al, al);
					jne(load);
					jmp(ptr[rip + retnSkipLabel]);
					L(load);
					mov(rcx, rbp);
					call(ptr[rip + callUnkFnLabel]);
					jmp(ptr[rip + retnOKLabel]);

					L(retnOKLabel);
					dq(targetAddr + 0x8);

					L(retnSkipLabel);
					dq(targetAddr + (IAL::IsAE() ? 0x4ED : 0x4EC));

					L(callHookFnLabel);
					dq(std::uintptr_t(TESLoadScreen_LoadForm_Hook));

					L(callUnkFnLabel);
					dq(TESLoadScreen_LoadForm_Unkf);
				}
			};

			auto target(TESLoadScreen_LoadForm + 0x36);
			LoadScreenLoadFormInject code(target);
			ISKSE::GetBranchTrampoline().Write6Branch(target, code.get());

			//IEvents::RegisterForEvent(Event::OnMessage, MessageHandler);

			Message("Loadscreen filter installed");
		}

		if (m_conf.disable_actor_fade)
		{
			if (IAL::IsAE())
			{
				constexpr std::uint8_t data1[] = { 0xEB, 0x61, 0x90, 0x90, 0x90, 0x90, 0x90 };
				safe_write(ActorFade_a + 0x4CA, data1, sizeof(data1));

				constexpr std::uint8_t data2[] = { 0x0F, 0xB6, 0x9D, 0x80, 0x02, 0x00, 0x00, 0xEB, 0x6A, 0x90, 0x90, 0x90 };
				safe_write(ActorFade_a + 0x4BA, data2, sizeof(data2));
			}
			else
			{
				constexpr std::uint8_t data1[] = { 0xEB, 0x60 };  // jmp 0x60
				safe_write(ActorFade_a + 0x4FC, data1, sizeof(data1));

				constexpr std::uint8_t data2[] = { 0xEB, 0x69 };  // jmp 0x69
				safe_write(ActorFade_a + 0x4F3, data2, sizeof(data2));
			}

			Message("Actor fade patch applied");
		}

		if (m_conf.disable_player_fade)
		{
			if (IAL::IsAE())
			{
				constexpr std::uint8_t data[] = { 0xEB, 0x58, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };  // jmp 0x58
				safe_write(PlayerFade_a, data, sizeof(data));
			}
			else
			{
				constexpr std::uint8_t data[] = { 0xEB, 0x58 };  // jmp 0x58
				safe_write(PlayerFade_a, data, sizeof(data));

			}
			Message("Player fade patch applied");
		}
	}

	void DMisc::RegisterHooks()
	{
		if (m_conf.skipmissingini)
		{
			RegisterHook(
				SkipNoINI + 0x3,
				GetFnAddr(&Structures::_SettingCollectionList::LoadIni_Hook),
				HookDescriptor::HookType::kWR6Call);
		}

		if (m_conf.disable_lens_flare)
		{
			IEvents::RegisterForEvent(Event::OnMessage, MessageHandler);
		}
	}

	bool DMisc::Prepare()
	{
		return true;
	}

	bool DMisc::TESLoadScreen_LoadForm_Hook(TESLoadScreen* a_form)
	{
		if (m_Instance.m_conf.dls_deny_all &&
		    m_Instance.m_conf.dls_allow.empty())
		{
			return false;
		}

		if (!m_Instance.m_pluginData->IsPopulated())
		{
			m_Instance.m_pluginData->Populate();
		}

		std::uint32_t modIndex;
		if (a_form->formID.GetPluginPartialIndex(modIndex))
		{
			auto pi = m_Instance.m_pluginData->Lookup(modIndex);
			if (pi)
			{
				if (m_Instance.m_conf.dls_allow.contains(pi->name))
				{
					return true;
				}

				if (m_Instance.m_conf.dls_deny_all ||
				    m_Instance.m_conf.dls_deny.contains(pi->name))
				{
					return false;
				}
			}
		}

		return true;
	}

	void DMisc::RemoveLensFlareFromWeathers()
	{
		if (auto dh = DataHandler::GetSingleton())
		{
			std::size_t count(0);

			for (auto& e : dh->arrWTHR)
			{
				if (e && e->lensFlare)
				{
					e->lensFlare = nullptr;
					count++;
				}
			}

			m_Instance.Message("Removed lens flare from %zu/%zu weather form(s)", count, static_cast<std::size_t>(dh->arrWTHR.count));
		}
	}

	void DMisc::MessageHandler(Event, void* a_args)
	{
		auto message = static_cast<SKSEMessagingInterface::Message*>(a_args);

		if (message->type == SKSEMessagingInterface::kMessage_DataLoaded)
		{
			RemoveLensFlareFromWeathers();
		}
	}
}
