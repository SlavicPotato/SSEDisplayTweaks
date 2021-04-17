#pragma once

namespace SDT
{
    class IConfig
    {
    public:
        static int Load();
        static void Clear();

        SKMP_FORCEINLINE const char* GetConfigValue(const std::string& key, const char* default) const
        {
            return GetValue(key, default);
        }

        SKMP_FORCEINLINE float GetConfigValue(const std::string& key, float default) const
        {
            return GetValue(key, default);
        }

        SKMP_FORCEINLINE double GetConfigValue(const std::string& key, double default) const
        {
            return GetValue(key, default);
        }

        SKMP_FORCEINLINE bool GetConfigValue(const std::string& key, bool default) const
        {
            return GetValue(key, default);
        }

        template <typename T, typename = std::enable_if_t<!std::is_same_v<T, bool> && (std::is_integral_v<T> || std::is_enum_v<T>) && std::is_convertible_v<T, long>>>
        T GetConfigValue(const std::string& key, T default) const
        {
            return static_cast<T>(GetValue(key, static_cast<long>(default)));
        }

        SKMP_FORCEINLINE bool HasConfigValue(const std::string& key) const
        {
            return Exists(key);
        }

        virtual const char* ModuleName() const noexcept = 0;

    private:

        template <typename T>
        T GetValue(const std::string& a_key, T a_default) const
        {
            std::string mod(ModuleName());

            if (m_confReaderCustom.ParseError() == 0)
            {
                auto valstr = m_confReaderCustom.Get(mod, a_key);
                if (valstr) {
                    return m_confReaderCustom.ParseValue(valstr, a_default);
                }
            }

            return m_confReader.Get(mod, a_key, a_default);
        }

        bool Exists(const std::string& a_key) const
        {
            std::string mod(ModuleName());

            if (m_confReaderCustom.ParseError() == 0)
            {
                if (m_confReaderCustom.Exists(mod, a_key))
                    return true;
            }

            return m_confReader.Exists(mod, a_key);
        }

        static INIReader m_confReader;
        static INIReader m_confReaderCustom;
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

    class IConfigGame
    {
    public:
        IConfigGame(const char* a_path)
        {
            auto base = std::make_unique_for_overwrite<char[]>(MAX_PATH);

            HRESULT hr = ::SHGetFolderPathA(
                nullptr,
                CSIDL_MYDOCUMENTS | CSIDL_FLAG_CREATE,
                nullptr,
                SHGFP_TYPE_CURRENT,
                base.get());

            if (SUCCEEDED(hr))
            {
                std::filesystem::path file;

                file = base.get();
                file /= a_path;

                m_reader.Load(file.string());
            }

        }

        template <class T>
        bool Get(const std::string& a_section, const std::string& a_key, T a_default, T& a_out) const
        {
            if (m_reader.ParseError() != 0) {
                return false;
            }

            auto valstr = m_reader.Get(a_section, a_key);
            if (valstr)
            {
                a_out = m_reader.ParseValue(valstr, a_default);
                return true;
            }

            return false;
        }


    private:

        INIReader m_reader;
    };


}