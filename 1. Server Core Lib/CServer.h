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
	_LOG(dfLOG_LEVEL_ERROR, L"///////CRASH : FILE[%s] Line[%d]",__FILEW__,__LINE__);\
	int *nptr = nullptr; *nptr = 1;\
}while(0)
#endif // !CRASH

//#define df_SENDTHREAD // 센드 스레드 적용

// ----------------------------------------------
// SESSION_ID
// ID가 하나씩 증가하며 항상 고유키
// 0은 사용이 되지않는 ID
// ----------------------------------------------

class CServer {
public:
	CServer(bool isEncryption = false);
	~CServer();
protected:
	// ==============================================
	// Server Interface
	// ==============================================
	bool Start(u_long IP, u_short port, BYTE workerThreadCount, BYTE maxRunThreadCount, BOOL nagle, u_short maxConnection);
	void Quit();
	ULONGLONG GetSessionCount() { return _curSessionCount; }
	bool DisconnectSession(SESSION_ID sessionID);
	bool SendPacket(SESSION_ID sessionID, Packet *pPacket);
	void SetTimeoutTime(DWORD ms) { _timeoutMillisec = ms; }

protected:
	// virtual
	virtual bool OnConnectionRequest(WCHAR *IPstr, DWORD IP, USHORT Port) = 0; 
	virtual void OnClientJoin(WCHAR *ipStr, DWORD ip, USHORT port, SESSION_ID sessionID) = 0;
	virtual void OnClientLeave(SESSION_ID sessionID) = 0;
	virtual void OnRecv(SESSION_ID sessionID, Packet *pPacket) = 0;
	virtual void OnSend(SESSION_ID sessionID) = 0; 
	virtual void OnError(int errorcode, const WCHAR *log) = 0;
	virtual void OnTimeout(SESSION_ID sessionID) = 0;
	virtual void OnMonitoringPerSec() = 0; // 1 초마다 갱신되는 모니터링

private:
	// ==============================================
	// Server Framework
	// ==============================================
	void Startup();
	bool CreateListenSocket();
	void BeginThreads();

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

	bool SendPost(SESSION *pSession );
	bool RecvPost(SESSION *pSession );
	void PostClientLeave(SESSION_ID sessionID); // leave 컨텐츠처리를 스레드 분리를 위한 함수
	bool TryGetRecvPacket(SESSION *pSession, Packet *pPacket);
	bool SetWSABuffer(WSABUF *BufSets, SESSION *pSession, bool isRecv );


private:
	// ==============================================
	// Session Management
	// ==============================================
	SESSION *AcquireSession(SESSION_ID sessionID );
	void ReturnSession(SESSION *pSession );
	inline bool IncrementIOCount(SESSION *pSession );
	inline bool DecrementIOCount(SESSION *pSession );

	bool ReleaseSession(SESSION *pSession );

	inline void SetSessionTimeoutTimer(SESSION *pSession) { InterlockedExchange(&pSession->_timeoutTimer, timeGetTime() + _timeoutMillisec); }

	SESSION *CreateSession(SOCKET sock, sockaddr_in clientaddr);
	SESSION_ID GeneratesessionID();
	inline USHORT sessionIDtoIndex(SESSION_ID sessionID);
	inline SESSION *FindSession(SESSION_ID sessionID);
	void InitializeIndex();

	inline void GetStringIP(WCHAR *str, sockaddr_in &addr) {
		wsprintf(str, L"%d.%d.%d.%d", addr.sin_addr.S_un.S_un_b.s_b1, addr.sin_addr.S_un.S_un_b.s_b2, addr.sin_addr.S_un.S_un_b.s_b3, addr.sin_addr.S_un.S_un_b.s_b4);
	}
private:
	// ==============================================
	// LOCK
	// ==============================================
	inline void SessionLock(SESSION *pSession);
	inline void SessionUnlock(SESSION *pSession);

	inline void SessionContainerLock();
	inline void SessionContainerUnlock();

private:
	// ==============================================
	// Server Variable
	// ==============================================

	// ----------------------------------------------
	// Listen Socket Info
	// ----------------------------------------------
	u_short									_Port;
	u_long									_bindIP;
	SOCKET									_listensock;

	// ----------------------------------------------
	// Option
	// ----------------------------------------------
	CParser									*_pConfigData;
	BYTE									_maxRunThreadCount;	// 최대 동시 실행 스레드 수
	BYTE									_workerThreadCount;	// 생성할 스레드 수
	u_short									_maxConnection;		// 최대 동접자 수
	bool									_isNagle;
	DWORD									_timeoutMillisec;

	// ----------------------------------------------
	// Network State
	// ----------------------------------------------
	bool									_isRunning;	// 서버가 진행중인가?
	bool									_isEncryptionPacket; // 패킷 암호화
	BYTE									_NumThreads;		// 몇개의 스레드가 생성되었는가

	// ----------------------------------------------
	// Handle
	// ----------------------------------------------
	HANDLE									_hIOCP;				// IOCP핸들
	CThread									*_tWorkers;			// WorkerThread Handle
	CThread									_tAccept = CThread(L"NetServer Accept Thread");
	CThread									_tMonitoring = CThread(L"NetServer Monitoring Thread");
	CThread									_tTimeout = CThread(L"NetServer Time Out Thread");
#ifdef df_SENDTHREAD
	CThread									_tSend = CThread(L"NetServer Send Thread");
#endif // df_SENDTHREAD
	// ----------------------------------------------
	// Session Container 
	// ----------------------------------------------
	SESSION									*_sessionContainer;
	Stack<USHORT>							_emptyIndex;
	SESSION_ID								_IDGenerater;	// 세션 ID생성기, 0 : 삭제된 세션의 ID
	SRWLOCK									_sessionContainerLock;

protected:
	// ==============================================
	// Monitering
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
		ULONGLONG							_sendedPacketPerSec;
		ULONGLONG							_sendBytePerSec;
		ULONGLONG							_acceptPerSec;
		ULONGLONG							_queueSize;
		ULONGLONG							_queueSizeAvg;
		ULONGLONG							_queueCapacity;
		ULONGLONG							_maxCapacity;
		ULONGLONG							_stackSize;
		ULONGLONG							_stackCapacity;
	};


	// 모니터링 변수
	volatile alignas(64) ULONGLONG			_curSessionCount;
	volatile alignas(64) ULONGLONG			_totalPacket;
	volatile alignas(64) ULONGLONG			_recvPacketCalc;
	volatile alignas(64) ULONGLONG			_recvPacketPerSec;
	volatile alignas(64) LONGLONG			_sendedPacketCalc;
	volatile alignas(64) LONGLONG			_sendedPacketPerSec;
	volatile alignas(64) ULONGLONG			_sendPacketCalc;
	volatile alignas(64) ULONGLONG			_sendPacketPerSec;
	volatile alignas(64) LONG64				_sendProcessedBytesCalc;
	volatile alignas(64) LONG64				_sendProcessedBytesTPS;
	volatile alignas(64) LONG64				_totalProcessedByte;
	volatile alignas(64) ULONGLONG			_acceptCalc;
	volatile alignas(64) ULONGLONG			_acceptPerSec;
	volatile alignas(64) ULONGLONG			_totalAcceptSession;
	volatile alignas(64) ULONGLONG			_totalDisconnectSession;

	void CalcTPS();
	MoniteringInfo GetMoniteringInfo();

	void ResetMonitor();

};



