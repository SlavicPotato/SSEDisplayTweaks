#include "pch.h"

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

namespace SDT
{
    static constexpr const char* CKEY_ENABLESTATS = "Enable";
    static constexpr const char* CKEY_OSDINITIAL = "InitiallyOn";
    static constexpr const char* CKEY_STATSFONT = "Font";
    static constexpr const char* CKEY_STATSFONTFILE = "FontFile";
    static constexpr const char* CKEY_STATSFONTCOLOR = "Color";
    static constexpr const char* CKEY_STATSFONTOUTLINECOLOR = "OutlineColor";
    static constexpr const char* CKEY_STATSOUTLINESIZE = "OutlineOffset";
    static constexpr const char* CKEY_STATSKEY = "ToggleKey";
    static constexpr const char* CKEY_STATSALIGN = "Align";
    static constexpr const char* CKEY_STATSOFFSET = "Offset";
    static constexpr const char* CKEY_STATSSCALE = "Scale";
    static constexpr const char* CKEY_STATSAUTOSCALE = "AutoScale";
    static constexpr const char* CKEY_STATSINTERVAL = "UpdateInterval";
    static constexpr const char* CKEY_STATSFLAGS = "Flags";
    static constexpr const char* CKEY_STATSITEMS = "Show";
    static constexpr const char* CKEY_COMBOKEY = "ComboKey";

    constexpr uint32_t F_SHOW_FPS = 1U << 0;
    constexpr uint32_t F_SHOW_FPS_SIMPLE = 1U << 2;
    constexpr uint32_t F_SHOW_FRAMETIME = 1U << 3;
    constexpr uint32_t F_SHOW_FRAMETIME_SIMPLE = 1U << 4;
    constexpr uint32_t F_SHOW_COUNTER = 1U << 5;
    constexpr uint32_t F_SHOW_VRAM_USAGE = 1U << 6;
    constexpr uint32_t F_SHOW_ALL = (F_SHOW_FPS | F_SHOW_FRAMETIME | F_SHOW_COUNTER | F_SHOW_VRAM_USAGE);

    using namespace DirectX;

    DOSD DOSD::m_Instance;

    DOSD::ItemToFlag_T DOSD::ItemToFlag = {
        {"fps", F_SHOW_FPS},
        {"bare_fps", F_SHOW_FPS_SIMPLE},
        {"frametime", F_SHOW_FRAMETIME},
        {"bare_frametime", F_SHOW_FRAMETIME_SIMPLE},
        {"counter", F_SHOW_COUNTER},
        {"vram", F_SHOW_VRAM_USAGE},
        {"all", F_SHOW_ALL}
    };

    DOSD::DOSD()
    {
        bufStats1[0] = 0x0;
        bufStats2[0] = 0x0;
        bufStats3[0] = 0x0;
        bufStats4[0] = 0x0;

        stats.lastUpdate = PerfCounter::Query();
        stats.frameCounter = 0;
        stats.lastFrameCount = 0;
        stats.draw = false;
        stats.warmup = 0;
    }

    void DOSD::LoadConfig()
    {
        m_conf.enabled = GetConfigValue(CKEY_ENABLESTATS, false);
        m_conf.initial = GetConfigValue(CKEY_OSDINITIAL, true);
        m_conf.font = GetConfigValue<Font>(CKEY_STATSFONT, Font::DroidSans);
        m_conf.font_file = GetConfigValue(CKEY_STATSFONTFILE, "");
        m_conf.font_color = GetConfigValue(CKEY_STATSFONTCOLOR, "255 255 255");
        m_conf.font_outline_color = GetConfigValue(CKEY_STATSFONTOUTLINECOLOR, "0 0 0");
        m_conf.outline_size = GetConfigValue(CKEY_STATSOUTLINESIZE, 1.0f);
        m_conf.combo_key = ConfigGetComboKey(GetConfigValue<int32_t>(CKEY_COMBOKEY, 1));
        m_conf.key = GetConfigValue<uint32_t>(CKEY_STATSKEY, DIK_INSERT);
        m_conf.align = ConfigGetStatsRendererAlignment(GetConfigValue(CKEY_STATSALIGN, 1));
        m_conf.offset = GetConfigValue(CKEY_STATSOFFSET, "4 4");
        m_conf.font_scale = GetConfigValue(CKEY_STATSSCALE, "1 1");
        m_conf.font_autoscale = GetConfigValue(CKEY_STATSAUTOSCALE, true);
        m_conf.interval = std::clamp(GetConfigValue(CKEY_STATSINTERVAL, 1.0f), 0.01f, 900.0f);
        m_conf.items = GetConfigValue(CKEY_STATSITEMS, "fps,frametime");
    }

