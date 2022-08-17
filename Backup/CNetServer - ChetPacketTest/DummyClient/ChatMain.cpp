#include "pch.h"
#include "CChatClient.h"

#define DUMMY_MAX 30
#define SERVER_PORT 7897
#define SERVER_IP L"127.0.0.1"

	CChatClient client[DUMMY_MAX];
int main() {

	for (int i = 0; i < DUMMY_MAX; ++i) {

		client[i].Start();
		client[i].Connect(SERVER_IP, SERVER_PORT);
	}
	for (int i = 0; i < DUMMY_MAX; ++i) {
		Sleep(1);
		client[i].Login(i, L"ID1", L"nick1", "token1");
	}
	for (int i = 0; i < DUMMY_MAX; ++i) {
		client[i].TryMoveSector(12+i, 14+i);
	}
	client[0].SendChatMessage(L"Hello1");



	//client.TryMoveSector(99,2);


	Sleep(100);
	for (int i = 0; i < DUMMY_MAX; ++i) {
		client[i].Disconnect();
	}

	while (true);

	return 0;
}
