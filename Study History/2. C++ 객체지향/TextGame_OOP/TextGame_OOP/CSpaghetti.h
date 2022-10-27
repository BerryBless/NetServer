#pragma once
#include "CMonster.h"
#include "CPlayer.h"

// ============================================================================
//						CSpaghetti
// ---------------------------------------------------------------------------
// 상속
// IObject <- CCreature <- CMonster <- CSpaghetti
// ---
// _pPlayer		: 플레이어 포인터를 캐싱
// _attackTick	: 공격을 해도될지 체크하는 틱카운터
// _moveTick	: 이동해도 될지 체크하는 틱카운터
// _attackspeed	: 몇번쨰 틱마다 공격을 할지(공격속도)
// _movespeed	: 몇번쨰 틱마다 이동을 할지(이동속도)
// ---
// 생성자 : CMonster(x,y,hp,attack)를 호출, 공격속도 = atspeed, 이동속도 = mospeed
// Update()	:	1. SrcToDestDir(_pos, pPlayer->_pos)를 호출하여 플레이어가 직선상에있을떄 공격속도에 맞추어 투사체 생성
//				2. _movespeed <= now - _moveTick 마다 랜덤방향으로 1칸이동
// Render() : _pos좌표에 'S' 출력
// OnCollison(pTarget) : pTarget의 타입이 플레이어라면 공격력만큼 피해를 줌
// ============================================================================

class CSpaghetti :public CMonster{
private :
	CPlayer *_pPlayer;	// 플레이어 포인터를 캐싱
	frame_t _attackTick;// 공격을 해도될지 체크하는 틱카운터
	frame_t _moveTick;	// 이동해도 될지 체크하는 틱카운터
	int _attackspeed;	// 공격속도
	int _movespeed;		// 이동속도
public:
	CSpaghetti(int iX, int iY, int hp, int attack,int atspeed, int mospeed);
	virtual void Update(); // 정해진 시간마다 이동, 공격
	virtual void Render(); // _pos좌표에 'S' 출력
	virtual void OnCollison(IObject *pTarget); // 플레이어와 부딧치면 데미지를 입힘
};

