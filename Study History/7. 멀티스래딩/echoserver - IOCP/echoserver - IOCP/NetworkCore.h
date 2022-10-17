#pragma once
#include "CPacket.h"
#include "CRingBuffer.h"
#include "CMonitoring.h"
#include "CSession.h"

#define dfEXIT_KEY 0xFFFFFFFF
#define dfNETWORK_PORT 6000
#define dfMESSAGE_SIZE 10

BOOL NetworkInitServer();		// 네트워크 초기화
void NetworkCloseServer();		// 네트워크 종료

unsigned int __stdcall WorkerThread(LPVOID arg);// 워커스레드
unsigned int __stdcall AcceptThread(LPVOID arg);// accept스레드

//////////////////////////////////////////////////////////////////////////////
// logic - 추적 가능하게
// 0 ~ 9999 IOCP 이외 대역
// 10000 ~ 19999 - 워커스레드에서 직접 호출
// 20000 ~ 29999 - CompleteSend() 에서 호출
// 30000 ~ 39999 - CompleteRecv() 에서 호출
//////////////////////////////////////////////////////////////////////////////

#define dfLOGIC_FROM_THREAD 0
#define dfLOGIC_FROM_WORKER 10000
#define dfLOGIC_FROM_CPMPLETE_SEND 20000
#define dfLOGIC_FROM_CPMPLETE_RECV 30000

bool CompleteSend(CSession *pSession, DWORD transferredSize);
bool CompleteRecv(CSession *pSession, DWORD transferredSize);

bool SendPost(CSession *pSession, int logic);	// WSAsend 해주는 곳
bool RecvPost(CSession *pSession, int logic, bool isAccept = false);	// WSArecv 해주는 곳

bool SetWSABuffer(WSABUF *pBufSet, CSession *pSession, bool isRecv, int logic);
bool SendPacket(CSession *pSession, CPacket *packet);


bool IncrementIOCount(CSession *pSession, int logic);
bool DecrementIOCount(CSession *pSession, int logic);
bool Disconnect(CSession *pSession, int logic); // 디스커넥트
bool Release(u_int64 sID); // 디스커넥트



extern SOCKET g_listenSock;	// 리슨 소켓
extern HANDLE g_hIOCP;// IOCP핸들

