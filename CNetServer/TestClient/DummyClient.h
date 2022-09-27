#pragma once
#include "CClient.h"
#include "CParser.h"

struct ClientSession {
	SESSION_ID _ID;
	ULONGLONG _data;
	bool _isLogin;
};
class DummyClient : public CClient {
public:
	DummyClient(const WCHAR *configFilePath);
	~DummyClient();

	void Connect();
private:
	// 가상
	virtual void OnEnterServer(SESSION_ID sessionID);//< 서버와의 연결 성공 후
	virtual void OnLeaveServer(SESSION_ID sessionID); //< 서버와의 연결이 끊어졌을 때
	virtual void OnRecv(SESSION_ID sessionID, Packet *pPacket);//< 패킷 수신 완료 후
	virtual void OnSend(SESSION_ID sessionID); //< 패킷 송신 완료 후
	virtual void OnError(int errorcode, const WCHAR *);//< 에러났을때 // TODO errorcode
	virtual void OnMonitoringPerSec() {};

private:
	void PacketProc(Packet *pPacket, SESSION_ID sessionID, WORD type);


private:
	// Client Management
	void InsertClient(SESSION_ID sessionID, ClientSession *pClient);
	void RemoveClient(SESSION_ID sessionID);
	ClientSession *FindClient(SESSION_ID sessionID);


private:
	unordered_map<SESSION_ID, ClientSession *> _clientMap;
	ObjectPool<ClientSession> _clientPool;

	CParser *_pConfigData;

	WCHAR _serverIP[20];
	int _serverPort;
};

