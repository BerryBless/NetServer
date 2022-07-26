#pragma once
#include "IObject.h"
// ============================================================================
//						CCreature
// ---------------------------------------------------------------------------
// 상속
// IObject <- CCreature
// ---
// 맴버 변수
// 게임내에 생명을 가진 오브젝트 (플레이어, 몬스터 등)
// 체력(_hp)와 공격력(_attack), 바라보는 방향(_dir)을 가짐
// ---
// 맴버 함수
// 생성자 : int iX, int iY, int hp, int attack 값을 받아 대입해서 생성
// 피격시 타겟 입장에서 호출될 가상함수 Hit
// Destroy() : 2번삭제 방지
// ============================================================================
class CCreature : public IObject{
public:
	int _hp;		// 체력
	int _attack;	// 공격력
	Direction _dir; // 바라보는 방향
public:
	CCreature(int iX, int iY, int hp, int attack);// 정보를 받아 기본적인 크리쳐 생성
public:
	virtual void Destroy(); // 2번삭제 안되게 _activate = false
	virtual void Hit(int damage) ;// 피격시 타겟 입장에서 호출될 가상함수 Hit
};

