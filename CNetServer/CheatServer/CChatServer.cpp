#include "pch.h"
#include "CChatServer.h"
#include "ChatServerProtocol.h"
#include "SS_MoniteringProtocol.h"
#include "Profiler.h"
#include <conio.h>
#include <time.h>
#include <stdio.h>
#ifndef UPDATE_THREAD
#define __PLAYER_MAP_LOCK()		this->PlayerMapLock()
#define __PLAYER_MAP_UNLOCK()	this->PlayerMapUnlock()
#define __SECTOR_LOCK(x, y)		this->SectorLock(x, y)
#define __SECTOR_UNLOCK(x, y)	this->SectorUnlock(x, y)

#else
#define __PLAYER_MAP_LOCK()		
#define __PLAYER_MAP_UNLOCK()	
#define __SECTOR_LOCK(x, y)		
#define __SECTOR_UNLOCK(x, y)	
#endif // !UPDATE_THREAD

CChatServer::CChatServer() : CServer(ENCRYPTED_PACKET), _startTime{ 0 }, _timeFormet{ 0 } {
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
	_LeaveCalc = 0;
	_LeaveTPS = 0;
	_UpdateCalc = 0;
	_UpdateTPS = 0;


	_totalSectorAroundSend = 0;
	_SectorAroundCount = 0;

	_curLogTimer = _preLogTimer = timeGetTime();

	SetTimeoutTime(40000);

	InitializeSRWLock(&_playerMapLock);

}

CChatServer::~CChatServer() {
}


void CChatServer::BeginServer(const WCHAR *szConfigFile) {
	if (_isRunning == true) return;
#ifdef UPDATE_THREAD
	_DequeueEvent = nullptr;
	_DequeueEvent = CreateEvent(nullptr, false, false, nullptr);
	if (_DequeueEvent == nullptr) {
		_LOG(dfLOG_LEVEL_ERROR, L"_DequeueEvent == nullptr");
		return;
	}
#endif // UPDATE_THREAD
	int port = 0;
	int wThreadCount = 0;
	int rThreadCount = 0;
	bool isNagle = false;
	int maxConnetion = 0;

	_pConfigData = new CParser(szConfigFile);
	// Server lib Config
	_pConfigData->SetNamespace(L"ServerLibConfig");
	_pConfigData->TryGetValue(L"ServerPort", port);
	_pConfigData->TryGetValue(L"WorkerThreadCount", wThreadCount);
	_pConfigData->TryGetValue(L"MaxRunningThreadCount", rThreadCount);
	_pConfigData->TryGetValue(L"isNagle", isNagle);
	_pConfigData->TryGetValue(L"MaxConnectionCount", maxConnetion);

	// Monitor Server Connect Config
	_pConfigData->SetNamespace(L"NetServerConfig");
	_pConfigData->TryGetValue(L"MonitorServerIP", _monitorServerIP);
	_pConfigData->TryGetValue(L"MonitorServerPort", _monitorServerPort);

	// Connect Monitor Servoer
	ASSERT_CRASH(_monitorServerConnection.ConnectMonitorServer(_monitorServerIP, _monitorServerPort, SERVER_TYPE::CHAT_SERVER));
	//_monitorServerConnection.ConnectMonitorServer(_monitorServerIP, _monitorServerPort, SERVER_TYPE::CHAT_SERVER);
	_LOG(dfLOG_LEVEL_NOTICE, L"ConnectMonitorServer");

	// Start Chat Server
	_isRunning = Start(INADDR_ANY, port, wThreadCount, rThreadCount, isNagle, maxConnetion);

	// Set StartTime
	time(&_startTime);
	localtime_s(&_timeFormet, &_startTime);
	BeginThread();

}


void CChatServer::CloseServer() {
	_isRunning = false;
#ifdef UPDATE_THREAD
	SetEvent(_DequeueEvent);
	CloseHandle(_DequeueEvent);
#endif // UPDATE_THREAD
	_LOG(dfLOG_LEVEL_NOTICE, L"CHAT SERVER CLOSE");
	CServer::Quit();
	for (int i = 0; i < SECTOR_Y_SIZE; i++)
		delete[] _sector[i];
	delete[] _sector;
}


