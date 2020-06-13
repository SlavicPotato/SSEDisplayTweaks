#pragma once

namespace SDT 
{
    class IAL
    {
    public:
        static void Unload();
        static bool IsLoaded() {
            return m_Instance.isLoaded;
        }

        static float GetLoadTime() {
            return PerfCounter::delta<float>(
                m_Instance.tLoadStart, m_Instance.tLoadEnd);
        }

        static bool HasBadQuery() {
            return m_Instance.hasBadQuery;
        }

        template <typename T>
        static T Addr(unsigned long long id)
        {
            T r = reinterpret_cast<T>(m_Instance.db.FindAddressById(id));
            if (!r) {
                m_Instance.hasBadQuery = true;
            }
            return r;
        }

        static uintptr_t Addr(unsigned long long id, uintptr_t offset)
        {
            void* addr = m_Instance.db.FindAddressById(id);
            if (addr == NULL) {
                m_Instance.hasBadQuery = true;
                return uintptr_t(0);
            }
            return reinterpret_cast<uintptr_t>(addr) + offset;
        }

        static bool Offset(unsigned long long id, uintptr_t& result)
        {
            unsigned long long r;
            if (!m_Instance.db.FindOffsetById(id, r)) {
                m_Instance.hasBadQuery = true;
                return false;
            }
            result = static_cast<uintptr_t>(r);
            return true;
        }

        static uintptr_t Offset(unsigned long long id)
        {
            unsigned long long r;
            if (!m_Instance.db.FindOffsetById(id, r)) {
                m_Instance.hasBadQuery = true;
                return uintptr_t(0);
            }
            return static_cast<uintptr_t>(r);
        }

    private:
        IAL();

        bool isLoaded;
        bool hasBadQuery;
        long long tLoadStart, tLoadEnd;

        VersionDb db;

        static IAL m_Instance;
    };
}