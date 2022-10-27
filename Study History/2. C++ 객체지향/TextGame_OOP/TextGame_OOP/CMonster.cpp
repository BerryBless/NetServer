#include "pch.h"
#include "CMonster.h"
#include "Managers.h"
#include "CSFX.h"
// 생성자 : CCreature를 호출, 이 오브젝트의 타입을 지정
CMonster::CMonster(int iX, int iY, int hp, int attack) : CCreature(iX, iY,hp,attack) {
	_type = ObjectType::Monster;
}

// Render() : _pos좌표에 'M' 출력
void CMonster::Render() {
	SPRITE_DRAW(_pos.X, _pos.Y, 'M');
}
// 파괴시 _pos에 이펙트 생성
void CMonster::Destroy() {
	_activate = false; // 2번삭제 방지
	INSTANTIATE(new CSFX(_pos.X, _pos.Y, 50));
}

// src->dest를 바라보는 방향, 직선상에 있지않다면 None
Direction CMonster::SrcToDestDir(Position src, Position dest) {
	Direction rDir = Direction::None; // src -> dest 바라보는 방향
// 상 하 좌 우 src의 직선 방향에 dest가 있는지 확인!
	if (src.X == dest.X) {
		// Up
		if (src.Y > dest.Y) { rDir = Direction::Up; }
		// Down
		if (src.Y < dest.Y) { rDir = Direction::Down; }
	} else if (src.Y == dest.Y) {
		// Left
		if (src.X > dest.X) { rDir = Direction::Left; }
		// Right
		if (src.X < dest.X) { rDir = Direction::Right; }
	}

	return rDir;
}

