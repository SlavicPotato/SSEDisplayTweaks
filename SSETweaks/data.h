#pragma once

#include <dxgicommon.h>

namespace SDT
{
    namespace AID
    {
        constexpr unsigned long long DXGIData = 524728;
        constexpr unsigned long long Present = 75461;
        constexpr unsigned long long RT0 = 35565;
        constexpr unsigned long long FMTProc = 35574;
        constexpr unsigned long long D3D11Create = 75589;
        constexpr unsigned long long D3D11CreateDeviceAndSwapChain_JMP = 102631;
        constexpr unsigned long long fMaxFrameCounterDifferenceToConsiderVisibleA = 517358;

        constexpr unsigned long long Init0 = 35548;
        constexpr unsigned long long LoadPluginINI = 35642;
        constexpr unsigned long long INIProc0 = 35652;

        constexpr unsigned long long SetExpressionOverride = 53926;

        constexpr unsigned long long INISettingsCollection = 524557;
        constexpr unsigned long long INISettingsCollection_offset1 = 22315;

        constexpr unsigned long long CalculateCRC32_64 = 66964;

        constexpr unsigned long long MenuManager = 514178;
        constexpr unsigned long long UIStringHolder = 514286;

        constexpr unsigned long long EventDispatcher_offset1 = 34381;
        constexpr unsigned long long EventDispatcher_offset2 = 28367;
        constexpr unsigned long long EventDispatcher_offset3 = 13805;

        constexpr unsigned long long WindowCreate = 75591;

        constexpr unsigned long long PhysCalcFix = 35605;
        constexpr unsigned long long PhysFuncUnk0 = 66987;

        constexpr unsigned long long D3DInit = 75595;

        constexpr unsigned long long PhysCalc = 76017;

        constexpr unsigned long long WindowSwapChainAdjust = 75453;
        constexpr unsigned long long WindowSwapChain2 = 75454;

        constexpr unsigned long long WinFunc0 = 75460;
        constexpr unsigned long long WinFunc1 = 75449;

        constexpr unsigned long long MT_Inject = 230155;

        constexpr unsigned long long UnkMovFunc0 = 40937;

        constexpr unsigned long long ScriptRunGame = 53117;
        constexpr unsigned long long ScriptRunUI = 53118;

        constexpr unsigned long long IsComplex = 516932;

        constexpr unsigned long long RDCImpl = 102238;

        constexpr unsigned long long FrameTimer = 523661;
        constexpr unsigned long long UnkFloat0 = 509776;
        constexpr unsigned long long UnkMM0 = 49805;

    }

    namespace Offsets
    {

        // indirect: 488B05????????4C8B098B503041FF5140 (mov rax, qword ptr ds:[?])  (524728)
        //constexpr uintptr_t DGXIData = 0x3025F00;

        //constexpr uintptr_t RTProc = 0xD69FF0;	

        // 40574883EC3048C7442420FEFFFFFF48895C2440488BD9488B0D????????E8????????B909000000+5E3  (35565)
        constexpr uintptr_t RTUnk0_GM_C = 0x5E3;
        // 40574883EC3048C7442420FEFFFFFF48895C2440488BD9488B0D????????E8????????B909000000+366  (35565)
        constexpr uintptr_t RTUnk0_UI_C = 0x366;
        // 4883EC280FB605????????488D0D????????33D28805????????3815????????0F4515????????E8????????  (35574)
        //constexpr uintptr_t RTUnk0_O = 0x5B3E40;

        // 40555356574156488DAC2440FFFFFF4881ECC001000048C74598FEFFFFFF8B0D????????65488B042558000000488B3CC841BE68070000418B1C3E899DF000000033F64189343E+632  (35548)
        constexpr uintptr_t LoadPluginINI_C = 0x632;
        // 40574883EC5048C7442420FEFFFFFF48895C2468488D4C2438E8????????90488D4C2448  (35642)
        //constexpr uintptr_t LoadPluginINI_O = 0x5B8F00;

        //constexpr uintptr_t JMPT_D3D11CreateDevice = 0x1372F02;

        // 40555357488DAC248092FFFFB8806E0000E8????????482BE0488D542460488D0D????????+2E3  (75589)
        constexpr uintptr_t D3D11CreateDeviceAndSwapChain_C = 0x2E3;

        constexpr uintptr_t CreateDXGIFactory_C = 0x25;

        //constexpr uintptr_t IDXGISwapChain_Present_C = 0xD6A027;

        // 35548
        constexpr uintptr_t bFullscreen_Patch = 0x82F;
        constexpr uintptr_t bBorderless_Patch = 0x83A;

        // 40555356574156488DAC2440FFFFFF4881ECC001000048C74598FEFFFFFF8B0D????????65488B042558000000488B3CC841BE68070000418B1C3E899DF000000033F64189343E+86D  (35548)
        constexpr uintptr_t DisplayRefreshRate = 0x86D;

