#pragma once
#include <direct.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <Dbghelp.h>
#include <psapi.h>
#include <crtdbg.h>

#pragma comment (lib, "Dbghelp.lib")

class CCrashDump {
public:

	CCrashDump() {
		_dumpCount = 0;

		_invalid_parameter_handler oldHandler, newHandler;
		newHandler = DumpInvalidParameterHander;

		oldHandler = _set_invalid_parameter_handler(newHandler);   // crt함수에 null 포인터 등을 넣었을 때
		_CrtSetReportMode(_CRT_WARN, 0);                     // CRT 오류 메시지 표시 중단. 바로 덤프로 남도록.
		_CrtSetReportMode(_CRT_ASSERT, 0);                     // CRT 오류 메시지 표시 중단. 바로 덤프로 남도록.
		_CrtSetReportMode(_CRT_ERROR, 0);                     // CRT 오류 메시지 표시 중단. 바로 덤프로 남도록.

		_CrtSetReportHook(_custom_Report_hook);

		//-----------------------------------------------------------------------
		// pure virtual function called 에러 핸들러를 사용자 정의 함수로 우회시킨다.
		//-----------------------------------------------------------------------
		_set_purecall_handler(DumpPurecallHander);

		SetHandlerDump();
	}

	static void Crash() {
		int *p = nullptr;
		*p = 10;
	}

	static LONG WINAPI DumpExceptionFilter(__in PEXCEPTION_POINTERS pExceptionPointer) {
		int iResult = _mkdir("../Dump");

		int workingMemory = 0;
		SYSTEMTIME nowTime;

		long dumpCount = InterlockedIncrement(&_dumpCount);


		//----------------------------------------------------------
		// 현재 프로세스의 메모리 사용량을 얻어온다.
		//----------------------------------------------------------
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


		//----------------------------------------------------------
		// 현재 날짜와 시간을 알아온다.
		//----------------------------------------------------------
		WCHAR filename[_MAX_PATH];

		GetLocalTime(&nowTime);
		wsprintf(filename, L"Dump_%d%02d%02d_%02d.%02d.%02d_%d_%dMB.dmp",
			nowTime.wYear, nowTime.wMonth, nowTime.wDay, nowTime.wHour, nowTime.wMinute, nowTime.wSecond, dumpCount, workingMemory);

		wprintf(L"\n\n\n////////////////// CRASH ERROR ///////////////////////\n\t %d.%d.%d/%d:%d:%d\n",
			nowTime.wYear, nowTime.wMonth, nowTime.wDay, nowTime.wHour, nowTime.wMinute, nowTime.wSecond);

		HANDLE hDumpFile = ::CreateFile(
			filename,
			GENERIC_WRITE,            // 프로세스 내에서 파일을 읽을 일이 없다.
			FILE_SHARE_WRITE,         // 다른 프로세스에서 이 파일에 동시 쓰기 접근 가능
			NULL,
			CREATE_ALWAYS,            // 항상 새로운 파일 생성
			FILE_ATTRIBUTE_NORMAL,      // 보통 파일로 지정
			NULL);

		if (hDumpFile != INVALID_HANDLE_VALUE) {
			_MINIDUMP_EXCEPTION_INFORMATION MinidumpExceptionInformation;

			MinidumpExceptionInformation.ThreadId = ::GetCurrentThreadId();
			MinidumpExceptionInformation.ExceptionPointers = pExceptionPointer;
			MinidumpExceptionInformation.ClientPointers = TRUE;

			MiniDumpWriteDump(
				GetCurrentProcess(), 
				GetCurrentProcessId(), 
				hDumpFile, 
				MiniDumpWithFullMemory, 
				&MinidumpExceptionInformation, 
				NULL, NULL);
			CloseHandle(hDumpFile);

			wprintf_s(L"CRASH DUMP SAVE FINISH!");
		}
		return EXCEPTION_EXECUTE_HANDLER;
	}

	static void SetHandlerDump() {
		SetUnhandledExceptionFilter(DumpExceptionFilter);
	}

	static void DumpInvalidParameterHander(const wchar_t *expression, const wchar_t *function, const wchar_t *file, unsigned int line, uintptr_t   pReserved) {
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

