#include "pch.h"
#include "CMonitorToolServer.h"
#include <conio.h>
#include <time.h>
#include <stdio.h>

CMonitorToolServer::CMonitorToolServer() : CServer(ENCRYPTED_PACKET), _isRunning{ false }{
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

void CMonitorToolServer::BroadcastPacket(Packet *pPacket) {
	for (auto iter = _clientMap.begin(); iter != _clientMap.end(); ++iter) {
		MonitorClient *pConnection = iter->second;
		SendPacket(pConnection->_id, pPacket);
	}
}

bool CMonitorToolServer::OnConnectionRequest(WCHAR *IPstr, DWORD IP, USHORT Port) {
	return _isRunning;
}

void CMonitorToolServer::OnClientJoin(WCHAR *ipStr, DWORD ip, USHORT port, SESSION_ID sessionID) {
	//TEMP

}

void CMonitorToolServer::OnClientLeave(SESSION_ID sessionID) {
}

void CMonitorToolServer::OnRecv(SESSION_ID sessionID, Packet *pPacket) {
	pPacket->AddRef();
	WORD type;
	(*pPacket) >> type;

	PacketProc(pPacket, sessionID, type);

	pPacket->SubRef();
}

void CMonitorToolServer::OnSend(SESSION_ID sessionID) {
}

void CMonitorToolServer::OnError(int errorcode, const WCHAR *log) {
}

void CMonitorToolServer::OnTimeout(SESSION_ID sessionID) {
}

void CMonitorToolServer::PacketProc(Packet *pPacket, SESSION_ID sessionID, WORD type) {
	switch (type) {
	case PACKET_TYPE::en_PACKET_CS_MONITOR_TOOL_REQ_LOGIN:
		PacketProcMonitorToolReqLogin(pPacket, sessionID);
		break;
	default:
		DisconnectSession(sessionID);
		break;
	}
}

void CMonitorToolServer::PacketProcMonitorToolReqLogin(Packet *pPacket, SESSION_ID sessionID) {
	Packet *pSendPacket = Packet::AllocAddRef();
	char LoginSessionKey[MONITOR_LOGIN_SESSION_KEY_SIZE + 1];
	pPacket->GetData(LoginSessionKey, MONITOR_LOGIN_SESSION_KEY_SIZE);
	_LOG(dfLOG_LEVEL_DEBUG, L"//OnClientJoin[%lld]", sessionID);
	// 비교
	if (memcmp(LoginSessionKey, _loginSessionKey, MONITOR_LOGIN_SESSION_KEY_SIZE) != 0) {
		// 오류
		_LOG(dfLOG_LEVEL_ERROR, L"//OnClientJoin[%lld] : wrong login code", sessionID);
		MakePacketMonitorToolReqLogin(pSendPacket, en_PACKET_CS_MONITOR_TOOL_RES_LOGIN::dfMONITOR_TOOL_LOGIN_ERR_SESSIONKEY);
		SendPacket(sessionID, pSendPacket);
		DisconnectSession(sessionID);

	} else {
		// 로그인
		_LOG(dfLOG_LEVEL_ERROR, L"Monitor Tool Login OK");
		MonitorClient *pConnection = new MonitorClient;
		pConnection->_id = sessionID;
		pConnection->_isLogin = true;
		InsertClient(sessionID, pConnection);
		MakePacketMonitorToolReqLogin(pSendPacket, en_PACKET_CS_MONITOR_TOOL_RES_LOGIN::dfMONITOR_TOOL_LOGIN_OK);
		SendPacket(sessionID, pSendPacket);
	}
	pSendPacket->SubRef();
}

void CMonitorToolServer::MakePacketMonitorToolReqLogin(Packet *pPacket, BYTE state) {
	WORD type = PACKET_TYPE::en_PACKET_CS_MONITOR_TOOL_RES_LOGIN;
	*pPacket << type << state;
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
