#pragma once
#include "CPacket.h"
#include "CRingBuffer.h"
#include "CMonitoring.h"
#include "CSession.h"
#include "NetworkHeader.h"

#define dfEXIT_KEY 0xFFFFFFFF
#define dfNETWORK_PORT 6000
#define dfMESSAGE_SIZE 10

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


BOOL NetworkInitServer();		// 네트워크 초기화
void NetworkCloseServer();		// 네트워크 종료

unsigned int __stdcall NetworkWorkerThread(LPVOID arg);// 워커스레드
unsigned int __stdcall NetworkAcceptThread(LPVOID arg);// accept스레드
unsigned int __stdcall NetworkMonitorThread(LPVOID arg);// 모니터링 스레드




bool SendProc(CSession *pSession, DWORD transferredSize);
bool RecvProc(CSession *pSession, DWORD transferredSize);

bool SendPost(CSession *pSession, int logic);	// WSAsend 해주는 곳
bool RecvPost(CSession *pSession, int logic, bool isAccept = false);	// WSArecv 해주는 곳

bool SetWSABuffer(WSABUF *pBufSet, CSession *pSession, bool isRecv, int logic);
bool SendPacket(u_int64 sID, CPacket *pPacket, int logic = 0);

void OnRecv(CSession *pSession, CPacket *pRecvPacket, CPacket *pSendPacket, NETWORK_HEADER header);
bool ComplateRecv(CSession *pSession, CPacket *pRecvPacket, NETWORK_HEADER *pHeader);

bool IncrementIOCount(CSession *pSession, int logic);
bool DecrementIOCount(CSession *pSession, int logic);
bool Disconnect(CSession *pSession, int logic); // 디스커넥트
bool Release(u_int64 sID); // 디스커넥트



extern SOCKET g_listenSock;	// 리슨 소켓
extern HANDLE g_hIOCP;// IOCP핸들

