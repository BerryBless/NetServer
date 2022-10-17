#pragma once
#include "framework.h"

// ===========================================
// FPS를 계산하는 클래스
// CalcFPS()가 있는 라인을 기준으로 FPS를 계산한다.
// ---
// 맴버 변수
// dfAVGCOUNT : 이 수만큼 표본을 쌓아 FPS를 계산한다.
// _dwCur : 현재시간 (timeGetTime)
// _dwOld : 이전시간 (_dwDelta 계산후 _dwCur)
// _dwDelta : 시간 변화량 (_dwCur - _dwOld)
// _dwFPS : 시간 변화량의 평균을 위한 누작
// _iFrameCount : 몇프레임 지났는지 (누적) 인덱스작업을 위해
// ---
// 맴버 함수
// 생성자 : 모든 맴버변수 초기값으로 0 지정
// CalcFPS : 이라인이 호출되는 곳을 기준으로 FPS를 계산한다.
// GetFPS : _dwFPS를 모두 더한후 dfAVGCOUNT로 나누어서 1000ms에 나눈다. (1000/avg)
// ===========================================

class CCalcFPS {
private:
	DWORD _dwOld;	// 이전시간
	DWORD _dwFPS;	// 현재시간
	int _iFrameCount;
public:
	CCalcFPS();
	~CCalcFPS();
	
	void CalcFPS();
	DWORD GetFPS();
};

