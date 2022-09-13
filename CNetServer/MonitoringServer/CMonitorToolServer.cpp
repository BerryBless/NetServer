#include "pch.h"
#include "CMonitorToolServer.h"
#include <conio.h>
#include <time.h>
#include <stdio.h>

CMonitorToolServer::CMonitorToolServer()  : CServer(true){
}

CMonitorToolServer::~CMonitorToolServer() {
}

void CMonitorToolServer::BeginServer(u_long IP, u_short port, BYTE workerThreadCount, BYTE maxRunThreadCount, BOOL nagle, u_short maxConnection) {
	_isRunning = Start(IP, port, workerThreadCount, maxRunThreadCount, nagle, maxConnection);
}

void CMonitorToolServer::BeginServer(const WCHAR *szConfigFile) {
}

void CMonitorToolServer::CloseServer() {
	_isRunning = false;
	CServer::Quit();
}

void CMonitorToolServer::CommandWait() {
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

void CMonitorToolServer::BroadcastPacket(Packet *pPacket) {
	for (auto iter = _clientMap.begin(); iter != _clientMap.end(); ++iter) {
		MonitorClient *pConnection = iter->second;
		SendPacket(pConnection->_id, pPacket);
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"//BroadcastPacket[%lld]", pConnection->_id);
	}
}

bool CMonitorToolServer::OnConnectionRequest(WCHAR *IPstr, DWORD IP, USHORT Port) {
	return _isRunning;
}

void CMonitorToolServer::OnClientJoin(WCHAR *ipStr, DWORD ip, USHORT port, SESSION_ID sessionID) {
	//TEMP
	CLogger::_Log(dfLOG_LEVEL_ERROR, L"//OnClientJoin[%lld]", sessionID);
	MonitorClient *pConnection = new MonitorClient;
	pConnection->_id = sessionID;
	pConnection->_isLogin = true;
	InsertClient(sessionID, pConnection);
}

void CMonitorToolServer::OnClientLeave(SESSION_ID sessionID) {
}

void CMonitorToolServer::OnRecv(SESSION_ID sessionID, Packet *pPacket) {
}

void CMonitorToolServer::OnSend(SESSION_ID sessionID) {
}

void CMonitorToolServer::OnError(int errorcode, const WCHAR *log) {
}

void CMonitorToolServer::OnTimeout(SESSION_ID sessionID) {
}

void CMonitorToolServer::InsertClient(SESSION_ID sessionID, MonitorClient *pClient) {
    _clientMap.emplace(::make_pair(sessionID, pClient));
}

void CMonitorToolServer::RemoveClient(SESSION_ID sessionID) {
	auto iter = _clientMap.find(sessionID);
	if (iter == _clientMap.end()) {
		return;
	}
	MonitorClient *pClient = iter->second;
	_clientMap.erase(iter);

	delete 	pClient;
}

CMonitorToolServer::MonitorClient *CMonitorToolServer::FindClient(SESSION_ID sessionID) {
	auto iter = _clientMap.find(sessionID);
	if (iter == _clientMap.end()) {
		return nullptr;
	}
	return iter->second;
}
