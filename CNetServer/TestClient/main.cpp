#include "pch.h"
#include "MornitoringClient.h"
int main() {
	MornitoringClient monitor;
	monitor.ConnectMonitor();
	Sleep(INFINITE);
	return 0;
}