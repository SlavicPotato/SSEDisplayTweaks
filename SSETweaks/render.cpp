#include "pch.h"

using namespace std;
using namespace std::chrono;
using namespace Microsoft::WRL;

namespace SDT
{
    constexpr char* SECTION_GENERAL = "General";
    constexpr char* SECTION_RENDER = "Render";

    constexpr char* CKEY_FULLSCREEN = "Fullscreen";
    constexpr char* CKEY_BORDERLESS = "Borderless";
    constexpr char* CKEY_UPSCALE = "BorderlessUpscale";
    constexpr char* CKEY_DISABLEBUFFERRESIZE = "DisableBufferResizing";
    constexpr char* CKEY_DISABLETARGETRESIZE = "DisableTargetResizing";
    constexpr char* CKEY_VSYNC = "EnableVSync";
    constexpr char* CKEY_VSYNCPRESENTINT = "VSyncPresentInterval";
    constexpr char* CKEY_MAXRR = "MaximumRefreshRate";
    constexpr char* CKEY_BUFFERCOUNT = "SwapBufferCount";
    constexpr char* CKEY_SWAPEFFECT = "SwapEffect";
    constexpr char* CKEY_SCALINGMODE = "ScalingMode";
    constexpr char* CKEY_MAXFRAMELAT = "MaxFrameLatency";
    constexpr char* CKEY_ENABLETEARING = "EnableTearing";

    constexpr char* CKEY_FPSLIMIT = "FramerateLimit";

    constexpr char* CKEY_LOADSCRFPSLIMIT = "LoadingScreenFramerateLimit";
    constexpr char* CKEY_LOADSCRFPSLIMITEX = "LoadingScreenLimitExtraTime";
    constexpr char* CKEY_INITIALLOADLIMITEX = "LoadingScreenLimitExtraTimePostLoad";
    constexpr char* CKEY_UIFPSLIMIT = "UIFramerateLimit";
    constexpr char* CKEY_UIMAPFPSLIMIT = "UIFramerateLimitMap";
    constexpr char* CKEY_UIINVFPSLIMIT = "UIFramerateLimitInventory";
    constexpr char* CKEY_UIJOURFPSLIMIT = "UIFramerateLimitJournal";
    constexpr char* CKEY_UICUSTOMFPSLIMIT = "UIFramerateLimitCustom";
    constexpr char* CKEY_UIMAINFPSLIMIT = "UIFramerateLimitMain";
    constexpr char* CKEY_UIRACEFPSLIMIT = "UIFramerateLimitRace";
    constexpr char* CKEY_UIPERKFPSLIMIT = "UIFramerateLimitPerk";
    constexpr char* CKEY_UIBOOKFPSLIMIT = "UIFramerateLimitBook";
    constexpr char* CKEY_UILOCKFPSLIMIT = "UIFramerateLimitLockpick";
    constexpr char* CKEY_UICONSOLEFPSLIMIT = "UIFramerateLimitConsole";
    constexpr char* CKEY_UITWEENFPSLIMIT = "UIFramerateLimitTween";
    constexpr char* CKEY_UISWFPSLIMIT = "UIFramerateLimitSleepWait";

    constexpr char* CKEY_LOADSCRFPSLIMIT_DV = "LoadingScreenFramerateLimitVSyncOff";
    constexpr char* CKEY_UIFPSLIMIT_DV = "UIFramerateLimitVSyncOff";
    constexpr char* CKEY_UIMAPFPSLIMIT_DV = "UIFramerateLimitMapVSyncOff";
    constexpr char* CKEY_UIINVFPSLIMIT_DV = "UIFramerateLimitInventoryVSyncOff";
    constexpr char* CKEY_UIJOURFPSLIMIT_DV = "UIFramerateLimitJournalVSyncOff";
    constexpr char* CKEY_UICUSTOMFPSLIMIT_DV = "UIFramerateLimitCustomVSyncOff";
    constexpr char* CKEY_UIMAINFPSLIMIT_DV = "UIFramerateLimitMainVSyncOff";
    constexpr char* CKEY_UIRACEFPSLIMIT_DV = "UIFramerateLimitRaceVSyncOff";
    constexpr char* CKEY_UIPERKFPSLIMIT_DV = "UIFramerateLimitPerkVSyncOff";
    constexpr char* CKEY_UIBOOKFPSLIMIT_DV = "UIFramerateLimitBookVSyncOff";
    constexpr char* CKEY_UILOCKFPSLIMIT_DV = "UIFramerateLimitLockpickVSyncOff";
    constexpr char* CKEY_UICONSOLEFPSLIMIT_DV = "UIFramerateLimitConsoleVSyncOff";
    constexpr char* CKEY_UITWEENFPSLIMIT_DV = "UIFramerateLimitTweenVSyncOff";
    constexpr char* CKEY_UISWFPSLIMIT_DV = "UIFramerateLimitSleepWaitVSyncOff";

    constexpr char* CKEY_ADJUSTINICFG = "AdjustGameSettings";

    using namespace Structures;
    using namespace Patching;

    PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN DRender::D3D11CreateDeviceAndSwapChain_O;
    DRender::CreateDXGIFactory_T DRender::CreateDXGIFactory_O;

    DRender DRender::m_Instance;

    DRender::SEMap DRender::cfgSwapEffectMap = {
        {"auto", -1},
        {"discard", 0},
        {"sequential", 1},
        {"flip_sequential", 2},
        {"flip_discard", 3}
    };

    DRender::SE2Map DRender::cfgSEtoSEMap = {
        {0, DXGI_SWAP_EFFECT_DISCARD},
        {1, DXGI_SWAP_EFFECT_SEQUENTIAL},
        {2, DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL},
        {3, DXGI_SWAP_EFFECT_FLIP_DISCARD}
    };

