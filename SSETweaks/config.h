#pragma once

namespace SDT
{
    class IConfig
    {
    public:
        static int Load();

        inline float GetConfigValue(const char* sect, const char* key, float default) const
        {
            return m_confReader.GetFloat(sect, key, default);
        }

        inline double GetConfigValue(const char* sect, const char* key, double default) const
        {
            return m_confReader.GetReal(sect, key, default);
        }

        inline bool GetConfigValue(const char* sect, const char* key, bool default) const
        {
            return m_confReader.GetBoolean(sect, key, default);
        }

        inline std::string GetConfigValue(const char* sect, const char* key, const char* default) const
        {
            return m_confReader.Get(sect, key, default);
        }

        template <typename T>
        inline T GetConfigValue(const char* sect, const char* key, T default) const
        {
            return static_cast<T>(m_confReader.GetInteger(sect, key, static_cast<long>(default)));
        }

    private:
        static INIReader m_confReader;
    public:
        IConfig() = default;
        virtual ~IConfig() noexcept = default;
    };
}
