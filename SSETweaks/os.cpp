#include "pch.h"

namespace SDT
{
    size_t GetAllocGranularity()
    {
        SYSTEM_INFO info;
        GetSystemInfo(&info);
        return static_cast<size_t>(info.dwAllocationGranularity);
    }
}