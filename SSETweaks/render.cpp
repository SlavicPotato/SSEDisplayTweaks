#include "pch.h"

#include <Src/PlatformHelpers.h>

using namespace Microsoft::WRL;

namespace SDT
{
    static constexpr const char* SECTION_GENERAL = "General";

    static constexpr const char* CKEY_FULLSCREEN = "Fullscreen";
    static constexpr const char* CKEY_BORDERLESS = "Borderless";
    static constexpr const char* CKEY_UPSCALE = "BorderlessUpscale";
    static constexpr const char* CKEY_UPSCALE_PRIMARY_MON = "BorderlessUpscaleRelativeToPrimaryMonitor";
    static constexpr const char* CKEY_DISABLEBUFFERRESIZE = "DisableBufferResizing";
    static constexpr const char* CKEY_DISABLETARGETRESIZE = "DisableTargetResizing";
    static constexpr const char* CKEY_VSYNC = "EnableVSync";
    static constexpr const char* CKEY_VSYNCPRESENTINT = "VSyncPresentInterval";
    static constexpr const char* CKEY_MAXRR = "MaximumRefreshRate";
    static constexpr const char* CKEY_BUFFERCOUNT = "SwapBufferCount";
    static constexpr const char* CKEY_SWAPEFFECT = "SwapEffect";
    static constexpr const char* CKEY_SCALINGMODE = "ScalingMode";
    static constexpr const char* CKEY_MAXFRAMELAT = "MaxFrameLatency";
    static constexpr const char* CKEY_ENABLETEARING = "EnableTearing";

    static constexpr const char* CKEY_FPSLIMIT = "FramerateLimit";

    static constexpr const char* CKEY_LOADSCRFPSLIMIT = "LoadingScreenFramerateLimit";
    static constexpr const char* CKEY_LOADSCRFPSLIMITEX = "LoadingScreenLimitExtraTime";
    static constexpr const char* CKEY_INITIALLOADLIMITEX = "LoadingScreenLimitExtraTimePostLoad";
    static constexpr const char* CKEY_UIFPSLIMIT = "UIFramerateLimit";
    static constexpr const char* CKEY_UIMAPFPSLIMIT = "UIFramerateLimitMap";
    static constexpr const char* CKEY_UIINVFPSLIMIT = "UIFramerateLimitInventory";
    static constexpr const char* CKEY_UIJOURFPSLIMIT = "UIFramerateLimitJournal";
    static constexpr const char* CKEY_UICUSTOMFPSLIMIT = "UIFramerateLimitCustom";
    static constexpr const char* CKEY_UIMAINFPSLIMIT = "UIFramerateLimitMain";
    static constexpr const char* CKEY_UIRACEFPSLIMIT = "UIFramerateLimitRace";
    static constexpr const char* CKEY_UIPERKFPSLIMIT = "UIFramerateLimitPerk";
    static constexpr const char* CKEY_UIBOOKFPSLIMIT = "UIFramerateLimitBook";
    static constexpr const char* CKEY_UILOCKFPSLIMIT = "UIFramerateLimitLockpick";
    static constexpr const char* CKEY_UICONSOLEFPSLIMIT = "UIFramerateLimitConsole";
    static constexpr const char* CKEY_UITWEENFPSLIMIT = "UIFramerateLimitTween";
    static constexpr const char* CKEY_UISWFPSLIMIT = "UIFramerateLimitSleepWait";

    static constexpr const char* CKEY_LOADSCRFPSLIMIT_DV = "LoadingScreenFramerateLimitVSyncOff";
    static constexpr const char* CKEY_UIFPSLIMIT_DV = "UIFramerateLimitVSyncOff";
    static constexpr const char* CKEY_UIMAPFPSLIMIT_DV = "UIFramerateLimitMapVSyncOff";
    static constexpr const char* CKEY_UIINVFPSLIMIT_DV = "UIFramerateLimitInventoryVSyncOff";
    static constexpr const char* CKEY_UIJOURFPSLIMIT_DV = "UIFramerateLimitJournalVSyncOff";
    static constexpr const char* CKEY_UICUSTOMFPSLIMIT_DV = "UIFramerateLimitCustomVSyncOff";
    static constexpr const char* CKEY_UIMAINFPSLIMIT_DV = "UIFramerateLimitMainVSyncOff";
    static constexpr const char* CKEY_UIRACEFPSLIMIT_DV = "UIFramerateLimitRaceVSyncOff";
    static constexpr const char* CKEY_UIPERKFPSLIMIT_DV = "UIFramerateLimitPerkVSyncOff";
    static constexpr const char* CKEY_UIBOOKFPSLIMIT_DV = "UIFramerateLimitBookVSyncOff";
    static constexpr const char* CKEY_UILOCKFPSLIMIT_DV = "UIFramerateLimitLockpickVSyncOff";
    static constexpr const char* CKEY_UICONSOLEFPSLIMIT_DV = "UIFramerateLimitConsoleVSyncOff";
    static constexpr const char* CKEY_UITWEENFPSLIMIT_DV = "UIFramerateLimitTweenVSyncOff";
    static constexpr const char* CKEY_UISWFPSLIMIT_DV = "UIFramerateLimitSleepWaitVSyncOff";

    static constexpr const char* CKEY_ADJUSTINICFG = "AdjustGameSettings";

    static constexpr const char* CKEY_RESOLUTON = "Resolution";
    static constexpr const char* CKEY_RESSCALE = "ResolutionScale";

    using namespace Structures;
    using namespace Patching;

