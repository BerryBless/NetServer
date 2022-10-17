#include "pch.h"
#include "Logger.h"

int g_LogLevel = dfLOG_LEVEL_BUG;
wchar_t g_wsLogBuf[256] = {0};
char g_sFilename[256] = dfLOGFILENAME;
void Log(wchar_t *wsString, int iLogLevel) {

	if (g_LogLevel > iLogLevel) return;
	FILE *fp;

	// 뒤에 추가
	fopen_s(&fp, g_sFilename, "a");
	if (fp == NULL) {
		// 없음 생성
		fopen_s(&fp, g_sFilename, "w");
	if (fp == NULL) {
		// crash
		int *p = nullptr; *p = 10;
		return;
	}
	} else {
		fseek(fp, 0, SEEK_END);
	}
	// timestemp
	time_t now = time(0);
	tm t;
	localtime_s(&t,&now);


	fwprintf_s(fp, L"[%02d:%02d:%02d] %s\n", t.tm_hour, t.tm_min, t.tm_sec,wsString);
	fwprintf_s(stdout, L"[%02d:%02d:%02d] %s\n", t.tm_hour, t.tm_min, t.tm_sec,wsString);

	fclose(fp);
}

void Log_nontimestamp(wchar_t *wsString, int iLogLevel) {
	if (g_LogLevel > iLogLevel) return;
	FILE *fp;

	// 뒤에 추가
	fopen_s(&fp, g_sFilename, "a");
	if (fp == NULL) {
		// 없음 생성
		fopen_s(&fp, g_sFilename, "w");
		if (fp == NULL) {
			// crash
			int *p = nullptr; *p = 10;
			return;
		}
	} else {
		fseek(fp, 0, SEEK_END);
	}

	fwprintf_s(fp, L"%s",  wsString);
	fwprintf_s(stdout, L"%s",  wsString);

	fclose(fp);
}
