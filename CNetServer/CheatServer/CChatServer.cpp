#include "pch.h"
#include "CChatServer.h"
#include "ChatServerProtocol.h"
#include "Profiler.h"
#include <conio.h>
#include <time.h>
#include <stdio.h>
//#define __PLAYER_MAP_LOCK()		this->PlayerMapLock()
//#define __PLAYER_MAP_UNLOCK()	this->PlayerMapUnlock()
//#define __SECTOR_LOCK(x, y)		this->SectorLock(x, y)
//#define __SECTOR_UNLOCK(x, y)	this->SectorUnlock(x, y)
#define __PLAYER_MAP_LOCK()		
#define __PLAYER_MAP_UNLOCK()	
#define __SECTOR_LOCK(x, y)		
#define __SECTOR_UNLOCK(x, y)	

CChatServer::CChatServer() :_hThread{ 0 }, _startTime{ 0 }, _timeFormet{ 0 } {
	CLogger::Initialize();
	CLogger::SetDirectory(L"serverlog");
	CLogger::SetLogLevel(dfLOG_LEVEL_DEBUG);
	_isRunning = false;

	// Sector 할당
	_sector = new SECTOR * [SECTOR_Y_SIZE];
	for (int i = 0; i < SECTOR_Y_SIZE; i++) {
		_sector[i] = new SECTOR[SECTOR_X_SIZE];
	}

	_SectorMoveCalc = 0;
	_SectorMoveTPS = 0;
	_ChatRecvCalc = 0;
	_ChatRecvTPS = 0;
	_ChatSendCalc = 0;
	_ChatSendTPS = 0;
	_LoginCalc = 0;
	_LoginTPS = 0;
	_UpdateCalc = 0;
	_UpdateTPS = 0;


	_totalSectorAroundSend = 0;
	_SectorAroundCount = 0;

	InitializeSRWLock(&_playerMapLock);

}

CChatServer::~CChatServer() {
}

/*void CChatServer::BeginServer(u_long IP, u_short port, BYTE workerThreadCount, BYTE maxRunThreadCount, BOOL nagle, u_short maxConnection) {
	if (_isRunning == true) return;
	_DequeueEvent = nullptr;
	_DequeueEvent = CreateEvent(nullptr, false, false, nullptr);
	if (_DequeueEvent == nullptr) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"_DequeueEvent == nullptr");
		return;
	}


	_isRunning = Start(IP, port, workerThreadCount, maxRunThreadCount, nagle, maxConnection);
	time(&_startTime);
	localtime_s(&_timeFormet, &_startTime);
	BeginThread();
}*/

void CChatServer::BeginServer(const WCHAR *szConfigFile) {
	if (_isRunning == true) return;
	_DequeueEvent = nullptr;
	_DequeueEvent = CreateEvent(nullptr, false, false, nullptr);
	if (_DequeueEvent == nullptr) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"_DequeueEvent == nullptr");
		return;
	}

	_isRunning = Start(szConfigFile);
	time(&_startTime);
	localtime_s(&_timeFormet, &_startTime);
	BeginThread();
}


void CChatServer::CloseServer() {
	Quit();
}


