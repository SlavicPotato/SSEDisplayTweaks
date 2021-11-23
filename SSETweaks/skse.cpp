#include "pch.h"

namespace SDT
{
	ISKSE ISKSE::m_Instance;

	void ISKSE::OnLogOpen()
	{
	}

	const char* ISKSE::GetLogPath() const
	{
		return PLUGIN_LOG_PATH;
	}

	const char* ISKSE::GetPluginName() const
	{
		return PLUGIN_NAME;
	};

	std::uint32_t ISKSE::GetPluginVersion() const
	{
		return MAKE_PLUGIN_VERSION(
			PLUGIN_VERSION_MAJOR,
			PLUGIN_VERSION_MINOR,
			PLUGIN_VERSION_REVISION);
	};

	bool ISKSE::CheckRuntimeVersion(std::uint32_t a_version) const
	{
		return a_version >= RUNTIME_VERSION_1_5_23 &&
		       a_version <= RUNTIME_VERSION_1_5_97;
	}

	bool ISKSE::CheckInterfaceVersion(
		std::uint32_t a_interfaceID,
		std::uint32_t a_interfaceVersion,
		std::uint32_t a_compiledInterfaceVersion) const
	{
		return true;
	}
}
