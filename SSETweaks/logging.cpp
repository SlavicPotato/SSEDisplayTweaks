#include "pch.h"

namespace SDT
{
    ICriticalSection ILog::m_logMutex;

    ILog::LLMap ILog::LogLevelMap = {
        {"debug", IDebugLog::kLevel_DebugMessage},
        {"verbose", IDebugLog::kLevel_VerboseMessage},
        {"message", IDebugLog::kLevel_Message},
        {"warning", IDebugLog::kLevel_Warning},
        {"error", IDebugLog::kLevel_Error},
        {"fatal", IDebugLog::kLevel_FatalError}
    };

    IDebugLog::LogLevel ILog::TranslateLogLevel(const std::string& level)
    {
        auto it = LogLevelMap.find(level);
        if (it != LogLevelMap.end()) {
            return it->second;
        }
        else {
            return IDebugLog::kLevel_Warning;
        }
    }
}