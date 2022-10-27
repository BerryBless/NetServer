#pragma once
#include "IObject.h"

// ============================================================================
//						CSFX
// ---------------------------------------------------------------------------
// 상속
// IObject <- CSFX
// ---
// 이펙트 출력 오브젝트
// ----
// _showframe	// 몇 프레임동안 보여줄지
// _renderTick	// 현재 몇프레임동안 보였는지 틱
// ----
// CSFX(x,y,showframe)		: (x,y)좌표에 showframe동안 이펙트를 출력하겠다, 이펙트는 충돌을 무시함
// Update()					: _renderTick이 _showframe을 넘었으면 이펙트 파괴
// Render()					: 이펙트출력
// ============================================================================

class CSFX : public IObject{
public:
	int _showframe;
	int _renderTick;
public:

	CSFX(int iX, int iY, int showframe);	// (x,y)좌표에 showframe동안 이펙트를 출력하겠다
	virtual void Update();					//_renderTick이 _showframe을 넘었으면 이펙트 파괴
	virtual void Render();					// 이펙트출력
	virtual void Destroy();					// 2번삭제 안되게 _activate = false
};