    void DOSD::PostLoadConfig()
    {
        auto rd = IDDispatcher::GetDriver<DRender>();
        if (!rd->IsOK()) {
            Warning("Render driver unavailable, disabling");
            m_conf.enabled = false;
        }

        if (m_conf.enabled)
        {
            if (!m_conf.key || m_conf.key >= InputMap::kMaxMacros) {
                Warning("Invalid toggle key, resetting to default");
                m_conf.key = DIK_INSERT;
            }

            stats.draw = m_conf.initial;
            stats.interval = static_cast<long long>(m_conf.interval * 1000000.0f);

            if (!m_conf.font || m_conf.font >= Font::FontMax) {
                m_conf.font = Font::DroidSans;
            }

            ConfigGetFlags(m_conf.items, stats.flags);
            ConfigParseColors(m_conf.font_color, stats.colors.font);
            ConfigParseColors(m_conf.font_outline_color, stats.colors.outline);
            ConfigParseScale(m_conf.font_scale, stats.scale);
            ConfigParseVector2(m_conf.offset, stats.offset);

            m_dRender = rd;
        }
    }

    void DOSD::RegisterHooks()
    {
        if (m_conf.enabled)
        {
            IEvents::RegisterForEvent(Event::OnD3D11PostCreate, OnD3D11PostCreate_OSD);
            IEvents::RegisterForEvent(Event::OnD3D11PostPostCreate, OnD3D11PostPostCreate_OSD);
            DInput::RegisterForKeyEvents(&inputEventHandler);

            struct PresentHook : JITASM::JITASM {
                PresentHook(uintptr_t targetAddr
                ) : JITASM()
                {
                    Xbyak::Label callLabel;
                    Xbyak::Label retnLabel;

                    mov(edx, dword[rax + 0x30]);
                    call(ptr[rip + callLabel]);
                    jmp(ptr[rip + retnLabel]);

                    L(retnLabel);
                    dq(targetAddr + 0x7);

                    L(callLabel);
                    dq(uintptr_t(StatsPresent_Hook));
                }
            };

            LogPatchBegin("Present wrapper");
            {
                PresentHook code(presentAddr);
                g_branchTrampoline.Write6Branch(presentAddr, code.get());

                Patching::safe_write<uint8_t>(presentAddr + 0x6, 0xCC);
            }
            LogPatchEnd("Present wrapper");
        }
    }

    bool DOSD::Prepare()
    {
        auto rd = IDDispatcher::GetDriver<DRender>();
        return rd && rd->IsOK();
    }

    int DOSD::ConfigGetFontResource(Font param)
    {
        return IDR_DROIDSANS;
    }

    StatsRenderer::Align DOSD::ConfigGetStatsRendererAlignment(int32_t param)
    {
        switch (param)
        {
        case 1:
            return StatsRenderer::Align::TOP_LEFT;
        case 2:
            return StatsRenderer::Align::TOP_RIGHT;
        case 3:
            return StatsRenderer::Align::BOTTOM_LEFT;
        case 4:
            return StatsRenderer::Align::BOTTOM_RIGHT;
        default:
            return StatsRenderer::Align::TOP_LEFT;
        }
    }

    uint32_t DOSD::ConfigGetComboKey(int32_t param)
    {
        switch (param)
        {
        case 1:
            return DIK_LSHIFT;
        case 2:
            return DIK_RSHIFT;
        case 3:
            return DIK_LCONTROL;
        case 4:
            return DIK_RCONTROL;
        case 5:
            return DIK_LALT;
        case 6:
            return DIK_RALT;
        case 7:
            return DIK_LWIN;
        case 8:
            return DIK_RWIN;
        default:
            return DIK_LSHIFT;
        }
    }

