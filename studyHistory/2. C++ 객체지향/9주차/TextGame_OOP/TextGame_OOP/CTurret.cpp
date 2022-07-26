#include "pch.h"
#include "CTurret.h"
#include "CProjectile.h"
#include "Managers.h"
CTurret::CTurret(int iX, int iY, int hp, int attack, int atspeed) : CMonster(iX, iY, hp, attack) {
	_attackspeed = atspeed;
	_attackTick = 0;
	_pPlayer = NULL;
}

// 플레이어가 직선상에있을떄 공격속도에 맞추어 투사체 생성
void CTurret::Update() {
	frame_t now = I_FRAMEWORK->Frame();

	if (_pPlayer == NULL)  // 플레이어가 캐싱되지 않았다면
		_pPlayer = (CPlayer *) I_FRAMEWORK->Find(ObjectType::Player); // 캐싱

	Direction diAttack;// 공격할 방향
	if (now - _attackTick >= _attackspeed) {
		// 플레이어 탐색
		diAttack = SrcToDestDir(this->_pos, _pPlayer->_pos);
		// 직선상에 플레이어가 없으면 바로 리턴
		if (diAttack == Direction::None) {
			return;
		}
		//직선상에 플레이어가 있으면 공격
		INSTANTIATE(new
			CProjectile(_pos.X + _dx[(int) diAttack], _pos.Y + _dy[(int) diAttack],
				_attack, _attack, diAttack));
		// 틱카운터 갱신
		_attackTick = now;
	}
}

void CTurret::Render() {
	SPRITE_DRAW(_pos.X, _pos.Y, 'T');
}
