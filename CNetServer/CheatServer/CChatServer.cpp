#include "pch.h"
#include "CChatServer.h"
#include "ChatServerProtocol.h"
#include "Profiler.h"
#include <conio.h>
#include <time.h>
#include <stdio.h>
#define __PLAYER_MAP_LOCK()		this->PlayerMapLock()
#define __PLAYER_MAP_UNLOCK()	this->PlayerMapUnlock()
#define __SECTOR_LOCK(x, y)		this->SectorLock(x, y)
#define __SECTOR_UNLOCK(x, y)	this->SectorUnlock(x, y)

CChatServer::CChatServer() {
	CLogger::Initialize();
	CLogger::SetDirectory(L"serverlog");
	CLogger::SetLogLevel(dfLOG_LEVEL_DEBUG);
	_isRunning = false;

	// TODO 스레드 분리 (jobQ)
	//_updateThread = INVALID_HANDLE_VALUE;
	//_monitorThread = INVALID_HANDLE_VALUE;

	// Sector 할당
	_sector = new SECTOR * [SECTOR_Y_SIZE];
	for (int i = 0; i < SECTOR_Y_SIZE; i++) {
		_sector[i] = new SECTOR[SECTOR_X_SIZE];
	}

	_SectorMoveCalc = 0;
	_SectorMoveTPS = 0;
	_ChatCalc = 0;
	_ChatTPS = 0;
	_LoginCalc = 0;
	_LoginTPS = 0;

	InitializeSRWLock(&_playerMapLock);

}

CChatServer::~CChatServer() {
}

void CChatServer::BeginServer(u_long IP, u_short port, BYTE workerThreadCount, BYTE maxRunThreadCount, BOOL nagle, u_short maxConnection) {
	if (_isRunning == true) return;
	_isRunning = Start(IP, port, workerThreadCount, maxRunThreadCount, nagle, maxConnection);

	time(&_startTime);
	localtime_s(&_timeFormet, &_startTime);
}


void CChatServer::CloseServer() {
	Quit();
}


void CChatServer::CommandWait() {
	int printTick = 0;

	for (;;) {
		if (_kbhit()) {
			char cmd = _getch();
			if (cmd == 'Q' || cmd == 'q') {

				Quit();
				break;
			}
			if (cmd == 'P' || cmd == 'p') {
				PRO_PRINT(L"CLanserver_PROFILE.log");
				PrintFileMonitor();
			}
			if (cmd == 'C' || cmd == 'c') {
				CRASH();
			}
			if (cmd == '1') {
				wprintf_s(L"CHANGE LOG LEVEL :: DEBUG\n");
				CLogger::SetLogLevel(dfLOG_LEVEL_DEBUG);
			}
			if (cmd == '2') {
				wprintf_s(L"CHANGE LOG LEVEL :: ERROR\n");
				CLogger::SetLogLevel(dfLOG_LEVEL_ERROR);
			}
			if (cmd == '3') {
				wprintf_s(L"CHANGE LOG LEVEL :: NOTICE\n");
				CLogger::SetLogLevel(dfLOG_LEVEL_NOTICE);
			}
		}
		// Monitor
		Sleep(1000);
		_hardMoniter.UpdateHardWareTime();
		_procMonitor.UpdateProcessTime();
		// profile
		if (printTick >= 300) {
			PrintFileMonitor();
			printTick = 0;
		} else
			PrintMonitor(stdout);
		printTick++;
	}

}

bool CChatServer::OnConnectionRequest(WCHAR *IPStr, u_long IP, u_short Port) {


	return _isRunning;
}

void CChatServer::OnClientJoin(SESSION_ID SessionID) {
}

void CChatServer::OnClientLeave(SESSION_ID SessionID) {
	//PacketProc(nullptr, SessionID, CHAT_PACKET_TYPE::ON_CLIENT_LEAVE);
	//Disconnect(SessionID);
}


void CChatServer::OnRecv(SESSION_ID SessionID, CPacket *pPacket) {
	pPacket->AddRef();
	// TODO jobQueue
	// TEMP 워커스레드에서 작업
	WORD type;
	(*pPacket) >> type;
	PacketProc(pPacket, SessionID, type);

	pPacket->SubRef();
}

void CChatServer::OnError(int errorcode, const WCHAR *log) {

}

