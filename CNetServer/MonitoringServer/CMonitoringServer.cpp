#include "pch.h"
#include "CMonitoringServer.h"
#include <conio.h>
#include <time.h>
#include <stdio.h>

CMonitoringServer::CMonitoringServer() :CServer(false) {
	_pMonitorToolServer = new CMonitorToolServer;
	_preLogTimer = _curLogTimer = timeGetTime();
}

CMonitoringServer::~CMonitoringServer() {
}

void CMonitoringServer::BeginServer(u_long IP, u_short monitorSPort, u_short toolSPort, BYTE workerThreadCount, BYTE maxRunThreadCount, BOOL nagle, u_short maxConnection) {
	_isRunning = Start(IP, monitorSPort, workerThreadCount, maxRunThreadCount, nagle, maxConnection);
	_pMonitorToolServer->BeginServer(IP, toolSPort, workerThreadCount, maxRunThreadCount, nagle, maxConnection);
}

void CMonitoringServer::BeginServer(const WCHAR *szConfigFile) {
	int MSPort = 0; // Monitor server
	int TSPort = 0; // Tool Server
	int wThreadCount = 0;
	int rThreadCount = 0;
	bool isNagle = false;
	int maxConnetion = 0;

	_pConfigData = new CParser(szConfigFile);
	// Server lib Config
	_pConfigData->SetNamespace(L"MonitorServerLibConfig");
	_pConfigData->TryGetValue(L"ServerPort", MSPort);
	_pConfigData->TryGetValue(L"WorkerThreadCount", wThreadCount);
	_pConfigData->TryGetValue(L"MaxRunningThreadCount", rThreadCount);
	_pConfigData->TryGetValue(L"isNagle", isNagle);
	_pConfigData->TryGetValue(L"MaxConnectionCount", maxConnetion);

	// Monitor Server Connect Config
	_pConfigData->SetNamespace(L"MonitorToolServerLibConfig");
	_pConfigData->TryGetValue(L"ServerPort", TSPort);

	// DVConnect Config
	WCHAR connectstring[512];
	_pConfigData->SetNamespace(L"DB");
	_pConfigData->TryGetValue(L"ConnectString", connectstring);
	ASSERT_CRASH(_DBPool.Connect(1, connectstring));

	// Start Chat Server
	BeginServer(INADDR_ANY, MSPort, TSPort, wThreadCount, rThreadCount, isNagle, maxConnetion);

	// Set StartTime
	time(&_startTime);
	localtime_s(&_timeFormet, &_startTime);
}

void CMonitoringServer::CloseServer() {
	_isRunning = false;
	delete _pMonitorToolServer;
	CServer::Quit();
}