    DRender DRender::m_Instance;

    DRender::SEMap DRender::cfgSwapEffectMap = {
        {"auto", -1},
        {"discard", 0},
        {"sequential", 1},
        {"flip_sequential", 2},
        {"flip_discard", 3}
    };

    DRender::SMMap DRender::cfgScalingModeMap = {
        {"unspecified", DXGI_MODE_SCALING_UNSPECIFIED},
        {"centered", DXGI_MODE_SCALING_CENTERED},
        {"stretched", DXGI_MODE_SCALING_STRETCHED}
    };

    DXGI_SWAP_EFFECT DRender::GetSwapEffect(int a_code)
    {
        switch (a_code)
        {
        case 0:
            return DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_DISCARD;
        case 1:
            return DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_SEQUENTIAL;
        case 2:
            return DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        case 3:
            return DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_DISCARD;
        default:
            return DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_DISCARD;
        }
    }

    const char* DRender::GetMenuDescription(MenuEvent a_event)
    {
        switch (a_event)
        {
        case MenuEvent::OnAnyMenu:
            return "UI";
        case MenuEvent::OnLoadingMenu:
            return "Loading screen";
        case MenuEvent::OnInventoryMenu:
            return "Inventory";
        case MenuEvent::OnMagicMenu:
            return "Magic";
        case MenuEvent::OnGiftMenu:
            return "Gift";
        case MenuEvent::OnBarterMenu:
            return "Barter";
        case MenuEvent::OnContainerMenu:
            return "Container";
        case MenuEvent::OnJournalMenu:
            return "Journal";
        case MenuEvent::OnMapMenu:
            return "Map";
        case MenuEvent::OnCustomMenu:
            return "Custom";
        case MenuEvent::OnMainMenu:
            return "Main";
        case MenuEvent::OnRaceSexMenu:
            return "Race";
        case MenuEvent::OnStatsMenu:
            return "Perk";
        case MenuEvent::OnBookMenu:
            return "Book";
        case MenuEvent::OnLockpickingMenu:
            return "Lockpicking";
        case MenuEvent::OnConsoleMenu:
            return "Console";
        case MenuEvent::OnTweenMenu:
            return "Tween";
        case MenuEvent::OnSleepWaitMenu:
            return "Sleep/Wait";
        case MenuEvent::OnFavoritesMenu:
            return "Favorites";
        default:
            return "Unknown";
        }
    }

    const char* DRender::GetSwapEffectOption(DXGI_SWAP_EFFECT a_swapEffect)
    {
        switch (a_swapEffect)
        {
        case DXGI_SWAP_EFFECT_DISCARD:
            return "discard";
        case DXGI_SWAP_EFFECT_SEQUENTIAL:
            return "sequential";
        case DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL:
            return "flip_sequential";
        case DXGI_SWAP_EFFECT_FLIP_DISCARD:
            return "flip_discard";
        default:
            return "unknown";
        }
    }

    void MenuFramerateLimit::SetLimit(MenuEvent code, float limit, bool disable_vsync)
    {
        if (limit <= 0.0f) {
            m_limits[Enum::Underlying(code)] = MenuFramerateLimitDescriptor(disable_vsync, 0LL);
        }
        else {
            m_limits[Enum::Underlying(code)] = MenuFramerateLimitDescriptor(
                disable_vsync,
                static_cast<long long>((1.0 / static_cast<double>(limit)) * 1000000.0)
            );
        }

        m_hasLimits = true;
    }

    bool MenuFramerateLimit::GetLimit(MenuEvent code, MenuFramerateLimitDescriptor& limit) const
    {
        auto& v = m_limits[Enum::Underlying(code)];

        if (v.enabled)
        {
            limit = v;
            return true;
        }

        return false;
    }

    bool MenuFramerateLimit::HasLimits() const
    {
        return m_hasLimits;
    }

    bool MenuFramerateLimit::HasLimit(MenuEvent code) const
    {
        return m_limits[Enum::Underlying(code)].enabled;
    }

    bool MenuFramerateLimit::GetCurrentLimit(MenuFramerateLimitDescriptor& limit) const
    {
        if (!HasLimits())
            return false;

        if (!m_stack.empty()) {

            for (auto it = m_stack.rbegin(); it != m_stack.rend(); ++it)
            {
                auto& v = m_limits[Enum::Underlying(*it)];
                if (v.enabled)
                {
                    limit = v;
                    return true;
                }
            }
        }

        auto& v = m_limits[Enum::Underlying(MenuEvent::OnAnyMenu)];
        if (v.enabled)
        {
            limit = v;
            return true;
        }

        return false;
    }

