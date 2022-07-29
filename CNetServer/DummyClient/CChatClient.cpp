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

void CChatClient::Login() {
	CPacket *pPacket = CPacket::AllocAddRef();
	_player._AccountNo = 12;
	wsprintf(_player._ID, L"_ID");
	wsprintf(_player._NickName, L"_NickName");
	sprintf_s(_player._TokenKey, "_TokenKey");

	MakePacketRequestLogin(pPacket, _player._AccountNo, _player._ID, _player._NickName, _player._TokenKey);

	SendPacket(pPacket);

	pPacket->SubRef();
}

void CChatClient::OnEnterJoinServer() {
}

void CChatClient::OnLeaveServer() {
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

void CChatClient::PacketProcResponseSectorMove(CPacket *pPacket) {
}

void CChatClient::PacketProcResponseMessage(CPacket *pPacket) {
}

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

void CChatClient::MakePacketRequestSectorMove(CPacket *pPacket, ACCOUNT_NO no, WORD sectorX, WORD sectorY) {
	pPacket->AddRef();
	CHAT_PACKET_TYPE type = CHAT_PACKET_TYPE::PACKET_CS_CHAT_REQ_SECTOR_MOVE;
	(*pPacket) << ((WORD) type);
	pPacket->PutData((char *) &no, sizeof(ACCOUNT_NO));
	(*pPacket) << sectorX << sectorY;
	pPacket->SubRef();
}

void CChatClient::MakePacketRequestMessage(CPacket *pPacket, ACCOUNT_NO no, WORD msgLen, const WCHAR *message) {
	pPacket->AddRef();
	CHAT_PACKET_TYPE type = CHAT_PACKET_TYPE::PACKET_CS_CHAT_REQ_MESSAGE;
	(*pPacket) << ((WORD) type);
	pPacket->PutData((char *) &no, sizeof(ACCOUNT_NO));
	(*pPacket) << msgLen;
	pPacket->PutData((char *) message, msgLen);
	pPacket->SubRef();
}
