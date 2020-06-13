#pragma once
namespace SDT
{
    class ILog
    {
    public:
        template<typename... Args>
        void Debug(const char* fmt, Args... args)
        {
            LogMessage(_DMESSAGE, nullptr, fmt, args...);
        }

        template<typename... Args>
        void Message(const char* fmt, Args... args)
        {
            LogMessage(_MESSAGE, nullptr, fmt, args...);
        }

        template<typename... Args>
        void Warning(const char* fmt, Args... args)
        {
            LogMessage(_WARNING, "WARNING", fmt, args...);
        }

        template<typename... Args>
        void Error(const char* fmt, Args... args)
        {
            LogMessage(_ERROR, "ERROR", fmt, args...);
        }

        template<typename... Args>
        void FatalError(const char* fmt, Args... args)
        {
            LogMessage(_FATALERROR, "FATAL", fmt, args...);
        }

        void LogPatchBegin(const char* id)
        {
            LogMessage(_DMESSAGE, nullptr, "[Patch] [%s] Writing..", id);
        }

        void LogPatchEnd(const char* id)
        {
            LogMessage(_DMESSAGE, nullptr, "[Patch] [%s] OK", id);
        }

        static IDebugLog::LogLevel TranslateLogLevel(const std::string& level);

        FN_NAMEPROC("ILog")
    private:

        typedef std::unordered_map<std::string, IDebugLog::LogLevel> LLMap;
        static LLMap LogLevelMap;

        typedef void (*SKSE_MsgProc)(const char* fmt, ...);

        template<typename... Args>
        void LogMessage(SKSE_MsgProc proc, const char* pfix, const char* fmt, Args... args)
        {
            m_logMutex.Enter();
            std::ostringstream _fmt;
            if (pfix != nullptr) {
                _fmt << "<" << pfix << "> ";
            }
            _fmt << "[" << ModuleName() << "] " << fmt;
            proc(_fmt.str().c_str(), args...);
            m_logMutex.Leave();
        }

        static ICriticalSection m_logMutex;
    protected:
        ILog() = default;
        virtual ~ILog() = default;
    };
}