    DRender::DRender() :
        fps_max(0),
        current_fps_max(0),
        has_swap_effect(false),
        limiter_installed(false),
        tearing_enabled(false),
        has_fl_override(false),
        fps_limit(-1),
        m_present_flags(0),
        lslExtraTime(0),
        lslPostLoadExtraTime(0),
        oo_expire_time(0),
        oo_current_fps_max(0),
        gameLoadState(0),
        m_dxgiFactory(nullptr),
        m_swapchain{ 0, 0, 0 },
        m_fl({
            MenuEvent::OnJournalMenu,
            MenuEvent::OnMainMenu,
            MenuEvent::OnMapMenu,
            MenuEvent::OnConsoleMenu,
            MenuEvent::OnCustomMenu,
            MenuEvent::OnRaceSexMenu,
            MenuEvent::OnInventoryMenu,
            MenuEvent::OnContainerMenu,
            MenuEvent::OnMagicMenu,
            MenuEvent::OnTweenMenu,
            MenuEvent::OnLoadingMenu,
            MenuEvent::OnMessageBoxMenu,
            MenuEvent::OnLockpickingMenu,
            MenuEvent::OnGiftMenu,
            MenuEvent::OnTrainingMenu,
            MenuEvent::OnTutorialMenu,
            MenuEvent::OnLevelUpMenu,
            MenuEvent::OnBarterMenu,
            MenuEvent::OnStatsMenu,
            MenuEvent::OnFavoritesMenu,
            MenuEvent::OnSleepWaitMenu,
            MenuEvent::OnQuantityMenu,
            MenuEvent::OnModManagerMenu,
            MenuEvent::OnBookMenu
            })
    {
        m_swapchain.flags = 0;
        m_swapchain.width = 0;
        m_swapchain.height = 0;
    }

    void DRender::LoadConfig()
    {
        m_conf.fullscreen = static_cast<std::uint8_t>(GetConfigValue(CKEY_FULLSCREEN, false));
        m_conf.borderless = static_cast<std::uint8_t>(GetConfigValue(CKEY_BORDERLESS, true));
        m_conf.upscale = GetConfigValue(CKEY_UPSCALE, false);
        m_conf.upscale_select_primary_monitor = GetConfigValue(CKEY_UPSCALE_PRIMARY_MON, true);
        m_conf.disablebufferresize = GetConfigValue(CKEY_DISABLEBUFFERRESIZE, false);
        m_conf.disabletargetresize = GetConfigValue(CKEY_DISABLETARGETRESIZE, false);

        m_conf.vsync_on = GetConfigValue(CKEY_VSYNC, true);
        m_conf.vsync_present_interval = std::clamp<std::uint32_t>(GetConfigValue<std::uint32_t>(CKEY_VSYNCPRESENTINT, 1), 1, 4);
        m_conf.max_rr = GetConfigValue(CKEY_MAXRR, 0);
        has_swap_effect = ConfigTranslateSwapEffect(
            GetConfigValue(CKEY_SWAPEFFECT, "auto"),
            m_conf.swap_effect
        );
        has_scaling_mode = ConfigTranslateScalingMode(
            GetConfigValue(CKEY_SCALINGMODE, "default"),
            m_conf.scaling_mode
        );
        m_conf.buffer_count = GetConfigValue(CKEY_BUFFERCOUNT, 0);
        if (m_conf.buffer_count > -1) {
            m_conf.buffer_count = std::clamp<std::int32_t>(m_conf.buffer_count, 0, 8);
        }
        m_conf.max_frame_latency = GetConfigValue(CKEY_MAXFRAMELAT, 0);
        if (m_conf.max_frame_latency > 0) {
            m_conf.max_frame_latency = std::min(m_conf.max_frame_latency, 16);
        }
        m_conf.enable_tearing = GetConfigValue(CKEY_ENABLETEARING, true);

        m_conf.limits.game = GetConfigValue(CKEY_FPSLIMIT, -1.0f);

        m_conf.limits.ui = GetConfigValue(CKEY_UIFPSLIMIT, 0.0f);
        m_conf.limits.ui_map = GetConfigValue(CKEY_UIMAPFPSLIMIT, 0.0f);
        m_conf.limits.ui_inventory = GetConfigValue(CKEY_UIINVFPSLIMIT, 0.0f);
        m_conf.limits.ui_journal = GetConfigValue(CKEY_UIJOURFPSLIMIT, 0.0f);
        m_conf.limits.ui_custom = GetConfigValue(CKEY_UICUSTOMFPSLIMIT, 0.0f);
        m_conf.limits.ui_main = GetConfigValue(CKEY_UIMAINFPSLIMIT, 0.0f);
        m_conf.limits.ui_race = GetConfigValue(CKEY_UIRACEFPSLIMIT, 0.0f);
        m_conf.limits.ui_perk = GetConfigValue(CKEY_UIPERKFPSLIMIT, 0.0f);
        m_conf.limits.ui_book = GetConfigValue(CKEY_UIBOOKFPSLIMIT, 0.0f);
        m_conf.limits.ui_lockpick = GetConfigValue(CKEY_UILOCKFPSLIMIT, 0.0f);
        m_conf.limits.ui_loadscreen = GetConfigValue(CKEY_LOADSCRFPSLIMIT, 0.0f);
        m_conf.limits.ui_console = GetConfigValue(CKEY_UICONSOLEFPSLIMIT, 0.0f);
        m_conf.limits.ui_tween = GetConfigValue(CKEY_UITWEENFPSLIMIT, 0.0f);
        m_conf.limits.ui_sw = GetConfigValue(CKEY_UISWFPSLIMIT, 0.0f);

        m_conf.limits.ui_dv = GetConfigValue(CKEY_UIFPSLIMIT_DV, true);
        m_conf.limits.ui_map_dv = GetConfigValue(CKEY_UIMAPFPSLIMIT_DV, true);
        m_conf.limits.ui_inventory_dv = GetConfigValue(CKEY_UIINVFPSLIMIT_DV, true);
        m_conf.limits.ui_journal_dv = GetConfigValue(CKEY_UIJOURFPSLIMIT_DV, true);
        m_conf.limits.ui_custom_dv = GetConfigValue(CKEY_UICUSTOMFPSLIMIT_DV, true);
        m_conf.limits.ui_main_dv = GetConfigValue(CKEY_UIMAINFPSLIMIT_DV, true);
        m_conf.limits.ui_race_dv = GetConfigValue(CKEY_UIRACEFPSLIMIT_DV, true);
        m_conf.limits.ui_perk_dv = GetConfigValue(CKEY_UIPERKFPSLIMIT_DV, true);
        m_conf.limits.ui_book_dv = GetConfigValue(CKEY_UIBOOKFPSLIMIT_DV, true);
        m_conf.limits.ui_lockpick_dv = GetConfigValue(CKEY_UILOCKFPSLIMIT_DV, true);
        m_conf.limits.ui_loadscreen_dv = GetConfigValue(CKEY_LOADSCRFPSLIMIT_DV, true);
        m_conf.limits.ui_console_dv = GetConfigValue(CKEY_UICONSOLEFPSLIMIT_DV, true);
        m_conf.limits.ui_tween_dv = GetConfigValue(CKEY_UITWEENFPSLIMIT_DV, true);
        m_conf.limits.ui_sw_dv = GetConfigValue(CKEY_UISWFPSLIMIT_DV, true);

        m_conf.limits.ui_loadscreenex = std::clamp<std::int32_t>(GetConfigValue<std::int32_t>(CKEY_LOADSCRFPSLIMITEX, 0), 0, 15);
        m_conf.limits.ui_initialloadex = std::clamp<std::int32_t>(GetConfigValue<std::int32_t>(CKEY_INITIALLOADLIMITEX, 4), 0, 30);

        m_conf.adjust_ini = IConfigS(SECTION_GENERAL).GetConfigValue(CKEY_ADJUSTINICFG, true);

        if (!ConfigParseResolution(GetConfigValue(CKEY_RESOLUTON, "-1 -1"), m_conf.resolution)) {
            m_conf.resolution[0] = -1;
            m_conf.resolution[1] = -1;
        }

        m_conf.resolution_scale = GetConfigValue(CKEY_RESSCALE, -1.0f);
    }

