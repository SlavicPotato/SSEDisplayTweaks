#pragma once

namespace SDT
{
    class IConfig
    {
    public:
        static int Load();

        float GetConfigValue(const char* sect, const char* key, float default) const;
        double GetConfigValue(const char* sect, const char* key, double default) const;
        bool GetConfigValue(const char* sect, const char* key, bool default) const;
        std::string GetConfigValue(const char* sect, const char* key, const char* default) const;

        template <typename T>
        T GetConfigValue(const char* sect, const char* key, T default) const
        {
            return static_cast<T>(m_confReader.GetInteger(sect, key, static_cast<long>(default)));
        }

    private:
        static INIReader m_confReader;
    public:
        IConfig() = default;
        virtual ~IConfig() = default;
    };
}