    DRender::SMMap DRender::cfgScalingModeMap = {
        {"unspecified", DXGI_MODE_SCALING_UNSPECIFIED},
        {"centered", DXGI_MODE_SCALING_CENTERED},
        {"stretched", DXGI_MODE_SCALING_STRETCHED}
    };

    DRender::SEMapR DRender::cfgSwapEffectMapR = {
        { DXGI_SWAP_EFFECT_DISCARD, "discard"},
        { DXGI_SWAP_EFFECT_SEQUENTIAL, "sequential"},
        { DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL, "flip_sequential"},
        { DXGI_SWAP_EFFECT_FLIP_DISCARD, "flip_discard"}
    };

    DRender::MCLDMap DRender::menuCodeToLimitDesc = {
        {OnAnyMenu, "UI"},
        {OnLoadingMenu, "Loading screen"},
        {OnInventoryMenu, "Inventory"},
        {OnMagicMenu, "Magic"},
        {OnGiftMenu, "Gift"},
        {OnBarterMenu, "Barter"},
        {OnContainerMenu, "Container"},
        {OnJournalMenu, "Journal"},
        {OnMapMenu, "Map"},
        {OnCustomMenu, "Custom"},
        {OnMainMenu, "Main"},
        {OnRaceSexMenu, "Race"},
        {OnStatsMenu, "Perk"},
        {OnBookMenu, "Book"},
        {OnLockpickingMenu, "Lockpicking"},
        {OnConsoleMenu, "Console"},
        {OnTweenMenu, "Tween"},
        {OnSleepWaitMenu, "Sleep/Wait"},
        {OnFavoritesMenu, "Favorites"},
    };

    void MenuFramerateLimit::SetLimit(MenuEvent code, float limit, bool disable_vsync)
    {
        if (limit <= 0.0f) {
            m_limits[code] = MenuFramerateLimitDescriptor(0, disable_vsync);
        }
        else {
            m_limits[code] = MenuFramerateLimitDescriptor(
                static_cast<long long>((1.0f / limit) * 1000000.0f),
                disable_vsync
            );
        }
    }

    bool MenuFramerateLimit::GetLimit(MenuEvent code, MenuFramerateLimitDescriptor& limit)
    {
        auto it = m_limits.find(code);
        if (it != m_limits.end()) {
            limit = it->second;
            return true;
        }
        return false;
    }

    bool MenuFramerateLimit::HasLimits()
    {
        return m_limits.size() > 0;
    }

    bool MenuFramerateLimit::HasLimit(MenuEvent code)
    {
        return m_limits.find(code) != m_limits.end();
    }

    bool MenuFramerateLimit::GetCurrentLimit(MenuFramerateLimitDescriptor& limit)
    {
        if (!m_limits.size()) {
            return false;
        }

        if (m_stack.size()) {
            auto it = m_limits.find(m_stack.back());
            if (it != m_limits.end()) {
                limit = it->second;
                return true;
            }
        }

        auto it = m_limits.find(OnAnyMenu);
        if (it != m_limits.end()) {
            limit = it->second;
            return true;
        }

        return false;
    }

    DRender::DRender() :
        fps_max(0), current_fps_max(0),
        has_swap_effect(false), limiter_installed(false),
        tearing_enabled(false), has_fl_override(false),
        fps_limit(-1), present_flags(0),
        lslExtraTime(0), lslPostLoadExtraTime(0),
        oo_expire_time(0), oo_current_fps_max(0),
        gameLoadState(0)
    {
        swapchain.flags = 0;
        swapchain.width = 0;
        swapchain.height = 0;

        m_uifl.SetTracked(
            MenuEventTrack::TrackMap({
                {OnJournalMenu, true},
                {OnMainMenu, true},
                {OnMapMenu, true},
                {OnConsoleMenu, true},
                {OnCustomMenu, true},
                {OnRaceSexMenu, true},
                {OnInventoryMenu, true},
                {OnContainerMenu, true},
                {OnMagicMenu, true},
                {OnTweenMenu, true},
                {OnLoadingMenu, true},
                {OnMessageBoxMenu, true},
                {OnLockpickingMenu, true},
                {OnGiftMenu, true},
                {OnTrainingMenu, true},
                {OnTutorialMenu, true},
                {OnLevelUpMenu, true},
                {OnBarterMenu, true},
                {OnStatsMenu, true},
                {OnFavoritesMenu, true},
                {OnSleepWaitMenu, true},
                {OnQuantityMenu, true},
                {OnModManagerMenu, true},
                {OnBookMenu, true},
                }));
    }

