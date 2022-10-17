#include "pch.h"
#include "CPortal.h"
#include "Managers.h"
#include "CParser.h" // Strcpy

CPortal::CPortal(int iX, int iY, const WCHAR* nextscene) {
	_pos.X = iX;
	_pos.Y = iY;
	_collison = true;
	_activate = true;
	_type = ObjectType::Portal;
	Strcpy(_nextscene, nextscene);	// 이 포탈을 타면 이동할 씬이름
}

void CPortal::Render() {
	SPRITE_DRAW(_pos.X, _pos.Y, '@');
}

// player가 충돌하면 이함수를 호출하여 다음씬을 로딩함
void CPortal::LoadNextScene() {
	I_SCENEMANAGER->Load(_nextscene);
}
