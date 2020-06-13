#pragma once

#include <chrono>

namespace SDT
{
    class PerfCounter
    {
    public:
        template <typename T>
        __forceinline static T delta(long long tp1, long long tp2) {
            return static_cast<T>(tp2 - tp1) / perf_freqf;
        }

        __forceinline static long long delta_us(long long tp1, long long tp2) {
            return ((tp2 - tp1) * 1000000LL) / perf_freq;
        }

        __forceinline static long long Query() {
            long long t;
            ::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&t));
            return t;
        }

        __forceinline static long long T(long long tp)
        {
            return (perf_freq / 1000000LL) * tp;
        }

    private:
        PerfCounter();

        static long long perf_freq;
        static float perf_freqf;
        static PerfCounter m_Instance;
    };

}