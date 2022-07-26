#include "pch.h"
#include "NetworkCore.h"



WSADATA g_wsaData;		// 윈속
SOCKADDR_IN g_listenAddr;	// 리슨 소켓 정보
SOCKET g_listenSock;	// 리슨 소켓
HANDLE g_hIOCP;// IOCP핸들



BOOL NetworkInitServer() {
	_LOG(dfLOG_LEVEL_ERROR, L"NetworkInitServer()..");
	int setRet;// 소켓 연결에 나오는 리턴

		// 윈속 초기화
	if (WSAStartup(MAKEWORD(2, 2), &g_wsaData) != 0) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// WSAStartup() errcode[%d]", WSAGetLastError());
		return FALSE;
	}
	_LOG(dfLOG_LEVEL_ERROR, L"WSAStartup OK..");

	// 소켓 기본정보
	memset(&g_listenAddr, 0, sizeof(g_listenAddr));
	g_listenAddr.sin_family = AF_INET;
	g_listenAddr.sin_port = htons(dfNETWORK_PORT);
	InetPton(AF_INET, L"0.0.0.0", &g_listenAddr.sin_addr);
	_LOG(dfLOG_LEVEL_ERROR, L"PORT [%d] g_listenAddr.sin_addr [%x] ", dfNETWORK_PORT, g_listenAddr.sin_addr);

	// socket()
	g_listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (g_listenSock == INVALID_SOCKET) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// socket() errcode[%d]", WSAGetLastError());
		return FALSE;
	}
	_LOG(dfLOG_LEVEL_ERROR, L"g_listenSock OK [%lld]..", g_listenSock);

	// bind()
	setRet = bind(g_listenSock, (SOCKADDR *) &g_listenAddr, sizeof(g_listenAddr));
	if (setRet == SOCKET_ERROR) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// bind() errcode[%d]", WSAGetLastError());
		return FALSE;
	}
	_LOG(dfLOG_LEVEL_ERROR, L"bind() OK [%d] ..", setRet);

	// listen()
	setRet = listen(g_listenSock, SOMAXCONN);
	if (setRet == SOCKET_ERROR) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// listen() errcode[%d]", WSAGetLastError());
		return FALSE;
	}
	_LOG(dfLOG_LEVEL_ERROR, L"listen() OK [%d] ..", setRet);

	// IOCP
	g_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (g_hIOCP == NULL) {
		_LOG(dfLOG_LEVEL_ERROR, L"////// CreateIoCompletionPort() errcode[%d]", WSAGetLastError());
		return 1;
	}
	_LOG(dfLOG_LEVEL_ERROR, L"CreateIoCompletionPort() OK ..");



	_LOG(dfLOG_LEVEL_ERROR, L"NetworkInitServer() OK..");
	return TRUE;
}
void NetworkCloseServer() {
	//---------------------------
	// 서버정리
	//---------------------------
	_LOG(dfLOG_LEVEL_ERROR, L"NetworkCloseServer()..");
	WSACleanup();
	_LOG(dfLOG_LEVEL_ERROR, L"WSACleanup OK..");
	CloseHandle(g_hIOCP);

	_LOG(dfLOG_LEVEL_ERROR, L"NetworkCloseServer() OK..");
}
unsigned int __stdcall WorkerThread(LPVOID arg) {
	DWORD transferredSize;
	unsigned long threadID = GetCurrentThreadId();
	u_int64 completionKey;
	WSAOVERLAPPED *pOverlapped;

	CSession *pSession;

	g_monitoring.ThreadRegistr(threadID);

	while (true) {
		transferredSize = 0;
		completionKey = NULL;
		pOverlapped = NULL;

		BOOL GQCSRet = GetQueuedCompletionStatus(g_hIOCP, &transferredSize, (PULONG_PTR) &completionKey, &pOverlapped, INFINITE);
		g_monitoring.ThreadCheck(threadID);

		if (pOverlapped == NULL) {
			// 스레드 종료 
			if (transferredSize == dfEXIT_KEY && (long long) completionKey == dfEXIT_KEY) {
				PostQueuedCompletionStatus(g_hIOCP, dfEXIT_KEY, dfEXIT_KEY, NULL); // 워커 하나 종료하라고 알려줌
				break;
			}
			_LOG(dfLOG_LEVEL_ERROR, L"overlapped is NULL ERROR CODE [%d]", WSAGetLastError());
			continue;
		}

		pSession = FindSession(completionKey);
		//pSession = completionKey;

		if (transferredSize > 0 && GQCSRet == TRUE) {
			if (pOverlapped == &pSession->_recvOverlapped) {
				if (CompleteRecv(pSession, transferredSize) == false) {

				}
			}
			if (pOverlapped == &pSession->_sendOverlapped) {
				CompleteSend(pSession, transferredSize);
			}
		}
		DecrementIOCount(pSession, dfLOGIC_FROM_WORKER + 100);
	}
	_LOG(dfLOG_LEVEL_ERROR, L"Worker Thread ID[%lld] IS Closed..", threadID);
	return 0;
}