    void DOSD::ConfigParseColors(const std::string& in, XMVECTORF32& out)
    {
        stl::vector<float> cols;
        StrHelpers::SplitString<float>(in, ' ', cols);
        if (cols.size() > 2)
        {
            for (int i = 0; i < 3; i++) {
                out.f[i] = std::clamp(cols[i], 0.0f, 255.0f) / 255.0f;
            }
            if (cols.size() > 3) {
                out.f[3] = std::clamp(cols[3], 0.0f, 255.0f) / 255.0f;
            }
            else {
                out.f[3] = 1.0f;
            }
        }
        else {
            out = Colors::White;
        }
    }

    void DOSD::ConfigParseScale(const std::string& in, DirectX::XMFLOAT2A& out)
    {
        stl::vector<float> scale;
        StrHelpers::SplitString<float>(in, ' ', scale);
        if (scale.size() > 0)
        {
            if (scale.size() > 1) {
                out.y = scale[1];
            }
            else {
                out.y = scale[0];
            }

            out.x = scale[0];
        }
        else {
            out.x = 1.0f;
            out.y = 1.0f;
        }
    }

    void DOSD::ConfigParseVector2(const std::string& in, DirectX::XMFLOAT2A& out)
    {
        stl::vector<float> v2;
        StrHelpers::SplitString<float>(in, ' ', v2);
        if (v2.size() > 0) {
            out.x = v2[0];
            if (v2.size() > 1) {
                out.y = v2[1];
            }
        }
    }

    void DOSD::ConfigGetFlags(const std::string& in, uint32_t& out)
    {
        out = 0U;

        stl::vector<std::string> items;
        StrHelpers::SplitString(in, ',', items);
        for (auto& s : items) {
            auto it = ItemToFlag.find(s);
            if (it != ItemToFlag.end()) {
                out |= it->second;
            }
        }
    }

    StatsRenderer::StatsRenderer(
        ID3D11Device* a_pDevice,
        ID3D11DeviceContext* a_pDeviceContext,
        UINT a_bufferX, UINT a_bufferY,
        const XMFLOAT2A& a_offset,
        float a_outlineSize,
        Align a_alignment,
        const XMFLOAT2A& a_scale,
        const XMVECTORF32& a_fontColor,
        const XMVECTORF32& a_outlineColor
    ) :
        m_isLoaded(false),
        m_pDevice(a_pDevice),
        m_bufferSize{ static_cast<float>(a_bufferX), static_cast<float>(a_bufferY) },
        m_offset(a_offset),
        m_alignment(a_alignment),
        m_scale(XMLoadFloat2A(&a_scale)),
        m_fontColor(a_fontColor),
        m_outlineColor(a_outlineColor),
        m_origin(g_XMZero)
    {
        m_commonStates = std::make_unique<CommonStates>(a_pDevice);
        m_spriteBatch = std::make_unique<SpriteBatch>(a_pDeviceContext);

        m_blendState = m_commonStates->NonPremultiplied();

        XMFLOAT2A tmp(a_outlineSize, a_outlineSize);
        m_outlineSize = XMLoadFloat2A(std::addressof(tmp));

        switch (m_alignment)
        {
        case Align::TOP_LEFT:
            m_pos = XMLoadFloat2A(&m_offset);
            break;
        case Align::TOP_RIGHT:
            m_pos = XMLoadFloat2A(&XMFLOAT2A(m_bufferSize.x, m_offset.y));
            break;
        case Align::BOTTOM_LEFT:
            m_pos = XMLoadFloat2A(&XMFLOAT2A(m_offset.x, m_bufferSize.y));
            break;
        case Align::BOTTOM_RIGHT:
            m_pos = XMLoadFloat2A(&XMFLOAT2A(m_bufferSize.x, m_bufferSize.y));
            break;
        }

        AdjustOutline();
    }

