#pragma once
#include "CMonster.h"
#include "CPlayer.h"


// ============================================================================
//						CTurret
// ---------------------------------------------------------------------------
// 상속
// IObject <- CCreature <- CMonster <- CTurret
// ---
// _pPlayer : 플레이어 포인터를 캐싱
// _attackTick : 공격을 해도될지 체크하는 틱카운터
// _attackspeed: 몇번쨰 틱마다 공격을 할지(공격속도)
// ---
// 생성자 : CMonster(x,y,hp,attack)를 호출, 공격속도 = atspeed
// Update() : SrcToDestDir(_pos, pPlayer->_pos)를 호출하여 플레이어가 직선상에있을떄 공격속도에 맞추어 투사체 생성
// Render() : _pos좌표에 'T' 출력
// ============================================================================

class CTurret : public CMonster {
private :
	CPlayer *_pPlayer;	// 플레이어 포인터를 캐싱
	frame_t _attackTick; // 공격 체크 틱
	int _attackspeed;// 공격속도
public:
	CTurret(int iX, int iY, int hp, int attack, int atspeed); // CMonster(x,y,hp,attack)를 호출, 공격속도 = atspeed
	virtual void Update(); // 플레이어가 직선상에있을떄 공격속도에 맞추어 투사체 생성
	virtual void Render(); // _pos좌표에 'T' 출력

};