void CChatServer::CommandWait() {
	int printTick = 0;

	for (;;) {
		char cmd = _getch();
		if (cmd == 'Q' || cmd == 'q') {
			//MemProfiler::Instance().PrintInfo();
			_LOG(dfLOG_LEVEL_NOTICE, L"Close Server from Cmd");
			CloseServer();
			break;
		}
		if (cmd == 'C' || cmd == 'c') {
			_LOG(dfLOG_LEVEL_NOTICE, L"Crash Server from Cmd");
			CRASH();
		}
		if (cmd == '1') {
			wprintf_s(L"CHANGE LOG LEVEL :: DEBUG\n");
			_SET_LOG_LEVEL(dfLOG_LEVEL_DEBUG);
		}
		if (cmd == '2') {
			wprintf_s(L"CHANGE LOG LEVEL :: ERROR\n");
			_SET_LOG_LEVEL(dfLOG_LEVEL_ERROR);
		}
		if (cmd == '3') {
			wprintf_s(L"CHANGE LOG LEVEL :: NOTICE\n");
			_SET_LOG_LEVEL(dfLOG_LEVEL_NOTICE);
		}
	}
}


bool CChatServer::UpdateProc() {
#ifdef UPDATE_THREAD


	while (_isRunning) {
		//if (_jobQueue.IsEmpty()) return _isRunning;
		WaitForSingleObject(_DequeueEvent, INFINITE);

		JobMessage *job;

		while (_jobQueue.dequeue(job)) {
			InterlockedIncrement(&_UpdateCalc);
			PacketProc(job->_pPacket, job->_sessionID, job->_Type);
			if (job->_pPacket != nullptr)
				job->_pPacket->SubRef(1);

			_jobMsgPool.Free(job);
		}
	}
	return _isRunning;
#else
	return false;
#endif // UPDATE_THREAD
}

void CChatServer::BeginThread() {
#ifdef UPDATE_THREAD
	// Update (jobQ Thread)
	_updateThread.SetThreadName(L"CHAT SERVER Update Thread");
	_updateThread.Launch(
		[](LPVOID arg) {
			CChatServer *pServer = (CChatServer *) arg;
			while (pServer->UpdateProc());
		}
	, this);
#endif
}

bool CChatServer::OnConnectionRequest(WCHAR *IPStr, DWORD IP, USHORT Port) {
	return _isRunning;
}

void CChatServer::OnClientJoin(WCHAR *ipStr, DWORD ip, USHORT port, SESSION_ID sessionID) {
}

void CChatServer::OnClientLeave(SESSION_ID sessionID) {
	_LOG(dfLOG_LEVEL_DEBUG, L"ERROR :: Time Out Case sessionID : %I64u", sessionID);
#ifdef UPDATE_THREAD
	JobMessage *job = _jobMsgPool.Alloc();
	job->_sessionID = sessionID;
	job->_Type = CHAT_PACKET_TYPE::ON_CLIENT_LEAVE;
	job->_pPacket = nullptr;

	_jobQueue.enqueue(job);
	SetEvent(_DequeueEvent);
#else
	PacketProc(nullptr, sessionID, CHAT_PACKET_TYPE::ON_CLIENT_LEAVE);
#endif // UPDATE_THREAD
}


void CChatServer::OnRecv(SESSION_ID sessionID, Packet *pPacket) {
	pPacket->AddRef(2);
	WORD type;
	(*pPacket) >> type;

#ifdef UPDATE_THREAD
	// jobQueue
	JobMessage *job = _jobMsgPool.Alloc();
	job->_sessionID = sessionID;
	job->_Type = type;
	job->_pPacket = pPacket;
	pPacket->AddRef();

	_jobQueue.enqueue(job);
	SetEvent(_DequeueEvent);
#else
	PacketProc(pPacket, sessionID, type);
#endif // UPDATE_THREAD

	pPacket->SubRef(2);
}
void CChatServer::OnSend(SESSION_ID sessionID) {

}
void CChatServer::OnError(int errorcode, const WCHAR *log) {

}

void CChatServer::OnTimeout(SESSION_ID sessionID) {
	_LOG(dfLOG_LEVEL_DEBUG, L"ERROR :: Time Out Case sessionID : %I64u", sessionID);
#ifdef UPDATE_THREAD
	JobMessage *job = _jobMsgPool.Alloc();
	job->_sessionID = sessionID;
	job->_Type = CHAT_PACKET_TYPE::ON_TIME_OUT;
	job->_pPacket = nullptr;

	//_jobQueue.enqueue(job);
	//SetEvent(_DequeueEvent);
#else
	//PacketProc(nullptr,sessionID, CHAT_PACKET_TYPE::ON_TIME_OUT);
#endif // UPDATE_THREAD
}

