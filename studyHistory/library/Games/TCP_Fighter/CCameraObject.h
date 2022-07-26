#pragma once
#include "CBaseObject.h"
// 클라에서 보여줄 카메라
class CCameraObject : public CBaseObject{
private:
	CBaseObject *_pTargetObject;
public: // 가상함수
	virtual void Update();		// 오브젝트가 할 행동
	virtual void Render(BYTE *bypDest, int iDestWidth, int iDestHeight, int iDestPitch) {}	// 그리기
	virtual void Collision(CBaseObject *cAttacker) {} // 충돌 (내가 맞는 입장)
	virtual void Destroy() {} // 삭제 _bActivate = false 해줘야함
	virtual void ActionInput(DWORD dwAction) {}	// 키보드 조작 또는 서버의 조작

public:
	void SetTarget(CBaseObject *pTarget);
};

