#include "pch.h"
#include "PacketProcess.h"

extern std::list<CCharacter *> g_sector[dfSECTOR_MAX_Y][dfSECTOR_MAX_X];

int g_syncCnt = 0;

BOOL PacketProc(CSession *pSession, CPacket *pPacket, BYTE	byType) {
	_LOG(dfLOG_LEVEL_DEBUG, L"PacketProc :: byType[%hhu] SID[%d]\n", byType, pSession->_SID);
	switch (byType) {
	case dfPACKET_CS_MOVE_START:
		return CS_MoveStart(pSession, pPacket);

	case dfPACKET_CS_MOVE_STOP:
		return CS_MoveStop(pSession, pPacket);

	case dfPACKET_CS_ATTACK1:
		return CS_ActionAttack_1(pSession, pPacket);

	case dfPACKET_CS_ATTACK2:
		return CS_ActionAttack_2(pSession, pPacket);

	case dfPACKET_CS_ATTACK3:
		return CS_ActionAttack_3(pSession, pPacket);

	case dfPACKET_CS_ECHO:
		return CS_ECHO(pSession, pPacket);

	default:
		return FALSE;
	}
}

//#define dfTEMP
BOOL InstantiateCharacter(CSession *pSession) {
	if (pSession == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// InstantiateCharacter : pSession IS NULL.");
		CRASH();
	}

	CCharacter *pCharacter = CreateCharacter(pSession->_SID);
	if (pCharacter == NULL) {
		//캐릭터 생성 실패
		_LOG(dfLOG_LEVEL_ERROR, L"////// CreateCharacter : SID[%d] There's already an object.", pSession->_SID);
		CRASH();
		return FALSE;
	}
	// 지역변수 
	st_PACKET_HEADER header;
	CPacket packet;

	// 초기값 넣기
	pCharacter->_pSession = pSession;
	pCharacter->_SID = pSession->_SID;
	pCharacter->_chHP = 100;
#ifdef dfTEMP
	pCharacter->_X = 100;// rand() % dfRANGE_MOVE_RIGHT;
	pCharacter->_Y = 100;// rand() % dfRANGE_MOVE_BOTTOM;
#else
	pCharacter->_X = rand() % dfRANGE_MOVE_RIGHT;
	pCharacter->_Y = rand() % dfRANGE_MOVE_BOTTOM;
#endif // dfTEMP

	pCharacter->_action = dfACTION_STAND;
	pCharacter->_direction = dfPACKET_MOVE_DIR_LL;
	pCharacter->_oldSecPos._X = dfSECTOR_MAX_X + 10; // 섹터 안겹치게 최대값
	pCharacter->_oldSecPos._Y = dfSECTOR_MAX_Y + 10;


	// 캐릭터 생성패킷보내기
	_LOG(dfLOG_LEVEL_DEBUG, L" CreateCharacter : SID[%d]  :: Create My Character ", pSession->_SID);
	MakePacket_Create_My_Character(&header, &packet,
		pSession->_SID, pCharacter->_direction, pCharacter->_X, pCharacter->_Y, pCharacter->_chHP);
	SendUnicast(pSession, &header, &packet);



	//---------------------------------------------------------------
	// 다른놈들을 나한테 보내기
	// 섹터
	//---------------------------------------------------------------
	_LOG(dfLOG_LEVEL_DEBUG, L" CreateCharacter : SID[%d]  :: Create Other Character", pSession->_SID);
	st_SECTOR_AROUND around;
	std::list<CCharacter *> *pSectorList; // 섹터에 있는 캐릭터 리스트
	CCharacter *pExistCharacter;	// 섹터에 있던 캐릭터
	Sector_AddCharacter(pCharacter);
	GetSectorAround(pCharacter->_curSecPos._X, pCharacter->_curSecPos._Y, &around);

	//---------------------------------------------------------------
	// 다른놈에게 내가 왔다고 알리기
	//---------------------------------------------------------------
	_LOG(dfLOG_LEVEL_DEBUG, L" CreateCharacter : SID[%d]  :: me -> other ", pSession->_SID);
	MakePacket_Create_Other_Character(&header, &packet,
		pCharacter->_SID,
		pCharacter->_direction,
		pCharacter->_X,
		pCharacter->_Y,
		pCharacter->_chHP);

	for (int i = 0; i < around._count; i++) {
		SendSectorOne(
			around._around[i]._X, around._around[i]._Y,
			pCharacter->_pSession,
			&header, &packet);
	}

	//---------------------------------------------------------------
	// 원래 있던 놈 다 보내주기
	//---------------------------------------------------------------
	_LOG(dfLOG_LEVEL_DEBUG, L" CreateCharacter : SID[%d]  :: other -> me", pSession->_SID);
	for (int i = 0; i < around._count; i++) {
		// 얻어진 섹터 리스트 접근
		pSectorList = &g_sector[around._around[i]._Y][around._around[i]._X];
		// 해당 섹터에 등록된 캐릭터를 뽑아서 생성패킷 만들어보냄
		for (auto iter = pSectorList->begin(); iter != pSectorList->end(); ++iter) {
			pExistCharacter = *iter;

			if (pExistCharacter == pCharacter) {
				// 나는 안보내도됨
				continue;
			}

			MakePacket_Create_Other_Character(&header, &packet,
				pExistCharacter->_SID,
				pExistCharacter->_direction,
				pExistCharacter->_X,
				pExistCharacter->_Y,
				pExistCharacter->_chHP);

			SendUnicast(pCharacter->_pSession, &header, &packet);

			// 그녀셕이 이동중일경우 이동패킷 보냄
			switch (pExistCharacter->_action) {
			case dfACTION_MOVE_LL:
			case dfACTION_MOVE_LU:
			case dfACTION_MOVE_UU:
			case dfACTION_MOVE_RU:
			case dfACTION_MOVE_RR:
			case dfACTION_MOVE_RD:
			case dfACTION_MOVE_DD:
			case dfACTION_MOVE_LD:
				_LOG(dfLOG_LEVEL_DEBUG, L"ChatacterSectorUpdatePacket 4) CHARACTER_MOVE\n\tID[%d] world pos[%d, %d], action[%d], movedir [%d] dir[%d]", pExistCharacter->_SID, pExistCharacter->_X, pExistCharacter->_Y, pExistCharacter->_action, pExistCharacter->_moveDirection, pExistCharacter->_direction);

				MakePacket_Move_Start(&header, &packet,
					pExistCharacter->_SID,
					pExistCharacter->_moveDirection,
					pExistCharacter->_X,
					pExistCharacter->_Y);

				SendUnicast(pCharacter->_pSession, &header, &packet);
				break;
			}

		}

	}


	_LOG(dfLOG_LEVEL_DEBUG, L"InstantiateCharacter :: SID[%d] WORLD[%d, %d] SECTOR[%d %d] ACTION[%d]\n", pCharacter->_SID, pCharacter->_X, pCharacter->_direction, pCharacter->_curSecPos._X, pCharacter->_curSecPos._Y, pCharacter->_action);
	return TRUE;
}

