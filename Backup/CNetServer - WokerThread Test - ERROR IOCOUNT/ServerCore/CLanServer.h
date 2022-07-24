#pragma once

#include "CRingBuffer.h"
#include "CPacket.h"
#include "CLogger.h"
#include "CObjectPool.hpp"
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

//#define df_SENDTHREAD

// ----------------------------------------------
// SESSION_ID
// ID가 하나씩 증가하며 항상 고유키
// 0은 사용이 되지않는 ID
// ----------------------------------------------

class CLanServer {
public:
	typedef u_int64 SESSION_ID;
	struct SESSION {
		SESSION_ID _ID;
		DWORD _isAlive;

		// IOCP Buffer
		WSAOVERLAPPED _recvOverlapped;
		WSAOVERLAPPED _sendOverlapped;
		CRingBuffer _recvQueue;
		Queue<CPacket *> _sendQueue;

		// session state
		DWORD _IOcount;
		DWORD _IOFlag;
		DWORD _sendPacketCnt;
		DWORD _lastRecvdTime;

		// session information
		SOCKET _sock;
		ULONG _IP;
		USHORT _port;

		// session lock
		SRWLOCK _lock;
		DWORD _ThreadBlockIdx;

		SESSION() {
			_ID = 0;
			_IOcount = 0;
			_IOFlag = 0;
			_sendPacketCnt = 0;
			ZeroMemory(&_recvOverlapped, sizeof(WSAOVERLAPPED));
			ZeroMemory(&_sendOverlapped, sizeof(WSAOVERLAPPED));
		}
	};
protected:
	CLanServer();
	~CLanServer();

	// ==============================================
	// Server Interface
	// ==============================================

	// ----------------------------------------------
	// Start()
	// Start(오픈 IP / 포트 / 워커스레드 수 (생성수, 러닝수) / 나글옵션 / 최대접속자 수)
	// TODO : Start(파싱할 파일) : 파일에서 파싱해서 start(ip, port, worker, run, nagle, conn) 호출
	// 
	// 설정값대로 서버를 실행
	// ----------------------------------------------
	bool Start(u_long IP, u_short port, BYTE workerThreadCount, BYTE maxRunThreadCount, BOOL nagle, u_short maxConnection);
	bool Start(wchar_t *wsConfigPath);


	// ----------------------------------------------
	// 서버 종료
	// Quit();				: 종료 신호를 보냄
	// WaitForThreadsFin()	: 모든 스레드 종료 기다리기
	// ----------------------------------------------
	void Quit();
	void WaitForThreadsFin();

	// ----------------------------------------------
	// GetSessionCount();
	// 
	// 현재 연결된 세션개수 얻어오기
	// ----------------------------------------------
	int GetSessionCount();

	// ----------------------------------------------
	// Disconnect(ID) 
	// 
	// 해당 ID에 대응하는 세션 종료
	//
	// return
	// TRUE  : SessionID IO취소 성공
	// FALSE : SessionID IO취소 실패
	// ----------------------------------------------
	bool Disconnect(SESSION_ID SessionID);

	// ----------------------------------------------
	// SendPacket(ID, pPacket)
	// 
	// 해당 ID에 대응하는 세션에서 pPacket를 보내기 시도
	// 
	// return 
	// TRUE  : ID에 해당하는 세션의 send buffer에 복사 성공
	// FALSE : ID에 해당하는 세션의 send buffer에 복사 실패
	// ----------------------------------------------
	bool SendPacket(SESSION_ID SessionID, CPacket *pPacket);


	// ----------------------------------------------
	// OnConnectionRequest(IP, port)
	// 
	// Accept() 직후 바로 호출되는 순수 가상함수
	// 
	// return 
	// TRUE  : 해당 (IP, port)에 대한연결 허용
	// FALSE : 해당 (IP, port)에 대한연결 비허용
	// ----------------------------------------------
	virtual bool OnConnectionRequest(u_long IP, u_short Port) = 0;

