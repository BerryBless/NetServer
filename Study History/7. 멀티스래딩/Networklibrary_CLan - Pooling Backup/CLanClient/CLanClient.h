#pragma once
#pragma comment(lib, "ws2_32")
#pragma warning(disable:26495)

#include <WS2tcpip.h>
#include <WinSock2.h>
#include <unordered_map>
#include "CRingBuffer.h"
#include "CPacket.h"
#include "Logger.h"

struct SESSION {
	// recv buffer
	WSAOVERLAPPED _recvOverlapped;
	CRingBuffer _recvQueue;
	// send buffer
	WSAOVERLAPPED _sendOverlapped;
	CRingBuffer _sendQueue;


	// session state
	BOOL _isSend;	// 보내는 중이면 TURE
	BOOL _isAlive;	// 살아있는 소켓이면 ture

	// session lock
	SRWLOCK _lock;

	// Init
	SESSION() {
		_isSend = false;
		_isAlive = false;

		ZeroMemory(&_recvOverlapped, sizeof(WSAOVERLAPPED));
		ZeroMemory(&_sendOverlapped, sizeof(WSAOVERLAPPED));

		InitializeSRWLock(&_lock);
	}
};



class CLanClient {
protected:
	// Client
	CLanClient();
	~CLanClient();


	// Server Interface
	bool Connect(ULONG serverIP, USHORT port, BYTE workerThreadCount, bool isNagle = false);//	바인딩 IP, 서버IP / 워커스레드 수 / 나글옵션
	bool Disconnect();//
	bool SendPacket(CPacket *pPacket); // 
	void Quit();

	virtual void OnEnterJoinServer() = 0; //< 서버와의 연결 성공 후
	virtual void OnLeaveServer() = 0; //< 서버와의 연결이 끊어졌을 때

	virtual void OnRecv(CPacket *pPacket) = 0; //< 하나의 패킷 수신 완료 후
	virtual void OnSend(int sendsize) = 0; //< 패킷 송신 완료 후

	//	virtual void OnWorkerThreadBegin() = 0;
//	virtual void OnWorkerThreadEnd() = 0;
	virtual void OnError(int errorcode, const WCHAR *log) = 0; // 에러 발생시 유저한테 알려줄곳


private:
	// Client Framework
	bool CreateSocket(); // 리슨소켓 만들기
	void BeginThreads(); // 스레드 실행

	static unsigned int __stdcall WorkerThread(LPVOID arg);// 워커스레드
	static unsigned int __stdcall MonitorThread(LPVOID arg);// 모니터링 스레드


	bool OnGQCS();
	bool SendProc(DWORD transferredSize); // GQCS 신호가 왔고 그 신호가 Send Overlapped일때 호출
	bool RecvProc(DWORD transferredSize); // GQCS 신호가 왔고 그 신호가 Recv Overlapped일때 호출

	bool NetMonitorProc(); // TPS 모니터링 

	bool SendPost(int logic);	// WSAsend 해주는 곳
	bool RecvPost(int logic, bool isAccept = false);	// WSArecv 해주는 곳
	bool SetWSABuffer(WSABUF *pBufSet, bool isRecv, int logic); // WSARecv, WSASend를 위한 WSABuffer를 셋팅

private:
	// Server Variable

	// Session
	SESSION _session; // 클라이언트 세션

	// server information
	SOCKET _sock;
	ULONG _IP;
	USHORT _port;

	// Option
	BYTE _maxRunThreadCount = 0;	// 최대 동시 실행 스레드 수
	BYTE _workerThreadCount = 0;	// 생성할 스레드 수
	bool _isNagle = false;

	// Network State
	bool _isRunning = false;	// 서버가 진행중인가?
	BYTE _NumThreads = 0;		// 몇개의 스레드가 생성되었는가

	// Handle
	HANDLE _hIOCP;				// IOCP핸들
	HANDLE *_hThreads;			// WorkerThread Handle Array

protected:
	// 상속될 각죵 옵션

	// 모니터링
	struct MONITOR {
		// 현재(계산중)
		DWORD _wsasendCalc;
		DWORD _wsarecvCalc;

		DWORD _sendPacketCalc;
		DWORD _recvPacketCalc;

		// 결과값
		DWORD _wsasendTPS;								// PostSend TPS
		DWORD _wsarecvTPS;								// PostRecv TPS

		DWORD _sendPacketTPS;							//
		DWORD _recvPacketTPS;							//
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