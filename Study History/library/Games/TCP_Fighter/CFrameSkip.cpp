#include "CFrameSkip.h"
#include <timeapi.h>
//#define TEMP
CFrameSkip::CFrameSkip() {
	// 초기화
	_isSkip = FALSE;
	_dwCur = timeGetTime();
	_dwOld = timeGetTime();
	_dwDelta = _dwCur - _dwOld;
	_dwSkipTime = 0;
}

CFrameSkip::~CFrameSkip() {
}

BOOL CFrameSkip::FrameSkip() {
#ifdef TEMP
	// 현재시간
	Sleep(_iDelayTime );
	return TRUE;

#endif // TEMP
#ifndef TEMP
	// 현재시간
	_dwCur = timeGetTime();
	
	// 로직 FPS계산
	_CLogicFPS.CalcFPS();

	if (_isSkip == TRUE) {
		// 지난번에 프레임스킵이 되었다면, 따라잡은만큼 old의 누적된 시간에서 빼준다
		// 1. 지난번 스킵된 시간을 현재 시간에서 빼고 (이득본 시간)
		// 2. 이득본 시간을 20에서 빼고(20은 50FPS맞추기위한 sleep값)
		// 3. 이전 프레임 시간에서 2번을 빼 이득본 시간을 최종으로 구하고
		// 4. _dwOld는 시간을 따라잡은만큼 빼주기
		_dwOld = _dwCur - (_dwDelta - (20 - (_dwCur - _dwSkipTime)));
		_isSkip = FALSE;
	}

	// 한프레임 시간 구하기
	_dwDelta = _dwCur - _dwOld;

	// 한프레임 이상 차이나면 한번 스킵해주기
	if (_dwDelta < 40) {
		// 스킵없음
		// 랜더링 FPS계산
		_CRenderFPS.CalcFPS();
		// 한바퀴를 돌았는데 _iDelayTime 보다 작으면 그만큼 sleep
		if (_dwDelta < 20) {
			Sleep(20 - _dwDelta);
		}
		// 20 이하 : sleep한 만큼 old에 더한다
		// 20 초과 : 초과한 만큼 old에서 뺸다 (누적)
		_dwOld = _dwCur - (_dwDelta - 20);

		return TRUE;
	} else {
		// 프레임스킵
		// 현재시간을 저장해둔다
		_isSkip = TRUE;
		_dwSkipTime = _dwCur;
		return FALSE;
	}
#endif // !TEMP
}

int CFrameSkip::GetLogicFPS() {
	return _CLogicFPS.GetFPS();
}

int CFrameSkip::GetRenderFPS() {
	return  _CRenderFPS.GetFPS();
}