#pragma region Client -> Server
//---------------------------------------------------------------
// 캐릭터 이동시작 패킷						Client -> Server
//
// 자신의 캐릭터 이동시작시 이 패킷을 보낸다.
// 이동 중에는 본 패킷을 보내지 않으며, 키 입력이 변경되었을 경우에만
// 보내줘야 한다.
//
// (왼쪽 이동중 위로 이동 / 왼쪽 이동중 왼쪽 위로 이동... 등등)
//
//	1	-	Direction	( 방향 디파인 값 8방향 사용 )
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------
BOOL CS_MoveStart(CSession *pSession, CPacket *pPacket) {

	if (pSession == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// CS_MoveStart : pSession IS NULL.");
		CRASH();
	}
	if (pPacket == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// CS_MoveStart : pPacket IS NULL.");
		CRASH();
	}


	CCharacter *pCharacter = FindCharacter(pSession->_SID);
	if (pCharacter == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// CS_MoveStart(). FindCharacter() : SID[%d] can't find the object.", pSession->_SID);
		CRASH();
		return FALSE;
	}
	st_PACKET_HEADER header;
	CPacket packet;
	BYTE dir;
	WORD x;
	WORD y;
	*pPacket >> dir;
	*pPacket >> x;
	*pPacket >> y;
	_LOG(dfLOG_LEVEL_DEBUG, L"CS_MoveStart get dir[%d], (x, y) [%d, %d]", dir, x, y);

	//---------------------------------------------------------------
	// 클라 좌표 보정
	//---------------------------------------------------------------
	if (abs(pCharacter->_X - x) > dfERROR_RANGE ||
		abs(pCharacter->_Y - y) > dfERROR_RANGE) {

		// 동기화 패킷 보내기
		_LOG(dfLOG_LEVEL_WARNING, L"/////// SYNC ERROR _ CS_MoveStart() ::\n\tCharacter ID[%d] hp[%d]\n\tCharacter Action [%d | %d]\n\tCharacter Position (%d, %d)\n\tRecv Position (%d, %d)",
			pCharacter->_SID, pCharacter->_chHP, pCharacter->_action, pCharacter->_moveDirection, pCharacter->_X, pCharacter->_Y, x, y);

		MakePacket_Sync(&header, &packet, pCharacter->_SID, pCharacter->_X, pCharacter->_Y);
		SendUnicast(pSession, &header, &packet);

		// 좌표 보정
		x = pCharacter->_X;
		y = pCharacter->_Y;
	}
	pCharacter->_X = x;
	pCharacter->_Y = y;
	//---------------------------------------------------------------
	// 현재 액션 == 방향값
	//---------------------------------------------------------------
	pCharacter->_action = dir;


	//---------------------------------------------------------------
	// 방향표시용
	//---------------------------------------------------------------
	pCharacter->_moveDirection = dir;


	//---------------------------------------------------------------
	// 바라볼 방향값
	//---------------------------------------------------------------
	switch (dir) {
	case dfACTION_MOVE_RU:
	case dfACTION_MOVE_RR:
	case dfACTION_MOVE_RD:
		pCharacter->_direction = dfACTION_MOVE_RR;
		break;
	case dfACTION_MOVE_LL:
	case dfACTION_MOVE_LU:
	case dfACTION_MOVE_LD:
		pCharacter->_direction = dfACTION_MOVE_LL;
		break;

	default:
		break;
	}

	//---------------------------------------------------------------
	// 이동인경우 섹터 업데이트
	//---------------------------------------------------------------
	if (Sector_UpdateCharacter(pCharacter)) {
		ChatacterSectorUpdatePacket(pCharacter);
	}


	//---------------------------------------------------------------
	// 내가 보이는 섹터에만 보내기
	//---------------------------------------------------------------
	MakePacket_Move_Start(&header, &packet, pCharacter->_SID, pCharacter->_moveDirection, pCharacter->_X, pCharacter->_Y);
	SendSectorAround(pCharacter, &header, &packet);
	_LOG(dfLOG_LEVEL_DEBUG, L"CS_MoveStart :: SID[%d] WORLD[%d, %d] SECTOR[%d %d] ACTION[%d]\n", pCharacter->_SID, pCharacter->_X, pCharacter->_direction, pCharacter->_curSecPos._X, pCharacter->_curSecPos._Y, pCharacter->_action);
	return TRUE;
}

