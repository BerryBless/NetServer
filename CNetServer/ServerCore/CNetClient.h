#pragma once
#include "RingBuffer.h"
#include "SerializingBuffer.h"
#include "CLogger.h"
#include "ObjectPool.hpp"
#include "CCrashDump.h"
#include "Stack.hpp"
#include "Queue.hpp"
#include "HardWareMoniter.h"
#include "ProcessMoniter.h"

#ifndef CRASH
#define CRASH() do{\
	CLogger::_Log(dfLOG_LEVEL_ERROR, L"///////CRASH : FILE[%s] Line[%d]",__FILEW__,__LINE__);\
	int *nptr = nullptr; *nptr = 1;\
}while(0)
#endif // !CRASH

class CNetClient {
public:
	struct SESSION {

		// IOCP Buffer
		WSAOVERLAPPED _recvOverlapped;
		WSAOVERLAPPED _sendOverlapped;
		RingBuffer _recvQueue;
		Queue<Packet *> _sendQueue;

		// session state
		alignas(64) DWORD _IOcount;
		alignas(64) DWORD _IOFlag;
		alignas(64) DWORD _sendPacketCnt;
		alignas(64) DWORD _isAlive;

		// Network
		SOCKET _sock;
		FD_SET _wset;
		FD_SET _errset;

		SESSION() {
			_IOcount = 0x80000000;
			_IOFlag = 0;
			_sendPacketCnt = 0;
			_isAlive = 0;
			ZeroMemory(&_recvOverlapped, sizeof(WSAOVERLAPPED));
			ZeroMemory(&_sendOverlapped, sizeof(WSAOVERLAPPED));
		}
	};

public:
	CNetClient();
	~CNetClient();
	// ==============================================
	// Client Interface
	// ==============================================
	bool Connect(const WCHAR *serverIP, USHORT serverPort);	// 서버IP
	bool Disconnect();
	bool SendPacket(Packet *pPacket);
	void SetThreadNum(BYTE worker, BYTE active);
	bool Start();
	void Quit();
	bool ConnectServer();

protected:

	virtual void OnEnterJoinServer() = 0; //< 서버와의 연결 성공 후
	virtual void OnLeaveServer() = 0; //< 서버와의 연결이 끊어졌을 때
	virtual void OnRecv(Packet *pPacket) = 0;
	virtual void OnSend(int sendsize) = 0;
	virtual void OnError(int errorcode, const WCHAR *) = 0;


	BOOL DomainToIP(const WCHAR *szDomain, IN_ADDR *pAddr);

private:
	// ==============================================
	// Client IOCP Framework
	// ==============================================

	void BeginThreads();
	static unsigned int __stdcall WorkerThread(LPVOID arg);
	static unsigned int __stdcall MonitorThread(LPVOID arg);


	bool OnGQCS();
	bool SendProc(DWORD transferredSize);
	bool RecvProc(DWORD transferredSize);
	bool NetMonitorProc();
	bool RegisterIocp();

	bool SendPost();
	bool RecvPost(bool isAccept = false);
	bool SetWSABuffer(WSABUF *BufSets, bool isRecv);

	bool IncrementIOCount(int logic);
	bool DecrementIOCount(int logic);

	bool ReleaseSessionProc(int logic);

	void Init();
	void CreateIOCP();
	bool CreateSocket();
	bool SetStartUp();
	bool SetTimeWaitZero();
	bool SetNonBlockSocket();
	bool SetNagle(bool sw);
private:
	// ==============================================
	// LOCK
	// ==============================================
	void Lock() { AcquireSRWLockExclusive(&_lock); }
	void Unlock() { ReleaseSRWLockExclusive(&_lock); }

private:
	SRWLOCK	_lock;
	// ----------------------------------------------
	// Network
	// ----------------------------------------------
	SESSION _client;
	u_short _serverPort;
	WCHAR _serverIP[32];

	// ----------------------------------------------
	// Network State
	// ----------------------------------------------
	long _isRunning = 0;		// 서버가 진행중인가?
	BYTE _NumThreads = 0;		// 몇개의 스레드가 생성되었는가

	// ----------------------------------------------
	// Option
	// ----------------------------------------------
	BYTE _maxRunThreadCount = 0;	// 최대 동시 실행 스레드 수
	BYTE _workerThreadCount = 0;	// 생성할 스레드 수
	bool _isNagle = false;

	// ----------------------------------------------
	// Handle
	// ----------------------------------------------
	HANDLE _hIOCP;				// IOCP핸들
	HANDLE *_hThreads;			// WorkerThread Handle

protected:
	// ==============================================
	// 모니터링
	// ==============================================
	struct MoniteringInfo {
		DWORD								_workerThreadCount;
		DWORD								_runningThreadCount;
		ULONGLONG							_totalPacket;
		ULONGLONG							_totalProecessedBytes;
		ULONGLONG							_recvPacketPerSec;
		ULONGLONG							_sendPacketPerSec;
		ULONGLONG							_queueSize;
		ULONGLONG							_stackSize;
		ULONGLONG							_stackCapacity;
	};


	// 모니터링 변수
	alignas(64) ULONGLONG					_totalPacket;
	alignas(64) ULONGLONG					_recvPacketCalc;
	alignas(64) ULONGLONG					_recvPacketPerSec;
	alignas(64) ULONGLONG					_sendPacketCalc;
	alignas(64) ULONGLONG					_sendPacketPerSec;
	alignas(64) LONGLONG					_totalProcessedBytes;

	void CalcTPS();
	MoniteringInfo GetMoniteringInfo();

	void ResetMonitor();

	// temp
	long incArr[10000];
	long incIdx = 0;
	long decArr[10000];
	long decIdx = 0;
};