    bool StatsRenderer::Load(int resource)
    {
        HRSRC hRes = ::FindResource(ISKSE::g_moduleHandle, MAKEINTRESOURCE(resource), RT_RCDATA);
        if (hRes == nullptr) {
            return false;
        }

        HGLOBAL hData = ::LoadResource(ISKSE::g_moduleHandle, hRes);
        if (hData == nullptr) {
            return false;
        }

        DWORD dSize = ::SizeofResource(ISKSE::g_moduleHandle, hRes);

        LPVOID pData = ::LockResource(hData);
        if (pData == nullptr) {
            goto finish;
        }

        try
        {
            m_font = std::make_unique<SpriteFont>(m_pDevice, reinterpret_cast<uint8_t* const>(pData), dSize);
        }
        catch (std::exception& e)
        {
            m_lastException = e;
            goto finish;
        }

        m_isLoaded = true;

    finish:
        FreeResource(pData);

        return m_isLoaded;
    }

    bool StatsRenderer::Load(const wchar_t* filename)
    {
        try
        {
            m_font = std::make_unique<SpriteFont>(m_pDevice, filename);
        }
        catch (std::exception& e)
        {
            m_lastException = e;
            return false;
        }

        return (m_isLoaded = true);
    }

    void StatsRenderer::DrawStrings()
    {
        if (!m_isLoaded) {
            return;
        }

        try {
            auto text = m_drawString.c_str();
            auto sb = m_spriteBatch.get();

            m_spriteBatch->Begin(SpriteSortMode_Deferred, m_blendState);
            m_font->DrawString(sb, text, m_outlinePos[0], m_outlineColor, 0.0f, m_origin, m_scale);
            m_font->DrawString(sb, text, m_outlinePos[1], m_outlineColor, 0.0f, m_origin, m_scale);
            m_font->DrawString(sb, text, m_outlinePos[2], m_outlineColor, 0.0f, m_origin, m_scale);
            m_font->DrawString(sb, text, m_outlinePos[3], m_outlineColor, 0.0f, m_origin, m_scale);
            m_font->DrawString(sb, text, m_pos, m_fontColor, 0.0f, m_origin, m_scale);
            m_spriteBatch->End();

        }
        catch (const std::exception&)
        {
        }
    }

    void StatsRenderer::Update()
    {
        if (!m_isLoaded) {
            return;
        }

        try {
            UpdateStrings();
            AdjustPosition();
        }
        catch (const std::exception&)
        {
        }
    }

    void StatsRenderer::UpdateStrings()
    {
        stl::wostringstream ss;

        for (const auto& f : m_callbacks) {
            auto buf = f();
            if (buf != nullptr && buf[0] != 0x0) {
                ss << buf << std::endl;
            }
        }

        m_drawString = ss.str();
    }

    void StatsRenderer::AdjustPosition()
    {

        switch (m_alignment)
        {
        case Align::TOP_RIGHT:
        {
            auto w = m_font->MeasureString(m_drawString.c_str(), false);

            auto x = m_bufferSize.x - (w.m128_f32[0] * m_scale.m128_f32[0]) - m_offset.x;
            if (x < m_pos.m128_f32[0]) {
                m_pos.m128_f32[0] = x;
                AdjustOutline();
            }
        }
        break;
        case Align::BOTTOM_LEFT:
        {
            auto w = m_font->MeasureString(m_drawString.c_str(), false);

            m_pos.m128_f32[1] = m_bufferSize.y - (w.m128_f32[1] * m_scale.m128_f32[1]) - m_offset.y;

            AdjustOutline();
        }
        break;
        case Align::BOTTOM_RIGHT:
        {
            auto w = m_font->MeasureString(m_drawString.c_str(), false);

            auto x = m_bufferSize.x - (w.m128_f32[0] * m_scale.m128_f32[0]) - m_offset.x;
            if (x < m_pos.m128_f32[0]) {
                m_pos.m128_f32[0] = x;
            }
            m_pos.m128_f32[1] = m_bufferSize.y - (w.m128_f32[1] * m_scale.m128_f32[1]) - m_offset.y;

            AdjustOutline();
        }
        break;
        }
    }

