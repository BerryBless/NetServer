#include "pch.h"
#include "CChatServer.h"


int main() {
	CChatServer server;
	server.BeginServer(L"ChatserverConfig.ini");
	server.CommandWait();
	return 0;
}