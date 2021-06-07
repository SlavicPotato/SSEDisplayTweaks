#include "pch.h"

namespace SDT
{
    IDriver::IDriver() :
        m_Initialized(false),
        m_IsOK(false)
    {
        IDDispatcher::RegisterDriver(this);
    };

    bool IDriver::Initialize()
    {
        if (m_Initialized) {
            return false;
        }

        LoadConfig();
        PostLoadConfig();
        RegisterHooks();

        if (IsOK()) {
            if (InstallHooks()) { // can't softfail atm
                Patch();
            }
            else {
                SetOK(false);
            }
        }

        m_Initialized = true;

        return true;
    };


    void IHook::RegisterHook(std::uintptr_t target, std::uintptr_t hook)
    {
        m_hooks.emplace_back(target, hook, HookDescriptor::HookType::kWR5Call);
    }

    void IHook::RegisterHook(std::uintptr_t target, std::uintptr_t hook, HookDescriptor::HookType type)
    {
        m_hooks.emplace_back(target, hook, type);
    }

    void IHook::RegisterHook(const HookDescriptor& hdesc)
    {
        m_hooks.emplace_back(hdesc);
    }

    void IHook::RegisterHook(HookDescriptor&& hdesc)
    {
        m_hooks.emplace_back(std::move(hdesc));
    }

    bool IHook::InstallHooks()
    {
        if (!m_hooks.size()) {
            return true;
        }

        uint32_t c = 0;

        for (const auto& hdesc : m_hooks)
        {
            if (hdesc.type == HookDescriptor::HookType::kWR5Call) 
            {
                Debug("BranchTrampoline::Write5Call %llX -> %llX", hdesc.wc_target, hdesc.wc_hook);
                ISKSE::GetBranchTrampoline().Write5Call(hdesc.wc_target, hdesc.wc_hook);
                c++;
            }
            else if (hdesc.type == HookDescriptor::HookType::kWR6Call) 
            {
                Debug("BranchTrampoline::Write6Call %llX -> %llX", hdesc.wc_target, hdesc.wc_hook);
                ISKSE::GetBranchTrampoline().Write6Call(hdesc.wc_target, hdesc.wc_hook);
                c++;
            }
        }

        Debug("%u hook(s) installed", c);

        return true;
    }
}