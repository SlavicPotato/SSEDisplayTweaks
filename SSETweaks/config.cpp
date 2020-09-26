#include "pch.h"

namespace SDT
{
    INIReader IConfig::m_confReader;

    int IConfig::Load()
    {
        m_confReader.Load(PLUGIN_INI_FILE);
        return m_confReader.ParseError();
    }
}