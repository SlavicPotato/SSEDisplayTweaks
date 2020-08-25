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

        void reset()
        {
            num = 0;
            fval = 0.0;
            s = PerfCounter::Query();
        }

        __forceinline bool update(long long m, long long& out)
        {
            auto e = PerfCounter::Query();
            auto delta = PerfCounter::delta_us(s, e);

            num++;

            if (delta < m || num == 0) {
                return false;
            }

            out = delta / num;

            num = 0;
            s = e;

            return true;
        }

        __forceinline void accum(double val)
        {
            num++;
            fval += val;
        }

        __forceinline bool get(double& out)
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

    class IStats
    {
    public:
        __forceinline static
            bool GetTime(uint8_t id, long long m, long long& out)
        {
            return data[id].update(m, out);
        }

        __forceinline static
            void Accum(uint8_t id, double val)
        {
            data[id].accum(val);
        }

        __forceinline static
            bool Addr(uint8_t id, double& out)
        {
            return data[id].get(out);
        }

        __forceinline static
            void Reset(uint8_t id)
        {
            return data[id].reset();
        }

    private:
        IStats() = default;

        static StatsCounter data[UINT8_MAX];
    };
}