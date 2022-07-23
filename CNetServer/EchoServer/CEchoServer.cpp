#include "pch.h"
#include "CEchoServer.h"
#include <conio.h>
#include <time.h>
#include <stdio.h>
#include "Profiler.h"

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

void CEchoServer::OnTimeout(SESSION_ID SessionID)
{
	printf_s("OnTimeout :: %lld\n", SessionID);
	Disconnect(SessionID);
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
	
	int printTick = 0;

	for (;;) {
		if (_kbhit()) {
			char cmd = _getch();
			if (cmd == 'Q' || cmd == 'q') {
				
				Quit();
				break;
			}
			if (cmd == 'P' || cmd == 'p') {
				PRO_PRINT(L"CLanserver_PROFILE.log");
				PrintFileMonitor();
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
		// Monitor
		Sleep(1000);
		_hardMoniter.UpdateHardWareTime();
		_procMonitor.UpdateProcessTime();
		// profile
		if (printTick >= 300) {
			PrintFileMonitor();
			printTick = 0;
		}else
			PrintMonitor(stdout);
		printTick++;
	}
}

void CEchoServer::PrintMonitor(FILE * outFP) {
	MoniteringInfo _monitor = GetMoniteringInfo();
	fwprintf_s(outFP ,L"\n\
====================================\n\
Start Time [%02d/%02d/%02d %02d:%02d:%02d]\n",
_timeFormet.tm_mon + 1, _timeFormet.tm_mday, (_timeFormet.tm_year + 1900) % 100, _timeFormet.tm_hour, _timeFormet.tm_min, _timeFormet.tm_sec);

	fwprintf_s(outFP ,L"\n\
-----------------------------------\n\
_PACKET_\n\
send packet TPS\t[%lld]\n\
recv packet TPS\t[%lld]\n",
_monitor._sendPacketCount, _monitor._recvPacketCount);
	fwprintf_s(outFP, L"\n\
-----------------------------------\n\
_TOTAL_\n\
packet\t\t\t[%lld]\n\
send processed Byte\t[%lld]\n\
accept Count\t\t[%lld]\n\
disconnect Count\t[%lld]\n",
_monitor._totalPacket, _monitor._totalProecessedBytes, _monitor._totalAcceptSession, _monitor._totalReleaseSession);
	fwprintf_s(outFP, L"\n\
----------------------------------- \n\
_MEMORY_\n\
Available\t[%lluMb]\n\
NPPool\t\t[%lluMb]\n\
Private Mem\t[%lluKb]\n",
_hardMoniter.AvailableMemoryMBytes(), _hardMoniter.NonPagedPoolMBytes(), _procMonitor.PrivateMemoryKBytes());
	fwprintf_s(outFP, L"\n\
----------------------------------- \n\
PROCESS\t[T %.1llf%% K %.1llf%% U %.1llf%%] \n\
CPU\t[T %.1llf%% K %.1llf%% U %.1llf%%]\n", 
_procMonitor.ProcessTotal(),_procMonitor.ProcessKernel(), _procMonitor.ProcessUser(),_hardMoniter.ProcessorTotal(), _hardMoniter.ProcessorKernel(), _hardMoniter.ProcessorUser());
;
}

void CEchoServer::PrintFileMonitor()
{
	WCHAR FILENAME[128] = L"";
	// timestemp
	tm t;
	time_t now;
	// timestemp
	time(&now);
	localtime_s(&t, &now);
#ifdef dfPROFILER
	// PROFILE
	swprintf_s(FILENAME, 50, L"Profile/%02d%02d%02d_%02d%02d%02d_PROFILE.log",
		t.tm_mon + 1, t.tm_mday, (t.tm_year + 1900) % 100,
		t.tm_hour, t.tm_min, t.tm_sec);
	PRO_PRINT(FILENAME);
#endif // dfPROFILER

	swprintf_s(FILENAME, 50, L"Monitor/%02d%02d%02d_%02d%02d%02d_SERVER_MONITOR.log",
		t.tm_mon + 1, t.tm_mday, (t.tm_year + 1900) % 100,
		t.tm_hour, t.tm_min, t.tm_sec);
	FILE *fp = stdout;
	_wfopen_s(&fp, FILENAME, L"a+");
	fseek(fp, 0, SEEK_END);
	PrintMonitor(fp);
	fclose(fp);
}

void CEchoServer::EchoProc(SESSION_ID sessionID, CPacket *pPacket) {
	PRO_BEGIN(L"Content_Send");
	SendPacket(sessionID, pPacket);
	PRO_END(L"Content_Send");
}

void CEchoServer::LockMap() {
}

void CEchoServer::UnlockMap() {
}
