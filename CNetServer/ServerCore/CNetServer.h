#pragma once

#include "CRingBuffer.h"
#include "CPacket.h"
#include "CLogger.h"
#include "ObjectPool_TLS.hpp"
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

#define df_SENDTHREAD

// ----------------------------------------------
// SESSION_ID
// ID가 하나씩 증가하며 항상 고유키
// 0은 사용이 되지않는 ID
// ----------------------------------------------

class CNetServer {
public:
	struct SESSION {
		SESSION_ID _ID;

		// IOCP Buffer
		WSAOVERLAPPED _recvOverlapped;
		WSAOVERLAPPED _sendOverlapped;
		CRingBuffer _recvQueue;
		Queue<CPacket *> _sendQueue;

		// session information
		SOCKET _sock;
		ULONG _IP;
		USHORT _port;
		WCHAR _IPStr[20];

		// session lock
		SRWLOCK _lock;

		// session state
		DWORD _lastRecvdTime;
		alignas(64) DWORD _IOcount;
		alignas(64) DWORD _IOFlag;
		alignas(64) DWORD _sendPacketCnt;
		SESSION() {
			_ID = 0;
			_IOcount = 0;
			_IOFlag = 0;
			_sendPacketCnt = 0;
			_sock = 0;
			_IP = 0;
			_port = 0;
			ZeroMemory(&_recvOverlapped, sizeof(WSAOVERLAPPED));
			ZeroMemory(&_sendOverlapped, sizeof(WSAOVERLAPPED));
		}
	};
public:
	CNetServer();
	~CNetServer();
protected:
	// ==============================================
	// Server Interface
	// ==============================================
	bool Start(u_long IP, u_short port, BYTE workerThreadCount, BYTE maxRunThreadCount, BOOL nagle, u_short maxConnection);
	bool Start(wchar_t *wsConfigPath);
	void Quit();
	ULONGLONG GetSessionCount() { return _curSessionCount; }
	bool Disconnect(SESSION_ID SessionID);
	bool SendPacket(SESSION_ID SessionID, CPacket *pPacket);


	virtual bool OnConnectionRequest(WCHAR* IPstr, u_long IP, u_short Port) = 0; // TODO IP주소 string
	virtual void OnClientJoin(SESSION_ID SessionID) = 0;
	virtual void OnClientLeave(SESSION_ID SessionID) = 0;
	virtual void OnRecv(SESSION_ID SessionID, CPacket *pPacket) = 0;
	virtual void OnError(int errorcode, const WCHAR *log) = 0;
	virtual void OnTimeout(SESSION_ID SessionID) = 0;

	BOOL DomainToIP(const WCHAR *szDomain, IN_ADDR *pAddr);
	void SetTimeoutTime(DWORD ms) { _timeoutMillisec = ms; }

private:
	// ==============================================
	// Server IOCP Framework
	// ==============================================
	bool CreateListenSocket();
	void BeginThreads();
	static unsigned int __stdcall WorkerThread(LPVOID arg);
	static unsigned int __stdcall AcceptThread(LPVOID arg);
	static unsigned int __stdcall MonitorThread(LPVOID arg);
	static unsigned int __stdcall TimeOutThread(LPVOID arg);
#ifdef df_SENDTHREAD
	static unsigned int __stdcall SendThread(LPVOID arg);
#endif // df_SENDTHREAD


	bool OnGQCS();
	bool SendProc(SESSION *pSession, DWORD transferredSize);
	bool RecvProc(SESSION *pSession, DWORD transferredSize);
	bool AcceptProc();
	bool NetMonitorProc();
	bool TimeOutProc();
#ifdef df_SENDTHREAD
	bool SendThreadProc();
#endif

	bool SendPost(SESSION *pSession, int logic);
	bool RecvPost(SESSION *pSession, int logic, bool isAccept = false);
	bool SetWSABuffer(WSABUF *BufSets, SESSION *pSession, bool isRecv, int logic);
	SESSION *GetSessionAddIORef(SESSION_ID sessionID, DWORD logic);

	void SessionSubIORef(SESSION *pSession, DWORD logic);
	bool ReleaseSession(SESSION *pSession, int logic);

private:
	// ==============================================
	// Session Management
	// ==============================================

	SESSION *CreateSession(SOCKET sock, SOCKADDR_IN addr);
	SESSION_ID GenerateSessionID();
	USHORT SessionIDtoIndex(SESSION_ID sessionID);
	SESSION *FindSession(SESSION_ID sessionID);
	void InitializeIndex();

