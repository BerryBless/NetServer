#pragma once
#include "CLanClient.h"

class CEchoClient : public CLanClient {
public:
	CEchoClient();
	~CEchoClient();
private:
	virtual void OnEnterJoinServer();
	virtual void OnLeaveServer();
	virtual void OnRecv(CPacket *);
	virtual void OnSend(int sendsize);
	virtual void OnError(int errorcode, const WCHAR *);

private:
	int _cnt = 0;
};

