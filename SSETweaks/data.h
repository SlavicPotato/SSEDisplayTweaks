#pragma once

#include <dxgicommon.h>

namespace SDT
{
	static inline constexpr std::uint64_t VER_1_6_342 = 0x0001000601560000;

	namespace AID
	{
		static inline constexpr unsigned long long DXGIData = 524728;
		static inline constexpr unsigned long long Present = 75461;
		static inline constexpr unsigned long long RT0 = 35565;
		static inline constexpr unsigned long long FMTProc = 35574;
		static inline constexpr unsigned long long D3D11Create = 75589;
		static inline constexpr unsigned long long D3D11CreateDeviceAndSwapChain_JMP = 102631;
		static inline constexpr unsigned long long fMaxFrameCounterDifferenceToConsiderVisibleA = 517358;

		static inline constexpr unsigned long long Init0 = 35548;
		static inline constexpr unsigned long long LoadPluginINI = 35642;
		static inline constexpr unsigned long long INIProc0 = 35652;

		static inline constexpr unsigned long long SetExpressionOverride = 53926;

		static inline constexpr unsigned long long INISettingsCollection = 524557;
		static inline constexpr unsigned long long INISettingsCollection_offset1 = 22315;

		static inline constexpr unsigned long long CalculateCRC32_64 = 66964;

		static inline constexpr unsigned long long MenuManager = 514178;
		static inline constexpr unsigned long long UIStringHolder = 514286;

		static inline constexpr unsigned long long EventDispatcher_offset1 = 34381;
		static inline constexpr unsigned long long EventDispatcher_offset2 = 28367;
		static inline constexpr unsigned long long EventDispatcher_offset3 = 13805;

		static inline constexpr unsigned long long WindowCreate = 75591;

		static inline constexpr unsigned long long PhysCalcFix = 35605;
		static inline constexpr unsigned long long PhysFuncUnk0 = 66987;

		static inline constexpr unsigned long long D3DInit = 75595;

		static inline constexpr unsigned long long PhysCalc = 76017;

		static inline constexpr unsigned long long WindowSwapChainAdjust = 75453;
		static inline constexpr unsigned long long WindowSwapChain2 = 75454;

		static inline constexpr unsigned long long WinFunc0 = 75460;
		static inline constexpr unsigned long long WinFunc1 = 75449;

		static inline constexpr unsigned long long MT_Inject = 230155;

		static inline constexpr unsigned long long UnkMovFunc0 = 40937;

		static inline constexpr unsigned long long ScriptRunGame = 53117;
		static inline constexpr unsigned long long ScriptRunUI = 53118;

		static inline constexpr unsigned long long IsComplex = 516932;

		static inline constexpr unsigned long long RDCImpl = 102238;

		static inline constexpr unsigned long long UnkFloat0 = 509776;
		static inline constexpr unsigned long long UnkMM0 = 49805;

		static inline constexpr unsigned long long FrameTimerNoSlow = 523661;
		static inline constexpr unsigned long long FrameTimerSlow = 523660;

		static inline constexpr unsigned long long MapLookHandler_ProcessButton = 52170;

		static inline constexpr unsigned long long AutoVanityState_Update = 49781;

		static inline constexpr unsigned long long PlayerControls_InputEvent_ProcessEvent = 41259;

		static inline constexpr unsigned long long CursorMenu_MenuEventHandler_ProcessThumbstick_Sub140ED3120 = 80428;

		static inline constexpr unsigned long long LockpickingMenu_ProcessMouseMove = 51076;

		static inline constexpr unsigned long long TESLoadScreen_LoadForm = 21366;
		static inline constexpr unsigned long long TESLoadScreen_LoadForm_Unkf = 13902;

		static inline constexpr unsigned long long UnkCoordData = 517043;

		static inline constexpr unsigned long long Sub_140707110 = 41292;

		static inline constexpr unsigned long long FreeCameraState_Update_Sub140848AA0 = 49819;

		static inline constexpr unsigned long long ActorFade = 32270;
		static inline constexpr unsigned long long PlayerFade = 49899;
	}

