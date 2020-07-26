#include "pch.h"

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

namespace SDT
{
    constexpr char* SECTION_OSD = "OSD";

    constexpr char* CKEY_ENABLESTATS = "Enable";
    constexpr char* CKEY_OSDINITIAL = "InitiallyOn";
    constexpr char* CKEY_STATSFONT = "Font";
    constexpr char* CKEY_STATSFONTFILE = "FontFile";
    constexpr char* CKEY_STATSFONTCOLOR = "Color";
    constexpr char* CKEY_STATSFONTOUTLINECOLOR = "OutlineColor";
    constexpr char* CKEY_STATSOUTLINESIZE = "OutlineOffset";
    constexpr char* CKEY_STATSKEY = "ToggleKey";
    constexpr char* CKEY_STATSALIGN = "Align";
    constexpr char* CKEY_STATSOFFSET = "Offset";
    constexpr char* CKEY_STATSSCALE = "Scale";
    constexpr char* CKEY_STATSAUTOSCALE = "AutoScale";
    constexpr char* CKEY_STATSINTERVAL = "UpdateInterval";
    constexpr char* CKEY_STATSFLAGS = "Flags";
    constexpr char* CKEY_STATSITEMS = "Show";
    constexpr char* CKEY_COMBOKEY = "ComboKey";

    constexpr uint32_t F_SHOW_FPS = 0x00000001U;
    constexpr uint32_t F_SHOW_FPS_SIMPLE = 0x00000002U;
    constexpr uint32_t F_SHOW_FRAMETIME = 0x00000004U;
    constexpr uint32_t F_SHOW_FRAMETIME_SIMPLE = 0x00000008U;
    constexpr uint32_t F_SHOW_COUNTER = 0x00000010U;
    constexpr uint32_t F_SHOW_ALL = (F_SHOW_FPS | F_SHOW_FRAMETIME | F_SHOW_COUNTER);

    using namespace std;
    using namespace DirectX;

    DOSD DOSD::m_Instance;

    DOSD::ItemToFlag_T DOSD::ItemToFlag = {
        {"fps", F_SHOW_FPS},
        {"bare_fps", F_SHOW_FPS_SIMPLE},
        {"frametime", F_SHOW_FRAMETIME},
        {"bare_frametime", F_SHOW_FRAMETIME_SIMPLE},
        {"counter", F_SHOW_COUNTER},
        {"all", F_SHOW_ALL}
    };

    DOSD::DOSD()
    {
        bufStats1[0] = 0x0;
        bufStats2[0] = 0x0;
        bufStats3[0] = 0x0;

        stats.lastUpdate = PerfCounter::Query();
        stats.frameCounter = 0;
        stats.lastFrameCount = 0;
        stats.draw = false;
    }

    void DOSD::LoadConfig()
    {
        conf.enabled = GetConfigValue(SECTION_OSD, CKEY_ENABLESTATS, false);
        conf.initial = GetConfigValue(SECTION_OSD, CKEY_OSDINITIAL, true);
        conf.font = GetConfigValue<Font>(SECTION_OSD, CKEY_STATSFONT, Font::DroidSans);
        conf.font_file = GetConfigValue(SECTION_OSD, CKEY_STATSFONTFILE, "");
        conf.font_color = GetConfigValue(SECTION_OSD, CKEY_STATSFONTCOLOR, "255 255 255");
        conf.font_outline_color = GetConfigValue(SECTION_OSD, CKEY_STATSFONTOUTLINECOLOR, "0 0 0");
        conf.outline_size = GetConfigValue(SECTION_OSD, CKEY_STATSOUTLINESIZE, 1.0f);
        conf.combo_key = ConfigGetComboKey(GetConfigValue<int32_t>(SECTION_OSD, CKEY_COMBOKEY, 1));
        conf.key = GetConfigValue<uint32_t>(SECTION_OSD, CKEY_STATSKEY, DIK_INSERT);
        conf.align = ConfigGetStatsRendererAlignment(GetConfigValue(SECTION_OSD, CKEY_STATSALIGN, 1));
        conf.offset = GetConfigValue(SECTION_OSD, CKEY_STATSOFFSET, "4 4");
        conf.font_scale = GetConfigValue(SECTION_OSD, CKEY_STATSSCALE, "1 1");
        conf.font_autoscale = GetConfigValue(SECTION_OSD, CKEY_STATSAUTOSCALE, true);
        conf.interval = clamp(GetConfigValue(SECTION_OSD, CKEY_STATSINTERVAL, 1.0f), 0.01f, 900.0f);
        conf.items = GetConfigValue(SECTION_OSD, CKEY_STATSITEMS, "fps,frametime");
    }

