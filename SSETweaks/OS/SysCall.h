#pragma once

#include <Unknwnbase.h>
#include <winternl.h>

#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)
#define STATUS_ALERTED ((NTSTATUS)0x00000101L)

class ISysCall
{
public:
	ISysCall();

	typedef NTSTATUS(WINAPI* NtWaitForSingleObject_t)(IN HANDLE ObjectHandle, IN BOOLEAN Alertable, IN PLARGE_INTEGER TimeOut OPTIONAL);

	static NtWaitForSingleObject_t NtWaitForSingleObject;

private:
	static ISysCall m_Instance;
};