void CChatServer::OnMonitoringPerSec() {
	FILE *fp = stdout;

	_SectorMoveTPS = InterlockedExchange(&_SectorMoveCalc, 0);
	_ChatRecvTPS = InterlockedExchange(&_ChatRecvCalc, 0);
	_ChatSendTPS = InterlockedExchange(&_ChatSendCalc, 0);
	_LoginTPS = InterlockedExchange(&_LoginCalc, 0);
	_LeaveTPS = InterlockedExchange(&_LeaveCalc, 0);
	_UpdateTPS = InterlockedExchange(&_UpdateCalc, 0);
	_hardMoniter.UpdateHardWareTime();
	_procMonitor.UpdateProcessTime();

	// Print Monitor
	_curLogTimer = timeGetTime();
	if (_curLogTimer - _preLogTimer >= 30000) {
		_preLogTimer = _curLogTimer;
		WCHAR FILENAME[128] = L"";
		// timestemp
		tm t;
		time_t now;
		// timestemp
		time(&now);
		localtime_s(&t, &now);
#ifdef dfPROFILER
		// PROFILE
		swprintf_s(FILENAME, 128, L"Log/Profile/%02d%02d%02d_%02d%02d%02d_CHAT_PROFILE.log",
			t.tm_mon + 1, t.tm_mday, (t.tm_year + 1900) % 100,
			t.tm_hour, t.tm_min, t.tm_sec);
		PRO_PRINT(FILENAME);
#endif // dfPROFILER

		swprintf_s(FILENAME, 128, L"Log/MonitorLog/%02d%02d%02d_%02d%02d%02d_CHAT_SERVER_MONITOR.log",
			t.tm_mon + 1, t.tm_mday, (t.tm_year + 1900) % 100,
			t.tm_hour, t.tm_min, t.tm_sec);
		_wfopen_s(&fp, FILENAME, L"a+");
		fseek(fp, 0, SEEK_END);
	}
	{
		tm curTimeFormat;
		time_t curTime;
		time(&curTime);
		localtime_s(&curTimeFormat, &curTime);

		MoniteringInfo monitor = GetMoniteringInfo();
		fwprintf_s(fp, L"=============================INFO=====================================\n");
		fwprintf_s(fp, L"Start Time [%02d/%02d/%02d %02d:%02d:%02d] System Time [%02d/%02d/%02d %02d:%02d:%02d]\n",
			_timeFormet.tm_mon + 1, _timeFormet.tm_mday, (_timeFormet.tm_year + 1900) % 100, _timeFormet.tm_hour, _timeFormet.tm_min, _timeFormet.tm_sec,
			curTimeFormat.tm_mon + 1, curTimeFormat.tm_mday, (curTimeFormat.tm_year + 1900) % 100, curTimeFormat.tm_hour, curTimeFormat.tm_min, curTimeFormat.tm_sec);
#ifdef df_SENDTHREAD
		fwprintf_s(fp, L"Send Thread::[TRUE]\t");
#else
		fwprintf_s(fp, L"Send Thread::[FALSE]\t");
#endif // sendthread

#ifdef UPDATE_THREAD
		fwprintf_s(fp, L"Update Thread::[TRUE]\n");
#else
		fwprintf_s(fp, L"Update Thread::[FALSE]\n");
#endif // UPDATE_THREAD

		fwprintf_s(fp, L"Worker Thread Count[%d]\tRunning Thread Count[%d]\n",
			monitor._workerThreadCount, monitor._runningThreadCount);
		fwprintf_s(fp, L"Currnt Session Count[%lld]\n",
			monitor._sessionCnt);
		fwprintf_s(fp, L"\n\
--------------------------------TPS-----------------------------------------\n\
Accept TPS\t[%lld]\tsend packet TPS\t[%lld]\trecv packet TPS\t[%lld]\n\
seded packet TPS[%lld]\trecved packet TPS[%lld]\n",
monitor._acceptPerSec, monitor._sendPacketPerSec, monitor._recvPacketPerSec,
monitor._sendedPacketPerSec, monitor._recvPacketPerSec);
#ifdef UPDATE_THREAD
		fwprintf_s(fp, L"\n\
Update TPS\t[%d]\tlogin TPS\t[%d]\tLeave TPS\t[%d]\n\
sector move TPS\t[%d]\tchat recv TPS\t[%d]\tchat send TPS\t[%d]\n\
", _UpdateTPS, _LoginTPS, _LeaveTPS, _SectorMoveTPS, _ChatRecvTPS, _ChatSendTPS);
#else
		fwprintf_s(fp, L"\n\
login TPS\t[%d]\tLeave TPS\t[%d]\n\
sector move TPS\t[%d]\tchat recv TPS\t[%d]\tchat send TPS\t[%d]\n\
", _LoginTPS, _LeaveTPS, _SectorMoveTPS, _ChatRecvTPS, _ChatSendTPS);
#endif
		fwprintf_s(fp, L"\n\
-------------------------------TOTAL----------------------------------------\n\
packet\t\t[%lld]\tsended Byte\t[%lld]\n\
accept Count\t[%lld]\tDisconnectSession Count[%lld]\n\
SEND SECTOR AVG\t[%lld]\t\n",
monitor._totalPacket, monitor._totalProecessedBytes, monitor._totalAcceptSession, monitor._totalReleaseSession,
_SectorAroundCount == 0 ? 0 : _totalSectorAroundSend / _SectorAroundCount);
		fwprintf_s(fp, L"\n\
-------------------------------MEMORY--------------------------------------- \n\
Available [%lluMb]\tNPPool\ [%lluMb] Private Mem\t[%lluKb]\n",
_hardMoniter.AvailableMemoryMBytes(), _hardMoniter.NonPagedPoolMBytes(), _procMonitor.PrivateMemoryKBytes());

		fwprintf_s(fp, L"Session Packet Queue Size [%lld]\tQueue AVG[%lld]\n",
			monitor._queueSize, monitor._queueSizeAvg);
		int packetPoolCapacity = Packet::_packetPool.GetCapacity();
		int packetPoolSize = Packet::_packetPool.GetSize();
		int playerMapSize = _playerMap.size();
#ifdef UPDATE_THREAD
		int jobPoolSize = _jobMsgPool.GetSize();
		fwprintf_s(fp, L"\
Packet pool Capacity\t[%d]\tsize\t[%d]\n\
JobMsgPool Capacity\t[%d]\tsize\t[%d]\n\
Player Pool Capacity\t[%d]\tsize\t[%d]\n\
JobQueue Capacity\t[%d]\tsize\t[%d]\n\
player map size\t\t[%lld]\n",
packetPoolCapacity, packetPoolSize,
_jobMsgPool.GetCapacity(), _jobMsgPool.GetSize(),
_playerPool.GetCapacity(), _playerPool.GetSize(),
_jobQueue.GetPoolCapacity(), jobPoolSize,
playerMapSize);
#else
		
		fwprintf_s(fp, L"\
Packet pool Capacity\t[%d]\tsize\t[%d]\n\
Player Pool Capacity\t[%d]\tsize\t[%d]\n\
player map size\t\t[%lld]\n",
packetPoolCapacity, packetPoolSize,
_playerPool.GetCapacity(), _playerPool.GetSize(),
playerMapSize);
#endif // UPDATE_THREAD
		fwprintf_s(fp, L"\n\
------------------------------CORE USAGE------------------------------------ \n\
PROCESS\t[T %.1llf%% K %.1llf%% U %.1llf%%]\tCPU\t[T %.1llf%% K %.1llf%% U %.1llf%%]\n",
_procMonitor.ProcessTotal(), _procMonitor.ProcessKernel(), _procMonitor.ProcessUser(),
_hardMoniter.ProcessorTotal(), _hardMoniter.ProcessorKernel(), _hardMoniter.ProcessorUser());
		if (fp != stdout)
			fclose(fp);


		_monitorServerConnection.SendMonitorPacket(SERVER_TYPE::CHAT_SERVER, CHAT_SERVER_MONITORING_TYPE::CHAT_SERVER_ON_OFF, _isRunning, _curLogTimer);
		_monitorServerConnection.SendMonitorPacket(SERVER_TYPE::CHAT_SERVER, CHAT_SERVER_MONITORING_TYPE::CHAT_SERVER_CPU_USAGE, _hardMoniter.ProcessorTotal(), _curLogTimer);
		_monitorServerConnection.SendMonitorPacket(SERVER_TYPE::CHAT_SERVER, CHAT_SERVER_MONITORING_TYPE::CHAT_SERVER_RECEIVE_PACKET_COUNT, monitor._recvPacketPerSec, _curLogTimer);
		_monitorServerConnection.SendMonitorPacket(SERVER_TYPE::CHAT_SERVER, CHAT_SERVER_MONITORING_TYPE::CHAT_SERVER_SEND_PACKET_COUNT, monitor._sendPacketPerSec, _curLogTimer);
		_monitorServerConnection.SendMonitorPacket(SERVER_TYPE::CHAT_SERVER, CHAT_SERVER_MONITORING_TYPE::CHAT_SERVER_SESSION_COUNTS, _curSessionCount, _curLogTimer);
		_monitorServerConnection.SendMonitorPacket(SERVER_TYPE::CHAT_SERVER, CHAT_SERVER_MONITORING_TYPE::CHAT_SERVER_PLAYER_COUNTS, playerMapSize, _curLogTimer);
		_monitorServerConnection.SendMonitorPacket(SERVER_TYPE::CHAT_SERVER, CHAT_SERVER_MONITORING_TYPE::CHAT_SERVER_PACKET_POOL_USAGE, packetPoolSize, _curLogTimer);
#ifdef UPDATE_THREAD
		_monitorServerConnection.SendMonitorPacket(SERVER_TYPE::CHAT_SERVER, CHAT_SERVER_MONITORING_TYPE::CHAT_SERVER_UPDATE_TPS, _UpdateTPS, _curLogTimer);
		_monitorServerConnection.SendMonitorPacket(SERVER_TYPE::CHAT_SERVER, CHAT_SERVER_MONITORING_TYPE::CHAT_SERVER_UPDATE_MSG_QUEUE_SIZE, jobPoolSize, _curLogTimer);
#endif // UPDATE_THREAD

	}





}

