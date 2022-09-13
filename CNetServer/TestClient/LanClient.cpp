#include "pch.h"
#include "LanClient.h"

LanClient::LanClient() : CClient(false){
	CClient::Start(1, 1, FALSE, 50);
}

LanClient::~LanClient() {
	CClient::Quit();
}

void LanClient::SendTest() {
}

void LanClient::Connect() {
	CClient::Connect(L"127.0.0.1", 6000);
}

void LanClient::OnEnterServer(SESSION_ID sessionID) {
	Player *pPlayer = new Player;
	pPlayer->_isLogin = false;
	pPlayer->_data = 0;
	pPlayer->_ID = sessionID;
	InsertPlayer(sessionID, pPlayer);
	pPlayer->_data = pPlayer->_ID + 1;
	Packet *pPacket = Packet::AllocAddRef();
	*pPacket << pPlayer->_data;
	SendPacket(pPlayer->_ID, pPacket);
	pPacket->SubRef();
}

void LanClient::OnLeaveServer(SESSION_ID sessionID) {
	RemovePlayer(sessionID);
	Sleep(0);
	this->Connect();
}

void LanClient::OnRecv(SESSION_ID sessionID, Packet *pPacket) {
	Player *pPlayer = FindPlayer(sessionID);
	if (pPlayer == nullptr) return;
	if (pPlayer->_isLogin == false) {
		int64_t value;
		*pPacket >> value;

		if (value == 0x7fffffffffffffff) {
			// printf("pass\n");
			pPlayer->_isLogin = true;
		} else {
			CRASH();
			printf("fail\n");
		}
	} else {
		ULONGLONG test = 0;

		*pPacket >> test;
		if (pPlayer->_data != test) {
			CRASH();
		}

		pPlayer->_data += 1;
		Packet *pSendEcho = Packet::AllocAddRef();
		*pSendEcho << pPlayer->_data;
		SendPacket(sessionID, pSendEcho);
		pSendEcho->SubRef();
		if (pPlayer->_data > pPlayer->_ID) {
			DisconnectSession(pPlayer->_ID);
		}
	}

}

void LanClient::InsertPlayer(SESSION_ID sessionID, Player *pPlayer) {
	_playerMap.emplace(::make_pair(sessionID, pPlayer));
}

void LanClient::RemovePlayer(SESSION_ID sessionID) {
	auto iter = _playerMap.find(sessionID);
	if (iter == _playerMap.end()) {
		return;
	}
	Player *pPlayer = iter->second;
	_playerMap.erase(iter);

	pPlayer->_isLogin = false;
	pPlayer->_data = 0;

	delete pPlayer;
}

LanClient::Player *LanClient::FindPlayer(SESSION_ID sessionID) {
	auto iter = _playerMap.find(sessionID);
	if (iter == _playerMap.end()) {
		return nullptr;
	}
	return iter->second;
}
