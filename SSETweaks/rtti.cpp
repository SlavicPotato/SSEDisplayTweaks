#include "pch.h"

namespace SDT
{
    void* IRTTI::addrs[] = {
        IAL::Addr<void*>(687041), // InputEvent
        IAL::Addr<void*>(687042)  // ButtonEvent
    };

    IRTTI IRTTI::m_Instance;
}