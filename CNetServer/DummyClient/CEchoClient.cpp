#include "pch.h"
#include "CEchoClient.h"

CEchoClient::CEchoClient() {
	Start();
}

CEchoClient::~CEchoClient() {
}

void CEchoClient::OnEnterJoinServer() {
}

void CEchoClient::OnLeaveServer() {

}

void CEchoClient::OnRecv(CPacket *pPacket) {
	if (_cnt++ == 0) {
		int64_t value;

		*pPacket >> value;

		if (value == 0x7fffffffffffffff) {
			printf("pass\n");
		} else {
			printf("fail\n");
		}
	} else {
		int test = 0;

		*pPacket >> test;

		//printf("%d\n", test);
	}
}

void CEchoClient::OnSend(int sendsize) {
}

void CEchoClient::OnError(int errorcode, const WCHAR *) {
}
