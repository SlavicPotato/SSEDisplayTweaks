#pragma once

namespace SDT
{
    struct StatsCounter
    {
    public:
        StatsCounter()
        {
            reset();
        }

        SKMP_FORCEINLINE void reset()
        {
            num = 0;
            fval = 0.0;
            s = IPerfCounter::Query();
        }

        SKMP_FORCEINLINE bool update(long long m, long long& out)
        {
            auto e = IPerfCounter::Query();
            auto delta = IPerfCounter::delta_us(s, e);

            num++;

            if (delta < m || num == 0) {
                return false;
            }

            out = delta / num;

            num = 0;
            s = e;

            return true;
        }

        SKMP_FORCEINLINE void accum(double val)
        {
            num++;
            fval += val;
        }

        SKMP_FORCEINLINE bool get(double& out)
        {
            if (!num) {
                return false;
            }

            out = fval / static_cast<double>(num);

            num = 0;
            fval = 0.0;

            return true;
        }

    private:
        long long s;

        double fval;
        uint64_t num;
    };

}