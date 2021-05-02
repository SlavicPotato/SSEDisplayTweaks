#pragma once

#include "OS/SysCall.h"

namespace SDT
{
    class FramerateLimiter
    {
    public:
        FramerateLimiter() :
            m_lastTimePoint(IPerfCounter::Query()),
            m_hTimer(nullptr)
        {
            /*if (ISysCall::NtWaitForSingleObject != nullptr) {
                m_hTimer = ::CreateWaitableTimerA(nullptr, FALSE, nullptr);
            }*/
        }

        void Wait(long long a_limit)
        {
            auto now = IPerfCounter::Query();
            auto interval = IPerfCounter::delta_us(m_lastTimePoint, now);
            auto waitTime = a_limit - interval;

            if (waitTime > 0)
            {
                auto deadline = now + IPerfCounter::T(waitTime);

                /*if (m_hTimer != nullptr && waitTime > 2000LL)
                {
                    WaitTimer(waitTime - 2000LL, deadline - IPerfCounter::T(2000LL));
                }*/

                WaitBusy(deadline);
            }

            m_lastTimePoint = IPerfCounter::Query();
        }

    private: 

        SKMP_FORCEINLINE void WaitTimer(long long a_waitTime, long long a_deadline)
        {
            LARGE_INTEGER dueTime;
            dueTime.QuadPart = static_cast <LONGLONG>(
                static_cast<long double>(a_waitTime));

            if (dueTime.QuadPart <= 0)
                return;

            dueTime.QuadPart =
                -(dueTime.QuadPart * 10LL);

            if (::SetWaitableTimer(m_hTimer, &dueTime,
                0, nullptr, nullptr, TRUE) == FALSE)
            {
                return;
            }

            NTSTATUS status = STATUS_ALERTED;

            while (status != STATUS_SUCCESS)
            {
                _mm_pause();

                auto to_next = IPerfCounter::delta_us(IPerfCounter::Query(), a_deadline);

                if (to_next <= 0LL) {
                    return;
                }

                LARGE_INTEGER timeOut;
                timeOut.QuadPart = -(to_next * 10LL);

                status = ISysCall::NtWaitForSingleObject(m_hTimer, FALSE, &timeOut);
            }

        }

        SKMP_FORCEINLINE void WaitBusy(long long a_deadline)
        {
            while (IPerfCounter::Query() < a_deadline)
            {
                /*auto start = __rdtsc();

                do
                {
                    _mm_pause();

                    if (IPerfCounter::Query() >= a_deadline)
                        return;

                } while ((__rdtsc() - start) < 5000i64);*/

                Sleep(0);
            }
        }

        long long m_lastTimePoint;
        HANDLE m_hTimer;
    };
}