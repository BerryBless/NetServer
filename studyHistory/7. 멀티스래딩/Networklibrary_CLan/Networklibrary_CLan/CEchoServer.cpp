#include "CEchoServer.h"
#include <conio.h>
#include <time.h>

bool CEchoServer::OnConnectionRequest(u_long IP, u_short Port) {
	return true;
}

void CEchoServer::OnClientJoin(SESSION_ID SessionID) {
	//* 로그인 패킷
	CPacket *pPacket = CPacket::AllocAddRef();

	pPacket->Clear();


	// 로그인 패킷
	int64_t value = 0x7fffffffffffffff;

	(*pPacket) << value;

	SendPacket(SessionID, pPacket);

	pPacket->SubRef(6666);
	//*/
}

void CEchoServer::OnClientLeave(SESSION_ID SessionID) {
}

void CEchoServer::OnRecv(SESSION_ID SessionID, CPacket *pPacket) {

	EchoProc(SessionID, pPacket);
}

void CEchoServer::OnError(int errorcode, const WCHAR *log) {
	wprintf_s(L"%d :: %s", errorcode, log);
}

bool CEchoServer::BeginServer(u_long IP, u_short port, BYTE workerThreadCount, BYTE maxRunThreadCount, BOOL nagle, u_short maxConnection) {
	if (Start(IP, port, workerThreadCount, maxRunThreadCount, nagle, maxConnection) == false)
		return false;

	time(&_startTime);
	localtime_s(&_timeFormet, &_startTime);

	KeyCheck();
	WaitForThreadsFin();
	return true;
}

void CEchoServer::KeyCheck() {
	for(;;) {
		if (_kbhit()) {
			char cmd = _getch();
			if (cmd == 'Q' || cmd == 'q') {
				Quit();
				break;
			}
			if (cmd == 'C' || cmd == 'c') {
				CRASH();
			}
			if (cmd == '1') {
				wprintf_s(L"CHANGE LOG LEVEL :: DEBUG\n");
				CLogger::SetLogLevel(dfLOG_LEVEL_DEBUG);
			}
			if (cmd == '2') {
				wprintf_s(L"CHANGE LOG LEVEL :: ERROR\n");
				CLogger::SetLogLevel(dfLOG_LEVEL_ERROR);
			}
			if (cmd == '3') {
				wprintf_s(L"CHANGE LOG LEVEL :: NOTICE\n");
				CLogger::SetLogLevel(dfLOG_LEVEL_NOTICE);
			}
		}
		// 땜빵
		Sleep(1000);
		PrintMonitor();
	}
}

void CEchoServer::PrintMonitor() {
	wprintf_s(
		L"\n\
====================================\n\
Start Time [%02d/%02d/%02d %02d:%02d:%02d]\n\
-----------------------------------\n\
send packet TPS[%d]\n\
recv packet TPS[%d]\n\
accept Count[%d]\n\
disconnect Count[%d]\n\
------------------------------------\n",
_timeFormet.tm_mon + 1, _timeFormet.tm_mday, (_timeFormet.tm_year + 1900) % 100, _timeFormet.tm_hour, _timeFormet.tm_min, _timeFormet.tm_sec,
_monitor._sendPacketTPS, _monitor._recvPacketTPS, _monitor._acceptCount, _monitor._disconnectCount);
}

void CEchoServer::EchoProc(SESSION_ID sessionID, CPacket *pPacket) {
	SendPacket(sessionID, pPacket);
}

void CEchoServer::LockMap() {
}

void CEchoServer::UnlockMap() {
}