    void DRender::LoadConfig()
    {
        conf.fullscreen = static_cast<uint8_t>(GetConfigValue(SECTION_RENDER, CKEY_FULLSCREEN, false));
        conf.borderless = static_cast<uint8_t>(GetConfigValue(SECTION_RENDER, CKEY_BORDERLESS, true));
        conf.upscale = GetConfigValue(SECTION_RENDER, CKEY_UPSCALE, false);
        conf.disablebufferresize = GetConfigValue(SECTION_RENDER, CKEY_DISABLEBUFFERRESIZE, false);
        conf.disabletargetresize = GetConfigValue(SECTION_RENDER, CKEY_DISABLETARGETRESIZE, false);

        conf.vsync_on = GetConfigValue(SECTION_RENDER, CKEY_VSYNC, true);
        conf.vsync_present_interval = clamp<uint32_t>(GetConfigValue<uint32_t>(SECTION_RENDER, CKEY_VSYNCPRESENTINT, 1), 1, 4);
        conf.max_rr = GetConfigValue(SECTION_RENDER, CKEY_MAXRR, 0);
        has_swap_effect = ConfigTranslateSwapEffect(
            GetConfigValue(SECTION_RENDER, CKEY_SWAPEFFECT, "auto"),
            conf.swap_effect
        );
        has_scaling_mode = ConfigTranslateScalingMode(
            GetConfigValue(SECTION_RENDER, CKEY_SCALINGMODE, "default"),
            conf.scaling_mode
        );
        conf.buffer_count = GetConfigValue(SECTION_RENDER, CKEY_BUFFERCOUNT, 0);
        if (conf.buffer_count > -1) {
            conf.buffer_count = clamp<uint32_t>(conf.buffer_count, 0, 8);
        }
        conf.max_frame_latency = GetConfigValue(SECTION_RENDER, CKEY_MAXFRAMELAT, 0);
        if (conf.max_frame_latency > 0) {
            conf.max_frame_latency = min(conf.max_frame_latency, 16);
        }
        conf.enable_tearing = GetConfigValue(SECTION_RENDER, CKEY_ENABLETEARING, true);

        conf.limits.game = GetConfigValue(SECTION_RENDER, CKEY_FPSLIMIT, -1.0f);

        conf.limits.ui = GetConfigValue(SECTION_RENDER, CKEY_UIFPSLIMIT, 0.0f);
        conf.limits.ui_map = GetConfigValue(SECTION_RENDER, CKEY_UIMAPFPSLIMIT, 0.0f);
        conf.limits.ui_inventory = GetConfigValue(SECTION_RENDER, CKEY_UIINVFPSLIMIT, 0.0f);
        conf.limits.ui_journal = GetConfigValue(SECTION_RENDER, CKEY_UIJOURFPSLIMIT, 0.0f);
        conf.limits.ui_custom = GetConfigValue(SECTION_RENDER, CKEY_UICUSTOMFPSLIMIT, 0.0f);
        conf.limits.ui_main = GetConfigValue(SECTION_RENDER, CKEY_UIMAINFPSLIMIT, 0.0f);
        conf.limits.ui_race = GetConfigValue(SECTION_RENDER, CKEY_UIRACEFPSLIMIT, 0.0f);
        conf.limits.ui_perk = GetConfigValue(SECTION_RENDER, CKEY_UIPERKFPSLIMIT, 0.0f);
        conf.limits.ui_book = GetConfigValue(SECTION_RENDER, CKEY_UIBOOKFPSLIMIT, 0.0f);
        conf.limits.ui_lockpick = GetConfigValue(SECTION_RENDER, CKEY_UILOCKFPSLIMIT, 0.0f);
        conf.limits.ui_loadscreen = GetConfigValue(SECTION_RENDER, CKEY_LOADSCRFPSLIMIT, 0.0f);
        conf.limits.ui_console = GetConfigValue(SECTION_RENDER, CKEY_UICONSOLEFPSLIMIT, 0.0f);
        conf.limits.ui_tween = GetConfigValue(SECTION_RENDER, CKEY_UITWEENFPSLIMIT, 0.0f);
        conf.limits.ui_sw = GetConfigValue(SECTION_RENDER, CKEY_UISWFPSLIMIT, 0.0f);

        conf.limits.ui_dv = GetConfigValue(SECTION_RENDER, CKEY_UIFPSLIMIT_DV, true);
        conf.limits.ui_map_dv = GetConfigValue(SECTION_RENDER, CKEY_UIMAPFPSLIMIT_DV, true);
        conf.limits.ui_inventory_dv = GetConfigValue(SECTION_RENDER, CKEY_UIINVFPSLIMIT_DV, true);
        conf.limits.ui_journal_dv = GetConfigValue(SECTION_RENDER, CKEY_UIJOURFPSLIMIT_DV, true);
        conf.limits.ui_custom_dv = GetConfigValue(SECTION_RENDER, CKEY_UICUSTOMFPSLIMIT_DV, true);
        conf.limits.ui_main_dv = GetConfigValue(SECTION_RENDER, CKEY_UIMAINFPSLIMIT_DV, true);
        conf.limits.ui_race_dv = GetConfigValue(SECTION_RENDER, CKEY_UIRACEFPSLIMIT_DV, true);
        conf.limits.ui_perk_dv = GetConfigValue(SECTION_RENDER, CKEY_UIPERKFPSLIMIT_DV, true);
        conf.limits.ui_book_dv = GetConfigValue(SECTION_RENDER, CKEY_UIBOOKFPSLIMIT_DV, true);
        conf.limits.ui_lockpick_dv = GetConfigValue(SECTION_RENDER, CKEY_UILOCKFPSLIMIT_DV, true);
        conf.limits.ui_loadscreen_dv = GetConfigValue(SECTION_RENDER, CKEY_LOADSCRFPSLIMIT_DV, true);
        conf.limits.ui_console_dv = GetConfigValue(SECTION_RENDER, CKEY_UICONSOLEFPSLIMIT_DV, true);
        conf.limits.ui_tween_dv = GetConfigValue(SECTION_RENDER, CKEY_UITWEENFPSLIMIT_DV, true);
        conf.limits.ui_sw_dv = GetConfigValue(SECTION_RENDER, CKEY_UISWFPSLIMIT_DV, true);

        conf.limits.ui_loadscreenex = clamp<int32_t>(GetConfigValue<int32_t>(SECTION_RENDER, CKEY_LOADSCRFPSLIMITEX, 0), 0, 15);
        conf.limits.ui_initialloadex = clamp<int32_t>(GetConfigValue<int32_t>(SECTION_RENDER, CKEY_INITIALLOADLIMITEX, 4), 0, 30);

        conf.adjust_ini = GetConfigValue(SECTION_GENERAL, CKEY_ADJUSTINICFG, true);
    }

    bool DRender::ConfigTranslateSwapEffect(string& param, int& out)
    {
        auto it = cfgSwapEffectMap.find(param);
        if (it != cfgSwapEffectMap.end()) {
            out = it->second;
            return true;
        }
        else {
            return false;
        }
    }

