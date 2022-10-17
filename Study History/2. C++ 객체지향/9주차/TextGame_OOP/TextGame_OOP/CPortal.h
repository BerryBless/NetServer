#pragma once
#include "IObject.h"
// ============================================================================
//						CPortal
// ---------------------------------------------------------------------------
// 상속
// IObject <- CPortal
// ---
// 포탈 오브젝트
// 플레이어가 포탈과 충돌하면 포탈에 있는 _nextscene 을 로딩합니다
// ----
// _nextscene				: 어떤 씬을 불러올지 키값
// ----
// CPortal(x,y,nextscene)	: (x,y)에 포탈 생성, nextscene을 저장해서 LoadNextScene때 nextscene을 로딩
// Render()					: _pos 에 @ 출력
// LoadNextScene()			: player가 충돌하면 이함수를 호출하여 다음씬을 로딩함
// ============================================================================

class CPortal :public IObject{
private :
	WCHAR _nextscene[16];	// 다음 씬을 로딩할 키 (L"stage1")
public:
	CPortal(int iX, int iY, const WCHAR * nextscene);
	virtual void Render();
	void LoadNextScene();	//player가 충돌하면 이함수를 호출하여 다음씬을 로딩함
};