void CChatServer::CommandWait() {
	int printTick = 0;

	for (;;) {
		char cmd = _getch();
		if (cmd == 'Q' || cmd == 'q') {
			//MemProfiler::Instance().PrintInfo();
			Quit();
			break;
		}
		if (cmd == 'P' || cmd == 'p') {
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
}

unsigned int __stdcall CChatServer::UpdateThread(LPVOID arg) {
	CChatServer *pServer = (CChatServer *) arg;
	while (!pServer->_isRunning);
	while (pServer->UpdateProc());
	CLogger::_Log(dfLOG_LEVEL_NOTICE, L"::CHAT SERVER -- Monitor Thread IS Closed..\n");
	return 0;
}

unsigned int __stdcall CChatServer::MonitoringThread(LPVOID arg) {
	CChatServer *pServer = (CChatServer *) arg;
	while (!pServer->_isRunning);
	while (pServer->MonitoringProc());
	CLogger::_Log(dfLOG_LEVEL_NOTICE, L"::CHAT SERVER -- Monitor Thread IS Closed..\n");
	return 0;
}

bool CChatServer::UpdateProc() {
	while (_isRunning) {
		//if (_jobQueue.IsEmpty()) return _isRunning;
		WaitForSingleObject(_DequeueEvent, INFINITE);

		JobMessage *job;

		while (_jobQueue.dequeue(job)) {
			InterlockedIncrement(&_UpdateCalc);
			PacketProc(job->_pPacket, job->_SessionID, job->_Type);
			if (job->_pPacket != nullptr)
				job->_pPacket->SubRef(1);

			_jobMsgPool.Free(job);
		}
	}
	return _isRunning;
}

bool CChatServer::MonitoringProc() {
	int PrintMonitorCount = 0;
	while (_isRunning) {

		Sleep(1000);
		_SectorMoveTPS = InterlockedExchange(&_SectorMoveCalc, 0);
		_ChatRecvTPS = InterlockedExchange(&_ChatRecvCalc, 0);
		_ChatSendTPS = InterlockedExchange(&_ChatSendCalc, 0);
		_LoginTPS = InterlockedExchange(&_LoginCalc, 0);
		_UpdateTPS = InterlockedExchange(&_UpdateCalc, 0);
		_hardMoniter.UpdateHardWareTime();
		_procMonitor.UpdateProcessTime();

		_TotalSectorSize = 0;
		for (int y = 0; y < SECTOR_Y_SIZE; ++y) {
			for (int x = 0; x < SECTOR_X_SIZE; ++x) {
				_TotalSectorSize += _sector[y][x]._playerSet.size();
			}
		}




		// 5분마다 저장
		if (PrintMonitorCount >= 300) {
			PrintFileMonitor();
			PrintMonitorCount = 0;
		} else {
			PrintMonitor(stdout);
		}
		++PrintMonitorCount;
	}

	return _isRunning;
}

void CChatServer::BeginThread() {
	int i = 0;
	// Update (jobQ Thread)
	_hThread[i++] = (HANDLE) _beginthreadex(nullptr, 0, UpdateThread, this, 0, nullptr);
	// MonitorThread
	_hThread[i++] = (HANDLE) _beginthreadex(nullptr, 0, MonitoringThread, this, 0, nullptr);
}

bool CChatServer::OnConnectionRequest(WCHAR *IPStr, DWORD IP, USHORT Port) {


	return _isRunning;
}

void CChatServer::OnClientJoin(WCHAR *ipStr, DWORD ip, USHORT port, ULONGLONG sessionID) {
}

void CChatServer::OnClientLeave(SESSION_ID SessionID) {
	CLogger::_Log(dfLOG_LEVEL_DEBUG, L"ERROR :: Time Out Case SessionID : %I64u", SessionID);
	JobMessage *job = _jobMsgPool.Alloc();
	job->_SessionID = SessionID;
	job->_Type = CHAT_PACKET_TYPE::ON_CLIENT_LEAVE;
	job->_pPacket = nullptr;

	_jobQueue.enqueue(job);
	SetEvent(_DequeueEvent);
}


void CChatServer::OnRecv(SESSION_ID SessionID, Packet *pPacket) {
	pPacket->AddRef(2);
	// jobQueue
	WORD type;
	(*pPacket) >> type;

	JobMessage *job = _jobMsgPool.Alloc();
	job->_SessionID = SessionID;
	job->_Type = type;
	job->_pPacket = pPacket;
	pPacket->AddRef();

	_jobQueue.enqueue(job);
	SetEvent(_DequeueEvent);

	pPacket->SubRef(2);
}
void CChatServer::OnSend(SESSION_ID SessionID) {

}
void CChatServer::OnError(int errorcode, const WCHAR *log) {

}

void CChatServer::OnTimeout(SESSION_ID SessionID) {
	/*CLogger::_Log(dfLOG_LEVEL_DEBUG, L"ERROR :: Time Out Case SessionID : %I64u", SessionID);
	JobMessage *job = _jobMsgPool.Alloc();
	job->_SessionID = SessionID;
	job->_Type = CHAT_PACKET_TYPE::ON_TIME_OUT;
	job->_pPacket = nullptr;

	_jobQueue.Enqueue(job);
	SetEvent(_DequeueEvent);*/
}

void CChatServer::PacketProc(Packet *pPacket, SESSION_ID SessionID, WORD type) {
	if (pPacket != nullptr)
		pPacket->AddRef(3);


	WCHAR errmsg[512];
	CLogger::_Log(dfLOG_LEVEL_DEBUG, L"TYPE : %d", type);
	switch (type) {
	case CHAT_PACKET_TYPE::PACKET_CS_CHAT_REQ_LOGIN:
		InterlockedIncrement(&_LoginCalc);
		PacketProcRequestLogin(pPacket, SessionID);
		break;
	case CHAT_PACKET_TYPE::PACKET_CS_CHAT_REQ_SECTOR_MOVE:
		InterlockedIncrement(&_SectorMoveCalc);
		PacketProcMoveSector(pPacket, SessionID);
		break;
	case CHAT_PACKET_TYPE::PACKET_CS_CHAT_REQ_MESSAGE:
		InterlockedIncrement(&_ChatRecvCalc);
		PacketProcChatRequire(pPacket, SessionID);
		break;
	case CHAT_PACKET_TYPE::ON_CLIENT_LEAVE:
		InterlockedIncrement(&_LoginCalc);
		RemovePlayer(SessionID);
		break;

	case CHAT_PACKET_TYPE::ON_TIME_OUT:
		wsprintf(errmsg, L"ERROR :: Time Out Case SessionID : %I64u", SessionID);
		CLogger::_Log(dfLOG_LEVEL_ERROR, errmsg);
		DisconnectSession(SessionID);
		break;

	default:
		wsprintf(errmsg, L"ERROR :: Session Default Case SessionID : %I64u", SessionID);
		CLogger::_Log(dfLOG_LEVEL_ERROR, errmsg);
		DisconnectSession(SessionID);
		break;
	}
	if (pPacket != nullptr)
		pPacket->SubRef(3);
}

// PACKET_CS_CHAT_REQ_LOGIN
void CChatServer::PacketProcRequestLogin(Packet *pPacket, SESSION_ID SessionID) {
	PRO_BEGIN(L"RequestLogin");
	pPacket->AddRef(4);
	BYTE status = FALSE;
	ACCOUNT_NO acno;

	if (pPacket->GetDataSize() < sizeof(ACCOUNT_NO)) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"pPacket->GetDataSize() < sizeof(ACCOUNT_NO)"); // TODO ERROR MSG
		DisconnectSession(SessionID);
	}
	(*pPacket).GetData((char *) &acno, sizeof(ACCOUNT_NO));

	if (pPacket->GetDataSize() != ID_MAX_SIZE + NICK_NAME_MAX_SIZE + TOKEN_KEY_SIZE) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"pPacket->GetDataSize() != ID_MAX_SIZE + NICK_NAME_MAX_SIZE + TOKEN_KEY_SIZE"); // TODO ERROR MSG
		DisconnectSession(SessionID);
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

	pPacket->SubRef(4);

	Packet *pResLoginPacket = Packet::AllocAddRef();
	MakePacketResponseLogin(pResLoginPacket, pPlayer->_AccountNo, status);
	SendPacket(pPlayer->_SessionID, pResLoginPacket);
	if (status == FALSE) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"status == FALSE"); // TODO ERROR MSG
		DisconnectSession(SessionID);
	}

	InterlockedIncrement(&_LoginCalc);
	CLogger::_Log(dfLOG_LEVEL_DEBUG, L"Login OK ANO[%d]", acno); // TODO ERROR MSG
	pResLoginPacket->SubRef();
	PRO_END(L"RequestLogin");
}