    bool DRender::ConfigTranslateSwapEffect(const std::string& param, int& out) const
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

    bool DRender::ConfigTranslateScalingMode(const std::string& param, DXGI_MODE_SCALING& out) const
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

    bool DRender::ConfigParseResolution(const std::string& in, std::int32_t(&a_out)[2])
    {
        stl::vector<std::int32_t> v2;
        StrHelpers::SplitString<std::int32_t>(in, 'x', v2);

        if (v2.size() < 2)
            return false;

        a_out[0] = v2[0];
        a_out[1] = v2[1];

        return true;
    }

    void DRender::UISetLimit(MenuEvent code, float limit, bool disable_vsync)
    {
        if (limit > 0.0f) {
            m_fl.SetLimit(code, limit, disable_vsync);
            Message("Framerate limit (%s): %.6g (VSync off: %s)",
                GetMenuDescription(code), limit, disable_vsync ? "true" : "false");
        }
        else if (limit < 0.0f) {
            m_fl.SetLimit(code, 0.0f, disable_vsync);
            Message("Framerate limit (%s): disabled (VSync off: %s)",
                GetMenuDescription(code), disable_vsync ? "true" : "false");
        }
    }

    void DRender::PostLoadConfig()
    {
        tts = IPerfCounter::Query();

        if (m_conf.upscale) {
            if (!(!m_conf.fullscreen && m_conf.borderless))
            {
                m_conf.upscale = false;
            }
            else {
                Message("Borderless upscaling enabled");
            }
        }

        vsync = m_conf.vsync_on ? m_conf.vsync_present_interval : 0;

        if (m_conf.limits.game > 0.0f) {
            current_fps_max = fps_max = static_cast<long long>((1.0 / static_cast<double>(m_conf.limits.game)) * 1000000.0);
            fps_limit = 1;
            Message("Framerate limit (game): %.6g", m_conf.limits.game);
        }
        else if (m_conf.limits.game == 0.0f) {
            fps_limit = 0;
        }
        else {
            fps_limit = -1;
        }

        UISetLimit(MenuEvent::OnAnyMenu, m_conf.limits.ui, m_conf.limits.ui_dv);
        UISetLimit(MenuEvent::OnLoadingMenu, m_conf.limits.ui_loadscreen, m_conf.limits.ui_loadscreen_dv);

        if (m_fl.HasLimit(MenuEvent::OnLoadingMenu)) {
            if (m_conf.limits.ui_loadscreenex > 0) {
                lslExtraTime = IPerfCounter::T(
                    m_conf.limits.ui_loadscreenex * 1000000);
            }
            if (m_conf.limits.ui_initialloadex > 0) {
                lslPostLoadExtraTime = IPerfCounter::T(
                    m_conf.limits.ui_initialloadex * 1000000);
            }
        }

        if (m_conf.limits.ui_inventory != 0.0f) {
            UISetLimit(MenuEvent::OnInventoryMenu, m_conf.limits.ui_inventory, m_conf.limits.ui_inventory_dv);
            UISetLimit(MenuEvent::OnMagicMenu, m_conf.limits.ui_inventory, m_conf.limits.ui_inventory_dv);
            UISetLimit(MenuEvent::OnGiftMenu, m_conf.limits.ui_inventory, m_conf.limits.ui_inventory_dv);
            UISetLimit(MenuEvent::OnBarterMenu, m_conf.limits.ui_inventory, m_conf.limits.ui_inventory_dv);
            UISetLimit(MenuEvent::OnContainerMenu, m_conf.limits.ui_inventory, m_conf.limits.ui_inventory_dv);
            UISetLimit(MenuEvent::OnFavoritesMenu, m_conf.limits.ui_inventory, m_conf.limits.ui_inventory_dv);
        }

        UISetLimit(MenuEvent::OnJournalMenu, m_conf.limits.ui_journal, m_conf.limits.ui_journal_dv);
        UISetLimit(MenuEvent::OnMapMenu, m_conf.limits.ui_map, m_conf.limits.ui_map_dv);
        UISetLimit(MenuEvent::OnCustomMenu, m_conf.limits.ui_custom, m_conf.limits.ui_custom_dv);
        UISetLimit(MenuEvent::OnMainMenu, m_conf.limits.ui_main, m_conf.limits.ui_main_dv);
        UISetLimit(MenuEvent::OnRaceSexMenu, m_conf.limits.ui_race, m_conf.limits.ui_race_dv);
        UISetLimit(MenuEvent::OnStatsMenu, m_conf.limits.ui_perk, m_conf.limits.ui_perk_dv);
        UISetLimit(MenuEvent::OnBookMenu, m_conf.limits.ui_book, m_conf.limits.ui_book_dv);
        UISetLimit(MenuEvent::OnLockpickingMenu, m_conf.limits.ui_lockpick, m_conf.limits.ui_lockpick_dv);
        UISetLimit(MenuEvent::OnConsoleMenu, m_conf.limits.ui_console, m_conf.limits.ui_console_dv);
        UISetLimit(MenuEvent::OnTweenMenu, m_conf.limits.ui_tween, m_conf.limits.ui_tween_dv);
        UISetLimit(MenuEvent::OnSleepWaitMenu, m_conf.limits.ui_sw, m_conf.limits.ui_sw_dv);
    }

