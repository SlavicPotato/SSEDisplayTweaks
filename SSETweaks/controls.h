#pragma once

namespace SDT
{
	class DControls :
		public IDriver,
		IConfig
	{
	public:
		static inline constexpr auto ID = DRIVER_ID::CONTROLS;

		FN_NAMEPROC("Controls");
		FN_ESSENTIAL(false);
		FN_DRVDEF(6);

	private:
		DControls() = default;

		virtual void LoadConfig() override;
		virtual void PostLoadConfig() override;
		virtual void Patch() override;
		virtual void RegisterHooks() override;
		virtual bool Prepare() override;
		virtual void PostPatch() override;

		struct
		{
			bool damping_fix;
			float tcpf_threshold;
			float fp_mount_horiz_sens;
			bool map_kb_movement;
			float map_kb_movement_speedmult;
			bool auto_vanity_camera;
			bool dialogue_look;
			bool dialogue_look_se;
			bool gamepad_cursor_speed;
			bool lockpick_rotation;
			bool freecam_verticalsens;
			bool freecam_translation;
			bool vertical_look_sens;
			bool slow_time_cam;
		} m_conf;

		void Patch_Damping();
		void Patch_FPMountHorizontalSens();
		void Patch_MapKBMovement();
		void Patch_AutoVanityCamera();
		void Patch_DialogueLook();
		void Patch_DialogueLook_Edge();
		void Patch_GamepadCursor();
		void Patch_LockpickRotation();
		void Patch_FreecamVerticalSens();
		void Patch_FreecamTranslation();
		void Patch_VerticalLookSensitivity();
		void Patch_SlowTimeCameraMovement();

		void WriteKBMovementPatchDir(
			std::uintptr_t a_address,
			float* a_speedAddr,
			bool a_isY = false);

		static void MouseSens_Hook(PlayerControls* a_controls, FirstPersonState* a_fpState);
		static void MouseSens_AE_Hook(FirstPersonState* a_fpState);
		static void AddMapCameraPos_Hook(MapCamera* a_camera, float x, float y, float z);
		static void PlayerControls_InputEvent_ProcessEvent_Edge_Hook(PlayerControls* a_controls);

		struct
		{
			float* fMouseHeadingXScale;
			float* fMouseHeadingSensitivity;
			float* fPCDialogueLookStart;
		} m_gv;

		decltype(&AddMapCameraPos_Hook) addCameraPos_o;

		inline static auto MT_Inject = IAL::Addr(AID::UnkMovFunc0, 0, Offsets::MT_Inject, 0);
		inline static auto MT_Inject_AE1 = IAL::Addr(0, 41996, 0, 0x389);
		inline static auto MT_Inject_AE2 = IAL::Addr(0, 41996, 0, 0x880);
		inline static auto FPSittingRotationSpeed = IAL::Addr<float*>(AID::UnkFloat0, 382476);
		inline static auto FMHS_Inject = IAL::Addr(AID::UnkMM0, 50724, Offsets::FMHS_Inject, 0x125);
		inline static auto MapLookHandler_ProcessButton = IAL::Addr<std::uintptr_t>(AID::MapLookHandler_ProcessButton, 53062);
		inline static auto AutoVanityState_Update = IAL::Addr<std::uintptr_t>(AID::AutoVanityState_Update, 50709);
		inline static auto PlayerControls_InputEvent_ProcessEvent = IAL::Addr<std::uintptr_t>(AID::PlayerControls_InputEvent_ProcessEvent, 42338);
		inline static auto CursorMenu_MenuEventHandler_ProcessThumbstick = IAL::Addr<std::uintptr_t>(AID::CursorMenu_MenuEventHandler_ProcessThumbstick_Sub140ED3120, 82540);
		inline static auto LockpickingMenu_ProcessMouseMove = IAL::Addr<std::uintptr_t>(AID::LockpickingMenu_ProcessMouseMove, 51955);
		inline static auto unkCoordData = IAL::Addr<Structures::UnkCoordData**>(AID::UnkCoordData, 403551);
		inline static auto FreeCameraState_Update = IAL::Addr<std::uintptr_t>(AID::FreeCameraState_Update_Sub140848AA0, 50749);
		inline static auto VerticalLookSens_ThirdPerson = IAL::Addr<std::uintptr_t>(49978, 50914);
		inline static auto VerticalLookSens_Dragon = IAL::Addr<std::uintptr_t>(32370, 33119);
		inline static auto VerticalLookSens_Horse = IAL::Addr<std::uintptr_t>(49839, 50770);
		inline static auto SlowTimeCameraMovementFix1 = IAL::Addr<std::uintptr_t>(49977, 50913);
		inline static auto SlowTimeCameraMovementFix2 = IAL::Addr<std::uintptr_t>(49980, 50911);
		inline static auto SlowTimeCameraMovementFix3 = IAL::Addr<std::uintptr_t>(49981, 50921);

		static void replace_st_timer(std::uintptr_t a_addr);

		inline static auto m_Sub_140707110 = IAL::Addr<decltype(PlayerControls_InputEvent_ProcessEvent_Edge_Hook)*>(41292, 42372);  // AE: unreferenced

		static DControls m_Instance;
	};
}