// PACKET_CS_CHAT_REQ_SECTOR_MOVE
void CChatServer::PacketProcMoveSector(Packet *pPacket, SESSION_ID SessionID) {
	PRO_BEGIN(L"MoveSector");
	ACCOUNT_NO no;
	WORD sx;
	WORD sy;

	// 패킷 꺼내기
	pPacket->AddRef(5);
	if (pPacket->GetDataSize() != (sizeof(no) + sizeof(sx) + sizeof(sy))) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"pPacket->GetDataSize() != (sizeof(no) + sizeof(sx) + sizeof(sy))"); // TODO ERROR MSG
		DisconnectSession(SessionID);
		pPacket->SubRef(5);
		return;
	}
	pPacket->GetData((char *) &no, sizeof(ACCOUNT_NO));
	(*pPacket) >> sx >> sy;
	pPacket->SubRef(6);
	// 섹터 범위 초과
	if (sx >= SECTOR_X_SIZE || sy >= SECTOR_Y_SIZE) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"PacketProcMoveSector(id [%I64u])  sx >= SECTOR_X_SIZE || sy >= SECTOR_Y_SIZE", SessionID); // TODO ERROR MSG
		DisconnectSession(SessionID);
		return;
	}


	// 플레이어 무결성
	Player *pPlayer = FindPlayer(SessionID);
	if (pPlayer == nullptr) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"PacketProcMoveSector(id [%I64u])  pPlayer == nullptr", SessionID); // TODO ERROR MSG
		DisconnectSession(SessionID);
		return;
	}if (pPlayer->_isLogin == false) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"PacketProcMoveSector(id [%I64u])  pPlayer->_isLogin == false", SessionID); // TODO ERROR MSG
		DisconnectSession(SessionID);
		return;
	}if (pPlayer->_AccountNo != no) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"PacketProcMoveSector(id [%I64u])  _AccountNo == no", SessionID); // TODO ERROR MSG
		DisconnectSession(SessionID);
		return;
	}



	// 기존에 있던 섹터가 있는지
	constexpr WORD comp = -1;
	if (pPlayer->_SectorX != comp && pPlayer->_SectorY != comp) {
		// 기존섹터 삭제
		__SECTOR_LOCK(pPlayer->_SectorX, pPlayer->_SectorY);
		{
			auto iter = _sector[pPlayer->_SectorY][pPlayer->_SectorX]._playerSet.find(pPlayer);
			if (iter != _sector[pPlayer->_SectorY][pPlayer->_SectorX]._playerSet.end()) {
				_sector[pPlayer->_SectorY][pPlayer->_SectorX]._playerSet.erase(iter);
			}
		}
		__SECTOR_UNLOCK(pPlayer->_SectorX, pPlayer->_SectorY);
	}
	// 이동
	pPlayer->_SectorX = sx;
	pPlayer->_SectorY = sy;
	__SECTOR_LOCK(pPlayer->_SectorX, pPlayer->_SectorY);
	{
		_sector[pPlayer->_SectorY][pPlayer->_SectorX]._playerSet.emplace(pPlayer);
	}
	__SECTOR_UNLOCK(pPlayer->_SectorX, pPlayer->_SectorY);

	// Send RES_SECTOR_MOVE Msg
	Packet *pResPacket = Packet::AllocAddRef();
	MakePacketResponseSectorMove(pResPacket, pPlayer->_AccountNo, pPlayer->_SectorX, pPlayer->_SectorY);
	SendPacket(pPlayer->_SessionID, pResPacket);
	pResPacket->SubRef(7);
	PRO_END(L"MoveSector");
}

