#include "pch.h"

namespace SDT
{
    IDriver::IDriver() {
        IDDispatcher::RegisterDriver(this);
    };

    bool IDriver::Initialize()
    {
        if (b_Initialized) {
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

        //RegisterPapyrusFunctions();

        b_Initialized = true;

        return true;
    };

    /*void IDriver::RegisterPapyrusFunctions()
    {
        if (papyrusFuncRegProc) {
            Debug("Registering Papyrus functions");
            if (!ISKSE::g_papyrus->Register(papyrusFuncRegProc))
            {
                Error("Failed to register Papyrus functions");
            }
        }
    }

    void IDriver::SetPapyrusFuncRegProc(SKSEPapyrusInterface::RegisterFunctions proc)
    {
        papyrusFuncRegProc = proc;
    }*/

    /*void IHook::RegisterHook(PVOID *target, PVOID detour)
    {
        hooks.push_back(HookDescriptor(target, detour));
    }*/

    void IHook::RegisterHook(uintptr_t target, uintptr_t hook)
    {
        hooks.push_back(HookDescriptor(target, hook, HookDescriptor::HookType::kWR5Call));
    }

    void IHook::RegisterHook(uintptr_t target, uintptr_t hook, HookDescriptor::HookType type)
    {
        hooks.push_back(HookDescriptor(target, hook, type));
    }

    void IHook::RegisterHook(HookDescriptor const& hdesc)
    {
        hooks.push_back(hdesc);
    }

    bool IHook::InstallHooks()
    {
        if (!hooks.size()) {
            return true;
        }

        uint32_t c = 0;

        for (const auto& hdesc : hooks) {
            if (hdesc.type == HookDescriptor::kWR5Call) {
                Debug("BranchTrampoline::Write5Call %llX -> %llX", hdesc.wc_target, hdesc.wc_hook);
                g_branchTrampoline.Write5Call(hdesc.wc_target, hdesc.wc_hook);
                c++;
            }
            else if (hdesc.type == HookDescriptor::kWR6Call) {
                Debug("BranchTrampoline::Write6Call %llX -> %llX", hdesc.wc_target, hdesc.wc_hook);
                g_branchTrampoline.Write6Call(hdesc.wc_target, hdesc.wc_hook);
                c++;
            }
        }

        /*LONG result = DetourTransactionBegin();
        if (result != NO_ERROR) {
            Error("DetourTransactionBegin failed: %lu", result);
            return false;
        }

        result = DetourUpdateThread(GetCurrentThread());
        if (result != NO_ERROR) {
            Error("DetourUpdateThread failed: %lu", result);
            goto abort_detours;
        }

        for (const auto &hdesc : hooks) {
            if (hdesc.type != HookDescriptor::kDetours) {
                continue;
            }

            PVOID target, detour;
            PDETOUR_TRAMPOLINE trampoline;
            LONG result = DetourAttachEx(hdesc.target, hdesc.detour, &trampoline, &target, &detour);
            if (result != NO_ERROR) {
                Error("DetourAttach failed: %lu (%llX -> %llX)", result, *hdesc.target, hdesc.detour);
                goto abort_detours;
            }
            else {
                Debug("DetourAttach %llX -> %llX", target, detour);
                c++;
            }
        }

        result = DetourTransactionCommit();
        if (result != NO_ERROR) {
            Error("DetourTransactionCommit failed: %lu", result);
            goto abort_detours;
        }*/

        Debug("%u hook(s) installed", c);

        return true;
        /*
            abort_detours:
                DetourTransactionAbort();
                return false;*/
    }



}