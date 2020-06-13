#include "pch.h"

namespace SDT
{
    JITASM::JITASM(size_t maxSize) :
        _endedAlloc(false),
        Xbyak::CodeGenerator(maxSize, g_localTrampoline.StartAlloc())
    {
    }

    JITASM::~JITASM() noexcept
    {
        done();
    }

    void JITASM::done()
    {
        if (!_endedAlloc) {
            _endedAlloc = true;
            g_localTrampoline.EndAlloc(getCurr());
        }
    }

    uintptr_t JITASM::get()
    {
        done();
        return reinterpret_cast<uintptr_t>(getCode());
    }
}