//---------------------------------------------------------------
// 캐릭터 이동중지 패킷						Client -> Server
//
// 이동중 키보드 입력이 없어서 정지되었을 때, 이 패킷을 서버에 보내준다.
//
//	1	-	Direction	( 방향 디파인 값 좌/우만 사용 )
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------
BOOL CS_MoveStop(CSession *pSession, CPacket *pPacket) {
	if (pSession == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// CS_MoveStop : pSession IS NULL.");
		CRASH();
	}
	if (pPacket == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// CS_MoveStop : pPacket IS NULL.");
		CRASH();
	}


	CCharacter *pCharacter = FindCharacter(pSession->_SID);
	if (pCharacter == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// CS_MoveStop(). FindCharacter() : SID[%d] can't find the object.", pSession->_SID);
		CRASH();
		return FALSE;
	}

	st_PACKET_HEADER header;
	CPacket packet;
	BYTE dir;
	WORD x;
	WORD y;


	*pPacket >> dir;
	*pPacket >> x;
	*pPacket >> y;
	_LOG(dfLOG_LEVEL_DEBUG, L"CS_MoveStop get dir[%d], (x, y) [%d, %d]", dir, x, y);

	//---------------------------------------------------------------
	// 클라 좌표 보정
	//---------------------------------------------------------------
	if (abs(pCharacter->_X - x) > dfERROR_RANGE ||
		abs(pCharacter->_Y - y) > dfERROR_RANGE) {

		// 동기화 패킷 보내기
		_LOG(dfLOG_LEVEL_WARNING, L"/////// SYNC ERROR _ CS_MoveStop() ::\n\tCharacter ID[%d] hp[%d]\n\tCharacter Action [%d | %d]\n\tCharacter Position (%d, %d)\n\tRecv Position (%d, %d)",
			pCharacter->_SID,pCharacter->_chHP, pCharacter->_action, pCharacter->_moveDirection, pCharacter->_X, pCharacter->_Y, x, y);

		MakePacket_Sync(&header, &packet, pCharacter->_SID, pCharacter->_X, pCharacter->_Y);
		SendUnicast(pSession, &header, &packet);

		// 좌표 보정
		x = pCharacter->_X;
		y = pCharacter->_Y;
	}
	pCharacter->_X = x;
	pCharacter->_Y = y;
	//---------------------------------------------------------------
	// 이동인경우 섹터 업데이트
	//---------------------------------------------------------------
	if (Sector_UpdateCharacter(pCharacter)) {
		ChatacterSectorUpdatePacket(pCharacter);
	}

	pCharacter->_direction = dir;
	pCharacter->_action = dfACTION_STAND;

	MakePacket_Move_Stop(&header, &packet, pSession->_SID, dir, x, y);

	//---------------------------------------------------------------
	// 내가 보이는 섹터에만 보내기
	//---------------------------------------------------------------
	SendSectorAround(pCharacter, &header, &packet);

	_LOG(dfLOG_LEVEL_DEBUG, L"CS_MoveStop :: SID[%d] WORLD[%d, %d] SECTOR[%d %d] ACTION[%d]\n", pCharacter->_SID, pCharacter->_X, pCharacter->_direction, pCharacter->_curSecPos._X, pCharacter->_curSecPos._Y, pCharacter->_action);
	return TRUE;
}

