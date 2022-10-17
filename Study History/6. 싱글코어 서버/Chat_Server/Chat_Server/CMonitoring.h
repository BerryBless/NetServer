#pragma once
#include <Windows.h>
#include <timeapi.h>

class CMonitoring {
private:
	DWORD _dwOld;	// 이전시간
	DWORD _dwTPS;	// TPS
	__int64 _dwTotal; // 누적 시행 시도
	int _iCounter;	// 실제 일어난 횟수
public:
	CMonitoring();
	~CMonitoring();

	// TPS를 갱신하면 TRUE;
	BOOL Check(BOOL bCnt = true);


	DWORD GetTPS();
	__int64 GetTotal() { return _dwTotal; }
};