	// ----------------------------------------------
	// OnClientJoin(ID)
	// 
	// Accept 후 접속처리 완료 후 호출
	// ----------------------------------------------
	virtual void OnClientJoin(SESSION_ID SessionID) = 0;

	// ----------------------------------------------
	// OnClientLeave(ID)
	// 
	// Release 후 호출
	// ----------------------------------------------
	virtual void OnClientLeave(SESSION_ID SessionID) = 0;


	// ----------------------------------------------
	// OnRecv(ID, pPacket)
	// 
	// 패킷 수신 완료 후 호출
	// ----------------------------------------------
	virtual void OnRecv(SESSION_ID SessionID, CPacket *pPacket) = 0;
	// ----------------------------------------------
	// OnError(errorcode, log)
	// 
	// 에러 발생시 유저한테 알려줄곳
	// ----------------------------------------------
	virtual void OnError(int errorcode, const WCHAR *log) = 0;


	virtual void OnTimeout(SESSION_ID SessionID) = 0;

	// ----------------------------------------------
	// DomainToIP()
	// 
	// 도메인주소로 IP주소 얻어오기
	// ----------------------------------------------
	BOOL DomainToIP(const WCHAR *szDomain, IN_ADDR *pAddr);

	void SetTimeoutTime(DWORD ms) { _timeoutMillisec = ms; }

private:
	// ==============================================
	// Server Framework
	// ==============================================



	// ----------------------------------------------
	// CreateListenSocket()
	// 
	// 리슨 소켓을 만들기
	//
	// return
	// TRUE  : 리슨 소켓 생성 성공
	// FALSE : 리슨 소켓 생성 실패
	// ----------------------------------------------
	bool CreateListenSocket();


	// ----------------------------------------------
	// BeginThreads()
	// 
	// 스레드를 설정에 따라 생성후 실행
	// ----------------------------------------------
	void BeginThreads();
	// ----------------------------------------------
	// WorkerThread(this)
	// 
	// GQCS() 를 호출할 스레드
	// ----------------------------------------------
	static unsigned int __stdcall WorkerThread(LPVOID arg);
	// ----------------------------------------------
	// AcceptThread(this)
	// 
	// Accept() 를 호출할 스레드
	// ----------------------------------------------
	static unsigned int __stdcall AcceptThread(LPVOID arg);
	// ----------------------------------------------
	// MonitorThread(this)
	// 
	// 모니터링 스레드
	// 1초마다 TPS를 계산
	// ----------------------------------------------
	static unsigned int __stdcall MonitorThread(LPVOID arg);

	static unsigned int __stdcall TimeOutThread(LPVOID arg);



#ifdef df_SENDTHREAD
	static unsigned int __stdcall SendThread(LPVOID arg);
#endif // df_SENDTHREAD




	// ----------------------------------------------
	// OnGQCS()
	// 
	// GetQueuedCompletionStatus()을 호출
	// 완료통지에 온 ID로 세션을 찾아
	// 세션에대해 완료통지에 맞게 함수를 호출
	//
	// return
	// TRUE  : 스레드를 계속 실행
	// FALSE : 스레드 종료
	// ----------------------------------------------
	bool OnGQCS();

	// ----------------------------------------------
	// SendProc(pSession, transferredSize)
	// 
	// 보낸 패킷을 세션의 sendQ에서 삭제, 풀로 반환
	// sendQ에 값이 남아있으면 Send를 시도
	// 
	// return
	// TRUE  : Send 완료통지에 대한 처리 성공
	// FALSE : Send 완료통지에 대한 처리 실패
	// ----------------------------------------------
	bool SendProc(SESSION *pSession, DWORD transferredSize);
	// ----------------------------------------------
	// RecvProc(pSession, transferredSize)
	// 
	// transferredSize만큼 pSession->_recvQueue의 rear를 미뤄주고
	// pSession->_recvQueue의 내용을 pPacket에 복사하고
	// OnRecv(pSession->_ID, pPacket); 을 호출
	// 
	// return
	// TRUE  : Recv 완료통지에 대한 처리 성공
	// FALSE : Recv 완료통지에 대한 처리 실패
	// ----------------------------------------------
	bool RecvProc(SESSION *pSession, DWORD transferredSize);

