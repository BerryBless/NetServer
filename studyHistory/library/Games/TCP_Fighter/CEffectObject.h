#pragma once
#include "CBaseObject.h"

// ==================================
// 이펙트 객체
// ---
// 생성자 - 해당좌표에 소환
// 업데이트 - 프레임을 업데이트후 자동소멸
// ==================================


class CEffectObject : public CBaseObject {
public:
	CEffectObject( int iX, int iY);
	~CEffectObject();

public: // 가상함수
	 void Update();		// 오브젝트가 할 행동
	 void Render(BYTE *bypDest, int iDestWidth, int iDestHeight, int iDestPitch);	// 그리기
};

