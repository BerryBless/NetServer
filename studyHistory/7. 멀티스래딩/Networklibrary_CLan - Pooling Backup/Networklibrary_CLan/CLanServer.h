#pragma once
#pragma comment(lib, "ws2_32")

#include <WS2tcpip.h>
#include <WinSock2.h>
#include <unordered_map>
#include "CRingBuffer.h"
#include "CPacket.h"
#include "Logger.h"
#include "CObjectPool.hpp"

typedef u_int64 SESSION_ID;

struct SESSION {
	// session id
	SESSION_ID _ID;

	// recv buffer
	WSAOVERLAPPED _recvOverlapped;
	CRingBuffer _recvQueue;
	// send buffer
	WSAOVERLAPPED _sendOverlapped;
	CRingBuffer _sendQueue;


	// session state
	DWORD _IOcount;	// WSA 함수가 걸릴때 +1 , 풀릴때 -1
	DWORD _enqPacketCnt; // 보낸 패킷(메시지 단위) 수
	DWORD _sendedPacketCnt; // 보낸 패킷(메시지 단위) 수
	BOOL _isSend;	// 보내는 중이면 TURE
	BOOL _isAlive;	// 살아있는 소켓이면 ture

	

	// session information
	SOCKET _sock;
	ULONG _IP;
	USHORT _port;

	// session lock
	// TODO lock pool에서 빼가기
	SRWLOCK _lock;

};



class CLanServer {
protected:
	// Server Management
	CLanServer();
	~CLanServer();

	//오픈 IP / 포트 / 워커스레드 수 (생성수, 러닝수) / 나글옵션 / 최대접속자 수
	bool Start(u_long IP, u_short port, BYTE workerThreadCount, BYTE maxRunThreadCount, BOOL nagle, u_short maxConnection);
	bool Start(wchar_t *wsConfigPath); // 파일에서 파싱해서 start(ip, port, worker, run, nagle, conn) 호출
	bool Stop();	// 서버 정지 (일시정지)
	void Quit();// 서버 종료
	void WaitForThreadsFin(); // 모든 스레드 종료 기다리기
	int GetSessionCount();	// 현재 동접자
	
	// Server Interface
	bool Disconnect(SESSION_ID SessionID);// SESSION_ID / HOST_ID
	bool SendPacket(SESSION_ID SessionID, CPacket *pPacket); // SESSION_ID / HOST_ID
	virtual bool OnConnectionRequest(u_long IP, u_short Port) = 0; //< accept 직후

	virtual void OnClientJoin(SESSION_ID SessionID) = 0; //< Accept 후 접속처리 완료 후 호출.
	virtual void OnClientLeave(SESSION_ID SessionID) = 0; //< Release 후 호출

	virtual void OnRecv(SESSION_ID SessionID, CPacket *packet) = 0; //< 패킷 수신 완료 후
		//	virtual void OnSend(SessionID, int sendsize) = 0;           < 패킷 송신 완료 후

		//	virtual void OnWorkerThreadBegin() = 0;                    < 워커스레드 GQCS 바로 하단에서 호출
		//	virtual void OnWorkerThreadEnd() = 0;                      < 워커스레드 1루프 종료 후

	virtual void OnError(int errorcode, const WCHAR *log) = 0; // 에러 발생시 유저한테 알려줄곳

private:
	// Server Framework
	bool CreateListenSocket(); // 리슨소켓 만들기
	void BeginThreads(); // 스레드 실행

	static unsigned int __stdcall WorkerThread(LPVOID arg);// 워커스레드
	static unsigned int __stdcall AcceptThread(LPVOID arg);// accept스레드
	static unsigned int __stdcall MonitorThread(LPVOID arg);// 모니터링 스레드


	bool OnGQCS();
	bool SendProc(SESSION *pSession, DWORD transferredSize); // GQCS 신호가 왔고 그 신호가 Send Overlapped일때 호출
	bool RecvProc(SESSION *pSession, DWORD transferredSize); // GQCS 신호가 왔고 그 신호가 Recv Overlapped일때 호출

	bool AcceptProc(); // Accept 성공시 호출
	bool NetMonitorProc(); // TPS 모니터링 

	bool SendPost(SESSION *pSession, int logic);	// WSAsend 해주는 곳
	bool RecvPost(SESSION *pSession, int logic, bool isAccept = false);	// WSArecv 해주는 곳
	bool SetWSABuffer(WSABUF *pBufSet, SESSION *pSession, bool isRecv, int logic); // WSARecv, WSASend를 위한 WSABuffer를 셋팅

	bool IncrementIOCount(SESSION *pSession, int logic);	// IOcount +1
	bool DecrementIOCount(SESSION *pSession, int logic);	// IOcount -1
	bool ReleaseSession(SESSION *pSession, int logic); // 세션을 지우기

private:
	// Session Management
	SESSION *CreateSession(SOCKET sock, SOCKADDR_IN addr);	// 세션 생성
	SESSION_ID GenerateSessionID();	// 정해진 알고리즘으로 세션 ID만들기 0은 나오면 안됨

	SESSION *FindSession(SESSION_ID sessionID);	// ID로 세션 찾기
	void InsertSessionData(SESSION *pSession);	// 세션 생성후 컨테이너에 넣기
	void DeleteSessionData(SESSION_ID sessionID);//세션 삭제 (릴리즈?)

private:
	// LOCK
	void SessionLock(SESSION *pSession);
	void SessionUnlock(SESSION *pSession);

	void SessionContainerLock();
	void SessionContainerUnlock();

private:
	// Server Variable

	// Listen Socket Info
	u_short _Port;
	u_long _bindIP;
	SOCKET _listensock;

	// Option
	BYTE _maxRunThreadCount = 0;	// 최대 동시 실행 스레드 수
	BYTE _workerThreadCount = 0;	// 생성할 스레드 수
	u_short _maxConnection = 0;		// 최대 동접자 수
	bool _isNagle = false;

	// Network State
	bool _isRunning = false;	// 서버가 진행중인가?
	BYTE _NumThreads = 0;		// 몇개의 스레드가 생성되었는가

	// Handle
	HANDLE _hIOCP;				// IOCP핸들
	HANDLE *_hThreads;			// WorkerThread Handle

	// Session Container 
	std::unordered_map<SESSION_ID, SESSION *> _sessionContainer; // 세션을 관리할 컨테이너
	SESSION_ID _IDGenerater = 1;	// 세션 ID생성기, 0 : 삭제된 세션의 ID
	SRWLOCK	_sessionContainerLock;

	// Pools
	CObjectPool<SESSION> _sessionPool;
	//CObjectPool<CPacket> _packetPool; // TEMP :: global value
protected:
	// TODO 
	// 상속될 각죵 옵션

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


//////////////////////////////////////////////////////////////////////////////
// logic - 추적 가능하게
// 0 ~ 9999 IOCP 이외 대역
// 10000 ~ 19999 - 워커스레드에서 직접 호출
// 20000 ~ 29999 - SendProc() 에서 호출
// 30000 ~ 39999 - RecvProc() 에서 호출
//////////////////////////////////////////////////////////////////////////////

#define dfLOGIC_FROM_THREAD 0
#define dfLOGIC_FROM_WORKER 10000
#define dfLOGIC_FROM_CPMPLETE_SEND 20000
#define dfLOGIC_FROM_CPMPLETE_RECV 30000


// TEMP
extern CObjectPool<CPacket> g_packetPool;