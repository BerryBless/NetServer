#pragma once

#include "CRingBuffer.h"

class CSession {
public:
	u_int64 _sessionID;
	SOCKET _sock;
	WSAOVERLAPPED _recvOverlapped;
	CRingBuffer _recvQueue;
	WSAOVERLAPPED _sendOverlapped;
	CRingBuffer _sendQueue;

	DWORD _IOcount;	// 항상 1
	BOOL _isSend;	// 보내는 중이면 TURE

	ULONG _IP;
	USHORT _port;

public:
	CSession();

	CSession(SOCKET sock, ULONG IP, USHORT port);
	

	void SetNetworkInfo(SOCKET sock, SOCKADDR_IN addr);

	void Init();

	void ClearBuffer();
};

extern std::map<u_int64, CSession *> g_sessions; // 세션 컨테이너 (ID, pSession)
extern SRWLOCK g_sessionContainerLock;

#define SESSION_CONTAINER_LOCK()	 AcquireSRWLockExclusive(&g_sessionContainerLock)
#define SESSION_CONTAINER_UNLOCK()	 ReleaseSRWLockExclusive(&g_sessionContainerLock)

CSession *CreateSession();	// 세션 생성
u_int64 InsertSession(CSession *pSession); // ID가 0 이면 에러
CSession *FindSession(u_int64 sID); // ID로 세션 찾기
bool DeleteSession(u_int64 sID);	// ID로 세션 지우기
void ReleaseSession(CSession *pSession);