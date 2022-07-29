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
	CLogger::SetLogLevel(dfLOG_LEVEL_ERROR);
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
	PacketProc(nullptr, SessionID, CHAT_PACKET_TYPE::ON_CLIENT_LEAVE);
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
	PacketProc(nullptr, SessionID, CHAT_PACKET_TYPE::ON_TIME_OUT);
}

void CChatServer::PacketProc(CPacket *pPacket, SESSION_ID SessionID, WORD type) {
	pPacket->AddRef();

	WCHAR errmsg[512];
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
		pPlayer->_isAlive = TRUE;
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
}


void CChatServer::PacketProcMoveSector(CPacket *pPacket, SESSION_ID SessionID) {
	pPacket->AddRef();



	pPacket->SubRef();
}

void CChatServer::PacketProcChatRequire(CPacket *pPacket, SESSION_ID SessionID) {
	pPacket->AddRef();



	pPacket->SubRef();
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
	pPacket->PutData((char *) message, msgLen);
	pPacket->SubRef();

}

void CChatServer::SendSector(CPacket *pPacket, WORD sectorX, WORD sectorY) {
	pPacket->AddRef();



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


	pPlayer->_isAlive = false;

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
