#pragma once

namespace SDT
{
	class DMisc :
		public IDriver,
		IConfig
	{
	public:
		static inline constexpr auto ID = DRIVER_ID::MISC;

		FN_NAMEPROC("Miscellaneous");
		FN_ESSENTIAL(false);
		FN_DRVDEF(6);

	private:
		DMisc();

		virtual void LoadConfig() override;
		virtual void PostLoadConfig() override;
		virtual void Patch() override;
		virtual void RegisterHooks() override;
		virtual bool Prepare() override;

		void ParseLoadscreenRules();

		static bool TESLoadScreen_LoadForm_Hook(TESLoadScreen* a_form);
		static void MessageHandler(Event, void*);

		static void RemoveLensFlareFromWeathers();

		struct
		{
			bool skipmissingini;
			bool loadscreen_filter;
			bool disable_lens_flare;
			stl::iunordered_set<std::string> dls_allow;
			stl::iunordered_set<std::string> dls_deny;
			bool dls_deny_all;
			bool disable_actor_fade;
			bool disable_player_fade;
		} m_conf;

		std::unique_ptr<IPluginInfo> m_pluginData;

		inline static auto SkipNoINI = IAL::Addr(AID::INIProc0, 36725, Offsets::SkipNoINI, 0x319);
		inline static auto TESLoadScreen_LoadForm = IAL::Addr<std::uintptr_t>(AID::TESLoadScreen_LoadForm, 21830);
		inline static auto TESLoadScreen_LoadForm_Unkf = IAL::Addr<std::uintptr_t>(AID::TESLoadScreen_LoadForm_Unkf, 13988);
		inline static auto ActorFade_a = IAL::Addr<std::uintptr_t>(AID::ActorFade, 33007);
		inline static auto PlayerFade_a = IAL::Addr(AID::PlayerFade, 50832, 0x431, 0x4DD);

		static DMisc m_Instance;
	};
}
