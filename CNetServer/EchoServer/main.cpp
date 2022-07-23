#include "pch.h"
#include "CEchoServer.h"
#include "Profiler.h"

#define dfWORKERTHREAD 8
#define dfRUNTHREAD 4
#define dfPORT 6000
#define MAX_SESSION_COUNT 20000

int main() {
	PRO_INIT(dfWORKERTHREAD * 2);

	CEchoServer server;

	server.BeginServer(INADDR_ANY, dfPORT, dfWORKERTHREAD, dfRUNTHREAD, false, MAX_SESSION_COUNT);
	return 0;
}