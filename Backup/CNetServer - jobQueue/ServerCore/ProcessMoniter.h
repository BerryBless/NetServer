#pragma once

#pragma comment(lib,"Pdh.lib")
#include <Pdh.h>

class ProcessMoniter {
public:
	ProcessMoniter(HANDLE process = INVALID_HANDLE_VALUE);

	void UpdateProcessTime();

	inline double ProcessTotal() { return _ProcessTotal; }
	inline double ProcessUser() { return _ProcessUser; }
	inline double ProcessKernel() { return _ProcessKernel; }

	inline unsigned long long PrivateMemoryBytes() { return _PrivateMemoryBytes; }
	inline unsigned long long PrivateMemoryKBytes() { return _PrivateMemoryBytes >> 10; }
	inline unsigned long long PrivateMemoryMBytes() { return _PrivateMemoryBytes >> 20; }
private:

	HANDLE _Process;
	WCHAR _ProcessName[MAX_PATH];
	int _NumOfProcessor;

	double _ProcessTotal;
	double _ProcessUser;
	double _ProcessKernel;

	ULARGE_INTEGER _ProcessLastKernel;
	ULARGE_INTEGER _ProcessLastUser;
	ULARGE_INTEGER _ProcessLastTime;

	//PDH_HQUERY _PrivateMemoryQuery;
	//PDH_HCOUNTER _PrivateMemoryCounter;
	unsigned long long _PrivateMemoryBytes;
};

