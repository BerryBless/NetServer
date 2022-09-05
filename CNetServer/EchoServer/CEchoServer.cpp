#include "pch.h"
#include "CEchoServer.h"
#include <conio.h>
#include <time.h>
#include <stdio.h>
#include "Profiler.h"



bool CEchoServer::OnConnectionRequest(WCHAR *IPStr, DWORD IP, USHORT Port) {
	return true;
}

void CEchoServer::OnClientJoin(WCHAR *ipStr, DWORD ip, USHORT port, ULONGLONG sessionID) {
	Packet *pPacket = Packet::AllocAddRef();

	pPacket->Clear();


	// 로그인 패킷
	int64_t value = 0x7fffffffffffffff;

	(*pPacket) << value;

	SendPacket(sessionID, pPacket);

	pPacket->SubRef(6666);
}

void CEchoServer::OnClientLeave(SESSION_ID SessionID) {
	int idx = SessionID >> (8 * 6);
	//if (idx == 0) CRASH();
}

void CEchoServer::OnRecv(SESSION_ID SessionID, Packet *pPacket) {

	EchoProc(SessionID, pPacket);
}

void CEchoServer::OnSend(SESSION_ID SessionID) {
}

void CEchoServer::OnError(int errorcode, const WCHAR *log) {
	//wprintf_s(L"%d :: %s", errorcode, log);
}

void CEchoServer::OnTimeout(SESSION_ID SessionID) {
	printf_s("OnTimeout :: %lld\n", SessionID);
	DisconnectSession(SessionID);
}

bool CEchoServer::BeginServer(u_long IP, u_short port, BYTE workerThreadCount, BYTE maxRunThreadCount, BOOL nagle, u_short maxConnection) {
	if (Start(IP, port, workerThreadCount, maxRunThreadCount, nagle, maxConnection) == false)
		return false;

	time(&_startTime);
	localtime_s(&_timeFormet, &_startTime);

	KeyCheck();
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
		} else
			PrintMonitor(stdout);
		printTick++;
	}
}

void CEchoServer::PrintMonitor(FILE *outFP) {
	MoniteringInfo monitor = GetMoniteringInfo();
	fwprintf_s(outFP, L"=============================INFO=====================================\n");
	fwprintf_s(outFP, L"Start Time [%02d/%02d/%02d %02d:%02d:%02d]\n",
_timeFormet.tm_mon + 1, _timeFormet.tm_mday, (_timeFormet.tm_year + 1900) % 100, _timeFormet.tm_hour, _timeFormet.tm_min, _timeFormet.tm_sec);
	fwprintf_s(outFP, L"Worker Thread Count[%d]\t\tRunning Thread Count[%d]\n",
		monitor._workerThreadCount, monitor._runningThreadCount);
	fwprintf_s(outFP, L"Packet Queue Size [%lld]\t Packet Queue AVG[%lld]\n",
		monitor._queueSize, monitor._queueSizeAvg);
	fwprintf_s(outFP, L"Currnt Session Count[%lld]\n",
		monitor._sessionCnt);
	fwprintf_s(outFP, L"\n\
-----------------------------TPS--------------------------------------\n\
Accept TPS\t[%lld]\n\
send packet TPS\t[%lld]\n\
recv packet TPS\t[%lld]\n\
send byte bps\t[%lld]\n",
monitor._acceptPerSec, monitor._sendPacketPerSec,  monitor._recvPacketPerSec, monitor._sendBytePerSec);
	fwprintf_s(outFP, L"\n\
----------------------------TOTAL-------------------------------------\n\
packet\t\t[%lld]\n\
sended Byte\t[%lld]\n\
accept Count\t[%lld]\n\
disconnect Count[%lld]\n",
monitor._totalPacket, monitor._totalProecessedBytes, monitor._totalAcceptSession, monitor._totalReleaseSession);
	fwprintf_s(outFP, L"\n\
----------------------------MEMORY------------------------------------ \n\
Available\t[%lluMb]\n\
NPPool\t\t[%lluMb]\n\
Private Mem\t[%lluKb]\n",
_hardMoniter.AvailableMemoryMBytes(), _hardMoniter.NonPagedPoolMBytes(), _procMonitor.PrivateMemoryKBytes());
	fwprintf_s(outFP, L"\n\
---------------------------CORE USAGE--------------------------------- \n\
PROCESS\t[T %.1llf%% K %.1llf%% U %.1llf%%] \n\
CPU\t[T %.1llf%% K %.1llf%% U %.1llf%%]\n",
_procMonitor.ProcessTotal(), _procMonitor.ProcessKernel(), _procMonitor.ProcessUser(),
_hardMoniter.ProcessorTotal(), _hardMoniter.ProcessorKernel(), _hardMoniter.ProcessorUser());

	AverageMonitor(monitor);
	PrintAverage(outFP);
}

