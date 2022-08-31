#pragma once

#include "RingBuffer.h"
#include "SerializingBuffer.h"
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

#define df_LOGGING_SESSION_LOGIC 1000
#define dfSESSION_SEND_PACKER_BUFFER_SIZE 200

#define df_SENDTHREAD

// ----------------------------------------------
// SESSION_ID
// ID가 하나씩 증가하며 항상 고유키
// 0은 사용이 되지않는 ID
// ----------------------------------------------

class CLanServer {
public:
	struct SESSION {
		SESSION_ID _ID;

		// IOCP Buffer
		WSAOVERLAPPED _recvOverlapped;
		WSAOVERLAPPED _sendOverlapped;
		RingBuffer _recvQueue;
		Queue<Packet *> _sendQueue;
		Packet *_pSendPacketBufs[dfSESSION_SEND_PACKER_BUFFER_SIZE];

		// session information
		SOCKET _sock;
		DWORD _IP;
		USHORT _port;
		WCHAR _IPStr[20];

		// session lock
		SRWLOCK _lock;

		// session state
		DWORD _lastRecvdTime;
		alignas(64) DWORD _IOcount;
		alignas(64) DWORD _IOFlag;
		alignas(64) DWORD _sendPacketCnt;
		alignas(64) DWORD _isAlive;

#ifdef df_LOGGING_SESSION_LOGIC
		alignas(64) DWORD _IncIndex;
		alignas(64) DWORD _DecIndex;
		int _IncLog[df_LOGGING_SESSION_LOGIC] = { 0 };
		int _DecLog[df_LOGGING_SESSION_LOGIC] = { 0 };
#endif // df_LOGGING_SESSION_LOGIC

		SESSION() {
			_ID = 0;
			_IOcount = 0x80000000;
			_IOFlag = 0;
			_sendPacketCnt = 0;
			_sock = 0;
			_IP = 0;
			_port = 0;
			_isAlive = 0;
			ZeroMemory(&_recvOverlapped, sizeof(WSAOVERLAPPED));
			ZeroMemory(&_sendOverlapped, sizeof(WSAOVERLAPPED));
		}
	};
public:
	CLanServer();
	~CLanServer();
protected:
	// ==============================================
	// Server Interface
	// ==============================================
	bool Start(u_long IP, u_short port, BYTE workerThreadCount, BYTE maxRunThreadCount, BOOL nagle, u_short maxConnection);
	bool Start(const wchar_t *wsConfigPath);
	void Quit();
	ULONGLONG GetSessionCount() { return _curSessionCount; }
	bool DisconnectSession(SESSION_ID SessionID);
	bool SendPacket(SESSION_ID SessionID, Packet *pPacket);


	virtual bool OnConnectionRequest(WCHAR *IPstr, DWORD IP, USHORT Port) = 0; // TODO IP주소 string
	virtual void OnClientJoin(WCHAR *ipStr, DWORD ip, USHORT port, ULONGLONG sessionID) = 0;
	virtual void OnClientLeave(SESSION_ID SessionID) = 0;
	virtual void OnRecv(SESSION_ID SessionID, Packet *pPacket) = 0;
	virtual void OnError(int errorcode, const WCHAR *log) = 0;
	virtual void OnTimeout(SESSION_ID SessionID) = 0;

	BOOL DomainToIP(const WCHAR *szDomain, IN_ADDR *pAddr);
	void SetTimeoutTime(DWORD ms) { _timeoutMillisec = ms; }

private:
	// ==============================================
	// Server IOCP Framework
	// ==============================================
	void Startup();
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
	bool TryAccept(SOCKET &clientSocket, sockaddr_in &clientAddr);
	bool AcceptProc();
	bool NetMonitorProc();
	bool TimeOutProc();
#ifdef df_SENDTHREAD
	bool SendThreadProc();
#endif

	bool SendPost(SESSION *pSession, int logic);
	bool RecvPost(SESSION *pSession, int logic);
	bool SetWSABuffer(WSABUF *BufSets, SESSION *pSession, bool isRecv, int logic);


private:
	// ==============================================
	// Session Management
	// ==============================================
	SESSION *AcquireSession(SESSION_ID sessionID, int logic);
	void ReturnSession(SESSION *pSession, int logic);

	bool ReleaseSession(SESSION *pSession, int logic);

	bool IncrementIOCount(SESSION *pSession, int logic);
	bool DecrementIOCount(SESSION *pSession, int logic);


	SESSION *CreateSession(SOCKET sock, sockaddr_in clientaddr);
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
	CParser *_pConfigData;
	BYTE _maxRunThreadCount;	// 최대 동시 실행 스레드 수
	BYTE _workerThreadCount;	// 생성할 스레드 수
	u_short _maxConnection;		// 최대 동접자 수
	bool _isNagle;
	DWORD _timeoutMillisec;

	// ----------------------------------------------
	// Network State
	// ----------------------------------------------
	bool _isRunning;	// 서버가 진행중인가?
	BYTE _NumThreads;		// 몇개의 스레드가 생성되었는가

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
	SESSION_ID _IDGenerater;	// 세션 ID생성기, 0 : 삭제된 세션의 ID
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
		ULONGLONG							_sendBytePerSec;
		ULONGLONG							_recvBytePerSec;
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
	volatile alignas(64) LONG64				_recvProcessedBytesCalc;
	volatile alignas(64) LONG64				_recvProcessedBytesTPS;
	volatile alignas(64) LONG64				_sendProcessedBytesCalc;
	volatile alignas(64) LONG64				_sendProcessedBytesTPS;
	volatile alignas(64) LONG64				_totalProcessedByte;
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