unsigned int __stdcall AcceptThread(LPVOID arg) {
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	SOCKET clientsock;
	HANDLE hResult;
	while (true) {
		clientsock = accept(g_listenSock, (SOCKADDR *) &clientaddr, &addrlen);
		if (clientsock == INVALID_SOCKET) {
			int err = WSAGetLastError();
			if (err == WSAENOTSOCK || err == WSAEINTR) {
				// 리슨소켓 닫음
				// 어셉트 스레드 종료
				break;
			}
			_LOG(dfLOG_LEVEL_ERROR, L"Socket Accept [Error: %d]", err);
			continue;
		}

		// 세션 만들기
		CSession *pSession = CreateSession();
		// 관리 컨테이너에 세션 추가
		pSession->_sessionID = InsertSession(pSession);
		if (pSession->_sessionID == 0) {
			// 컨테이너에 추가 실패
			CRASH();
		}

		pSession->Init();
		pSession->SetNetworkInfo(clientsock, clientaddr);

		// IOCP
		hResult = CreateIoCompletionPort((HANDLE) clientsock, g_hIOCP, (ULONG_PTR) pSession->_sessionID, NULL);
		if (hResult == NULL) {
			_LOG(dfLOG_LEVEL_ERROR, L"////// Accept CreateIoCompletionPort() errcode[%d]", WSAGetLastError());
			return 1;
		}
		g_monitoring.IncrementSessionCount();
		RecvPost(pSession, dfLOGIC_FROM_THREAD + 200, true);
	}
	_LOG(dfLOG_LEVEL_ERROR, L"Accept Thread Closed");
}

bool CompleteSend(CSession *pSession, DWORD transferredSize) {
	pSession->_sendQueue.MoveFront(transferredSize);

	int msgCount = transferredSize / dfMESSAGE_SIZE;
	int msgByte = transferredSize;
	g_monitoring.SendMsgAdd(msgCount);

	//InterlockedExchange8((CHAR *) &pSession->_isSend, FALSE);
	pSession->_isSend = false;
	if (pSession->_sendQueue.GetUseSize() > 0)
		SendPost(pSession, dfLOGIC_FROM_CPMPLETE_SEND + 100);

	return false;
}

