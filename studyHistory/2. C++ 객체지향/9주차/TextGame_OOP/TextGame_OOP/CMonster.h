#pragma once
#include "CCreature.h"

// ============================================================================
//						CMonster
// ---------------------------------------------------------------------------
// 상속
// IObject <- CCreature <- CMonster
// ---
// 생성자 : CCreature를 호출, 이 오브젝트의 타입을 지정
// Render() : _pos좌표에 'M' 출력
// Destroy() : 파괴시 _pos에 이펙트 생성
// SrcToDestDir(src, dest) : src->dest를 바라보는 방향, 직선상에 있지않다면 None
// ============================================================================
class CMonster : protected CCreature{
public:
	CMonster(int iX, int iY, int hp, int attack);// CCreature를 호출, 이 오브젝트의 타입을 지정

public:
	virtual void Render();//  _pos좌표에 'M' 출력
	virtual void Destroy(); // 파괴시 _pos에 이펙트 생성

	
	Direction SrcToDestDir(Position src, Position dest);// src->dest를 바라보는 방향, 직선상에 있지않다면 None
};