void CChatServer::OnTimeout(SESSION_ID SessionID) {
	CLogger::_Log(dfLOG_LEVEL_ERROR, L"ERROR :: Time Out Case SessionID : %I64u", SessionID);
	Disconnect(SessionID);
	//PacketProc(nullptr, SessionID, CHAT_PACKET_TYPE::ON_TIME_OUT);
}

void CChatServer::PacketProc(CPacket *pPacket, SESSION_ID SessionID, WORD type) {
	pPacket->AddRef();

	WCHAR errmsg[512];
	CLogger::_Log(dfLOG_LEVEL_DEBUG, L"TYPE : %d", type);
	switch (type) {
	case CHAT_PACKET_TYPE::PACKET_CS_CHAT_REQ_LOGIN:
		PacketProcRequestLogin(pPacket, SessionID);
		break;
	case CHAT_PACKET_TYPE::PACKET_CS_CHAT_REQ_SECTOR_MOVE:
		PacketProcMoveSector(pPacket, SessionID);
		break;
	case CHAT_PACKET_TYPE::PACKET_CS_CHAT_REQ_MESSAGE:
		PacketProcChatRequire(pPacket, SessionID);
		break;
	case CHAT_PACKET_TYPE::ON_CLIENT_LEAVE:
		RemovePlayer(SessionID);
		break;

	case CHAT_PACKET_TYPE::ON_TIME_OUT:
		wsprintf(errmsg, L"ERROR :: Time Out Case SessionID : %I64u", SessionID);
		CLogger::_Log(dfLOG_LEVEL_ERROR, errmsg);
		Disconnect(SessionID);
		break;

	default:
		wsprintf(errmsg, L"ERROR :: Session Default Case SessionID : %I64u", SessionID);
		CLogger::_Log(dfLOG_LEVEL_ERROR, errmsg);
		Disconnect(SessionID);
		break;
	}

	pPacket->SubRef();
}

// PACKET_CS_CHAT_REQ_LOGIN
void CChatServer::PacketProcRequestLogin(CPacket *pPacket, SESSION_ID SessionID) {
	pPacket->AddRef();
	BYTE status = FALSE;
	ACCOUNT_NO acno;

	if (pPacket->GetDataSize() < sizeof(ACCOUNT_NO)) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"pPacket->GetDataSize() < sizeof(ACCOUNT_NO)"); // TODO ERROR MSG
		Disconnect(SessionID);
	}
	(*pPacket).GetData((char *) &acno, sizeof(ACCOUNT_NO));

	if (pPacket->GetDataSize() != ID_MAX_SIZE + NICK_NAME_MAX_SIZE + TOKEN_KEY_SIZE) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"pPacket->GetDataSize() != ID_MAX_SIZE + NICK_NAME_MAX_SIZE + TOKEN_KEY_SIZE"); // TODO ERROR MSG
		Disconnect(SessionID);
	}
	Player *pPlayer = FindPlayer(SessionID);
	if (pPlayer == nullptr) {
		// new Player
		pPlayer = _playerPool.Alloc();

		pPacket->GetData((char *) pPlayer->_ID, ID_MAX_SIZE);
		pPacket->GetData((char *) pPlayer->_NickName, NICK_NAME_MAX_SIZE);
		pPacket->GetData((char *) pPlayer->_TokenKey, TOKEN_KEY_SIZE);

		pPlayer->_AccountNo = acno;
		pPlayer->_SectorX = -1;
		pPlayer->_SectorY = -1;
		pPlayer->_SessionID = SessionID;

		InsertPlayer(SessionID, pPlayer);
		pPlayer->_isLogin = TRUE;
		status = TRUE;
	}

	pPacket->SubRef();

	CPacket *pResLoginPacket = CPacket::AllocAddRef();
	MakePacketResponseLogin(pResLoginPacket, pPlayer->_AccountNo, status);
	SendPacket(pPlayer->_SessionID, pResLoginPacket);
	if (status == FALSE) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"status == FALSE"); // TODO ERROR MSG
		Disconnect(SessionID);
	}

	InterlockedIncrement(&_LoginCalc);
	CLogger::_Log(dfLOG_LEVEL_DEBUG, L"Login OK ANO[%d]", acno); // TODO ERROR MSG

}

