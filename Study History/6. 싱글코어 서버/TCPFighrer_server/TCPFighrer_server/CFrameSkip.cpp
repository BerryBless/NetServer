#include "pch.h"
#include "CFrameSkip.h"
#include "NetworkCore.h"
#include "PacketProcess.h"//g_syncCnt
#include "ContentCore.h" // g_FrameUpdate

extern int g_syncCnt ;
extern CCalcFPSModule g_sendTPS;
extern CCalcFPSModule g_recvTPS;

CFrameSkip::CFrameSkip() {
	// 초기화
	_isSkip = FALSE;
	_curTime = Now();
	_oldTime = Now();
	_deltatime = _curTime - _oldTime;
}

CFrameSkip::~CFrameSkip() {
}

BOOL CFrameSkip::FrameSkip() {
	// 현재시간
	_curTime = Now();

	// 한프레임 시간 구하기
	_deltatime += _curTime - _oldTime;
	_oldTime = _curTime ;

	// 네트워크 FPS 구하기
	_loopFPS.Check();

	if (_deltatime < dfDTD) {
		return FALSE;
	}

	// 업데이트 FPS계산
	if (_gameFPS.Check()) {
		//---------------------------
		// 모니터링 처리
		//---------------------------
		_LOG(dfLOG_LEVEL_ERROR, L"LoopFPS[%d] | LogicFPS [%d] | FixedUpdate [%d] | SYNC COUNT[%d]"
			, _loopFPS.GetTPS(), _gameFPS.GetTPS(), g_FixedUpdate, g_syncCnt);
		_gameFPS.LogInfo();
		_LOG(dfLOG_LEVEL_ERROR, L"connect session [%d]", g_sessionMap.size() );
		_LOG(dfLOG_LEVEL_ERROR, L"send TPS [%d] , recv TPS [%d]\n-----------------------------------------------", g_sendTPS.GetTPS(), g_recvTPS.GetTPS());

		g_FixedUpdate = 0;
	}

	return TRUE;
}

int CFrameSkip::FixedUpdateCount() {
	int cnt = _deltatime / dfDTD;
	_deltatime = _deltatime % dfDTD;
	return cnt;
}

int CFrameSkip::GetLogicFPS() {
	return _gameFPS.GetTPS();
}

int CFrameSkip::GetNetworkFPS() {
	return _loopFPS.GetTPS();
}


DWORD CFrameSkip::Now() {
	return timeGetTime();
}