void CEchoServer::PrintAverage(FILE *outFP) {
	fwprintf_s(outFP, L"\n----------------------------AVERAGE-----------------------------------\n");
	fwprintf_s(outFP, L"after %lld second\n", _avgTotal);
	fwprintf_s(outFP, L"Accept TPS[%lld] send packet TPS[%lld] recv packet TPS[%lld]\n",
		_avgMonitor._acceptPerSec / _avgTotal,
		_avgMonitor._recvPacketPerSec / _avgTotal,
		_avgMonitor._sendPacketPerSec / _avgTotal);

	fwprintf_s(outFP, L"Available[%lluMb] NPPool[%lluMb] Private Mem[%lluKb]\n",
		_avgMonitor._availableMemory / _avgTotal,
		_avgMonitor._NPPool / _avgTotal,
		_avgMonitor._privateMemory / _avgTotal);

	fwprintf_s(outFP, L"PROCESS\t[T %.1llf%% K %.1llf%% U %.1llf%%]\n",
		_avgMonitor._procCPUTotal / (double) _avgTotal,
		_avgMonitor._procCPUKernel / (double) _avgTotal,
		_avgMonitor._procCPUUser / (double) _avgTotal);

	fwprintf_s(outFP, L"CPU\t[T %.1llf%% K %.1llf%% U %.1llf%%]\n",
		_avgMonitor._hardCPUTotal / (double) _avgTotal,
		_avgMonitor._hardCPUKernel / (double) _avgTotal,
		_avgMonitor._hardCPUUser / (double) _avgTotal);
}

void CEchoServer::PrintFileMonitor() {
	WCHAR FILENAME[128] = L"";
	// timestemp
	tm t;
	time_t now;
	// timestemp
	time(&now);
	localtime_s(&t, &now);
#ifdef dfPROFILER
	// PROFILE
	swprintf_s(FILENAME, 128, L"ServerLog/Profile/%02d%02d%02d_%02d%02d%02d_CHAT_PROFILE.log",
		t.tm_mon + 1, t.tm_mday, (t.tm_year + 1900) % 100,
		t.tm_hour, t.tm_min, t.tm_sec);
	PRO_PRINT(FILENAME);
#endif // dfPROFILER
	swprintf_s(FILENAME, 128, L"ServerLog/MonitorLog/%02d%02d%02d_%02d%02d%02d_CHAT_SERVER_MONITOR.log",
		t.tm_mon + 1, t.tm_mday, (t.tm_year + 1900) % 100,
		t.tm_hour, t.tm_min, t.tm_sec);
	FILE *fp = stdout;
	_wfopen_s(&fp, FILENAME, L"a+");
	fseek(fp, 0, SEEK_END);
	PrintMonitor(fp);
	fclose(fp);
}

void CEchoServer::AverageMonitor(MoniteringInfo monitor) {
	_avgMonitor._acceptPerSec += monitor._acceptPerSec;
	_avgMonitor._recvPacketPerSec += monitor._recvPacketPerSec;
	_avgMonitor._sendPacketPerSec += monitor._sendPacketPerSec;

	_avgMonitor._availableMemory += _hardMoniter.AvailableMemoryMBytes();
	_avgMonitor._NPPool += _hardMoniter.NonPagedPoolMBytes();
	_avgMonitor._privateMemory += _procMonitor.PrivateMemoryKBytes();


	_avgMonitor._procCPUTotal += _procMonitor.ProcessTotal();
	_avgMonitor._procCPUKernel += _procMonitor.ProcessKernel();
	_avgMonitor._procCPUUser += _procMonitor.ProcessUser();

	_avgMonitor._hardCPUTotal += _hardMoniter.ProcessorTotal();
	_avgMonitor._hardCPUKernel += _hardMoniter.ProcessorKernel();
	_avgMonitor._hardCPUUser += _hardMoniter.ProcessorUser();

	++_avgTotal;


}

void CEchoServer::EchoProc(SESSION_ID sessionID, Packet *pPacket) {
	PRO_BEGIN(L"Content_Send");
	SendPacket(sessionID, pPacket);
	PRO_END(L"Content_Send");
}

void CEchoServer::LockMap() {
}

void CEchoServer::UnlockMap() {
}
