#include "pch.h"

namespace SDT {
    static constexpr const char* CONF_SECT_MAIN = "Main";
    static constexpr const char* CONF_MAIN_KEY_LOGGING = "LogLevel";

    static bool Initialize(const SKSEInterface* skse)
    {
        int result = IConfig::Load();
        if (result != 0) {
            gLog.Warning(
                "WARNING: Unable to load the configuration file (%d)", result);
        }
        else {
            gLog.SetLogLevel(ILog::TranslateLogLevel(
                SDT::IConfigS(CONF_SECT_MAIN).GetConfigValue(CONF_MAIN_KEY_LOGGING, "debug")));
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

        gLog.Debug("[Trampoline] branch: %zu/%zu, codegen: %zu/%u",
            ISKSE::branchTrampolineSize - g_branchTrampoline.Remain(),
            ISKSE::branchTrampolineSize,
            ISKSE::localTrampolineSize - g_localTrampoline.Remain(),
            ISKSE::localTrampolineSize);

        IConfig::Clear();

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
        ASSERT(SDT::ISKSE::moduleHandle != nullptr);

        gLog.Message("%s version %s (runtime %u.%u.%u.%u)",
            PLUGIN_NAME, PLUGIN_VERSION_VERSTRING,
            GET_EXE_VERSION_MAJOR(skse->runtimeVersion),
            GET_EXE_VERSION_MINOR(skse->runtimeVersion),
            GET_EXE_VERSION_BUILD(skse->runtimeVersion),
            GET_EXE_VERSION_SUB(skse->runtimeVersion));

        if (!IAL::IsLoaded()) {
            gLog.FatalError("Could not load the address library, make sure it's installed");
            return false;
        }

        if (IAL::HasBadQuery()) {
            gLog.FatalError("One or more addresses could not be retrieved from the address library");
            return false;
        }

        PerfTimer timer;

        timer.Start();

        bool result = SDT::Initialize(skse);

        auto tInit = timer.Stop();

        timer.Start();

        IAL::Release();

        auto tUnload = timer.Stop();

        if (result) {
            gLog.Debug("[%s] db load: %.3f ms, init: %.3f ms, db unload: %.3f ms", __FUNCTION__,
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
        SDT::ISKSE::moduleHandle = hModule;
        break;
    }
    return TRUE;
}