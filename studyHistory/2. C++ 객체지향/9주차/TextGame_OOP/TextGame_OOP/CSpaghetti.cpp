#include "pch.h"
#include "CSpaghetti.h"
#include "Managers.h"
#include "CProjectile.h"

// 생성자 : CMonster(x,y,hp,attack)를 호출, 공격속도 = atspeed, 이동속도 = mospeed
CSpaghetti::CSpaghetti(int iX, int iY, int hp, int attack, int atspeed, int mospeed)
	: CMonster(iX, iY, hp, attack) {
	_pPlayer = NULL;
	_attackspeed = atspeed;
	_movespeed = mospeed;
	_moveTick = 0;
	_attackTick = 0;
}

// Update()	:	1. SrcToDestDir(_pos, pPlayer->_pos)를 호출하여 플레이어가 직선상에있을떄 공격속도에 맞추어 투사체 생성
//				2. _movespeed <= now - _moveTick 마다 랜덤방향으로 1칸이동
void CSpaghetti::Update() {
	frame_t now = I_FRAMEWORK->Frame();

	// 플레이어가 캐싱되지 않았다면 캐싱
	if (_pPlayer == NULL)
		_pPlayer = (CPlayer *) I_FRAMEWORK->Find(ObjectType::Player);

	// 공격
	Direction diAttack;// 공격할 방향
	if (now - _attackTick >= _attackspeed) {
		// 플레이어 탐색
		diAttack = SrcToDestDir(this->_pos, _pPlayer->_pos);
		// 직선상에 있으면 화살 생성
		if (diAttack != Direction::None) {
			INSTANTIATE(new
				CProjectile(_pos.X + _dx[(int) diAttack], _pos.Y + _dy[(int) diAttack],
					_attack, _attack, diAttack));

			// 틱갱신
			_attackTick = now;
		}
	}
	// 이동
	if (now - _moveTick >= _movespeed) {
		// 랜덤방향으로
		int r = rand() % 4;
		_dir = (Direction) r;

		// 이동
		OBJECT_MOVE(_pos.X + _dx[(int) _dir], _pos.Y + _dy[(int) _dir]);

		// 틱갱신
		_moveTick = now;
	}
}

// _pos좌표에 'S' 출력
void CSpaghetti::Render() {
	SPRITE_DRAW(_pos.X, _pos.Y, 'S');
}
// 플레이어와 부딧치면 데미지를 입힘
void CSpaghetti::OnCollison(IObject *pTarget) {
	if (pTarget->_type == ObjectType::Player) {
		((CPlayer *) pTarget)->Hit(_attack);
	}
}
