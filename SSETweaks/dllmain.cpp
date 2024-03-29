#include "pch.h"

namespace SDT
{
	static constexpr const char* CONF_SECT_MAIN        = "Main";
	static constexpr const char* CONF_MAIN_KEY_LOGGING = "LogLevel";

	class Initializer :
		IConfig
	{
	public:
		int Run(const SKSEInterface* a_skse)
		{
			int result = LoadConfiguration();
			if (result != 0)
			{
				gLog.Warning("Unable to load main configuration file (%d)", result);
			}

			gLog.SetLogLevel(ILog::TranslateLogLevel(
				GetConfigValue(CONF_MAIN_KEY_LOGGING, "debug")));

			if (IsCustomLoaded())
			{
				gLog.Message("Custom configuration loaded");
			}

			auto& skse = ISKSE::GetSingleton();

			if (!skse.QueryInterfaces(a_skse))
			{
				gLog.FatalError("Could not query SKSE interfaces");
				return false;
			}

			if (!skse.CreateTrampolines(a_skse))
			{
				return false;
			}

			result = Initialize(a_skse);

			if (result == 0)
			{
				auto usageBranch = skse.GetTrampolineUsage(TrampolineID::kBranch);
				auto usageLocal  = skse.GetTrampolineUsage(TrampolineID::kLocal);

				gLog.Debug(
					"[Trampoline] branch: %zu/%zu, codegen: %zu/%u",
					usageBranch.used,
					usageBranch.total,
					usageLocal.used,
					usageLocal.total);
			}

			ClearConfiguration();

			return result;
		}

	private:
		int Initialize(const SKSEInterface* a_skse)
		{
			if (!IEvents::Initialize())
			{
				return 1;
			}

			if (!IDDispatcher::InitializeDrivers())
			{
				return -1;
			}

			return 0;
		}

		virtual const char* ModuleName() const noexcept
		{
			return CONF_SECT_MAIN;
		}
	};
}

extern "C" {
	bool SKSEPlugin_Query(const SKSEInterface* a_skse, PluginInfo* a_info)
	{
		return SDT::ISKSE::GetSingleton().Query(a_skse, a_info);
	}

	bool SKSEPlugin_Load(const SKSEInterface* skse)
	{
		auto& iskse = SDT::ISKSE::GetSingleton();

		if (IAL::IsAE())
		{
			iskse.SetPluginHandle(skse->GetPluginHandle());
			iskse.OpenLog(skse->runtimeVersion);
		}

		ASSERT(iskse.ModuleHandle() != nullptr);

		gLog.Message(
			"%s version %s (runtime %u.%u.%u.%u)",
			PLUGIN_NAME,
			PLUGIN_VERSION_VERSTRING,
			GET_EXE_VERSION_MAJOR(skse->runtimeVersion),
			GET_EXE_VERSION_MINOR(skse->runtimeVersion),
			GET_EXE_VERSION_BUILD(skse->runtimeVersion),
			GET_EXE_VERSION_SUB(skse->runtimeVersion));

		if (!IAL::IsLoaded())
		{
			WinApi::MessageBoxErrorLog(
				PLUGIN_NAME,
				"Could not load the address library");
			return false;
		}

		if (IAL::HasBadQuery())
		{
			WinApi::MessageBoxErrorFmtLog(
				PLUGIN_NAME,
				"One or more addresses could not be retrieved from the address library\n\nLast bad ID: '%llu'",
				IAL::GetLastBadQueryID());
			return false;
		}

		/*PerfTimer timer;
		timer.Start();*/

		bool ok(false);

		int result = SDT::Initializer().Run(skse);
		if (result == 0)
		{
			ok = true;
		}
		else if (result == -1)
		{
			WinApi::MessageBoxError(
				PLUGIN_NAME,
				"An unrecoverable error has occured during plugin initialization. "
				"Some patches have already been applied.\n"
				"The game process will be terminated for safety reasons.\n\n"
				"See the log for more info.");
			std::_Exit(1);
		}

		/*auto tInit = timer.Stop();

		auto dbEntries = IAL::Size();

		timer.Start();

		IAL::Release();

		auto tUnload = timer.Stop();

		if (ok)
		{
			gLog.Debug(
				"[%s] db load: %.3f ms, init: %.3f ms, db unload: %.3f ms [%zu record(s)]",
				__FUNCTION__,
				IAL::GetLoadTime() * 1000.0f,
				tInit * 1000.0f,
				tUnload * 1000.0f,
				dbEntries);
		}*/

		IAL::Release();

		return ok;
	}

	SKSEPluginVersionData SKSEPlugin_Version = {
		SKSEPluginVersionData::kVersion,

		MAKE_PLUGIN_VERSION(
			PLUGIN_VERSION_MAJOR,
			PLUGIN_VERSION_MINOR,
			PLUGIN_VERSION_REVISION),
		PLUGIN_NAME,
		PLUGIN_AUTHOR,
		"n/a",
		SKSEPluginVersionData::kVersionIndependentEx_NoStructUse,
		SKSEPluginVersionData::kVersionIndependent_AddressLibraryPostAE,
		{ RUNTIME_VERSION_1_6_318, RUNTIME_VERSION_1_6_323, 0 },
		0,
	};
};

BOOL APIENTRY DllMain(
	HMODULE hModule,
	DWORD   ul_reason_for_call,
	LPVOID  lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		SDT::ISKSE::GetSingleton().SetModuleHandle(hModule);
		break;
	}
	return TRUE;
}