void CMonitoringServer::CommandWait() {
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

bool CMonitoringServer::OnConnectionRequest(WCHAR *IPstr, DWORD IP, USHORT Port) {
	return _isRunning;
}

void CMonitoringServer::OnClientJoin(WCHAR *ipStr, DWORD ip, USHORT port, SESSION_ID sessionID) {
	_LOG(dfLOG_LEVEL_ERROR, L"Join Server : ipStr[%s:%hd] %I64u", ipStr, ip, sessionID);
}

void CMonitoringServer::OnClientLeave(SESSION_ID sessionID) {
	_LOG(dfLOG_LEVEL_ERROR, L"Leave Server : %I64u", sessionID);
}

void CMonitoringServer::OnRecv(SESSION_ID sessionID, Packet *pPacket) {
	pPacket->AddRef();
	WORD type;
	(*pPacket) >> type;

	PacketProc(pPacket, sessionID, type);


	pPacket->SubRef();
}

void CMonitoringServer::OnSend(SESSION_ID sessionID) {
}

void CMonitoringServer::OnError(int errorcode, const WCHAR *log) {
}

void CMonitoringServer::OnTimeout(SESSION_ID sessionID) {
}

void CMonitoringServer::OnMonitoringPerSec() {
	_curLogTimer = timeGetTime();
	if (_curLogTimer - _preLogTimer >= 6000) {
		_preLogTimer = _curLogTimer;
		if (_C_RunningFlag) {
			printf_s("=============================================================\n");
			printf_s("chatserver :: ON\n");
			printf_s("CPU\t\t{ TOTAL[%lld], TICK[%lld], min[%lld], MAX[%lld] }\n",
				_C_CPU[this->TOTAL], _C_CPU[this->TICK], _C_CPU[this->MIN], _C_CPU[this->MAX]);
			printf_s("PrivateBytes\t{ TOTAL[%lld], TICK[%lld], min[%lld], MAX[%lld] }\n",
				_C_PrivateBytes[this->TOTAL], _C_PrivateBytes[this->TICK], _C_PrivateBytes[this->MIN], _C_PrivateBytes[this->MAX]);
			printf_s("PacketPoolSize\t{ TOTAL[%lld], TICK[%lld], min[%lld], MAX[%lld] }\n",
				_C_PacketPoolSize[this->TOTAL], _C_PacketPoolSize[this->TICK], _C_PacketPoolSize[this->MIN], _C_PacketPoolSize[this->MAX]);
			printf_s("SessionCount\t{ TOTAL[%lld], TICK[%lld], min[%lld], MAX[%lld] }\n",
				_C_SessionCount[this->TOTAL], _C_SessionCount[this->TICK], _C_SessionCount[this->MIN], _C_SessionCount[this->MAX]);
			printf_s("PlayerCount\t{ TOTAL[%lld], TICK[%lld], min[%lld], MAX[%lld] }\n",
				_C_PlayerCount[this->TOTAL], _C_PlayerCount[this->TICK], _C_PlayerCount[this->MIN], _C_PlayerCount[this->MAX]);
			printf_s("UpdateTPS\t{ TOTAL[%lld], TICK[%lld], min[%lld], MAX[%lld] }\n",
				_C_UpdateTPS[this->TOTAL], _C_UpdateTPS[this->TICK], _C_UpdateTPS[this->MIN], _C_UpdateTPS[this->MAX]);
			printf_s("JobQueue\t{ TOTAL[%lld], TICK[%lld], min[%lld], MAX[%lld] }\n",
				_C_JobQueue[this->TOTAL], _C_JobQueue[this->TICK], _C_JobQueue[this->MIN], _C_JobQueue[this->MAX]);
		}
	}
}

void CMonitoringServer::PacketProc(Packet *pPacket, SESSION_ID sessionID, WORD type) {
	switch (type) {
	case PACKET_TYPE::en_PACKET_SS_MONITOR_LOGIN:
		PacketProcMonitorLogin(pPacket, sessionID);
		break;
	case PACKET_TYPE::en_PACKET_SS_MONITOR_DATA_UPDATE:
		PacketProcMonitorDataUpdate(pPacket, sessionID);
		break;
	default:
		break;
	}
}

void CMonitoringServer::PacketProcMonitorLogin(Packet *pPacket, SESSION_ID sessionID) {
	int serverNo;
	*pPacket >> serverNo;
	ServerConnect *pConnection = new ServerConnect;
	pConnection->_id = sessionID;
	pConnection->_serverNo = serverNo;
	pConnection->_isLogin = true;
	InsertServer(sessionID, pConnection);
}

void CMonitoringServer::PacketProcMonitorDataUpdate(Packet *pPacket, SESSION_ID sessionID) {
	BYTE serverNo;
	BYTE dataType;
	int dataValue;
	int timeStamp;
	*pPacket >> serverNo >> dataType >> dataValue >> timeStamp;
	ServerConnect *pConnection = FindServer(sessionID);
	ASSERT_CRASH(pConnection != nullptr);
	if (pConnection->_isLogin == false) {
		DisconnectSession(sessionID);
	}


	switch (serverNo) {
	case SERVER_TYPE::MONITORING_SERVER:
		break;
	case SERVER_TYPE::GAME_SERVER:
		break;
	case SERVER_TYPE::CHAT_SERVER:
		DataUpdateChatServer(dataType, dataValue, timeStamp);
		break;
	case SERVER_TYPE::LOGIN_SERVER:
		break;
	default:
		break;
	}

	Packet *pSendPacket = Packet::AllocAddRef();
	MakePacketMonitorDataUpdate(pSendPacket, serverNo, dataType, dataValue, timeStamp);

	_pMonitorToolServer->BroadcastPacket(pSendPacket);
	pSendPacket->SubRef();

}

void CMonitoringServer::ResetChatServerData() {
	memmove_s(this->_C_CPU, sizeof(_Template), _Template, sizeof(_Template));
	memmove_s(this->_C_PrivateBytes, sizeof(_Template), _Template, sizeof(_Template));
	memmove_s(this->_C_PacketPoolSize, sizeof(_Template), _Template, sizeof(_Template));
	memmove_s(this->_C_SessionCount, sizeof(_Template), _Template, sizeof(_Template));
	memmove_s(this->_C_PlayerCount, sizeof(_Template), _Template, sizeof(_Template));
	memmove_s(this->_C_UpdateTPS, sizeof(_Template), _Template, sizeof(_Template));
	memmove_s(this->_C_JobQueue, sizeof(_Template), _Template, sizeof(_Template));
}

void CMonitoringServer::DataUpdateChatServer(BYTE dataType, int dataValue, int timeStamp) {
	ULONGLONG *target = nullptr;
	WCHAR dataName[20];
	switch (dataType) {
	case CHAT_SERVER_MONITORING_TYPE::CHAT_SERVER_ON_OFF:
		_C_RunningFlag = !_C_RunningFlag;
		break;
	case CHAT_SERVER_MONITORING_TYPE::CHAT_SERVER_CPU_USAGE:
		target = _C_CPU;
		swprintf_s(dataName, L"CPU_USAGE");
		break;
	case CHAT_SERVER_MONITORING_TYPE::CHAT_SERVER_PRIVATE_BYTES:
		target = _C_PrivateBytes;
		swprintf_s(dataName, L"PRIVATE_BYTES");
		break;
	case CHAT_SERVER_MONITORING_TYPE::CHAT_SERVER_SESSION_COUNTS:
		target = _C_SessionCount;
		swprintf_s(dataName, L"SESSION_COUNTS");
		break;
	case CHAT_SERVER_MONITORING_TYPE::CHAT_SERVER_PLAYER_COUNTS:
		swprintf_s(dataName, L"PLAYER_COUNTS");
		target = _C_PlayerCount;
		break;
	case CHAT_SERVER_MONITORING_TYPE::CHAT_SERVER_UPDATE_TPS:
		target = _C_UpdateTPS;
		swprintf_s(dataName, L"UPDATE_TPS");
		break;
	case CHAT_SERVER_MONITORING_TYPE::CHAT_SERVER_PACKET_POOL_USAGE:
		target = _C_PacketPoolSize;
		swprintf_s(dataName, L"POOL_USAGE");
		break;
	case CHAT_SERVER_MONITORING_TYPE::CHAT_SERVER_UPDATE_MSG_QUEUE_SIZE:
		target = _C_JobQueue;
		swprintf_s(dataName, L"QUEUE_SIZE");
		break;
	default:
		break;
	}
	if (target != nullptr) {
		target[this->TOTAL] += dataValue;
		target[this->MIN] = min(target[this->MIN], dataValue);
		target[this->MAX] = max(target[this->MAX], dataValue);
		++target[this->TICK];
		QueryDataBase(SERVER_TYPE::CHAT_SERVER, dataName, target[this->TOTAL], target[this->MIN], target[this->MAX], target[this->TICK]);
	}
}

void CMonitoringServer::MakePacketMonitorDataUpdate(Packet *pPacket, BYTE serverNo, BYTE dataType, int dataValue, int timeStamp) {
	WORD type = PACKET_TYPE::en_PACKET_CS_MONITOR_TOOL_DATA_UPDATE;
	*pPacket << type << serverNo << dataType << dataValue << timeStamp;
}

void CMonitoringServer::InsertServer(SESSION_ID sessionID, ServerConnect *pConnection) {
	_serverMap.emplace(::make_pair(sessionID, pConnection));
}

void CMonitoringServer::RemoveServer(SESSION_ID sessionID) {
	auto iter = _serverMap.find(sessionID);
	if (iter == _serverMap.end()) {
		return;
	}
	ServerConnect *pConnection = iter->second;
	_serverMap.erase(iter);

	delete 	pConnection;
}

CMonitoringServer::ServerConnect *CMonitoringServer::FindServer(SESSION_ID sessionID) {
	auto iter = _serverMap.find(sessionID);
	if (iter == _serverMap.end()) {
		return nullptr;
	}
	return iter->second;
}

void CMonitoringServer::QueryDataBase(int serverNo, const WCHAR *dataType, ULONGLONG total, ULONGLONG min, ULONGLONG max, ULONGLONG tick) {
	DBConnection *dbConn = _DBPool.Pop();
	time_t now = time(nullptr);
	tm t;
	localtime_s(&t, &now);
	WCHAR query[512];
	int idx = -1;
	switch (serverNo) {
	case SERVER_TYPE::CHAT_SERVER:
		idx = 0;
		break;
	default:
		break;
	}
	swprintf_s(query, _QUERY_FORMAT[idx], t.tm_year % 100, t.tm_mon + 1, dataType, total, min, max, tick);
	if (dbConn->Execute(query) == false) {
		wprintf_s(L"%s", query);
		swprintf_s(query, _QUERY_FORMAT[idx + 1], t.tm_year % 100, t.tm_mon + 1);
		ASSERT_CRASH(dbConn->Execute(query));

		swprintf_s(query, _QUERY_FORMAT[idx], t.tm_year % 100, t.tm_mon + 1, dataType, total, min, max, tick);
		ASSERT_CRASH(dbConn->Execute(query));
	}
	_DBPool.Push(dbConn);
}
