#include "pch.h"
#include "CMonitoringServer.h"
#define dfWORKERTHREAD 1
#define dfRUNTHREAD 1
#define dfPORT 11600
#define MAX_SESSION_COUNT 20000


int main() {
	CMonitoringServer monitor;
	monitor.BeginServer(L"MonitoringConfig.ini");
	monitor.CommandWait();
	return 0;
}