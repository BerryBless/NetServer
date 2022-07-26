#include "pch.h"
#include "CProjectile.h"
#include "CCreature.h"
#include "Managers.h"

// (x,y) 좌표에 attack의 공격력, speed의 프레임마다 dir방향으로 이동하는 투사체 발싸
CProjectile::CProjectile(int iX, int iY, int attack,int speed, Direction dir) {
	_pos.X = iX;
	_pos.Y = iY;
	_attack = attack;
	_dir = dir;
	_type = ObjectType::Projectile;
	_speed = speed;
	_tick = 0;
	_collison = true;
	_activate = true;
}

// 매 프레임 마다 _dir 방향으로 한칸
void CProjectile::Update() {
	frame_t now = I_FRAMEWORK->Frame();
	if (now - _tick >= _speed) {
		if (OBJECT_MOVE(_pos.X + _dx[(int) _dir], _pos.Y + _dy[(int) _dir]) == false)
			DESTROY(this);	// 이동에 실패했으면 파괴
		_tick = now;
	}
}

// target이 크리쳐일경우 target->Hit(attack) 호출
void CProjectile::OnCollison(IObject *pTarget) {
	// 크리쳐에만 유요한 충돌이다!
	CCreature *pCreature = dynamic_cast<CCreature *> (pTarget);
	if (pCreature != NULL) {
		pCreature->Hit(this->_attack);
	}
}

	// _dir 방향에 따른 스프라이트 출력
void CProjectile::Render() {
	switch (_dir) {
	case Direction::Up:
		SPRITE_DRAW(_pos.X, _pos.Y, '^');
		break;
	case Direction::Right:
		SPRITE_DRAW(_pos.X, _pos.Y, '>');
		break;
	case Direction::Down:
		SPRITE_DRAW(_pos.X, _pos.Y, 'v');
		break;
	case Direction::Left:
		SPRITE_DRAW(_pos.X, _pos.Y, '<');
		break;
	default:
		break;
	}
}

// 2번 파괴 방지
void CProjectile::Destroy() {
	_activate = false;
}
