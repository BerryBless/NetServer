#include "pch.h"
#include "CMonitoringServer.h"

CMonitoringServer::CMonitoringServer() :CServer (false){
	_pMonitorToolServer = new CMonitorToolServer;
}

CMonitoringServer::~CMonitoringServer() {
	delete _pMonitorToolServer;
}

bool CMonitoringServer::OnConnectionRequest(WCHAR *IPstr, DWORD IP, USHORT Port) {
	return false;
}

void CMonitoringServer::OnClientJoin(WCHAR *ipStr, DWORD ip, USHORT port, SESSION_ID sessionID) {
}

void CMonitoringServer::OnClientLeave(SESSION_ID sessionID) {
}

void CMonitoringServer::OnRecv(SESSION_ID sessionID, Packet *pPacket) {
}

void CMonitoringServer::OnSend(SESSION_ID sessionID) {
}

void CMonitoringServer::OnError(int errorcode, const WCHAR *log) {
}

void CMonitoringServer::OnTimeout(SESSION_ID sessionID) {
}

void CMonitoringServer::PacketProc(Packet *pPacket, SESSION_ID sessionID, WORD type) {
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
