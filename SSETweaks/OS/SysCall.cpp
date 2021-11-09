#include "pch.h"

#include "SysCall.h"

#pragma warning(disable: 4073)
#pragma init_seg(lib)

ISysCall::NtWaitForSingleObject_t ISysCall::NtWaitForSingleObject(nullptr);

ISysCall ISysCall::m_Instance;

ISysCall::ISysCall()
{
	/*HMODULE hModule = GetModuleHandleA("ntdll.dll");

    if (hModule != nullptr) {
        NtWaitForSingleObject = reinterpret_cast<NtWaitForSingleObject_t>(GetProcAddress(hModule, "NtWaitForSingleObject"));
    }*/
}