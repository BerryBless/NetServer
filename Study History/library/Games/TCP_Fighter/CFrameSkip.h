#pragma once
#include "framework.h"
#include "CCalcFPS.h"



// =============================================
// 프레임 스킵
/*

랜더링을 스킵하지 않는 경우
	시작부분과 끝나는 시간의 시간차를 20ms에서 빼서, 그 값만큼 sleep을 건다.

랜더링을 스킵하는 경우
	초과한 시간을 누적으로 보관하며 가져가야 한다.
	초과분이 1프레임이 됐을 때 프레임을 버린다.
	20ms가 넘어간 애들을 보관해 뒀다가, 20ms가 나오면 프레임 스킵을 한번 한다.
	이것도 완벽히 정확한게 아니다.
		랜더링 한 번 패스한게 20ms가 나오는게 아니기 때문에.
		따라서 20ms가 넘어서 스킵을 한 번 했다고 누적분을 0으로 초기화 하는 것이 아니다.
		초과했다는 것은 20ms보다 조금 더 넘었다는것.
		즉 자투리 시간을 더 버리게 되면 시간이 틀어지게 된다.
	따라서, 보관하되 시간으로 합산되게 가져가자.
		3ms가 초과했다면, 다음프레임의 시간으로 합산되어 나오게 한다.
		그러다 이게 1프레임이 되면 스킵하고 넘긴다.
		그럼 다음 프레임은 스킵되었으므로 빨라지고, 한 프레임은 자동적으로 작아지게 되어 따라잡을 수 있는 구조가 된다.
*/
//	---
// 맴버변수
// _isSkip : 이전프레임이 Render를 스킵했으면 TRUE
// _dwOld : 이전시간 + 누적시간 (느린시간을 보관하되 시간으로 합산되게 가져가자.)
// _dwCur : 현재시간 (timeGetTime)
// _dwDelta : _dwCur -  _dwOld
// _dwSkipTime : Render가 스킵되었다면 이전프레임의 _dwCur
// 
// _CLogicFPS : 로직의 FPS
// _CRenderFPS : 랜더의 FPS
// ---
// 맴버함수
// FrameSkip : 랜더링을 스킵되어야 하는지 확인 (스킵이면 FALSE)
// GetLogicFPS : 로직 FPS
// GetRenderFPS : 렌더 FPS
// =============================================



class CFrameSkip {
private:
	BOOL _isSkip;	// 이전프레임이 스킵 되었느냐
	DWORD _dwOld;	// 이전시간 + 누적시간
	DWORD _dwCur;	// 현재시간
	DWORD _dwDelta;	// 현재 - 이전(+누적)
	DWORD _dwSkipTime;	// 스킵된 시간저장

	CCalcFPS _CLogicFPS;
	CCalcFPS _CRenderFPS;

public:
	CFrameSkip();
	~CFrameSkip();

	BOOL FrameSkip();	// TRUE면 랜더링, FALSE면 스킵
	int GetLogicFPS();
	int GetRenderFPS();
};

