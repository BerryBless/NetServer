#include "pch.h"
#include "CCalcFPSModule.h"

CCalcFPSModule::CCalcFPSModule() {
	_oldTime = timeGetTime();
	_oldCntTime = timeGetTime();
	_TPS = 0;
	_iCounter = 0;

	_timeMin = 0x7fffffff;
	_timeMax = -1;
	_timeTotal = 0;
}

CCalcFPSModule::~CCalcFPSModule() {
}

BOOL CCalcFPSModule::Check(BOOL bCnt) {
	DWORD dwCur = timeGetTime(); // 현재시간
	if (bCnt) {
		 _DeltaTime = dwCur - _oldCntTime;
		_iCounter++;

		// MAX
		if ((int) _timeMax < (int) _DeltaTime) {
			_timeMax = _DeltaTime;
		}
		// MIN
		if ((int) _timeMin > (int)_DeltaTime) {
			_timeMin = _DeltaTime;
		}
		// TOTAL
		_timeTotal += _DeltaTime;


		_oldCntTime = dwCur;
	}
	if (dwCur - _oldTime >= 1000) {
		// 1초마다 갱신
		_TPS = (_iCounter * 1000) / (dwCur - _oldTime);
		_oldTime = dwCur;

		// 평균
		if (_iCounter > 0)
			_timeAvg = _timeTotal / _iCounter;
		// 초기화
		_iCounter = 0;

		return TRUE;
	}
	return FALSE;
}

void CCalcFPSModule::LogInfo() {
	_LOG(dfLOG_LEVEL_ERROR, L"delta time : min [%d], MAX[%d], total[%d], avg[%d]", _timeMin, _timeMax, _timeTotal, _timeAvg);
	_timeMin = 0x7fffffff;
	_timeMax = -1;
	_timeTotal = 0;
}

DWORD CCalcFPSModule::GetTPS() {
	Check(false);
	return _TPS;
}

DWORD CCalcFPSModule::DeltaTime() {
	return _DeltaTime;
}
