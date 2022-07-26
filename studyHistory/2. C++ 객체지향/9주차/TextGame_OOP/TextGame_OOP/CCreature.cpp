#include "pch.h"
#include "CCreature.h"
#include "Managers.h"

// 정보를 받아 기본적인 크리쳐 생성
CCreature::CCreature(int iX, int iY, int hp, int attack) {
	_pos.X = iX;
	_pos.Y = iY;
	_hp = hp;
	_attack = attack;
	_dir = Direction::None;
	_collison = true;
	_activate = true;
}

// 중복파괴 방지
void CCreature::Destroy() {
	_activate = false;
}

// 피격시 타겟 입장에서 호출될 가상함수 Hit 
void CCreature::Hit(int damage) {
	// 모든 상황에서 통용될 hit
	// 맞아서 _hp가 깎였는데 hp가 0보다 작거나 같으면 소멸
	this->_hp -= damage;
	if (_hp <= 0) {
		DESTROY(this);
	}
}
