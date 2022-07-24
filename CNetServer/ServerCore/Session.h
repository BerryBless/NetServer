#pragma once
#include "CorePch.h"

typedef u_int64 SESSION_ID;
struct SESSION {
	SESSION_ID _ID;

	// IOCP Buffer
	WSAOVERLAPPED _recvOverlapped;
	WSAOVERLAPPED _sendOverlapped;
	CRingBuffer _recvQueue;
	Queue<CPacket *> _sendQueue;



	// session information
	SOCKET _sock;
	ULONG _IP;
	USHORT _port;

	// session lock
	SRWLOCK _lock;


	// session state
	DWORD _lastRecvdTime;
	alignas(64) DWORD _IOcount;
	alignas(64) DWORD _IOFlag;
	alignas(64) DWORD _sendPacketCnt;
	SESSION() {
		_ID = 0;
		_IOcount = 0;
		_IOFlag = 0;
		_sendPacketCnt = 0;
		_sock = 0;
		_IP = 0;
		_port = 0;
		ZeroMemory(&_recvOverlapped, sizeof(WSAOVERLAPPED));
		ZeroMemory(&_sendOverlapped, sizeof(WSAOVERLAPPED));
	}
};