//---------------------------------------------------------------
// 캐릭터 공격 패킷							Client -> Server
//
// 공격 키 입력시 본 패킷을 서버에게 보낸다.
// 충돌 및 데미지에 대한 결과는 서버에서 알려 줄 것이다.
//
// 공격 동작 시작시 한번만 서버에게 보내줘야 한다.
//
//	1	-	Direction	( 방향 디파인 값. 좌/우만 사용 )
//	2	-	X
//	2	-	Y	
//
//---------------------------------------------------------------
BOOL CS_ActionAttack_1(CSession *pSession, CPacket *pPacket) {
	if (pSession == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// CS_ActionAttack_1 : pSession IS NULL.");
		CRASH();
	}
	if (pPacket == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// CS_ActionAttack_1 : pPacket IS NULL.");
		CRASH();
	}



	CCharacter *pAttacker = FindCharacter(pSession->_SID);
	if (pAttacker == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// CS_ActionAttack_1 FindCharacter : SID[%d] can't find the object.", pSession->_SID);
		CRASH();
		return FALSE;
	}
	pAttacker->_action = dfACTION_ATTACK1;

	// 패킷 만들어 보내기
	HitScanAndSend(pAttacker);
	_LOG(dfLOG_LEVEL_DEBUG, L"CS_ActionAttack_1 :: SID[%d]\n", pAttacker->_SID);
	return TRUE;
}

//---------------------------------------------------------------
// 캐릭터 공격 패킷							Client -> Server
//
// 공격 키 입력시 본 패킷을 서버에게 보낸다.
// 충돌 및 데미지에 대한 결과는 서버에서 알려 줄 것이다.
//
// 공격 동작 시작시 한번만 서버에게 보내줘야 한다.
//
//	1	-	Direction	( 방향 디파인 값. 좌/우만 사용 )
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------
BOOL CS_ActionAttack_2(CSession *pSession, CPacket *pPacket) {
	if (pSession == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// CS_ActionAttack_2 : pSession IS NULL.");
		CRASH();
	}
	if (pPacket == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// CS_ActionAttack_2 : pPacket IS NULL.");
		CRASH();
	}



	CCharacter *pAttacker = FindCharacter(pSession->_SID);
	if (pAttacker == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// CS_ActionAttack_2 FindCharacter : SID[%d] can't find the object.", pSession->_SID);
		CRASH();
		return FALSE;
	}
	pAttacker->_action = dfACTION_ATTACK2;

	// 패킷 만들어 보내기
	HitScanAndSend(pAttacker);
	_LOG(dfLOG_LEVEL_DEBUG, L"CS_ActionAttack_2 :: SID[%d]\n", pAttacker->_SID);
	return TRUE;
}