    void StatsRenderer::AdjustOutline()
    {
        m_outlinePos[0] = _mm_add_ps(m_pos, m_outlineSize);
        m_outlinePos[1] = _mm_addsub_ps(m_pos, m_outlineSize);
        m_outlinePos[2] = _mm_sub_ps(m_pos, m_outlineSize);
        m_outlinePos[3].m128_f32[0] = m_pos.m128_f32[0] + m_outlineSize.m128_f32[0];
        m_outlinePos[3].m128_f32[1] = m_pos.m128_f32[1] - m_outlineSize.m128_f32[1];
    }


    void DOSD::KeyPressHandler::ReceiveEvent(KeyEvent ev, UInt32 keyCode)
    {
        switch (ev)
        {
        case KeyEvent::KeyDown:
        {
            if (keyCode == m_Instance.m_conf.combo_key) {
                combo_down = true;
            }
            else if (keyCode == m_Instance.m_conf.key) {
                if (combo_down) {
                    if (!m_Instance.stats.draw) {
                        m_Instance.stats.warmup = 2;
                        m_Instance.stats.draw = true;
                    }
                    else {
                        m_Instance.stats.draw = false;
                    }
                }
            }
        }
        break;
        case KeyEvent::KeyUp:
        {
            if (keyCode == m_Instance.m_conf.combo_key) {
                combo_down = false;
            }
        }
        break;
        }
    }

    const wchar_t* DOSD::StatsRendererCallback_FPS()
    {
        ::_snwprintf_s(m_Instance.bufStats1, _TRUNCATE,
            L"FPS: %.1f",
            1.0 / (m_Instance.stats.cur.frametime / 1000000.0));

        return m_Instance.bufStats1;
    }

    const wchar_t* DOSD::StatsRendererCallback_SimpleFPS()
    {
        ::_snwprintf_s(m_Instance.bufStats1, _TRUNCATE,
            L"%.1f",
            1.0 / (m_Instance.stats.cur.frametime / 1000000.0));

        return m_Instance.bufStats1;
    }

    const wchar_t* DOSD::StatsRendererCallback_Frametime()
    {
        ::_snwprintf_s(m_Instance.bufStats2, _TRUNCATE,
            L"Frametime: %.2f ms",
            m_Instance.stats.cur.frametime / 1000.0);

        return m_Instance.bufStats2;
    }

    const wchar_t* DOSD::StatsRendererCallback_SimpleFrametime()
    {
        ::_snwprintf_s(m_Instance.bufStats2, _TRUNCATE,
            L"%.2f",
            m_Instance.stats.cur.frametime / 1000.0);

        return m_Instance.bufStats2;
    }

    const wchar_t* DOSD::StatsRendererCallback_Counter()
    {
        ::_snwprintf_s(m_Instance.bufStats3, _TRUNCATE,
            L"Frames: %llu",
            m_Instance.stats.frameCounter);

        return m_Instance.bufStats3;
    }

    const wchar_t* DOSD::StatsRendererCallback_VRAM()
    {
        DXGI_QUERY_VIDEO_MEMORY_INFO info;
        if (SUCCEEDED(m_Instance.m_adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, std::addressof(info))))
        {
            ::_snwprintf_s(m_Instance.bufStats4, _TRUNCATE,
                L"VRAM: %llu/%llu",
                info.CurrentUsage / (1024 * 1024),
                info.Budget / (1024 * 1024));
        }
        else {
            _snwprintf_s(m_Instance.bufStats4, _TRUNCATE, L"VRAM: QUERY ERROR");
        }

