#pragma once

namespace SDT
{
    class IConfig
    {
    public:
        static int Load();

        SKMP_FORCEINLINE std::string GetConfigValue(const char* key, const char* default) const
        {
            return m_confReader.Get(ModuleName(), key, default);
        }

        SKMP_FORCEINLINE float GetConfigValue(const char* key, float default) const
        {
            return m_confReader.GetFloat(ModuleName(), key, default);
        }

        SKMP_FORCEINLINE double GetConfigValue(const char* key, double default) const
        {
            return m_confReader.GetReal(ModuleName(), key, default);
        }

        SKMP_FORCEINLINE bool GetConfigValue(const char* key, bool default) const
        {
            return m_confReader.GetBoolean(ModuleName(), key, default);
        }

        template <typename T, typename = std::enable_if_t<!std::is_same_v<T, bool> && (std::is_integral_v<T> || std::is_enum_v<T>) && std::is_convertible_v<T, long>>>
        T GetConfigValue(const char* key, T default) const
        {
            return static_cast<T>(m_confReader.GetInteger(ModuleName(), key, static_cast<long>(default)));
        }

        virtual const char* ModuleName() const noexcept = 0;

    private:
        static INIReader m_confReader;
    };

    class IConfigS : public IConfig
    {
    public:
        IConfigS() = delete;

        IConfigS(const char* a_name)
            : m_sectionName(a_name)
        {
        }

        virtual const char* ModuleName() const noexcept {
            return m_sectionName.c_str();
        }

    private:
        std::string m_sectionName;
    };
}
