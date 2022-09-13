#include "pch.h"
#include "CMonitoringServer.h"
#define dfWORKERTHREAD 1
#define dfRUNTHREAD 1
#define dfPORT 11600
#define MAX_SESSION_COUNT 20000


int main() {
	CMonitoringServer monitor;
	monitor.BeginServer(INADDR_ANY, dfPORT, dfWORKERTHREAD, dfRUNTHREAD, FALSE, MAX_SESSION_COUNT);
	monitor.CommandWait();
	return 0;
}