    bool DRender::ConfigTranslateScalingMode(string& param, DXGI_MODE_SCALING& out)
    {
        auto it = cfgScalingModeMap.find(param);
        if (it != cfgScalingModeMap.end()) {
            out = it->second;
            return true;
        }
        else {
            return false;
        }
    }

    string DRender::SwapEffectToConfigKey(DXGI_SWAP_EFFECT param)
    {
        auto it = cfgSwapEffectMapR.find(param);
        if (it != cfgSwapEffectMapR.end()) {
            return it->second;
        }
        else {
            return "unknown";
        }
    }

    void DRender::UISetLimit(MenuEvent code, float limit, bool disable_vsync)
    {
        if (limit > 0.0f) {
            m_uifl.SetLimit(code, limit, disable_vsync);
            Message("Framerate limit (%s): %.6g (VSync off: %s)",
                menuCodeToLimitDesc[code].c_str(), limit, disable_vsync ? "true" : "false");
        }
        else if (limit < 0.0f) {
            m_uifl.SetLimit(code, 0.0f, disable_vsync);
            Message("Framerate limit (%s): disabled (VSync off: %s)",
                menuCodeToLimitDesc[code].c_str(), disable_vsync ? "true" : "false");
        }
    }

    void DRender::PostLoadConfig()
    {
        tts = PerfCounter::Query();

        if (conf.upscale) {
            if (!(!conf.fullscreen && conf.borderless))
            {
                conf.upscale = false;
            }
            else {
                Message("Borderless upscaling enabled");
            }
        }

        vsync = conf.vsync_on ? conf.vsync_present_interval : 0;

        if (conf.limits.game > 0.0f) {
            current_fps_max = fps_max = static_cast<long long>((1.0f / conf.limits.game) * 1000000.0f);
            fps_limit = 1;
            Message("Framerate limit (game): %.6g", conf.limits.game);
        }
        else if (conf.limits.game == 0.0f) {
            fps_limit = 0;
        }
        else {
            fps_limit = -1;
        }

        UISetLimit(OnAnyMenu, conf.limits.ui, conf.limits.ui_dv);
        UISetLimit(OnLoadingMenu, conf.limits.ui_loadscreen, conf.limits.ui_loadscreen_dv);

        if (m_uifl.HasLimit(OnLoadingMenu)) {
            if (conf.limits.ui_loadscreenex > 0) {
                lslExtraTime = PerfCounter::T(
                    conf.limits.ui_loadscreenex * 1000000);
            }
            if (conf.limits.ui_initialloadex > 0) {
                lslPostLoadExtraTime = PerfCounter::T(
                    conf.limits.ui_initialloadex * 1000000);
            }
        }

        if (conf.limits.ui_inventory != 0.0f) {
            UISetLimit(OnInventoryMenu, conf.limits.ui_inventory, conf.limits.ui_inventory_dv);
            UISetLimit(OnMagicMenu, conf.limits.ui_inventory, conf.limits.ui_inventory_dv);
            UISetLimit(OnGiftMenu, conf.limits.ui_inventory, conf.limits.ui_inventory_dv);
            UISetLimit(OnBarterMenu, conf.limits.ui_inventory, conf.limits.ui_inventory_dv);
            UISetLimit(OnContainerMenu, conf.limits.ui_inventory, conf.limits.ui_inventory_dv);
            UISetLimit(OnFavoritesMenu, conf.limits.ui_inventory, conf.limits.ui_inventory_dv);
        }

        UISetLimit(OnJournalMenu, conf.limits.ui_journal, conf.limits.ui_journal_dv);
        UISetLimit(OnMapMenu, conf.limits.ui_map, conf.limits.ui_map_dv);
        UISetLimit(OnCustomMenu, conf.limits.ui_custom, conf.limits.ui_custom_dv);
        UISetLimit(OnMainMenu, conf.limits.ui_main, conf.limits.ui_main_dv);
        UISetLimit(OnRaceSexMenu, conf.limits.ui_race, conf.limits.ui_race_dv);
        UISetLimit(OnStatsMenu, conf.limits.ui_perk, conf.limits.ui_perk_dv);
        UISetLimit(OnBookMenu, conf.limits.ui_book, conf.limits.ui_book_dv);
        UISetLimit(OnLockpickingMenu, conf.limits.ui_lockpick, conf.limits.ui_lockpick_dv);
        UISetLimit(OnConsoleMenu, conf.limits.ui_console, conf.limits.ui_console_dv);
        UISetLimit(OnTweenMenu, conf.limits.ui_tween, conf.limits.ui_tween_dv);
        UISetLimit(OnSleepWaitMenu, conf.limits.ui_sw, conf.limits.ui_sw_dv);
    }