	// ----------------------------------------------
	// AcceptProc()
	// 
	// Accept() 호출후
	// OnConnectionRequest()호출
	// 접속이 가능하면 세션을 만들고 OnClientJoin(ID)호출
	// 
	// return
	// TRUE  : 스레드를 계속 실행
	// FALSE : 스레드 종료
	// ----------------------------------------------
	bool AcceptProc();

	// ----------------------------------------------
	// NetMonitorProc()
	// 
	// sendPacketTPS, recvPacketTPS을 1초마다 계산
	// 
	// return
	// TRUE  : 스레드를 계속 실행
	// FALSE : 스레드 종료
	// ----------------------------------------------
	bool NetMonitorProc();

#ifdef df_SENDTHREAD
	bool SendThreadProc();
#endif


	bool TimeOutProc();

	// ----------------------------------------------
	// SendPost(pSession, logic)
	// logic : 어느 곳에서 호출을 했다를 알 수 있는 디버깅용 숫자
	// 
	// pSession의 sendQ에 있는 패킷을 100개까지 보내기
	// 
	// return
	// TRUE  : WSASend 성공 (IO_PENDING, 동기로 완료)
	// FALSE : WSASend 실패 (그외 모든 경우)
	// ----------------------------------------------
	bool SendPost(SESSION *pSession, int logic);

	// ----------------------------------------------
	// RecvPost(pSession, logic, isAccept)
	// logic : 어느 곳에서 호출을 했다를 알 수 있는 디버깅용 숫자
	// isAccept: Accept에서 호출한 코드
	// 
	// pSession에 대하여 WSARecv를 걸기
	// 
	// return
	// TRUE  : WSARecv 성공 (IO_PENDING, 동기로 완료)
	// FALSE : WSARecv 실패 (그외 모든 경우)
	// ----------------------------------------------
	bool RecvPost(SESSION *pSession, int logic, bool isAccept = false);

	// ----------------------------------------------
	// SetWSABuffer(BufSets, pSession, isRecv, logic)
	// BufSets	: 세팅할 WSABUF배열
	// logic	: 어느 곳에서 호출을 했다를 알 수 있는 디버깅용 숫자
	// 
	// pSession에 대하여 WSARecv를 걸기
	// 
	// return
	// TRUE  : WSABUF 세팅 성공
	// FALSE : WSABUF 세팅 실패
	// ----------------------------------------------
	bool SetWSABuffer(WSABUF *BufSets, SESSION *pSession, bool isRecv, int logic);

	// ----------------------------------------------
	// IncrementIOCount( pSession,  logic)
	// logic	: 어느 곳에서 호출을 했다를 알 수 있는 디버깅용 숫자
	// 
	// pSession의 IOcount 를 interlocked로 증가
	// 
	// return
	// TRUE  : IOcount 증가 성공
	// FALSE : IOcount 증가 실패
	// ----------------------------------------------
	bool IncrementIOCount(SESSION *pSession, int logic);
	// ----------------------------------------------
	// DecrementIOCount( pSession,  logic)
	// logic	: 어느 곳에서 호출을 했다를 알 수 있는 디버깅용 숫자
	// 
	// pSession의 IOcount 를 interlocked로 감소
	// 
	// return
	// TRUE  : IOcount 증가 성공
	// FALSE : IOcount 증가 실패
	// ----------------------------------------------
	bool DecrementIOCount(SESSION *pSession, int logic);

	// ----------------------------------------------
	// ReleaseSession( pSession,  logic)
	// logic	: 어느 곳에서 호출을 했다를 알 수 있는 디버깅용 숫자
	// 
	// pSession을 사용종료
	// OnClientLeave()
	// closesocket()
	// DeleteSessionData()
	// 순서대로 호출
	// 
	// return
	// TRUE  : ReleaseSession 성공
	// FALSE : ReleaseSession 실패
	// ----------------------------------------------
	bool ReleaseSession(SESSION *pSession, int logic);

private:
	// ==============================================
	// Session Management
	// ==============================================

