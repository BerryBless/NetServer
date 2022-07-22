#include "pch.h"
#include "CEchoServer.h"

#define dfWORKERTHREAD 4
#define dfRUNTHREAD 2
#define dfPORT 6000
#define MAX_SESSION_COUNT 20000

int main() {
	CEchoServer server;

	server.BeginServer(INADDR_ANY, dfPORT, dfWORKERTHREAD, dfRUNTHREAD, false, MAX_SESSION_COUNT);
	return 0;
}