    void DRender::Patch()
    {
        if (conf.max_rr > 0) {
            safe_write(
                DisplayRefreshRate + 0x4,
                static_cast<uint32_t>(conf.max_rr)
            );
            Message("Refresh rate patch applied (max %d Hz)", conf.max_rr);
        }

        safe_write(
            bBorderless_Patch,
            reinterpret_cast<const void*>(Payloads::bw_patch),
            sizeof(Payloads::bw_patch)
        );
        safe_write(
            bBorderless_Patch + 0x1,
            static_cast<uint32_t>(m_Instance.conf.borderless)
        );

        safe_write(
            bFullscreen_Patch,
            reinterpret_cast<const void*>(Payloads::bw_patch),
            sizeof(Payloads::bw_patch)
        );
        safe_write(
            bFullscreen_Patch + 0x1,
            static_cast<uint32_t>(m_Instance.conf.fullscreen)
        );

        if (HasLimits()) {
            safe_write(
                Present_Limiter + 0x6,
                reinterpret_cast<const void*>(Payloads::nopjmp),
                sizeof(Payloads::nopjmp)
            );

            limiter_installed = true;
        }

        if (conf.max_frame_latency > 0) {
            safe_write(
                MaxFrameLatency,
                static_cast<uint32_t>(conf.max_frame_latency)
            );
            Message("Maximum frame latency: %d", conf.max_frame_latency);
        }

        if (!conf.fullscreen) {
            if (conf.disablebufferresize) {
                safe_write(
                    ResizeBuffersDisable,
                    reinterpret_cast<const void*>(Payloads::ResizeBuffersDisable),
                    sizeof(Payloads::ResizeBuffersDisable)
                );
                Debug("Disabled swap chain buffer resizing");
            }

            if (conf.disabletargetresize) {
                safe_write(
                    ResizeTargetDisable,
                    reinterpret_cast<const void*>(Payloads::ResizeTargetDisable),
                    sizeof(Payloads::ResizeTargetDisable)
                );

                Debug("Disabled swap chain target resizing");
            }
        }
        else {
            struct ResizeTargetInjectArgs : JITASM {
                ResizeTargetInjectArgs(uintptr_t retnAddr, uintptr_t mdescAddr
                ) : JITASM()
                {
                    Xbyak::Label mdescLabel;
                    Xbyak::Label retnLabel;

                    mov(rdx, ptr[rip + mdescLabel]);
                    mov(rax, ptr[rcx]);
                    jmp(ptr[rip + retnLabel]);

                    L(retnLabel);
                    dq(retnAddr);

                    L(mdescLabel);
                    dq(mdescAddr);
                }
            };

            LogPatchBegin("IDXGISwapChain::ResizeTarget");
            {
                ResizeTargetInjectArgs code(
                    ResizeTarget + 0x8,
                    uintptr_t(&modeDesc));
                g_branchTrampoline.Write6Branch(
                    ResizeTarget, code.get());
                safe_memset(ResizeTarget + 0x6, 0xCC, 0x2);
            }
            LogPatchEnd("IDXGISwapChain::ResizeTarget");
        }

        if (conf.fullscreen == 0 && conf.enable_tearing
            && (!conf.vsync_on || limiter_installed))
        {
            struct PresentFlagsInject : JITASM {
                PresentFlagsInject(uintptr_t retnAddr, uintptr_t flagsAddr)
                    : JITASM()
                {
                    Xbyak::Label flagsLabel;
                    Xbyak::Label retnLabel;

                    mov(rcx, ptr[rip + flagsLabel]);
                    mov(r8d, ptr[rcx]);
                    mov(rcx, ptr[rax + 0x18]);
                    jmp(ptr[rip + retnLabel]);

                    L(retnLabel);
                    dq(retnAddr);

                    L(flagsLabel);
                    dq(flagsAddr);
                }
            };

            LogPatchBegin("IDXGISwapChain::Present");
            {
                PresentFlagsInject code(
                    Present_Flags_Inject + 0x7,
                    uintptr_t(&present_flags));

                g_branchTrampoline.Write6Branch(
                    Present_Flags_Inject, code.get());

                safe_write<uint8_t>(Present_Flags_Inject + 0x6, 0xCC);
            }
            LogPatchEnd("IDXGISwapChain::Present");
        }

        {
            struct ResizeBuffersInjectArgs : JITASM {
                ResizeBuffersInjectArgs(uintptr_t retnAddr, uintptr_t swdAddr
                ) : JITASM()
                {
                    Xbyak::Label retnLabel;
                    Xbyak::Label bdLabel;

                    mov(rdx, ptr[rip + bdLabel]);
                    mov(r8d, ptr[rdx]);
                    mov(r9d, ptr[rdx + 4]);
                    mov(eax, ptr[rdx + 8]);
                    mov(ptr[rsp + 0x28], eax);

                    mov(dword[rsp + 0x20], DXGI_FORMAT_UNKNOWN);

                    xor (edx, edx);

                    jmp(ptr[rip + retnLabel]);

                    L(retnLabel);
                    dq(retnAddr);

                    L(bdLabel);
                    dq(swdAddr);
                }
            };

            LogPatchBegin("IDXGISwapChain::ResizeBuffers");
            {
                ResizeBuffersInjectArgs code(
                    ResizeBuffers_Inject + 0x18,
                    uintptr_t(&swapchain));

                g_branchTrampoline.Write6Branch(
                    ResizeBuffers_Inject, code.get());

                safe_memset(ResizeBuffers_Inject + 0x6, 0xCC, 0x12);
            }
            LogPatchEnd("IDXGISwapChain::ResizeBuffers");
        }
    }

    void DRender::RegisterHooks()
    {
        bool isHooked = false;

        if (Hook::Call5(CreateDXGIFactory_C, reinterpret_cast<uintptr_t>(CreateDXGIFactory_Hook), CreateDXGIFactory_O)) {
            if (Hook::Call5(D3D11CreateDeviceAndSwapChain_C, reinterpret_cast<uintptr_t>(D3D11CreateDeviceAndSwapChain_Hook), D3D11CreateDeviceAndSwapChain_O)) {
                isHooked = true; 
            }
            else {
                Error("D3D11CreateDeviceAndSwapChain hook failed");
            }
        }
        else {
            Error("CreateDXGIFactory hook failed");
        }

        if (!isHooked) {
            SetOK(false);
            return;
        }

        if (HasLimits()) {
            RegisterHook(
                Present_Limiter,
                reinterpret_cast<uintptr_t>(Throttle),
                HookDescriptor::kWR6Call
            );
        }

        if (m_uifl.HasLimits()) {
            IEvents::RegisterForEvent(MenuEvent::OnAnyMenu, OnMenuEvent);
            if (lslPostLoadExtraTime > 0) {
                IEvents::RegisterForEvent(Event::OnMessage, MessageHandler);
            }
        }

        IEvents::RegisterForEvent(Event::OnConfigLoad, OnConfigLoad);
    }

