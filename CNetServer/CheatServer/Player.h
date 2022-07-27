#pragma once

#include "CorePch.h"

typedef unsigned long long ACCOUNT_NO;

struct Player {
	Player() : _isAlive(0), _SessionID(0), _AccountNo(0), _SectorX(0), _SectorY(0), _ID{ 0 }, _NickName{ 0 }, _TokenKey{ 0 }{};
	Player(SESSION_ID sessionID, INT64 accountNo, WORD sectorX, WORD sectorY, const WCHAR *ID, const WCHAR *nickname)
		:_SessionID(sessionID), _AccountNo(accountNo), _SectorX(sectorX), _SectorY(sectorY), _TokenKey{ 0 }
	{
		memcpy_s(_ID, 20, ID, 20);
		memcpy_s(_NickName, 20, nickname, 20);
	}

	SESSION_ID _SessionID;

	DWORD _isAlive;
	ACCOUNT_NO _AccountNo;
	WORD _SectorX;
	WORD _SectorY;
	WCHAR _ID[20];
	WCHAR _NickName[20];
	char _TokenKey[64];
};
