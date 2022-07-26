#pragma once
#include "CCreature.h"
#include "CQueue.h"
// ============================================================================
//						CPlayer
// ---------------------------------------------------------------------------
// 상속
// IObject <- CCreature <- CPlayer
// ---
// 플레이어 오브젝트
// 인풋 업데이트는 씬매니져에서 담당
// ---
// 맴버 함수
// 생성자	: 기본정보를 넣고, _type을 Player로
// Render()	: _pos좌표에 'P'출력
// OnCollison() : 충돌한게 포탈이면 다음씬으로
// Destroy(): 게임을 재시작함
// Input(input): input큐에 있는 메시지를 처리
// ============================================================================
class CPlayer :public CCreature {
public:
	CPlayer(int iX, int iY, int hp, int attack); // 기본정보를 넣고, _type을 Player로

	virtual void Render();	//_pos좌표에 'P'출력
	virtual void OnCollison(IObject *pTarget);	// 충돌한게 포탈이면 다음씬으로
	virtual void Destroy(); // 게임을 재시작함
	void Input(CQueue<InputMessage> &input); // input큐에 있는 메시지를 처리
};

