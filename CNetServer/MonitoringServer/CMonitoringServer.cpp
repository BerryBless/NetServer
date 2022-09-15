#include "pch.h"
#include "CMonitoringServer.h"
#include <conio.h>
#include <time.h>
#include <stdio.h>

CMonitoringServer::CMonitoringServer() :CServer (false){
	_pMonitorToolServer = new CMonitorToolServer;
}

CMonitoringServer::~CMonitoringServer() {
}

void CMonitoringServer::BeginServer(u_long IP,  u_short monitorSPort, u_short toolSPort, BYTE workerThreadCount, BYTE maxRunThreadCount, BOOL nagle, u_short maxConnection) {
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
	*pPacket >> serverNo>> dataType >> dataValue >> timeStamp;
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
	this->_C_RunningFlag = false;
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
	switch (dataType) {
	case CHAT_SERVER_MONITORING_TYPE::CHAT_SERVER_ON_OFF:	_C_RunningFlag = true; break;
	case CHAT_SERVER_MONITORING_TYPE::CHAT_SERVER_CPU_USAGE:
		target = _C_CPU;
		break;
	case CHAT_SERVER_MONITORING_TYPE::CHAT_SERVER_PRIVATE_BYTES:
		target = _C_PrivateBytes;
		break;
	case CHAT_SERVER_MONITORING_TYPE::CHAT_SERVER_SESSION_COUNTS:
		target = _C_SessionCount;
		break;
	case CHAT_SERVER_MONITORING_TYPE::CHAT_SERVER_PLAYER_COUNTS:
		target = _C_PlayerCount;
		break;
	case CHAT_SERVER_MONITORING_TYPE::CHAT_SERVER_UPDATE_TPS:
		target = _C_UpdateTPS;
		break;
	case CHAT_SERVER_MONITORING_TYPE::CHAT_SERVER_PACKET_POOL_USAGE:
		target = _C_PacketPoolSize;
		break;
	case CHAT_SERVER_MONITORING_TYPE::CHAT_SERVER_UPDATE_MSG_QUEUE_SIZE:
		target = _C_JobQueue;
		break;
	default:
		break;
	}
	if (target != nullptr) {
		target[this->TOTAL] += dataValue;
		target[this->MIN] = min(target[this->MIN], dataValue);
		target[this->MAX] = max(target[this->MAX], dataValue);
		++target[this->TICK];
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