	// ----------------------------------------------
	// CreateSession( sock,  addr)
	// 
	// 세션 객체를 할당 받고
	// 세션을 초기화
	// 그 세션을 세션관리 컨테이너에 등록
	// 
	// return
	// 생성된 세션 포인터
	// ----------------------------------------------
	SESSION *CreateSession(SOCKET sock, SOCKADDR_IN addr);

	// ----------------------------------------------
	// GenerateSessionID()
	// 
	// 정해진 알고리즘으로 고유한 세션 ID만들기 
	// 0은 사용하지 않는 ID
	// 현재 알고리즘 : 하나씩 증가
	// 
	// return
	// 생성된 세션 ID
	// ----------------------------------------------
	SESSION_ID GenerateSessionID();

	// ----------------------------------------------
	// SessionIDtoIndex()
	// 
	// SessionID에서 특정계산을 하여
	// _sessionContainer의 Index를 구한다.
	// 
	// return
	// Index
	// ----------------------------------------------
	USHORT SessionIDtoIndex(SESSION_ID sessionID);

	// ----------------------------------------------
	// SessionIDtoIndex()
	// 
	// 세션관리 Index를 초기화 한다
	// 최대 수용 인원수 만큼 빈 인덱스 스택에 넣기
	// ----------------------------------------------
	void InitializeIndex();


	// ----------------------------------------------
	// GenerateSessionID(ID)
	// 
	// ID로 세션관리 컨테이너에서 검색
	// 
	// return
	// 검색된 세션의 포인터
	// 실패시 NULL
	// ----------------------------------------------
	SESSION *FindSession(SESSION_ID sessionID);

	// ----------------------------------------------
	// InsertSessionData(pSession)
	// 
	// ID를 키값으로 관리 컨테이너에서 삭제
	// 해당 세션을 오브젝트 풀에 반환
	// ----------------------------------------------
	void DeleteSessionData(SESSION_ID sessionID);

private:
	// ==============================================
	// LOCK
	// ==============================================


	// ----------------------------------------------
	// SessionLock(pSession)
	// SessionUnlock(pSession)
	// 
	// 해당 세션의 LOCK, UNLOCK
	// ----------------------------------------------
	void SessionLock(SESSION *pSession);
	void SessionUnlock(SESSION *pSession);

	// ----------------------------------------------
	// SessionContainerLock()
	// SessionContainerUnlock()
	// 
	// 세션관리 컨테이너의 LOCK, UNLOCK
	// ----------------------------------------------
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

	// ----------------------------------------------
	// Pools
	// ----------------------------------------------
	//CObjectPool<CPacket> _packetPool; // TEMP :: global value
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
		//ULONGLONG							_recvPacketCount;
		ULONGLONG							_recvPacketPerSec;
		//ULONGLONG							_sendPacketCount;
		ULONGLONG							_sendPacketPerSec;
		ULONGLONG							_acceptPerSec;
		ULONGLONG							_queueSize;
		ULONGLONG							_queueSizeAvg;
		ULONGLONG							_queueCapacity;
		ULONGLONG							_maxCapacity;
		ULONGLONG							_stackSize;
		ULONGLONG							_stackCapacity;
	};


	// 패킷 처리 수치 및 패킷처리 완료 바이트수
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

};


//////////////////////////////////////////////////////////////////////////////
// logic - 디버깅 가능하게
//////////////////////////////////////////////////////////////////////////////
#define dfLOGIC_WORKER			10000
#define dfLOGIC_ACCEPT			20000
#define dfLOGIC_CPMPLETE_SEND	2000
#define dfLOGIC_CPMPLETE_RECV	3000
#define dfLOGIC_SEND_PACKET		5000
#define dfLOGIC_DISCONNECT		7000
#define dfLOGIC_DECREMENT_IO	300
#define dfLOGIC_INCREMENT_IO	200
#define dfLOGIC_RELEASE_SESSION 1
#define dfLOGIC_SET_BUFFER		2