    bool DRender::Prepare()
    {
        bLockFramerate = ISKSE::GetINISettingAddr<uint8_t*>("bLockFramerate:Display");
        if (!bLockFramerate) {
            return false;
        }

        iFPSClamp = ISKSE::GetINISettingAddr<int32_t*>("iFPSClamp:General");
        if (!iFPSClamp) {
            return false;
        }

        return true;
    }

    void DRender::MessageHandler(Event m_code, void* args)
    {
        auto message = reinterpret_cast<SKSEMessagingInterface::Message*>(args);

        switch (message->type)
        {
        case SKSEMessagingInterface::kMessage_NewGame:
        case SKSEMessagingInterface::kMessage_PostLoadGame:
            if (m_Instance.gameLoadState == 0) {
                m_Instance.gameLoadState = 1;
            }
            break;
        case SKSEMessagingInterface::kMessage_PreLoadGame:
            m_Instance.gameLoadState = 0;
            break;
        }
    }

    void DRender::OnConfigLoad(Event m_code, void* args)
    {
        if (m_Instance.limiter_installed || m_Instance.fps_limit == 0) {
            *m_Instance.bLockFramerate = 0;
        }

        if (m_Instance.conf.adjust_ini) {
            if (*m_Instance.iFPSClamp != 0)
            {
                m_Instance.Message("Setting iFPSClamp=0");
                *m_Instance.iFPSClamp = 0;
            }
        }
    }

    void DRender::SetFPSLimitOverride(long long max, bool disable_vsync)
    {
        if (conf.vsync_on) {
            if (disable_vsync) {
                (*DXGIData)->SyncInterval = 0;
                if (tearing_enabled) {
                    present_flags |= DXGI_PRESENT_ALLOW_TEARING;
                }
            }
            else {
                (*DXGIData)->SyncInterval = vsync;
                if (tearing_enabled) {
                    present_flags &= ~DXGI_PRESENT_ALLOW_TEARING;
                }
            }
        }

        current_fps_max = max;
        has_fl_override = true;
    }

    void DRender::ResetFPSLimitOverride()
    {
        if (!has_fl_override) {
            return;
        }

        if (conf.vsync_on) {
            (*DXGIData)->SyncInterval = vsync;
            if (tearing_enabled) {
                present_flags &= ~DXGI_PRESENT_ALLOW_TEARING;
            }
        }

        current_fps_max = fps_max;
        has_fl_override = false;
    }

    bool DRender::OnMenuEvent(MenuEvent code, MenuOpenCloseEvent* evn, EventDispatcher<MenuOpenCloseEvent>*)
    {
        auto& fl = m_Instance.m_uifl;
        MenuFramerateLimitDescriptor ld;

        auto mm = MenuManager::GetSingleton();
        if (mm->InPausedMenu())
        {
            fl.Track(code, evn->opening);

            if (fl.GetCurrentLimit(ld)) {
                m_Instance.SetFPSLimitOverride(ld.limit, ld.disable_vsync);
            }
            else {
                m_Instance.ResetFPSLimitOverride();
            }
        }
        else
        {
            fl.ClearTracked();
            m_Instance.ResetFPSLimitOverride();
        }

        if (m_Instance.lslExtraTime > 0 || m_Instance.lslPostLoadExtraTime > 0) {
            if (code == MenuEvent::OnLoadingMenu && evn->opening == false) {
                if (fl.GetLimit(MenuEvent::OnLoadingMenu, ld)) {
                    if (m_Instance.lslPostLoadExtraTime > 0 && m_Instance.gameLoadState == 1) {
                        m_Instance.gameLoadState = 2;
                        m_Instance.oo_expire_time = PerfCounter::Query() + m_Instance.lslPostLoadExtraTime;
                        m_Instance.oo_current_fps_max = ld.limit;
                    }
                    else if (m_Instance.lslPostLoadExtraTime > 0) {
                        m_Instance.oo_expire_time = PerfCounter::Query() + m_Instance.lslExtraTime;
                        m_Instance.oo_current_fps_max = ld.limit;
                    }
                }
            }
        }

        return true;
    }

    void DRender::Throttle()
    {
        auto e = PerfCounter::Query();

        long long limit;
        if (m_Instance.oo_expire_time > 0) {
            if (e < m_Instance.oo_expire_time) {
                limit = m_Instance.oo_current_fps_max;
            }
            else {
                m_Instance.oo_expire_time = 0;
                limit = m_Instance.current_fps_max;
            }
        }
        else {
            limit = m_Instance.current_fps_max;
        }

        if (limit > 0) {
            while (PerfCounter::delta_us(m_Instance.tts, e) < limit)
            {
                ::Sleep(0);
                e = PerfCounter::Query();
            }
        }
        m_Instance.tts = e;
    }

    bool DRender::ValidateDisplayMode(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc) const
    {
        if (pSwapChainDesc->BufferDesc.RefreshRate.Numerator > 0 &&
            !pSwapChainDesc->BufferDesc.RefreshRate.Denominator) {
            return false;
        }

        return true;
    }

    UINT DRender::GetRefreshRate(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc) const
    {
        if (!pSwapChainDesc->BufferDesc.RefreshRate.Denominator) {
            return 0U;
        }
        return pSwapChainDesc->BufferDesc.RefreshRate.Numerator /
            pSwapChainDesc->BufferDesc.RefreshRate.Denominator;
    }

