#pragma once

namespace SDT 
{
    void safe_write(uintptr_t addr, const void* data, size_t len);
    void safe_memset(uintptr_t addr, int val, size_t len);
    bool validate_mem(uintptr_t addr, const void* data, size_t len);

    template <typename T>
    __inline void safe_write(uintptr_t addr, T val)
    {
        safe_write(addr, reinterpret_cast<const void*>(&val), sizeof(T));
    }
}