bool CompleteRecv(CSession *pSession, DWORD transferredSize) {

	// recv신호가 옴

	/// 여기서 문제!!
	// 원인) 스레드가 동시에 깨어나서 MoveRear가 동시에 된다?
	int movRet = pSession->_recvQueue.MoveRear(transferredSize);
	if (transferredSize != movRet) {
		_LOG(dfLOG_LEVEL_ERROR, L" ID[%lld] :: transferredSize[%d] != movRet[%d]", pSession->_sessionID, transferredSize, movRet);
		CRASH();
	}
	int msgCount = transferredSize / dfMESSAGE_SIZE;
	g_monitoring.RecvMsgAdd(msgCount);
	int msgByte = transferredSize;
	CPacket recvPacket;
	WORD header;
	while (true) {
		if (msgByte < sizeof(header))
			break;

		recvPacket.Clear();

		int peekRet = pSession->_recvQueue.Peek((char *) &header, sizeof(header));
		if (peekRet != sizeof(header)) {
			_LOG(dfLOG_LEVEL_DEBUG, L"less recv ringbuffer - header");
			break;
		}


		if (msgByte < header + sizeof(header)) {
			_LOG(dfLOG_LEVEL_DEBUG, L"less recv ringbuffer - payload");
			break;
		}
		int deqRet = pSession->_recvQueue.Dequeue(recvPacket.GetBufferPtr(), header + sizeof(header));
		if (deqRet != header + sizeof(header)) {
			_LOG(dfLOG_LEVEL_ERROR, L"ID[%lld] :: Recv Queue Use Size[%d] deqRet[%d]", pSession->_sessionID, pSession->_recvQueue.GetUseSize(), deqRet);
			break;
		}

		int moveWriteRet = recvPacket.MoveWritePos(deqRet);
		if(moveWriteRet != deqRet){
			_LOG(dfLOG_LEVEL_ERROR, L"ID[%lld] :: moveWriteRet Size[%d] deqRet[%d]", pSession->_sessionID, deqRet, moveWriteRet);
			break;
		}
		msgByte -= 10;

		// sendpacket();
		int sendsize = recvPacket.GetDataSize();
		int enqRet = pSession->_sendQueue.Enqueue(recvPacket.GetBufferPtr(), sendsize);
		if (enqRet != sendsize) {
			_LOG(dfLOG_LEVEL_ERROR, L"ID[%lld] :: Send Queue Use Size[%d] deqRet[%d]", pSession->_sessionID, pSession->_sendQueue.GetUseSize(), deqRet);
			CRASH();
		}
	}

	// SendPost
	if (pSession->_sendQueue.GetUseSize() > 0) {
		if (SendPost(pSession, dfLOGIC_FROM_CPMPLETE_RECV + 100) == FALSE) {
		}
	}

	// RecvPost
	return RecvPost(pSession, dfLOGIC_FROM_CPMPLETE_RECV + 200);
}

bool SendPost(CSession *pSession, int logic) {
	BOOL isSend = InterlockedExchange8((CHAR *) &pSession->_isSend, TRUE);
	if (isSend == TRUE) {
		return FALSE;
	}
	if (pSession->_sendQueue.GetUseSize() == 0) {
		if (InterlockedExchange8((CHAR *) &pSession->_isSend, FALSE) == FALSE) {
			_LOG(dfLOG_LEVEL_ERROR, L"SendPost(%d)", logic);
			CRASH();
		}
	}

	WSABUF bufferSet[2]; // 0 : 현재, 1 : 링버퍼 처음
	DWORD byteSends;

	SetWSABuffer(bufferSet, pSession, FALSE, logic + 1);

	// IO 카운트
	IncrementIOCount(pSession, logic + 1);

	// 오버랩 초기화
	memset(&pSession->_sendOverlapped, 0, sizeof(pSession->_sendOverlapped));
	// WSASend()
	int sendRet = WSASend(pSession->_sock, bufferSet, 2, &byteSends, 0, &pSession->_sendOverlapped, nullptr);
	if (sendRet == SOCKET_ERROR) {
		int err = WSAGetLastError();

		if (err != WSA_IO_PENDING) {
			if (err != 10054 && err != 10053)
				_LOG(dfLOG_LEVEL_ERROR, L"//// WSASend(%d) ERROR [%d]", logic, err);
			// 디스커넥트
			DecrementIOCount(pSession, logic + 1);
		}
		// 비동기로 잘들어감
		//_LOG(dfLOG_LEVEL_ERROR, L"//// WSASend WSA_IO_PENDING [%d]", err);
	}
	// 동기로 완료
	g_monitoring.SendCheck();
	return true;
}