//---------------------------------------------------------------
// 캐릭터 공격 패킷							Client -> Server
//
// 공격 키 입력시 본 패킷을 서버에게 보낸다.
// 충돌 및 데미지에 대한 결과는 서버에서 알려 줄 것이다.
//
// 공격 동작 시작시 한번만 서버에게 보내줘야 한다.
//
//	1	-	Direction	( 방향 디파인 값. 좌/우만 사용 )
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------
BOOL CS_ActionAttack_3(CSession *pSession, CPacket *pPacket) {
	if (pSession == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// CS_ActionAttack_3 : pSession IS NULL.");
		CRASH();
	}
	if (pPacket == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// CS_ActionAttack_3 : pPacket IS NULL.");
		CRASH();
	}


	CCharacter *pAttacker = FindCharacter(pSession->_SID);
	if (pAttacker == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// CS_ActionAttack_1 FindCharacter : SID[%d] can't find the object.", pSession->_SID);
		CRASH();
		return FALSE;
	}
	pAttacker->_action = dfACTION_ATTACK3;

	// 패킷 만들어 보내기
	HitScanAndSend(pAttacker);
	_LOG(dfLOG_LEVEL_DEBUG, L"CS_ActionAttack_1 :: SID[%d]\n", pAttacker->_SID);
	return TRUE;
}

//---------------------------------------------------------------
// Echo 용 패킷					Client -> Server
//
//	4	-	Time
//
//---------------------------------------------------------------
BOOL CS_ECHO(CSession *pSession, CPacket *pPacket) {
	if (pSession == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// CS_ECHO : pSession IS NULL.");
		CRASH();
	}
	if (pPacket == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// CS_ECHO : pPacket IS NULL.");
		CRASH();
	}


	_LOG(dfLOG_LEVEL_DEBUG, L"Recv ECHO");

	DWORD time;
	*pPacket >> time;
	_LOG(dfLOG_LEVEL_DEBUG, L"Recv ECHO time[%d]", time);



	st_PACKET_HEADER header;
	CPacket packet;
	_LOG(dfLOG_LEVEL_DEBUG, L"Send ECHO");

	MakePacket_Echo(&header, &packet, time);
	_LOG(dfLOG_LEVEL_DEBUG, L"Send ECHO now[%d]", time);

	SendUnicast(pSession, &header, &packet);

	return TRUE;
}


#pragma endregion



#pragma region Server -> Client
//---------------------------------------------------------------
// 클라이언트 자신의 캐릭터 할당		Server -> Client
//
// 서버에 접속시 최초로 받게되는 패킷으로 자신이 할당받은 ID 와
// 자신의 최초 위치, HP 를 받게 된다. (처음에 한번 받게 됨)
//
// 이 패킷을 받으면 자신의 ID,X,Y,HP 를 저장하고 캐릭터를 생성시켜야 한다.
//
//	4	-	ID
//	1	-	Direction
//	2	-	X
//	2	-	Y
//	1	-	HP
//
//---------------------------------------------------------------
void MakePacket_Create_My_Character(st_PACKET_HEADER *pHeader, CPacket *pPacket, DWORD SID, BYTE dir, WORD X, WORD Y, BYTE HP) {
	if (pHeader == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// MakePacket_Create_My_Character : pHeader IS NULL.");
		CRASH();
	}
	if (pPacket == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// MakePacket_Create_My_Character : pPacket IS NULL.");
		CRASH();
	}



	// 패킷 초기화
	pPacket->Clear();

	// 패킷 만들기
	*pPacket << SID;
	*pPacket << dir;
	*pPacket << X;
	*pPacket << Y;
	*pPacket << HP;

	// 헤더 만들기
	pHeader->byCode = dfPACKET_CODE;
	pHeader->byType = dfPACKET_SC_CREATE_MY_CHARACTER;
	pHeader->bySize = (BYTE) pPacket->GetDataSize();
	_LOG(dfLOG_LEVEL_DEBUG, L"MakePacket_Create_My_Character (%d %d %d %d %d)", SID, dir, X, Y, HP);
}


