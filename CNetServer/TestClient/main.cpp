#include "pch.h"
#include "MornitoringClient.h"
int main() {
	MornitoringClient monitor;
	monitor.ConnectMonitor();
	while (1);
	return 0;
}