bool RecvPost(CSession *pSession, int logic, bool isAccept) {
	WSABUF bufferSet[2]; // 0 : 현재, 1 : 링버퍼 처음
	DWORD flag = 0;
	DWORD byteRecvs;

	SetWSABuffer(bufferSet, pSession, TRUE, logic + 1);

	if (isAccept)
		bufferSet[1].len = 0;

	// IO 카운트
	IncrementIOCount(pSession, logic + 1);

	// 오버랩 초기화
	memset(&pSession->_recvOverlapped, 0, sizeof(pSession->_recvOverlapped));
	//WSARecv()
	int recvRet = WSARecv(pSession->_sock, bufferSet, 2, &byteRecvs, &flag, &pSession->_recvOverlapped, nullptr);
	if (recvRet == SOCKET_ERROR) {
		int err = WSAGetLastError();

		if (err != WSA_IO_PENDING) {
			if (err != 10054 && err != 10053)
				_LOG(dfLOG_LEVEL_ERROR, L"//// WSARecv ERROR [%d]", err);
			// 디스커넥트
			DecrementIOCount(pSession, logic + 1);
			return false;
		}
	}
	// 동기로 완료
	g_monitoring.RecvCheck();
	return true;
}

bool SetWSABuffer(WSABUF *pBufSet, CSession *pSession, bool isRecv, int logic) {
	if (isRecv) {
		char *pBuf = pSession->_recvQueue.GetBufferPtr();
		char *pRear = pSession->_recvQueue.GetRearBufferPtr();
		int  enSize = pSession->_recvQueue.DirectEnqueueSize();
		int  frSize = pSession->_recvQueue.GetFreeSize();
		// recv버퍼에 넣기
		pBufSet[0].buf = pRear;
		pBufSet[0].len = enSize;
		pBufSet[1].buf = pBuf;
		pBufSet[1].len = frSize - enSize;
		if (pBufSet[0].len + pBufSet[1].len > frSize) {
			_LOG(dfLOG_LEVEL_ERROR, L"SetWSABuffer(%d) :: pBufSet[0].len + pBufSet[1].len != FreeSize", logic);
			CRASH();
		}
	} else {
		// send버퍼에서 빼가기
		char *pBuf = pSession->_recvQueue.GetBufferPtr();
		char *pFront = pSession->_sendQueue.GetFrontBufferPtr();

		int  deSize = pSession->_sendQueue.DirectDequeueSize();
		int usSize = pSession->_sendQueue.GetUseSize();

		pBufSet[0].buf = pFront;
		pBufSet[0].len = deSize;
		pBufSet[1].buf = pBuf;
		pBufSet[1].len = usSize - deSize;
		if (pBufSet[0].len + pBufSet[1].len > usSize) {
			_LOG(dfLOG_LEVEL_ERROR, L"SetWSABuffer(%d) :: pBufSet[0].len + pBufSet[1].len != UseSize", logic);
			CRASH();
		}
	}

	return true;
}

bool SendPacket(CSession *pSession, CPacket *packet) {

	return true;
}

u_int64 g_debugTargetID = 0;

