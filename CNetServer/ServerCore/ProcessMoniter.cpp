#include "pch.h"
#include "ProcessMoniter.h"
#include <strsafe.h>
#include <Psapi.h>
ProcessMoniter::ProcessMoniter(HANDLE process)
{
	if (_Process == INVALID_HANDLE_VALUE)
		_Process = GetCurrentProcess();
	//if (_PrivateMemoryQuery == nullptr)
	//	PdhOpenQuery(NULL, NULL, &_PrivateMemoryQuery);

	GetModuleBaseName(_Process, nullptr, _ProcessName, MAX_PATH - 1);

	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);
	_NumOfProcessor = SystemInfo.dwNumberOfProcessors;

	UpdateProcessTime();
}

void ProcessMoniter::UpdateProcessTime()
{
	ULARGE_INTEGER dummy;
	ULARGE_INTEGER nowTime;

	ULARGE_INTEGER kernel;
	ULARGE_INTEGER user;
	GetSystemTimeAsFileTime((LPFILETIME) &nowTime);
	GetProcessTimes(_Process, (LPFILETIME) &dummy, (LPFILETIME) &dummy, (LPFILETIME) &kernel, (LPFILETIME) &user);

	ULONGLONG timeDiff;
	ULONGLONG userDiff;
	ULONGLONG kernelDiff;
	ULONGLONG total;
	timeDiff = nowTime.QuadPart - _ProcessLastTime.QuadPart;
	userDiff = user.QuadPart - _ProcessLastUser.QuadPart;
	kernelDiff = kernel.QuadPart - _ProcessLastKernel.QuadPart;
	total = kernelDiff + userDiff;
	_ProcessTotal = (float) (total / (double) _NumOfProcessor / (double) timeDiff * 100.0f);
	_ProcessKernel = (float) (kernelDiff / (double) _NumOfProcessor / (double) timeDiff * 100.0f);
	_ProcessUser = (float) (userDiff / (double) _NumOfProcessor / (double) timeDiff * 100.0f);
	_ProcessLastTime = nowTime;
	_ProcessLastKernel = kernel;
	_ProcessLastUser = user;


	PROCESS_MEMORY_COUNTERS_EX  counter;
	if (GetProcessMemoryInfo(_Process, (PROCESS_MEMORY_COUNTERS *) &counter, sizeof(counter)))
		_PrivateMemoryBytes = counter.PrivateUsage;
}
