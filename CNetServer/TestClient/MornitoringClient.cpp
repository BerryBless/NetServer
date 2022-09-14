#include "pch.h"
#include "MornitoringClient.h"
#include "CommonProtocol.h"

MornitoringClient::MornitoringClient() : CClient(ENCRYPTED_PACKET) {
	CClient::Start(1, 1, FALSE, 3);
}

MornitoringClient::~MornitoringClient() {
	CClient::Quit();
}

void MornitoringClient::ConnectMonitor() {
	CClient::Connect(L"127.0.0.1", 11600);
}

void MornitoringClient::OnEnterServer(SESSION_ID sessionID) {
	_LOG(dfLOG_LEVEL_ERROR, L"//OnEnterServer[%lld]", sessionID);
	Packet *pPacket = Packet::AllocAddRef();
	MakePacketMonitorToolResLogin(pPacket, _loginSessionKey);
	SendPacket(sessionID, pPacket);
	pPacket->SubRef();
}

void MornitoringClient::OnLeaveServer(SESSION_ID sessionID) {
	_LOG(dfLOG_LEVEL_ERROR, L"//OnLeaveServer[%lld]", sessionID);
}

void MornitoringClient::OnRecv(SESSION_ID sessionID, Packet *pPacket) {
	_LOG(dfLOG_LEVEL_DEBUG, L"//OnRecv");
	pPacket->AddRef();
	WORD type;
	(*pPacket) >> type;

	PacketProc(pPacket, sessionID, type);

	pPacket->SubRef();
}

void MornitoringClient::PacketProc(Packet *pPacket, SESSION_ID sessionID, WORD type) {
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

void MornitoringClient::PacketProcMonitorToolResLogin(Packet *pPacket, SESSION_ID sessionID) {
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

void MornitoringClient::PacketProcMonitorToolDataUpdate(Packet *pPacket, SESSION_ID sessionID) {
	// TODO 그래프로 보여주기

	BYTE serverNo;
	BYTE dataType;
	int dataValue;
	int timeStamp;
	*pPacket >> serverNo >> dataType >> dataValue >> timeStamp;
	printf_s("serverNo[%d], dataType[%d], dataValue[%d] timeStamp[%d]\n", serverNo, dataType, dataValue, timeStamp);
}

void MornitoringClient::MakePacketMonitorToolResLogin(Packet *pPacket, const char *loginSessionKey) {
	WORD type = PACKET_TYPE::en_PACKET_CS_MONITOR_TOOL_REQ_LOGIN;
	*pPacket << type;
	pPacket->PutData((char *) loginSessionKey, MONITOR_LOGIN_SESSION_KEY_SIZE);
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
