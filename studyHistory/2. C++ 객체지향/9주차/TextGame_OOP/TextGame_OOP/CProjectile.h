#pragma once
#include "IObject.h"

// ============================================================================
//						CProjectile
// ---------------------------------------------------------------------------
// 상속
// IObject <- CProjectile
// ---
// 투사체 오브젝트
// ----
// 	_dir;		// 진행방향
// _speed;		// 투사체 속도
// _tick;		// 투사체 이동속도 계산할 틱
// _attack;		// 투사체 데미지
// ----
// CProjectile(x,y, attack, speed, dir) : (x,y) 좌표에 attack의 공격력, speed의 프레임마다 dir방향으로 이동하는 투사체 발싸
// Render()					: _dir 방향에 따른 스프라이트 출력
// OnCollison(target)		: target이 크리쳐일경우 target->Hit(attack) 호출
// Destroy					: 2번삭제 안되게 _activate = false
// ============================================================================


class CProjectile : public IObject{
public:
	Direction _dir;		// 진행방향
	int _speed;			// 투사체 속도
	frame_t _tick;		// 투사체 이동속도 계산할 틱
	int _attack;		// 투사체 데미지

public:
	CProjectile(int iX, int iY,int attack, int speed, Direction dir);	// 발싸!
	virtual void Update();// 매 프레임 마다 _dir 방향으로 한칸
	virtual void OnCollison(IObject *pTarget); // target이 크리쳐일경우 target->Hit(attack) 호출
	virtual void Render();// _dir 방향에 따른 스프라이트 출력
	virtual void Destroy(); // _activate = false
};

