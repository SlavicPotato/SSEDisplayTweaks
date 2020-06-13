#include "pch.h"

namespace SDT {
    void safe_write(uintptr_t addr, const void* data, size_t len)
    {
        DWORD oldProtect;
        ASSERT(VirtualProtect(reinterpret_cast<void*>(addr), len, PAGE_EXECUTE_READWRITE, &oldProtect));
        memcpy(reinterpret_cast<void*>(addr), data, len);
        ASSERT(VirtualProtect(reinterpret_cast<void*>(addr), len, oldProtect, &oldProtect));
    }

    void safe_memset(uintptr_t addr, int val, size_t len)
    {
        DWORD oldProtect;
        ASSERT(VirtualProtect(reinterpret_cast<void*>(addr), len, PAGE_EXECUTE_READWRITE, &oldProtect));
        memset(reinterpret_cast<void*>(addr), val, len);
        ASSERT(VirtualProtect(reinterpret_cast<void*>(addr), len, oldProtect, &oldProtect));
    }

    bool validate_mem(uintptr_t addr, const void* data, size_t len)
    {
        return memcmp(reinterpret_cast<void*>(addr), data, len) == 0;
    }
}