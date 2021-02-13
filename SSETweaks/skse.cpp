#include "pch.h"

namespace SDT
{
    constexpr uint32_t MIN_MESSAGING_INTERFACE_VER = 2U;

    HMODULE ISKSE::g_moduleHandle = nullptr;
    PluginHandle ISKSE::g_pluginHandle = kPluginHandle_Invalid;

    SKSEMessagingInterface* ISKSE::g_messaging;

    size_t ISKSE::branchTrampolineSize;
    size_t ISKSE::localTrampolineSize;

    bool ISKSE::Query(const SKSEInterface* skse, PluginInfo* info)
    {
        gLog.OpenRelative(CSIDL_MYDOCUMENTS, PLUGIN_LOG_PATH);
        gLog.SetLogLevel(IDebugLog::LogLevel::Message);

        info->infoVersion = PluginInfo::kInfoVersion;
        info->name = PLUGIN_NAME;
        info->version = MAKE_PLUGIN_VERSION(
            PLUGIN_VERSION_MAJOR,
            PLUGIN_VERSION_MINOR,
            PLUGIN_VERSION_REVISION);

        if (skse->isEditor) {
            gLog.FatalError("Loaded in editor, marking as incompatible");
            return false;
        }

        if (skse->runtimeVersion < RUNTIME_VERSION_1_5_23)
        {
            gLog.FatalError("Unsupported runtime version %u.%u.%u.%u, expected >= %u.%u.%u.%u",
                GET_EXE_VERSION_MAJOR(skse->runtimeVersion),
                GET_EXE_VERSION_MINOR(skse->runtimeVersion),
                GET_EXE_VERSION_BUILD(skse->runtimeVersion),
                GET_EXE_VERSION_SUB(skse->runtimeVersion),
                GET_EXE_VERSION_MAJOR(RUNTIME_VERSION_1_5_23),
                GET_EXE_VERSION_MINOR(RUNTIME_VERSION_1_5_23),
                GET_EXE_VERSION_BUILD(RUNTIME_VERSION_1_5_23),
                GET_EXE_VERSION_SUB(RUNTIME_VERSION_1_5_23));
            return false;
        }

        g_pluginHandle = skse->GetPluginHandle();

        return true;
    }

    bool ISKSE::Initialize(const SKSEInterface* skse)
    {
        g_messaging = reinterpret_cast<SKSEMessagingInterface*>(skse->QueryInterface(kInterface_Messaging));
        if (g_messaging == nullptr) {
            gLog.FatalError("Could not get messaging interface");
            return false;
        }

        if (g_messaging->interfaceVersion < MIN_MESSAGING_INTERFACE_VER) {
            gLog.FatalError("Messaging interface too old (%u expected >= %u)",
                g_messaging->interfaceVersion, MIN_MESSAGING_INTERFACE_VER);
            return false;
        }

        branchTrampolineSize = Hook::InitBranchTrampoline(skse, MAX_TRAMPOLINE_BRANCH);
        if (!branchTrampolineSize) {
            gLog.FatalError("Could not create branch trampoline.");
            return false;
        }

        localTrampolineSize = Hook::InitLocalTrampoline(skse, MAX_TRAMPOLINE_CODEGEN);
        if (!localTrampolineSize) {
            gLog.FatalError("Could not create codegen trampoline.");
            return false;
        }

        return true;
    }
}
