#pragma once
#include "Define.h"
#include "Queue.hpp"
#include "SerializingBuffer.h"
#include "RingBuffer.h"
#define df_LOGGING_SESSION_LOGIC 1000
#define dfSESSION_SEND_PACKER_BUFFER_SIZE 200

struct SESSION {
	SESSION_ID _ID;

	// IOCP Buffer
	WSAOVERLAPPED _recvOverlapped;
	WSAOVERLAPPED _sendOverlapped;
	RingBuffer _recvQueue;
	Queue<Packet *> _sendQueue;
	Packet *_pSendPacketBufs[dfSESSION_SEND_PACKER_BUFFER_SIZE];

	// session information
	SOCKET _sock;
	DWORD _IP;
	USHORT _port;
	WCHAR _IPStr[20];

	// session lock
	SRWLOCK _lock;

	// session state
	DWORD _lastActiveTime;
	alignas(64) DWORD _IOcount;
	alignas(64) DWORD _IOFlag;
	alignas(64) DWORD _sendPacketCnt;
	alignas(64) DWORD _isAlive;


	SESSION() {
		_ID = 0;
		_IOcount = 0x80000000;
		_IOFlag = 0;
		_sendPacketCnt = 0;
		_sock = 0;
		_IP = 0;
		_port = 0;
		_isAlive = 0;
		ZeroMemory(&_recvOverlapped, sizeof(WSAOVERLAPPED));
		ZeroMemory(&_sendOverlapped, sizeof(WSAOVERLAPPED));
	}
};