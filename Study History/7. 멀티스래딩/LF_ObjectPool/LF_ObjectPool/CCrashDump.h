#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <crtdbg.h>  // For _CrtSetReportMode
#include <Windows.h>
#include <psapi.h>
#include <minidumpapiset.h>
#pragma comment(lib, "Dbghelp.lib")


class CCrashDump {
public:

	CCrashDump() {
		_dumpCount = 0;

		_invalid_parameter_handler oldHandler, newHandler;
		newHandler = DumpInvalidParameterHander;

		oldHandler = _set_invalid_parameter_handler(newHandler);
		_CrtSetReportMode(_CRT_WARN, 0);
		_CrtSetReportMode(_CRT_ASSERT, 0);
		_CrtSetReportMode(_CRT_ERROR, 0);

		_CrtSetReportHook(_custom_Report_hook);

		_set_purecall_handler(DumpPurecallHander);

		SetHandlerDump();

	}

	static void Crash() {
		int *p = nullptr;
		*p = 10;
	}

	static LONG WINAPI DumpExceptionFilter(__in PEXCEPTION_POINTERS pExceptionPointer) {
		int workingMemory = 0;
		SYSTEMTIME nowTime;

		long dumpCount = InterlockedIncrement(&_dumpCount);

		HANDLE hProcess = 0;
		PROCESS_MEMORY_COUNTERS pmc;

		hProcess = GetCurrentProcess();

		if (hProcess == NULL) {
			return 0;
		}

		if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
			workingMemory = (int) (pmc.WorkingSetSize / 1024/1024);
		}
		CloseHandle(hProcess);


		// 현재 날자와 시간
		WCHAR filename[_MAX_PATH];

		GetLocalTime(&nowTime);
		wsprintf(filename, L"Dump_%d%02d%02d_%02d.%02d.%02d_%d_%dMB.dmp",
			nowTime.wYear, nowTime.wMonth, nowTime.wDay, nowTime.wHour, nowTime.wMinute, nowTime.wSecond, dumpCount, workingMemory);

		wprintf(L"\n\n\n////////////////// CRASH ERROR ///////////////////////\n\t %d.%d.%d/%d:%d:%d\n",
			nowTime.wYear, nowTime.wMonth, nowTime.wDay, nowTime.wHour, nowTime.wMinute, nowTime.wSecond);

		HANDLE hDumpFile = ::CreateFile(filename,
			GENERIC_WRITE,
			FILE_SHARE_WRITE,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (hDumpFile != INVALID_HANDLE_VALUE) {
			_MINIDUMP_EXCEPTION_INFORMATION minidumpExceptioInfromation;

			minidumpExceptioInfromation.ThreadId = ::GetCurrentThreadId();
			minidumpExceptioInfromation.ExceptionPointers = pExceptionPointer;
			minidumpExceptioInfromation.ClientPointers = TRUE;

			MiniDumpWriteDump(GetCurrentProcess(),
				GetCurrentProcessId(),
				hDumpFile,
				MiniDumpWithFullMemory,
				&minidumpExceptioInfromation,
				NULL, NULL);

			CloseHandle(hDumpFile);

			wprintf_s(L"CRASH DUMP SAVE FINISH!");
		}
		return EXCEPTION_EXECUTE_HANDLER;
	}

	static void SetHandlerDump() {
		SetUnhandledExceptionFilter(DumpExceptionFilter);
	}

	static void DumpInvalidParameterHander(const wchar_t *expression, const wchar_t  *function, const wchar_t *file, unsigned int line, uintptr_t pReserved) {
		Crash();
	}

	static int _custom_Report_hook(int ireposttype, char *message, int *returnvalue) {
		Crash();
		return 0;
	}

	static void DumpPurecallHander() {
		Crash();
	}

	static long _dumpCount;
};