	namespace Offsets
	{
		// indirect: 488B05????????4C8B098B503041FF5140 (mov rax, qword ptr ds:[?])  (524728)
		//static inline constexpr std::uintptr_t DGXIData = 0x3025F00;

		//static inline constexpr std::uintptr_t RTProc = 0xD69FF0;

		// 40574883EC3048C7442420FEFFFFFF48895C2440488BD9488B0D????????E8????????B909000000+5E3  (35565)
		static inline constexpr std::uintptr_t RTUnk0_GM_C = 0x5E3;
		// 40574883EC3048C7442420FEFFFFFF48895C2440488BD9488B0D????????E8????????B909000000+366  (35565)
		static inline constexpr std::uintptr_t RTUnk0_UI_C = 0x366;
		// 4883EC280FB605????????488D0D????????33D28805????????3815????????0F4515????????E8????????  (35574)
		//static inline constexpr std::uintptr_t RTUnk0_O = 0x5B3E40;

		// 40555356574156488DAC2440FFFFFF4881ECC001000048C74598FEFFFFFF8B0D????????65488B042558000000488B3CC841BE68070000418B1C3E899DF000000033F64189343E+632  (35548)
		static inline constexpr std::uintptr_t LoadPluginINI_C = 0x632;
		// 40574883EC5048C7442420FEFFFFFF48895C2468488D4C2438E8????????90488D4C2448  (35642)
		//static inline constexpr std::uintptr_t LoadPluginINI_O = 0x5B8F00;

		// 40555356574156488DAC2440FFFFFF4881ECC001000048C74598FEFFFFFF8B0D????????65488B042558000000488B3CC8+A32 (35548)
		static inline constexpr uintptr_t PopulateUIStringHolder_C = 0xA32;

		//static inline constexpr std::uintptr_t JMPT_D3D11CreateDevice = 0x1372F02;

		// 40555357488DAC248092FFFFB8806E0000E8????????482BE0488D542460488D0D????????+2E3  (75589)
		static inline constexpr std::uintptr_t D3D11CreateDeviceAndSwapChain_C = 0x2E3;

		static inline constexpr std::uintptr_t CreateDXGIFactory_C = 0x25;

		//static inline constexpr std::uintptr_t IDXGISwapChain_Present_C = 0xD6A027;

		// 35548
		static inline constexpr std::uintptr_t bFullscreen_Patch = 0x82F;
		static inline constexpr std::uintptr_t bBorderless_Patch = 0x83A;
		static inline constexpr std::uintptr_t iSizeW_Patch = 0x845;
		static inline constexpr std::uintptr_t iSizeH_Patch = 0x84F;

		// 40555356574156488DAC2440FFFFFF4881ECC001000048C74598FEFFFFFF8B0D????????65488B042558000000488B3CC841BE68070000418B1C3E899DF000000033F64189343E+86D  (35548)
		static inline constexpr std::uintptr_t DisplayRefreshRate = 0x86D;

		// 4C8BDC49895B1049896B184989732041564881EC30040000418D41FF458BF1498BD88BEA488BF183F80F  (53926)
		static inline constexpr std::uintptr_t SetExpressionOverride_lea = 0x1A;
		static inline constexpr std::uintptr_t SetExpressionOverride_cmp = 0x29;

		// 4883EC28B901000000E8????????803D????????007405E8????????488B05????????4533C0488B4818488B05????????4C8B098B5030  (75461)
		//static inline constexpr std::uintptr_t PresentPatchTarget = 0xD6A2B0;
		//static inline constexpr std::uintptr_t Present_Limiter = 0x3B;
		static inline constexpr std::uintptr_t Present_Flags_Inject = 0x23;

		// 488BC456574154415641574881EC6002000048C7442430FEFFFFFF4889581048896820488BD9488D5008+153  (35652)
		static inline constexpr std::uintptr_t SkipNoINI = 0x153;

		// 4C8BDC49895B0849897310574881ECC0000000488B510833F6+163  (75591)
		static inline constexpr std::uintptr_t WindowCreate = 0x163;