// PACKET_CS_CHAT_REQ_MESSAGE
void CChatServer::PacketProcChatRequire(Packet *pPacket, SESSION_ID SessionID) {
	PRO_BEGIN(L"ChatRequire");
	ACCOUNT_NO no;
	WORD msgLen;
	WCHAR message[MASSAGE_MAX_SIZE];
	pPacket->AddRef(6);

	if (pPacket->GetDataSize() < sizeof(ACCOUNT_NO) + sizeof(msgLen)) {
		DisconnectSession(SessionID);
		pPacket->SubRef(8);
		return;
	}
	pPacket->GetData((char *) &no, sizeof(ACCOUNT_NO));
	(*pPacket) >> msgLen;

	if (pPacket->GetDataSize() != msgLen) {
		DisconnectSession(SessionID);
		pPacket->SubRef(9);
		return;
	}
	pPacket->GetData((char *) message, msgLen);
	message[msgLen / 2] = '\0';




	pPacket->SubRef(10);
	Player *pSender = FindPlayer(SessionID);
	if (pSender == nullptr) {
		//TODO ERROR
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"pSender == nullptr");
		DisconnectSession(SessionID);
		return;
	}
	if (pSender->_isLogin == false) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"pSender->_isLogin == false");
		DisconnectSession(SessionID);
		return;
	}
	if (no != pSender->_AccountNo) {
		//TODO ERROR
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"no != pSender->_AccountNo");
		DisconnectSession(SessionID);
		return;
	}
	if (pSender->_SectorX >= 50 || pSender->_SectorY >= 50) {
		//TODO ERROR
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"pSender->_SectorX >= 50 || pSender->_SectorY >= 50");
		DisconnectSession(SessionID);
		return;
	}
	PRO_END(L"ChatRequire");
	Packet *pResPacket = Packet::AllocAddRef();


	MakePacketResponseMessage(pResPacket, pSender->_AccountNo, pSender->_ID, pSender->_NickName, msgLen, message);
	//BroadcastSector(pResPacket, pSender->_SectorX, pSender->_SectorY, nullptr);
	BroadcastSectorAround(pResPacket, pSender->_SectorX, pSender->_SectorY, nullptr);

	pResPacket->SubRef(11);
}


