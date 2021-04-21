#include "pch.h"

namespace SDT
{
	INIReader IConfig::m_confReader;
	INIReader IConfig::m_confReaderCustom;

	int IConfig::LoadConfiguration()
	{
		m_confReader.Load(PLUGIN_INI_FILE);
		m_confReaderCustom.Load(PLUGIN_INI_CUSTOM_FILE);

		return m_confReader.ParseError();
	}

	void IConfig::ClearConfiguration()
	{
		m_confReader.Clear();
		m_confReaderCustom.Clear();
	}
}