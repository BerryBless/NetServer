#pragma once
#include "CLanServer.h"
#include <unordered_map>

class CEchoServer : public CLanServer {

public:
	// virtual
	virtual bool OnConnectionRequest(u_long IP, u_short Port); //< accept 직후

	virtual void OnClientJoin(SESSION_ID SessionID); //< Accept 후 접속처리 완료 후 호출.
	virtual void OnClientLeave(SESSION_ID SessionID); //< Release 후 호출

	virtual void OnRecv(SESSION_ID SessionID, CPacket *packet); //< 패킷 수신 완료 후
		//	virtual void OnSend(SessionID, int sendsize);           < 패킷 송신 완료 후

		//	virtual void OnWorkerThreadBegin();                    < 워커스레드 GQCS 바로 하단에서 호출
		//	virtual void OnWorkerThreadEnd();                      < 워커스레드 1루프 종료 후

	virtual void OnError(int errorcode, const WCHAR *log); // 에러 발생시 유저한테 알려줄곳

	virtual void OnTimeout(SESSION_ID SessionID);
public:
	bool BeginServer(u_long IP, u_short port, BYTE workerThreadCount, BYTE maxRunThreadCount, BOOL nagle, u_short maxConnection);

private:
	void KeyCheck();
	void PrintMonitor(FILE *outFP);
	void PrintAverage(FILE *outFP);
	void PrintFileMonitor();
	void AverageMonitor(MoniteringInfo monitor);

	void EchoProc(SESSION_ID sessionID, CPacket *pPacket);
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

