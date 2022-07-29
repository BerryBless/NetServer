#include "pch.h"
#include "CChatClient.h"

#define SERVER_PORT 7897
#define SERVER_IP L"127.0.0.1"

int main() {
	CChatClient client;
	client.Start();
	client.Connect(SERVER_IP, SERVER_PORT);

	client.Login();
	//client.Login();
	client.TryMoveSector(12,13);
	client.TryMoveSector(1,2);
	client.TryMoveSector(99,2);

	while (true);

	return 0;
}
