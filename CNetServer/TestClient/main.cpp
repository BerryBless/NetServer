#include "pch.h"
#include "DummyClient.h"
int main() {
	DummyClient monitor(L"MonitoringToolConfig.ini");
	monitor.Connect();
	Sleep(INFINITE);
	return 0;
}