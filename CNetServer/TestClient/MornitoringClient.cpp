#include "pch.h"
#include "MornitoringClient.h"
#include "CommonProtocol.h"

MornitoringClient::MornitoringClient() : CClient (true){
	CClient::Start(1, 1, FALSE, 3);
}

MornitoringClient::~MornitoringClient() {
	CClient::Quit();
}

void MornitoringClient::ConnectMonitor() {
	CClient::Connect(L"127.0.0.1", 11601);
}

void MornitoringClient::OnEnterServer(SESSION_ID sessionID) {
	CLogger::_Log(dfLOG_LEVEL_ERROR, L"//OnEnterServer[%lld]", sessionID);
	Client *pClient = new Client;
	pClient->_isLogin = false;
	pClient->_data = 0;
	pClient->_ID = sessionID;
	InsertClient(sessionID, pClient);
}

void MornitoringClient::OnLeaveServer(SESSION_ID sessionID) {
	CLogger::_Log(dfLOG_LEVEL_ERROR, L"//OnLeaveServer[%lld]", sessionID);
}

void MornitoringClient::OnRecv(SESSION_ID sessionID, Packet *pPacket) {
	CLogger::_Log(dfLOG_LEVEL_ERROR, L"//OnRecv");
	pPacket->AddRef();
	WORD type;
	(*pPacket) >> type;

	//PacketProc(pPacket, sessionID, type);
	if (PACKET_TYPE::en_PACKET_CS_MONITOR_TOOL_DATA_UPDATE == type) {
		BYTE serverNo;
		BYTE dataType;
		int dataValue;
		int timeStamp;
		*pPacket >> serverNo >> dataType >> dataValue >> timeStamp;
		printf_s("serverNo[%d], dataType[%d], dataValue[%d] timeStamp[%d]\n", serverNo, dataType, dataValue, timeStamp);

	}

	pPacket->SubRef();
}

void MornitoringClient::InsertClient(SESSION_ID sessionID, Client *pClient) {
	_ClientMap.emplace(::make_pair(sessionID, pClient));
}

void MornitoringClient::RemoveClient(SESSION_ID sessionID) {
	auto iter = _ClientMap.find(sessionID);
	if (iter == _ClientMap.end()) {
		return;
	}
	Client *pClient = iter->second;
	_ClientMap.erase(iter);

	pClient->_isLogin = false;
	pClient->_data = 0;

	delete pClient;
}

MornitoringClient::Client *MornitoringClient::FindClient(SESSION_ID sessionID) {
	auto iter = _ClientMap.find(sessionID);
	if (iter == _ClientMap.end()) {
		return nullptr;
	}
	return iter->second;
}
