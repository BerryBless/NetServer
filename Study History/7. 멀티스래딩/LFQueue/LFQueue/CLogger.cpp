#include "CLogger.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <strsafe.h>
#include <locale.h>
#include <windows.h>
#include <direct.h>

#define dfLOG_BUF_SIZE 512 // 로그 버퍼 사이즈

int CLogger::_logLevel = dfLOG_LEVEL_DEBUG;
DWORD CLogger::_logCount = 0;
SRWLOCK CLogger::_lock;
WCHAR CLogger::_filePath[MAX_PATH];

void CLogger::_Log(int logLevel, const WCHAR *format, ...) {
	if (_logLevel > logLevel) return;
	DWORD lcnt = InterlockedIncrement(&_logCount);

	WCHAR log[dfLOG_BUF_SIZE] = {0};
	WCHAR *pLog = log;
	int lenRet = 0;		// 로그 나눠서 복사
	int lentotal = 0; // 로그길이

	bool bSpill = false;

	va_list ap; // 가변인자

	// timestemp
	tm t;
	time_t now;
	
	time(&now);
	localtime_s(&t,&now);

	// log level
	switch (logLevel) {
	case dfLOG_LEVEL_DEBUG:
		lenRet = swprintf_s(pLog, dfLOG_BUF_SIZE, L"[DEBUG] ");
		break;
	case dfLOG_LEVEL_ERROR:
		lenRet = swprintf_s(pLog, dfLOG_BUF_SIZE, L"[ERROR] ");
		break;
	case dfLOG_LEVEL_NOTICE:
		lenRet = swprintf_s(pLog, dfLOG_BUF_SIZE, L"[NOTICE] ");
		break;
	default:
		break;
	}
	pLog += lenRet;
	lentotal += lenRet;

	// log time
	lenRet = swprintf_s(pLog, dfLOG_BUF_SIZE - lentotal, L"[%d] [%02d/%02d/%02d %02d:%02d:%02d] ",
		lcnt, t.tm_mon + 1, t.tm_mday, (t.tm_year + 1900) % 100,
		t.tm_hour, t.tm_min, t.tm_sec);

	pLog += lenRet;
	lentotal += lenRet;

	// 가변인자 safe string
	va_start(ap, format);
	do{
		DWORD len;
		if (dfLOG_BUF_SIZE > lentotal)
			len = dfLOG_BUF_SIZE - lentotal;
		else {
			len = 0;
		}
		if (FAILED(StringCchVPrintf(pLog, len, format, ap))) {
			// 여기서 문자열 짤림처리
			bSpill = true;
		}
	} while (0);
	va_end(ap);




	// 콘솔창에 띄우기
	wprintf_s(L"%s\n",log);
	// 디버그는 파일로깅 안남김
	if (logLevel == dfLOG_LEVEL_DEBUG) return;

	// 파일 로깅
	WCHAR fileName[MAX_PATH] = {0};

	swprintf_s(fileName, _countof(fileName), L"%s/%04d%02d_Log.log",
		_filePath, t.tm_year + 1900, t.tm_mon + 1);

	FileLock();
	do{
		FILE *fp;

		do {
			_wfopen_s(&fp, fileName, L"a+");
		} while (fp == nullptr);

		if(!bSpill)
			fwprintf_s(fp, L"%s\n", log);
		else 
			fwprintf_s(fp, L"[TRUNCATED LOG]%s\n", log);

		fclose(fp);
	} while (0);
	FileUnlock();
}

void CLogger::Initialize() {
	ZeroMemory(_filePath, MAX_PATH);
	setlocale(LC_ALL, ""); // 로컬 문자 지원
	InitializeSRWLock(&_lock);
}

void CLogger::SetDirectory(const WCHAR *path) {
	ZeroMemory(_filePath, MAX_PATH);
	StringCchPrintf(_filePath, MAX_PATH, path); // safe string
	errno_t err;
	if (_wmkdir(_filePath) == -1) {
		// 실패
		_get_errno(&err);
		if(err != 17) // 이미 있어서 실패가 아님
			wprintf_s(L"/////////////////////FAIL MAKE DIRECTORY\n ERROR NO[%d] :: DIR [%s]\n", err,_filePath);
	}
}

void CLogger::FileLock() {
	AcquireSRWLockExclusive(&_lock);
}

void CLogger::FileUnlock() {
	ReleaseSRWLockExclusive(&_lock);
}
