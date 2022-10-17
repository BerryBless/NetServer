#include "pch.h"
#include "CWall.h"
#include "Managers.h"
CWall::CWall(int iX, int iY) {
	_pos.X = iX;
	_pos.Y = iY;
	_collison = true;
	_activate = true;
	_type = ObjectType::Wall;
}
 void CWall::Render() {
	 SPRITE_DRAW(_pos.X,_pos.Y,'#');
}