    float DRender::GetMaxFramerate(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc) const
    {
        float maxt = 0.0f;

        if (conf.vsync_on && pSwapChainDesc->Windowed == FALSE) {
            maxt = static_cast<float>(GetRefreshRate(pSwapChainDesc));
        }

        if (fps_limit == 1) {
            if (!(conf.vsync_on && pSwapChainDesc->Windowed == FALSE) ||
                conf.limits.game < maxt)
            {
                maxt = conf.limits.game;
            }
        }

        return maxt;
    }

    DXGI_SWAP_EFFECT DRender::AutoGetSwapEffect(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc) const
    {
        if (pSwapChainDesc->Windowed == TRUE) {
            if (dxgi.caps & DXGI_CAP_FLIP_DISCARD) {
                return DXGI_SWAP_EFFECT_FLIP_DISCARD;
            }
            else if (dxgi.caps & DXGI_CAP_FLIP_SEQUENTIAL) {
                return DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
            }
        }
        return DXGI_SWAP_EFFECT_DISCARD;
    }


    DXGI_SWAP_EFFECT DRender::ManualGetSwapEffect(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc)
    {
        DXGI_SWAP_EFFECT se = cfgSEtoSEMap[conf.swap_effect];

        if (pSwapChainDesc->Windowed == TRUE) {
            DXGI_SWAP_EFFECT nse = se;

            if (nse == DXGI_SWAP_EFFECT_FLIP_DISCARD &&
                !(dxgi.caps & DXGI_CAP_FLIP_DISCARD))
            {
                nse = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
            }

            if (nse == DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL &&
                !(dxgi.caps & DXGI_CAP_FLIP_SEQUENTIAL))
            {
                nse = DXGI_SWAP_EFFECT_DISCARD;
            }

            if (nse != se) {
                Warning("%s not supported, using %s",
                    SwapEffectToConfigKey(se).c_str(),
                    SwapEffectToConfigKey(nse).c_str()
                );
                se = nse;
            }
        }

        return se;
    }

    void DRender::ApplyD3DSettings(DXGI_SWAP_CHAIN_DESC* pSwapChainDesc)
    {
        (*DXGIData)->SyncInterval = static_cast<UInt32>(vsync);

        if (pSwapChainDesc->Windowed == TRUE) {
            DXGI_GetCapabilities();
        }

        if (has_swap_effect) {
            if (conf.swap_effect == -1) {
                pSwapChainDesc->SwapEffect = AutoGetSwapEffect(pSwapChainDesc);
            }
            else {
                pSwapChainDesc->SwapEffect = ManualGetSwapEffect(pSwapChainDesc);
            }
        }

        bool flip_model = IsFlipOn(pSwapChainDesc);

        if (pSwapChainDesc->Windowed == TRUE)
        {
            if (conf.enable_tearing && flip_model &&
                (!conf.vsync_on || limiter_installed))
            {
                if (dxgi.caps & DXGI_CAP_TEARING) {
                    pSwapChainDesc->Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

                    if (!conf.vsync_on) {
                        present_flags |= DXGI_PRESENT_ALLOW_TEARING;
                    }

                    tearing_enabled = true;
                }
                else {
                    Warning("DXGI_FEATURE_PRESENT_ALLOW_TEARING not supported");
                }
            }
        }
        else {
            if (has_scaling_mode) {
                pSwapChainDesc->BufferDesc.Scaling = conf.scaling_mode;
            }
        }

        if (conf.buffer_count == 0) {
            if (flip_model) {
                pSwapChainDesc->BufferCount = 3;
            }
            else {
                pSwapChainDesc->BufferCount = 2;
            }
        }
        else if (conf.buffer_count > 0) {
            pSwapChainDesc->BufferCount = static_cast<UINT>(conf.buffer_count);
        }

        if (flip_model) {
            pSwapChainDesc->SampleDesc.Count = 1;
            pSwapChainDesc->SampleDesc.Quality = 0;

            if (pSwapChainDesc->BufferCount < 2) {
                pSwapChainDesc->BufferCount = 2;
                Warning("Buffer count below the minimum required for flip model, increasing to 2");
            }
        }

        if (conf.upscale) {
            swapchain.width = pSwapChainDesc->BufferDesc.Width;
            swapchain.height = pSwapChainDesc->BufferDesc.Height;
        }

        modeDesc = pSwapChainDesc->BufferDesc;

        if (pSwapChainDesc->Windowed == FALSE) {
            modeDesc.Format = DXGI_FORMAT_UNKNOWN;
        }

        swapchain.flags = pSwapChainDesc->Flags;
    }

    void DRender::OnD3D11PreCreate(IDXGIAdapter* pAdapter, const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc)
    {
        if (!m_Instance.ValidateDisplayMode(pSwapChainDesc)) {
            m_Instance.Warning("Invalid refresh rate: (%u/%u)",
                pSwapChainDesc->BufferDesc.RefreshRate.Numerator,
                pSwapChainDesc->BufferDesc.RefreshRate.Denominator);
        }

        m_Instance.ApplyD3DSettings(const_cast<DXGI_SWAP_CHAIN_DESC*>(pSwapChainDesc));

        m_Instance.Message(
            "[D3D] Requesting mode: %ux%u@%u | VSync: %u | Windowed: %d",
            pSwapChainDesc->BufferDesc.Width, pSwapChainDesc->BufferDesc.Height,
            m_Instance.GetRefreshRate(pSwapChainDesc),
            m_Instance.vsync, pSwapChainDesc->Windowed);

        m_Instance.Debug("[D3D] SwapEffect: %s | SwapBufferCount: %u | Tearing: %d | Flags: 0x%.8X",
            m_Instance.SwapEffectToConfigKey(pSwapChainDesc->SwapEffect).c_str(), pSwapChainDesc->BufferCount,
            m_Instance.tearing_enabled, pSwapChainDesc->Flags);

        m_Instance.Message("[D3D] Windowed hardware composition support: %s",
            m_Instance.HasWindowedHWCompositionSupport(pAdapter) ? "yes" : "no");

        if (pSwapChainDesc->Windowed == TRUE) {
            if (!m_Instance.IsFlipOn(pSwapChainDesc)) {
                if (!(m_Instance.dxgi.caps & (DXGI_CAP_FLIP_DISCARD | DXGI_CAP_FLIP_SEQUENTIAL))) {
                    m_Instance.Warning("Flip not supported on your system, switch to exclusive fullscreen for better peformance");
                }
                else {
                    m_Instance.Warning("Switch to exclusive fullscreen or set SwapEffect to flip_discard or flip_sequential for better peformance");
                }
            }
        }
        else {
            if (m_Instance.IsFlipOn(pSwapChainDesc)) {
                m_Instance.Warning("Using flip in exclusive fullscreen may cause issues");
            }
        }
    }