// PACKET_SC_CHAT_RES_LOGIN
void CChatServer::MakePacketResponseLogin(Packet *pPacket, ACCOUNT_NO account_no, BYTE status) {
	CHAT_PACKET_TYPE type = CHAT_PACKET_TYPE::PACKET_SC_CHAT_RES_LOGIN;
	(*pPacket) << ((WORD) type) << status << (__int64) account_no;
}

//PACKET_SC_CHAT_RES_SECTOR_MOVE
void CChatServer::MakePacketResponseSectorMove(Packet *pPacket, ACCOUNT_NO account_no, WORD sectorX, WORD sectorY) {
	CHAT_PACKET_TYPE type = CHAT_PACKET_TYPE::PACKET_SC_CHAT_RES_SECTOR_MOVE;
	(*pPacket) << ((WORD) type);
	pPacket->PutData((char *) &account_no, sizeof(ACCOUNT_NO));
	(*pPacket) << sectorX << sectorY;

}

//PACKET_SC_CHAT_RES_MESSAGE
void CChatServer::MakePacketResponseMessage(Packet *pPacket, ACCOUNT_NO account_no, const WCHAR *ID, const WCHAR *nickName, WORD msgLen, const WCHAR *message) {
	CHAT_PACKET_TYPE type = CHAT_PACKET_TYPE::PACKET_SC_CHAT_RES_MESSAGE;
	(*pPacket) << ((WORD) type);
	pPacket->PutData((char *) &account_no, sizeof(ACCOUNT_NO));
	pPacket->PutData((char *) ID, ID_MAX_SIZE);
	pPacket->PutData((char *) nickName, NICK_NAME_MAX_SIZE);
	(*pPacket) << msgLen;
	pPacket->PutData((char *) message, msgLen);

}

void CChatServer::BroadcastSector(Packet *pPacket, WORD sectorX, WORD sectorY, Player *exPlayer = nullptr) {
	pPacket->AddRef(15);

	__SECTOR_LOCK(sectorX, sectorY);
	{
		SECTOR *pSector = &_sector[sectorY][sectorX];
		for (auto iter = pSector->_playerSet.begin(); iter != pSector->_playerSet.end(); ++iter) {
			Player *pPlayer = (*iter);
			if (pPlayer == exPlayer) continue;
			SendPacket(pPlayer->_SessionID, pPacket);
			InterlockedIncrement(&_ChatSendCalc);
			InterlockedIncrement64(&_totalSectorAroundSend);
		}
	}
	__SECTOR_UNLOCK(sectorX, sectorY);

	InterlockedIncrement64(&_SectorAroundCount);

	pPacket->SubRef(15);
}