// PACKET_CS_CHAT_REQ_SECTOR_MOVE
void CChatServer::PacketProcMoveSector(CPacket *pPacket, SESSION_ID SessionID) {
	ACCOUNT_NO no;
	WORD sx;
	WORD sy;

	// 패킷 꺼내기
	pPacket->AddRef();
	if (pPacket->GetDataSize() != (sizeof(no) + sizeof(sx) + sizeof(sy))) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"pPacket->GetDataSize() != (sizeof(no) + sizeof(sx) + sizeof(sy))"); // TODO ERROR MSG
		Disconnect(SessionID);
		pPacket->SubRef();
		return;
	}
	pPacket->GetData((char *) &no, sizeof(ACCOUNT_NO));
	(*pPacket) >> sx >> sy;
	pPacket->SubRef();

	// 플레이어 무결성
	Player *pPlayer = FindPlayer(SessionID);
	if (pPlayer == nullptr) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"PacketProcMoveSector(id [%I64u])  pPlayer == nullptr", SessionID); // TODO ERROR MSG
		Disconnect(SessionID);
		return;
	}if (pPlayer->_isLogin == false) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"PacketProcMoveSector(id [%I64u])  pPlayer->_isLogin == false", SessionID); // TODO ERROR MSG
		Disconnect(SessionID);
		return;
	}

	// 섹터 범위 초과
	if (sx >= SECTOR_X_SIZE || sy >= SECTOR_Y_SIZE) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"PacketProcMoveSector(id [%I64u])  sx >= SECTOR_X_SIZE || sy >= SECTOR_Y_SIZE", SessionID); // TODO ERROR MSG
		Disconnect(SessionID);
		return;
	}

	// 기존에 있던 섹터가 있는지
	constexpr WORD comp = -1;
	if (pPlayer->_SectorX != comp && pPlayer->_SectorY != comp) {
		// 기존섹터 삭제
		auto iter = _sector[pPlayer->_SectorY][pPlayer->_SectorX]._playerSet.find(pPlayer);
		if (iter != _sector[pPlayer->_SectorY][pPlayer->_SectorX]._playerSet.end()) {
			_sector[pPlayer->_SectorY][pPlayer->_SectorX]._playerSet.erase(iter);
		}
	}
	// 이동
	pPlayer->_SectorX = sx;
	pPlayer->_SectorY = sy;
	_sector[pPlayer->_SectorY][pPlayer->_SectorX]._playerSet.emplace(pPlayer);

	// Send RES_SECTOR_MOVE Msg
	CPacket *pResPacket = CPacket::AllocAddRef();
	MakePacketResponseSectorMove(pResPacket, pPlayer->_AccountNo, pPlayer->_SectorX, pPlayer->_SectorY);
	SendPacket(pPlayer->_SessionID, pResPacket);
	pResPacket->SubRef();
}

// PACKET_CS_CHAT_REQ_MESSAGE
void CChatServer::PacketProcChatRequire(CPacket *pPacket, SESSION_ID SessionID) {
	ACCOUNT_NO no;
	WORD msgLen;
	WCHAR message[MASSAGE_MAX_SIZE];
	pPacket->AddRef();


	pPacket->GetData((char *) &no, sizeof(ACCOUNT_NO));
	(*pPacket) >> msgLen;
	pPacket->GetData((char *) message, msgLen * sizeof(WCHAR));
	message[msgLen] = '\0';


	pPacket->SubRef();
	Player *pSender = FindPlayer (SessionID);
	if (no != pSender->_AccountNo) {
		//TODO ERROR
	}

	CPacket *pResPacket = CPacket::AllocAddRef();

	MakePacketResponseMessage(pResPacket, pSender->_AccountNo, pSender->_ID, pSender->_NickName, msgLen, message);
	BroadcastSectorAround(pResPacket, pSender->_SectorX, pSender->_SectorY, nullptr);

	pResPacket->SubRef();
}


// PACKET_SC_CHAT_RES_LOGIN
void CChatServer::MakePacketResponseLogin(CPacket *pPacket, ACCOUNT_NO account_no, BYTE status) {
	pPacket->AddRef();
	CHAT_PACKET_TYPE type = CHAT_PACKET_TYPE::PACKET_SC_CHAT_RES_LOGIN;
	(*pPacket) << ((WORD) type) << status << (__int64) account_no;
	pPacket->SubRef();
}