    void DRender::OnD3D11PostCreate(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, ID3D11Device** ppDevice)
    {
    }

    HRESULT WINAPI DRender::D3D11CreateDeviceAndSwapChain_Hook(
        _In_opt_ IDXGIAdapter* pAdapter,
        D3D_DRIVER_TYPE DriverType,
        HMODULE Software,
        UINT Flags,
        _In_reads_opt_(FeatureLevels) CONST D3D_FEATURE_LEVEL* pFeatureLevels,
        UINT FeatureLevels,
        UINT SDKVersion,
        _In_opt_ CONST DXGI_SWAP_CHAIN_DESC* pSwapChainDesc,
        _COM_Outptr_opt_ IDXGISwapChain** ppSwapChain,
        _COM_Outptr_opt_ ID3D11Device** ppDevice,
        _Out_opt_ D3D_FEATURE_LEVEL* pFeatureLevel,
        _COM_Outptr_opt_ ID3D11DeviceContext** ppImmediateContext)
    {
        m_Instance.OnD3D11PreCreate(pAdapter, pSwapChainDesc);

        auto evd_pre = D3D11CreateEventPre(pSwapChainDesc);
        IEvents::TriggerEvent(Event::OnD3D11PreCreate, reinterpret_cast<void*>(&evd_pre));

        HRESULT hr = D3D11CreateDeviceAndSwapChain_O(
            pAdapter, DriverType, Software, Flags,
            pFeatureLevels, FeatureLevels, SDKVersion,
            pSwapChainDesc, ppSwapChain, ppDevice, pFeatureLevel,
            ppImmediateContext);

        if (hr == S_OK) {
            m_Instance.OnD3D11PostCreate(pSwapChainDesc, ppDevice);

            auto evd_post = D3D11CreateEventPost(pSwapChainDesc, *ppDevice, *ppImmediateContext, *ppSwapChain);
            IEvents::TriggerEvent(Event::OnD3D11PostCreate, reinterpret_cast<void*>(&evd_post));
            IEvents::TriggerEvent(Event::OnD3D11PostPostCreate, reinterpret_cast<void*>(&evd_post));
        }
        else {
            m_Instance.FatalError("D3D11CreateDeviceAndSwapChain failed: 0x%lX", hr);
        }

        return hr;
    }

    HRESULT WINAPI DRender::CreateDXGIFactory_Hook(REFIID riid, _COM_Outptr_ void** ppFactory)
    {
        HRESULT hr = CreateDXGIFactory_O(riid, ppFactory);
        if (SUCCEEDED(hr)) {
            m_Instance.pFactory = reinterpret_cast<IDXGIFactory*>(*ppFactory);
        }
        else {
            m_Instance.FatalError("CreateDXGIFactory failed (%lX)", hr);
            abort();
        }
        return hr;
    }

    void DRender::DXGI_GetCapabilities()
    {
        dxgi.caps = 0;

        {
            ComPtr<IDXGIFactory5> tmp;
            if (SUCCEEDED(pFactory->QueryInterface(__uuidof(IDXGIFactory5), &tmp))) {
                dxgi.caps = (
                    DXGI_CAP_FLIP_SEQUENTIAL |
                    DXGI_CAP_FLIP_DISCARD);

                BOOL allowTearing;
                HRESULT hr = tmp->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));

                if (SUCCEEDED(hr) && allowTearing) {
                    dxgi.caps |= DXGI_CAP_TEARING;
                }

                return;
            }
        }

        {
            ComPtr<IDXGIFactory4> tmp;
            if (SUCCEEDED(pFactory->QueryInterface(__uuidof(IDXGIFactory4), &tmp))) {
                dxgi.caps = (
                    DXGI_CAP_FLIP_SEQUENTIAL |
                    DXGI_CAP_FLIP_DISCARD);

                return;
            }
        }

        {
            ComPtr<IDXGIFactory3> tmp;
            if (SUCCEEDED(pFactory->QueryInterface(__uuidof(IDXGIFactory3), &tmp))) {
                dxgi.caps = DXGI_CAP_FLIP_SEQUENTIAL;
            }
        }
    }

    bool DRender::HasWindowedHWCompositionSupport(IDXGIAdapter* adapter) const
    {
        if (adapter == nullptr) {
            return false;
        }

        for (UINT i = 0;; ++i)
        {
            ComPtr<IDXGIOutput> output;
            if (FAILED(adapter->EnumOutputs(i, &output))) {
                break;
            }

            ComPtr<IDXGIOutput6> output6;
            if (SUCCEEDED(output.As(&output6)))
            {
                UINT flags;
                HRESULT hr = output6->CheckHardwareCompositionSupport(&flags);

                if (SUCCEEDED(hr) && (flags & DXGI_HARDWARE_COMPOSITION_SUPPORT_FLAG_WINDOWED)) {
                    return true;
                }
            }
        }

        return false;
    }
}