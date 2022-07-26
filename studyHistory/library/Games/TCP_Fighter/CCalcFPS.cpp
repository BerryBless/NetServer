#include "CCalcFPS.h"
#include <timeapi.h>

CCalcFPS::CCalcFPS() {
	_dwOld = timeGetTime();
	_dwFPS = 0;
	_iFrameCount = 0;
}

CCalcFPS::~CCalcFPS() {
}

void CCalcFPS::CalcFPS() {
	DWORD dwCur = timeGetTime();
	if (dwCur - _dwOld >= 1000) {
		_dwFPS = (_iFrameCount * 1000) / (dwCur - _dwOld);
		_iFrameCount = 0;
		_dwOld = dwCur;
	}
	_iFrameCount++;
}

DWORD CCalcFPS::GetFPS() {
	return _dwFPS;
}