void CChatServer::PacketProc(Packet *pPacket, SESSION_ID sessionID, WORD type) {
	if (pPacket != nullptr)
		pPacket->AddRef(3);


	WCHAR errmsg[512];
	_LOG(dfLOG_LEVEL_DEBUG, L"TYPE : %d", type);
	switch (type) {
	case CHAT_PACKET_TYPE::PACKET_CS_CHAT_REQ_LOGIN:
		InterlockedIncrement(&_LoginCalc);
		PacketProcRequestLogin(pPacket, sessionID);
		break;
	case CHAT_PACKET_TYPE::PACKET_CS_CHAT_REQ_SECTOR_MOVE:
		InterlockedIncrement(&_SectorMoveCalc);
		PacketProcMoveSector(pPacket, sessionID);
		break;
	case CHAT_PACKET_TYPE::PACKET_CS_CHAT_REQ_MESSAGE:
		InterlockedIncrement(&_ChatRecvCalc);
		PacketProcChatRequire(pPacket, sessionID);
		break;
	case CHAT_PACKET_TYPE::ON_CLIENT_LEAVE:
		InterlockedIncrement(&_LeaveCalc);
		RemovePlayer(sessionID);
		break;
	case CHAT_PACKET_TYPE::ON_TIME_OUT:
		DisconnectSession(sessionID);
		break;
	case CHAT_PACKET_TYPE::PACKET_CS_CHAT_REQ_HEARTBEAT:
		//PacketProcHeartBeat(pPacket, sessionID);
		break;
	default:
		wsprintf(errmsg, L"ERROR :: Session Default Case sessionID : %I64u", sessionID);
		_LOG(dfLOG_LEVEL_ERROR, errmsg);
		DisconnectSession(sessionID);
		break;
	}
	if (pPacket != nullptr)
		pPacket->SubRef(3);
}

