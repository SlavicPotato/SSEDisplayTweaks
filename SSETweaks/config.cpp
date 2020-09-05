#include "pch.h"

namespace SDT
{
    INIReader IConfig::m_confReader;

    int IConfig::Load()
    {
        m_confReader.Load(PLUGIN_INI_FILE);
        return m_confReader.ParseError();
    }

    float IConfig::GetConfigValue(const char* sect, const char* key, float default) const
    {
        return m_confReader.GetFloat(sect, key, default);
    }

    double IConfig::GetConfigValue(const char* sect, const char* key, double default) const
    {
        return m_confReader.GetReal(sect, key, default);
    }

    bool IConfig::GetConfigValue(const char* sect, const char* key, bool default) const
    {
        return m_confReader.GetBoolean(sect, key, default);
    }

    std::string IConfig::GetConfigValue(const char* sect, const char* key, const char* default) const
    {
        return m_confReader.Get(sect, key, default);
    }
}