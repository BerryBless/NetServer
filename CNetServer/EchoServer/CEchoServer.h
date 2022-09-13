#pragma once
#include "CServer.h"
//#include "CNetServer.h"
#include <unordered_map>

class CEchoServer : public CServer {
//class CEchoServer : public CNetServer {

public:
	// virtual
	virtual bool OnConnectionRequest(WCHAR *IPStr, DWORD IP, USHORT Port); //< accept 직후
	virtual void OnClientJoin(WCHAR *ipStr, DWORD ip, USHORT port, ULONGLONG sessionID); //< Accept 후 접속처리 완료 후 호출.
	virtual void OnClientLeave(SESSION_ID sessionID); //< Release 후 호출
	virtual void OnRecv(SESSION_ID sessionID, Packet *packet); //< 패킷 수신 완료 후
	virtual void OnSend(SESSION_ID sessionID); //< 패킷 수신 완료 후
	virtual void OnError(int errorcode, const WCHAR *log); // 에러 발생시 유저한테 알려줄곳
	virtual void OnTimeout(SESSION_ID sessionID);


public:
	bool BeginServer(u_long IP, u_short port, BYTE workerThreadCount, BYTE maxRunThreadCount, BOOL nagle, u_short maxConnection);

private:
	void KeyCheck();
	void PrintMonitor(FILE *outFP);
	void PrintAverage(FILE *outFP);
	void PrintFileMonitor();
	void AverageMonitor(MoniteringInfo monitor);

	void EchoProc(SESSION_ID sessionID, Packet *pPacket);
	void LockMap();
	void UnlockMap();
private:
	std::unordered_map<SESSION_ID, SOCKET> _clientMap;
	SRWLOCK  _SessionLock;

	// server start timestemp
	tm _timeFormet;
	time_t _startTime;

	HardWareMoniter _hardMoniter;
	ProcessMoniter _procMonitor;


private:
	struct AVGMonitor {

		ULONGLONG							_acceptPerSec = 0;
		ULONGLONG							_recvPacketPerSec = 0;
		ULONGLONG							_sendPacketPerSec = 0;


		ULONGLONG							_availableMemory = 0;
		ULONGLONG							_NPPool = 0;
		ULONGLONG							_privateMemory = 0;

		double								_procCPUTotal = 0;
		double								_procCPUKernel = 0;
		double								_procCPUUser = 0;

		double								_hardCPUTotal = 0;
		double								_hardCPUKernel = 0;
		double								_hardCPUUser = 0;
	};

	AVGMonitor _avgMonitor;
	LONG64	_avgTotal = 0;
};

