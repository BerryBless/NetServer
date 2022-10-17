#pragma once
class CCalcFPSModule {
private:
	DWORD _oldTime;	// 이전갱신시간
	DWORD _oldCntTime;	// 이전카운트시간
	int _iCounter;	// 실제 일어난 횟수
	DWORD _timeTotal;	// 총합
	DWORD _timeMin;	// 시간 간격 최솟값
	DWORD _timeMax;	// 시간 간격 최댓값
	DWORD _timeAvg;	// 평균
	DWORD _TPS;	// TPS
	DWORD _DeltaTime;
public:
	CCalcFPSModule();
	~CCalcFPSModule();

	// TPS를 갱신하면 TRUE;
	BOOL Check(BOOL bCnt = true);

	void LogInfo();
	DWORD GetTPS();
	DWORD DeltaTime();
};