bool IncrementIOCount(CSession *pSession, int logic) {
	if (pSession == NULL) return false;
	DWORD retval = InterlockedIncrement(&pSession->_IOcount);
	_LOG(dfLOG_LEVEL_DEBUG, L"IncrementIOCount() ID[%lld] :: logic[%d]", pSession->_sessionID, logic);
	if (g_LogLevel <= dfLOG_LEVEL_WARNING) {
		if (retval > 4 && g_debugTargetID == 0) {
			g_debugTargetID = pSession->_sessionID;
			_LOG(dfLOG_LEVEL_WARNING, L"IncrementIOCount(%d) targeting ID[%lld] :: RecvQ Use Size [%d] SendQ Use Size[%d] IOCount[%d :: %d] isSend[%d]", logic, pSession->_sessionID, pSession->_recvQueue.GetUseSize(), pSession->_sendQueue.GetUseSize(), retval, pSession->_IOcount, pSession->_isSend);
		}
		if (g_debugTargetID == pSession->_sessionID) {
			_LOG(dfLOG_LEVEL_WARNING, L"IncrementIOCount(%d) ID[%lld] :: RecvQ Use Size [%d] SendQ Use Size[%d] IOCount[%d :: %d] isSend[%d]", logic, pSession->_sessionID,pSession->_recvQueue.GetUseSize(), pSession->_sendQueue.GetUseSize(), retval, pSession->_IOcount, pSession->_isSend);
		}

	}
	return true;
}

bool DecrementIOCount(CSession *pSession, int logic) {
	if (pSession == NULL) return false;
	DWORD retval = InterlockedDecrement(&pSession->_IOcount);
	_LOG(dfLOG_LEVEL_DEBUG, L"DecrementIOCount(SID[%lld]  logic[%d])", pSession->_sessionID, logic);
	if (g_LogLevel <= dfLOG_LEVEL_WARNING) {
		if (g_debugTargetID == pSession->_sessionID) {
			_LOG(dfLOG_LEVEL_WARNING, L"DecrementIOCount(%d) ID[%lld] :: RecvQ Use Size [%d] SendQ Use Size[%d] IOCount[%d :: %d] isSend[%d]", logic, pSession->_sessionID, pSession->_recvQueue.GetUseSize(), pSession->_sendQueue.GetUseSize(), retval, pSession->_IOcount, pSession->_isSend);
		}
	}
	if (retval > 5) {
		_LOG(dfLOG_LEVEL_DEBUG,
			L"DecrementIOCount(%d) pSession->_IOcount [%d]\n\
SOCK[%d] :: recv [%d]byte, send [%d]byte, IOCount[%d], isSend [%d]",
logic, pSession->_IOcount, pSession->_sock, pSession->_recvQueue.GetUseSize(), pSession->_sendQueue.GetUseSize(), pSession->_IOcount, pSession->_isSend);
		//CRASH();
	}
	if (retval == 0) {
		Disconnect(pSession, logic + 1);
		return false;
	}
	if (retval < 0) {
		_LOG(dfLOG_LEVEL_ERROR,
			L"DecrementIOCount(%d) pSession->_IOcount [%d]\n\
SOCK[%d] :: recv [%d]byte, send [%d]byte, IOCount[%d], isSend [%d]",
logic, pSession->_IOcount, pSession->_sock, pSession->_recvQueue.GetUseSize(), pSession->_sendQueue.GetUseSize(), pSession->_IOcount, pSession->_isSend);
		CRASH();
	}

	return true;
}

bool Disconnect(CSession *pSession, int logic) {
	if (pSession == NULL) return false;
	// 모니터링
	g_monitoring.DecrementSessionCount();

	_LOG(dfLOG_LEVEL_DEBUG, L"//// Disconnect socket[%d]", pSession->_sock);

	if (g_LogLevel <= dfLOG_LEVEL_WARNING) {
		if (g_debugTargetID == pSession->_sessionID) {
			_LOG(dfLOG_LEVEL_WARNING, L"Release (%d) ID[%lld]", logic, pSession->_sessionID);
			g_debugTargetID = 0;
		}
	}
	// 컨테이너 에서 삭제
	if (DeleteSession(pSession->_sessionID) == false) {
		_LOG(dfLOG_LEVEL_ERROR, L"Disconnect ID[%lld], logic[%d]", pSession->_sessionID, logic);

		return false;
	}

	// 소켓 끊기
	closesocket(pSession->_sock);

	// 메모리 프리
	ReleaseSession(pSession);

	return true;
}

bool Release(CSession *pSession) {
	return false;
}