    void DRender::Patch()
    {
        if (m_conf.max_rr > 0) {
            safe_write(
                DisplayRefreshRate + 0x4,
                static_cast<std::uint32_t>(m_conf.max_rr)
            );
            Message("Refresh rate patch applied (max %d Hz)", m_conf.max_rr);
        }

        safe_write(
            bBorderless_Patch,
            reinterpret_cast<const void*>(Payloads::bw_patch),
            sizeof(Payloads::bw_patch)
        );
        safe_write(
            bBorderless_Patch + 0x1,
            static_cast<std::uint32_t>(m_Instance.m_conf.borderless)
        );

        safe_write(
            bFullscreen_Patch,
            reinterpret_cast<const void*>(Payloads::bw_patch),
            sizeof(Payloads::bw_patch)
        );
        safe_write(
            bFullscreen_Patch + 0x1,
            static_cast<std::uint32_t>(m_Instance.m_conf.fullscreen)
        );

        if (HasLimits()) {
            safe_write(
                Present_Limiter + 0x6,
                reinterpret_cast<const void*>(Payloads::nopjmp),
                sizeof(Payloads::nopjmp)
            );

            limiter_installed = true;
        }

        if (m_conf.max_frame_latency > 0) {
            safe_write(
                MaxFrameLatency,
                static_cast<std::uint32_t>(m_conf.max_frame_latency)
            );
            Message("Maximum frame latency: %d", m_conf.max_frame_latency);
        }

        if (!m_conf.fullscreen)
        {
            if (m_conf.disablebufferresize) {
                safe_write(
                    ResizeBuffersDisable,
                    reinterpret_cast<const void*>(Payloads::ResizeBuffersDisable),
                    sizeof(Payloads::ResizeBuffersDisable)
                );
                Debug("Disabled swap chain buffer resizing");
            }

            if (m_conf.disabletargetresize) {
                safe_write(
                    ResizeTargetDisable,
                    reinterpret_cast<const void*>(Payloads::ResizeTargetDisable),
                    sizeof(Payloads::ResizeTargetDisable)
                );

                Debug("Disabled swap chain target resizing");
            }

            bool patchRes(false);
            std::int32_t w, h;

            if (m_conf.resolution[0] > 0 && m_conf.resolution[1] > 0)
            {
                w = m_conf.resolution[0];
                h = m_conf.resolution[1];

                patchRes = true;
            }
            else
            {
                w = *m_gv.iSizeW;
                h = *m_gv.iSizeH;
            }

            if (m_conf.resolution_scale > 0.0f)
            {
                w = static_cast<std::int32_t>(static_cast<float>(w) * m_conf.resolution_scale);
                h = static_cast<std::int32_t>(static_cast<float>(h) * m_conf.resolution_scale);

                patchRes = true;
            }

            if (patchRes)
            {
                w = std::max<std::int32_t>(w, 1);
                h = std::max<std::int32_t>(h, 1);

                safe_write(
                    iSizeW_Patch,
                    Payloads::res_patch,
                    sizeof(Payloads::res_patch)
                );
                safe_write(iSizeW_Patch + 0x1, w);

                safe_write(
                    iSizeH_Patch,
                    Payloads::res_patch,
                    sizeof(Payloads::res_patch)
                );
                safe_write(iSizeH_Patch + 0x1, h);

                Message("Resolution override: (%dx%d)", w, h);
            }
        }
        else {
            struct ResizeTargetInjectArgs : JITASM::JITASM {
                ResizeTargetInjectArgs(std::uintptr_t retnAddr, std::uintptr_t mdescAddr
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
                    std::uintptr_t(&modeDesc));
                g_branchTrampoline.Write6Branch(
                    ResizeTarget, code.get());
                safe_memset(ResizeTarget + 0x6, 0xCC, 0x2);
            }
            LogPatchEnd("IDXGISwapChain::ResizeTarget");
        }

        if (m_conf.fullscreen == 0 && m_conf.enable_tearing
            && (!m_conf.vsync_on || limiter_installed))
        {
            struct PresentFlagsInject : JITASM::JITASM {
                PresentFlagsInject(std::uintptr_t retnAddr, std::uintptr_t flagsAddr)
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
                    std::uintptr_t(&m_present_flags));

                g_branchTrampoline.Write6Branch(
                    Present_Flags_Inject, code.get());

                safe_write<std::uint8_t>(Present_Flags_Inject + 0x6, 0xCC);
            }
            LogPatchEnd("IDXGISwapChain::Present");
        }

