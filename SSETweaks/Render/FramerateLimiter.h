#pragma once

//#include "OS/SysCall.h"

namespace SDT
{
    class FramerateLimiter
    {
    public:
        FramerateLimiter() :
            m_lastTimePoint(IPerfCounter::Query())
            //m_hTimer(nullptr)
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

                /*if (m_hTimer != nullptr)
                {
                    WaitTimer(waitTime, deadline);
                }
                else
                {
                    if (waitTime >= 2000LL)
                    {
                        Sleep(static_cast<DWORD>((waitTime - 1000LL) / 1000LL));
                    }
                }*/

                if (waitTime >= 2000LL)
                {
                    Sleep(static_cast<DWORD>((waitTime - 1000LL) / 1000LL));
                }

                WaitBusy(deadline);

                //_DMESSAGE("%d", c);

            }

            m_lastTimePoint = IPerfCounter::Query();
        }

    private:

        /*SKMP_FORCEINLINE void WaitTimer(long long a_waitTime, long long a_deadline)
        {
            LARGE_INTEGER dueTime;
            dueTime.QuadPart = static_cast <LONGLONG>(
                static_cast<long double>(a_waitTime) * 0.7L);

            if (dueTime.QuadPart <= 0)
                return;

            dueTime.QuadPart =
                -(dueTime.QuadPart * 10LL);

            if (SetWaitableTimer(m_hTimer, &dueTime,
                0, nullptr, nullptr, TRUE) == FALSE)
            {
                return;
            }

            NTSTATUS status = STATUS_ALERTED;

            while (status != STATUS_SUCCESS)
            {
                auto to_next = IPerfCounter::delta_us(IPerfCounter::Query(), a_deadline);

                if (to_next <= 0LL) {
                    return;
                }

                LARGE_INTEGER timeout;
                timeout.QuadPart = -(to_next * 10LL);

                status = ISysCall::NtWaitForSingleObject(m_hTimer, FALSE, &timeout);

                _mm_pause();
            }

        }*/

        SKMP_FORCEINLINE void WaitBusy(long long a_deadline)
        {
            long long now;

            while ((now = IPerfCounter::Query()) < a_deadline)
            {
                if (IPerfCounter::delta_us(now, a_deadline) > 1000L)
                {
                    auto start = __rdtsc();

                    do
                    {
                        _mm_pause();

                        auto d = IPerfCounter::delta_us(IPerfCounter::Query(), a_deadline);

                        if (d <= 0LL)
                            return;

                        if (d <= 1000LL)
                            break;

                    } while ((__rdtsc() - start) < 1000i64);

                    SwitchToThread();
                }
            }
        }

        long long m_lastTimePoint;
        //HANDLE m_hTimer;
    };
}