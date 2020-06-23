#include "pch.h"

namespace SDT {
    constexpr char* CONF_SECT_MAIN = "Main";
    constexpr char* CONF_MAIN_KEY_LOGGING = "LogLevel";

    static bool Initialize(const SKSEInterface* skse)
    {
        int result = IConfig::Load();
        if (result != 0) {
            _WARNING(
                "WARNING: Unable to load the configuration file (%d)", result);
        }
        else {
            SDT::IConfig conf;
            gLog.SetLogLevel(SDT::ILog::TranslateLogLevel(
                conf.GetConfigValue(CONF_SECT_MAIN, CONF_MAIN_KEY_LOGGING, "debug")));
        }

        if (!ISKSE::Initialize(skse)) {
            return false;
        }

        if (!IEvents::Initialize()) {
            return false;
        }

        if (!IDDispatcher::InitializeDrivers()) {
            return false;
        }

        // Shouldn't be necessary since SKSE already flushes after plugin load
        FlushInstructionCache(GetCurrentProcess(), NULL, 0);

        _DMESSAGE("[Trampoline] branch: %zu/%zu, codegen: %zu/%u",
            MAX_TRAMPOLINE_BRANCH - g_branchTrampoline.Remain(),
            MAX_TRAMPOLINE_BRANCH,
            MAX_TRAMPOLINE_CODEGEN - g_localTrampoline.Remain(),
            MAX_TRAMPOLINE_CODEGEN);

        return true;
    }
}

extern "C"
{
    bool SKSEPlugin_Query(const SKSEInterface* skse, PluginInfo* info)
    {
        return SDT::ISKSE::Query(skse, info);
    }

    bool SKSEPlugin_Load(const SKSEInterface* skse)
    {
        _MESSAGE("%s version %s (runtime %d.%d.%d.%d)",
            PLUGIN_NAME, PLUGIN_VERSION_VERSTRING,
            GET_EXE_VERSION_MAJOR(skse->runtimeVersion),
            GET_EXE_VERSION_MINOR(skse->runtimeVersion),
            GET_EXE_VERSION_BUILD(skse->runtimeVersion),
            GET_EXE_VERSION_SUB(skse->runtimeVersion));

        if (!IAL::IsLoaded()) {
            _FATALERROR("Could not load the address library");
            return false;
        }

        if (IAL::HasBadQuery()) {
            _FATALERROR("One or more addresses could not be retrieved from the database");
            return false;
        }

        auto tStart = PerfCounter::Query();

        bool result = SDT::Initialize(skse);

        auto tInit = PerfCounter::delta(
            tStart, PerfCounter::Query());

        tStart = PerfCounter::Query();

        IAL::Release();

        auto tUnload = PerfCounter::delta(
            tStart, PerfCounter::Query());

        if (result) {
            _DMESSAGE("[%s] db load: %.3f ms, init: %.3f ms, db unload: %.3f ms", __FUNCTION__,
                IAL::GetLoadTime() * 1000.0f, tInit * 1000.0f, tUnload * 1000.0f);
        }

        return result;
    }
};

BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        SDT::ISKSE::g_moduleHandle = hModule;
        break;
    }
    return TRUE;
}