// PACKET_CS_CHAT_REQ_LOGIN
void CChatServer::PacketProcRequestLogin(Packet *pPacket, SESSION_ID sessionID) {
	PRO_BEGIN(L"RequestLogin");
	pPacket->AddRef(4);
	BYTE status = FALSE;
	ACCOUNT_NO acno;

	if (pPacket->GetDataSize() < sizeof(ACCOUNT_NO)) {
		_LOG(dfLOG_LEVEL_ERROR, L"pPacket->GetDataSize() < sizeof(ACCOUNT_NO)"); // TODO ERROR MSG
		DisconnectSession(sessionID);
	}
	//(*pPacket).GetData((char *) &acno, sizeof(ACCOUNT_NO));
	(*pPacket) >> acno;

	if (pPacket->GetDataSize() != ID_MAX_SIZE + NICK_NAME_MAX_SIZE + TOKEN_KEY_SIZE) {
		_LOG(dfLOG_LEVEL_ERROR, L"pPacket->GetDataSize() != ID_MAX_SIZE + NICK_NAME_MAX_SIZE + TOKEN_KEY_SIZE"); // TODO ERROR MSG
		DisconnectSession(sessionID);
	}
	Player *pPlayer = FindPlayer(sessionID);
	if (pPlayer == nullptr) {
		// new Player
		pPlayer = _playerPool.Alloc();

		pPacket->GetData((char *) pPlayer->_ID, ID_MAX_SIZE);
		pPacket->GetData((char *) pPlayer->_NickName, NICK_NAME_MAX_SIZE);
		pPacket->GetData((char *) pPlayer->_TokenKey, TOKEN_KEY_SIZE);

		pPlayer->_AccountNo = acno;
		pPlayer->_SectorX = -1;
		pPlayer->_SectorY = -1;
		pPlayer->_sessionID = sessionID;

		InsertPlayer(sessionID, pPlayer);
		pPlayer->_isLogin = TRUE;
		status = TRUE;
	}

	pPacket->SubRef(4);

	Packet *pResLoginPacket = Packet::AllocAddRef();
	MakePacketResponseLogin(pResLoginPacket, pPlayer->_AccountNo, status);
	SendPacket(pPlayer->_sessionID, pResLoginPacket);
	if (status == FALSE) {
		_LOG(dfLOG_LEVEL_ERROR, L"status == FALSE"); // TODO ERROR MSG
		DisconnectSession(sessionID);
	}

	InterlockedIncrement(&_LoginCalc);
	_LOG(dfLOG_LEVEL_DEBUG, L"Login OK ANO[%d]", acno); // TODO ERROR MSG
	pResLoginPacket->SubRef();
	PRO_END(L"RequestLogin");
}

