#include "pch.h"
#include "CMonitoringServer.h"
#include <conio.h>
#include <time.h>
#include <stdio.h>

CMonitoringServer::CMonitoringServer() :CServer (false){
	_pMonitorToolServer = new CMonitorToolServer;
}

CMonitoringServer::~CMonitoringServer() {
	delete _pMonitorToolServer;
}

void CMonitoringServer::BeginServer(u_long IP, u_short port, BYTE workerThreadCount, BYTE maxRunThreadCount, BOOL nagle, u_short maxConnection) {
	_isRunning = Start(IP, port, workerThreadCount, maxRunThreadCount, nagle, maxConnection);
}

void CMonitoringServer::BeginServer(const WCHAR *szConfigFile) {
}

void CMonitoringServer::CloseServer() {
	_isRunning = false;
	CServer::Quit();
}

void CMonitoringServer::CommandWait() {
	int printTick = 0;

	for (;;) {
		char cmd = _getch();
		if (cmd == 'Q' || cmd == 'q') {
			//MemProfiler::Instance().PrintInfo();
			CLogger::_Log(dfLOG_LEVEL_NOTICE, L"Close Server from Cmd");
			CloseServer();
			break;
		}
		if (cmd == 'C' || cmd == 'c') {
			CLogger::_Log(dfLOG_LEVEL_NOTICE, L"Crash Server from Cmd");
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

bool CMonitoringServer::OnConnectionRequest(WCHAR *IPstr, DWORD IP, USHORT Port) {
	return _isRunning;
}

void CMonitoringServer::OnClientJoin(WCHAR *ipStr, DWORD ip, USHORT port, SESSION_ID sessionID) {
	CLogger::_Log(dfLOG_LEVEL_ERROR, L"Join Server : ipStr[%s:%hd] %I64u", ipStr, ip, sessionID);
}

void CMonitoringServer::OnClientLeave(SESSION_ID sessionID) {
	CLogger::_Log(dfLOG_LEVEL_ERROR, L"Leave Server : %I64u", sessionID);
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
}

void CMonitoringServer::PacketProcMonitorDataUpdate(Packet *pPacket, SESSION_ID sessionID) {
	BYTE dataType;
	int dataValue;
	int timeStamp;
	*pPacket >> dataType >> dataValue >> timeStamp;
	ServerConnect *pConnection = FindServer(sessionID);
	ASSERT_CRASH(pConnection == nullptr);
	if (pConnection->_isLogin == false) {
		DisconnectSession(sessionID);
	}
	


}

void CMonitoringServer::MakePacketMonitorDataUpdate(Packet *pPacket, BYTE serverNo, BYTE dataType, int dataValue, int timeStamp) {
}

void CMonitoringServer::InsertServer(SESSION_ID sessionID, ServerConnect *pServer) {
	_serverMap.emplace(::make_pair(sessionID, pServer));
}

void CMonitoringServer::RemoveServer(SESSION_ID sessionID) {
	auto iter = _serverMap.find(sessionID);
	if (iter == _serverMap.end()) {
		return;
	}
	ServerConnect *pServer = iter->second;
	_serverMap.erase(iter);

	delete 	pServer;
}

CMonitoringServer::ServerConnect *CMonitoringServer::FindServer(SESSION_ID sessionID) {
	auto iter = _serverMap.find(sessionID);
	if (iter == _serverMap.end()) {
		return nullptr;
	}
	return iter->second;
}
