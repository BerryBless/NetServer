#include "pch.h"
#include "CChatClient.h"
#include "ChatServerProtocol.h"
#include "Profiler.h"
#include <conio.h>
#include <time.h>
#include <stdio.h>

CChatClient::CChatClient() {
}

CChatClient::~CChatClient() {
}

void CChatClient::Login(ACCOUNT_NO ano, const WCHAR *ID, const WCHAR *Nick, const char* tokenkey) {
	CPacket *pPacket = CPacket::AllocAddRef();
	_player._AccountNo = ano;
	wcscpy_s(_player._ID, ID_MAX_SIZE, ID);
	wcscpy_s(_player._NickName, NICK_NAME_MAX_SIZE, Nick);
	strcpy_s(_player._TokenKey, TOKEN_KEY_SIZE, tokenkey);

	MakePacketRequestLogin(pPacket, _player._AccountNo, _player._ID, _player._NickName, _player._TokenKey);

	SendPacket(pPacket);

	pPacket->SubRef();
}

void CChatClient::TryMoveSector(WORD sx, WORD sy) {
	CPacket *pPacket = CPacket::AllocAddRef();

	MakePacketRequestSectorMove(pPacket, _player._AccountNo, sx, sy);
	SendPacket(pPacket);

	pPacket->SubRef();
}

void CChatClient::SendChatMessage(const WCHAR *msg) {
	WORD len = wcslen(msg);
	CPacket *pPacket = CPacket::AllocAddRef();

	MakePacketRequestMessage(pPacket, _player._AccountNo, len, msg);
	SendPacket(pPacket);

	pPacket->SubRef();
}

void CChatClient::Disconnect() {
	CNetClient::Disconnect();
}


void CChatClient::OnEnterJoinServer() {
	printf_s("OnEnterJoinServer()\n");
}

void CChatClient::OnLeaveServer() {
	printf_s("OnLeaveServer()\n");
}

void CChatClient::OnRecv(CPacket *pPacket) {
	pPacket->AddRef();

	WORD type;
	(*pPacket) >> type;
	PacketProc(pPacket, type);

	pPacket->SubRef();
}

void CChatClient::OnSend(int sendsize) {
}

void CChatClient::OnError(int errorcode, const WCHAR *) {
}

void CChatClient::PacketProc(CPacket *pPacket, WORD type) {
	pPacket->AddRef();
	switch (type) {
	case PACKET_SC_CHAT_RES_LOGIN:
		PacketProcResponseLogin(pPacket);
		break;
	case PACKET_SC_CHAT_RES_SECTOR_MOVE:
		PacketProcResponseSectorMove(pPacket);
		break;
	case PACKET_SC_CHAT_RES_MESSAGE:
		PacketProcResponseMessage(pPacket);
		break;
	default:
		//TODO error
		break;
	}
	pPacket->SubRef();
}

//PACKET_SC_CHAT_RES_LOGIN
void CChatClient::PacketProcResponseLogin(CPacket *pPacket) {
	pPacket->AddRef();
	BYTE status = FALSE;
	ACCOUNT_NO acno;

	if (pPacket->GetDataSize() < sizeof(status)) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"pPacket->GetDataSize() < sizeof(ACCOUNT_NO)"); // TODO ERROR MSG
		Disconnect();
	}
	(*pPacket) >> status;

	if (pPacket->GetDataSize() < sizeof(ACCOUNT_NO)) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"pPacket->GetDataSize() < sizeof(ACCOUNT_NO)"); // TODO ERROR MSG
		Disconnect();
	}
	(*pPacket).GetData((char *) &acno, sizeof(ACCOUNT_NO));

	pPacket->SubRef();

	printf_s("No[%lld] : [%d]", acno, status);
}

