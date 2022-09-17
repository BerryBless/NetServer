#include "pch.h"
#include "MonitoringClient.h"
int main() {
	MonitoringClient monitor(L"MonitoringToolConfig.ini");
	monitor.ConnectMonitor();
	Sleep(INFINITE);
	return 0;
}