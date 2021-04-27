#pragma once

#include "plugin.h"

namespace WinApi
{
    __forceinline void MessageBoxError(const char* a_message)
    {
        MessageBoxA(NULL, a_message, PLUGIN_NAME, MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
    }
}