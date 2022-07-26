#pragma once
#include "CRingBuffer.h"
#include <Windows.h>
class CSession {
public:
	SOCKET _sock;
	DWORD _SID; // 세션 ID
	CRingBuffer _recvQ;	// 수신 큐
	CRingBuffer _sendQ; // 송신 큐
	DWORD _LastRecvTime; // 마지막으로 송신한 시간
public:
	~CSession();
};