//---------------------------------------------------------------
// 다른 클라이언트의 캐릭터 생성 패킷		Server -> Client
//
// 처음 서버에 접속시 이미 접속되어 있던 캐릭터들의 정보
// 또는 게임중에 접속된 클라이언트들의 생성 용 정보.
//
//
//	4	-	ID
//	1	-	Direction
//	2	-	X
//	2	-	Y
//	1	-	HP
//
//---------------------------------------------------------------
void MakePacket_Create_Other_Character(st_PACKET_HEADER *pHeader, CPacket *pPacket, DWORD SID, BYTE dir, WORD X, WORD Y, BYTE HP) {
	if (pHeader == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// MakePacket_Create_Other_Character : pHeader IS NULL.");
		CRASH();
	}
	if (pPacket == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// MakePacket_Create_Other_Character : pPacket IS NULL.");
		CRASH();
	}

	// 패킷 초기화
	pPacket->Clear();

	// 패킷 만들기
	*pPacket << SID;
	*pPacket << dir;
	*pPacket << X;
	*pPacket << Y;
	*pPacket << HP;

	// 헤더 만들기
	pHeader->byCode = dfPACKET_CODE;
	pHeader->byType = dfPACKET_SC_CREATE_OTHER_CHARACTER;
	pHeader->bySize = (BYTE) pPacket->GetDataSize();
	_LOG(dfLOG_LEVEL_DEBUG, L"MakePacket_Create_Other_Character (%d %d %d %d %d)", SID, dir, X, Y, HP);
}


//---------------------------------------------------------------
// 캐릭터 삭제 패킷						Server -> Client
//
// 캐릭터의 접속해제 또는 캐릭터가 죽었을때 전송됨.
//
//	4	-	ID
//
//---------------------------------------------------------------
void MakePacket_Delete_Character(st_PACKET_HEADER *pHeader, CPacket *pPacket, DWORD SID) {
	if (pHeader == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// MakePacket_Delete_Character : pHeader IS NULL.");
		CRASH();
	}
	if (pPacket == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// MakePacket_Delete_Character : pPacket IS NULL.");
		CRASH();
	}

	// 패킷 초기화
	pPacket->Clear();

	// 패킷 만들기
	*pPacket << SID;

	// 헤더 만들기
	pHeader->byCode = dfPACKET_CODE;
	pHeader->byType = dfPACKET_SC_DELETE_CHARACTER;
	pHeader->bySize = (BYTE) pPacket->GetDataSize();
	_LOG(dfLOG_LEVEL_DEBUG, L"MakePacket_Delete_Character (%d )", SID);
}


//---------------------------------------------------------------
// 캐릭터 이동시작 패킷						Server -> Client
//
// 다른 유저의 캐릭터 이동시 본 패킷을 받는다.
// 패킷 수신시 해당 캐릭터를 찾아 이동처리를 해주도록 한다.
//
// 패킷 수신 시 해당 키가 계속해서 눌린것으로 생각하고
// 해당 방향으로 계속 이동을 하고 있어야만 한다.
//
//	4	-	ID
//	1	-	Direction	( 방향 디파인 값 8방향 )
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------
void MakePacket_Move_Start(st_PACKET_HEADER *pHeader, CPacket *pPacket, DWORD SID, BYTE dir, WORD X, WORD Y) {
	if (pHeader == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// MakePacket_Move_Start : pHeader IS NULL.");
		CRASH();
	}
	if (pPacket == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// MakePacket_Move_Start : pPacket IS NULL.");
		CRASH();
	}

	// 패킷 초기화
	pPacket->Clear();

	// 패킷 만들기
	*pPacket << SID;
	*pPacket << dir;
	*pPacket << X;
	*pPacket << Y;

	// 헤더 만들기
	pHeader->byCode = dfPACKET_CODE;
	pHeader->byType = dfPACKET_SC_MOVE_START;
	pHeader->bySize = (BYTE) pPacket->GetDataSize();
	_LOG(dfLOG_LEVEL_DEBUG, L"MakePacket_Move_Start (%d %d %d %d)", SID, dir, X, Y);
}


