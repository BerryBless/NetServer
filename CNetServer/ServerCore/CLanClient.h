#pragma once
#include "CRingBuffer.h"
#include "CPacket.h"
#include "CLogger.h"
#include "CObjectPool.hpp"
#include "CCrashDump.h"
#include "Stack.hpp"
#include "Queue.hpp"

class CLanClient {
public:
	struct SESSION {

		// IOCP Buffer
		WSAOVERLAPPED _recvOverlapped;
		WSAOVERLAPPED _sendOverlapped;
		CRingBuffer _recvQueue;
		Queue<CPacket *> _sendQueue;

		// session state
		DWORD _IOcount;
		DWORD _enqPacketCnt;
		DWORD _sendedPacketCnt;
		BOOL _isSend;

		// Network
		SOCKET _sock;
		FD_SET _wset;
		FD_SET _errset;

		SESSION() {
			_IOcount = 0;
			_isSend = false;
			_sendedPacketCnt = 0;
			ZeroMemory(&_recvOverlapped, sizeof(WSAOVERLAPPED));
			ZeroMemory(&_sendOverlapped, sizeof(WSAOVERLAPPED));
		}
	};

public:
	CLanClient();
	~CLanClient();

	bool Connect(const WCHAR *serverIP, USHORT serverPort);	// 서버IP
	bool Disconnect();
	bool SendPacket(CPacket *pPacket);
	void SetThreadNum(BYTE worker, BYTE active);
protected:
	bool Start();
	void Quit();
	bool ConnectServer();


	virtual void OnEnterJoinServer() = 0; //< 서버와의 연결 성공 후
	virtual void OnLeaveServer() = 0; //< 서버와의 연결이 끊어졌을 때
	virtual void OnRecv(CPacket *pPacket) = 0;
	virtual void OnSend(int sendsize) = 0;
	virtual void OnError(int errorcode, const WCHAR *) = 0;



	BOOL DomainToIP(const WCHAR *szDomain, IN_ADDR *pAddr);

private:

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

	bool ReleaseProc(int logic);

	void Init();
	void CreateIOCP();
	bool CreateSocket();
	bool SetStartUp();
	bool SetTimeWaitZero();
	bool SetNonBlockSocket();
	bool SetNagle(bool sw);
private:

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
	bool _isRunning = false;	// 서버가 진행중인가?
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
	// 모니터링
	struct MONITOR {
		// 현재(계산중)
		DWORD _sendPacketCalc;
		DWORD _recvPacketCalc;

		// 결과값
		DWORD _sendPacketTPS;
		DWORD _recvPacketTPS;

		DWORD _acceptCount;
		DWORD _disconnectCount;
	};
	MONITOR _monitor;
};

