#pragma once
#include "Sector.h"
#include "Session.h"
struct st_SECTOR_POS;


class CCharacter {
public:
	CSession *_pSession;
	DWORD _SID;				// 고유 ID

	DWORD _action;		// 현재 행동
	BYTE _direction;		// 현재 바라보는 방향
	BYTE _moveDirection;	// 현재 이동 방향

	// 월드좌표
	SHORT _X;				
	SHORT _Y;				

	// 섹터
	struct st_SECTOR_POS _curSecPos;
	struct st_SECTOR_POS _oldSecPos;

	// "스테이더스 오픈"
	char _chHP;

public:
	~CCharacter();
};

