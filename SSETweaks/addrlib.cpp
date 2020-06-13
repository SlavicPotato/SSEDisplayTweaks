#include "pch.h"

#pragma warning(disable: 4073)
#pragma init_seg(lib)

namespace SDT
{
    IAL IAL::m_Instance;

    IAL::IAL() :
        hasBadQuery(false)
    {
        tLoadStart = SDT::PerfCounter::Query();
        isLoaded = db.Load();
        tLoadEnd = SDT::PerfCounter::Query();
    }

    void IAL::Unload()
    {
        m_Instance.db.Clear();
    }
}