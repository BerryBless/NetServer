#pragma once
#include "CRingBuffer.h"
#include "Protocol.h"
#include <string>
#define df_LOBBY -1
class CClient {
private:
public:
	SOCKET _sock;		// 클라이언트 소켓 
	SOCKADDR_IN _Addr;		// 연결정보

	DWORD _ID;			// ID
	DWORD _EnterRoomID;	// 입장한 룸번호

	CRingBuffer _recvQ;	// 수신 링버퍼
	CRingBuffer _sendQ;	// 송신 링버퍼

	std::wstring _username; // 유저 닉네임

public:
	void Init(DWORD , SOCKET , SOCKADDR_IN);
};