        {
            struct ResizeBuffersInjectArgs : JITASM::JITASM {
                ResizeBuffersInjectArgs(std::uintptr_t retnAddr, std::uintptr_t swdAddr
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
                    std::uintptr_t(&m_swapchain));

                g_branchTrampoline.Write6Branch(
                    ResizeBuffers_Inject, code.get());

                safe_memset(ResizeBuffers_Inject + 0x6, 0xCC, 0x12);
            }
            LogPatchEnd("IDXGISwapChain::ResizeBuffers");
        }
    }

    void DRender::RegisterHooks()
    {
        if (!Hook::Call5(CreateDXGIFactory_C,
            reinterpret_cast<std::uintptr_t>(CreateDXGIFactory_Hook),
            m_createDXGIFactory_O)) 
        {
            Warning("CreateDXGIFactory hook failed");
        }

        if (!Hook::Call5(D3D11CreateDeviceAndSwapChain_C,
            reinterpret_cast<std::uintptr_t>(D3D11CreateDeviceAndSwapChain_Hook),
            m_D3D11CreateDeviceAndSwapChain_O))
        {
            Error("D3D11CreateDeviceAndSwapChain hook failed");
            SetOK(false);
            return;
        }

        if (HasLimits()) {
            RegisterHook(
                Present_Limiter,
                reinterpret_cast<std::uintptr_t>(Throttle),
                HookDescriptor::HookType::kWR6Call
            );
        }

        if (m_fl.HasLimits()) {
            IEvents::RegisterForEvent(MenuEvent::OnAnyMenu, OnMenuEvent);
            if (lslPostLoadExtraTime > 0) {
                IEvents::RegisterForEvent(Event::OnMessage, MessageHandler);
            }
        }

        IEvents::RegisterForEvent(Event::OnConfigLoad, OnConfigLoad);
    }

    bool DRender::Prepare()
    {
        m_gv.bLockFramerate = ISKSE::GetINISettingAddr<std::uint8_t>("bLockFramerate:Display");
        if (!m_gv.bLockFramerate) {
            return false;
        }

        m_gv.iFPSClamp = ISKSE::GetINISettingAddr<std::int32_t>("iFPSClamp:General");
        if (!m_gv.iFPSClamp) {
            return false;
        }

        m_gv.iSizeW = ISKSE::GetINIPrefSettingAddr<std::int32_t>("iSize W:Display");
        if (!m_gv.iSizeW) {
            return false;
        }

        m_gv.iSizeH = ISKSE::GetINIPrefSettingAddr<std::int32_t>("iSize H:Display");
        if (!m_gv.iSizeH) {
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
            *m_Instance.m_gv.bLockFramerate = 0;
        }

        if (m_Instance.m_conf.adjust_ini) {
            if (*m_Instance.m_gv.iFPSClamp != 0)
            {
                m_Instance.Message("Setting iFPSClamp=0");
                *m_Instance.m_gv.iFPSClamp = 0;
            }
        }
    }

    void DRender::SetFPSLimitOverride(long long max, bool disable_vsync)
    {
        if (m_conf.vsync_on) {
            if (disable_vsync) {
                (*DXGIData)->SyncInterval = 0;
                if (tearing_enabled) {
                    m_present_flags |= DXGI_PRESENT_ALLOW_TEARING;
                }
            }
            else {
                (*DXGIData)->SyncInterval = vsync;
                if (tearing_enabled) {
                    m_present_flags &= ~DXGI_PRESENT_ALLOW_TEARING;
                }
            }
        }

        current_fps_max = max;
        has_fl_override = true;
    }

    void DRender::SetFPSLimitPost(long long a_max, long long a_expire)
    {
        m_Instance.oo_current_fps_max = a_max;
        m_Instance.oo_expire_time = a_expire;
    }

    void DRender::ResetFPSLimitOverride()
    {
        if (!has_fl_override) {
            return;
        }

        if (m_conf.vsync_on) {
            (*DXGIData)->SyncInterval = vsync;
            if (tearing_enabled) {
                m_present_flags &= ~DXGI_PRESENT_ALLOW_TEARING;
            }
        }

        current_fps_max = fps_max;
        has_fl_override = false;
    }

    void DRender::QueueFPSLimitOverride(long long max, bool disable_vsync)
    {
        m_afTasks.AddTask([=, this]() { SetFPSLimitOverride(max, disable_vsync); });
    }

    void DRender::QueueFPSLimitPost(long long a_max, long long a_expire)
    {
        m_afTasks.AddTask([=, this]() { SetFPSLimitPost(a_max, a_expire); });
    }

    void DRender::QueueFPSLimitOverrideReset()
    {
        m_afTasks.AddTask([this]() { ResetFPSLimitOverride(); });
    }

    bool DRender::HandleMenuEvent(MenuEvent a_code, MenuOpenCloseEvent* a_evn)
    {
        auto mm = MenuManager::GetSingleton();
        if (!mm)
            return true;

        MenuFramerateLimitDescriptor ld;
        if (mm->InPausedMenu())
        {
            m_fl.Track(a_code, a_evn->opening);

            if (m_fl.GetCurrentLimit(ld)) {
                QueueFPSLimitOverride(ld.limit, ld.disable_vsync);
            }
            else {
                QueueFPSLimitOverrideReset();
            }
        }
        else
        {
            m_fl.ClearStack();
            QueueFPSLimitOverrideReset();
        }

        if (lslExtraTime > 0 || lslPostLoadExtraTime > 0)
        {
            if (a_code == MenuEvent::OnLoadingMenu && a_evn->opening == false)
            {
                if (m_fl.GetLimit(MenuEvent::OnLoadingMenu, ld))
                {
                    if (lslPostLoadExtraTime > 0 && gameLoadState == 1)
                    {
                        gameLoadState = 0;
                        QueueFPSLimitPost(
                            ld.limit,
                            IPerfCounter::Query() + lslPostLoadExtraTime);
                    }
                    else if (lslExtraTime > 0)
                    {
                        QueueFPSLimitPost(
                            ld.limit,
                            IPerfCounter::Query() + lslExtraTime);
                    }
                }
            }
        }

        return true;
    }

    bool DRender::OnMenuEvent(MenuEvent a_code, MenuOpenCloseEvent* a_evn, EventDispatcher<MenuOpenCloseEvent>*)
    {
        return m_Instance.HandleMenuEvent(a_code, a_evn);
    }

    void DRender::Throttle()
    {
        m_Instance.m_afTasks.ProcessTasks();

        auto e = IPerfCounter::Query();

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
            while (IPerfCounter::delta_us(m_Instance.tts, e) < limit)
            {
                ::Sleep(0);
                e = IPerfCounter::Query();
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

        if (m_conf.vsync_on && pSwapChainDesc->Windowed == FALSE) {
            maxt = static_cast<float>(GetRefreshRate(pSwapChainDesc));
        }

        if (fps_limit == 1) {
            if (!(m_conf.vsync_on && pSwapChainDesc->Windowed == FALSE) ||
                m_conf.limits.game < maxt)
            {
                maxt = m_conf.limits.game;
            }
        }

        return maxt;
    }

    DXGI_SWAP_EFFECT DRender::AutoGetSwapEffect(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc) const
    {
        if (pSwapChainDesc->Windowed == TRUE) {
            if (m_dxgi.caps & DXGI_CAP_FLIP_DISCARD) {
                return DXGI_SWAP_EFFECT_FLIP_DISCARD;
            }
            else if (m_dxgi.caps & DXGI_CAP_FLIP_SEQUENTIAL) {
                return DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
            }
        }
        return DXGI_SWAP_EFFECT_DISCARD;
    }


    DXGI_SWAP_EFFECT DRender::ManualGetSwapEffect(const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc)
    {
        auto se = GetSwapEffect(m_conf.swap_effect);

        if (pSwapChainDesc->Windowed == TRUE) {
            auto nse = se;

            if (nse == DXGI_SWAP_EFFECT_FLIP_DISCARD &&
                !(m_dxgi.caps & DXGI_CAP_FLIP_DISCARD))
            {
                nse = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
            }

            if (nse == DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL &&
                !(m_dxgi.caps & DXGI_CAP_FLIP_SEQUENTIAL))
            {
                nse = DXGI_SWAP_EFFECT_DISCARD;
            }

            if (nse != se) {
                Warning("%s not supported, using %s",
                    GetSwapEffectOption(se),
                    GetSwapEffectOption(nse)
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
            if (m_conf.swap_effect == -1) {
                pSwapChainDesc->SwapEffect = AutoGetSwapEffect(pSwapChainDesc);
            }
            else {
                pSwapChainDesc->SwapEffect = ManualGetSwapEffect(pSwapChainDesc);
            }
        }

        bool flip_model = IsFlipOn(pSwapChainDesc);

        if (pSwapChainDesc->Windowed == TRUE)
        {
            if (m_conf.enable_tearing && flip_model &&
                (!m_conf.vsync_on || limiter_installed))
            {
                if (m_dxgi.caps & DXGI_CAP_TEARING) {
                    pSwapChainDesc->Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

                    if (!m_conf.vsync_on) {
                        m_present_flags |= DXGI_PRESENT_ALLOW_TEARING;
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
                pSwapChainDesc->BufferDesc.Scaling = m_conf.scaling_mode;
            }
        }

        if (m_conf.buffer_count == 0) {
            if (flip_model) {
                pSwapChainDesc->BufferCount = 3;
            }
            else {
                pSwapChainDesc->BufferCount = 2;
            }
        }
        else if (m_conf.buffer_count > 0) {
            pSwapChainDesc->BufferCount = static_cast<UINT>(m_conf.buffer_count);
        }

        if (flip_model) {
            pSwapChainDesc->SampleDesc.Count = 1;
            pSwapChainDesc->SampleDesc.Quality = 0;

            if (pSwapChainDesc->BufferCount < 2) {
                pSwapChainDesc->BufferCount = 2;
                Warning("Buffer count below the minimum required for flip model, increasing to 2");
            }
        }

        if (m_conf.upscale) {
            m_swapchain.width = pSwapChainDesc->BufferDesc.Width;
            m_swapchain.height = pSwapChainDesc->BufferDesc.Height;
        }

        modeDesc = pSwapChainDesc->BufferDesc;

        if (pSwapChainDesc->Windowed == FALSE) {
            modeDesc.Format = DXGI_FORMAT_UNKNOWN;
        }

        m_swapchain.flags = pSwapChainDesc->Flags;
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
            GetSwapEffectOption(pSwapChainDesc->SwapEffect), pSwapChainDesc->BufferCount,
            m_Instance.tearing_enabled, pSwapChainDesc->Flags);

        m_Instance.Message("[D3D] Windowed hardware composition support: %s",
            m_Instance.HasWindowedHWCompositionSupport(pAdapter) ? "yes" : "no");

        if (pSwapChainDesc->Windowed == TRUE) {
            if (!m_Instance.IsFlipOn(pSwapChainDesc)) {
                if (!(m_Instance.m_dxgi.caps & (DXGI_CAP_FLIP_DISCARD | DXGI_CAP_FLIP_SEQUENTIAL))) {
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

        HRESULT hr = m_Instance.m_D3D11CreateDeviceAndSwapChain_O(
            pAdapter, DriverType, Software, Flags,
            pFeatureLevels, FeatureLevels, SDKVersion,
            pSwapChainDesc, ppSwapChain, ppDevice, pFeatureLevel,
            ppImmediateContext);

        if (hr == S_OK) {
            m_Instance.OnD3D11PostCreate(pSwapChainDesc, ppDevice);

            auto evd_post = D3D11CreateEventPost(pSwapChainDesc, *ppDevice, *ppImmediateContext, *ppSwapChain, pAdapter);
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
        HRESULT hr = m_Instance.m_createDXGIFactory_O(riid, ppFactory);
        if (SUCCEEDED(hr)) {
            m_Instance.m_dxgiFactory = static_cast<IDXGIFactory*>(*ppFactory);
        }
        else {
            m_Instance.FatalError("CreateDXGIFactory failed (%lX)", hr);
            abort();
        }
        return hr;
    }

    IDXGIFactory* DRender::DXGI_GetFactory() const
    {
        HMODULE hModule = LoadLibraryA("dxgi.dll");
        if (!hModule)
            return nullptr;

        auto func = reinterpret_cast<CreateDXGIFactory_T>(GetProcAddress(hModule, "CreateDXGIFactory"));
        if (!func)
            return nullptr;

        IDXGIFactory* pFactory;
        if (!SUCCEEDED(func(IID_PPV_ARGS(&pFactory))))
            return nullptr;

        return pFactory;
    }

    void DRender::DXGI_GetCapabilities()
    {
        bool release;
        IDXGIFactory* factory;

        if (!m_dxgiFactory)
        {
            Warning("IDXGIFactory not set, attempting to create..");

            factory = DXGI_GetFactory();
            if (!factory) {
                Error("Couldn't create IDXGIFactory, assuming DXGI_CAPS_ALL");
                m_dxgi.caps = DXGI_CAPS_ALL;
                return;
            }

            release = true;
        }
        else {
            factory = m_dxgiFactory;
            release = false;
        }

        m_dxgi.caps = 0;

        do
        {
            {
                ComPtr<IDXGIFactory5> tmp;
                if (SUCCEEDED(factory->QueryInterface(IID_PPV_ARGS(&tmp)))) {
                    m_dxgi.caps = (
                        DXGI_CAP_FLIP_SEQUENTIAL |
                        DXGI_CAP_FLIP_DISCARD);

                    BOOL allowTearing;
                    HRESULT hr = tmp->CheckFeatureSupport(
                        DXGI_FEATURE_PRESENT_ALLOW_TEARING,
                        &allowTearing, sizeof(allowTearing));

                    if (SUCCEEDED(hr) && allowTearing) {
                        m_dxgi.caps |= DXGI_CAP_TEARING;
                    }

                    break;
                }
            }

            {
                ComPtr<IDXGIFactory4> tmp;
                if (SUCCEEDED(factory->QueryInterface(IID_PPV_ARGS(&tmp)))) {
                    m_dxgi.caps = (
                        DXGI_CAP_FLIP_SEQUENTIAL |
                        DXGI_CAP_FLIP_DISCARD);

                    break;
                }
            }

            {
                ComPtr<IDXGIFactory3> tmp;
                if (SUCCEEDED(factory->QueryInterface(IID_PPV_ARGS(&tmp)))) {
                    m_dxgi.caps = DXGI_CAP_FLIP_SEQUENTIAL;
                }
            }
        } while (0);

        if (release)
            factory->Release();
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


    bool DRender::QueryVideoMemoryInfo(
        IDXGISwapChain* a_swapChain,
        DXGI_QUERY_VIDEO_MEMORY_INFO& a_out) const
    {
        try
        {
            ComPtr<IDXGIDevice> pDXGIDevice;
            DirectX::ThrowIfFailed(a_swapChain->GetDevice(IID_PPV_ARGS(pDXGIDevice.GetAddressOf())));

            ComPtr<IDXGIAdapter> pDXGIAdapter;
            DirectX::ThrowIfFailed(pDXGIDevice->GetAdapter(pDXGIAdapter.GetAddressOf()));

            ComPtr<IDXGIAdapter3> adapter;
            DirectX::ThrowIfFailed(pDXGIAdapter.As(&adapter));

            DirectX::ThrowIfFailed(adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, std::addressof(a_out)));

            return true;
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

}