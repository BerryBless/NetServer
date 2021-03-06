#pragma once
#include "IScene.h"
// ============================================================================
//						CEndingScene
// ---------------------------------------------------------------------------
// 상속
// IScene <- CEndingScene
// ----
// 엔딩씬
// ----
// Update()	: 엔터 입력 대기, 입력하면 프로그램 종료
// Render()	: 엔딩출력
// ============================================================================
class CEndingScene : public IScene{
	virtual void Update();
	virtual void Render();
};