//---------------------------------------------------------------
// 캐릭터 이동중지 패킷						Server -> Client
//
// ID 에 해당하는 캐릭터가 이동을 멈춘것이므로
// 캐릭터를 찾아서 방향과, 좌표를 입력해주고 멈추도록 처리한다.
//
//	4	-	ID
//	1	-	Direction	( 방향 디파인 값. 좌/우만 사용 )
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------
void MakePacket_Move_Stop(st_PACKET_HEADER *pHeader, CPacket *pPacket, DWORD SID, BYTE dir, WORD X, WORD Y) {
	if (pHeader == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// MakePacket_Move_Stop : pHeader IS NULL.");
		CRASH();
	}
	if (pPacket == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// MakePacket_Move_Stop : pPacket IS NULL.");
		CRASH();
	}

	// 패킷 초기화
	pPacket->Clear();

	// 패킷 만들기
	*pPacket << SID;
	*pPacket << dir;
	*pPacket << X;
	*pPacket << Y;

	// 헤더 만들기
	pHeader->byCode = dfPACKET_CODE;
	pHeader->byType = dfPACKET_SC_MOVE_STOP;
	pHeader->bySize = (BYTE) pPacket->GetDataSize();
	_LOG(dfLOG_LEVEL_DEBUG, L"MakePacket_Move_Stop (%d %d %d %d)", SID, dir, X, Y);
}


//---------------------------------------------------------------
// 캐릭터 공격 패킷							Server -> Client
//
// 패킷 수신시 해당 캐릭터를 찾아서 공격1번 동작으로 액션을 취해준다.
// 방향이 다를 경우에는 해당 방향으로 바꾼 후 해준다.
//
//	4	-	ID
//	1	-	Direction	( 방향 디파인 값. 좌/우만 사용 )
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------
void MakePacket_Action_Attack1(st_PACKET_HEADER *pHeader, CPacket *pPacket, DWORD SID, BYTE dir, WORD X, WORD Y) {
	if (pHeader == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// MakePacket_Action_Attack1 : pHeader IS NULL.");
		CRASH();
	}
	if (pPacket == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// MakePacket_Action_Attack1 : pPacket IS NULL.");
		CRASH();
	}

	// 패킷 초기화
	pPacket->Clear();

	// 패킷 만들기
	*pPacket << SID;
	*pPacket << dir;
	*pPacket << X;
	*pPacket << Y;

	// 헤더 만들기
	pHeader->byCode = dfPACKET_CODE;
	pHeader->byType = dfPACKET_SC_ATTACK1;
	pHeader->bySize = (BYTE) pPacket->GetDataSize();
	_LOG(dfLOG_LEVEL_DEBUG, L"MakePacket_Action_Attack1 (%d %d %d %d)", SID, dir, X, Y);
}


//---------------------------------------------------------------
// 캐릭터 공격 패킷							Server -> Client
//
// 패킷 수신시 해당 캐릭터를 찾아서 공격1번 동작으로 액션을 취해준다.
// 방향이 다를 경우에는 해당 방향으로 바꾼 후 해준다.
//
//	4	-	ID
//	1	-	Direction	( 방향 디파인 값. 좌/우만 사용 )
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------
void MakePacket_Action_Attack2(st_PACKET_HEADER *pHeader, CPacket *pPacket, DWORD SID, BYTE dir, WORD X, WORD Y) {
	if (pHeader == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// MakePacket_Action_Attack2 : pHeader IS NULL.");
		CRASH();
	}
	if (pPacket == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// MakePacket_Action_Attack2 : pPacket IS NULL.");
		CRASH();
	}

	// 패킷 초기화
	pPacket->Clear();

	// 패킷 만들기
	*pPacket << SID;
	*pPacket << dir;
	*pPacket << X;
	*pPacket << Y;

	// 헤더 만들기
	pHeader->byCode = dfPACKET_CODE;
	pHeader->byType = dfPACKET_SC_ATTACK2;
	pHeader->bySize = (BYTE) pPacket->GetDataSize();
	_LOG(dfLOG_LEVEL_DEBUG, L"MakePacket_Action_Attack2 (%d %d %d %d)", SID, dir, X, Y);
}