void CChatServer::BroadcastSectorAround(Packet *pPacket, WORD sectorX, WORD sectorY, Player *exPlayer = nullptr) {
	/*int beginX = sectorX - 1;
	int beginY = sectorY - 1;

	pPacket->AddRef();

	for (int dy = 0; dy < 3; dy++) {
		if (((beginY + dy) < 0) || (beginY + dy) >= 50) continue;
		for (int dx = 0; dx < 3; dx++) {
			if (((beginX + dx) < 0) || ((beginX + dx) >= 50)) continue;
			SECTOR *pSector = &_sector[beginY + dy][beginX + dx];
			for (auto iter = pSector->_playerSet.begin(); iter != pSector->_playerSet.end(); ++iter) {
				Player *pPlayer = (*iter);
				if (pPlayer == exPlayer) continue;
				SendPacket(pPlayer->_SessionID, pPacket);
				InterlockedIncrement(&_ChatSendCalc);
				InterlockedIncrement64(&_totalSectorAroundSend);
			}
		}

	}
	pPacket->SubRef();
	InterlockedIncrement64(&_SectorAroundCount);*/



	WORD sx = sectorX <= 0 ? 0 : sectorX - 1;
	WORD sy = sectorY <= 0 ? 0 : sectorY - 1;
	WORD ex = sectorX >= SECTOR_X_SIZE - 1 ? SECTOR_X_SIZE - 1 : sectorX + 1;
	WORD ey = sectorY >= SECTOR_Y_SIZE - 1 ? SECTOR_Y_SIZE - 1 : sectorY + 1;
	pPacket->AddRef(16);

	/*for (WORD dy = sy; dy <= ey; ++dy) {
		for (WORD dx = sx; dx <= ex; ++dx) {
			__SECTOR_LOCK(dx, dy);
		}
	}*/

	{
		for (WORD dy = sy; dy <= ey; ++dy) {
			for (WORD dx = sx; dx <= ex; ++dx) {
				PRO_BEGIN(L"ChatBroadcast");

				//SECTOR *pSector = &_sector[dy][dx];
				//for (auto iter = pSector->_playerSet.begin(); iter != pSector->_playerSet.end(); ++iter) {
				::unordered_set<Player *> *pSectorPlayerSet = &_sector[dy][dx]._playerSet;
				for (auto iter = pSectorPlayerSet->begin(); iter != pSectorPlayerSet->end(); ++iter) {
					Player *pPlayer = (*iter);
					if (pPlayer == exPlayer) continue;
					SendPacket(pPlayer->_SessionID, pPacket);
					InterlockedIncrement(&_ChatSendCalc);
					InterlockedIncrement64(&_totalSectorAroundSend);
				}

				PRO_END(L"ChatBroadcast");
			}
		}
		InterlockedIncrement64(&_SectorAroundCount);
	}

	/*for (WORD dy = sy; dy <= ey; ++dy) {
		for (WORD dx = sx; dx <= ex; ++dx) {
			__SECTOR_UNLOCK(dx, dy);
		}
	}*/

	pPacket->SubRef(16);
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

	MoniteringInfo monitor = GetMoniteringInfo();
	fwprintf_s(fp, L"=============================INFO=====================================\n");
	fwprintf_s(fp, L"Start Time [%02d/%02d/%02d %02d:%02d:%02d]\n",
		_timeFormet.tm_mon + 1, _timeFormet.tm_mday, (_timeFormet.tm_year + 1900) % 100, _timeFormet.tm_hour, _timeFormet.tm_min, _timeFormet.tm_sec);
	fwprintf_s(fp, L"Worker Thread Count[%d]\t\tRunning Thread Count[%d]\n",
		monitor._workerThreadCount, monitor._runningThreadCount);
	fwprintf_s(fp, L"Packet Queue Size [%lld]\t Packet Queue AVG[%lld]\n",
		monitor._queueSize, monitor._queueSizeAvg);
	fwprintf_s(fp, L"Currnt Session Count[%lld]\n",
		monitor._sessionCnt);
	fwprintf_s(fp, L"\n\
-----------------------------TPS--------------------------------------\n\
Accept TPS\t[%lld]\n\
send packet TPS\t[%lld]\trecv packet TPS\t[%lld]\n\
seded packet TPS[%lld]\n\
Update TPS\t[%d]\n\
login TPS\t[%d]\tsector move TPS\t[%d]\n\
chat recv TPS\t[%d]\tchat send TPS\t[%d]\n\
",
monitor._acceptPerSec, monitor._sendPacketPerSec, monitor._recvPacketPerSec, monitor._sendedPacketPerSec, _UpdateTPS, _LoginTPS, _SectorMoveTPS, _ChatRecvTPS, _ChatSendTPS);

	fwprintf_s(fp, L"\n\
----------------------------TOTAL-------------------------------------\n\
packet\t\t[%lld]\tsended Byte\t[%lld]\n\
accept Count\t[%lld]\tDisconnectSession Count[%lld]\n\
SEND SECTOR AVG\t[%lld]\t\n",
monitor._totalPacket, monitor._totalProecessedBytes, monitor._totalAcceptSession, monitor._totalReleaseSession,
_SectorAroundCount == 0 ? 0 : _totalSectorAroundSend / _SectorAroundCount);
	fwprintf_s(fp, L"\n\
----------------------------MEMORY------------------------------------ \n\
Available [%lluMb]\tNPPool\ [%lluMb] Private Mem\t[%lluKb]\n",
_hardMoniter.AvailableMemoryMBytes(), _hardMoniter.NonPagedPoolMBytes(), _procMonitor.PrivateMemoryKBytes());
	fwprintf_s(fp, L"\
Packet pool Capacity\t[%d]\tPacket pool size\t[%d]\n\
JobMsgPool Capacity\t[%d]\tJobMsgPool size\t\t[%d]\n\
Player Pool Capacity\t[%d]\tPlayer Pool size\t[%d]\n\
JobQueue Capacity\t[%d]\tJobQueue Pool size\t[%d]\n\
player map size\t\t[%lld]\tTotal Sector Container Size[%lld]\n",
Packet::_packetPool.GetCapacity(), Packet::_packetPool.GetSize(),
_jobMsgPool.GetCapacity(), _jobMsgPool.GetSize(),
_playerPool.GetCapacity(), _playerPool.GetSize(),
_jobQueue.GetPoolCapacity(), _jobQueue.GetPoolSize(),
_playerMap.size(), _TotalSectorSize);
	fwprintf_s(fp, L"\n\
---------------------------CORE USAGE--------------------------------- \n\
PROCESS\t[T %.1llf%% K %.1llf%% U %.1llf%%]\tCPU\t[T %.1llf%% K %.1llf%% U %.1llf%%]\n",
_procMonitor.ProcessTotal(), _procMonitor.ProcessKernel(), _procMonitor.ProcessUser(),
_hardMoniter.ProcessorTotal(), _hardMoniter.ProcessorKernel(), _hardMoniter.ProcessorUser());
}

void CChatServer::PrintFileMonitor() {
	WCHAR FILENAME[128] = L"";
	// timestemp
	tm t;
	time_t now;
	// timestemp
	time(&now);
	localtime_s(&t, &now);
#ifdef dfPROFILER
	// PROFILE
	swprintf_s(FILENAME, 128, L"ServerLog/Profile/%02d%02d%02d_%02d%02d%02d_CHAT_PROFILE.log",
		t.tm_mon + 1, t.tm_mday, (t.tm_year + 1900) % 100,
		t.tm_hour, t.tm_min, t.tm_sec);
	PRO_PRINT(FILENAME);
#endif // dfPROFILER

	swprintf_s(FILENAME, 128, L"ServerLog/MonitorLog/%02d%02d%02d_%02d%02d%02d_CHAT_SERVER_MONITOR.log",
		t.tm_mon + 1, t.tm_mday, (t.tm_year + 1900) % 100,
		t.tm_hour, t.tm_min, t.tm_sec);
	FILE *fp = stdout;
	_wfopen_s(&fp, FILENAME, L"a+");
	fseek(fp, 0, SEEK_END);
	PrintMonitor(fp);
	fclose(fp);
}
