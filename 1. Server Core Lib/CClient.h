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
	_LOG(dfLOG_LEVEL_ERROR, L"///////CRASH : FILE[%s] Line[%d]",__FILEW__,__LINE__);\
	int *nptr = nullptr; *nptr = 1;\
}while(0)
#endif // !CRASH

class CClient {
public:
	CClient(bool isEncryption = false);
	~CClient();
protected:
	// ==============================================
	// Client Interface
	// ==============================================
	ULONGLONG GetSessionCount() { return _curSessionCount; }
	bool DisconnectSession(SESSION_ID sessionID);
	bool SendPacket(SESSION_ID sessionID, Packet *pPacket);

protected:
	bool Start(BYTE workerThreadCount, BYTE maxRunThreadCount, BOOL nagle, u_short maxConnection);
	void Quit();
	bool Connect(const WCHAR *serverIP, USHORT serverPort);

	BOOL DomainToIP(const WCHAR *szDomain, IN_ADDR *pAddr);

protected:
	// tirtual
	virtual void OnEnterServer(SESSION_ID sessionID) = 0; //< 서버와의 연결 성공 후
	virtual void OnLeaveServer(SESSION_ID sessionID) = 0; //< 서버와의 연결이 끊어졌을 때
	virtual void OnRecv(SESSION_ID sessionID, Packet *pPacket) = 0;//< 패킷 수신 완료 후
	virtual void OnSend(SESSION_ID sessionID) = 0; //< 패킷 송신 완료 후
	virtual void OnError(int errorcode, const WCHAR *) = 0;//< 에러났을때 // TODO errorcode
	//virtual void OnMonitoringPerSec() = 0;



private:
	// ==============================================
	// Client Framework
	// ==============================================
	void Startup();
	void BeginThreads();

	bool OnGQCS();
	bool SendProc(SESSION *pSession, DWORD transferredSize);
	bool RecvProc(SESSION *pSession, DWORD transferredSize);
	bool TryConnectServer(SOCKET &socket, sockaddr_in &addr);
	//bool NetMonitorProc();

	bool SendPost(SESSION *pSession  = 0);
	bool RecvPost(SESSION *pSession  = 0);
	void PostClientLeave(SESSION_ID sessionID); // leave 컨텐츠처리를 스레드 분리를 위한 함수
	bool TryGetRecvPacket(SESSION *pSession, Packet *pPacket);
	bool SetWSABuffer(WSABUF *BufSets, SESSION *pSession, bool isRecv  = 0);


	void CreateIOCP();
	bool SetWSAStartUp();
	SOCKET CreateSocket();
	bool SetTimeWaitZero(SOCKET sock);
	bool SetNonBlockSocket(SOCKET sock);
	bool SetNagle(SOCKET sock, bool sw);
private:
	// ==============================================
	// Session Management
	// ==============================================
	SESSION *AcquireSession(SESSION_ID sessionID  = 0);
	void ReturnSession(SESSION *pSession  = 0);
	inline bool IncrementIOCount(SESSION *pSession  = 0);
	inline bool DecrementIOCount(SESSION *pSession  = 0);

	bool ReleaseSession(SESSION *pSession );


	SESSION *CreateSession(SOCKET sock, sockaddr_in servAddr);
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
	inline void SessionLock(SESSION *pSession) { AcquireSRWLockExclusive(&pSession->_lock); }
	inline void SessionUnlock(SESSION *pSession) { ReleaseSRWLockExclusive(&pSession->_lock); }
	inline void SessionSLock(SESSION *pSession) { AcquireSRWLockShared(&pSession->_lock); }
	inline void SessionSUnlock(SESSION *pSession) { ReleaseSRWLockShared(&pSession->_lock); }

	inline void Lock() { AcquireSRWLockExclusive(&_lock); }
	inline void Unlock() { ReleaseSRWLockExclusive(&_lock); }
	inline void SLock() { AcquireSRWLockShared(&_lock); }
	inline void SUnlock() { ReleaseSRWLockShared(&_lock); }
	SRWLOCK	_lock;
private:
	// ----------------------------------------------
	// Network State
	// ----------------------------------------------
	long									_isRunning = 0;		// 서버가 진행중인가?
	bool									_isEncryptionPacket ;
	BYTE									_numThreads = 0;		// 몇개의 스레드가 생성되었는가

	// ----------------------------------------------
	// Option
	// ----------------------------------------------
	CParser									*_pConfigData;
	BYTE									_maxRunThreadCount = 0;	// 최대 동시 실행 스레드 수
	BYTE									_workerThreadCount = 0;	// 생성할 스레드 수
	BYTE									_maxConnection = 0;
	bool									_isNagle = false;

	// ----------------------------------------------
	// Handle
	// ----------------------------------------------
	HANDLE									_hIOCP;				// IOCP핸들

	// ----------------------------------------------
	// THREAD
	// ----------------------------------------------
	CThread									*_tWorkers;
	//CThread									_tMonitoring = CThread(L"LanClient Monitoring Thread");

	// ----------------------------------------------
	// Session Container 
	// ----------------------------------------------
	SESSION									*_sessionContainer;
	Stack<USHORT>							_emptyIndex;
	SESSION_ID								_IDGenerater;	// 세션 ID생성기, 0 : 삭제된 세션의 ID
	SRWLOCK									_sessionContainerLock;
protected:
	// ==============================================
	// 모니터링
	// ==============================================
	/*struct MoniteringInfo {
		DWORD								_workerThreadCount;
		DWORD								_runningThreadCount;
		ULONGLONG							_sessionCnt;
		ULONGLONG							_totalPacket;
		ULONGLONG							_totalProecessedBytes;
		ULONGLONG							_totalConnectSession;
		ULONGLONG							_totalReleaseSession;
		ULONGLONG							_recvPacketPerSec;
		ULONGLONG							_sendPacketPerSec;
		ULONGLONG							_sendedPacketPerSec;
		ULONGLONG							_sendBytePerSec;
		ULONGLONG							_connectPerSec;
		ULONGLONG							_queueSize;
		ULONGLONG							_queueSizeAvg;
		ULONGLONG							_queueCapacity;
		ULONGLONG							_maxCapacity;
		ULONGLONG							_stackSize;
		ULONGLONG							_stackCapacity;
	};*/


	//// 모니터링 변수
	volatile alignas(64) ULONGLONG			_curSessionCount;
	volatile alignas(64) ULONGLONG			_totalPacket;
	//volatile alignas(64) ULONGLONG			_recvPacketCalc;
	//volatile alignas(64) ULONGLONG			_recvPacketPerSec;
	//volatile alignas(64) LONGLONG			_sendedPacketCalc;
	//volatile alignas(64) LONGLONG			_sendedPacketPerSec;
	//volatile alignas(64) ULONGLONG			_sendPacketCalc;
	//volatile alignas(64) ULONGLONG			_sendPacketPerSec;
	//volatile alignas(64) LONG64				_sendProcessedBytesCalc;
	//volatile alignas(64) LONG64				_sendProcessedBytesTPS;
	//volatile alignas(64) LONG64				_totalProcessedByte;
	//volatile alignas(64) ULONGLONG			_connectCalc;
	//volatile alignas(64) ULONGLONG			_connectPerSec;
	volatile alignas(64) ULONGLONG			_totalConnectSession;
	volatile alignas(64) ULONGLONG			_totalDisconnectSession;

	//void CalcTPS();
	//MoniteringInfo GetMoniteringInfo();

	//void ResetMonitor();
};

