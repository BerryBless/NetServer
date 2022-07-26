#include "CEchoServer.h"

bool CEchoServer::OnConnectionRequest(u_long IP, u_short Port) {
	return true;
}

void CEchoServer::OnClientJoin(SESSION_ID SessionID) {
	CPacket *pPacket = g_packetPool.Alloc();

	// ·Î±×ÀÎ ÆÐÅ¶
	int64_t value = 0x7fffffffffffffff;
	
	(*pPacket) << value;

	SendPacket(SessionID, pPacket);
}

void CEchoServer::OnClientLeave(SESSION_ID SessionID) {
}

void CEchoServer::OnRecv(SESSION_ID SessionID, CPacket *pPacket) {
	SendPacket(SessionID, pPacket);
	//EchoProc(SessionID, packet);
}

void CEchoServer::OnError(int errorcode, const WCHAR *log) {
	wprintf_s(L"%d :: %s", errorcode, log);
}

bool CEchoServer::BeginServer(u_long IP, u_short port, BYTE workerThreadCount, BYTE maxRunThreadCount, BOOL nagle, u_short maxConnection) {
	if (Start(IP, port, workerThreadCount, maxRunThreadCount, nagle, maxConnection) == false)
		return false;
	KeyCheck();
	WaitForThreadsFin();
	return true;
}

void CEchoServer::KeyCheck() {
	while (true) {
		if (_kbhit()) {
			char cmd = _getch();
			if (cmd == 'Q' || cmd == 'q') {
				Quit();
				break;
			}
			if (cmd == 'W' || cmd == 'w') {

			}
			if (cmd == 'P' || cmd == 'p') {
			}
		}
		// ¶«»§
		Sleep(1000);
		PrintMonitor();
	}
}

void CEchoServer::PrintMonitor() {
	_LOG(dfLOG_LEVEL_ERROR,
		L"\n====================================\n\
send packet TPS[%d]\n\
recv packet TPS[%d]\n\
accept Count[%d]\n\
disconnect Count[%d]\n\
------------------------------------\n",
_monitor._sendPacketTPS, _monitor._recvPacketTPS,  _monitor._acceptCount, _monitor._disconnectCount);
}

void CEchoServer::EchoProc(SESSION_ID sessionID, CPacket *pPacket) {
	SendPacket(sessionID, pPacket);
}

void CEchoServer::LockMap() {
}

void CEchoServer::UnlockMap() {
}
