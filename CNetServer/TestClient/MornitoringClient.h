#pragma once
#include "CClient.h"
class MornitoringClient : public CClient {
	struct Client {
		SESSION_ID _ID;
		ULONGLONG _data;
		bool _isLogin;
	};

public:
	MornitoringClient();
	~MornitoringClient();

	void ConnectMonitor();
private:
	// 가상
	virtual void OnEnterServer(SESSION_ID sessionID);//< 서버와의 연결 성공 후
	virtual void OnLeaveServer(SESSION_ID sessionID); //< 서버와의 연결이 끊어졌을 때
	virtual void OnRecv(SESSION_ID sessionID, Packet *pPacket);//< 패킷 수신 완료 후
	virtual void OnSend(SESSION_ID sessionID) {} //< 패킷 송신 완료 후
	virtual void OnError(int errorcode, const WCHAR *) {}//< 에러났을때 // TODO errorcode

private:
	// Client Management
	void InsertClient(SESSION_ID sessionID, Client *pClient);
	void RemoveClient(SESSION_ID sessionID);
	Client *FindClient(SESSION_ID sessionID);

private:
	unordered_map<SESSION_ID, Client *> _ClientMap;
};