//PACKET_SC_CHAT_RES_SECTOR_MOVE
void CChatClient::PacketProcResponseSectorMove(CPacket *pPacket) {
	ACCOUNT_NO no;
	WORD sx;
	WORD sy;
	pPacket->AddRef();
	if (pPacket->GetDataSize() < (sizeof(no) + sizeof(sx) + sizeof(sy))) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"pPacket->GetDataSize() < (sizeof(no) + sizeof(sx) + sizeof(sy))"); // TODO ERROR MSG
		Disconnect();
	}

	pPacket->GetData((char *) &no, sizeof(ACCOUNT_NO));
	(*pPacket) >> sx >> sy;
	pPacket->SubRef();

	printf_s("\n PACKET_SC_CHAT_RES_SECTOR_MOVE:: %lld (%d, %d)\n", no, sx, sy);
	if (_player._AccountNo != no) {
		CLogger::_Log(dfLOG_LEVEL_ERROR, L"_player._AccountNo != no"); // TODO ERROR MSG
		Disconnect();
	}

	_player._SectorX = sx;
	_player._SectorX = sy;
}

//PACKET_SC_CHAT_RES_MESSAGE
void CChatClient::PacketProcResponseMessage(CPacket *pPacket) {
	Player sender;
	WORD msgLen;
	WCHAR message[MASSAGE_MAX_SIZE];
	pPacket->AddRef();

	pPacket->GetData((char *) &sender._AccountNo, sizeof(ACCOUNT_NO));
	pPacket->GetData((char *) sender._ID, ID_MAX_SIZE);
	pPacket->GetData((char *) sender._NickName, NICK_NAME_MAX_SIZE);
	(*pPacket) >> msgLen;
	pPacket->GetData((char *) message, msgLen * sizeof(WCHAR));
	message[msgLen] = '\0';
	pPacket->SubRef();

	wprintf_s(L"ID [%s]_ Nick[%s] :: [%s]", sender._ID, sender._NickName, message);
}

//PACKET_CS_CHAT_REQ_LOGIN
void CChatClient::MakePacketRequestLogin(CPacket *pPacket, ACCOUNT_NO no, WCHAR *ID, WCHAR *nick, char *token) {
	pPacket->AddRef();
	CHAT_PACKET_TYPE type = CHAT_PACKET_TYPE::PACKET_CS_CHAT_REQ_LOGIN;
	(*pPacket) << ((WORD) type);
	pPacket->PutData((char *) &no, sizeof(ACCOUNT_NO));
	pPacket->PutData((char *) ID, ID_MAX_SIZE);
	pPacket->PutData((char *) nick, NICK_NAME_MAX_SIZE);
	pPacket->PutData((char *) token, TOKEN_KEY_SIZE);

	pPacket->SubRef();
}

//PACKET_CS_CHAT_REQ_SECTOR_MOVE
void CChatClient::MakePacketRequestSectorMove(CPacket *pPacket, ACCOUNT_NO no, WORD sectorX, WORD sectorY) {
	pPacket->AddRef();
	CHAT_PACKET_TYPE type = CHAT_PACKET_TYPE::PACKET_CS_CHAT_REQ_SECTOR_MOVE;
	(*pPacket) << ((WORD) type);
	pPacket->PutData((char *) &no, sizeof(ACCOUNT_NO));
	(*pPacket) << sectorX << sectorY;
	pPacket->SubRef();
}

//PACKET_CS_CHAT_REQ_MESSAGE
void CChatClient::MakePacketRequestMessage(CPacket *pPacket, ACCOUNT_NO no, WORD msgLen, const WCHAR *message) {
	pPacket->AddRef();
	CHAT_PACKET_TYPE type = CHAT_PACKET_TYPE::PACKET_CS_CHAT_REQ_MESSAGE;
	(*pPacket) << ((WORD) type);
	pPacket->PutData((char *) &no, sizeof(ACCOUNT_NO));
	(*pPacket) << msgLen;
	pPacket->PutData((char *) message, msgLen * sizeof(WCHAR));
	pPacket->SubRef();
}