	inline void GetStringIP(WCHAR *str, sockaddr_in &addr) {
		wsprintf(str, L"%d.%d.%d.%d", addr.sin_addr.S_un.S_un_b.s_b1, addr.sin_addr.S_un.S_un_b.s_b2, addr.sin_addr.S_un.S_un_b.s_b3, addr.sin_addr.S_un.S_un_b.s_b4);
	}
private:
	// ==============================================
	// LOCK
	// ==============================================


	void SessionLock(SESSION *pSession);
	void SessionUnlock(SESSION *pSession);

	void SessionContainerLock();
	void SessionContainerUnlock();

private:
	// ==============================================
	// Server Variable
	// ==============================================

	// ----------------------------------------------
	// Listen Socket Info
	// ----------------------------------------------
	u_short _Port;
	u_long _bindIP;
	SOCKET _listensock;

	// ----------------------------------------------
	// Option
	// ----------------------------------------------
	BYTE _maxRunThreadCount = 0;	// 최대 동시 실행 스레드 수
	BYTE _workerThreadCount = 0;	// 생성할 스레드 수
	u_short _maxConnection = 0;		// 최대 동접자 수
	bool _isNagle = false;
	DWORD _timeoutMillisec;

	// ----------------------------------------------
	// Network State
	// ----------------------------------------------
	bool _isRunning = false;	// 서버가 진행중인가?
	BYTE _NumThreads = 0;		// 몇개의 스레드가 생성되었는가

	// ----------------------------------------------
	// Handle
	// ----------------------------------------------
	HANDLE _hIOCP;				// IOCP핸들
	HANDLE *_hThreads;			// WorkerThread Handle

	// ----------------------------------------------
	// Session Container 
	// ----------------------------------------------
	SESSION *_sessionContainer;
	Stack<USHORT> _emptyIndex;
	SESSION_ID _IDGenerater = 1;	// 세션 ID생성기, 0 : 삭제된 세션의 ID
	SRWLOCK	_sessionContainerLock;

protected:
	// ==============================================
	// 모니터링
	// ==============================================
	struct MoniteringInfo {
		DWORD								_workerThreadCount;
		DWORD								_runningThreadCount;
		ULONGLONG							_sessionCnt;
		ULONGLONG							_totalPacket;
		ULONGLONG							_totalProecessedBytes;
		ULONGLONG							_totalAcceptSession;
		ULONGLONG							_totalReleaseSession;
		ULONGLONG							_recvPacketPerSec;
		ULONGLONG							_sendPacketPerSec;
		ULONGLONG							_acceptPerSec;
		ULONGLONG							_queueSize;
		ULONGLONG							_queueSizeAvg;
		ULONGLONG							_queueCapacity;
		ULONGLONG							_maxCapacity;
		ULONGLONG							_stackSize;
		ULONGLONG							_stackCapacity;
	};


	// 모니터링 변수
	alignas(64) ULONGLONG					_curSessionCount;
	alignas(64) ULONGLONG					_totalPacket;
	alignas(64) ULONGLONG					_recvPacketCalc;
	alignas(64) ULONGLONG					_recvPacketPerSec;
	alignas(64) ULONGLONG					_sendPacketCalc;
	alignas(64) ULONGLONG					_sendPacketPerSec;
	alignas(64) LONGLONG					_totalProcessedBytes;
	alignas(64) ULONGLONG					_acceptCalc;
	alignas(64) ULONGLONG					_acceptPerSec;
	alignas(64) ULONGLONG					_totalAcceptSession;
	alignas(64) ULONGLONG					_totalDisconnectSession;

	void CalcTPS();
	MoniteringInfo GetMoniteringInfo();

	void ResetMonitor();

};


//////////////////////////////////////////////////////////////////////////////
// logic - 디버깅 가능하게
//////////////////////////////////////////////////////////////////////////////
#define dfLOGIC_WORKER			10000
#define dfLOGIC_ACCEPT			20000
#define dfLOGIC_CPMPLETE_SEND	2000
#define dfLOGIC_CPMPLETE_RECV	3000
#define dfLOGIC_SEND_PACKET		6000
#define dfLOGIC_DISCONNECT		7000
#define dfLOGIC_DECREMENT_IO	300
#define dfLOGIC_INCREMENT_IO	200
#define dfLOGIC_RELEASE_SESSION 1
#define dfLOGIC_SET_BUFFER		2