		// 0F28D80F57D2F30F111D????????84D27529+0xE  (76017)
		static inline constexpr std::uintptr_t PhysCalcFix = 0xE;

		// (35574)
		static inline constexpr std::uintptr_t PhysCalcHT = 0x9F;

		static inline constexpr std::uintptr_t MaxFrameLatency = 0x91;

		static inline constexpr std::uintptr_t ResizeBuffers_Inject = 0x217;

		static inline constexpr std::uintptr_t GetClientRect1 = 0x192;

		static inline constexpr std::uintptr_t ResizeBuffersDisable = 0x27;
		static inline constexpr std::uintptr_t ResizeTargetDisable = 0x22;
		static inline constexpr std::uintptr_t ResizeTarget = 0x10A;

		static inline constexpr std::uintptr_t MT_Inject = 0x300;
		static inline constexpr std::uintptr_t FMHS_Inject = 0xAD;

		static inline constexpr std::uintptr_t ScriptUpdateBudgetGame = 0x90;
		static inline constexpr std::uintptr_t ScriptUpdateBudgetUI = 0x90;

		static inline constexpr std::uintptr_t Present = 0x34;

		static inline constexpr std::uintptr_t MapLookHandler_ProcessButton_Up = 0xB7;
		static inline constexpr std::uintptr_t MapLookHandler_ProcessButton_Down = 0xE3;
		static inline constexpr std::uintptr_t MapLookHandler_ProcessButton_Left = 0x113;
		static inline constexpr std::uintptr_t MapLookHandler_ProcessButton_Right = 0x140;
		static inline constexpr std::uintptr_t MapLookHandler_ProcessButton_Add = 0x155;

		static inline constexpr std::uintptr_t AutoVanityState_Update_IncrementAngle = 0xE2;

		static inline constexpr std::uintptr_t PlayerControls_InputEvent_ProcessEvent_LoadDLSpeed = 0x1CC;
		static inline constexpr std::uintptr_t PlayerControls_InputEvent_ProcessEvent_movssix = 0x1E6;

		static inline constexpr std::uintptr_t CursorMenu_MenuEventHandler_ProcessThumbstick_MulCS = 0x4A;

		static inline constexpr std::uintptr_t LockpickingMenu_ProcessMouseMove_MulFT = 0x42;
	}

	namespace OffsetsAE
	{
		static inline constexpr std::uintptr_t CreateDXGIFactory_C = 0xDAD6D0 + 0x25;
		static inline constexpr std::uintptr_t D3D11CreateDeviceAndSwapChain_C = 0xDAD6D0 + 0x2C0;

		static inline constexpr std::uintptr_t Present_Flags_Inject = 0xDA5BE0 + 0x8E;
		static inline constexpr std::uintptr_t Present = 0xDA5BE0 + 0x9F;

		static inline constexpr std::uintptr_t bFullscreen_Patch = 0x5D41A0 + 0xCB0;
		static inline constexpr std::uintptr_t bBorderless_Patch = 0x5D41A0 + 0xCBB;
		static inline constexpr std::uintptr_t iSizeW_Patch = 0x5D41A0 + 0xCC6;
		static inline constexpr std::uintptr_t iSizeH_Patch = 0x5D41A0 + 0xCD0;
		static inline constexpr std::uintptr_t DisplayRefreshRate = 0x5D41A0 + 0xCEE;

		static inline constexpr std::uintptr_t MaxFrameLatency = 0xDA3850 + 0x2FE;

		static inline constexpr std::uintptr_t ResizeBuffers_Inject = 0xDA5030 + 0x2C4;
		static inline constexpr std::uintptr_t ResizeBuffersDisable = 0xDA5030 + 0x26;  // uncond jmp

		static inline constexpr std::uintptr_t ResizeTargetDisable = 0xDA54C0 + 0x24;  // uncond jmp

		static inline constexpr std::uintptr_t ResizeTarget = 0xDA54C0 + 0xF9;

		static inline constexpr std::uintptr_t WindowCreate = 0xDA3850 + 0x22C;
		static inline constexpr std::uintptr_t GetClientRect1 = 0xDA5A00 + 0x18B;

