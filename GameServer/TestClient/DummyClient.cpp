#include "pch.h"
#include "DummyClient.h"

DummyClient::DummyClient(const WCHAR *configFilePath) {
	_pConfigData = new CParser(configFilePath);

	_pConfigData->SetNamespace(L"ServerConnectConfig");
	_pConfigData->TryGetValue(L"ServerIP", _serverIP);
	_pConfigData->TryGetValue(L"ServerPort", _serverPort);

}

DummyClient::~DummyClient() {
	CClient::Quit();
	delete _pConfigData;
}

void DummyClient::Connect() {
	CClient::Connect(_serverIP, _serverPort);
}

void DummyClient::OnEnterServer(SESSION_ID sessionID) {
	// TODO 패킷받으면 연결?
	ClientSession *pClient = _clientPool.Alloc();
	pClient->_isLogin = false;
	pClient->_data = 0;
	pClient->_ID = sessionID;
	InsertClient(sessionID, pClient);
}

void DummyClient::OnLeaveServer(SESSION_ID sessionID) {
}

void DummyClient::OnRecv(SESSION_ID sessionID, Packet *pPacket) {
	pPacket->AddRef();
	WORD type;
	(*pPacket) >> type;

	PacketProc(pPacket, sessionID, type);

	pPacket->SubRef();
}

void DummyClient::OnSend(SESSION_ID sessionID) {
}

void DummyClient::OnError(int errorcode, const WCHAR *) {
}

void DummyClient::PacketProc(Packet *pPacket, SESSION_ID sessionID, WORD type) {
	switch (type) {
	default:
		_LOG(dfLOG_LEVEL_ERROR, L"Default Case Monitoring Clinet Packet Procsess");
		DisconnectSession(sessionID);
		break;
	}
}

void DummyClient::InsertClient(SESSION_ID sessionID, ClientSession *pClient) {
	_clientMap.emplace(::make_pair(sessionID, pClient));
}

void DummyClient::RemoveClient(SESSION_ID sessionID) {
	auto iter = _clientMap.find(sessionID);
	if (iter == _clientMap.end()) {
		return;
	}
	ClientSession *pClient = iter->second;
	_clientMap.erase(iter);

	pClient->_isLogin = false;
	pClient->_data = 0;

	_clientPool.Free(pClient);
}

ClientSession *DummyClient::FindClient(SESSION_ID sessionID) {
	auto iter = _clientMap.find(sessionID);
	if (iter == _clientMap.end()) {
		return nullptr;
	}
	return iter->second;
}