        // 4C8BDC49895B1049896B184989732041564881EC30040000418D41FF458BF1498BD88BEA488BF183F80F  (53926)
        constexpr uintptr_t SetExpressionOverride_lea = 0x1A;
        constexpr uintptr_t SetExpressionOverride_cmp = 0x29;

        // 4883EC28B901000000E8????????803D????????007405E8????????488B05????????4533C0488B4818488B05????????4C8B098B5030  (75461)
        //constexpr uintptr_t PresentPatchTarget = 0xD6A2B0;
        constexpr uintptr_t Present_Limiter = 0xE;
        constexpr uintptr_t Present_Flags_Inject = 0x23;

        // 488BC456574154415641574881EC6002000048C7442430FEFFFFFF4889581048896820488BD9488D5008+153  (35652)
        constexpr uintptr_t SkipNoINI = 0x153;

        // 4C8BDC49895B0849897310574881ECC0000000488B510833F6+163  (75591)
        constexpr uintptr_t WindowCreate = 0x163;

        // 0F28D80F57D2F30F111D????????84D27529+0xE  (76017)
        constexpr uintptr_t PhysCalcFix = 0xE;

        // (35574)
        constexpr uintptr_t PhysCalcHT = 0x69;

        constexpr uintptr_t MaxFrameLatency = 0x91;

        constexpr uintptr_t ResizeBuffers_Inject = 0x217;

        constexpr uintptr_t GetClientRect1 = 0x192;

        constexpr uintptr_t ResizeBuffersDisable = 0x27;
        constexpr uintptr_t ResizeTargetDisable = 0x22;
        constexpr uintptr_t ResizeTarget = 0x10A;

        constexpr uintptr_t MT_Inject = 0x300;
        constexpr uintptr_t FMHS_Inject = 0xAD;

        constexpr uintptr_t ScriptUpdateBudgetGame = 0x90;
        constexpr uintptr_t ScriptUpdateBudgetUI = 0x90;

        constexpr uintptr_t Present = 0x34;
    }

    namespace Payloads
    {
        constexpr uint8_t SkipNoINI[] = { 0x4C, 0x89, 0xF1 };
        constexpr uint8_t seoFix_lea[] = { 0x01, 0x90 };
        constexpr uint8_t seoFix_cmp = 0x10;
        constexpr uint8_t nopx3[] = { 0x90, 0x90, 0x90 };
        constexpr uint8_t nopjmp[] = { 0x90, 0xEB };
        constexpr uint8_t bw_patch[] = {
            0xB8, 0x01, 0x00, 0x00, 0x00, 0x90, 0x90
        };
        constexpr uint8_t PhysCalcFix[] = {
            0x90, 0x90, 0x90, 0x90
        };
        constexpr uint8_t ResizeBuffersDisable[] = {
            0xE9, 0x89, 0x02, 0x00, 0x00, 0x90
        };
        constexpr uint8_t ResizeTargetDisable[] = {
            0xE9, 0x43, 0x01, 0x00, 0x00, 0x90
        };
    }

    namespace Structures
    {
        typedef struct __IDXGIDataSub
        {
            UInt32 unk0;
            UInt32 unk4;
            UInt32 ht;
            UInt32 unk12;
            UInt32 SizeW;
            UInt32 SizeH;
            UInt32 unk24;
            UInt32 unk28;
            UInt32 unk32;
            UInt32 unk36;
            UInt32 unk40;
            UInt32 unk44;
            UInt32 unk48;
            UInt32 unk52;
            UInt32 unk56;
            UInt32 unk60;
            UInt32 unk64;
            UInt32 unk68;
            UInt32 unk72;
            UInt32 unk76;
        } IDXGIDataSub;

        STATIC_ASSERT(sizeof(IDXGIDataSub) == 80);

        typedef struct __IDXGIData
        {
            UInt32 unk0;
            UInt32 reqRefreshRate;
            UInt32 ht;
            DXGI_RATIONAL tRefreshRate;
            UInt32 unk20;
            UInt32 unk24;
            UInt32 unk28;
            UInt32 unk32;
            UInt32 unk36;
            UInt32 unk40;
            UInt32 unk44;
            UInt32 SyncInterval;
            UInt32 unk52;
            UInt32 unk56;
            UInt32 unk60;
            UInt32 unk64;
            UInt32 unk68;
            IDXGIDataSub unk72[32];
        } IDXGIData;

        STATIC_ASSERT(sizeof(IDXGIData) == 2632);

        class _SettingCollectionList {
        public:
            virtual void Unk_01() = 0;
            virtual void Unk_02() = 0;
            virtual void Unk_03() = 0;
            virtual void Unk_04() = 0;
            virtual void Unk_05() = 0;
            virtual void Unk_06() = 0;
            virtual void Unk_07() = 0;
            virtual void Unk_08() = 0;
            virtual void Unk_09() = 0;
            virtual void LoadINI() = 0;

            char inipath[260]; // MAX_PATH
            uint8_t pad10C[12];
            SettingCollectionList::Entry items;

            void LoadIni_Hook();
        };

        STATIC_ASSERT(offsetof(_SettingCollectionList, items) == 0x118);
    }
}
