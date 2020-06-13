#pragma once

#define RTTIInputEvent      0
#define RTTIButtonEvent     1

namespace SDT
{
    class IRTTI :
        ILog
    {
        typedef void* (*RDCImpl_T)(void*, uint32_t, const void*, const void*, uint32_t);
    public:
        template <typename T>
        static __forceinline T* Cast(void* obj, size_t fromIndex, size_t toIndex) {
            return reinterpret_cast<T*>(RDCImpl(obj, 0, addrs[fromIndex], addrs[toIndex], 0));
        }
    private:
        IRTTI() = default;

        inline static auto RDCImpl = IAL::Addr<RDCImpl_T>(AID::RDCImpl);

        static void* addrs[];

        static IRTTI m_Instance;
    };

}