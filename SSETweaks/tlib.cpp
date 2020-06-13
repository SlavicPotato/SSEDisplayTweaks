#include "pch.h"

#pragma warning(disable: 4073)
#pragma init_seg(lib)

namespace SDT
{
    long long PerfCounter::perf_freq;
    float PerfCounter::perf_freqf;

    PerfCounter PerfCounter::m_Instance;

    PerfCounter::PerfCounter()
    {
        ::QueryPerformanceFrequency(
            reinterpret_cast<LARGE_INTEGER*>(&perf_freq));
        perf_freqf = static_cast<float>(perf_freq);
    }
}