#include "CMonitoring.h"

CMonitoring::CMonitoring() {
	_dwOld = timeGetTime();
	_dwTPS = 0;
	_dwTotal = 0;
	_iCounter = 0;
}

CMonitoring::~CMonitoring() {
}

BOOL CMonitoring::Check(BOOL bCnt) {
	DWORD dwCur = timeGetTime(); // 현재시간
	if (bCnt) {
		_iCounter++;
		_dwTotal++;
	}
	if (dwCur - _dwOld >= 1000) {
		// 1초마다 갱신
		_dwTPS = (_iCounter * 1000) / (dwCur - _dwOld);
		_iCounter = 0;
		_dwOld = dwCur;
		return TRUE;
	}
	return FALSE;
}

DWORD CMonitoring::GetTPS() {
	Check(false);
	return _dwTPS;
}


