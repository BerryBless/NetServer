#include "pch.h"
#include "CMonitorToolServer.h"

CMonitorToolServer::CMonitorToolServer()  : CServer(true){
}

CMonitorToolServer::~CMonitorToolServer() {
}

bool CMonitorToolServer::OnConnectionRequest(WCHAR *IPstr, DWORD IP, USHORT Port) {
    return false;
}

void CMonitorToolServer::OnClientJoin(WCHAR *ipStr, DWORD ip, USHORT port, SESSION_ID sessionID) {
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
