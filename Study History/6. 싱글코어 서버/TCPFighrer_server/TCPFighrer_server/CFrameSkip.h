#pragma once
#include "CCalcFPSModule.h"

//	---
// 맴버변수
// _isSkip : 이전프레임이 Render를 스킵했으면 TRUE
// _oldTime : 이전시간 + 누적시간 (느린시간을 보관하되 시간으로 합산되게 가져가자.)
// _curTime : 현재시간 (timeGetTime)
// _deltatime : _curTime -  _oldTime
// _dwSkipTime : Render가 스킵되었다면 이전프레임의 _curTime
// 
// _gameFPS : 로직의 FPS
// _CRenderFPS : 랜더의 FPS
// ---
// 맴버함수
// FrameSkip : 랜더링을 스킵되어야 하는지 확인 (스킵이면 FALSE)
// GetLogicFPS : 로직 FPS
// GetRenderFPS : 렌더 FPS


class CFrameSkip {

private:
	BOOL _isSkip;	// 이전프레임이 스킵 되었느냐
	DWORD _oldTime;	// 이전시간 + 누적시간
	DWORD _curTime;	// 현재시간
	DWORD _deltatime;	// 델타타임

	CCalcFPSModule _loopFPS;
	CCalcFPSModule _gameFPS;

public:
	CFrameSkip();
	~CFrameSkip();

	BOOL FrameSkip();	// TRUE면 실행, FALSE면 스킵
	int FixedUpdateCount(); // FixedUpdate할 카운터! 
	int GetLogicFPS();
	int GetNetworkFPS();

	// 1프레임간의 간격 기준 dfDTDms
	DWORD Now(); // timegettime
};


