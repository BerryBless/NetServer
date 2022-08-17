#include "pch.h"
#include "CEchoClient.h"

LONG g_sendCalc = 0;
LONG g_recvCalc = 0;

CEchoClient::CEchoClient() {
	Start();
	ZeroMemory(_testPacket, df_SENDOVER);
}

CEchoClient::~CEchoClient() {
}

void CEchoClient::OnEnterJoinServer() {
}

void CEchoClient::OnLeaveServer() {
	_login = 0;
}

void CEchoClient::OnRecv(CPacket *pPacket) {
	pPacket->AddRef();
	if (_login == 0) {
		int64_t value;

		*pPacket >> value;

		if (value == 0x7fffffffffffffff) {
			_login = 2;
		} else {
			CRASH();
			printf("fail\n");
		}
	} else {
		recvCheck(pPacket);
	}
	pPacket->SubRef(1);
	InterlockedIncrement(&g_recvCalc);

}

void CEchoClient::OnSend(int sendsize) {
}

void CEchoClient::OnError(int errorcode, const WCHAR *) {
}

void CEchoClient::Test() {
	DWORD startTime = timeGetTime();
	DWORD checkTime;
	sendTest();
	while (_wait) {
		checkTime = timeGetTime();
		if (checkTime - startTime >= 500) {
			//CRASH();
		}
		recvTest();
	}
}

void CEchoClient::sendTest() {
	for (int i = 0; i < df_SENDOVER; i++) {
		int index = _cnt % df_SENDOVER;
		_testPacket[index]._code = df_PACKET_CODE;
		_testPacket[index]._cnt = _cnt;
		_testPacket[index]._sendTime = timeGetTime();
		_testPacket[index]._recved = FALSE;
		_cnt++;

		CPacket *pPacket = CPacket::AllocAddRef();
		(*pPacket) << _testPacket[index]._code;
		(*pPacket) << _testPacket[index]._cnt;
		(*pPacket) << _testPacket[index]._sendTime;

		SendPacket(pPacket);
		InterlockedIncrement(&g_sendCalc);

		pPacket->SubRef(2);
	}
	_wait = true;
}

void CEchoClient::recvTest() {
	for (int i = 0; i < df_SENDOVER; i++) {
		if (_testPacket[i]._recved == FALSE)
			return;
	}
	_wait = false;
}

void CEchoClient::recvCheck(CPacket *pPacket) {
	pPacket->AddRef();

	Protocol recvPacket;
	int index;
	(*pPacket) >> recvPacket._code;
	if (recvPacket._code != df_PACKET_CODE) CRASH();

	(*pPacket) >> recvPacket._cnt;
	index = recvPacket._cnt % df_SENDOVER;
	if (_testPacket[index]._cnt != recvPacket._cnt) CRASH();

	(*pPacket) >> recvPacket._sendTime;
	DWORD aTime = timeGetTime();
	_totalRTT += (aTime - recvPacket._sendTime);
	++_countRTT;


	_testPacket[index]._recved = TRUE;

	pPacket->SubRef(3);

}