//PACKET_SC_CHAT_RES_SECTOR_MOVE
void CChatServer::MakePacketResponseSectorMove(CPacket *pPacket, ACCOUNT_NO account_no, WORD sectorX, WORD sectorY) {
	pPacket->AddRef();
	CHAT_PACKET_TYPE type = CHAT_PACKET_TYPE::PACKET_SC_CHAT_RES_SECTOR_MOVE;
	(*pPacket) << ((WORD) type);
	pPacket->PutData((char *) &account_no, sizeof(ACCOUNT_NO));
	(*pPacket) << sectorX << sectorY;
	pPacket->SubRef();

}

//PACKET_SC_CHAT_RES_MESSAGE
void CChatServer::MakePacketResponseMessage(CPacket *pPacket, ACCOUNT_NO account_no, const WCHAR *ID, const WCHAR *nickName, WORD msgLen, const WCHAR *message) {
	pPacket->AddRef();
	CHAT_PACKET_TYPE type = CHAT_PACKET_TYPE::PACKET_SC_CHAT_RES_MESSAGE;
	(*pPacket) << ((WORD) type);
	pPacket->PutData((char *) &account_no, sizeof(ACCOUNT_NO));
	pPacket->PutData((char *) ID, ID_MAX_SIZE);
	pPacket->PutData((char *) nickName, NICK_NAME_MAX_SIZE);
	(*pPacket) << msgLen;
	pPacket->PutData((char *) message, msgLen * sizeof(WCHAR));
	pPacket->SubRef();

}

void CChatServer::BroadcastSector(CPacket *pPacket, WORD sectorX, WORD sectorY, Player *ex = nullptr) {
	pPacket->AddRef();

	SECTOR *pSector = &_sector[sectorY][sectorX];
	for (auto iter = pSector->_playerSet.begin(); iter != pSector->_playerSet.end(); ++iter) {
		Player *pPlayer = (*iter);
		if (pPlayer == ex) continue;
		SendPacket(pPlayer->_SessionID, pPacket);
	}

	pPacket->SubRef();
}

void CChatServer::BroadcastSectorAround(CPacket *pPacket, WORD sectorX, WORD sectorY, Player *ex = nullptr) {
	pPacket->AddRef();
	WORD sx;
	WORD sy;
	for (int dy = -1; dy < 2; ++dy) {
		sy = sectorY + dy;
		if (sy < 0 || sy >= SECTOR_Y_SIZE) continue;
		for (int dx = -1; dx < 2; ++dx) {
			sx = sectorX + dx;
			if (sx < 0 || sx >= SECTOR_X_SIZE) continue;
			BroadcastSector(pPacket, sx, sy, ex);
		}
	}


	pPacket->SubRef();
}


void CChatServer::InsertPlayer(ULONGLONG SessionID, Player *pPlayer) {
	_playerMap.emplace(::make_pair(SessionID, pPlayer));
}

void CChatServer::RemovePlayer(ULONGLONG SessionID) {
	__PLAYER_MAP_LOCK();
	auto iter = _playerMap.find(SessionID);
	if (iter == _playerMap.end()) {
		__PLAYER_MAP_UNLOCK();
		return;
	}

	Player *pPlayer = iter->second;
	_playerMap.erase(iter);
	__PLAYER_MAP_UNLOCK();


	pPlayer->_isLogin = false;

	constexpr WORD comp = -1;
	WORD sx = pPlayer->_SectorX;
	WORD sy = pPlayer->_SectorY;
	pPlayer->_SectorX = comp;
	pPlayer->_SectorY = comp;
	if (sx == comp || sy == comp) {
		_playerPool.Free(pPlayer);
		return;
	}

	__SECTOR_LOCK(sx, sy);
	for (auto iter = _sector[sy][sx]._playerSet.begin(); iter != _sector[sy][sx]._playerSet.end(); ++iter) {
		if ((*iter)->_SessionID == SessionID) {
			_sector[sy][sx]._playerSet.erase(iter);
			break;
		}
	}
	__SECTOR_UNLOCK(sx, sy);

	_playerPool.Free(pPlayer);
}

Player *CChatServer::FindPlayer(ULONGLONG SessionID) {
	__PLAYER_MAP_LOCK();
	auto iter = _playerMap.find(SessionID);
	if (iter == _playerMap.end()) {
		__PLAYER_MAP_UNLOCK();
		return nullptr;
	}
	__PLAYER_MAP_UNLOCK();
	return iter->second;
}

void CChatServer::PrintMonitor(FILE *fp) {
}

void CChatServer::PrintFileMonitor() {
}
