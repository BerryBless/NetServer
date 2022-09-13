#pragma once

#include "CorePch.h"

typedef unsigned long long ACCOUNT_NO;

struct Player {
	Player() : _ID{ 0 }, _NickName{ 0 }, _TokenKey{ 0 } {
		_sessionID = 0;

		_isLogin = 0;
		_AccountNo = 0;
		_SectorX = -1;
		_SectorY = -1;
	};
	Player(SESSION_ID sessionID, INT64 accountNo, WORD sectorX, WORD sectorY, const WCHAR *ID, const WCHAR *nickname) : _TokenKey{ 0 } {
		_sessionID = sessionID;

		_isLogin = 0;
		_AccountNo = accountNo;
		_SectorX = sectorX;
		_SectorY = sectorY;


		memcpy_s(_ID, 20, ID, 20);
		memcpy_s(_NickName, 20, nickname, 20);
	}

	SESSION_ID	_sessionID;

	DWORD		_isLogin;
	ACCOUNT_NO	_AccountNo;
	WORD		_SectorX;
	WORD		_SectorY;
	WCHAR		_ID[20];
	WCHAR		_NickName[20];
	char		_TokenKey[64];
};
