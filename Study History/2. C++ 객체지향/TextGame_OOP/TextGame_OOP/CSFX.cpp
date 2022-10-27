#include "pch.h"
#include "CSFX.h"
#include "Managers.h"

// (x,y)좌표에 showframe동안 이펙트를 출력하겠다
CSFX::CSFX(int iX, int iY, int showframe) {
	_pos.X = iX;
	_pos.Y = iY;
	_showframe = showframe;
	_type = ObjectType::SFX;
	_collison = false;	// 이펙트는 충돌을 무시하겠다
	_activate = true;
	_renderTick = 0;
}

// _renderTick이 _showframe을 넘었으면 이펙트 파괴
void CSFX::Update() {
	_activate = false;	// 중복파괴 방지
	if (_renderTick >= _showframe)
		DESTROY(this);
}

// 이펙트 출력
void CSFX::Render() {
	// 5 프레임 마다 바꾸기
	++_renderTick;
	if (_renderTick % 10 < 5)
		SPRITE_DRAW(_pos.X, _pos.Y, 'O');
	else
		SPRITE_DRAW(_pos.X, _pos.Y, 'X');
}

void CSFX::Destroy() {
	_activate = false;
}

