#include "pch.h"
#include "CSession.h"

CSession::CSession() {
	Init();
	_sock = INVALID_SOCKET;
}

CSession::CSession(SOCKET sock, ULONG IP, USHORT port) {
	Init();
	_sock = sock;
	_IP = IP;
	_port = port;
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

std::map<u_int64,CSession *> g_sessions; // 세션 컨테이너


	// 락 초기화
SRWLOCK g_sessionContainerLock;

CSession *CreateSession() {
	return new CSession;
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

bool DeleteSession(u_int64 sID) {
	bool ret = false;
	SESSION_CONTAINER_LOCK();
	do {
		auto iter = g_sessions.find(sID);
		if (iter == g_sessions.end())
			break;
		g_sessions.erase(iter);
		ret = true;
	} while (0);
	SESSION_CONTAINER_UNLOCK();
	return ret;
}

void ReleaseSession(CSession *pSession) {
	delete pSession;
}