		static inline constexpr std::uintptr_t SetExpressionOverride_lea = 0x979050 + 0x18;
		static inline constexpr std::uintptr_t SetExpressionOverride_cmp = 0x979050 + 0x28;

		static inline constexpr std::uintptr_t ScriptUpdateBudgetGame = 0x94F6E0 + 0x90;
		static inline constexpr std::uintptr_t ScriptUpdateBudgetUI = 0x94F7C0 + 0x90;

		static inline constexpr std::uintptr_t SkipNoINI = 0x5E2C90 + 0x319;

		static inline constexpr std::uintptr_t TESLoadScreen_LoadForm = 0x2F8C70;
		static inline constexpr std::uintptr_t TESLoadScreen_LoadForm_Unkf = 0x187720;

		static inline constexpr std::uintptr_t ActorFade = 0x50D640;
		static inline constexpr std::uintptr_t PlayerFade = 0x878630 + 0x4DD;

		static inline constexpr std::uintptr_t PhysCalcHT = 0x5DB3D0 + 0xA6;
		static inline constexpr std::uintptr_t PhysCalc = 0xDE5F20;
		static inline constexpr std::uintptr_t IsComplex = 0x2FC161C;

		static inline constexpr std::uintptr_t FrameTimerSlow = 0x30064C8;
		static inline constexpr std::uintptr_t FrameTimerNoSlow = 0x30064CC;

		static inline constexpr std::uintptr_t LoadPluginINI_C = 0x5D41A0 + 0xA71;
		static inline constexpr std::uintptr_t PopulateUIStringHolder_C = 0x5D41A0 + 0xE85;

		static inline constexpr std::uintptr_t MT_Inject1 = 0x724ED0 + 0x389;
		static inline constexpr std::uintptr_t MT_Inject2 = 0x724ED0 + 0x880;

		static inline constexpr std::uintptr_t FMHS_Inject = 0x872A30 + 0x125;
		static inline constexpr std::uintptr_t UnkFloat0 = 0x1E876D8;
		static inline constexpr std::uintptr_t MapLookHandler_ProcessButton = 0x910870;
		static inline constexpr std::uintptr_t MapLookHandler_ProcessButton_Up = 0xC6;
		static inline constexpr std::uintptr_t MapLookHandler_ProcessButton_Down = 0xF2;
		static inline constexpr std::uintptr_t MapLookHandler_ProcessButton_Left = 0x122;
		static inline constexpr std::uintptr_t MapLookHandler_ProcessButton_Right = 0x14F;
		static inline constexpr std::uintptr_t MapLookHandler_ProcessButton_Add = 0x164;

		static inline constexpr std::uintptr_t AutoVanityState_Update = 0x871AB0;
		static inline constexpr std::uintptr_t AutoVanityState_Update_IncrementAngle = 0xE2;

		static inline constexpr std::uintptr_t PlayerControls_InputEvent_ProcessEvent = 0x72E720;
		static inline constexpr std::uintptr_t PlayerControls_InputEvent_ProcessEvent_LoadDLSpeed = 0x574;

		static inline constexpr std::uintptr_t UnkCoordData = 0x2FC1C10;

		static inline constexpr std::uintptr_t CursorMenu_MenuEventHandler_ProcessThumbstick_Sub140F1A3B0 = 0xF1A3B0;
		static inline constexpr std::uintptr_t CursorMenu_MenuEventHandler_ProcessThumbstick_MulCS = 0x5A;

		static inline constexpr std::uintptr_t LockpickingMenu_ProcessMouseMove = 0x8C70D0;
		static inline constexpr std::uintptr_t LockpickingMenu_ProcessMouseMove_MulFT = 0x42;

		static inline constexpr std::uintptr_t FreeCameraState_Update_Sub140874400 = 0x874400;

	}

