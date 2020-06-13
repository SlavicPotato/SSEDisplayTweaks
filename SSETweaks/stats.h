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

        bool __forceinline update(long long m, long long& out)
        {
            auto e = PerfCounter::Query();
            auto delta = PerfCounter::delta_us(s, e);

            num++;

            if (delta < m) {
                return false;
            }

            out = delta / num;

            num = 0;
            s = e;

            return true;
        }

        void __forceinline accum(double val)
        {
            num++;
            fval += val;
        }

        bool __forceinline get(double& out)
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
        static __forceinline
            bool GetTime(uint8_t id, long long m, long long& out)
        {
            return data[id].update(m, out);
        }

        static __forceinline
            void Accum(uint8_t id, double val)
        {
            data[id].accum(val);
        }

        static __forceinline
            bool Addr(uint8_t id, double& out)
        {
            return data[id].get(out);
        }

        static __forceinline
            void Reset(uint8_t id)
        {
            return data[id].reset();
        }

    private:
        IStats() = default;

        static StatsCounter data[UINT8_MAX];
    };
}