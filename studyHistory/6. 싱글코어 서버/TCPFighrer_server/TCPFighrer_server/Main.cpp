#include "pch.h"
#include "NetworkCore.h"
#include "ContentCore.h"
#include "CFrameSkip.h"
bool g_bShutdown = false;
CFrameSkip g_Timer;
extern int g_LogLevel;

void ServerControl() {
	static bool ControlMode = false;

	if (_kbhit()) {
		WCHAR wcKey = _getwch();

		// UNLOCK
		if (L'u' == wcKey || L'U' == wcKey) {
			ControlMode = true;
			wprintf_s(L"######## Server Control Unlocked\n");
			wprintf_s(L"######## \t L - Server Control Lock \n");
			wprintf_s(L"######## \t U - Server Control Unlock \n");
			wprintf_s(L"######## \t Q - Quit \n");
			wprintf_s(L"######## \t 0 - LOG_LEVEL_DEBUG \n");
			wprintf_s(L"######## \t 1 - LOG_LEVEL_WARNING \n");
			wprintf_s(L"######## \t 2 - LOG_LEVEL_ERROR \n");

		}

		// LOCK
		if (L'l' == wcKey || L'L' == wcKey) {
			ControlMode = false;
			wprintf_s(L"######## Server Control Locked\n");
		}

		// CONTROL
		// 종료
		if ((L'q' == wcKey || L'Q' == wcKey) && ControlMode) {
			g_bShutdown = true;
			wprintf_s(L"######## Server Closing...\n");
		}

		// 로그레벨
		if ((L'1' == wcKey) && ControlMode) {
			g_LogLevel = dfLOG_LEVEL_DEBUG;
			_LOG(dfLOG_LEVEL_ERROR, L"######## LOG_LEVEL_DEBUG");
		}
		if ((L'2' == wcKey) && ControlMode) {
			g_LogLevel = dfLOG_LEVEL_WARNING;
			_LOG(dfLOG_LEVEL_ERROR, L"######## LOG_LEVEL_WARNING");
		}
		if ((L'3' == wcKey) && ControlMode) {
			g_LogLevel = dfLOG_LEVEL_BUG;
			_LOG(dfLOG_LEVEL_ERROR, L"######## dfLOG_LEVEL_BUG");
		}
		if ((L'4' == wcKey) && ControlMode) {
			g_LogLevel = dfLOG_LEVEL_ERROR;
			_LOG(dfLOG_LEVEL_ERROR, L"######## LOG_LEVEL_ERROR");
		}

		// 더미테스트 켜기 / 끄기
		if ((L'd' == wcKey || L'D' == wcKey) && ControlMode) {
			g_dummytest = !g_dummytest;
			if (g_dummytest)
				wprintf_s(L"######## Dummy on!\n");
			else
				wprintf_s(L"######## Dummy off...\n");
		}

	}

}
int main() {
	timeBeginPeriod(1);// timegettime 1초단위로 증가하게.
	_wsetlocale(LC_ALL, L"korean");// 한국어 설정

	wprintf_s(L"######## \t L - Server Control Lock \n");
	wprintf_s(L"######## \t U - Server Control Unlock \n");
	wprintf_s(L"######## \t D - DUMMY TEST TOGGLE \n");
	wprintf_s(L"######## \t Q - Quit \n");
	wprintf_s(L"######## \t 0 - LOG_LEVEL_DEBUG \n");
	wprintf_s(L"######## \t 1 - LOG_LEVEL_WARNING \n");
	wprintf_s(L"######## \t 2 - LOG_LEVEL_ERROR \n");

	// 네트워크 초기화
	NetworkInitServer();
	while (!g_bShutdown) {
		_LOG(dfLOG_LEVEL_DEBUG, L"=================================================\nMAIN LOOP");
		// 서버 컨트롤
		ServerControl();

		// 네트워크부
		NetworkPorc();

		// 컨탠츠부 
		if (g_Timer.FrameSkip()) {
			ContentUpdate();
		}
		_LOG(dfLOG_LEVEL_DEBUG, L"\nMAIN LOOP END\n=================================================");
	}
	// 네트워크 정리
	NetworkCloseServer();
	return 0;
}