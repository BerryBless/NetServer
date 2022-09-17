#include "pch.h"
#include "MonitoringClient.h"
#include "CommonProtocol.h"

MonitoringClient::MonitoringClient(const WCHAR *szConfigFile) : CClient(ENCRYPTED_PACKET) {
	CClient::Start(1, 1, FALSE, 3);
	_pConfigData = new CParser(szConfigFile);
	_pConfigData->SetNamespace(L"MonitorClientConfig");

	// Monitor Server Connect Config
	_pConfigData->SetNamespace(L"MonitorServerConnect");
	_pConfigData->TryGetValue(L"MonitorServerIP", _monitorServerIP);
	_pConfigData->TryGetValue(L"MonitorServerPort", _monitorServerPort);

}

MonitoringClient::~MonitoringClient() {
	CClient::Quit();
}

void MonitoringClient::ConnectMonitor() {
	CClient::Connect(_monitorServerIP, _monitorServerPort);
}

void MonitoringClient::OnEnterServer(SESSION_ID sessionID) {
	_LOG(dfLOG_LEVEL_ERROR, L"//OnEnterServer[%lld]", sessionID);
	Packet *pPacket = Packet::AllocAddRef();
	MakePacketMonitorToolResLogin(pPacket, _loginSessionKey);
	SendPacket(sessionID, pPacket);
	pPacket->SubRef();
}

void MonitoringClient::OnLeaveServer(SESSION_ID sessionID) {
	_LOG(dfLOG_LEVEL_ERROR, L"//OnLeaveServer[%lld]", sessionID);
}

void MonitoringClient::OnRecv(SESSION_ID sessionID, Packet *pPacket) {
	_LOG(dfLOG_LEVEL_DEBUG, L"//OnRecv");
	pPacket->AddRef();
	WORD type;
	(*pPacket) >> type;

	PacketProc(pPacket, sessionID, type);

	pPacket->SubRef();
}

void MonitoringClient::PacketProc(Packet *pPacket, SESSION_ID sessionID, WORD type) {
	switch (type) {
	case PACKET_TYPE::en_PACKET_CS_MONITOR_TOOL_DATA_UPDATE:
		PacketProcMonitorToolDataUpdate(pPacket, sessionID);
		break;
	case PACKET_TYPE::en_PACKET_CS_MONITOR_TOOL_RES_LOGIN:
		PacketProcMonitorToolResLogin(pPacket, sessionID);
		break;
	default:
		_LOG(dfLOG_LEVEL_ERROR, L"Default Case Monitoring Clinet Packet Procsess");
		DisconnectSession(sessionID);
		break;
	}
}

void MonitoringClient::PacketProcMonitorToolResLogin(Packet *pPacket, SESSION_ID sessionID) {
	BYTE state;
	*pPacket >> state;
	switch (state) {
	case en_PACKET_CS_MONITOR_TOOL_RES_LOGIN::dfMONITOR_TOOL_LOGIN_OK:
	{
		Client *pClient = new Client;
		pClient->_isLogin = false;
		pClient->_data = 0;
		pClient->_ID = sessionID;
		InsertClient(sessionID, pClient);
	}
	break;
	case en_PACKET_CS_MONITOR_TOOL_RES_LOGIN::dfMONITOR_TOOL_LOGIN_ERR_SESSIONKEY:
	{
		DisconnectSession(sessionID);
		this->Quit(); // TODO 종료 함수 만들기
	}
	break;

	default:
		break;
	}
}

void MonitoringClient::PacketProcMonitorToolDataUpdate(Packet *pPacket, SESSION_ID sessionID) {
	// TODO 그래프로 보여주기

	BYTE serverNo;
	BYTE dataType;
	int dataValue;
	int timeStamp;
	*pPacket >> serverNo >> dataType >> dataValue >> timeStamp;
	printf_s("serverNo[%d], dataType[%d], dataValue[%d] timeStamp[%d]\n", serverNo, dataType, dataValue, timeStamp);
}

void MonitoringClient::MakePacketMonitorToolResLogin(Packet *pPacket, const char *loginSessionKey) {
	WORD type = PACKET_TYPE::en_PACKET_CS_MONITOR_TOOL_REQ_LOGIN;
	*pPacket << type;
	pPacket->PutData((char *) loginSessionKey, MONITOR_LOGIN_SESSION_KEY_SIZE);
}

void MonitoringClient::InsertClient(SESSION_ID sessionID, Client *pClient) {
	_ClientMap.emplace(::make_pair(sessionID, pClient));
}

void MonitoringClient::RemoveClient(SESSION_ID sessionID) {
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

MonitoringClient::Client *MonitoringClient::FindClient(SESSION_ID sessionID) {
	auto iter = _ClientMap.find(sessionID);
	if (iter == _ClientMap.end()) {
		return nullptr;
	}
	return iter->second;
}