// PACKET_CS_CHAT_REQ_SECTOR_MOVE
void CChatServer::PacketProcMoveSector(Packet *pPacket, SESSION_ID sessionID) {
	PRO_BEGIN(L"MoveSector");
	ACCOUNT_NO no;
	WORD sx;
	WORD sy;

	// 패킷 꺼내기
	pPacket->AddRef(5);
	if (pPacket->GetDataSize() != (sizeof(no) + sizeof(sx) + sizeof(sy))) {
		_LOG(dfLOG_LEVEL_ERROR, L"pPacket->GetDataSize() != (sizeof(no) + sizeof(sx) + sizeof(sy))"); // TODO ERROR MSG
		DisconnectSession(sessionID);
		pPacket->SubRef(5);
		return;
	}
	//pPacket->GetData((char *) &no, sizeof(ACCOUNT_NO));
	(*pPacket) >> no >> sx >> sy;
	pPacket->SubRef(6);
	// 섹터 범위 초과
	if (sx >= SECTOR_X_SIZE || sy >= SECTOR_Y_SIZE) {
		_LOG(dfLOG_LEVEL_ERROR, L"PacketProcMoveSector(id [%I64u])  sx >= SECTOR_X_SIZE || sy >= SECTOR_Y_SIZE", sessionID); // TODO ERROR MSG
		DisconnectSession(sessionID);
		return;
	}


	// 플레이어 무결성
	Player *pPlayer = FindPlayer(sessionID);
	if (pPlayer == nullptr) {
		_LOG(dfLOG_LEVEL_ERROR, L"PacketProcMoveSector(id [%I64u])  pPlayer == nullptr", sessionID); // TODO ERROR MSG
		DisconnectSession(sessionID);
		return;
	}if (pPlayer->_isLogin == false) {
		_LOG(dfLOG_LEVEL_ERROR, L"PacketProcMoveSector(id [%I64u])  pPlayer->_isLogin == false", sessionID); // TODO ERROR MSG
		DisconnectSession(sessionID);
		return;
	}if (pPlayer->_AccountNo != no) {
		_LOG(dfLOG_LEVEL_ERROR, L"PacketProcMoveSector(id [%I64u])  _AccountNo == no", sessionID); // TODO ERROR MSG
		DisconnectSession(sessionID);
		return;
	}



	// 기존에 있던 섹터가 있는지
#ifdef UPDATE_THREAD
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
#else
	constexpr WORD comp = -1;
	if (pPlayer->_SectorX == comp && pPlayer->_SectorY == comp) {
		// 처음 시도 경우
		__SECTOR_LOCK(sx, sy);
		{
			pPlayer->_SectorX = sx;
			pPlayer->_SectorY = sy;
			_sector[pPlayer->_SectorY][pPlayer->_SectorX]._playerSet.emplace(pPlayer);
		}
		__SECTOR_UNLOCK(sx, sy);
	} else if (pPlayer->_SectorX != sx || pPlayer->_SectorY != sy) {
		// 섹터간의 이동
		WORD cx = pPlayer->_SectorX;
		WORD cy = pPlayer->_SectorY;
		pPlayer->_SectorX = sx;
		pPlayer->_SectorY = sy;
		for (;;) {
			// 두 섹터 락 시도
			if (TrySectorLock(cx, cy) == false) continue;
			if (TrySectorLock(sx, sy) == false) {
				SectorUnlock(cx, cy);
				YieldProcessor();
				//Sleep(0);
				continue;
			}
			{
				auto iter = _sector[cy][cx]._playerSet.find(pPlayer);
				if (iter != _sector[cy][cx]._playerSet.end()) {
					_sector[cy][cx]._playerSet.erase(iter);
				}
				_sector[sy][sx]._playerSet.emplace(pPlayer);
			}
			SectorUnlock(sx, sy);
			SectorUnlock(cx, cy);

			break;
		}
	}
#endif // UPDATE_THREAD

	// Send RES_SECTOR_MOVE Msg
	Packet *pResPacket = Packet::AllocAddRef();
	MakePacketResponseSectorMove(pResPacket, pPlayer->_AccountNo, pPlayer->_SectorX, pPlayer->_SectorY);
	SendPacket(pPlayer->_sessionID, pResPacket);
	pResPacket->SubRef(7);
	PRO_END(L"MoveSector");
}

