#pragma once

namespace SDT
{
    namespace RTTI
    {
        constexpr uint32_t InputEvent = 0;
        constexpr uint32_t ButtonEvent = 1;
    }

    class IRTTI :
        ILog
    {
        typedef void* (*RDCImpl_T)(void*, uint32_t, const void*, const void*, uint32_t);
    public:
        template <typename T>
        static __forceinline T* Cast(void* obj, uint32_t fromIndex, uint32_t toIndex) {
            return reinterpret_cast<T*>(RDCImpl(obj, 0, addrs[fromIndex], addrs[toIndex], 0));
        }
    private:
        IRTTI() = default;

        inline static auto RDCImpl = IAL::Addr<RDCImpl_T>(AID::RDCImpl);

        static void* addrs[];

        static IRTTI m_Instance;
    };

}