	namespace Payloads
	{
		static inline constexpr std::uint8_t SkipNoINI[] = { 0x4C, 0x89, 0xF1 };
		static inline constexpr std::uint8_t SkipNoINI_AE[] = { 0x48, 0x8B, 0xCF };
		static inline constexpr std::uint8_t seoFix_lea[] = { 0x01, 0x90 };
		static inline constexpr std::uint8_t seoFix_cmp = 0x10;
		static inline constexpr std::uint8_t nopx3[] = { 0x90, 0x90, 0x90 };
		static inline constexpr std::uint8_t nopjmp[] = { 0x90, 0xEB };
		static inline constexpr std::uint8_t bw_patch[] = {
			0xB8,
			0x01,
			0x00,
			0x00,
			0x00,
			0x90,
			0x90
		};
		static inline constexpr std::uint8_t res_patch[] = {
			0xB8,
			0x00,
			0x00,
			0x00,
			0x00,
			0x90
		};
		static inline constexpr std::uint8_t PhysCalcFix[] = {
			0x90,
			0x90,
			0x90,
			0x90
		};
		static inline constexpr std::uint8_t ResizeBuffersDisable[] = {
			0xE9,
			0x89,
			0x02,
			0x00,
			0x00,
			0x90
		};
		static inline constexpr std::uint8_t ResizeBuffersDisable_AE[] = {
			0xE9,
			0x45,
			0x04,
			0x00,
			0x00,
			0x90
		};
		static inline constexpr std::uint8_t ResizeTargetDisable[] = {
			0xE9,
			0x43,
			0x01,
			0x00,
			0x00,
			0x90
		};
		static inline constexpr std::uint8_t ResizeTargetDisable_AE[] = {
			0xE9,
			0x30,
			0x01,
			0x00,
			0x00,
			0x90
		};
	}

	namespace Structures
	{
		typedef struct __IDXGIDataSub
		{
			std::uint32_t unk0;
			std::uint32_t unk4;
			std::uint32_t ht;
			std::uint32_t unk12;
			std::uint32_t SizeW;
			std::uint32_t SizeH;
			std::uint32_t unk24;
			std::uint32_t unk28;
			std::uint32_t unk32;
			std::uint32_t unk36;
			std::uint32_t unk40;
			std::uint32_t unk44;
			std::uint32_t unk48;
			std::uint32_t unk52;
			std::uint32_t unk56;
			std::uint32_t unk60;
			std::uint32_t unk64;
			std::uint32_t unk68;
			std::uint32_t unk72;
			std::uint32_t unk76;
		} IDXGIDataSub;

		static_assert(sizeof(IDXGIDataSub) == 80);

		typedef struct __IDXGIData
		{
			std::uint32_t unk0;
			std::uint32_t reqRefreshRate;
			std::uint32_t ht;
			DXGI_RATIONAL tRefreshRate;
			std::uint32_t unk20;
			std::uint32_t unk24;
			std::uint32_t unk28;
			std::uint32_t unk32;
			std::uint32_t unk36;
			std::uint32_t unk40;
			std::uint32_t unk44;
			std::uint32_t _SyncInterval;
			std::uint32_t unk52;
			std::uint32_t unk56;
			std::uint32_t unk60;
			std::uint32_t unk64;
			std::uint32_t unk68;
			IDXGIDataSub unk72[32];
		} IDXGIData;

		static_assert(sizeof(IDXGIData) == 2632);

		class _SettingCollectionList
		{
		public:
			virtual void Unk_01();
			virtual void Unk_02();
			virtual void Unk_03();
			virtual void Unk_04();
			virtual void Unk_05();
			virtual void Unk_06();
			virtual void Unk_07();
			virtual void Unk_08();
			virtual void Unk_09();
			virtual void LoadINI();

			char inipath[260];  // MAX_PATH
			std::uint8_t pad10C[12];
			SettingCollectionList::ConfigEntry items;

			void LoadIni_Hook();
		};

		static_assert(offsetof(_SettingCollectionList, items) == 0x118);

		struct UnkCoordData
		{
			std::uint32_t unk00;      // 00
			NiPoint2 cursorPos;       // 04
			NiPoint2 topLeft;         // 0C
			NiPoint2 bottomRight;     // 14
			float cursorSensitivity;  // 1C
									  // ...
		};
	}
}