//---------------------------------------------------------------
// 캐릭터 공격 패킷							Server -> Client
//
// 패킷 수신시 해당 캐릭터를 찾아서 공격1번 동작으로 액션을 취해준다.
// 방향이 다를 경우에는 해당 방향으로 바꾼 후 해준다.
//
//	4	-	ID
//	1	-	Direction	( 방향 디파인 값. 좌/우만 사용 )
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------
void MakePacket_Action_Attack3(st_PACKET_HEADER *pHeader, CPacket *pPacket, DWORD SID, BYTE dir, WORD X, WORD Y) {
	if (pHeader == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// MakePacket_Action_Attack3 : pHeader IS NULL.");
		CRASH();
	}
	if (pPacket == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// MakePacket_Action_Attack3 : pPacket IS NULL.");
		CRASH();
	}


	// 패킷 초기화
	pPacket->Clear();

	// 패킷 만들기
	*pPacket << SID;
	*pPacket << dir;
	*pPacket << X;
	*pPacket << Y;

	// 헤더 만들기
	pHeader->byCode = dfPACKET_CODE;
	pHeader->byType = dfPACKET_SC_ATTACK3;
	pHeader->bySize = (BYTE) pPacket->GetDataSize();
	_LOG(dfLOG_LEVEL_DEBUG, L"MakePacket_Action_Attack3 (%d %d %d %d)", SID, dir, X, Y);
}


//---------------------------------------------------------------
// 캐릭터 데미지 패킷							Server -> Client
//
// 공격에 맞은 캐릭터의 정보를 보냄.
//
//	4	-	AttackID	( 공격자 ID )
//	4	-	DamageID	( 피해자 ID )
//	1	-	DamageHP	( 피해자 HP )
//
//---------------------------------------------------------------
void MakePacket_Damage(st_PACKET_HEADER *pHeader, CPacket *pPacket, DWORD AttackID, DWORD DamageID, BYTE HP) {
	if (pHeader == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// MakePacket_Damage : pHeader IS NULL.");
		CRASH();
	}
	if (pPacket == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// MakePacket_Damage : pPacket IS NULL.");
		CRASH();
	}


	// 패킷 초기화
	pPacket->Clear();

	// 패킷 만들기
	*pPacket << AttackID;
	*pPacket << DamageID;
	*pPacket << HP;

	// 헤더 만들기
	pHeader->byCode = dfPACKET_CODE;
	pHeader->byType = dfPACKET_SC_DAMAGE;
	pHeader->bySize = (BYTE) pPacket->GetDataSize();
	_LOG(dfLOG_LEVEL_DEBUG, L"MakePacket_Damage (%d %d %d)", AttackID, DamageID, HP);
}



//---------------------------------------------------------------
// 동기화를 위한 패킷					Server -> Client
//
// 서버로부터 동기화 패킷을 받으면 해당 캐릭터를 찾아서
// 캐릭터 좌표를 보정해준다.
//
//	4	-	ID
//	2	-	X
//	2	-	Y
//
//---------------------------------------------------------------
void MakePacket_Sync(st_PACKET_HEADER *pHeader, CPacket *pPacket, DWORD SID, WORD X, WORD Y) {
	if (pHeader == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// MakePacket_Sync : pHeader IS NULL.");
		CRASH();
	}
	if (pPacket == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// MakePacket_Sync : pPacket IS NULL.");
		CRASH();
	}


	// 패킷 초기화
	pPacket->Clear();

	// 패킷 만들기
	*pPacket << SID;
	*pPacket << X;
	*pPacket << Y;

	// 헤더 만들기
	pHeader->byCode = dfPACKET_CODE;
	pHeader->byType = dfPACKET_SC_SYNC;
	pHeader->bySize = (BYTE) pPacket->GetDataSize();
	_LOG(dfLOG_LEVEL_DEBUG, L"MakePacket_Sync (%d %d %d)", SID, X, Y);
	g_syncCnt++;
}

//---------------------------------------------------------------
// Echo 응답 패킷				Server -> Client
//
//	4	-	Time
//
//---------------------------------------------------------------

void MakePacket_Echo(st_PACKET_HEADER *pHeader, CPacket *pPacket, DWORD dwTime) {
	if (pHeader == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// MakePacket_Echo : pHeader IS NULL.");
		CRASH();
	}
	if (pPacket == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// MakePacket_Echo : pPacket IS NULL.");
		CRASH();
	}



	// 패킷 초기화
	pPacket->Clear();

	// 패킷 만들기
	*pPacket << dwTime;

	// 헤더 만들기
	pHeader->byCode = dfPACKET_CODE;
	pHeader->byType = dfPACKET_SC_ECHO;
	pHeader->bySize = (BYTE) pPacket->GetDataSize();
}

#pragma endregion