        return m_Instance.bufStats4;
    }

    HRESULT STDMETHODCALLTYPE DOSD::StatsPresent_Hook(
        IDXGISwapChain* pSwapChain,
        UINT SyncInterval,
        UINT PresentFlags)
    {
        if (m_Instance.stats.draw) {
            if (!m_Instance.stats.warmup)
                m_Instance.statsRenderer->DrawStrings();
        }

        HRESULT hr = pSwapChain->Present(SyncInterval, PresentFlags);

        m_Instance.stats.frameCounter++;

        if (m_Instance.stats.draw) {

            auto e = PerfCounter::Query();
            auto deltaT = PerfCounter::delta_us(m_Instance.stats.lastUpdate, e);

            if (m_Instance.stats.warmup || deltaT >= m_Instance.stats.interval)
            {
                auto deltaFC =
                    m_Instance.stats.frameCounter -
                    m_Instance.stats.lastFrameCount;

                m_Instance.stats.cur.frametime =
                    static_cast<double>(deltaT) / static_cast<double>(deltaFC);

                m_Instance.stats.lastFrameCount =
                    m_Instance.stats.frameCounter;
                m_Instance.stats.lastUpdate = e;

                if (m_Instance.stats.warmup) {
                    m_Instance.stats.warmup--;
                }

                m_Instance.statsRenderer->Update();
            }
        }

        return hr;
    };

    void DOSD::OnD3D11PostCreate_OSD(Event code, void* data)
    {
        auto info = reinterpret_cast<D3D11CreateEventPost*>(data);

        m_Instance.statsRenderer = std::make_unique<StatsRenderer>(
            info->m_pDevice, info->m_pImmediateContext,
            info->m_pSwapChainDesc->BufferDesc.Width,
            info->m_pSwapChainDesc->BufferDesc.Height,
            m_Instance.stats.offset,
            m_Instance.m_conf.outline_size,
            m_Instance.m_conf.align,
            m_Instance.stats.scale,
            m_Instance.stats.colors.font,
            m_Instance.stats.colors.outline);

        bool res = false;
        bool isCustom = false;

        if (!m_Instance.m_conf.font_file.empty())
        {
            std::wostringstream ss;
            ss << OSD_FONT_PATH << StrHelpers::ToWString(
                m_Instance.m_conf.font_file);

            auto file = ss.str();

            m_Instance.Debug("Loading OSD font from '%s'", StrHelpers::ToNative(file).c_str());

            if (!(res = m_Instance.statsRenderer->Load(file.c_str()))) {
                m_Instance.Warning("Couldn't load font, falling back to built-in");
            }
            else {
                isCustom = true;
            }
        }

        if (!res) {
            res = m_Instance.statsRenderer->Load(
                ConfigGetFontResource(m_Instance.m_conf.font));
        }

        if (res)
        {
            if (m_Instance.stats.flags & F_SHOW_FPS_SIMPLE) {
                m_Instance.AddStatsCallback(StatsRendererCallback_SimpleFPS);
            }
            else if (m_Instance.stats.flags & F_SHOW_FPS) {
                m_Instance.AddStatsCallback(StatsRendererCallback_FPS);
            }
            if (m_Instance.stats.flags & F_SHOW_FRAMETIME_SIMPLE) {
                m_Instance.AddStatsCallback(StatsRendererCallback_SimpleFrametime);
            }
            else if (m_Instance.stats.flags & F_SHOW_FRAMETIME) {
                m_Instance.AddStatsCallback(StatsRendererCallback_Frametime);
            }
            if (m_Instance.stats.flags & F_SHOW_COUNTER) {
                m_Instance.AddStatsCallback(StatsRendererCallback_Counter);
            }

            if (m_Instance.stats.flags & F_SHOW_VRAM_USAGE)
            {
                if (SUCCEEDED(info->m_pAdapter->QueryInterface(IID_PPV_ARGS(m_Instance.m_adapter.GetAddressOf()))))
                {
                    m_Instance.AddStatsCallback(StatsRendererCallback_VRAM);
                }
            }

            if (!isCustom) {
                m_Instance.statsRenderer->MulScale(XMFLOAT2A(1.0f, 0.9f));
            }

        }
        else {
            m_Instance.Error("Couldn't load OSD renderer");
        }
    }

    void DOSD::OnD3D11PostPostCreate_OSD(Event code, void* data)
    {
        auto info = static_cast<D3D11CreateEventPost*>(data);

        if (!m_Instance.statsRenderer->IsLoaded())
            return;

        if (m_Instance.m_conf.font_autoscale)
        {
            auto numCallbacks = m_Instance.statsRenderer->GetNumCallbacks();
            if (numCallbacks > 2) {
                m_Instance.statsRenderer->MulScale(0.6f);
            }
            else if (numCallbacks > 1) {
                m_Instance.statsRenderer->MulScale(0.7f);
            }
        }
    }
}