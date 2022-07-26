#include "pch.h"
#include "CSession.h"
#include "CMonitoring.h"
CSession::CSession() {
	Init();
	_sock = INVALID_SOCKET;
	InitializeSRWLock(&_lock);
}

CSession::CSession(SOCKET sock, ULONG IP, USHORT port) {
	Init();
	_sock = sock;
	_IP = IP;
	_port = port;
	InitializeSRWLock(&_lock);
}

CSession::~CSession() {
}

void CSession::SetNetworkInfo(SOCKET sock, SOCKADDR_IN addr) {
	_sock = sock;
	_IP = addr.sin_addr.S_un.S_addr;
	_port = addr.sin_port;
}

void CSession::Init() {
	ClearBuffer();
	_isSend = false;
	_IOcount = 0;
}

void CSession::ClearBuffer() {
	memset(&_recvOverlapped, 0, sizeof(_recvOverlapped));
	memset(&_sendOverlapped, 0, sizeof(_sendOverlapped));
	_recvQueue.ClearBuffer();
	_sendQueue.ClearBuffer();
}

std::map<u_int64, CSession *> g_sessions; // 세션 컨테이너


	// 락 초기화
SRWLOCK g_sessionContainerLock;

void SessionMapLock() {
	AcquireSRWLockExclusive(&g_sessionContainerLock);
}

void SessionMapUnlock() {
	ReleaseSRWLockExclusive(&g_sessionContainerLock);
}

void SessionLock(CSession *pSession) {
	if (pSession == NULL) {
		CRASH();
	}
	AcquireSRWLockExclusive(&pSession->_lock);
}

void SessionUnlock(CSession *pSession) {
	if (pSession == NULL) {
		CRASH();
	}
	ReleaseSRWLockExclusive(&pSession->_lock);
}

CSession *CreateSession(SOCKET sock, SOCKADDR_IN addr) {
	CSession *pSession = new CSession;
	pSession->Init();
	pSession->SetNetworkInfo(sock, addr);
	// 관리 컨테이너에 세션 추가
	pSession->_sessionID = InsertSession(pSession);
	if (pSession->_sessionID == 0) {
		// 컨테이너에 추가 실패
		CRASH();
	}
	g_monitoring.IncrementSessionCount();
	return pSession;
}
u_int64 g_idGen = 0;
u_int64 InsertSession(CSession *pSession) {
	u_int64 ret = 0;
	SESSION_CONTAINER_LOCK();
	g_idGen++;
	g_sessions.insert(std::make_pair(g_idGen, pSession));
	ret = g_idGen;
	SESSION_CONTAINER_UNLOCK();
	return ret;
}

CSession *FindSession(u_int64 sID) {
	CSession *pSession = NULL;
	SESSION_CONTAINER_LOCK();
	do {
		auto iter = g_sessions.find(sID);
		if (iter == g_sessions.end())
			break;
		pSession = iter->second;
	} while (0);
	SESSION_CONTAINER_UNLOCK();
	return pSession;
}


void ReleaseSession(u_int64 sID) {
	CSession *pSession;
	SESSION_CONTAINER_LOCK();
	do {
		auto iter = g_sessions.find(sID);
		if (iter == g_sessions.end())
			break;
		g_sessions.erase(iter);
		pSession = iter->second;

		SESSION_LOCK(pSession);
		SESSION_CONTAINER_UNLOCK();
		SESSION_UNLOCK(pSession);

		delete pSession;
		g_monitoring.DecrementSessionCount();
		return;
	} while (0);
	CRASH();
}
