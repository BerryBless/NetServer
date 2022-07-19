#include "pch.h"
#include "CEchoServer.h"

#define dfWORKERTHREAD 16
#define dfRUNTHREAD 8
#define dfPORT 6000

int main() {
	CEchoServer server;

	server.BeginServer(INADDR_ANY, dfPORT, dfWORKERTHREAD, dfRUNTHREAD, false, 200);
	return 0;
}