// PACKET_CS_CHAT_REQ_MESSAGE
void CChatServer::PacketProcChatRequire(Packet *pPacket, SESSION_ID sessionID) {
	PRO_BEGIN(L"ChatRequire");
	ACCOUNT_NO no;
	WORD msgLen;
	WCHAR message[MASSAGE_MAX_SIZE];
	pPacket->AddRef(6);

	if (pPacket->GetDataSize() < sizeof(ACCOUNT_NO) + sizeof(msgLen)) {
		DisconnectSession(sessionID);
		pPacket->SubRef(8);
		return;
	}
	//pPacket->GetData((char *) &no, sizeof(ACCOUNT_NO));
	(*pPacket) >> no >> msgLen;

	if (pPacket->GetDataSize() != msgLen) {
		DisconnectSession(sessionID);
		pPacket->SubRef(9);
		return;
	}
	pPacket->GetData((char *) message, msgLen);
	message[msgLen / 2] = '\0';




	pPacket->SubRef(10);
	Player *pSender = FindPlayer(sessionID);
	if (pSender == nullptr) {
		//TODO ERROR
		_LOG(dfLOG_LEVEL_ERROR, L"pSender == nullptr");
		DisconnectSession(sessionID);
		return;
	}
	if (pSender->_isLogin == false) {
		_LOG(dfLOG_LEVEL_ERROR, L"pSender->_isLogin == false");
		DisconnectSession(sessionID);
		return;
	}
	if (no != pSender->_AccountNo) {
		//TODO ERROR
		_LOG(dfLOG_LEVEL_ERROR, L"no != pSender->_AccountNo");
		DisconnectSession(sessionID);
		return;
	}
	if (pSender->_SectorX >= 50 || pSender->_SectorY >= 50) {
		//TODO ERROR
		_LOG(dfLOG_LEVEL_ERROR, L"pSender->_SectorX >= 50 || pSender->_SectorY >= 50");
		DisconnectSession(sessionID);
		return;
	}
	Packet *pResPacket = Packet::AllocAddRef();
	MakePacketResponseMessage(pResPacket, pSender->_AccountNo, pSender->_ID, pSender->_NickName, msgLen, message);
	BroadcastSectorAround(pResPacket, pSender->_SectorX, pSender->_SectorY, nullptr);

	pResPacket->SubRef(11);
	PRO_END(L"ChatRequire");
}
void CChatServer::PacketProcHeartBeat(Packet *pPacket, SESSION_ID sessionID) {
	Player *pPlayer = FindPlayer(sessionID);
	if (pPlayer == nullptr) {
		DisconnectSession(sessionID);
	}
}

// PACKET_SC_CHAT_RES_LOGIN
void CChatServer::MakePacketResponseLogin(Packet *pPacket, ACCOUNT_NO account_no, BYTE status) {
	CHAT_PACKET_TYPE type = CHAT_PACKET_TYPE::PACKET_SC_CHAT_RES_LOGIN;
	(*pPacket) << ((WORD) type) << status << account_no;
}

//PACKET_SC_CHAT_RES_SECTOR_MOVE
void CChatServer::MakePacketResponseSectorMove(Packet *pPacket, ACCOUNT_NO account_no, WORD sectorX, WORD sectorY) {
	CHAT_PACKET_TYPE type = CHAT_PACKET_TYPE::PACKET_SC_CHAT_RES_SECTOR_MOVE;
	(*pPacket) << ((WORD) type) << account_no << sectorX << sectorY;

}

