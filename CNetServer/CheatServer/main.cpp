#include "pch.h"
#include "CChatServer.h"

#define dfWORKERTHREAD 8
#define dfRUNTHREAD 4
#define dfPORT 7897
#define MAX_SESSION_COUNT 20000


int main() {
	CChatServer server;
	server.BeginServer(INADDR_ANY, dfPORT, dfWORKERTHREAD, dfRUNTHREAD, false, MAX_SESSION_COUNT);
	server.CommandWait();
	return 0;
}