    void DOSD::PostLoadConfig()
    {
        auto rd = IDDispatcher::GetDriver<DRender>(DRIVER_RENDER);
        if (!rd->IsOK()) {
            Warning("Render driver unavailable, disabling");
            conf.enabled = false;
        }

        if (conf.enabled)
        {
            if (!conf.key || conf.key >= InputMap::kMaxMacros) {
                Warning("Invalid toggle key, resetting to default");
                conf.key = DIK_INSERT;
            }

            stats.draw = conf.initial;
            stats.interval = static_cast<long long>(conf.interval * 1000000.0f);

            if (!conf.font || conf.font >= Font::FontMax) {
                conf.font = Font::DroidSans;
            }

            ConfigGetFlags(conf.items, stats.flags);
            ConfigParseColors(conf.font_color, stats.colors.font);
            ConfigParseColors(conf.font_outline_color, stats.colors.outline);
            ConfigParseScale(conf.font_scale, stats.scale);
            ConfigParseVector2(conf.offset, stats.offset);
        }
    }

    void DOSD::RegisterHooks()
    {
        if (conf.enabled)
        {
            IEvents::RegisterForEvent(Event::OnD3D11PostCreate, OnD3D11PostCreate_OSD);
            IEvents::RegisterForEvent(Event::OnD3D11PostPostCreate, OnD3D11PostPostCreate_OSD);
            DInput::RegisterForKeyEvents(&inputEventHandler);

            struct PresentHook : JITASM {
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
        auto rd = IDDispatcher::GetDriver<DRender>(DRIVER_RENDER);
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

    void DOSD::ConfigParseColors(const string& in, XMVECTORF32& out)
    {
        vector<float> cols;
        StrHelpers::SplitString<float>(in, ' ', cols);
        if (cols.size() > 2)
        {
            for (int i = 0; i < 3; i++) {
                out.f[i] = clamp(cols[i], 0.0f, 255.0f) / 255.0f;
            }
            if (cols.size() > 3) {
                out.f[3] = clamp(cols[3], 0.0f, 255.0f) / 255.0f;
            }
            else {
                out.f[3] = 1.0f;
            }
        }
        else {
            out = Colors::White;
        }
    }

    void DOSD::ConfigParseScale(const std::string& in, DirectX::XMFLOAT2& out)
    {
        vector<float> scale;
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

    void DOSD::ConfigParseVector2(const std::string& in, DirectX::XMFLOAT2& out)
    {
        vector<float> v2;
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

        vector<string> items;
        StrHelpers::SplitString(in, ',', items);
        for (auto s : items) {
            transform(s.begin(), s.end(), s.begin(), ::tolower);
            auto it = ItemToFlag.find(s);
            if (it != ItemToFlag.end()) {
                out |= it->second;
            }
        }
    }

    StatsRenderer::StatsRenderer(
        ID3D11Device* pDevice,
        ID3D11DeviceContext* pDeviceContext,
        UINT bufferX, UINT bufferY,
        const XMFLOAT2& offset,
        float outlineSize,
        Align alignment,
        const XMFLOAT2& _scale,
        const XMVECTORF32& fontColor,
        const XMVECTORF32& outlineColor
    ) :
        isLoaded(false)
    {
        m_commonStates = make_unique<CommonStates>(pDevice);
        m_spriteBatch = make_unique<SpriteBatch>(pDeviceContext);

        m_blendState = m_commonStates->NonPremultiplied();

        m_pDevice = pDevice;

        bx = static_cast<float>(bufferX);
        by = static_cast<float>(bufferY);
        off = offset;
        align = alignment;
        origin = XMFLOAT2(0.0f, 0.0f);
        scale = _scale;
        fcol = fontColor;
        ocol = outlineColor;
        ol = outlineSize;

        switch (align)
        {
        case Align::TOP_LEFT:
            pos = off;
            break;
        case Align::TOP_RIGHT:
            pos = XMFLOAT2(bx, off.y);
            break;
        case Align::BOTTOM_LEFT:
            pos = XMFLOAT2(off.x, by);
            break;
        case Align::BOTTOM_RIGHT:
            pos = XMFLOAT2(bx, by);
            break;
        }

        AdjustOutline();
    }

    bool StatsRenderer::Load(int resource)
    {
        HRSRC hRes = ::FindResource(ISKSE::g_moduleHandle, MAKEINTRESOURCE(resource), RT_RCDATA);
        if (hRes == NULL) {
            return false;
        }

        HGLOBAL hData = ::LoadResource(ISKSE::g_moduleHandle, hRes);
        if (hData == NULL) {
            return false;
        }

        DWORD dSize = ::SizeofResource(ISKSE::g_moduleHandle, hRes);

        LPVOID pData = ::LockResource(hData);
        if (pData == NULL) {
            return false;
        }

        try
        {
            m_font = std::make_unique<SpriteFont>(m_pDevice, reinterpret_cast<uint8_t* const>(pData), dSize);
        }
        catch (std::exception& e)
        {
            e_last = e;
            goto finish;
        }

        isLoaded = true;

    finish:
        FreeResource(pData);

        return isLoaded;
    }

    bool StatsRenderer::Load(const wchar_t* filename)
    {
        try
        {
            m_font = make_unique<SpriteFont>(m_pDevice, filename);
        }
        catch (std::exception& e)
        {
            e_last = e;
            return false;
        }

        return (isLoaded = true);
    }

    void StatsRenderer::AddCallback(Callback cb)
    {
        callbacks.push_back(cb);
    }

    size_t StatsRenderer::GetNumCallbacks()
    {
        return callbacks.size();
    }

    void StatsRenderer::MulScale(float mul)
    {
        scale.x *= mul;
        scale.y *= mul;
    }

    void StatsRenderer::MulScale(const DirectX::XMFLOAT2& mul)
    {
        scale.x *= mul.x;
        scale.y *= mul.y;
    }

    void StatsRenderer::DrawStrings() const
    {
        if (!isLoaded) {
            return;
        }

        try {
            auto text = s.c_str();
            auto sb = m_spriteBatch.get();

            m_spriteBatch->Begin(SpriteSortMode_Deferred, m_blendState);
            m_font->DrawString(sb, text, opos[0], ocol, 0.0f, origin, scale);
            m_font->DrawString(sb, text, opos[1], ocol, 0.0f, origin, scale);
            m_font->DrawString(sb, text, opos[2], ocol, 0.0f, origin, scale);
            m_font->DrawString(sb, text, opos[3], ocol, 0.0f, origin, scale);
            m_font->DrawString(sb, text, pos, fcol, 0.0f, origin, scale);
            m_spriteBatch->End();
        }
        catch (...)
        {
        }
    }

    void StatsRenderer::Update()
    {
        if (!isLoaded) {
            return;
        }

        try {
            UpdateStrings();
            AdjustPosition();
        }
        catch (...)
        {
        }
    }

    void StatsRenderer::UpdateStrings()
    {
        ss.str(L"");
        ss.clear();

        for (const auto f : callbacks) {
            auto buf = f();
            if (buf != nullptr && buf[0] != 0x0) {
                ss << buf << std::endl;
            }
        }

        s = ss.str();
    }

    void StatsRenderer::AdjustPosition()
    {
        switch (align)
        {
        case Align::TOP_RIGHT:
        {
            XMFLOAT2 sw;
            XMStoreFloat2(&sw, m_font->MeasureString(s.c_str(), false));

            auto x = bx - (sw.x * scale.x) - off.x;
            if (x < pos.x) {
                pos.x = x;
                AdjustOutline();
            }
        }
        break;
        case Align::BOTTOM_LEFT:
        {
            XMFLOAT2 sw;
            XMStoreFloat2(&sw, m_font->MeasureString(s.c_str(), false));

            pos.y = by - (sw.y * scale.y) - off.y;
            AdjustOutline();
        }
        break;
        case Align::BOTTOM_RIGHT:
        {
            XMFLOAT2 sw;
            XMStoreFloat2(&sw, m_font->MeasureString(s.c_str(), false));

            auto x = bx - (sw.x * scale.x) - off.x;
            if (x < pos.x) {
                pos.x = x;
            }
            pos.y = by - (sw.y * scale.y) - off.y;
            AdjustOutline();
        }
        break;
        }
    }

    void StatsRenderer::AdjustOutline()
    {
        opos[0] = XMFLOAT2(pos.x + ol, pos.y + ol);
        opos[1] = XMFLOAT2(pos.x - ol, pos.y + ol);
        opos[2] = XMFLOAT2(pos.x - ol, pos.y - ol);
        opos[3] = XMFLOAT2(pos.x + ol, pos.y - ol);
    }

    void DOSD::KeyPressHandler::ReceiveEvent(KeyEvent ev, UInt32 keyCode)
    {
        switch (ev)
        {
        case KeyEvent::KeyDown:
        {
            if (keyCode == m_Instance.conf.combo_key) {
                combo_down = true;
            }
            else if (keyCode == m_Instance.conf.key) {
                if (combo_down) {
                    m_Instance.stats.draw = !m_Instance.stats.draw;
                }
            }
        }
        break;
        case KeyEvent::KeyUp:
        {
            if (keyCode == m_Instance.conf.combo_key) {
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

    HRESULT STDMETHODCALLTYPE DOSD::StatsPresent_Hook(
        IDXGISwapChain* pSwapChain,
        UINT SyncInterval,
        UINT PresentFlags)
    {
        if (m_Instance.stats.draw) {
            m_Instance.statsRenderer->DrawStrings();
        }

        HRESULT hr = pSwapChain->Present(SyncInterval, PresentFlags);

        auto e = PerfCounter::Query();
        auto deltaT = PerfCounter::delta_us(
            m_Instance.stats.lastUpdate, e);

        m_Instance.stats.frameCounter++;

        if (deltaT >= m_Instance.stats.interval)
        {
            auto deltaFC =
                m_Instance.stats.frameCounter -
                m_Instance.stats.lastFrameCount;

            m_Instance.stats.cur.frametime =
                static_cast<double>(deltaT) /
                static_cast<double>(deltaFC);

            m_Instance.stats.lastFrameCount =
                m_Instance.stats.frameCounter;
            m_Instance.stats.lastUpdate = e;

            m_Instance.statsRenderer->Update();
        }

        return hr;
    };

    void DOSD::OnD3D11PostCreate_OSD(Event code, void* data)
    {
        auto info = reinterpret_cast<D3D11CreateEventPost*>(data);

        m_Instance.statsRenderer = make_unique<StatsRenderer>(
            info->m_pDevice, info->m_pImmediateContext,
            info->m_pSwapChainDesc->BufferDesc.Width,
            info->m_pSwapChainDesc->BufferDesc.Height,
            m_Instance.stats.offset,
            m_Instance.conf.outline_size,
            m_Instance.conf.align,
            m_Instance.stats.scale,
            m_Instance.stats.colors.font,
            m_Instance.stats.colors.outline);

        bool res = false;
        bool isCustom = false;

        if (!m_Instance.conf.font_file.empty())
        {
            wostringstream ss;
            ss << OSD_FONT_PATH << StrHelpers::ToWString(
                m_Instance.conf.font_file);

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
                ConfigGetFontResource(m_Instance.conf.font));
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

            if (!isCustom) {
                m_Instance.statsRenderer->MulScale(XMFLOAT2(1.0f, 0.9f));
            }
        }
        else {
            m_Instance.Error("Couldn't load OSD renderer");
        }
    }

    void DOSD::OnD3D11PostPostCreate_OSD(Event code, void* data)
    {
        if (m_Instance.statsRenderer->IsLoaded()) {
            if (m_Instance.conf.font_autoscale)
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
}