//PACKET_SC_CHAT_RES_MESSAGE
void CChatServer::MakePacketResponseMessage(Packet *pPacket, ACCOUNT_NO account_no, const WCHAR *ID, const WCHAR *nickName, WORD msgLen, const WCHAR *message) {
	CHAT_PACKET_TYPE type = CHAT_PACKET_TYPE::PACKET_SC_CHAT_RES_MESSAGE;
	(*pPacket) << ((WORD) type) << account_no;
	pPacket->PutData((char *) ID, ID_MAX_SIZE);
	pPacket->PutData((char *) nickName, NICK_NAME_MAX_SIZE);
	(*pPacket) << msgLen;
	pPacket->PutData((char *) message, msgLen);

}

void CChatServer::BroadcastSector(Packet *pPacket, WORD sectorX, WORD sectorY, Player *exPlayer = nullptr) {
	pPacket->AddRef(15);

	SectorSLock(sectorX, sectorY);
	{
		SECTOR *pSector = &_sector[sectorY][sectorX];
		for (auto iter = pSector->_playerSet.begin(); iter != pSector->_playerSet.end(); ++iter) {
			Player *pPlayer = (*iter);
			if (pPlayer == exPlayer) continue;
			SendPacket(pPlayer->_sessionID, pPacket);
			InterlockedIncrement(&_ChatSendCalc);
			InterlockedIncrement64(&_totalSectorAroundSend);
		}
	}
	SectorSUnlock(sectorX, sectorY);

	InterlockedIncrement64(&_SectorAroundCount);

	pPacket->SubRef(15);
}

void CChatServer::BroadcastSectorAround(Packet *pPacket, WORD sectorX, WORD sectorY, Player *exPlayer = nullptr) {
	WORD sx = sectorX <= 0 ? 0 : sectorX - 1;
	WORD sy = sectorY <= 0 ? 0 : sectorY - 1;
	WORD ex = sectorX >= SECTOR_X_SIZE - 1 ? SECTOR_X_SIZE - 1 : sectorX + 1;
	WORD ey = sectorY >= SECTOR_Y_SIZE - 1 ? SECTOR_Y_SIZE - 1 : sectorY + 1;
	pPacket->AddRef(16);

#ifndef UPDATE_THREAD


	for (WORD dy = sy; dy <= ey; ++dy) {
		for (WORD dx = sx; dx <= ex; ++dx) {
			SectorSLock(dx, dy);
		}
	}
#endif // !UPDATE_THREAD

	{
		for (WORD dy = sy; dy <= ey; ++dy) {
			for (WORD dx = sx; dx <= ex; ++dx) {
				//SECTOR *pSector = &_sector[dy][dx];
				//for (auto iter = pSector->_playerSet.begin(); iter != pSector->_playerSet.end(); ++iter) {

				::unordered_set<Player *> *pSectorPlayerSet = &_sector[dy][dx]._playerSet;
				for (auto iter = pSectorPlayerSet->begin(); iter != pSectorPlayerSet->end(); ++iter) {
					Player *pPlayer = (*iter);
					if (pPlayer == exPlayer || pPlayer->_isLogin == false) continue;
					SendPacket(pPlayer->_sessionID, pPacket);
					InterlockedIncrement(&_ChatSendCalc);
					InterlockedIncrement64(&_totalSectorAroundSend);
				}
			}
		}
		InterlockedIncrement64(&_SectorAroundCount);
	}
#ifndef UPDATE_THREAD

	for (WORD dy = sy; dy <= ey; ++dy) {
		for (WORD dx = sx; dx <= ex; ++dx) {
			SectorSUnlock(dx, dy);
		}
	}
#endif // !UPDATE_THREAD

	pPacket->SubRef(16);
}

void CChatServer::InsertPlayer(SESSION_ID sessionID, Player *pPlayer) {
	__PLAYER_MAP_LOCK();
	_playerMap.emplace(::make_pair(sessionID, pPlayer));
	__PLAYER_MAP_UNLOCK();
}

void CChatServer::RemovePlayer(SESSION_ID sessionID) {
	__PLAYER_MAP_LOCK();
	auto iter = _playerMap.find(sessionID);
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
		if ((*iter)->_sessionID == sessionID) {
			_sector[sy][sx]._playerSet.erase(iter);
			break;
		}
	}
	__SECTOR_UNLOCK(sx, sy);

	_playerPool.Free(pPlayer);
}

Player *CChatServer::FindPlayer(SESSION_ID sessionID) {
	__PLAYER_MAP_LOCK();
	auto iter = _playerMap.find(sessionID);
	if (iter == _playerMap.end()) {
		__PLAYER_MAP_UNLOCK();
		return nullptr;
	}
	__PLAYER_MAP_UNLOCK();
	return iter->second;
}
