#pragma once
#include "CLanClient.h"
//#include "CNetClient.h"
#define df_PACKET_CODE 0x73
#define df_SENDOVER 100
class CEchoClient : public CLanClient {
//class CEchoClient : public CNetClient {
public:
	struct Protocol {
		BYTE _code; // 0x73
		DWORD _cnt;
		DWORD _sendTime;
		BOOL _recved;
	};

public:
	CEchoClient();
	~CEchoClient();
	void Test();
private:
	virtual void OnEnterJoinServer();
	virtual void OnLeaveServer();
	virtual void OnRecv(CPacket *);
	virtual void OnSend(int sendsize);
	virtual void OnError(int errorcode, const WCHAR *);

	void sendTest();
	void recvTest();
	void recvCheck(CPacket *);

private:
	DWORD _login = 0;
	DWORD _cnt = 0;

	bool _wait;

	DWORD _totalRTT =0 ;
	DWORD _countRTT =0;

	Protocol _testPacket[df_SENDOVER];
};

